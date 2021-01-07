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
// Application Name     -   Transceiver mode
// Application Overview -   This is a sample application demonstrating the
//                          use of raw sockets on a CC3200 device.Based on the
//                          user input, the application either transmits data
//                          on the channel requested or collects Rx statistics
//                          on the channel.
//
//*****************************************************************************


//*****************************************************************************
//
//! \addtogroup transceiver
//! @{
//
//*****************************************************************************

// Standard includes
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Simplelink includes
#include "simplelink.h"

//Driverlib includes
#include "hw_types.h"
#include "hw_ints.h"
#include "interrupt.h"
#include "utils.h"
#include "uart.h"
#include "hw_memmap.h"
#include "prcm.h"
#include "rom.h"
#include "rom_map.h"

//Common interface includes
#include "common.h"
#ifndef NOTERM
#include "uart_if.h"
#endif

#include "pinmux.h"


#define APPLICATION_NAME        "TRANSCEIVER_MODE"
#define APPLICATION_VERSION     "1.4.0"

#define PREAMBLE            1        /* Preamble value 0- short, 1- long */
#define CPU_CYCLES_1MSEC (80*1000)

// Application specific status/error codes
typedef enum{
    // Choosing -0x7D0 to avoid overlap w/ host-driver's error codes
    TX_CONTINUOUS_FAILED = -0x7D0,
    RX_STATISTICS_FAILED = TX_CONTINUOUS_FAILED - 1,
    DEVICE_NOT_IN_STATION_MODE = RX_STATISTICS_FAILED - 1,

    STATUS_CODE_MAX = -0xBB8
}e_AppStatusCodes;


typedef struct
{
    int choice;
    int channel;
    int packets;
    SlRateIndex_e rate;
    int Txpower;
}UserIn;

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
volatile unsigned long  g_ulStatus = 0;//SimpleLink Status
unsigned long  g_ulGatewayIP = 0; //Network Gateway IP address
unsigned char  g_ucConnectionSSID[SSID_LEN_MAX+1]; //Connection SSID
unsigned char  g_ucConnectionBSSID[BSSID_LEN_MAX]; //Connection BSSID

char RawData_Ping[] = {
       /*---- wlan header start -----*/
       0x88,                                /* version , type sub type */
       0x02,                                /* Frame control flag */
       0x2C, 0x00,
       0x00, 0x23, 0x75, 0x55,0x55, 0x55,   /* destination */
       0x00, 0x22, 0x75, 0x55,0x55, 0x55,   /* bssid */
       0x08, 0x00, 0x28, 0x19,0x02, 0x85,   /* source */
       0x80, 0x42, 0x00, 0x00,
       0xAA, 0xAA, 0x03, 0x00, 0x00, 0x00, 0x08, 0x00, /* LLC */
       /*---- ip header start -----*/
       0x45, 0x00, 0x00, 0x54, 0x96, 0xA1, 0x00, 0x00, 0x40, 0x01,
       0x57, 0xFA,                          /* checksum */
       0xc0, 0xa8, 0x01, 0x64,              /* src ip */
       0xc0, 0xa8, 0x01, 0x02,              /* dest ip  */
       /* payload - ping/icmp */
       0x08, 0x00, 0xA5, 0x51,
       0x5E, 0x18, 0x00, 0x00, 0x41, 0x08, 0xBB, 0x8D, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00};

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
static UserIn UserInput();
static int Tx_continuous(int iChannel,SlRateIndex_e rate,int iNumberOfPackets,\
                                   int iTxPowerLevel,long dIntervalMiliSec);
