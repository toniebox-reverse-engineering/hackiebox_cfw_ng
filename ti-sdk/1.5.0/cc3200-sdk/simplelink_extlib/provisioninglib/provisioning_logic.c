/*
 * provisioning_logic.c - Provisioning library logic management:
 *     - Decide whether provisioning is needed at all
 *     - if needed, select smart config or AP provisioning
 *     - switch from smart config to AP provisioning
 *
 * Created on: Jul 31, 2014
 * Modified one: Mar 18, 2015
 *
 * Copyright (C) 2015 Texas Instruments Incorporated - http://www.ti.com/ 
 * 
 * 
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted provided that the following conditions 
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the   
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>

#include "simplelink.h"
#include "provisioning_defs.h"
#include "provisioning_api.h"
#include "internal_api.h"

/*****************************************************************************/
/* Globals                                                                   */
/*****************************************************************************/

_u8 g_StopUDPBcast;

// A flag indicating whether to execute the library handlers
_u8 g_UseProvisioningLibHandlers = FALSE;


SlWlanMode_e  g_iCurrentSlMode; // SL mode (AP or STA) as received from last sl_Start
_u8 g_ucSlEventBitMask = 0;
_u8 g_ucRxTokensBitMask = 0;

// Newly added profile parameters
_u8 g_cNewProfileSsidLen;
_u8 g_cNewProfileSsid[MAXIMAL_SSID_LENGTH];

// Configuration set by user application on library init
slExtLibProvCfg_t g_ProvisioningCfg;

// Sync object, used for waking up the library when timeout occurs or external
// events happened
// FALSE = waiting. TRUE = event happened.

// Time to wait on sync object.
_i32 g_libraryTimeout = SL_EXT_PROV_WAIT_FOREVER;

// Indication that IP address was acquired.
// This is in addition to g_ucSlStateBitMask which is used for events notification
// and is immediately clear (doesn't indicate device state).
obtainIpState_e g_IpObtainState = OBTAIN_IP_NOT_OBTAINED;


// the event received sync object
_SlSyncObj_t g_EventRecvSyncObj;


// Return codes of the library, user application may use them for making decisions
// whether to continue activating the library or close  it
typedef enum
{
    PROVRUN_SUCCESS,  // Provisioning sequence was successfully completed.
                                // Profile was configured and confirmed.
                                // SL is in STA mode.
                                // User application may kill the thread.
    
    PROVRUN_FAILURE,  // Provisioning sequence was terminated by application
                                // although sequence has failed.
                                // SL mode is taken from configuration parameter mModeAfterFailure
    
    PROVRUN_ONGOING,  // Provisioning sequence is still ongoing
    PROVRUN_TIMEOUT,  // Timeout on getting commands from user (no HTTP tokens were received)
    PROVRUN_DEV_ERROR // Device error was discovered during provisioning sequence
} slExtLibProvRunReturnCode_e;

// Provisioning logic state
static provisioningLogicStates_e g_ucProvisioningState = PROVISIONING_LOGIC_IDLE;

// Indicates a transition to provisioning was triggered by auto switch mode
static _u8 g_ucAutoSwitchedProvisioning = 0;

// Device default URN
char gDefaultDeviceName[MAX_DEVICE_NAME_LEN];

/*****************************************************************************/
/* Internal APIs                                                             */
/*****************************************************************************/
static void startApProvisioning(int aEnableStaScan);
static void continueApProvisioning();
static slExtLibProvRunReturnCode_e handleTimeout();
static slExtLibProvRunReturnCode_e handleHttpEvents();
static slExtLibProvRunReturnCode_e handleWlanEvents();
slExtLibProvRunReturnCode_e ProvisioningRun();

//****************************************************************************
//                      Driver Events Notifications
//****************************************************************************

