/*
 * events_handler.c - Provisioning library Simplelink events handlers:
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

#include <stdio.h>
#include "simplelink.h"
#include "provisioning_defs.h"
#include "internal_api.h"


extern _u8 g_UseProvisioningLibHandlers;

extern _SlSyncObj_t g_EventRecvSyncObj;
extern int Report(const char *format, ...);

//*****************************************************************************
//
//! \brief This function serves as first level handler for HTTP GET/POST tokens
//!        It runs under driver context and performs only operation that can run
//!        from this context. For operations that can't is sets an indication of
//!        received token and preempts the provisioning context.
//!
//! \param pSlHttpServerEvent Pointer indicating http server event
//! \param pSlHttpServerResponse Pointer indicating http server response
//!
//! \return None
//!
//*****************************************************************************
_SlEventPropogationStatus_e sl_Provisioning_HttpServerEventHdl(
                            SlHttpServerEvent_t    *apSlHttpServerEvent,
                            SlHttpServerResponse_t *apSlHttpServerResponse)
{

	if (g_UseProvisioningLibHandlers)
	{
		switch (apSlHttpServerEvent->Event)
		{
			// handle GET token
			case SL_NETAPP_HTTPGETTOKENVALUE_EVENT:
			{
				unsigned char *ptr = apSlHttpServerEvent->EventData.httpTokenName.data;
				LOG_INFO("ProvHttpServerEventHdl: Got token %s\n\r", ptr);
				apSlHttpServerResponse->ResponseData.token_value.len = 0;

				// Get scan results token
				if (0 == memcmp(ptr, GET_NETWORKS_TOKEN_PREFIX, STRING_TOKEN_SIZE-1))
				{
					networksListHandleGetNetworkToken(ptr, apSlHttpServerResponse);
				}

				// Get configuration result token
				else if (0 == memcmp(ptr, GET_CFG_RESULT_TOKEN, STRING_TOKEN_SIZE))
				{
					cfgConfirmGetResult(apSlHttpServerResponse);

					// Enable the run function in order to return completion indication
					sl_SyncObjSignal(&g_EventRecvSyncObj);
				}

				// Get product version token
				else if (0 == memcmp(ptr, GET_PRODUCT_VERSION_TOKEN, STRING_TOKEN_SIZE))
				{
					strcpy((char *)apSlHttpServerResponse->ResponseData.token_value.data, SL_EXT_PROVLIB_PRODUCT_VERSION);
					apSlHttpServerResponse->ResponseData.token_value.len = strlen(SL_EXT_PROVLIB_PRODUCT_VERSION);
					LOG_INFO("ProvHttpServerEventHdl: Product version: %s\n\r", SL_EXT_PROVLIB_PRODUCT_VERSION);
				}

				// Get product version token
				else if (0 == memcmp(ptr, GET_DEVICE_NAME_TOKEN, STRING_TOKEN_SIZE))
				{
					strcpy((char *)apSlHttpServerResponse->ResponseData.token_value.data, gDefaultDeviceName);
					apSlHttpServerResponse->ResponseData.token_value.len = strlen(gDefaultDeviceName);
					LOG_INFO("ProvHttpServerEventHdl: Device name: %s\n\r", gDefaultDeviceName);
				}
			}
				break;

			// handle POST token
			case SL_NETAPP_HTTPPOSTTOKENVALUE_EVENT:
			{
				unsigned char *ptr = apSlHttpServerEvent->EventData.httpPostData.token_name.data;
				LOG_INFO("httpServerCallback: Post token %s\n\r", ptr);

				// Add profile token
				if ( 0 == memcmp(ptr, POST_ADD_PROFILE_TOKEN, STRING_TOKEN_SIZE) )
				{
					Report("[Provisioning] Profile successfully added - SSID: %s\r\n",apSlHttpServerEvent->EventData.httpPostData.token_value.data);

					//LOG_INFO("httpServerCallback: Post add profile token, SSID %s\n\r", apSlHttpServerEvent->EventData.httpPostData.token_value.data);
					SET_ADD_RPOFILE_RECEIVED();
					g_cNewProfileSsidLen = apSlHttpServerEvent->EventData.httpPostData.token_value.len;
					memcpy(g_cNewProfileSsid,
						   apSlHttpServerEvent->EventData.httpPostData.token_value.data, g_cNewProfileSsidLen);

					sl_SyncObjSignal(&g_EventRecvSyncObj);
				}

				// Force scan token
				else if ( 0 == memcmp(ptr, POST_START_SCAN_TOKEN, STRING_TOKEN_SIZE) )
				{
					//LOG_DEBUG("httpServerCallback: Post force scan token, value %s\n\r", apSlHttpServerEvent->EventData.httpPostData.token_value.data);
					SET_FORCE_SCAN_RECEIVED();
					sl_SyncObjSignal(&g_EventRecvSyncObj);
				}
			}
				break;

			default:
				break;
		}

		// if we are in the middle of provisioning do not pass this event to app
		return EVENT_PROPAGATION_BLOCK;
	}

	return EVENT_PROPAGATION_CONTINUE;
}

//*****************************************************************************
//
//! \brief This function serves as first level WLAN events handler.
//!        It runs under driver context and performs only operation that can run
//!        from this context. For operations that can't is sets an indication of
//!        received token and preempts the provisioning context.
//!
//! \param apEventInfo Pointer to the WLAN event information
//!
//! \return None
//!
//*****************************************************************************
_SlEventPropogationStatus_e sl_Provisioning_WlanEventHdl(SlWlanEvent_t *apEventInfo)
{
    unsigned long event = apEventInfo->Event;

    if(g_UseProvisioningLibHandlers)
    {
    	switch (event)
    	    {
    	        case SL_WLAN_CONNECT_EVENT:
    	            SET_DEVICE_CONNECTED();
    	            LOG_INFO("provisioning_WlanEventHdl: SL STA is connected to %s\n\r", apEventInfo->EventData.STAandP2PModeWlanConnected.ssid_name);
    	            sl_SyncObjSignal(&g_EventRecvSyncObj);
    	            break;

    	        case SL_WLAN_DISCONNECT_EVENT:
    	            SET_DEVICE_DISCONNECTED();
    	            LOG_INFO("provisioning_WlanEventHdl: SL STA is disconnected\n\r");
    	            sl_SyncObjSignal(&g_EventRecvSyncObj);
    	            break;

    	        case SL_WLAN_STA_CONNECTED_EVENT:
    	            // Once a STA is connected to SL AP, wait for provisioning commands
    	            // time to wait is defined by application configuration.
    	            g_libraryTimeout = SL_EXT_PROV_WAIT_FOREVER;
    	            if (0 < g_ProvisioningCfg.Timeout10Secs)
    	            {
    	                g_libraryTimeout = ((_i32)g_ProvisioningCfg.Timeout10Secs) * 1000 * 10;
    	            }
    	            LOG_INFO("provisioning_WlanEventHdl: STA %.2x-%.2x-%.2x-%.2x-%.2x-%.2x is connected to SL AP, provisioning TO %d\n\r",
    	                                apEventInfo->EventData.APModeStaConnected.mac[0],
										apEventInfo->EventData.APModeStaConnected.mac[1],
										apEventInfo->EventData.APModeStaConnected.mac[2],
										apEventInfo->EventData.APModeStaConnected.mac[3],
										apEventInfo->EventData.APModeStaConnected.mac[4],
										apEventInfo->EventData.APModeStaConnected.mac[5],

    	                                g_libraryTimeout);
    	            sl_SyncObjSignal(&g_EventRecvSyncObj);
    	            break;

    	        case SL_WLAN_STA_DISCONNECTED_EVENT:
    	            CLEAR_STA_CONNECTED_TO_AP();
    	            LOG_INFO("provisioning_WlanEventHdl: STA was disconnected from SL AP\n\r");
    	            sl_SyncObjSignal(&g_EventRecvSyncObj);
    	            break;
    	    }
    }


    return EVENT_PROPAGATION_CONTINUE;
}

//*****************************************************************************
//
//! \brief This function serves as first level network application events handler.
//!        It runs under driver context and performs only operation that can run
//!        from this context. For operations that can't is sets an indication of
//!        received token and preempts the provisioning context.
//!
//! \param apEventInfo Pointer to the net app event information
//!
//! \return None
//!
//*****************************************************************************
_SlEventPropogationStatus_e sl_Provisioning_NetAppEventHdl(SlNetAppEvent_t *apNetAppEvent)
{
    int signalNeeded = !(OBTAIN_IP_AS_AP == g_IpObtainState);

    if(g_UseProvisioningLibHandlers)
    {
    	if (SL_NETAPP_IPV4_IPACQUIRED_EVENT == apNetAppEvent->Event)
    	    {
    	        // Note that this function is activated from WLAN event
    	        // context and current device mode may not be updated yet
    	        // in proviosning context. Hence current mode isn't reported below.
    	        LOG_INFO("provisioning_NetAppHdl: IP acquired 0x%x signalNeeded %d\n\r", apNetAppEvent->EventData.ipAcquiredV4.ip, signalNeeded);

    	        // Note - the order for the code below is important.
    	        // Sync object should not be given for AP mode.
    	        // Since contexts priorities are unknown update g_IpObtainState
    	        // before giving the sync object.
    	        g_IpObtainState = OBTAIN_IP_OBTAINED;
    	        if (signalNeeded)
    	        {
    	        	SET_IP_ACQUIRED();
    	            sl_SyncObjSignal(&g_EventRecvSyncObj);
    	        }
    	    }
    }

    return EVENT_PROPAGATION_CONTINUE;
}


