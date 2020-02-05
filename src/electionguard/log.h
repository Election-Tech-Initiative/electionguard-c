#ifndef __LOG_H__
#define __LOG_H__

#include <stdarg.h>
#include <stdio.h>

static void log_stdout(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

static void log_stderr(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
}

#ifdef TRACE
#define TRACE_PRINT(x) do { log_stdout x; } while (0)
#else
#define TRACE_PRINT(x) do {} while (0)
#endif

#ifdef DEBUG
#define DEBUG_PRINT(x) do { log_stdout x; } while (0)
#else
#define DEBUG_PRINT(x) do {} while (0)
#endif

#define INFO_PRINT(x) do { log_stdout x; } while (0)
#define ERROR_PRINT(x) do { log_stderr x; } while (0)

#endif /* __LOG_H__ */