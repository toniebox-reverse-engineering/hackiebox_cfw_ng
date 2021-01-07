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
// Application Name     - AES Demo
// Application Overview - The application is a reference to usage of AES 
//                        DriverLib functions on CC3200. Developers can refer to 
//                        this simple application and re-use the functions in 
//                        their applications.
//
//*****************************************************************************

//*****************************************************************************
//
//! \addtogroup aes_demo
//! @{
//
//*****************************************************************************

// Standard includes
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

// Driverlib includes
#include "hw_aes.h"
#include "hw_memmap.h"
#include "hw_ints.h"
#include "hw_common_reg.h"
#include "hw_types.h"
#include "rom.h"
#include "rom_map.h"
#include "aes.h"
#include "interrupt.h"
#include "prcm.h"
#include "uart.h"
#include "utils.h"

// Common interface includes
#include "uart_if.h"

#include "aes_vector.h"
#include "aes_userinput.h"
#include "pinmux.h"

//
// Undefine USER_INPUT to use default values
//
#define USER_INPUT
#define APPLICATION_VERSION   "1.4.0"
#define UART_PRINT            Report
#define FOREVER               1
#define APP_NAME              "AES Reference"


//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
unsigned int pui32AES128Key[4] = {
    0x16157e2b, 0xa6d2ae28, 0x8815f7ab, 0x3c4fcf09
};

unsigned int pui32AES192Key[6] = {
    0xf7b0738e, 0x52640eda, 0x2bf310c8, 0xe5799080,
    0xd2eaf862, 0x7b6b2c52
};

unsigned int pui32AES256Key[8] = {
    0x10eb3d60, 0xbe71ca15, 0xf0ae732b, 0x81777d85,
    0x072c351f, 0xd708613b, 0xa310982d, 0xf4df1409
};

unsigned int pui32AESPlainText[16] = {
    0xe2bec16b, 0x969f402e, 0x117e3de9, 0x2a179373,
    0x578a2dae, 0x9cac031e, 0xac6fb79e, 0x518eaf45,
    0x461cc830, 0x11e45ca3, 0x19c1fbe5, 0xef520a1a,
    0x45249ff6, 0x179b4fdf, 0x7b412bad, 0x10376ce6
};

unsigned int pui32AESCipherText[16] = {
         0xb47bd73a, 0x60367a0d, 0xf3ca9ea8, 0x97ef6624,
         0x85d5d3f5, 0x9d69b903, 0x5a8985e7, 0xafbafd96,
         0x7fcdb143, 0x23ce8e59, 0xe3001b88, 0x880603ed,
         0x5e780c7b, 0x3fade827, 0x71202382, 0xd45d7204
};

sAESTestVector psAESCBCTestVectors =
{

        AES_CFG_KEY_SIZE_128BIT,
        pui32AES128Key,
        0,
        0,
        {0x03020100, 0x07060504, 0x0b0a0908, 0x0f0e0d0c},
        {0x00000000, 0x00000000, 0x00000000, 0x00000000},
        64,
        pui32AESPlainText,
        0,
        0,
        {0xacab4976, 0x46b21981, 0x9b8ee9ce, 0x7d19e912,
         0x9bcb8650, 0xee197250, 0x3a11db95, 0xb2787691,
         0xb8d6be73, 0x3b74c1e3, 0x9ee61671, 0x16952222,
         0xa1caf13f, 0x09ac1f68, 0x30ca0e12, 0xa7e18675}

};

static volatile bool g_bContextInIntFlag;
static volatile bool g_bDataInIntFlag;
static volatile bool g_bContextOutIntFlag;
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
void AESCrypt(unsigned int uiConfig,unsigned int uiKeySize,unsigned int *puiKey1,
                    unsigned int *puiData, unsigned int *puiResult,
                    unsigned int uiDataLength,unsigned int *uiIV);
void AESIntHandler(void);
unsigned int * LoadDefaultValues(unsigned int ui32Config,unsigned int *uiConfig,
                          unsigned int *uiKeySize, unsigned int **uiIV,
                          unsigned int **puiKey1,unsigned int *uiDataLength,
                          unsigned int **puiResult);
static void BoardInit(void);


