//*****************************************************************************
// lp3p0_plat_ops.c
//
// Board specific operations
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

//****************************************************************************
//
//! \addtogroup sensor_profile
//! @{
//
//****************************************************************************
#include <string.h>

// Driverlib includes
#include "hw_types.h"
#include "hw_memmap.h"
#include "rom.h"
#include "rom_map.h"
#include "prcm.h"
#include "cc_types.h"

// Oslib includes
#include "osi.h"

// Middleware
#include "dma_hal.h"
#include "cc_pm.h"
#include "cc_pm_ops.h"
#include "cc_io_park.h"
#include "gpio_hal.h"

// Common interface includes
#include "utils_if.h"

#include "pinmux.h"



extern tDMAControlTable dma_ctrl_table[64];
int user_sel_power_mode;

#ifdef DEBUG_GPIO
#define GPIO_09                    9
extern cc_hndl tGPIODbgHndl;
#endif

struct soc_io_park cc32xx_io_park[] = {
        {PIN_01, "GPIO_10", WEAK_PULL_DOWN_STD},
        {PIN_02, "GPIO_11", WEAK_PULL_DOWN_STD},
        {PIN_03, "GPIO_12", WEAK_PULL_DOWN_STD},
        {PIN_04, "GPIO_13", WEAK_PULL_DOWN_STD},
        {PIN_05, "GPIO_14", WEAK_PULL_DOWN_STD},
        {PIN_06, "GPIO_15", WEAK_PULL_DOWN_STD},
        {PIN_07, "GPIO_16", WEAK_PULL_DOWN_STD},
        {PIN_08, "GPIO_17", WEAK_PULL_DOWN_STD},
        {PIN_15, "GPIO_22", WEAK_PULL_DOWN_STD},
        {PIN_16, "GPIO_23/JTAG_TDI", WEAK_PULL_DOWN_STD},
        {PIN_17, "GPIO_24/JTAG_TDO", WEAK_PULL_DOWN_STD},
        {PIN_18, "GPIO_28", WEAK_PULL_DOWN_STD},
        {PIN_19, "GPIO_28//JTAG_TCK", WEAK_PULL_DOWN_STD},
        {PIN_20, "GPIO_29/JTAG_TMS", WEAK_PULL_DOWN_STD},
        {PIN_21, "GPIO_25/SOP2", WEAK_PULL_DOWN_STD},
        {PIN_45, "DCDC_FLASH_SW_P", WEAK_PULL_DOWN_STD},
        {PIN_50, "GPIO_00", WEAK_PULL_DOWN_STD},
        {PIN_52, "GPIO_32", WEAK_PULL_DOWN_STD},
        {PIN_53, "GPIO_30", WEAK_PULL_DOWN_STD},
        {PIN_55, "GPIO_01", WEAK_PULL_DOWN_STD},
        {PIN_57, "GPIO_02", WEAK_PULL_DOWN_STD},
        {PIN_58, "GPIO_03", WEAK_PULL_DOWN_STD},
        {PIN_59, "GPIO_04", WEAK_PULL_DOWN_STD},
        {PIN_60, "GPIO_05", WEAK_PULL_DOWN_STD},
        {PIN_61, "GPIO_06", WEAK_PULL_DOWN_STD},
        {PIN_62, "GPIO_07", WEAK_PULL_DOWN_STD},
        {PIN_63, "GPIO_08", WEAK_PULL_DOWN_STD},
        {PIN_64, "GPIO_09", WEAK_PULL_DOWN_STD}
};

//****************************************************************************
//
//! \brief  This function checks if the system can go into a lower power mode
//!
//! \param  target is the low power mode, whose feasibilty we are checking
//!
//! \return true if the system can go into the target state, false if it can't.
//
//****************************************************************************
bool lp3p0_can_try_pm_state(enum soc_pm target)
{
        bool retval = false;

        switch(target) {
                case e_pm_S0:
                        break;
                case e_pm_S1:
                        if(user_sel_power_mode & POWER_POLICY_SLEEP){
                                retval = true;
                        }
                        break;
                case e_pm_S2:
                        /*if(user_sel_power_mode & POWER_POLICY_DEEPSLEEP){
                                retval = true;
                        }*/
                        break;
                case e_pm_S3:
                        if(user_sel_power_mode & POWER_POLICY_STANDBY){
                                retval = true;
                        }
                        break;
                case e_pm_S4:
                        if(user_sel_power_mode & POWER_POLICY_HIBERNATE){
                                retval = true;
                        }
                        break;
                default:
                        break;
        }
        
        return retval;
}


