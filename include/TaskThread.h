/*
* --*TaskThread*--
* @Author: Payton
* @Last  : Payton
*/

#include <Common.h>

#include <condition_variable>
#include <list>
#include <mutex>
#include <assert.h>

namespace sc
{
class Thread;

class TaskThread
{
public:
	TaskThread();
	~TaskThread();
	void start();
	void stop();
	
	void run_in_thread(VOID_FUNC);
	void assert_in_thread(){assert(tid_ == pthread_self());}
private:
	void wakeup(){func_cond_.notify_one();}
	void loop();
private:
	pthread_t tid_;
	bool running_;
	std::recursive_mutex func_mutex_;
	std::list<VOID_FUNC> loop_functions_read_;
	std::list<VOID_FUNC> loop_functions_write_;
	std::condition_variable_any func_cond_;
  	Thread* loop_thread_;
};

}//namespace sc