static int RxStatisticsCollect();
static void DisplayBanner(char * AppName);
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
            // available in 'slWlanConnectAsyncResponse_t' - Applications
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
                UART_PRINT("[WLAN EVENT]Device disconnected from the AP: %s,"
                           " BSSID: %x:%x:%x:%x:%x:%x on application's"
                           " request \n\r",
                           g_ucConnectionSSID,g_ucConnectionBSSID[0],
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


//*****************************************************************************
//
//! Application startup display on UART
//!
//! \param  AppName
//!
//! \return none
//!
//*****************************************************************************
static void DisplayBanner(char * AppName)
{
    UART_PRINT("\n\n\n\r");
    UART_PRINT("\t\t *************************************************\n\r");
    UART_PRINT("\t\t\t CC3200 %s Application       \n\r", AppName);
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
//! UserInput
//!
//! This function
//!        1. Function for reading the user input.
//!
//! \return none
//
//*****************************************************************************
static UserIn UserInput()
{
    UserIn User;
    char acCmdStore[512];
    int iRetVal;
    int iRightInput = 0;

    do
    {
        UART_PRINT("\r\nOptions:\r\n1. Send packets.\r\n2. Collect statistics "
                        "about received packets.\r\n");
        UART_PRINT("Enter the option to use: ");
        iRetVal = GetCmd(acCmdStore, sizeof(acCmdStore));
        if(iRetVal == 0)
        {
            //
            // No input. Just an enter pressed probably. Display a prompt.
            //
            UART_PRINT("\n\n\rEnter Valid Input.");
            iRightInput = 0;
        }
        else
        {
            User.choice = (int)strtoul(acCmdStore,0,10);
            if(User.choice != 1 && User.choice != 2)
            {
                UART_PRINT("\n\n\rWrong Input");
                iRightInput = 0;
            }
            else
            {
                iRightInput = 1;
            }
        }
        UART_PRINT("\n\r");
    }while(!iRightInput);



    if (User.choice == 1)
    {
        /*channel*/
        do
        {
            UART_PRINT("Enter the channel to use[1:13]: ");
            iRetVal = GetCmd(acCmdStore, sizeof(acCmdStore));
            if(iRetVal == 0)
            {
                //
                // No input. Just an enter pressed probably. Display a prompt.
                //
                UART_PRINT("\n\rEnter Valid Input.");
                iRightInput = 0;
            }
            else
            {
                User.channel = (int)strtoul(acCmdStore,0,10);
                if(User.channel <= 0 || User.channel > 13)
                {
                    UART_PRINT("\n\rWrong Input");
                    iRightInput = 0;
                }
                else
                {
                    iRightInput = 1;
                }
            }

            UART_PRINT("\r\n");
        }while(!iRightInput);

        /*Number of packets*/
        do
        {
            UART_PRINT("Enter the number of packets to send : ");
            iRetVal = GetCmd(acCmdStore, sizeof(acCmdStore));
            if(iRetVal == 0)
            {
                //
                // No input. Just an enter pressed probably. Display a prompt.
                //
                UART_PRINT("\n\rEnter Valid Input.");
                iRightInput = 0;
            }
            else
            {
                User.packets = (int)strtoul(acCmdStore,0,10);
                if(User.packets <= 0 && User.packets > 65535)
                {
                    UART_PRINT("\n\rWrong Input");
                    iRightInput = 0;
                }
                else
                {
                    iRightInput = 1;
                }
            }
            UART_PRINT("\r\n");
        }while(!iRightInput);

        /*Rate*/
        do
        {
            UART_PRINT("Enter the rate: ");
            iRetVal = GetCmd(acCmdStore, sizeof(acCmdStore));
            if(iRetVal == 0)
            {
                //
                // No input. Just an enter pressed probably. Display a prompt.
                //
                UART_PRINT("\n\rEnter Valid Input.");
                iRightInput = 0;
            }
            else
            {
                User.rate = (SlRateIndex_e)strtoul(acCmdStore,0,10);
                if(User.rate < RATE_1M || User.rate > RATE_MCS_7)
                {
                    UART_PRINT("\n\rWrong Input");
                    iRightInput = 0;
                }
                else
                {
                    iRightInput = 1;
                }
            }

            UART_PRINT("\r\n");
        }while(!iRightInput);
        /* Tx Power level */
        do
        {
            UART_PRINT("Enter the Tx power[0:15]: ");
            iRetVal = GetCmd(acCmdStore, sizeof(acCmdStore));
            if(iRetVal == 0)
            {
                //
                // No input. Just an enter pressed probably. Display a prompt.
                //
                UART_PRINT("\n\rEnter Valid Input.");
                iRightInput = 0;
            }
            else
            {
                User.Txpower = (int)strtoul(acCmdStore,0,10);
                if(User.Txpower < 0 || User.Txpower > 15)
                {
                    UART_PRINT("\n\rWrong Input");
                    iRightInput = 0;
                }
                else
                {
                    iRightInput = 1;
                }
            }

            UART_PRINT("\r\n");
        }while(!iRightInput);
    }

    return User;
}

//*****************************************************************************
//
//! Tx_continuous
//!
//! This function
//!        1. Function for sending out pinging data on the
//!                channel given by the user.
//!
//! \return none
//
//*****************************************************************************
static int Tx_continuous(int iChannel,SlRateIndex_e rate,int iNumberOfPackets,
                                    int iTxPowerLevel,long dIntervalMiliSec)
{
    int iSoc;
    long lRetVal = -1;
    long ulIndex;

    iSoc = sl_Socket(SL_AF_RF,SL_SOCK_RAW,iChannel);
    ASSERT_ON_ERROR(iSoc);

    UART_PRINT("Transmitting data...\r\n");
    for(ulIndex = 0 ; ulIndex < iNumberOfPackets ; ulIndex++)
    {
        lRetVal = sl_Send(iSoc,RawData_Ping,sizeof(RawData_Ping),\
                   SL_RAW_RF_TX_PARAMS(iChannel,  rate, iTxPowerLevel, PREAMBLE));
        if(lRetVal < 0)
        {
            sl_Close(iSoc);
            ASSERT_ON_ERROR(lRetVal);
        }
        //Sleep(dIntervalMiliSec);
        MAP_UtilsDelay(4000000);
    }

    lRetVal = sl_Close(iSoc);
    ASSERT_ON_ERROR(lRetVal);

    UART_PRINT("Transmission complete.\r\n");
    return SUCCESS;
}

//*****************************************************************************
//
//! RxStatisticsCollect
//!
//! This function
//!        1. Function for performing the statistics by listening on the
//!                channel given by the user.
//!
//! \return none
//
//*****************************************************************************
static int RxStatisticsCollect()
{
    int iSoc;
    char acBuffer[1500];
    SlGetRxStatResponse_t rxStatResp;
    //char cChar;
    int iIndex;
    int iChannel;
    long lRetVal = -1;
    int iRightInput = 0;
    char acCmdStore[512];
    struct SlTimeval_t timeval;

    timeval.tv_sec =  0;             // Seconds
    timeval.tv_usec = 20000;         // Microseconds.

    //
    //Clear all fields to defaults
    //
    rxStatResp.ReceivedValidPacketsNumber = 0;
    rxStatResp.ReceivedFcsErrorPacketsNumber = 0;
    rxStatResp.ReceivedAddressMismatchPacketsNumber = 0;
    rxStatResp.AvarageMgMntRssi = 0;
    rxStatResp.AvarageDataCtrlRssi = 0;
    for(iIndex = 0 ; iIndex < SIZE_OF_RSSI_HISTOGRAM ; iIndex++)
    {
        rxStatResp.RssiHistogram[iIndex] = 0;
    }
    for(iIndex = 0 ; iIndex < NUM_OF_RATE_INDEXES ; iIndex++)
    {
        rxStatResp.RateHistogram[iIndex] = 0;
    }
    rxStatResp.GetTimeStamp = 0;
    rxStatResp.StartTimeStamp = 0;

    //
    //Prompt the user for channel number
    //
    do
    {
        UART_PRINT("\n\rEnter the channel to listen[1-13]:");
        //
        // Wait to receive a character over UART
        //
        lRetVal = GetCmd(acCmdStore, sizeof(acCmdStore));
        if(lRetVal == 0)
        {
            //
            // No input. Just an enter pressed probably. Display a prompt.
            //
            UART_PRINT("\n\rEnter Valid Input.");
            iRightInput = 0;
        }
        else
        {
            iChannel = (int)strtoul(acCmdStore,0,10);
            if(iChannel <= 0 || iChannel > 13)
            {
                UART_PRINT("\n\rWrong Input");
                iRightInput = 0;
            }
            else
            {
                iRightInput = 1;
            }
        }

    }while(!iRightInput);


    UART_PRINT("\n\rPress any key to start collecting statistics...");

    //
    // Wait to receive a character over UART
    //
    MAP_UARTCharGet(CONSOLE);

    //
    //Start Rx statistics collection
    //
    lRetVal = sl_WlanRxStatStart();
    ASSERT_ON_ERROR(lRetVal);

    //
    //Open Socket on the channel to listen
    //
    iSoc = sl_Socket(SL_AF_RF,SL_SOCK_RAW,iChannel);
    ASSERT_ON_ERROR(iSoc);

    // Enable receive timeout
    lRetVal = sl_SetSockOpt(iSoc,SL_SOL_SOCKET,SL_SO_RCVTIMEO, &timeval, \
                                  sizeof(timeval));
    ASSERT_ON_ERROR(lRetVal);

    lRetVal = sl_Recv(iSoc,acBuffer,1470,0);
    if(lRetVal < 0 && lRetVal != SL_EAGAIN)
    {
        //error
        ASSERT_ON_ERROR(sl_Close(iSoc));
        ASSERT_ON_ERROR(lRetVal);
    }

    UART_PRINT("\n\rPress any key to stop and display the statistics...");

    //
    // Wait to receive a character over UART
    //
    MAP_UARTCharGet(CONSOLE);

    //
    //Get the Statistics collected in this time window
    //
    lRetVal = sl_WlanRxStatGet(&rxStatResp,0);
    if(lRetVal < 0)
    {
        //error
        ASSERT_ON_ERROR(sl_Close(iSoc));
        ASSERT_ON_ERROR(lRetVal);
    }

    //
    //Printing the collected statistics
    //
    UART_PRINT("\n\n\n\r\t\t=========================================== \n \
               \r");
    UART_PRINT("\n\r\t\t\t\tRx Statistics \n\r");
    UART_PRINT("\t\t=========================================== \n\r");

    UART_PRINT("\n\n\rThe data sampled over %ld microsec\n\n\r",    \
               (unsigned int)(rxStatResp.GetTimeStamp -
                              rxStatResp.StartTimeStamp));

    UART_PRINT("Number of Valid Packets Received:  %d\n\r",
                            rxStatResp.ReceivedValidPacketsNumber);
    UART_PRINT("Number of Packets Received Packets with FCS: %d\n\r",
                   rxStatResp.ReceivedFcsErrorPacketsNumber);
    UART_PRINT("Number of Packets Received Packets with PLCP: %d\n\n\r",   \
                   rxStatResp.ReceivedAddressMismatchPacketsNumber);
    UART_PRINT("Average Rssi for management packets: %d\
                    \n\rAverage Rssi for other packets: %d\n\r",
                   rxStatResp.AvarageMgMntRssi,rxStatResp.AvarageDataCtrlRssi);
    for(iIndex = 0 ; iIndex < SIZE_OF_RSSI_HISTOGRAM ; iIndex++)
    {
        UART_PRINT("Number of packets with RSSI in range %d dbm - %d dbm: %d\n\r",
                     ((-40+(-8*iIndex))),((-40+(-8*(iIndex+1)))+1),
                       rxStatResp.RssiHistogram[iIndex]);
    }
    UART_PRINT("\n\r");

    //for(iIndex = 0 ; iIndex < NUM_OF_RATE_INDEXES ; iIndex++)
    iIndex = 0;
    {
        UART_PRINT("Number of Packets with Rate 1Mbps  : %d\n\r",
                 rxStatResp.RateHistogram[iIndex++]);
        UART_PRINT("Number of Packets with Rate 2Mbps  : %d\n\r",
                 rxStatResp.RateHistogram[iIndex++]);
        UART_PRINT("Number of Packets with Rate 5.5Mbps  : %d\n\r",
                 rxStatResp.RateHistogram[iIndex++]);
        UART_PRINT("Number of Packets with Rate 11Mbps  : %d\n\r",
                 rxStatResp.RateHistogram[iIndex++]);
        UART_PRINT("Number of Packets with Rate 6Mbps  : %d\n\r",
                 rxStatResp.RateHistogram[iIndex++]);
        UART_PRINT("Number of Packets with Rate 9Mbps  : %d\n\r",
                 rxStatResp.RateHistogram[iIndex++]);
        UART_PRINT("Number of Packets with Rate 12Mbps  : %d\n\r",
                 rxStatResp.RateHistogram[iIndex++]);
        UART_PRINT("Number of Packets with Rate 18Mbps  : %d\n\r",
                 rxStatResp.RateHistogram[iIndex++]);
        UART_PRINT("Number of Packets with Rate 24Mbps  : %d\n\r",
                 rxStatResp.RateHistogram[iIndex++]);
        UART_PRINT("Number of Packets with Rate 36Mbps  : %d\n\r",
                 rxStatResp.RateHistogram[iIndex++]);
        UART_PRINT("Number of Packets with Rate 48Mbps  : %d\n\r",
                 rxStatResp.RateHistogram[iIndex++]);
        UART_PRINT("Number of Packets with Rate 54Mbps  : %d\n\r",
                 rxStatResp.RateHistogram[iIndex++]);
        UART_PRINT("Number of Packets with Rate MCS_0  : %d\n\r",
                 rxStatResp.RateHistogram[iIndex++]);
        UART_PRINT("Number of Packets with Rate MCS_1  : %d\n\r",
                 rxStatResp.RateHistogram[iIndex++]);
        UART_PRINT("Number of Packets with Rate MCS_2  : %d\n\r",
                 rxStatResp.RateHistogram[iIndex++]);
        UART_PRINT("Number of Packets with Rate MCS_3  : %d\n\r",
                 rxStatResp.RateHistogram[iIndex++]);
        UART_PRINT("Number of Packets with Rate MCS_4  : %d\n\r",
                 rxStatResp.RateHistogram[iIndex++]);
        UART_PRINT("Number of Packets with Rate MCS_5  : %d\n\r",
                 rxStatResp.RateHistogram[iIndex++]);
        UART_PRINT("Number of Packets with Rate MCS_6  : %d\n\r",
                 rxStatResp.RateHistogram[iIndex++]);
        UART_PRINT("Number of Packets with Rate MCS_7  : %d\n\r",
                 rxStatResp.RateHistogram[iIndex++]);
     }

    //
    //Stop Rx statistics collection
    //
    lRetVal = sl_WlanRxStatStop();
    ASSERT_ON_ERROR(lRetVal);

    //
    //Close the socket
    //
    lRetVal = sl_Close(iSoc);
    ASSERT_ON_ERROR(lRetVal);

    return SUCCESS;
}

//*****************************************************************************
//
//! main
//!
//! This function
//!        1. Main function for the application.
//!        2. Make sure there is no profile before activating this test.
//!     3. This test is not optimized for current consumption at this stage.
//!
//! \return none
//
//*****************************************************************************

int main()
{
    UserIn User;
    int iFlag = 1;
    long lRetVal = -1;
    char cChar;
    unsigned char policyVal;
    
    //
    // Initialize Board configuration
    //
    BoardInit();
    
    //
    //
    //Pin muxing
    //
    PinMuxConfig();
    
    // Configuring UART
    //
    InitTerm();
    DisplayBanner(APPLICATION_NAME);

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
          UART_PRINT("Failed to configure the device in its default state \n\r");
          LOOP_FOREVER();
    }

    UART_PRINT("Device is configured in default state \n\r");

    CLR_STATUS_BIT_ALL(g_ulStatus);

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
    // reset all network policies
    //
    lRetVal = sl_WlanPolicySet(  SL_POLICY_CONNECTION,
                  SL_CONNECTION_POLICY(0,0,0,0,0),
                  &policyVal,
                  1 /*PolicyValLen*/);
    if (lRetVal < 0)
    {
        UART_PRINT("Failed to set policy \n\r");
        LOOP_FOREVER();
    }

    while (iFlag)
    { 
    User = UserInput();

    switch(User.choice)
    {
    case(1):

        /*******An example of Tx continuous on user selected channel, rate 11,
        * user selected number of packets, minimal delay between packets*******/
        lRetVal = Tx_continuous(User.channel,User.rate,User.packets, \
                             User.Txpower,0);
        if(lRetVal < 0)
        {
            UART_PRINT("Error during transmission of raw data\n\r");
            LOOP_FOREVER();
        }
        break;
    case(2):

        /******An example of Rx statistics using user selected channel *******/
        lRetVal = RxStatisticsCollect();
        if(lRetVal < 0)
        {
            UART_PRINT("Error while collecting statistics data\n\r");
            LOOP_FOREVER();
        }
        break;
    }

    UART_PRINT("\n\rEnter \"1\" to restart or \"0\" to quit: ");
    //
    // Wait to receive a character over UART
    //
    cChar = MAP_UARTCharGet(CONSOLE);
    //
    // Echo the received character
    //
    MAP_UARTCharPut(CONSOLE, cChar);
    UART_PRINT("\n\r");
    iFlag = atoi(&cChar);
    }
    UART_PRINT("\r\nEnding the application....");
    //
    // power off network processor
    //
    lRetVal = sl_Stop(SL_STOP_TIMEOUT);

    LOOP_FOREVER();

}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
