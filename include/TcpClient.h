/*
* --*TcpClient*--
* @Author: Payton
* @Last  : Payton
*/
#ifndef _TCPCLIENT_H_
#define _TCPCLIENT_H_

#include <noncopyable.h>
#include <Singleton.h>
#include <Common.h>

#include <string>
#include <pthread.h>

namespace sc
{
class TcpManager;
class Buffer;
class TcpClient:noncopyable
{
public:
	TcpClient();
	~TcpClient();

	void set_tcp_manager(TcpManager* tcp_manager);
	void start();
	void stop();

	uint64_t connect(std::string ip,int port,MSG_FUNC message_callback,UINT64_FUNC established_callback);
	void disconnect(uint64_t fd_key);

	void pool_dump();
	int push_send_data(Buffer* buff);
	Buffer* pop_buffer();
	void push_buffer(Buffer* buf);
private:
	bool running_;
	bool self_manager_;
	TcpManager* tcp_manager_;
};


#define TCPCLIENT_DESTORY Singleton<TcpClient>::destory()
#define TCPCLIENT_INSTANCE Singleton<TcpClient>::instance()

}//namespace sc

#endif //_TCPCLIENT_H_