//*****************************************************************************
//! \brief Translate device MAC address to string
//!			This function was copied from sl_device_name in order to produce the
//!         exact default URN name SL published via mDNS.
//!
//! \param   pointer to device MAC address
//! \param   pointer to MAC string
//! \return  none
//*****************************************************************************
static void mac_addr_to_string (const unsigned char *mac_addr, char *mac_addr_string)
{
    int i ;
    const char hex_digit_to_string[16] =
    {
            '0', '1', '2', '3', '4', '5', '6', '7',
            '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
    };


    for ( i = 0 ; i < SL_MAC_ADDR_LEN ; i++ )
    {
        const char upper_nibble = (mac_addr[i] & 0xF0) >> 4 ;
        const char lower_nibble = (mac_addr[i] & 0x0F) >> 0 ;
        const int  j            = i*2                       ;
        mac_addr_string[j+0] = hex_digit_to_string[upper_nibble];
        mac_addr_string[j+1] = hex_digit_to_string[lower_nibble];
    } // for i loop. //
}

//*****************************************************************************
//! See provisioning_api.h for documentation
//*****************************************************************************
_i32 sl_extlib_ProvisioningStart(unsigned char aCurrMode, slExtLibProvCfg_t *apCfg)
{	
    // If device is in STA mode and connected to an AP, give it a chance to
    // acquire IP address

    if (IS_DEVICE_CONNECTED())
    {
    	LOG_ERROR("Device Connected to AP but continue anyway (debug mode) ...\r\n");
    	//LOG_DEBUG("provisioningStart: device is connected to an AP, no need to start provisioning \n\r");
    	//return SL_EXT_PROV_ERROR_NOT_NEEDED;
    }

    // start handling the events notified by the device
    g_UseProvisioningLibHandlers = TRUE;

	// Store configuration
    g_ProvisioningCfg = *apCfg;

    g_iCurrentSlMode = (SlWlanMode_e)aCurrMode;

    LOG_INFO("configuration parameters: auto start enabled %d \n\r", g_ProvisioningCfg.AutoStartEnabled);

    memset(gDefaultDeviceName, 0, MAX_DEVICE_NAME_LEN);

    // Auto switched indication is cleared only when library is first started (i.e. here)
    // Autonomous activation indication is never cleared and the library shall try to
    // reconnect periodically.
    g_ucAutoSwitchedProvisioning = 0;

    //Initialize the event trigger

    /* Create the sync object to signal on event reception */
    sl_SyncObjCreate(&g_EventRecvSyncObj, "EventRecvSyncObj");

    // Start provisioning regardless of profiles existence
    CLEAR_IP_AND_CONNECTION();
    startApProvisioning(TRUE);
	
	while (1)
    {
		int ret = 0;
        // blocking function
        ret = ProvisioningRun();
        LOG_DEBUG("ProvisioningRun() = %d\n\r", ret);
        if (ret == PROVRUN_SUCCESS)
        {
            if (g_ProvisioningCfg.AutoStartEnabled == 0)
            {
            	// stop handling the events notified by the device
            	g_UseProvisioningLibHandlers = FALSE;

            	sl_SyncObjDelete(&g_EventRecvSyncObj);
                return SL_EXT_PROV_SUCCESS;
            }
            else
            {
                // Provisioning was successfully completed, monitoring connection
            }
        }
        else if (ret == PROVRUN_TIMEOUT)
        {
        	// stop handling the events notified by the device
        	g_UseProvisioningLibHandlers = FALSE;

        	sl_SyncObjDelete(&g_EventRecvSyncObj);
            return SL_EXT_PROV_TIMEOUT;
        }
    }

	//sl_SyncObjDelete(&g_EventRecvSyncObj);
}

