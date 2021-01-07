/*
 * OtaApp.h - Header OTA APP Implementation
 *
 * Copyright (C) 2014 Texas Instruments Incorporated
 *
 * All rights reserved. Property of Texas Instruments Incorporated.
 * Restricted rights to use, duplicate or disclose this code are
 * granted through contract.
 * The program may not be used without the written permission of
 * Texas Instruments Incorporated or against the terms and conditions
 * stipulated in the agreement under which this program has been supplied,
 * and under no circumstances can it be used with non-TI connectivity device.
 *
*/
#ifndef __OTA_APP_H__
#define __OTA_APP_H__

#ifdef    __cplusplus
extern "C" {
#endif

typedef enum
{
    OTA_STATE_IDLE = 0,
    OTA_STATE_CONNECT_SERVER,
    OTA_STATE_RESOURCE_LIST,
    OTA_STATE_METADATA,
    OTA_STATE_CONNECT_CDN,
    OTA_STATE_DOWNLOADING,
    OTA_STATE_WAIT_CONFIRM,
    OTA_STATE_NUMBER_OF_STATES
} OtaState_e;

typedef struct
{
    OtaState_e state;
    _i32 runMode;
    _u8 vendorStr[30];
    OtaOptServerInfo_t *pOtaServerInfo;
    OtaFileMetadata_t  *pResourceMetadata;
    _i32 resetNwp;
    _i32 resetMcu;
    _u8 isMCUAppUpdate;

    void *pvOtaClient;
    void *pvCdnClient;
    void *pvLogClient;

    /* updateCheck info */
    _i32  numUpdates;
    _u8 *file_path;
    _i32  file_size;
} OtaApp_t;

#ifdef  __cplusplus
}
#endif /* __cplusplus */

#endif /* __OTA_APP_H__ */
