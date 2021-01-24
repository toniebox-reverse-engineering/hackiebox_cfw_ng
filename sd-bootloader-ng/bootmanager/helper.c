#include "helper.h"
#include <stdint.h>

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

uint8_t xtob_split(char a, char b) {
    a = (a <= '9') ? a - '0' : (a & 0x7) + 9;
    b = (b <= '9') ? b - '0' : (b & 0x7) + 9;

    return (a << 4) + b;
}
uint8_t xtob(char* hexByte) {
    return xtob_split(hexByte[0], hexByte[1]);
}