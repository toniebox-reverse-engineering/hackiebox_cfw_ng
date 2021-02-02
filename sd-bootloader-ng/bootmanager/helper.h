#ifndef __HELPER_H__
#define __HELPER_H__
#ifdef __cplusplus
extern "C"
{
#endif
#include "globalDefines.h"
#include "utils.h"
#include <stdbool.h>
#include <stdint.h>

#define max(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b;       \
})

#define min(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b;       \
})

#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

void UtilsDelayUs(unsigned long delayUs);
void UtilsDelayMs(unsigned long delayMs);
void UtilsDelayMsWD(unsigned long delayMs);

void btox(char *hexstr, const char *binarr, int hexstrlen);
uint8_t xtob(char* hexByte);

bool SdFileExists(char* filename);

#ifdef __cplusplus
}
#endif
#endif