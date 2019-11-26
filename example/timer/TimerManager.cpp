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
	static int64_t last = 0;
	LOG_INFO("timeout:%ld:%ld",now,now - last);
	last = now;
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
		sc::long_unixstamp() + 2 * USEC_PER_SEC,
		100,
		"timeout_handle"
	);
	if(timer_handler->fd() > 0)
	{
		LOG_INFO("add timer_handler:%d",timer_handler->fd());
		timer_handler->set_recycle_func(std::bind(&decltype(event_pool_)::push,&event_pool_,std::placeholders::_1));
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