//*****************************************************************************
//! \brief Run the provisioning library
//!        Handle received events - indication for events occurrence is set in
//!        the Set API.
//!
//!        Internal events:
//!          - configuration timeout (no profile was added) - switch to next
//!            provisioning method
//!
//!        SL WLAN/net app events:
//!          - SL connected - used to feed confirmation state machine
//!          - SL disconnected - used if auto switch to provisioning is enabled
//!          - IP address acquired - updates confirmation state machine
//!          - profile added via Smart Config - switch to confirmation stage
//!          - failed to add profile with Smart Config - switch to AP provisioning
//!
//!        HTTP events:
//!          - force scan token - handled only in AP provisioning while in
//!            configuration stage, force immediate scan
//!          - profile added - switch to confirmation stage
//!          - cfg result was taken - provisioning done
//!
//! \param   SL current operational mode
//! \return  PROVRUN_ONGOING - provisioning is on going,
//!          PROVRUN_SUCCESS - provisioning was successfully completed
//!          PROVRUN_FAILURE - provisioning was terminated although not
//!                                              completed successfully
//*****************************************************************************
slExtLibProvRunReturnCode_e ProvisioningRun()
{
    int  retVal;
    slExtLibProvRunReturnCode_e rc;

    // Todo - should we check here if host is blocking?
    if(g_libraryTimeout == SL_EXT_PROV_WAIT_FOREVER)
    {
        LOG_DEBUG("provisioningRun: g_libraryTimeout FOREVER\n\r");
    }
    else
    {
        LOG_DEBUG("provisioningRun: g_libraryTimeout %u\n\r", g_libraryTimeout);
    }
    
    // Waiting for event, host application needs to maintain the loop until event triggers
    //g_libraryEvent = FALSE;
    // Wait for async event
    retVal = sl_SyncObjWait(&g_EventRecvSyncObj, g_libraryTimeout);

    //retVal = sl_extlib_ProvEventTimeoutHdl(&g_libraryEvent, g_libraryTimeout);

    LOG_DEBUG("provisioningRun: retval %d g_ucRxTokensBitMask 0x%x g_ucSlEventBitMask 0x%x g_ucProvisioningState %d g_NetListHandlingState %d\n\r",
    		   retVal, g_ucRxTokensBitMask, g_ucSlEventBitMask, g_ucProvisioningState, g_NetListHandlingState);
    
    // application code
    if (retVal < 0)
    {
        return handleTimeout();
    }

	// Event is triggered before timeout, proceed to NWP handlers
    rc = handleHttpEvents();
    if (rc == PROVRUN_ONGOING)
    {
        rc = handleWlanEvents();
    }

    return rc;
}

//*****************************************************************************
//! \brief This function starts AP provisioning, it starts scan and returns
//!        The process is continued after a timeout on waiting for scan results
//!
//! \param   aEnableStaScan - only if TRUE device is allowed to switch to STA
//!                           mode in order to scan.
//! \return  on device error -1, otherwise 0
//!          result doesn't indicate whether configuration succeeded
//*****************************************************************************
static void startApProvisioning(int aEnableStaScan)
{
	_u8  deviceNameLen = MAX_DEVICE_NAME_LEN;
	char deviceName[MAX_DEVICE_NAME_LEN];
	_u8  macAddressVal[SL_MAC_ADDR_LEN];
	_u8  macAddressLen = SL_MAC_ADDR_LEN;

    LOG_DEBUG("startApProvisioning: starting AP provisioning\n\r");

    // Create default URN for the first activation of AP provisioning
    if (PROVISIONING_LOGIC_IDLE == g_ucProvisioningState)
    {
		sl_NetCfgGet(SL_MAC_ADDRESS_GET, NULL, &macAddressLen, macAddressVal);
		mac_addr_to_string(macAddressVal, gDefaultDeviceName);

		sl_NetAppGet(SL_NET_APP_DEVICE_CONFIG_ID, NETAPP_SET_GET_DEV_CONF_OPT_DEVICE_URN, &deviceNameLen, (_u8 *)deviceName);
		if ( 0 == memcmp(deviceName, "mysimplelink", strlen("mysimplelink")) )
		{
			strcat(gDefaultDeviceName, "@mysimplelink");
		}
		else
		{
			memset(gDefaultDeviceName, 0, MAX_DEVICE_NAME_LEN);
			memcpy(gDefaultDeviceName, deviceName, strlen(deviceName));
		}
		LOG_INFO("Device name %s, default device name %s length %d\n\r", deviceName, gDefaultDeviceName, strlen(gDefaultDeviceName));
    }

    // Update provisioning logic state machine
    g_ucProvisioningState = PROVISIONING_LOGIC_AP_CONFIG;

    sl_WlanSetMode(ROLE_STA);

    // Start scanning, when scan completes the device is configured to AP mode
    networksListScan(aEnableStaScan);
}

