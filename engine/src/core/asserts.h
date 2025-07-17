#pragma once

#include "defines.h"

//Disable assertions by commenting THIISSSSS out
#define ASSERTIONS_ENABLED

#ifdef ASSERTIONS_ENABLED
#if _MSC_VER
#include <intrin.h>
#define debugBreak() __debugbreak();
#else
#define debugBreak() __builtin_trap();
#endif

void report_assertion_failure(const char* expression, const char* message, const char* file, i32 line);

#define ASSERT(expr)                                           \
{                                                               \
   if(expr){                                                    \
    } else{                                                     \
    report_assertion_failure(#expr,"",__FILE__,__LINE__);       \
    debugBreak();                                               \
   }                                                            \
}                                                               \
 
#define ASSERT_MSG(expr, message)                              \
{                                                               \
   if(expr){                                                    \
    } else{                                                     \
    report_assertion_failure(#expr,message, __FILE__, __LINE__);\
    debugBreak();                                               \
   }                                                            \
}                                                               

#ifdef _DEBUG
#define ASSERT_DEBUG(expr)                                     \
{                                                               \
   if(expr){                                                    \
    } else{                                                     \
    report_assertion_failure(#expr,"",__FILE__,__LINE__);       \
    debugBreak();                                               \
   }                                                            \
}                                                               
#else
#define ASSERT_DEBUG(expr)
#endif

#else
#define ASSERT(expr)
#define ASSERT_MSG(expr, message)
#define ASSERT_DEBUG(expr)
#endif