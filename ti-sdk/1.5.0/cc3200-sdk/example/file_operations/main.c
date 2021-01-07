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
// Application Name     -   File operations 
// Application Overview -   This example demonstate the file operations that 
//                          can be performed by the applications. The 
//                          application use the serial-flash as the storage 
//                          medium.
//
//*****************************************************************************


//*****************************************************************************
//
//! \addtogroup filesystem_demo
//! @{
//
//*****************************************************************************
#include <stdlib.h>
#include <string.h>

// Simplelink includes
#include "simplelink.h"

//Driverlib includes
#include "hw_types.h"
#include "hw_ints.h"
#include "rom.h"
#include "rom_map.h"
#include "interrupt.h"
#include "prcm.h"

//Common interface includes
#include "gpio_if.h"
#include "common.h"
#ifndef NOTERM
#include "uart_if.h"
#endif
#include "pinmux.h"


#define APPLICATION_NAME        "FILE OPERATIONS"
#define APPLICATION_VERSION     "1.4.0"

#define SL_MAX_FILE_SIZE        64L*1024L       /* 64KB file */
#define BUF_SIZE                2048
#define USER_FILE_NAME          "fs_demo.txt"

/* Application specific status/error codes */
typedef enum{
    // Choosing this number to avoid overlap w/ host-driver's error codes
    FILE_ALREADY_EXIST = -0x7D0,
    FILE_CLOSE_ERROR = FILE_ALREADY_EXIST - 1,
    FILE_NOT_MATCHED = FILE_CLOSE_ERROR - 1,
    FILE_OPEN_READ_FAILED = FILE_NOT_MATCHED - 1,
    FILE_OPEN_WRITE_FAILED = FILE_OPEN_READ_FAILED -1,
    FILE_READ_FAILED = FILE_OPEN_WRITE_FAILED - 1,
    FILE_WRITE_FAILED = FILE_READ_FAILED - 1,

    STATUS_CODE_MAX = -0xBB8
}e_AppStatusCodes;

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
unsigned char gaucCmpBuf[BUF_SIZE];
const unsigned char gaucOldMacDonald[] = "Old MacDonald had a farm,E-I-E-I-O, \
And on his farm he had a cow, \
E-I-E-I-O, \
With a moo-moo here, \
And a moo-moo there, \
Here a moo, there a moo, \
Everywhere a moo-moo. \
Old MacDonald had a farm, \
E-I-E-I-O. \
Old MacDonald had a farm, \
E-I-E-I-O, \
And on his farm he had a pig, \
E-I-E-I-O, \
With an oink-oink here, \
And an oink-oink there, \
Here an oink, there an oink, \
Everywhere an oink-oink. \
Old MacDonald had a farm, \
E-I-E-I-O. \
Old MacDonald had a farm, \
E-I-E-I-O, \
And on his farm he had a duck, \
E-I-E-I-O, \
With a quack-quack here, \
And a quack-quack there, \
Here a quack, there a quack, \
Everywhere a quack-quack. \
Old MacDonald had a farm, \
E-I-E-I-O. \
Old MacDonald had a farm, \
E-I-E-I-O, \
And on his farm he had a horse, \
E-I-E-I-O, \
With a neigh-neigh here, \
And a neigh-neigh there, \
Here a neigh, there a neigh, \
Everywhere a neigh-neigh. \
Old MacDonald had a farm, \
E-I-E-I-O. \
Old MacDonald had a farm, \
E-I-E-I-O, \
And on his farm he had a donkey, \
E-I-E-I-O, \
With a hee-haw here, \
And a hee-haw there, \
Here a hee, there a hee, \
Everywhere a hee-haw. \
Old MacDonald had a farm, \
E-I-E-I-O. \
Old MacDonald had a farm, \
E-I-E-I-O, \
And on his farm he had some chickens, \
E-I-E-I-O, \
With a cluck-cluck here, \
And a cluck-cluck there, \
Here a cluck, there a cluck, \
Everywhere a cluck-cluck. \
Old MacDonald had a farm, \
E-I-E-I-O.";

#if defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif

//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************


//*****************************************************************************
// SimpleLink Asynchronous Event Handlers -- Start
//*****************************************************************************