//*****************************************************************************
//! \brief This function continues the AP provisioning after waiting for scan
//!
//! \param   none
//! \return  on device error -1, otherwise 0
//!          result doesn't indicate whether configuration succeeded
//*****************************************************************************
static void continueApProvisioning()
{
    LOG_DEBUG("continueApProvisioning\n\r");

    // Wait for configuration application to fetch scan results
    // If AP provisioning is autonomously started or Smart Config is enabled,
    // waiting time is limited.
    // Otherwise, if user application hasn't set a timeout the device waits forever.
    if (g_ucAutoSwitchedProvisioning)
    {
        g_libraryTimeout = PROVISIONING_TO_2EXIT_AUTO_PROVISIONING_MSECS;
    }
    else
    {
        g_libraryTimeout = SL_EXT_PROV_WAIT_FOREVER;

        if (0 < g_ProvisioningCfg.Timeout10Secs)
        {
            g_libraryTimeout = ((_i32)g_ProvisioningCfg.Timeout10Secs) * 1000 * 10;
            LOG_DEBUG("startApProvisioning waiting for user commands for %d msecs\n\r", g_libraryTimeout);
        }
    }
}

//*****************************************************************************
//! \brief This function sets device mode (if differs from current device mode)
//!
//! \param   desired mode
//! \return  on device error -1, otherwise 0
//*****************************************************************************
int SetDeviceMode(SlWlanMode_e aMode)
{
    int retVal;

    if (aMode == g_iCurrentSlMode)
    {
        return 0;
    }

    LOG_DEBUG("SetDeviceMode: setting device to %d\n\r", aMode);

    retVal = sl_WlanSetMode(aMode);
    if(retVal < 0) LOG_ERROR("Failure in line %d\n\r", __LINE__);

    if (ROLE_AP != g_iCurrentSlMode)
    {
        sl_WlanDisconnect();
    }
    sl_Stop(SL_STOP_TIMEOUT_MSEC);

    CLEAR_IP_AND_CONNECTION();
    g_IpObtainState = (ROLE_AP == aMode) ? OBTAIN_IP_AS_AP : OBTAIN_IP_AS_STA;

    g_iCurrentSlMode = (SlWlanMode_e)sl_Start(NULL,NULL,NULL);
    if (aMode != g_iCurrentSlMode)
    {
        LOG_ERROR("SetDeviceMode: failed to change device mode.Desired %d, current %d\n\r", aMode, g_iCurrentSlMode);
        return -1;
    }

    LOG_DEBUG("SetDeviceMode: device was set to %d\n\r", g_iCurrentSlMode);

    // Wait for AP to indicate IP is obtained
    if (ROLE_AP == g_iCurrentSlMode)
    {
        // Wait for IP address assignment
        sl_extlib_ProvWaitHdl(AP_IP_ACQUIRED_TIMEOUT_MSECS);

        if (g_IpObtainState != OBTAIN_IP_OBTAINED)
        {
            LOG_ERROR("SetDeviceMode: AP failed to get IP\n\r");
            return -1;
        }
    }

    return 0;
}

