//*****************************************************************************
// Copyright (C) 2014 Texas Instruments Incorporated
//
// All rights reserved. Property of Texas Instruments Incorporated.
// Restricted rights to use, duplicate or disclose this code are
// granted through contract.
// The program may not be used without the written permission of
// Texas Instruments Incorporated or against the terms and conditions
// stipulated in the agreement under which this program has been supplied,
// and under no circumstances can it be used with non-TI connectivity device.
//
//*****************************************************************************


#include <stdio.h>

//simplink include 
#include "simplelink.h"

//common interface include
#include "common.h"
#ifndef NOTERM
#include "uart_if.h"
#endif

#define WLAN_DEL_ALL_PROFILES        0xff



//*****************************************************************************
//
//!   \brief Connecting to a WLAN Accesspoint using SmartConfig provisioning
//!
//!    This function enables SmartConfig provisioning for adding a new 
//!    connection profile to CC3200. Since we have set the connection policy 
//!    to Auto, once SmartConfig is complete,CC3200 will connect 
//!    automatically to the new connection profile added by smartConfig.
//!
//!   \param[in]               None
//!
//! \return  0 - Success
//!            -1 - Failure
//!
//!   \warning           If the WLAN connection fails or we don't acquire an 
//!                         IP address,We will be stuck in this function forever.
//!
//*****************************************************************************
long SmartConfigConnect()
{
    unsigned char policyVal;
    long lRetVal = -1;

    //
    // Clear all profiles 
    // This is of course not a must, it is used in this example to make sure
    // we will connect to the new profile added by SmartConfig
    //
    lRetVal = sl_WlanProfileDel(WLAN_DEL_ALL_PROFILES);
    ASSERT_ON_ERROR(lRetVal);

    //set AUTO policy
    lRetVal = sl_WlanPolicySet(  SL_POLICY_CONNECTION,
                      SL_CONNECTION_POLICY(1,0,0,0,0),
                      &policyVal,
                      1 /*PolicyValLen*/);
    ASSERT_ON_ERROR(lRetVal);
        
    //
    // Start SmartConfig
    // This example uses the unsecured SmartConfig method
    //
    lRetVal = sl_WlanSmartConfigStart(0,                /*groupIdBitmask*/
                           SMART_CONFIG_CIPHER_NONE,    /*cipher*/
                           0,                           /*publicKeyLen*/
                           0,                           /*group1KeyLen*/
                           0,                           /*group2KeyLen */
                           NULL,                        /*publicKey */
                           NULL,                        /*group1Key */
                           NULL);                       /*group2Key*/  
                        
    ASSERT_ON_ERROR(lRetVal);
        
    return SUCCESS;

}

//*****************************************************************************
//
//! 
//!    \brief Stop SmartConfig provisioning
//!
//!    This function Stops SmartConfig provisioning 
//!
//!    \param[in]                   None
//!
//! \return  0 - Success
//!            -1 - Failure
//!
//!   \note
//!
//*****************************************************************************
long SmartConfigStop()
{
    long lRetVal = -1;
    
    lRetVal = sl_WlanSmartConfigStop();
    ASSERT_ON_ERROR(lRetVal);

    return SUCCESS;
}
