#ifndef __LOGGER_H__
#define __LOGGER_H__
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

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

#define Logger_trace(...) Logger_log(DEBUG_LOG_LEVEL_TRACE, true, __FILE__, __func__, __LINE__, __VA_ARGS__)
#define Logger_debug(...) Logger_log(DEBUG_LOG_LEVEL_DEBUG, true, __FILE__, __func__, __LINE__, __VA_ARGS__)
#define Logger_info(...)  Logger_log(DEBUG_LOG_LEVEL_INFO, true,  __FILE__, __func__, __LINE__, __VA_ARGS__)
#define Logger_warn(...)  Logger_log(DEBUG_LOG_LEVEL_WARN, true,  __FILE__, __func__, __LINE__, __VA_ARGS__)
#define Logger_error(...) Logger_log(DEBUG_LOG_LEVEL_ERROR, true, __FILE__, __func__, __LINE__, __VA_ARGS__)
#define Logger_fatal(...) Logger_log(DEBUG_LOG_LEVEL_FATAL, true, __FILE__, __func__, __LINE__, __VA_ARGS__)

#define Logger_trace_nonl(...) Logger_log(DEBUG_LOG_LEVEL_TRACE, false, __FILE__, __func__, __LINE__, __VA_ARGS__)
#define Logger_debug_nonl(...) Logger_log(DEBUG_LOG_LEVEL_DEBUG, false, __FILE__, __func__, __LINE__, __VA_ARGS__)
#define Logger_info_nonl(...)  Logger_log(DEBUG_LOG_LEVEL_INFO, false,  __FILE__, __func__, __LINE__, __VA_ARGS__)
#define Logger_warn_nonl(...)  Logger_log(DEBUG_LOG_LEVEL_WARN, false,  __FILE__, __func__, __LINE__, __VA_ARGS__)
#define Logger_error_nonl(...) Logger_log(DEBUG_LOG_LEVEL_ERROR, false, __FILE__, __func__, __LINE__, __VA_ARGS__)
#define Logger_fatal_nonl(...) Logger_log(DEBUG_LOG_LEVEL_FATAL, false, __FILE__, __func__, __LINE__, __VA_ARGS__)

void Logger_log(uint8_t level, bool newLine, const char *file, const char *function, int line, const char *fmt, ...);



#ifdef __cplusplus
}
#endif
#endif