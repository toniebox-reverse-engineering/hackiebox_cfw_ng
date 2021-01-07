/******************************************************************************
*
*   Copyright (C) 2014 Texas Instruments Incorporated
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

#include "sl_mqtt_server.h"
#include "sl_mqtt_client.h"

#ifndef __SERVER_CLIENT_CBS_H__
#define	__SERVER_CLIENT_CBS_H__

#ifdef	__cplusplus
extern "C" {
#endif
#define MAX_CONNECTION          1
#define PUB_MSG_BUF_SIZE        2048
    
#define PUB_PUSH_BUTTON_PRESSED     1
#define MSG_RECV_BY_SERVER      	10
#define MSG_RECV_BY_CLIENT      	11
#define BROKER_DISCONNECTION		12
#define DISC_PUSH_BUTTON_PRESSED	13
    
unsigned short connect_cb(const char *clientId_str, long clientId_len,
               const char *password_str, long password_len,
               const char *username_str, long username_len, void **usr);

void recv_cb(const char *topstr, long toplen, const void *payload, long pay_len,
             bool dup, unsigned char qos, bool retain);
											
void disconnect_cb(void *usr, bool due2err);

void event_cb(void *usr, long evt, void *buf, unsigned long len);

void event_client_cb(void *app_hndl, long evt, const void *buf, unsigned long len);

void recv_client_cb(void *app_hndl, const char  *topstr, long top_len,
                    const void *payload, long pay_len,
                    bool dup, unsigned char qos, bool retain);

void disconnectbridge_cb(void *app_hndl);

struct publish_msg_header
{
    unsigned int topic_len; 
    unsigned int pay_len;
    bool retain;
    bool dup;
    unsigned char qos; 
};
 
struct client_info
{
    void *ctx;
};

struct msg_queue{
    int event;
    void *msg_ptr;
};

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif

