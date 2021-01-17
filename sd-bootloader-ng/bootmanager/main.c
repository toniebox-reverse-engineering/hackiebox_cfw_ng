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

#ifdef FAST_BOOT
#include "filesystem.h"
#endif



//*****************************************************************************
// Local Variables
//*****************************************************************************
static long lFileHandle;
static int  iRetVal;
static SlFsFileInfo_t pFsFileInfo;
#ifdef FAST_BOOT
static SlFsFileInfo2_t pFsFileInfo2;
#endif

#ifndef FAST_BOOT
static unsigned long ulFactoryImgToken;
#endif

static unsigned long ulUserImg1Token;
static unsigned long ulUserImg2Token;
static unsigned long ulBootInfoToken;
static unsigned long ulBootInfoCreateFlag;
#ifdef FAST_BOOT
static tBoolean bIsNwpStarted;
#endif



//*****************************************************************************
// Vector Table
extern void (* const g_pfnVectors[])(void);


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
BoardInit(void)
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
#ifndef ccs
void Run(unsigned long ulBaseLoc)
{

  //
  // Set the SP
  //
  __asm("	ldr    sp,[r0]\n"
	"	add    r0,r0,#4");

  //
  // Jump to entry code
  //
  __asm("	ldr    r1,[r0]\n"
        "	bx     r1");
}
#else
__asm("    .sect \".text:Run\"\n"
      "    .clink\n"
      "    .thumbfunc Run\n"
      "    .thumb\n"
      "Run:\n"
      "    ldr    sp,[r0]\n"
      "    add    r0,r0,#4\n"
      "    ldr    r1,[r0]\n"
      "    bx     r1");
