#include "config.h"

sGeneralSettings Config_generalSettings = {0, false};
sImageInfo Config_imageInfos[IMG_MAX_COUNT];

static jsmn_stream_parser parser;
static char jsonGroupName[17];
static char jsonValueName[17];

static uint8_t GetImageNumber(const char* imageId)
{
  uint8_t factor = 0;
  uint8_t id = 0;
  if (strncmp(imageId, IMG_OFW_NAME, 3) == 0)
  {
    factor = 0;
  } else if (strncmp(imageId, IMG_CFW_NAME, 3) == 0)
  {
    factor = 1;
  } else if (strncmp(imageId, IMG_ADD_NAME, 3) == 0)
  {
    factor = 2;
  }
  id = (char)(imageId[3] - 0x31);
  if (id>=0 && id<3) {
    return id + 3*factor;
  }
  return 0;
}

static void jsmn_start_arr(void *user_arg) {
    uint8_t test = 1;
    /* An example of using the user arg / context pointer in a callback */
    //printf("Array started. Parser id = %d\n", *parser_id);
}
static void jsmn_end_arr(void *user_arg) {
    uint8_t test = 1;
    //printf("Array ended\n");
}
static void jsmn_start_obj(void *user_arg) {
    uint8_t test = 1;
    //printf("Object started\n");
}
static void jsmn_end_obj(void *user_arg) {
    uint8_t test = 1;
    //printf("Object ended\n");
}
static void jsmn_obj_key(const char *key, size_t key_len, void *user_arg) {
    uint8_t len;
    switch (parser.stack_height)
    {
    case 1:
      len = min(key_len, COUNT_OF(jsonGroupName)-1);
      strncpy(jsonGroupName, key, len);
      jsonGroupName[min(key_len, COUNT_OF(jsonGroupName))] = '\0';
      break;
    case 3:
      len = min(key_len, COUNT_OF(jsonValueName)-1);
      strncpy(jsonValueName, key, len);
      jsonValueName[min(key_len, COUNT_OF(jsonValueName))] = '\0';
      break;
    }
}
static void jsmn_str(const char *value, size_t len, void *user_arg) {
    if (parser.stack_height != 4)
      return;

    if (strcmp("general", jsonGroupName) == 0)
    {
      if (strcmp("activeImg", jsonValueName) == 0)
      {
        Config_generalSettings.activeImage = GetImageNumber(value);
      }
    }
    else if (strncmp(jsonGroupName, "ofw", 3) == 0
      || strncmp(jsonGroupName, "cfw", 3)
      || strncmp(jsonGroupName, "add", 3))
    {
      uint8_t imageNumber = GetImageNumber(jsonGroupName);

    }
    
}
static void jsmn_primitive(const char *value, size_t len, void *user_arg) {
    if (parser.stack_height != 4)
      return;

    if (strcmp("general", jsonGroupName) == 0)
    {
      if (strcmp("waitForPress", jsonValueName) == 0)
      {
        Config_generalSettings.waitForPress = (value[0] == 't');
      }
    }
    else if (strncmp(jsonGroupName, "ofw", 3) == 0
      || strncmp(jsonGroupName, "cfw", 3)
      || strncmp(jsonGroupName, "add", 3))
    {
      uint8_t imageNumber = GetImageNumber(jsonGroupName);
      if (strcmp("checkHash", jsonValueName) == 0) 
      {
        Config_imageInfos[imageNumber].checkHash = (value[0] == 't');
      }
      else if (strcmp("hashFile", jsonValueName) == 0) 
      {
        Config_imageInfos[imageNumber].hashFile = (value[0] == 't');
      }
      else if (strcmp("watchdog", jsonValueName) == 0) 
      {
        Config_imageInfos[imageNumber].watchdog = (value[0] == 't');
      }
      else if (strcmp("ofwFix", jsonValueName) == 0) 
      {
        Config_imageInfos[imageNumber].ofwFix = (value[0] == 't');
      }
    }
}

static jsmn_stream_callbacks_t cbs = {
    jsmn_start_arr,
    jsmn_end_arr,
    jsmn_start_obj,
    jsmn_end_obj,
    jsmn_obj_key,
    jsmn_str,
    jsmn_primitive
};


void Config_initImageInfos(void) {
  for (uint8_t i = 0; i < IMG_MAX_COUNT; i++)
  {
    Config_imageInfos[i].fileExists = false;
    Config_imageInfos[i].checkHash = true;
    Config_imageInfos[i].hashFile = false;
    Config_imageInfos[i].watchdog = false;
    Config_imageInfos[i].ofwFix = false;
  }
}

void Config_readJsonCfg(void) {
  //TODO ERRORS

  FIL ffile;
  uint8_t ffs_result;

  ffs_result = f_open(&ffile, CFG_SD_PATH, FA_READ);
  if (ffs_result == FR_OK) {
    uint32_t filesize = f_size(&ffile);
    uint32_t bytesRead = 0;
    uint32_t allBytesRead = 0;
    
    jsmn_stream_init(&parser, &cbs, NULL);
    char buffer[128];
    while (allBytesRead<filesize)
    {
      ffs_result = f_read(&ffile, buffer, COUNT_OF(buffer), &bytesRead);
      if (ffs_result != FR_OK)
        break;

      for (uint32_t i = 0; i < bytesRead; i++)
      {
        jsmn_stream_parse(&parser, buffer[i]);
      }
      allBytesRead += bytesRead;
    }
    f_close(&ffile);
  }
}