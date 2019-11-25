/*
* --*ClientEntity*--
* @Author: Payton
* @Last  : Payton
*/

#ifndef _CLIENTENTITY_H_
#define _CLIENTENTITY_H_

#include <list>
#include <mutex>
#include <stdint.h>
#include <Common.h>
namespace sc
{
class EventManager;
class Buffer;
class TcpManager;

#define FLAG_CLOSE		0B100
#define FLAG_NEXT		0B010
#define FLAG_HANDLE		0B001
#define FLAG_UPDATE		0B011
#define FLAG_FINISH		0B100

#define FLAG_NOREAD		0B01
#define FLAG_NOWRITE	0B10

class ClientEntity
{
public:
	ClientEntity();
	~ClientEntity();

	int fd(){return fd_;}
	uint64_t fd_key(){return fd_key_;}
	uint32_t loop_index(){return loop_index_;}
	void set_loop_index(uint32_t loop_index){loop_index_ = loop_index;}
	void set_fd_key(uint64_t fd_key){fd_key_ = fd_key;}
	bool is_close(){return close_;}
	void set_close(){close_ = true;}
	void set_event_loop(EventManager* event_loop){event_loop_ = event_loop;}
	void set_msg_callback(MSG_FUNC func){this->message_callback_ = func;}
	void set_established_callback(UINT64_FUNC func){this->established_callback_ = func;}
	void set_tcp_manager(TcpManager* manager_){this->tcp_manager_ = manager_;}
	UINT64_FUNC established_callback(){return this->established_callback_;}

public:
	void init(int fd,const char* addr_ip,uint16_t addr_port);
	void req_close();
	void close();
	void handle_event(int fd, uint32_t events);

	int push_send_pack(Buffer* buff);
	void handle_unpack();
	void timeout_send();

public:
	bool can_close(){return (this->is_close() == false && read_flag_ & FLAG_CLOSE) > 0;}
	bool is_update(){return (read_flag_ & FLAG_UPDATE) == 2;}
	void handle_update(){__sync_fetch_and_or(&read_flag_,FLAG_HANDLE);}
	void finish_update(){__sync_fetch_and_and(&read_flag_,FLAG_FINISH);}
	void set_update(){__sync_fetch_and_or(&read_flag_,FLAG_NEXT);}
	void force_close(){__sync_fetch_and_or(&read_flag_,FLAG_CLOSE);}

	bool is_write_update(){return (write_flag_ & FLAG_UPDATE) == 2;}
	void handle_write_update(){__sync_fetch_and_or(&write_flag_,FLAG_HANDLE);}
	void set_write_update(){__sync_fetch_and_or(&write_flag_,FLAG_NEXT);}
	void finish_write_update(){__sync_fetch_and_and(&write_flag_,FLAG_FINISH);}

	bool ready_close(){return close_flag_ > 0;}
	bool can_read(){return (close_flag_ & FLAG_NOREAD) == 0;}
	bool can_write(){return (close_flag_ & FLAG_NOWRITE) == 0;}
	void close_read(){GUARD_LOCK(_guard_lock,callback_mutex_);__sync_fetch_and_or(&close_flag_,FLAG_NOREAD);}
	void close_write(){__sync_fetch_and_or(&close_flag_,FLAG_NOWRITE);}
private:
	int read_data();
	void do_handle_unpack();
	int write_data();
	int do_write_data();

	void finish_unpack(Buffer*);
	void watch_write(int opt);
	void watch_read(int opt);

	void clear();
private:
	volatile int read_flag_;
	volatile int close_flag_;//关闭收/发
	volatile int write_flag_;
	bool close_;
	int fd_;
	uint64_t fd_key_;
	char addr_ip_[128];
	uint16_t addr_port_;

	MSG_FUNC message_callback_;
	UINT64_FUNC established_callback_;

	uint32_t loop_index_;
	EventManager* event_loop_;
	TcpManager* tcp_manager_;

	std::recursive_mutex read_mutex_;
	Buffer* cur_unpcak_;
	Buffer* front_unpcak_;
	std::list<Buffer*> unpack_list_;
	std::list<Buffer*> recv_list_;

	std::recursive_mutex write_mutex_;
	std::list<Buffer*> wait_pack_list_;
	std::list<Buffer*> wait_write_list_;
	bool wait_write_;

	std::recursive_mutex send_mutex_;
	std::recursive_mutex unpack_mutex_;
	std::recursive_mutex callback_mutex_;
};

}//namespace sc

#endif //_CLIENTENTITY_H_