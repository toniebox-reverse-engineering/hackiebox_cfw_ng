//*****************************************************************************
// crc_userinput.c
//
// Functions for CRC USER Input
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
//! \addtogroup crc_api
//! @{
//
//*****************************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "hw_dthe.h"
#include "hw_memmap.h"
#include "hw_types.h"
#include "rom.h"
#include "rom_map.h"
#include "crc.h"
#include "uart.h"
#include "interrupt.h"
#include "prcm.h"
#include "udma.h"
#include "crc_userinput.h"
#include "uart_if.h"

#define UART_PRINT Report
#define CCM0_BASE DTHE_BASE
#define INPUT_MESSAGE_SIZE  520

unsigned int puiPlainMsg[INPUT_MESSAGE_SIZE];

//*****************************************************************************
//
//! CRCParser 
//!
//! \param  ucCMDBuffer - Buffer which contains command from UART
//! \param  uiConfig - Sets Configuration Value
//!
//! \return none
//!
//*****************************************************************************
bool
CRCParser( char *ucCMDBuffer,unsigned int *uiConfig)
{
    char *ucInpString,cChar;
    ucInpString = strtok(ucCMDBuffer, " ");

    //
    // Check Whether Command is valid
    //
    if((ucInpString != NULL) && ((!strcmp(ucInpString,"crc"))))
    {
        //
        // Get which Algorithm you are using
        //
        ucInpString = strtok(NULL, " ");

        //
        // Get Key Length
        //
        if(ucInpString != NULL && ((!strcmp(ucInpString,"P8005")) || \
                            (!strcmp(ucInpString,"p8005"))))
            *uiConfig=CRC_CFG_TYPE_P8005;
        else if (ucInpString != NULL && (!strcmp(ucInpString,"P1021") || \
                            (!strcmp(ucInpString,"p1021"))))
            *uiConfig=CRC_CFG_TYPE_P1021;
        else if (ucInpString != NULL && (!strcmp(ucInpString,"P4C11DB7") || \
                            !strcmp(ucInpString,"p4c11db7")))
            *uiConfig=CRC_CFG_TYPE_P4C11DB7;
        else if (ucInpString != NULL && (!strcmp(ucInpString,"P1EDC6F41") || \
                        !strcmp(ucInpString,"p1edc6f41") ))
            *uiConfig=CRC_CFG_TYPE_P1EDC6F41;
        else if (ucInpString != NULL && (!strcmp(ucInpString,"TCPCHKSUM") || \
                        !strcmp(ucInpString,"tcpchksum")))
            *uiConfig=CRC_CFG_TYPE_TCPCHKSUM;
        else
        {
            UART_PRINT("\n\r Invalid CRC type \n\r");
            return false;
        }

        UART_PRINT("\n\rPress 1 to Initialize Seed\n\rPress 2 to Initialize to"
                       "all 0's\n\rPress 3 to Initialize to all 1's \n\r");
     
        //
        // Get the option
        //
        cChar = MAP_UARTCharGet(UARTA0_BASE);

        //
        // Echo the received character
        //
        MAP_UARTCharPut(UARTA0_BASE, cChar);
        UART_PRINT("\n\r");
        if(cChar=='1' )
        {
            *uiConfig|=CRC_CFG_INIT_SEED;
        }
        else if(cChar=='2')
        {
            *uiConfig|=CRC_CFG_INIT_0;
        }
        else if(cChar=='3')
        {
            *uiConfig|=CRC_CFG_INIT_1;
        }
        else
        {
            UART_PRINT("\n\r Invalid Input \n\r");
            return false;
        }
        UART_PRINT("\n\rPress 1 for byte write\n\rPress 2 for word write \n\r");

        //
        // Get the option
        //
        cChar = MAP_UARTCharGet(UARTA0_BASE);

        //
        // Echo the received character
        //
        MAP_UARTCharPut(UARTA0_BASE, cChar);
        UART_PRINT("\n\r");
        if(cChar=='1' )
        {
            *uiConfig|=CRC_CFG_SIZE_8BIT;

        }
        else if(cChar=='2')
        {
            *uiConfig|=CRC_CFG_SIZE_32BIT;

        }
        else
        {
            UART_PRINT("\n\r Invalid Input \n\r");
            return false;
        }
#if 0
        UART_PRINT("\n\rPress 1 for Result Inverse\n\rPress 2 for Output "
                        "Inverse \n\r");

        //
        // Get the option
        //
        cChar = MAP_UARTCharGet(UARTA0_BASE);

        //
        // Echo the received character
        //
        MAP_UARTCharPut(UARTA0_BASE, cChar);
        UART_PRINT("\n\r");
        if(cChar=='1' )
        {
        *uiConfig|=CRC_CFG_RESINV;
        }
        else if(cChar=='2')
        {
        *uiConfig|=CRC_CFG_OBR;
        }
        else
        {
        UART_PRINT("\n\r Invalid Input \n\r");
        return false;
        }
#endif
    }
    else
    {
        return false;
    }
    return true;
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
    UART_PRINT("crc <crc_mode> - Generates the CRC Result\n\r");
    UART_PRINT("\n\r");
    UART_PRINT("Parameters \n\r");
    UART_PRINT("---------- \n\r");
    UART_PRINT("crc_mode - Algorithm to be selected by the user [P8005|P1021|"
                    "P4C11DB7|P1EDC6F41|TCPCHKSUM] \n\r");
    UART_PRINT("--------------------------------------------------------------"
                "----------------- \n\r\n\r");
    UART_PRINT("\n\r");

}
//*****************************************************************************
//
//! ReadFromUser - Populate the parameters from User
//!
//! \param uiConfig - Configuration Value
//! \param uiDataLength- Datalength 
//! \param puiResult - CRC Result
//!
//! \return pointer to Input Data
//!
//*****************************************************************************
unsigned int*
ReadFromUser(unsigned int *uiConfig,unsigned int *uiDataLength,
                unsigned int *puiResult)
{

    char ucCmdBuffer[INPUT_MESSAGE_SIZE],*pucMsgBuff;
    unsigned int *uiData;
    pucMsgBuff=( char*)&puiPlainMsg[0];

    //
    // Get the Command
    //
    UsageDisplay();
    UART_PRINT("cmd# ");
    GetCmd(ucCmdBuffer,INPUT_MESSAGE_SIZE);
    if(CRCParser(ucCmdBuffer,uiConfig))
    {
        uiData=GetMsg(pucMsgBuff,uiDataLength,uiConfig);
        memset(puiResult,0,sizeof(unsigned int));
    }
    else
    {
        return NULL;
    }
    return uiData;
}

