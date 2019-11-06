/*
* --*Thread*--
* @Author: Payton
* @Last  : Payton
*/
#ifndef _THREAD_H_
#define _THREAD_H_

#include "Common.h"
#include "noncopyable.h"
#include <string>
#include <pthread.h>

namespace sc
{
	extern pid_t gettid();
	extern std::string gettname();
	class Thread : noncopyable
	{
	public:
		explicit Thread(VOID_FUNC func,const std::string&& thread_name = std::string()):
			init_(true),func_(std::move(func)),tname_(thread_name),tid_(0),rtid_(0){}

		Thread() = default;
		~Thread() = default;
		int init_func(VOID_FUNC func,const std::string&& thread_name = std::string());
		bool is_init() {return this->init_;}
		int start();
		int stop();

		virtual void handle_exit(){return;};
	public:
		pid_t rtid(){return rtid_;}
		pthread_t tid(){return tid_;}
		void set_rtid(pid_t rtid){rtid_ = rtid;}
		const std::string& tname(){return tname_;}

		void start_func(){func_();}
		static void* inner_start(void* obj);
		static void pthread_cleanup(void* obj);

		static void sleep_usec(int64_t usec);
	private:
		bool init_;
	  	VOID_FUNC func_;
		std::string tname_;
		pthread_t tid_;
		pid_t rtid_;

	}; //class Thread
}// namespace sc

#endif