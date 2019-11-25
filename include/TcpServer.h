/*
* --*TcpServer*--
* @Author: Payton
* @Last  : Payton
*/
#ifndef _TCPSERVER_H_
#define _TCPSERVER_H_

#include <noncopyable.h>
#include <Common.h>
#include <Singleton.h>

#include <stdint.h>

namespace sc
{
class TcpManager;

class TcpServer:noncopyable
{
public:
	TcpServer();
	~TcpServer();
	void handle_accept(int fd, uint32_t events);
	void set_tcp_manager(TcpManager* tcp_manager);
	
	int init(std::string ip,int port,size_t reuseaddr = 1);
	int start();
	void stop();

	void set_msg_callback(MSG_FUNC func);
	int push_send_data(Buffer* buff);
	void pool_dump();
private:
	void socket_init(uint32_t index);
	void do_init_socket(uint32_t index);

private:
	std::string ip_;
	int port_;
	size_t reuseaddr_;

	bool init_;
	bool running_;
	bool self_manager_;

	int* listen_fd_;
	TcpManager* tcp_manager_;
};

#define TCPSERVER_DESTORY Singleton<TcpServer>::destory()
#define TCPSERVER_INSTANCE Singleton<TcpServer>::instance()
}//namespace sc
#endif// _TCPSERVER_H_