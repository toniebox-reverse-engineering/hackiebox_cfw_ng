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
// Application Name     - Watchdog System Demo
// Application Overview - This application illustrates full system recovery,
//                        using watchdog, including network subsystem.
//
//*****************************************************************************


//****************************************************************************
//
//! \addtogroup udp_socket_with_wdt_reset
//! @{
//
//****************************************************************************

#include <stdlib.h>
#include <string.h>

// simplelink includes
#include "simplelink.h"
#include "wlan.h"

// driverlib includes
#include "hw_ints.h"
#include "hw_types.h"
#include "hw_memmap.h"
#include "rom.h"
#include "rom_map.h"
#include "interrupt.h"
#include "prcm.h"
#include "utils.h"
#include "uart.h"
#include "wdt.h"

// common interface includes
#include "udma_if.h"
#ifndef NOTERM
#include "uart_if.h"
#endif
#include "wdt_if.h"
#include "common.h"

#include "pinmux.h"



#define IP_ADDR                 0xc0a8006A /* 192.168.0.106 */
#define APPLICATION_NAME        "Watchdog System Demo"
#define APPLICATION_VERSION     "1.4.0"

#define PORT_NUM           5001
#define BUF_SIZE           1400
#define UDP_PACKET_COUNT   20


// Application specific status/error codes
typedef enum{
    // Choosing -0x7D0 to avoid overlap w/ host-driver's error codes
    LAN_CONNECTION_FAILED = -0x7D0,
    INTERNET_CONNECTION_FAILED = LAN_CONNECTION_FAILED - 1,
    DEVICE_NOT_IN_STATION_MODE = INTERNET_CONNECTION_FAILED - 1,

    STATUS_CODE_MAX = -0xBB8
}e_AppStatusCodes;


//****************************************************************************
//                      LOCAL FUNCTION PROTOTYPES
//****************************************************************************
int BsdUdpClient(unsigned short usPort, unsigned long ulDesitnationIP);
static long WlanConnect();
static void DisplayBanner();
static void BoardInit();
static void InitializeAppVariables();
static long ConfigureSimpleLinkToDefaultState();


//*****************************************************************************
// GLOBAL VARIABLES -- Start
//*****************************************************************************
volatile unsigned long  g_ulStatus = 0;//SimpleLink Status
unsigned long  g_ulGatewayIP = 0; //Network Gateway IP address
unsigned char  g_ucConnectionSSID[SSID_LEN_MAX+1]; //Connection SSID
unsigned char  g_ucConnectionBSSID[BSSID_LEN_MAX]; //Connection BSSID
unsigned int   g_uiPortNum = PORT_NUM;
unsigned long  g_ulPacketCount = UDP_PACKET_COUNT;
unsigned char  g_ucSimplelinkstarted = 0;
unsigned long  g_ulIpAddr = 0;
char g_cBsdBuf[BUF_SIZE];

#if defined(ccs) || defined(gcc)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif
//*****************************************************************************
// GLOBAL VARIABLES -- End
//*****************************************************************************


//*****************************************************************************
// SimpleLink Asynchronous Event Handlers -- Start
//*****************************************************************************


