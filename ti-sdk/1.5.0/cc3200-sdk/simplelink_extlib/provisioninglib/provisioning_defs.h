/*
 * provisioning_defs.h - Provisioning library definitions and global variables
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

#ifndef AP_PROVISIONING_DEFS_H_
#define AP_PROVISIONING_DEFS_H_

#include <stdio.h>
#include <stdarg.h>
#include "simplelink.h"

// ============================================================================
//  Debug definitions (logging)
// ============================================================================

//#define DEBUG_MODE

#define LOG_CONSOLE(...) \
{\
	char buffer[256];\
	sprintf(buffer, "[DEBUG ProvLib] ");\
	sprintf(buffer + strlen(buffer), __VA_ARGS__);\
	Report(buffer);\
};


#if defined (ERROR_MODE) | defined (INFO_MODE) | defined (DEBUG_MODE)
#include "uart_if.h"
#endif

#ifdef DEBUG_MODE
#define LOG_DEBUG(...) \
        {\
            char buffer[256];\
            sprintf(buffer, "[DEBUG ProvLib] ");\
			sprintf(buffer + strlen(buffer), __VA_ARGS__);\
            Report(buffer);\
        };
#else
#define LOG_DEBUG(...) \
        {\
        };
#endif

#if defined (INFO_MODE) | defined (DEBUG_MODE)
#define LOG_INFO(...) \
        {\
            char buffer[256];\
            sprintf(buffer, "[INFO  ProvLib] ");\
			sprintf(buffer + strlen(buffer), __VA_ARGS__);\
            Report(buffer);\
        };
#else
#define LOG_INFO(...) \
        {\
        };
#endif

#if defined (ERROR_MODE) | defined (INFO_MODE) | defined (DEBUG_MODE)
#define LOG_ERROR(...) \
        {\
            char buffer[256];\
            sprintf(buffer, "[ERROR ProvLib] ");\
			sprintf(buffer + strlen(buffer), __VA_ARGS__);\
            Report(buffer);\
        };
#else
#define LOG_ERROR(...) \
        {\
        };
#endif

// ============================================================================
//  Provisioning logic definitions
// ============================================================================

// Provisioning logic toggles between Smart Config and AP provisioning until of
// them succeeds. The below defines the time to remain in each mode.

// Smart Config interval
#define PROVISIONING_TO_2SWITCH2_AP_MSECS   (30000) //30*1000

// AP provisioning interval
#define PROVISIONING_TO_2SWITCH2_SC_MSECS   (20000) //20*1000

// Maximal time to wait for IP address when profiles exist on startup
#define STA_STARTUP_IP_ACQUIRED_TIMEOUT_MSECS  (5000)

// Maximal time to wait for WLAN connection
#define STA_WLAN_ASSOC_TIMEOUT_MSECS        (4000)

// Maximal time to wait for IP address in STA mode
#define STA_IP_ACQUIRED_TIMEOUT_MSECS       (30000)

// Maximal time to wait for IP address in AP mode
#define AP_IP_ACQUIRED_TIMEOUT_MSECS		(100)

// ============================================================================
//  Networks list related definitions
// ============================================================================

// Maximal number of networks to detect during scan
#define MAX_NETWORKS_TO_GET				(15)

// Scan time
#define WAIT_SCAN_TIMEOUT_MSECS			(3*1000)

// Time in minutes between two consecutive served scan requests.
// A request arriving faster than the timeout below doesn't issue scan,
// instead existing results are returned.
#define SCAN_GET_NETWORKS_TIMEOUT_SECS	(30)

// This macro should be replaced with version inspection
// (for R1.0 AP scan isn't supported)
#define IS_AP_SCAN_SUPPORTED()			(0)

// ============================================================================
//  Configuration confirmation related definitions
// ============================================================================

// Time to wait in STA mode for the application to fetch provisioning result,
// after successful connection to configured AP. After thisi time the device shall
// switch to AP mode and wait for application connection.
// 15/01/2015 - TO was reduced from 25 seconds to 20 seconds.
//              This is the same as application TO. When mDNS works it is more
//              than enough. If mDNS fails increasing TO doesn't help.
#define GET_RESULT_AS_STA_AFTER_AP_MAX_MSECS    (30000)

// Time to wait in AP mode for the application to fetch provisioning result,
// after successful connection to configured AP. After this time the device shall
// switch to the mode before provisioning has started.
#define GET_RESULT_AS_AP_MAX_MSECS              (60000)

// The below is taken from SL code to define maximal SL device name
#define MAX_DEVICE_NAME_LEN	(35)

// =====================================
//  AP provisioning user defined tokens
// =====================================

// ----------------------------------------------------------------------------
// The below token definitions should be compliant to SL internal definitions

// Token name length supported by SL
#define STRING_TOKEN_SIZE           (10)

// Token information string length supported by SL
#define STRING_TOKEN_DATA_SIZE      (64)
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Get tokens

// Get one network from scan results
#define GET_FIRST_NETWORK_TOKEN     ("__SL_G_NW0")
#define GET_NEXT_NETWORK_TOKEN      ("__SL_G_NW1")

// The below is just for internal usage, x stands for result index (in hex format)
#define GET_NETWORKS_TOKEN_PREFIX   ("__SL_G_NWx")

// Get the add profile operation result
#define GET_CFG_RESULT_TOKEN        ("__SL_G_MCR")

// Get product version; used by configuring application to select configuration
// API (HTTP/REST)
#define GET_PRODUCT_VERSION_TOKEN   ("__SL_G_REV")

// Get device name. Note that the returned device name is the one read when provisioning starts.
// If name is changed manually during the provisioning sequence (for example via WEB browser)
// the library shall be out of sync.
#define GET_DEVICE_NAME_TOKEN   	("__SL_G_DEN")

/* Files to hold the tokens above */
#define SL_FILE_PARAM_PRODUCT_VERSION   "/www/param_product_version.txt"
#define SL_FILE_PARAM_CFG_RESULT        "/www/param_cfg_result.txt"
#define SL_FILE_PARAM_DEVICE_NAME       "/www/param_device_name.txt"
#define SL_FILE_NETLIST                 "/www/netlist.txt"

