//*****************************************************************************
// user_app_config.h
//
// User application configuration selection
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
#ifndef __USER_APP_CONFIG_H__
#define __USER_APP_CONFIG_H__

#define GPIO_13                 13
#define GPIO_17                 17
#define GPIO_09                 9

int clk_enbl_op(struct soc_module *hw_module, enum soc_pm Sx);
int clk_dsbl_op(struct soc_module *hw_module, enum soc_pm Sx);

// specific settings for link spi (for power mgmt. framework)
struct cc_spi_config link_spi_settings = {
        PRCM_LSPI,
        LSPI_BASE,
        { 
                (LSPI_BASE+MCSPI_O_TX0),
                UDMA_CH13_LSPI_TX,
                (LSPI_BASE+MCSPI_O_RX0),
                UDMA_CH12_LSPI_RX
        },
        INT_LSPI,
        { SPI_LATCH_ODD_EDGE, SPI_CLK_ACTIVE_HIGH},
        SPI_CS_ACTIVE_HIGH,
        SPI_CS_SW_CTRL_AUTO,
        13000000,
        SPI_WORDLEN_32,
        true,
        /*SPI_DMA_OPS*/SPI_POLLED_OPS,
        NULL,
        NULL,
        NULL
};

// specific settings for uart module (for power mgmt. framework)
struct cc_uart_config uart_settings = {
        PRCM_UARTA0,
        {
                UARTA0_BASE,
                80000000,
                115200,
                (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                 UART_CONFIG_PAR_NONE),
                16,
                NULL
        },
        {
                (UARTA0_BASE + UART_O_DR),
                UDMA_CH9_UARTA0_TX,
                (UARTA0_BASE + UART_O_DR),
                UDMA_CH8_UARTA0_RX
        },
        e_int_based,
        e_int_based,
        e_one_half,
        e_one_half,
        e_true,
        e_true,
        INT_UARTA0,
        NULL
};

// structure conatining general information of the all the modules associated
// with power mgmt. framework
struct soc_module modules[ ] = {
        /* ----------- Mandatory for Link SPI for networking ----------*/
        {
                PRCM_LSPI,                    /* Module ID */
                LSPI_BASE,                    /* Base Addr */
                INT_LSPI,                     /* Interrupt */
                e_freq_mhz80,                 /* Frequency */
                (volatile unsigned long*)(ARCM_BASE +
                    APPS_RCM_O_MCSPI_A2_CLK_GATING), /* Clock Reg */
                e_pm_S1,                      /* Lowest PM */
                &link_spi_settings,           /* Specifics */
                NULL,                         /* SW Driver */
                NULL,                         /* Next item */
                clk_enbl_op,              /* Clk en op */
                clk_dsbl_op,              /* Clk ds op */
                spi_driver_load               /* Driver LD */
        },
        /* ----------- Add any application specific modules here ----------*/
        {
                PRCM_UARTA0,                /* Module ID */
                UARTA0_BASE,                /* Base Addr */
                INT_UARTA0,                 /* Interrupt */
                e_freq_mhz80,               /* Frequency */
                (volatile unsigned long*)(ARCM_BASE +
                        APPS_RCM_O_UART_A0_CLK_GATING), /* Clock Reg */
                e_pm_S1,                    /* Lowest PM */
                &uart_settings,             /* Specifics */
                NULL,                       /* SW Driver */
                NULL,                       /* Next item */
                clk_enbl_op,                /* Clk en op */
                clk_dsbl_op,                /* Clk ds op */
                uart_driver_load            /* Driver LD */
        }

};

// List of GPIOs used in the application
/* ----------- Update this list as per the application ----------*/
int gpio_list[] = {
        GPIO_09,
        GPIO_13,
        GPIO_17
};

#endif //  __USER_APP_CONFIG_H__
