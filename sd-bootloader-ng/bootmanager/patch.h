#ifndef __PATCH_H__
#define __PATCH_H__
#ifdef __cplusplus
extern "C"
{
#endif

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "utils.h"

#include "ff.h"
#include "jsmn_stream.h"

#include "globalDefines.h"
#include "helper.h"

void Patch_read(char* name);

#ifdef __cplusplus
}
#endif
#endif