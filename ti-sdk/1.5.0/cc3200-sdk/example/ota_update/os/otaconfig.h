//*****************************************************************************
//
// Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
//
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions
//  are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the
//    distribution.
//
//    Neither the name of Texas Instruments Incorporated nor the names of
//    its contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
//  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//*****************************************************************************
#ifndef __OTA_H__
#define __OTA_H__


#define OTA_SERVER_NAME                 "api.dropbox.com"
#define OTA_SERVER_IP_ADDRESS           0x00000000
#define OTA_SERVER_SECURED              1
#define OTA_SERVER_APP_TOKEN            "<dropbox access token>"
#define OTA_VENDOR_STRING               "Vid01_Pid00_Ver00"




#define OSI_STACK_SIZE          2048
#define GMT_DIFF_TIME_HRS       5
#define GMT_DIFF_TIME_MINS      30

#define OTA_STOPPED             0
#define OTA_INPROGRESS          1
#define OTA_DONE                2
#define OTA_NO_UPDATES          3
#define OTA_ERROR_RETRY         4
#define OTA_ERROR               5


#define TIME2013                3565987200u      /* 113 years + 28 days(leap) */
#define YEAR2013                2013
#define SEC_IN_MIN              60
#define SEC_IN_HOUR             3600
#define SEC_IN_DAY              86400

#define NET_STAT_OFF            0
#define NET_STAT_STARTED        1
#define NET_STAT_CONN           2
#define NET_STAT_CONNED         3

#define APP_VER_MAJOR           1
#define APP_VER_MINOR           0
#define APP_VER_BUILD           0

typedef struct
{
  unsigned char ucGmtDiffHr;
  unsigned char ucGmtDiffMins;
  unsigned long ulNtpServerIP;
  int           iSocket;

}tGetTime;



typedef struct
{
  char ucAppVersion[30];
  SlVersionFull sNwpVersion;
  char ucNwpVersion[50];
  char ucTimeZone[20];
  unsigned char ucServerIndex;
  unsigned char ucNetStat;
  unsigned long ulOTAErrorCount;

  union
  {
    unsigned long ulServerIP;
    unsigned char ucServerIP[4];
  };

  unsigned char ucUTCTime[30];
  unsigned char ucLocalTime[30];
  signed   int  iOTAStatus;

}tDisplayInfo;


#endif //__OTA_H__
