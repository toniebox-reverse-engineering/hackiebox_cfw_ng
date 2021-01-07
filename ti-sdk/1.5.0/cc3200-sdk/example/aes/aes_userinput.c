//*****************************************************************************
// aes_userinput.c
//
// Functions for AES USER Input
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
//! \addtogroup aes_userinput
//! @{
//
//*****************************************************************************

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "hw_aes.h"
#include "hw_memmap.h"
#include "hw_types.h"
#include "rom.h"
#include "rom_map.h"
#include "aes.h"
#include "interrupt.h"
#include "prcm.h"
#include "uart_if.h"
#include "uart.h"
#include "pinmux.h"
#include "aes_userinput.h"

#define UART_PRINT Report
#define BUFFER_LEN 520
unsigned int uiKey128[4],uiKey192[6],uiKey256[8], uiKeyUserDefined[BUFFER_LEN];
unsigned int pui32AESPlainMsg[16],pui32AESCipherTxt[16],
            uiIV[4]={0x03020100, 0x07060504, 0x0b0a0908, 0x0f0e0d0c};

char *AES128Key1 = "abcdefghijklmnpq";
char *AES128Key2 = "rstuvwxyz1234567";
char *AES128Key3 = "12345678abcdefgh";

char *AES192Key1 = "abcdefghijklmnpqrstuvwxy";
char *AES192Key2 = "rstuvwxyz1234567abcdefgh";
char *AES192Key3 = "12345678abcdefghrstuvwxy";

char *AES256Key1 = "abcdefghijklmnpqrstuvwxy12345678";
char *AES256Key2 = "rstuvwxyz1234567abcdefghijklmnpq";
char *AES256Key3 = "12345678abcdefghrstuvwxyijklmnpq";

unsigned int ui32aes_mode;

