//*****************************************************************************
// udma_if.c
//
// uDMA interface file
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

#include "hw_types.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_uart.h"
#include "hw_udma.h"
#include "rom.h"
#include "rom_map.h"
#include "gpio.h"
#include "udma.h"
#include "interrupt.h"
#include "prcm.h"
#include "uart.h"
#include "udma_if.h"


//*****************************************************************************
// UDMA Control Table
//*****************************************************************************
#if defined(gcc)
static tDMAControlTable gpCtlTbl[CTL_TBL_SIZE] __attribute__(( aligned(1024)));
#endif
#if defined(ccs)
#pragma DATA_ALIGN(gpCtlTbl, 1024)
tDMAControlTable gpCtlTbl[CTL_TBL_SIZE];
#endif
#if defined(ewarm)
#pragma data_alignment=1024
tDMAControlTable gpCtlTbl[CTL_TBL_SIZE];
#endif


//*****************************************************************************
//
//! Default DMA software interrupt handler
//!
//! \param None
//!
//! This function
//!        1. Invoked when DMA operation is complete
//!
//! \return None.
//
//*****************************************************************************
void
DmaSwIntHandler(void)
{
   MAP_uDMAIntClear(MAP_uDMAIntStatus());
}

//*****************************************************************************
//
//! Default DMA error interrupt handler
//!
//! \param None
//!
//! This function
//!        1. Invoked when DMA operation is in error
//!
//! \return None.
//
//*****************************************************************************
void
DmaErrorIntHandler(void)
{
    MAP_uDMAIntClear(MAP_uDMAIntStatus());
}

//*****************************************************************************
//
//! Initialize the DMA controller
//!
//! \param None
//!
//! This function initializes
//!        1. Initializes the McASP module
//!
//! \return None.
//
//*****************************************************************************
void UDMAInit()
{
    unsigned long ulNdx;

    //
    // Enable McASP at the PRCM module
    //
    MAP_PRCMPeripheralClkEnable(PRCM_UDMA,PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralReset(PRCM_UDMA);

    //
    // Register interrupt handlers
    //
    MAP_uDMAIntRegister(UDMA_INT_SW, DmaSwIntHandler);
    MAP_uDMAIntRegister(UDMA_INT_ERR, DmaErrorIntHandler);

    //
    // Enable uDMA using master enable
    //
    MAP_uDMAEnable();

    //
    // Set Control Table
    //
    for(ulNdx =0; ulNdx < sizeof(gpCtlTbl); ulNdx++)
    {
        *((unsigned char *)gpCtlTbl + ulNdx) = 0;
    }

    MAP_uDMAControlBaseSet(gpCtlTbl);
}


//*****************************************************************************
//
//! De-Initialize the DMA controller
//!
//! \param None
//!
//! This function
//!        1. De-Initializes the uDMA module
//!
//! \return None.
//
//*****************************************************************************
void UDMADeInit()
{
    //
    // UnRegister interrupt handlers
    //
    MAP_uDMAIntUnregister(UDMA_INT_SW);
    MAP_uDMAIntUnregister(UDMA_INT_ERR);
    //
    // Disable the uDMA
    //
    MAP_uDMADisable();
}
//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
