//*****************************************************************************
// device_status.c - Device Status
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

//****************************************************************************
//
//! \addtogroup oob
//! @{
//
//****************************************************************************

#include "simplelink.h"
#include "device_status.h"


//******************************************************************************
//                            GLOBAL VARIABLES
//******************************************************************************
unsigned int g_uiPingPacketsRecv = 0;
unsigned int g_uiPingDone = 0;


//****************************************************************************
//
//!    \brief call back function for the ping test
//!
//!    \param  pPingReport is the pointer to the structure containing the result
//!         for the ping test
//!
//!    \return None
//
//****************************************************************************
void SimpleLinkPingReport(SlPingReport_t *pPingReport)
{
    g_uiPingDone = 1;
    g_uiPingPacketsRecv = pPingReport->PacketsReceived;
}


//****************************************************************************
//
//!    \brief pings to ip address of domain "www.ti.com"
//!
//! This function pings to the default gateway to ensure the wlan cannection,
//! then check for the internet connection, if present then get the ip address
//! of Domain name "www.ti.com" and pings to it
//!
//!
//!    \return -1 for unsuccessful LAN connection, -2 for problem with internet
//!         conection and 0 for succesful ping to the Domain name
//
//****************************************************************************
int ConnectionTest()
{
    int iStatus = 0;
  
    SlPingStartCommand_t PingParams;
    SlPingReport_t PingReport;
    unsigned long ulIpAddr;
    // Set the ping parameters
    PingParams.PingIntervalTime = 1000;
    PingParams.PingSize = 10;
    PingParams.PingRequestTimeout = 3000;
    PingParams.TotalNumberOfAttempts = 3;
    PingParams.Flags = 0;

    g_uiPingDone = 0;
    g_uiPingPacketsRecv = 0;


    /* Check for Internet connection */
    /* Querying for ti.com IP address */
    iStatus = sl_NetAppDnsGetHostByName((signed char *)"www.ti.com",
                                           10, &ulIpAddr, SL_AF_INET);
    if (iStatus < 0)
    {
        // LAN connection is successful
        // Problem with Internet connection
        return -2;
    }


    // Replace the ping address to match ti.com IP address
    PingParams.Ip = ulIpAddr;

    // Try to ping www.ti.com
    sl_NetAppPingStart((SlPingStartCommand_t*)&PingParams, SL_AF_INET,
             (SlPingReport_t*)&PingReport, SimpleLinkPingReport);

    while (!g_uiPingDone)
    {

    }
    
    if (g_uiPingPacketsRecv)
    {
        // LAN connection is successful
        // Internet connection is successful
        g_uiPingPacketsRecv = 0;
        return 0;
    }
    else
    {
        // LAN connection is successful
        // Problem with Internet connection
        return -2;
    }
    
}



//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