static slExtLibProvRunReturnCode_e handleTimeout()
{
    slExtLibProvRunReturnCode_e rc = PROVRUN_ONGOING;

    if (NETS_LIST_HNDL_SCANNING == g_NetListHandlingState)
    {
        networksListReadScanResults();

        // If scan was issued as part of AP provisioning, continue to provisioning
        // next steps
        if (PROVISIONING_LOGIC_AP_CONFIG == g_ucProvisioningState)
        {
            continueApProvisioning();
        }

        return PROVRUN_ONGOING;
    }

    //  If timeout occurred while waiting for configuration in AP mode:
    //  if the switch to AP was an auto switch - give another chance to connection
    //  based existing profiles
    //  Else
    //  It's up to user application to decide whether to stop provisioning or keep waiting.
    if (PROVISIONING_LOGIC_AP_CONFIG == g_ucProvisioningState)
    {
        if (g_ucAutoSwitchedProvisioning)
        {
            LOG_DEBUG("handleTimeout: AP provisioning TO, will try to connect to existing profiles\n\r");

            g_ucProvisioningState = PROVISIONING_LOGIC_WAIT_CONNECT;
            g_libraryTimeout = PROVISIONING_TO_2ENTER_AUTO_PROVISIONING_MSECS;

            SetDeviceMode(ROLE_STA);
        }
        else if (0 < g_ProvisioningCfg.Timeout10Secs)
        {
            LOG_DEBUG("handleTimeout: timeout on waiting for user commands, switching to mode %d & exiting\n\r", g_ProvisioningCfg.ModeAfterTimeout);
            SetDeviceMode((SlWlanMode_e)g_ProvisioningCfg.ModeAfterTimeout);
            g_libraryTimeout = SL_EXT_PROV_WAIT_FOREVER;
            rc = PROVRUN_TIMEOUT;
        }
        return rc;
    }

    // If timeout occurred while waiting for confirmation in AP mode
    // behaviour depends on confirmation state and configuration result.
    if (PROVISIONING_LOGIC_AP_CONFIRM == g_ucProvisioningState)
    {
        // cfgNotifyTimeout changes g_cfgConfirmState state, have to keep a snapshot
        cfgConfirmStates_e prevCfgConfirmState = g_cfgConfirmState;

        cfgNotifyTimeout();

        if (CFG_CONFIRM_STATE_GET_RESULT_AP == prevCfgConfirmState)
        {
            if (CFG_CONFIRM_RES_SUCCESS == g_ConfigResult)
            {
                // cfg has succeeded; switch to STA mode and set the timeout to
                // forever as no provisioning event (exceprt for STA disconnected)
                // is expected
                SetDeviceMode(ROLE_STA);
                g_libraryTimeout = SL_EXT_PROV_WAIT_FOREVER;
                rc = PROVRUN_SUCCESS;
            }
            else
            {
            	SetDeviceMode((SlWlanMode_e)g_ProvisioningCfg.ModeAfterFailure);
            }
        }

        return rc;
    }

    // Timeout on waiting for full connection, switch to provisioning mode
    // but try every XXX to reconnect
    if (PROVISIONING_LOGIC_WAIT_CONNECT == g_ucProvisioningState)
    {
        g_ucAutoSwitchedProvisioning = 1;
        startApProvisioning(FALSE);

        return PROVRUN_ONGOING;
    }

    return PROVRUN_ONGOING;
}

