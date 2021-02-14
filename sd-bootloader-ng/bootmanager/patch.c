#include "patch.h"

static sSearchPosition searchPosition;
static uint32_t positions[PATCH_MAX_POSITIONS];
static uint8_t positionCount;
static bool positionSearchFailed;

static sSearchAndReplacePatch searchAndReplacePatch;
static char* image;
static uint32_t imageLen;

static jsmn_stream_parser parser;
static char jsonGroupName[17];
static char jsonValueName[17];
static uint8_t cursor = 0;

static bool searchInMemory(char* search, char* searchMask, uint8_t length, uint32_t* position) {
  bool found = false;

  uint32_t offset = 0;
  for (offset=0; offset<imageLen-length; offset++) {
    if (searchMask[0] == 0x00)
      continue;
    if (search[0] != image[offset])
      continue;

    uint32_t offset2;
    for (offset2=1; offset2<length; offset2++) {
      if (searchMask[offset2] == 0x00)
        continue;
      if (search[offset2] != image[offset+offset2]) {
        offset2 = 0;
        break;
      }
    }
    if (offset2 == length) {
      *position = offset;
      found = true;
      break;
    }
  }

  return found;
}

static void clearSearchPosition() {
  sSearchPosition* pos = &searchPosition;
  pos->length = 0;
  pos->offset = 0;
  memset(pos->search, 0x00, COUNT_OF(pos->search));
  memset(pos->searchMask, 0x00, COUNT_OF(pos->searchMask));
}
static void doSearchPosition() {
  if (positionSearchFailed)
    return;

  sSearchPosition* pos = &searchPosition;
  if (pos->length == 0 || positionCount >= COUNT_OF(positions)) {
    positionSearchFailed = true;
    clearSearchPosition();
    return;
  }
  
  uint32_t offset = 0;
  if (!searchInMemory(pos->search, pos->searchMask, pos->length, &offset)) {
    positionSearchFailed = true;
    clearSearchPosition();
    return;
  }

  positions[positionCount] =  offset + pos->offset;
  positionCount++;

  clearSearchPosition();
}

static void clearSearchAndReplace() {
  sSearchAndReplacePatch* patch = &searchAndReplacePatch;
  patch->length = 0;
  memset(patch->search, 0x00, COUNT_OF(patch->search));
  memset(patch->searchMask, 0x00, COUNT_OF(patch->searchMask));
  memset(patch->replace, 0x00, COUNT_OF(patch->replace));
  memset(patch->replaceMask, 0x00, COUNT_OF(patch->replaceMask));
}

static void doSearchAndReplace() {
  sSearchAndReplacePatch* patch = &searchAndReplacePatch;
  if (patch->length > 0) {
    uint32_t offset = 0;
    if (searchInMemory(patch->search, patch->searchMask, patch->length, &offset)) {
      for (uint32_t replaceOffset=0; replaceOffset<patch->length; replaceOffset++) {
        if (patch->replaceMask[replaceOffset] == 0x00)
          continue;
        image[offset+replaceOffset] = patch->replace[replaceOffset];
      }
    }
  }
  clearSearchAndReplace();
}

static void jsmn_start_arr(void *user_arg) {
  cursor = 0;
}
static void jsmn_end_arr(void *user_arg) {
  if (parser.stack_height == 6) {
    if (strcmp("positions", jsonGroupName) == 0) {
      if (strcmp("search", jsonValueName) == 0) {
        searchPosition.length = cursor;
      }
    } else if (strcmp("searchAndReplace", jsonGroupName) == 0) {
        if (strcmp("search", jsonValueName) == 0 || strcmp("replace", jsonValueName) == 0) {
          if (searchAndReplacePatch.length == 0) {
            searchAndReplacePatch.length = cursor;
          } else {
            searchAndReplacePatch.length = min(cursor, searchAndReplacePatch.length);
            doSearchAndReplace();
          }
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

  if (strcmp("positions", jsonGroupName) == 0) {
    doSearchPosition();
  }
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
        if (strcmp("positions", jsonGroupName) == 0 && key_len == 2) {
          if (cursor >= PATCH_MAX_BYTES)
            return;
              
          char* values;
          char* mask;
          if (strcmp("search", jsonValueName) == 0) {
            values = searchPosition.search;
            mask = searchPosition.searchMask;
          }
          if (strcmp("??", key) == 0) {
            mask[cursor] = 0x00;
          } else {
            values[cursor] = (char)xtob((char*)key);
            mask[cursor] = 0xFF;
          }
          cursor++;
        } else if (strcmp("searchAndReplace", jsonGroupName) == 0 && key_len == 2) {
          if (cursor >= PATCH_MAX_BYTES)
            return;
              
          char* values;
          char* mask;
          if (strcmp("search", jsonValueName) == 0) {
            values = searchAndReplacePatch.search;
            mask = searchAndReplacePatch.searchMask;
          } else if (strcmp("replace", jsonValueName) == 0) {
            values = searchAndReplacePatch.replace;
            mask = searchAndReplacePatch.replaceMask;
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

void Patch_Apply(char* imageBytes, char* patchName, uint32_t imageLength) {
  //TODO ERRORS

  FIL ffile;
  uint8_t ffs_result;

  char filepath[COUNT_OF(PATCH_SD_BASE_PATH)+32+5];
  strcpy(filepath, PATCH_SD_BASE_PATH);
  char* filename = filepath + COUNT_OF(PATCH_SD_BASE_PATH)-1;
  strcpy(filename, patchName);
  char* fileext = filename + strlen(filename);
  strcpy(fileext, ".json");

  ffs_result = f_open(&ffile, filepath, FA_READ);
  if (ffs_result == FR_OK) {
    uint32_t filesize = f_size(&ffile);
    uint32_t bytesRead = 0;
    uint32_t allBytesRead = 0;
    clearSearchPosition();
    positionCount = 0;
    positionSearchFailed = false;
    clearSearchAndReplace();
    image = imageBytes;
    imageLen = imageLength;
    
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