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
#include "cc_timer.h"
#include "uart_drv.h"
#include "spi_drv.h"
#include "gpio_hal.h"
#include "rtc_hal.h"
#include "uart_hal.h"
#include "dma_hal.h"
#include "interrupt_hal.h"
#include "osi.h"
#include "user_app_config.h"

#define CTL_TBL_SIZE	        64	//32*2 entries
#ifdef ccs
#pragma DATA_ALIGN(dma_ctrl_table, 1024)
tDMAControlTable dma_ctrl_table[CTL_TBL_SIZE];
#else
#pragma data_alignment=1024
tDMAControlTable dma_ctrl_table[CTL_TBL_SIZE];
#endif

extern int gpio_intr_hndlr(int gpio_num);
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
typedef struct
{
    unsigned short ucMajorVerNum;
    unsigned short ucMinorVerNum;
    unsigned short ucSubMinorVerNum;
    unsigned short ucDay;
    unsigned short ucMonth;
    unsigned short ucYear;
}tROMVersion;

volatile int sw_simulate_rtc = 0;
volatile int sw_simulate_timer = 0;
cc_hndl timer_hndl;

//****************************************************************************
//
//! \brief  This function ungates the clk for the peripheral given as argument
//!
//! \param  hw_module is the pointer to the module structure for a peripheral 
//! \param  Sx is the power state of the peripheral after enabling the clk
//!
//! \return     0
//
//****************************************************************************
int clk_enbl_op(struct soc_module *hw_module, enum soc_pm Sx)
{
        switch(Sx) {
                
        case e_pm_S0:
        case e_pm_S1:
                MAP_PRCMPeripheralClkEnable(hw_module->module_id,
                                        PRCM_RUN_MODE_CLK|PRCM_SLP_MODE_CLK);
                break;
        case e_pm_S2:
            /*
                MAP_PRCMPeripheralClkEnable(hw_module->module_id,
                                        PRCM_DSLP_MODE_CLK);
            */
                break;
                
        default: break;
                
        }
        
        return 0;
}

//****************************************************************************
//
//! \brief  This function gates the clk for the peripheral given as argument
//!
//! \param  hw_module is the pointer to the module structure for a peripheral 
//! \param  Sx is the power state of the peripheral after gating the clk
//!
//! \return     0
//
//****************************************************************************
int clk_dsbl_op(struct soc_module *hw_module, enum soc_pm Sx)
{
        switch(Sx) {
                
        case e_pm_S0:
        case e_pm_S1:
                MAP_PRCMPeripheralClkDisable(hw_module->module_id,
                						 PRCM_RUN_MODE_CLK|PRCM_SLP_MODE_CLK);
                break;
        case e_pm_S2:
            /*
                MAP_PRCMPeripheralClkDisable(hw_module->module_id,
                                        PRCM_DSLP_MODE_CLK);
            */
                break;
                
        default: break;
                
        }
        
        return 0;
}

