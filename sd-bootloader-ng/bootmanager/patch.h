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

typedef struct sSearchAndReplacePatch
{
  uint8_t length;

  char search[PATCH_MAX_BYTES];
  char searchMask[PATCH_MAX_BYTES];

  char replace[PATCH_MAX_BYTES];
  char replaceMask[PATCH_MAX_BYTES];

} sSearchAndReplacePatch;

typedef struct sSearchPosition
{
  uint8_t length;

  int32_t offset;
  char search[PATCH_MAX_BYTES];
  char searchMask[PATCH_MAX_BYTES];

} sSearchPosition;

void Patch_Apply(char* imageBytes, char* patchName, uint32_t imageLength);

#ifdef __cplusplus
}
#endif
#endif