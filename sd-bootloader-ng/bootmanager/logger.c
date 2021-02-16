#include "logger.h"

#include "hw_ints.h"
#include "hw_types.h"
#include "hw_memmap.h"
#include "hw_gprcm.h"
#include "rom.h"
#include "rom_map.h"
#include "prcm.h"
#include "gpio.h"
#include "pin.h"
#include "uart.h"

#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>

#include "globalDefines.h"
#include "printf.h"



#ifndef NO_DEBUG_LOG

void Logger_init(void) {
    MAP_PRCMPeripheralClkEnable(PRCM_UARTA0, PRCM_RUN_MODE_CLK);
    MAP_PinTypeUART(PIN_55, PIN_MODE_3); //UART0_TX
    //MAP_PinTypeUART(PIN_57, PIN_MODE_3); //UART0_RX //Do not use, as muxed with big ear
    MAP_UARTConfigSetExpClk(UARTA0_BASE,MAP_PRCMPeripheralClockGet(PRCM_UARTA0), 
                DEBUG_LOG_BAUD, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                UART_CONFIG_PAR_NONE));
    MAP_UARTEnable(UARTA0_BASE);
}

void Logger_print(const char* message) {
    if(message != NULL) {
        while(*message != '\0') {
            MAP_UARTCharPut(UARTA0_BASE, *message++);
        }
    }
}
void Logger_println(const char* message) {
    Logger_print(message);
    Logger_print("\r\n");
}

static const char *level_strings[] = {
  "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};
#ifdef DEBUG_LOG_COLORED
static const char *level_colors[] = {
  "\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m"
};
#endif
static void uart_callback(Logger_Event *event) {
#ifdef DEBUG_LOG_COLORED
  printf(
    "%s%-5s\x1b[0m \x1b[90m%s:%d:%s():\x1b[0m ",
    level_colors[event->level], level_strings[event->level],
    event->file, event->line, event->function);
#else
  printf(
    "%-5s %s:%i:%s: ",
    level_strings[event->level], event->file, event->line, event->function);
#endif
  vprintf(event->fmt, event->ap);
  if (event->newLine)
    printf("\r\n");
}

void Logger_log(uint8_t level, bool newLine, const char *file, const char *function, int line, const char *fmt, ...) {
    if (level < DEBUG_LOG_LEVEL)
        return;
    
    Logger_Event event = {
        .fmt = fmt,
        .file = file,
        .function = function,
        .line = line,
        .level = level,
        .newLine = newLine,
    };

    va_start(event.ap, fmt);
    uart_callback(&event);
    va_end(event.ap);
}
#else 
void Logger_init(void) { }
void Logger_log(uint8_t level, bool newLine, const char *file, const char *function, int line, const char *fmt, ...)  { }
#endif