//*****************************************************************************
//
//! GetMsg - Get the message from User
//!
//! \param pucMsgBuff - Message Buffer into which Message is populated
//! \param uiDataLength- Datalength 
//! \param uiConfig - Configuraion Value
//!
//! \return pointer to Input Data
//!
//*****************************************************************************
unsigned int*
GetMsg(char *pucMsgBuff,unsigned int *uiDataLength,unsigned int *uiConfig)
{
    unsigned int uiMsgLen = 0;
    unsigned int *uiData;
    UART_PRINT("\n\r Enter the Message \n\r");
    uiMsgLen=GetCmd(pucMsgBuff, INPUT_MESSAGE_SIZE);
    if(*uiConfig & CRC_CFG_SIZE_8BIT)
    {
    	*uiDataLength = uiMsgLen;
    	uiData=(unsigned int *)malloc(*uiDataLength);
    	memset(uiData,0,*uiDataLength);
    	memcpy(uiData,pucMsgBuff,*uiDataLength);
    }
    else
    {
    	*uiDataLength = (uiMsgLen+3)/4;
    	uiData=(unsigned int *)malloc((*uiDataLength) * sizeof(unsigned int));
    	memset(uiData,0,((*uiDataLength) * sizeof(unsigned int)));
    	memcpy(uiData,pucMsgBuff,uiMsgLen);
    }
    return uiData;
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

