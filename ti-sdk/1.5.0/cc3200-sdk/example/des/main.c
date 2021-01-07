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
// Application Name     - DES
// Application Overview - The application is a reference to usage of DES 
//                        DriverLib functions on CC3200. Developer/User can 
//                        refer to this simple application and re-use the 
//                        functions in their applications.
//
//*****************************************************************************

//*****************************************************************************
//
//! \addtogroup des_api
//! @{
//
//*****************************************************************************

// Standard includes
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

// Driverlib includes
#include "hw_des.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "hw_types.h"
#include "utils.h"
#include "rom.h"
#include "rom_map.h"
#include "des.h"
#include "interrupt.h"
#include "prcm.h"
#include "udma.h"
#include "uart.h"

// Common interface includes
#include "uart_if.h"

#include "des_vector.h"
#include "des_userinput.h"
#include "pinmux.h"


//
// Undefine USER_INPUT to use default values
//
#define USER_INPUT
#define APPLICATION_VERSION "1.4.0"
#define UART_PRINT          Report
#define FOREVER             1
#define APP_NAME            "DES Reference"

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************

//
// Interrupt Flags
//
volatile static bool g_bContextInIntFlag;
static volatile bool g_bDataInIntFlag;
static volatile bool g_bDataOutIntFlag;

#if defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif

//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************


/****************************************************************************/
/*                      LOCAL FUNCTION PROTOTYPES                           */
/****************************************************************************/
void DESIntHandler(void);
void DESCrypt(unsigned int uiConfig,unsigned char *puiKey1,unsigned char *puiData,
        unsigned char *puiResult,unsigned int uiDataLength,unsigned char *uiIV);
unsigned char * LoadDefaultValues(unsigned int ui32Config,unsigned int *uiConfig,
                        unsigned char **uiIV,unsigned char **puiKey1,unsigned int 
                                        *uiDataLength,unsigned char **puiResult);
static void BoardInit(void);
/****************************************************************************/

//*****************************************************************************
//
//! DESIntHandler - DES Interrupt Handler
//!
//! \param None
//!
//! The function handles different interrupts based on the Interrupt Resource
//!
//! \return Returns None
//
//*****************************************************************************
void
DESIntHandler(void)
{
    uint32_t ui32IntStatus;

    //
    // Read the DES masked interrupt status.
    //
    ui32IntStatus = MAP_DESIntStatus(DES_BASE, true);

    //
    // set flags depending on the interrupt source.
    //
    if(ui32IntStatus & DES_INT_CONTEXT_IN)
    {
        MAP_DESIntDisable(DES_BASE, DES_INT_CONTEXT_IN);
        g_bContextInIntFlag = true;
        
    }
    if(ui32IntStatus & DES_INT_DATA_IN)
    {
        MAP_DESIntDisable(DES_BASE, DES_INT_DATA_IN);
        g_bDataInIntFlag = true;
    }
    if(ui32IntStatus & DES_INT_DATA_OUT)
    {
        MAP_DESIntDisable(DES_BASE, DES_INT_DATA_OUT);
        g_bDataOutIntFlag = true;
        
    }
    

}

//*****************************************************************************
//
//! LoadDefaultValues - Populate the values from predefined vectors
//!
//! \param uiConfig- Configuration Value
//! \param uiIV - Initialization Vector
//! \param puiKey1 - Key Value
//! \param uiDataLength - Data Length Used
//! \param puiResult - Result Value
//!
//!
//! \return Pointer to the Input Data
//
//*****************************************************************************
unsigned char *
LoadDefaultValues(unsigned int ui32Config,unsigned int *uiConfig,unsigned char **uiIV,
    unsigned char **puiKey1,unsigned int *uiDataLength,unsigned char **puiResult)
{
     unsigned char *uiData;
     
     *uiConfig=ui32Config;
     
     //
     // Read the Key
     //
     *puiKey1=psDESTestVectors.pui32Key;
     
     //
     // Read the Initialization vector
     //
     *uiIV=&psDESTestVectors.pui32IV[0];
     
     //
     // Read DataLength and allocate Result and Data Variables accordingly
     //
     *uiDataLength=psDESTestVectors.ui32DataLength;
     *puiResult=(unsigned char*)malloc(*uiDataLength);
     if(*puiResult != NULL)
     {
         memset(*puiResult,0,*uiDataLength);
     }
     else
     {
         //Failed to allocate memory
         UART_PRINT("Failed to allocate memory");
         return 0;
     }
     uiData=(unsigned char*)malloc(*uiDataLength);
     if(uiData != NULL)
     {
         memset(uiData,0,*uiDataLength);
     }
     else
     {
         //Failed to allocate memory
         UART_PRINT("Failed to allocate memory");
         return 0;
     }
     
     //
     // Copy Plain Text/ Cipher Text into the Data variables based on Encryption 
     // or Decryption
     //
     if(ui32Config & DES_CFG_DIR_ENCRYPT)
        memcpy(uiData,psDESTestVectors.pui32PlainText,*uiDataLength);
     else
       memcpy(uiData,psDESTestVectors.pui32CipherText,*uiDataLength);
     return uiData; 
      
}

