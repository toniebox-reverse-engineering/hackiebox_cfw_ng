//*****************************************************************************
// timer_if.c
//
// timer interface file: contains different interface functions for timer APIs
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

// Driverlib includes
#include "hw_types.h"
#include "hw_memmap.h"
#include "hw_ints.h"
#include "debug.h"
#include "interrupt.h"
#include "timer.h"
#include "rom.h"
#include "rom_map.h"
#include "prcm.h"

// TI-RTOS includes
#if defined(USE_TIRTOS) || defined(USE_FREERTOS) || defined(SL_PLATFORM_MULTI_THREADED)
#include <stdlib.h>
#include "osi.h"
#endif

#include "timer_if.h"


static unsigned char
GetPeripheralIntNum(unsigned long ulBase, unsigned long ulTimer)
{
    if(ulTimer == TIMER_A)
    {
       switch(ulBase)
       {
           case TIMERA0_BASE:
                 return INT_TIMERA0A;
           case TIMERA1_BASE:
                 return INT_TIMERA1A;
           case TIMERA2_BASE:
                 return INT_TIMERA2A;
           case TIMERA3_BASE:
                 return INT_TIMERA3A;
           default:
                 return INT_TIMERA0A;
           }
    }
    else if(ulTimer == TIMER_B)
    {
       switch(ulBase)
       {
           case TIMERA0_BASE:
                 return INT_TIMERA0B;
           case TIMERA1_BASE:
                 return INT_TIMERA1B;
           case TIMERA2_BASE:
                 return INT_TIMERA2B;
           case TIMERA3_BASE:
                 return INT_TIMERA3B;
           default:
                 return INT_TIMERA0B;
           }
    }
    else
    {
        return INT_TIMERA0A;
    }

}