//*****************************************************************************
//
//! AES Crypt Function
//!
//! This function Configures Key,Mode and carries out Encryption/Decryption in 
//!                                                  CPU mode
//! \param uiConfig - Configuration Value
//! \param uiKeySize - KeySize used.(128,192 or 256 bit)
//! \param puiKey1 - Key Used
//! \param puiData - Input Data
//! \param puiResult - Resultant Output Data
//! \param uiDataLength - Input Data Length
//! \param uiIV - Initialization Vector used
//!
//! \return none
//
//*****************************************************************************
void 
AESCrypt(unsigned int uiConfig,unsigned int uiKeySize,unsigned int *puiKey1,
            unsigned int *puiData,unsigned int *puiResult,
            unsigned int uiDataLength,unsigned int *uiIV)
{
    //
    // Step1:  Enable Interrupts
    // Step2:  Wait for Context Ready Inteerupt 
    // Step3:  Set the Configuration Parameters (Direction,AES Mode and Key Size) 
    // Step4:  Set the Initialization Vector 
    // Step5:  Write Key 
    // Step6:  Start the Crypt Process 
    //
    
    //
    // Clear the flags.
    //
    g_bContextInIntFlag = false;
    g_bDataInIntFlag = false;
    g_bContextOutIntFlag = false;
    g_bDataOutIntFlag = false;
    
    //
    // Enable all interrupts.
    //
    MAP_AESIntEnable(AES_BASE, AES_INT_CONTEXT_IN |
                         AES_INT_CONTEXT_OUT | AES_INT_DATA_IN |
                         AES_INT_DATA_OUT);
    
    //
    // Wait for the context in flag, the flag will be set in the Interrupt handler.
    //
    while(!g_bContextInIntFlag)
    {
    }
    
    //
    // Configure the AES module with direction (encryption or decryption) and 
    // the key size.
    //
    MAP_AESConfigSet(AES_BASE, uiConfig |uiKeySize);
    
    //
    // Write the initial value registers if needed, depends on the mode.
    //
    if(((uiConfig & AES_CFG_MODE_M) == AES_CFG_MODE_CBC) ||
               ((uiConfig & AES_CFG_MODE_M) == AES_CFG_MODE_CFB) ||
               ((uiConfig & AES_CFG_MODE_M) == AES_CFG_MODE_CTR) ||
               ((uiConfig & AES_CFG_MODE_M) == AES_CFG_MODE_ICM) 
               )
    {
    MAP_AESIVSet(AES_BASE, (unsigned char *)uiIV);
    }
    
    //
    // Write key1.
    //
    MAP_AESKey1Set(AES_BASE,(unsigned char *)puiKey1,uiKeySize);
    
    //
    // Start Crypt Process
    //
    MAP_AESDataProcess(AES_BASE, (unsigned char *)puiData, 
                        (unsigned char *)puiResult, uiDataLength);
}

//*****************************************************************************
//
//! AES Interrupt Handler
//!
//! This function
//!        1. Handles Interrupts based on Interrupt Sources. Set Flags
//! \param none
//!
//! \return none
//
//*****************************************************************************

void
AESIntHandler(void)
{
    uint32_t uiIntStatus;
    
    //
    // Read the AES masked interrupt status.
    //
    uiIntStatus = MAP_AESIntStatus(AES_BASE, true);
    
    //
    // Set Different flags depending on the interrupt source.
    //
    if(uiIntStatus & AES_INT_CONTEXT_IN)
    {
        MAP_AESIntDisable(AES_BASE, AES_INT_CONTEXT_IN);
        g_bContextInIntFlag = true;
    }
    if(uiIntStatus & AES_INT_DATA_IN)
    {
        MAP_AESIntDisable(AES_BASE, AES_INT_DATA_IN);
        g_bDataInIntFlag = true;
    }
    if(uiIntStatus & AES_INT_CONTEXT_OUT)
    {
        MAP_AESIntDisable(AES_BASE, AES_INT_CONTEXT_OUT);
        g_bContextOutIntFlag = true;
    }
    if(uiIntStatus & AES_INT_DATA_OUT)
    {
        MAP_AESIntDisable(AES_BASE, AES_INT_DATA_OUT);
        g_bDataOutIntFlag = true;
    }
    

}

