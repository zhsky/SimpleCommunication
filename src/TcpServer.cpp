/*
* --*TcpServer*--
* @Author: Payton
* @Last  : Payton
*/

#include <TcpServer.h>
#include <TcpManager.h>
#include <Log.h>

#include <signal.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

using std::placeholders::_1;
using std::placeholders::_2;
namespace sc
{
TcpServer::TcpServer():ip_(""),port_(0),reuseaddr_(0),init_(false),running_(false),self_manager_(false),listen_fd_(nullptr),tcp_manager_(nullptr)
{
}

TcpServer::~TcpServer()
{
	if(running_) this->stop();
	if(self_manager_) delete tcp_manager_;
	if(init_)
	{
		delete[] listen_fd_;
		init_ = false;
	}
}

void TcpServer::set_tcp_manager(TcpManager* tcp_manager)
{
	if(this->tcp_manager_ != nullptr)
	{
		LOG_ERROR("SERVER TCP MANAGER EXIST");
		return;
	}
	self_manager_ = false;
	this->tcp_manager_ = tcp_manager;
}

void TcpServer::pool_dump()
{
	this->tcp_manager_->pool_dump();
}

void TcpServer::handle_accept(int fd, uint32_t events)
{
	struct sockaddr_in addr;
	socklen_t addr_len;
	char addr_ip[128];
	
	memset(&addr,0,sizeof(addr));
	addr_len = sizeof(addr);
	while(true)
	{
		int connfd = accept4(fd,(struct sockaddr*)&addr,&addr_len,SOCK_NONBLOCK|SOCK_CLOEXEC);
		if(connfd < 0)
		{
			if(errno == EINTR) continue;
			if(errno == ECONNABORTED || errno == EAGAIN)
				return;
			LOG_ERROR("ERROR socket accept4 %s",strerror(errno));
			return;
		}

		int recv_buff_size = 1 * 1024 * 1024;
		if(::setsockopt(connfd,SOL_SOCKET,SO_RCVBUF,&recv_buff_size,sizeof(recv_buff_size)) < 0)
		{
			LOG_ERROR("ERROR socket SO_RCVBUF %s",strerror(errno));
			close(connfd);
			continue;
		}

		int send_buff_size = 1 * 1024 * 1024;
		if(::setsockopt(connfd,SOL_SOCKET,SO_SNDBUF,&send_buff_size,sizeof(send_buff_size)) < 0)
		{
			LOG_ERROR("ERROR socket SO_SNDBUF %s",strerror(errno));
			close(connfd);
			continue;
		}

		int delay_flag = 1;
		if(::setsockopt(connfd,IPPROTO_TCP,TCP_NODELAY,&delay_flag,sizeof(delay_flag)) < 0)
		{
			LOG_ERROR("ERROR socket TCP_NODELAY %s",strerror(errno));
			close(connfd);
			continue;
		}

		memset(addr_ip,0,sizeof(addr_ip));
	    ::inet_ntop(AF_INET, &addr.sin_addr, addr_ip, addr_len);

		LOG_INFO("accept socket %d,%s:%d",connfd,addr_ip,ntohs(addr.sin_port));

		uint64_t fd_key = this->tcp_manager_->insert_fd(connfd);
		ClientEntity* client = this->tcp_manager_->pop_client();
		client->set_tcp_manager(this->tcp_manager_);
		client->set_fd_key(fd_key);
		client->init(connfd,addr_ip,ntohs(addr.sin_port));
		this->tcp_manager_->add_client(client);
	}
}

int TcpServer::init(std::string ip,int port,size_t reuseaddr)
{
	if(init_)
	{
		LOG_ERROR("ERROR HAD INIT");
		return -1;
	}
    ::signal(SIGPIPE, SIG_IGN);
	init_ = true;
	ip_ = ip;
	port_ = port;
	reuseaddr_ = reuseaddr > 0 ? reuseaddr : 1;
	listen_fd_ = new int[reuseaddr_]();

	if(this->tcp_manager_ == nullptr)
	{
		this->tcp_manager_ = new TcpManager();
		self_manager_ = true;
	}
	
	tcp_manager_->init(reuseaddr_);
	return 0;
}


int TcpServer::start()
{
	if(!init_)
	{
		LOG_ERROR("ERROR NOT INIT");
		abort();
	}
	if(running_)
	{
		LOG_ERROR("ERROR RENNING");
		return -1;
	}
	running_ = true;
	for(uint32_t i = 0; i < reuseaddr_; ++i)
	{
		this->socket_init(i);
	}
	this->tcp_manager_->start();
	LOG_INFO("TcpServer start");
	return 0;
}

void TcpServer::stop()
{
	if(!running_) return;
	running_ = false;
	this->tcp_manager_->stop();
	memset(listen_fd_,0,sizeof(int) * reuseaddr_);
	LOG_INFO("TcpServer stop");
}

void TcpServer::set_msg_callback(MSG_FUNC func)
{
	this->tcp_manager_->set_msg_callback(func);
}

void TcpServer::socket_init(uint32_t index)
{
	this->do_init_socket(index);

	char handle_name[32] = {'\0'};
	snprintf(handle_name,31,"handle_name%d",index);

	EventHandle* handle = this->tcp_manager_->pop_handle();
	handle->set_recycle_func(std::bind(&TcpManager::push_handle,this->tcp_manager_,::_1));
	IO_FUNC func = std::bind(&TcpServer::handle_accept,this,::_1,::_2);
	handle->bind(func,listen_fd_[index],std::string(handle_name));
	if(handle->fd() > 0 && this->tcp_manager_->add_acceptor(handle) == 0) return;

	LOG_ERROR("index:%d add_handle %d ERROR",index,handle->fd());
	abort();
}

void TcpServer::do_init_socket(uint32_t index)
{
	if((listen_fd_[index] = ::socket(AF_INET,SOCK_STREAM|SOCK_NONBLOCK,0)) < 0)
	{
		LOG_ERROR("ERROR socket %s",strerror(errno));
		abort();
	}

	struct sockaddr_in addr;
	memset(&addr,0,sizeof(sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port_);

	if(inet_pton(AF_INET,this->ip_.c_str(),&addr.sin_addr) < 0)
	{
		LOG_ERROR("ERROR inet_pton %s",strerror(errno));
		abort();
	}
	
	int addr_flag = 1;
	if(::setsockopt(listen_fd_[index],SOL_SOCKET,SO_REUSEADDR,&addr_flag,sizeof(addr_flag)) < 0)
	{
		LOG_ERROR("ERROR socket SO_REUSEADDR %s",strerror(errno));
		abort();
	}

	if (reuseaddr_ > 1)
	{
		int port_flag = 1;
		if(::setsockopt(listen_fd_[index],SOL_SOCKET,SO_REUSEPORT,&port_flag,sizeof(port_flag)) < 0)
		{
			LOG_ERROR("ERROR socket SO_REUSEPORT %s",strerror(errno));
			abort();
		}
	}
	
	if(::bind(listen_fd_[index], (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		LOG_ERROR("ERROR socket bind %s",strerror(errno));
		abort();
	}
	
	if(::listen(listen_fd_[index], 1024) < 0)
	{
		LOG_ERROR("ERROR socket listen %s",strerror(errno));
		abort();
	}
}

int TcpServer::push_send_data(Buffer* buff)
{
	return this->tcp_manager_->push_send_pack(buff);
}

}