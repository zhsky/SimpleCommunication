/*
* --*noncopyable*--
* @Author: Payton
* @Last  : Payton
*/
#ifndef _NONCOPYABLE_H_
#define _NONCOPYABLE_H_

namespace sc
{
	class noncopyable
	{
	public:
		noncopyable(const noncopyable&) = delete;
		void operator=(const noncopyable&) = delete;
	protected:
		noncopyable() = default;
		~noncopyable() = default;
	};//class noncopyable
}//namespace sc

#endif