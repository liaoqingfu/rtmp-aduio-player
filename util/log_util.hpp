/**
 * @file LogUtils.h
 * @brief 日志功能定义
 * @author kofera.deng <dengyi@comtom.cn>
 * @version 1.0.0
 * @date 2013-12-30
 */
#ifndef LOG_UTILS_H
#define LOG_UTILS_H

#include <string>
#include <string.h>
#include <stdio.h>


#if defined (_LOG4CPP)
#include <log4cpp/Category.hh>

namespace log4cpp
{
	class Category;
};

#endif /* _LOG4CPP */

void initLog4cpp();
void writeLog4cpp(char *str, int size);

void initializeLog4cpp(const std::string& logfile);
void releaseLog4cpp();

#if defined (_LOG4CPP)
log4cpp::Category& getLogCategory(const char* categoryName);
#endif

#if defined (_LOG4CPP)
#define DECLARE_STATIC_LOG() static log4cpp::Category& log1
#define DEFINE_STATIC_LOG(ClassName) log4cpp::Category& ClassName::log = getLogCategory(#ClassName)
#else
#define DECLARE_STATIC_LOG()
#define DEFINE_STATIC_LOG(ClassName)
#endif


extern void suck(const char* fmt, ...);
#define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1):__FILE__)
#define makePrefix(fmt) std::string(__FILENAME__).append("::").append(__FUNCTION__).append("() - ").append(fmt).c_str()
//#define makePrefix(fmt) std::string(fmt).c_str()

#if defined (_LOG4CPP)
#define LogDebug(fmt, ...) log1.debug(makePrefix(fmt), ##__VA_ARGS__)
#define LogInfo(fmt, ...) log.info(makePrefix(fmt), ##__VA_ARGS__)
#define LogNotice(fmt, ...) log.notice(makePrefix(fmt), ##__VA_ARGS__)
#define LogError(fmt, ...) log.error(makePrefix(fmt), ##__VA_ARGS__)
#else
#define LogDebug(fmt, ...)	suck(makePrefix(fmt), ##__VA_ARGS__)
#define LogInfo(fmt, ...)	suck(makePrefix(fmt), ##__VA_ARGS__)
#define LogNotice(fmt, ...)	suck(makePrefix(fmt), ##__VA_ARGS__)
#define LogError(fmt, ...)	suck(makePrefix(fmt), ##__VA_ARGS__)
#endif

#define FunEntry(...) LogDebug(" Entry... " ##__VA_ARGS__)
#define FunExit(...) LogDebug(" Exit... " ##__VA_ARGS__)

#endif /*LOG_UTILS_H*/
