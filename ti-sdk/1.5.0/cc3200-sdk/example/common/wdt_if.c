//*****************************************************************************
// wdt_if.c
//
// watchdog timer interface APIs
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

#include <stdio.h>
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "prcm.h"
#include "wdt.h"
#include "rom.h"
#include "rom_map.h"
#include "interrupt.h"
#include "wdt_if.h"
#if defined(USE_TIRTOS) || defined(USE_FREERTOS) || defined(SL_PLATFORM_MULTI_THREADED)
#include "osi.h"
#endif
/****************************************************************************/
/*                       FUNCTION DEFINITIONS                               */
/****************************************************************************/

//****************************************************************************
//
//! Initialize the watchdog timer
//!
//! \param fpAppWDTCB is the WDT interrupt handler to be registered
//! \param uiReloadVal is the reload value to be set to the WDT
//! 
//! This function  
//!        1. Initializes the WDT
//!
//! \return None.
//
//****************************************************************************
void WDT_IF_Init(fAPPWDTDevCallbk fpAppWDTCB,
                   unsigned int uiReloadVal)
{
    //
    // Enable the peripherals used by this example.
    //
    MAP_PRCMPeripheralClkEnable(PRCM_WDT, PRCM_RUN_MODE_CLK);

    //
    // Unlock to be able to configure the registers
    //
    MAP_WatchdogUnlock(WDT_BASE);

    if(fpAppWDTCB != NULL)
    {
#if defined(USE_TIRTOS) || defined(USE_FREERTOS) || defined(SL_PLATFORM_MULTI_THREADED) 
        // USE_TIRTOS: if app uses TI-RTOS (either networking/non-networking)
        // USE_FREERTOS: if app uses Free-RTOS (either networking/non-networking)
        // SL_PLATFORM_MULTI_THREADED: if app uses any OS + networking(simplelink)
        osi_InterruptRegister(INT_WDT, fpAppWDTCB, INT_PRIORITY_LVL_1);
#else
		MAP_IntPrioritySet(INT_WDT, INT_PRIORITY_LVL_1);
        MAP_WatchdogIntRegister(WDT_BASE,fpAppWDTCB);
#endif
    }

    //
    // Set the watchdog timer reload value
    //
    MAP_WatchdogReloadSet(WDT_BASE,uiReloadVal);

    //
    // Start the timer. Once the timer is started, it cannot be disable.
    //
    MAP_WatchdogEnable(WDT_BASE);

}
//****************************************************************************
//
//! DeInitialize the watchdog timer
//!
//! \param None
//! 
//! This function  
//!        1. DeInitializes the WDT
//!
//! \return None.
//
//****************************************************************************
void WDT_IF_DeInit()
{
    //
    // Unlock to be able to configure the registers
    //
    MAP_WatchdogUnlock(WDT_BASE);

    //
    // Disable stalling of the watchdog timer during debug events
    //
    MAP_WatchdogStallDisable(WDT_BASE);

    //
    // Clear the interrupt
    //
    MAP_WatchdogIntClear(WDT_BASE);

    //
    // Unregister the interrupt
    //
    MAP_WatchdogIntUnregister(WDT_BASE);
}
//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
