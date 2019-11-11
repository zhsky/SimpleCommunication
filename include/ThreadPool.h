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

namespace sc
{

class Thread;
class ThreadPool:noncopyable
{
public:
	typedef std::function<void()> TaskFun;
	typedef std::pair<TaskFun,std::string> Task;
	typedef std::unique_ptr<Thread> ThreadPtr;
	ThreadPool(size_t max_tasks = 0):max_tasks_(max_tasks),running_(false){};
	~ThreadPool();

	void start(int thread_n = 0);
	void stop();

	void accept_task(TaskFun task,std::string&& task_name);
private:
	void thread_loop();
	Task require_task();

private:
	std::condition_variable in_cond_;
	std::condition_variable out_cond_;

	std::mutex list_mutex_;
	std::list< Task > task_list_;

	std::vector<ThreadPtr> threads_;

	size_t max_tasks_;
	bool running_;
};


}//namespace sc



#endif