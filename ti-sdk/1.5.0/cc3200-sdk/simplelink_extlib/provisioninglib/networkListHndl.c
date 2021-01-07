/*
 * networkListHndl.c - Provisioning library network list handling state machine management
 *  	- wait for scan trigger
 *  	- manage scan (R1.0 in STA mode; R2.0 AP mode) and get scan results
 *  	- wait for add profile indication
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

#include "provisioning_api.h"
#include "provisioning_defs.h"
#include "internal_api.h"

/*****************************************************************************/
/* Local functions                                                           */
/*****************************************************************************/
static int getSlTimeSec();

/*****************************************************************************/
/* Macros definitions                                                        */
/*****************************************************************************/

/*****************************************************************************/
/* Internal types definitions                                                */
/*****************************************************************************/

/*****************************************************************************/
/* Globals                                                                   */
/*****************************************************************************/

// SM states
networksListHandling_e g_NetListHandlingState = NETS_LIST_HNDL_WAIT_IDLE;

// Networks in device vicinity, updated by activating sl_WlanGetNetworkList
Sl_WlanNetworkEntry_t  g_Networks[MAX_NETWORKS_TO_GET];
unsigned char          g_ucNumNetworksInList = 0;

// Index of the next network to send to application.
// Networks are retrieved using get first network token or get next network token.
// For R1.0, the counter is managed by the library.
// For R2.0, get networks is implemented in the NWP.
static unsigned char   g_ucNextNetworkToGet;

// Scan result timestamp; can be used for testing results validity
static int    			g_cScanTimestampSecs = -1;

//*****************************************************************************
//! \brief This function returns an entry in the networks list.
//!        It should be invoked after at least one scan has occured.
//!
//!  Note: This function is activated from event context, activation of
//!        SL API is forbidden !!!
//!
//! \param   IN  pointer to the received token, which carries network index
//! \param   OUT pointer to the response, to be filled by this function
//! \return  0 - on success, -1 - error in case of request error
//*****************************************************************************
int networksListHandleGetNetworkToken(unsigned char *apToken,
		                              SlHttpServerResponse_t *apSlHttpServerResponse)
{
	int  networkIndex;
	int  ssidLength;
	char temp[] = ";";
	char emptyNet[] = "X;";
	char securityType = '0';

	// Check if we received get first or get next token
    if (0 == memcmp(GET_FIRST_NETWORK_TOKEN, apToken, STRING_TOKEN_SIZE))
    {
        g_ucNextNetworkToGet = 0;
    }
    else if (0 != memcmp(GET_NEXT_NETWORK_TOKEN, apToken, STRING_TOKEN_SIZE))
    {
        LOG_ERROR("networksListHandleGetNetworkToken: ERROR got unexpected token %s, exiting \n\r", apToken);
        return (-1);
    }
    // else
    // got get next token

	// When we get here scan results are ready:
	// 	- if AP scan isn't supported scan was already performed in STA mode
	//  - if AP scan is supported, scan was already performed
	// anyway, send the updated result
	networkIndex = g_ucNextNetworkToGet++;

	LOG_DEBUG("networksListHandleGetNetworkToken network ID %d, num available networks %d\n\r",
	            networkIndex, g_ucNumNetworksInList);

	// If requested index exceeds number of available result:
	if (g_ucNumNetworksInList <= networkIndex)
	{
		// send empty indication
		memcpy(apSlHttpServerResponse->ResponseData.token_value.data,
				emptyNet, strlen(emptyNet));
		apSlHttpServerResponse->ResponseData.token_value.len = 2;

		// and clear network parameters
		if (networkIndex < MAX_NETWORKS_TO_GET)
		{
			g_Networks[networkIndex].ssid_len = 0;
		}

		LOG_DEBUG("Scan result [%d] is empty, sending %s\n\r", networkIndex, apSlHttpServerResponse->ResponseData.token_value.data);
		return 0;
	}

	// Fill SSID & length, RSSI and security type are also sent in this token.
	// Structure:
	//      1 char - security type
	//      variable length - SSID string
	//      1 char - separator (;)

	ssidLength = g_Networks[networkIndex].ssid_len;

	// Security type
	securityType += g_Networks[networkIndex].sec_type;
	*(apSlHttpServerResponse->ResponseData.token_value.data) = securityType;

	// SSID
	memcpy(&(apSlHttpServerResponse->ResponseData.token_value.data[1]),
	       g_Networks[networkIndex].ssid, ssidLength);

	// One empty char between SSID and next field
	*(&(apSlHttpServerResponse->ResponseData.token_value.data[ssidLength+1])) = temp[0];

	apSlHttpServerResponse->ResponseData.token_value.len = ssidLength+2;

	if (g_Networks[networkIndex].ssid_len)
	{
		/*
	    LOG_DEBUG("Scan result [%d] SSID len %d SSID %s\n\r",
	            networkIndex,
	            g_Networks[networkIndex].ssid_len,
	            apSlHttpServerResponse->ResponseData.token_value.data);
	     */
	}

	return 0;
}

