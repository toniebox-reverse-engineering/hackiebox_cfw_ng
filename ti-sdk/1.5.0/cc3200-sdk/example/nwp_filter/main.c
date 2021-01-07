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


//*****************************************************************************
//
// Application Name     -   NWP Filter Application
// Application Overview -   This sample application demonstrates how one can
//                          define and manage the rx filtering process. This
//                          application filters the input traffic based on MAC
//                          address and IP address.
//
//*****************************************************************************


//*****************************************************************************
//
//! \addtogroup nwp_filter
//! @{
//
//*****************************************************************************

#include <stdio.h>

// Simplelink includes
#include "simplelink.h"

//Driverlib includes
#include "hw_types.h"
#include "hw_ints.h"
#include "rom.h"
#include "rom_map.h"
#include "interrupt.h"
#include "prcm.h"
#include "utils.h"
#include "pinmux.h"

//Common interface includes
#include "common.h"
#ifndef NOTERM
#include "uart_if.h"
#endif


#define APPLICATION_VERSION     "1.4.0"
#define IP_ADDR_LENGTH      4
#define PORT_NUM            5001
#define BUF_SIZE            1400
#define PACKET_COUNT        100

// Application specific status/error codes
typedef enum{
    // Choosing -0x7D0 to avoid overlap w/ host-driver's error codes
    LAN_CONNECTION_FAILED = -0x7D0,
    INTERNET_CONNECTION_FAILED = LAN_CONNECTION_FAILED - 1,
    DEVICE_NOT_IN_STATION_MODE = INTERNET_CONNECTION_FAILED - 1,

    STATUS_CODE_MAX = -0xBB8
}e_AppStatusCodes;


//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
volatile unsigned long  g_ulStatus = 0;//SimpleLink Status
unsigned long  g_ulGatewayIP = 0; //Network Gateway IP address
unsigned char  g_ucConnectionSSID[SSID_LEN_MAX+1]; //Connection SSID
unsigned char  g_ucConnectionBSSID[BSSID_LEN_MAX]; //Connection BSSID
char g_cRx_Buffer[16]; // IP address
// MAC address to be filtered out
unsigned char g_ucMacAddress[SL_MAC_ADDR_LEN] = {0xC0, 0xCB, 0x38, 0x1D, 0x09,
                                                 0x17};
// IP address to be filtered out
unsigned char g_ucIpAddress[IP_ADDR_LENGTH] = {0xC0, 0xA8, 0x00, 0x68};

union
{
    char cBsdBuf[BUF_SIZE];
    unsigned int uiDemoBuf[BUF_SIZE/4];
}g_uBuf;

#if defined(ccs) || defined(gcc)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif
//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************


//****************************************************************************
//                      LOCAL FUNCTION PROTOTYPES
//****************************************************************************
static void InitializeAppVariables();
static long ConfigureSimpleLinkToDefaultState();
static long WlanConnect();
static int CreateFilters();
static signed long EnableAllFilters();
static long BsdTcpServer(unsigned short Port);
static int NwpFilter(void *pvParameters);
static void BoardInit(void);

