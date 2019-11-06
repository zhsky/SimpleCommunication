/*
* --*main*--
* @Author: Payton
* @Last  : Payton
*/
#include "TimerManager.h"
#include <unistd.h>



int main()
{
	MANAGER_INSTANCE->start();
	sleep(5);
	MANAGER_INSTANCE->remove_timer();
	sleep(5);
	MANAGER_INSTANCE->stop();
	MANAGER_DESTORY;
	return 0;
}