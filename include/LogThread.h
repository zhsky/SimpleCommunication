/*
* --*LogThread*--
* @Author: Payton
* @Last  : Payton
*/

#ifndef _LOGTHREAD_H_
#define _LOGTHREAD_H_

#include <noncopyable.h>
#include <Singleton.h>
#include <ObjectPool.h>
#include <Buffer.h>

#include <cstring>
#include <mutex>
#include <list>
#include <condition_variable>

namespace sc
{
class Thread;
class Buffer;
class LogFile;
class Log:noncopyable
{
public:
	Log();
	~Log();

	int start(const std::string& base_filename = "log_");
	int stop();

	Buffer* pop_buff_log(){return this->buff_pool_.pop();};
	void log_info(Buffer* info);
private:
	void loop_write();
private:
	bool running_;


	std::list<Buffer*> log_list_;
	std::list<Buffer*> write_list_;

	ObjectPool<Buffer> buff_pool_;
	std::recursive_mutex list_mutex_;
  	Thread* loop_thread_;

	LogFile* file_;
	std::condition_variable_any out_cond_;

};

#define LOG_DESTORY Singleton<Log>::destory()
#define LOG_INSTANCE Singleton<Log>::instance()
}//namespace sc

#endif //_LOGTHREAD_H_