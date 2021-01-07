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
// Application Name     -   Mode configuration 
// Application Overview -   This sample application demonstrates how one can
//                          switch between different networking modes(STA, AP
//                          or P2P).
//*****************************************************************************

//****************************************************************************
//
//! \addtogroup mode_config
//! @{
//
//****************************************************************************

// Simplelink includes
#include "simplelink.h"

//Driverlib includes
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "rom.h"
#include "rom_map.h"
#include "interrupt.h"
#include "prcm.h"
#include "uart.h"

//Common interface includes
#ifndef NOTERM
#include "uart_if.h"
#endif
#include "pinmux.h"

#ifdef NOTERM
#define UART_PRINT(x,...)
#define ERR_PRINT(x)
#else
#define UART_PRINT Report
#define ERR_PRINT(x) Report("Error [%d] at line [%d] in function [%s]  \n\r",x,__LINE__,__FUNCTION__)
#endif

#define APPLICATION_VERSION     "1.4.0"
#define FOREVER                 1
#define APP_NAME                "Mode Switch"
#define SL_STOP_TIMEOUT         200

// Loop forever, user can change it as per application's requirement
#define LOOP_FOREVER() \
            {\
                while(1); \
            }

// check the error code and handle it
#define ASSERT_ON_ERROR(error_code)\
            {\
                 if(error_code < 0) \
                   {\
                        ERR_PRINT(error_code);\
                        return error_code;\
                 }\
            }

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
unsigned short g_usConnectionStatus = 0;
volatile unsigned short g_usIpObtained = 0;

#if defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif
//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************


//****************************************************************************
//                      LOCAL FUNCTION PROTOTYPES
//****************************************************************************
static void DisplayBanner();
static long DisplayCurrentConfig(int iMode);
static long ConfigureMode(int iMode);
static int GetSsidName(char *pcSsidName, unsigned int uiMaxLen);
static void BoardInit();


/****************************************************************************/
/*                      LOCAL FUNCTION DEFINITIONS                          */
/****************************************************************************/
//*****************************************************************************
//
//! \brief This function handles General Events
//!
//! \param[in]     pDevEvent - Pointer to General Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkGeneralEventHandler(SlDeviceEvent_t *pDevEvent)
{
    if(!pDevEvent)
    {
        return;
    }

    //
    // Most of the general errors are not FATAL are are to be handled
    // appropriately by the application
    //
    UART_PRINT("[GENERAL EVENT] - ID=[%d] Sender=[%d]\n\n",
               pDevEvent->EventData.deviceEvent.status,
               pDevEvent->EventData.deviceEvent.sender);
}

//****************************************************************************
//
//!    \brief                         None
//!
//! \param[in]                     pSock - Pointer to Socket Event Info
//!
//! \return                        None
//
//****************************************************************************
void SimpleLinkSockEventHandler(SlSockEvent_t *pSock)
{
}

//****************************************************************************
//
//!    \brief This function handles WLAN events
//!
//! \param  pSlWlanEvent is the event passed to the handler
//!
//! \return None
//
//****************************************************************************
void SimpleLinkWlanEventHandler(SlWlanEvent_t *pSlWlanEvent)
{
    switch(((SlWlanEvent_t*)pSlWlanEvent)->Event)
    {
        case SL_WLAN_CONNECT_EVENT:
           g_usConnectionStatus = 1;
           break;
        case SL_WLAN_DISCONNECT_EVENT:
           g_usConnectionStatus = 0;
           break;
        default:
          break;
    }
}

//****************************************************************************
//
//!    \brief This function handles events for IP address acquisition via DHCP
//!           indication
//!
//! \param  pNetAppEvent is the event passed to the Handler
//!
//! \return None
//
//****************************************************************************
void SimpleLinkNetAppEventHandler(SlNetAppEvent_t *pNetAppEvent)
{
    SlNetAppEvent_t *pNetApp = (SlNetAppEvent_t *)pNetAppEvent;

    switch( pNetApp->Event )
    {
    case SL_NETAPP_IPV4_IPACQUIRED_EVENT:
          g_usIpObtained = 1;
          break;
    default:
        break;
    }
}

//*****************************************************************************
//
//! This function gets triggered when HTTP Server receives Application
//! defined GET and POST HTTP Tokens.
//!
//! \param pSlHttpServerEvent Pointer indicating http server event
//! \param pSlHttpServerResponse Pointer indicating http server response
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkHttpServerCallback(SlHttpServerEvent_t *pSlHttpServerEvent, 
                                SlHttpServerResponse_t *pSlHttpServerResponse)
{
}


//****************************************************************************
//
//! read the current configuration and displays the same.
//!
//! \param iMode is the current mode of the device
//! This function
//!    1. display the current networking mode and also dislays the ssid name
//!       in cas eof AP mode.
//!
//! \return 0: success, -ve: failure
//
//****************************************************************************
long DisplayCurrentConfig(int iMode)
{
    char pcSsidName[32];
    unsigned short len;
    unsigned short  config_opt;
    long lRetVal = -1;

    if(iMode == ROLE_STA)
    {
        UART_PRINT("Device is in STA mode\n\r");
    }
    else if(iMode == ROLE_AP)
    {
        UART_PRINT("Device is in AP mode\n\r");
        while(!g_usIpObtained)
        {
            _SlNonOsMainLoopTask();
        }

        len = 32;
        config_opt = WLAN_AP_OPT_SSID;
        lRetVal = sl_WlanGet(SL_WLAN_CFG_AP_ID, &config_opt , &len,
                                (unsigned char*) pcSsidName);
        ASSERT_ON_ERROR(lRetVal);

        UART_PRINT("AP name: %s\n\r",pcSsidName);
    }
    else if(iMode == ROLE_P2P)
    {
        UART_PRINT("Device is in P2P mode\n\r");
    }
    else
    {
        UART_PRINT("Could not retrieve mode\n\r");
    }

    return 0;
}

