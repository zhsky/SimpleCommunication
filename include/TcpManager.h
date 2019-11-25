/*
* --*TcpManager*--
* @Author: Payton
* @Last  : Payton
*/
#ifndef _TCPMANAGER_H_
#define _TCPMANAGER_H_

#include <noncopyable.h>
#include <ObjectPool.h>
#include <EventHandle.h>
#include <Buffer.h>
#include <ClientEntity.h>

#include <unordered_map>
#include <string>
#include <stdint.h>
#include <pthread.h>

namespace sc
{
class TaskThread;
class ThreadPool;

class TcpManager:noncopyable
{
public:
	TcpManager();
	~TcpManager();
	int init(size_t event_size);
	int start();
	void stop();
	void pool_dump();
	void set_msg_callback(MSG_FUNC func){this->message_callback_ = func;}

	Buffer* pop_buffer(){return this->buffer_pool_.pop();}
	void push_buffer(Buffer* buf){this->buffer_pool_.push(buf);}

	ClientEntity* pop_client(){return this->client_pool_.pop();}
	void push_client(ClientEntity* client){this->client_pool_.push(client);}

	EventHandle* pop_handle(){return this->handle_pool_.pop();}
	void push_handle(EventHandle* handle){this->handle_pool_.push(handle);}

public:
	uint64_t insert_fd(int fd);
	int add_acceptor(EventHandle* handle);
	void add_client(ClientEntity*);
	void remove_client(int fd);
	void send_timeout(int64_t now);
	void close_client(uint64_t fd_key);
	int push_send_pack(Buffer* buff);
private:
	void start_send_timer();
	void do_add_client(ClientEntity*);
	void do_remove_client(int fd);
	void do_send_timeout();
	uint32_t event_index(){++acceptor_index_; acceptor_index_ = acceptor_index_ % event_size_; return acceptor_index_;}
	ClientEntity* get_client(int fd);

	int get_fd(uint64_t key);
	void remove_fd(uint64_t key);
private:
	bool init_;
	bool running_;

	size_t event_size_;

	ObjectPool<Buffer> buffer_pool_;
	ObjectPool<ClientEntity> client_pool_;
	ObjectPool<EventHandle> handle_pool_;

	MSG_FUNC message_callback_;
	std::unordered_map<int,ClientEntity*> client_map_;//main_thread_Ïß³Ì¶ÁÐ´£¬ÆäËûÏß³Ì¶Á

	uint32_t acceptor_index_;
	EventManager* acceptor_loop_;
	EventManager* socket_loop_;
	ThreadPool* packer_thread_;
	TaskThread* main_thread_;
	
	pthread_rwlock_t rwlock_;

	pthread_rwlock_t map_rwlock_;
	volatile uint64_t fd_key_;
	std::unordered_map<uint64_t,int> connect_map_;
};

}//namespace sc



#endif //_TCPMANAGER_H_