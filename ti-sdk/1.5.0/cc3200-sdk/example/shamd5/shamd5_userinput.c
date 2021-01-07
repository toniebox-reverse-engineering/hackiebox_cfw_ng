//*****************************************************************************
// shamd5_userinput.c
//
// APIs for shamd5 user input
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
//! \addtogroup shamd5_api
//! @{
//
//*****************************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// Driverlib includes
#include "hw_shamd5.h"
#include "hw_memmap.h"
#include "hw_types.h"
#include "rom.h"
#include "rom_map.h"
#include "shamd5.h"
#include "interrupt.h"
#include "prcm.h"
#include "uart.h"

#include "pinmux.h"
#include "shamd5_userinput.h"
#include "uart_if.h"


#define UART_PRINT Report


unsigned int uiHMACKey[16],puiPlainMsg[16];
extern unsigned int uiHMAC;
char *HMACKey1,*HMACKey2,*HMACKey3;


//*****************************************************************************
//
//! Set Keys. Sets the Pre Defined Keys
//!
//! \param None
//!
//! \return none
//
//*****************************************************************************
void
SetKeys()
{
    HMACKey1 = MemAllocAndCpy(64, \
            "p$d0Kotrp$d0Kotrp$d0Kotrp$d0Kotrp$d0Kotrp$d0Kotrp$d0Kotrp$d0Kotr");
    HMACKey2 = MemAllocAndCpy(64, \
            "abababababababababababababababababababababababababababababababab");
    HMACKey3 = MemAllocAndCpy(64, \
            "cdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcd");
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
void
UsageDisplay()
{
    UART_PRINT("Command Usage \n\r");
    UART_PRINT("-------------- \n\r");
    UART_PRINT("hash <shamd5_mode> - Generates the Hash value\n\r");
    UART_PRINT("\n\r");
    UART_PRINT("Parameters \n\r");
    UART_PRINT("---------- \n\r");
    UART_PRINT("shamd5_mode - Algorithm to be selected by the user [MD5|SHA1|"
              "SHA224|SHA256|HMAC_MD5|HMAC_SHA1|HMAC_SHA224|HMAC_SHA256] \n\r");
    UART_PRINT("-------------------------------------------------------------"
                "---------------- \n\r\n\r");
    UART_PRINT("\n\r");

    
}
//*****************************************************************************
//
//! memory allocate and copy string
//!
//! This function
//!        1. Allocate memory and defines the Default Keys
//! \param None
//!
//! \return None
//
//*****************************************************************************
char *
MemAllocAndCpy(int size, char *keyVal)
{
  char * key =( char*)malloc(size);
  strcpy(key, keyVal);
  return key;
}

//*****************************************************************************
//
//! Get Key - Gets the Key into the Buffer from User
//!
//! \param  pucKeyBuff is the Key buffer to which Key will be populated
//!
//! \return Success or Failure
//!
//*****************************************************************************
bool
GetKey(char *pucKeyBuff)
{
    char cChar;
    unsigned int uiMsgLen;
    if(uiHMAC)
    {
        UART_PRINT("\n\rDo you want to use Pre-Defined Key or not (y/n) ? \n\r");
        //
        // Get the option
        //
        cChar = MAP_UARTCharGet(UARTA0_BASE);
        //
        // Echo the received character
        //
        MAP_UARTCharPut(UARTA0_BASE, cChar);
        UART_PRINT("\n\r");
        if(cChar=='y' || cChar=='Y' )
        {
            //
            // Fetch the key
            //
            UART_PRINT("\n\r Press 1 for Key - %s\n\r Press 2 for Key - %s \n\r"
                        " Press 3 for Key - %s\n\r",HMACKey1,HMACKey2,HMACKey3);
            cChar = MAP_UARTCharGet(UARTA0_BASE);
            //
            // Echo the received character
            //
            MAP_UARTCharPut(UARTA0_BASE, cChar);
            UART_PRINT("\n\r");
            if(cChar=='1' )
              memcpy(&uiHMACKey,HMACKey1,64);
            else if(cChar=='2')
              memcpy(&uiHMACKey,HMACKey2,64);
            else if(cChar=='3')
              memcpy(&uiHMACKey,HMACKey3,64);
            else
            {
                UART_PRINT("\n\r Wrong Key \n\r");
                return false;
            }

       }
       else if(cChar=='n' || cChar=='N')
       {
            //
            // Ask for the Key
            //
            UART_PRINT("Enter the Key \n\r");
            uiMsgLen=GetCmd(pucKeyBuff,520);
            if(uiMsgLen!=64)
            {
                UART_PRINT("\n\r Enter Valid Key of length 64\n\r");
                return false;
            }
       }
       else
       {
            UART_PRINT("\n\r Invalid Input \n\r");
            return false;
       }
    }

    return true;
}

//*****************************************************************************
//
//! Get Msg - Gets the Message into the Buffer from User
//!
//! \param  pucMsgBuff is the Msg buffer to which Plain Text will be populated
//! \param  uiDataLength is the Length of the Data entered
//!
//! \return Pointer to Plain Text
//!
//*****************************************************************************
unsigned char * GetMsg(char *pucMsgBuff,unsigned int *uiDataLength)
{
    unsigned int uiMsgLen,iSize;
    unsigned char *uiData;
    UART_PRINT("\n\r Enter the Message \n\r");
    
    //
    // Get Message
    //
    uiMsgLen=GetCmd(pucMsgBuff, 520);
    iSize=uiMsgLen;
    *uiDataLength=iSize;
    
    //
    // Copy Messge into Data variable
    //
    uiData=(unsigned char *)malloc(*uiDataLength);
    memset(uiData,0,*uiDataLength);
    memcpy(uiData,pucMsgBuff,(uiMsgLen));
    return uiData;
}

//*****************************************************************************
//
//! SHAMD5Parser - Populates the parameters from User
//!
//! \param  ucCMD is the Command buffer to which Command will be populated
//! \param  uiConfig is the Configuration Value
//! \param  uiHashLength is the Hash Length
//!
//! \return Success or Failure
//!
//*****************************************************************************
bool SHAMD5Parser( char *ucCMD,unsigned int *uiConfig,unsigned int *uiHashLength)
{
    char *ucInpString;
    ucInpString = strtok(ucCMD, " ");

    //
    // Check Whether Command is valid
    //
    if((ucInpString != NULL) && (!strcmp(ucInpString,"hash")))
    {
        ucInpString=strtok(NULL," ");
        //
        // Get which Algorithm is using
        //
        if((ucInpString != NULL) && (!strcmp(ucInpString,"MD5") || \
                                !strcmp(ucInpString,"md5")))
        {
            *uiConfig=SHAMD5_ALGO_MD5;
            *uiHashLength=16;
            uiHMAC=0;
        }
        else if((ucInpString != NULL) && (!strcmp(ucInpString,"SHA1") || \
                                !strcmp(ucInpString,"sha1")))
        {
            *uiConfig=SHAMD5_ALGO_SHA1;
            *uiHashLength=20;
            uiHMAC=0;
        }
        else if((ucInpString != NULL) && (!strcmp(ucInpString,"SHA224") || \
                                !strcmp(ucInpString,"sha224")))
        {
            *uiConfig=SHAMD5_ALGO_SHA224;
            *uiHashLength=28;
            uiHMAC=0;
        }
        else if((ucInpString != NULL) && (!strcmp(ucInpString,"SHA256") || \
                                !strcmp(ucInpString,"sha256")))
        {
            *uiConfig=SHAMD5_ALGO_SHA256;
            *uiHashLength=32;
            uiHMAC=0;
        }
        else if((ucInpString != NULL) && (!strcmp(ucInpString,"HMAC_MD5") || \
                            !strcmp(ucInpString,"hmac_md5")))
        {
            *uiConfig=SHAMD5_ALGO_HMAC_MD5;
            *uiHashLength=16;
            uiHMAC=1;
        }
        else if((ucInpString != NULL) && (!strcmp(ucInpString,"HMAC_SHA1") || \
                            !strcmp(ucInpString,"hmac_sha1")))
        {
            *uiConfig=SHAMD5_ALGO_HMAC_SHA1;
            *uiHashLength=20;
            uiHMAC=1;
        }
        else if((ucInpString != NULL) && (!strcmp(ucInpString,"HMAC_SHA224") || \
                        !strcmp(ucInpString,"hmac_sha224")))
        {
            *uiConfig=SHAMD5_ALGO_HMAC_SHA224;
            *uiHashLength=28;
            uiHMAC=1;
        }
        else if((ucInpString != NULL) && (!strcmp(ucInpString,"HMAC_SHA256") || \
                        !strcmp(ucInpString,"hmac_sha256")))
        {
            *uiConfig=SHAMD5_ALGO_HMAC_SHA256;
            *uiHashLength=32;
            uiHMAC=1;
        }
        else
        {
            UART_PRINT("\n\r Invalid Algorithm\n\r");
            return false;
        }
        return true;
    }
    else
    {
        UART_PRINT("\n\r Invalid Command \n\r");
        return false;
    }
}

//*****************************************************************************
//
//! ReadFromUser - Populates the parameters from User
//!
//! \param  uiConfig Configuration Value
//! \param  uiHashLength is the Length of Hash Value
//! \param  uiKey is the Key Used
//! \param  uiDataLength is the Length of Data
//! \param  puiResult is the Result
//!
//! \return pointer to plain text
//!
//*****************************************************************************
unsigned char *
ReadFromUser(unsigned int *uiConfig,unsigned int *uiHashLength,unsigned 
              char **uiKey,unsigned int *uiDataLength,unsigned char **puiResult)
{
    char ucCmdBuffer[520],*pucKeyBuff,*pucMsgBuff;
    unsigned char *uiData;

    //
    // POinting KeyBuffer into appropriate Keys. 
    //
    pucKeyBuff=(char*)&uiHMACKey[0];
    *uiKey=(unsigned char*)&uiHMACKey[0];
    pucMsgBuff=( char*)&puiPlainMsg[0];

    //
    // Set Default Values
    //
    SetKeys();

    //
    // Usage Display
    //
    UsageDisplay();

    //
    // Get the Command
    //
    UART_PRINT("cmd# ");
    GetCmd(ucCmdBuffer,520);
    if(SHAMD5Parser(ucCmdBuffer,uiConfig,uiHashLength))
    {
        if(GetKey(pucKeyBuff))
        {
            uiData=GetMsg(pucMsgBuff,uiDataLength);
            *puiResult=(unsigned char *)malloc(64);
            memset(*puiResult,0,64);
        }
        else
        {
            UART_PRINT("\n\r Invalid Key \n\r");
            return NULL;
        }
    }
    else
    {
        UART_PRINT("\n\r Wrong Input \n\r");
        return NULL;
    }
    return uiData;
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
