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

#include "hw_types.h"
#include "common.h"
#include "simplelink.h"
#include "wlan.h"
#include "net.h"


//****************************************************************************
//
//! \addtogroup ota_update_os
//! @{
//
//****************************************************************************


//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
unsigned long  g_ulStatus;             // SimpleLink Status
unsigned long  g_ulGatewayIP;          // Network Gateway IP address
unsigned char  g_ucConnectionSSID[33]; // Connection SSID
unsigned char  g_ucConnectionBSSID[6]; // Connection BSSID
unsigned char  g_ucSimplelinkstarted;
unsigned long  g_ulIpAddr;


//*****************************************************************************
//
//! \brief The Function Handles WLAN Events
//!
//! \param[in]  pWlanEvent - Pointer to WLAN Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkWlanEventHandler(SlWlanEvent_t *pWlanEvent)
{
	if(!pWlanEvent)
	{
		return;
	}

    switch(pWlanEvent->Event)
    {
        case SL_WLAN_CONNECT_EVENT:
        {
            SET_STATUS_BIT(g_ulStatus, STATUS_BIT_CONNECTION);

            //
            // Information about the connected AP (like name, MAC etc) will be
            // available in 'slWlanConnectAsyncResponse_t'-Applications
            // can use it if required
            //
            //  slWlanConnectAsyncResponse_t *pEventData = NULL;
            // pEventData = &pWlanEvent->EventData.STAandP2PModeWlanConnected;
            //

            // Copy new connection SSID and BSSID to global parameters
            memcpy(g_ucConnectionSSID,pWlanEvent->EventData.
                   STAandP2PModeWlanConnected.ssid_name,
                   pWlanEvent->EventData.STAandP2PModeWlanConnected.ssid_len);
            memcpy(g_ucConnectionBSSID,
                   pWlanEvent->EventData.STAandP2PModeWlanConnected.bssid,
                   SL_BSSID_LENGTH);
        }
        break;

        case SL_WLAN_DISCONNECT_EVENT:
        {
            CLR_STATUS_BIT(g_ulStatus, STATUS_BIT_CONNECTION);
            CLR_STATUS_BIT(g_ulStatus, STATUS_BIT_IP_AQUIRED);

            memset(g_ucConnectionSSID,0,sizeof(g_ucConnectionSSID));
            memset(g_ucConnectionBSSID,0,sizeof(g_ucConnectionBSSID));
        }
        break;
    }
}

//*****************************************************************************
//
//! \brief This function handles network events such as IP acquisition, IP
//!           leased, IP released etc.
//!
//! \param[in]  pNetAppEvent - Pointer to NetApp Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkNetAppEventHandler(SlNetAppEvent_t *pNetAppEvent)
{
	if(!pNetAppEvent)
	{
		return;
	}

    switch(pNetAppEvent->Event)
    {
        case SL_NETAPP_IPV4_IPACQUIRED_EVENT:
        {
            SlIpV4AcquiredAsync_t *pEventData = NULL;

            SET_STATUS_BIT(g_ulStatus, STATUS_BIT_IP_AQUIRED);

            //Ip Acquired Event Data
            pEventData = &pNetAppEvent->EventData.ipAcquiredV4;

            g_ulIpAddr = pEventData->ip;

            //Gateway IP address
            g_ulGatewayIP = pEventData->gateway;
        }
        break;

    }
}


//*****************************************************************************
//
//! \brief This function handles HTTP server events
//!
//! \param[in]  pServerEvent - Contains the relevant event information
//! \param[in]    pServerResponse - Should be filled by the user with the
//!                                      relevant response information
//!
//! \return None
//!
//****************************************************************************
void SimpleLinkHttpServerCallback(SlHttpServerEvent_t *pHttpEvent,
                                  SlHttpServerResponse_t *pHttpResponse)
{
  return;
}

//*****************************************************************************
//
//! \brief This function handles General Events
//!
//! \param[in]     pDevEvent - Pointer to General Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkGeneralEventHandler(SlDeviceEvent_t *pDevEvent)
{
	if(!pDevEvent)
	{
		return;
	}
}


//*****************************************************************************
//
//! This function handles socket events indication
//!
//! \param[in]      pSock - Pointer to Socket Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkSockEventHandler(SlSockEvent_t *pSock)
{
  return;
}


