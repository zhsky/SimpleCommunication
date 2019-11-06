/*
* --*Singleton*--
* @Author: Payton
* @Last  : Payton
*/
#ifndef _SINGLETON_H_
#define _SINGLETON_H_

#include "noncopyable.h"
#include <pthread.h>
#include <stdlib.h>
#include <Log.h>

namespace sc
{
template<typename T>
class Singleton:noncopyable
{
public:
	Singleton() = delete;
	~Singleton() = delete;

	static T* instance()
	{
		pthread_once(&once_,&Singleton::init);
		return instance_;
	}

	//主动调用，或atexit调用
	static void destory()
	{
		typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
		(void) sizeof(T_must_be_complete_type);
		if(instance_ != nullptr) 
		{
			delete instance_;
			LOG_INFO("%s destory!",typeid(T).name());
		}
		instance_ = nullptr;

	}
private:
	static void init()
	{
		instance_ = new T();
		LOG_INFO("new instance %s",typeid(T).name());
		atexit(destory);
	}

private:
	static pthread_once_t once_;
	static T* instance_;
};

template<typename T>
pthread_once_t Singleton<T>::once_ = PTHREAD_ONCE_INIT;
template<typename T>
T* Singleton<T>::instance_ = nullptr;

}// namespace sc

#endif