//*****************************************************************************
//
//! This function gets triggered when HTTP Server receives Application
//! defined GET and POST HTTP Tokens.
//!
//! \param pHttpServerEvent Pointer indicating http server event
//! \param pHttpServerResponse Pointer indicating http server response
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkHttpServerCallback(SlHttpServerEvent_t *pSlHttpServerEvent, 
                                  SlHttpServerResponse_t *pSlHttpServerResponse)
{

}

//*****************************************************************************
//
//!  \brief This function handles WLAN events
//!
//!  \param[in] pSlWlanEvent is the event passed to the handler
//!
//!  \return None
//
//*****************************************************************************
void SimpleLinkWlanEventHandler(SlWlanEvent_t *pSlWlanEvent)
{

}

//*****************************************************************************
//
//!  \brief This function handles events for IP address 
//!           acquisition via DHCP indication
//!
//!  \param[in] pNetAppEvent is the event passed to the handler
//!
//!  \return None
//
//*****************************************************************************
void SimpleLinkNetAppEventHandler(SlNetAppEvent_t *pNetAppEvent)
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
// SimpleLink Asynchronous Event Handlers -- End
//*****************************************************************************

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
    // In case of TI-RTOS vector table is initialize by OS itself
#ifndef USE_TIRTOS
    //
    // Set vector table base
    //
#if defined(ccs)
    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
#endif
#if defined(ewarm)
    MAP_IntVTableBaseSet((unsigned long)&__vector_table);
#endif
#endif
    //
    // Enable Processor
    //
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}

//*****************************************************************************
//
//!  This funtion includes the following steps:
//!  -open a user file for writing
//!  -write "Old MacDonalds" child song 37 times to get just below a 64KB file
//!  -close the user file
//!
//!  /param[out] ulToken : file token
//!  /param[out] lFileHandle : file handle
//!
//!  /return  0:Success, -ve: failure
//
//*****************************************************************************
long WriteFileToDevice(unsigned long *ulToken, long *lFileHandle)
{
    long lRetVal = -1;
    int iLoopCnt = 0;

    //
    //  create a user file
    //
    lRetVal = sl_FsOpen((unsigned char *)USER_FILE_NAME,
                FS_MODE_OPEN_CREATE(65536, \
                          _FS_FILE_OPEN_FLAG_COMMIT|_FS_FILE_PUBLIC_WRITE),
                        ulToken,
                        lFileHandle);
    if(lRetVal < 0)
    {
        //
        // File may already be created
        //
        lRetVal = sl_FsClose(*lFileHandle, 0, 0, 0);
        ASSERT_ON_ERROR(lRetVal);
    }
    else
    {
        //
        // close the user file
        //
        lRetVal = sl_FsClose(*lFileHandle, 0, 0, 0);
        if (SL_RET_CODE_OK != lRetVal)
        {
            ASSERT_ON_ERROR(FILE_CLOSE_ERROR);
        }
    }
    
    //
    //  open a user file for writing
    //
    lRetVal = sl_FsOpen((unsigned char *)USER_FILE_NAME,
                        FS_MODE_OPEN_WRITE, 
                        ulToken,
                        lFileHandle);
    if(lRetVal < 0)
    {
        lRetVal = sl_FsClose(*lFileHandle, 0, 0, 0);
        ASSERT_ON_ERROR(FILE_OPEN_WRITE_FAILED);
    }
    
    //
    // write "Old MacDonalds" child song as many times to get just below a 64KB file
    //
    for (iLoopCnt = 0; 
            iLoopCnt < (SL_MAX_FILE_SIZE / sizeof(gaucOldMacDonald)); 
            iLoopCnt++)
    {
        lRetVal = sl_FsWrite(*lFileHandle,
                    (unsigned int)(iLoopCnt * sizeof(gaucOldMacDonald)), 
                    (unsigned char *)gaucOldMacDonald, sizeof(gaucOldMacDonald));
        if (lRetVal < 0)
        {
            lRetVal = sl_FsClose(*lFileHandle, 0, 0, 0);
            ASSERT_ON_ERROR(FILE_WRITE_FAILED);
        }
    }
    
    //
    // close the user file
    //
    lRetVal = sl_FsClose(*lFileHandle, 0, 0, 0);
    if (SL_RET_CODE_OK != lRetVal)
    {
        ASSERT_ON_ERROR(FILE_CLOSE_ERROR);
    }

    return SUCCESS;
}

