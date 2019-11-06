/*
* --*Common*--
* @Author: Payton
* @Last  : Payton
*/
#ifndef _COMMON_H_
#define _COMMON_H_

#include <functional>
#include "Log.h"
namespace sc
{
	#define IO_EVENT	0x1
	#define TIMER_EVENT	0x2
	#define USEC	(1000 * 1000)
	#define SECOND	1
	#define MINUTE	60
	#define HOUR	(60 * MINUTE)
	#define DAY		(24 * HOUR)
	#define WEEK	(7 * DAY)

	typedef std::function<void()> VOID_FUNC;
	typedef std::function<void(uint32_t)> IO_FUNC;
	typedef std::function<void(int64_t)> TIME_FUNC;

}//namespace sc
#endif