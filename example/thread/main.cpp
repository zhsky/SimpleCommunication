/*
* --*main*--
* @Author: Payton
* @Last  : Payton
*/
#include <ThreadManager.h>
#include <LogThread.h>
#include <unistd.h>

using namespace sc;


int main()
{
	LOG_INSTANCE->start();
	MANAGER_INSTANCE->start();
	sleep(3);
	MANAGER_INSTANCE->stop();
	LOG_INSTANCE->stop();
	return 0;
}