//*****************************************************************************
// speaker.c
//
// LINE OUT (Speaker Operation)
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

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
extern tUDPSocket g_UdpSock;
int g_iReceiveCount =0;
int g_iRetVal =0;
int iCount =0;
extern unsigned long  g_ulStatus;
extern unsigned char g_ucSpkrStartFlag;
extern unsigned char g_loopback;
unsigned char speaker_data[16*1024];
extern tCircularBuffer *pPlayBuffer;
//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************


//*****************************************************************************
//
//! Speaker Routine 
//!
//! \param pvParameters     Parameters to the task's entry function
//!
//! \return None
//
//*****************************************************************************
void Speaker( void *pvParameters )
{
    long iRetVal = -1;


    while(1)
    {
        while(g_ucSpkrStartFlag || g_loopback)
        {     

            if(!g_loopback)
            {
                fd_set readfds,writefds;
                struct SlTimeval_t tv;
                FD_ZERO(&readfds);
                FD_ZERO(&writefds);
                FD_SET(g_UdpSock.iSockDesc,&readfds);
                FD_SET(g_UdpSock.iSockDesc,&writefds);
                tv.tv_sec = 0;
                tv.tv_usec = 2000000;
                int rv = select(g_UdpSock.iSockDesc, &readfds, NULL, NULL, &tv);
                if(rv <= 0)
                {
                    continue;
                }
                if (FD_ISSET(g_UdpSock.iSockDesc, &readfds) )
                {
                    g_iRetVal = recvfrom(g_UdpSock.iSockDesc, (char*)(speaker_data),\
                                          PACKET_SIZE*16, 0,\
                                          (struct sockaddr *)&(g_UdpSock.Client),\
                                          (SlSocklen_t*)&(g_UdpSock.iClientLength));
                }

                if(g_iRetVal>0)
                {
                    iRetVal = FillBuffer(pPlayBuffer, (unsigned char*)speaker_data,\
                                          g_iRetVal);
                    if(iRetVal < 0)
                    {
                        UART_PRINT("Unable to fill buffer");
                        LOOP_FOREVER();
                    }
                }
            }
            else
            {
                MAP_UtilsDelay(1000);
            }
        }
        MAP_UtilsDelay(1000);
    }
}

