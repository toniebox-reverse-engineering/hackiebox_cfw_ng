#ifndef __LOGGER_H__
#define __LOGGER_H__
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include "globalDefines.h"

typedef struct Logger_Event {
  va_list ap;
  const char *fmt;
  const char *file;
  const char *function;
  void *udata;
  int line;
  uint8_t level;
  bool newLine;
} Logger_Event;

void Logger_init(void);

#if DEBUG_LOG_LEVEL_TRACE >= DEBUG_LOG_LEVEL
#define Logger_trace(...)      Logger_log_marco(DEBUG_LOG_LEVEL_TRACE, true,  __VA_ARGS__)
#define Logger_trace_nonl(...) Logger_log_marco(DEBUG_LOG_LEVEL_TRACE, false, __VA_ARGS__)
#else
#define Logger_trace(...)
#define Logger_trace_nonl(...)
#endif     

#if DEBUG_LOG_LEVEL_DEBUG >= DEBUG_LOG_LEVEL 
#define Logger_debug(...)      Logger_log_marco(DEBUG_LOG_LEVEL_DEBUG, true,  __VA_ARGS__)
#define Logger_debug_nonl(...) Logger_log_marco(DEBUG_LOG_LEVEL_DEBUG, false, __VA_ARGS__)
#else
#define Logger_debug(...)
#define Logger_debug_nonl(...)
#endif     

#if DEBUG_LOG_LEVEL_INFO >= DEBUG_LOG_LEVEL
#define Logger_info(...)       Logger_log_marco(DEBUG_LOG_LEVEL_INFO, true,   __VA_ARGS__)
#define Logger_info_nonl(...)  Logger_log_marco(DEBUG_LOG_LEVEL_INFO, false,  __VA_ARGS__)
#else
#define Logger_info(...)
#define Logger_info_nonl(...)
#endif     

#if DEBUG_LOG_LEVEL_WARN >= DEBUG_LOG_LEVEL
#define Logger_warn(...)       Logger_log_marco(DEBUG_LOG_LEVEL_WARN, true,   __VA_ARGS__)
#define Logger_warn_nonl(...)  Logger_log_marco(DEBUG_LOG_LEVEL_WARN, false,  __VA_ARGS__)
#else
#define Logger_warn(...)
#define Logger_warn_nonl(...)
#endif     

#if DEBUG_LOG_LEVEL_ERROR >= DEBUG_LOG_LEVEL
#define Logger_error(...)      Logger_log_marco(DEBUG_LOG_LEVEL_ERROR, true,  __VA_ARGS__)
#define Logger_error_nonl(...) Logger_log_marco(DEBUG_LOG_LEVEL_ERROR, false, __VA_ARGS__)
#else
#define Logger_error(...)
#define Logger_error_nonl(...)
#endif     

#if DEBUG_LOG_LEVEL_FATAL >= DEBUG_LOG_LEVEL
#define Logger_fatal(...)      Logger_log_marco(DEBUG_LOG_LEVEL_FATAL, true,  __VA_ARGS__)
#define Logger_fatal_nonl(...) Logger_log_marco(DEBUG_LOG_LEVEL_FATAL, false, __VA_ARGS__)
#else
#define Logger_fatal(...)
#define Logger_fatal_nonl(...)
#endif

#define Logger_log_marco(level, newline, ...) Logger_log(level, newline, __FILE__, __func__, __LINE__, __VA_ARGS__)
void Logger_log(uint8_t level, bool newLine, const char *file, const char *function, int line, const char *fmt, ...);

void Logger_setLevel(uint8_t level);
bool Logger_needed(uint8_t level);
void Logger_newLine(void);

#ifdef __cplusplus
}
#endif
#endif