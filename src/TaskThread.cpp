/*
* --*TaskThread*--
* @Author: Payton
* @Last  : Payton
*/

#include <TaskThread.h>
#include <Thread.h>
#include <Log.h>
namespace sc
{

TaskThread::TaskThread():tid_(0),running_(false),loop_thread_(nullptr)
{
	loop_functions_read_.clear();
	loop_functions_write_.clear();
}

TaskThread::~TaskThread()
{
	if(loop_thread_ != nullptr)
		delete this->loop_thread_;
	this->loop_thread_ = nullptr;
}

void TaskThread::start()
{
	if(this->running_)
	{
		LOG_WARNING("WARN RUNNING");
		return ;
	}
	if(loop_thread_ == nullptr) loop_thread_ = new Thread();

	this->running_ = true;
	this->loop_thread_->init_func(std::bind(&TaskThread::loop,this),"TaskThread");
	this->loop_thread_->start();
}

void TaskThread::stop()
{
	if(!running_) return;
	running_ = false;
	this->wakeup();
	this->loop_thread_->stop();
	delete this->loop_thread_;
	this->loop_thread_ = nullptr;
}

void TaskThread::run_in_thread(VOID_FUNC func)
{
	GUARD_LOCK(_lock_guard,func_mutex_);
	this->loop_functions_write_.emplace_back(func);
	this->wakeup();
}

void TaskThread::loop()
{
	this->tid_ = pthread_self();
	while(running_)
	{
		{
			UNIQUE_LOCK(_unique_lock,func_mutex_);
			func_cond_.wait(_unique_lock);
			this->loop_functions_read_.splice(this->loop_functions_read_.end(),this->loop_functions_write_);
		}
		if(running_ && this->loop_functions_read_.empty() == false)
		{
			for(const auto& func : this->loop_functions_read_)
			{
				func();
			}
			this->loop_functions_read_.clear();
		}
	}
}

}