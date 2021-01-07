//*****************************************************************************
// button_if.c
//
// CC3200 Launchpad button interface APIs
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

#include <stdlib.h>
#include "hw_types.h"
#include "hw_ints.h"
#include "rom.h"
#include "rom_map.h"
#include "gpio.h"
#include "interrupt.h"
#include "pin.h"
#include "hw_memmap.h"
#ifdef SL_PLATFORM_MULTI_THREADED  /* If OS-based application */
#include "osi.h"
#endif
#include "button_if.h"

P_INT_HANDLER g_S2InterruptHdl;
P_INT_HANDLER g_S3InterruptHdl;

//*****************************************************************************
//
//! GPIO Interrupt Handler for S3 button
//!
//! \param None
//!
//! \return None
//
//*****************************************************************************
void GPIOs3IntHandler()
{

    unsigned long ulPinState =  GPIOIntStatus(GPIOA1_BASE,1);

    if(ulPinState & GPIO_PIN_5)
    {
        Button_IF_DisableInterrupt(SW3);
        g_S3InterruptHdl();
    }
}
//*****************************************************************************
//
//! GPIO Interrupt Handler for S2 button
//!
//! \param None
//!
//! \return None
//
//*****************************************************************************
void GPIOs2IntHandler()
{
    unsigned long ulPinState =  GPIOIntStatus(GPIOA2_BASE,1);
    if(ulPinState & GPIO_PIN_6)
    {
        Button_IF_DisableInterrupt(SW2);
        g_S2InterruptHdl();
    }
}

//*****************************************************************************
//
//!  \brief Initialize Push Button GPIO
//!
//! \param[in] S2InterruptHdl          GPIO Interrupt Handler for SW2 LP button
//! \param[in] S3InterruptHdl          GPIO Interrupt Handler for SW3 LP button

//!
//! \return none
//!
//! \brief  Initializes Push Button Ports and Pins
//
//*****************************************************************************
void Button_IF_Init(P_INT_HANDLER S2InterruptHdl,P_INT_HANDLER S3InterruptHdl )
{
    if(S3InterruptHdl != NULL)
    {
        //
        // Set Interrupt Type for GPIO
        //
        MAP_GPIOIntTypeSet(GPIOA1_BASE,GPIO_PIN_5,GPIO_FALLING_EDGE);  
        
        g_S3InterruptHdl = S3InterruptHdl;
        
        //
        // Register Interrupt handler
        //
#if defined(USE_TIRTOS) || defined(USE_FREERTOS) || defined(SL_PLATFORM_MULTI_THREADED) 
    // USE_TIRTOS: if app uses TI-RTOS (either networking/non-networking)
    // USE_FREERTOS: if app uses Free-RTOS (either networking/non-networking)
    // SL_PLATFORM_MULTI_THREADED: if app uses any OS + networking(simplelink)
        osi_InterruptRegister(INT_GPIOA1,(P_OSI_INTR_ENTRY)GPIOs3IntHandler, \
                                INT_PRIORITY_LVL_1);
#else                   
		MAP_IntPrioritySet(INT_GPIOA1, INT_PRIORITY_LVL_1);
        MAP_GPIOIntRegister(GPIOA1_BASE, GPIOs3IntHandler);
#endif    
        //
        // Enable Interrupt
        //
        MAP_GPIOIntClear(GPIOA1_BASE,GPIO_PIN_5);
        MAP_GPIOIntEnable(GPIOA1_BASE,GPIO_INT_PIN_5);
    }
    
    if(S2InterruptHdl != NULL)
    {
        //
        // Set Interrupt Type for GPIO
        //
        MAP_GPIOIntTypeSet(GPIOA2_BASE,GPIO_PIN_6,GPIO_FALLING_EDGE);

        g_S2InterruptHdl = S2InterruptHdl;
        
        //
        // Register Interrupt handler
        //
#if defined(USE_TIRTOS) || defined(USE_FREERTOS) || defined(SL_PLATFORM_MULTI_THREADED) 
    // USE_TIRTOS: if app uses TI-RTOS (either networking/non-networking)
    // USE_FREERTOS: if app uses Free-RTOS (either networking/non-networking)
    // SL_PLATFORM_MULTI_THREADED: if app uses any OS + networking(simplelink)
        osi_InterruptRegister(INT_GPIOA2,(P_OSI_INTR_ENTRY)GPIOs2IntHandler, \
                                INT_PRIORITY_LVL_1);
#else                
		MAP_IntPrioritySet(INT_GPIOA2, INT_PRIORITY_LVL_1);
        MAP_GPIOIntRegister(GPIOA2_BASE, GPIOs2IntHandler);
#endif    
                            
        //
        // Enable Interrupt
        //
        MAP_GPIOIntClear(GPIOA2_BASE,GPIO_PIN_6);
        MAP_GPIOIntEnable(GPIOA2_BASE,GPIO_INT_PIN_6);
    }
}

//*****************************************************************************
//
//!  \brief Enables Push Button GPIO Interrupt
//!
//! \param[in] ucSwitch               Push Button Swich Enum - SW2,SW3
//!
//! \return none
//!
//
//*****************************************************************************
void Button_IF_EnableInterrupt(unsigned char ucSwitch)
{
    if(ucSwitch & SW2)
    {
        //Enable GPIO Interrupt
        MAP_GPIOIntClear(GPIOA2_BASE,GPIO_PIN_6);
        MAP_IntPendClear(INT_GPIOA2);
        MAP_IntEnable(INT_GPIOA2);
        MAP_GPIOIntEnable(GPIOA2_BASE,GPIO_PIN_6);
    }
    
    if(ucSwitch & SW3)
    {
         //Enable GPIO Interrupt 
         MAP_GPIOIntClear(GPIOA1_BASE,GPIO_PIN_5);     
         MAP_IntPendClear(INT_GPIOA1);
         MAP_IntEnable(INT_GPIOA1);
         MAP_GPIOIntEnable(GPIOA1_BASE,GPIO_PIN_5);  
    }
}


//*****************************************************************************
//
//!  \brief Disables Push Button GPIO Interrupt
//!
//! \param[in] ucSwitch               Push Button Swich Enum - SW2,SW3
//!
//! \return none
//!
//
//*****************************************************************************
void Button_IF_DisableInterrupt(unsigned char ucSwitch)
{
    if(ucSwitch & SW2)
    {
        //Clear and Disable GPIO Interrupt
        MAP_GPIOIntDisable(GPIOA2_BASE,GPIO_PIN_6);
        MAP_GPIOIntClear(GPIOA2_BASE,GPIO_PIN_6);
        MAP_IntDisable(INT_GPIOA2);
    }
    
    if(ucSwitch & SW3)
    {
        //Clear and Disable GPIO Interrupt
        MAP_GPIOIntDisable(GPIOA1_BASE,GPIO_PIN_5);
        MAP_GPIOIntClear(GPIOA1_BASE,GPIO_PIN_5);        
        MAP_IntDisable(INT_GPIOA1);  
    }
}
