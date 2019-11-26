/*
* --*main*--
* @Author: Payton
* @Last  : Payton
*/
#include "TimerManager.h"
#include "LogThread.h"
#include <unistd.h>

using namespace sc;


int main()
{
	LOG_INSTANCE->start();
	MANAGER_INSTANCE->start();
	sleep(5);
	MANAGER_INSTANCE->remove_timer();
	sleep(5);
	MANAGER_INSTANCE->stop();
	MANAGER_DESTORY;

	LOG_INSTANCE->stop();
	return 0;
}