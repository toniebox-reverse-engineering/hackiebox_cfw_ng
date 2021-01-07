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
// Application Name     - Power measurement Non Os
// Application Overview - Power measurement appliction enables the user to measure
//						  current values, power consumption and other such parameters
//                        for CC3200, when the device is various use cases
//                        The other main objective behind this application is to
//						  introduce the user to the easily configurable power management
//						  framework. This example is for non os environment.
//
//*****************************************************************************

//****************************************************************************
//
//! \addtogroup Power managment Non Os
//! @{
//
//****************************************************************************

/* Standard includes */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

/* driverlib includes */
#include "hw_ints.h"
#include "hw_types.h"
#include "hw_memmap.h"
#include "interrupt.h"
#include "prcm.h"
#include "utils.h"
#include "rom.h"
#include "rom_map.h"

/* Simplelink includes */
#include "simplelink.h"

/* middleware includes */
#include "cc_types.h"
#include "cc_pm.h"
#include "cc_pm_ops.h"

/* Common interface includes */
#include "uart_if.h"
#include "udma_if.h"
#include "utils_if.h"
#include "pinmux.h"
#include "common.h"



/*	constant defines  */
#define APPLICATION_VERSION 			"1.4.0"
#define FOREVER                 		1
#define WK_LPDS 						0   /* Wake from LPDS */
#define WK_HIB							1   /* Wake from Hibernate */
#define HIBERNATE_MEASURE 				1
#define SLEEP_MEASURE					2
#define TRANSCEIVER_MODE 				3
#define ALWAYS_CONNECTED				4
#define INTERMITTENTLY_CONNECTED		5
#define RAW_SOCKET						0
#define UDP_SOCKET						1
#define TCP_SOCKET						2
#define SEC_TCP_SOCKET  				3
#define STATIC_IP						0
#define DHCP							1
#define DATE                			1    	/* Current Date */
#define MONTH               			5     	/* Month 1-12 */
#define YEAR                			2015  	/* Current year */
#define HOUR                			12    	/* Time - hours */
#define MINUTE              			30    	/* Time - minutes */
#define SECOND              			0     	/* Time - seconds */

/* Values for below macros shall be modified as per user/use case. */

#define IP_ADDR                 	SL_IPV4_VAL(192,168,39,200) /* Destination Address */
#define PORT_NUM                	5001
#define SSL_PORT					443
#define MY_IP_ADDRESS   			SL_IPV4_VAL(192,168,39,1)	/* the Device (simplelink) IP address */
#define GW_IP_ADDRESS  				SL_IPV4_VAL(192,168,39,241) /* Gate Way IP address */
#define NOT_ACTIVE_TME_SEC 			5
#define LSI_SLEEP_DURATION_IN_MSEC	(100)  	/* in msec */
#define BUF_SIZE                	1400
#define PACKET_COUNT        		0 /* setting the packet count to zero in always connected results in Idle profile and 1 packet in all other use cases */
#define TAG_TUNED_CHANNEL			1 /* The channel number that the device will work in when at TRANSCEIVER_MODE */
#define TAG_FRAME_TRANSMIT_RATE		6
#define TAG_FRAME_TRANSMIT_POWER	7 /* 0 - Highest TX powwer ...15 - Lowest TX power */
#define TAG_FRAME_LENGTH			100
#define INTERACTIVE					1   /* 1 interactive (simple mode), 0 batch (advenced mode) */

/*
*****************************************************************************
//                 GLOBAL VARIABLES -- Start
*****************************************************************************
*/
/* user defined */
signed short   g_ActiveUseCase 	= TRANSCEIVER_MODE; /* options -->  HIBERNATE_MEASURE; SLEEP_MEASURE; ALWAYS_CONNECTED; INTERMITTENTLY_CONNECTED; TRANSCEIVER_MODE; */
unsigned char  g_SocketType 	= UDP_SOCKET; 	 /* option --> RAW_SOCKET; UDP_SOCKET; TCP_SOCKET; SEC_TCP_SOCKET; */
unsigned char  g_IpV4Option		= STATIC_IP;     /* option --> STATIC_IP; DHCP; */
unsigned char  g_CcaBypass		= 1; 			 /* default bypass, if CCA is required use the value 0; */


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

char g_cBsdBuf[BUF_SIZE];
unsigned long g_ResetCause;
/* internal use (for intercative mode) */
char g_SsidName[25] = "";
char g_PassKey[64] 	= "";
char g_SecType;
unsigned int  g_myIpAddr = MY_IP_ADDRESS;
unsigned int  g_GwIPAddr = GW_IP_ADDRESS;

/*
*****************************************************************************
//                 GLOBAL VARIABLES -- End
*****************************************************************************
*/
enum ap_events{
    EVENT_CONNECTION = 0x1,
    EVENT_DISCONNECTION = 0x2,
    EVENT_IP_ACQUIRED = 0x4,
    WDOG_EXPIRED = 0x8,
    CONNECTION_FAILED = 0x10
};

