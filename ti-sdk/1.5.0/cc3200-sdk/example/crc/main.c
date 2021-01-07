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
// Application Name     - CRC
// Application Overview - The application is a reference to usage of CRC 
//                        DriverLib functions on CC3200. Developer/User can 
//                        refer to this simple application and re-use the 
//                        functions in their applications. This application can 
//                        be used with or without "Uart Terminal".
//
//*****************************************************************************

//*****************************************************************************
//
//! \addtogroup crc_api
//! @{
//
//*****************************************************************************

// Standard includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

// Driverlib includes
#include "hw_dthe.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "hw_ints.h"
#include "hw_types.h"
#include "rom.h"
#include "rom_map.h"
#include "crc.h"
#include "uart.h"
#include "utils.h"
#include "interrupt.h"
#include "prcm.h"
#include "udma.h"

// Common interface includes
#include "uart_if.h"

#include "pinmux.h"
#include "crc_vector.h"
#include "crc_userinput.h"

#define USER_INPUT
#define APPLICATION_VERSION "1.4.0"
#define CCM0_BASE           DTHE_BASE
#define UART_PRINT          Report
#define FOREVER             1
#define APP_NAME            "CRC Reference"

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
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
//
//! Function - RunCRC
//!
//! \param uiConfig - Configuration Value
//! \param uiDataLength - DataLength Used
//! \param uiSeed - Seed Value
//! \param puiData - Plain Text Data used
//! \param puiResult - Result Value
//!
//! \return None
//
//*****************************************************************************
void
RunCRC(unsigned int uiConfig,unsigned int uiDataLength,unsigned int uiSeed,
            unsigned int *puiData,unsigned int *puiResult)
{
  //
  // Step1: Set the Configuration Parameters 
  // Step2: Write the seed value
  // Step3: Start CRC generation
  //

  //
  // Configure the CRC engine.
  //
  MAP_CRCConfigSet(CCM0_BASE, uiConfig);
  
  //
  // Write the seed.
  //
  MAP_CRCSeedSet(CCM0_BASE, uiSeed);
  *puiResult = MAP_CRCDataProcess(CCM0_BASE, (void*) puiData, 
                                        uiDataLength, uiConfig);
}

//*****************************************************************************
//
//! Function - LoadDefaultValues
//!
//! \param uiConfig - Configuration Value
//! \param uiDataLength - DataLength Used
//! \param uiSeed - Seed Value
//! \param puiResult - Result Value
//!
//! \return Pointer to the Data
//
//*****************************************************************************
unsigned int *
LoadDefaultValues(unsigned int ui32Config,unsigned int *uiConfig,unsigned int 
                    *uiDataLength, unsigned int *uiSeed,unsigned int *puiResult)
{
    unsigned int *uiData;

    *uiConfig=ui32Config;
    
    //
    // Read Input Data size
    //
    if(ui32Config & CRC_CFG_SIZE_8BIT)
    {
        *uiDataLength = 64;
    }
    else
    {
        *uiDataLength = 16;
    }
    //
    // Read Seed Value
    //
    *uiSeed=g_psCRC8005TestVectors.ui32Seed;
    *puiResult = 0;
    uiData=(unsigned int*)malloc(64);
    if(uiData != NULL)
    {
    memset(uiData,0,64);
    //
    // Read the Data
    //
    memcpy(uiData,g_psCRC8005TestVectors.ui32Data,64);
    }
    return uiData; 
   
}

//*****************************************************************************
//
//! Application startup display on UART
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
static void
DisplayBanner(char * AppName)
{
    Report("\n\n\n\r");
    Report("\t\t *************************************************\n\r");
    Report("\t\t       CC3200 %s Application       \n\r", AppName);
    Report("\t\t *************************************************\n\r");
    Report("\n\n\n\r");
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
/* In case of TI-RTOS vector table is initialize by OS itself */
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
//! Main 
//!
//! \param None
//!
//! \return None
//
//*****************************************************************************
void 
main()
{
    unsigned int uiConfig,uiSeed=0x0000a5a5,uiDataLength,*puiData,uiResult;
    
    //
    // Initialize Board configurations
    //
    BoardInit();
    
    //
    // Configuring UART for Receiving input and displaying output
    // 1. PinMux setting
    // 2. Initialize UART
    // 3. Displaying Banner
    //
    PinMuxConfig();
    InitTerm();
    DisplayBanner(APP_NAME);

    //
    // Enable CRC Module
    //
    MAP_PRCMPeripheralClkEnable(PRCM_DTHE, PRCM_RUN_MODE_CLK);
#ifdef USER_INPUT
    while(FOREVER)
    {
        //
        // Read values either from User or from Vector based on macro USER_INPUT
        // defined or not
        //

        //
        // Read the values from the user over uart and Populate the variables
        //
        puiData=ReadFromUser(&uiConfig,&uiDataLength,&uiResult);
        if(puiData == NULL)
        {
            UART_PRINT("\n\rInvalid Input. Please try again. \n\r");
            continue;
        }
#else
        //
        // Load Default values
        //
        UART_PRINT("Running CRC-16-IBM 0x8005 Vectors\n");
        puiData=LoadDefaultValues(CRC_CFG_INIT_SEED | CRC_CFG_TYPE_P8005 |
                                   CRC_CFG_SIZE_32BIT,
                                   &uiConfig,&uiDataLength,&uiSeed,&uiResult);
#endif

        //
        // Carry out Encryption
        //
        UART_PRINT("\n\r CRC Generation in progress....");
        RunCRC(uiConfig,uiDataLength,uiSeed,puiData,&uiResult);
        UART_PRINT("\n\r CRC Result is generated\n\r");
        
        //
        // Display/Verify Result
        //

#ifdef USER_INPUT
        //
        // Display Plain Text
        //
        UART_PRINT("\n\r The CRC Result in hex is: 0x%02x \n\r",uiResult);
        if(puiData)
        {
            free(puiData);
        }
    }
#else
    //
    // Comapre Cipher Text and Plain Text with the expected values from 
    // predefined vector
    //
    UART_PRINT("\n\r The Generated CRC Result in hex is: 0x%02x \n\r",uiResult);
    UART_PRINT("\n\r The Expected CRC in hex is: 0x%02x \n\r",
                                            g_psCRC8005TestVectors.ui32Result);
    if(uiResult==g_psCRC8005TestVectors.ui32Result)
    {
      UART_PRINT("\n\r\n\r CRC result is verified successfully");
    }
    else
    {
      UART_PRINT("\n\r\n\r Error in CRC generation");
    }

    while(FOREVER);

#endif

}
//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