//*****************************************************************************
//
//! \brief The Function Handles WLAN Events
//!
//! \param[in]  pSlWlanEvent - Pointer to WLAN Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkWlanEventHandler(SlWlanEvent_t *pSlWlanEvent)
{
     switch(((SlWlanEvent_t*)pSlWlanEvent)->Event)
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
            // pEventData = &pSlWlanEvent->EventData.STAandP2PModeWlanConnected;
            //
            
            // Copy new connection SSID and BSSID to global parameters
            memcpy(g_ucConnectionSSID,pSlWlanEvent->EventData.
                   STAandP2PModeWlanConnected.ssid_name,
                   pSlWlanEvent->EventData.STAandP2PModeWlanConnected.ssid_len);
            memcpy(g_ucConnectionBSSID,
                    pSlWlanEvent->EventData.STAandP2PModeWlanConnected.bssid,
                   SL_BSSID_LENGTH);

            UART_PRINT("[WLAN EVENT] STA Connected to the AP: %s , BSSID: "
                        "%x:%x:%x:%x:%x:%x\n\r", g_ucConnectionSSID,
                        g_ucConnectionBSSID[0], g_ucConnectionBSSID[1], 
                        g_ucConnectionBSSID[2], g_ucConnectionBSSID[3],
                        g_ucConnectionBSSID[4], g_ucConnectionBSSID[5]);
        }
        break;

        case SL_WLAN_DISCONNECT_EVENT:
        {
            slWlanConnectAsyncResponse_t*  pEventData = NULL;

            CLR_STATUS_BIT(g_ulStatus, STATUS_BIT_CONNECTION);
            CLR_STATUS_BIT(g_ulStatus, STATUS_BIT_IP_AQUIRED);

            pEventData = &pSlWlanEvent->EventData.STAandP2PModeDisconnected;

            // If the user has initiated 'Disconnect' request, 
            //'reason_code' is SL_WLAN_DISCONNECT_USER_INITIATED_DISCONNECTION 
            if(SL_WLAN_DISCONNECT_USER_INITIATED_DISCONNECTION == pEventData->reason_code)
            {
                UART_PRINT("[WLAN EVENT]Device disconnected from the AP: %s, "
                           "BSSID: %x:%x:%x:%x:%x:%x on application's request "
                           "\n\r", g_ucConnectionSSID,g_ucConnectionBSSID[0],
                           g_ucConnectionBSSID[1],g_ucConnectionBSSID[2],
                           g_ucConnectionBSSID[3],g_ucConnectionBSSID[4],
                           g_ucConnectionBSSID[5]);
            }
            else
            {
                UART_PRINT("[WLAN ERROR]Device disconnected from the AP AP: %s,"
                           " BSSID: %x:%x:%x:%x:%x:%x on an ERROR..!! \n\r",
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
            UART_PRINT("[WLAN EVENT] Unexpected event \n\r");
        }
        break;
     }

}

