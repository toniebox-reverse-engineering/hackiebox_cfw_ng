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
// Application Name     - Interrupt Demo Application
// Application Overview - The objective of this application is to showcase 
//                        interrupt preemption and tail-chaining capabilities. 
//                        Nested interrupts are synthesized when the interrupts 
//                        have the same priority, increasing priorities and 
//                        decreasing priorities. With increasing priorities, 
//                        preemption will occur; in the other two cases tail-
//                        chaining will occur.
//
//*****************************************************************************

//*****************************************************************************
//
//! \addtogroup InterruptsReferenceApp
//! @{
//
//*****************************************************************************

// Standard includes
#include <stdlib.h>
#include <string.h>

// Driverlib includes
#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "hw_types.h"
#include "hw_timer.h"
#include "uart.h"
#include "prcm.h"
#include "rom.h"
#include "rom_map.h"
#include "utils.h"
#include "timer.h"
#include "interrupt.h"

// Common interface includes
#include "systick_if.h"
#include "uart_if.h"

#include "pinmux.h"

#define APPLICATION_VERSION  "1.4.0"
#define APP_NAME             "Interrupt Reference"
#define SYSCLK               80000000
#define UART_PRINT           Report

// This is the time we delay inside the A0 timer interrupt handler.
#define SLOW_TIMER_DELAY_uS 2000

// Interrupt priorities used within the test.
#define LOW_PRIORITY  0xFF
#define HIGH_PRIORITY 0x00
#define MIDDLE_PRIORITY 0x80

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************

// Globals used to track calls to the test interrupt handlers.
static volatile unsigned long g_ulA0IntCount;
static volatile unsigned long g_ulA1IntCount;
static volatile unsigned long g_ulA2IntCount;
static volatile tBoolean g_bA1CountChanged;
static volatile tBoolean g_bA2CountChanged;

// Globals used to save and restore interrupt settings.
static unsigned long g_ulTimer0APriority;
static unsigned long g_ulTimer1APriority;
static unsigned long g_lPriorityGrouping;

