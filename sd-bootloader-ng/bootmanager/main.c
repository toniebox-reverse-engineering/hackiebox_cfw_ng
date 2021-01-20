//*****************************************************************************
//
// Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
//
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions
//  are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the
//    distribution.
//
//    Neither the name of Texas Instruments Incorporated nor the names of
//    its contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
//  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//*****************************************************************************

//*****************************************************************************
//
// Application Name     - Application Bootloader
// Application Overview - This application showcases the secondary bootloader operations to manage
//					   updates to application image.
//
//*****************************************************************************

//****************************************************************************
//
//! \addtogroup application_bootloader
//! @{
//
//****************************************************************************

#include <stdint.h>
#include <stdbool.h>

#include "hw_ints.h"
#include "hw_types.h"
#include "hw_memmap.h"
#include "hw_gprcm.h"
#include "hw_common_reg.h"
#include "rom.h"
#include "rom_map.h"
#include "prcm.h"
#include "simplelink.h"
#include "interrupt.h"
#include "gpio.h"
#include "pin.h"
#include "udma_if.h"
#include "flc.h"
#include "bootmgr.h"

#include "utils.h"
//#include "sdhost.h" //TODO: fixes some compiler warnings, even diskio.h should load it!

#include "ff.h"
#include "diskio.h"

static FATFS fatfs;

#include "jsmn_stream.h"

//#define FIXED_BOOT_IMAGE

#define IMG_OFW_ID_1 0
#define IMG_OFW_ID_2 1
#define IMG_OFW_ID_3 2
#define IMG_CFW_ID_1 3
#define IMG_CFW_ID_2 4
#define IMG_CFW_ID_3 5
#define IMG_ADD_ID_1 6
#define IMG_ADD_ID_2 7
#define IMG_ADD_ID_3 8

#define SD_PATH_BASE "/revvox/boot/"
#define SD_PATH_BASE_LEN 13
#define IMG_SD_NAME "ng-CCCN.bin"

#define IMG_FLASH_PATH "/sys/pre-img.bin"
#define IMG_SD_PATH SD_PATH_BASE IMG_SD_NAME
#define IMG_SD_NAME_REPL1_POS 3
#define IMG_SD_NAME_REPL2_POS 6
#define IMG_SD_PATH_REPL1_POS SD_PATH_BASE_LEN + IMG_SD_NAME_REPL1_POS
#define IMG_SD_PATH_REPL2_POS SD_PATH_BASE_LEN + IMG_SD_NAME_REPL2_POS
#define IMG_OFW_NAME "ofw"
#define IMG_CFW_NAME "cfw"
#define IMG_ADD_NAME "add"
#define CFG_SD_PATH SD_PATH_BASE "ngCfg.json"

#define IMG_SD_BOOTLOADER_NAME "ngBootloader.bin"
#define IMG_SD_BOOTLOADER_PATH SD_PATH_BASE IMG_SD_BOOTLOADER_NAME

#define IMG_MAX_COUNT 9


#define max(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b;       \
})

#define min(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b;       \
})

#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))


typedef struct sGeneralSettings
{
  uint8_t activeImage;
  bool waitForPress;

} sGeneralSettings;
static sGeneralSettings generalSettings = {0, false};

typedef struct sImageInfo
{
  bool fileExists;
  bool checkHash;
} sImageInfo;
static sImageInfo aImageInfo[IMG_MAX_COUNT];

char* imagePath; //TODO!
static char* GetImagePathById(uint8_t number) {
  //char* imagePath;
  char id = (char)((number%3) + 0x31); //See Ascii Table - 1 starts at 0x31
  char* name;

  if (number < 3) {
    name = IMG_OFW_NAME;
  } else if (number < 6) {
    name = IMG_CFW_NAME;
  } else /*if (number < 9)*/ {
    name = IMG_ADD_NAME;
  }

  imagePath = IMG_SD_PATH;
  for (uint8_t i=0; i<3; i++)
  {
    imagePath[IMG_SD_PATH_REPL1_POS+i] = name[i];
  }
  imagePath[IMG_SD_PATH_REPL2_POS] = id;

  return imagePath;
}
//*****************************************************************************
// Vector Table
extern void (*const g_pfnVectors[])(void);