//*****************************************************************************
//
//! Function - Read Predefined Values. Populate Key,PlainText,Mode etc from 
//!   pre-defined Vectors
//!
//! \param ui32Config - Configuration Value (Direction | Mode |KeySize)
//! \out uiConfig - Configuration value 
//! \out uiKeySize - Key Size used
//! \out uiIV - Initialization Vector
//! \out puiKey1 - Key Used
//! \out uiDataLength - DataLength Used
//! \out puiResult - Result
//!
//! \return Returns /\e true on success or \e false on failure.
//
//*****************************************************************************
unsigned int *
LoadDefaultValues(unsigned int ui32Config,unsigned int *uiConfig,
                    unsigned int *uiKeySize,
                    unsigned int **uiIV,unsigned int **puiKey1,
                    unsigned int *uiDataLength,unsigned int **puiResult)
{
    unsigned int *uiData;
    
    //
    // Populate all the out parameters from pre-defined vector
    //
    *uiConfig=ui32Config;
    
    //
    // Read Key and Key size
    //
    *puiKey1=psAESCBCTestVectors.pui32Key1;
    *uiKeySize=psAESCBCTestVectors.ui32KeySize;
    
    //
    // Read Initialization Vector
    //
    *uiIV=&psAESCBCTestVectors.pui32IV[0];
    
    //
    // Read Data Length and allocate Result and Data variables accordingly
    //
    *uiDataLength=psAESCBCTestVectors.ui32DataLength;
    *puiResult=(unsigned int*)malloc(*uiDataLength);
    if(*puiResult != NULL)
    {
        memset(*puiResult,0,*uiDataLength);
    }
    else
    {
        //Failed to allocate memory
        UART_PRINT("Failed to allocate memory");
        return NULL;
    }
    uiData=(unsigned int*)malloc(*uiDataLength);
    if(uiData != NULL)
    {
        memset(uiData,0,*uiDataLength);
    }
    else
    {
        //Failed to allocate memory
        UART_PRINT("Failed to allocate memory");
        return NULL;
    }
    //
    // Copy Plain Text or Cipher Text into the variable Data
    //
    if(ui32Config & AES_CFG_DIR_ENCRYPT)
        memcpy(uiData,psAESCBCTestVectors.pui32PlainText,*uiDataLength);
    else
         memcpy(uiData,psAESCBCTestVectors.pui32CipherText,*uiDataLength);
    
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
//! main - calls Crypt function after populating either from pre- defined vector 
//! or from User
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
void 
main()
{
    unsigned int uiConfig,uiKeySize,*puiKey1,*puiData,*puiResult=NULL,
    uiDataLength,uiIV[4]={0x03020100, 0x07060504, 0x0b0a0908, 0x0f0e0d0c};

#ifdef USER_INPUT
    unsigned int uiCharCount;
    unsigned char* pucResult;
#else
    unsigned int *puiIV;
    puiIV=&uiIV[0];

#endif
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
    // Enable AES Module
    //
    MAP_PRCMPeripheralClkEnable(PRCM_DTHE, PRCM_RUN_MODE_CLK);
    
    //
    // Enable AES interrupts.
    //
    MAP_AESIntRegister(AES_BASE, AESIntHandler);
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
        puiData = ReadFromUser(&uiConfig,&uiKeySize,&puiKey1,&uiDataLength,\
                                &puiResult);
        if((puiData == NULL) || (puiResult == NULL))
        {
            continue;
        }
#else
        //
        // Load Default values
        //
        puiData = LoadDefaultValues(AES_CFG_DIR_ENCRYPT | AES_CFG_MODE_CBC ,
                                    &uiConfig,&uiKeySize,&puiIV,&puiKey1,
                                    &uiDataLength,&puiResult);
        
        if((puiData == NULL) || (puiResult == NULL))
        {
            while(FOREVER);
        }
#endif

        //
        // Carry out Encryption
        //
        UART_PRINT("\n\r Encryption in progress....");
        AESCrypt(uiConfig,uiKeySize,puiKey1,puiData,puiResult,uiDataLength,uiIV);
        UART_PRINT("\n\r Encryption done, cipher text created");
        
        //
        // Copy Result into Data Vector to continue with Decryption. and change 
        // config value
        //
        memcpy(puiData,puiResult,uiDataLength);
        uiConfig &= ~(1 << 2);
        //
        // Carry out Decryption
        //
        UART_PRINT("\n\r\n\r Decryption in progress....");
        AESCrypt(uiConfig,uiKeySize,puiKey1,puiData,puiResult,uiDataLength,uiIV);
        UART_PRINT("\n\r Decryption done");
        
        //
        // Display/Verify Result
        //

    #ifdef USER_INPUT
        //
        // Display Plain Text
        //
        UART_PRINT("\n\r Text after decryption ");
        pucResult = (unsigned char *)puiResult;
        for(uiCharCount=0;uiCharCount<uiDataLength;uiCharCount++)
        {
            UART_PRINT("%c",*(pucResult+uiCharCount));
        }
        UART_PRINT("\n\r");
        if(puiResult)
        {
        	free(puiResult);
        }
        if(puiData)
        {
        	free(puiData);
        }
    }
    #else
        //
        // Compare Cipher Text and Plain Text with the expected values from 
        // predefined vector
        //
        if(memcmp(puiData,psAESCBCTestVectors.pui32CipherText,
                        psAESCBCTestVectors.ui32DataLength)==0)
        {
            UART_PRINT("\n\r\n\r Encryption verification Successful");
        }
        else
        {
            UART_PRINT("\n\r\n\r Error in Encryption");
        }

        if(memcmp(puiResult,psAESCBCTestVectors.pui32PlainText,
                        psAESCBCTestVectors.ui32DataLength)==0)
        {
            UART_PRINT("\n\r Decryption verification Successful");
        }
        else
        {
            UART_PRINT("\n\r\n\r Error in Decryption");
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
