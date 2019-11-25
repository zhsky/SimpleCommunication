/*
* --*ThreadMutex*--
* @Author: Payton
* @Last  : Payton
*/
#ifndef _THREADMUTEX_H_
#define _THREADMUTEX_H_

#include <pthread.h>
#include <stdint.h>
namespace sc
{

class RMutexGuard
{
public:
	RMutexGuard(pthread_rwlock_t& lock_):local_lock_(lock_)
	{
		pthread_rwlock_rdlock(&local_lock_);
	}
	~RMutexGuard()
	{
		pthread_rwlock_unlock(&local_lock_);
	}
private:
	pthread_rwlock_t& local_lock_;
};

class WMutexGuard
{
public:
	WMutexGuard(pthread_rwlock_t& lock_):local_lock_(lock_)
	{
		pthread_rwlock_wrlock(&local_lock_);
	}
	~WMutexGuard()
	{
		pthread_rwlock_unlock(&local_lock_);
	}
private:
	pthread_rwlock_t& local_lock_;
};


}//namespace sc

#endif //_THREADMUTEX_H_