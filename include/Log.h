/*
* --*Log*--
* @Author: Payton
* @Last  : Payton
*/
#ifndef _LOG_H_
#define _LOG_H_

#include <string>
namespace sc
{
extern void log_info(int log_type,const std::string& fmt, ...);

#define LOG_ERROR(FMT, ...) log_info(1,"ERROR %s:%d %s: "#FMT,__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define LOG_INFO(FMT, ...) log_info(1,"INFO  %s:%d %s: "#FMT,__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define LOG_WARNING(FMT, ...) log_info(1,"WARN  %s:%d %s: "#FMT,__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define LOG_TRACE(FMT, ...) log_info(2,"TRACE  %s:%d %s: "#FMT,__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define LOG_STDEER(FMT, ...) fprintf(stderr,"FATAL ERROR %s:%d %s: "#FMT,__FILE__, __LINE__, __func__, ##__VA_ARGS__)

#ifdef DEBUG_MODULE
#define LOG_DEBUG(FMT, ...) log_info(1,"DEBUG %s:%d %s: "#FMT,__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#define LOG_DEBUG(FMT, ...)
#endif	//DEBUG_MODULE

}//namespace sc

#endif	//_LOG_H_