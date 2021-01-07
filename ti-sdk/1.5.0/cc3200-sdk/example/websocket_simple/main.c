//*****************************************************************************
// main.c
//
// Reference code to demonstrate getting the current time using an NTP server.
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

//****************************************************************************
//
//! \addtogroup main
//! @{
//
//****************************************************************************

#include <stdio.h>
#include <string.h>

// Driverlib Includes
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "systick.h"
#include "pin.h"
#include "prcm.h"
#include "utils.h"
#include "rom_map.h"
#include "interrupt.h"

// SimpleLink include
#include "simplelink.h"

// Free-RTOS\TI-RTOS include
#include "osi.h"

// Common interface includes
#ifndef NOTERM
#include "uart_if.h"
#endif
#include "common.h"
#include "gpio_if.h"
#include "pinmux.h"
#include "httpserverapp.h"

//*****************************************************************************
//                      GLOBAL VARIABLES for VECTOR TABLE
//*****************************************************************************
#if defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif

//*****************************************************************************
//                          LOCAL DEFINES
//*****************************************************************************
#define APP_NAME		                "Simple Websocket"
#define SPAWN_TASK_PRIORITY             9
#define HTTP_SERVER_APP_TASK_PRIORITY   3
#define COUNTER_APP_TASK_PRIORITY       1
#define LED_APP_TASK_PRIORITY           2
#define HTTP_STACK_SIZE                 4096
#define OSI_STACK_SIZE                  2048

//*****************************************************************************
//
//! Application startup display on UART
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************

#ifndef NOTERM
static void
DisplayBanner(char * AppName)
{

    Report("\n\n\n\r");
    Report("\t\t *************************************************\n\r");
    Report("\t\t	  CC3200 %s Application       \n\r", AppName);
    Report("\t\t *************************************************\n\r");
    Report("\n\n\n\r");
}
#endif

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
    IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
#endif
#if defined(ewarm)
    IntVTableBaseSet((unsigned long)&__vector_table);
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
//							MAIN FUNCTION
//****************************************************************************
void main()
{
	//
	// Board Initialization
	//
	BoardInit();

	//
	// Pinmux for UART and GPIOs
	//
	PinMuxConfig();

#ifndef NOTERM
	//
	// Configuring UART
	//
	InitTerm();
	
    //
    // Display Application Banner
    //
    DisplayBanner(APP_NAME);
#endif

    GPIO_IF_LedConfigure(LED1|LED3);
    GPIO_IF_LedOff(MCU_RED_LED_GPIO);
    GPIO_IF_LedOff(MCU_GREEN_LED_GPIO);

    //
    // Start the SimpleLink Host
    //
    VStartSimpleLinkSpawnTask(SPAWN_TASK_PRIORITY);

    //
    // Start the HttpServer task, counter task, and task for red LED control
    //
    //
    osi_TaskCreate(HttpServerAppTask,
                    "WebSocketApp",
                        HTTP_STACK_SIZE,
                        NULL,
                        HTTP_SERVER_APP_TASK_PRIORITY,
                        NULL );

    osi_TaskCreate(CounterAppTask,
                    "Counter application",
                        OSI_STACK_SIZE,
                        NULL,
                        COUNTER_APP_TASK_PRIORITY,
                        NULL );

    //
    // Start the task scheduler
    //
    osi_start();

	LOOP_FOREVER();
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
