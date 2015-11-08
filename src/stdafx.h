// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
#ifndef stdafx_h__
#define stdafx_h__
#if _MSC_VER > 1000
  #pragma once
#endif

#if defined(_WIN32) && !defined(NOMINMAX)
 #define NOMINMAX
#endif

#ifdef __GNUC__
# define GCC_VERSION   ( __GNUC__*10 + __GNUC_MINOR__ )
#else
# define GCC_VERSION 0
#endif

#ifdef _WIN32
  #include "targetver.h"
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>

  #if !defined(pthread_self)
    #define pthread_self GetCurrentThreadId
  #endif
#endif // _WIN32

#if GCC_VERSION >= 47
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"
#endif

// warning: boost::error_info has virtual methods but no virtual destructor
// some boost headers have the GCC system_header pragma so the warning is seen when
// including some non-system header like shared_ptr.hpp or enable_shared_from_this.hpp

#include <boost/shared_ptr.hpp>

#if GCC_VERSION >= 47
  #pragma GCC diagnostic pop
#endif


#define SMPP_TRACE() do { \
	smpp_log_profile("trace"); \
} while(false)



#endif // stdafx_h__