#endif


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
#ifndef FAST_BOOT
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
  if( 0 == iRetVal )
  {
    //
    // Get the file size using File Info structure
    //
    iRetVal = sl_FsGetInfo(ImgName, ulToken,&pFsFileInfo);

    //
    // Check for failure
    //
    if( 0 == iRetVal )
    {

      //
      // Read the application into SRAM
      //
      iRetVal = sl_FsRead(lFileHandle,0, (unsigned char *)APP_IMG_SRAM_OFFSET,
                 pFsFileInfo.FileLen );

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
#else
void LoadAndExecute(unsigned char *ImgName, unsigned long ulToken)
{

  if( true == bIsNwpStarted )
  {
    //
    // Open the file for reading
    //
    iRetVal = sl_FsOpen(ImgName, FS_MODE_OPEN_READ,
                          &ulToken, &lFileHandle);
    //
    // Check if successfully opened
    //
    if( 0 == iRetVal )
    {
      //
      // Get the file size using File Info structure
      //
      iRetVal = sl_FsGetInfo(ImgName, ulToken,&pFsFileInfo);

      //
      // Check for failure
      //
      if( 0 == iRetVal )
      {
        //
        // Read the application into SRAM
        //
        iRetVal = sl_FsRead(lFileHandle,0, (unsigned char *)APP_IMG_SRAM_OFFSET,
                   pFsFileInfo.FileLen );

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
  else
  {
    //
    // Open the file for reading
    //
    lFileHandle = fs_Open(1,ImgName,
                    _FS_MODE_OPEN_READ,&ulToken); 
    //
    // Check if successfully opened
    //
    if((lFileHandle > 0) )
    {
      //
      // Get the file size using File Info structure
      //
      iRetVal = fs_GetFileInfo(1,ImgName,&ulToken,&pFsFileInfo2);

      //
      // Check for failure
      //
      if( 0 == iRetVal )
      {

        //
        // Read the application into SRAM
        //
        iRetVal = fs_Read(lFileHandle, 0, (char *)APP_IMG_SRAM_OFFSET, 
                         pFsFileInfo2.Size);

        //
        // Execute the application.
        //
        Run(APP_IMG_SRAM_OFFSET);
      }
    }
  }
}
#endif

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
#ifndef FAST_BOOT
static long BootInfoWrite(sBootInfo_t *psBootInfo)
{
  long lFileHandle;
  unsigned long ulToken;


  
  //
  // Open the boot info file for write
  //
  if( 0 == sl_FsOpen((unsigned char *)IMG_BOOT_INFO, FS_MODE_OPEN_WRITE,
                      &ulToken, &lFileHandle) )
  {
    //
    // Write the boot info
    //
    if( 0 < sl_FsWrite(lFileHandle,0, (unsigned char *)psBootInfo,
                         sizeof(sBootInfo_t)) )
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
#else
static long BootInfoWrite(sBootInfo_t *psBootInfo, tBoolean bCreate)
{
  long lFileHandle;
  unsigned long ulToken;
  long lRet = -1;
  
  //
  // Start slhost to get NVMEM service
  //
  sl_Start(NULL, NULL, NULL);

  //
  // Mark NWP service as statred
  //
  bIsNwpStarted = true;

  if(bCreate)
  {
    //
    // Create a new boot info file
    //
    lRet = sl_FsOpen((unsigned char *)IMG_BOOT_INFO,
                        FS_MODE_OPEN_CREATE(2*sizeof(sBootInfo_t),
                                            ulBootInfoCreateFlag),
                                            &ulBootInfoToken,
                                            &lFileHandle);
  }
  else
  {
     lRet = sl_FsOpen((unsigned char *)IMG_BOOT_INFO, FS_MODE_OPEN_WRITE,
                      &ulToken, &lFileHandle);
  }
    
  //
  // Open the boot info file for write
  //
  if( 0 == lRet )
  {
    //
    // Write the boot info
    //
    if( 0 < sl_FsWrite(lFileHandle,0, (unsigned char *)psBootInfo,
                         sizeof(sBootInfo_t)) )
    {

    //
    // Close the file
    //
    sl_FsClose(lFileHandle, 0, 0, 0);

    //
    // Set success
    //
    lRet = 0;
   }

  }

  //
  // Return failure
  //
  return lRet;
}
#endif

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
  if( IMG_STATUS_NOTEST == ulImgStatus )
  {

    //
    // Since no test image boot the acive image.
    //
    switch(ucActiveImg)
    {

    case IMG_ACT_USER1:
      LoadAndExecute((unsigned char *)IMG_USER_1,ulUserImg1Token);
      break;

    case IMG_ACT_USER2:
      LoadAndExecute((unsigned char *)IMG_USER_2,ulUserImg2Token);
      break;
      
#ifndef FAST_BOOT
    default:
      LoadAndExecute((unsigned char *)IMG_FACTORY_DEFAULT,ulFactoryImgToken);
      break;
#endif
    }
  }
  else if( IMG_STATUS_TESTREADY == ulImgStatus )
  {
    //
    // Some image waiting to be tested; Change the status to testing
    // in boot info file
    //
    psBootInfo->ulImgStatus = IMG_STATUS_TESTING;
    
#ifndef FAST_BOOT
    BootInfoWrite(psBootInfo);
#else
    BootInfoWrite(psBootInfo,false);
#endif

    //
    // Boot the test image ( the non-active image )
    //
    switch(ucActiveImg)
    {

    case IMG_ACT_USER1:
      LoadAndExecute((unsigned char *)IMG_USER_2,ulUserImg2Token);
      break;

    default:
      LoadAndExecute((unsigned char *)IMG_USER_1,ulUserImg1Token);
    }
  }
  else if( IMG_STATUS_TESTING == ulImgStatus )
  {

    //
    // Something went wrong while in testing.
    // Change the status to no test
    //
    psBootInfo->ulImgStatus = IMG_STATUS_NOTEST;
    
#ifndef FAST_BOOT
    BootInfoWrite(psBootInfo);
#else
    BootInfoWrite(psBootInfo,false);
#endif

    //
    // Boot the active image.
    //
    switch(ucActiveImg)
    {

    case IMG_ACT_USER1:
      LoadAndExecute((unsigned char *)IMG_USER_1,ulUserImg1Token);
      break;

    case IMG_ACT_USER2:
      LoadAndExecute((unsigned char *)IMG_USER_2,ulUserImg2Token);
      break;
      
#ifndef FAST_BOOT
    default:
      LoadAndExecute((unsigned char *)IMG_FACTORY_DEFAULT,ulFactoryImgToken);
      break;
#endif
    }
  }

  //
  // Boot info might be corrupted go into infinite loop
  //
  while(1)
  {

  }

}


//*****************************************************************************
//
//! Checks if the device is secure
//!
//! This function checks if the device is a secure device or not.
//!
//! \return Returns \b true if device is secure, \b false otherwise
//
//*****************************************************************************
static inline tBoolean IsSecureMCU()
{
  unsigned long ulChipId;

  ulChipId =(HWREG(GPRCM_BASE + GPRCM_O_GPRCM_EFUSE_READ_REG2) >> 16) & 0x1F;

  if((ulChipId != DEVICE_IS_CC3101RS) &&(ulChipId != DEVICE_IS_CC3101S))
  {
    //
    // Return non-Secure
    //
    return false;
  }

  //
  // Return secure
  //
  return true;
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
#ifndef FAST_BOOT
static int CreateDefaultBootInfo(sBootInfo_t *psBootInfo)
{

    //
    // Set the status to no test
    //
    psBootInfo->ulImgStatus = IMG_STATUS_NOTEST;

    //
    // Check if factor default image exists
    //
    iRetVal = sl_FsGetInfo((unsigned char *)IMG_FACTORY_DEFAULT, 0,&pFsFileInfo);
    if(iRetVal == 0)
    {
      psBootInfo->ucActiveImg = IMG_ACT_FACTORY;
      return 0;
    }

    iRetVal = sl_FsGetInfo((unsigned char *)IMG_USER_1, 0,&pFsFileInfo);
    if(iRetVal == 0)
    {
      psBootInfo->ucActiveImg = IMG_ACT_USER1;
      return 0;
    }

    iRetVal = sl_FsGetInfo((unsigned char *)IMG_USER_2, 0,&pFsFileInfo);
    if(iRetVal == 0)
    {
      psBootInfo->ucActiveImg = IMG_ACT_USER2;
      return 0;
    }

    return -1;
}
#else
static int CreateDefaultBootInfo(sBootInfo_t *psBootInfo)
{

    //
    // Set the status to no test
    //
    psBootInfo->ulImgStatus = IMG_STATUS_NOTEST;


    iRetVal = fs_GetFileInfo(1, (unsigned char *)IMG_USER_1, 0, &pFsFileInfo2);
   
    if(iRetVal == 0)
    {
      psBootInfo->ucActiveImg = IMG_ACT_USER1;
      return 0;
    }
    
    iRetVal = fs_GetFileInfo(1, (unsigned char *)IMG_USER_2, 0, &pFsFileInfo2);
    
    
    if(iRetVal == 0)
    {
      psBootInfo->ucActiveImg = IMG_ACT_USER2;
      return 0;
    }

    return -1;
}
#endif

#define HAL_FCPU_MHZ                        80U
#define HAL_FCPU_HZ                         (1000000U * HAL_FCPU_MHZ)
#define HAL_SYSTICK_PERIOD_US               1000U
#define UTILS_DELAY_US_TO_COUNT(us)         (((us) * HAL_FCPU_MHZ) / 6)

#define TONIEBOX_GREEN_LED_PRCM                     PRCM_GPIOA3
#define TONIEBOX_BIG_EAR_PRCM                       PRCM_GPIOA0
#define TONIEBOX_SMALL_EAR_PRCM                     PRCM_GPIOA0
#define TONIEBOX_GREEN_LED_PORT                     GPIOA3_BASE
#define TONIEBOX_BIG_EAR_PORT                       GPIOA0_BASE
#define TONIEBOX_SMALL_EAR_PORT                     GPIOA0_BASE
#define TONIEBOX_SD_PORT                            GPIOA0_BASE

#define TONIEBOX_GREEN_LED_GPIO                     pin_GP25
#define TONIEBOX_GREEN_LED_PIN_NUM                  PIN_21      // GP25/SOP2
#define TONIEBOX_BIG_EAR_PIN_NUM                    PIN_57      // GP02
#define TONIEBOX_SMALL_EAR_PIN_NUM                  PIN_59      // GP04
#define TONIEBOX_GREEN_LED_PORT_PIN                 GPIO_PIN_1
#define TONIEBOX_BIG_EAR_PORT_PIN                   GPIO_PIN_2
#define TONIEBOX_SMALL_EAR_PORT_PIN                 GPIO_PIN_4
#define TONIEBOX_SD_PORT_PIN                        GPIO_PIN_3

static void prebootmgr_blink(int times, int wait_us);
static void prebootmgr_blink(int times, int wait_us) {
    for (int i=0; i<times; i++) {
        MAP_GPIOPinWrite(TONIEBOX_GREEN_LED_PORT, TONIEBOX_GREEN_LED_PORT_PIN, 0xFF);
        UtilsDelay(UTILS_DELAY_US_TO_COUNT(wait_us * 1000));
        MAP_GPIOPinWrite(TONIEBOX_GREEN_LED_PORT, TONIEBOX_GREEN_LED_PORT_PIN, 0);
        UtilsDelay(UTILS_DELAY_US_TO_COUNT(wait_us * 1000));
    }
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
  BoardInit();

  MAP_PRCMPeripheralClkEnable(TONIEBOX_GREEN_LED_PRCM, PRCM_RUN_MODE_CLK | PRCM_SLP_MODE_CLK);
  MAP_PinTypeGPIO(TONIEBOX_GREEN_LED_PIN_NUM, PIN_MODE_0, false);
  MAP_GPIODirModeSet(TONIEBOX_GREEN_LED_PORT, TONIEBOX_GREEN_LED_PORT_PIN, GPIO_DIR_MODE_OUT);

  while(1) {
    prebootmgr_blink(3, 500);
    prebootmgr_blink(3, 250);
  }

/*
  //
  // Initialize the DMA
  //
  UDMAInit();

#ifdef FAST_BOOT
  bIsNwpStarted = false;
#endif
  
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
  // Check if its a secure MCU
  //
  if ( IsSecureMCU() )
  {
#ifndef FAST_BOOT    
    ulFactoryImgToken     = FACTORY_IMG_TOKEN;
#endif
    ulUserImg1Token       = USER_IMG_1_TOKEN;
    ulUserImg2Token       = USER_IMG_2_TOKEN;
    ulBootInfoToken       = USER_BOOT_INFO_TOKEN;
    ulBootInfoCreateFlag  = _FS_FILE_OPEN_FLAG_COMMIT|_FS_FILE_OPEN_FLAG_SECURE|
                            _FS_FILE_OPEN_FLAG_NO_SIGNATURE_TEST|
                            _FS_FILE_PUBLIC_WRITE|_FS_FILE_OPEN_FLAG_VENDOR;
  }


#ifndef FAST_BOOT  
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
  
#else
  
  //
  // Open Boot info file for reading
  //
  lFileHandle = fs_Open(1, (unsigned char *)IMG_BOOT_INFO,
                    _FS_MODE_OPEN_READ, &ulBootInfoToken);
  
  //
  // If successful, load the boot info
  // else create a new file with default boot info.
  //
  if( lFileHandle > 0 )
  {
    
    iRetVal = fs_Read(lFileHandle, 0, (char *)&sBootInfo, sizeof(sBootInfo_t));
    
    //
    // Close boot info function
    //
    fs_Close(0,lFileHandle, NULL );

  }
  else
  {

    //
    // Create a default boot info
    //
    if( 0 == CreateDefaultBootInfo(&sBootInfo) )
    {
    
      //
      // Save the bootinfo
      //
      BootInfoWrite(&sBootInfo,true);
    }
    
  }
 
#endif
  
  //
  // Load and execute the image base on boot info.
  //
  ImageLoader(&sBootInfo);
*/
  //
  // Infinite loop
  //
  while(1)
  {

  }
}

