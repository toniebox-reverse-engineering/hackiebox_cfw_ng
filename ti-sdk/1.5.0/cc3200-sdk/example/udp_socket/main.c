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
// Application Name     - UDP Socket
// Application Overview - This particular application illustrates how this
//                        device can be used as a client or server for UDP
//                        communication.
//
//*****************************************************************************


//****************************************************************************
//
//! \addtogroup udp_socket
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

// common interface includes
#include "udma_if.h"
#include "common.h"
#ifndef NOTERM
#include "uart_if.h"
#endif

#include "pinmux.h"


#define APPLICATION_NAME        "UDP Socket"
#define APPLICATION_VERSION     "1.4.0"

#define IP_ADDR            0xc0a80064 /* 192.168.0.100 */
#define PORT_NUM           5001
#define BUF_SIZE           1400
#define UDP_PACKET_COUNT   1000

// Application specific status/error codes
typedef enum{
    // Choosing -0x7D0 to avoid overlap w/ host-driver's error codes
    SOCKET_CREATE_ERROR = -0x7D0,
    BIND_ERROR = SOCKET_CREATE_ERROR - 1,
    SEND_ERROR = BIND_ERROR - 1,
    RECV_ERROR = SEND_ERROR -1,
    SOCKET_CLOSE = RECV_ERROR -1,  
    DEVICE_NOT_IN_STATION_MODE = SOCKET_CLOSE - 1,
    STATUS_CODE_MAX = -0xBB8
}e_AppStatusCodes;



//****************************************************************************
//                      LOCAL FUNCTION PROTOTYPES
//****************************************************************************
int BsdUdpClient(unsigned short usPort);
int BsdUdpServer(unsigned short usPort);
static long WlanConnect();
static void DisplayBanner();
static void BoardInit();
static void InitializeAppVariables();
static long ConfigureSimpleLinkToDefaultState();


//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
volatile unsigned long  g_ulStatus = 0;//SimpleLink Status
unsigned long  g_ulGatewayIP = 0; //Network Gateway IP address
unsigned char  g_ucConnectionSSID[SSID_LEN_MAX+1]; //Connection SSID
unsigned char  g_ucConnectionBSSID[BSSID_LEN_MAX]; //Connection BSSID
unsigned long  g_ulDestinationIp = IP_ADDR;        // Client IP address
unsigned int   g_uiPortNum = PORT_NUM;
volatile unsigned long  g_ulPacketCount = UDP_PACKET_COUNT;
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
//                 GLOBAL VARIABLES -- End
//*****************************************************************************



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

            UART_PRINT("[NETAPP EVENT] IP Acquired: IP=%d.%d.%d.%d , "
                        "Gateway=%d.%d.%d.%d\n\r",

                        SL_IPV4_BYTE(g_ulIpAddr,3),
                        SL_IPV4_BYTE(g_ulIpAddr,2),
                        SL_IPV4_BYTE(g_ulIpAddr,1),
                        SL_IPV4_BYTE(g_ulIpAddr,0),
                        SL_IPV4_BYTE(g_ulGatewayIP,3),
                        SL_IPV4_BYTE(g_ulGatewayIP,2),
                        SL_IPV4_BYTE(g_ulGatewayIP,1),
                        SL_IPV4_BYTE(g_ulGatewayIP,0));
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
    //
    // This application doesn't work w/ socket - Events are not expected
    //
 
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
    g_ulDestinationIp = IP_ADDR;
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
//!    \brief Parse the input IP address from the user
//!
//!    \param[in]                     ucCMD (char pointer to input string)
//!
//!    \return                        0 : if correct IP, -1 : incorrect IP
//
//****************************************************************************
int IpAddressParser(char *ucCMD)
{
    int i=0;
    unsigned int uiUserInputData;
    unsigned long ulUserIpAddress = 0;
    char *ucInpString;
    ucInpString = strtok(ucCMD, ".");
    uiUserInputData = (int)strtoul(ucInpString,0,10);
    while(i<4)
    {
        //
       // Check Whether IP is valid
       //
       if((ucInpString != NULL) && (uiUserInputData < 256))
       {
           ulUserIpAddress |= uiUserInputData;
           if(i < 3)
               ulUserIpAddress = ulUserIpAddress << 8;
           ucInpString=strtok(NULL,".");
           uiUserInputData = (int)strtoul(ucInpString,0,10);
           i++;
       }
       else
       {
           return -1;
       }
    }
    g_ulDestinationIp = ulUserIpAddress;
    return SUCCESS;
}