//****************************************************************************
//
//! \brief  This function backs up necessary registers and data(before S3)
//!
//! \param  none
//!
//! \return none
//
//****************************************************************************
void lp3p0_back_up_soc_data(void)
{
#ifdef DEBUG_GPIO
        cc_gpio_write(tGPIODbgHndl, GPIO_09, 0);
#endif
        /* Invoking the default CC3xxx service impl. */
        cc_back_up_soc_data(PRCM_SRAM_COL_1|PRCM_SRAM_COL_2|PRCM_SRAM_COL_3);

        /* Park the IO PINs safely to avoid any leakage */
        cc_io_park_safe(cc32xx_io_park, 
                sizeof(cc32xx_io_park)/sizeof(struct soc_io_park));
        
        /* Park the Antennas selection pins */
        HWREG(0x4402E108) = 0x00000E61;
        HWREG(0x4402E10C) = 0x00000E61;      
        
        return;
}

//****************************************************************************
//
//! \brief  This function restore the backed up data (after S3)
//!
//! \param  none
//!
//! \return none
//
//****************************************************************************
void lp3p0_restore_soc_data(void)
{       
        /* Invoking the default CC3xxx service impl. */
        cc_restore_soc_data();
		
		PRCMCC3200MCUInit();
		
        //
        // Configure the pinmux settings for the peripherals exercised
        //
        PinMuxConfig();   
        
        cc_gpio_restore_context();
#ifdef DEBUG_GPIO
        cc_gpio_write(tGPIODbgHndl, GPIO_09, 1);
#endif
		/* Initialize timer services */
        MAP_PRCMPeripheralClkEnable(PRCM_TIMERA0, PRCM_RUN_MODE_CLK|
                                                    PRCM_SLP_MODE_CLK);
        /* ungates the clk for the shared SPI*/
        MAP_PRCMPeripheralClkEnable(PRCM_SSPI, PRCM_RUN_MODE_CLK|
                                    PRCM_SLP_MODE_CLK);
        /* Initialize the DMA module */
        MAP_PRCMPeripheralClkEnable(PRCM_UDMA, PRCM_RUN_MODE_CLK|
                                    PRCM_SLP_MODE_CLK);
        MAP_uDMAControlBaseSet(dma_ctrl_table);
        MAP_uDMAEnable();
        
        MAP_PRCMIntEnable(PRCM_INT_SLOW_CLK_CTR);
}

//****************************************************************************
//
//! \brief  This function sets up the power policy for the system
//!
//! \param  power_policy is the lowest power mode the device will try to get
//!         into
//!
//! \return none
//
//****************************************************************************
void lp3p0_setup_power_policy(int power_policy)
{
        user_sel_power_mode = power_policy;
        return;
}

//****************************************************************************
//
//! \brief  This function puts the system into hibernate
//!
//! \param  none
//!
//! \return none
//
//****************************************************************************
int lp3p0_enter_hibernate()
{
       Utils_SpiFlashDeepPowerDown();
       return (cc_enter_S4());
}

//****************************************************************************
//
//! \brief  This function hooks up the platform specific APIs with the power
//!         mgmt. framework.
//!
//! \param  lp3p0_pm_ops is the structure holding the platform specific routines
//!
//! \return 0
//
//****************************************************************************
int lp3p0_get_pm_ops(struct platform_pm_ops *lp3p0_pm_ops)
{
        lp3p0_pm_ops->set_up_wkup_srcs = cc_set_up_wkup_srcs;
        lp3p0_pm_ops->handle_S3_wakeup = cc_handle_S3_wakeup;
        lp3p0_pm_ops->are_irqs_pending = cc_are_irqs_pending;
        lp3p0_pm_ops->can_try_pm_state = lp3p0_can_try_pm_state;
        lp3p0_pm_ops->enter_S4 = lp3p0_enter_hibernate;
        lp3p0_pm_ops->enter_S3 = cc_enter_S3;
        lp3p0_pm_ops->enter_S2 = cc_enter_S2;
        lp3p0_pm_ops->enter_S1 = cc_enter_S1;
        lp3p0_pm_ops->back_up_soc_data = lp3p0_back_up_soc_data;
        lp3p0_pm_ops->restore_soc_data = lp3p0_restore_soc_data;
        lp3p0_pm_ops->dsbl_sys_irq = osi_EnterCritical;
        lp3p0_pm_ops->enbl_sys_irq = osi_ExitCritical;

        return 0;
}
