/*
* --*EventManager*--
* @Author: Payton
* @Last  : Payton
*/
#ifndef _EVENTMANAGER_H_
#define _EVENTMANAGER_H_

#include <memory>
#include <list>
#include <map>
#include <mutex>
#include <sys/epoll.h>
#include <assert.h>
#include <Thread.h>
#include <pthread.h>

namespace sc
{
	class EventHandle;

	class EventManager:noncopyable
	{
	public:
		EventManager();
		~EventManager();

		EventHandle* get_handle(int fd);
		int add_handle(EventHandle*);
		void update_handle(int fd);
		int remove_handle(int fd);
		void do_remove_handle(int fd);//loop线程调用

		void run_in_handle_loop(VOID_FUNC func);

		void start_loop();
		void stop_loop();
	private:
		void assert_in_thread(){assert(tid_ == pthread_self());}
		int do_update_handle(int fd);
		void do_add_handle(EventHandle* handle_ptr);

		void ready_quit(){quit_ = true;this->loop_thread_.stop();}
		void loop();
		void loop_func();
		void loop_io();
	private:
		pthread_t tid_;
		int epoll_fd_;
		std::map<int,EventHandle* > handler_map_;//loop_thread_线程读写，其他线程读

		std::recursive_mutex func_mutex_;
		std::list<VOID_FUNC> loop_functions_read_;
		std::list<VOID_FUNC> loop_functions_write_;

  		typedef std::vector<struct epoll_event> EventList;
  		EventList events_;

  		Thread loop_thread_;
  		bool quit_;
		pthread_rwlock_t rwlock_;
	};//class EventManager
}//namespace sc

#endif