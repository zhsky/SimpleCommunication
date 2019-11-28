/*
* --*EventManager*--
* @Author: Payton
* @Last  : Payton
*/
#include <EventManager.h>
#include <Log.h>
#include <EventHandle.h>
#include <Timestamp.h>
#include <ThreadMutex.h>
#include <errno.h>
#include <cstring>
#include <unistd.h>

#define MAX_EVENT 1024
#define EVENT_TIMEOUT 1

namespace sc
{
	EventManager::EventManager():events_(MAX_EVENT),quit_(false)
	{
		pthread_rwlockattr_t attr;
		pthread_rwlockattr_init(&attr);
		pthread_rwlockattr_setkind_np(&attr, PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP);
		::pthread_rwlock_init(&rwlock_, &attr);
		if(epoll_fd_ = epoll_create1(EPOLL_CLOEXEC); epoll_fd_ < -1)
		{
			LOG_ERROR("epoll_create ERROR,%d,%s",errno,strerror(errno));
		}
	}

	EventManager::~EventManager()
	{
		this->stop_loop();
		pthread_rwlock_destroy(&rwlock_);
		::close(epoll_fd_);
		epoll_fd_= 0;
	}

	EventHandle* EventManager::get_handle(int fd)
	{
		RGUARD_LOCK(__guard_lock,rwlock_);
		if(auto iter = handler_map_.find(fd);iter != handler_map_.end())
			return iter->second;
		return nullptr;
	}

	void EventManager::update_handle(int fd)
	{
		this->run_in_handle_loop(std::bind(&EventManager::do_update_handle,this,fd));
	}
	
	int EventManager::do_update_handle(int fd)
	{
		EventHandle* handle_ptr = this->get_handle(fd);
		if(handle_ptr == nullptr) return -1;

		struct epoll_event event;
		memset(&event, 0, sizeof(event));

		event.events |= handle_ptr->event_flag();
		event.data.ptr = handle_ptr;

		if(::epoll_ctl(this->epoll_fd_,EPOLL_CTL_MOD,fd,&event) < 0)
		{
			LOG_ERROR("epoll_ctl ERROR EPOLL_CTL_MOD fd:%d, writev %s",fd,strerror(errno));
			return -1;
		}

		return 0;
	}

	int EventManager::add_handle(EventHandle* handle_ptr)
	{
		if(quit_ == true) return -1;
		if(handle_ptr->fd() <= 0) return -2;
		this->run_in_handle_loop(std::bind(&EventManager::do_add_handle,this,handle_ptr));
		return 0;
	}

	int EventManager::remove_handle(int fd)
	{
		if(quit_ == true) return -1;
		if(fd <= 0) return -2;
		this->run_in_handle_loop(std::bind(&EventManager::do_remove_handle,this,fd));
		return 0;
	}

	void EventManager::run_in_handle_loop(VOID_FUNC func)
	{
		GUARD_LOCK(_lock_guard,func_mutex_);
		this->loop_functions_write_.emplace_back(func);
	}

	void EventManager::do_add_handle(EventHandle* handle_ptr)
	{
		assert_in_thread();
		const std::string& event_name = handle_ptr->ename();
		int fd = handle_ptr->fd();
		if(handler_map_.find(handle_ptr->fd()) != handler_map_.end())
		{
			LOG_ERROR("REPEAT %ld,%d,%s",fd,handle_ptr->event_type(),event_name.c_str());
			return;
		}

		struct epoll_event event;
		memset(&event, 0, sizeof(event));

		event.events |= handle_ptr->event_flag();
		event.data.ptr = handle_ptr;

		{
			WGUARD_LOCK(__guard_lock,rwlock_);
			handler_map_[handle_ptr->fd()] = handle_ptr;
		}

		if(::epoll_ctl(this->epoll_fd_,EPOLL_CTL_ADD,fd,&event) < 0)
		{
			perror("epoll_ctl EPOLL_CTL_ADD:");
			LOG_ERROR("epoll_ctl ERROR EPOLL_CTL_ADD fd:%d, writev %s",fd,strerror(errno));
			{
				WGUARD_LOCK(__guard_lock,rwlock_);
				handler_map_.erase(fd);
			}
			return;
		}
		LOG_INFO("fd:%ld, name:%s",fd,event_name.c_str());
		return;
	}

	void EventManager::do_remove_handle(int fd)
	{
		assert_in_thread();
		EventHandle* handle_ptr = nullptr;
		
		if(auto iter = handler_map_.find(fd);iter == handler_map_.end())
		{
			LOG_ERROR("ERROR not found %ld",fd);
			return;
		}
		else
		{
			handle_ptr = iter->second;
			{
				WGUARD_LOCK(__guard_lock,rwlock_);
				handler_map_.erase(iter);
			}
		}

		if(::epoll_ctl(this->epoll_fd_,EPOLL_CTL_DEL,fd,nullptr) < 0)
		{
			LOG_ERROR("epoll_ctl ERROR EPOLL_CTL_DEL %ld,errno:%d,%s",fd,errno,strerror(errno));
			return;
		}
		LOG_INFO("fd:%ld, name:%s",fd,handle_ptr->ename().c_str());
		handle_ptr->recycle_self();
		return;
	}

	void EventManager::start_loop()
	{
		quit_ = false;
		this->loop_thread_.init_func(std::bind(&EventManager::loop,this),"event_loop_thread");
		this->loop_thread_.start();
	}

	void EventManager::stop_loop()
	{
		if(!quit_) this->ready_quit();
		for(auto& [fd,handle_ptr] : handler_map_)
		{
			::epoll_ctl(this->epoll_fd_,EPOLL_CTL_DEL,fd,nullptr);
			handle_ptr->recycle_self();
		}
		this->handler_map_.clear();
		this->loop_functions_read_.clear();
		this->loop_functions_write_.clear();
		this->events_.clear();
	}

	void EventManager::loop()
	{
		this->tid_ = pthread_self();
		while(!quit_)
		{
			this->loop_func();
			this->loop_io();
		}
	}

	void EventManager::loop_func()
	{
		{
			GUARD_LOCK(_lock_guard,func_mutex_);
			if(loop_functions_read_.empty() && loop_functions_write_.empty()) return;
			this->loop_functions_read_.splice(loop_functions_read_.end(),this->loop_functions_write_);
		}
		for(const auto& func : this->loop_functions_read_)
		{
			func();
		}
		this->loop_functions_read_.clear();
	}

	void EventManager::loop_io()
	{
		int fd_num = ::epoll_wait(this->epoll_fd_, &*(this->events_.begin()), this->events_.size(), EVENT_TIMEOUT);
		if(fd_num == 0 || quit_) return;
		if(fd_num < 0 && errno != EINTR)
		{
			perror("epoll_wait:");
			LOG_ERROR("epoll_wait ERROR:%d,%s",errno,strerror(errno));
			return ;
		}
		if(fd_num == static_cast<int>(this->events_.size()))
		{
			this->events_.resize(this->events_.size() * 2 );
		}
		for(int i = 0; i < fd_num; ++i)
		{
			EventHandle* handle_ptr = static_cast<EventHandle*>(this->events_[i].data.ptr);
			handle_ptr->run_func(long_unixstamp(),this->events_[i].events);
		}
	}
}//namespace sc

