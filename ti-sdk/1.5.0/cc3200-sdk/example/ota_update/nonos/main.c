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


//*****************************************************************************
//
// Application Name     -
// Application Overview -
//
// Application Details  -
//
//*****************************************************************************

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// simplelink includes
#include "simplelink.h"
#include "wlan.h"
#include "nonos.h"

// driverlib includes
#include "hw_ints.h"
#include "hw_types.h"
#include "hw_memmap.h"
#include "rom.h"
#include "rom_map.h"
#include "interrupt.h"
#include "prcm.h"
#include "utils.h"
#include "uart.h"

// common interface includes
#include "udma_if.h"
#include "common.h"
#include "uart_if.h"
#include "button_if.h"
#include "net.h"

#include "pinmux.h"
#include "flc_api.h"
#include "flc.h"
#include "ota_api.h"
#include "otaconfig.h"
#include "time.h"
#include "task.h"

//*****************************************************************************
// System state Macros
//*****************************************************************************
#define SYS_STATE_WAIT          0
#define SYS_STATE_RUN           1
#define SYS_STATE_REBOOT        2
#define SYS_STATE_TEST_REBOOT   3

//*****************************************************************************
// Number of servers in NTP server list
//*****************************************************************************
#define NOF_NTP_SERVER         (sizeof(g_acSNTPserver)/30)

//*****************************************************************************
// Get time task state Macros
//*****************************************************************************
#define GET_TIME_TASK_STATE_INIT        0
#define GET_TIME_TASK_STATE_CONN        1
#define GET_TIME_TASK_STATE_OPEN_SOCK   2
#define GET_TIME_TASK_STATE_GET_IP      3
#define GET_TIME_TASK_STATE_SET_SERVER  4
#define GET_TIME_TASK_STATE_GET_TIME    5
#define GET_TIME_TASK_STATE_DONE        6

//*****************************************************************************
// OTA Update task state Macros
//*****************************************************************************
#define OTA_UPDATE_STATE_WAIT_START     0
#define OTA_UPDATE_STATE_WAIT_BTN       1
#define OTA_UPDATE_STATE_INIT           2
#define OTA_UPDATE_STATE_RUN            3
#define OTA_UPDATE_STATE_DONE           4

//*****************************************************************************
// Factory Resettask state Macros
//*****************************************************************************
#define FACTORY_RST_STATE_WAIT_BTN      0
#define FACTORY_RST_STATE_RUN           1

//*****************************************************************************
// Extern functions and symbols
//*****************************************************************************
#if defined(ccs) || defined(gcc)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif

//*****************************************************************************
// Function Prototypes
//*****************************************************************************
void DisplayRefresh(void);

//*****************************************************************************
// Global static variables
//*****************************************************************************
static tSyncObj g_NetStatSyncObj;
static tSyncObj g_OTAStatSyncObj;
static tSyncObj g_DispRefreshSyncObj;
static tSyncObj g_FactResetSyncObj;
static unsigned long g_ulSysState;
static OtaOptServerInfo_t g_otaOptServerInfo;
static void *pvOtaApp;
static tDisplayInfo sDisplayInfo;


//*****************************************************************************
// NTP Server List
//*****************************************************************************
//! ######################### list of SNTP servers ############################
//! ##
//! ##          hostname             |        IP       |       location
//! ## -----------------------------------------------------------------------------
//! ##   2.in.pool.ntp.org           | 113.30.137.34   |
//! ##   dmz0.la-archdiocese.net     | 209.151.225.100 |   Los Angeles, CA
//! ##   ntp.inode.at                | 195.58.160.5    |   Vienna
//! ##   ntp3.proserve.nl            | 212.204.198.85  |   Amsterdam
//! ##   ntp.spadhausen.com          | 109.168.118.249 |   Milano - Italy
//! ##   Optimussupreme.64bitVPS.com | 216.128.88.62   |   Brooklyn, New York
//! ##   ntp.mazzanet.id.au          | 203.206.205.83  |   Regional Victoria, Australia
//! ##   a.ntp.br                    | 200.160.0.8     |   Sao Paulo, Brazil
//! ###################################################################
const char g_acSNTPserver[][30] =
{
  "dmz0.la-archdiocese.net",
  "2.in.pool.ntp.org",
  "ntp.inode.at",
  "ntp3.proserve.nl",
  "ntp.spadhausen.com",
  "Optimussupreme.64bitVPS.com",
  "ntp.mazzanet.id.au",
  "a.ntp.br"
};


//*****************************************************************************
// Tuesday is the 1st day in 2013 - the relative year
//*****************************************************************************
const char g_acDaysOfWeek2013[7][3] = {{"Tue"},
                                    {"Wed"},
                                    {"Thu"},
                                    {"Fri"},
                                    {"Sat"},
                                    {"Sun"},
                                    {"Mon"}};