//*****************************************************************************
//! \brief This function puts the device in its default state. It:
//!           - Set the mode to STATION
//!           - Configures connection policy to Auto and AutoSmartConfig
//!           - Deletes all the stored profiles
//!           - Enables DHCP
//!           - Disables Scan policy
//!           - Sets Tx power to maximum
//!           - Sets power policy to normal
//!           - Unregister mDNS services
//!           - Remove all filters
//!
//! \param   none
//! \return  On success, zero is returned. On error, negative is returned
//*****************************************************************************
long NetStartDefaultState()
{
    SlVersionFull   ver = {0};
    _WlanRxFilterOperationCommandBuff_t  RxFilterIdMask = {0};

    unsigned char ucVal = 1;
    unsigned char ucConfigOpt = 0;
    unsigned char ucConfigLen = 0;
    unsigned char ucPower = 0;

    long lRetVal = -1;
    long lMode = -1;

    lMode = sl_Start(0, 0, 0);

    // If the device is not in station-mode, try configuring it in station-mode
    if (ROLE_STA != lMode)
    {
        if (ROLE_AP == lMode)
        {
            // If the device is in AP mode, we need to wait for this event
            // before doing anything
            while(!IS_IP_ACQUIRED(g_ulStatus))
            {
#ifndef SL_PLATFORM_MULTI_THREADED
              _SlNonOsMainLoopTask();
#endif
            }
        }

        // Switch to STA role and restart
        lRetVal = sl_WlanSetMode(ROLE_STA);


        lRetVal = sl_Stop(0xFF);


        lRetVal = sl_Start(0, 0, 0);
        if( lRetVal != 0)
        {
          return lRetVal;
        }


        // Check if the device is in station again
        if (ROLE_STA != lRetVal)
        {
            // We don't want to proceed if the device is not coming up in
            // STA-mode
            LOOP_FOREVER();
        }
    }

    // Get the device's version-information
    ucConfigOpt = SL_DEVICE_GENERAL_VERSION;
    ucConfigLen = sizeof(ver);
    lRetVal = sl_DevGet(SL_DEVICE_GENERAL_CONFIGURATION, &ucConfigOpt,
                                &ucConfigLen, (unsigned char *)(&ver));

    if( lRetVal != 0)
    {
      return lRetVal;
    }


    // Set connection policy to Auto + SmartConfig
    //      (Device's default connection policy)
    lRetVal = sl_WlanPolicySet(SL_POLICY_CONNECTION,
                                SL_CONNECTION_POLICY(1, 0, 0, 0, 1), NULL, 0);
    if( lRetVal != 0)
    {
      return lRetVal;
    }


    // Remove all profiles
    lRetVal = sl_WlanProfileDel(0xFF);
    if( lRetVal != 0)
    {
      return lRetVal;
    }

    //
    // Device in station-mode. Disconnect previous connection if any
    // The function returns 0 if 'Disconnected done', negative number if already
    // disconnected Wait for 'disconnection' event if 0 is returned, Ignore
    // other return-codes
    //
    lRetVal = sl_WlanDisconnect();
    if(0 == lRetVal)
    {
        // Wait
        while(IS_CONNECTED(g_ulStatus))
        {
#ifndef SL_PLATFORM_MULTI_THREADED
              _SlNonOsMainLoopTask();
#endif
        }
    }

    // Enable DHCP client
    lRetVal = sl_NetCfgSet(SL_IPV4_STA_P2P_CL_DHCP_ENABLE,1,1,&ucVal);

    if( lRetVal != 0)
    {
      return lRetVal;
    }


    // Disable scan
    ucConfigOpt = SL_SCAN_POLICY(0);
    lRetVal = sl_WlanPolicySet(SL_POLICY_SCAN , ucConfigOpt, NULL, 0);

    if( lRetVal != 0)
    {
      return lRetVal;
    }


    // Set Tx power level for station mode
    // Number between 0-15, as dB offset from max power - 0 will set max power
    ucPower = 0;
    lRetVal = sl_WlanSet(SL_WLAN_CFG_GENERAL_PARAM_ID,
            WLAN_GENERAL_PARAM_OPT_STA_TX_POWER, 1, (unsigned char *)&ucPower);

    if( lRetVal != 0)
    {
      return lRetVal;
    }


    // Set PM policy to normal
    lRetVal = sl_WlanPolicySet(SL_POLICY_PM , SL_NORMAL_POLICY, NULL, 0);

    if( lRetVal != 0)
    {
      return lRetVal;
    }

    // Unregister mDNS services
    lRetVal = sl_NetAppMDNSUnRegisterService(0, 0);

    if( lRetVal != 0)
    {
      return lRetVal;
    }


    // Remove  all 64 filters (8*8)
    memset(RxFilterIdMask.FilterIdMask, 0xFF, 8);
    lRetVal = sl_WlanRxFilterSet(SL_REMOVE_RX_FILTER, (_u8 *)&RxFilterIdMask,
                       sizeof(_WlanRxFilterOperationCommandBuff_t));

    if( lRetVal != 0)
    {
      return lRetVal;
    }


    //
    // Initialize the variables
    //
    g_ulStatus = 0;
    g_ulGatewayIP = 0;
    g_ucSimplelinkstarted = 0;
    g_ulIpAddr = 0;

    return lRetVal;
}

