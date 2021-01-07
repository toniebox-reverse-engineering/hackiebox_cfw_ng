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
#include "mqtt_common.h"
#include "sl_mqtt_server.h"
#include "server_core.h"
#include "client_mgmt.h"
#include "server_util.h"
#include "cc32xx_sl_net.h"
#include "osi.h"


/*-------------------------------------------------------------------------
 * MQTT Routines
 *-------------------------------------------------------------------------
 */
OsiLockObj_t MutexLockObj;

// wrapper function
static void mutex_lock(void* mutex_hndl)
{
    osi_LockObjLock(mutex_hndl, OSI_WAIT_FOREVER);
}

// wrapper function
static void mutex_unlock(void* mutex_hndl)
{
    osi_LockObjUnlock(mutex_hndl);
}

struct mqtt_server_lib_cfg server_cfg = {
   0,
   0,
   &MutexLockObj,
   mutex_lock,
   mutex_unlock,
   NULL,             /*Debug print*/
   true
};

struct mqtt_server_app_cfg app_config = {
   NULL
};


/*Task Priority and Response Time*/
_u32 g_srvr_wait_secs,g_srvr_task_priority;
struct device_net_services net_ops = {comm_open,tcp_send,tcp_recv,
                                send_dest,recv_from,comm_close,
                                tcp_listen,tcp_accept,tcp_select,rtc_secs};
static void VMqttServerRunTask(void *pvParams);

SlMqttServerCbs_t cbs_obj,*cbs_ptr=NULL;
struct ctrl_struct{
        _const void* plugin_hndl;
}app_hndl;


u16 sl_server_connect_cb (_const struct utf8_string *clientId,
                _const struct utf8_string *username,
                _const struct utf8_string *password,
                void **app_usr)
{
        return( cbs_obj.sl_ExtLib_MqttConn((_const char*)MQ_CONN_UTF8_BUF(clientId),
                    MQ_CONN_UTF8_LEN(clientId), (_const char*)MQ_CONN_UTF8_BUF(username),
                    MQ_CONN_UTF8_LEN(username), (_const char*)MQ_CONN_UTF8_BUF(password),
                    MQ_CONN_UTF8_LEN(password), app_usr));
}

void sl_server_publish_cb(_const struct utf8_string *topic,
                          _const u8 *payload, u32 pay_len,
                          bool dup, u8 qos, bool retain)
{
        cbs_obj.sl_ExtLib_MqttRecv((_const char*)MQ_CONN_UTF8_BUF(topic),
               MQ_CONN_UTF8_LEN(topic), payload, pay_len, dup, qos, retain);
}

void sl_server_disconn_cb(_const void *app_usr, bool due2err)
{
        cbs_obj.sl_ExtLib_MqttDisconn((void*)app_usr, due2err);
}

_i32 sl_ExtLib_MqttServerSend(_const char *topic, _const void *data, _i32 len,
                               _u8 qos, bool retain, _u32 flags)
{
        struct utf8_string topic_utf8={NULL};
        topic_utf8.buffer = (char*)topic;
        topic_utf8.length = strlen(topic);
        return( mqtt_server_app_pub_send(&topic_utf8,data, len,
                                         (enum mqtt_qos)qos, retain));
}

_i32 sl_ExtLib_MqttTopicEnroll(_const char *topic)
{
        struct utf8_string topic_utf8={NULL};
        topic_utf8.buffer = (char*)topic;
        topic_utf8.length = strlen(topic);
        return( mqtt_server_topic_enroll((void*)(app_hndl.plugin_hndl),
                                         &topic_utf8,MQTT_QOS2));

}

_i32 sl_ExtLib_MqttTopicDisenroll(_const char *topic)
{
        struct utf8_string topic_utf8={NULL};
        topic_utf8.buffer = (char*)topic;
        topic_utf8.length = strlen(topic);
       return( mqtt_server_topic_disenroll((void*)&(app_hndl.plugin_hndl),
                                           &topic_utf8));
}

struct mqtt_server_app_cbs server_appcallbacks =
{
        sl_server_connect_cb,
        sl_server_publish_cb,
        sl_server_disconn_cb
};

static volatile enum _e_SlMqttServerState {

	ePaused     = 1,
	eGoToPaused,
	eActive,
	eGoToActive,
	eErrors

} serverState = ePaused;

static volatile bool initFn_awaits = true; /* Flag used by init function */

static _SlSyncObj_t serverSem, *pSemObj;
static _SlSyncObj_t rxTaskSem, *pSemTsk;