//*****************************************************************************
// Month string list
//*****************************************************************************
const char g_acMonthOfYear[12][3] = {{"Jan"},
                                  {"Feb"},
                                  {"Mar"},
                                  {"Apr"},
                                  {"May"},
                                  {"Jun"},
                                  {"Jul"},
                                  {"Aug"},
                                  {"Sep"},
                                  {"Oct"},
                                  {"Nov"},
                                  {"Dec"}};

//*****************************************************************************
// Days per month
//*****************************************************************************
const char g_acNumOfDaysPerMonth[12] = {31, 28, 31, 30, 31, 30,
                                        31, 31, 30, 31, 30, 31};

//*****************************************************************************
//
//! Application startup display on UART
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
static void
DisplayBanner(char * AppName)
{

    UART_PRINT("\n\n\n\r");
    UART_PRINT("\t\t *************************************************\n\r");
    UART_PRINT("\t\t         CC3200 %s Application       \n\r", AppName);
    UART_PRINT("\t\t *************************************************\n\r");
    UART_PRINT("\n\n\n\r");
}


#ifdef USE_FREERTOS
//*****************************************************************************
//
//! \brief Application defined idle task hook
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
void
vApplicationIdleHook( void)
{
    //Handle Idle Hook for Profiling, Power Management etc
}

//*****************************************************************************
//
//! \brief Application defined malloc failed hook
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
void vApplicationMallocFailedHook()
{
    //Handle Memory Allocation Errors
    while(1)
    {
    }
}

//*****************************************************************************
//
//! \brief Application defined hook (or callback) function - assert
//!
//! \param[in]  pcFile - Pointer to the File Name
//! \param[in]  ulLine - Line Number
//!
//! \return none
//!
//*****************************************************************************
void
vAssertCalled( const char *pcFile, unsigned long ulLine )
{
    //Handle Assert here
    while(1)
    {
    }
}

//*****************************************************************************
//
//! \brief Application defined stack overflow hook
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
void vApplicationStackOverflowHook( OsiTaskHandle *pxTask,
                                   signed char *pcTaskName)
{
    //Handle FreeRTOS Stack Overflow
    while(1)
    {
    }
}
#endif //USE_FREERTOS


//*****************************************************************************
//
//! Board Initialization & Configuration
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************
static void
BoardInit(void)
{
/* In case of TI-RTOS vector table is initialize by OS itself */
#ifndef USE_TIRTOS

    //
    // Set vector table base
    //
#if defined(ccs) || defined(gcc)
    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
#endif
#if defined(ewarm)
    MAP_IntVTableBaseSet((unsigned long)&__vector_table);
#endif
#endif

    //
    // Enable Processor
    //
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}

//*****************************************************************************
//
//! Convert time in seconds to a string of date and time
//!
//! \param ulTime is time in seconds
//! \param ulStrTime is pointer to an array of characters
//!
//! \return None
//
//*****************************************************************************
static void TimeToString(unsigned long ulTime, unsigned char *ulStrTime)
{

    short isGeneralVar;
    unsigned long ulGeneralVar;
    unsigned long ulGeneralVar1;
    unsigned char iIndex;

    //
    // seconds are relative to 0h on 1 January 1900
    //
    ulTime -= TIME2013;

    //
    // day, number of days since beginning of 2013
    //
    isGeneralVar = ulTime/SEC_IN_DAY;
    memcpy(ulStrTime, g_acDaysOfWeek2013[isGeneralVar%7], 3);
    ulStrTime += 3;
    *ulStrTime++ = '\x20';

    //
    // month
    //
    isGeneralVar %= 365;
    for (iIndex = 0; iIndex < 12; iIndex++)
    {
        isGeneralVar -= g_acNumOfDaysPerMonth[iIndex];
        if (isGeneralVar < 0)
                break;
    }

    if(iIndex == 12)
    {
        iIndex = 0;
    }

    memcpy(ulStrTime, g_acMonthOfYear[iIndex], 3);
    ulStrTime += 3;
    *ulStrTime++ = '\x20';

    //
    // date
    // restore the day in current month
    //
    isGeneralVar += g_acNumOfDaysPerMonth[iIndex];
    ulStrTime += sprintf((char *)ulStrTime,"%02d ",isGeneralVar + 1);

    //
    // year
    // number of days since beginning of 2013
    //
    ulGeneralVar = ulTime/SEC_IN_DAY;
    ulGeneralVar /= 365;

    ulStrTime += sprintf((char *)ulStrTime,"%4d ",YEAR2013 + ulGeneralVar);

    //
    // time
    //
    ulGeneralVar = ulTime%SEC_IN_DAY;

    // number of seconds per hour
    ulGeneralVar1 = ulGeneralVar%SEC_IN_HOUR;

    // number of hours
    ulGeneralVar /= SEC_IN_HOUR;
    ulStrTime += sprintf((char *)ulStrTime,"%02d:",ulGeneralVar);


    // number of minutes per hour
    ulGeneralVar = ulGeneralVar1/SEC_IN_MIN;

    // number of seconds per minute
    ulGeneralVar1 %= SEC_IN_MIN;
    ulStrTime += sprintf((char *)ulStrTime,"%02d:",ulGeneralVar);

    sprintf((char *)ulStrTime,"%02d",ulGeneralVar1);

}

