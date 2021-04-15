#include "patch.h"
#include "armAsm.h"
#include <stdlib.h> 
#include <stddef.h>
#include "printf.h"
#include "logger.h"

static sSearchPosition searchPosition;
static uint32_t positions[PATCH_MAX_POSITIONS];
static uint8_t positionCount;
static bool positionSearchFailed;

static sSearchAndReplacePatch searchAndReplacePatch;
static char* image;
static uint32_t imageLen;

static sAsmReplace asmReplace;

static jsmn_stream_parser parser;
static char jsonGroupName[17];
static char jsonValueName[17];
static char jsonSpeciName[17];
static char jsonSpeciSubName[17];
static uint8_t cursor = 0;

static bool searchInMemory(char* search, char* searchMask, uint8_t length, uint32_t* position) {
  bool found = false;

  uint32_t offset;
  uint32_t longestHitPos = 0;
  uint8_t longestHitLen = 0;
  for (offset=0; offset<imageLen-length; offset++) {
    if (search[0] != image[offset] && searchMask[0] != 0x00)
      continue;

    uint8_t offset2;
    for (offset2=1; offset2<length; offset2++) {
      if (searchMask[offset2] == 0x00)
        continue;
      if (search[offset2] != image[offset+offset2]) {
        
        if (offset2 > longestHitLen) {
          longestHitPos = offset;
          longestHitLen = offset2;
        }

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

  if (!found) {
    Logger_error("searchInMemory failed, best at offset=0x%x with length=%i of %ibytes", longestHitPos, longestHitLen, length);
    if (Logger_needed(DEBUG_LOG_LEVEL_DEBUG)) {
      Logger_debug_nonl("search  = ");
      for (uint8_t offset=0; offset<length; offset++) {
          if (searchMask[offset] == 0xff) { 
            printf("%02x ", (uint8_t)search[offset]);
          } else {
            printf("?? ");
          }
      }
      Logger_newLine();
      Logger_debug_nonl("besthit = ");
      for (uint8_t offset=0; offset<longestHitLen; offset++) {
          printf("%02x", (uint8_t)image[longestHitPos+offset]);
          if (offset<longestHitLen - 1)
            printf(" ");
      }
      for (uint8_t offset=longestHitLen; offset<length; offset++) {
          if (image[longestHitPos+offset] != search[offset] && searchMask[offset] == 0xff) { 
            printf("|", (uint8_t)search[offset]);
          } else {
            printf(" ");
          }
          printf("%02x", (uint8_t)image[longestHitPos+offset]);
      }
      Logger_newLine();           
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
  pos->deasmAddress = false;
}
static void doSearchPosition() {
  if (positionSearchFailed)
    return;

  sSearchPosition* pos = &searchPosition;
  if (pos->length == 0 || positionCount >= COUNT_OF(positions)) {
    Logger_error("SearchPosition length=0 or too many positions(%i)", positionCount);
    positionSearchFailed = true;
    clearSearchPosition();
    return;
  }
  
  uint32_t offset = 0;
  if (!searchInMemory(pos->search, pos->searchMask, pos->length, &offset)) {
    Logger_error("SearchPosition %i not found", positionCount);
    positionSearchFailed = true;
    clearSearchPosition();
    return;
  }

  positions[positionCount] = offset + pos->offset;
  if (pos->deasmAddress) {
    uint32_t addr = APP_IMG_SRAM_OFFSET + positions[positionCount];
    uint32_t target = 0;
    ArmDasmT(addr, (char*)addr, &target, NULL);
    positions[positionCount] = target - APP_IMG_SRAM_OFFSET;
  }

  Logger_info("SearchPosition %i found @0x%x", positionCount, positions[positionCount]);

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
  patch->searchMemPos = 0;
}

static void doSearchAndReplace() {
  sSearchAndReplacePatch* patch = &searchAndReplacePatch;
  if (!positionSearchFailed) {
    if (patch->length > 0) {
      if (patch->searchMemPos > 0 || searchInMemory(patch->search, patch->searchMask, patch->length, &patch->searchMemPos)) {
        Logger_debug("Replace %ib @0x%x", patch->length, patch->searchMemPos);
        Logger_trace_nonl("search  = ");
        if (Logger_needed(DEBUG_LOG_LEVEL)) {
          for (uint8_t replaceOffset=0; replaceOffset<patch->length; replaceOffset++) {  
            if (patch->searchMask[replaceOffset] == 0x00) {
              printf(" ??");
            } else {
              printf(" %02x", (uint8_t)patch->search[replaceOffset]);
            }
          }
          Logger_newLine();
        }
        Logger_trace_nonl("replace = ");
        for (uint8_t replaceOffset=0; replaceOffset<patch->length; replaceOffset++) {      
          if (Logger_needed(DEBUG_LOG_LEVEL)) {
            if (patch->replaceMask[replaceOffset] == 0x00) {
              printf(" ??");
            } else {
              if (patch->replace[replaceOffset] != image[patch->searchMemPos+replaceOffset]) {
                printf("|");
              } else {
                printf(" ");
              }
              printf("%02x", (uint8_t)patch->replace[replaceOffset]);
            }
          }
          if (patch->replaceMask[replaceOffset] == 0x00)
            continue;
          
          image[patch->searchMemPos+replaceOffset] = patch->replace[replaceOffset];
        }
        if (Logger_needed(DEBUG_LOG_LEVEL))
          Logger_newLine();
      } else {
        Logger_error("Patch not applied, searchInMemory failed");
      }
    } else {
      Logger_error("Patch length == 0");
    }
  } else {
    Logger_error("Error during positionSearch, skip SearchAndReplace");
  }
  clearSearchAndReplace();
}

static void clearAsmReplace() {
  memset(asmReplace.instruction, 0x00, COUNT_OF(asmReplace.instruction));
  memset(asmReplace.parameter, 0x00, COUNT_OF(asmReplace.parameter));
  asmReplace.length = 0;
}
static void doAsmReplace() {
  if (asmReplace.length == 0) {
    Logger_error("asmReplace.length == 0");
    return;
  }

  if (asmReplace.parameter[0] != 'p' || asmReplace.parameter[1] == '\0') {
    Logger_error("Invalid parameter %s", asmReplace.parameter);
    goto cleanUp;
  }

  uint8_t id = strtoul(&asmReplace.parameter[1], NULL, 0);

  if (positionCount <= id) {
    Logger_error("positionCount(%i) <= id(%i) ", positionCount, id);
    goto cleanUp;
  }

  uint32_t target = APP_IMG_SRAM_OFFSET + positions[id];

  uint32_t offset = searchAndReplacePatch.searchMemPos; //TODO! Searching twice and assert that search is before replace.
  uint32_t pc = APP_IMG_SRAM_OFFSET + offset + cursor; 

  if (strcmp("b", asmReplace.instruction) == 0) {
    char instr[2];
    ArmAsmT_b(pc, target, instr);

    searchAndReplacePatch.replace[cursor] = instr[0];
    searchAndReplacePatch.replaceMask[cursor] = 0xFF;
    searchAndReplacePatch.replace[cursor+1] = instr[1];
    searchAndReplacePatch.replaceMask[cursor+1] = 0xFF;
  } else if (strcmp("bl", asmReplace.instruction) == 0) {
    char instr[4];
    ArmAsmT_bl(pc, target, instr);
    searchAndReplacePatch.replace[cursor] = instr[0];
    searchAndReplacePatch.replaceMask[cursor] = 0xFF;
    searchAndReplacePatch.replace[cursor+1] = instr[1];
    searchAndReplacePatch.replaceMask[cursor+1] = 0xFF;
    searchAndReplacePatch.replace[cursor+2] = instr[2];
    searchAndReplacePatch.replaceMask[cursor+2] = 0xFF;
    searchAndReplacePatch.replace[cursor+3] = instr[3];
    searchAndReplacePatch.replaceMask[cursor+3] = 0xFF;
  } else {
    Logger_error("Unknown instruction %s", asmReplace.instruction);
  }

  cleanUp:
  cursor += asmReplace.length;
  clearAsmReplace();
}

static bool jsmn_hasIgnoreName(void) {
    if (jsonGroupName[0]=='_' || jsonValueName[0]=='_' || jsonSpeciName[0]=='_' || jsonSpeciSubName[0]=='_') //Ignore
      return true;
    return false;
}
static void jsmn_start_arr(void *user_arg) {
  cursor = 0;
}
static void jsmn_end_arr(void *user_arg) {
  if (parser.stack_height == 6) {
    if (jsmn_hasIgnoreName())
      return;
    if (strcmp("positions", jsonGroupName) == 0) {
      if (strcmp("search", jsonValueName) == 0) {
        searchPosition.length = cursor;
      }
    } else if (strcmp("searchAndReplace", jsonGroupName) == 0) {
        if (strcmp("search", jsonValueName) == 0 || strcmp("replace", jsonValueName) == 0) {
          if (searchAndReplacePatch.length == 0) {
            searchAndReplacePatch.length = cursor;
            //TODO! Searching twice and assert that search is before replace.
            searchInMemory(searchAndReplacePatch.search, searchAndReplacePatch.searchMask, searchAndReplacePatch.length, &searchAndReplacePatch.searchMemPos);  
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
  switch (parser.stack_height)
  {
  case 4:
    if (jsmn_hasIgnoreName())
      break;
    if (strcmp("positions", jsonGroupName) == 0) {
      doSearchPosition();
    }
    break;
  case 7:
    if (jsmn_hasIgnoreName())
      break;
    if (strcmp("searchAndReplace", jsonGroupName) == 0
      && strcmp("replace", jsonValueName) == 0
      && strcmp("asm", jsonSpeciName) == 0) {
      doAsmReplace();
    }
    break;
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
    case 4:
      len = min(key_len, COUNT_OF(jsonValueName)-1);
      strncpy(jsonValueName, key, len);
      jsonValueName[min(key_len, COUNT_OF(jsonValueName))] = '\0';
      break;
    case 6:
        if (jsmn_hasIgnoreName())
          break;
        if (strcmp("positions", jsonGroupName) == 0 && key_len == 2) {
          if (cursor >= PATCH_MAX_BYTES)
            return;
              
          char* values;
          char* mask;
          if (strcmp("search", jsonValueName) == 0) {
            values = searchPosition.search;
            mask = searchPosition.searchMask;
          } else {
            return;
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
          } else {
            break;
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
      case 7:
        len = min(key_len, COUNT_OF(jsonSpeciName)-1);
        strncpy(jsonSpeciName, key, len);
        jsonSpeciName[min(key_len, COUNT_OF(jsonSpeciName))] = '\0';
        break;
      case 9:
        len = min(key_len, COUNT_OF(jsonSpeciSubName)-1);
        strncpy(jsonSpeciSubName, key, len);
        jsonSpeciSubName[min(key_len, COUNT_OF(jsonSpeciSubName))] = '\0';
        break;
    }
}
static void jsmn_str(const char *value, size_t len, void *user_arg) {
    uint8_t tar_len;
    switch (parser.stack_height) {
    case 10:
      if (jsmn_hasIgnoreName())
        break;
      if (strcmp("searchAndReplace", jsonGroupName) == 0
        && strcmp("replace", jsonValueName) == 0
        && strcmp("asm", jsonSpeciName) == 0) {

        if (strcmp("instr", jsonSpeciSubName) == 0) {
          tar_len = min(len, COUNT_OF(asmReplace.instruction)-1);
          strncpy(asmReplace.instruction, value, tar_len);
          asmReplace.instruction[min(len, COUNT_OF(asmReplace.instruction))] = '\0';
        } else if (strcmp("param", jsonSpeciSubName) == 0) {
          tar_len = min(len, COUNT_OF(asmReplace.parameter)-1);
          strncpy(asmReplace.parameter, value, tar_len);
          asmReplace.parameter[min(len, COUNT_OF(asmReplace.parameter))] = '\0';
        }
      }
      break;
    }
}
static void jsmn_primitive(const char *value, size_t len, void *user_arg) {
    switch (parser.stack_height) {
    case 5:
      if (jsmn_hasIgnoreName())
        break;
      if (strcmp("positions", jsonGroupName) == 0) {
        if (strcmp("offset", jsonValueName) == 0) {
          searchPosition.offset = (int32_t)strtol(value, NULL, 0);
        } else if (strcmp("deasmAddress", jsonValueName) == 0) {
          searchPosition.deasmAddress = (value[0] == 't');
        }
      }
      break;
    case 10:
      if (jsmn_hasIgnoreName())
        break;
      if (strcmp("searchAndReplace", jsonGroupName) == 0
        && strcmp("replace", jsonValueName) == 0
        && strcmp("asm", jsonSpeciName) == 0) {
        if (strcmp("length", jsonSpeciSubName) == 0) {
          asmReplace.length = (int8_t)strtoul(value, NULL, 0);
        }
      }
      break;
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
    clearAsmReplace();
    image = imageBytes;
    imageLen = imageLength;

    Logger_info("Read patch %s", patchName);

    jsmn_stream_init(&parser, &cbs, NULL);
    char buffer[512];
    while (allBytesRead<filesize)
    {
      ffs_result = f_read(&ffile, buffer, COUNT_OF(buffer), &bytesRead);
      if (ffs_result != FR_OK) {
        Logger_error("Error while reading patch %s file...", patchName);
        break;
      }

      for (uint16_t i = 0; i < bytesRead; i++)
      {
        jsmn_stream_parse(&parser, buffer[i]);
      }
      allBytesRead += bytesRead;
    }
    f_close(&ffile);
  } else {
    Logger_error("Open sd:%s failed error=%i", filepath, ffs_result);
  }
}