/* Application specific status/error codes */
typedef enum{
    // Choosing -0x7D0 to avoid overlap w/ host-driver's error codes
    DEVICE_NOT_IN_STATION_MODE = -0x7D0,
    BSD_UDP_CLIENT_FAILED = DEVICE_NOT_IN_STATION_MODE - 1,
    TCP_CLIENT_FAILED = BSD_UDP_CLIENT_FAILED -1,

    STATUS_CODE_MAX = -0xBB8
}e_AppStatusCodes;
/*
****************************************************************************
//                      LOCAL FUNCTION PROTOTYPES
****************************************************************************
*/
void sl_WlanEvtHdlr(SlWlanEvent_t *pSlWlanEvent);
void sl_NetAppEvtHdlr(SlNetAppEvent_t *pNetAppEvent);
static void DisplayBanner();
static long WlanConnect();
void WlanDisconnect();
static void BoardInit();
static void getAPinfo();
static int  getIpAddr();
static void setUseCase();
static long ConfigureSimpleLinkToDefaultState();
static void PrepareData(unsigned int Size);
void SwitchToStaMode(int iMode);
void interMettentConnet(unsigned long RstCause);
void alwaysConnected();
void transceiverMode(unsigned long RstCause);
static int SetTime();
static int BsdUdpClient(unsigned short Port,unsigned short numOfPacket, short Sid);
static int BsdTcpClient(unsigned short Port,unsigned short numOfPacket, short Sid, short InitConnection);
static int BsdTcpSecClient(unsigned short Port,unsigned short numOfPacket,short Sid, short InitConnection);
extern void lp3p0_setup_power_policy(int power_policy);
extern int platform_init();
extern int set_rtc_as_wk_src(int pwr_mode, unsigned long time_interval,
                             bool is_periodic);
extern int set_gpio_as_wk_src(int pwr_mode, unsigned long wk_pin,
                              unsigned long trigger_type);
