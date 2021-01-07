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

#ifndef DEMO_CONFIG_H
#define DEMO_CONFIG_H

//#include "config_defs.h"

//
// Modify the following settings as necessary to run the demo
//

#define IP_ALLOC_METHOD USE_DHCP


///////////////////////////////////////////////////////////////////////////////
//All SMTP defines
#define SMTP_BUF_LEN 		100
#define GMAIL_HOST_NAME         "smtp.gmail.com"
#define GMAIL_HOST_PORT         465
#define USER                    "<username@gmail.com>" //Set Sender/Source Email Address
#define USER_RFC                "<username@gmail.com>" //Set Sender/Source Email Address
#define PASS                    "<password>"              //Set Sender/Source Email Password
#define RCPT_RFC                "<username@gmail.com>"  //Set Recipient Email Password
#define EMAIL_SUB               "Hello from Simplelink!"
///////////////////////////////////////////////////////////////////////////////

#endif
