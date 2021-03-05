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
#include "stdlib-add.h"

#include "hw_ints.h"
#include "hw_types.h"
#include "hw_memmap.h"
#include "hw_gprcm.h"
#include "hw_common_reg.h"
#include "hw_nvic.h"
#include "hw_shamd5.h"
#include "hw_dthe.h"
#include "hw_adc.h"
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
#include "shamd5.h"
#include "adc.h"

#include "sdhost.h" //TODO: fixes some compiler warnings, even diskio.h should load it!

#include "ff.h"
#include "diskio.h"

static FATFS fatfs;

#include "jsmn_stream.h"

#include "config.h"
#include "patch.h"
#include "globalDefines.h"
#include "watchdog.h"
#include "logger.h"
#include "ofwParse.h"

#include "wiring.h"


static char* pImgRun = (char*)APP_IMG_SRAM_OFFSET;
static char imagePath[] = IMG_SD_PATH;
static char flashPath[] = OFW_IMAGE_PATH;
static bool slInitialized = false;
static sBootInfoCust bootInfoData = { FW_SLOT_INVALID, FW_STATE_INVALID };
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
#pragma GCC push_options
#pragma GCC optimize ("O0") //Don't optimize or it may fail
static void Run(unsigned long ulBaseLoc)
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
#pragma GCC pop_options




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

