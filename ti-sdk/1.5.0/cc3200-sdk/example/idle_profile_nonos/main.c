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
// Application Name     - Idle Profile Non Os
// Application Overview - Idle profile enables the user to measure current 
//                        values, power consumption and other such parameters 
//                        for CC3200, when the device is essentially idle(both 
//                        NWP and APPS subsystems in low power deep sleep 
//                        condition). The other main objective behind this 
//                        application is to introduce the user to the easily 
//                        configurable power management framework. This
//                        example is for non os environment.
//
//*****************************************************************************

//****************************************************************************
//
//! \addtogroup idle_profile_nonos
//! @{
//
//****************************************************************************

// Standard includes
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

//driverlib includes
#include "hw_ints.h"
#include "hw_types.h"
#include "hw_memmap.h"
#include "interrupt.h"
#include "prcm.h"
#include "utils.h"
#include "rom.h"
#include "rom_map.h"

// Simplelink includes
#include "simplelink.h"

//middleware includes
#include "cc_types.h"
#include "cc_pm.h"
#include "cc_pm_ops.h"

// Common interface includes
#include "uart_if.h"
#include "udma_if.h"

#include "pinmux.h"
#include "common.h"

#define APPLICATION_VERSION "1.4.0"

//
// Values for below macros shall be modified as per access-point(AP) properties
// SimpleLink device will connect to following AP when application is executed
//
#define GPIO_SRC_WKUP           PRCM_LPDS_GPIO13  
#define LPDS_DUR_SEC            5
#define FOREVER                 1
#define BUFF_SIZE               1472

#define WK_LPDS 				0   /*Wake from LPDS */
#define WK_HIB					1   /*Wake from Hibernate */

/* commands */
#define APP_SLEEP				0x8     
#define APP_SEND				0x10
#define APP_RECV				0x20
#define APP_ERROR				0x40

/* udp transmission related parameters */
#define IP_ADDR                 0xFFFFFFFF /* broadcast */
#define PORT_NUM                5001
#define BUF_SIZE                1400
#define UDP_PACKET_COUNT        1000

enum ap_events{
    EVENT_CONNECTION = 0x1,
    EVENT_DISCONNECTION = 0x2,
    EVENT_IP_ACQUIRED = 0x4,
    WDOG_EXPIRED = 0x8,
    CONNECTION_FAILED = 0x10
};

//****************************************************************************
//                      LOCAL FUNCTION PROTOTYPES
//****************************************************************************
void sl_WlanEvtHdlr(SlWlanEvent_t *pSlWlanEvent);
void sl_NetAppEvtHdlr(SlNetAppEvent_t *pNetAppEvent);
static void DisplayBanner();
static long WlanConnect();
//void WlanDisconnect();
static void BoardInit();
void SwitchToStaMode(int iMode);

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
#ifndef USE_TIRTOS
#if defined(ccs) || defined(gcc)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif
#endif

unsigned long g_ulIpAddr = 0;
unsigned long g_ulStatus = 0;
char g_cErrBuff[100];
unsigned long  g_ulDestinationIp = IP_ADDR;        // Client IP address
unsigned int   g_uiPortNum = PORT_NUM;
unsigned long  g_ulPacketCount = UDP_PACKET_COUNT;
char g_cBsdBuf[BUF_SIZE];

//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************

//*****************************************************************************
//                      Local Function Prototypes
//*****************************************************************************
extern void lp3p0_setup_power_policy(int power_policy);
extern int platform_init();
extern int set_rtc_as_wk_src(int pwr_mode, unsigned long time_interval,
                             bool is_periodic);
extern int set_gpio_as_wk_src(int pwr_mode, unsigned long wk_pin,
                              unsigned long trigger_type);
extern void set_host_irq_as_lpds_wk_src();

//*****************************************************************************
//
//! DisplayBanner
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
static void
DisplayBanner()
{
    UART_PRINT("\n\n\n\r");
    UART_PRINT("\t\t   *********************************************\n\r");
    UART_PRINT("\t\t     CC3200 Idle Profile Non Os Application   \n\r");
    UART_PRINT("\t\t   *********************************************\n\r");
    UART_PRINT("\n\n\n\r");

}

//*****************************************************************************
//
//! Application defined idle task hook
//! 
//! \param  none
//! 
//! \return none
//!
//*****************************************************************************
void SimpleLinkSyncWaitLoopCallback()
{
    //
    // setting host IRQ as a wake up src
    //
	set_host_irq_as_lpds_wk_src(true);
    
    //
    // executing idle task
    //
    cc_idle_task_pm();
    
    //
    // removing host IRQ as a wake ip src
    //
    set_host_irq_as_lpds_wk_src(false);
}

