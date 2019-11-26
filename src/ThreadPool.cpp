/*
* --*ThreadPool*--
* @Author: Payton
* @Last  : Payton
*/

#include <ThreadPool.h>
#include <Log.h>
#include <assert.h>

namespace sc
{

ThreadPool::~ThreadPool()
{
	if(this->running_ == true)
		this->stop();
}

void ThreadPool::start(int thread_n)
{
	if(this->running_)
	{
		LOG_WARNING("WARN RUNNING");
		return;
	}
	LOG_INFO("ThreadPool start thread number:%d",thread_n);
	this->running_ = true;
	for(int i = 0; i < thread_n; ++i)
	{
		char tname[39] = {'\0'};
		snprintf(tname,38,"task_thread_%d",i + 1);
		this->threads_.emplace_back(new Thread(std::bind(&ThreadPool::thread_loop,this),tname));
		this->threads_[i]->start();
	}
}

void ThreadPool::stop()
{
	LOG_INFO("ThreadPool stop");
	{
		GUARD_LOCK(_unique_lock,list_mutex_);
		this->running_ = false;
		this->task_list_.clear();
		out_cond_.notify_all();
	}

	for(auto& t : this->threads_)
	{
		t->stop();
	}
	this->threads_.clear();
}

void ThreadPool::accept_task(TaskFun task)
{
	if(this->threads_.empty()) 
	{
		task();
		return;
	}
	else
	{
		if(this->task_list_.size() / this->threads_.size() > 10)
		{
			LOG_WARNING("TOO MUCH TASK %d", this->task_list_.size());
		}
		{
			GUARD_LOCK(_unique_lock,list_mutex_);
			this->task_list_.emplace_back(task);
		}

		out_cond_.notify_all();
	}
}

void ThreadPool::accept_task_list(std::list<TaskFun>& task_list)
{
	if(this->threads_.empty()) 
	{
		for(auto& task : task_list)
			task();
		return;
	}
	else
	{
		if(this->task_list_.size() / this->threads_.size() > 10)
		{
			LOG_WARNING("TOO MUCH TASK %d", this->task_list_.size());
		}
		{
			GUARD_LOCK(_unique_lock,list_mutex_);
			this->task_list_.splice(this->task_list_.end(),task_list);
		}

		out_cond_.notify_all();
	}
}

ThreadPool::TaskFun ThreadPool::require_task()
{
	UNIQUE_LOCK(_unique_lock,list_mutex_);
	while(this->running_ && this->task_list_.empty())
	{
		out_cond_.wait(_unique_lock);
	}

	if(this->task_list_.empty() == false)
	{
		auto task = this->task_list_.front();
		this->task_list_.pop_front();
		return std::move(task);
	}
	return TaskFun();
}

void ThreadPool::thread_loop()
{
	while(running_)
	{
		auto task = std::move(require_task());
		try
		{
			if(task) task();
		}
		catch(const std::exception& ex)
		{
			LOG_ERROR("ERROR EXCEPTION what:%s",ex.what());
			continue;
		}
		catch(...)
		{
			LOG_ERROR("ERROR UNCATCH EXCEPTION in function %s");
			throw;
		}
	}
}



}//namespace sc