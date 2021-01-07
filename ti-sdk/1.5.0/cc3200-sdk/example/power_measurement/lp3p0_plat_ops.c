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
//! \addtogroup idle_profile_nonos
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

// Middleware includes
#include "cc_types.h"
#include "cc_pm.h"
#include "cc_pm_ops.h"
#include "cc_io_park.h"

// simplelink includes
#include "cc_pal.h"

#include "cpu.h"

#include "uart_if.h"
#include "udma_if.h"

// Common interface includes
#include "utils_if.h"

#include "pinmux.h"

int user_sel_power_mode;

enum wk_src
{
	WK_RTC = 0x1,
	WK_GPIO = 0x2,
	WK_HOST_IRQ = 0x4
};

#define WK_LPDS 				0
#define WK_HIB					1

struct wkup_info
{
	enum wk_src wk_type;
	unsigned long timer_interval;
	bool is_periodic;
	unsigned long wk_gpio_pin;
	unsigned long trigger_type;
};

struct wkup_info lpds_wk_info, hib_wk_info;
int intr_ref_cnt = 0;

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
//! \brief  enables various peripheral after coming out of low power mode
//!
//! \param  none
//!
//! \return none
//
//****************************************************************************
void enable_peripherals()
{
    //Initialising uDMA
	UDMAInit();
    
    //Initialising the link SPI
	spi_Open(NULL, NULL);
	
    //Initialising the UART terminal
    InitTerm();
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

        /* disabling all wk up srcs */
        PRCMLPDSWakeupSourceDisable(PRCM_LPDS_HOST_IRQ|PRCM_LPDS_GPIO|PRCM_LPDS_TIMER);

        //
        // Configure the pinmux settings for the peripherals exercised
        //
        PinMuxConfig();   
        
        //
        // enable peripherals
        //
        enable_peripherals();

        /* ungates the clk for the shared SPI*/
        MAP_PRCMPeripheralClkEnable(PRCM_SSPI, PRCM_RUN_MODE_CLK|
                                    PRCM_SLP_MODE_CLK);
        
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
//! \brief  Setting various wake sources for the device
//!
//! \param  target is the lowest power mode that the deveice will exercise
//!
//! \return 0 if success, -1 in case of error
//
//****************************************************************************
int set_wkup_srcs(enum soc_pm target)
{
	int iRetVal = -1;
	switch(target) {
			case e_pm_S0:
			case e_pm_S1:
			case e_pm_S2:
				/* These handle the cases of run, sleep, deepsleep.
				   Wake source is configured outside this scope in
				   individual peripherals */
				break;
			case e_pm_S3:
				if(lpds_wk_info.wk_type &  WK_RTC)
				{
						/* Setup the LPDS wake time */
						MAP_PRCMLPDSIntervalSet(lpds_wk_info.timer_interval * 32768);
						/* Enable the wake source to be timer */
						MAP_PRCMLPDSWakeupSourceEnable(PRCM_LPDS_TIMER);
						iRetVal = 0;
				}
				if(lpds_wk_info.wk_type &  WK_GPIO)
				{
						MAP_PRCMLPDSWakeUpGPIOSelect(lpds_wk_info.wk_gpio_pin,lpds_wk_info.trigger_type);
						MAP_PRCMLPDSWakeupSourceEnable(PRCM_LPDS_GPIO);
						iRetVal = 0;
				}
				if(lpds_wk_info.wk_type &  WK_HOST_IRQ)
				{
						/* Set LPDS Wakeup source as NWP request */
						MAP_PRCMLPDSWakeupSourceEnable(PRCM_LPDS_HOST_IRQ);
						iRetVal = 0;
				}
				if(lpds_wk_info.is_periodic == false)
				{
						lpds_wk_info.wk_type &= (~WK_RTC);
				}
				break;
			case e_pm_S4:
				if(hib_wk_info.wk_type &  WK_RTC)
				{
					/* Setup the LPDS wake time */
					MAP_PRCMHibernateIntervalSet(hib_wk_info.timer_interval * 32768);
					/* Enable the wake source to be timer */
					MAP_PRCMHibernateWakeupSourceEnable(PRCM_HIB_SLOW_CLK_CTR);
					iRetVal = 0;
				}
				if(hib_wk_info.wk_type &  WK_GPIO)
				{
						MAP_PRCMHibernateWakeUpGPIOSelect(hib_wk_info.wk_gpio_pin,hib_wk_info.trigger_type);
						MAP_PRCMHibernateWakeupSourceEnable(hib_wk_info.wk_gpio_pin);
						iRetVal = 0;
				}
				break;
			default:
				return -1;
	}
	return iRetVal;
}
/* check if IRQs are pending */
bool irqs_pending_check()
{
	return false;
}

/* Disable system / global interrupt. ref counted for use by nested calls */
u32 disable_system_irq(void)
{
        u32 primask;
        /* Disable the interrupts */
        primask = CPUcpsid();
        intr_ref_cnt++;

        return(primask);
}

/* Enable system / global interrupt. */
void enable_system_irq(u32 primask)
{

        if(intr_ref_cnt && (--intr_ref_cnt == 0)) {
                /* Enable the interrupts */
                primask = CPUcpsie();
        }
        UNUSED(primask);
        return;
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
        lp3p0_pm_ops->set_up_wkup_srcs = set_wkup_srcs;
        lp3p0_pm_ops->handle_S3_wakeup = cc_handle_S3_wakeup;
        lp3p0_pm_ops->are_irqs_pending = cc_are_irqs_pending;
        lp3p0_pm_ops->can_try_pm_state = lp3p0_can_try_pm_state;
        lp3p0_pm_ops->enter_S4 = lp3p0_enter_hibernate;
        lp3p0_pm_ops->enter_S3 = cc_enter_S3;
        lp3p0_pm_ops->enter_S2 = cc_enter_S2;
        lp3p0_pm_ops->enter_S1 = cc_enter_S1;
        lp3p0_pm_ops->back_up_soc_data = lp3p0_back_up_soc_data;
        lp3p0_pm_ops->restore_soc_data = lp3p0_restore_soc_data;
        lp3p0_pm_ops->dsbl_sys_irq = disable_system_irq;
        lp3p0_pm_ops->enbl_sys_irq = enable_system_irq;

        return 0;
}

//****************************************************************************
//
//! \brief  This function set up timer as wake source for various low power
//!         modes
//!
//! \param  pwr_mode is the low power mode for which gpio will be set as a wake
//!         source
//! \param  time_interval is the timer value to be configured for wake up.
//! \param  is_periodic determines if the timer is periodic or one shot.
//!
//! \return 0 if success, -1 in case of error
//
//****************************************************************************
int set_rtc_as_wk_src(int pwr_mode, unsigned long time_interval, bool is_periodic)
{
		if(pwr_mode == WK_HIB)
		{
				hib_wk_info.wk_type |= WK_RTC;
				hib_wk_info.timer_interval = time_interval;
				hib_wk_info.is_periodic = is_periodic; /* don't care */
		}else if(pwr_mode == WK_LPDS)
		{

				lpds_wk_info.wk_type |= WK_RTC;
				lpds_wk_info.timer_interval = time_interval;
				lpds_wk_info.is_periodic = is_periodic;
		}else
		{
				return -1;
		}
		return 0;
}

//****************************************************************************
//
//! \brief  This function set up GPIO as wake source for various low power modes
//!
//! \param  pwr_mode is the low power mode for which gpio will be set as a wake
//!         source
//! \param  wk_pin is the gpio to be set as wake source
//! \param  trigger_type is the gpio trigger(RISING EDGE, FALLING EDGE, LOW,
//!         HIGH)
//!
//! \return 0 if success, -1 in case of error
//
//****************************************************************************
int set_gpio_as_wk_src(int pwr_mode, unsigned long wk_pin,unsigned long trigger_type)
{
		if(pwr_mode == WK_HIB)
		{
				hib_wk_info.wk_type |= WK_GPIO;
				hib_wk_info.wk_gpio_pin = wk_pin;
				hib_wk_info.trigger_type = trigger_type;
		}else if(pwr_mode == WK_LPDS)
		{
				lpds_wk_info.wk_type |= WK_GPIO;
				lpds_wk_info.wk_gpio_pin = wk_pin;
				lpds_wk_info.trigger_type = trigger_type;
		}else
		{
				return -1;
		}
		return 0;
}

//****************************************************************************
//
//! \brief  setting host IRQ as a wake up source from low power modes
//!
//! \param  enable decides whether to setup or remove host IRQ as wake up source
//!
//! \return None
//
//****************************************************************************
void set_host_irq_as_lpds_wk_src(bool enable)
{
		if(enable)
		{
				lpds_wk_info.wk_type |= WK_HOST_IRQ;
		}
		else
		{
				lpds_wk_info.wk_type &= ~WK_HOST_IRQ;
		}
}