//*****************************************************************************
//! \brief This function serves as second level handler for HTTP GET/POST tokens.
//!        It runs under provisioning context.
//!
//! \return None
//!
//*****************************************************************************
static slExtLibProvRunReturnCode_e handleHttpEvents()
{
    int retVal;

    if (NO_PENDING_TOKENS())
	{
		return PROVRUN_ONGOING;
	}

    // Received force scan token.
    // If needed, device switches to STA mode for scan. In any case, it returns
    // to the mode before scan.
    // ToDo - the function below waits for SL scan. For non-OS host might be split
    // in order to avoid the waiting interval.
    if (IS_FORCE_SCAN_RECEIVED())
    {
        networksListScan(TRUE);
        CLEAR_FORCE_SCAN_RECEIVED();

        return PROVRUN_ONGOING;
    }

    // Notify a new profile was added (via AP provisioning)
    if (IS_ADD_RPOFILE_RECEIVED())
    {
        CLEAR_ADD_RPOFILE_RECEIVED();

        LOG_DEBUG("provisioningRun: handle add profile; g_NetListHandlingState %d g_ucProvisioningState %d\n\r",
                    g_NetListHandlingState, g_ucProvisioningState);

        // Protect against false activation, handle the token only if we're waiting for it
        if (NETS_LIST_HNDL_PUBLISH_NETS_LIST == g_NetListHandlingState)
            // && (PROVISIONING_LOGIC_AP_CONFIG == g_ucProvisioningState) )
        {
            LOG_INFO("provisioningRun: handle add profile\n\r");

            retVal = cfgConfirmStart((char*) g_cNewProfileSsid, g_cNewProfileSsidLen);

            // If any kind of error occured, no point to continue; just return
            // an error code to user application
            if (retVal < 0)
            {
                return PROVRUN_DEV_ERROR;
            }

            // If everything is OK, update provisioning state machine
            g_ucProvisioningState = PROVISIONING_LOGIC_AP_CONFIRM;
        }

        return PROVRUN_ONGOING;
    }

    // Notify configuration result was taken
    if (IS_GET_RESULT_RECEIVED())
    {
    	SlWlanMode_e curr_state = g_iCurrentSlMode;

        CLEAR_GET_RESUL_RECEIVED();

        // Update configuration module, this may change device state
        cfgNotifyResultTaken();

        g_libraryTimeout = SL_EXT_PROV_WAIT_FOREVER;

        // If configuration succeeded, provisioning can be ended.
        if (CFG_CONFIRM_RES_SUCCESS == g_ConfigResult)
        {
            g_ucProvisioningState = PROVISIONING_LOGIC_DONE;

            if (g_ProvisioningCfg.AutoStartEnabled)
            {
                // If result was taken from STA mode, just monitor the connection
                if (ROLE_STA == curr_state)
                {
                    g_ucProvisioningState = PROVISIONING_LOGIC_WAIT_DISCONNECT;
                }
                // else, the device has just switched to STA mode & should wait for AP connection
                else
                {
                    LOG_DEBUG("provisioningRun: auto connection waits for connection after result was taken\n\r");

                    g_ucProvisioningState = PROVISIONING_LOGIC_WAIT_CONNECT;
                    g_libraryTimeout = PROVISIONING_TO_2ENTER_AUTO_PROVISIONING_MSECS;
                }
            }

            return PROVRUN_SUCCESS;
        }
        else // any kind of error
        {
            // If error result was taken, it was taken from AP mode for sure;
            // remain in AP mode so user can configure another profile
            if (PROVISIONING_LOGIC_AP_CONFIRM == g_ucProvisioningState)
            {
                // start AP provisioning; as an application is already connected to SL AP
                // scanning in STA mode is disabled
                startApProvisioning(FALSE);
            }
            else
            {
                LOG_ERROR("provisioningRun: result taken from  unexpected state %d\n\r", g_ucProvisioningState);

                // If result was taken from unexpected state, assume it was already taken before.
                // An example is SC configuration failure followed by a new trial. If result isn't
                // cleared and SC fails, the AP mode shall not succeed since the application assumes
                // there's no need to set the new profile.
                g_ConfigResult = CFG_CONFIRM_RES_NOT_STARTED;
            }

            return PROVRUN_ONGOING;
        }
    }

    if (IS_TERMINATE_PRO_RECEIVED())
    {
        if (CFG_CONFIRM_RES_SUCCESS == g_ConfigResult)
        {
            LOG_DEBUG("provisioningRun: got terminate provisioning token, switching to STA mode\n\r");
            SetDeviceMode(ROLE_STA);

            return PROVRUN_SUCCESS;
        }
        else
        {
            LOG_DEBUG("provisioningRun: got terminate provisioning token, switching to %d mode\n\r",
                       g_ProvisioningCfg.ModeAfterFailure);
            SetDeviceMode((SlWlanMode_e)g_ProvisioningCfg.ModeAfterFailure);

            return PROVRUN_FAILURE;
        }
    }

    return PROVRUN_ONGOING;
}