//*****************************************************************************
// SimpleLink Asynchronous Event Handlers -- Start
//*****************************************************************************


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

            UART_PRINT("[WLAN EVENT] STA Connected to the AP: %s ,"
                        "BSSID: %x:%x:%x:%x:%x:%x\n\r",
                      g_ucConnectionSSID,g_ucConnectionBSSID[0],
                      g_ucConnectionBSSID[1],g_ucConnectionBSSID[2],
                      g_ucConnectionBSSID[3],g_ucConnectionBSSID[4],
                      g_ucConnectionBSSID[5]);
        }
        break;

        case SL_WLAN_DISCONNECT_EVENT:
        {
            slWlanConnectAsyncResponse_t*  pEventData = NULL;

            CLR_STATUS_BIT(g_ulStatus, STATUS_BIT_CONNECTION);
            CLR_STATUS_BIT(g_ulStatus, STATUS_BIT_IP_AQUIRED);

            pEventData = &pWlanEvent->EventData.STAandP2PModeDisconnected;

            // If the user has initiated 'Disconnect' request,
            //'reason_code' is SL_WLAN_DISCONNECT_USER_INITIATED_DISCONNECTION
            if(SL_WLAN_DISCONNECT_USER_INITIATED_DISCONNECTION == pEventData->reason_code)
            {
                UART_PRINT("[WLAN EVENT]Device disconnected from the AP: %s,"
                    "BSSID: %x:%x:%x:%x:%x:%x on application's request \n\r",
                           g_ucConnectionSSID,g_ucConnectionBSSID[0],
                           g_ucConnectionBSSID[1],g_ucConnectionBSSID[2],
                           g_ucConnectionBSSID[3],g_ucConnectionBSSID[4],
                           g_ucConnectionBSSID[5]);
            }
            else
            {
                UART_PRINT("[WLAN ERROR]Device disconnected from the AP AP: %s,"
                            "BSSID: %x:%x:%x:%x:%x:%x on an ERROR..!! \n\r",
                           g_ucConnectionSSID,g_ucConnectionBSSID[0],
                           g_ucConnectionBSSID[1],g_ucConnectionBSSID[2],
                           g_ucConnectionBSSID[3],g_ucConnectionBSSID[4],
                           g_ucConnectionBSSID[5]);
            }
            memset(g_ucConnectionSSID,0,sizeof(g_ucConnectionSSID));
            memset(g_ucConnectionBSSID,0,sizeof(g_ucConnectionBSSID));
        }
        break;

        default:
        {
            UART_PRINT("[WLAN EVENT] Unexpected event [0x%x]\n\r",
                       pWlanEvent->Event);
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
    switch(pNetAppEvent->Event)
    {
        case SL_NETAPP_IPV4_IPACQUIRED_EVENT:
        {
            SlIpV4AcquiredAsync_t *pEventData = NULL;

            SET_STATUS_BIT(g_ulStatus, STATUS_BIT_IP_AQUIRED);

            //Ip Acquired Event Data
            pEventData = &pNetAppEvent->EventData.ipAcquiredV4;

            //Gateway IP address
            g_ulGatewayIP = pEventData->gateway;

            sprintf(g_cRx_Buffer, "%d.%d.%d.%d",
                    SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip,3),
                    SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip,2),
                    SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip,1),
                    SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip,0));

            UART_PRINT("[NETAPP EVENT] IP Acquired: IP=%d.%d.%d.%d , "
                        "Gateway=%d.%d.%d.%d\n\r",
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip,3),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip,2),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip,1),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip,0),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.gateway,3),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.gateway,2),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.gateway,1),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.gateway,0));
        }
        break;

        default:
        {
            UART_PRINT("[NETAPP EVENT] Unexpected event [0x%x] \n\r",
                       pNetAppEvent->Event);
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
    // Unused in this application
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
    //
    // Most of the general errors are not FATAL are are to be handled
    // appropriately by the application
    //
    UART_PRINT("[GENERAL EVENT] - ID=[%d] Sender=[%d]\n\n",
               pDevEvent->EventData.deviceEvent.status,
               pDevEvent->EventData.deviceEvent.sender);
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
    //
    // This application doesn't work w/ socket - Events are not expected
    //
    switch( pSock->Event )
    {
        case SL_SOCKET_TX_FAILED_EVENT:
            switch( pSock->socketAsyncEvent.SockTxFailData.status)
            {
                case SL_ECLOSE: 
                    UART_PRINT("[SOCK ERROR] - close socket (%d) operation "
                                "failed to transmit all queued packets\n\n", 
                                    pSock->socketAsyncEvent.SockTxFailData.sd);
                    break;
                default: 
                    UART_PRINT("[SOCK ERROR] - TX FAILED  :  socket %d , reason "
                                "(%d) \n\n",
                                pSock->socketAsyncEvent.SockTxFailData.sd, pSock->socketAsyncEvent.SockTxFailData.status);
                  break;
            }
            break;

        default:
        	UART_PRINT("[SOCK EVENT] - Unexpected Event [%x0x]\n\n",pSock->Event);
          break;
    }

}


//*****************************************************************************
// SimpleLink Asynchronous Event Handlers -- End
//*****************************************************************************


