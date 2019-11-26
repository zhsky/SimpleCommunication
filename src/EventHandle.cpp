/*
* --*EventHandle*--
* @Author: Payton
* @Last  : Payton
*/

#include <EventHandle.h>
#include <Log.h>
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
	
	int EventHandle::bind(IO_FUNC func,int fd, std::string&& event_name)
	{
		ename_ = std::move(event_name);
		event_type_ = IO_EVENT;
		io_func_ = func;
		event_flags_ = EPOLLIN | EPOLLET;
		fd_ = fd;

		return 0;
	}

	int EventHandle::bind(TIME_FUNC func,int64_t timeout,int interval,std::string&& event_name)
	{
		ename_ = std::move(event_name),
		event_type_ = TIMER_EVENT;
		timer_func_ = func;
		fd_ = create_timerfd();
		event_flags_ = EPOLLIN | EPOLLET;

		if(fd_ > 0)
		{
			struct itimerspec ts;

			ts.it_value.tv_sec = timeout / USEC_PER_SEC;
			ts.it_value.tv_nsec = timeout % USEC_PER_SEC * 1000;
			ts.it_interval.tv_sec = interval / USEC_PER_SEC;
			ts.it_interval.tv_nsec = interval % USEC_PER_SEC * 1000;
			if(int ret = ::timerfd_settime(fd_,1,&ts,NULL); ret != 0)
			{
				LOG_ERROR("timerfd_settime ERROR,fd:%ld,ret:%d,name:%s",fd_,ret,ename_.c_str());
				return -1;
			}
		}
		else
		{
			LOG_ERROR("create_timerfd ERROR,fd:%ld,%s,name:%s",fd_,strerror(fd_),ename_.c_str());
			return -1;
		}
		return 0;
	}

	void EventHandle::run_func(int64_t now, uint32_t event_flags)
	{
		if(event_type_ == IO_EVENT) 
		{
			io_func_(fd_,event_flags);
		}
		else 
		{
			static struct itimerspec curr_value;
			timerfd_gettime(fd_,&curr_value);
			timer_func_(now);
		}
	}

	void EventHandle::recycle_self()
	{
		if(recycle_func_ == nullptr)
		{
			LOG_WARNING("WARNING NULL RECYCLE FUNC,%s",ename_.c_str());
			return;
		}
		recycle_func_(this);
	}
	EventHandle::~EventHandle()
	{
		if(event_type_ == TIMER_EVENT) ::close(fd_);
		event_type_ = 0;
		event_flags_ = 0;

		io_func_ = nullptr;
		timer_func_ = nullptr;
		recycle_func_ = nullptr;
		LOG_INFO("destory EventHandle,fd:%ld,name:%s",fd_,ename_.c_str());
		fd_ = 0;
		ename_ = "";
	}

}//namespace sc