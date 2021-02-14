#ifndef __CLOG_H__
#define __CLOG_H__
#include <stdio.h>

#define LOG_DEBUG(...) printf(__VA_ARGS__);
#define LOG_INFO(...) printf(__VA_ARGS__);
#define LOG_WARNING(...) printf(__VA_ARGS__);
#define LOG_ERROR(...) printf(__VA_ARGS__);

#endif
