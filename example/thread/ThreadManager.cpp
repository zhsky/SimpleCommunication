/*
* --*ThreadManager*--
* @Author: Payton
* @Last  : Payton
*/
#include "ThreadManager.h"
#include <cstring>
#include <cstdlib>

void ThreadManager::start()
{
	this->runing_ = true;
	this->tpool_.start(1);

	for(int i = 0; i < 1; ++i)
	{
		char tname[32] = {'\0'};
		snprintf(tname,31,"task_producer-%d",i + 1);
		tlist_.emplace_back(new sc::Thread(std::bind(&ThreadManager::gen_task,this),tname));
		tlist_[i]->start();
	}
}

void ThreadManager::stop()
{
	this->runing_ = false;
	for(auto& t : this->tlist_)
	{
		t->stop();
	}

	this->tpool_.stop();
}

void ThreadManager::gen_task()
{
	char tid[1024] = {'\0'};
	snprintf(tid,1023,"task_pid-%ld",pthread_self());
	while(this->runing_)
	{
		this->tpool_.accept_task(std::bind(&ThreadManager::handle_task,this),tid);
		int waits =  std::rand() % 1000000;
		sc::Thread::sleep_usec(waits);
	}
}

void ThreadManager::handle_task()
{
	int waits =  std::rand() % 2000000;
	sc::Thread::sleep_usec(waits);
	LOG_INFO("%s,finish",sc::gettname().c_str());
}