/*
 * internal_api.h - Provisioning library internal API
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

#ifndef AP_PROVISIONING_API_H_
#define AP_PROVISIONING_API_H_

#include "simplelink.h"
#include "provisioning_api.h"       // for slExtLibProvCfg_t definition

/*****************************************************************************/
/* Global variables                                                          */
/*****************************************************************************/

extern SlWlanMode_e g_iCurrentSlMode;          // SL mode (AP or STA) as received from last sl_Start

extern _u8          g_ucSlEventBitMask;        // Bit mask holding SL device indications
extern _u8          g_ucRxTokensBitMask;       // Bit mask holding received tokens indications

extern cfgConfirmStates_e       g_cfgConfirmState;
extern cfgConfirmResult_e       g_ConfigResult;   // Provisioning sequence result
extern networksListHandling_e   g_NetListHandlingState;   // Networks list handling state machine state

// Last received SSID (used for testing whether network was found around)
extern _u8          g_cNewProfileSsid[];
extern _u8          g_cNewProfileSsidLen;

// Scan results info
extern _u8          g_ucNumNetworksInList;
extern Sl_WlanNetworkEntry_t    g_Networks[];

// Sync object, used for arming a timer
extern _u8          g_libraryEvent;

// Configuration set by user application on library init
extern slExtLibProvCfg_t        g_ProvisioningCfg;

// Time to wait on sync object
extern _i32                     g_libraryTimeout;

// Indication that IP address was acquired.
extern obtainIpState_e          g_IpObtainState;

// Hold deafult device name, originated by the device before device name was ever set by user.
extern char gDefaultDeviceName[];

extern SlDateTime_t             g_PowerUpTime;
extern SlDateTime_t             g_ProvisioningCompleteTime;

/*****************************************************************************/
/* AP Provisioning APIs                                                      */
/*****************************************************************************/

// Provisioning state indication- TRUE for AP confirmation state, FALSE for SC confirmation
provisioningLogicStates_e getProvisioningState();

// Networks list handling functions; responsible for managing WLAN scan,
// getting and displaying avalilable networks list
void networksListReadScanResults();
int  networksListScan(int aEnableStaScan);
int  networksListHandleGetNetworkToken(unsigned char *apToken, SlHttpServerResponse_t *apSlHttpServerResponse);
void networksListHandleCfgResult(cfgConfirmResult_e aCfgResult);
void networksListHandleStaDisconnectedEvent();

// Profiles adding/removing and result confirmation functions
int  cfgConfirmStart(char *apSsid, int aSsidLen);
void cfgConfirmGetResult(SlHttpServerResponse_t *apSlHttpServerResponse);
void cfgNotifyStaConnected();
void cfgNotifyStaIpAcquired();
void cfgNotifyTimeout();
void cfgNotifyResultTaken();

// Utility functions, used by provisioning modules
int SetDeviceMode(SlWlanMode_e aMode);

// Macros for setting/testing device state
// Using these macros allow easy replacement of global variables with one state variable
#define IS_DEVICE_CONNECTED()           (g_ucSlEventBitMask & SL_STATE_AP_ASSOC)
#define IS_DEVICE_DISCONNECTED()        (g_ucSlEventBitMask & SL_STATE_AP_DISASSOC)
#define IS_IP_ACQUIRED()                (g_ucSlEventBitMask & SL_STATE_IP_ALLOC)
#define IS_STA_CONNECTED_TO_AP()        (g_ucSlEventBitMask & SL_STATE_AP_CONNECTED)
#define NO_PENDING_EVENTS()             (0 == g_ucSlEventBitMask)

