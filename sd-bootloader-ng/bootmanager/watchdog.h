#ifndef __WATCHDOG_H__
#define __WATCHDOG_H__
#ifdef __cplusplus
extern "C"
{
#endif
#include <stdint.h>
#include <stdbool.h>

#include "globalDefines.h"

void watchdog_feed(void);
bool watchdog_start(void);
bool watchdog_start_slow(void);
bool watchdog_stop(void);

#ifdef __cplusplus
}
#endif
#endif