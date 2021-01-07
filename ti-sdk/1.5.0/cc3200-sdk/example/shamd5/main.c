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
// Application Name     - SHA-MD5
// Application Overview - The application is a reference to usage of DES
//                        DriverLib functions on CC3200. Developer/User can 
//                        refer to this simple application and re-use the 
//                        functions in their applications
//
//*****************************************************************************

//*****************************************************************************
//
//! \addtogroup shamd5_api
//! @{
//
//*****************************************************************************

// Standard includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// Driverlib includes
#include "hw_shamd5.h"
#include "hw_memmap.h"
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_common_reg.h"
#include "rom.h"
#include "rom_map.h"
#include "shamd5.h"
#include "interrupt.h"
#include "prcm.h"
#include "uart.h"
#include "utils.h"

// Common interface includes
#include "uart_if.h"

#include "pinmux.h"
#include "shamd5_vector.h"
#include "shamd5_userinput.h"


#define USER_INPUT
#define APPLICATION_VERSION  "1.4.0"
#define APP_NAME             "SHAMD5 Reference"
#define UART_PRINT           Report
#define FOREVER              1

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
unsigned int uiHMAC;

// Flags to check that interrupts were successfully generated.
volatile bool g_bContextReadyFlag;
volatile bool g_bParthashReadyFlag;
volatile bool g_bInputReadyFlag;
volatile bool g_bOutputReadyFlag;

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
// Function Prototypes
//*****************************************************************************
void SHAMD5IntHandler(void);
void GenerateHash(unsigned int uiConfig,unsigned char *puiKey1,unsigned char 
                *puiData,unsigned char *puiResult,unsigned int uiDataLength);
unsigned char * LoadDefaultValues(unsigned int ui32Config,unsigned int *uiConfig,
                        unsigned int *uiHashLength,unsigned char **puiKey1,
                        unsigned int *uiDataLength,unsigned char **puiResult);

//*****************************************************************************
//
//! SHAMD5IntHandler - Interrupt Handler which handles different interrupts from 
//! different sources
//!
//! \param None
//!
//! \return None
//
//*****************************************************************************
void
SHAMD5IntHandler(void)
{
    uint32_t ui32IntStatus;
    //
    // Read the SHA/MD5 masked interrupt status.
    //
    ui32IntStatus = MAP_SHAMD5IntStatus(SHAMD5_BASE, true);
    if(ui32IntStatus & SHAMD5_INT_CONTEXT_READY)
    {
        MAP_SHAMD5IntDisable(SHAMD5_BASE, SHAMD5_INT_CONTEXT_READY);
        g_bContextReadyFlag = true;
    
    }
    if(ui32IntStatus & SHAMD5_INT_PARTHASH_READY)
    {
        MAP_SHAMD5IntDisable(SHAMD5_BASE, SHAMD5_INT_PARTHASH_READY);
        g_bParthashReadyFlag=true;
    
    }
    if(ui32IntStatus & SHAMD5_INT_INPUT_READY)
    {
        MAP_SHAMD5IntDisable(SHAMD5_BASE, SHAMD5_INT_INPUT_READY);
        g_bInputReadyFlag = true;
    
    }
    if(ui32IntStatus & SHAMD5_INT_OUTPUT_READY)
    {
        MAP_SHAMD5IntDisable(SHAMD5_BASE, SHAMD5_INT_OUTPUT_READY);
        g_bOutputReadyFlag = true;
    
    }
    
}

//*****************************************************************************
//
//! GenerateHash - Generates the Hash value of the Plain Text
//!
//! \param uiConfig - Configuration Value
//! \param puiKey1 - Key Used 
//! \param puiData - Plain Text used
//! \param puiResult - Hash Value Generated
//! \param uiDataLength - DataLength Used
//!
//! \return None
//
//*****************************************************************************