//*****************************************************************************
//
//! \brief This function initializes the application variables
//!
//! \param    None
//!
//! \return None
//!
//*****************************************************************************
static void InitializeAppVariables()
{
    g_ulStatus = 0;
    g_ulGatewayIP = 0;
    memset(g_ucConnectionSSID,0,sizeof(g_ucConnectionSSID));
    memset(g_ucConnectionBSSID,0,sizeof(g_ucConnectionBSSID));
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
static long ConfigureSimpleLinkToDefaultState()
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
    ASSERT_ON_ERROR(lMode);

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
        ASSERT_ON_ERROR(lRetVal);

        lRetVal = sl_Stop(0xFF);
        ASSERT_ON_ERROR(lRetVal);

        lRetVal = sl_Start(0, 0, 0);
        ASSERT_ON_ERROR(lRetVal);

        // Check if the device is in station again 
        if (ROLE_STA != lRetVal)
        {
            // We don't want to proceed if the device is not coming up in STA-mode 
            return DEVICE_NOT_IN_STATION_MODE;
        }
    }
    
    // Get the device's version-information
    ucConfigOpt = SL_DEVICE_GENERAL_VERSION;
    ucConfigLen = sizeof(ver);
    lRetVal = sl_DevGet(SL_DEVICE_GENERAL_CONFIGURATION, &ucConfigOpt, 
                                &ucConfigLen, (unsigned char *)(&ver));
    ASSERT_ON_ERROR(lRetVal);
    
    UART_PRINT("Host Driver Version: %s\n\r",SL_DRIVER_VERSION);
    UART_PRINT("Build Version %d.%d.%d.%d.31.%d.%d.%d.%d.%d.%d.%d.%d\n\r",
    ver.NwpVersion[0],ver.NwpVersion[1],ver.NwpVersion[2],ver.NwpVersion[3],
    ver.ChipFwAndPhyVersion.FwVersion[0],ver.ChipFwAndPhyVersion.FwVersion[1],
    ver.ChipFwAndPhyVersion.FwVersion[2],ver.ChipFwAndPhyVersion.FwVersion[3],
    ver.ChipFwAndPhyVersion.PhyVersion[0],ver.ChipFwAndPhyVersion.PhyVersion[1],
    ver.ChipFwAndPhyVersion.PhyVersion[2],ver.ChipFwAndPhyVersion.PhyVersion[3]);

    // Set connection policy to Auto + SmartConfig 
    //      (Device's default connection policy)
    lRetVal = sl_WlanPolicySet(SL_POLICY_CONNECTION, 
                                SL_CONNECTION_POLICY(1, 0, 0, 0, 1), NULL, 0);
    ASSERT_ON_ERROR(lRetVal);

    // Remove all profiles
    lRetVal = sl_WlanProfileDel(0xFF);
    ASSERT_ON_ERROR(lRetVal);

    

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
    ASSERT_ON_ERROR(lRetVal);

    // Disable scan
    ucConfigOpt = SL_SCAN_POLICY(0);
    lRetVal = sl_WlanPolicySet(SL_POLICY_SCAN , ucConfigOpt, NULL, 0);
    ASSERT_ON_ERROR(lRetVal);

    // Set Tx power level for station mode
    // Number between 0-15, as dB offset from max power - 0 will set max power
    ucPower = 0;
    lRetVal = sl_WlanSet(SL_WLAN_CFG_GENERAL_PARAM_ID, 
            WLAN_GENERAL_PARAM_OPT_STA_TX_POWER, 1, (unsigned char *)&ucPower);
    ASSERT_ON_ERROR(lRetVal);

    // Set PM policy to normal
    lRetVal = sl_WlanPolicySet(SL_POLICY_PM , SL_NORMAL_POLICY, NULL, 0);
    ASSERT_ON_ERROR(lRetVal);

    // Unregister mDNS services
    lRetVal = sl_NetAppMDNSUnRegisterService(0, 0);
    ASSERT_ON_ERROR(lRetVal);

    // Remove  all 64 filters (8*8)
    memset(RxFilterIdMask.FilterIdMask, 0xFF, 8);
    lRetVal = sl_WlanRxFilterSet(SL_REMOVE_RX_FILTER, (_u8 *)&RxFilterIdMask,
                       sizeof(_WlanRxFilterOperationCommandBuff_t));
    ASSERT_ON_ERROR(lRetVal);

    lRetVal = sl_Stop(SL_STOP_TIMEOUT);
    ASSERT_ON_ERROR(lRetVal);

    InitializeAppVariables();
    
    return lRetVal; // Success
}


