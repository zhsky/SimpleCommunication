/*
* --*ObjectPool*--
* @Author: Payton
* @Last  : Payton
*/
#include <Common.h>
#include <noncopyable.h>
#include <list>
#include <map>
#include <mutex>

namespace sc
{
template<typename T>
class ObjectPool:noncopyable
{
public:
	ObjectPool(int block_size = 1024);
	~ObjectPool();

	T* pop();
	void push(T*);

	void dump_info();
	void shrink();//free all idle block
private:
	class Block: public std::list<T*>
	{
	public:
		int64_t _begin;
		int64_t _end;
		bool valid(int64_t addr){return _begin <= addr and addr <= _end;}
	};
	typedef std::pair<int64_t, Block> BlockNode;
private:
	BlockNode& make_new_block();
	void free_all();
private:
	std::list<BlockNode> block_list_;
	size_t block_size_;
	std::mutex block_mutex_;
};


template <typename T>
ObjectPool<T>::ObjectPool(int block_size):block_size_(block_size)
{
	LOG_INFO("ObjectPool<%s> INIT",typeid(T).name());
	this->make_new_block();
}

template <typename T>
ObjectPool<T>::~ObjectPool()
{
	LOG_INFO("ObjectPool<%s> deconstruct",typeid(T).name());
	this->free_all();
}

template <typename T>
typename ObjectPool<T>::BlockNode& ObjectPool<T>::make_new_block()
{
	void *p = malloc(sizeof(T) * this->block_size_);
	int64_t key = reinterpret_cast<int64_t>(p);

	Block obj_list;
	T* obj_ptr = static_cast<T*>(p);
	for(size_t i = 0; i < this->block_size_; ++i)
	{
		T *ptr= obj_ptr + i;
		obj_list.push_back(ptr);
	}
	obj_list._begin = reinterpret_cast<int64_t>(obj_list.front());
	obj_list._end = reinterpret_cast<int64_t>(obj_list.back());

	LOG_INFO("ObjectPool<%s> new_block begin:%ld,end:%ld",typeid(T).name(),obj_list._begin,obj_list._end);
	this->block_list_.emplace_back(key,std::move(obj_list));
	return this->block_list_.back();
}

template <typename T>
void ObjectPool<T>::free_all()
{
	for(auto& [key,value] : this->block_list_)
	{
		void* p = reinterpret_cast<void*>(key);
		free(p);
	}
	this->block_list_.clear();
}

template <typename T>
void ObjectPool<T>::dump_info()
{
	GUARD_LOCK(_lock_guard,block_mutex_);
	int64_t total_memory = this->block_list_.size() * this->block_size_ * sizeof(T);
	LOG_INFO("*****ObjectPool<%s> block_length:%d, block_size:%d,total_memory:%ld*****",
		typeid(T).name(),this->block_list_.size(),this->block_size_,total_memory);
	for(const auto& [_,obj_list] : this->block_list_)
	{
		LOG_INFO("*****ObjectPool<%s> IDLE:%d,begin:%ld,end:%ld*****",typeid(T).name(),obj_list.size(),obj_list._begin,obj_list._end);
	}
}

template <typename T>
void ObjectPool<T>::shrink()
{
	GUARD_LOCK(_lock_guard,block_mutex_);
	LOG_INFO("ObjectPool<%s> before shrink size:%d",typeid(T).name(),this->block_list_.size());
	for(auto iter = this->block_list_.begin(); iter != this->block_list_.end();)
	{
		if(this->block_list_.size() == 1) break;
		if(iter->second.size() == this->block_size_)
		{
			void* p = reinterpret_cast<void*>(iter->first);
			free(p);
			this->block_list_.erase(iter++);
			continue;
		}
	}
	LOG_INFO("ObjectPool<%s> after shrink size:%d",typeid(T).name(),this->block_list_.size());
}

template <typename T>
T* ObjectPool<T>::pop()
{
	static auto pop_list = [](auto& obj_list){
		T* ret = obj_list.front();
		obj_list.pop_front();
		ret = new(ret) T();
		return ret;
	};
	GUARD_LOCK(_lock_guard,block_mutex_);
	for(auto& [_,obj_list] : this->block_list_)
	{
		if(obj_list.empty()) continue;
		return pop_list(obj_list);
	}
	BlockNode& block_node = this->make_new_block();
	return pop_list(block_node.second);
}

template <typename T>
void ObjectPool<T>::push(T* obj)
{
	obj->~T();
	int64_t obj_index = reinterpret_cast<int64_t>(obj);

	{
		GUARD_LOCK(_lock_guard,block_mutex_);
		for(auto& [_,obj_list] : this->block_list_)
		{
			if(obj_list.valid(obj_index)) 
			{
				obj_list.push_back(obj);
				return;
			}
		}
	}

	LOG_ERROR("ERROR NOT FOUND BLOCK,%ld",obj_index);
	this->dump_info();
	return;
}




}//namespace sc