//*****************************************************************************
//
//! Board Initialization & Configuration
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************
static void
BoardInitBase(void)
{

  //
  // Set vector table base
  //
  MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);

  //
  // Enable Processor Interrupts
  //
  IntMasterEnable();
  IntEnable(FAULT_SYSTICK);

  //
  // Mandatory MCU Initialization
  //
  PRCMCC3200MCUInit();
}

//*****************************************************************************
//
//! Executed the application from given location
//!
//! \param ulBaseLoc is the base address of the application
//!
//! This function execution the application loaded at \e ulBaseLoc. It assumes
//! the vector table is placed at the base address thus sets the new Stack
//! pointer from the first word.
//!
//! \return None.
//
//*****************************************************************************
void Run(unsigned long ulBaseLoc)
{

  //
  // Set the SP
  //
  __asm("	ldr    sp,[r0]\n"
        "	add    r0,r0,#4"); //Get address of ResetISR from .intvecs

  //
  // Jump to entry code
  //
  __asm("	ldr    r1,[r0]\n"
        "	bx     r1");
}

#define COLOR_GREEN 1
#define COLOR_BLUE 2
#define COLOR_RED 4
#define COLOR_CYAN 3


#define HAL_FCPU_MHZ 80U
#define HAL_FCPU_HZ (1000000U * HAL_FCPU_MHZ)
#define HAL_SYSTICK_PERIOD_US 1000U
#define UTILS_DELAY_US_TO_COUNT(us) (((us)*HAL_FCPU_MHZ) / 6)

#define APP_IMG_SRAM_OFFSET 0x20004000
#define CFG_SRAM_OFFSET 0x20000000

#define EAR_BIG_PRCM PRCM_GPIOA0
#define EAR_SMALL_PRCM PRCM_GPIOA0

#define LED_GREEN_PORT GPIOA3_BASE
#define LED_BLUE_PORT GPIOA3_BASE
#define EAR_BIG_PORT GPIOA0_BASE
#define EAR_SMALL_PORT GPIOA0_BASE
#define POWER_SD_PORT GPIOA0_BASE
#define POWER_PORT GPIOA0_BASE

//#define LED_GREEN_GPIO pin_GP25
#define LED_GREEN_PIN_NUM PIN_21 // GP25/SOP2
#define LED_BLUE_PIN_NUM PIN_17 //GP24
#define EAR_BIG_PIN_NUM PIN_57   // GP02
#define EAR_SMALL_PIN_NUM PIN_59 // GP04
#define POWER_SD_PIN_NUM PIN_58 //GP03
#define POWER_PIN_NUM PIN_61 //GP06

#define LED_GREEN_PORT_MASK GPIO_PIN_1
#define LED_BLUE_PORT_MASK GPIO_PIN_0
#define EAR_BIG_PORT_MASK GPIO_PIN_2
#define EAR_SMALL_PORT_MASK GPIO_PIN_4
#define POWER_SD_PORT_MASK GPIO_PIN_3
#define POWER_PORT_MASK GPIO_PIN_6


static void LedGreenOn() {
  MAP_GPIOPinWrite(LED_GREEN_PORT, LED_GREEN_PORT_MASK, LED_GREEN_PORT_MASK);
}
static void LedGreenOff() {
  MAP_GPIOPinWrite(LED_GREEN_PORT, LED_GREEN_PORT_MASK, 0x00);
}
static void LedBlueOn() {
  MAP_GPIOPinWrite(LED_BLUE_PORT, LED_BLUE_PORT_MASK, LED_BLUE_PORT_MASK);
}
static void LedBlueOff() {
  MAP_GPIOPinWrite(LED_BLUE_PORT, LED_BLUE_PORT_MASK, 0x00);
}

static void LedOn(uint8_t color) {
  if (color & COLOR_GREEN) {
    LedGreenOn();
  } if (color & COLOR_BLUE) {
    LedBlueOn();
  }
}
static void LedOff(uint8_t color) {
  if (color & COLOR_GREEN) {
    LedGreenOff();
  } if (color & COLOR_BLUE) {
    LedBlueOff();
  }
}