//*****************************************************************************
//
//! Gets the current time from the selected SNTP server
//!
//! \brief  This function obtains the NTP time from the server.
//!
//! \param  pGetTime is pointer to Get time structure
//!
//! \return 0 : success, -ve : failure
//!
//
//*****************************************************************************
long GetSNTPTime(tGetTime *pGetTime)
{

/*
                            NTP Packet Header:


       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9  0  1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |LI | VN  |Mode |    Stratum    |     Poll      |   Precision    |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                          Root  Delay                           |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                       Root  Dispersion                         |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                     Reference Identifier                       |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                                                                |
      |                    Reference Timestamp (64)                    |
      |                                                                |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                                                                |
      |                    Originate Timestamp (64)                    |
      |                                                                |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                                                                |
      |                     Receive Timestamp (64)                     |
      |                                                                |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                                                                |
      |                     Transmit Timestamp (64)                    |
      |                                                                |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                 Key Identifier (optional) (32)                 |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                                                                |
      |                                                                |
      |                 Message Digest (optional) (128)                |
      |                                                                |
      |                                                                |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

*/
    char cDataBuf[48];
    long lRetVal = 0;
    SlSockAddr_t sAddr;
    unsigned long ulElapsedSec;

    //
    // Send a query ? to the NTP server to get the NTP time
    //
    memset(cDataBuf, 0, sizeof(cDataBuf));
    cDataBuf[0] = '\x1b';

    sAddr.sa_family = AF_INET;

    // the source port
    sAddr.sa_data[0] = 0x00;
    sAddr.sa_data[1] = 0x7B;    // UDP port number for NTP is 123
    sAddr.sa_data[2] = (char)((pGetTime->ulNtpServerIP >>24)&0xff);
    sAddr.sa_data[3] = (char)((pGetTime->ulNtpServerIP >>16)&0xff);
    sAddr.sa_data[4] = (char)((pGetTime->ulNtpServerIP >>8)&0xff);
    sAddr.sa_data[5] = (char)(pGetTime->ulNtpServerIP&0xff);

    lRetVal = sl_SendTo(pGetTime->iSocket,
                     cDataBuf,
                     sizeof(cDataBuf), 0,
                     &sAddr, sizeof(sAddr));

    if (lRetVal != sizeof(cDataBuf))
    {
      return FAILURE;
    }

    lRetVal = sl_Recv(pGetTime->iSocket,
                       cDataBuf, sizeof(cDataBuf), 0);

    //
    // Confirm that the MODE is 4 --> server
    //
    if ((cDataBuf[0] & 0x7) != 4)    // expect only server response
    {
         return FAILURE;
    }
    else
    {
        //
        // Getting the data from the Transmit Timestamp (seconds) field
        // This is the time at which the reply departed the
        // server for the client
        //
        ulElapsedSec = cDataBuf[40];
        ulElapsedSec <<= 8;
        ulElapsedSec += cDataBuf[41];
        ulElapsedSec <<= 8;
        ulElapsedSec += cDataBuf[42];
        ulElapsedSec <<= 8;
        ulElapsedSec += cDataBuf[43];

        //
        // Compute the UTC time
        //
        TimeToString(ulElapsedSec, sDisplayInfo.ucUTCTime);

        //
        // Set the time zone
        //
        ulElapsedSec += (pGetTime->ucGmtDiffHr * SEC_IN_HOUR);
        ulElapsedSec += (pGetTime->ucGmtDiffMins * SEC_IN_MIN);

        //
        // Compute the local time
        //
        TimeToString(ulElapsedSec, sDisplayInfo.ucLocalTime);
    }

    return SUCCESS;
}

//****************************************************************************
//
//! Reboot the MCU by requesting hibernate for a short duration
//!
//! \return None
//
//****************************************************************************
static void RebootMCU()
{

  //
  // Configure hibernate RTC wakeup
  //
  PRCMHibernateWakeupSourceEnable(PRCM_HIB_SLOW_CLK_CTR);

  //
  // Delay loop
  //
  MAP_UtilsDelay(8000000);

  //
  // Set wake up time
  //
  PRCMHibernateIntervalSet(330);

  //
  // Request hibernate
  //
  PRCMHibernateEnter();

  //
  // Control should never reach here
  //
  while(1)
  {

  }
}


