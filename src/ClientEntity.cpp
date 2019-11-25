/*
* --*ClientEntity*--
* @Author: Payton
* @Last  : Payton
*/

#include <ClientEntity.h>
#include <TcpManager.h>
#include <EventManager.h>
#include <Log.h>

#include <assert.h>
#include <sys/epoll.h>
#include <errno.h>
#include <unistd.h>
#include <sys/uio.h>

namespace sc
{

ClientEntity::ClientEntity():read_flag_(0),close_flag_(0),close_(false),write_flag_(0),fd_(0),
	addr_port_(0),message_callback_(nullptr),established_callback_(nullptr),loop_index_(0),
	event_loop_(nullptr),tcp_manager_(nullptr),cur_unpcak_(nullptr),front_unpcak_(nullptr),wait_write_(false)
{
	memset(addr_ip_,0,128);
	unpack_list_.clear();
	recv_list_.clear();
	wait_pack_list_.clear();
	wait_write_list_.clear();
}

ClientEntity::~ClientEntity()
{
	::close(fd_);
	this->clear();
	loop_index_ = addr_port_ = fd_ = close_flag_ = read_flag_ = write_flag_ = 0;
	wait_write_ = close_ = false;
	front_unpcak_ = nullptr;
	cur_unpcak_ = nullptr;
	tcp_manager_ = nullptr;
	event_loop_ = nullptr;
	message_callback_ = nullptr;
	established_callback_ = nullptr;
	event_loop_ = nullptr;
}

void ClientEntity::clear()
{
	for(auto& buff : unpack_list_)
	{
		this->tcp_manager_->push_buffer(buff);
	}
	this->unpack_list_.clear();

	{
		GUARD_LOCK(_guard_lock1,read_mutex_);
		for(auto& buff : recv_list_)
		{
			this->tcp_manager_->push_buffer(buff);
		}
		this->recv_list_.clear();
	}
	
	{
		GUARD_LOCK(_guard_lock2,write_mutex_);
		for(auto& buff : wait_pack_list_)
		{
			this->tcp_manager_->push_buffer(buff);
		}
		this->wait_pack_list_.clear();
	}

	for(auto& buff : wait_write_list_)
	{
		this->tcp_manager_->push_buffer(buff);
	}
	this->wait_write_list_.clear();
}


void ClientEntity::init(int fd,const char* addr_ip,uint16_t addr_port)
{
	memset(this->addr_ip_,0,128);
	this->fd_ = fd;
	strncpy(this->addr_ip_,addr_ip,127);
	this->addr_port_ = addr_port;
}

void ClientEntity::req_close()
{
	this->event_loop_->run_in_handle_loop(std::bind(&ClientEntity::close,this));
}

void ClientEntity::close()
{
	//event loop thread
	this->event_loop_->do_remove_handle(this->fd_);
	this->tcp_manager_->remove_client(this->fd_);
}

void ClientEntity::handle_event(int fd, uint32_t events)
{
	if(fd != fd_)
	{
		LOG_ERROR("LOG_ERROR,fd:%d,fd_:%d",fd,fd_);
	}
	assert(fd == fd_);
	if(this->ready_close()) return;

	if(EPOLLIN & events && this->read_data() < 0)
	{
		this->watch_read(0);
		return;
	}
	
	if(EPOLLOUT & events)
	{
		this->watch_write(0);
		this->write_data();
		return;
	}
}

int ClientEntity::read_data()
{
	Buffer* buff = this->tcp_manager_->pop_buffer();
	int readn = 0;
	while(true)
	{
		buff->ensureWriteable(8 * 1024);
		readn = read(this->fd_,buff->writeptr(),buff->writeableBytes());
		if(readn < 0)
		{
			if(errno == EINTR) 
				continue;
			else if(errno == EAGAIN) 
				break;

			LOG_ERROR("ERROR socket fd:%d, read %s",fd_,strerror(errno));
			this->tcp_manager_->push_buffer(buff);
			this->close_write();
			this->set_update();
			return -1;
		}
		else if(readn == 0)
		{
			LOG_INFO("fd:%d read 0, close",this->fd_);
			this->close_write();
			this->set_update();
			break;
		}
		else
		{
			buff->set_write_index(buff->get_write_index() + readn);
		}
	}

	if(buff->readableBytes() > 0)
	{
		GUARD_LOCK(_guard_lock,read_mutex_);
		this->recv_list_.push_back(buff);
		this->set_update();
	}
	else
	{
		this->tcp_manager_->push_buffer(buff);
	}
	return readn == 0 ? -1 : 0;
}

//in packer thread
void ClientEntity::handle_unpack()
{
	GUARD_LOCK(_guard_lock,unpack_mutex_);
	if(this->is_close()) return;
	do_handle_unpack();
}
//|4byte length + data|
void ClientEntity::do_handle_unpack()
{
	{
		GUARD_LOCK(_guard_lock,read_mutex_);
		if(this->recv_list_.empty() && this->unpack_list_.empty())
		{
			this->finish_update();
			if(this->ready_close()) this->force_close();
			return;
		}
		this->finish_update();
		this->unpack_list_.splice(this->unpack_list_.end(),this->recv_list_);
	}

	while(this->unpack_list_.empty() == false)
	{
		if(cur_unpcak_ == nullptr)
		{
			cur_unpcak_ = this->tcp_manager_->pop_buffer();
			cur_unpcak_->writeInt64(this->fd_key_);
			cur_unpcak_->set_read_index(sizeof(uint64_t));
		}

		if(front_unpcak_ == nullptr)
		{
			front_unpcak_ = this->unpack_list_.front();
		}

		int32_t len = 0;
		if(cur_unpcak_->readableBytes() >= sizeof(int32_t))
			cur_unpcak_->peekInt32(len);
		else
		{
			int32_t need_len_len = sizeof(int32_t) - cur_unpcak_->readableBytes();

			if(front_unpcak_->readableBytes() >= (size_t)need_len_len)
			{
				cur_unpcak_->append(front_unpcak_->readptr(),need_len_len);
				front_unpcak_->set_read_index(front_unpcak_->get_read_index() + need_len_len);
				cur_unpcak_->peekInt32(len);
			}
			else
			{
				//等待完整的len数值
				cur_unpcak_->append(front_unpcak_->readptr(),front_unpcak_->readableBytes());
				this->tcp_manager_->push_buffer(front_unpcak_);
				this->unpack_list_.pop_front();
				front_unpcak_ = nullptr;
				continue;
			}
		}

		if(len <= 0)
		{
			LOG_ERROR("ERROR recv len 0, fd:%d",fd_);
			this->clear();
			this->close_read();
			this->force_close();
			return;
		}

		int32_t need_data_len = len - (cur_unpcak_->readableBytes() - sizeof(int32_t));
		if(front_unpcak_->readableBytes() >= (size_t)need_data_len)
		{
			cur_unpcak_->append(front_unpcak_->readptr(),need_data_len);
			front_unpcak_->set_read_index(front_unpcak_->get_read_index() + need_data_len);

			cur_unpcak_->set_read_index(0);
			this->finish_unpack(cur_unpcak_);
			cur_unpcak_ = nullptr;
			if(front_unpcak_->readableBytes() == 0)
			{
				this->tcp_manager_->push_buffer(front_unpcak_);
				this->unpack_list_.pop_front();
				front_unpcak_ = nullptr;
			}
			this->set_update();
			return;
		}

		cur_unpcak_->append(front_unpcak_->readptr(),front_unpcak_->readableBytes());
		this->tcp_manager_->push_buffer(front_unpcak_);
		this->unpack_list_.pop_front();
		front_unpcak_ = nullptr;
	}
	if(this->ready_close()) this->force_close();
}

void ClientEntity::finish_unpack(Buffer* data)
{
	GUARD_LOCK(_guard_lock,callback_mutex_);
	message_callback_(data);
	this->tcp_manager_->push_buffer(data);
}

int ClientEntity::push_send_pack(Buffer* buff)
{
	if(this->ready_close()) return -1;
	Buffer* tmp = this->tcp_manager_->pop_buffer();
	tmp->append(buff->readptr(),buff->readableBytes());
	GUARD_LOCK(_guard_lock,write_mutex_);
	this->wait_pack_list_.push_back(tmp);
	this->set_write_update();
	return 0;
}

void ClientEntity::timeout_send()
{
	if(this->wait_write_) return;
	this->write_data();
}

int ClientEntity::write_data()
{
	if(send_mutex_.try_lock() == false) return 0;
	int ret = do_write_data();
	send_mutex_.unlock();
	return ret;
}

int ClientEntity::do_write_data()
{
	{
		GUARD_LOCK(_guard_lock,write_mutex_);
		if(this->wait_write_list_.empty() && this->wait_pack_list_.empty())
		{
			this->finish_write_update();
			if(this->ready_close()) this->force_close();
			return 0;
		}
		this->finish_write_update();
		this->wait_write_list_.splice(this->wait_write_list_.end(),this->wait_pack_list_);
	}

	std::vector<iovec> iov_vec;
	struct iovec data;
	int32_t total_bytes = 0;
	for(auto& packer : this->wait_write_list_)
	{
		data.iov_base = packer->readptr();
		data.iov_len = packer->readableBytes();
		iov_vec.push_back(data);
		total_bytes += data.iov_len;
	}

	int32_t send_bytes = writev(this->fd_,&(iov_vec[0]),iov_vec.size());
	if(send_bytes < 0)
	{
		if(errno == EINTR || errno == EAGAIN)
			return 0;
		
		LOG_ERROR("ERROR socket fd:%d, writev %s",fd_,strerror(errno));
		this->close_write();
		this->set_update();
		return 0;
	}
	if(send_bytes < total_bytes)
	{
		this->watch_write(1);
	}

	while(send_bytes > 0 && this->wait_write_list_.empty() == false)
	{
		Buffer* buff = this->wait_write_list_.front();
		int32_t readableBytes = buff->readableBytes();
		if(send_bytes >= readableBytes)
		{
			send_bytes -= readableBytes;
			this->wait_write_list_.pop_front();
			this->tcp_manager_->push_buffer(buff);
			continue;
		}
		else
		{
			buff->set_read_index(buff->get_read_index() + send_bytes);
			send_bytes = 0;
			break;
		}
	}

	if(this->ready_close()) this->force_close();
	return 0;
}


void ClientEntity::watch_write(int opt)
{
	EventHandle* handle = this->event_loop_->get_handle(this->fd_);
	if(handle == nullptr) return;
	if(opt == 1)
	{
		int flag = handle->event_flag();
		flag |= EPOLLOUT;
		handle->set_event_flag(flag);
		wait_write_ = true;
	}
	else
	{
		int flag = handle->event_flag();
		flag &= ~EPOLLOUT;
		handle->set_event_flag(flag);
		wait_write_ = false;
	}
	this->event_loop_->update_handle(this->fd_);
}

void ClientEntity::watch_read(int opt)
{
	EventHandle* handle = this->event_loop_->get_handle(this->fd_);
	if(handle == nullptr) return;
	if(opt == 1)
	{
		int flag = handle->event_flag();
		flag |= EPOLLIN;
		handle->set_event_flag(flag);
	}
	else
	{
		int flag = handle->event_flag();
		flag &= ~EPOLLIN;
		handle->set_event_flag(flag);
	}
	this->event_loop_->update_handle(this->fd_);
}

}//namespace sc