//****************************************************************************
//
//! \brief Connecting to a WLAN Accesspoint
//!
//!  This function connects to the required AP (SSID_NAME) with Security
//!  parameters specified in te form of macros at the top of this file
//!
//! \param  None
//!
//! \return  None
//!
//! \warning    If the WLAN connection fails or we don't aquire an IP
//!            address, It will be stuck in this function forever.
//
//****************************************************************************
static long WlanConnect()
{
    SlSecParams_t secParams = {0};
    long lRetVal = 0;

    secParams.Key = SECURITY_KEY;
    secParams.KeyLen = strlen(SECURITY_KEY);
    secParams.Type = SECURITY_TYPE;

    lRetVal = sl_WlanConnect(SSID_NAME, strlen(SSID_NAME), 0, &secParams, 0);
    ASSERT_ON_ERROR(lRetVal);

    // Wait for WLAN Event
    while((!IS_CONNECTED(g_ulStatus))|| (!IS_IP_ACQUIRED(g_ulStatus)))
    {
        _SlNonOsMainLoopTask();
    }

    return SUCCESS;
}

//*****************************************************************************
//
//!This function creates rx filters. Two filters are defined in this example:
//!(1) Drop incoming packets according to source MAC address
//!(2) Drop incoming packets according to source IP address
//!
//! \param None
//!
//! \return 0 on success, -1 on failure
//!
//*****************************************************************************
static int CreateFilters()
{
    long                 lRetVal;
    SlrxFilterID_t              FilterId = 0;
    SlrxFilterRuleType_t         RuleType;
    SlrxFilterFlags_t             FilterFlags;
    SlrxFilterRule_t            Rule;
    SlrxFilterTrigger_t         Trigger;
    SlrxFilterAction_t          Action;

    unsigned char ucMacMask[6]    = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    unsigned char ucIPMask[4]    = {0xFF, 0xFF, 0xFF, 0xFF};

    /*************************************************************************/
    /* Build filter to drop incoming packets according to source MAC address */
    /*************************************************************************/

    //
    //define filter as parent
    //
    Trigger.ParentFilterID = 0; 
    
    //
    //no trigger to activate the filter.
    //
    Trigger.Trigger = NO_TRIGGER; 
    
    //
    //connection state and role
    //
    Trigger.TriggerArgConnectionState.IntRepresentation = \
                                    RX_FILTER_CONNECTION_STATE_STA_CONNECTED;
    Trigger.TriggerArgRoleStatus.IntRepresentation = RX_FILTER_ROLE_STA;
    
    //
    // header and Combination types are only supported. Combination for logical 
    // AND/OR between two filters
    //
    RuleType = HEADER; 
    Rule.HeaderType.RuleHeaderfield = MAC_SRC_ADDRESS_FIELD;
    memcpy( Rule.HeaderType.RuleHeaderArgsAndMask.RuleHeaderArgs.RxFilterDB6BytesRuleArgs[0], 
            g_ucMacAddress , SL_MAC_ADDR_LEN);
    memcpy( Rule.HeaderType.RuleHeaderArgsAndMask.RuleHeaderArgsMask, ucMacMask,
                SL_MAC_ADDR_LEN);
    Rule.HeaderType.RuleCompareFunc = COMPARE_FUNC_EQUAL;
    
    //
    //Action
    //
    Action.ActionType.IntRepresentation = RX_FILTER_ACTION_DROP;
    FilterFlags.IntRepresentation = RX_FILTER_BINARY;
    lRetVal = sl_WlanRxFilterAdd(RuleType, FilterFlags, &Rule, &Trigger,
                                    &Action, &FilterId);
    ASSERT_ON_ERROR(lRetVal);

    /*************************************************************************/
    /* Build filter to drop incoming packets according to source IP address  */
    /*************************************************************************/
    
    //
    //define filter as parent
    //
    Trigger.ParentFilterID = 0; 
    
    //
    //no trigger to activate the filter.
    //
    Trigger.Trigger = NO_TRIGGER; 
    
    //
    //connection state and role
    //
    Trigger.TriggerArgConnectionState.IntRepresentation = \
                                        RX_FILTER_CONNECTION_STATE_STA_CONNECTED;
    Trigger.TriggerArgRoleStatus.IntRepresentation = RX_FILTER_ROLE_STA;
    
    //
    // header and Combination types are only supported. Combination for logical 
    // AND/OR between two filters
    //
    RuleType = HEADER; 
    Rule.HeaderType.RuleHeaderfield = IPV4_SRC_ADRRESS_FIELD;
    memcpy(Rule.HeaderType.RuleHeaderArgsAndMask.RuleHeaderArgs.RxFilterDB4BytesRuleArgs[0],
                g_ucIpAddress , IP_ADDR_LENGTH);
    memcpy(Rule.HeaderType.RuleHeaderArgsAndMask.RuleHeaderArgsMask, ucIPMask,
                IP_ADDR_LENGTH);
    Rule.HeaderType.RuleCompareFunc = COMPARE_FUNC_EQUAL;
    
    //
    //Action
    //
    Action.ActionType.IntRepresentation = RX_FILTER_ACTION_DROP;
    FilterFlags.IntRepresentation = RX_FILTER_BINARY;
    lRetVal = sl_WlanRxFilterAdd(RuleType, FilterFlags, &Rule, &Trigger,
                                    &Action, &FilterId);
    ASSERT_ON_ERROR(lRetVal);

    return 0;
}

