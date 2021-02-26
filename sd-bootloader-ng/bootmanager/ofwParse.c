#include "ofwParse.h"
#include <string.h>

enum BOOTINFO_PARSE_RESULT Bootinfo_Parse(char content[8], sBootInfoCust* bootinfo) {
    uint8_t firmwareRaw;
    uint32_t stateRaw;
    memcpy(&firmwareRaw, &content[0], 1);
    memcpy(&stateRaw, &content[4], 4);


    //Logic from OFW Bootloader
    if (firmwareRaw == 0x01) {
        bootinfo->firmware = FW_SLOT_MCUIMG2;
    } else if ((firmwareRaw & 0xfd) == 0) {
        bootinfo->firmware = FW_SLOT_MCUIMG1;
    } else {
        bootinfo->firmware = FW_SLOT_MCUIMG3;
    }

    //Logic needs to be reversed from bootloader see function @0x20038648
    switch (stateRaw) {
    case FW_STATE_NOTEST:
    case FW_STATE_TESTING:
    case FW_STATE_TESTREADY:
        bootinfo->state = stateRaw;
        break;
    
    default:
        bootinfo->state = FW_STATE_INVALID;
    }
}

void Bootinfo_Generate(sBootInfoCust* bootinfo, char result[8]) {
    uint32_t firmwareRaw;
    uint32_t stateRaw;
    //TODO
    memcpy(&result[0], &bootinfo->firmware, 1);
    memcpy(&result[4], &bootinfo->state, 4);
}