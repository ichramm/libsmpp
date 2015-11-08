/*!
 * \file logger.cpp
 * \author ichramm
 *
 * Created on May 5, 2012, 05:04 PM
 */
#include "stdafx.h"
#include "logger.h"

#include "../libsmpp.h"

#include <boost/make_shared.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <cstdio>
#include <cstdarg>
#include <time.h>

#if defined(__linux__)
 #if !defined(__NR_gettid)
  #include <asm/unistd.h>
 #endif
 #if defined(__NR_gettid)
  #define GETTID() syscall(__NR_gettid)
 #else
  #define GETTID() getpid()
 #endif
 #define GETPID() getpid()
 // GCC is assumed here
 #define function_with_attribute(attr,name) static void __attribute__((attr)) name()
#elif defined(_WIN32)
 #define GETTID() GetCurrentThreadId()
 #define GETPID() GetCurrentProcessId()
 #define function_with_attribute(attr,name) void name()
#endif

using namespace std;
using namespace boost;

namespace
{
	class LoggerImpl
	{
	private:
		Logger *m_logger;
		mutex   m_mutex;

	public:
		LoggerImpl()
		 : m_logger(NULL)
		{}

		void SetLogger(Logger *logger)
		{
			lock_guard<mutex> lock(m_mutex);
			m_logger = logger;
		}

		void Log(LogLevel level, const char *format, va_list args)
		{
			size_t buffer_size = m_logger ? 4096 : 8092;

			string message(buffer_size, '\0');

			if ( vsnprintf(&message[0], buffer_size, format, args) == -1) {
				message[buffer_size-1] = '\0';
			}

			LogFunction logFunction = NULL;

			lock_guard<mutex> lock(m_mutex);
			switch (level)
			{
			case LevelDebug:
				logFunction = m_logger ? m_logger->debug : NULL;
				break;
			case LevelInfo:
				logFunction = m_logger ? m_logger->notice : NULL;
				break;
			case LevelWarn:
				logFunction = m_logger ? m_logger->warning : NULL;
				break;
			case LevelError:
				logFunction = m_logger ? m_logger->error : NULL;
				break;
			case LevelFatal:
				logFunction = m_logger ? m_logger->fatal : NULL;
				break;
			case LevelProfile:
				logFunction = m_logger ? m_logger->profile : NULL;
				break;
			}

			if ( logFunction ) {
				logFunction(message.c_str());
			} else {
#if defined (__linux__)
				timespec tp;
				clock_gettime(CLOCK_MONOTONIC, &tp);
				fprintf(stderr, "time[%lu.%04lu] th[%ld] - %s\n", tp.tv_sec, tp.tv_nsec/(1000*100),
					   (long int)GETTID(), message.c_str());
#else
				fprintf(stderr, "time[%lu] th[%ld] - %s\n", time(NULL),
					   (long int)GETTID(), message.c_str());
#endif
			}
		}
	};
}

static LoggerImpl *g_logger = NULL;

// msvc: void smpp_logger_load()
// gcc : static void __attribute__((contructor)) smpp_logger_load()
function_with_attribute(constructor, smpp_logger_load)
{
	g_logger = new LoggerImpl();
}

// msvc: void smpp_logger_unload()
// gcc : static void __attribute__((destructor)) smpp_logger_unload()
function_with_attribute(destructor, smpp_logger_unload)
{
	delete g_logger;
	g_logger = NULL;
}

extern "C" void smpp_log(LogLevel level, const char *format, ... )
{
	if(g_logger != NULL)
	{
		va_list args;
		va_start(args, format);
		g_logger->Log(level, format, args);
		va_end(args);
	}
}

extern "C" int smpp_log_check_mask(unsigned int /*mask*/)
{
	return 1;
}

extern "C" SMPP_API int libSMPP_SetLogger(Logger *logger)
{
	if(g_logger != NULL)
	{
		g_logger->SetLogger(logger);
	}

	return 0;
}
