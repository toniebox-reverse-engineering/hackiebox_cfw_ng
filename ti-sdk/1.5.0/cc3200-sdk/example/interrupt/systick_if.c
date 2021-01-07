//*****************************************************************************
// systick_if.c 
//
// API(s) for SysTick Utilities
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
//! \addtogroup InterruptsReferenceApp
//! @{
//
//*****************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hw_ints.h"
#include "hw_nvic.h"
#include "hw_memmap.h"
#include "hw_types.h"
#include <debug.h>
#include "rom.h"
#include "rom_map.h"
#include "gpio.h"
#include "uart.h"
#include "prcm.h"
#include "interrupt.h"
#include "systick.h"
#include "systick_if.h"

//*****************************************************************************
//
// Fudge factor used to correct for timing changes in SysCtlDelay caused by
// turning on the prefetch buffer.  The number of cycles delayed is multiplied
// by (PREFETCH_FUDGE_NUM/PREFETCH_FUDGE_DEN) if the prefetch buffer is
// enabled.
//
//*****************************************************************************
#define PREFETCH_FUDGE_NUM  9
#define PREFETCH_FUDGE_DEN 10

//*****************************************************************************
//
// Variables and labels related to our system tick counter. Note that
// SYSTICK_RELOAD_VALUE must be < 0x418937 or TICKS_TO_MILLISECONDS will
// return a bogus value thanks to an overflow.
//
//*****************************************************************************
#define SYSTICK_RELOAD_VALUE    0x400000
#define SYSTICK_RELOAD_MS       TICKS_TO_MILLISECONDS(SYSTICK_RELOAD_VALUE)
#define SYSTICK_CYCLE_ROUNDING  (SYSTICK_RELOAD_VALUE -                    \
                                 MILLISECONDS_TO_TICKS(SYSTICK_RELOAD_MS))

static volatile unsigned long g_ulSysTickCount = 0;
static volatile unsigned long g_ulSysTimeError = 0;
static volatile unsigned long g_ulSysTime = 0;
static unsigned long g_ulSysTickMs;
static volatile unsigned long g_ulSysTickRounding;
extern tBoolean g_bVerbose;

//*****************************************************************************
// Timer count values.
//*****************************************************************************
static volatile unsigned long g_ulTimerSysTicks;

//*****************************************************************************
//
// System clock rate. We read this once using SysCtlClockGet to save having
// to make multiple function calls in the TICKS_TO_MILLISECONDS and
// TICKS_TO_MICROSECONDS macros.
//
//*****************************************************************************
unsigned long g_ulClockRate;

//*****************************************************************************
//
// The number of processor cycles to execute one pass of the delay loop.
//
//*****************************************************************************
#define DELAY_LOOP_INSTRUCTIONS 3



//*****************************************************************************
//
//! The handler for the SysTick interrupt.
//!
//! \param None
//!
//! \return None
//!
//*****************************************************************************
void
SysTickHandler(void)
{
    //
    // Increment the SysTick count, elapsed time and cycle error values.
    //
    g_ulSysTickCount++;
    g_ulSysTime += g_ulSysTickMs;
    g_ulSysTimeError += SYSTICK_CYCLE_ROUNDING;

    //
    // If the accumulated cycle error exceeds 1 tick, adjust the counts
    // accordingly.
    //
    if(g_ulSysTimeError >= SYSTICK_RELOAD_VALUE)
    {
        g_ulSysTime += g_ulSysTickMs;
        g_ulSysTimeError -= SYSTICK_RELOAD_VALUE;
        g_ulSysTickCount++;
    }

    //
    // Increment the wraps if we are using a timer API.
    //
    g_ulTimerSysTicks++;
}

//*****************************************************************************
//
//! SysTick Initialization 
//!
//! \param None
//!
//! \return None
//!
//*****************************************************************************
tBoolean
SysTickInit(void)
{
    //
    // Query the current system clock rate.
    //
    g_ulClockRate = 80000000;

    //
    // Set up for the system tick calculations. We keep copies of the number
    // of milliseconds per cycle and the rounding error to prevent the need
    // to perform these calculations on every interrupt (the macros are not
    // merely static values).
    //
    g_ulSysTickCount = 0;
    g_ulSysTickMs = SYSTICK_RELOAD_MS;
    g_ulSysTickRounding = SYSTICK_CYCLE_ROUNDING;

    //
    // Set up the system tick to run and interrupt when it times out.
    //
    SysTickIntEnable();
    SysTickPeriodSet(SYSTICK_RELOAD_VALUE);
    SysTickEnable();

    return(true);
}

//*****************************************************************************
//
//! Get the elapsed time since the application started.
//!
//! \param None
//!
//! This function returns the number of milliseconds since the application
//! started running. The granularity of the timing is MILLISECONDS_PER_TICK,
//! (currently set to 100).
//!
//! \return The number of milliseconds that the application has been running.
//*****************************************************************************
unsigned long
UTUtilsGetSysTime(void)
{
    unsigned long ulTickVal1;
    unsigned long ulTickVal2;
    unsigned long ulExtraTicks;
    unsigned long ulCycleTicks;
    unsigned long ulCycleMs;
    unsigned long ulSysTime;

    //
    // Read the system tick value and take a snapshot of the wrap count. We
    // read the tick value twice to determine if there is a chance that we
    // wrapped during the process.
    //
    do
    {
        ulTickVal1 = SysTickValueGet();
        ulCycleMs = g_ulSysTime;
        ulExtraTicks = g_ulSysTimeError;
        ulTickVal2 = SysTickValueGet();
    }
    while (ulTickVal2 > ulTickVal1);

    //
    // Determine how many ticks the systick has ticked since it last timed
    // out. This value must be less than the reload period so is safe to
    // pass to TICKS_TO_MILLISECONDS.
    //
    ulCycleTicks = SYSTICK_RELOAD_VALUE - ulTickVal2;

    //
    // The time to return is the total time for completed cycles (ulCycleMs)
    // plus the time since the last timeout (ulCycleTicks) plus the
    // outstanding accumulated error (ulExtraTicks).
    //
    ulSysTime = ulCycleMs + TICKS_TO_MILLISECONDS(ulCycleTicks) +
                            TICKS_TO_MILLISECONDS(ulExtraTicks);

    //
    // Return the calculated time.
    //
    return(ulSysTime);
}

