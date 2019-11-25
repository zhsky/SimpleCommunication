/*
* --*ThreadPool*--
* @Author: Payton
* @Last  : Payton
*/
#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

#include <noncopyable.h>
#include <mutex>
#include <list>
#include <functional>
#include <condition_variable>
#include <cstring>
#include <memory>
#include <sys/types.h>
#include <Thread.h>

namespace sc
{

class ThreadPool:noncopyable
{
#define TASK_SIZE 100
public:
	typedef std::function<void()> TaskFun;
	typedef std::unique_ptr<Thread> ThreadPtr;
	ThreadPool(uint max_tasks = TASK_SIZE):running_(false){};
	~ThreadPool();

	void start(int thread_n = 0);
	void stop();

	void accept_task(TaskFun task);
	void accept_task_list(std::list<TaskFun>& task_list);
private:
	void thread_loop();
	TaskFun require_task();
private:
	std::condition_variable_any out_cond_;

	std::recursive_mutex list_mutex_;
	std::list< TaskFun > task_list_;

	std::vector<ThreadPtr> threads_;

	bool running_;
};


}//namespace sc



#endif
