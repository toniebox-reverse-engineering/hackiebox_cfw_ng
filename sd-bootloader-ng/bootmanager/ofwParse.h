#ifndef __OFWPARSE_H__
#define __OFWPARSE_H__
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#define OFW_BOOTINFO_PATH "/sys/mcubootinfo.bin"
#define OFW_IMAGE_PATH "/sys/mcuimgN.bin"

enum BOOTINFO_FIRMWARE {
    FW_SLOT_MCUIMG1 = 0,
    FW_SLOT_MCUIMG2 = 1,
    FW_SLOT_MCUIMG3 = 2,
    FW_SLOT_INVALID = 0xFF,
};
enum BOOTINFO_STATE {
    FW_STATE_NOTEST = 0xABCDDCBA,
    FW_STATE_TESTING = 0x12344321,
    FW_STATE_TESTREADY = 0x56788765,
    FW_STATE_INVALID = 0xFFFFFFFF
};

enum BOOTINFO_PARSE_RESULT {
    BOOTINFO_RESULT_OK = 0x00,
    BOOTINFO_RESULT_REWRITE = 0x01,
    BOOTINFO_RESULT_FAIL = 0xFF,
};

typedef struct sBootInfoCust
{
  enum BOOTINFO_FIRMWARE firmware;
  enum BOOTINFO_STATE state;
} sBootInfoCust;

enum BOOTINFO_PARSE_RESULT Bootinfo_Parse(char content[8], sBootInfoCust* bootinfo);
//void Bootinfo_Generate(sBootInfoCust* bootinfo, char result[8]);

#ifdef __cplusplus
}
#endif
#endif