i32 sl_ExtLib_MqttServerInit(_const SlMqttServerCfg_t  *cfg,
                             _const SlMqttServerCbs_t  *app_cbs)
{
    i32 ret;
    
    //valid loopback port has to be specified for correct operations
    if(cfg->loopback_port == 0)
    {
        return -1;
    }
    server_cfg.listener_port=cfg->server_info.port_number;
    server_cfg.debug_printf = cfg->dbg_print;
    server_cfg.loopback_port = cfg->loopback_port;
    cbs_ptr=&cbs_obj;
    memcpy(cbs_ptr,app_cbs,sizeof(SlMqttServerCbs_t ));
   
    g_srvr_task_priority=(_u32)cfg->rx_tsk_priority;
    g_srvr_wait_secs=(_u32)cfg->resp_time;
    ret = osi_LockObjCreate(&MutexLockObj);
    if(!ret)
    {
        ret=mqtt_server_init(&server_cfg, &app_config);
    }
    else
    { 
        ret = osi_LockObjDelete(&MutexLockObj);
        return(-1);
    }
    /* registering the device specific implementations for net operations */
    mqtt_server_register_net_svc(&net_ops);
    
    /* registering the apps callbacks */
    app_hndl.plugin_hndl = mqtt_server_app_register(&server_appcallbacks, "temp");

    initFn_awaits = true;

    pSemObj = &serverSem;
    sl_SyncObjCreate(pSemObj, "SlMqttServerInitObj");
    sl_SyncObjWait(pSemObj, SL_OS_NO_WAIT); /* Make Sem = 0 */

    pSemTsk = &rxTaskSem;      /* Semaphore for RX LIB Task */
    sl_SyncObjCreate(pSemTsk, "SlMqttServerTaskSem");
    sl_SyncObjWait(pSemTsk, SL_OS_NO_WAIT); /* Make Sem = 0 */

    /* start the Server Run task */
    osi_TaskCreate( VMqttServerRunTask, (_const signed char*) "MQTTServerRun",
                    2048, NULL, g_srvr_task_priority, NULL );

    /* Give a chance to the newly created task to run - let's do short wait */
    osi_SyncObjWait(pSemObj, SL_OS_WAIT_FOREVER);

    return ret;
}

static inline void _sl_MqttServerStateSet(enum _e_SlMqttServerState newState)
{
    unsigned long flags = osi_EnterCritical();
    serverState = newState;
    osi_ExitCritical(flags);
}

static inline enum _e_SlMqttServerState _sl_MqttServerStateGet()
{
    enum _e_SlMqttServerState state;

    unsigned long flags = osi_EnterCritical();
    state = serverState;
    osi_ExitCritical(flags);

    return state;
}

static void VMqttServerRunTask(void *pvParams)
{
    while(ePaused == _sl_MqttServerStateGet()) {
        if(true == initFn_awaits) {
            initFn_awaits = false;
            osi_SyncObjSignal(pSemObj); /* Declare: I am all set to run */
        }

        osi_SyncObjWait(pSemTsk, SL_OS_WAIT_FOREVER); /* Park task here */
        _sl_MqttServerStateSet(eActive);    /* Parking over - get going */
        if(0 != mqtt_server_run(g_srvr_wait_secs))
            _sl_MqttServerStateSet(eErrors);
        else
            _sl_MqttServerStateSet(ePaused);
    }

    osi_SyncObjWait(pSemTsk, SL_OS_WAIT_FOREVER);
    return;
}

_i32 sl_ExtLib_MqttServerActivate(void)
{
    _i32 rv = 0;

    unsigned long flags = osi_EnterCritical();

    if(ePaused == serverState) {
        serverState = eGoToActive;
		osi_SyncObjSignal(pSemTsk);    /* Activate: Get the task to run */
    } else
        rv = -1;

    osi_ExitCritical(flags);

    if(0 == rv) {
                osi_Sleep(20);
        rv = (eActive == _sl_MqttServerStateGet()) ? 0 : -1;
    }

    return rv;
}


_i32 sl_ExtLib_MqttServerPause(void)
{
    _i32 rv = 0;

    unsigned long flags = osi_EnterCritical();

    if(eActive == serverState)
        serverState = eGoToPaused;
    else
        rv = -1;

    osi_ExitCritical(flags);

    if(0 == rv) {
        enum _e_SlMqttServerState state;

        mqtt_server_pause_locked();

        while(eGoToPaused == (state = _sl_MqttServerStateGet()))
            osi_Sleep(10);

        rv = (ePaused == state) ? 0 : -1;
    }

    return rv;
}

