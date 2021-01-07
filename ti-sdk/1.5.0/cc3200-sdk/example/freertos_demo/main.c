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
// Application Name     - Free-RTOS Demo
// Application Overview - The objective of this application is to showcase 
//					      FreeRTOS features like multiple task creation and 
//						  inter-task communication using queues. This application 
//						  creates two tasks and one message queue. Task one sends 
//						  a constant message into the queue, and task two receives 
//						  the  message from the queue. After receiving each 
//						  message, the second task prints the message to a serial 
//						  terminal over UART.
//
//*****************************************************************************

//*****************************************************************************
//
//! \addtogroup freertos_demo
//! @{
//
//*****************************************************************************

//*****************************************************************************
// Same application can be used to demonstrate TI-RTOS demo with a small change
// in CCS project proerties: 
//      a. add ti_rtos_config in workspace and add as dependencies in 
//         project setting Build->Dependecies->add
//      b. Define USE_TIRTOS in Properties->Build->Advanced Options->
//         Predefined Symbols instead of USE_FREERTOS
//      c. add ti_rtos.a in linker files search option
//*****************************************************************************

// Standard includes.
#include <stdio.h>
#include <stdlib.h>

#include "osi.h"

// Driverlib includes
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "hw_types.h"
#include "hw_ints.h"
#include "interrupt.h"
#include "rom.h"
#include "rom_map.h"
#include "uart.h"
#include "prcm.h"
#include "utils.h"

// Common interface includes
#include "uart_if.h"

#include "pinmux.h"

//*****************************************************************************
//                      MACRO DEFINITIONS
//*****************************************************************************
#define APPLICATION_VERSION     "1.4.0"
#define UART_PRINT              Report
#define SPAWN_TASK_PRIORITY     9
#define OSI_STACK_SIZE          2048
#define APP_NAME                "FreeRTOS Demo"
#define MAX_MSG_LENGTH			16

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
// The queue used to send strings to the task1.
OsiMsgQ_t MsgQ;

#ifndef USE_TIRTOS
/* in case of TI-RTOS don't include startup_*.c in app project */
#if defined(gcc) || defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif
#endif
//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************


//*****************************************************************************
//                      LOCAL FUNCTION DEFINITIONS
//*****************************************************************************
static void vTestTask1( void *pvParameters );
static void vTestTask2( void *pvParameters );
static void BoardInit();


#ifdef USE_FREERTOS
//*****************************************************************************
// FreeRTOS User Hook Functions enabled in FreeRTOSConfig.h
//*****************************************************************************

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

//******************************************************************************
//
//! First test task
//!
//! \param pvParameters is the parameter passed to the task while creating it.
//!
//!    This Function
//!        1. Receive message from the Queue and display it on the terminal.
//!
//! \return none
//
//******************************************************************************
void vTestTask1( void *pvParameters )
{
	char pcMessage[MAX_MSG_LENGTH];
    for( ;; )
    {
    	/* Wait for a message to arrive. */
    	osi_MsgQRead(&MsgQ, pcMessage, OSI_WAIT_FOREVER);

		UART_PRINT("message = ");
		UART_PRINT(pcMessage);
		UART_PRINT("\n\r");
		osi_Sleep(200);
    }
}

//******************************************************************************
//
//! Second test task
//!
//! \param pvParameters is the parameter passed to the task while creating it.
//!
//!    This Function
//!        1. Creates a message and send it to the queue.
//!
//! \return none
//
//******************************************************************************
void vTestTask2( void *pvParameters )
{
   unsigned long ul_2;
   const char *pcInterruptMessage[4] = {"Welcome","to","CC32xx"
           ,"development !\n"};

   ul_2 =0;

   for( ;; )
     {
       /* Queue a message for the print task to display on the UART CONSOLE. */
	   osi_MsgQWrite(&MsgQ, (void*) pcInterruptMessage[ul_2 % 4], OSI_NO_WAIT);
	   ul_2++;
	   osi_Sleep(200);
     }
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
    Report("\t\t    CC3200 %s Application       \n\r", AppName);
    Report("\t\t *************************************************\n\r");
    Report("\n\n\n\r");
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
//!  main function handling the freertos_demo.
//!
//! \param  None
//!
//! \return none
//
//*****************************************************************************
int main( void )
{
    //
    // Initialize the board
    //
    BoardInit();

    PinMuxConfig();

    //
    // Initializing the terminal
    //
    InitTerm();

    //
    // Clearing the terminal
    //
    ClearTerm();

    //
    // Diasplay Banner
    //
    DisplayBanner(APP_NAME);

    //
    // Creating a queue for 10 elements.
    //
    OsiReturnVal_e osi_retVal;
    osi_retVal = osi_MsgQCreate(&MsgQ, "MSGQ", MAX_MSG_LENGTH, 10);
    if(osi_retVal != OSI_OK)
    {
    	// Queue was not created and must not be used.
    	while(1);
    }

    //
    // Create the Queue Receive task
    //
    osi_TaskCreate( vTestTask1, "TASK1",\
    							OSI_STACK_SIZE, NULL, 1, NULL );

    //
    // Create the Queue Send task
    //
    osi_TaskCreate( vTestTask2, "TASK2",\
    							OSI_STACK_SIZE,NULL, 1, NULL );

    //
    // Start the task scheduler
    //
    osi_start();

    return 0;
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