//****************************************************************************
//
//! Task function implementing the gettime functionality using an NTP server
//!
//! \param none
//!
//! This function
//!    1. Initializes the required peripherals
//!    2. Initializes network driver and connects to the default AP
//!    3. Creates a UDP socket, gets the NTP server IP address using DNS
//!    4. Periodically gets the NTP time and displays the time
//!
//! \return Returns \b TASK_RET_IN_PROG.
//
//****************************************************************************
int GetNTPTimeTask(void *pvParameters)
{
    SlSecParams_t SecurityParams;
    long lRetVal = -1;
    struct SlTimeval_t timeVal;
    static char cTaskOwnState = GET_TIME_TASK_STATE_INIT;
    static tGetTime sGetTime;
    long OptionLen;
    int SetCommitInt;
    unsigned char OptionVal;

    switch(cTaskOwnState)
    {

    case GET_TIME_TASK_STATE_INIT:
        //
        // Start networking service in default STA_MODE state
        //
        lRetVal = NetStartDefaultState();
        if(lRetVal < 0)
        {
          return TASK_RET_DONE;
        }

        //
        // Set the status as stated
        //
        sDisplayInfo.ucNetStat = NET_STAT_STARTED;

        //
        // Set the time zone
        //
        sGetTime.ucGmtDiffHr   = GMT_DIFF_TIME_HRS;
        sGetTime.ucGmtDiffMins = GMT_DIFF_TIME_MINS;

        //
        // Set conneting status
        //
        sDisplayInfo.ucNetStat = NET_STAT_CONN;

        //
        // Get the Version Info
        //
        NetFwInfoGet(&sDisplayInfo.sNwpVersion);

        //
        // Make the formated string for firmware version
        //
        sprintf(sDisplayInfo.ucNwpVersion,
                "%d.%d.%d.%d.31.%d.%d.%d.%d.%d.%d.%d.%d",
                sDisplayInfo.sNwpVersion.NwpVersion[0],
                sDisplayInfo.sNwpVersion.NwpVersion[1],
                sDisplayInfo.sNwpVersion.NwpVersion[2],
                sDisplayInfo.sNwpVersion.NwpVersion[3],
                sDisplayInfo.sNwpVersion.ChipFwAndPhyVersion.FwVersion[0],
                sDisplayInfo.sNwpVersion.ChipFwAndPhyVersion.FwVersion[1],
                sDisplayInfo.sNwpVersion.ChipFwAndPhyVersion.FwVersion[2],
                sDisplayInfo.sNwpVersion.ChipFwAndPhyVersion.FwVersion[3],
                sDisplayInfo.sNwpVersion.ChipFwAndPhyVersion.PhyVersion[0],
                sDisplayInfo.sNwpVersion.ChipFwAndPhyVersion.PhyVersion[1],
                sDisplayInfo.sNwpVersion.ChipFwAndPhyVersion.PhyVersion[2],
                sDisplayInfo.sNwpVersion.ChipFwAndPhyVersion.PhyVersion[3]);

        //
        // Set next task state
        //
        cTaskOwnState = GET_TIME_TASK_STATE_CONN;

        //
        // Signal display refresh
        //
        DisplayRefresh();
        break;

    case GET_TIME_TASK_STATE_CONN:

        //
        // Initialize AP security params
        //
        SecurityParams.Key = (signed char *)SECURITY_KEY;
        SecurityParams.KeyLen = strlen(SECURITY_KEY);
        SecurityParams.Type = SECURITY_TYPE;

        //
        // Connect to Access Point
        //
        lRetVal = NetWlanConnect(SSID_NAME,&SecurityParams);

        //
        // Failed to connect to AP, kill the task
        //
        if(lRetVal != 0)
        {
          return TASK_RET_DONE;
        }

        //
	// Check if this image is booted in test mode
	//
	sl_extLib_OtaGet(pvOtaApp,EXTLIB_OTA_GET_OPT_IS_PENDING_COMMIT,
                         &OptionLen,&OptionVal);

	if(OptionVal == true)
	{


            SetCommitInt = OTA_ACTION_IMAGE_COMMITED;
            sl_extLib_OtaSet(pvOtaApp, EXTLIB_OTA_SET_OPT_IMAGE_COMMIT,
                             sizeof(int), (_u8 *)&SetCommitInt);

            //
            // Set status
            //
            g_ulSysState = SYS_STATE_TEST_REBOOT;
            DisplayRefresh();

            //
            // Reboot the MCU
            //
            cTaskOwnState = GET_TIME_TASK_STATE_DONE;
            break;
	}

        //
        // Set the status
        //
        sDisplayInfo.ucNetStat = NET_STAT_CONNED;
        DisplayRefresh();

        //
        // Signal the network start
        //
        TaskSyncObjSignal(&g_NetStatSyncObj);

        //
        // Set next task state
        //
        cTaskOwnState = GET_TIME_TASK_STATE_OPEN_SOCK;

        break;

    case GET_TIME_TASK_STATE_OPEN_SOCK:

        //
        // Create UDP socket
        //
        sGetTime.iSocket = sl_Socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

        //
        // Failed to create a socket, kill the task
        //
        while(sGetTime.iSocket < 0)
        {
          return TASK_RET_DONE;
        }

        //
        // Set socket time option
        //
        timeVal.tv_sec =  20;
        timeVal.tv_usec = 0;
        sl_SetSockOpt(sGetTime.iSocket,SOL_SOCKET,SL_SO_RCVTIMEO, &timeVal,
                      sizeof(timeVal));

        //
        // Initialize Server Index
        //
        sDisplayInfo.ucServerIndex = 0;
        DisplayRefresh();

        //
        // Set next task state
        //
        cTaskOwnState = GET_TIME_TASK_STATE_GET_IP;
        break;


    case GET_TIME_TASK_STATE_GET_IP:

        //
        // Get the NTP server host IP address using the DNS lookup
        //
        lRetVal = NetGetHostIP((char*)g_acSNTPserver[sDisplayInfo.ucServerIndex],
                         &sGetTime.ulNtpServerIP);

        sDisplayInfo.ulServerIP = sGetTime.ulNtpServerIP;
        DisplayRefresh();

        //
        // Set the net task state based on return value
        //
        if(lRetVal >= 0)
        {
          //
          // Go to get time
          //
          cTaskOwnState = GET_TIME_TASK_STATE_GET_TIME;
        }
        else
        {
          //
          // Go back and try a new NTP server
          //
          cTaskOwnState = GET_TIME_TASK_STATE_SET_SERVER;
        }
        break;

    case GET_TIME_TASK_STATE_GET_TIME:

        //
        // Get the NTP time and display the time
        //
        lRetVal = GetSNTPTime(&sGetTime);
        DisplayRefresh();

        //
        // On failure go back and try a new NTP server
        //
        if(lRetVal != 0)
        {
          //
          // Chech and wait if we are disconnected
          //
          while( !NetIsConnectedToAP() )
          {

          }

          cTaskOwnState = GET_TIME_TASK_STATE_SET_SERVER;
        }

        //
        // End the task if Reboot was requested
        //
        if(g_ulSysState == SYS_STATE_REBOOT)
        {
          cTaskOwnState = GET_TIME_TASK_STATE_DONE;
        }

        //
        // Set some sleep
        //
        TaskSleep(5*1000);
        break;

    case GET_TIME_TASK_STATE_SET_SERVER:

          strcpy((char *)sDisplayInfo.ucUTCTime,"NTP Server Error. Retrying...");
          sDisplayInfo.ucLocalTime[0]='-';
          sDisplayInfo.ucLocalTime[1]='\0';

          sDisplayInfo.ucServerIndex
            = ((sDisplayInfo.ucServerIndex + 1)%NOF_NTP_SERVER);

          cTaskOwnState = GET_TIME_TASK_STATE_GET_IP;
          DisplayRefresh();
          break;

    default:

        //
        // Disconnect and stop networking service
        //
        NetStop();

        sDisplayInfo.ucNetStat = NET_STAT_OFF;
        DisplayRefresh();

        //
        // Reboot nonw
        //
        RebootMCU();

        //
        // Return task done
        //
        return TASK_RET_DONE;
    }

    //
    // Return task in progress
    //
    return TASK_RET_IN_PROG;
}