//*****************************************************************************
//
//!    Initializing the Timer
//!
//! \param ePeripheral is the peripheral which need to be initialized.
//! \param ulBase is the base address for the timer.
//! \param ulConfig is the configuration for the timer.
//! \param ulTimer selects amoung the TIMER_A or TIMER_B or TIMER_BOTH.
//! \param ulValue is the timer prescale value which must be between 0 and
//! 255 (inclusive) for 16/32-bit timers and between 0 and 65535 (inclusive)
//! for 32/64-bit timers.
//! This function
//!     1. Enables and reset the peripheral for the timer.
//!     2. Configures and set the prescale value for the timer.
//!
//! \return none
//
//*****************************************************************************
void Timer_IF_Init( unsigned long ePeripheral, unsigned long ulBase, unsigned
               long ulConfig, unsigned long ulTimer, unsigned long ulValue)
{
    //
    // Initialize GPT A0 (in 32 bit mode) as periodic down counter.
    //
    MAP_PRCMPeripheralClkEnable(ePeripheral, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralReset(ePeripheral);
    MAP_TimerConfigure(ulBase,ulConfig);
    MAP_TimerPrescaleSet(ulBase,ulTimer,ulValue);
}

//*****************************************************************************
//
//!    setting up the timer
//!
//! \param ulBase is the base address for the timer.
//! \param ulTimer selects between the TIMER_A or TIMER_B or TIMER_BOTH.
//! \param TimerBaseIntHandler is the pointer to the function that handles the
//!    interrupt for the Timer
//!
//! This function
//!     1. Register the function handler for the timer interrupt.
//!     2. enables the timer interrupt.
//!
//! \return none
//
//*****************************************************************************
void Timer_IF_IntSetup(unsigned long ulBase, unsigned long ulTimer, 
                   void (*TimerBaseIntHandler)(void))
{
  //
  // Setup the interrupts for the timer timeouts.
  //
#if defined(USE_TIRTOS) || defined(USE_FREERTOS) || defined(SL_PLATFORM_MULTI_THREADED) 
    // USE_TIRTOS: if app uses TI-RTOS (either networking/non-networking)
    // USE_FREERTOS: if app uses Free-RTOS (either networking/non-networking)
    // SL_PLATFORM_MULTI_THREADED: if app uses any OS + networking(simplelink)
      if(ulTimer == TIMER_BOTH)
      {
          osi_InterruptRegister(GetPeripheralIntNum(ulBase, TIMER_A),
                                   TimerBaseIntHandler, INT_PRIORITY_LVL_1);
          osi_InterruptRegister(GetPeripheralIntNum(ulBase, TIMER_B),
                                  TimerBaseIntHandler, INT_PRIORITY_LVL_1);
      }
      else
      {
          osi_InterruptRegister(GetPeripheralIntNum(ulBase, ulTimer),
                                   TimerBaseIntHandler, INT_PRIORITY_LVL_1);
      }
        
#else
	  MAP_IntPrioritySet(GetPeripheralIntNum(ulBase, ulTimer), INT_PRIORITY_LVL_1);
      MAP_TimerIntRegister(ulBase, ulTimer, TimerBaseIntHandler);
#endif
 

  if(ulTimer == TIMER_BOTH)
  {
    MAP_TimerIntEnable(ulBase, TIMER_TIMA_TIMEOUT|TIMER_TIMB_TIMEOUT);
  }
  else
  {
    MAP_TimerIntEnable(ulBase, ((ulTimer == TIMER_A) ? TIMER_TIMA_TIMEOUT : 
                                   TIMER_TIMB_TIMEOUT));
  }
}

//*****************************************************************************
//
//!    clears the timer interrupt
//!
//! \param ulBase is the base address for the timer.
//!
//! This function
//!     1. clears the interrupt with given base.
//!
//! \return none
//
//*****************************************************************************
void Timer_IF_InterruptClear(unsigned long ulBase)
{
    unsigned long ulInts;
    ulInts = MAP_TimerIntStatus(ulBase, true);
    //
    // Clear the timer interrupt.
    //
    MAP_TimerIntClear(ulBase, ulInts);
}

//*****************************************************************************
//
//!    starts the timer
//!
//! \param ulBase is the base address for the timer.
//! \param ulTimer selects amoung the TIMER_A or TIMER_B or TIMER_BOTH.
//! \param ulValue is the time delay in mSec after that run out, 
//!                 timer gives an interrupt.
//!
//! This function
//!     1. Load the Timer with the specified value.
//!     2. enables the timer.
//!
//! \return none
//!
//! \Note- HW Timer runs on 80MHz clock 
//
//*****************************************************************************
void Timer_IF_Start(unsigned long ulBase, unsigned long ulTimer, 
                unsigned long ulValue)
{
    MAP_TimerLoadSet(ulBase,ulTimer,MILLISECONDS_TO_TICKS(ulValue));
    //
    // Enable the GPT 
    //
    MAP_TimerEnable(ulBase,ulTimer);
}

//*****************************************************************************
//
//!    disable the timer
//!
//! \param ulBase is the base address for the timer.
//! \param ulTimer selects amoung the TIMER_A or TIMER_B or TIMER_BOTH.
//!
//! This function
//!     1. disables the interupt.
//!
//! \return none
//
//*****************************************************************************
void Timer_IF_Stop(unsigned long ulBase, unsigned long ulTimer)
{
    //
    // Disable the GPT 
    //
    MAP_TimerDisable(ulBase,ulTimer);
}

//*****************************************************************************
//
//!    De-Initialize the timer
//!
//! \param uiGPTBaseAddr
//! \param ulTimer
//!
//! This function 
//!        1. disable the timer interrupts
//!        2. unregister the timer interrupt
//!
//!    \return None.
//
//*****************************************************************************
void Timer_IF_DeInit(unsigned long ulBase,unsigned long ulTimer)
{
    //
    // Disable the timer interrupt
    //
    MAP_TimerIntDisable(ulBase,TIMER_TIMA_TIMEOUT|TIMER_TIMB_TIMEOUT);
    //
    // Unregister the timer interrupt
    //
    MAP_TimerIntUnregister(ulBase,ulTimer);
}

//*****************************************************************************
//
//!    starts the timer
//!
//! \param ulBase is the base address for the timer.
//! \param ulTimer selects between the TIMER A and TIMER B.
//! \param ulValue is timer reload value (mSec) after which the timer will run out and gives an interrupt.
//!
//! This function
//!     1. Reload the Timer with the specified value.
//!
//! \return none
//
//*****************************************************************************
void Timer_IF_ReLoad(unsigned long ulBase, unsigned long ulTimer, 
                unsigned long ulValue)
{
    MAP_TimerLoadSet(ulBase,ulTimer,MILLISECONDS_TO_TICKS(ulValue));
}

//*****************************************************************************
//
//!    starts the timer
//!
//! \param ulBase is the base address for the timer.
//! \param ulTimer selects amoung the TIMER_A or TIMER_B or TIMER_BOTH.
//!
//! This function
//!     1. returns the timer value.
//!
//! \return Timer Value.
//
//*****************************************************************************
unsigned int Timer_IF_GetCount(unsigned long ulBase, unsigned long ulTimer)
{
    unsigned long ulCounter;
    ulCounter = MAP_TimerValueGet(ulBase, ulTimer);
    return 0xFFFFFFFF - ulCounter;
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

