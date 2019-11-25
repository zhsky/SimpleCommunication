/*
* --*TcpClient*--
* @Author: Payton
* @Last  : Payton
*/

#include <TcpClient.h>
#include <TcpManager.h>
#include <ThreadMutex.h>

#include <signal.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

namespace sc
{
TcpClient::TcpClient():running_(false),tcp_manager_(nullptr),self_manager_(false)
{
}

TcpClient::~TcpClient()
{
	this->stop();
	if(self_manager_) delete this->tcp_manager_;
}

void TcpClient::set_tcp_manager(TcpManager* tcp_manager)
{
	if(this->tcp_manager_ != nullptr)
	{
		LOG_ERROR("CLIENT TCP MANAGER EXIST");
		return;
	}
	self_manager_ = false;
	this->tcp_manager_ = tcp_manager;
}

void TcpClient::start()
{
    ::signal(SIGPIPE, SIG_IGN);
	running_ = true;
	if(this->tcp_manager_ == nullptr)
	{
		this->tcp_manager_ = new TcpManager();
		self_manager_ = true;
	}
	tcp_manager_->init(1);
	this->tcp_manager_->start();
}

void TcpClient::stop()
{
	if(!running_) return;
	running_ = false;
	this->tcp_manager_->stop();
}

int TcpClient::connect(std::string ip,int port,MSG_FUNC message_callback,UINT64_FUNC established_callback)
{
	int connect_fd = 0;

	if((connect_fd = ::socket(AF_INET,SOCK_STREAM,0)) < 0)
	{
		LOG_ERROR("ERROR socket %s",strerror(errno));
		return -1;
	}

	struct sockaddr_in addr;
	memset(&addr,0,sizeof(sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	if(inet_pton(AF_INET,ip.c_str(),&addr.sin_addr) < 0)
	{
		LOG_ERROR("ERROR inet_pton %s",strerror(errno));
		return -1;
	}

	if(::connect(connect_fd, (struct sockaddr *)&addr,sizeof(addr)) < 0)
	{
		LOG_ERROR("ERROR connect %s",strerror(errno));
		return -1;
	}

	int recv_buff_size = 1 * 1024 * 1024;
	if(::setsockopt(connect_fd,SOL_SOCKET,SO_RCVBUF,&recv_buff_size,sizeof(recv_buff_size)) < 0)
	{
		LOG_ERROR("ERROR socket SO_RCVBUF %s",strerror(errno));
		close(connect_fd);
		return -1;
	}

	int send_buff_size = 1 * 1024 * 1024;
	if(::setsockopt(connect_fd,SOL_SOCKET,SO_SNDBUF,&send_buff_size,sizeof(send_buff_size)) < 0)
	{
		LOG_ERROR("ERROR socket SO_SNDBUF %s",strerror(errno));
		close(connect_fd);
		return -1;
	}

	int delay_flag = 1;
	if(::setsockopt(connect_fd,IPPROTO_TCP,TCP_NODELAY,&delay_flag,sizeof(delay_flag)) < 0)
	{
		LOG_ERROR("ERROR socket TCP_NODELAY %s",strerror(errno));
		close(connect_fd);
		return -1;
	}
	if(fcntl(connect_fd,F_SETFL,fcntl(connect_fd,F_GETFL,0)|O_NONBLOCK) < 0)
	{
		LOG_ERROR("ERROR socket O_NONBLOCK %s",strerror(errno));
		close(connect_fd);
		return -1;
	}

	LOG_INFO("%d connected %s:%d",connect_fd,ip.c_str(),port);

	uint64_t fd_key = this->tcp_manager_->insert_fd(connect_fd);
	ClientEntity* client = this->tcp_manager_->pop_client();
	client->set_tcp_manager(this->tcp_manager_);
	client->set_msg_callback(message_callback);
	client->set_established_callback(established_callback);
	client->set_fd_key(fd_key);
	client->init(connect_fd,ip.c_str(),port);
	this->tcp_manager_->add_client(client);
	return fd_key;
}

void TcpClient::disconnect(uint64_t fd_key)
{
	this->tcp_manager_->close_client(fd_key);
}

void TcpClient::pool_dump()
{
	this->tcp_manager_->pool_dump();
}

int TcpClient::push_send_data(Buffer* buff)
{
	return this->tcp_manager_->push_send_pack(buff);
}

Buffer* TcpClient::pop_buffer()
{
	return this->tcp_manager_->pop_buffer();
}
void TcpClient::push_buffer(Buffer* buf)
{
	this->tcp_manager_->push_buffer(buf);
}

}//namespace sc