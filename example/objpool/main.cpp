/*
* --*main*--
* @Author: Payton
* @Last  : Payton
*/
#include <vector>
#include <Log.h>
#include <ObjectPool.h>

class Simple
{
public:
	Simple():size_(0){}
	~Simple(){}

	void push(int x){vec_.push_back(x);size_ = vec_.size();}
	void info(){
		LOG_INFO("Simple size %d",size_);
		for(auto i: vec_)
			LOG_INFO("Simple vec %d",i);
	}

	std::vector<int> vec_;
	size_t size_;
};
int main()
{
	sc::ObjectPool<Simple> pool(3);
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

	return 0;
}