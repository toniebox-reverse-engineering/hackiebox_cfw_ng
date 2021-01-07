//*****************************************************************************
// adc_userinput.c 
//
// Functions for ADC USER Input
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
//! \addtogroup adc_userinput
//! @{
//
//*****************************************************************************

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "hw_memmap.h"
#include "hw_types.h"
#include "rom.h"
#include "rom_map.h"
#include "interrupt.h"
#include "prcm.h"
#include "uart_if.h"
#include "uart.h"
#include "pinmux.h"
#include "pin.h"
#include "adc_userinput.h"

#define UART_PRINT Report

//*****************************************************************************
//
//! ADC Parser Function
//!
//! This function
//!        1. Parses and gets the Configuration
//! \param ucCMD - Command Buffer
//! \out puiAdcInputPin - ADC input pin
//!
//! \return Success or Failure
//
//*****************************************************************************
bool
ADCParser( char *ucCMD,unsigned long *puiAdcInputPin)
{
    char *ucInpString;
    ucInpString = strtok(ucCMD, " ");

    //
    // Check Whether Command is valid
    //
    if((ucInpString != NULL) && ((!strcmp(ucInpString,"adcdemo"))))
    {
        ucInpString=strtok(NULL," ");
        //
        // Get ADC input pin
        //
        if(!strcmp(ucInpString,"58"))
        {
            *puiAdcInputPin = PIN_58;
        }
        else if(!strcmp(ucInpString,"59"))
        {
            *puiAdcInputPin = PIN_59;
        }
        else if(!strcmp(ucInpString,"60"))
        {
            *puiAdcInputPin = PIN_60;
        }
        else
        {
            UART_PRINT("\n\rInvalid ADC input pin\n\r");
            return false;
        }
        return true;
   }
   else
   {
       UART_PRINT("\n\rInvalid Command \n\r");
       return false;
   }
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
    //Report("*********************************************\n\r");
    UART_PRINT("Command Usage \n\r");
    UART_PRINT("-------------- \n\r");
    UART_PRINT("adcdemo <adc_inputpin>\n\r");
    UART_PRINT("\n\r");
    UART_PRINT("Parameters \n\r");
    UART_PRINT("-------------- \n\r");
    UART_PRINT("adc_inputpin - Pin to which Analog signal is connected  "
                    "[58|59|60] \n\r");
    UART_PRINT("-----------------------------------------------------------"
                "-------------- \n\r\n\r");
    UART_PRINT("\n\r");

}

//*****************************************************************************
//
//! ReadFrom User Function
//!
//! This function
//!        1. Gets Input from User
//! \out puiAdcInputPin - ADC input pin
//!
//! \return Success or Failure
//
//*****************************************************************************
bool 
ReadFromUser(unsigned long * puiAdcInputPin)
{
    char ucCmdBuffer[64];
    UsageDisplay();
    UART_PRINT("cmd# ");
    GetCmd(ucCmdBuffer,64);
    if(ADCParser(ucCmdBuffer,puiAdcInputPin))
    {
        return true;
    }
    else
    {
        return false;
    }
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