static void prebootmgr_blink_color(int times, int wait_us, uint8_t color)
{
  for (int i = 0; i < times; i++)
  {
    LedOn(color);
    UtilsDelay(UTILS_DELAY_US_TO_COUNT(wait_us * 1000));
    LedOff(color);
    UtilsDelay(UTILS_DELAY_US_TO_COUNT(wait_us * 1000));
  }
}

static void prebootmgr_blink(int times, int wait_us) {
  prebootmgr_blink_color(times, wait_us, COLOR_GREEN);
}

static void prebootmgr_blink_error(int times, int wait_us) {
  #ifdef FIXED_BOOT_IMAGE
  prebootmgr_blink_color(times, wait_us, COLOR_BLUE);
  #else
  prebootmgr_blink_color(times, wait_us, COLOR_GREEN);
  #endif
}

static void SdInit(void)
{
  //Power SD
  MAP_PinTypeGPIO(POWER_SD_PIN_NUM, PIN_MODE_0, false);
  MAP_GPIODirModeSet(POWER_SD_PORT, POWER_SD_PORT_MASK, GPIO_DIR_MODE_OUT);
  
  MAP_PinTypeSDHost(PIN_64, PIN_MODE_6); //SDHost_D0
  MAP_PinTypeSDHost(PIN_01, PIN_MODE_6); //SDHost_CLK
  MAP_PinTypeSDHost(PIN_02, PIN_MODE_6); //SDHost_CMD

  MAP_GPIOPinWrite(POWER_SD_PORT, POWER_SD_PORT_MASK, 0x00); //SIC! 

  // Set the SD card clock as output pin
  MAP_PinDirModeSet(PIN_01, PIN_DIR_MODE_OUT);
  // Enable Pull up on data
  MAP_PinConfigSet(PIN_64, PIN_STRENGTH_4MA, PIN_TYPE_STD_PU);
  // Enable Pull up on CMD
  MAP_PinConfigSet(PIN_02, PIN_STRENGTH_4MA, PIN_TYPE_STD_PU);

  // Enable SD peripheral clock
  MAP_PRCMPeripheralClkEnable(PRCM_SDHOST, PRCM_RUN_MODE_CLK | PRCM_SLP_MODE_CLK);
	// Reset MMCHS
	MAP_PRCMPeripheralReset(PRCM_SDHOST);
	// Configure MMCHS
	MAP_SDHostInit(SDHOST_BASE);
	// Configure card clock
	MAP_SDHostSetExpClk(SDHOST_BASE, MAP_PRCMPeripheralClockGet(PRCM_SDHOST), 15000000);
  MAP_SDHostBlockSizeSet(SDHOST_BASE, 512); //SD_SECTOR_SIZE
  
}

static void BoardInitCustom(void)
{
  MAP_PRCMPeripheralClkEnable(PRCM_GPIOA0, PRCM_RUN_MODE_CLK | PRCM_SLP_MODE_CLK); //Clock for GPIOA0 (Ear Buttons / SD Power / Power)
  MAP_PRCMPeripheralClkEnable(PRCM_GPIOA3, PRCM_RUN_MODE_CLK | PRCM_SLP_MODE_CLK); //Clock for GPIOA3 (Green/Blue LED)

  //Green LED
  MAP_PinTypeGPIO(LED_GREEN_PIN_NUM, PIN_MODE_0, false);
  MAP_GPIODirModeSet(LED_GREEN_PORT, LED_GREEN_PORT_MASK, GPIO_DIR_MODE_OUT);
  //Blue LED
  MAP_PinTypeGPIO(LED_BLUE_PIN_NUM, PIN_MODE_0, false);
  MAP_GPIODirModeSet(LED_BLUE_PORT, LED_BLUE_PORT_MASK, GPIO_DIR_MODE_OUT);

  //Big Ear
  MAP_PinTypeGPIO(EAR_BIG_PIN_NUM, PIN_MODE_0, false);
  MAP_GPIODirModeSet(EAR_BIG_PORT, EAR_BIG_PORT_MASK, GPIO_DIR_MODE_IN);
  //Small Ear
  MAP_PinTypeGPIO(EAR_SMALL_PIN_NUM, PIN_MODE_0, false);
  MAP_GPIODirModeSet(EAR_SMALL_PORT, EAR_SMALL_PORT_MASK, GPIO_DIR_MODE_IN);

  //Power other peripherals
  MAP_PinTypeGPIO(POWER_PIN_NUM, PIN_MODE_0, false);
  MAP_GPIODirModeSet(POWER_PORT, POWER_PORT_MASK, GPIO_DIR_MODE_OUT);
  MAP_GPIOPinWrite(POWER_PORT, POWER_PORT_MASK, POWER_PORT_MASK);
  
  SdInit();

  sl_Start(NULL, NULL, NULL);
}
static void BoardDeinitCustom(void)
{
  sl_Stop(30);
  //Power off SD
  MAP_GPIOPinWrite(POWER_SD_PORT, POWER_SD_PORT_MASK, POWER_SD_PORT_MASK); //SIC! 
  //Power off other peripherals
  MAP_GPIOPinWrite(POWER_PORT, POWER_PORT_MASK, 0x00);
}

