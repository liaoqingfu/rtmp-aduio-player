#include "log_util.hpp"

#if defined (_LOG4CPP)
#include <log4cpp/PropertyConfigurator.hh>

// appenders
#include <log4cpp/Appender.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/RollingFileAppender.hh>
#include <log4cpp/AbortAppender.hh>

#include <log4cpp/RemoteSyslogAppender.hh>

// layouts
#include <log4cpp/Layout.hh>
#include <log4cpp/BasicLayout.hh>
#include <log4cpp/SimpleLayout.hh>
#include <log4cpp/PatternLayout.hh>

#include <log4cpp/Priority.hh>

#else
#include <stdio.h>
#include <stdarg.h>
#include <string>
#include <pthread.h>
static pthread_mutex_t printMutex;
#endif /* _LOG4CPP */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

static FILE  *fFile = NULL;

long getfilesize(FILE *pFile)
{
    // check FILE*.
    if( pFile == NULL)
    {
            return -1;
    }

    // get current file pointer offset.
    long cur_pos = ftell(pFile);
    if(cur_pos == -1)
    {
            return -1;
    }

    // fseek to the position of file end.
    if(fseek(pFile, 0L, SEEK_END) == -1)
    {
            return -1;
    }

    // get file size.
    long size = ftell(pFile);

    // fseek back to previous pos.
    if(fseek(pFile, cur_pos, SEEK_SET) == -1)
    {
            return -1;
    }

    // deal returns.
    return size;
}

void writeLog4cpp(char *str, int size)
{
    if(fFile)
    {
        if(getfilesize(fFile) > 1024*512)//only about 2m memory is free while app is running, for better performance, log file size needs to be stricted to be 512K.(old setting is 5M)
        {
            system("cat  /dev/null  > /tmp/tbapp.log");
            fseek(fFile, 0, SEEK_SET);
        }
        fwrite(str, 1, size, fFile);
        fflush(fFile);
    }
}

void initLog4cpp()
{
    fFile = fopen("/tmp/tbapp.log", "a+");
    if(fFile == NULL)
    {
        return;
    }
}

void initializeLog4cpp(const std::string& logfile)
{
#if defined (_LOG4CPP)
    try
    {
        log4cpp::PropertyConfigurator::configure(logfile);
    }
    catch (log4cpp::ConfigureFailure& f)
    {
        std::cerr << "ConfigureFailure" << f.what() << std::endl;

        log4cpp::Appender* appender = new log4cpp::OstreamAppender("console", &std::cout);
        log4cpp::PatternLayout* patternLayout = new log4cpp::PatternLayout();
        patternLayout->setConversionPattern("%d [%t] %p - %m%n");

        appender->setLayout(patternLayout);

        log4cpp::Category& root = log4cpp::Category::getRoot();
        root.addAppender(appender);
        root.setPriority(log4cpp::Priority::DEBUG);
    }
#else
    initLog4cpp();
    pthread_mutex_init(&printMutex, NULL);
#endif
}

void releaseLog4cpp()
{
#if defined (_LOG4CPP)
    log4cpp::Category::shutdown();
#else
    pthread_mutex_destroy(&printMutex);
#endif
}

#if defined(_LOG4CPP)
log4cpp::Category& getLogCategory(const char* categoryName)
{
    std::string name(categoryName);
    return log4cpp::Category::getInstance(name);
}
#endif

void suck (const char* fmt, ...)
{
#ifndef _LOG4CPP
    char buf[1024] = {0};
    char timeBuf[512] = {0};
    timeval now;
    gettimeofday(&now, NULL);
    const time_t timeseconds = now.tv_sec;
    strftime(timeBuf, 128, "%m-%d %H:%M:%S ", localtime(&timeseconds));
    long int timemillseconds = now.tv_usec / 1000;
    sprintf(timeBuf + strlen(timeBuf), "%ld ", timemillseconds);

    std::string fmt_str(timeBuf);
    fmt_str += fmt;
    fmt_str += " \n";

    pthread_mutex_lock(&printMutex);
    va_list arglist;
    va_start(arglist, fmt);
    //vfprintf(stderr, fmt_str.c_str(), arglist);
    vsnprintf(buf, sizeof(buf), fmt_str.c_str(), arglist);
    fprintf(stderr, buf);
    writeLog4cpp(buf, strlen(buf));
    va_end(arglist);
    pthread_mutex_unlock(&printMutex);
#endif
}
