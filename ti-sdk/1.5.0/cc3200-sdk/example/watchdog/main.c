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
// Application Name     - Watchdog Timer Demo
// Application Overview - This example application demonstrates the use of the 
//                        watchdog as a simple heartbeat for the system.  If the 
//                        watchdog is not periodically fed, it will reset the 
//                        system.  Each time the watchdog is fed, the LED is 
//                        inverted so that it is easy to see that it is being 
//                        fed, which occurs once every second. And after 10 
//                        cycles of interrupt no more feeding.
//
//*****************************************************************************

//*****************************************************************************
//
//! \addtogroup watchdog
//! @{
//
//*****************************************************************************

// Standard includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Driverlib includes
#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "hw_nvic.h"
#include "hw_types.h"
#include "hw_wdt.h"
#include "wdt.h"
#include "prcm.h"
#include "debug.h"
#include "rom.h"
#include "rom_map.h"
#include "interrupt.h"
#include "utils.h"

// Common interface includes
#include "gpio_if.h"
#include "wdt_if.h"
#include "pinmux.h"

#define APPLICATION_VERSION          "1.4.0"
#define WD_PERIOD_MS                 1000
#define MAP_SysCtlClockGet           80000000
#define LED_GPIO                     MCU_RED_LED_GPIO    /* RED LED */
#define MILLISECONDS_TO_TICKS(ms)    ((MAP_SysCtlClockGet / 1000) * (ms))
#define FOREVER                      1

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
volatile tBoolean g_bFeedWatchdog = true;
volatile unsigned long g_ulWatchdogCycles;

#if defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif
//*****************************************************************************
//                  GLOBAL VARIABLES -- End
//*****************************************************************************


//*****************************************************************************
//
//! The interrupt handler for the watchdog timer
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************
void WatchdogIntHandler(void)
{
    //
    // If we have been told to stop feeding the watchdog, return immediately
    // without clearing the interrupt.  This will cause the system to reset
    // next time the watchdog interrupt fires.
    //
    if(!g_bFeedWatchdog)
    {
        return;
    }
    //
    // After 10 interrupts, switch On LED6 to indicate system reset
    // and don't clear watchdog interrupt which causes system reset
    //
    if(g_ulWatchdogCycles >= 10)
    {
        GPIO_IF_LedOn(MCU_RED_LED_GPIO);
        MAP_UtilsDelay(800000);
        return;
    }
    //
    // Clear the watchdog interrupt.
    //
    MAP_WatchdogIntClear(WDT_BASE);
    GPIO_IF_LedOn(MCU_RED_LED_GPIO);
    MAP_UtilsDelay(800000);
    GPIO_IF_LedOff(MCU_RED_LED_GPIO);
    //
    // Increment our interrupt counter.
    //
    g_ulWatchdogCycles++;

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

//*****************************************************************************
//
//!    main function demonstrates the use of the watchdog timer to perform system
//! reset.
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************
void main(void)
{
    tBoolean bRetcode;
    //
    // Initialize the board
    //
    BoardInit();
    //
    // Pinmuxing for LEDs
    //
    PinMuxConfig();
    //
    // configure RED LED
    //
    GPIO_IF_LedConfigure(LED1);

    GPIO_IF_LedOff(MCU_RED_LED_GPIO);

    //
    // Set up the watchdog interrupt handler.
    //
    WDT_IF_Init(WatchdogIntHandler, MILLISECONDS_TO_TICKS(WD_PERIOD_MS));
    
    bRetcode = MAP_WatchdogRunning(WDT_BASE);
    if(!bRetcode)
    {
       WDT_IF_DeInit();
    }

    //
    // Loop forever while the LED winks as watchdog interrupts are handled.
    //
    while(FOREVER)
    {

    }
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
