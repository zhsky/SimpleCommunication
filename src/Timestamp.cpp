/*
* --*Timestamp*--
* @Author: Payton
* @Last  : Payton
*/
#include "Common.h"
#include "Timestamp.h"

namespace sc
{
	void Timestamp::normalize()
	{
		if(this->tv_.tv_usec >= USEC_PER_SEC)
		{
			do{
				this->tv_.tv_sec++;
				this->tv_.tv_usec -= USEC_PER_SEC;
			}while(this->tv_.tv_usec >= USEC_PER_SEC);
		}
		else if(this->tv_.tv_usec < 0)
		{
			do{
				this->tv_.tv_sec--;
				this->tv_.tv_usec += USEC_PER_SEC;
			}while(this->tv_.tv_usec < 0 && this->tv_.tv_sec > 0);
		}
	}

	bool Timestamp::valid() const
	{
		if(this->tv_.tv_sec > 0)
		{
			return this->tv_.tv_usec >= 0;
		}
		else if(this->tv_.tv_sec == 0)
		{
			return this->tv_.tv_usec > 0;
		}
		else
			return false;
	}

	void Timestamp::add_time(int64_t sec,int64_t usec)
	{
		this->tv_.tv_sec += sec;
		this->tv_.tv_usec += usec;
		this->normalize();
		return ;
	}

	Timestamp& Timestamp::operator +=(const Timestamp& t2)
	{
		this->tv_.tv_sec += t2.tv_sec();
		this->tv_.tv_usec += t2.tv_usec();
		this->normalize();
		return *this;
	}
	Timestamp& Timestamp::operator -=(const Timestamp& t2)
	{
		this->tv_.tv_sec -= t2.tv_sec();
		this->tv_.tv_usec -= t2.tv_usec();
		this->normalize();
		return *this;
	}
	bool Timestamp::operator >(const Timestamp& t2) const
	{
		return this->long_unixstamp() > t2.long_unixstamp();
	}
	bool Timestamp::operator >=(const Timestamp& t2) const
	{
		return this->long_unixstamp() >= t2.long_unixstamp();
	}
	bool Timestamp::operator <(const Timestamp& t2) const
	{
		return this->long_unixstamp() < t2.long_unixstamp();
	}
	bool Timestamp::operator <=(const Timestamp& t2) const
	{
		return this->long_unixstamp() <= t2.long_unixstamp();
	}


	Timestamp operator +(const Timestamp& t1, const Timestamp& t2)
	{
		Timestamp t(t1);
		t += t2;
		return t;
	}
	Timestamp operator -(const Timestamp& t1, const Timestamp& t2)
	{
		Timestamp t(t1);
		t -= t2;
		return t;
	}

	Timestamp gettimeofday()	
	{
		return Timestamp();
	}

	int64_t unixstamp()	
	{
		return Timestamp().unixstamp();
	}

	int64_t long_unixstamp()
	{
		return Timestamp().long_unixstamp();
	}
}