//*****************************************************************************
//
//! Waits for the value of a counter to reach or exceed a target value.
//!
//! \param pulCount is a pointer to the counter variable whose value is to be
//! polled until it reaches or exceeds the supplied target value.
//! \param ulTarget is the value that /b *pulCount must reach before the
//! the function will return /e true.
//! \param ulTimeoutMs is the maximum time that the function will wait for
//! the target to be reached before returning /e false.
//!
//! This function polls the value of a variable supplied in /b pulCount,
//! waiting for it to reach or exceed /b ulTarget. If the target is reached
//! within /b ulTimeoutMs milliseconds, the function returns /e true. If the
//! timeout is reached before the target value is reached, /e false is
//! returned.
//!
//! \return Returns /e true if the target value is reached or exceeded or
//! /e false if the function times out before this occurs.
//
//*****************************************************************************
tBoolean
UTUtilsWaitForCount(volatile unsigned long *pulCount,
                    unsigned long ulTarget,
                    unsigned long ulTimeoutMs)
{
    unsigned long ulTimeToEnd;
    static unsigned long ulSysTimeStart;
    
    //
    // When should we stop polling?
    //
    ulSysTimeStart = UTUtilsGetSysTime();

    ulTimeToEnd = ulSysTimeStart + ulTimeoutMs;

    //
    // Keep checking for the target value being reached until we run out
    // of time.
    //

    //
    // This is not completely safe since it will not handle rollover well
    // but this will only be a problem if the function is called during a
    // ulTimeoutMs millisecond window 49.7 days after the application
    // starts. If it takes us a month and a half to run our testcase, we
    // have far bigger things to worry about than this.
    //
    
    while(UTUtilsGetSysTime() < ulTimeToEnd)
    {
        //
        // Read the counter and see if it got to the target value yet
        //
        if(*pulCount >= ulTarget)
        {
            //
            // Target reached so return true
            //
           return(true);
        }
    }

    return(false);
}

//*****************************************************************************
//
//! Delays execution for a given number of clock cycles.
//!
//! \param ulTimeoutCycles is the number of cycles to wait before
//! returning to the caller.
//!
//! This function delays for a given number of clock cycles. The delay is
//! is implemented using a loop of known length but the actual delay will be
//! approximate, especially if interrupts are active.
//!
//! \return None.
//
//*****************************************************************************
#if defined(ccs)
__asm("    .sect \".text:SysCtlDelay\"\n"
"    .clink\n"
"    .thumbfunc SysCtlDelay\n"
"    .thumb\n"
"    .global SysCtlDelay\n"
"SysCtlDelay:\n"
"    subs r0, #1\n"
"    bne.n SysCtlDelay\n"
"    bx lr\n");
#endif


#if defined(ewarm)

void
SysCtlDelay(unsigned long ulCount)
{
    __asm("    subs    r0, #1\n"
          "    bne.n   SysCtlDelay\n"
          "    bx      lr");
}
#endif
void
UTUtilsDelayCycles(unsigned long ulTimeoutCycles)
{
    //
    // Call the assembler delay function to spin uselessly for the desired
    // period of time.
    //
#ifdef ENABLE_PREFETCH
    SysCtlDelay((ulTimeoutCycles * PREFETCH_FUDGE_NUM)/
                (DELAY_LOOP_INSTRUCTIONS * PREFETCH_FUDGE_DEN));
#else
    SysCtlDelay(ulTimeoutCycles / DELAY_LOOP_INSTRUCTIONS);
#endif

    return;
}

//*****************************************************************************
//
//! Delays execution for a give number of microseconds.
//!
//! \param ulTimeoutMicroS is the number of microseconds to wait before
//! returning to the caller.
//!
//! This function delays for a given number of microseconds. The delay is
//! calculated from the current system clock and is implemented using a loop
//! of known length. The actual delay will be approximate.
//!
//! The valid range for ulTimeMicroS is from 0 to 8500000 (0 to 8.5 seconds).
//! Values of ulTimeMicroS greater than 8.5 million will cause overflow
//! resulting in delays significantly shorter than expected.
//!
//! \return None.
//
//*****************************************************************************
void
UTUtilsDelay(unsigned long ulTimeoutMicroS)
{
    unsigned long ulDelay;
    unsigned long ulClock;

    //
    // Determine the current clock rate in Hz
    //
    ulClock = MAP_SysCtlClockGet;

    //
    // Calculate the number of cycles we need to burn to achieve the
    // desired delay making sure that we don't overflow or underflow
    // if ulTimeoutMicroS is within the supported range.
    //
    ulDelay = (((ulClock / 100000) * ulTimeoutMicroS) / 10);

    //
    // Call the assembler delay function to spin uselessly for the desired
    // period of time.
    //
    UTUtilsDelayCycles(ulDelay);
    return;
}


//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
