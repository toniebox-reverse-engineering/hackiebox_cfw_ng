#include "patch.h"

sSearchAndReplacePatch Patch_searchAndReplace;

static jsmn_stream_parser parser;
static char jsonGroupName[17];
static char jsonValueName[17];
static uint8_t cursor = 0;

static void jsmn_start_arr(void *user_arg) {
    cursor = 0;
}
static void jsmn_end_arr(void *user_arg) {
  if (parser.stack_height != 6)
    return;

  if (strcmp("searchAndReplace", jsonGroupName) == 0) {
      if (strcmp("search", jsonValueName) == 0 || strcmp("replace", jsonValueName) == 0) {
          if (Patch_searchAndReplace.length == 0) {
            Patch_searchAndReplace.length = cursor;
          } else {
            Patch_searchAndReplace.length = min(cursor, Patch_searchAndReplace.length);
          }
      } 
  }
}
static void jsmn_start_obj(void *user_arg) {
    uint8_t test = 1;
    //printf("Object started\n");
}
static void jsmn_end_obj(void *user_arg) {
  if (parser.stack_height != 4)
    return;
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
    case 4:
      len = min(key_len, COUNT_OF(jsonValueName)-1);
      strncpy(jsonValueName, key, len);
      jsonValueName[min(key_len, COUNT_OF(jsonValueName))] = '\0';
      break;
    case 6:
        if (strcmp("searchAndReplace", jsonGroupName) == 0 && key_len == 2) {
            if (cursor >= PATCH_MAX_BYTES)
                return;
                
            char* values;
            char* mask;
            if (strcmp("search", jsonValueName) == 0) {
                values = Patch_searchAndReplace.search;
                mask = Patch_searchAndReplace.searchMask;
            } else if (strcmp("replace", jsonValueName) == 0) {
                values = Patch_searchAndReplace.replace;
                mask = Patch_searchAndReplace.replaceMask;
            } 
            if (strcmp("??", key) == 0) {
                mask[cursor] = 0x00;
            } else {
                values[cursor] = (char)xtob((char*)key);
                mask[cursor] = 0xFF;
            }
            cursor++;
        }  
        break;
    }
}
static void jsmn_str(const char *value, size_t len, void *user_arg) {
    if (parser.stack_height != 5)
      return;  
}
static void jsmn_primitive(const char *value, size_t len, void *user_arg) {
    if (parser.stack_height != 4)
      return;

    if (strcmp("general", jsonGroupName) == 0)
    {
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

void Patch_Read(char* name) {
  //TODO ERRORS

  FIL ffile;
  uint8_t ffs_result;

  char filepath[COUNT_OF(PATCH_SD_BASE_PATH)+32+5];
  strcpy(filepath, PATCH_SD_BASE_PATH);
  char* filename = filepath + COUNT_OF(PATCH_SD_BASE_PATH)-1;
  strcpy(filename, name);
  char* fileext = filename + strlen(filename);
  strcpy(fileext, ".json");

  Patch_searchAndReplace.length = 0;

  
  ffs_result = f_open(&ffile, filepath, FA_READ);
  if (ffs_result == FR_OK) {
    uint32_t filesize = f_size(&ffile);
    uint32_t bytesRead = 0;
    uint32_t allBytesRead = 0;
    
    jsmn_stream_init(&parser, &cbs, NULL);
    char buffer[512];
    while (allBytesRead<filesize)
    {
      ffs_result = f_read(&ffile, buffer, COUNT_OF(buffer), &bytesRead);
      if (ffs_result != FR_OK)
        break;

      for (uint16_t i = 0; i < bytesRead; i++)
      {
        jsmn_stream_parse(&parser, buffer[i]);
      }
      allBytesRead += bytesRead;
    }
    f_close(&ffile);
  }
}