#define SL_SET_NETLIST_TOKENS "__SL_G_NW0__SL_G_NW1__SL_G_NW1__SL_G_NW1__SL_G_NW1__SL_G_NW1__SL_G_NW1__SL_G_NW1__SL_G_NW1__SL_G_NW1__SL_G_NW1__SL_G_NW1__SL_G_NW1__SL_G_NW1__SL_G_NW1__SL_G_NW1__SL_G_NW1__SL_G_NW1__SL_G_NW1__SL_G_NW1__SL_G_NW1__SL_G_NW1__SL_G_NW1__SL_G_NW1__SL_G_NW1__SL_G_NW1__SL_G_NW1__SL_G_NW1__SL_G_NW1__SL_G_NW1"

// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
// Post tokens

// Force immediate scan
// R1 - have to switch to STA mode, scan and return to AP mode
//      proviosioning device may lose WiFi connection
// R2 - have to check when was the last scan and when the next one is expected
//      and decide whether to scan or not. For simplicity can always respond with
//      immediate scan.
#define POST_START_SCAN_TOKEN		("__SL_P_UFS")

// The post below is used to progress the provisioning state machine.
// It can be removed if the module runs on the same processor as the HTTP server
#define POST_ADD_PROFILE_TOKEN		("__SL_P_UAN")

// ----------------------------------------------------------------------------

// =====================================
//  AP provisioning timeouts
// =====================================
#define SL_STOP_TIMEOUT_MSEC								(200)

// Time interval to wait for connection before autonomously starting AP provisioning mode
#define PROVISIONING_TO_2ENTER_AUTO_PROVISIONING_MSECS		(28*1000)

// Time interval to stay in AP provisioning mode if invoked autonomously
#define PROVISIONING_TO_2EXIT_AUTO_PROVISIONING_MSECS		(30*1000)

// =====================================
//  Provisioning logic states
// =====================================
typedef enum
{
    PROVISIONING_LOGIC_IDLE,            // provisioning non-active, initial state
    PROVISIONING_LOGIC_AP_CONFIG,       // provisioning is in AP provisioning configuration stage
    PROVISIONING_LOGIC_AP_CONFIRM,      // provisioning is in AP provisioning confirmation stage
    PROVISIONING_LOGIC_DONE,            // If auto switch is not enabled this is the final state

    // The below are valid if AUTO switch to provisioning is enabled and provisioning is idle
    // (either SL started in STA mode & profiles exist or provisioning done).

    PROVISIONING_LOGIC_WAIT_CONNECT,    // If STA is not connected yet, wait for connection
    PROVISIONING_LOGIC_WAIT_DISCONNECT, // If STA is connected, wait for disconnection before exiting (or wait forever)
    PROVISIONING_LOGIC_WAIT_STA_CONNECT // Try to connect to existing profiles, when timer expires return to provisionign mode
} provisioningLogicStates_e;

