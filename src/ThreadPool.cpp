/*
* --*ThreadPool*--
* @Author: Payton
* @Last  : Payton
*/

#include <ThreadPool.h>

namespace sc
{

#define UNIQUE_LOCK(MUTEX) std::unique_lock<std::mutex> _unique_lock(MUTEX);

ThreadPool::~ThreadPool()
{
	if(this->running_ == true)
		this->stop();
}

void ThreadPool::start(int thread_n)
{
	LOG_INFO("ThreadPool start thread number:%d",thread_n);
	if(this->threads_.empty() == false) return;

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
		UNIQUE_LOCK(list_mutex_);
		this->running_ = false;
		out_cond_.notify_all();
	}

	for(auto& t : this->threads_)
	{
		t->stop();
	}
	this->threads_.clear();
}

void ThreadPool::accept_task(TaskFun task,std::string&& task_name)
{
	if(this->threads_.empty()) 
	{
		task();
		return;
	}
	else
	{
		UNIQUE_LOCK(list_mutex_);
		if(max_tasks_ > 0)
		{
			while(this->task_list_.size() >= max_tasks_)
			{
				in_cond_.wait(_unique_lock);
			}
		}

		this->task_list_.push_back(std::make_pair(std::move(task),std::move(task_name)));
		out_cond_.notify_all();
	}
}

ThreadPool::Task ThreadPool::require_task()
{
	UNIQUE_LOCK(list_mutex_);
	while(this->running_ && this->task_list_.empty())
	{
		out_cond_.wait(_unique_lock);
	}

	if(this->task_list_.empty() == false)
	{
		auto task = this->task_list_.front();
		this->task_list_.pop_front();
		if(max_tasks_ > 0)
			in_cond_.notify_all();
		return task;
	}
	_unique_lock.unlock();

	TaskFun task;
	return std::make_pair(task,"");
}

void ThreadPool::thread_loop()
{
	while(running_)
	{
		auto task = std::move(require_task());
		try
		{
			if(task.first) task.first();
		}
		catch(const std::exception& ex)
		{
			LOG_ERROR("ERROR EXCEPTION in function %s,what:%s",task.second.c_str(),ex.what());
			continue;
		}
		catch(...)
		{
			LOG_ERROR("ERROR UNCATCH EXCEPTION in function %s",task.second.c_str());
			throw;
		}
	}
}

}//namespace sc