/*
* --*Thread*--
* @Author: Payton
* @Last  : Payton
*/

#include "Thread.h"
#include <unistd.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdint.h>
#include <cstddef>
#include <sys/prctl.h>
#include <cstring>

namespace sc
{
	pid_t gettid()
	{
		return static_cast<pid_t>(::syscall(SYS_gettid));
	}

	std::string gettname()
	{
		char tname[16];
		prctl(PR_GET_NAME, tname);
		return tname;
	}

	void* Thread::inner_start(void* obj)
	{
		Thread* self = static_cast<Thread*>(obj);
		self->set_rtid(gettid());
		prctl(PR_SET_NAME, self->tname().c_str());
		LOG_INFO("thread started:tid:%ld,rtid:%d,name:%s",pthread_self(),self->rtid(),self->tname().c_str());

		pthread_cleanup_push(Thread::pthread_cleanup,obj);
		self->start_func();
		pthread_cleanup_pop(1);

		return NULL;
	}

	void Thread::pthread_cleanup(void* obj)
	{
		Thread* self = static_cast<Thread*>(obj);
		LOG_INFO("Thread exit %s",self->tname().c_str());
		self->handle_exit();
		return;
	}

	int Thread::init_func(VOID_FUNC func,const std::string&& thread_name)
	{
		func_ = std::move(func);
		exit_func_ = nullptr;
		tname_ = std::move(thread_name);
		tid_ = 0;
		rtid_ = 0;
		init_ = true;
		return 0;
	}

	int Thread::start()
	{
		if(init_ == false) return -1;
		if(int ret = pthread_create(&this->tid_,NULL,&Thread::inner_start,this); ret != 0)
		{
			LOG_ERROR("thread started FAILED,name:%s,ret:%d",this->tname().c_str(),ret);
			return ret;
		}
		else
		{
			return 0;
		}
	}

	int Thread::stop()
	{
		LOG_INFO("thread require stop,name:%s",this->tname().c_str());
		if(this->tid_ == 0) return 0;
		pthread_t tid = this->tid_;
		this->tid_ = 0;
		return pthread_join(tid,NULL);
	}

	void Thread::handle_exit()
	{
		if(exit_func_ != nullptr) exit_func_();
		return;
	}

	void Thread::sleep_usec(int64_t usec)
	{
		struct timespec ts = {0,0};
		ts.tv_sec = static_cast<time_t>(usec / 1000000);
		ts.tv_nsec = static_cast<time_t>(usec % 1000000 * 1000);
		nanosleep(&ts,NULL);
	}

}//namespace sc