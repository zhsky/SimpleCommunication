/*
* --*main*--
* @Author: Payton
* @Last  : Payton
*/
#include <ThreadManager.h>
#include <unistd.h>



int main()
{
	MANAGER_INSTANCE->start();
	sleep(3);
	MANAGER_INSTANCE->stop();
	return 0;
}