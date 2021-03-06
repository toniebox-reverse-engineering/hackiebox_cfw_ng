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
  uint16_t waitTimeoutInS;
  uint16_t minBatteryLevel;
  bool serialLog;
  uint8_t logLevel;
  bool logColor;

} sGeneralSettings;
typedef struct sImageInfo
{
  bool fileExists;
  bool checkHash;
  bool hashFile;
  bool watchdog;
  bool ofwFix;
  bool ofwSimBL;
  bool bootFlashImg;
  char flashImg[CONFIG_FLASH_PATH_MAX];
  char patches[PATCH_MAX_PER_IMAGE][PATCH_MAX_NAME_LENGTH];
} sImageInfo;

extern sGeneralSettings Config_generalSettings;
extern sImageInfo Config_imageInfos[IMG_MAX_COUNT];

void Config_InitImageInfos(void);
void Config_ReadJsonCfg(void);


#ifdef __cplusplus
}
#endif
#endif