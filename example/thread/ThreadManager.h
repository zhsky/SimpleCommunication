/*
* --*ThreadManager*--
* @Author: Payton
* @Last  : Payton
*/

#ifndef _THREADMANAGER_H_
#define _THREADMANAGER_H_

#include <Singleton.h>
#include <Thread.h>
#include <ThreadPool.h>
#include <vector>
#include <memory>
#include <map>

class ThreadManager
{
public:
	ThreadManager():tpool_(10),runing_(false){}
	~ThreadManager() = default;

	void gen_task();

	void handle_task();
	void start();
	void stop();
private:
	sc::ThreadPool tpool_;
	std::vector< std::unique_ptr<sc::Thread> > tlist_;

	bool runing_;
};

#define MANAGER_DESTORY sc::Singleton<ThreadManager>::destory()
#define MANAGER_INSTANCE sc::Singleton<ThreadManager>::instance()

#endif