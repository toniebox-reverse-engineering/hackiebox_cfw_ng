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
#include <stdio.h>
#include <stdlib.h>
#include "server_client_cbs.h"
#include "uart_if.h"
#include "osi.h"

#define APP_PRINT    Report

/*Message Queue*/
extern OsiMsgQ_t g_PBQueue;
extern char  *topic[];
struct client_info client_info_table[MAX_CONNECTION];

/* Server callbacks */
SlMqttServerCbs_t server_callbacks =
{
    connect_cb,
    recv_cb,
    disconnect_cb,
};

/* client callbacks */
SlMqttClientCbs_t client_callbacks =
{
    recv_client_cb,
    event_client_cb,
    disconnectbridge_cb
};

/**********************SERVER CALLBACKS****************************************/
//****************************************************************************
//
//! Callback in case of a local client connecting to the server
//!
//! \param clientId_str is the pointer to client ID for the incoming connection
//!    \param clientId_len is the length of the client ID string
//! \param username_str is the pointer to the username for the client
//!    \param username_len is the length of the username string
//! \param password_str is the pointer to password for the client
//! \param password_len is the length of the password string
//! \param user is the pointer to the list of client info
//
//! return 0
//
//****************************************************************************
unsigned short connect_cb(const char *clientId_str, long clientId_len,
               const char *username_str, long username_len,
               const char *password_str, long password_len, void **usr)
{
    APP_PRINT("CONNECTION\n\r");
    APP_PRINT("client ID = %s\n\r",clientId_str);
    APP_PRINT("username = %s\n\r",username_str);
    APP_PRINT("password = %s\n\r",password_str);
    *usr = &client_info_table[0];
    return 0;
}

//****************************************************************************
//
//! Callback in case the server receives a msg from local client that needs to
//! be published to the remote broker
//!
//! \param topstr is the pointer topic name for the incoming msg
//!    \param top_len is the length of the topic string
//! \param payload is the pointer to the msg payload
//!    \param pay_len is the length of the username string
//! \param dup is the flag indicating if the msg is a duplicate
//! \param qos is the qos of the incoming msg
//! \param retain flag will decide if this message has to be retained or not
//!
//! This function:
//! 1. packs the info of the incoming message (along with the payload) and
//!signals the main task about occurence of this event.
//!
//! return none
//
//****************************************************************************
void recv_cb(const char *topstr, long top_len, const void *payload, long pay_len,
             bool dup, unsigned char qos, bool retain)
{   
    unsigned int buf_size_reqd = 0;
    unsigned int topic_offset = sizeof(struct publish_msg_header);
    unsigned int payload_offset = sizeof(struct publish_msg_header)+top_len+1;
    struct publish_msg_header msg_head;
    char *pub_buff = NULL;
    struct msg_queue que_elem;
    
    buf_size_reqd += sizeof(struct publish_msg_header);
    buf_size_reqd += top_len +1;
    buf_size_reqd += pay_len +1;
    pub_buff = (char *)malloc(buf_size_reqd);
    
    if(pub_buff == NULL)
    {
        APP_PRINT("malloc failed: recv_cb\n\r");
        while(1);
    }
    msg_head.topic_len = top_len;
    msg_head.pay_len = pay_len;
    msg_head.retain = retain;
    msg_head.dup = dup;
    msg_head.qos = qos;
    memcpy((void*)pub_buff, &msg_head, sizeof(struct publish_msg_header));
    
    /* copying the topic name into the buffer */
    memcpy((void*)(pub_buff+topic_offset), (const void*)topstr, top_len);
    memset((void*)(pub_buff+topic_offset+top_len), '\0', 1);
    
    /* copying the payload into the buffer */
    memcpy((void*)(pub_buff+payload_offset), (const void*)payload, pay_len);
    memset((void*)(pub_buff+payload_offset+pay_len), '\0', 1);
    
    APP_PRINT("\n\rMsg Recvd. by server\n\r");
    /* printing the topic name */
    APP_PRINT("TOPIC: %s\n\r", pub_buff+topic_offset);
    /* printing the payload */
    APP_PRINT("PAYLOAD: %s\n\r", pub_buff+payload_offset);
    
    /* printing other info */
    APP_PRINT("QOS: %d\n\r",qos);
    if(retain)
        APP_PRINT("Retained\n\r");
    if(dup)
        APP_PRINT("Duplicate\n\r");
    
    /* filling the que element details */
    que_elem.event = MSG_RECV_BY_SERVER;
    que_elem.msg_ptr = pub_buff;
    
    /* signal to the main task */
    osi_MsgQWrite(&g_PBQueue,&que_elem,OSI_WAIT_FOREVER);
    return;
}