//****************************************************************************
//
//! \brief This function handles WLAN events
//!
//! \param[in]  pSlWlanEvent is the event passed to the handler
//!
//! \return    None
//
//****************************************************************************
void sl_WlanEvtHdlr(SlWlanEvent_t *pSlWlanEvent)
{
    switch(pSlWlanEvent->Event)
    {
    case SL_WLAN_CONNECT_EVENT:
        UART_PRINT("C\n\r");
        break;
    case SL_WLAN_DISCONNECT_EVENT:
        UART_PRINT("D\n\r");
        break;
    default:
        break;
    }
}

//****************************************************************************
//
//! \brief This function handles events for IP address acquisition via DHCP
//!           indication
//!
//! \param[in]    pNetAppEvent is the event passed to the handler
//!
//! \return     None
//
//****************************************************************************
void sl_NetAppEvtHdlr(SlNetAppEvent_t *pNetAppEvent)
{
    switch(pNetAppEvent->Event)
    {
    case SL_NETAPP_IPV4_IPACQUIRED_EVENT:
    case SL_NETAPP_IPV6_IPACQUIRED_EVENT:
        g_ulIpAddr = pNetAppEvent->EventData.ipAcquiredV4.ip;
        g_ulStatus = 1;
        UART_PRINT("IP: %d.%d.%d.%d \n\r",(g_ulIpAddr&0xFF000000)>>24,
                              (g_ulIpAddr&0x00FF0000)>>16,
                              (g_ulIpAddr&0x0000FF00)>>8,
                              (g_ulIpAddr&0x000000FF));
        break;
    default:
        break;
    }
}

