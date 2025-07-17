#include "logging.h"

void logger(const char* level, const char* level_format, const char* format, ...){
  
    va_list args;
    va_start(args, format);
    char msg[4096];
    strcpy(msg, level);
    strcat(msg, level_format);
    strcat(msg, format);
    strcat(msg, RESET);
    strcat(msg, "\n");
    vprintf(msg, args);
    
    fflush(stdout);
}

void report_assertion_failure(const char* expression, const char* message, const char* file, i32 line){
    ERROR("Assertion Failure: %s, message: %s, in file: %s, on line: %i", expression, message, file, line);
}