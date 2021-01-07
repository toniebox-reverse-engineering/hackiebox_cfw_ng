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
// Application Name     -   Provisioning AP
// Application Overview -   This is a sample application demonstrating how to
//                          provision a CC3200 device.
//
//*****************************************************************************


//****************************************************************************
//
//! \addtogroup simplelink_provisioning
//! @{
//
//****************************************************************************

// Simplelink includes
#include "simplelink.h"

//Driverlib includes
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "rom.h"
#include "rom_map.h"
#include "interrupt.h"
#include "prcm.h"
#include "utils.h"
#include "timer.h"

// Provisioning lib include
#include "provisioning_api.h"
#include "provisioning_defs.h"

//Common interface includes
#include "pinmux.h"
#include "gpio_if.h"
#include "common.h"
#include "timer_if.h"
#ifndef NOTERM
#include "uart_if.h"
#endif


#define APPLICATION_NAME        "AP Provisioning"
#define APPLICATION_VERSION     "1.4.0"


#define WLAN_DEL_ALL_PROFILES   0xFF


#define SL_PARAM_PRODUCT_VERSION_DATA 	"R1.0"
#define PROVISIONING_TIMEOUT            300 //Number of seconds to wait for provisioning completion

// Application specific status/error codes
typedef enum{
    // Choosing -0x7D0 to avoid overlap w/ host-driver's error codes
    LAN_CONNECTION_FAILED = -0x7D0,
    DEVICE_NOT_IN_STATION_MODE = LAN_CONNECTION_FAILED - 1,
    STATUS_CODE_MAX = -0xBB8
}e_AppStatusCodes;


//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
volatile unsigned long  g_ulStatus = 0;//SimpleLink Status
unsigned long  g_ulGatewayIP = 0; //Network Gateway IP address
unsigned long  g_ulStaIp = 0;
unsigned char  g_ucConnectionSSID[SSID_LEN_MAX+1]; //Connection SSID
unsigned char  g_ucConnectionBSSID[BSSID_LEN_MAX]; //Connection BSSID

unsigned long 	g_ulTimerA2Base;
_u8 volatile g_TimerATimedOut;
_u8 volatile g_TimerBTimedOut;

