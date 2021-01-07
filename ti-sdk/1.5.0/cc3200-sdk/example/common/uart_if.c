//*****************************************************************************
// uart_if.c
//
// uart interface file: Prototypes and Macros for UARTLogger
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

// Standard includes
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Driverlib includes
#include "hw_types.h"
#include "hw_memmap.h"
#include "prcm.h"
#include "pin.h"
#include "uart.h"
#include "rom.h"
#include "rom_map.h"

#if defined(USE_FREERTOS) || defined(USE_TI_RTOS)
#include "osi.h"
#endif

#include "uart_if.h"

#define IS_SPACE(x)       (x == 32 ? 1 : 0)

//*****************************************************************************
// Global variable indicating command is present
//*****************************************************************************
static unsigned long __Errorlog;

//*****************************************************************************
// Global variable indicating input length
//*****************************************************************************
unsigned int ilen=1;


//*****************************************************************************
//
//! Initialization
//!
//! This function
//!        1. Configures the UART to be used.
//!
//! \return none
//
//*****************************************************************************
void 
InitTerm()
{
#ifndef NOTERM
  MAP_UARTConfigSetExpClk(CONSOLE,MAP_PRCMPeripheralClockGet(CONSOLE_PERIPH), 
                  UART_BAUD_RATE, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                   UART_CONFIG_PAR_NONE));
#endif
  __Errorlog = 0;
}

//*****************************************************************************
//
//!    Outputs a character string to the console
//!
//! \param str is the pointer to the string to be printed
//!
//! This function
//!        1. prints the input string character by character on to the console.
//!
//! \return none
//
//*****************************************************************************
void 
Message(const char *str)
{
#ifndef NOTERM
    if(str != NULL)
    {
        while(*str!='\0')
        {
            MAP_UARTCharPut(CONSOLE,*str++);
        }
    }
#endif
}

//*****************************************************************************
//
//!    Clear the console window
//!
//! This function
//!        1. clears the console window.
//!
//! \return none
//
//*****************************************************************************
void 
ClearTerm()
{
    Message("\33[2J\r");
}

//*****************************************************************************
//
//! Error Function
//!
//! \param 
//!
//! \return none
//! 
//*****************************************************************************
void 
Error(char *pcFormat, ...)
{
#ifndef NOTERM
    char  cBuf[256];
    va_list list;
    va_start(list,pcFormat);
    vsnprintf(cBuf,256,pcFormat,list);
    Message(cBuf);
#endif
    __Errorlog++;
}

//*****************************************************************************
//
//! Get the Command string from UART
//!
//! \param  pucBuffer is the command store to which command will be populated
//! \param  ucBufLen is the length of buffer store available
//!
//! \return Length of the bytes received. -1 if buffer length exceeded.
//! 
//*****************************************************************************
int
GetCmd(char *pcBuffer, unsigned int uiBufLen)
{
    char cChar;
    int iLen = 0;
    
    //
    // Wait to receive a character over UART
    //
    while(MAP_UARTCharsAvail(CONSOLE) == false)
    {
#if defined(USE_FREERTOS) || defined(USE_TI_RTOS)
    	osi_Sleep(1);
#endif
    }
    cChar = MAP_UARTCharGetNonBlocking(CONSOLE);
    
    //
    // Echo the received character
    //
    MAP_UARTCharPut(CONSOLE, cChar);
    iLen = 0;
    
    //
    // Checking the end of Command
    //
    while((cChar != '\r') && (cChar !='\n') )
    {
        //
        // Handling overflow of buffer
        //
        if(iLen >= uiBufLen)
        {
            return -1;
        }
        
        //
        // Copying Data from UART into a buffer
        //
        if(cChar != '\b')
        { 
            *(pcBuffer + iLen) = cChar;
            iLen++;
        }
        else
        {
            //
            // Deleting last character when you hit backspace 
            //
            if(iLen)
            {
                iLen--;
            }
        }
        //
        // Wait to receive a character over UART
        //
        while(MAP_UARTCharsAvail(CONSOLE) == false)
        {
#if defined(USE_FREERTOS) || defined(USE_TI_RTOS)
        	osi_Sleep(1);
#endif
        }
        cChar = MAP_UARTCharGetNonBlocking(CONSOLE);
        //
        // Echo the received character
        //
        MAP_UARTCharPut(CONSOLE, cChar);
    }

    *(pcBuffer + iLen) = '\0';

    Report("\n\r");

    return iLen;
}

//*****************************************************************************
//
//!    Trim the spaces from left and right end of given string
//!
//! \param  Input string on which trimming happens
//!
//! \return length of trimmed string
//
//*****************************************************************************
int TrimSpace(char * pcInput)
{
    size_t size;
    char *endStr, *strData = pcInput;
    char index = 0;
    size = strlen(strData);

    if (!size)
        return 0;

    endStr = strData + size - 1;
    while (endStr >= strData && IS_SPACE(*endStr))
        endStr--;
    *(endStr + 1) = '\0';

    while (*strData && IS_SPACE(*strData))
    {
        strData++;
        index++;
    }
    memmove(pcInput,strData,strlen(strData)+1);

    return strlen(pcInput);
}

//*****************************************************************************
//
//!    prints the formatted string on to the console
//!
//! \param format is a pointer to the character string specifying the format in
//!           the following arguments need to be interpreted.
//! \param [variable number of] arguments according to the format in the first
//!         parameters
//! This function
//!        1. prints the formatted error statement.
//!
//! \return count of characters printed
//
//*****************************************************************************
int Report(const char *pcFormat, ...)
{
 int iRet = 0;
#ifndef NOTERM

  char *pcBuff, *pcTemp;
  int iSize = 256;
 
  va_list list;
  pcBuff = (char*)malloc(iSize);
  if(pcBuff == NULL)
  {
      return -1;
  }
  while(1)
  {
      va_start(list,pcFormat);
      iRet = vsnprintf(pcBuff,iSize,pcFormat,list);
      va_end(list);
      if(iRet > -1 && iRet < iSize)
      {
          break;
      }
      else
      {
          iSize*=2;
          if((pcTemp=realloc(pcBuff,iSize))==NULL)
          { 
              Message("Could not reallocate memory\n\r");
              iRet = -1;
              break;
          }
          else
          {
              pcBuff=pcTemp;
          }
          
      }
  }
  Message(pcBuff);
  free(pcBuff);
  
#endif
  return iRet;
}
