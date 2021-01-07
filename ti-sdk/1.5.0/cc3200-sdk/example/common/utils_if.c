//*****************************************************************************
//  utils_if.c
//
//  Contains utility routines.
//
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
#include <stdint.h>
#include <stdbool.h>
#include "hw_types.h"
#include "hw_memmap.h"
#include "rom.h"
#include "rom_map.h"
#include "hwspinlock.h"
#include "prcm.h"
#include "spi.h"

//*****************************************************************************
//
//! \addtogroup utils_if_api
//! @{
//
//*****************************************************************************


//*****************************************************************************
// Defines.
//*****************************************************************************
#define INSTR_READ_STATUS       0x05
#define INSTR_DEEP_POWER_DOWN   0xB9
#define STATUS_BUSY             0x01
#define MAX_SEMAPHORE_TAKE_TRIES (4000000)

//****************************************************************************
//
//! Put SPI flash into Deep Power Down mode
//!
//! Note:SPI flash is a shared resource between MCU and Network processing 
//!      units. This routine should only be exercised after all the network
//!      processing has been stopped. To stop network processing use sl_stop API
//! \param None
//!
//! \return Status, 0:Pass, -1:Fail
//
//****************************************************************************
int Utils_SpiFlashDeepPowerDown(void)
{
    unsigned long ulStatus=0;

    //
    // Acquire SSPI HwSpinlock.
    //
    if (0 != MAP_HwSpinLockTryAcquire(HWSPINLOCK_SSPI, MAX_SEMAPHORE_TAKE_TRIES))
    {
        return -1;
    }

    //
    // Enable clock for SSPI module
    //
    MAP_PRCMPeripheralClkEnable(PRCM_SSPI, PRCM_RUN_MODE_CLK);

    //
    // Reset SSPI at PRCM level and wait for reset to complete
    //
    MAP_PRCMPeripheralReset(PRCM_SSPI);
    while(MAP_PRCMPeripheralStatusGet(PRCM_SSPI)== false)
    {
    }

    //
    // Reset SSPI at module level
    //
    MAP_SPIReset(SSPI_BASE);

    //
    // Configure SSPI module
    //
    MAP_SPIConfigSetExpClk(SSPI_BASE,PRCMPeripheralClockGet(PRCM_SSPI),
                     20000000,SPI_MODE_MASTER,SPI_SUB_MODE_0,
                     (SPI_SW_CTRL_CS |
                     SPI_4PIN_MODE |
                     SPI_TURBO_OFF |
                     SPI_CS_ACTIVELOW |
                     SPI_WL_8));

    //
    // Enable SSPI module
    //
    MAP_SPIEnable(SSPI_BASE);

    //
    // Enable chip select for the spi flash.
    //
    MAP_SPICSEnable(SSPI_BASE);

    //
    // Wait for spi flash.
    //
    do
    {
        //
        // Send the status register read instruction and read back a dummy byte.
        //
        MAP_SPIDataPut(SSPI_BASE,INSTR_READ_STATUS);
        MAP_SPIDataGet(SSPI_BASE,&ulStatus);

        //
        // Write a dummy byte then read back the actual status.
        //
        MAP_SPIDataPut(SSPI_BASE,0xFF);
        MAP_SPIDataGet(SSPI_BASE,&ulStatus);
    }
    while((ulStatus & 0xFF )== STATUS_BUSY);

    //
    // Disable chip select for the spi flash.
    //
    MAP_SPICSDisable(SSPI_BASE);

    //
    // Start another CS enable sequence for Power down command.
    //
    MAP_SPICSEnable(SSPI_BASE);

    //
    // Send Deep Power Down command to spi flash
    //
    MAP_SPIDataPut(SSPI_BASE,INSTR_DEEP_POWER_DOWN);

    //
    // Disable chip select for the spi flash.
    //
    MAP_SPICSDisable(SSPI_BASE);

    //
    // Release SSPI HwSpinlock.
    //
    MAP_HwSpinLockRelease(HWSPINLOCK_SSPI);

    //
    // Return as Pass.
    //
    return 0;
}

//****************************************************************************
//
//! Used to trigger a hibernate cycle for the SOC
//!
//! Note:This routine should only be exercised after all the network
//!      processing has been stopped. To stop network processing use sl_stop API
//! \param None
//!
//! \return None
//
//****************************************************************************
void Utils_TriggerHibCycle(void)
{
    //
    // Configure the HIB module RTC wake time
    //
    MAP_PRCMHibernateIntervalSet(330);

    //
    // Enable the HIB RTC
    //
    MAP_PRCMHibernateWakeupSourceEnable(PRCM_HIB_SLOW_CLK_CTR);

    //
    // Enter HIBernate mode
    //
    MAP_PRCMHibernateEnter();
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