//****************************************************************************
//
//! Signal display refresh
//!
//! \return None.
//
//****************************************************************************
void DisplayRefresh()
{
  //
  // Signal Refresh display
  //
  TaskSyncObjSignal(&g_DispRefreshSyncObj);
}

//****************************************************************************
//
//! Task implememting the dispaly on uart terminal
//!
//! \return Returns \b TASK_RET_IN_PROG.
//
//****************************************************************************
int DisplayTask(void *pvParameters)
{
    //
    // Clear screen
    //
    Message("\033[2J\033[H\033[?25l");

    //
    // Display Banner
    //
    DisplayBanner("OTA Update");

    //
    // Application version
    //
    Report("App Version\t\t: %s\n\r",sDisplayInfo.ucAppVersion);

    //
    // Network F/W version
    //
    Report("Nwp Version\t\t: %s\n\n\r",sDisplayInfo.ucNwpVersion);

    Message("Wifi Status\t\t: ");

    switch(sDisplayInfo.ucNetStat)
    {
    case NET_STAT_OFF:
      Message("Power Off");
      break;

    case NET_STAT_STARTED:
      Message("Power On");
      break;

    case NET_STAT_CONN:
      Message("Connecting...");
      break;

    case NET_STAT_CONNED:
      Message("Connected to "SSID_NAME);
    }
    Message("\n\n\r");

    //
    // Display Server Info
    //
    Report("NTP Server\t\t: %s\n\r",g_acSNTPserver[sDisplayInfo.ucServerIndex]);
    Report("NTP Server IP\t\t: %d.%d.%d.%d\n\n\r",
           sDisplayInfo.ucServerIP[3],
           sDisplayInfo.ucServerIP[2],
           sDisplayInfo.ucServerIP[1],
           sDisplayInfo.ucServerIP[0]);


    //
    // Display Local Time
    //
    Report("GTM Time\t\t: %s\n\r",sDisplayInfo.ucUTCTime);


    //
    // Display Local Time
    //
    Report("Local Time (%s)\t: %s\n\n\r",sDisplayInfo.ucTimeZone,
           sDisplayInfo.ucLocalTime);

    //
    // Display OTA update status
    //
    Message("OTA Update Status\t: ");
    switch(sDisplayInfo.iOTAStatus)
    {
    case OTA_STOPPED :
      if(g_ulSysState == SYS_STATE_TEST_REBOOT)
      {
        Message("Booted in Test Mode.");
      }
      else
      {
        Message("Press SW3 : Updates or SW2: Factory Reset");
      }
      break;


    case OTA_INPROGRESS :
      Message("In Progress...");
      break;


    case OTA_DONE :
      Message("Completed");
      break;


    case OTA_NO_UPDATES :
      Message("No Updates found. Press SW3 to check for updates");
      break;


    case OTA_ERROR_RETRY:
      Message("Error Retrying...");
      break;

    case OTA_ERROR:
      Message("Failed to access server after 10 retries");
      break;
    }

    //
    // Is reboot requested
    //
    if( (g_ulSysState == SYS_STATE_REBOOT) ||
       (g_ulSysState == SYS_STATE_TEST_REBOOT) )
    {
      Message(" Rebooting...");
    }

    //
    // Wait a refresh request
    //
    TaskSyncObjWait(&g_DispRefreshSyncObj);


    //
    // Return progress.
    //
    return TASK_RET_IN_PROG;

}