//*****************************************************************************
//! \brief This function updates the SM according to configuration result
//!
//! \param   aCfgResult - CFG_CONFIRM_RES_SUCCESS - succeeded, else - failed
//! \return  none
//*****************************************************************************
void networksListHandleCfgResult(cfgConfirmResult_e aCfgResult)
{
    if (CFG_CONFIRM_RES_SUCCESS == g_ConfigResult)
    {
        // If one confiuration succeeds, set state to DONE.
        // Configuration application won't be able to force AP provisioning.
        // Device returns to STA mode anyway.
        g_NetListHandlingState = NETS_LIST_HNDL_DONE;
    }
    else
    {
        // Wait for the application to refresh the scan results
        // If AP scan is supported, periodic scan is performed. There's no need
        // to go through the scan state.
        // Even if not, we don't want to switch to STA mode for scan.
        g_NetListHandlingState = NETS_LIST_HNDL_PUBLISH_NETS_LIST;
    }
}

//*****************************************************************************
//! \brief This function handles a disconnection event while we're in AP mode.
//!        Nothibg to do here
//!
//! \param   none
//! \return  none
//*****************************************************************************
void networksListHandleStaDisconnectedEvent()
{
    LOG_DEBUG("networksListHandleStaDisconnectedEvent\n\r");
}

//*****************************************************************************
//! \brief This function is used to start a scan cycle
//!        - if time elapsed from last scan < threshold, assume results are still
//!           valid and don't kick another scan
//!        - if device isn't in the requested scan mode, set the device mode
//!
//! \param   aEnableStaScan - only if TRUE device is allowed to switch to STA mode
//!                           in order to scan.
//! \return  0 - success, -1 device error
//*****************************************************************************
int networksListScan(int aEnableStaScan)
{
    int retVal;
    int scanAsSta = (0 == IS_AP_SCAN_SUPPORTED());
    SlWlanMode_e  scanMode = (1 == scanAsSta)? ROLE_STA : ROLE_AP;

    LOG_DEBUG("networksListScan: scan mode %d (1 STA, 0 AP) \n\r", scanAsSta);

    if ( (0 == aEnableStaScan) && scanAsSta )
    {
        LOG_INFO("networksListScan: skipping STA scan \n\r");
        g_NetListHandlingState = NETS_LIST_HNDL_PUBLISH_NETS_LIST;

        return 0;
    }

    // Check scan results validity; avoid frequent scans especially for R1.0
    // where it involves role switch
    if (0 <= g_cScanTimestampSecs)
    {
        int currSec;

        currSec = getSlTimeSec();
        LOG_DEBUG("networksListScan: Curr time %d last scanned at %d\n\r", currSec,  g_cScanTimestampSecs);

        if ( (currSec - g_cScanTimestampSecs) < SCAN_GET_NETWORKS_TIMEOUT_SECS)
        {
			// we're not going to scan, so state is changed to publish
        	g_NetListHandlingState = NETS_LIST_HNDL_PUBLISH_NETS_LIST;
            LOG_DEBUG("networksListScan: dropping scan request\n\r");
            return 0;
        }
    }

    // else: this is the first scan since system was started
    g_NetListHandlingState = NETS_LIST_HNDL_SCANNING;
    g_ucNumNetworksInList = 0;
    g_ucNextNetworkToGet = 0;

    SetDeviceMode(scanMode);
    if (scanMode != g_iCurrentSlMode)
    {
        return -1;
    }

    // hack, to avoid a known bug.
    // happens only when profiles exist.
    sl_extlib_ProvWaitHdl(1000);

    // configure scan period and enable scan, this triggers an immediate scan
    retVal = sl_WlanPolicySet(SL_POLICY_SCAN,
                              SL_SCAN_POLICY_EN(1), NULL, 0);
    if(retVal < 0) LOG_ERROR("Failure in line %d\n\r", __LINE__);

    // arm timer, scan results are collected on timer expiry
    g_libraryTimeout = WAIT_SCAN_TIMEOUT_MSECS;

    return 0;
}