void NetStop()
{
    unsigned long lRetVal;

    //
    // Disconnect from the AP
    //
    lRetVal = sl_WlanDisconnect();
    if(0 == lRetVal)
    {
        // Wait
        while(IS_CONNECTED(g_ulStatus))
        {
#ifndef SL_PLATFORM_MULTI_THREADED
              _SlNonOsMainLoopTask();
#endif
        }
    }

    //
    // Stop the simplelink host
    //
    sl_Stop(SL_STOP_TIMEOUT);

    //
    // Reset all variables
    //
    g_ulStatus = 0;
    g_ulGatewayIP = 0;
    g_ucSimplelinkstarted = 0;
    g_ulIpAddr = 0;
}

//****************************************************************************
//
//!  \brief Connecting to a WLAN Accesspoint
//!
//!   This function connects to the required AP (SSID_NAME) with Security
//!   parameters specified in te form of macros at the top of this file
//!
//!   \param[in]              None
//!
//!   \return       status value
//!
//!   \warning    If the WLAN connection fails or we don't aquire an IP
//!            address, It will be stuck in this function forever.
//
//****************************************************************************
long NetWlanConnect(const char *cssid, SlSecParams_t *secParams)
{
    long lRetVal = 0;

    lRetVal = sl_WlanConnect((signed char *)cssid, strlen(cssid), 0, secParams, 0);
    if( lRetVal != 0)
    {
      return lRetVal;
    }

    while((!IS_CONNECTED(g_ulStatus)) || (!IS_IP_ACQUIRED(g_ulStatus)))
    {
        // Wait for WLAN Event
#ifndef SL_PLATFORM_MULTI_THREADED
              _SlNonOsMainLoopTask();
#endif

    }

    return SUCCESS;

}

//*****************************************************************************
//
//! Network_IF_GetHostIP
//!
//! \brief  This function obtains the server IP address using a DNS lookup
//!
//! \param[in]  pcHostName        The server hostname
//! \param[out] pDestinationIP    This parameter is filled with host IP address.
//!
//! \return On success, +ve value is returned. On error, -ve value is returned
//!
//
//*****************************************************************************
long NetGetHostIP( char* pcHostName,unsigned long * pDestinationIP )
{
    long lStatus = 0;

    lStatus = sl_NetAppDnsGetHostByName((signed char *) pcHostName,
                                        strlen(pcHostName),
    					pDestinationIP, SL_AF_INET);

    return lStatus;

}

//****************************************************************************
//
//! Get the device mac address
//!
//! \brief This function gets the MAC address of the device
//!
//! \param pMACAddress is pointer to variable where MAC address will be stored
//!
//! \return None.
//
//****************************************************************************
void NetMACAddressGet(unsigned char *pMACAddress)
{
    unsigned char macAddressLen = SL_MAC_ADDR_LEN;

    //
    // Get the MAC address
    //
    sl_NetCfgGet(SL_MAC_ADDRESS_GET, NULL, &macAddressLen, pMACAddress);

}

//*****************************************************************************
//
//! \brief The Function gets the network firmware info
//!
//! \param[out]  sVersion - Pointer to SlVersionFull Info structure
//!
//! \return None
//!
//*****************************************************************************
long NetFwInfoGet(SlVersionFull *sVersion)
{
  unsigned long lRetVal;
  unsigned char ucConfigOpt = 0;
  unsigned char ucConfigLen = 0;

  //
  // Get the device's version-information
  //
  ucConfigOpt = SL_DEVICE_GENERAL_VERSION;
  ucConfigLen = sizeof(SlVersionFull);
  lRetVal = sl_DevGet(SL_DEVICE_GENERAL_CONFIGURATION, &ucConfigOpt,
                              &ucConfigLen, (unsigned char *)(sVersion));

  return lRetVal;
}

//*****************************************************************************
//
//! \brief The Function check if wifi is connected to AP 
//!
//!
//! \return Returns \b true if connected, \b false otherwise.
//!
//*****************************************************************************
tBoolean NetIsConnectedToAP()
{
  return IS_CONNECTED(g_ulStatus);
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