//*****************************************************************************
//
//!  This funtion includes the following steps:
//!    -open the user file for reading
//!    -read the data and compare with the stored buffer
//!    -close the user file
//!
//!  /param[in] ulToken : file token
//!  /param[in] lFileHandle : file handle
//!
//!  /return 0: success, -ve:failure
//
//*****************************************************************************
long ReadFileFromDevice(unsigned long ulToken, long lFileHandle)
{
    long lRetVal = -1;
    int iLoopCnt = 0;

    //
    // open a user file for reading
    //
    lRetVal = sl_FsOpen((unsigned char *)USER_FILE_NAME,
                        FS_MODE_OPEN_READ,
                        &ulToken,
                        &lFileHandle);
    if(lRetVal < 0)
    {
        lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
        ASSERT_ON_ERROR(FILE_OPEN_READ_FAILED);
    }

    //
    // read the data and compare with the stored buffer
    //
    for (iLoopCnt = 0;
            iLoopCnt < (SL_MAX_FILE_SIZE / sizeof(gaucOldMacDonald));
            iLoopCnt++)
    {
        lRetVal = sl_FsRead(lFileHandle,
                    (unsigned int)(iLoopCnt * sizeof(gaucOldMacDonald)),
                     gaucCmpBuf, sizeof(gaucOldMacDonald));
        if ((lRetVal < 0) || (lRetVal != sizeof(gaucOldMacDonald)))
        {
            lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
            ASSERT_ON_ERROR(FILE_READ_FAILED);
        }

        lRetVal = memcmp(gaucOldMacDonald,
                         gaucCmpBuf,
                         sizeof(gaucOldMacDonald));
        if (lRetVal != 0)
        {
            ASSERT_ON_ERROR(FILE_NOT_MATCHED);
        }
    }

    //
    // close the user file
    //
    lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
    if (SL_RET_CODE_OK != lRetVal)
    {
        ASSERT_ON_ERROR(FILE_CLOSE_ERROR);
    }

    return SUCCESS;
}

//*****************************************************************************
//
//! \brief  the aim of this example code is to demonstrate File-system
//!          capabilities of the device.
//!         For simplicity, the serial flash is used as the device under test.
//!
//! \param  None
//!
//! \return none
//!
//! \note   Green LED is turned solid in case of success
//!         Red LED is turned solid in case of failure
//
//*****************************************************************************
void main()
{
    long lRetVal;
    unsigned char policyVal;
    long lFileHandle;
    unsigned long ulToken;

    //
    // Initialize Board configurations
    //
    BoardInit();

    //
    // Configure the pinmux settings for the peripherals exercised
    //
    PinMuxConfig();

    //
    // Configure LEDs
    //
    GPIO_IF_LedConfigure(LED1|LED3);

    GPIO_IF_LedOff(MCU_RED_LED_GPIO);
    GPIO_IF_LedOff(MCU_GREEN_LED_GPIO);
	
#ifndef NOTERM
	InitTerm();
#endif

    //
    // Initializing the CC3200 networking layers
    //
    lRetVal = sl_Start(NULL, NULL, NULL);
    if(lRetVal < 0)
    {
        GPIO_IF_LedOn(MCU_RED_LED_GPIO);
        LOOP_FOREVER();
    }

    //
    // reset all network policies
    //
    lRetVal = sl_WlanPolicySet(  SL_POLICY_CONNECTION,
                    SL_CONNECTION_POLICY(0,0,0,0,0),
                    &policyVal,
                    1 /*PolicyValLen*/);
    if(lRetVal < 0)
    {
        GPIO_IF_LedOn(MCU_RED_LED_GPIO);
        LOOP_FOREVER();
    }
    
    if(WriteFileToDevice(&ulToken, &lFileHandle) < 0)
    {
        GPIO_IF_LedOn(MCU_RED_LED_GPIO);
        LOOP_FOREVER();
    }

    if(ReadFileFromDevice(ulToken, lFileHandle) < 0)
    {
        GPIO_IF_LedOn(MCU_RED_LED_GPIO);
         LOOP_FOREVER();
    }

    //
    // turn ON the green LED indicating success
    //
    GPIO_IF_LedOn(MCU_GREEN_LED_GPIO);
    lRetVal = sl_Stop(SL_STOP_TIMEOUT);

       LOOP_FOREVER();

}
//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
