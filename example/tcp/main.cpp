/*
* --*main*--
* @Author: Payton
* @Last  : Payton
*/
#include <unistd.h>
#include <signal.h>
#include <Log.h>
#include <TcpServer.h>
#include <TcpClient.h>
#include <Buffer.h>
#include <ClientEntity.h>
#include <string.h>
#include "LogThread.h"

using namespace sc;
bool running;

void server_handle_msg(Buffer* buff)
{
	LOG_INFO("readableBytes %ld",buff->readableBytes());
	TCPSERVER_INSTANCE->push_send_data(buff);
	// int64_t ID;
	// std::string Name;
	// buff->readInt64(ID);
	// buff->readString(Name);
	// LOG_INFO("%ld,%s",ID,Name.c_str());
}

void client_handle_msg(Buffer* buff)
{
	Buffer* send_buff = TCPCLIENT_INSTANCE->pop_buffer();
	send_buff->append(buff->readptr(),buff->readableBytes());
	TCPCLIENT_INSTANCE->push_send_data(send_buff);
	TCPCLIENT_INSTANCE->push_buffer(send_buff);

	LOG_INFO("readBytes %ld",buff->readableBytes());
	uint64_t fd_key;
	int32_t buff_size;
	std::string data;
	buff->readInt64(fd_key);
	buff->readInt32(buff_size);
	buff->readString(data);
	LOG_INFO("fd_key:%lld,%s",fd_key,data.c_str());

}

void client_send_data(uint64_t fd_key)
{
	Buffer* buff = TCPCLIENT_INSTANCE->pop_buffer();

	std::string data = "test client";
	uint16_t len = data.length();
	uint32_t buff_size = data.length() + sizeof(uint16_t);
	buff->writeInt64(fd_key);
	buff->writeInt32(buff_size);
	buff->writeInt16(len);
	buff->append(data.c_str(),len);
	TCPCLIENT_INSTANCE->push_send_data(buff);

	LOG_INFO("data.length:%d,writeBytes %ld",data.length(),buff->readableBytes());
	TCPCLIENT_INSTANCE->push_buffer(buff);
}

void signal_handle(int signo)
{
	if(signo == 50)
		TCPSERVER_INSTANCE->pool_dump();
	else if(signo == 51)
	{
		TCPSERVER_INSTANCE->stop();
		running = false;
	}
	else if(signo == 52)
	{
		TCPCLIENT_INSTANCE->pool_dump();
	}
	else if(signo == 53)
	{
		TCPCLIENT_INSTANCE->stop();
		running = false;
	}
}

int start_server()
{
	running = true;
	signal(50,signal_handle);
	signal(51,signal_handle);
	TCPSERVER_INSTANCE->init("0.0.0.0",6000,10);
	TCPSERVER_INSTANCE->set_msg_callback(server_handle_msg);
	TCPSERVER_INSTANCE->start();
	while(running) sleep(10000);
	return 0;
}

int start_client()
{
	running = true;
	signal(52,signal_handle);
	signal(53,signal_handle);
	TCPCLIENT_INSTANCE->start();
	TCPCLIENT_INSTANCE->connect("192.168.16.117",6000,client_handle_msg,client_send_data);
	while(running) sleep(10000);
	return 0;
}

int main(int argc, char *argv[])
{
	LOG_INSTANCE->start();
	if(argc == 2)
	{
		if(strcmp(argv[1],"server") == 0) start_server();
		if(strcmp(argv[1],"client") == 0) start_client();
	}
	LOG_INSTANCE->stop();
}