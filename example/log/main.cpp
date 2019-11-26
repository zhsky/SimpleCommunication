/*
* --*main*--
* @Author: Payton
* @Last  : Payton
*/

#include <LogThread.h>
#include <unistd.h>
#include <Thread.h>
#include <Log.h>

using namespace sc;

bool runing = true;

void loging()
{
	char log_tmp[3000]={'\0'};
	for(int i = 0; i < 2999; ++i)
	{
		log_tmp[i]='1';
	}
	while(runing)
	{
		LOG_INFO("%s",log_tmp);
	}
}


int main()
{
	LOG_INSTANCE->start();

	Thread thread(std::bind(&loging),"log_thread");
	thread.start();
	sleep(10);
	runing = false;
	thread.stop();
	LOG_INSTANCE->stop();
}