/*
* --*LogFile*--
* @Author: Payton
* @Last  : Payton
*/

#ifndef _LOGFILE_H_
#define _LOGFILE_H_
#include <string>

namespace sc
{

#define FILEBUFF_SIZE 16*4096 
class LogFile
{
public:
	LogFile();
	~LogFile();

	void init(const std::string& base_filename);
	void flush();
	void append(const char*,size_t);
private:
	void open_fp();
	void close_fp();
	std::string get_logfile_name();
	void check_upadte_fp();
private:
	uint32_t write_size_;
	char buffer_[FILEBUFF_SIZE];

	std::string base_filename_;
	FILE* fp_;
};


}//namespace sc

#endif //_LOGFILE_H_