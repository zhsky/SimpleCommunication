/*
* --*TimerManager*--
* @Author: Payton
* @Last  : Payton
*/

#include "TimerManager.h"
#include <unistd.h>
#include "Timestamp.h"

void TimerManager::run_thread()
{
	{
		std::shared_ptr<sc::EventHandle> timer_handler(new sc::EventHandle(
			std::bind(&TimerManager::handler_timeout,this,std::placeholders::_1),
			sc::long_unixstamp() + 2 * USEC,
			USEC,
			"timeout_handle")
		);
		if(timer_handler->fd() > 0)
		{
			LOG_INFO("add timer_handler");
			this->timer_fd_ = timer_handler->fd();
			event_manager_.add_handle(std::move(timer_handler));
			timer_handler = nullptr;
		}
	}

	event_manager_.loop();
}

void TimerManager::handler_timeout(int64_t now)
{
	int64_t count;
	read(this->timer_fd_,&count,sizeof(count));
	LOG_INFO("count:%d,timeout:%ld",count,now / USEC);
}

void TimerManager::remove_timer()
{
	event_manager_.remove_handle(this->timer_fd_);
}

int TimerManager::start()
{
	if(this->thread_.is_init() == false)
		this->thread_.init_func(std::bind(&TimerManager::run_thread,this),"test_thread");
	this->thread_.start();
	return 0;
}

int TimerManager::stop()
{
	event_manager_.ready_quit();
	this->thread_.stop();
	return 0;
}