#ifndef __CONFIG_H__
#define __CONFIG_H__
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

#include "helper.h"
#include "globalDefines.h"

typedef struct sGeneralSettings
{
  uint8_t activeImage;
  bool waitForPress;

} sGeneralSettings;
typedef struct sImageInfo
{
  bool fileExists;
  bool checkHash;
  bool hashFile;
  bool watchdog;
  bool ofwFix;
} sImageInfo;

extern sGeneralSettings Config_generalSettings;
extern sImageInfo Config_imageInfos[IMG_MAX_COUNT];

void Config_InitImageInfos(void);
void Config_ReadJsonCfg(void);


#ifdef __cplusplus
}
#endif
#endif