//*****************************************************************************
//
//! This function activates all pre-defined rx filters. It's also possible to 
//! activate a specific filter by assigning only the required indexes in 
//! FilterIdMask
//!
//! \param None
//!
//! \return None
//!
//*****************************************************************************
signed long EnableAllFilters()
{
    signed long lRetVal = -1;
    _WlanRxFilterOperationCommandBuff_t     RxFilterIdMask ;
    //
    //Enable all 64 filters (8*8) - bit=1 represents enabled filter
    //
    memset( RxFilterIdMask.FilterIdMask, 0xFF , 8);
    lRetVal = sl_WlanRxFilterSet( SL_ENABLE_DISABLE_RX_FILTER,
                        (unsigned char *)&RxFilterIdMask,
                       sizeof(_WlanRxFilterOperationCommandBuff_t));

    return lRetVal;
}

//*****************************************************************************
//
//! This function opens a TCP socket in Listen mode and waits for an incoming 
//! TCP connection.. If a socket connection is established then the function 
//! will try to read 1000 TCP packets from the connected client.
//!
//! \param port number on which the server will be listening on
//!
//! \return  0 on success, -ve on Error.
//!
//*****************************************************************************
static long BsdTcpServer(unsigned short Port)
{
    SlSockAddrIn_t  Addr;
    SlSockAddrIn_t  LocalAddr;
    int             indexCount,iAddrSize,SockID,iStatus,newSockID;
    long            LoopCount = 0;
    long            nonBlocking = 1;
    SlTimeval_t     timeVal;

    for (indexCount=0 ; indexCount<BUF_SIZE ; indexCount++)
    {
        g_uBuf.cBsdBuf[indexCount] = (char)(indexCount % 10);
    }

    LocalAddr.sin_family = SL_AF_INET;
    LocalAddr.sin_port = sl_Htons((unsigned short)Port);
    LocalAddr.sin_addr.s_addr = 0;
    SockID = sl_Socket(SL_AF_INET,SL_SOCK_STREAM, 0);
    ASSERT_ON_ERROR(SockID);

    iAddrSize = sizeof(SlSockAddrIn_t);
    iStatus = sl_Bind(SockID, (SlSockAddr_t *)&LocalAddr, iAddrSize);

    if( iStatus < 0 )
    {
        // error
        ASSERT_ON_ERROR(sl_Close(SockID));
        ASSERT_ON_ERROR(iStatus);
    }

    iStatus = sl_Listen(SockID, 0);
    if( iStatus < 0 )
    {
        sl_Close(SockID);
        ASSERT_ON_ERROR(iStatus);
    }

    iStatus = sl_SetSockOpt(SockID, SL_SOL_SOCKET, SL_SO_NONBLOCKING, \
                            &nonBlocking,
                            sizeof(nonBlocking));
    newSockID = SL_EAGAIN;
    while( newSockID < 0 )
    {
        newSockID = sl_Accept(SockID, ( struct SlSockAddr_t *)&Addr, 
                                        (SlSocklen_t*)&iAddrSize);
        if( newSockID == SL_EAGAIN )
        {
                MAP_UtilsDelay(80000);
        }
        else if( newSockID < 0 )
        {
            ASSERT_ON_ERROR(sl_Close(SockID));
            // error
            ASSERT_ON_ERROR(newSockID);
        }
    }
    timeVal.tv_sec = 1;
    timeVal.tv_usec = 0;
    if( newSockID >= 0 )
    {
        iStatus = sl_SetSockOpt(newSockID, SL_SOL_SOCKET, SL_SO_RCVTIMEO,
                                &timeVal, sizeof(timeVal));
        ASSERT_ON_ERROR(iStatus);
    }

    while (LoopCount < PACKET_COUNT)
    {
        iStatus = sl_Recv(newSockID, g_uBuf.cBsdBuf, BUF_SIZE, 0);
        if( iStatus <= 0 )
        {
            // error
            ASSERT_ON_ERROR(sl_Close(newSockID));
            ASSERT_ON_ERROR(sl_Close(SockID));
            ASSERT_ON_ERROR(iStatus);
        }

        LoopCount++;
    }

    ASSERT_ON_ERROR(sl_Close(newSockID));
    ASSERT_ON_ERROR(sl_Close(SockID));
    return 0;
}

