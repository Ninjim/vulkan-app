#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "defines.h"
#include "asserts.h"

#ifndef RESET
#define RESET          "\x1b[0m"
#endif

#ifndef ERROR_CLR
#define ERROR_CLR          "\x1b[31m"
#endif

#ifndef SUCCESS_CLR
#define SUCCESS_CLR      "\x1b[32m"
#endif

#ifndef WARNING_CLR
#define WARNING_CLR       "\x1b[33m"
#endif

#ifndef TRACE_CLR
#define TRACE_CLR         "\x1b[36m"
#endif

#ifndef LOG_CLR
#define LOG_CLR        "\x1b[37m"
#endif

void logger(const char* level, const char* level_format, const char* format, ...);

#ifndef ERROR
#define ERROR(...) logger(ERROR_CLR, "[ERROR]: ", ##__VA_ARGS__);
#endif

#ifndef TRACE
#define TRACE(...) logger(TRACE_CLR, "[TRACE]: ", ##__VA_ARGS__);
#endif

#ifndef LOG
#define LOG(...) logger(LOG_CLR, "[LOGGING]: ", ##__VA_ARGS__);
#endif

#ifndef WARNING
#define WARNING(...) logger(WARNING_CLR,"[WARNING]: ", ##__VA_ARGS__);
#endif



#ifndef SUCCESS
#define SUCCESS(...) logger(SUCCESS_CLR, "[SUCCESS]: ", ##__VA_ARGS__);
#endif




