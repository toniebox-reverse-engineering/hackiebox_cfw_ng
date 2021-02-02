#include "watchdog.h"

#include "hw_types.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "prcm.h"
#include "wdt.h"
#include "rom.h"
#include "rom_map.h"
#include "interrupt.h"

volatile uint8_t watchdog_feed_state;

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
static void watchdog_handler(void) {
  if (watchdog_feed_state > 0) {
    MAP_WatchdogIntClear(WDT_BASE);
    watchdog_eat();
  }
}
static bool initWatchdog(unsigned long ulLoadVal, void (*pfnHandler)(void)) {
  watchdog_feed();

  MAP_PRCMPeripheralClkEnable(PRCM_WDT, PRCM_RUN_MODE_CLK);
  MAP_WatchdogUnlock(WDT_BASE);
  MAP_IntPrioritySet(INT_WDT, INT_PRIORITY_LVL_1);
  MAP_WatchdogStallEnable(WDT_BASE); //Allow Debugging
  MAP_WatchdogIntRegister(WDT_BASE, pfnHandler);
  MAP_WatchdogReloadSet(WDT_BASE, ulLoadVal);
  MAP_WatchdogEnable(WDT_BASE);

  return MAP_WatchdogRunning(WDT_BASE);
}

bool watchdog_start(void) {
  #ifndef DISABLE_WATCHDOG
  return initWatchdog(MILLISECONDS_TO_TICKS(1000*WATCHDOG_CHECK_S), watchdog_handler);
  #else
  watchdog_stop();
  return true;
  #endif
}

static void watchdog_handler_always(void) {
  MAP_WatchdogIntClear(WDT_BASE);
}
void watchdog_stop(void) {
  initWatchdog(0xFFFFFFFF, watchdog_handler_always);
}