static bool EarSmallPressed(void) {
  return !(EAR_SMALL_PORT_MASK & MAP_GPIOPinRead(EAR_SMALL_PORT, EAR_SMALL_PORT_MASK));
}
static bool EarBigPressed(void) {
  return !(EAR_BIG_PORT_MASK & MAP_GPIOPinRead(EAR_BIG_PORT, EAR_BIG_PORT_MASK));
}

static bool SdFileExists(char* filename) {
  FIL ffile;
  if (f_open(&ffile, filename, FA_READ) == FR_OK) {
      f_close(&ffile); 
      return true;
  }
  return false;
}

static uint8_t Selector(uint8_t startNumber) {
  int8_t counter = startNumber;

  while (!aImageInfo[counter].fileExists)
  {
    if (counter < 9) {
      counter +=1;
    } else {
      counter = 0;
    }
  }

  LedGreenOn();
  while (EarSmallPressed()) {
      UtilsDelay(UTILS_DELAY_US_TO_COUNT(10 * 1000)); //Wait while pressed
  }
  LedGreenOff();
  
  LedBlueOn();
  while (generalSettings.waitForPress)
  {
    if (EarBigPressed())
      break;
  }
  LedBlueOff();  

  while (EarBigPressed()) {
    if (EarSmallPressed()) {
        do
        {
          if (counter < 9) {
            counter +=1;
          } else {
            counter = 0;
          }
        } while (!aImageInfo[counter].fileExists);
        while (EarSmallPressed()) {
            UtilsDelay(UTILS_DELAY_US_TO_COUNT(10 * 1000)); //Wait while pressed
        }
    }
    if (counter < 3) {
      prebootmgr_blink_color((counter+1)-0, 100, COLOR_GREEN);
    } else if (counter < 6) {
      prebootmgr_blink_color((counter+1)-3, 100, COLOR_BLUE);
    } else /*if (counter < 9)*/ {
      prebootmgr_blink_color((counter+1)-6, 100, COLOR_CYAN);
    }
    UtilsDelay(UTILS_DELAY_US_TO_COUNT(500 * 1000));
  }
  return counter;
}

static bool SdImageExists(uint8_t number) {
  char* image = GetImagePathById(number);
  return SdFileExists(image);
}
static bool CheckSdImages() {
  bool hasValidImage = false;
  for (uint8_t i=0; i<IMG_MAX_COUNT; i++)
  {
    aImageInfo[i].fileExists = SdImageExists(i);
    hasValidImage |= aImageInfo[i].fileExists;
  }
  return hasValidImage;
}

static uint8_t GetImageNumber(char* imageId)
{
  uint8_t factor = 0;
  uint8_t id = 0;
  if (strncmp(imageId, IMG_OFW_NAME, 3) == 0)
  {
    factor = 0;
  } else if (strncmp(imageId, IMG_CFW_NAME, 3) == 0)
  {
    factor = 1;
  } else if (strncmp(imageId, IMG_CFW_NAME, 3) == 0)
  {
    factor = 2;
  }
  id = (char)(imageId[3] - 0x31);
  if (id>=0 && id<3) {
    return id + 3*factor;
  }
  return 0;
}

