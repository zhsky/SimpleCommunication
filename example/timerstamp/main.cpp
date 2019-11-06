/*
* --*main*--
* @Author: Payton
* @Last  : Payton
*/
#include <Log.h>
#include <Timestamp.h>
#include "Common.h"

using namespace sc;

Timestamp add_test()
{
	Timestamp t1,t2(10,0);
	LOG_INFO("add_test unixstamp:%ld",t1.unixstamp());
	return t1 + t2;
}

Timestamp sub_test()
{
	Timestamp t1,t2(10,0);
	LOG_INFO("add_test unixstamp:%ld",t1.unixstamp());
	return t1 - t2;
}

void test()
{
	Timestamp t1(std::move(add_test()));
	LOG_INFO("add_test unixstamp:%ld long_unixstamp:%ld",t1.unixstamp(),t1.long_unixstamp());
	LOG_INFO("add_test tv_sec:%ld,tv_usec:%ld",t1.tv().tv_sec,t1.tv().tv_usec);

	Timestamp t2(t1);
	LOG_INFO("t1 == t2 true,%d",(t1 >= t2));
	LOG_INFO("t1 == t2 true,%d",(t1 <= t2));
	t2 = std::move(sub_test());
	LOG_INFO("t1 >= t2 true,%d",(t1 >= t2));
	LOG_INFO("t2 <= t1 true,%d",(t2 <= t1));

	Timestamp t3(t1.unixstamp() + 10);
	LOG_INFO("t3 > t1 true,%d",(t3 > t1));
	LOG_INFO("t1 < t3 true,%d",(t1 < t3));

	t1 -= t1;
	LOG_INFO("valid,long_unixstamp:%ld,%d",t1.long_unixstamp(),t1.valid());

	t1.add_time(0,USEC);
	LOG_INFO("add_test tv_sec:%ld,tv_usec:%ld",t1.tv().tv_sec,t1.tv().tv_usec);
}

int main()
{
	test();
	return 0;
}