static void LedSet(uint8_t color) {
  if (color & COLOR_GREEN) {
    LedGreenOn();
  } else {
    LedGreenOff();
  }
  if (color & COLOR_BLUE) {
    LedBlueOn();
  } else {
    LedBlueOff();
  }
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

static void prebootmgr_blink_color(int times, int wait_ms, uint8_t color)
{
  for (int i = 0; i < times; i++)
  {
    LedSet(color);
    UtilsDelayMsWD(wait_ms);
    LedSet(COLOR_BLACK);
    UtilsDelayMsWD(wait_ms);
  }
}

static void prebootmgr_blink(int times, int wait_ms) {
  prebootmgr_blink_color(times, wait_ms, COLOR_GREEN);
}

static void prebootmgr_blink_error(int times, int wait_ms) {
  #ifdef FIXED_BOOT_IMAGE
  prebootmgr_blink_color(times, wait_ms, COLOR_BLUE);
  #else
  prebootmgr_blink_color(times, wait_ms, COLOR_GREEN);
  #endif
}

static void watchdog_recovery_sequence() {
  //force NWP to idle State
  HWREG(0x400F70B8) = 0x1;
  UtilsDelay(800000/5);

  //Clear the interrupt
  HWREG(0x400F70B0) = 0x1;
  UtilsDelay(800000/5);

  //reset NWP, WLAN domains
  HWREG(0x4402E16C) |= 0x2;
  UtilsDelay(800);

  //Wnsure ANA DCDC is moved to PFM mode before envoking hibernate
  HWREG(0x4402F024) &= 0xF7FFFFFF;
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
  MAP_PRCMPeripheralClkEnable(PRCM_GPIOA0, PRCM_RUN_MODE_CLK); //Clock for GPIOA0 (Ear Buttons / SD Power / Power)
  MAP_PRCMPeripheralClkEnable(PRCM_GPIOA2, PRCM_RUN_MODE_CLK); //Clock for GPIOAA (Charger)
  MAP_PRCMPeripheralClkEnable(PRCM_GPIOA3, PRCM_RUN_MODE_CLK); //Clock for GPIOA3 (Green/Blue LED)

  //MAP_PRCMPeripheralClkEnable(PRCM_WDT, PRCM_RUN_MODE_CLK);
  
  //MAP_PRCMPeripheralClkEnable(PRCM_I2CA0, PRCM_RUN_MODE_CLK);
  //MAP_PRCMPeripheralClkEnable(PRCM_GSPI, PRCM_RUN_MODE_CLK);

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

  //Charger
  MAP_PinTypeGPIO(CHARGER_PIN_NUM, PIN_MODE_0, false);
  MAP_GPIODirModeSet(CHARGER_PORT, CHARGER_PORT_MASK, GPIO_DIR_MODE_IN);

  //Power other peripherals
  MAP_PinTypeGPIO(POWER_PIN_NUM, PIN_MODE_0, false);
  MAP_GPIODirModeSet(POWER_PORT, POWER_PORT_MASK, GPIO_DIR_MODE_OUT);
  MAP_GPIOPinWrite(POWER_PORT, POWER_PORT_MASK, POWER_PORT_MASK);

  //Init SHAMD5
  MAP_PRCMPeripheralClkEnable(PRCM_DTHE, PRCM_RUN_MODE_CLK | PRCM_SLP_MODE_CLK);
  MAP_PRCMPeripheralReset(PRCM_DTHE);

  Logger_init();

  SdInit();
}

static void SimpleLinkInit() {
  if (slInitialized)
    return;

  watchdog_recovery_sequence();
  sl_Start(NULL, NULL, NULL);
  slInitialized = true;
}
static void SimpleLinkDeinit() {
  if (!slInitialized)
    return;

  sl_Stop(30);
  slInitialized = false; 
}


static void BoardDeinitCustom(void)
{
  Logger_debug("Prepare board deinitialization...");
  SimpleLinkDeinit();
  watchdog_feed();
  //Power off SD
  MAP_GPIOPinWrite(POWER_SD_PORT, POWER_SD_PORT_MASK, POWER_SD_PORT_MASK); //SIC! 
  //Power off other peripherals
  MAP_GPIOPinWrite(POWER_PORT, POWER_PORT_MASK, 0x00);

  PRCMHibernateWakeupSourceDisable(PRCM_HIB_GPIO17);//Disable charger Wakeup
}

static volatile bool EarSmallPressed(void) {
  return !(EAR_SMALL_PORT_MASK & MAP_GPIOPinRead(EAR_SMALL_PORT, EAR_SMALL_PORT_MASK));
}
static volatile bool EarBigPressed(void) {
  return !(EAR_BIG_PORT_MASK & MAP_GPIOPinRead(EAR_BIG_PORT, EAR_BIG_PORT_MASK));
}

static char* GetFlashPathById(uint8_t number) {
  char id = (char)((number%3) + 0x31);
  flashPath[11] = id;
  return flashPath;
}
static char* GetImagePathById(uint8_t number) {
  char id = (char)((number%3) + 0x31); //See Ascii Table - 1 starts at 0x31
  char* name;

  if (number < 3) {
    name = IMG_OFW_NAME;
  } else if (number < 6) {
    name = IMG_CFW_NAME;
  } else /*if (number < 9)*/ {
    name = IMG_ADD_NAME;
  }

  for (uint8_t i=0; i<3; i++)
  {
    imagePath[IMG_SD_PATH_REPL1_POS+i] = name[i];
  }
  imagePath[IMG_SD_PATH_REPL2_POS] = id;

  return imagePath;
}
static bool SdImageExists(uint8_t number) {
  char* image = GetImagePathById(number);
  return SdFileExists(image);
}
static bool ReadBootInfo() {
  _i32 fhandle;
  SlFsFileInfo_t pFsFileInfo;
  char bootNfo[8];
  SimpleLinkInit();

  char* bootInfoPath = OFW_BOOTINFO_PATH;

  if (bootInfoData.firmware != FW_SLOT_INVALID && bootInfoData.state != FW_STATE_INVALID)
    return true;

  if (!sl_FsOpen(bootInfoPath, FS_MODE_OPEN_READ, NULL, &fhandle)) {
    if (!sl_FsGetInfo(bootInfoPath, 0, &pFsFileInfo)) {
      if (pFsFileInfo.FileLen == sl_FsRead(fhandle, 0, bootNfo, 8)) {
        sl_FsClose(fhandle, 0, 0, 0);
        enum BOOTINFO_PARSE_RESULT result = Bootinfo_Parse(bootNfo, &bootInfoData);
        if (result == BOOTINFO_RESULT_OK) {
          Logger_info("Read bootinfo firmware=%i, state=0x%x", bootInfoData.firmware, bootInfoData.state);
          return true;
        } else {
          Logger_error("Could not parse bootinfo error=%i", result);
        }
      } else {
        Logger_error("Could not read 8 bytes from bootinfo");
      }
    } else {
      Logger_error("Could not open bootinfo metadata");
    }
  } else {
    Logger_error("Could not open bootinfo from flash:%s", &OFW_BOOTINFO_PATH);
  }
  return false;
}

static bool CheckSdImages() {
  bool hasValidImage = false;
  for (uint8_t i=0; i<IMG_MAX_COUNT; i++) {
    if (Config_imageInfos[i].ofwSimBL) {
      Config_imageInfos[i].fileExists = ReadBootInfo();
    } else {
      Config_imageInfos[i].fileExists = SdImageExists(i);
    }
    hasValidImage |= Config_imageInfos[i].fileExists;
  }
  return hasValidImage;
}

static bool isChargerConnected() {
  return (CHARGER_PORT_MASK & MAP_GPIOPinRead(CHARGER_PORT, CHARGER_PORT_MASK));
}
static uint16_t getBatteryLevel()
{
  uint16_t channel = ADC_CH_3;
  uint16_t uiIndex = 0;
  uint32_t ulSample;

  MAP_PinTypeADC(BATTERY_LEVEL_PIN_NUM, PIN_MODE_255);

	while(MAP_ADCFIFOLvlGet(ADC_BASE, channel))
    MAP_ADCFIFORead(ADC_BASE, channel); // flush the channel's FIFO if not empty

	MAP_ADCTimerConfig(ADC_BASE,2^17); // Configure ADC timer which is used to timestamp the ADC data samples
	MAP_ADCTimerEnable(ADC_BASE); // Enable ADC timer which is used to timestamp the ADC data samples
	MAP_ADCEnable(ADC_BASE); // Enable ADC module
	MAP_ADCChannelEnable(ADC_BASE, channel); // Enable ADC channel

  while(!MAP_ADCFIFOLvlGet(ADC_BASE, channel)) { }
  ulSample = MAP_ADCFIFORead(ADC_BASE, channel);

  MAP_ADCDisable(ADC_BASE);
  MAP_ADCChannelDisable(ADC_BASE, channel);
  MAP_ADCTimerDisable(ADC_BASE);

	return (ulSample >> 2 ) & 0x0FFF;	
}

static void hibernate() {
  Logger_info("Prepare hibernation");
  watchdog_stop();
  BoardDeinitCustom();
  
  PRCMHibernateWakeupSourceEnable(PRCM_HIB_GPIO2 | PRCM_HIB_GPIO4); //enable ear wakeup interrupt
  PRCMHibernateEnter();
}

static void checkBattery() {
  #ifndef FIXED_BOOT_IMAGE
  if (isChargerConnected())
    return;
  
  uint16_t batteryLevel = getBatteryLevel();
  if (batteryLevel < Config_generalSettings.minBatteryLevel) {
    Logger_error("Battery level is %i, treshold %i", batteryLevel, Config_generalSettings.minBatteryLevel);
    prebootmgr_blink_error(2, 66);
    prebootmgr_blink_error(2, 133);
    prebootmgr_blink_error(2, 66);

    hibernate();
  }
  #endif
}

static uint8_t Selector(uint8_t startNumber) {
  int8_t counter = startNumber;

  while (!Config_imageInfos[counter].fileExists)
  {
    counter = (counter+1) % COUNT_OF(Config_imageInfos);
  }

  LedSet(COLOR_GREEN);
  while (EarSmallPressed()) {
    UtilsDelayMsWD(10); //Wait while pressed
    watchdog_feed();
  }  
  
  uint8_t colors[] = { COLOR_BLACK, COLOR_BLUE, COLOR_GREEN, COLOR_CYAN };
  uint8_t curColorId = 0;
  if (Config_generalSettings.waitForPress) {
    LedSet(COLOR_BLUE);
    while (EarSmallPressed() || EarBigPressed()) {
      UtilsDelayMsWD(10); //Wait while pressed
      watchdog_feed();
    } 
  }

  //uint32_t millisStart = millis();
  uint32_t millisState = 0;
  uint32_t secondsDelta = 0;
  while (Config_generalSettings.waitForPress)
  {
    LedSet(colors[curColorId]);
    UtilsDelayMsWD(250);
    
    if (curColorId<COUNT_OF(colors)-1) {
      curColorId++;
    } else {
      curColorId = 0;
    }

    if (EarBigPressed()) {
      LedSet(COLOR_BLACK);
      UtilsDelayMsWD(250);
      break;
    }
      
    //TODO WORKAROUND
    //secondsDelta = (millis() - millisStart) / 1000;
    millisState += 250;
    secondsDelta = millisState / 1000;
    if (secondsDelta > Config_generalSettings.waitTimeoutInS)
      hibernate();
    
    watchdog_feed();
    checkBattery();
  } 

  LedSet(COLOR_BLACK);
  while (EarBigPressed()) {
    if (EarSmallPressed()) {
        do
        {
          counter = (counter+1) % COUNT_OF(Config_imageInfos);
        } while (!Config_imageInfos[counter].fileExists);
        while (EarSmallPressed()) {
            UtilsDelayMsWD(10); //Wait while pressed
        }
    }
    if (counter < 3) {
      prebootmgr_blink_color((counter+1)-0, 100, COLOR_GREEN);
    } else if (counter < 6) {
      prebootmgr_blink_color((counter+1)-3, 100, COLOR_BLUE);
    } else /*if (counter < 9)*/ {
      prebootmgr_blink_color((counter+1)-6, 100, COLOR_CYAN);
    }
    UtilsDelayMsWD(500);
    watchdog_feed();
  }
  return counter;
}
static bool prepareRun(sImageInfo* imageInfo, char* imagePath, uint32_t filesize) {
  FIL ffile;
  uint8_t ffs_result;

  #ifndef FIXED_BOOT_IMAGE
  if (imageInfo->checkHash) {
    char hashExp[65];
  #endif
    char hashAct[65];
    uint8_t hashActRaw[32];
  #ifndef FIXED_BOOT_IMAGE

    hashExp[64] = '\0';
    hashAct[64] = '\0';

    if (imageInfo->hashFile) {
      Logger_debug("Read hash from sha-file");
      
      char* shaFile = HASH_SD_PATH;
      memcpy(shaFile+IMG_SD_PATH_REPL1_POS, imagePath+IMG_SD_PATH_REPL1_POS, 4);
      Logger_debug("Open sd:%s ...", shaFile);
      ffs_result = f_open(&ffile, shaFile, FA_READ);
      if (ffs_result == FR_OK) {
        ffs_result = f_read(&ffile, hashExp, 64, NULL);
        if (ffs_result == FR_OK) {

        } else {
          Logger_error("Read sd:%s failed", shaFile);
        }
      } else {
        Logger_error("Open sd:%s failed", shaFile);
      }
    } else {
      Logger_debug("Read hash from the end of the image");
      filesize -= 64; //sha256 is 64 bytes long.
      memcpy(hashExp, (char*)(pImgRun + filesize), 64);
    }
  #endif
    MAP_SHAMD5ConfigSet(SHAMD5_BASE, SHAMD5_ALGO_SHA256);
    MAP_SHAMD5DataProcess(SHAMD5_BASE, pImgRun, filesize, hashActRaw);
    btox(hashAct, hashActRaw, 64);
  #ifndef FIXED_BOOT_IMAGE
    if (strncmp(hashAct, hashExp, 64) != 0) {
      Logger_error("SHA256 differs, source=%s", imageInfo->hashFile?"sha-file":"firmware");
      Logger_error(" hashAct=%s ", hashAct);
      Logger_error(" hashExo=%s", hashExp);

      prebootmgr_blink_error(10, 50);
      
      Config_generalSettings.waitForPress = true;
      return false;
    } else {
  #endif
      Logger_info("SHA256 hash=%s ", hashAct);
  #ifndef FIXED_BOOT_IMAGE
    }
  }
  if (imageInfo->ofwFix) {
    uint32_t* pCheck1 = (uint32_t*)(pImgRun+filesize-0x04);
    uint32_t* pCheck2 = (uint32_t*)(pImgRun+filesize-0x04-0x6c);
    uint32_t* pTarget = (uint32_t*)(pImgRun+filesize-0x04-0x04);

    char* gitHash = pImgRun+filesize-0x60; //len 7
    char* creationDate = pImgRun+filesize-0x58; //len 12/28

    char* version1 = NULL;
    char* version2 = NULL;

    char* strFinder = ((char*)pCheck2) - 1;
    if (strFinder[0] == 0x00) {
      for (uint8_t i=0; i<32; i++) {
        strFinder--;
        if (strFinder[0] == 0x00) {
          version2 = strFinder+1;
          break;
        }
      }
      if (version2 != NULL) {
        for (uint8_t i=0; i<32; i++) {
          strFinder--;
          if (strFinder[0] < 0x20 //ASCII Space
              || strFinder[0] > 0x7E) { //ASCII ~
            version1 = strFinder+1;
            break;
          }
        }
      }
    }

    Logger_info("Found following OFW metadata:");
    if (version1 != NULL)
      Logger_info(" version1=%s", version1);
    if (version2 != NULL)
      Logger_info(" version2=%s", version2);
    if (strnlen(gitHash, 8) == 7)
      Logger_info(" git hash=%s", gitHash);
    if (strnlen(creationDate, 13) == 12 //latest format
        || strnlen(creationDate, 29) == 28) //old format
      Logger_info(" creationDate=%s", creationDate);

    Logger_debug("Apply OFW fix");
    if (*pCheck1 == 0xBEAC0005 && *pCheck1 == *pCheck2) {
      *pTarget = 0x0010014C;
    } else {
      Logger_error("OFW fix failed");
      Logger_error(" *pCheck1=0x%X *pCheck2=0x%X *pTarget=0x%X", *pCheck1, *pCheck2, *pTarget);
      Logger_error(" pCheck1=0x%X pCheck2=0x%X pTarget=0x%X", pCheck1, pCheck2, pTarget);
    }
  }

  for (uint8_t i=0; i<PATCH_MAX_PER_IMAGE; i++) {
    if (imageInfo->patches[i][0] == '\0')
      break;
    Patch_Apply(pImgRun, imageInfo->patches[i], filesize);
  }
  #endif

  checkBattery();
  BoardDeinitCustom();

  #ifdef FIXED_BOOT_IMAGE
    watchdog_start_slow();
  #else
  if (!imageInfo->watchdog) {
    watchdog_stop();
  } else {
    watchdog_start_slow();
  }
  #endif
  watchdog_feed();

  if (imageInfo->ofwSimBL) {
    Logger_info("Start firmware flash:%s ...", GetFlashPathById(bootInfoData.firmware));
  } else {
    Logger_info("Start firmware sd:%s ...", imagePath);
  }

  Run((unsigned long)pImgRun);
}

int main()
{
  FIL ffile;
  _i32 fhandle;
  SlFsFileInfo_t pFsFileInfo;
  uint8_t ffs_result;

  BoardInitBase();
  BoardInitCustom();
  watchdog_start();

  #ifndef FIXED_BOOT_IMAGE
  Logger_info("Bootloader initialized");
  #else
  Logger_info("Preloader initialized");
  #endif

  if (PRCM_WDT_RESET == MAP_PRCMSysResetCauseGet()) {
    Logger_warn("Wakeup from WDT_Reset");
    watchdog_recovery_sequence();

    prebootmgr_blink_error(5, 33);
    prebootmgr_blink_error(5, 66);
    prebootmgr_blink_error(5, 33);

    hibernate();
  }

  #ifndef FIXED_BOOT_IMAGE
  Config_InitImageInfos();
  #endif

  UtilsDelayMsWD(100);
  Logger_debug("Mounting sd card...");
  ffs_result = f_mount(&fatfs, "0", 1);
  if (ffs_result == FR_OK) {
    #ifdef FIXED_BOOT_IMAGE
    char* image = IMG_SD_BOOTLOADER_PATH;
    if (SdFileExists(image)) {
    #else
    Config_ReadJsonCfg();
    if (CheckSdImages()) {
      checkBattery();

      retrySelection:
        Config_generalSettings.activeImage = Selector(Config_generalSettings.activeImage);
        
      uint8_t selectedImgNum = Config_generalSettings.activeImage;
      char* image = GetImagePathById(selectedImgNum);

      if (Config_imageInfos[selectedImgNum].ofwSimBL) {
        char* flashImage = GetFlashPathById(bootInfoData.firmware);
        
        Logger_debug("Open flash:%s ...", flashImage);
        _i32 flash_res = sl_FsOpen(flashImage, FS_MODE_OPEN_READ, NULL, &fhandle);
        if (!flash_res) {
          flash_res = sl_FsGetInfo(flashImage, 0, &pFsFileInfo);
          if (!flash_res) {
            if (pFsFileInfo.FileLen == sl_FsRead(fhandle, 0, (unsigned char *)APP_IMG_SRAM_OFFSET, pFsFileInfo.FileLen)) {
                sl_FsClose(fhandle, 0, 0, 0);
                Logger_debug("Read %i bytes", pFsFileInfo.FileLen);

                if (!prepareRun(&Config_imageInfos[selectedImgNum], image, pFsFileInfo.FileLen))
                  goto retrySelection;
            } else {
              //TODO
              Logger_error("Reading flash file failed");
            }
          } else {
            //TODO
            Logger_error("Reading flash file meta failed error=%i", flash_res);
          }
        } else {
          //TODO
          Logger_error("Opening flash file failed error=%i", flash_res);
        }
      } else {
      #endif
        Logger_debug("Open sd:%s ...", image);
        ffs_result = f_open(&ffile, image, FA_READ);
        if (ffs_result == FR_OK) {
            uint32_t filesize = f_size(&ffile);
            Logger_debug("Read sd:%s ...", image);
            ffs_result = f_read(&ffile, pImgRun, filesize, &filesize);
            if (ffs_result == FR_OK) {
              f_close(&ffile);
              Logger_debug("Read %i bytes", filesize);
              #ifdef FIXED_BOOT_IMAGE
                prepareRun(NULL, image, filesize);
              #else
                if (!prepareRun(&Config_imageInfos[selectedImgNum], image, filesize))
                  goto retrySelection;
              #endif
            } else {
                Logger_error("Reading file failed error=%i", ffs_result);
                UtilsDelayMsWD(1000);
                prebootmgr_blink_error(4, 500);
                UtilsDelayMsWD(2000);
                prebootmgr_blink_error(ffs_result, 1000);
            }
        } else {
            Logger_error("Opening file failed error=%i", ffs_result);
            UtilsDelayMsWD(1000);
            prebootmgr_blink_error(3, 500);
            UtilsDelayMsWD(2000);
            prebootmgr_blink_error(ffs_result, 1000);
        }
      #ifndef FIXED_BOOT_IMAGE
      }
      #endif
    } else {
      UtilsDelayMsWD(2000);
      Logger_error("No image found");
    }
  } else {
    Logger_error("Mounting sd failed error=%i", ffs_result);
    UtilsDelayMsWD(500);
    prebootmgr_blink_error(2, 500);
    UtilsDelayMsWD(500);
    prebootmgr_blink_error(ffs_result, 1000);
  }

  SimpleLinkInit();
  if (!sl_FsOpen(IMG_FLASH_PATH, FS_MODE_OPEN_READ, NULL, &fhandle)) {
      if (!sl_FsGetInfo(IMG_FLASH_PATH, 0, &pFsFileInfo)) {
          if (pFsFileInfo.FileLen == sl_FsRead(fhandle, 0, (unsigned char *)APP_IMG_SRAM_OFFSET, pFsFileInfo.FileLen)) {
              sl_FsClose(fhandle, 0, 0, 0);
              BoardDeinitCustom();
              Logger_info("Start fallback firmware flash:%s ...", IMG_FLASH_PATH);
              Run(APP_IMG_SRAM_OFFSET);
          }
      }
  }
  
  prebootmgr_blink_error(3, 33);
  prebootmgr_blink_error(3, 66);
  prebootmgr_blink_error(3, 33);
  
  hibernate();
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