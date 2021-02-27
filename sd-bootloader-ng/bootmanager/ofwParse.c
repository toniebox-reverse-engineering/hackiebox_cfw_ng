#include "ofwParse.h"
#include <string.h>

enum BOOTINFO_PARSE_RESULT Bootinfo_Parse(char content[8], sBootInfoCust* bootinfo) {
    uint8_t firmwareRaw;
    uint32_t stateRaw;
    memcpy(&firmwareRaw, &content[0], 1);
    memcpy(&stateRaw, &content[4], 4);

    if (firmwareRaw == 0x00) {
        bootinfo->firmware = FW_SLOT_MCUIMG1;
    } else if (firmwareRaw == 0x01) {
        bootinfo->firmware = FW_SLOT_MCUIMG2;
    } else if (firmwareRaw == 0x02) {
        bootinfo->firmware = FW_SLOT_MCUIMG3;
    } else {
        bootinfo->firmware = FW_SLOT_INVALID;
        return BOOTINFO_RESULT_FAIL;
    }

    //Logic needs to be reversed from bootloader see function @0x20038648
    switch (stateRaw) {
    case FW_STATE_NOTEST:
    case FW_STATE_TESTING:
    case FW_STATE_TESTREADY:
        bootinfo->state = stateRaw;
        return BOOTINFO_RESULT_OK;
        break;
    
    default:
        bootinfo->state = FW_STATE_INVALID;
    }
    return BOOTINFO_RESULT_FAIL;
}

void Bootinfo_Generate(sBootInfoCust* bootinfo, char result[8]) {
    uint32_t firmwareRaw;
    uint32_t stateRaw;
    //TODO
    memcpy(&result[0], &bootinfo->firmware, 1);
    memcpy(&result[4], &bootinfo->state, 4);
}