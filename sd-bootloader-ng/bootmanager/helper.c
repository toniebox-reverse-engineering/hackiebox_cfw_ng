#include "helper.h"

void UtilsDelayUs(unsigned long delayUs) {
    UtilsDelay(UTILS_DELAY_US_TO_COUNT(delayUs));
}
void UtilsDelayMs(unsigned long delayMs) {
    UtilsDelayUs(1000*delayMs);
}

void btox(char *hexstr, const char *binarr, int hexstrlen) {
    const char characters[]= "0123456789abcdef";
    while (--hexstrlen >= 0) {
      hexstr[hexstrlen] = characters[(binarr[hexstrlen>>1] >> ((1 - (hexstrlen&1)) << 2)) & 0xF];
    }
}