extern void set_host_irq_as_lpds_wk_src();
/*
*****************************************************************************
//
//! DisplayBanner
//!
//! \param  none
//!
//! \return none
//!
*****************************************************************************
*/
static void
DisplayBanner()
{
    UART_PRINT("\n\n\n\r");
    UART_PRINT("\t\t   *********************************************\n\r");
    UART_PRINT("\t\t     CC3200 Power Managment Non Os Application  \n\r");
    UART_PRINT("\t\t   *********************************************\n\r");
    UART_PRINT("\n\n\n\r");

}
/*
*****************************************************************************
//
//! Application defined idle task hook
//! 
//! \param  none
//! 
//! \return none
//!
*****************************************************************************
*/
void SimpleLinkSyncWaitLoopCallback()
{
	/* setting host IRQ as a wake up src */
    set_host_irq_as_lpds_wk_src(true);
    /* executing idle task */
    cc_idle_task_pm();
    /* removing host IRQ as a wake ip src */
    set_host_irq_as_lpds_wk_src(false);
}
/*
****************************************************************************
//
//! \brief This function handles WLAN events
//!
//! \param[in]  pSlWlanEvent is the event passed to the handler
//!
//! \return    None
//
****************************************************************************
*/
void sl_WlanEvtHdlr(SlWlanEvent_t *pSlWlanEvent)
{
    switch(pSlWlanEvent->Event)
    {
    case SL_WLAN_CONNECT_EVENT:
        /* uncomment for debug */
    	/* UART_PRINT("WLAN Connected !\n\r"); */
        break;
    case SL_WLAN_DISCONNECT_EVENT:
        /* UART_PRINT("WLAN Disconnected !\n\r"); */
        break;
    default:
        break;
    }
}
/*
****************************************************************************
//
//! \brief This function handles events for IP address acquisition via DHCP
//!           indication
//!
//! \param[in]    pNetAppEvent is the event passed to the handler
//!
//! \return     None
//
****************************************************************************
*/
void sl_NetAppEvtHdlr(SlNetAppEvent_t *pNetAppEvent)
{
    switch(pNetAppEvent->Event)
    {
    case SL_NETAPP_IPV4_IPACQUIRED_EVENT:

    case SL_NETAPP_IPV6_IPACQUIRED_EVENT:
        g_ulIpAddr = pNetAppEvent->EventData.ipAcquiredV4.ip;
        g_ulStatus = 1;
        /* uncomment for Debug */
        /*
        UART_PRINT("IP: %d.%d.%d.%d \n\r",(g_ulIpAddr&0xFF000000)>>24,
                              (g_ulIpAddr&0x00FF0000)>>16,
                              (g_ulIpAddr&0x0000FF00)>>8,
                              (g_ulIpAddr&0x000000FF));
        */
        break;
    default:
        break;
    }
}
/*
*****************************************************************************
//
//! This function gets triggered when HTTP Server receives Application
//! defined GET and POST HTTP Tokens.
//!
//! \param pSlHttpServerEvent Pointer indicating http server event
//! \param pSlHttpServerResponse Pointer indicating http server response
//!
//! \return None
//!
*****************************************************************************
*/
void sl_HttpServerCallback(SlHttpServerEvent_t *pSlHttpServerEvent,
                           SlHttpServerResponse_t *pSlHttpServerResponse)
{
}
/*
*****************************************************************************
//
//! This function handles socket events indication
//!
//! \param[in]      pSock - Pointer to Socket Event Info
//!
//! \return None
//!
*****************************************************************************
*/
void SimpleLinkSockEventHandler(SlSockEvent_t *pSock)
{
	UART_PRINT("Socket Event! \n\r");
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

/*
****************************************************************************
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
****************************************************************************
*/
static long WlanConnect()
{
    SlSecParams_t secParams = {0};
    long lRetVal = -1;

    secParams.Key = (INTERACTIVE) ? (signed char*)g_PassKey : (signed char*)SECURITY_KEY;
    secParams.KeyLen = (INTERACTIVE) ? strlen(g_PassKey) : strlen(SECURITY_KEY);
    secParams.Type = (INTERACTIVE) ? g_SecType : SECURITY_TYPE;

    if (INTERACTIVE) {
    	lRetVal = sl_WlanConnect((signed char*)g_SsidName, strlen(g_SsidName), 0, \
    	                                    &secParams, 0);
    } else {
    	lRetVal = sl_WlanConnect((signed char*)SSID_NAME, strlen(SSID_NAME), 0, \
                                    &secParams, 0);
    }
    ASSERT_ON_ERROR(lRetVal);

    while(g_ulStatus != 1)
    {
        /* Wait for WLAN Event */
        _SlNonOsMainLoopTask();
    }
    return SUCCESS;
}
/*
*****************************************************************************
//
//! Board Initialization & Configuration
//!
//! \param  None
//!
//! \return None
//
*****************************************************************************
*/
static void
BoardInit(void)
{
/* In case of TI-RTOS vector table is initialize by OS itself */
#ifndef USE_TIRTOS
  /* Set vector table base */
#if defined(ccs) || defined(gcc)
    IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
#endif
#if defined(ewarm)
    IntVTableBaseSet((unsigned long)&__vector_table);
#endif
#endif
  /* Enable Processor */
  MAP_IntMasterEnable();
  MAP_IntEnable(FAULT_SYSTICK);

  PRCMCC3200MCUInit();
}
/*
*****************************************************************************
//
//! Check the device mode and switch to STATION(STA) mode
//! restart the NWP to activate STATION mode
//!
//! \param  iMode (device mode)
//!
//! \return None
//
*****************************************************************************
 */
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

/*
****************************************************************************
//                            MAIN FUNCTION
****************************************************************************
*/
int main(void)
{
    long lRetVal;

	_u8  configOpt = SL_DEVICE_GENERAL_VERSION;
	_u8  configLen;
	SlVersionFull   ver = {0};

	configLen = sizeof(ver);

	g_ResetCause = MAP_PRCMSysResetCauseGet();
    /* Board Initialization */
    BoardInit();
    /* uDMA Initialization */
	UDMAInit();
    
    /*
    // Configure the pinmux settings for the peripherals exercised
    // Note: pinmux has been modified after the output from pin mux tools
    // to enable sleep clk for the peripherals exercised
    */
    PinMuxConfig();
    /* Initialize the platform */
	platform_init();
    /* Initialise the UART terminal */
    InitTerm();
    g_ulStatus = 0;

    if (g_ResetCause == PRCM_POWER_ON) {
    	/* Display Application banner */
    	DisplayBanner();
    	if (INTERACTIVE) {
    		/* display manu */
    		setUseCase();
    		/* In TRANSCEIVER_MODE there is no need to set the below attributes */
    		if (!(g_ActiveUseCase == TRANSCEIVER_MODE || g_ActiveUseCase == SLEEP_MEASURE || g_ActiveUseCase== HIBERNATE_MEASURE)) {
    			getAPinfo();
    			getIpAddr();
    		};

    	}
    	if (g_ActiveUseCase == TRANSCEIVER_MODE) {
    		/* flag that differ TRANSCEIVER_MODE from ALWAYS_CONNECTED */
    		HWREGB(0x4402FC20) = TRANSCEIVER_MODE;
    	}
    	/* default settings according to use case*/
    	lRetVal = ConfigureSimpleLinkToDefaultState();
    	if (lRetVal < 0)
    	{
    		UART_PRINT("Failed to start the device in default state ! \n\r");
    		LOOP_FOREVER();
    	}
    }
    if (g_ResetCause == PRCM_HIB_EXIT) {
    	UART_PRINT(" Woken from Hibernate !\n\r");
    	/* get what was the usecase before entering HIBERNATE */
    	if(HWREGB(0x4402FC20) == TRANSCEIVER_MODE) { //
    		g_ActiveUseCase = TRANSCEIVER_MODE;
    	} else {
    		g_ActiveUseCase = INTERMITTENTLY_CONNECTED;
    	}
    }

    switch (g_ActiveUseCase) {
    case HIBERNATE_MEASURE :
    	UART_PRINT(" Hibernate measure use case start measure! \n\r");
    	lp3p0_setup_power_policy(POWER_POLICY_HIBERNATE);
    	MAP_UtilsDelay(80000);
    	set_rtc_as_wk_src(WK_HIB, 999999, false);
    	cc_idle_task_pm();
    	break;
	case SLEEP_MEASURE :
		UART_PRINT(" Sleep measure use case start measure Now! \n\r");
		lRetVal = sl_Start(0, 0, 0);
		ASSERT_ON_ERROR(lRetVal);
		lRetVal = sl_DevGet(SL_DEVICE_GENERAL_CONFIGURATION, &configOpt, &configLen, (_u8 *)(&ver));
		lp3p0_setup_power_policy(POWER_POLICY_STANDBY);
		set_rtc_as_wk_src(WK_LPDS, 999999, false);
		cc_idle_task_pm();
		break;
	case  INTERMITTENTLY_CONNECTED :
		interMettentConnet(g_ResetCause);
		break;
	case ALWAYS_CONNECTED :
		alwaysConnected();
		break;
	case TRANSCEIVER_MODE :
		transceiverMode(g_ResetCause);
		break;
	}

}
/*
*****************************************************************************
//
//! get and set the AP parameter (SSID_NAME,security mode and passkey) interactivly
//!
//! \param  None
//!
//!\return  None
//
*****************************************************************************
*/
static void getAPinfo() {
	char buf[64];

	UART_PRINT(" Please Insert SSID name (24 char max length): ");
	GetCmd(buf,64);
	UART_PRINT("\n\r");
	memcpy(g_SsidName,buf,strlen(buf) + 1);
	UART_PRINT(" Please select security mode (1 for open,2 for WEP and 3 for WPA): ");
	GetCmd(buf,64);
	UART_PRINT("\n\r");
	switch (atoi(buf)) {
	case 1 :
		g_SecType = SL_SEC_TYPE_OPEN;
		break;
	case 2 :
		g_SecType = SL_SEC_TYPE_WEP;
		break;
	case 3 :
		g_SecType = SL_SEC_TYPE_WPA;
		break;
	default :
		UART_PRINT("Illegal selection, setting security mode to OPEN(default) .\n\r");
		g_SecType = SL_SEC_TYPE_OPEN;
		break;
	}
	if (g_SecType != SL_SEC_TYPE_OPEN) {
		UART_PRINT(" Please enter passkey: ");
		GetCmd(buf,64);
		UART_PRINT("\n\r");
		memcpy(g_PassKey,buf,strlen(buf) + 1);
	}

}

/*
*****************************************************************************
//
//! Set the the use case interactivly.
//!
//! \param  None
//!
//!\return  None
//
*****************************************************************************
*/
static void setUseCase() {
	char buf[2];
	UART_PRINT(" Please select use case ():\n\r");
	UART_PRINT("  1 for HIBERNATE_MEASURE.\n\r");
	UART_PRINT("  2 for SLEEP_MEASURE. \n\r");
	UART_PRINT("  3 for INTERMITTENTLY_CONNECTED. \n\r");
	UART_PRINT("  4 for ALWAYS_CONNECTED. \n\r");
	UART_PRINT("  5 for TRANSCEIVER_MODE. \n\r");
	UART_PRINT(" Enter your selection: ");
	GetCmd(buf,2);
	UART_PRINT("\n\r");

	UART_PRINT(" ********************************************* \n\r");
	switch (atoi(buf)) {
	case 1 :
		g_ActiveUseCase = HIBERNATE_MEASURE;
		break;
	case 2 :
		g_ActiveUseCase = SLEEP_MEASURE;
		break;
	case 3 :
		g_ActiveUseCase = INTERMITTENTLY_CONNECTED;
		break;
	case 4 :
		g_ActiveUseCase = ALWAYS_CONNECTED;
		break;
	case 5 :
		g_ActiveUseCase = TRANSCEIVER_MODE;
		break;
	default :
		UART_PRINT(" Invalid selection, setting HIBERNATE_MEASURE (default) \n\r");
		g_ActiveUseCase = HIBERNATE_MEASURE;
		break;
	}
}
/*
*****************************************************************************
//
//! Get the IP address obtaining method and also the address value if the method is static IP.
//!
//! \param  None
//!
//!\return  None
//
*****************************************************************************
*/
static int getIpAddr() {
	char buf[16];
	char *s1, *s2, *s3, *s4;

	UART_PRINT(" Please Select IP address obtaining method(1 for Static IP any other number for DHCP): ");
	GetCmd(buf,16);
	UART_PRINT("\n\r");
	g_IpV4Option = (atoi(buf) != 1) ? DHCP : STATIC_IP;
	if (g_IpV4Option == STATIC_IP) {
		/* get the IP address */
		UART_PRINT(" Please enter the device static IP address(use commas or dots, NO SPACES!): ");
		GetCmd(buf,16);
		UART_PRINT("\n\r");
		/* parse the input address string */
		s1 = strtok(buf,".,"); /* first pass */
		s2 = strtok(NULL,".,");
		s3 = strtok(NULL,".,");
		s4 = strtok(NULL,".,");
		g_myIpAddr = SL_IPV4_VAL((const char *)atoi(s1), (const char *)atoi(s2),
								 (const char *)atoi(s3), (const char *)atoi(s4));
		/* get the gateway IP address */
		UART_PRINT(" Please enter the Gateway IP address(make sure it is in the same domain as the device): ");
		GetCmd(buf,16);
		UART_PRINT("\n\r");
		s1 = strtok(buf,".,"); /* first pass */
		s2 = strtok(NULL,".,");
		s3 = strtok(NULL,".,");
		s4 = strtok(NULL,".,");
		g_GwIPAddr = SL_IPV4_VAL((const char *)atoi(s1), (const char *)atoi(s2),
				                 (const char *)atoi(s3), (const char *)atoi(s4));
	}

	return SUCCESS;
}
/*
*****************************************************************************
//
//! Configure the device to Default state according to the given use case
//!
//! \param  None
//!
//!\return  Negtive int value on failure other value on succusses
//
*****************************************************************************
*/
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

    SlNetCfgIpV4Args_t ipV4;

    lMode = sl_Start(0, 0, 0);
    ASSERT_ON_ERROR(lMode);

    /* configure device to station mode */
    SwitchToStaMode(lMode);

    /* Get the device's version-information */
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
    /*
    * Set connection policy to Auto + SmartConfig
    *      (Device's default connection policy)
    */
    lRetVal = sl_WlanPolicySet(SL_POLICY_CONNECTION,
                                SL_CONNECTION_POLICY(1, 0, 0, 0, 1), NULL, 0);
    ASSERT_ON_ERROR(lRetVal);

    /* Remove all profiles */
    lRetVal = sl_WlanProfileDel(0xFF);
    ASSERT_ON_ERROR(lRetVal);
    /*
    // Device in station-mode. Disconnect previous connection if any
    // The function returns 0 if 'Disconnected done', negative number if already
    // disconnected Wait for 'disconnection' event if 0 is returned, Ignore
    // other return-codes
    */
    lRetVal = sl_WlanDisconnect();
    if(0 == lRetVal)
    {
        /* Wait for connection */
        while(IS_CONNECTED(g_ulStatus))
        {

              _SlNonOsMainLoopTask();
        }
    }
    /* static IP or DHCP configuration */
    if (g_IpV4Option == STATIC_IP) {
    	ipV4.ipV4 = g_myIpAddr;
       	ipV4.ipV4Mask = (_u32)SL_IPV4_VAL(255,255,255,0);
       	ipV4.ipV4Gateway = g_GwIPAddr;
       	ipV4.ipV4DnsServer = g_GwIPAddr;
       	sl_NetCfgSet(SL_IPV4_STA_P2P_CL_STATIC_ENABLE,IPCONFIG_MODE_ENABLE_IPV4,sizeof(SlNetCfgIpV4Args_t),(_u8 *)&ipV4);
       	UART_PRINT("Configured to Static IP Address \n\r");
    }
    if (g_IpV4Option == DHCP) {
    	/* Enable DHCP client */
    	UART_PRINT("IP Address will be obtained through DHCP \n\r");
    	lRetVal = sl_NetCfgSet(SL_IPV4_STA_P2P_CL_DHCP_ENABLE,1,1,&ucVal);
    	ASSERT_ON_ERROR(lRetVal);
    }

    /* Disable scan */
    ucConfigOpt = SL_SCAN_POLICY(0);
    lRetVal = sl_WlanPolicySet(SL_POLICY_SCAN , ucConfigOpt, NULL, 0);
    ASSERT_ON_ERROR(lRetVal);
    /*
    // Set Tx power level for station mode
    // Number between 0-15, as dB offset from max power - 0 will set max power
    */
    ucPower = 0;
    lRetVal = sl_WlanSet(SL_WLAN_CFG_GENERAL_PARAM_ID,
            WLAN_GENERAL_PARAM_OPT_STA_TX_POWER, 1, (unsigned char *)&ucPower);
    ASSERT_ON_ERROR(lRetVal);

    if (g_ActiveUseCase == TRANSCEIVER_MODE) {
    	lRetVal = sl_WlanPolicySet(SL_POLICY_PM ,SL_LOW_POWER_POLICY , NULL, 0);
    	ASSERT_ON_ERROR(lRetVal);
    } else {
    	/* Set PM policy to normal */
    	lRetVal = sl_WlanPolicySet(SL_POLICY_PM , SL_NORMAL_POLICY, NULL, 0);
    	ASSERT_ON_ERROR(lRetVal);
    }
    /* set the sleep interval */
    if (g_ActiveUseCase == ALWAYS_CONNECTED && LSI_SLEEP_DURATION_IN_MSEC > 100) {
    	_u16 PolicyBuff[4] = {0,0,LSI_SLEEP_DURATION_IN_MSEC,0}; /* PolicyBuff[2] is max sleep time in mSec */
    	sl_WlanPolicySet(SL_POLICY_PM , SL_LONG_SLEEP_INTERVAL_POLICY, (_u8*)PolicyBuff,sizeof(PolicyBuff));
    }

    /* Unregister mDNS services */
    lRetVal = sl_NetAppMDNSUnRegisterService(0, 0);
    ASSERT_ON_ERROR(lRetVal);
    /* disable mdns */
    sl_NetAppStop(SL_NET_APP_MDNS_ID);
    /* Remove  all 64 filters (8*8) */
    memset(RxFilterIdMask.FilterIdMask, 0xFF, 8);
    lRetVal = sl_WlanRxFilterSet(SL_REMOVE_RX_FILTER, (_u8 *)&RxFilterIdMask,
                       sizeof(_WlanRxFilterOperationCommandBuff_t));
    ASSERT_ON_ERROR(lRetVal);

    if (g_ActiveUseCase == INTERMITTENTLY_CONNECTED) {
    	/* Set connection policy to Auto + fast connect in  */
    	lRetVal = sl_WlanPolicySet(SL_POLICY_CONNECTION, SL_CONNECTION_POLICY(1, 1, 0, 0, 0), NULL, 0);
    	ASSERT_ON_ERROR(lRetVal);
    }
    if (g_ActiveUseCase == SLEEP_MEASURE || g_ActiveUseCase == TRANSCEIVER_MODE) {
      	/* Set connection policy to None  */
      	lRetVal = sl_WlanPolicySet(SL_POLICY_CONNECTION, SL_CONNECTION_POLICY(0, 0, 0, 0, 0), NULL, 0);
        	ASSERT_ON_ERROR(lRetVal);
    }
    if (g_SocketType == SEC_TCP_SOCKET) {
    	ASSERT_ON_ERROR(SetTime());
    };

    lRetVal = sl_Stop(10);
    ASSERT_ON_ERROR(lRetVal);

    g_ulStatus = 0;

    return lRetVal; // Success
}
/*
*****************************************************************************
//
//! This function implements the intermittently connected use case.
//!
//! \param  RstCause - if the reset cause was wake on power on pass PRCM_POWER_ON
//!					   if the reset cause was wake from hibernate pass PRCM_HIB_EXIT
//!
//!\return  None
//
*****************************************************************************
*/
void interMettentConnet(unsigned long RstCause) {
	long lRetVal = -1;

	/* packet creation  */
	PrepareData(BUF_SIZE);

	lRetVal = sl_Start(0,0,0);
	if (lRetVal < 0 || ROLE_STA != lRetVal)
	{
		UART_PRINT("Failed to start the device \n\r");
		LOOP_FOREVER();
	}
	/* connect to the AP for the first time */
	if (RstCause == PRCM_POWER_ON) {
		lRetVal = WlanConnect();
		if (lRetVal < 0) {
			UART_PRINT("Failed to Connect the device to AP! \n\r");
			LOOP_FOREVER();
		}
		UART_PRINT(" Intermittently use case started! \n\r");
	}
	if (RstCause == PRCM_HIB_EXIT) {
		/* wait for connection */
		while(g_ulStatus != 1)   {
			_SlNonOsMainLoopTask();
		}
	}
	/* open socket + tx */
	switch (g_SocketType) {
	case UDP_SOCKET :
		BsdUdpClient(PORT_NUM,1,-1);
		break;
	case TCP_SOCKET :
		BsdTcpClient(PORT_NUM,1,-1,1);
		break;
	case SEC_TCP_SOCKET :
		BsdTcpSecClient(SSL_PORT,1,-1,1);
		break;
	}
	sl_Stop(10);
	/* Set wake up source and hibernate */
	lp3p0_setup_power_policy(POWER_POLICY_HIBERNATE);
	set_rtc_as_wk_src(WK_HIB, NOT_ACTIVE_TME_SEC, false);
	cc_idle_task_pm();

}
/*
*****************************************************************************
//
//! This function implements the always connected use case.
//!
//! \param  None
//!
//!\return  None
//
*****************************************************************************
*/
void alwaysConnected() {
	long 	lRetVal = -1;
	short   SockID = 0;
	short   DoInit = 1;

	lRetVal = sl_Start(0, 0, 0);
	if (lRetVal < 0) {
		UART_PRINT("Failed to start the device! \n\r");
		LOOP_FOREVER();
	}
	lRetVal = WlanConnect();
	if (lRetVal < 0) {
		UART_PRINT("Failed to connect to AP \n\r");
		LOOP_FOREVER();
	}
	UART_PRINT(" Entering Alwyas Connected use case! \n\r");
	MAP_UtilsDelay(80000);
	/* prepare packet & open socket */
	PrepareData(BUF_SIZE);
	switch (g_SocketType) {
	case  UDP_SOCKET :
		SockID = sl_Socket(SL_AF_INET,SL_SOCK_DGRAM, 0);
		break;
	case TCP_SOCKET :
		SockID = sl_Socket(SL_AF_INET,SL_SOCK_STREAM, 0);
		break;
	case SEC_TCP_SOCKET :
		SockID = sl_Socket(SL_AF_INET,SL_SOCK_STREAM, SL_SEC_SOCKET);
		break;
	}
	if (SockID < 0) {
		UART_PRINT("Failed to open socket \n\r");
		LOOP_FOREVER();
	}
	lp3p0_setup_power_policy(POWER_POLICY_STANDBY);
	while (FOREVER) {
		switch (g_SocketType) {
		case UDP_SOCKET :
			BsdUdpClient(PORT_NUM,PACKET_COUNT,SockID);
			break;
		case TCP_SOCKET :
			BsdTcpClient(PORT_NUM,PACKET_COUNT,SockID,DoInit);
			break;
		case SEC_TCP_SOCKET :
			BsdTcpSecClient(SSL_PORT,PACKET_COUNT,SockID,DoInit);
			break;
		}
		DoInit = 0;
		_SlNonOsMainLoopTask();
		/* wake up source settings & sleep */
		set_rtc_as_wk_src(WK_LPDS, NOT_ACTIVE_TME_SEC, false);
		set_host_irq_as_lpds_wk_src(true);
		cc_idle_task_pm();
		if (PRCM_LPDS_HOST_IRQ == MAP_PRCMLPDSWakeupCauseGet()) {
			/* clear the host IRQ & re-enter LPDS state */
			_SlNonOsMainLoopTask();
			set_rtc_as_wk_src(WK_LPDS, NOT_ACTIVE_TME_SEC, false);
			cc_idle_task_pm();
		};
	}

}
/*
*****************************************************************************
//
//! This function implements the transceiver mode use case.
//!
//! \param  RstCause - if the reset cause was wake on power on pass PRCM_POWER_ON
//!					   if the reset cause was wake from hibernate pass PRCM_HIB_EXIT
//!
//!\return  None
//
*****************************************************************************
*/
void transceiverMode(unsigned long RstCause) {
	long 	lRetVal = -1;
	short   SockID = 0;
	short 	idx;
	/* Printing the the use case name on first cycle */
	if (RstCause == PRCM_POWER_ON) {
		UART_PRINT(" Transceiver Mode use case started! \n\r");
	}
	/* prepare packet & start wlan & open socket */
	PrepareData(TAG_FRAME_LENGTH);
	lRetVal = sl_Start(0, 0, 0);
	if (lRetVal < 0) {
		UART_PRINT("Failed to start the device! \n\r");
		LOOP_FOREVER();
	}
	if (g_CcaBypass) {
		SockID = sl_Socket(SL_AF_RF,SL_SOCK_RAW,0);
	} else {
		SockID = sl_Socket(SL_AF_RF,SL_SOCK_DGRAM,0);
	}
	if (SockID < 0) {
		UART_PRINT("Failed to open RAW socket for trasceiver mode! \n\r");
		LOOP_FOREVER();
	}
	/* send packets */
	for (idx = 0;idx <= PACKET_COUNT;idx++) {
		lRetVal = sl_Send(SockID,g_cBsdBuf,TAG_FRAME_LENGTH, SL_RAW_RF_TX_PARAMS(TAG_TUNED_CHANNEL,TAG_FRAME_TRANSMIT_RATE,TAG_FRAME_TRANSMIT_POWER,1));
		if (lRetVal < 0) {
			UART_PRINT("Failed to send data in transceiver mode! \n\r");
			LOOP_FOREVER();
		}
		MAP_UtilsDelay(80000);
	}
	/* close socket & stop device */
	sl_Close(SockID);
	sl_Stop(10);
	/* Set wake up source and hibernate */
	lp3p0_setup_power_policy(POWER_POLICY_HIBERNATE);
	set_rtc_as_wk_src(WK_HIB, NOT_ACTIVE_TME_SEC, false);
	cc_idle_task_pm();
}
/*
*****************************************************************************
//
//! This function prerare the data that will be transmitted.
//!
//! \param  None
//!
//!\return  None
//
*****************************************************************************
*/
static void PrepareData(unsigned int Size) {
	int             iCounter;
	/* filling the buffer (packet data) */
	for (iCounter=0 ; iCounter<Size ; iCounter++)
	{
		g_cBsdBuf[iCounter] = (char)(iCounter % 10);
	}
}
/*
****************************************************************************
//
//! \brief Opening a UDPP client side socket and sending data
//!
//! This function opens a UDP socket if needed and tries to connect to a Server IP_ADDR
//!    than trasnsmit #numOfPacket packets.
//!
//! \param[in]      port number on which the server will be listening on
//! \param[in]      number of packet that will be transmitted
//! \param[in]      socket id, if one is already open. if this value is 0 open new socket
//!
//! \return    0 on success, -1 on Error.
//
****************************************************************************
*/
static int BsdUdpClient(unsigned short Port,unsigned short numOfPacket,short Sid) {
	SlSockAddrIn_t  	Addr;
	short            	SockID = 0;
	short 				Status = 0;
	unsigned short      LoopCount = 0;

	Addr.sin_family = SL_AF_INET;
	Addr.sin_port = sl_Htons((unsigned short)Port);
	Addr.sin_addr.s_addr = sl_Htonl((unsigned int)IP_ADDR);
	/* open socket if not exsit */
	if (Sid < 0) {
		SockID = sl_Socket(SL_AF_INET,SL_SOCK_DGRAM, 0);
		if( SockID < 0 )
		{
			ASSERT_ON_ERROR(SockID);
		}
	} else { /* socket already open */
		SockID = Sid;
	}
	while (LoopCount < numOfPacket) {
		Status = sl_SendTo(SockID,g_cBsdBuf, BUF_SIZE, 0,
		    	                               (SlSockAddr_t *)&Addr, sizeof(SlSockAddrIn_t));
		if( Status <= 0 )
		{
			Status = sl_Close(SockID);
			ASSERT_ON_ERROR(BSD_UDP_CLIENT_FAILED);
		}
		LoopCount++;
	}
	if (Sid < 0) {
		Status = sl_Close(SockID);
		ASSERT_ON_ERROR(Status);
	}
	return SUCCESS;
}
/*
****************************************************************************
//
//! \brief Opening a TCP client side socket and sending data
//!
//! This function opens a TCP socket if needed and tries to connect to a Server IP_ADDR
//!    than trasnsmit #numOfPacket packets.
//!
//! \param[in]      port number on which the server will be listening on
//! \param[in]      number of packet that will be transmitted
//! \param[in]      socket id, if one is already open. if this value is 0 open new socket
//! \param[in]      Init the socket (connection) or not
//!
//! \return    0 on success, -1 on Error.
//
****************************************************************************
*/
static int BsdTcpClient(unsigned short Port,unsigned short numOfPacket,short Sid,short InitConnection) {
	SlSockAddrIn_t  	Addr;
	short            	SockID = 0;
	short 				Status = 0;
	unsigned short      LoopCount = 0;

	Addr.sin_family = SL_AF_INET;
	Addr.sin_port = sl_Htons((unsigned short)Port);
	Addr.sin_addr.s_addr = sl_Htonl((unsigned int)IP_ADDR);
	if (Sid < 0) { /* need to open socket */
		/* creating a TCP socket */
		SockID = sl_Socket(SL_AF_INET,SL_SOCK_STREAM, 0);
		if(SockID < 0)
		{
			ASSERT_ON_ERROR(TCP_CLIENT_FAILED);
		}
	} else { /* socket already open */
		SockID = Sid;
	}
	if(InitConnection) {
		/* connecting to TCP server */
		Status = sl_Connect(SockID, ( SlSockAddr_t *)&Addr,sizeof(SlSockAddrIn_t));
    	if( Status < 0 ) {
        	/* error */
        	ASSERT_ON_ERROR(sl_Close(SockID));
        	ASSERT_ON_ERROR(TCP_CLIENT_FAILED);
    	}
	}
    while (LoopCount < numOfPacket) {
        /* sending packet */
        Status = sl_Send(SockID, g_cBsdBuf, BUF_SIZE, 0 );
        if( Status <= 0 )
        {
            /* error */
            ASSERT_ON_ERROR(sl_Close(SockID));
            ASSERT_ON_ERROR(TCP_CLIENT_FAILED);
        }
        LoopCount++;
    }
    if (Sid < 0) {
    	/* closing the socket after sending packets */
    	ASSERT_ON_ERROR(sl_Close(SockID));
    }
    return SUCCESS;
}
/*
****************************************************************************
//
//! \brief Opening a TCP secure client side socket and sending data
//!
//! This function opens a TCP secure socket if needed and tries to connect to a Server IP_ADDR
//!    than trasnsmit #numOfPacket packets.
//!
//! \param[in]      port number on which the server will be listening on
//! \param[in]      number of packet that will be transmitted
//! \param[in]      socket id, if one is already open. if this value is 0 open new socket
//! \param[in]      Init the socket (connection) or not
//!
//! \return    0 on success, -1 on Error.
//
****************************************************************************
*/
static int BsdTcpSecClient(unsigned short Port,unsigned short numOfPacket,short Sid,short InitConnection) {
	SlSockAddrIn_t  	Addr;
	short            	SockID = 0;
	short 				Status = 0;
	unsigned short      LoopCount = 0;
    unsigned int  		cipher = SL_SEC_MASK_SSL_RSA_WITH_RC4_128_SHA;
    unsigned char   	method = SL_SO_SEC_METHOD_SSLV3;

    Addr.sin_family = SL_AF_INET;
    Addr.sin_port = sl_Htons((unsigned short)Port);
    Addr.sin_addr.s_addr = sl_Htonl((unsigned int)IP_ADDR);
	if (Sid < 0) { // need to open socket
		SockID = sl_Socket(SL_AF_INET,SL_SOCK_STREAM, SL_SEC_SOCKET);
		if(SockID < 0) {
			ASSERT_ON_ERROR(TCP_CLIENT_FAILED);
		}
	} else { /* socket already open */
		SockID = Sid;
	}
	if (InitConnection) {
		/* set secure connection settings */
		Status = sl_SetSockOpt(SockID, SL_SOL_SOCKET, SL_SO_SECMETHOD,
	                               &method, sizeof(method));
		ASSERT_ON_ERROR(Status);
		Status = sl_SetSockOpt(SockID, SL_SOL_SOCKET, SL_SO_SECURE_MASK,
	                               &cipher, sizeof(cipher));
		ASSERT_ON_ERROR(Status);
		/* uncomment the lines below if you have flashed the device with relevant CA,
		 * and define SL_SSL_CA_CERT to its path
		 */
		/*
		retVal = sl_SetSockOpt(g_SockID, SL_SOL_SOCKET, SL_SO_SECURE_FILES_CA_FILE_NAME,
	           SL_SSL_CA_CERT, pal_Strlen(SL_SSL_CA_CERT));
		if( retVal < 0 ) {
	    	CLI_Write(" Failed to configure the socket \n\r");
	    	LOOP_FOREVER();
	 	 }
		 */
		/* connect to the peer server */
		Status = sl_Connect(SockID, ( SlSockAddr_t *)&Addr, sizeof(SlSockAddrIn_t));
		if ((Status < 0) && (Status != -453)) { // ignore authentication error, if the above lines are commented (SL_SSL_CA_CERT)
			UART_PRINT(" Failed to connect w/ server \n\r");
			LOOP_FOREVER();
		}
		UART_PRINT(" Connection with secure server established successfully \n\r");
	}
	while (LoopCount < numOfPacket) {
		/* sending packet */
		Status = sl_Send(SockID, g_cBsdBuf, BUF_SIZE, 0 );
		if( Status <= 0 )
		{
			/* error */
			ASSERT_ON_ERROR(sl_Close(SockID));
			ASSERT_ON_ERROR(TCP_CLIENT_FAILED);
	    }
		LoopCount++;
	}
	if (Sid < 0) {
		/* closing the socket after sending packets */
		ASSERT_ON_ERROR(sl_Close(SockID));
	}

	return SUCCESS;
}
/*
****************************************************************************
//
//! \brief Setting the device time needed for secure connection estblishment
//!
//! \param[in]      None
//!
//! \return    0 on success, -1 on Error.
//
****************************************************************************
*/
static int SetTime() {
	_i32 retVal 			= -1;
	SlDateTime_t dateTime	= {0};

	dateTime.sl_tm_day  = DATE;
	dateTime.sl_tm_mon  = MONTH;
	dateTime.sl_tm_year = YEAR;
	dateTime.sl_tm_hour = HOUR;
	dateTime.sl_tm_min  = MINUTE;
	dateTime.sl_tm_sec  = SECOND;

	retVal = sl_DevSet(SL_DEVICE_GENERAL_CONFIGURATION,SL_DEVICE_GENERAL_CONFIGURATION_DATE_TIME,
	            sizeof(SlDateTime_t),(_u8 *)(&dateTime));
	ASSERT_ON_ERROR(retVal);

	return SUCCESS;
}



