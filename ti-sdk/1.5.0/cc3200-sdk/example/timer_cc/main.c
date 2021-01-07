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
// Application Name     - Timer Count Capture
// Application Overview - This application showcases Timer's count capture 
//                        feature to measure frequency of an external signal.
//
//*****************************************************************************

// Driverlib includes
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "interrupt.h"
#include "prcm.h"
#include "gpio.h"
#include "utils.h"
#include "timer.h"
#include "rom.h"
#include "rom_map.h"
#include "pin.h"

// Common interface includes
#include "uart_if.h"
#include "pinmux.h"


#define APPLICATION_VERSION     "1.4.0"
#define APP_NAME        "Timer Count Capture"
#define TIMER_FREQ      80000000

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
static unsigned long g_ulSamples[2];
static unsigned long g_ulFreq;

#if defined(ccs) || defined(gcc)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif
//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************


//*****************************************************************************
//
//! Timer interrupt handler
//
//*****************************************************************************
static void TimerIntHandler()
{
    //
    // Clear the interrupt
    //
    MAP_TimerIntClear(TIMERA2_BASE,TIMER_CAPA_EVENT);

    //
    // Get the samples and compute the frequency
    //
    g_ulSamples[0] = g_ulSamples[1];
    g_ulSamples[1] = MAP_TimerValueGet(TIMERA2_BASE,TIMER_A);
	g_ulFreq = ((g_ulSamples[0] > g_ulSamples[1]) ?
				(TIMER_FREQ / (g_ulSamples[0] - g_ulSamples[1])) :
				(TIMER_FREQ / (65535 - (g_ulSamples[1] - g_ulSamples[0]))));
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
void
DisplayBanner(char * AppName)
{

    Report("\n\n\n\r");
    Report("\t\t *************************************************\n\r");
    Report("\t\t\t  CC3200 %s Application       \n\r", AppName);
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
void
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
//! Main  Function
//
//*****************************************************************************
int main()
{

    //
    // Initialize Board configurations
    //
    BoardInit();

    //
    // Pinmux for UART
    //
    PinMuxConfig();

    //
    // Configuring UART
    //
    InitTerm();

    //
    // Display Application Banner
    //
    DisplayBanner(APP_NAME);

    //
    // Enable pull down
    //
    MAP_PinConfigSet(PIN_05,PIN_TYPE_STD_PD,PIN_STRENGTH_6MA);


    //
    // Register timer interrupt hander
    //
    MAP_TimerIntRegister(TIMERA2_BASE,TIMER_A,TimerIntHandler);

    //
    // Configure the timer in edge count mode
    //
    MAP_TimerConfigure(TIMERA2_BASE, (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_TIME));

    //
    // Set the detection edge
    //
    MAP_TimerControlEvent(TIMERA2_BASE,TIMER_A,TIMER_EVENT_POS_EDGE);

    //
    // Set the reload value
    //
    MAP_TimerLoadSet(TIMERA2_BASE,TIMER_A,0xffff);


    //
    // Enable capture event interrupt
    //
    MAP_TimerIntEnable(TIMERA2_BASE,TIMER_CAPA_EVENT);

    //
    // Enable Timer
    //
    MAP_TimerEnable(TIMERA2_BASE,TIMER_A);


    while(1)
    {
        //
        // Report the calculate frequency
        //
        Report("Frequency : %d Hz\n\n\r",g_ulFreq);

        //
        // Delay loop
        //
        MAP_UtilsDelay(80000000/5);
    }
}