//****************************************************************************
//
//! Sets the OTA server info and vendor ID
//!
//! \param pvOtaApp pointer to OtaApp handler
//! \param ucVendorStr vendor string
//! \param pfnOTACallBack is  pointer to callback function
//!
//! This function sets the OTA server info and vendor ID.
//!
//! \return None.
//
//****************************************************************************
int OTAServerInfoSet(void **pvOtaApp, char *vendorStr)
{

    //
    // Set OTA server info
    //
    g_otaOptServerInfo.ip_address = OTA_SERVER_IP_ADDRESS;
    g_otaOptServerInfo.secured_connection = OTA_SERVER_SECURED;
    strcpy((char *)g_otaOptServerInfo.server_domain, OTA_SERVER_NAME);
    strcpy((char *)g_otaOptServerInfo.rest_hdr_val, OTA_SERVER_APP_TOKEN);

    //
    // Set OTA server Info
    //
    sl_extLib_OtaSet(*pvOtaApp, EXTLIB_OTA_SET_OPT_SERVER_INFO,
                     sizeof(g_otaOptServerInfo), (_u8 *)&g_otaOptServerInfo);

    //
    // Set vendor ID.
    //
    sl_extLib_OtaSet(*pvOtaApp, EXTLIB_OTA_SET_OPT_VENDOR_ID, strlen(vendorStr),
                     (_u8 *)vendorStr);

    //
    // Return ok status
    //
    return RUN_STAT_OK;
}



