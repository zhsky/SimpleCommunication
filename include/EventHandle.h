/*
* --*EventHandle*--
* @Author: Payton
* @Last  : Payton
*/
#ifndef _EVENTHANDLE_H_
#define _EVENTHANDLE_H_

#include "Common.h"
#include "noncopyable.h"
#include <cstring>

namespace sc
{
class EventHandle:noncopyable
{
public:
	EventHandle():ename_(""),event_type_(0),event_flags_(0),fd_(0),recycle_func_(nullptr),
		io_func_(nullptr),timer_func_(nullptr){}
	~EventHandle();

	int bind(IO_FUNC func, int fd, std::string&& event_name = std::string());
	int bind(TIME_FUNC func,int64_t timeout,int interval,std::string&& event_name = std::string());

	void set_recycle_func(HANDLE_FUNC func){recycle_func_ = func;}
	void recycle_self();

	void set_event_flag(uint32_t flags){this->event_flags_ = flags;}
	uint32_t event_flag(){return this->event_flags_;}
	void run_func(int64_t now, int fd, uint32_t event_flags);
	int event_type(){return event_type_;}
	int fd(){return fd_;}
	const std::string& ename(){return ename_;}
private:
	std::string ename_;
	int event_type_;
	int event_flags_;
	int fd_;

	HANDLE_FUNC recycle_func_;
	IO_FUNC io_func_;
	TIME_FUNC timer_func_;
};//class EventHandle
}//namespace sc

#endif