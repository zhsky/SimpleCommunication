/*
* --*EventHandle*--
* @Author: Payton
* @Last  : Payton
*/

#include "EventHandle.h"
#include <sys/timerfd.h>
#include <unistd.h>
#include <sys/epoll.h>

namespace sc
{
	int create_timerfd()
	{
		int timerfd = ::timerfd_create(CLOCK_REALTIME,TFD_NONBLOCK | TFD_CLOEXEC);
		if (timerfd < 0)
			return errno;
		return timerfd;
	}

	EventHandle::EventHandle(IO_FUNC func,std::string&& event_name):
		ename_(std::move(event_name)),event_type_(IO_EVENT),io_func_(func),event_flags_(EPOLLIN | EPOLLET)
	{

	}

	EventHandle::EventHandle(TIME_FUNC func,int64_t timeout,int interval,std::string&& event_name):
		ename_(std::move(event_name)),
		event_type_(TIMER_EVENT),timer_func_(func),
		fd_(create_timerfd()),event_flags_(EPOLLIN | EPOLLET)
	{
		if(fd_ > 0)
		{
			struct itimerspec ts;
 
			ts.it_value.tv_sec = timeout / USEC;
			ts.it_value.tv_nsec = timeout % USEC * 1000;
			ts.it_interval.tv_sec = interval / USEC;
			ts.it_interval.tv_nsec = interval % USEC * 1000;
			if(int ret = ::timerfd_settime(fd_,1,&ts,NULL); ret != 0)
			{
				LOG_ERROR("timerfd_settime ERROR,fd:%ld,ret:%d,name:%s",fd_,ret,ename_.c_str());
			}
		}
		else
		{
			LOG_ERROR("create_timerfd ERROR,fd:%ld,%s,name:%s",fd_,strerror(fd_),ename_.c_str());
		}
	}

	void EventHandle::run_func(int64_t now,uint32_t event_flags)
	{
		if(event_type_ == IO_EVENT) io_func_(event_flags);
		else timer_func_(now);
	}

	EventHandle::~EventHandle()
	{
		LOG_DEBUG("destory EventHandle,fd:%ld,name:%s",fd_,ename_.c_str());
		::close(fd_);
	}
}//namespace sc