// =====================================
// Configuration module types
// =====================================
// The below denies the return codes sent to the configuration application.
// Note !!!
// Order must not be changed; it is aligned with R2.0 return codes.
typedef enum
{
    CFG_CONFIRM_RES_NOT_STARTED			= 0, // Confirmation hasn't started yet i.e. no new profiles to confirm
    CFG_CONFIRM_RES_NETWORK_NOT_FOUND	= 1, // Configured AP isn't a scan result, no confirmation for this case
    CFG_CONFIRM_RES_CONNECTION_FAILED	= 2, // Configured AP is a scan result, yet WLAN connection has failed (timeout)
    CFG_CONFIRM_RES_IP_NOT_ACQUIRED		= 3, // Failed to obtain IP address
    CFG_CONFIRM_RES_SUCCESS_AP_MODE		= 4, // STA successfully connected to AP, IP address obtained, result is taken from AP mode
    CFG_CONFIRM_RES_SUCCESS				= 5, // STA successfully connected to AP, IP address obtained, result is taken from STA mode
    CFG_CONFIRM_RES_GENERAL_ERROR       = 6  // Indicates an unknown error in the sequence, probably device error
} cfgConfirmResult_e;

typedef enum
{
    CFG_CONFIRM_STATE_IDLE,
    CFG_CONFIRM_STATE_WAIT_ASSOC,       // SL in STA mode, tries to connect to AP
    CFG_CONFIRM_STATE_WAIT_IP_ADD,      // SL in STA mode, tries to acquire IP address
    CFG_CONFIRM_STATE_GET_RESULT_STA,   // SL in STA mode, waits for "get result" token
    CFG_CONFIRM_STATE_GET_RESULT_AP     // SL in AP mode, waits for "get result" token
} cfgConfirmStates_e;

// =====================================
// Networks handler states
// =====================================
typedef enum
{
    NETS_LIST_HNDL_WAIT_IDLE,           // Initialization state
    NETS_LIST_HNDL_SCANNING,
    NETS_LIST_HNDL_PUBLISH_NETS_LIST,
    NETS_LIST_HNDL_DONE
} networksListHandling_e;

// =====================================
// AP provisioning states
// =====================================
typedef enum
{
    AP_PROVISIONING_IDLE,
    AP_PROVISIONING_WAIT_SCAN_REQUEST,
    AP_PROVISIONING_SCANNING,
    AP_PROVISIONING_PUBLISH_NETS_LIST,
    AP_PROVISIONING_DONE
} apProvisioningStates_e;

// =====================================
// Getting IP address type & state
// =====================================
typedef enum
{
    OBTAIN_IP_NOT_OBTAINED,
    OBTAIN_IP_AS_STA,
    OBTAIN_IP_AS_AP,
    OBTAIN_IP_OBTAINED
} obtainIpState_e;

// ============================================================================
//  Provisioning utilities
// ============================================================================

typedef enum
{
    // STA mode state
    SL_STATE_AP_ASSOC               = 0x0001, // SL device is associated to AP
    SL_STATE_AP_DISASSOC            = 0x0002, // SL device is dis-associated from AP
    SL_STATE_IP_ALLOC               = 0x0004, // IP address obtained
    SL_STATE_IP_LEASED              = 0x0008, // IP address leased

    // AP mode state
    SL_STATE_AP_CONNECTED           = 0X0010  // A STA is connected to the SL AP
} slState_e;

typedef enum
{
    PRO_USER_TOKEN_GET_NETWORKS     = 0X0001, // Received get scan results token
    PRO_USER_TOKEN_ADD_PROFILE      = 0X0002, // Received add profile token
    PRO_USER_TOKEN_FORCE_SCAN       = 0X0004, // Received force scan token
    PRO_USER_TOKEN_GET_RESULT       = 0X0008, // Received get provisioning result
    PRO_USER_TOKEN_UUID             = 0X0010, // Received IoT UUID
    PRO_USER_TOKEN_TERMINATE        = 0X0020  // Received terminate provisioning token
} provisioningUserTokens_e;

#endif /* AP_PROVISIONING_DEFS_H_ */
