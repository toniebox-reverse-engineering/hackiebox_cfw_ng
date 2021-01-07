//*****************************************************************************
// des_userinput.c
//
// des userinput function APIs
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
//! \addtogroup des_userinput
//! @{
//
//*****************************************************************************

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "hw_des.h"
#include "hw_memmap.h"
#include "hw_types.h"
#include "rom.h"
#include "rom_map.h"
#include "des.h"
#include "interrupt.h"
#include "prcm.h"
#include "udma.h"
#include "pinmux.h"
#include "uart.h"
#include "uart_if.h"
#include "des_userinput.h"

#define UART_PRINT Report
#define BUFFER_LEN 520
char *DESKey1,*DESKey2,*DESKey3,*TDESKey1,*TDESKey2,*TDESKey3;
unsigned int pui32AESPlainMsg[16],pui32AESCipherTxt[16],uiDESKey[2],uiTDESKey[6];
unsigned int ui32DesMode = 0;

//*****************************************************************************
//
//! Set Keys with some default values
//!
//! \param None
//!
//! \return None
//
//*****************************************************************************
void
SetKeys()
{
    DESKey1 = MemAllocAndCpy(8, "abcdefgh");
    DESKey2 = MemAllocAndCpy(8, "rstuvwxy");
    DESKey3 = MemAllocAndCpy(8, "12345678");
    
    TDESKey1 = MemAllocAndCpy(24, "abcdefghijklmnpqrstuvwxy");
    TDESKey2 = MemAllocAndCpy(24, "rstuvwxyz1234567abcdefgh");
    TDESKey3 = MemAllocAndCpy(24, "12345678abcdefghrstuvwxy");
  
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
    UART_PRINT("\n\rCommand Usage \n\r");
    UART_PRINT("-------------- \n\r");
    UART_PRINT("desdemo <des_mode> - DES Encryption and Decryption \n\r");
    UART_PRINT("\n\r");
    UART_PRINT("Parameters \n\r");
    UART_PRINT("---------- \n\r");
    UART_PRINT("des_mode - DES algorithm to be selected by the user [ECB|CBC|CFB|TECB|TCBC|TCFB] \n\r");
    UART_PRINT("--------------------------------------------------------------------------------\n\r\n\r");
    UART_PRINT("\n\r");

    
}
//*****************************************************************************
//
//! DESParser - Parses the command and update uiConfig and uiKeySize values
//!
//! \param  ucCMDBuffer - Buffer which contains command from UART
//! \param uiConfig - Configuartion Value
//! \param uiKeySize - Key Size Used
//!
//! \return Success or Failure
//!
//*****************************************************************************
bool
DESParser( char *ucCMD,unsigned int *uiConfig,unsigned int *uiKeySize)
{
    char *ucInpString;
    ucInpString = strtok(ucCMD, " ");

    //
    // Check Whether Command is valid
    //
    if((ucInpString != NULL) && ((!strcmp(ucInpString,"desdemo")) ))
    {
       
        *uiConfig=DES_CFG_DIR_ENCRYPT;
        ui32DesMode = 0;

        ucInpString=strtok(NULL," ");

        //
        // Get which Algorithm you are using for Encryption or Decryption
        //
        if(ucInpString != NULL && (!strcmp(ucInpString,"ECB") || \
                                !strcmp(ucInpString,"ecb")))
        {
            *uiConfig|=DES_CFG_MODE_ECB|DES_CFG_SINGLE;
            *uiKeySize=8;
        }
        else if(ucInpString != NULL && (!strcmp(ucInpString,"CBC") || \
                                !strcmp(ucInpString,"cbc")))
        {
            *uiConfig|=DES_CFG_MODE_CBC|DES_CFG_SINGLE;
            *uiKeySize=8;
        }
        else if(ucInpString != NULL && (!strcmp(ucInpString,"CFB") || \
                                !strcmp(ucInpString,"cfb")))
        {
            *uiConfig|=DES_CFG_MODE_CFB|DES_CFG_SINGLE;
            *uiKeySize=8;
            ui32DesMode = DES_CFG_MODE_CFB;
        }
        else if(ucInpString != NULL && (!strcmp(ucInpString,"TECB") || \
                            !strcmp(ucInpString,"tecb")))
        {
            *uiConfig|=DES_CFG_MODE_ECB|DES_CFG_TRIPLE;
            *uiKeySize=24;
        }
        else if(ucInpString != NULL && (!strcmp(ucInpString,"TCBC") || \
                            !strcmp(ucInpString,"tcbc")))
        {
            *uiConfig|=DES_CFG_MODE_CBC|DES_CFG_TRIPLE;
            *uiKeySize=24;
        }
        else if(ucInpString != NULL && (!strcmp(ucInpString,"TCFB") || \
                        !strcmp(ucInpString,"tcfb")))
        {
            *uiConfig|=DES_CFG_MODE_CFB|DES_CFG_TRIPLE;
            *uiKeySize=24;
            ui32DesMode = DES_CFG_MODE_CFB;
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
//! GetKey - Gets the Key from User
//!
//! \param  uiKeySize - Key Size Used
//! \param pucKeyBuff - Key Buffer into which Key will be populated
//!
//! \return Success or Failure
//!
//*****************************************************************************
bool
GetKey(unsigned int uiKeySize,char *pucKeyBuff)
{
    char cChar;
    unsigned int uiMsgLen;
    UART_PRINT("\n\r Do you want to use Pre-Defined Key ???(y/n) \n\r");
    
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
        if(uiKeySize==8)
        {
            UART_PRINT("\n\r Press 1 for Key - %s\n\r Press 2 for Key - %s "
                        "\n\r Press 3 for Key - %s\n\r",DESKey1,DESKey2,DESKey3);
            cChar = MAP_UARTCharGet(UARTA0_BASE);
            
            //
            // Echo the received character
            //
            MAP_UARTCharPut(UARTA0_BASE, cChar);
            UART_PRINT("\n\r");
            if(cChar=='1' )
              memcpy(&uiDESKey,DESKey1,8);
            else if(cChar=='2')
              memcpy(&uiDESKey,DESKey2,8);
            else if(cChar=='3')
              memcpy(&uiDESKey,DESKey3,8);
            else
            {
              UART_PRINT("\n\r Invalid Input \n\r");
              return false;
            }
        }
        else if(uiKeySize==24)
        {
            UART_PRINT("\n\r Press 1 for Key - %s\n\r Press 2 for Key - %s "
                     "\n\r Press 3 for Key - %s\n\r",TDESKey1,TDESKey2,TDESKey3);
            cChar = MAP_UARTCharGet(UARTA0_BASE);
            
            //
            // Echo the received character
            //
            MAP_UARTCharPut(UARTA0_BASE, cChar);
            UART_PRINT("\n\r");
            if(cChar=='1' )
              memcpy(&uiTDESKey,TDESKey1,24);
            else if(cChar=='2')
              memcpy(&uiTDESKey,TDESKey2,24);
            else if(cChar=='3')
              memcpy(&uiTDESKey,TDESKey3,24);
            else
            {
              UART_PRINT("\n\r Invalid Input \n\r");
              return false;
            }
        }


      }
      else if(cChar=='n' || cChar=='N')
      {
          //
          // Ask for the Key
          //
          UART_PRINT("\n\rEnter the Key \n\r");
          uiMsgLen=GetCmd(pucKeyBuff,BUFFER_LEN);
          if(uiMsgLen!=uiKeySize)
          {
            UART_PRINT("\n\r Enter Valid Key of length %d\n\r",uiKeySize);
            return false;
          }

      }
    else
    {
        UART_PRINT("\n\r Invalid Input \n\r");
        return false;
    }
    return true;
}
//*****************************************************************************
//
//! GetMsg - Gets the Message from User
//!
//! \param  uiDataLength - DataLength Used
//! \param pucMsgBuff - Message Buffer into which Message will be populated
//!
//! \return Pointer to Input Data
//!
//*****************************************************************************
unsigned char*
GetMsg( char *pucMsgBuff,unsigned int *uiDataLength)
{
  
    int iMod=0;
    unsigned int uiMsgLen,iSize;
    unsigned char *uiData;

    UART_PRINT("\n\r Enter the Message \n\r");
    uiMsgLen=GetCmd(pucMsgBuff, BUFFER_LEN);

    //
    // For ECB and CBC, data should be blocks of 16 bytes.
    //
    iSize=uiMsgLen;
    //      if (ui32DesMode != DES_CFG_MODE_CFB)
    {
        iMod=uiMsgLen%8;
        if(iMod!=0)
        {
                iSize=((uiMsgLen/8)+1)*8;
        }
    }

    //
    // Allocate Memory Buffer
    //
    *uiDataLength=iSize;
     uiData=(unsigned char *)malloc(*uiDataLength);
     memset(uiData,0,*uiDataLength);
     memcpy(uiData,pucMsgBuff,(uiMsgLen));


  return uiData;
}
//*****************************************************************************
//
//! ReadFromUser - Gets the Data from User
//!
//! \out  uiConfig - Configuration Value Used
//! \out uiKeySize - KeySize Used
//! \out uiKey - Key Used
//! \out uiDataLength - DataLength Used
//! \out puiResult - Result Value
//!
//! \return Pointer to Input Data
//!
//*****************************************************************************

unsigned char*
ReadFromUser(unsigned int *uiConfig,unsigned int *uiKeySize,unsigned char **uiKey,
    unsigned int *uiDataLength,unsigned char **puiResult)
{
    
    char ucCmdBuffer[BUFFER_LEN],*pucKeyBuff,*pucMsgBuff;
    unsigned char *uiData;

    pucMsgBuff=( char*)&pui32AESPlainMsg[0];
    
    //
    // Usage Display
    //
    UsageDisplay();
    
    //
    // Set keys
    //
    SetKeys();
    
    //
    // Get the Command
    //
    UART_PRINT("cmd# ");
    GetCmd(ucCmdBuffer,BUFFER_LEN);

    if(DESParser(ucCmdBuffer,uiConfig,uiKeySize))
    {

     //
     // Get Key Length
     //
     if(*uiKeySize==8)
     {
       //
       // KeyBuff is used when User inputs the Key
       //
       pucKeyBuff=(char*)&uiDESKey[0];
       *uiKey=(unsigned char*)&uiDESKey[0];
       
     }
     else if(*uiKeySize==24)
     {
       pucKeyBuff=( char*)&uiTDESKey[0];
       *uiKey=(unsigned char*)&uiTDESKey[0];
     }
     else
     {
       pucKeyBuff = (char *) malloc(BUFFER_LEN);
     }
     if(GetKey(*uiKeySize,pucKeyBuff))
     {
       uiData=GetMsg(pucMsgBuff,uiDataLength);
       *puiResult=(unsigned char *)malloc(*uiDataLength);
       memset(*puiResult,0,*uiDataLength);
       
     }
     else
     {
       UART_PRINT("\n\r Invalid Key \n\r");
       return NULL;
     }
    }
    else
    {
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


