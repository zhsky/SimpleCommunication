/*
* --*Log*--
* @Author: Payton
* @Last  : Payton
*/
#ifndef _LOG_H_
#define _LOG_H_

#include <iostream>
#include <sstream>
#include <cstring>
#include <execinfo.h>

template<typename... Args>
void log(const std::string& fmt, Args... args)
{
	char log_buff[1024]={'\0'};
	snprintf(log_buff,1023,fmt.c_str(),args...);
	std::cout << log_buff << std::endl;
}

template<typename... Args>
void log_trace(const std::string& fmt, Args... args)
{
	std::ostringstream io_stream;
	char log_buff[1024]={'\0'};
	snprintf(log_buff,1023,fmt.c_str(),args...);
	io_stream << log_buff << std::endl;

	void* buffer[512];
	int nptrs = backtrace(buffer,512);
	char** strings = backtrace_symbols(buffer,nptrs);
	for(int i = 0; i < nptrs; ++i)
		io_stream << strings[i];
	io_stream << std::endl;
	std::cerr << io_stream.str();
}

#define LOG_ERROR(FMT, ...) log("ERROR %s:%d %s: "#FMT,__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define LOG_INFO(FMT, ...) log("INFO  %s:%d %s: "#FMT,__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define LOG_WARNING(FMT, ...) log("WARN  %s:%d %s: "#FMT,__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define LOG_TRACE(FMT, ...) log_trace("TRACE  %s:%d %s: "#FMT,__FILE__, __LINE__, __func__, ##__VA_ARGS__)

#ifdef DEBUG_MODULE
#define LOG_DEBUG(FMT, ...) log("DEBUG %s:%d %s: "#FMT,__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#define LOG_DEBUG(FMT, ...)
#endif	//DEBUG_MODULE

#endif	//_LOG_H_