//*****************************************************************************
//
//! DESCrypt - Des Crypt Function
//!
//! \param uiConfig - Configuration Value
//! \param puiKey1 - Key Used
//! \param puiData - Input Data
//! \param puiResult - Result Value
//! \param uiDataLength - DataLength used
//! \param uiIV - initialization Vector
//!
//! \return None
//
//*****************************************************************************

void
DESCrypt(unsigned int uiConfig,unsigned char *puiKey1,unsigned char *puiData,
        unsigned char *puiResult,unsigned int uiDataLength,unsigned char *uiIV)
{
    //
    // Step1:  Enable Interrupts
    // Step2:  Wait for Context Ready Inteerupt 
    // Step3:  Set the Configuration Parameters (Direction,AES Mode) 
    // Step4:  Set the Initialization Vector 
    // Step5:  Write Key 
    // Step6:  Start the Crypt Process 
    //
    
    //
    // Clear the flags.
    //
    g_bContextInIntFlag = false;
    g_bDataInIntFlag = false;
    g_bDataOutIntFlag = false;

    //
    // Enable all interrupts.
    //
    MAP_DESIntEnable(DES_BASE, DES_INT_CONTEXT_IN |
                 DES_INT_DATA_IN |
                 DES_INT_DATA_OUT);

    //
    // Wait for the context in flag.
    //
    while(!g_bContextInIntFlag)
    {
    }
    
    //
    // Configure the DES module.
    //
    MAP_DESConfigSet(DES_BASE, uiConfig);
    
    //
    // Set the key.
    //
    MAP_DESKeySet(DES_BASE, puiKey1);

    //
    // Write the initial value registers if needed.
    //
    if((uiConfig & DES_CFG_MODE_CBC) ||
        (uiConfig & DES_CFG_MODE_CFB))
    {
        MAP_DESIVSet(DES_BASE, uiIV);
    }

    MAP_DESDataProcess(DES_BASE, puiData, puiResult,uiDataLength); 
 

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
    Report("\t\t     CC3200 %s Application       \n\r", AppName);
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
//! Main - Which populates paramters either from predefined vectors or from user
//! and running the DES Crypt Function
//!
//! \param None
//!
//! \return None
//
//*****************************************************************************
void 
main()
{
  
    unsigned int uiConfig,uiDataLength;
    unsigned char *puiKey1,*puiData,*puiResult, *puiIV,
    uiIV[8]={0x6d, 0x8e, 0xca, 0xc4, 0x3b, 0x27, 0xc8, 0x85};
    
#ifdef USER_INPUT
    unsigned int uiKeySize;
    unsigned int ui32CharCount;
    unsigned char * pui8Result;
#endif

    puiIV=&uiIV[0];
    
    //
    // Initialize the board
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
    // Enable DES Module
    //
    MAP_PRCMPeripheralClkEnable(PRCM_DTHE, PRCM_RUN_MODE_CLK);
    
    //
    // Enable interrupts.
    //
    MAP_DESIntRegister(DES_BASE, DESIntHandler);

    //
    // Read values either from User or from Vector based on macro USER_INPUT 
    // defined or not
    //
#ifdef USER_INPUT
    while(FOREVER)
    {

        //
        // Read the values from the user over uart and Populate the variables
        //
        puiData=ReadFromUser(&uiConfig,&uiKeySize,&puiKey1,&uiDataLength,
                                &puiResult);
        if(puiData==NULL)
        {
            continue;
         }
#else
        UART_PRINT("Running DES-ECB  Vectors\n\r");
        
        //
        // Load Default values
        //
        puiData=LoadDefaultValues(DES_CFG_DIR_ENCRYPT | DES_CFG_MODE_ECB |
          DES_CFG_SINGLE,&uiConfig,&puiIV,&puiKey1,&uiDataLength,&puiResult);
#endif

        //
        // Carry out Encryption
        //
        UART_PRINT("\n\r Encryption in progress....");
        DESCrypt(uiConfig,puiKey1,puiData,puiResult,uiDataLength,puiIV);
        UART_PRINT("\n\r Encryption done, cipher text created");
        
        //
        // Copy Result into Data Vector to continue with Decryption. and change 
        // config value
        //
        memcpy(puiData,puiResult,uiDataLength);
        uiConfig &= ~(1 << 2);
        
        //
        // Carry out Decrypt Function
        //
        UART_PRINT("\n\r\n\r Decryption in progress....");
        DESCrypt(uiConfig,puiKey1,puiData,puiResult,uiDataLength,uiIV);
        UART_PRINT("\n\r Decryption done");

#ifdef USER_INPUT
        //
        // Display Plain Text
        //
        UART_PRINT("\n\r Text after decryption :");
        pui8Result = (unsigned char *)puiResult;
        for(ui32CharCount=0;ui32CharCount<uiDataLength;ui32CharCount++)
        {
                UART_PRINT("%c",*(pui8Result+ui32CharCount));
        }
        UART_PRINT("\n\r");

    }
#else
    //
    // Comapre Cipher Text and Plain Text with the expected values from 
    // predefined vector
    //
    if(memcmp(puiData,psDESTestVectors.pui32CipherText,
                    psDESTestVectors.ui32DataLength)==0)
    {                
        UART_PRINT("\n\r\n\r Encryption verified");
    }
    if(memcmp(puiResult,psDESTestVectors.pui32PlainText,
                        psDESTestVectors.ui32DataLength)==0)
    {
        UART_PRINT("\n\r Decryption verification successful");
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
