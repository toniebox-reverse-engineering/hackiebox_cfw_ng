/******************************************************************************
*
*   Copyright (C) 2013 Texas Instruments Incorporated
*
*   All rights reserved. Property of Texas Instruments Incorporated.
*   Restricted rights to use, duplicate or disclose this code are
*   granted through contract.
*
*   The program may not be used without the written permission of
*   Texas Instruments Incorporated or against the terms and conditions
*   stipulated in the agreement under which this program has been supplied,
*   and under no circumstances can it be used with non-TI connectivity device.
*
******************************************************************************/
/**
 * @defgroup Httpserverapp
 *
 * @{
 */

#ifndef __HTTP_APP_H__
#define __HTTP_APP_H__

#define DEVICE_NAME                   "httpserver"
#define TI_NAME                       "cc3200_"
#define mDNS_SERV                     "._device-info._tcp.local"
#define TTL_MDNS_SERV                 4500

#define MAX_DEV_NAME_SIZE                       23

typedef struct
{
  unsigned char DevNameLen;
  unsigned char DevName[MAX_DEV_NAME_SIZE];
}t_DevName;

typedef struct
{
	char    mDNSServNameUnReg[64];
	unsigned char   mDNSServNameUnRegLen;

}t_mDNSService;

extern void HttpServerAppTask(void *);
extern void CounterAppTask(void *);


#endif   //__HTTP_APP_H__