//*****************************************************************************
//
//! AES Parser Function
//!
//! This function
//!        1. Parses and gets the Configuration,Key size
//! \param ucCMD - Command Buffer
//! \out uiConfig - Configuration value (Enc/Dec | Key Size)
//! \out uiKeySize - Key Size Used
//!
//! \return Success or Failure
//
//*****************************************************************************
bool
AESParser( char *ucCMD,unsigned int *uiConfig,unsigned int *uiKeySize)
{
    char *ucInpString;
    ucInpString = strtok(ucCMD, " ");

    //
    // Check Whether Command is valid
    //
    if((ucInpString != NULL) && ((!strcmp(ucInpString,"aesdemo"))))
    {

        *uiConfig=AES_CFG_DIR_ENCRYPT;
        ucInpString=strtok(NULL," ");
        
        //
        // Get which Algorithm you are using for Encryption or Decryption
        //
        if(ucInpString != NULL && ((!strcmp(ucInpString,"ECB")) || \
                                !strcmp(ucInpString,"ecb")))
        {
            *uiConfig|=AES_CFG_MODE_ECB;
            ui32aes_mode = AES_CFG_MODE_ECB;

        }
        else if(ucInpString != NULL && (!strcmp(ucInpString,"CBC") || \
                                !strcmp(ucInpString,"cbc")))
        {
            *uiConfig|=AES_CFG_MODE_CBC;
            ui32aes_mode = AES_CFG_MODE_CBC;

        }
        else if(ucInpString != NULL && (!strcmp(ucInpString,"CTR") || \
                                !strcmp(ucInpString,"ctr")))
        {
            *uiConfig|=AES_CFG_MODE_CTR;
            ui32aes_mode = AES_CFG_MODE_CTR;

        }
        else if(ucInpString != NULL && (!strcmp(ucInpString,"ICM") || \
                                !strcmp(ucInpString,"icm")))
        {
            *uiConfig|=AES_CFG_MODE_ICM;
            ui32aes_mode = AES_CFG_MODE_ICM;

        }
        else if(ucInpString != NULL && (!strcmp(ucInpString,"CFB") || \
                                !strcmp(ucInpString,"cfb")))
        {
            *uiConfig|=AES_CFG_MODE_CFB;
            ui32aes_mode = AES_CFG_MODE_CFB;
        }
        else
        {
            UART_PRINT("\n\r Invalid Algorithm\n\r");
            return false;
        }
        
        //  
        // Get Key Length
        //
        ucInpString=strtok(NULL,"");
        if(ucInpString != NULL)
        {
            *uiKeySize=strtoul(ucInpString, NULL, 10);
        }
        if(*uiKeySize==128)
            *uiKeySize=AES_CFG_KEY_SIZE_128BIT;
        else if(*uiKeySize==192)
            *uiKeySize=AES_CFG_KEY_SIZE_192BIT;
        else if(*uiKeySize==256)
            *uiKeySize=AES_CFG_KEY_SIZE_256BIT;
        else
        {
            UART_PRINT("\n\r Invalid KeySize \n\r");
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
//! Get Key Function
//!
//! This function
//!        1. Gets the Key from the User
//! \param uiKeySize - Key Size used
//! \out pucKeyBuff - KeyBuffer in which Key is stored
//!
//! \return Success or Failure
//
//*****************************************************************************
bool
GetKey(unsigned int uiKeySize, char *pucKeyBuff)
{
  char cChar;
  unsigned int uiMsgLen;
  char	keyLen;

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
      if(uiKeySize==AES_CFG_KEY_SIZE_128BIT)
      {
        UART_PRINT("\n\r Press 1 for Key - %s\n\r Press 2 for Key - %s \n\r "
                   "Press 3 for Key - %s\n\r",AES128Key1,AES128Key2,AES128Key3);
        cChar = MAP_UARTCharGet(UARTA0_BASE);
        
        //
        // Echo the received character
        //
        MAP_UARTCharPut(UARTA0_BASE, cChar);
        UART_PRINT("\n\r");
        if(cChar=='1' )
          memcpy(&uiKey128,AES128Key1,16);
        else if(cChar=='2')
          memcpy(&uiKey128,AES128Key2,16);
        else if(cChar=='3')
          memcpy(&uiKey128,AES128Key3,16);
        else
        {
          UART_PRINT("\n\r Invalid Input \n\r");
          return false;
        }
      }
      else if(uiKeySize==AES_CFG_KEY_SIZE_192BIT)
      {
        UART_PRINT("\n\r Press 1 for Key - %s\n\r Press 2 for Key - %s \n\r "
                   "Press 3 for Key - %s\n\r",AES192Key1,AES192Key2,AES192Key3);
        cChar = MAP_UARTCharGet(UARTA0_BASE);
        
        //
        // Echo the received character
        //
        MAP_UARTCharPut(UARTA0_BASE, cChar);
        UART_PRINT("\n\r");
        if(cChar=='1' )
          memcpy(&uiKey192,AES192Key1,24);
        else if(cChar=='2')
          memcpy(&uiKey192,AES192Key2,24);
        else if(cChar=='3')
          memcpy(&uiKey192,AES192Key3,24);
        else
        {
          UART_PRINT("\n\r Invalid Input \n\r");
          return false;
        }
      }
      else if(uiKeySize==AES_CFG_KEY_SIZE_256BIT)
      {
        UART_PRINT("\n\r Press 1 for Key - %s\n\r Press 2 for Key - %s \n\r"
                   " Press 3 for Key - %s\n\r",AES256Key1,AES256Key2,AES256Key3);
        cChar = MAP_UARTCharGet(UARTA0_BASE);
        
        //
        // Echo the received character
        //
        MAP_UARTCharPut(UARTA0_BASE, cChar);
        UART_PRINT("\n\r");
        if(cChar=='1' )
          memcpy(&uiKey256,AES256Key1,32);
        else if(cChar=='2')
          memcpy(&uiKey256,AES256Key2,32);
        else if(cChar=='3')
          memcpy(&uiKey256,AES256Key3,32);
        else
        {
          UART_PRINT("\n\r Invalid Input \n\r");
          return false;
        }
      }

  }

  else if(cChar=='n' || cChar=='N')
  {
	  if(uiKeySize==AES_CFG_KEY_SIZE_128BIT)
	  {
		  keyLen = 16;
	  }
	  else if(uiKeySize==AES_CFG_KEY_SIZE_192BIT)
	  {
		  keyLen = 24;
	  }
	  else if(uiKeySize==AES_CFG_KEY_SIZE_256BIT)
	  {
		  keyLen = 32;
	  }


      //
      // Ask for the Key
      //
      UART_PRINT("\n\rEnter the Key \n\r");
      uiMsgLen=GetCmd(pucKeyBuff,520);
      if(uiMsgLen!=keyLen)
      {
        UART_PRINT("\n\r Enter Valid Key of length %d\n\r",keyLen);
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
//! Get Msg Function
//!
//! This function
//!        1. Gets the Message from the User
//! \out pucMsgBuff - Message Buffer used to store Plain Text
//! \out uiDataLength - Data Length
//!
//! \return pointer to Plain Text
//
//*****************************************************************************
unsigned int*
GetMsg( char *pucMsgBuff,unsigned int *uiDataLength)
{

    int iMod=0;
    unsigned int uiMsgLen,iSize;
    unsigned int *uiData;
    UART_PRINT("\n\r Enter the Message \n\r");
    uiMsgLen=GetCmd(pucMsgBuff, 520);
    if(uiMsgLen == 0)
    {
    return 0;
    }

    //
    // For ECB and CBC mode, message should be blocks of 16 bytes.
    //
    iSize=uiMsgLen;
    if((ui32aes_mode==AES_CFG_MODE_ECB)||(ui32aes_mode==AES_CFG_MODE_CBC))
    {
        iMod = uiMsgLen%16;
        if(iMod!=0)
        {
            iSize = ((uiMsgLen/16)+1)*16;
        }
    }

    //
    // Allocate Memory Buffer
    //
    *uiDataLength=iSize;
    uiData=(unsigned int *)malloc(*uiDataLength);
    if(uiData == NULL)
    {
        UART_PRINT("Failed to allocate memory\n\r");
        return(NULL);
    }
    memset(uiData,0,*uiDataLength);
    memcpy(uiData,pucMsgBuff,(uiMsgLen));
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
void
UsageDisplay()
{

    UART_PRINT("\nCommand Usage \n\r");
    UART_PRINT("-------------- \n\r");
    UART_PRINT("aesdemo <aes_mode> <key_len> - AES Encryption and Decryption\n\r");
    UART_PRINT("\n\r");
    UART_PRINT("Parameters \n\r");
    UART_PRINT("-------------- \n\r");
    UART_PRINT("aes_mode - AES algorithm to be selected by the user [ECB|CBC|"
                "CTR|ICM|CFB] \n\r");
    UART_PRINT("key_len - Key length for decryption [128|192|256] \n\r");
    UART_PRINT("--------------------------------------------------------------"
                "----------- \n\r\n\r");
    UART_PRINT("\n\r");

}

//*****************************************************************************
//
//! ReadFrom User Function
//!
//! This function
//!        1. Gets Input from User
//! \out uiConfig - Configuration Value
//! \out uiKeySize - Key Size
//! \out uiKey - Key Used
//! \out uiDataLength - DataLength
//! \out puiResult - Result
//!
//! \return pointer to Plain Text
//
//*****************************************************************************
unsigned int*
ReadFromUser(unsigned int *uiConfig,unsigned int *uiKeySize,unsigned int **uiKey
             ,unsigned int *uiDataLength,unsigned int **puiResult)
{
    char ucCmdBuffer[520],*pucKeyBuff,*pucMsgBuff;
    unsigned int *uiData;
    pui32AESPlainMsg[0] = '\0';
    pucMsgBuff=( char*)&pui32AESPlainMsg[0];
    
    UsageDisplay();
    UART_PRINT("cmd# ");
    GetCmd(ucCmdBuffer,520);
    if(AESParser(ucCmdBuffer,uiConfig,uiKeySize))
    {
        //
        // Get Key Length
        //
        if(*uiKeySize==AES_CFG_KEY_SIZE_128BIT)
        {
            pucKeyBuff=(char*)&uiKey128[0];
            *uiKey=(unsigned int*)&uiKey128[0];
        }
        else if(*uiKeySize==AES_CFG_KEY_SIZE_192BIT)
        {
            pucKeyBuff=( char*)&uiKey192[0];
            *uiKey=(unsigned int*)&uiKey192[0];
        }
        else if(*uiKeySize==AES_CFG_KEY_SIZE_256BIT)
        {
            pucKeyBuff=( char*)&uiKey256[0];
            *uiKey=(unsigned int*)&uiKey256[0];
        }
        else
        {
            pucKeyBuff = (char *)&uiKeyUserDefined[0];
        }

        if(GetKey(*uiKeySize,pucKeyBuff))
        {
            uiData=GetMsg(pucMsgBuff,uiDataLength);
            if(uiData == NULL)
            {
                return NULL;
            }
            *puiResult=(unsigned int *)malloc(*uiDataLength);
            if(*puiResult == NULL)
            {
                free(uiData);
                UART_PRINT("Failed to allocate memory\n\r");
                return NULL;
            }
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
