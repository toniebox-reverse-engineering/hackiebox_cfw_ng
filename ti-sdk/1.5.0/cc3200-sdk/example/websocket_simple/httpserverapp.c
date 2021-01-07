//*****************************************************************************
// httpserver_app.c
//
// camera application macro & APIs
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
//! \addtogroup Httpserverapp
//! @{
//
//*****************************************************************************

#include <string.h>
#include <stdlib.h>

// Driverlib Includes
#include "rom_map.h"
#include "hw_types.h"
#include "hw_memmap.h"
#include "prcm.h"
#include "utils.h"
#include "timer.h"

// SimpleLink include
#include "simplelink.h"

// Free-RTOS/TI-RTOS include
#include "osi.h"

// HTTP lib includes
#include "HttpCore.h"
#include "HttpRequest.h"
#include "WebSockHandler.h"

// Common-interface includes
#include "network_if.h"
#include "uart_if.h"
#include "common.h"
#include "timer_if.h"
#include "gpio_if.h"
#include "httpserverapp.h"

typedef struct
{
    UINT16 connection;
    char * buffer;
}event_msg;

/****************************************************************************
                              Global variables
****************************************************************************/
char *startcounter = "start";
char *stopcounter = "stop";
UINT8 g_success = 0;
int g_close = 0;
UINT16 g_uConnection;
static volatile unsigned long g_ulBase;
static OsiSyncObj_t g_CounterSyncObj;
OsiMsgQ_t g_recvQueue;

void WebSocketCloseSessionHandler(void)
{
	g_close = 1;
}


/*!
 *  \brief                  This websocket Event is called when WebSocket Server receives data
 *                          from client. Declared in WebSockHandler.h (webserver library), but must be
 *                          implemented by application.
 *
 *
 *  \param[in]  uConnection Websocket Client Id
 *  \param[in] *ReadBuffer      Pointer to the buffer that holds the payload.
 *
 *  \return                 none.
 *
 */
void WebSocketRecvEventHandler(UINT16 uConnection, char *ReadBuffer)
{
    g_close = 0;
    event_msg msg;

    msg.connection = uConnection;
    msg.buffer = ReadBuffer;

    if (!strcmp(msg.buffer,startcounter))
    {
        g_uConnection = msg.connection;
        // Signal to Counter task to start counter
        osi_SyncObjSignal(&g_CounterSyncObj);
    }
    else if (!strcmp(msg.buffer,stopcounter))
    {
        // Stop counter timer
        Timer_IF_Stop(g_ulBase, TIMER_A);
    }
}


/*!
 * 	\brief 						This websocket Event indicates successful handshake with client
 * 								Once this is called the server can start sending data packets over websocket using
 * 								the sl_WebSocketSend API.
 *
 *
 * 	\param[in] uConnection			Websocket Client Id
 *
 * 	\return						none
 */
void WebSocketHandshakeEventHandler(UINT16 uConnection)
{
	g_success = 1;
}

//*****************************************************************************
//
//! Timer interrupt handler
//!
//! \param  None
//!
//! \return none
//
//*****************************************************************************
void TimerIntHandler(void)
{
    //
    // Clear the timer interrupt.
    //
    Timer_IF_InterruptClear(g_ulBase);

    //
    // Signal to Counter task to send over websocket
    //
    osi_SyncObjSignal(&g_CounterSyncObj);
}

//****************************************************************************
//
//! CounterAppTask
//!
//! \param Initialize timer, wait for timer interrupt, send counter value over websocket
//!
//! \return none
//!
//****************************************************************************

void CounterAppTask(void * param)
{
    struct HttpBlob Write;
    UINT8 Opcode = 0x02;
    unsigned long g_ulTimerInts = 0;

    //
    // Base address for timer
    //
    g_ulBase = TIMERA0_BASE;

    //
    // Configuring the timer
    //
    Timer_IF_Init(PRCM_TIMERA0, g_ulBase, TIMER_CFG_PERIODIC, TIMER_A, 0);

    //
    // Setup the interrupts for the timer timeout.
    //
    Timer_IF_IntSetup(g_ulBase, TIMER_A, TimerIntHandler);

    //
    // Create semaphore object for counter sync
    //
    osi_SyncObjCreate(&g_CounterSyncObj);

    UART_PRINT("Waiting for websocket connection. Connect to your mysimplelink-xxxxxx Access Point and navigate to mysimplelink.net/websocket_demo.html\n\r");

    //
    // Wait for first recv event to start counter
    //
    osi_SyncObjWait(&g_CounterSyncObj, OSI_WAIT_FOREVER);

    //
    // Start timer
    // Value in msecs
    //
    Timer_IF_Start(g_ulBase, TIMER_A, 1000);

    UART_PRINT("Counter timer started\n\r");

    while(1)
    {
        //
        // Wait for timer interrupt
        //
        osi_SyncObjWait(&g_CounterSyncObj, OSI_WAIT_FOREVER);

        //
        // Increase counter
        //
        g_ulTimerInts++;

        //
        // Send updated counter over websocket
        //
        Write.uLength = sizeof(g_ulTimerInts);
        Write.pData = (UINT8 *)(&g_ulTimerInts);
        if(!sl_WebSocketSend(g_uConnection, Write, Opcode))
        {
            UART_PRINT("Error: Cannot send websocket counter update\r\n");
        }

        GPIO_IF_LedToggle(MCU_GREEN_LED_GPIO);
    }
}

//****************************************************************************
//
//! HttpServerAppTask
//!
//! \param Initialize the network processor, stop internal HTTP server, start webserver
//!
//! \return none
//!
//****************************************************************************

void HttpServerAppTask(void * param)
{
	long lRetVal = -1;
	
	//Start SimpleLink in AP Mode
	lRetVal = Network_IF_InitDriver(ROLE_AP);
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }	

	//Stop Internal HTTP Server
	lRetVal = sl_NetAppStop(SL_NET_APP_HTTP_SERVER_ID);
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }	

    UART_PRINT("Start Websocket Server \n\r");

    //
	// Run application library HTTP Server
    // Note this function does not return
    //
	HttpServerInitAndRun(NULL);

	LOOP_FOREVER();

}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