//*****************************************************************************
//
//! This function gets triggered when HTTP Server receives Application
//! defined GET and POST HTTP Tokens.
//!
//! \param pSlHttpServerEvent Pointer indicating http server event
//! \param pSlHttpServerResponse Pointer indicating http server response
//!
//! \return None
//!
//*****************************************************************************
void sl_HttpServerCallback(SlHttpServerEvent_t *pSlHttpServerEvent,
                           SlHttpServerResponse_t *pSlHttpServerResponse)
{
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

    while(g_ulStatus != 1)
    {
        // Wait for WLAN Event
        _SlNonOsMainLoopTask();
    }

    return SUCCESS;
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
    IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
#endif
#if defined(ewarm)
    IntVTableBaseSet((unsigned long)&__vector_table);
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
//! Check the device mode and switch to STATION(STA) mode
//! restart the NWP to activate STATION mode
//!
//! \param  iMode (device mode)
//!
//! \return None
//
//*****************************************************************************
void SwitchToStaMode(int iMode)
{
    if(iMode != ROLE_STA)
    {
        sl_WlanSetMode(ROLE_STA);
        MAP_UtilsDelay(80000);
        sl_Stop(10);
        MAP_UtilsDelay(80000);
        sl_Start(NULL,NULL,NULL);
    }

}

//*****************************************************************************
//
//! Parse the input string command
//! 
//! \param  buff is the input buffer cmd to be parsed 
//! \param  cmd is the pointer to the cmd code for the input command
//!
//!\return  None
//
//*****************************************************************************
void ParseCmd(char* buff, signed char*cmd)
{
	if(strcmp(buff, "sleep") == 0){
		*cmd = APP_SLEEP;
	}else if(strcmp(buff, "send") == 0){
		*cmd = APP_SEND;
	}else if(strcmp(buff, "recv") == 0){
		*cmd = APP_RECV;
	}else{
		*cmd = APP_ERROR;
	}
}

//****************************************************************************
//                            MAIN FUNCTION
//****************************************************************************
int main(void)
{
    long lRetVal;
	char cCmdBuff[20];
	signed char cCmd = APP_SLEEP;

    SlSockAddrIn_t  sAddr;
    SlSockAddrIn_t  sLocalAddr;
    SlSockAddrIn_t  sBrdAddr;
    int             iCounter;
    int             iAddrSize;
    int             iSockID;
    int             iStatus;
    long            lLoopCount = 0;
    short           sTestBufLen;
    struct SlTimeval_t timeVal;

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
    // Note: pinmux has been modified after the output from pin mux tools
    // to enable sleep clk for the peripherals exercised
    //
    PinMuxConfig();

    //
	// Initialize the platform
	//
	platform_init();

    //
    // Initialise the UART terminal
    //
    InitTerm();

    //
	// Display banner
	//
    DisplayBanner();
    
	//
    // starting the simplelink
    //
	lRetVal = sl_Start(NULL, NULL, NULL);
	if (lRetVal < 0)
	{
		UART_PRINT("Failed to start the device \n\r");
		LOOP_FOREVER();
	}

    //
    // Swtich to STA mode if device is not
    //
    SwitchToStaMode(lRetVal);
    
    //
    // set connection policy
    //
    sl_WlanPolicySet(SL_POLICY_CONNECTION, 
                                SL_CONNECTION_POLICY(0, 0, 0, 0, 0), NULL, 0);
	//
	// Set the power management policy of NWP
	//
	lRetVal = sl_WlanPolicySet(SL_POLICY_PM, SL_NORMAL_POLICY, NULL, 0);

    UART_PRINT("Trying to Connect to AP: %s ...\r\n",SSID_NAME);

    //
    //Connecting to WLAN AP
    //
    lRetVal = WlanConnect();
    if(lRetVal < 0)
    {
        UART_PRINT("Failed to establish connection w/ an AP \n\r");
        LOOP_FOREVER();
    }

    // filling the buffer
    for (iCounter=0 ; iCounter<BUF_SIZE ; iCounter++)
    {
        g_cBsdBuf[iCounter] = (char)(iCounter % 10);
    }
    
	sTestBufLen  = BUF_SIZE;
	//filling the UDP server socket address
	sLocalAddr.sin_family = SL_AF_INET;
	sLocalAddr.sin_port = sl_Htons((unsigned short)PORT_NUM);
	sLocalAddr.sin_addr.s_addr = 0;

	//filling the UDP server socket address
	sBrdAddr.sin_family = SL_AF_INET;
	sBrdAddr.sin_port = sl_Htons((unsigned short)PORT_NUM);
	sBrdAddr.sin_addr.s_addr = sl_Htonl((unsigned int)g_ulDestinationIp);

	iAddrSize = sizeof(SlSockAddrIn_t);

	// creating a UDP socket
	iSockID = sl_Socket(SL_AF_INET,SL_SOCK_DGRAM, 0);

    /* setting time out for socket recv */
    timeVal.tv_sec =  5;             // Seconds
    timeVal.tv_usec = 0;             // Microseconds. 10000 microseconds resolution
    sl_SetSockOpt(iSockID,SL_SOL_SOCKET,SL_SO_RCVTIMEO, (_u8 *)&timeVal, sizeof(timeVal));

    // binding the UDP socket to the UDP server address
    iStatus = sl_Bind(iSockID, (SlSockAddr_t *)&sLocalAddr, iAddrSize);
    if( iStatus < 0 )
    {
        // error
        sl_Close(iSockID);
        ASSERT_ON_ERROR(iStatus);
    }

	//
	// setting Apps power policy
	//
	lp3p0_setup_power_policy(POWER_POLICY_STANDBY);
    
    UART_PRINT("enter one of the following command:\n\r");
    UART_PRINT("sleep - for putting the system into LPDS mode\n\r");
    UART_PRINT("        GPIO 13 and timer(5 sec) are the wk source configured\n\r");
    UART_PRINT("recv  - for receiving 1000 UDP packets\n\r");
    UART_PRINT("send  - for broadcasting 1000 UDP packets\n\r");
	
    do{
        
		UART_PRINT("cmd#");
		//
		// get cmd over UART
		//
		GetCmd(cCmdBuff, 20);

		//
		// parse the command
		//
		ParseCmd(cCmdBuff, &cCmd);

		if(cCmd == APP_SLEEP)
		{
            // just to clear any pending event
            _SlNonOsMainLoopTask();
            
			//
			// set timer and gpio as wake src
			//
			set_rtc_as_wk_src(WK_LPDS, LPDS_DUR_SEC, false);
			set_gpio_as_wk_src(WK_LPDS, GPIO_SRC_WKUP, PRCM_LPDS_FALL_EDGE);
			cc_idle_task_pm();
		}
		else if(cCmd == APP_RECV)
		{
			lLoopCount = 0;
		    /// waits for 1000 packets from a UDP client
		    while (lLoopCount < g_ulPacketCount)
		    {
		        iStatus = sl_RecvFrom(iSockID, g_cBsdBuf, sTestBufLen, 0,
		                     ( SlSockAddr_t *)&sAddr, (SlSocklen_t*)&iAddrSize );

				if( iStatus < 0 )
				{
					//error
					break;
				}
				lLoopCount++;
		    }
		    UART_PRINT("Recieved %u packets successfully \n\r",lLoopCount);
		    if(lLoopCount != g_ulPacketCount)
		    {
                if(iStatus == SL_EAGAIN)
                {
                    UART_PRINT("timed out\n\r");
                }
                else
                {
                    UART_PRINT("recv error: %d\n\r", iStatus);
                }
		    }
		}
		else if(cCmd == APP_SEND)
		{
			lLoopCount = 0;
		    // sending 1000 packets to the UDP server
		    while (lLoopCount < g_ulPacketCount)
		    {
		        // sending packet
		        iStatus = sl_SendTo(iSockID, g_cBsdBuf, sTestBufLen, 0,
		                                (SlSockAddr_t *)&sBrdAddr, iAddrSize);
		        if( iStatus <= 0 )
		        {
		            // error
		            UART_PRINT("send error\n\r");
                    break;
		        }
		        lLoopCount++;
		    }
		    UART_PRINT("Sent %u packets successfully\n\r",lLoopCount);
		}
	}while(FOREVER);
}
