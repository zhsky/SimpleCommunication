/*
* --*EventManager*--
* @Author: Payton
* @Last  : Payton
*/
#include "EventManager.h"
#include "Timestamp.h"
#include <errno.h>
#include <cstring>
#include <unistd.h>

#define MAX_EVENT 1024
#define EVENT_TIMEOUT 10000
#define GUARD_LOCK(MUTEX) std::lock_guard<std::mutex> _mutex_lock(MUTEX);
	
namespace sc
{
	EventManager::EventManager():events_(MAX_EVENT),quit_(false)
	{
		if(epoll_fd_ = epoll_create1(EPOLL_CLOEXEC); epoll_fd_ < -1)
		{
			LOG_ERROR("epoll_create ERROR,%d,%s",errno,strerror(errno));
		}
	}

	EventManager::~EventManager()
	{
		if(!quit_) this->ready_quit();
		::close(epoll_fd_);
		this->handler_map_.clear();
		this->loop_functions_.clear();
	}

	int EventManager::add_handle(std::shared_ptr<sc::EventHandle>&& handle_ptr)
	{
		if(quit_ == true) return -1;
		if(handle_ptr->fd() <= 0) return -2;
		GUARD_LOCK(func_mutex_);
		this->loop_functions_.push_back(std::bind(&EventManager::do_add_handle,this,handle_ptr));
		return 0;
	}

	int EventManager::remove_handle(int fd)
	{
		if(quit_ == true) return -1;
		if(fd <= 0) return -2;
		GUARD_LOCK(func_mutex_);
		this->loop_functions_.push_back(std::bind(&EventManager::do_remove_handle,this,fd));
		return 0;
	}

	void EventManager::do_add_handle(std::shared_ptr<sc::EventHandle>& handle_ptr)
	{
		const std::string& event_name = handle_ptr->ename();
		if(handler_map_.find(handle_ptr->fd()) != handler_map_.end())
		{
			LOG_ERROR("REPEAT %ld,%d,%s",handle_ptr->fd(),handle_ptr->event_type(),event_name.c_str());
			return;
		}

		struct epoll_event event;
		memset(&event, 0, sizeof(event));

		event.data.fd = handle_ptr->fd();
		event.events |= handle_ptr->event_flag();

		handler_map_.insert(std::make_pair(handle_ptr->fd(),handle_ptr));
		handle_ptr = nullptr;

		if(::epoll_ctl(this->epoll_fd_,EPOLL_CTL_ADD,event.data.fd,&event) < 0)
		{
			LOG_ERROR("epoll_ctl ERROR EPOLL_CTL_ADD");
			handler_map_.erase(event.data.fd);
			return;
		}
		LOG_INFO("fd:%ld, name:%s",event.data.fd,event_name.c_str());
		return;
	}

	void EventManager::do_remove_handle(int fd)
	{
		if(handler_map_.find(fd) == handler_map_.end())
		{
			LOG_ERROR("ERROR not found %ld",fd);
			return;
		}
		std::shared_ptr<sc::EventHandle> handle_ptr = handler_map_[fd];
		handler_map_.erase(fd);

		struct epoll_event event;
		memset(&event, 0, sizeof(event));
		if(::epoll_ctl(this->epoll_fd_,EPOLL_CTL_DEL,fd,&event) < 0)
		{
			perror("epoll_ctl:");
			LOG_ERROR("epoll_ctl ERROR EPOLL_CTL_DEL %ld,errno:%d,%s",fd,errno,strerror(errno));
			return;
		}
		LOG_INFO("fd:%ld, name:%s",fd,handle_ptr->ename().c_str());
		return;
	}

	void EventManager::start_loop()
	{
		if(this->loop_thread_.is_init()) return;
		this->loop_thread_.init_func(std::bind(&EventManager::loop,this),"event_loop_thread");
		this->loop_thread_.start();
	}

	void EventManager::loop()
	{
		while(!quit_)
		{
			this->loop_func();
			this->loop_io();
		}
	}

	void EventManager::loop_func()
	{
		GUARD_LOCK(func_mutex_);
		for(const auto& func : this->loop_functions_)
		{
			func();
		}
		this->loop_functions_.clear();
	}

	void EventManager::loop_io()
	{
		int fd_num = ::epoll_wait(this->epoll_fd_, &*(this->events_.begin()), static_cast<int>(this->events_.size()), EVENT_TIMEOUT);
		if(fd_num < 0 && errno != EINTR)
		{
			LOG_ERROR("epoll_wait ERROR:%d,%s",errno,strerror(errno));
			return ;
		}
		if(fd_num == static_cast<int>(this->events_.size()))
		{
			this->events_.resize(this->events_.size() * 2 );
		}
		for(int i = 0; i < fd_num; ++i)
		{
			if(auto iter = handler_map_.find(this->events_[i].data.fd); iter != handler_map_.end())
			{
				iter->second->run_func(long_unixstamp(),this->events_[i].events);
			}
		}
	}
}//namespace sc