//*****************************************************************************
//
//! UserInput
//!
//! This function
//!        1. Function for reading the user input for UDP RX/TX
//!
//! \return none
//
//*****************************************************************************
long UserInput()
{
    int iInput = 0;
    char acCmdStore[50];
    int lRetVal;
    int iRightInput = 0;
    unsigned long ulUserInputData = 0;

    UART_PRINT("Default settings: SSID Name: %s, PORT = %d, Packet Count = %d, "
                  "Destination IP: %d.%d.%d.%d\n\r",
            SSID_NAME, g_uiPortNum, g_ulPacketCount,
            SL_IPV4_BYTE(g_ulDestinationIp,3),
            SL_IPV4_BYTE(g_ulDestinationIp,2),
            SL_IPV4_BYTE(g_ulDestinationIp,1),
            SL_IPV4_BYTE(g_ulDestinationIp,0));

    do
    {
        UART_PRINT("\r\nOptions:\r\n1. Send UDP packets.\r\n2. Receive UDP "
                    "packets.\r\n3. Settings.\r\n4. Exit\r\n");
        UART_PRINT("Enter the option to use: ");
        lRetVal = GetCmd(acCmdStore, sizeof(acCmdStore));
        if(lRetVal == 0)
        {
          //
          // No input. Just an enter pressed probably. Display a prompt.
          //
          UART_PRINT("\n\n\rEnter Valid Input.");
        }
        else
        {
            iInput  = (int)strtoul(acCmdStore,0,10);
            if(iInput  == 1)
            {
                UART_PRINT("Run iperf command \"iperf.exe -u -s -i 1\" and "
                            "press Enter\n\r");
                //
                // Wait to receive a character over UART
                //
                MAP_UARTCharGet(CONSOLE);
                UART_PRINT("Sending UDP packets...\n\r");

                // Before proceeding, please make sure to have a server 
                // waiting on PORT_NUM
                lRetVal = BsdUdpClient(g_uiPortNum);
                ASSERT_ON_ERROR(lRetVal);
            }
            else if(iInput  == 2)
            {
                UART_PRINT("Run iperf command \"iperf.exe -u -c %d.%d.%d.%d -i 1 "
                            "-t 100000\" and press Enter\n\r",
                          SL_IPV4_BYTE(g_ulIpAddr,3), SL_IPV4_BYTE(g_ulIpAddr,2),
                          SL_IPV4_BYTE(g_ulIpAddr,1), SL_IPV4_BYTE(g_ulIpAddr,0));
                
                //
                // Wait to receive a character over UART
                //
                MAP_UARTCharGet(CONSOLE);
                UART_PRINT("Receiving UDP packets...\n\r");
                
                // After calling this function, you can start sending data 
                // to CC3200 IP address on PORT_NUM
                lRetVal = BsdUdpServer(g_uiPortNum);
                ASSERT_ON_ERROR(lRetVal);
            }
            else if(iInput  == 3)
            {
              iRightInput = 0;
                do
                {
                    UART_PRINT("\n\rSetting Options:\n\r1. PORT\n\r2. Packet "
                               "Count\n\r3. Destination IP\n\r4. Main Menu\r\n");
                    UART_PRINT("Enter the option to use: ");
                    lRetVal = GetCmd(acCmdStore, sizeof(acCmdStore));
                    if(lRetVal == 0)
                    {
                        //
                        // No input. Just an enter pressed probably. Display prompt.
                        //
                        UART_PRINT("\n\n\rEnter Valid Input.");
                    }
                    else
                    {

                        iInput  = (int)strtoul(acCmdStore,0,10);
                        //SettingInput(iInput);
                        switch(iInput)
                        {
                            case 1:
                            do
                            {
                                UART_PRINT("Enter new Port: ");
                                lRetVal = GetCmd(acCmdStore, sizeof(acCmdStore));
                                if(lRetVal == 0)
                                {
                                  //
                                  // No input. Just an enter pressed probably. 
                                  // Display a prompt.
                                  //
                                  UART_PRINT("\n\rEnter Valid Input.");
                                  iRightInput = 0;
                                }
                                else
                                {
                                  ulUserInputData = (int)strtoul(acCmdStore,0,10);
                                  if(ulUserInputData <= 0 || ulUserInputData > 65535)
                                  {
                                    UART_PRINT("\n\rWrong Input");
                                    iRightInput = 0;
                                  }
                                  else
                                  {
                                    g_uiPortNum = ulUserInputData;
                                    iRightInput = 1;
                                  }
                                }

                                UART_PRINT("\r\n");
                            }while(!iRightInput);

                            iRightInput = 0;
                            break;
                        case 2:
                            do
                            {
                                UART_PRINT("Enter Packet Count: ");
                                lRetVal = GetCmd(acCmdStore, sizeof(acCmdStore));
                                if(lRetVal == 0)
                                {
                                  //
                                  // No input. Just an enter pressed probably.
                                  // Display a prompt.
                                  //
                                  UART_PRINT("\n\rEnter Valid Input.");
                                  iRightInput = 0;
                                }
                                else
                                {
                                    ulUserInputData = (int)strtoul(acCmdStore,0,10);
                                  if(ulUserInputData <= 0 || ulUserInputData > 9999999)
                                  {
                                    UART_PRINT("\n\rWrong Input");
                                    iRightInput = 0;
                                  }
                                  else
                                  {
                                      g_ulPacketCount = ulUserInputData;
                                    iRightInput = 1;
                                  }
                                }

                                UART_PRINT("\r\n");
                            }while(!iRightInput);
                            iRightInput = 0;
                            break;
                        case 3:
                            do
                            {
                                UART_PRINT("Enter Destination IP: ");
                                lRetVal = GetCmd(acCmdStore, sizeof(acCmdStore));
                                if(lRetVal == 0)
                                {
                                  //
                                  // No input. Just an enter pressed probably. 
                                  // Display a prompt.
                                  //
                                  UART_PRINT("\n\rEnter Valid Input.");
                                  iRightInput = 0;
                                }
                                else
                                {
                                if(IpAddressParser(acCmdStore) < 0)
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
                            iRightInput = 0;
                            break;
                        case 4:
                            iRightInput = 1;
                            break;
                            
                        default:
                            break;

                        }

                    }
                }while(!iRightInput);

            }
            else if(iInput == 4)
            {
              break;
            }
            else
            {
              UART_PRINT("\n\n\rWrong Input");
            }
        }
        UART_PRINT("\n\r");
    }while(1);

    return 0 ;

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
int BsdUdpClient(unsigned short usPort)
{
    int             iCounter;
    short           sTestBufLen;
    SlSockAddrIn_t  sAddr;
    int             iAddrSize;
    int             iSockID;
    int             iStatus;
    unsigned long   lLoopCount = 0;

    // filling the buffer
    for (iCounter=0 ; iCounter<BUF_SIZE ; iCounter++)
    {
        g_cBsdBuf[iCounter] = (char)(iCounter % 10);
    }

    sTestBufLen  = BUF_SIZE;

    //filling the UDP server socket address
    sAddr.sin_family = SL_AF_INET;
    sAddr.sin_port = sl_Htons((unsigned short)usPort);
    sAddr.sin_addr.s_addr = sl_Htonl((unsigned int)g_ulDestinationIp);

    iAddrSize = sizeof(SlSockAddrIn_t);

    // creating a UDP socket
    iSockID = sl_Socket(SL_AF_INET,SL_SOCK_DGRAM, 0);
    if( iSockID < 0 )
    {
        // error
        ASSERT_ON_ERROR(SOCKET_CREATE_ERROR);
    }

    // for a UDP connection connect is not required
    // sending 1000 packets to the UDP server
    while (lLoopCount < g_ulPacketCount)
    {
        g_cBsdBuf[0] = lLoopCount >> 24  & 0xFF;
        g_cBsdBuf[1] = lLoopCount >> 16  & 0xFF;
        g_cBsdBuf[2] = lLoopCount >> 8  & 0xFF;
        g_cBsdBuf[3] = lLoopCount & 0xFF;
        
		// sending packet
        iStatus = sl_SendTo(iSockID, g_cBsdBuf, sTestBufLen, 0,
                                (SlSockAddr_t *)&sAddr, iAddrSize);
        if( iStatus <= 0 )
        {
            // error
            sl_Close(iSockID);
            ASSERT_ON_ERROR(SEND_ERROR);
        }
        lLoopCount++;
    }

    UART_PRINT("Sent %u packets successfully\n\r",g_ulPacketCount);

    //closing the socket after sending 1000 packets
    sl_Close(iSockID);

    return SUCCESS;
}

//****************************************************************************
//
//! \brief Opening a UDP server side socket and receiving data
//!
//! This function opens a UDP socket in Listen mode and waits for an incoming
//! UDP connection.
//!    If a socket connection is established then the function will try to
//!    read 1000 UDP packets from the connected client.
//!
//! \param[in]          port number on which the server will be listening on
//!
//! \return             0 on success, Negative value on Error.
//
//****************************************************************************
int BsdUdpServer(unsigned short usPort)
{
    SlSockAddrIn_t  sAddr;
    SlSockAddrIn_t  sLocalAddr;
    int             iCounter;
    int             iAddrSize;
    int             iSockID;
    int             iStatus;
    long            lLoopCount = 0;
    short           sTestBufLen;

    // filling the buffer
    for (iCounter=0 ; iCounter<BUF_SIZE ; iCounter++)
    {
        g_cBsdBuf[iCounter] = (char)(iCounter % 10);
    }

    sTestBufLen  = BUF_SIZE;
    //filling the UDP server socket address
    sLocalAddr.sin_family = SL_AF_INET;
    sLocalAddr.sin_port = sl_Htons((unsigned short)usPort);
    sLocalAddr.sin_addr.s_addr = 0;

    iAddrSize = sizeof(SlSockAddrIn_t);

    // creating a UDP socket
    iSockID = sl_Socket(SL_AF_INET,SL_SOCK_DGRAM, 0);
    if( iSockID < 0 )
    {
        // error
        ASSERT_ON_ERROR(SOCKET_CREATE_ERROR);
    }

    // binding the UDP socket to the UDP server address
    iStatus = sl_Bind(iSockID, (SlSockAddr_t *)&sLocalAddr, iAddrSize);
    if( iStatus < 0 )
    {
        // error
        sl_Close(iSockID);
        ASSERT_ON_ERROR(BIND_ERROR);
    }

    // no listen or accept is required as UDP is connectionless protocol
    /// waits for 1000 packets from a UDP client
    while (lLoopCount < g_ulPacketCount)
    {
        iStatus = sl_RecvFrom(iSockID, g_cBsdBuf, sTestBufLen, 0,
                     ( SlSockAddr_t *)&sAddr, (SlSocklen_t*)&iAddrSize );

    if( iStatus < 0 )
    {
        // error
        sl_Close(iSockID);
        ASSERT_ON_ERROR(RECV_ERROR);
    }
    lLoopCount++;
    }

    UART_PRINT("Recieved %u packets successfully\n\r",g_ulPacketCount);

    //closing the socket after receiving 1000 packets
    sl_Close(iSockID);

    return SUCCESS;
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
    long lRetVal = 0;

    secParams.Key = (signed char*)SECURITY_KEY;
    secParams.KeyLen = strlen(SECURITY_KEY);
    secParams.Type = SECURITY_TYPE;

    lRetVal = sl_WlanConnect((signed char*)SSID_NAME, strlen(SSID_NAME), 0, \
                                    &secParams, 0);
    ASSERT_ON_ERROR(lRetVal);

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
    UART_PRINT("\t\t      CC3200 %s Application       \n\r", AppName);
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

//****************************************************************************
//                            MAIN FUNCTION
//****************************************************************************
void main()
{
    long lRetVal = -1;

    //
    // Board Initialization
    //
    BoardInit();

    //
    // uDMA Initialization
    //
    UDMAInit();

    //
    // Configure the pinmux settings for the peripherals exercised
    //
    PinMuxConfig();

    //
    // Configuring UART
    //
    InitTerm();

    //
    // Display banner
    //
    DisplayBanner(APPLICATION_NAME);

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
    lRetVal = ConfigureSimpleLinkToDefaultState();

    if(lRetVal < 0)
    {
        if (DEVICE_NOT_IN_STATION_MODE == lRetVal)
          UART_PRINT("Failed to configure the device in its default state \n\r");

        LOOP_FOREVER();
    }

    UART_PRINT("Device is configured in default state \n\r");

    //
    // Asumption is that the device is configured in station mode already
    // and it is in its default state
    //
    lRetVal = sl_Start(0, 0, 0);
    if (lRetVal < 0 || lRetVal != ROLE_STA)
    {
        UART_PRINT("Failed to start the device \n\r");
        LOOP_FOREVER();
    }

    UART_PRINT("Device started as STATION \n\r");

    UART_PRINT("Connecting to AP: %s ...\r\n",SSID_NAME);

    //
    //Connecting to WLAN AP
    //
    lRetVal = WlanConnect();
    if(lRetVal < 0)
    {
        UART_PRINT("Failed to establish connection w/ an AP \n\r");
        LOOP_FOREVER();
    }

    UART_PRINT("Connected to AP: %s \n\r",SSID_NAME);

    UART_PRINT("Device IP: %d.%d.%d.%d\n\r\n\r",
                SL_IPV4_BYTE(g_ulIpAddr,3),
                SL_IPV4_BYTE(g_ulIpAddr,2),
                SL_IPV4_BYTE(g_ulIpAddr,1),
                SL_IPV4_BYTE(g_ulIpAddr,0));

#ifdef USER_INPUT_ENABLE
    lRetVal = UserInput();
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }

#else
    lRetVal = BsdUdpClient(PORT_NUM);
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }

    lRetVal = BsdUdpServer(PORT_NUM);
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }
#endif

    UART_PRINT("Exiting Application ...\n\r");

    //
    // power off the network processor
    //
    lRetVal = sl_Stop(SL_STOP_TIMEOUT);

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