char jsonGroupName[8];
char jsonValueName[17];

jsmn_stream_parser parser;
void jsmn_start_arr(void *user_arg) {
    uint8_t test = 1;
    /* An example of using the user arg / context pointer in a callback */
    //printf("Array started. Parser id = %d\n", *parser_id);
}
void jsmn_end_arr(void *user_arg) {
    uint8_t test = 1;
    //printf("Array ended\n");
}
void jsmn_start_obj(void *user_arg) {
    uint8_t test = 1;
    //printf("Object started\n");
}
void jsmn_end_obj(void *user_arg) {
    uint8_t test = 1;
    //printf("Object ended\n");
}
void jsmn_obj_key(const char *key, size_t key_len, void *user_arg) {

    switch (parser.stack_height)
    {
    case 1:
      strncpy(jsonGroupName, key, min(key_len, COUNT_OF(jsonGroupName)));
      jsonGroupName[COUNT_OF(jsonGroupName)] = '\0';
      break;
    case 3:
      strncpy(jsonValueName, key, min(key_len, COUNT_OF(jsonValueName)));
      jsonValueName[COUNT_OF(jsonValueName)] = '\0';
      break;
    }
}
void jsmn_str(const char *value, size_t len, void *user_arg) {
    if (parser.stack_height != 4)
      return;

    if (strcmp("general", jsonGroupName) == 0)
    {
      if (strcmp("activeImg", jsonValueName) == 0)
      {
        generalSettings.activeImage = GetImageNumber(value);
      }
    }
    else if (strncmp(jsonGroupName, "ofw", 3) == 0
      || strncmp(jsonGroupName, "cfw", 3)
      || strncmp(jsonGroupName, "add", 3))
    {
      uint8_t imageNumber = GetImageNumber(jsonGroupName);

    }
    
}
void jsmn_primitive(const char *value, size_t len, void *user_arg) {
    if (parser.stack_height != 4)
      return;

    if (strcmp("general", jsonGroupName) == 0)
    {
      if (strcmp("waitForPress", jsonValueName) == 0)
      {
        generalSettings.waitForPress = (value[0] == 't');
      }
    }
    else if (strncmp(jsonGroupName, "ofw", 3) == 0
      || strncmp(jsonGroupName, "cfw", 3)
      || strncmp(jsonGroupName, "add", 3))
    {
      uint8_t imageNumber = GetImageNumber(jsonGroupName);
      if (strcmp("sha256", jsonGroupName) == 0) 
      {
        aImageInfo[imageNumber].checkHash = (value[0] == 't');
      }
    }
}

jsmn_stream_callbacks_t cbs = {
    jsmn_start_arr,
    jsmn_end_arr,
    jsmn_start_obj,
    jsmn_end_obj,
    jsmn_obj_key,
    jsmn_str,
    jsmn_primitive
};

