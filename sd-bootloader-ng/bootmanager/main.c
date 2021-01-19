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
//#include "sdhost.h"

#include "ff.h"
#include "diskio.h"

static FATFS fatfs;


#define IMG_OFW_ID_1 0
#define IMG_OFW_ID_2 1
#define IMG_OFW_ID_3 2
#define IMG_CFW_ID_1 3
#define IMG_CFW_ID_2 4
#define IMG_CFW_ID_3 5
#define IMG_ADD_ID_1 6
#define IMG_ADD_ID_2 7
#define IMG_ADD_ID_3 8

#define IMG_FLASH_PATH "/sys/pre-img.bin"
#define IMG_SD_PATH "/revvox/boot/ng-CCCN.bin"
#define IMG_SD_PATH_REPL1_LEN 16
#define IMG_SD_PATH_REPL2_LEN 19
#define IMG_OFW_NAME "ofw"
#define IMG_CFW_NAME "cfw"
#define IMG_ADD_NAME "add"

#define IMG_MAX_COUNT 9
typedef struct sImageInfo
{
  bool fileExists;
  bool checkHash;
} sImageInfo;
static sImageInfo aImageInfo[IMG_MAX_COUNT];

static char* GetImagePathById(uint8_t number) {
  char* imagePath;
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
    imagePath[IMG_SD_PATH_REPL1_LEN+i] = name[i];
  }
  imagePath[IMG_SD_PATH_REPL2_LEN] = id;

  return imagePath;
}

//*****************************************************************************
// Local Variables
//*****************************************************************************
static long lFileHandle;
static int iRetVal;
static SlFsFileInfo_t pFsFileInfo;

static unsigned long ulFactoryImgToken;

static unsigned long ulUserImg1Token;
static unsigned long ulUserImg2Token;
static unsigned long ulBootInfoToken;
static unsigned long ulBootInfoCreateFlag;

//*****************************************************************************
// Vector Table
extern void (*const g_pfnVectors[])(void);

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

//*****************************************************************************
//
//! Load the application from sFlash and execute
//!
//! \param ImgName is the name of the application image on sFlash
//! \param ulToken is the token for reading file (relevant on secure devices only)
//!
//! This function loads the specified application from sFlash and executes it.
//!
//! \return None.
//
//*****************************************************************************
void LoadAndExecute(unsigned char *ImgName, unsigned long ulToken)
{

  //
  // Open the file for reading
  //
  iRetVal = sl_FsOpen(ImgName, FS_MODE_OPEN_READ,
                      &ulToken, &lFileHandle);
  //
  // Check if successfully opened
  //
  if (0 == iRetVal)
  {
    //
    // Get the file size using File Info structure
    //
    iRetVal = sl_FsGetInfo(ImgName, ulToken, &pFsFileInfo);

    //
    // Check for failure
    //
    if (0 == iRetVal)
    {

      //
      // Read the application into SRAM
      //
      iRetVal = sl_FsRead(lFileHandle, 0, (unsigned char *)APP_IMG_SRAM_OFFSET,
                          pFsFileInfo.FileLen);

      //
      // Stop the network services
      //
      sl_Stop(30);

      //
      // Execute the application.
      //
      Run(APP_IMG_SRAM_OFFSET);
    }
  }
}

//*****************************************************************************
//
//! Writes into the boot info file.
//!
//! \param psBootInfo is pointer to the boot info structure.
//!
//! This function writes the boot info into the boot info file in the sFlash
//!
//! \return Return 0 on success, -1 otherwise.
//
//*****************************************************************************
static long BootInfoWrite(sBootInfo_t *psBootInfo)
{
  long lFileHandle;
  unsigned long ulToken;

  //
  // Open the boot info file for write
  //
  if (0 == sl_FsOpen((unsigned char *)IMG_BOOT_INFO, FS_MODE_OPEN_WRITE,
                     &ulToken, &lFileHandle))
  {
    //
    // Write the boot info
    //
    if (0 < sl_FsWrite(lFileHandle, 0, (unsigned char *)psBootInfo,
                       sizeof(sBootInfo_t)))
    {

      //
      // Close the file
      //
      sl_FsClose(lFileHandle, 0, 0, 0);

      //
      // Return success
      //
      return 0;
    }
  }

  //
  // Return failure
  //
  return -1;
}

