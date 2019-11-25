/*
* --*TimerManager*--
* @Author: Payton
* @Last  : Payton
*/

#include "TimerManager.h"
#include <unistd.h>
#include "Timestamp.h"

using namespace sc;
void TimerManager::handler_timeout(int64_t now)
{
	LOG_INFO("timeout:%ld",now / USEC);
}

void TimerManager::remove_timer()
{
	event_manager_.remove_handle(this->timer_fd_);
}

int TimerManager::start()
{
	sc::EventHandle* timer_handler = event_pool_.pop();
	timer_handler->bind(
		std::bind(&TimerManager::handler_timeout,this,std::placeholders::_1),
		sc::long_unixstamp() + 2 * USEC,
		USEC,
		"timeout_handle"
	);
	if(timer_handler->fd() > 0)
	{
		LOG_INFO("add timer_handler");
		this->timer_fd_ = timer_handler->fd();
		event_manager_.add_handle(timer_handler);
	}
	event_manager_.start_loop();
	return 0;
}

int TimerManager::stop()
{
	event_manager_.stop_loop();
	return 0;
}