#if defined(ccs)
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
int startProvisioning();
static void BoardInit(void);
static void InitializeAppVariables();

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
            // available in 'slWlanConnectAsyncResponse_t'
            // Applications can use it if required
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

            UART_PRINT("[WLAN EVENT] STA Connected to the AP: %s , "
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
                UART_PRINT("[WLAN EVENT]Device disconnected from the AP: %s, "
                           "BSSID: %x:%x:%x:%x:%x:%x on application's "
                           "request \n\r",
                           g_ucConnectionSSID,g_ucConnectionBSSID[0],
                           g_ucConnectionBSSID[1],g_ucConnectionBSSID[2],
                           g_ucConnectionBSSID[3],g_ucConnectionBSSID[4],
                           g_ucConnectionBSSID[5]);
            }
            else
            {
                UART_PRINT("[WLAN ERROR]Device disconnected from the AP AP: %s, "
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

        case SL_WLAN_STA_CONNECTED_EVENT:
        {
        	// when device is in AP mode and any client connects to device cc3xxx
        	SET_STATUS_BIT(g_ulStatus, STATUS_BIT_CONNECTION);
        	CLR_STATUS_BIT(g_ulStatus, STATUS_BIT_CONNECTION_FAILED);

        	//
        	// Information about the connected client (like SSID, MAC etc) will
        	// be available in 'slPeerInfoAsyncResponse_t' - Applications
        	// can use it if required
        	//
        	// slPeerInfoAsyncResponse_t *pEventData = NULL;
        	// pEventData = &pSlWlanEvent->EventData.APModeStaConnected;
        	//

        }
        break;

        case SL_WLAN_STA_DISCONNECTED_EVENT:
        {
        	// when client disconnects from device (AP)
        	CLR_STATUS_BIT(g_ulStatus, STATUS_BIT_CONNECTION);
        	CLR_STATUS_BIT(g_ulStatus, STATUS_BIT_IP_LEASED);

        	//
        	// Information about the connected client (like SSID, MAC etc) will
        	// be available in 'slPeerInfoAsyncResponse_t' - Applications
        	// can use it if required
        	//
        	// slPeerInfoAsyncResponse_t *pEventData = NULL;
        	// pEventData = &pSlWlanEvent->EventData.APModestaDisconnected;
        	//
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

            //Gateway IP address
            g_ulGatewayIP = pEventData->gateway;

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
        case SL_NETAPP_IP_LEASED_EVENT:
        {
        	SET_STATUS_BIT(g_ulStatus, STATUS_BIT_IP_LEASED);
        	//UART_PRINT("[NETAPP EVENT] IP leased to a client\n\r");

        	//
        	// Information about the IP-Leased details(like IP-Leased,lease-time,
        	// mac etc) will be available in 'SlIpLeasedAsync_t' - Applications
        	// can use it if required
        	//
        	// SlIpLeasedAsync_t *pEventData = NULL;
        	// pEventData = &pNetAppEvent->EventData.ipLeased;
        	//

        	SlIpLeasedAsync_t *pEventData = NULL;
        	pEventData = &pNetAppEvent->EventData.ipLeased;
        	g_ulStaIp = pEventData->ip_address;
        	UART_PRINT("[NETAPP EVENT] IP Leased to Client: IP=%d.%d.%d.%d \n\r",
        	                        SL_IPV4_BYTE(g_ulStaIp,3), SL_IPV4_BYTE(g_ulStaIp,2),
        	                        SL_IPV4_BYTE(g_ulStaIp,1), SL_IPV4_BYTE(g_ulStaIp,0));
        }
        break;

        case SL_NETAPP_IP_RELEASED_EVENT:
        {
        	CLR_STATUS_BIT(g_ulStatus, STATUS_BIT_IP_LEASED);
        	UART_PRINT("[NETAPP EVENT] IP Released for Client: IP=%d.%d.%d.%d \n\r",
        	                        SL_IPV4_BYTE(g_ulStaIp,3), SL_IPV4_BYTE(g_ulStaIp,2),
        	                        SL_IPV4_BYTE(g_ulStaIp,1), SL_IPV4_BYTE(g_ulStaIp,0));

        	//
        	// Information about the IP-Released details (like IP-address, mac
        	// etc) will be available in 'SlIpReleasedAsync_t' - Applications
        	// can use it if required
        	//
        	// SlIpReleasedAsync_t *pEventData = NULL;
        	// pEventData = &pNetAppEvent->EventData.ipReleased;
        	//
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
    if(!pDevEvent)
    {
        return;
    }

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
    if(!pSock)
    {
        return;
    }

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

void generalTimeoutHandler(void)
{
    Timer_IF_InterruptClear(TIMERA1_BASE);
    g_TimerBTimedOut++;
}

// General waiting function using timer
void waitmSec(_i32 timeout)
{
    //Initializes & Starts timer
    Timer_IF_Init(PRCM_TIMERA1, TIMERA1_BASE, TIMER_CFG_ONE_SHOT, TIMER_A, 0);
    Timer_IF_IntSetup(TIMERA1_BASE, TIMER_A, generalTimeoutHandler);

    g_TimerBTimedOut = 0;

    Timer_IF_Start(TIMERA1_BASE, TIMER_A, timeout);

    while(g_TimerBTimedOut != 1)
    {
        // waiting...
#ifndef SL_PLATFORM_MULTI_THREADED
        _SlNonOsMainLoopTask();
#endif
    }

    //Stops timer
    Timer_IF_Stop(TIMERA1_BASE, TIMER_A);
    Timer_IF_DeInit(TIMERA1_BASE, TIMER_A);
}

void timeoutHandler(void)
{
    Timer_IF_InterruptClear(TIMERA0_BASE);
    g_TimerATimedOut++;
}

//*****************************************************************************
// Provisioning Callbacks
//*****************************************************************************
_i8 sl_extlib_ProvEventTimeoutHdl(_u8* event, _i32 timeout)
{
    if(timeout == SL_EXT_PROV_WAIT_FOREVER)
    {
        // Waiting forever, no timeout
        while(*event == FALSE)
        {
            // waiting...
#ifndef SL_PLATFORM_MULTI_THREADED
            _SlNonOsMainLoopTask();
#endif
        }
    }
    else
    {
    	//On CC3200, a value greater than 53687 will overflow the buffer,
		//therefore divide the timeout into smaller pieces.
		int divider = 10;

		//Initializes & Starts timer
		Timer_IF_Init(PRCM_TIMERA0, TIMERA0_BASE, TIMER_CFG_PERIODIC, TIMER_A, 0);
		Timer_IF_IntSetup(TIMERA0_BASE, TIMER_A, timeoutHandler);

		g_TimerATimedOut = 0;

		Timer_IF_Start(TIMERA0_BASE, TIMER_A, timeout/divider);

        //Check event or wait until timeout
        while(*event == FALSE && g_TimerATimedOut != divider)
        {
            // waiting...
#ifndef SL_PLATFORM_MULTI_THREADED
            _SlNonOsMainLoopTask();
#endif
        }

        //Stops timer
		Timer_IF_Stop(TIMERA0_BASE, PRCM_TIMERA0);
		Timer_IF_DeInit(TIMERA0_BASE, PRCM_TIMERA0);

		// check if timeout occured
		if(g_TimerATimedOut == divider)
		{
			return -1;
		}

    }


    return 0;
}

void sl_extlib_ProvWaitHdl(_i32 timeout)
{
    waitmSec(timeout);
}


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
    g_ulStaIp = 0;
    //g_ConnectTimeoutCnt = 0;
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

//*****************************************************************************
//
//! Application startup display on UART
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
static void
DisplayBanner(char * AppName)
{
    Report("\n\n\n\r");
    Report("\t\t *************************************************\n\r");
    Report("\t\t       CC3200 %s Application       \n\r", AppName);
    Report("\t\t *************************************************\n\r");
    Report("\n\n\n\r");
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
static void
BoardInit(void)
{
	// In case of TI-RTOS vector table is initialize by OS itself
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
//! \brief Function for handling provisioning (adding a profile to a new AP).
//!
//! This function assumes running mostly on first time configurations, so
//! one time settings are handled here.
//!
//! \param[in]		None
//!
//! \return         None
//!
//*****************************************************************************
int startProvisioning(void)
{
	long				lRetVal = -1;
	long 				FileHandle = 0;
	slExtLibProvCfg_t	cfg;
	SlFsFileInfo_t 		FsFileInfo;


	UART_PRINT("Starting Provisioning..\n\r");


	// Enable RX Statistics
	sl_WlanRxStatStart();

	// Check if version token file exists in the device FS.
	// If not, than create a file and write the required token

	// Creating the param_product_version.txt file once
	if (SL_FS_ERR_FILE_NOT_EXISTS == sl_FsGetInfo(SL_FILE_PARAM_PRODUCT_VERSION, 0 , &FsFileInfo))
	{
		sl_FsOpen(SL_FILE_PARAM_PRODUCT_VERSION, FS_MODE_OPEN_CREATE(100, _FS_FILE_OPEN_FLAG_COMMIT), NULL, &FileHandle);
		sl_FsWrite(FileHandle, NULL, SL_PARAM_PRODUCT_VERSION_DATA, strlen(SL_PARAM_PRODUCT_VERSION_DATA));
		sl_FsClose(FileHandle, NULL, NULL, NULL);
	}

	// Creating the config result file once
	if (SL_FS_ERR_FILE_NOT_EXISTS == sl_FsGetInfo(SL_FILE_PARAM_CFG_RESULT, 0 , &FsFileInfo))
	{
		sl_FsOpen(SL_FILE_PARAM_CFG_RESULT, FS_MODE_OPEN_CREATE(100, _FS_FILE_OPEN_FLAG_COMMIT), NULL, &FileHandle);
		sl_FsWrite(FileHandle, NULL, GET_CFG_RESULT_TOKEN, strlen(GET_CFG_RESULT_TOKEN));
		sl_FsClose(FileHandle, NULL, NULL, NULL);
	}

	// Creating the param device name file once/
	if (SL_FS_ERR_FILE_NOT_EXISTS == sl_FsGetInfo(SL_FILE_PARAM_DEVICE_NAME, 0 , &FsFileInfo))
	{
		sl_FsOpen(SL_FILE_PARAM_DEVICE_NAME, FS_MODE_OPEN_CREATE(100, _FS_FILE_OPEN_FLAG_COMMIT), NULL, &FileHandle);
		sl_FsWrite(FileHandle, NULL, GET_DEVICE_NAME_TOKEN, strlen(GET_DEVICE_NAME_TOKEN));
		sl_FsClose(FileHandle, NULL, NULL, NULL);
	}

	// Creating the netlist name file once/
	if (SL_FS_ERR_FILE_NOT_EXISTS == sl_FsGetInfo(SL_FILE_NETLIST, 0 , &FsFileInfo))
	{
		sl_FsOpen(SL_FILE_NETLIST, FS_MODE_OPEN_CREATE(100, _FS_FILE_OPEN_FLAG_COMMIT), NULL, &FileHandle);
		sl_FsWrite(FileHandle, NULL, SL_SET_NETLIST_TOKENS, strlen(SL_SET_NETLIST_TOKENS));
		sl_FsClose(FileHandle, NULL, NULL, NULL);
	}

	// Initializes configuration
	cfg.IsBlocking         = 1;    //Unused
	cfg.AutoStartEnabled   = 0;
	cfg.Timeout10Secs      = PROVISIONING_TIMEOUT/10;
	cfg.ModeAfterFailure   = ROLE_STA;
	cfg.ModeAfterTimeout   = ROLE_STA;

	lRetVal = sl_extlib_ProvisioningStart(ROLE_STA, &cfg);
	ASSERT_ON_ERROR(lRetVal);

	// Wait for WLAN Event
	while((!IS_CONNECTED(g_ulStatus)) || (!IS_IP_ACQUIRED(g_ulStatus)))
	{
		_SlNonOsMainLoopTask();
	}

	//
	// Turn ON the RED LED to indicate connection success
	//
	GPIO_IF_LedOn(MCU_RED_LED_GPIO);

	//wait for few moments
	MAP_UtilsDelay(80000000);

	return SUCCESS;
}


int main(void)
{
    long lRetVal = -1;
    //
    // Initialize Board configurations
    //

    BoardInit();
    //
    // Configure the pinmux settings for the peripherals exercised
    //
    PinMuxConfig();

#ifndef NOTERM
    InitTerm();
#endif

    //
	// Display banner
	//
	DisplayBanner(APPLICATION_NAME);

    // configure RED LED
    GPIO_IF_LedConfigure(LED1);

    GPIO_IF_LedOff(MCU_RED_LED_GPIO);

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
           UART_PRINT("Failed to configure the device in its "
                         "default state \n\r");

       LOOP_FOREVER();
   }

   UART_PRINT("Device is configured in default state \n\r");

   CLR_STATUS_BIT_ALL(g_ulStatus);

   //Start simplelink
   lRetVal = sl_Start(0,0,0);
   if (lRetVal < 0 || ROLE_STA != lRetVal)
   {
       UART_PRINT("Failed to start the device \n\r");
       LOOP_FOREVER();
   }

   UART_PRINT("Device started as STATION \n\r");

   // Connect to our AP using SmartConfig method
   lRetVal = startProvisioning();
   if(lRetVal < 0)
   {
	   ERR_PRINT(lRetVal);
   }    
   else
   {
	   UART_PRINT("Provisioning Succedded \n\r");
   }

   LOOP_FOREVER();
}
