/*
* --*main*--
* @Author: Payton
* @Last  : Payton
*/
#include <vector>
#include <memory>
#include <Log.h>
#include "LogThread.h"

using namespace sc;

class Simple;
sc::ObjectPool<Simple> pool;
class Simple
{
public:
	Simple():size_(0){LOG_INFO("Simple");}
	~Simple(){LOG_INFO("~Simple");}

	void push(int x){vec_.push_back(x);size_ = vec_.size();}
	void info(){
		LOG_INFO("Simple size %d",size_);
		for(auto i: vec_)
			LOG_INFO("Simple vec %d",i);
	}
	void operator delete(void *p)
	{
		LOG_INFO("delete");
		Simple* ptr = static_cast<Simple*>(p);
		pool.push(ptr);
	}
	std::vector<int> vec_;
	size_t size_;
};

void func1()
{
	std::shared_ptr<Simple> p(pool.pop());
	pool.dump_info();
}

int main()
{
	LOG_INSTANCE->start();
	func1();
	LOG_INSTANCE->stop();
	return 0;
}

void func()
{
	std::list<Simple*> obj_list;
	LOG_INFO("pop");
	for(int i = 0; i  < 7; ++i)
	{
		obj_list.push_back(pool.pop());
	}
	pool.dump_info();

	LOG_INFO("push");
	for(auto& ptr : obj_list)
		pool.push(ptr);
	obj_list.clear();
	pool.dump_info();

	LOG_INFO("shrink");
	pool.shrink();
	pool.dump_info();

	LOG_INFO("");
	Simple* ptr = pool.pop();
	ptr->push(1);
	ptr->push(2);
	ptr->info();
	pool.push(ptr),ptr = nullptr;

	ptr = pool.pop();
	ptr->info();
}