//****************************************************************************
//
//! Confgiures the mode in which the device will work
//!
//! \param iMode is the current mode of the device
//!
//! This function
//!    1. prompt user for desired configuration and accordingly configure the
//!          networking mode(STA or AP).
//!       2. also give the user the option to configure the ssid name in case of
//!       AP mode.
//!
//! \return 0: success, -ve: failure.
//
//****************************************************************************
long ConfigureMode(int iMode)
{
    char cCharacter = 'a';
    char pcSsidName[33];
    unsigned short   len;
    long lRetVal = -1;

    while(cCharacter != '1' && cCharacter != '2' && cCharacter != '3')
    {
        UART_PRINT("Select a mode to switch to:\n\r");
        UART_PRINT("1. STA mode\n\r2. AP Mode\n\r3. P2P Mode\n\r> ");
        cCharacter = MAP_UARTCharGet(CONSOLE);
        MAP_UARTCharPut(CONSOLE,cCharacter);
        MAP_UARTCharPut(CONSOLE,'\n');
        MAP_UARTCharPut(CONSOLE,'\r');
    }
    if(cCharacter == '1')
    {
        lRetVal = sl_WlanSetMode(ROLE_STA);
        ASSERT_ON_ERROR(lRetVal);
        UART_PRINT("Mode configured\n\r");
    }
    else if(cCharacter == '2')
    {
        UART_PRINT("Enter the SSID name: ");
        GetSsidName(pcSsidName,33);
        _SlNonOsMainLoopTask();
        lRetVal = sl_WlanSetMode(ROLE_AP);
        ASSERT_ON_ERROR(lRetVal);

        len = strlen(pcSsidName);
        lRetVal = sl_WlanSet(SL_WLAN_CFG_AP_ID, WLAN_AP_OPT_SSID, len,
                                (unsigned char*) pcSsidName);
        ASSERT_ON_ERROR(lRetVal);

        UART_PRINT("Mode configured\n\r");
    }
    else if(cCharacter == '3')
    {
        lRetVal = sl_WlanSetMode(ROLE_P2P);
        ASSERT_ON_ERROR(lRetVal);

        UART_PRINT("Mode configured\n\r");
    }
    else
    {
       UART_PRINT("Incorrect input\n\r");
    }
    return 0;
}

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

    Report("\n\n\n\r");
    Report("\t\t *************************************************\n\r");
    Report("\t\t      CC3200 %s Application       \n\r", AppName);
    Report("\t\t *************************************************\n\r");
    Report("\n\n\n\r");
}

//****************************************************************************
//
//! Get Ssid name form the user over UART
//!
//! \param pcSsidName is a pointer to the array which will contain the ssid name
//! 
//! This function  
//!    1. gets the ssid name string over uart
//!
//! \return iStringLength is the length of the ssid(user input).
//
//****************************************************************************
int GetSsidName(char *pcSsidName, unsigned int uiMaxLen)
{
    char cCharacter;
    int iStringLength = 0;
    while(1)
    {          
        //
        // Fetching the input from the terminal.
        //
        cCharacter = MAP_UARTCharGet(CONSOLE);
        
        if(cCharacter == '\r' || cCharacter == '\n' ||
           (iStringLength >= uiMaxLen-1))
        {
            if(iStringLength >= uiMaxLen-1)
            {                
                MAP_UARTCharPut(CONSOLE, cCharacter);
                pcSsidName[iStringLength] = cCharacter;
                iStringLength++;
                
            }
            MAP_UARTCharPut(CONSOLE, '\n');
            MAP_UARTCharPut(CONSOLE, '\r');
            pcSsidName[iStringLength] = '\0';
            break;
        }
        else
        {
            MAP_UARTCharPut(CONSOLE, cCharacter);
            pcSsidName[iStringLength] = cCharacter;
            iStringLength++;
        }
    }
    return(iStringLength);
}

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
#if defined(ccs)
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


//****************************************************************************
//                            MAIN FUNCTION
//****************************************************************************
void main()
{
    int iMode = 0;
    long lRetVal = -1;

    //
    // Board Initialization
    //
    BoardInit();

    //
    // Configure the pinmux settings for the peripherals exercised
    //
    PinMuxConfig();

    //
    // Configuring UART
    //
#ifndef NOTERM
    InitTerm();
#endif

    //
    // Display banner
    //
    DisplayBanner(APP_NAME);

    while(1)
    {
        //
        // Starting SimpleLink network processor
        //
        iMode = sl_Start(NULL,NULL,NULL);
        if(iMode < 0)
        {
            ERR_PRINT("Failed to start network processor\n\r");
            LOOP_FOREVER();
        }

        //
        // Display current mode and SSID name (for AP mode)
        //
        lRetVal = DisplayCurrentConfig(iMode);
        if(lRetVal < 0)
        {
            ERR_PRINT("Could not retrieve current device configuration\n\r");
            LOOP_FOREVER();
        }

        //
        // Configure the networking mode and SSID name (for AP mode)
        //
        lRetVal = ConfigureMode(iMode);
        if(lRetVal < 0)
        {
            ERR_PRINT("Could not configure new mode\n\r");
            LOOP_FOREVER();
        }

        UART_PRINT("Restarting network device... \n\r\n\r");
        lRetVal = sl_Stop(SL_STOP_TIMEOUT);
        if(lRetVal < 0)
        {
            ERR_PRINT("Failed to stop network processor\n\r");
            LOOP_FOREVER();
        }
    }
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
