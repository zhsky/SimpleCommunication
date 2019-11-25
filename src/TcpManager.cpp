/*
* --*TcpManager*--
* @Author: Payton
* @Last  : Payton
*/

#include <TcpManager.h>
#include <EventManager.h>
#include <Timestamp.h>
#include <Log.h>
#include <TaskThread.h>
#include <ThreadPool.h>
#include <ThreadMutex.h>

using std::placeholders::_1;
using std::placeholders::_2;
namespace sc
{

TcpManager::TcpManager():init_(false),running_(false),event_size_(0),fd_key_(0),message_callback_(nullptr),
	acceptor_index_(0),acceptor_loop_(nullptr),socket_loop_(nullptr),packer_thread_(nullptr),main_thread_(nullptr)
{
	pthread_rwlockattr_t attr;
	pthread_rwlockattr_init(&attr);
	pthread_rwlockattr_setkind_np(&attr, PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP);
	::pthread_rwlock_init(&rwlock_, &attr);
	::pthread_rwlock_init(&map_rwlock_, &attr);
	buffer_pool_.free_all();
	client_pool_.free_all();
	handle_pool_.free_all();
	client_map_.clear();
}

TcpManager::~TcpManager()
{
	this->stop();
	pthread_rwlock_destroy(&rwlock_);
	pthread_rwlock_destroy(&map_rwlock_);
	if(init_)
	{
		delete acceptor_loop_;
		acceptor_loop_ = 0;
		delete[] socket_loop_;
		socket_loop_ = 0;
		delete main_thread_;
		delete packer_thread_;
		init_ = false;
	}
}

int TcpManager::init(size_t event_size)
{
	if(init_)
	{
		LOG_ERROR("ERROR HAD INIT");
		return -1;
	}
	init_ = true;
	event_size_ = event_size > 0 ? event_size : 1;

	acceptor_loop_ = new EventManager();
	socket_loop_ = new EventManager[event_size_]();
	main_thread_ = new TaskThread();
	packer_thread_ = new ThreadPool();
	return 0;
}

int TcpManager::start()
{
	if(!init_)
	{
		LOG_ERROR("ERROR NOT INIT");
		abort();
	}
	if(running_)
	{
		LOG_ERROR("ERROR RENNING");
		return -1;
	}
	running_ = true;

	fd_key_ = 0;
	this->start_send_timer();
	acceptor_loop_->start_loop();
	for(uint32_t i = 0; i < event_size_; ++i)
	{
		socket_loop_[i].start_loop();
	}

	packer_thread_->start(16);
	this->main_thread_->start();
	LOG_INFO("TcpManager start");
	return 0;
}

void TcpManager::stop()
{
	if(!running_) return;
	running_ = false;
	this->main_thread_->stop();

	acceptor_loop_->stop_loop();
	for(uint32_t i = 0; i < event_size_; ++i)
		socket_loop_[i].stop_loop();
	acceptor_index_ = 0;
	packer_thread_->stop();

	this->client_map_.clear();
	this->connect_map_.clear();
	buffer_pool_.free_all();
	client_pool_.free_all();
	handle_pool_.free_all();
	LOG_INFO("TcpManager stop");
}


void TcpManager::pool_dump()
{
	this->buffer_pool_.dump_info();
	this->client_pool_.dump_info();
	this->handle_pool_.dump_info();
}

int TcpManager::add_acceptor(EventHandle* handle)
{
	return acceptor_loop_->add_handle(handle);
}

void TcpManager::add_client(ClientEntity* client)
{
	auto fun = std::bind(&TcpManager::do_add_client,this,client);
	this->main_thread_->run_in_thread(fun);
}

void TcpManager::do_add_client(ClientEntity* client)
{
	this->main_thread_->assert_in_thread();
	uint32_t index = event_index();
	client->set_loop_index(index);
	client->set_event_loop(&socket_loop_[index]);
	if(message_callback_ != nullptr) client->set_msg_callback(message_callback_);

	{
		WGUARD_LOCK(__guard_lock,rwlock_);
		this->client_map_[client->fd()] = client;
	}

	EventHandle* handle = this->pop_handle();
	handle->set_recycle_func(std::bind(&TcpManager::push_handle,this,::_1));
	handle->bind(std::bind(&ClientEntity::handle_event,client,::_1,::_2),client->fd(),"client");
	socket_loop_[index].add_handle(handle);
	if(client->established_callback() != nullptr){client->established_callback()(client->fd_key());}
}

void TcpManager::close_client(uint64_t fd_key)
{
	int fd = this->get_fd(fd_key);
	if(fd == 0)
	{
		LOG_WARNING("WARNING fd_key NOT EXIST,%d",fd_key);
		return;
	}
	ClientEntity* client = this->get_client(fd);
	if(client != nullptr) client->close_read();
	this->remove_fd(fd_key);
}

void TcpManager::remove_client(int fd)
{
	auto fun = std::bind(&TcpManager::do_remove_client,this,fd);
	this->main_thread_->run_in_thread(fun);
}
void TcpManager::do_remove_client(int fd)
{
	this->main_thread_->assert_in_thread();
	auto iter = this->client_map_.find(fd);
	if(iter != this->client_map_.end())
	{
		ClientEntity* client = iter->second;
		{
			WGUARD_LOCK(__guard_lock,rwlock_);
			this->client_map_.erase(iter);
		}
		this->push_client(client);
		LOG_INFO("remove client fd:%ld",fd);
	}
	else
	{
		LOG_ERROR("ERROR fd:%d NOT EXIST",fd);
		return;
	}

}

void TcpManager::start_send_timer()
{
	EventHandle* timer_handler = this->pop_handle();

	auto fun = std::bind(&TcpManager::send_timeout,this,::_1);
	timer_handler->bind(
		fun,
		long_unixstamp(),
		1000,
		"send_timer"
	);
	if(timer_handler->fd() > 0)
	{
		LOG_INFO("add send_timer");
		timer_handler->set_recycle_func(std::bind(&TcpManager::push_handle,this,::_1));
		acceptor_loop_->add_handle(timer_handler);
	}
	else
	{
		this->push_handle(timer_handler);
	}
}

void TcpManager::send_timeout(int64_t)
{
	this->main_thread_->run_in_thread(std::bind(&TcpManager::do_send_timeout,this));
}
void TcpManager::do_send_timeout()
{
	this->main_thread_->assert_in_thread();
	static std::list< std::function<void()> > fun_list;

	fun_list.clear();
	for(auto& [_,client] : client_map_)
	{
		if(client->is_close()) continue;
		if(client->can_close())
		{
			client->set_close();
			fun_list.emplace_back(std::bind(&ClientEntity::req_close,client));
			continue;
		}
		if(client->is_update() && client->can_read())
		{
			client->handle_update();
			fun_list.emplace_back(std::bind(&ClientEntity::handle_unpack,client));
		}
		if(client->is_write_update() && client->can_write())
		{
			client->handle_write_update();
			fun_list.emplace_back(std::bind(&ClientEntity::timeout_send,client));
		}
	}

	if(fun_list.empty() == false)
		packer_thread_->accept_task_list(fun_list);
}

ClientEntity* TcpManager::get_client(int fd)
{
	RGUARD_LOCK(__guard_lock,rwlock_);
	auto iter = this->client_map_.find(fd);
	if(iter == this->client_map_.end()) return nullptr;
	return iter->second;
}

int TcpManager::push_send_pack(Buffer* buff)
{
	uint64_t fd_key = 0;
	buff->readInt64(fd_key);
	int fd = this->get_fd(fd_key);
	if(fd == 0)
	{
		LOG_ERROR("ERROR fd_key:%d NOT EXIST",fd_key);
		return -1;
	}
	ClientEntity* client = this->get_client(fd);
	if(client != nullptr) return client->push_send_pack(buff);
	return -1;
}

int TcpManager::get_fd(uint64_t key)
{
	RGUARD_LOCK(__guard_lock,map_rwlock_);
	auto iter = this->connect_map_.find(key);
	if(iter == this->connect_map_.end())
		return 0;
	return iter->second;
}

uint64_t TcpManager::insert_fd(int fd)
{
	uint64_t key = __sync_add_and_fetch(&fd_key_,1);
	WGUARD_LOCK(__guard_lock,map_rwlock_);
	this->connect_map_[key] = fd;
	return key;
}

void TcpManager::remove_fd(uint64_t key)
{
	WGUARD_LOCK(__guard_lock,map_rwlock_);
	this->connect_map_.erase(key);
}

}