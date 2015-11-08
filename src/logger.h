/*!
 * \file logger.h
 * \author ichramm
 *
 * Created on May 5, 2012, 05:04 PM
 */
#ifndef logger_h__
#define logger_h__

#if GCC_VERSION > 0
 #define attribute_format(a,b) __attribute__((format(printf, a, b)))
#else
 #define attribute_format(a,b)
#endif

#define smpp_log_debug(fmt,...) if(smpp_log_check_mask(LevelDebug)) \
	smpp_log(LevelDebug, "%s:%d %s - " fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

#define smpp_log_info(fmt,...) if(smpp_log_check_mask(LevelInfo)) \
	smpp_log(LevelInfo, "%s:%d %s - " fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

#define smpp_log_warning(fmt,...) if(smpp_log_check_mask(LevelWarn)) \
	smpp_log(LevelWarn, "%s:%d %s - " fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

#define smpp_log_error(fmt,...) if(smpp_log_check_mask(LevelError)) \
	smpp_log(LevelError, "%s:%d %s - " fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

#define smpp_log_fatal(fmt,...) if(smpp_log_check_mask(LevelFatal)) \
	smpp_log(LevelFatal, "%s:%d %s - " fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

#define smpp_log_profile(fmt,...) if(smpp_log_check_mask(LevelProfile)) \
	smpp_log(LevelProfile, "%s:%d %s - " fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

enum LogLevel
{
	LevelDebug,
	LevelInfo,
	LevelWarn,
	LevelError,
	LevelFatal,
	LevelProfile
};

#if defined(__cplusplus) || defined(c_plusplus)
extern "C"
{
#endif

/*! \return 0 if log flag \mask is disabled, non zero if flags is enabled */
int smpp_log_check_mask(unsigned int mask);

/*! logs */
void smpp_log(enum LogLevel level, const char *format, ...)
		attribute_format(2,3);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif // logger_h__