_u8 compressScanResults(_u8 NumOfScanEntries)
{
	_u8 i=0;
	_u8 j=0;
	_u8 NumOfUniqueSSID = 0;
	_u8 UniqueSSIDCounter = 0;

	Sl_WlanNetworkEntry_t* pToSearch=NULL;
	Sl_WlanNetworkEntry_t* pCurr=NULL;

	for (j=0; j < (NumOfScanEntries-1) ; j++)
	{
		pToSearch = &g_Networks[j];

		if (pToSearch->ssid_len == 0)
			continue;

		// search till the end of the list
		for (i=(j+1) ; i < NumOfScanEntries; i++)
		{
		   pCurr = &g_Networks[i];

		   if (pCurr->ssid_len == 0)
			continue;


		   // if the length is identical compare the ssid
		   if (pCurr->ssid_len == pToSearch->ssid_len)
		   {

			   // ssid are identical
			   if (strncmp((char*)pToSearch->ssid, (char*)pCurr->ssid, pToSearch->ssid_len) == 0)
			   {
				   //LOG_DEBUG("SSID Identical found! %s\n", pToSearch->ssid);

				   if (pToSearch->rssi > pCurr->rssi)
				   {
					   //clear the current ssid entry by setting its len to zero
					   // the current idx always larger the the one we start searching for
					   pCurr->ssid_len = 0;
					   memset(pCurr, 0, sizeof(Sl_WlanNetworkEntry_t));
				   }
				   else
				   {
						// if the one that we started compared from has weaker rssi
						// override the one with weaker signal by the one with stronger signal (rssi)
						memcpy(pToSearch, pCurr, sizeof(Sl_WlanNetworkEntry_t));

						// now clear the current entry
						pCurr->ssid_len = 0;
						memset(pCurr, 0, sizeof(Sl_WlanNetworkEntry_t));
				   }
			   }
		   }

		}

	}


	// now 'compress' the table and clear zero length SSIDs
	for (i=0; i< NumOfScanEntries; i++)
	{
		if (g_Networks[i].ssid_len != 0)
		{
			NumOfUniqueSSID++;
		}
	}


	// now 'compress' the table and clear zero length SSIDs
	for (i=0; i< NumOfScanEntries; i++)
	{
		if (g_Networks[i].ssid_len != 0)
		{
			UniqueSSIDCounter++;
		}
		else
		{
		   // zero ssid len found. replace it with existing entry
		   j = i +1; // strating from next entry

		   while ((g_Networks[j].ssid_len ==0) && (j < NumOfScanEntries))
			   j++;

		   // stopping point
		   if (j == NumOfScanEntries)
		   {
			   break; // exit the loop
		   }
		   else
		   {
			  // entry to replace found
			   memcpy(&g_Networks[i], &g_Networks[j], sizeof(Sl_WlanNetworkEntry_t));

			   g_Networks[j].ssid_len = 0;
			   memset(&g_Networks[j], 0, sizeof(Sl_WlanNetworkEntry_t));

			   UniqueSSIDCounter++;
		   }

		}

		// stopping point
		if (NumOfUniqueSSID == UniqueSSIDCounter)
			break;
	}

	return NumOfUniqueSSID;

}


//*****************************************************************************
//! \brief This function reads scan results from the NWP.
//!        It is activated on timeout expiration (after scan was started)
//!
//! \param   none
//! \return  none
//*****************************************************************************
void networksListReadScanResults()
{
    int i;

	// get scan results
    g_ucNumNetworksInList = sl_WlanGetNetworkList(0, MAX_NETWORKS_TO_GET, g_Networks);


    if (g_ucNumNetworksInList)
    {
    	// Keep scan timestamp. To be used to filter future requests if not enough
		// time has elapsed since last scan.
		// Only the minutes filed is used in order to minimize memory usage
		g_cScanTimestampSecs = getSlTimeSec();
    }
	

    g_ucNumNetworksInList = compressScanResults(g_ucNumNetworksInList);

    LOG_DEBUG("Scan complete, got %d networks\n\r", g_ucNumNetworksInList);

    for (i=0 ; i<g_ucNumNetworksInList ; ++i)
	{
		LOG_DEBUG("[%d] SSID %s [%d]\n\r",i, g_Networks[i].ssid, g_Networks[i].rssi);
	}


    // If scanned in STA mode, return to AP mode
    SetDeviceMode(ROLE_AP);

    // Scan results are valid, update state
    g_NetListHandlingState = NETS_LIST_HNDL_PUBLISH_NETS_LIST;
}

//*****************************************************************************
//! \brief This function reads timestamp from NWP since there's no timer API
//!        in the driver library. Timestamp (in minutes) is used to filter
//!        frequent scan requests.
//!
//! \param   none
//! \return  none
//*****************************************************************************
static int getSlTimeSec()
{
    SlDateTime_t  dateTime;
    _u8 configLen;
    _u8 configOpt;

    configLen = sizeof(SlDateTime_t);
    configOpt = SL_DEVICE_GENERAL_CONFIGURATION_DATE_TIME;
    sl_DevGet(SL_DEVICE_GENERAL_CONFIGURATION,&configOpt, &configLen,(_u8 *)(&dateTime));

    return (int)(60*dateTime.sl_tm_min+dateTime.sl_tm_sec);
}
