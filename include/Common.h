/*
* --*Common*--
* @Author: Payton
* @Last  : Payton
*/
#ifndef _COMMON_H_
#define _COMMON_H_

#include <functional>
namespace sc
{
	class Buffer;
	class EventHandle;
	class RMutexGuard;
	class WMutexGuard;
	#define IO_EVENT	0x1
	#define TIMER_EVENT	0x2
	#define USEC_PER_SEC	(1000 * 1000)
	#define SECOND	1
	#define MINUTE	60
	#define HOUR	(60 * MINUTE)
	#define DAY		(24 * HOUR)
	#define WEEK	(7 * DAY)

	typedef std::function<void()> VOID_FUNC;
	typedef std::function<void(int,uint32_t)> IO_FUNC;
	typedef std::function<void(int64_t)> TIME_FUNC;
	typedef std::function<void(Buffer*)> MSG_FUNC;
	typedef std::function<void(uint64_t)> UINT64_FUNC;
	typedef std::function<void(EventHandle*)> HANDLE_FUNC;

	#define GUARD_LOCK(OBJ,MUTEX) std::lock_guard<std::recursive_mutex> OBJ(MUTEX);
	#define UNIQUE_LOCK(OBJ,MUTEX) std::unique_lock<std::recursive_mutex> OBJ(MUTEX);
	#define RGUARD_LOCK(OBJ,RWMUTEX) RMutexGuard OBJ(RWMUTEX);
	#define WGUARD_LOCK(OBJ,RWMUTEX) WMutexGuard OBJ(RWMUTEX);

}//namespace sc
#endif