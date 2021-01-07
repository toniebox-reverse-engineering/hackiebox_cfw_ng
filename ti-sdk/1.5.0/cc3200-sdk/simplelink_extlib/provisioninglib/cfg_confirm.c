/*
 * cfgConfirm.c -
 *      1. Manage the configuration confirmation state machine for AP provisioning
 *          - switch to STA mode and try to connect to configured AP
 *          - wait for result to be taken in STA mode (if connection succeeds)
 *          - wait for result to be taken in AP mode (if not taken from STA mode)
 *      2. Update cfg configuration result
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

#include "provisioning_defs.h"
#include "internal_api.h"


extern _u8 g_StopUDPBcast;

extern int Report(const char *format, ...);

/*****************************************************************************/
/* Internal types definitions                                                */
/*****************************************************************************/

/*****************************************************************************/
/* Globals                                                                   */
/*****************************************************************************/

// Configuration SM state
cfgConfirmStates_e g_cfgConfirmState = CFG_CONFIRM_STATE_IDLE;

// Configuration result 
cfgConfirmResult_e g_ConfigResult = CFG_CONFIRM_RES_NOT_STARTED;

//*****************************************************************************
//! \brief This function returns the confirmation result
//!
//! \param   Pointer to the HTTP response to be updated by this function
//! \return  none
//*****************************************************************************
void cfgConfirmGetResult(SlHttpServerResponse_t *apSlHttpServerResponse)
{
	apSlHttpServerResponse->ResponseData.token_value.data[0] = '0' + g_ConfigResult;

	apSlHttpServerResponse->ResponseData.token_value.len = 1;

    LOG_INFO("cfgConfirmFillResult: Returned result %s\n\r",
                 apSlHttpServerResponse->ResponseData.token_value.data);

    // Once result is taken, the provisioning sequence is termninated; mark the
    // result was taken only if the configuration sequence was started.
    // This function is activated from event handler context, the provisioning
    // SM shall be updated from provisioning context.
    if (CFG_CONFIRM_RES_NOT_STARTED != g_ConfigResult)
    {
        SET_GET_RESULT_RECEIVED();
        // Note that cfg state and result may still be used by other modules, don't reset !!!
    }


    // if provisioning succeeded we can stop the UDP broadcast
    if (CFG_CONFIRM_RES_SUCCESS == g_ConfigResult)
    {
    	// print message to logger
    	Report("[Provisioning] Profile Confirmation Success! \r\n");
    	g_StopUDPBcast = TRUE;
    }
}

//*****************************************************************************
//! \brief This function updates module state when SL STA is connected to an AP
//!
//! \param   none
//! \return  none
//*****************************************************************************
void cfgNotifyStaConnected()
{
    if (CFG_CONFIRM_STATE_WAIT_ASSOC != g_cfgConfirmState)
        return;

    LOG_DEBUG("cfgNotifyStaConnected: Connected to AP, waiting for IP address\n\r");

    // Device is connected, now wait for IP address
    g_libraryTimeout = STA_IP_ACQUIRED_TIMEOUT_MSECS;
    g_cfgConfirmState = CFG_CONFIRM_STATE_WAIT_IP_ADD;
}

//*****************************************************************************
//! \brief This function updates module state when IP address is obtained
//!
//! \param   none
//! \return  none
//*****************************************************************************
void cfgNotifyStaIpAcquired()
{
    if (CFG_CONFIRM_STATE_WAIT_IP_ADD != g_cfgConfirmState)
        return;

    LOG_DEBUG("cfgNotifyStaIpAcquired: got IP address, waiting to publish result as STA\n\r");

    // Connection succeeded and IP address was obtained,
    // wait in STA mode for someone to take the result
    g_cfgConfirmState = CFG_CONFIRM_STATE_GET_RESULT_STA;
	
    // Assume we got here after AP provisioning mode
    g_libraryTimeout = GET_RESULT_AS_STA_AFTER_AP_MAX_MSECS;

    // Configuration sequence was successfully completed, update result
    g_ConfigResult = CFG_CONFIRM_RES_SUCCESS;

    // Inform networks list handler with configuration success
    networksListHandleCfgResult(g_ConfigResult);
}