// STA state macros
#define SET_DEVICE_CONNECTED()          (g_ucSlEventBitMask |= SL_STATE_AP_ASSOC)
#define SET_DEVICE_DISCONNECTED()       (g_ucSlEventBitMask |= SL_STATE_AP_DISASSOC)
#define SET_IP_ACQUIRED()               (g_ucSlEventBitMask |= SL_STATE_IP_ALLOC)
#define CLEAR_DEVICE_CONNECTED()        (g_ucSlEventBitMask &= ~SL_STATE_AP_ASSOC)
#define CLEAR_DEVICE_DISCONNECTED()     (g_ucSlEventBitMask &= ~SL_STATE_AP_DISASSOC)
#define CLEAR_IP_ACQUIRED()             (g_ucSlEventBitMask &= ~SL_STATE_IP_ALLOC)
#define CLEAR_IP_AND_CONNECTION()       (g_ucSlEventBitMask &= ~(SL_STATE_IP_ALLOC | SL_STATE_AP_ASSOC))

// AP state macros
#define SET_STA_CONNECTED_TO_AP()       (g_ucSlEventBitMask |= SL_STATE_AP_CONNECTED)
#define CLEAR_STA_CONNECTED_TO_AP()     (g_ucSlEventBitMask &= ~SL_STATE_AP_CONNECTED)

// Macros for testing received tokens
#define IS_GET_NETWORKS_RECEIVED()      (g_ucRxTokensBitMask & PRO_USER_TOKEN_GET_NETWORKS)
#define IS_ADD_RPOFILE_RECEIVED()       (g_ucRxTokensBitMask & PRO_USER_TOKEN_ADD_PROFILE)
#define IS_FORCE_SCAN_RECEIVED()        (g_ucRxTokensBitMask & PRO_USER_TOKEN_FORCE_SCAN)
#define IS_GET_RESULT_RECEIVED()        (g_ucRxTokensBitMask & PRO_USER_TOKEN_GET_RESULT)
#define IS_IOT_UUID_RECEIVED()          (g_ucRxTokensBitMask & PRO_USER_TOKEN_UUID)
#define IS_TERMINATE_PRO_RECEIVED()     (g_ucRxTokensBitMask & PRO_USER_TOKEN_TERMINATE)
#define NO_PENDING_TOKENS()         	(0 == g_ucRxTokensBitMask)

#define SET_GET_NETWORKS_RECEIVED()     (g_ucRxTokensBitMask |= PRO_USER_TOKEN_GET_NETWORKS)
#define SET_ADD_RPOFILE_RECEIVED()      (g_ucRxTokensBitMask |= PRO_USER_TOKEN_ADD_PROFILE)
#define SET_FORCE_SCAN_RECEIVED()       (g_ucRxTokensBitMask |= PRO_USER_TOKEN_FORCE_SCAN)
#define SET_GET_RESULT_RECEIVED()       (g_ucRxTokensBitMask |= PRO_USER_TOKEN_GET_RESULT)
#define SET_IOT_UUID_RECEIVED()         (g_ucRxTokensBitMask |= PRO_USER_TOKEN_UUID)
#define SET_TERMINATE_PRO_RECEIVED()    (g_ucRxTokensBitMask |= PRO_USER_TOKEN_TERMINATE)

#define CLEAR_GET_NETWORKS_RECEIVED()   (g_ucRxTokensBitMask &= ~PRO_USER_TOKEN_GET_NETWORKS)
#define CLEAR_ADD_RPOFILE_RECEIVED()    (g_ucRxTokensBitMask &= ~PRO_USER_TOKEN_ADD_PROFILE)
#define CLEAR_FORCE_SCAN_RECEIVED()     (g_ucRxTokensBitMask &= ~PRO_USER_TOKEN_FORCE_SCAN)
#define CLEAR_GET_RESUL_RECEIVED()      (g_ucRxTokensBitMask &= ~PRO_USER_TOKEN_GET_RESULT)
#define CLEAR_IOT_UUID_RECEIVED()       (g_ucRxTokensBitMask &= ~PRO_USER_TOKEN_UUID)
#define CLEAR_TERMINATE_PRO_RECEIVED()  (g_ucRxTokensBitMask &= ~PRO_USER_TOKEN_TERMINATE)

#endif /* AP_PROVISIONING_API_H_ */
