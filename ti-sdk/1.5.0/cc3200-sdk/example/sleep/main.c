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
// Application Name     - Sleep 
// Application Overview - The objective of this application is to showcase the 
//                        sleep power modes supported by the 
//                        CC32xx device using two of the modules-
//                       * WatchDog Timer based Sleep
//                       * General Purpose Timer (GPT) based Sleep
//                        
//                        The device enters the low power modes (sleep) 
//                        on executing the wifi instructions. Also there are some 
//                        pre-requisite settings to be performed before the 
//                        device enters the low power modes.
//
//*****************************************************************************

//****************************************************************************
//
//! \addtogroup sleep
//! @{
//
//****************************************************************************

#include <stdio.h>

// Driverlib includes
#include "hw_types.h"
#include "hw_memmap.h"
#include "hw_apps_rcm.h"
#include "hw_ints.h"
#include "prcm.h"
#include "timer.h"
#include "wdt.h"
#include "rom.h"
#include "rom_map.h"
#include "interrupt.h"
#include "utils.h"
#include "hw_common_reg.h"

// Common interface includes
#include "timer_if.h"
#include "wdt_if.h"
#include "uart_if.h"
#include "pinmux.h"

#define APPLICATION_VERSION     "1.4.0"
#define APP_NAME                "Sleep"
#define ARM_SYSTEM_CTRL_REG     0xE000ED10
#define SYS_CLK                 80000000
#define DBG_PRINT               Report

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
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
//                  LOCAL FUNCTION PROTOTYPES                           
//*****************************************************************************
void AppWDTCallBackHandler();
void AppGPTCallBackHandler();
static void BoardInit();
static void PerformPRCMSleepWDTWakeup();
static void PerformPRCMSleepGPTWakeup();


//*****************************************************************************
//
//! Application callback handler for WDT interrupt
//!
//! \param none
//! 
//! This function  
//!    1. Clears the WDT interrupt
//!
//! \return None.
//
//*****************************************************************************
void AppWDTCallBackHandler()
{
    MAP_WatchdogIntClear(WDT_BASE);
    DBG_PRINT("WDT Interrupt occured\n\r");
}

//*****************************************************************************
//
//! Application callback handler for GPT Timer A0 interrupt
//!
//! \param none
//! 
//! This function  
//!    1. Clears the WDT interrupt
//!
//! \return None.
//
//*****************************************************************************
void AppGPTCallBackHandler()
{
    MAP_TimerIntClear(TIMERA0_BASE,TIMER_TIMB_TIMEOUT|TIMER_TIMA_TIMEOUT);
    DBG_PRINT("GPT TimerA0 Interrupt occured\n\r");
}

//****************************************************************************
//
//! Implements Sleep followed by wakeup using WDT timeout
//!
//! \param none
//! 
//! This function  
//!    1. Implements Sleep followed by wakeup using WDT
//!
//! \return None.
//
//****************************************************************************
void PerformPRCMSleepWDTWakeup()
{
    //
    // Initialize the WDT
    //
    WDT_IF_Init(AppWDTCallBackHandler, (4 * SYS_CLK));

    //
    // Enable the Sleep Clock
    //
    MAP_PRCMPeripheralClkEnable(PRCM_WDT, PRCM_SLP_MODE_CLK);

    //
    // Enter SLEEP...WaitForInterrupt ARM intrinsic
    //
    DBG_PRINT("WDT_SLEEP: Entering Sleep\n\r");
    MAP_UtilsDelay(80000);
    MAP_PRCMSleepEnter();
    DBG_PRINT("WDT_SLEEP: Exiting Sleep\n\r");
    
    //
    // Disable the Sleep Clock
    //
    MAP_PRCMPeripheralClkDisable(PRCM_WDT, PRCM_SLP_MODE_CLK);
    
    //
    // Deinitialize the WDT
    //
    WDT_IF_DeInit();
    
    //
    // PowerOff WDT
    //
    MAP_PRCMPeripheralClkDisable(PRCM_WDT, PRCM_RUN_MODE_CLK);
}

//****************************************************************************
//
//! Implements Sleep followed by wakeup using GPT timeout
//!
//! \param none
//! 
//! This function  
//!    1. Implements Sleep followed by wakeup using GPT
//!
//! \return None.
//
//****************************************************************************
void PerformPRCMSleepGPTWakeup()
{
    //
    // Power On the GPT along with sleep clock
    //
    MAP_PRCMPeripheralClkEnable(PRCM_TIMERA0, PRCM_RUN_MODE_CLK);
    
    //
    // Initialize the GPT as One Shot timer
    //
    Timer_IF_Init(PRCM_TIMERA0, TIMERA0_BASE, TIMER_CFG_ONE_SHOT, TIMER_BOTH, 0);
    Timer_IF_IntSetup(TIMERA0_BASE, TIMER_BOTH, AppGPTCallBackHandler);

	//
	// Start timer with value in mSec
	//
	Timer_IF_Start(TIMERA0_BASE, TIMER_BOTH, 4000);
    
    //
    // Enable the Sleep Clock
    //
    MAP_PRCMPeripheralClkEnable(PRCM_TIMERA0, PRCM_SLP_MODE_CLK);
    
    //
    // Enter SLEEP...WaitForInterrupt ARM intrinsic
    //
    DBG_PRINT("GPT_SLEEP: Entering Sleep\n\r");
    MAP_UtilsDelay(80000);
    MAP_PRCMSleepEnter();
    DBG_PRINT("GPT_SLEEP: Exiting Sleep\n\r");
    
    //
    // Disable the Sleep Clock
    //
    MAP_PRCMPeripheralClkDisable(PRCM_TIMERA0, PRCM_SLP_MODE_CLK);
    
    //
    // Deinitialize the GPT
    //
    Timer_IF_Stop(TIMERA0_BASE, TIMER_BOTH);
    Timer_IF_DeInit(TIMERA0_BASE, TIMER_BOTH);
    
    //
    // PowerOff GPT
    //
    MAP_PRCMPeripheralClkDisable(PRCM_TIMERA0, PRCM_RUN_MODE_CLK);
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

//****************************************************************************
//
//! Main function invoking the sleep functionality
//!
//! \param none
//! 
//! This function  
//!    1. Invokes the API that implements Sleep followed by wakeup using WDT
//!    2. Invokes the API that implements DeepSleep followed by wakeup 
//!       using WDT
//!    3. Invokes the API that implements Sleep followed by wakeup using GPT
//!    4. Invokes the API that implements DeepSleep followed by wakeup 
//!       using GPT
//!
//! \return None.
//
//****************************************************************************
void main()
{
    //
    // Initialize the MCU
    //
    BoardInit();
    
    //
    // Configure the pinmux settings for the peripherals exercised
    //
    PinMuxConfig();
    
    //
    // Initialze the UART
    //
    InitTerm();

    DisplayBanner(APP_NAME);
    
    DBG_PRINT("SLEEP: Test Begin\n\r");
    
    //
    // Sleep followed by wakeup using WDT
    //
    PerformPRCMSleepWDTWakeup();
    
    //
    // Sleep followed by wakeup using GPT
    //
    PerformPRCMSleepGPTWakeup();
    
    DBG_PRINT("SLEEP: Test Complete\n\r");

    //
    // Done. Loop here
    //
    while(1);
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