//*****************************************************************************
//
//! Load the proper image based on information from boot info and executes it.
//!
//! \param psBootInfo is pointer to the boot info structure.
//!
//! This function loads the proper image based on information from boot info
//! and executes it. \e psBootInfo should be properly initialized.
//!
//! \return None.
//
//*****************************************************************************
static void ImageLoader(sBootInfo_t *psBootInfo)
{
  unsigned char ucActiveImg;
  unsigned long ulImgStatus;

  //
  // Get the active image and image status
  //
  ucActiveImg = psBootInfo->ucActiveImg;
  ulImgStatus = psBootInfo->ulImgStatus;

  //
  // Boot image based on image status and active image configuration
  //
  if (IMG_STATUS_NOTEST == ulImgStatus)
  {

    //
    // Since no test image boot the acive image.
    //
    switch (ucActiveImg)
    {

    case IMG_ACT_USER1:
      LoadAndExecute((unsigned char *)IMG_USER_1, ulUserImg1Token);
      break;

    case IMG_ACT_USER2:
      LoadAndExecute((unsigned char *)IMG_USER_2, ulUserImg2Token);
      break;

#ifndef FAST_BOOT
    default:
      LoadAndExecute((unsigned char *)IMG_FACTORY_DEFAULT, ulFactoryImgToken);
      break;
#endif
    }
  }
  else if (IMG_STATUS_TESTREADY == ulImgStatus)
  {
    //
    // Some image waiting to be tested; Change the status to testing
    // in boot info file
    //
    psBootInfo->ulImgStatus = IMG_STATUS_TESTING;

#ifndef FAST_BOOT
    BootInfoWrite(psBootInfo);
#else
    BootInfoWrite(psBootInfo, false);
#endif

    //
    // Boot the test image ( the non-active image )
    //
    switch (ucActiveImg)
    {

    case IMG_ACT_USER1:
      LoadAndExecute((unsigned char *)IMG_USER_2, ulUserImg2Token);
      break;

    default:
      LoadAndExecute((unsigned char *)IMG_USER_1, ulUserImg1Token);
    }
  }
  else if (IMG_STATUS_TESTING == ulImgStatus)
  {

    //
    // Something went wrong while in testing.
    // Change the status to no test
    //
    psBootInfo->ulImgStatus = IMG_STATUS_NOTEST;

#ifndef FAST_BOOT
    BootInfoWrite(psBootInfo);
#else
    BootInfoWrite(psBootInfo, false);
#endif

    //
    // Boot the active image.
    //
    switch (ucActiveImg)
    {

    case IMG_ACT_USER1:
      LoadAndExecute((unsigned char *)IMG_USER_1, ulUserImg1Token);
      break;

    case IMG_ACT_USER2:
      LoadAndExecute((unsigned char *)IMG_USER_2, ulUserImg2Token);
      break;

#ifndef FAST_BOOT
    default:
      LoadAndExecute((unsigned char *)IMG_FACTORY_DEFAULT, ulFactoryImgToken);
      break;
#endif
    }
  }

  //
  // Boot info might be corrupted go into infinite loop
  //
  while (1)
  {
  }
}


