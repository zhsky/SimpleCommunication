/*
* --*EventManager*--
* @Author: Payton
* @Last  : Payton
*/
#ifndef _EVENTMANAGER_H_
#define _EVENTMANAGER_H_

#include "EventHandle.h"
#include <memory>
#include <vector>
#include <map>
#include <mutex>
#include <sys/epoll.h>
#include <Thread.h>

namespace sc
{
	class EventManager:noncopyable
	{
	public:
		EventManager();
		~EventManager();

		int add_handle(std::shared_ptr<sc::EventHandle>&&);
		int remove_handle(int fd);

		void start_loop();
		void ready_quit(){quit_ = true;this->loop_thread_.stop();}
	private:
		void do_add_handle(std::shared_ptr<sc::EventHandle>& handle_ptr);
		void do_remove_handle(int fd);

		void loop();
		void loop_func();
		void loop_io();
	private:
		int epoll_fd_;
		std::map<int,std::shared_ptr<sc::EventHandle> > handler_map_;

		std::mutex func_mutex_;
		std::vector<VOID_FUNC> loop_functions_;

  		typedef std::vector<struct epoll_event> EventList;
  		EventList events_;

  		Thread loop_thread_;
  		bool quit_;
	};//class EventManager
}//namespace sc

#endif