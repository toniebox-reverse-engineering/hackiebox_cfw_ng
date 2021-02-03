#include "watchdog.h"

#include "hw_types.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "prcm.h"
#include "wdt.h"
#include "rom.h"
#include "rom_map.h"
#include "interrupt.h"

volatile static uint8_t watchdog_feed_state;
#ifndef SIMPLE_WATCHDOG
volatile static bool watchdog_init = false;
#endif

void watchdog_feed(void) {
  watchdog_feed_state = WATCHDOG_TIMEOUT_S;
}
static void watchdog_unfeed(void) {
  watchdog_feed_state = 0;
}
void watchdog_eat(void) {
  if (watchdog_feed_state > WATCHDOG_CHECK_S) {
    watchdog_feed_state -= WATCHDOG_CHECK_S;
  } else {
    watchdog_feed_state = 0;
  }
}
static void watchdog_clear(void) {
  #ifndef DISABLE_WATCHDOG
  MAP_WatchdogIntClear(WDT_BASE);
  #endif
}
static void watchdog_handler(void) {
  if (watchdog_feed_state > 0) {
    watchdog_clear();
    watchdog_eat();
  }
}
static bool initWatchdog(unsigned long ulLoadVal, void (*pfnHandler)(void)) {
  #ifndef DISABLE_WATCHDOG
  watchdog_feed();

  MAP_WatchdogUnlock(WDT_BASE);
  MAP_IntPrioritySet(INT_WDT, INT_PRIORITY_LVL_1);
  MAP_WatchdogStallEnable(WDT_BASE); //Allow Debugging
  MAP_WatchdogIntUnregister(WDT_BASE);
  MAP_WatchdogIntRegister(WDT_BASE, pfnHandler);
  MAP_WatchdogReloadSet(WDT_BASE, ulLoadVal);
  MAP_WatchdogEnable(WDT_BASE);

  return MAP_WatchdogRunning(WDT_BASE);
  #endif
}

bool watchdog_start(void) {
  #ifndef SIMPLE_WATCHDOG
  return initWatchdog(MILLISECONDS_TO_TICKS(1000*WATCHDOG_CHECK_S), watchdog_handler);
  #else
  return watchdog_stop();
  #endif
}

bool watchdog_start_slow(void) {
  #ifndef SIMPLE_WATCHDOG
  return initWatchdog(MILLISECONDS_TO_TICKS(1000*WATCHDOG_TIMEOUT_SLOW_S), watchdog_clear);
  #else
  return watchdog_stop();
  #endif
}
bool watchdog_stop(void) {
  #ifndef NOSTOP_WATCHDOG
  #ifndef SIMPLE_WATCHDOG
  if (watchdog_init)
    return true;
  watchdog_init = true;
  #endif
  return initWatchdog(0x00FFFFFF, watchdog_clear);
  #endif
}