void
GenerateHash(unsigned int uiConfig,unsigned char *puiKey1,unsigned char *puiData,
        unsigned char *puiResult,unsigned int uiDataLength)
{
  
    //
    // Step1: Enable Interrupts
    // Step2: Wait for Context Ready Inteerupt
    // Step3: Set the Configuration Parameters (Hash Algorithm)
    // Step4: Set Key depends on Algorithm
    // Step5: Start Hash Generation
    //
    
    //
    // Clear the flags
    //
    g_bContextReadyFlag = false;
    g_bInputReadyFlag = false;
    //
    // Enable interrupts.
    //
    MAP_SHAMD5IntEnable(SHAMD5_BASE, SHAMD5_INT_CONTEXT_READY |
                    SHAMD5_INT_PARTHASH_READY |
                    SHAMD5_INT_INPUT_READY |
                    SHAMD5_INT_OUTPUT_READY);
    //
    // Wait for the context ready flag.
    //
    while(!g_bContextReadyFlag)
    {
    }
    //
    // Configure the SHA/MD5 module.
    //
    MAP_SHAMD5ConfigSet(SHAMD5_BASE, uiConfig); 
    //
    // If Keyed Hashing is used, Set Key and start Hash Generation
    //
    if(uiHMAC)
    {
        MAP_SHAMD5HMACKeySet(SHAMD5_BASE, puiKey1);
        MAP_SHAMD5HMACProcess(SHAMD5_BASE, 
                          puiData, 
                          uiDataLength, 
                          puiResult);

    }
    else
    {
    //
    // Perform the hashing operation
    //
    MAP_SHAMD5DataProcess(SHAMD5_BASE, puiData, uiDataLength,
                          puiResult);
    
    }
    
}
//*****************************************************************************
//
//! LoadDefaultValues - Populate the values from pre-defined vectors
//!
//! \param uiConfig - Configuration Value
//! \param uiHashLength - Hash Value Length
//! \param puiKey1 - Key Used 
//! \param puiResult - Hash Value Generated
//! \param uiDataLength - DataLength Used
//!
//! \return Pointer to Plain Text
//
//*****************************************************************************
unsigned char *
LoadDefaultValues(unsigned int ui32Config,unsigned int *uiConfig,unsigned int 
                *uiHashLength, unsigned char **puiKey1,unsigned int 
                *uiDataLength,unsigned char **puiResult)
{
    unsigned char *uiData;
    //
    // Populate the values from pre-defined Vectors
    //
    *uiConfig=ui32Config;
    //
    // Read Key
    //
    *puiKey1=g_psHMACShaMD5TestVectors.puiHMACKey;
    //
    // Read Hash Length
    //
    *uiHashLength=g_psHMACShaMD5TestVectors.uiHashLength;
    //
    // Read Data Length and allocate Data and Result accordingly
    //
    *uiDataLength=g_psHMACShaMD5TestVectors.uiDataLength;
    *puiResult=(unsigned char*)malloc(64);
    memset(*puiResult,0,64);
    uiData=(unsigned char*)malloc(*uiDataLength);
    memset(uiData,0,*uiDataLength);
    memcpy(uiData,g_psHMACShaMD5TestVectors.puiPlainText,*uiDataLength);
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
    Report("\t\t      CC3200 %s Application       \n\r", AppName);
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
//! main - populate the parameters from predefines Test Vector or User
//!
//! \param None
//!
//! \return None
//
//*****************************************************************************
void
main()
{
    unsigned int uiConfig,uiHashLength,uiDataLength;
    unsigned char *puiKey1,*puiData,*puiResult;
    unsigned int u8count;
#ifndef USER_INPUT
    unsigned char *puiTempExpResult;
#endif
    //
    // Initialize board configurations
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
    // Enable the module .
    //
    MAP_PRCMPeripheralClkEnable(PRCM_DTHE, PRCM_RUN_MODE_CLK);
    //
    // Enable interrupts.
    //
    MAP_SHAMD5IntRegister(SHAMD5_BASE, SHAMD5IntHandler);
    
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
        puiData=ReadFromUser(&uiConfig,&uiHashLength,&puiKey1,&uiDataLength,
                                &puiResult);
        if(puiData==NULL)
        {
            continue;
        }
#else
        //
        // Load Default values
        //
        UART_PRINT("Running Keyed Hashing HMAC_MD5\n\r\n\r");
        UART_PRINT("loading default values\n\r\n\r");
        uiHMAC=1;
        puiData= LoadDefaultValues(SHAMD5_ALGO_HMAC_MD5,&uiConfig,&uiHashLength,
                                        &puiKey1,&uiDataLength,&puiResult);
        UART_PRINT("Data Length (in Bytes) %d\n\r\n\r",uiDataLength);
#endif

        //
        // Generate Hash Value
        //
        UART_PRINT("\n\rHashing in Progress... \n\r");
        GenerateHash(uiConfig,puiKey1,puiData,puiResult,uiDataLength);
        UART_PRINT("Hash Value is generated\n\r");
        //
        // Display/Verify Result
        //

#ifdef USER_INPUT
        //
        // Display Hash Value Generated
        //
        UART_PRINT("\n\r The Hash Value in Hex is: 0x");
        for(u8count=0;u8count<uiHashLength;u8count++)
        {
          UART_PRINT("%02x",*(puiResult+u8count));
        }
        UART_PRINT("\n\r");
    } //end while(FOREVER)
#else
        //
        // Comapre Hash Generated and expected values from predefined vector
        //
        UART_PRINT("Hash Length (in Bytes) %d\n\r\n\r",uiHashLength);
        UART_PRINT("\n\r Computed Hash Value in Hex is: ");
        for(u8count=0;u8count<uiHashLength;u8count++)
        {
          UART_PRINT("%02x",*(puiResult+u8count));
        }
        UART_PRINT("\n\r");

        puiTempExpResult = (unsigned char *)g_psHMACShaMD5TestVectors.puiExpectedHash;

        UART_PRINT("\n\r Expected Hash Value in Hex is: ");
        for(u8count=0;u8count<uiHashLength;u8count++)
        {
          UART_PRINT("%02x",*(puiTempExpResult+u8count));
        }
        UART_PRINT("\n\r");

        if(memcmp(puiResult,g_psHMACShaMD5TestVectors.puiExpectedHash,uiHashLength)==0)
        {
          UART_PRINT("\n\r Hashing verified successfully");
        }
        else
        {
            UART_PRINT("\n\r Error in Hashing computation");
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