static void SendUdpBCast(_u8 DurationInSec)
{
   // char * msg = "10.1.7.99,mcasttest";  // this is an option to inject the data

   char ipNameCsv[64];
   int sock;
   _u8 i;

	SlSockAddrIn_t  Addr;
	_i16 AddrSize = sizeof(SlSockAddrIn_t);
   _u8 my_device_name[35];
   _u8 optionLen = 35;

   // Read the URN
	sl_NetAppGet (SL_NET_APP_DEVICE_CONFIG_ID, NETAPP_SET_GET_DEV_CONF_OPT_DEVICE_URN, &optionLen, (_u8 *)my_device_name);

	{
	    _u8 len = sizeof(SlNetCfgIpV4Args_t);
		_u8 ConfigOpt = 0;   //return value could be one of the following: SL_NETCFG_ADDR_DHCP / SL_NETCFG_ADDR_DHCP_LLA / SL_NETCFG_ADDR_STATIC
		SlNetCfgIpV4Args_t ipV4 = {0};

		sl_NetCfgGet(SL_IPV4_STA_P2P_CL_GET_INFO,&ConfigOpt,&len,
		                                  (unsigned char *)&ipV4);


	   LOG_DEBUG(">>>>> In SendUdpBCast: IP Is %d.%d.%d.%d \r\n",
	   SL_IPV4_BYTE(ipV4.ipV4,3),SL_IPV4_BYTE(ipV4.ipV4,2),SL_IPV4_BYTE(ipV4.ipV4,1),SL_IPV4_BYTE(ipV4.ipV4,0));

	   sprintf(ipNameCsv,"%d.%d.%d.%d,%s",(_u8)SL_IPV4_BYTE(ipV4.ipV4,3),(_u8)SL_IPV4_BYTE(ipV4.ipV4,2),(_u8)SL_IPV4_BYTE(ipV4.ipV4,1),(_u8)SL_IPV4_BYTE(ipV4.ipV4,0),my_device_name);
   }


   if( (sock = sl_Socket(SL_AF_INET, SL_SOCK_DGRAM, SL_IPPROTO_UDP)) == -1)
   {
	  //perror("socket : ");
	  return;
   }

	Addr.sin_family = SL_AF_INET;
	Addr.sin_port = sl_Htons(1501);
	Addr.sin_addr.s_addr = sl_Htonl(SL_IPV4_VAL(255,255,255,255));

	g_StopUDPBcast = FALSE;

	for (i=1; i<= DurationInSec; i++)
	{
		LOG_DEBUG(">>>>> SendUdpBCast Packet #%d \r\n", i);
		sl_SendTo(sock, ipNameCsv, strlen(ipNameCsv), 0, (SlSockAddr_t *)&Addr, AddrSize);
		sl_extlib_ProvWaitHdl(1000);

		// stop the udp bcast
		if (g_StopUDPBcast == TRUE)
		break;
	}

	sl_Close(sock);
}

//*****************************************************************************
//! \brief This function serves as second level handler for WLAN events.
//!        It runs under provisioning context.
//!
//! \return None
//!
//*****************************************************************************
static slExtLibProvRunReturnCode_e handleWlanEvents()
{
    slExtLibProvRunReturnCode_e rc = PROVRUN_ONGOING;

    if (NO_PENDING_EVENTS())
    {
        return PROVRUN_ONGOING;
    }

    // connection indication is expected:
    //    1. during confirmation stage
    //    2. if provisioning is idle and auto switch to provisioning is enabled
    if (IS_DEVICE_CONNECTED())
    {
        CLEAR_DEVICE_CONNECTED();
        cfgNotifyStaConnected();
        return rc;
    }

    if (IS_DEVICE_DISCONNECTED())
    {
        g_IpObtainState = OBTAIN_IP_NOT_OBTAINED;
        CLEAR_DEVICE_DISCONNECTED();

        // If disconnection occurs while waiting for success configuration result
        // to be taken from STA, continue (as in case of timeout in taking result)
        // to publish result in AP mode
        if (PROVISIONING_LOGIC_AP_CONFIRM == g_ucProvisioningState)
        {
            LOG_DEBUG("provisioningRun: disconnected while waiting to publish result\n\r");
            cfgNotifyTimeout();
        }
        // SL STA was disconnected and auto provisioning is enabled:
        //      set the time to wait for sync object (on reconnection) and restart the library
        else if (PROVISIONING_LOGIC_WAIT_DISCONNECT == g_ucProvisioningState)
        {
            LOG_DEBUG("provisioningRun: disconnected, auto connection waits for connection\n\r");
            g_libraryTimeout = PROVISIONING_TO_2ENTER_AUTO_PROVISIONING_MSECS;
            g_ucProvisioningState = PROVISIONING_LOGIC_WAIT_CONNECT;
        }
        return rc;
    }

    if (IS_IP_ACQUIRED())
    {
        CLEAR_IP_ACQUIRED();
        cfgNotifyStaIpAcquired();

        /* send few broadcast packets to accelerate closing the app loop  */
        SendUdpBCast(25);

        if (PROVISIONING_LOGIC_WAIT_CONNECT == g_ucProvisioningState)
        {
            LOG_DEBUG("provisioningRun: connected, auto monitoring connection state\n\r");
            g_ucProvisioningState = PROVISIONING_LOGIC_WAIT_DISCONNECT;
            g_libraryTimeout = SL_EXT_PROV_WAIT_FOREVER;
        }
        return rc;
    }

    // Provisioning is still ongoing
    return rc;
}