//****************************************************************************
//
//! \brief  This function simulates the nvic interrupt for RTC
//!
//! \param  none
//!
//! \return none
//
//****************************************************************************
void sw_simulate_rtc_intr()
{
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
void sw_simulate_timer_intr()
{
        sw_simulate_timer = 1;
        MAP_IntPendSet(INT_TIMERA0A);
}

//****************************************************************************
//
//! \brief  This function initialise the rtc module, hence necessary if one
//!         wishes to use rtc tiemr
//!
//! \param  none
//!
//! \return  0 if successful, -1 in the case of error
//
//****************************************************************************
static int rtc_module_init()
{
        struct hw_timer_cfg hw_timer_config;
        int retval;

        sw_simulate_rtc = 0;
        /* Setup the RTC timer configurations */
        hw_timer_config.base_addr = 0;
        hw_timer_config.freq_hz = 32768;
        hw_timer_config.source = HW_REALTIME_CLK;
        hw_timer_config.user_tfw = true;
        hw_timer_config.cb.tfw_register_hwt_ops = cc_timer_register_hwt_ops;
        hw_timer_config.cb_param = NULL;
        hw_timer_config.enbl_irqc = osi_ExitCritical;
        hw_timer_config.dsbl_irqc = osi_EnterCritical;
        hw_timer_config.set_irq = sw_simulate_rtc_intr/*cc_rtc_isr*/;

        retval = cc_rtc_init(&hw_timer_config);
		cc_rtc_fast_read_config(true);
        return retval;
}
//****************************************************************************
//
//! \brief  This function initialise the timer module, hence necessary if one
//!         wishes to use the timer module
//!
//! \param  none
//!
//! \return  0 if successful, -1 in the case of error
//
//****************************************************************************
static int hwt32_timer_init()
{
        struct hw_timer_cfg hw_timer_config;

        sw_simulate_timer = 0;
        /* Setup the timer module configurations */
        hw_timer_config.base_addr = TIMERA0_BASE; /* deafult value, can be changed */
        hw_timer_config.freq_hz = 80000000;
        hw_timer_config.source = HW_MONOTONE_CTR;
        hw_timer_config.user_tfw = true;
        hw_timer_config.cb.tfw_register_hwt_ops = cc_timer_register_hwt_ops;
        hw_timer_config.cb_param = NULL;
        hw_timer_config.enbl_irqc = osi_ExitCritical;
        hw_timer_config.dsbl_irqc = osi_EnterCritical;
        hw_timer_config.set_irq = sw_simulate_timer_intr;

        timer_hndl = cc_hwt32_init(&hw_timer_config);
        if(NULL != timer_hndl)
        {
            return 0;
        }
        else
        {
            return -1;
        }
}

//****************************************************************************
//
//! \brief  This function initialise the gpio module, hence necessary if one
//!         wishes to use GPIOs
//!
//! \param  none
//!
//! \return  0 if successful, -1 in the case of error
//
//****************************************************************************
static int gpio_module_init(int *gpios_in_use, int count)
{
#define EXTRACT_PORT(gpio)      (gpio >> 3)
#define EXTRACT_PIN(gpio)       (gpio & 0x7)
#define SETUP_GPIO_PORT_INFO(indx, id, addr, irq, valm, spm)     \
        {                                                        \
        gpio_config.port_info[indx].module_id = id;              \
        gpio_config.port_info[indx].base_addr = addr;            \
        gpio_config.port_info[indx].irqvec_id = irq;             \
        gpio_config.port_info[indx].validity_mask = valm;        \
        gpio_config.port_info[indx].special_purpose_mask = spm;  \
        }
#define GPIO_PORT0_WAKE_SOURCES 0x14
#define GPIO_PORT1_WAKE_SOURCES 0x28
#define GPIO_PORT2_WAKE_SOURCES 0x2
#define GPIO_PORT3_WAKE_SOURCES 0x5

        struct cc_gpio_config gpio_config;
        int retval, loopcnt;
        unsigned char port0_validity = 0, port1_validity = 0;
        unsigned char port2_validity = 0, port3_validity = 0;
        unsigned int port_num = 0;
        unsigned char pin_num, bitmask_pin;
        
        memset(&gpio_config, 0 ,sizeof(struct cc_gpio_config));

        /* Setup the GPIO validity mask */
        for(loopcnt = 0; loopcnt < count; loopcnt++) {
                port_num = EXTRACT_PORT(gpios_in_use[loopcnt]);
                pin_num = EXTRACT_PIN(gpios_in_use[loopcnt]);
                bitmask_pin = (1 << pin_num);
                
                if(0 == port_num) { /* Port 0 */
                        port0_validity |= bitmask_pin;
                } else if(1 == port_num) { /* Port 1 */
                        port1_validity |= bitmask_pin;
                } else if(2 == port_num) { /* Port 2 */
                        port2_validity |= bitmask_pin;
                } else { /* Port 3 */
                        port3_validity |= bitmask_pin;
                }
        }

        /* Setup the GPIO module configurations */
        if(port0_validity) {
                SETUP_GPIO_PORT_INFO(0,
                                     PRCM_GPIOA0,
                                     GPIOA0_BASE,
                                     INT_GPIOA0,
                                     port0_validity,
                                     GPIO_PORT0_WAKE_SOURCES);
        }

        if(port1_validity) {
                SETUP_GPIO_PORT_INFO(1,
                                     PRCM_GPIOA1,
                                     GPIOA1_BASE,
                                     INT_GPIOA1,
                                     port1_validity,
                                     GPIO_PORT1_WAKE_SOURCES);
        }

        if(port2_validity) {
                SETUP_GPIO_PORT_INFO(2,
                                     PRCM_GPIOA2,
                                     GPIOA2_BASE,
                                     INT_GPIOA2,
                                     port2_validity,
                                     GPIO_PORT2_WAKE_SOURCES);
        }

        if(port3_validity) {
                SETUP_GPIO_PORT_INFO(3,
                                     PRCM_GPIOA3,
                                     GPIOA3_BASE,
                                     INT_GPIOA3,
                                     port3_validity,
                                     GPIO_PORT3_WAKE_SOURCES);
        }

        gpio_config.drv_notify_cb = gpio_intr_hndlr;
        gpio_config.enbl_irqc = osi_ExitCritical;
        gpio_config.dsbl_irqc = osi_EnterCritical;
        
        retval = cc_gpio_init(&gpio_config);

        return retval;
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
void prcm_interrupt_handler(void *intr_param)
{
        int status;

        /* Read the interrupt status, also clears the status */
        status = MAP_PRCMIntStatus();

        if((PRCM_INT_SLOW_CLK_CTR == status) || (sw_simulate_rtc)) {
                sw_simulate_rtc = 0;
                /* Invoke the RTC interrupt handler */
                cc_rtc_isr();
        } else if(0 == status) {
                /* Invoke the wake from LPDS interrupt handler */
                wake_interrupt_handler();
        } else {
        }
}

void timer_interrupt_handler(void *intr_param)
{
    sw_simulate_timer = 0;
    cc_hwt32_isr(timer_hndl);
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
int platform_init(void)
{
        int retval = 0;
        struct cc_dma_cfg dma_config;
        struct platform_pm_ops lp3p0_pm_ops;
        struct cc_timer_setup timer_setup;
        tROMVersion* pRomVersion = (tROMVersion *)(ROM_VERSION_ADDR);
        u32 *spi_baud_rate;

        /* Disable clocks for modules that are not used on this platform */
        /* Initialize the PM framework */
        retval = cc_pm_framework_init();
        
        /* Get the SOC specific platform operations */
        lp3p0_get_pm_ops(&lp3p0_pm_ops);

        /* Register PM ops with framework */
        retval = cc_pm_platform_ops_register(&lp3p0_pm_ops);
        
        /* Register the power management ISR */
        register_isr(INT_PRCM, prcm_interrupt_handler, NULL);
        
        /* Register the timer ISR */
        register_isr(INT_TIMERA0A, timer_interrupt_handler, NULL);

        /* Initialize the platform specific services */
        timer_setup.enbl_irqc = osi_ExitCritical;
        timer_setup.dsbl_irqc = osi_EnterCritical;
        retval = cc_timer_module_init(&timer_setup);
        /* Initialize RTC services */
        retval = rtc_module_init();

        /* Initialize timer services */
        MAP_PRCMPeripheralClkEnable(PRCM_TIMERA0, PRCM_RUN_MODE_CLK|
                                                    PRCM_SLP_MODE_CLK);
        retval = hwt32_timer_init();

        /* Initialize GPIO services */
        retval = gpio_module_init(&gpio_list[0], 
                        (sizeof(gpio_list)/sizeof(int)));
        
        /* Initialize the DMA module */
        MAP_PRCMPeripheralClkEnable(PRCM_UDMA, PRCM_RUN_MODE_CLK|
                                    PRCM_SLP_MODE_CLK);
        dma_config.ctl_table = dma_ctrl_table;
        retval = cc_dma_init(&dma_config);

        spi_baud_rate = &(((struct cc_spi_config*)(modules[0].hw_detail))->baud_rate);
        /* deciding the SPI baud rate depending upon PG version */
        if(pRomVersion->ucMinorVerNum == ROM_VER_PG1_21 )
		{
        	*spi_baud_rate = SPI_RATE_13M;
		}
		else if(pRomVersion->ucMinorVerNum == ROM_VER_PG1_32)
		{
			*spi_baud_rate = SPI_RATE_13M;
		}
		else if(pRomVersion->ucMinorVerNum >= ROM_VER_PG1_33)
		{
			*spi_baud_rate = SPI_RATE_20M;
		}

        /* Load the application specific modules */
#define NUM_SOC_MODULES(modules) (sizeof(modules) / sizeof(struct soc_module))

        if(-1 == cc_modules_load(modules, NUM_SOC_MODULES(modules))) {
                /* Should modules be unloaded? */
                retval = -1;
        }
        
        return retval;
}
