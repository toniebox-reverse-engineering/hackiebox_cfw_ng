/*
 * OtaCommon.h - Common header file for OTA library
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
#ifndef __OTA_COMMON_H__
#define __OTA_COMMON_H__

#ifdef    __cplusplus
extern "C" {
#endif

/* #define TI_OTA_SERVER */
#define OTA_DROPBOX_V2



#define OTA_SERVER_REST_HDR             "Authorization: Bearer "
#ifdef OTA_DROPBOX_V2
/* Dropbox V2 requests */
#define OTA_SERVER_REST_REQ_DIR         "/2/files/list_folder"            /* returns files/folder list */
#define OTA_SERVER_REST_REQ_FILE_URL    "/2/files/get_temporary_link"     /* returns A url that serves the media directly */
#define OTA_SERVER_REST_FILES_PUT       "/1/files_put/auto/"
#else // DROPBOX V1 or TI_OTA_SERVER
#define OTA_SERVER_REST_UPDATE_CHK      "/1/metadata/auto/" // returns files/folder list
#define OTA_SERVER_REST_RSRC_METADATA   "/1/media/auto"     // returns A url that serves the media directly
#define OTA_SERVER_REST_FILES_PUT       "/1/files_put/auto/"
#endif


/* General return values */
#define OTA_STATUS_OK        0
#define OTA_STATUS_ERROR    -1
#define OTA_STATUS_ERROR_CONTINUOUS_ACCESS    -1000

#define MEDIA_URL_BUF_LEN          256

#include "extlibs_common.h"

#ifdef  __cplusplus
}
#endif /* __cplusplus */

#endif /* __OTA_COMMON_H__ */