//*****************************************************************************
//
//! Main function
//!
//! This is the main function for this application bootloader
//!
//! \return None
//
//*****************************************************************************
int main()
{

  sBootInfo_t sBootInfo;
  FIL ffile;
  uint8_t ffs_result;

  //
  // Board Initialization
  //
  BoardInitBase();
  BoardInitCustom();

  #ifndef FIXED_BOOT_IMAGE
  for (uint8_t i = 0; i < IMG_MAX_COUNT; i++)
  {
    aImageInfo[i].fileExists = false;
    aImageInfo[i].checkHash = true;
  }
  #endif

  UtilsDelay(UTILS_DELAY_US_TO_COUNT(100 * 1000));
  ffs_result = f_mount(&fatfs, "0", 1);
  if (ffs_result == FR_OK)
  {
    #ifdef FIXED_BOOT_IMAGE
    char* image = IMG_SD_BOOTLOADER_PATH;
    if (SdFileExists(image)) {
    #else
    if (CheckSdImages()) {
      char activeImageName[4];

      ffs_result = f_open(&ffile, CFG_SD_PATH, FA_READ);
      if (ffs_result == FR_OK) {
        uint32_t filesize = f_size(&ffile);
        uint32_t bytesRead = 0;
        uint32_t allBytesRead = 0;
        if (filesize>0x3000) //Maximum 0x3000, 12kB for JSON should be enough
          filesize = 0x3000;
        
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
      }

      uint8_t selectedImgNum = Selector(generalSettings.activeImage);
      char* image = GetImagePathById(selectedImgNum);
    #endif

      ffs_result = f_open(&ffile, image, FA_READ);
      if (ffs_result == FR_OK) {
          uint32_t filesize = f_size(&ffile);

          unsigned long* pImgRun = (unsigned long *)APP_IMG_SRAM_OFFSET;
          ffs_result = f_read(&ffile, pImgRun, filesize, &filesize);
          if (ffs_result == FR_OK) {
            f_close(&ffile); 
            BoardDeinitCustom();
            Run((unsigned long)pImgRun);
          } else {
              UtilsDelay(UTILS_DELAY_US_TO_COUNT(1000 * 1000));
              prebootmgr_blink_error(4, 500);
              UtilsDelay(UTILS_DELAY_US_TO_COUNT(2000 * 1000));
              prebootmgr_blink_error(ffs_result, 1000);
          }
      } else {
          UtilsDelay(UTILS_DELAY_US_TO_COUNT(1000 * 1000));
          prebootmgr_blink_error(3, 500);
          UtilsDelay(UTILS_DELAY_US_TO_COUNT(2000 * 1000));
          prebootmgr_blink_error(ffs_result, 1000);
      }
    } else {
      //TODO: No bootable files on sd found
    }
    UtilsDelay(UTILS_DELAY_US_TO_COUNT(2000 * 1000));
  }

  UtilsDelay(UTILS_DELAY_US_TO_COUNT(500 * 1000));
  prebootmgr_blink_error(2, 500);
  UtilsDelay(UTILS_DELAY_US_TO_COUNT(500 * 1000));
  prebootmgr_blink_error(ffs_result, 1000);

  SlFsFileInfo_t pFsFileInfo;
  _i32 fhandle;
  if (!sl_FsOpen(IMG_FLASH_PATH, FS_MODE_OPEN_READ, NULL, &fhandle)) {
      if (!sl_FsGetInfo(IMG_FLASH_PATH, 0, &pFsFileInfo)) {
          if (pFsFileInfo.FileLen == sl_FsRead(fhandle, 0, (unsigned char *)APP_IMG_SRAM_OFFSET, pFsFileInfo.FileLen)) {
              sl_FsClose(fhandle, 0, 0, 0);
              BoardDeinitCustom();
              Run(APP_IMG_SRAM_OFFSET);
          }
      }
  }

  
  while (true)
  {
    prebootmgr_blink_error(3, 33);
    prebootmgr_blink_error(3, 66);
    prebootmgr_blink_error(3, 33);

    __asm volatile(" dsb \n"
                   " isb \n"
                   " wfi \n");
  }
}
//*****************************************************************************
// WLAN Event handler callback hookup function
//*****************************************************************************
void SimpleLinkWlanEventHandler(SlWlanEvent_t *pWlanEvent)
{
}

//*****************************************************************************
//
//! \brief This function handles General Events
//!
//! \param[in]     pDevEvent - Pointer to General Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkGeneralEventHandler(SlDeviceEvent_t *pDevEvent)
{
}

//*****************************************************************************
// HTTP Server callback hookup function
//*****************************************************************************
void SimpleLinkHttpServerCallback(SlHttpServerEvent_t *pHttpEvent,
                                  SlHttpServerResponse_t *pHttpResponse)
{
}

//*****************************************************************************
// Net APP Event callback hookup function
//*****************************************************************************
void SimpleLinkNetAppEventHandler(SlNetAppEvent_t *pNetAppEvent)
{
}

//*****************************************************************************
//
//! This function handles socket events indication
//!
//! \param[in]      pSock - Pointer to Socket Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkSockEventHandler(SlSockEvent_t *pSock)
{
}