//****************************************************************************
//
//! Callback in case of a local client disconnecting from the server
//!
//! \param user is the pointer to the list of client info
//! \param due2err indicates if the disconnection has happened due to error
//
//! return none
//
//****************************************************************************
void disconnect_cb(void *usr, bool due2err)
{
    struct msg_queue que_elem;
    APP_PRINT("DISCONNECTION\n\r");

    /* filling the que element details */
    que_elem.event = BROKER_DISCONNECTION;
    que_elem.msg_ptr = NULL;

    /* signal to the main task */
    osi_MsgQWrite(&g_PBQueue,&que_elem,OSI_WAIT_FOREVER);

    return;
}

/**********************CLIENT CALLBACKS****************************************/
//****************************************************************************
//
//! Callback in case the client receives a msg from the remote broker
//!
//! \param app_hndl is the ctx handl for the connection
//! \param topstr is a pointer to the topic string
//!    \param top_len is the length of the topic string
//! \param payload is the pointer to the msg payload
//!    \param pay_len is the length of the username string
//! \param dup is the flag indicating if the msg is a duplicate
//! \param qos is the qos of the incoming msg
//! \param retain flag will decide if this message has to be retained or not
//!
//! This function:
//! 1. packs the info of the incoming message from the remote broker (along with
//! the payload) and signals the main task about occurence of this event.
//!
//! return none
//
//****************************************************************************
void recv_client_cb(void *app_hndl, const char  *topstr, long top_len,
                    const void *payload, long pay_len,
                    bool dup, unsigned char qos, bool retain)

{
    unsigned int buf_size_reqd = 0;
    unsigned int topic_offset = sizeof(struct publish_msg_header);
    unsigned int payload_offset = sizeof(struct publish_msg_header)+top_len+1;
    struct publish_msg_header msg_head;
    char *pub_buff = NULL;
    struct msg_queue que_elem;
    
    buf_size_reqd += sizeof(struct publish_msg_header);
    buf_size_reqd += top_len +1;
    buf_size_reqd += pay_len +1;
    pub_buff = (char *)malloc(buf_size_reqd);
    
    if(pub_buff == NULL)
    {
        APP_PRINT("malloc failed: recv_cb\n\r");
        while(1);
    }
    msg_head.topic_len = top_len;
    msg_head.pay_len = pay_len;
    msg_head.retain = retain;
    msg_head.dup = dup;
    msg_head.qos = qos;
    memcpy((void*)pub_buff, &msg_head, sizeof(struct publish_msg_header));
    
    /* copying the topic name into the buffer */
    memcpy((void*)(pub_buff+topic_offset), (const void*)topstr, top_len);
    memset((void*)(pub_buff+topic_offset+top_len), '\0', 1);
    
    /* copying the payload into the buffer */
    memcpy((void*)(pub_buff+payload_offset), (const void*)payload, pay_len);
    memset((void*)(pub_buff+payload_offset+pay_len), '\0', 1);
    
    APP_PRINT("\n\rMsg Recvd. by client\n\r");
    /* printing the topic name */
    APP_PRINT("TOPIC: %s\n\r", pub_buff+topic_offset);
    /* printing the payload */
    APP_PRINT("PAYLOAD: %s\n\r", pub_buff+payload_offset);
    
    /* printing other info */
    APP_PRINT("QOS: %d\n\r",qos);
    if(retain)
        APP_PRINT("Retained\n\r");
    if(dup)
        APP_PRINT("Duplicate\n\r");
    
    /* filling the que element details */
    que_elem.event = MSG_RECV_BY_CLIENT;
    que_elem.msg_ptr = pub_buff;
    
    /* signal to the main task */
    osi_MsgQWrite(&g_PBQueue,&que_elem,OSI_WAIT_FOREVER);
    return;
}

//****************************************************************************
//
//! Callback in case of various event (for clients connection with remote broker)
//!
//! \param app_hndl is the ctx handl for the connection
//! \param evt is a event occured
//! \param buff is the pointer to the buffer for data (for this event)
//! \param len is the length of the buffer data
//!
//! return none
//
//****************************************************************************
void event_client_cb(void *app_hndl, long evt, const void *buf, unsigned long len)
{
    int i = 0;
    switch(evt)
    {
      case SL_MQTT_CL_EVT_PUBACK:
        APP_PRINT("PubAck:\n\r");
        APP_PRINT("%s\n\r",buf);
        break;
    
      case SL_MQTT_CL_EVT_SUBACK:
        APP_PRINT("Sub Ack:\n\r");
        APP_PRINT("Granted QoS Levels are:\n\r");
        for(i=0;i<len;i++)
        {
          APP_PRINT("%s :QoS %d\n\r",topic[i],((unsigned char*)buf)[i]);
          
        }
        break;
        
      case SL_MQTT_CL_EVT_UNSUBACK:
        APP_PRINT("UnSub Ack \n\r");
        APP_PRINT("%s\n\r",buf);
        break;
    
      default:
        break;
  
    }

}

//****************************************************************************
//
//! Callback in case the client disconnects from the remote broker
//!
//! \param app_hndl is the ctx handl for the connection
//!
//! return none
//
//****************************************************************************
void disconnectbridge_cb(void *app_hndl)
{
    APP_PRINT("BRIDGE DISCONNECTION\n\r");
    return;
}

