/*
* --*TimerManager*--
* @Author: Payton
* @Last  : Payton
*/
#ifndef _TIMERSTAMP_H_
#define _TIMERSTAMP_H_

#include "Singleton.h"
#include "EventManager.h"
#include <stdint.h>

class TimerManager
{
public:
	TimerManager() = default;
	~TimerManager() = default;

	int start();
	int stop();

	void handler_timeout(int64_t now);
	void handler_ioinput();

	void remove_timer();
private:
	sc::EventManager event_manager_;
	int timer_fd_ = 0;
};

#define MANAGER_DESTORY sc::Singleton<TimerManager>::destory()
#define MANAGER_INSTANCE sc::Singleton<TimerManager>::instance()

#endif