//****************************************************************************
//
//! Task function implementing the OTA update functionality
//!
//! \param none
//!
//! \return None.
//
//****************************************************************************
int OTAUpdateTask(void *pvParameters)
{
    int iRet;
    int SetCommitInt = 1;
    unsigned char ucVendorStr[50];
    unsigned long ulVendorStrLen;
    static char cTaskOwnState = OTA_UPDATE_STATE_WAIT_START;

    switch(cTaskOwnState)
    {

    case OTA_UPDATE_STATE_WAIT_START:

        cTaskOwnState = OTA_UPDATE_STATE_WAIT_BTN;

        //
        // Wait for sl_Start and wlan connect to complete
        //
        TaskSyncObjWait(&g_NetStatSyncObj);
        break;

    case OTA_UPDATE_STATE_WAIT_BTN:

        cTaskOwnState = OTA_UPDATE_STATE_INIT;

        //
        // Wait for button press
        //
        TaskSyncObjWait(&g_OTAStatSyncObj);
        Button_IF_EnableInterrupt(SW3);
        break;

    case OTA_UPDATE_STATE_INIT:

        //
	// Create vendor string from NWP version and Vendor Prefix
	//
        strcpy((char *)ucVendorStr,OTA_VENDOR_STRING);

        ulVendorStrLen = strlen(OTA_VENDOR_STRING);

        sprintf((char *)&ucVendorStr[ulVendorStrLen],"_%02lu%02lu%02lu%02lu",
            sDisplayInfo.sNwpVersion.NwpVersion[0],
            sDisplayInfo.sNwpVersion.NwpVersion[1],
            sDisplayInfo.sNwpVersion.NwpVersion[2],
            sDisplayInfo.sNwpVersion.NwpVersion[3]);

        //
        // Initializa OTA service
        //
        OTAServerInfoSet(&pvOtaApp,(char *)ucVendorStr);

        //
        // Set the OTA status and system state to RUN
        //
        sDisplayInfo.iOTAStatus = OTA_INPROGRESS;
        g_ulSysState	        = SYS_STATE_RUN;
        cTaskOwnState           = OTA_UPDATE_STATE_RUN;
        DisplayRefresh();
        break;

    case OTA_UPDATE_STATE_RUN:

        iRet = sl_extLib_OtaRun(pvOtaApp);

        if ( iRet < 0 )
        {

            if( RUN_STAT_ERROR_CONTINUOUS_ACCESS_FAILURES == iRet )
            {
              //
              // Schelude a restart.
              //
              sDisplayInfo.iOTAStatus = OTA_ERROR;
              g_ulSysState =  SYS_STATE_REBOOT;

            }
            else
            {
              sDisplayInfo.iOTAStatus = OTA_ERROR_RETRY;
            }

        }
        else if( iRet == RUN_STAT_NO_UPDATES )
        {

            sDisplayInfo.iOTAStatus = OTA_NO_UPDATES;
            g_ulSysState =  SYS_STATE_WAIT;
            cTaskOwnState = OTA_UPDATE_STATE_WAIT_BTN;
            //
            // Enable button Interrupt
            //
            Button_IF_EnableInterrupt(SW3);
            DisplayRefresh();
        }
        else if ((iRet & RUN_STAT_DOWNLOAD_DONE))
        {
            //
            // Set OTA File for testing
            //
            iRet = sl_extLib_OtaSet(pvOtaApp, EXTLIB_OTA_SET_OPT_IMAGE_TEST,
                                    sizeof(int), (_u8 *)&SetCommitInt);

            sDisplayInfo.iOTAStatus = OTA_DONE;

            if (iRet & (OTA_ACTION_RESET_MCU| OTA_ACTION_RESET_NWP) )
            {
                g_ulSysState  = SYS_STATE_REBOOT;
                cTaskOwnState = OTA_UPDATE_STATE_DONE;
            }
            else
            {
                g_ulSysState =  SYS_STATE_WAIT;
                cTaskOwnState = OTA_UPDATE_STATE_WAIT_BTN;
                //
                // Enable button Interrupt
                //
                Button_IF_EnableInterrupt(SW3);
            }
        }
        else if(sDisplayInfo.iOTAStatus == OTA_ERROR_RETRY)
        {
          sDisplayInfo.iOTAStatus = OTA_INPROGRESS;

        }

        DisplayRefresh();
        break;

    default:
      return TASK_RET_DONE;

    }

    return TASK_RET_IN_PROG;
}

