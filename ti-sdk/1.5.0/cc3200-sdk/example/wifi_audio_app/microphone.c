//*****************************************************************************
// Microphone.c
//
// Line IN (Microphone interface)
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

// Hardware & driverlib library includes
#include "rom.h"
#include "rom_map.h"
#include "utils.h"
#include "hw_ints.h"

// simplelink include
#include "simplelink.h"

// common interface includes
#include "common.h"
#include "uart_if.h"
// Demo app includes
#include "network.h"
#include "circ_buff.h"
#include "audioCodec.h"

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************

extern tUDPSocket g_UdpSock;
int g_iSentCount =0;
unsigned long g_ulConnected = 0; 
extern int g_iReceiveCount;
extern unsigned long  g_ulStatus;
extern volatile unsigned char g_ucMicStartFlag;
extern unsigned char g_loopback;

extern tCircularBuffer *pPlayBuffer;
extern tCircularBuffer *pRecordBuffer;


//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************


#ifdef MULTICAST
//*****************************************************************************
//
//! Send Multicast Packet 
//!
//! \param none
//!
//! \return  0 - Success
//!            -1 - Failure
//
//*****************************************************************************
long SendMulticastPacket()
{
    long lRetVal = -1;
    int uiPort = 5050;
    struct sockaddr_in stClient;
    stClient.sin_family = AF_INET;
    stClient.sin_addr.s_addr = htonl(ADDR_MULTICAST_GROUP);
    stClient.sin_port = htons(uiPort);
  
    lRetVal = sendto(g_UdpSock.iSockDesc, (char*)(pRecordBuffer->pucReadPtr),\
                      PACKET_SIZE, 0,(struct sockaddr*)&(stClient),\
                      sizeof(stClient));    
    ASSERT_ON_ERROR(lRetVal);
    
    return SUCCESS;
}
#endif

//*****************************************************************************
//
//! Microphone Routine 
//!
//! \param pvParameters     Parameters to the task's entry function
//!
//! \return None
//
//*****************************************************************************

void Microphone( void *pvParameters )
{
    long lRetVal = -1;

#ifdef MULTICAST
    //Wait for Network Connection
    while((!IS_IP_ACQUIRED(g_ulStatus)))
    {

    }
#endif //MULTICAST

    while(1)
    {     
        while(g_ucMicStartFlag || g_loopback)
        {
            int iBufferFilled = 0;
            iBufferFilled = GetBufferSize(pRecordBuffer);
            if(iBufferFilled >= (2*PACKET_SIZE))
            { 
                if(!g_loopback)
                {

#ifndef MULTICAST          
                    lRetVal = sendto(g_UdpSock.iSockDesc, \
                                       (char*)(pRecordBuffer->pucReadPtr),PACKET_SIZE,\
                                       0,(struct sockaddr*)&(g_UdpSock.Client),\
                                       sizeof(g_UdpSock.Client));
                    if(lRetVal < 0)
                    {
                        UART_PRINT("Unable to send data\n\r");
                        LOOP_FOREVER();
                    }

#else	//MULTICAST
                    lRetVal = SendMulticastPacket();
                    if(lRetVal < 0)
                    {
                        UART_PRINT("Unable to send data\n\r");
                        LOOP_FOREVER();
                    }

#endif     //MULTICAST      
                }
                else
                {
                    lRetVal = FillBuffer(pPlayBuffer,\
                                          (unsigned char*)(pRecordBuffer->pucReadPtr), \
                                          PACKET_SIZE);
                    if(lRetVal < 0)
                    {
                        UART_PRINT("Unable to fill buffer\n\r");
                    }
                    g_iReceiveCount++;
                }
                UpdateReadPtr(pRecordBuffer, PACKET_SIZE);
                g_iSentCount++;
            }
        }      
        MAP_UtilsDelay(1000);
    }
}
