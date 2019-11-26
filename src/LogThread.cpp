/*
* --*LogThread*--
* @Author: Payton
* @Last  : Payton
*/
#include <LogThread.h>
#include <Common.h>
#include <Thread.h>
#include <LogFile.h>

#include <stdio.h>
#include <stdarg.h>
#include <execinfo.h>
#include <time.h>
#include <iomanip>

namespace sc
{
void log_info(int log_type,const std::string& fmt, ...)
{
	struct tm tm_v;
	time_t time_v = time(NULL);
	localtime_r(&time_v, &tm_v);
	Buffer* log_buff = LOG_INSTANCE->pop_buff_log();

	snprintf(log_buff->readptr(),11,"[%.2d:%.2d:%.2d]",tm_v.tm_hour,tm_v.tm_min,tm_v.tm_sec);
	log_buff->set_write_index(10);

	va_list args;
	va_start(args, fmt);
	size_t len = vsnprintf(log_buff->writeptr(),log_buff->writeableBytes() - 1,fmt.c_str(), args);
	va_end (args);
	log_buff->set_write_index(log_buff->get_write_index() + len);
	log_buff->append("\n",1);

	if(log_type == 2)
	{
		void* buffer[512];
		int nptrs = backtrace(buffer,512);
		char** strings = backtrace_symbols(buffer,nptrs);
		for(int i = 0; i < nptrs; ++i)
			log_buff->append(strings[i],strlen(strings[i]));
		log_buff->append("\n",1);
	}
	LOG_INSTANCE->log_info(log_buff);
}

Log::Log()
{
	this->running_ = false;
	this->loop_thread_ = new Thread();
	this->file_ = new LogFile();

	this->log_list_.clear();
	this->write_list_.clear();
}

Log::~Log()
{
	this->running_ = false;
	delete this->loop_thread_;
	delete this->file_;
	this->log_list_.clear();
	this->write_list_.clear();
}

void Log::log_info(Buffer* buff)
{
	GUARD_LOCK(__logck_guard,list_mutex_);
	this->write_list_.push_back(buff);
	out_cond_.notify_one();
}

void Log::loop_write()
{
	do{
		{
			UNIQUE_LOCK(__logck_guard,list_mutex_);
			if(this->write_list_.empty())
			{
				out_cond_.wait(__logck_guard);
			}
			this->log_list_.splice(this->log_list_.end(),this->write_list_);
		}

		while(this->log_list_.empty() == false)
		{
			Buffer* buff = this->log_list_.front();
			this->file_->append(buff->readptr(),buff->readableBytes());
			this->buff_pool_.push(buff);
			this->log_list_.pop_front();
		}
	}while(running_);
}

int Log::start(const std::string& base_filename)
{
	if(this->running_)
	{
		fprintf(stderr,"WARN RUNNING\n");
		return -1;
	}

	this->running_ = true;
	this->file_->init(base_filename);
	this->loop_thread_->init_func(std::bind(&Log::loop_write,this),"LogThread");
	this->loop_thread_->start();
	return 0;
}

int Log::stop()
{
	if(!running_) return 0;
	running_ = false;
	out_cond_.notify_one();
	this->loop_thread_->stop();
	return 0;
}

}//namespace sc