//****************************************************************************
//
//! \internal
//!
//! Set the factory image as the new boot image
//!
//! return None
//
//****************************************************************************
static int FactoryResetTask(void *pvParameters)
{
  sBootInfo_t sBootInfo;
  long lFileHandle;
  unsigned long ulToken;
  static unsigned char ucTaskOwnState = FACTORY_RST_STATE_WAIT_BTN;

  switch(ucTaskOwnState)
  {

  case FACTORY_RST_STATE_WAIT_BTN:
    //
    // Wait for button press
    //
    TaskSyncObjWait(&g_FactResetSyncObj);

    //
    // Switch to RUN state
    //
    ucTaskOwnState = FACTORY_RST_STATE_RUN;

    //
    // Break
    //
    break;

  case FACTORY_RST_STATE_RUN:

    //
    // Read the boot Info
    //
    if( 0 == sl_FsOpen((unsigned char *)IMG_BOOT_INFO, FS_MODE_OPEN_READ,
                       &ulToken, &lFileHandle) )
    {
        if( 0 > sl_FsRead(lFileHandle, 0, (unsigned char *)&sBootInfo,
                         sizeof(sBootInfo_t)) )
        {
          return TASK_RET_DONE;
        }
        sl_FsClose(lFileHandle, 0, 0, 0);
    }

    //
    // Set the factory default
    //
    sBootInfo.ucActiveImg = IMG_ACT_FACTORY;
    sBootInfo.ulImgStatus = IMG_STATUS_NOTEST;

    //
    // Save the new configuration
    //
    if( 0 == sl_FsOpen((unsigned char *)IMG_BOOT_INFO, FS_MODE_OPEN_WRITE,
                       &ulToken, &lFileHandle) )
    {
        sl_FsWrite(lFileHandle, 0, (unsigned char *)&sBootInfo,
                   sizeof(sBootInfo_t));
        sl_FsClose(lFileHandle, 0, 0, 0);
    }

    //
    // Force reboot
    //
    g_ulSysState  = SYS_STATE_REBOOT;

    //
    // Return done.
    //
    return TASK_RET_DONE;
  }

  return TASK_RET_IN_PROG;
}

//****************************************************************************
//
//! \internal
//!
//! Switch 2 Interuupt handler
//!
//! \return None
//
//****************************************************************************
static void OnPressSW2Handler()
{
  //
  // Signal for button press
  //
  TaskSyncObjSignal(&g_FactResetSyncObj);
}

//****************************************************************************
//
//! \internal
//!
//! Switch 3 Interuupt handler
//!
//! \return None
//
//****************************************************************************
static void OnPressSW3Handler()
{
  //
  // Signal the button Press
  //
  TaskSyncObjSignal(&g_OTAStatSyncObj);
}


//*****************************************************************************
//
//! This function calls the nonos loop for SL Host
//!
//! \return Returns \b TASK_RET_IN_PROG
//
//*****************************************************************************
int NonOSMainLoopTask(void *pvParameters)
{
   _SlNonOsMainLoopTask();

   return TASK_RET_IN_PROG;
}

//*****************************************************************************
//
//*****************************************************************************
void main()
{

  //
  // Initialize Board configurations
  //
  BoardInit();

  //
  // Do Pin mux
  //
  PinMuxConfig();

  //
  // Enable and configure DMA
  //
  UDMAInit();

  //
  // Initialize the display
  //
  InitTerm();

  //
  // Initialize the buttons
  //
  Button_IF_Init(OnPressSW2Handler,OnPressSW3Handler);
  Button_IF_DisableInterrupt(SW3);

  //
  // Initialize OTA
  //
  pvOtaApp = sl_extLib_OtaInit(RUN_MODE_NONE_OS | RUN_MODE_BLOCKING,0);

  //
  // Initialize the global flag
  //
  g_ulSysState = SYS_STATE_RUN;

  //
  // Create sync object to signal Sl_Start and Wlan Connect complete
  //
  TaskSyncObjCreate(&g_NetStatSyncObj);

  //
  // Create sync object to signal OTA start
  //
  TaskSyncObjCreate(&g_OTAStatSyncObj);

  //
  // Create sync object to signal display refresh
  //
  TaskSyncObjCreate(&g_DispRefreshSyncObj);

  //
  // Create sync object to signal Factory reset event
  //
  TaskSyncObjCreate(&g_FactResetSyncObj);

  //
  // Initialize Display Info
  //
  memset(&sDisplayInfo,0,sizeof(tDisplayInfo));

  sDisplayInfo.ucLocalTime[0] = '-';
  sDisplayInfo.ucUTCTime[0]   = '-';

  sprintf(sDisplayInfo.ucTimeZone,"%+03d:%02d",
          (char)GMT_DIFF_TIME_HRS,
          (char)GMT_DIFF_TIME_MINS);


  sprintf(sDisplayInfo.ucAppVersion,"%d.%d.%d",
          APP_VER_MAJOR,
          APP_VER_MINOR,
          APP_VER_BUILD);


  //
  // Start the Print task
  //
  TaskCreate(DisplayTask, NULL);

  //
  // Start the GetNTPTime task
  //
  TaskCreate(GetNTPTimeTask, NULL);

  //
  // OTA Update Task
  //
  TaskCreate(OTAUpdateTask, NULL);

  //
  // SL main loop task;
  //
  TaskCreate(NonOSMainLoopTask, NULL);

  //
  // Factory Reset Task
  //
  TaskCreate(FactoryResetTask,NULL);

  //
  // Start the task scheduler
  //
  TaskMainLoopStart();

  //
  // Infinite loop. Control Should never reach here
  //
  while(1)
  {

  }
}