//*****************************************************************************
//
//! NwpFilter - Function which Creates and Enables the Filter
//!
//! \param  pvParameters
//!
//! \return 0
//!
//*****************************************************************************
static int NwpFilter(void *pvParameters)
{
    long lRetVal = 0;
    InitializeAppVariables();

    //
    // Following function configure the device to default state by cleaning
    // the persistent settings stored in NVMEM (viz. connection profiles &
    // policies, power policy etc)
    //
    // Applications may choose to skip this step if the developer is sure
    // that the device is in its default state at start of applicaton
    //
    // Note that all profiles and persistent settings that were done on the
    // device will be lost
    //
    lRetVal = ConfigureSimpleLinkToDefaultState();
    if(lRetVal < 0)
    {
        if (DEVICE_NOT_IN_STATION_MODE == lRetVal)
        {
            UART_PRINT("Failed to configure the device in its default state\n\r");
        }

        LOOP_FOREVER();
    }

    UART_PRINT("Device is configured in default state \n\r");

    //
    // Assumption is that the device is configured in station mode already
    // and it is in its default state
    //
    lRetVal = sl_Start(0, 0, 0);
    if (lRetVal < 0 || ROLE_STA != lRetVal)
    {
        UART_PRINT("Failed to start the device \n\r");
        LOOP_FOREVER();
    }

    UART_PRINT("Device started as STATION \n\r");

    //
    // Reset policy settings
    //
    lRetVal = sl_WlanPolicySet(SL_POLICY_CONNECTION, \
                                SL_CONNECTION_POLICY(0,0,0,0,0), 0, 0);

    //
    // This function creates two different filters:
    // (1) Filter packets according to remote MAC
    // (2) Filter packets according to remote IP address
    //
    lRetVal=CreateFilters();
    if (lRetVal < 0)
    {
        UART_PRINT("Filter creation failed\n\r");
        LOOP_FOREVER();
    }

    //
    // Activate pre-defined filters  - The filters will be deleted upon reset
    //
    if(EnableAllFilters() < 0)
    {
        UART_PRINT("Unable to enable filter \n\r");
        LOOP_FOREVER();
    }
    
    //
    //Connecting to WLAN AP
    //
    lRetVal = WlanConnect();
    if(lRetVal < 0)
    {
        UART_PRINT("Failed to establish connection w/ an AP \n\r");
        LOOP_FOREVER();
    }

    UART_PRINT("Connection established w/ AP and IP is aquired \n\r");

    //
    // After calling this function, you can start sending data to CC3200 IP 
    // address on PORT_NUM TCP connection will be refused if remote MAC/IP is of 
    // the filtered. It's also possible to enable the filters after TCP 
    // connection, resulting in TCP packets to not being received 
    //
    lRetVal = BsdTcpServer(PORT_NUM);
    if(lRetVal < 0)
    {
        UART_PRINT("Failure in TCP server\n\r");
        LOOP_FOREVER();
    }

    //
    // Power off Network processor
    //
    lRetVal = sl_Stop(SL_STOP_TIMEOUT);

    return 0;
}

//*****************************************************************************
//
//! Board Initialization & Configuration
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************
static void BoardInit(void)
{
/* In case of TI-RTOS vector table is initialize by OS itself */
#ifndef USE_TIRTOS
    //
    // Set vector table base
    //
#if defined(ccs)
    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
#endif
#if defined(ewarm)
    MAP_IntVTableBaseSet((unsigned long)&__vector_table);
#endif
#endif
    //
    // Enable Processor
    //
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}

//*****************************************************************************
//
//! Main 
//!
//! \param  none
//!
//! \return 0
//!
//*****************************************************************************
int
main()
{
    //
    // Board Initialisation
    //
    BoardInit();
	
#ifndef NOTERM
	InitTerm();
#endif

    //
    // Call the NwpFilter
    //
    NwpFilter(NULL);
    LOOP_FOREVER();
}
//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