//*****************************************************************************
//! \brief This function handles timeout notification
//!
//! \param   none
//! \return  none
//*****************************************************************************
void cfgNotifyTimeout()
{
    int waitAsAp = 1;
    int i;

    switch (g_cfgConfirmState)
    {
        case CFG_CONFIRM_STATE_GET_RESULT_STA:
            LOG_DEBUG("cfgNotifyTimeout: timeout on taking result as STA\n\r");
            // g_cfgConfirmState is updated below, after the switch case
            break;

        case CFG_CONFIRM_STATE_GET_RESULT_AP:
            LOG_DEBUG("cfgNotifyTimeout: timeout on taking result as AP\n\r");
            g_cfgConfirmState = CFG_CONFIRM_STATE_IDLE;
            // this is the only state that doesn't change to CFG_CONFIRM_STATE_GET_RESULT_AP
            waitAsAp = 0;
            break;

        case CFG_CONFIRM_STATE_WAIT_ASSOC:
            // in case of timeout, either AP isn't around or a wrong password is used.
            // if AP isn't a scan result assume it is not a hidden network and
            // report of authentication failure.
            // If hidden network exists & password is incorrect - report is AP not found

            // check if scan results
            for (i=0 ; i<g_ucNumNetworksInList ; ++i)
            {
                // no point to compare SSIDs if their length is not equal
                if (g_cNewProfileSsidLen != g_Networks[i].ssid_len)
                    continue;

                // length equal, compare SSID
                if (0 == memcmp(g_cNewProfileSsid, g_Networks[i].ssid, g_cNewProfileSsidLen))
                {
                    LOG_DEBUG("cfgNotifyTimeout: Connection timeout for AP %s (scan result) \n\r", g_Networks[i].ssid);

                    g_ConfigResult = CFG_CONFIRM_RES_CONNECTION_FAILED;
                    break;
                }
            }

            // if not found, set the global result and return
            if (i == g_ucNumNetworksInList)
            {
                LOG_DEBUG("cfgNotifyTimeout: AP %s isn't a scan result \n\r", g_Networks[i].ssid);
                g_ConfigResult = CFG_CONFIRM_RES_NETWORK_NOT_FOUND;
            }
            break;

        case CFG_CONFIRM_STATE_WAIT_IP_ADD:
            LOG_DEBUG("cfgNotifyTimeout: IP address timeout\n\r");
            g_ConfigResult = CFG_CONFIRM_RES_IP_NOT_ACQUIRED;
            break;
    }


    // In any case of failure wait for result to be take from SL AP
    if (waitAsAp)
    {
        g_cfgConfirmState = CFG_CONFIRM_STATE_GET_RESULT_AP;
        // In case of success, limit the time we're waiting in AP mode for result to be taken
        // On case of failure, wait forever - allow user to correct the configuration
        g_libraryTimeout = (CFG_CONFIRM_RES_SUCCESS == g_ConfigResult)? GET_RESULT_AS_AP_MAX_MSECS : SL_EXT_PROV_WAIT_FOREVER;
        SetDeviceMode(ROLE_AP);
        if (ROLE_AP != g_iCurrentSlMode)
        {
            LOG_ERROR("cfgNotifyTimeout: failed to switch to AP mode, mode is %d\n\r", g_iCurrentSlMode);
        }

        // Inform networks list handler of configuration result
        networksListHandleCfgResult(g_ConfigResult);
    }
}

//*****************************************************************************
//! \brief Notify the module result was taken. This resets the state machine and
//!        the device to desired mode.
//!
//! \param   none
//! \return  none
//*****************************************************************************
void cfgNotifyResultTaken()
{	
    // Result was taken, confirmation stage is complete regardless of result and SL mode
    g_cfgConfirmState = CFG_CONFIRM_STATE_IDLE;


    // ----------------------------------
    // Handle successful configuration:
    //      switch to STA mode anyway
    // ----------------------------------
    if (CFG_CONFIRM_RES_SUCCESS == g_ConfigResult)
    {
        LOG_DEBUG("cfgNotifyResultTaken: provisioning done, switching to STA\n\r");

        // Sleep for 1 second - give a chance to the application to successfully read the result
        SetDeviceMode(ROLE_STA);
    }
}

//*****************************************************************************
//! \brief This function is the entry point to the state machine which handles
//!        configuration confirmation.
//!        Device may be either in AP or STA mode when function is invoked.
//!        When function returns the device waits as STA (with timeout) for
//!        WiFi connection
//!
//! \param   apSsid               - SSID to confirm
//!          aSsidLen             - SSID length
//! \return  0 on success, -1 otherwise
//*****************************************************************************
int cfgConfirmStart(char *apSsid, int aSsidLen)
{
    LOG_DEBUG("cfgConfirmStart activated\n\r");

    // Prepare scan result
    g_ConfigResult = CFG_CONFIRM_RES_NOT_STARTED;
    CLEAR_GET_RESUL_RECEIVED();

    // Prepare state and timeout for failure, may be changed later
    g_cfgConfirmState = CFG_CONFIRM_STATE_GET_RESULT_AP;
    g_libraryTimeout = SL_EXT_PROV_WAIT_FOREVER;

    if (0 == aSsidLen)
    {
        LOG_ERROR("cfgConfigConfirmAp: error in configured AP parameters, length is 0\n\r");
        g_ConfigResult = CFG_CONFIRM_RES_GENERAL_ERROR;
        return -1;
    }

	// ----------------------------------------------------------------
	// Check WLAN connection
	// ----------------------------------------------------------------
    LOG_DEBUG("cfgConfigConfirmAp: will try to connect to selected AP\n\r");

    // Make sure auto connect is enabled (might be disabled after SC)
    sl_WlanPolicySet(SL_POLICY_CONNECTION,
                      SL_CONNECTION_POLICY(1,0,0,0,0),
                      NULL,
                      0);

    // Switch to STA mode (DHCP was already enabled)
	SetDeviceMode(ROLE_STA);

	// verify the device has switched to STA mode
	if (ROLE_STA != g_iCurrentSlMode)
	{
	    g_ConfigResult = CFG_CONFIRM_RES_GENERAL_ERROR;

		LOG_ERROR("cfgConfigConfirmAp: failed to switch to STA mode, mode is %d\n\r", g_iCurrentSlMode);
		return -1;
	}

	// Device is in STA mode, wait for WLAN connection/timeout
	g_libraryTimeout = STA_WLAN_ASSOC_TIMEOUT_MSECS;
	g_cfgConfirmState = CFG_CONFIRM_STATE_WAIT_ASSOC;

	return 0;
}