//*****************************************************************************
//
//! \brief This function handles network events such as IP acquisition, IP
//!           leased, IP released etc.
//!
//! \param pNetAppEvent - Pointer indicating device acquired IP
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkNetAppEventHandler(SlNetAppEvent_t *pNetAppEvent)
{

    switch(pNetAppEvent->Event)
    {
        case SL_NETAPP_IPV4_IPACQUIRED_EVENT:
        case SL_NETAPP_IPV6_IPACQUIRED_EVENT:
        {
            SET_STATUS_BIT(g_ulStatus, STATUS_BIT_IP_AQUIRED);

            //
            // Information about the IPv4 & IPv6 details (like IP, gateway,dns 
            // etc) will be available in 'SlIpV4AcquiredAsync_t / 
            // SlIpV6AcquiredAsync_t' - Applications can use it if required
            //
            // For IPv4:
            //

            SlIpV4AcquiredAsync_t *pEventData = NULL;

            //Ip Acquired Event Data
            pEventData = &pNetAppEvent->EventData.ipAcquiredV4;

            g_ulIpAddr = pEventData->ip;

            //
            // For IPv6:
            //
            // SlIpV4AcquiredAsync_t *pEventData = NULL;
            // pEventData = &pNetAppEvent->EventData.ipAcquiredV6;
            //
        }
        break;

        default:
        {
            UART_PRINT("[NETAPP EVENT] Unexpected event \n\r");
        }
        break;
    }
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
//
//! This function gets triggered when HTTP Server receives Application
//! defined GET and POST HTTP Tokens.
//!
//! \param pHttpServerEvent Pointer indicating http server event
//! \param pHttpServerResponse Pointer indicating http server response
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkHttpServerCallback(SlHttpServerEvent_t *pSlHttpServerEvent, 
                                SlHttpServerResponse_t *pSlHttpServerResponse)
{

}


//*****************************************************************************
// SimpleLink Asynchronous Event Handlers -- End
//*****************************************************************************


//*****************************************************************************
//
//! This function initializes the application variables
//!
//! \param[in]    None
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
    g_uiPortNum = PORT_NUM;
    g_ulPacketCount = UDP_PACKET_COUNT;
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
long ConfigureSimpleLinkToDefaultState()
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
//! Routine to feed to watchdog
//!
//! \param  none
//!
//! \return none
//
//*****************************************************************************
static void WatchdogAck()
{
  //
  // Acknowledge watchdog by clearing interrupt
  //
  WatchdogIntClear(WDT_BASE);
}

//****************************************************************************
//
//! \brief Opening a UDP client side socket and sending data
//!
//! This function opens a UDP socket and tries to connect to a Server IP_ADDR
//!    waiting on port PORT_NUM.
//!    Then the function will send 1000 UDP packets to the server.
//!
//! \param[in]  port number on which the server will be listening on
//!
//! \return    0 on success, -1 on Error.
//
//****************************************************************************
int BsdUdpClient(unsigned short usPort, unsigned long ulDesitnationIP)
{
    int             iCounter;
    short           sTestBufLen;
    SlSockAddrIn_t  sAddr;
    int             iAddrSize;
    int             iSockID;
    int             iStatus;
    int             iPackets;

    // filling the buffer
    for (iCounter=0 ; iCounter<BUF_SIZE ; iCounter++)
    {
        g_cBsdBuf[iCounter] = (char)(iCounter % 10);
    }

    sTestBufLen  = BUF_SIZE;

    //
    // Filling the UDP server socket address
    //
    sAddr.sin_family = SL_AF_INET;
    sAddr.sin_port = sl_Htons((unsigned short)usPort);
    sAddr.sin_addr.s_addr = sl_Htonl((unsigned int)ulDesitnationIP);

    iAddrSize = sizeof(SlSockAddrIn_t);

    //
    // Initialize number of packets variable
    //
    iPackets = UDP_PACKET_COUNT;

    //
    // Creating a UDP socket
    //
    iSockID = sl_Socket(SL_AF_INET,SL_SOCK_DGRAM, 0);

    if( iSockID < 0 )
    {
        // error
        return -1;
    }

    UART_PRINT("\n\rSending Packets\n\r");
    //
    // Loop forever sending packets
    //
    while (1)
    {
        //
        // Sending packet
        //
        iStatus = sl_SendTo(iSockID, g_cBsdBuf, sTestBufLen, 0,
                        ( SlSockAddr_t *)&sAddr, iAddrSize);


        //
        // Decrement the packet count
        //
        iPackets--;

        UART_PRINT(".");

        //
        // If send fails or packet count reaches 0 stop acking the watchdog
        // so that it resets the system
        //
        if( iStatus > 0  && iPackets > 0)
        {
            WatchdogAck();
        }

        UtilsDelay(8000000);

    }

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
static long WlanConnect()
{
    SlSecParams_t secParams = {0};
    signed long lretVal = 0;

    secParams.Key = SECURITY_KEY;
    secParams.KeyLen = strlen(SECURITY_KEY);
    secParams.Type = SECURITY_TYPE;

    lretVal = sl_WlanConnect(SSID_NAME, strlen(SSID_NAME), 0, &secParams, 0);
    ASSERT_ON_ERROR(lretVal);

    while((!IS_CONNECTED(g_ulStatus)) || (!IS_IP_ACQUIRED(g_ulStatus)))
    {
        // Wait for WLAN Event
#ifndef SL_PLATFORM_MULTI_THREADED
              _SlNonOsMainLoopTask();
#endif

    }

    UNUSED(lretVal);
    return SUCCESS;

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
    UART_PRINT("\n\n\n\r");
    UART_PRINT("\t\t *************************************************\n\r");
    UART_PRINT("\t\t      CC3200 %s       \n\r", AppName);
    UART_PRINT("\t\t *************************************************\n\r");
    UART_PRINT("\n\n\n\r");
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
/* In case of TI-RTOS vector table is initialize by OS itself */
#ifndef USE_TIRTOS
    //
    // Set vector table base
    //
#if defined(ccs) || defined(gcc)
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
//! Mandatory Configuration to put the PM into safe state before entering hibernate
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************
static inline void HIBEntrePreamble()
{
    HWREG(0x400F70B8) = 1;
    UtilsDelay(800000/5);
    HWREG(0x400F70B0) = 1;
    UtilsDelay(800000/5);

    HWREG(0x4402E16C) |= 0x2;
    UtilsDelay(800);
    HWREG(0x4402F024) &= 0xF7FFFFFF;
}

//****************************************************************************
//                            MAIN FUNCTION
//****************************************************************************
void main()
{
    long retVal = -1;
    unsigned long ulResetCause;
    unsigned long ulDestinationIP;

    //
    // Board Initialization
    //
    BoardInit();

    //
    // Configure the pinmux settings for the peripherals exercised
    //
    PinMuxConfig();

    //
    // Configuring UART
    //
    InitTerm();

    //
    // Initialize WDT
    //
    WDT_IF_Init(NULL,80000000 * 10);

    //
    // Get the reset cause
    //
    ulResetCause = PRCMSysResetCauseGet();

    //
    // If watchdog triggered reset request hibernate
    // to clean boot the system
    //
    if( ulResetCause == PRCM_WDT_RESET )
    {
        HIBEntrePreamble();
        MAP_PRCMOCRRegisterWrite(0,1);
        MAP_PRCMHibernateWakeupSourceEnable(PRCM_HIB_SLOW_CLK_CTR);
        MAP_PRCMHibernateIntervalSet(330);
        MAP_PRCMHibernateEnter();
    }

    //
    // uDMA Initialization
    //
    UDMAInit();

    //
    // Display banner
    //
    DisplayBanner(APPLICATION_NAME);

    if( ulResetCause == PRCM_HIB_EXIT &&  (MAP_PRCMOCRRegisterRead(0) & 1) == 1 )
    {
        UART_PRINT("Reset Cause        : Watchdog Reset\n\r");
    }
    else
    {
        UART_PRINT("Reset Cause        : Power On\n\r");

        //
        // Initialize the variables.
        //
        InitializeAppVariables();

        //
        // Following function configure the device to default state by cleaning
        // the persistent settings stored in NVMEM (viz. connection profiles &
        // policies, power policy etc)
        //
        // Applications may choose to skip this step if the developer is sure
        // that the device is in its desired state at start of applicaton
        //
        // Note that all profiles and persistent settings that were done on the
        // device will be lost
        //
        retVal = ConfigureSimpleLinkToDefaultState();

        if(retVal < 0)
        {
            if (DEVICE_NOT_IN_STATION_MODE == retVal)
                UART_PRINT("Failed to configure the device in its default"
                            " state \n\r");

            LOOP_FOREVER();
        }
    }

    //
    // Set destination IP
    //
    ulDestinationIP = IP_ADDR;

    //
    // Asumption is that the device is configured in station mode already
    // and it is in its default state
    //
    retVal = sl_Start(0, 0, 0);

    //
    // Acknowledge the watchdog so that it doesn't resets
    //
    WatchdogAck();

    if (retVal < 0 || retVal != ROLE_STA)
    {
        UART_PRINT("Failed to start the device \n\r");
        LOOP_FOREVER();
    }

    //
    //Connecting to WLAN AP
    //
    retVal = WlanConnect();

    //
    // Acknowledge the watchdog so that it doesn't resets
    //
    WatchdogAck();

    if(retVal < 0)
    {
        UART_PRINT("Failed to establish connection w/ an AP \n\r");
        LOOP_FOREVER();
    }

    UART_PRINT("Connected to AP    : %s \n\r",SSID_NAME);

    UART_PRINT("Device IP          : %d.%d.%d.%d\n\r\n\r",
                SL_IPV4_BYTE(g_ulIpAddr,3),
                SL_IPV4_BYTE(g_ulIpAddr,2),
                SL_IPV4_BYTE(g_ulIpAddr,1),
                SL_IPV4_BYTE(g_ulIpAddr,0));


    UART_PRINT("\nStarting UDP Client\n\n\r");

    UART_PRINT("Source IP          : %d.%d.%d.%d\n\r"
                 "Destination IP     : %d.%d.%d.%d\n\r"
                 "PORT               : %d\n\r",
                 SL_IPV4_BYTE(g_ulIpAddr,3),
                 SL_IPV4_BYTE(g_ulIpAddr,2),
                 SL_IPV4_BYTE(g_ulIpAddr,1),
                 SL_IPV4_BYTE(g_ulIpAddr,0),
                 SL_IPV4_BYTE(ulDestinationIP,3),
                 SL_IPV4_BYTE(ulDestinationIP,2),
                 SL_IPV4_BYTE(ulDestinationIP,1),
                 SL_IPV4_BYTE(ulDestinationIP,0),
                 g_uiPortNum);


    //
    // Acknowledge the watchdog so that it doesn't resets
    //
    WatchdogAck();

    //
    // Send packets
    //
    BsdUdpClient(PORT_NUM,ulDestinationIP);


    //
    // power off the network processor
    //
    sl_Stop(SL_STOP_TIMEOUT);
    while (1)
    {
        _SlNonOsMainLoopTask();
    }
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
