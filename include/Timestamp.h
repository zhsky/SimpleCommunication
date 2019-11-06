/*
* --*Timestamp*--
* @Author: Payton
* @Last  : Payton
*/
#ifndef _TIMESTAMP_H_
#define _TIMESTAMP_H_

#include <sys/time.h>
#include <stdint.h>
#include <cstddef>

namespace sc
{
	class Timestamp
	{
	public:
		Timestamp()	{::gettimeofday(&(this->tv_), NULL);}
		Timestamp(const Timestamp& t)	{this->tv_ = t.tv();}
		explicit Timestamp(int64_t sec)	{this->tv_.tv_sec = sec; this->tv_.tv_usec = 0;}
		explicit Timestamp(int64_t sec,int64_t usec):Timestamp(sec)	{this->tv_.tv_usec = usec;this->normalize();}
		~Timestamp() = default;

		const timeval& tv() const {return this->tv_;}
		int64_t tv_sec() const 	{return this->tv_.tv_sec;}
		int64_t tv_usec() const {return this->tv_.tv_usec;}
	
		int64_t unixstamp() const {return tv_.tv_sec;}
		int64_t long_unixstamp() const {return tv_.tv_sec * 1e6 + tv_.tv_usec;}

		void normalize();
		bool valid() const;
		void add_time(int64_t sec,int64_t usec);
		Timestamp& operator +=(const Timestamp& t2);
		Timestamp& operator -=(const Timestamp& t2);
		bool operator >(const Timestamp& t2) const;
		bool operator >=(const Timestamp& t2) const;
		bool operator <(const Timestamp& t2) const;
		bool operator <=(const Timestamp& t2) const;
	private:
		timeval tv_;
	};

	extern Timestamp gettimeofday();
	extern int64_t unixstamp();
	extern int64_t long_unixstamp();
	Timestamp operator +(const Timestamp& t1, const Timestamp& t2);
	Timestamp operator -(const Timestamp& t1, const Timestamp& t2);
}//namespace sc

#endif