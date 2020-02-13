/*
* --*LogFile*--
* @Author: Payton
* @Last  : Payton
*/
#include <LogFile.h>
#include <stdio.h>
#include <string.h>


namespace sc
{
#define FILE_SIZE 100 * 1024 *1024

LogFile::LogFile():write_size_(0),fp_(nullptr)
{
}

LogFile::~LogFile()
{
	this->close_fp();
}

void LogFile::init(const std::string& base_filename)
{
	this->base_filename_ = base_filename;
	this->open_fp();
}

void LogFile::flush()
{
	::fflush(fp_);
}

void LogFile::open_fp()
{
	std::string fname = get_logfile_name();
	if((fp_ = ::fopen(fname.c_str(), "ae")) == nullptr)
	{
		fprintf(stderr,"ERROR fopen,%s,%s",fname.c_str(),strerror(errno));
		abort();
	}
	::setbuffer(fp_, buffer_, FILEBUFF_SIZE);
}

void LogFile::close_fp()
{
	if(fp_ != nullptr)
	{
		this->flush();
		fclose(fp_);
		fp_ = nullptr;
	}
}

std::string LogFile::get_logfile_name()
{
	std::string filename(this->base_filename_);

	char timebuff[32];
	time_t lt;
	struct tm* ptr;
	lt=time(NULL);
	ptr=localtime(&lt);
	strftime(timebuff,31,"%Y%m%d-%H%M%S",ptr);
	filename.append(timebuff,strlen(timebuff));
	
	return filename;
}

void LogFile::check_upadte_fp()
{
	if(write_size_ >= FILE_SIZE)
	{
		this->close_fp();
		this->open_fp();
		write_size_ = 0;
	}
}

void LogFile::append(const char* log,size_t len)
{
	size_t write_len = fwrite_unlocked(log,1,len,fp_);
	size_t remain = len;
	remain -= write_len;
	while(remain > 0)
	{
		size_t x = fwrite_unlocked(log + write_len,1,remain,fp_);
		if(x == 0) break;
		write_len += x;
		remain -= x;
	}
	write_size_ += write_len;
	this->check_upadte_fp();
}

}//namespace sc