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
  uint32_t searchMemPos;

  char replace[PATCH_MAX_BYTES];
  char replaceMask[PATCH_MAX_BYTES];

} sSearchAndReplacePatch;

typedef struct sSearchPosition
{
  uint8_t length;

  int32_t offset;
  char search[PATCH_MAX_BYTES];
  char searchMask[PATCH_MAX_BYTES];
  bool deasmAddress;

} sSearchPosition;

typedef struct sAsmReplace
{
  char instruction[4];
  char parameter[3];
  uint8_t length;

} sAsmReplace;

void Patch_Apply(char* imageBytes, char* patchName, uint32_t imageLength);

#ifdef __cplusplus
}
#endif
#endif