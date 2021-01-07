//*****************************************************************************
// lp3p0_board.c
//
// board specific APIs
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
//! \addtogroup idle_profile
//! @{
//
//****************************************************************************
#include "hw_types.h"
#include "hw_memmap.h"
#include "hw_gpio.h"
#include "hw_common_reg.h"
#include "interrupt.h"
#include "rom.h"
#include "rom_map.h"
#include "gpio.h"
#include "hw_ints.h"
#include "hw_nvic.h"
#include "prcm.h"
#include "hw_uart.h"
#include "uart.h"
#include "hw_apps_rcm.h"
#include "hw_mcspi.h"
#include "udma.h"
#include "cc_pm.h"
#include "cc_pm_ops.h"

extern int lp3p0_get_pm_ops(struct platform_pm_ops *lp3p0_pm_ops);

#define CC32XX_MOD_ID_I2C_0    0x01
#define CC32XX_MOD_ID_SPI_0    0x02

#define ROM_VERSION_ADDR                0x00000400
#define ROM_VER_PG1_21                  1
#define ROM_VER_PG1_32                  2
#define ROM_VER_PG1_33                  3
#define SPI_RATE_13M 		            13000000
#define SPI_RATE_20M                    20000000

//Structure definition to determine the ROM version
typedef struct {
	unsigned short ucMajorVerNum;
	unsigned short ucMinorVerNum;
	unsigned short ucSubMinorVerNum;
	unsigned short ucDay;
	unsigned short ucMonth;
	unsigned short ucYear;
} tROMVersion;

volatile int sw_simulate_rtc = 0;
volatile int sw_simulate_timer = 0;

//****************************************************************************
//
//! \brief  This function simulates the nvic interrupt for RTC
//!
//! \param  none
//!
//! \return none
//
//****************************************************************************
void sw_simulate_rtc_intr() {
	sw_simulate_rtc = 1;
	MAP_IntPendSet(INT_PRCM);
}

//****************************************************************************
//
//! \brief  This function simulates the nvic interrupt for TIMERA0
//!
//! \param  none
//!
//! \return none
//
//****************************************************************************
void sw_simulate_timer_intr() {
	sw_simulate_timer = 1;
	MAP_IntPendSet(INT_TIMERA0A);
}

//****************************************************************************
//
//! \brief  This function handles the PRCM interrupt
//!
//! \param  intr_param is a void pointer (not used)
//!
//! \return none
//
//****************************************************************************
void prcm_interrupt_handler() {
	int status;

	/* Read the interrupt status, also clears the status */
	status = MAP_PRCMIntStatus();

	if ((PRCM_INT_SLOW_CLK_CTR == status) || (sw_simulate_rtc)) {
		sw_simulate_rtc = 0;
		/* Invoke the RTC interrupt handler */

	} else if (0 == status) {
		/* Invoke the wake from LPDS interrupt handler */
		wake_interrupt_handler();
	} else {
	}
}

//****************************************************************************
//
//! \brief  This function initialise the basic and application specific modules
//!         for a CC3200 platform
//!
//! \param  none
//!
//! \return 0 if application specific modules are loaded succesfully, -1 in case
//!         failure
//
//****************************************************************************
int platform_init(void) {
	int retval = 0;
	struct platform_pm_ops lp3p0_pm_ops;

	/* Initialize the PM framework */
	retval = cc_pm_framework_init();

	/* Get the SOC specific platform operations */
	lp3p0_get_pm_ops(&lp3p0_pm_ops);

	/* Register PM ops with framework */
	retval = cc_pm_platform_ops_register(&lp3p0_pm_ops);

	/* Register the interrupt with the global interrupt handler */
	MAP_IntRegister(INT_PRCM, prcm_interrupt_handler);

	/* Enable the interrupt */
	MAP_IntEnable(INT_PRCM);

	MAP_PRCMIntEnable(PRCM_INT_SLOW_CLK_CTR);

	MAP_PRCMPeripheralClkEnable(PRCM_UDMA, PRCM_RUN_MODE_CLK |
	PRCM_SLP_MODE_CLK);

	return retval;
}
