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
volatile static bool watchdog_init = false;

static void watchdog_clear(void) {
  if (watchdog_init)
    MAP_WatchdogIntClear(WDT_BASE);
}
void watchdog_feed(void) {
  watchdog_feed_state = WATCHDOG_TIMEOUT_S;
  watchdog_clear();
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
static void watchdog_handler(void) {
  if (watchdog_feed_state > 0) {
    watchdog_clear();
    watchdog_eat();
  }
}
static bool initWatchdog(unsigned long ulLoadVal, void (*pfnHandler)(void)) {
  #ifndef DISABLE_WATCHDOG
  if (watchdog_init) {
    watchdog_stop();
  }
  watchdog_feed();

  MAP_PRCMPeripheralClkEnable(PRCM_WDT, PRCM_RUN_MODE_CLK);

  watchdog_init = MAP_WatchdogRunning(WDT_BASE);

  MAP_WatchdogUnlock(WDT_BASE);
  MAP_IntPrioritySet(INT_WDT, INT_PRIORITY_LVL_1);
  MAP_WatchdogStallEnable(WDT_BASE); //Allow Debugging
  MAP_WatchdogIntRegister(WDT_BASE, pfnHandler);
  MAP_WatchdogReloadSet(WDT_BASE, ulLoadVal);
  if (!watchdog_init)
    MAP_WatchdogEnable(WDT_BASE);

  watchdog_init = MAP_WatchdogRunning(WDT_BASE);
  #endif
  return watchdog_init;
}

bool watchdog_start(void) {
  return initWatchdog(MILLISECONDS_TO_TICKS(1000*WATCHDOG_CHECK_S), watchdog_handler);
}
bool watchdog_start_slow(void) {
  return initWatchdog(MILLISECONDS_TO_TICKS(1000*WATCHDOG_TIMEOUT_SLOW_S), watchdog_clear);
}
bool watchdog_stop(void) {
  watchdog_init = false;

  MAP_WatchdogUnlock(WDT_BASE);
  MAP_WatchdogReloadSet(WDT_BASE, 0xFFFFFFFF); //set timer to high value
  MAP_WatchdogIntClear(WDT_BASE);
  MAP_WatchdogIntUnregister(WDT_BASE);

  //watchdog_init = MAP_WatchdogRunning(WDT_BASE);

  //MAP_PRCMPeripheralClkDisable(PRCM_WDT, PRCM_RUN_MODE_CLK);
}