//*****************************************************************************
//
//!\internal
//!
//! Creates default boot info structure
//!
//! \param psBootInfo is pointer to boot info structure to be initialized
//!
//! This function initializes the boot info structure \e psBootInfo based on
//! application image(s) found on sFlash storage. The default boot image is set
//! to one of Factory, User1 or User2 image in the same priority order.
//!
//! \retunr Returns 0 on success, -1 otherwise.
//
//*****************************************************************************
static int CreateDefaultBootInfo(sBootInfo_t *psBootInfo)
{

  //
  // Set the status to no test
  //
  psBootInfo->ulImgStatus = IMG_STATUS_NOTEST;

  //
  // Check if factor default image exists
  //
  iRetVal = sl_FsGetInfo((unsigned char *)IMG_FACTORY_DEFAULT, 0, &pFsFileInfo);
  if (iRetVal == 0)
  {
    psBootInfo->ucActiveImg = IMG_ACT_FACTORY;
    return 0;
  }

  iRetVal = sl_FsGetInfo((unsigned char *)IMG_USER_1, 0, &pFsFileInfo);
  if (iRetVal == 0)
  {
    psBootInfo->ucActiveImg = IMG_ACT_USER1;
    return 0;
  }

  iRetVal = sl_FsGetInfo((unsigned char *)IMG_USER_2, 0, &pFsFileInfo);
  if (iRetVal == 0)
  {
    psBootInfo->ucActiveImg = IMG_ACT_USER2;
    return 0;
  }

  return -1;
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
        
      /*
        switch (psBootInfo->ActiveImg) {
            case IMG_ACT_UPDATE1:
                psBootInfo->ActiveImg = IMG_ACT_UPDATE2;
                break;                
            case IMG_ACT_UPDATE2:
                psBootInfo->ActiveImg = IMG_ACT_UPDATE3;
                break;            
            case IMG_ACT_UPDATE3:
                psBootInfo->ActiveImg = IMG_ACT_FACTORY;
                break;
            default:
                psBootInfo->ActiveImg = IMG_ACT_UPDATE1;
                break;
            }*/
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
  FIL ffile;
  if (f_open(&ffile, image, FA_READ) == FR_OK) {
      f_close(&ffile); 
      return true;
  }
  return false;
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

  //
  // Board Initialization
  //
  BoardInitBase();
  BoardInitCustom();

  UtilsDelay(UTILS_DELAY_US_TO_COUNT(100 * 1000));
  uint8_t ffs_result = f_mount(&fatfs, "0", 1);
  if (ffs_result == FR_OK)
  {
    if (CheckSdImages()) {
      uint8_t imageNumber = Selector(0);
      char* image = GetImagePathById(imageNumber);

      FIL ffile;
      uint8_t ffs_result;

      ffs_result = f_open(&ffile, image, FA_READ);
      if (ffs_result == FR_OK) {
          uint32_t filesize = f_size(&ffile);
          ffs_result = f_read(&ffile, (unsigned char *)APP_IMG_SRAM_OFFSET, filesize, &filesize);
          if (ffs_result == FR_OK) {
            f_close(&ffile); 
            BoardDeinitCustom();
            Run(APP_IMG_SRAM_OFFSET);
          } else {
              UtilsDelay(UTILS_DELAY_US_TO_COUNT(1000 * 1000));
              prebootmgr_blink(4, 500);
              UtilsDelay(UTILS_DELAY_US_TO_COUNT(2000 * 1000));
              prebootmgr_blink(ffs_result, 1000);
          }
      } else {
          UtilsDelay(UTILS_DELAY_US_TO_COUNT(1000 * 1000));
          prebootmgr_blink(3, 500);
          UtilsDelay(UTILS_DELAY_US_TO_COUNT(2000 * 1000));
          prebootmgr_blink(ffs_result, 1000);
      }
    } else {
      //TODO: No bootable files on sd found
    }
    UtilsDelay(UTILS_DELAY_US_TO_COUNT(2000 * 1000));
  }

  UtilsDelay(UTILS_DELAY_US_TO_COUNT(500 * 1000));
  prebootmgr_blink(2, 500);
  UtilsDelay(UTILS_DELAY_US_TO_COUNT(500 * 1000));
  prebootmgr_blink(ffs_result, 1000);

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
    prebootmgr_blink(3, 33);
    prebootmgr_blink(3, 66);
    prebootmgr_blink(3, 33);

    __asm volatile(" dsb \n"
                   " isb \n"
                   " wfi \n");
  }
  


  prebootmgr_blink_color(3, 33, COLOR_GREEN);
  prebootmgr_blink_color(3, 33, COLOR_BLUE);

  /*
  //
  // Initialize the DMA
  //
  UDMAInit();
  //
  // Default configuration
  //
  sBootInfo.ucActiveImg = IMG_ACT_FACTORY;
  sBootInfo.ulImgStatus = IMG_STATUS_NOTEST;

  //
  // Initialize boot info file create flag
  //
  ulBootInfoCreateFlag  = _FS_FILE_OPEN_FLAG_COMMIT|_FS_FILE_PUBLIC_WRITE;

  //
  // Start slhost to get NVMEM service
  //
  sl_Start(NULL, NULL, NULL);

    //
  // Open Boot info file for reading
  //
  iRetVal = sl_FsOpen((unsigned char *)IMG_BOOT_INFO,
                        FS_MODE_OPEN_READ,
                        &ulBootInfoToken,
                        &lFileHandle);
  
  
  //
  // If successful, load the boot info
  // else create a new file with default boot info.
  //
  if( 0 == iRetVal )
  {
    iRetVal = sl_FsRead(lFileHandle,0,
                         (unsigned char *)&sBootInfo,
                         sizeof(sBootInfo_t));

  }
  else
  {

    //
    // Create a new boot info file
    //
    iRetVal = sl_FsOpen((unsigned char *)IMG_BOOT_INFO,
                        FS_MODE_OPEN_CREATE(2*sizeof(sBootInfo_t),
                                            ulBootInfoCreateFlag),
                                            &ulBootInfoToken,
                                            &lFileHandle);

    //
    // Create a default boot info
    //
    iRetVal = CreateDefaultBootInfo(&sBootInfo);

    if(iRetVal != 0)
    {
      //
      // Can't boot no bootable image found
      //
      while(1)
      {

      }
    }

    //
    // Write the default boot info.
    //
    iRetVal = sl_FsWrite(lFileHandle,0,
                         (unsigned char *)&sBootInfo,
                         sizeof(sBootInfo_t));
  }

  //
  // Close boot info function
  //
  sl_FsClose(lFileHandle, 0, 0, 0);
    
  //
  // Load and execute the image base on boot info.
  //
  ImageLoader(&sBootInfo);
*/
  //
  // Infinite loop
  //
  while (1)
  {
  }
}