#if defined(ccs)
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
//! \InterruptTestInit
//!
//! Performs  initialization.
//! This function is called at the start of the test to enable any peripherals
//! used during the test.
//!
//! \param None.
//!
//! \return None.
//
//*****************************************************************************
static void
InterruptTestInit(void)
{
    //
    // Enable and Reset the timer blocks
    //
    MAP_PRCMPeripheralClkEnable(PRCM_TIMERA0, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralClkEnable(PRCM_TIMERA1, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralClkEnable(PRCM_TIMERA2, PRCM_RUN_MODE_CLK);

    MAP_PRCMPeripheralReset(PRCM_TIMERA0);
    MAP_PRCMPeripheralReset(PRCM_TIMERA1);
    MAP_PRCMPeripheralReset(PRCM_TIMERA2);

    //
    // Remember the timer interrupt priorities and priority group settings as
    // they are on entry so that we can restore them on exit.
    //
    g_ulTimer0APriority = MAP_IntPriorityGet(INT_TIMERA0A);
    g_ulTimer1APriority = MAP_IntPriorityGet(INT_TIMERA1A);
    g_lPriorityGrouping = MAP_IntPriorityGroupingGet();
    
    SysTickInit();

}

//*****************************************************************************
//
//! \InterruptTestTerm
//! Performs termination processing.
//! This function is called once the test completes to tidy up as required.
//!
//! \param None.
//!
//! \return None.
//
//*****************************************************************************
static void
InterruptTestTerm(void)
{

    //
    // Unhook our interrupt handlers if they are still hooked.
    //
    TimerIntUnregister(TIMERA0_BASE, TIMER_A);
    TimerIntUnregister(TIMERA1_BASE, TIMER_A);
    TimerIntUnregister(TIMERA2_BASE, TIMER_A);

    //
    // Restore the original timer interrupt priorities and priority group
    // settings.
    //
    MAP_IntPriorityGroupingSet(g_lPriorityGrouping);
    MAP_IntPrioritySet(INT_TIMERA0A, (unsigned char)g_ulTimer0APriority);
    MAP_IntPrioritySet(INT_TIMERA1A, (unsigned char)g_ulTimer1APriority);

    //
    // Reset and Disable the timer blocks
    //
    MAP_PRCMPeripheralReset(PRCM_TIMERA0);
    MAP_PRCMPeripheralReset(PRCM_TIMERA1);
    MAP_PRCMPeripheralReset(PRCM_TIMERA2);

    MAP_PRCMPeripheralClkDisable(PRCM_TIMERA0, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralClkDisable(PRCM_TIMERA1, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralClkDisable(PRCM_TIMERA2, PRCM_RUN_MODE_CLK);

}

//*****************************************************************************
//
//! \TimerA0IntHandler
//!
//! Handles interrupts for Timer A0, 
//!
//! \param None.
//!
//! \return None.
//
//*****************************************************************************
static void
TimerA0IntHandler(void)
{
    unsigned long ulStatus;
    unsigned long ulCountOnEntry;
    
    //
    // Clear all interrupts for Timer unit 0.
    //
    ulStatus = MAP_TimerIntStatus(TIMERA0_BASE, true);
    MAP_TimerIntClear(TIMERA0_BASE, ulStatus);
    
    //
    // Take a snapshot of the other interrupt's counter.
    //
    ulCountOnEntry = g_ulA1IntCount;
    MAP_TimerEnable(TIMERA1_BASE, TIMER_A);
    
    //
    // We wait 2mS to give the A1 interrupt a chance to fire. 
    //
    UTUtilsDelay(SLOW_TIMER_DELAY_uS);
    
    //
    // Determine whether the Timer A1 interrupt handler was processed during our
    // delay and set the global flag accordingly.
    //
    if(ulCountOnEntry == g_ulA1IntCount)
    {
        g_bA1CountChanged = false;
    }
    else
    {
        g_bA1CountChanged = true;
    }

    //
    // Increment our interrupt counter.
    //
    g_ulA0IntCount++;

    //
    // Alert to the user
    //
    UART_PRINT("Completed TimerA0 Interrupt Handler \n\r");
}

//*****************************************************************************
//
//! \TimerA1IntHandler
//!
//! Handles interrupts for Timer A1.
//! This interrupt handler clears the source of the interrupt, calls Timer A2,
//! increments a counter and returns.
//!
//! \param None.
//!
//! \return None.
//
//*****************************************************************************
static void
TimerA1IntHandler(void)
{
    unsigned long ulStatus;
    unsigned long ulCountOnEntry;

    //
    // Clear all interrupts for Timer.
    //
    ulStatus = MAP_TimerIntStatus(TIMERA1_BASE, true);
    MAP_TimerIntClear(TIMERA1_BASE, ulStatus);

    ulCountOnEntry=g_ulA2IntCount;
    MAP_TimerEnable(TIMERA2_BASE, TIMER_A);

    //
    // We wait 2mS to give Timer A2 interrupt a chance to fire 
    //
    UTUtilsDelay(SLOW_TIMER_DELAY_uS);

    //
    // Determine whether the fast interrupt handler was processed during our
    // delay and set the global flag accordingly.
    //
    if(ulCountOnEntry==g_ulA2IntCount)
    {
      g_bA2CountChanged=false;

    }
    else
    {
      g_bA2CountChanged=true;
    }

    //
    // Increment our global interrupt counter.
    //
    g_ulA1IntCount++;
    //
    // Alert to the User
    //
    UART_PRINT("Completed TimerA1 Interrupt Handler \n\r");
}

//*****************************************************************************
//
//! \TimerA2IntHandler
//!
//! Handles interrupts for Timer A2.
//! This interrupt handler clears the source of the interrupt and
//! increments a counter and returns.
//!
//! \param None.
//!
//! \return None.
//
//*****************************************************************************
static void
TimerA2IntHandler(void)
{
    unsigned long ulStatus;
    
    //
    // Clear all interrupts for Timer. 
    //
    ulStatus = MAP_TimerIntStatus(TIMERA2_BASE, true);
    MAP_TimerIntClear(TIMERA2_BASE, ulStatus);

    //
    // Increment our global interrupt counter.
    //
    g_ulA2IntCount++;
    
    //
    // Alert to the User
    //
    UART_PRINT("Completed TimerA2 Interrupt Handler \n\r");
}

//*****************************************************************************
//
//! \PerformIntTest
//!
//! Performs the repeated steps in running each test scenario.
//!
//! \param ucPriorityA0 is the interrupt priority to be used for Timer A0
//! \param ucPriorityA1 is the interrupt priority to be used for Timer A1
//! \param ucPriorityA2 is the interrupt priority to be used for Timer A2
//!
//! This function performs all the steps which are common to each test scenario
//! inside function InterruptTest.
//!
//! \return None.
//
//*****************************************************************************
tBoolean
PerformIntTest(unsigned long ulPriBits, unsigned char ucPriorityA0,
       unsigned char ucPriorityA1,unsigned char ucPriorityA2)
{
    tBoolean bRetcode;
    unsigned long ulStatus;
    
    //
    // Set the appropriate interrupt priorities.
    //
    MAP_IntPriorityGroupingSet(ulPriBits);
    MAP_IntPrioritySet(INT_TIMERA0A, ucPriorityA0);
    MAP_IntPrioritySet(INT_TIMERA1A, ucPriorityA1);
    MAP_IntPrioritySet(INT_TIMERA2A, ucPriorityA2);
    
    //
    // Clear any pending timer interrupts
    //
    ulStatus = MAP_TimerIntStatus(TIMERA0_BASE, false);
    MAP_TimerIntClear(TIMERA0_BASE, ulStatus);
    ulStatus = MAP_TimerIntStatus(TIMERA1_BASE, false);
    MAP_TimerIntClear(TIMERA1_BASE, ulStatus);
    ulStatus = MAP_TimerIntStatus(TIMERA2_BASE, false);
    MAP_TimerIntClear(TIMERA2_BASE, ulStatus);
    
    //
    // Clear all the counters and flags used by the interrupt handlers.
    //
    g_ulA0IntCount = 0;
    g_ulA1IntCount = 0;
    g_ulA2IntCount=0;
    g_bA1CountChanged = false;
    
    //
    // Enable three timer interrupts
    //
    MAP_TimerIntEnable(TIMERA0_BASE, TIMER_TIMA_TIMEOUT);
    MAP_TimerIntEnable(TIMERA1_BASE, TIMER_TIMA_TIMEOUT);
    MAP_TimerIntEnable(TIMERA2_BASE, TIMER_TIMA_TIMEOUT);
    
    //
    // Enable Timer A0
    //
    MAP_TimerEnable(TIMERA0_BASE, TIMER_A);
    
    //
    // Wait for Timer 0/A to fire.
    //
    bRetcode = UTUtilsWaitForCount(&g_ulA0IntCount, 1,
                                       ((SLOW_TIMER_DELAY_uS*3)/1000));
       
  
    //
    // Stop All timers and disable their interrupts
    //
    MAP_TimerDisable(TIMERA2_BASE, TIMER_A);
    MAP_TimerDisable(TIMERA1_BASE, TIMER_A);
    MAP_TimerDisable(TIMERA0_BASE, TIMER_A);
    MAP_TimerIntDisable(TIMERA2_BASE, TIMER_TIMA_TIMEOUT);
    MAP_TimerIntDisable(TIMERA1_BASE, TIMER_TIMA_TIMEOUT);
    MAP_TimerIntDisable(TIMERA0_BASE, TIMER_TIMA_TIMEOUT);

    return(bRetcode);
}

//*****************************************************************************
//
//! Main test implementation function 
//!
//! It uses 3 different timers to test that interrupts can be enabled
//! and disabled successfully and that preemption occurs as expected when
//! different priority levels are assigned.
//
//! \return Returns 1 on Success.
//
//*****************************************************************************
int
DoInterruptTest()
{
    tTestResult eResult;
    
    //
    // indicate that the test is started.
    //
    UART_PRINT("Interrupt test starting...\n\r\n\r");
    
    //
    // Assume we will pass until we determine otherwise.
    //
    eResult = TEST_PASSED;
    
    //
    // Perform any required initialization prior to performing the test.
    //
    InterruptTestInit();
    
    //
    // Set up the 3 timers we use in this test. Timer A0 - 500uS.
    // Timer A1 - 500 uS. Timer A2 - 500 uS
    //
    MAP_TimerConfigure(TIMERA0_BASE, TIMER_CFG_ONE_SHOT);
    MAP_TimerLoadSet(TIMERA0_BASE, TIMER_A,
                     MICROSECONDS_TO_TICKS(SLOW_TIMER_DELAY_uS/4));

    MAP_TimerConfigure(TIMERA1_BASE, TIMER_CFG_ONE_SHOT);
    MAP_TimerLoadSet(TIMERA1_BASE, TIMER_A,
                     MICROSECONDS_TO_TICKS(SLOW_TIMER_DELAY_uS/4));

    MAP_TimerConfigure(TIMERA2_BASE, TIMER_CFG_ONE_SHOT);
    MAP_TimerLoadSet(TIMERA2_BASE, TIMER_A,
                     MICROSECONDS_TO_TICKS(SLOW_TIMER_DELAY_uS/4));

    //
    // Hook our interrupt handlers
    //
    MAP_TimerIntRegister(TIMERA0_BASE, TIMER_A, TimerA0IntHandler);
    MAP_TimerIntRegister(TIMERA1_BASE, TIMER_A, TimerA1IntHandler);
    MAP_TimerIntRegister(TIMERA2_BASE, TIMER_A, TimerA2IntHandler);

    UART_PRINT("Equal Priorities Testing. A0=A1=A2 \n\r");
    UART_PRINT("Interrupt Triggering Order A0 => A1 => A2 \n\r");
    UART_PRINT("**********************************************\n\r");
    
    //
    // Scenario 1 - set three timers to the same interrupt priority.
    // In this case, we expect to see that the order of execuition of
    // interrupts are A0,A1,A2.
    //
    PerformIntTest(3, LOW_PRIORITY, LOW_PRIORITY,LOW_PRIORITY);

    //
    // Checking the Order of Execuition. A0,A1,A2.
    //
    if((g_ulA1IntCount == 0 && g_ulA2IntCount==0 ) || g_bA1CountChanged ||
       g_bA2CountChanged )
    {
        //
        // Something went wrong. Fail the test.
        //
        UART_PRINT("Interrupt failure.\n\r");
        eResult = TEST_FAILED;
        return 0;
    }

    //
    // Scenario 2 : Increasing Priorities
    // Priorities are set as: A0 < A1 < A2
    //
    if(eResult == TEST_PASSED)
    {

        UART_PRINT("Succesfully executed Equal Priority \n\r\n\r");
        UART_PRINT("Increasing Priority :  A0 < A1 < A2\n\r");
        UART_PRINT("Interrupt Triggering Order A0 => A1 => A2 \n\r");
        UART_PRINT("**********************************************\n\r");
    
        //
        // Perform the test for this scenario.
        //
        PerformIntTest(3, LOW_PRIORITY,MIDDLE_PRIORITY,
                                  HIGH_PRIORITY);

        //
        // Order of Execuition should be as: A2,A1,A0.
        //
        if((g_ulA1IntCount == 0) || !g_bA1CountChanged || !g_bA2CountChanged)
                                                                  
        {
            //
            // Something went wrong. Fail the test.
            //
            UART_PRINT("Interrupt failure. \n\r");
            eResult = TEST_FAILED;
            return 0;
        }
    }

    //
    // Scenario 3 - Decreasing Priorities
    // Priorities are set as: A0 > A1 > A2
    //
    if(eResult == TEST_PASSED)
    {
        UART_PRINT("Succesfully executed Increasing Priority \n\r\n\r");
        UART_PRINT("Decreasing Priority :  A0 > A1 > A2\n\r");
        UART_PRINT("Interrupt Triggering Order A0 => A1 => A2 \n\r");
        UART_PRINT("**********************************************\n\r");

        //
        // Perform the test for this scenario.
        //
        PerformIntTest(3, HIGH_PRIORITY, MIDDLE_PRIORITY, LOW_PRIORITY);
        
        //
        // Order of execuition should be : A0,A1,A2
        //
        if((g_ulA1IntCount == 0) || g_bA1CountChanged || g_bA2CountChanged)
        {
            //
            // Something went wrong. Fail the test.
            //
            UART_PRINT("Interrupt failure\n\r");

            eResult = TEST_FAILED;
            return 0;
        }
    }

    UART_PRINT("Succesfully executed Decreasing Priority \n\r\n\r");

    //
    // Perform any clean up required after the test has completed.
    //
    InterruptTestTerm();
    
    //
    // Let the user know that this test has completed.
    //
    UART_PRINT("Interrupt test ended.\n\r");
    return 1;
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
//! Main function handling the interrupt example
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
void
main()
{
    //
    // Initialize board configurations
    BoardInit();
    PinMuxConfig();
    
    //
    // Configuring UART
    //
    InitTerm();

    //
    // Display Banner
    //
    DisplayBanner(APP_NAME);
    
    //
    // Do Interrupt Test
    //
    DoInterruptTest();
    while(1);
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
