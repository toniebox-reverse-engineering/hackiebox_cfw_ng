//*****************************************************************************
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


//*****************************************************************************
//
// Application Name     -   Wifi Audio Application
// Application Overview -   This is a sample application demonstrating 
//                          Bi-directional audio transfer.Audio is streamed
//                          from one LP and rendered on another LP over wifi. 
//
//*****************************************************************************

//****************************************************************************
//
//! \addtogroup wifi_audio_app
//! @{
//
//****************************************************************************
#include <stdlib.h>
#include <string.h>

//SimpleLink includes
#include "simplelink.h"

// free-rtos/ ti_rtos includes
#include "osi.h"

// Hardware & DriverLib library includes.
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "interrupt.h"
#include "i2s.h"
#include "udma.h"
#include "gpio.h"
#include "gpio_if.h"
#include "prcm.h"
#include "rom.h"
#include "rom_map.h"
#include "pin.h"
#include "utils.h"

//Common interface includes
#include "common.h"
#include "udma_if.h"
#include "uart_if.h"
#include "i2c_if.h"



//App include
#include "pinmux.h"
#include "network.h"
#include "circ_buff.h"
#include "control.h"
#include "audioCodec.h"
#include "i2s_if.h"
#include "pcm_handler.h"


#define APPLICATION_VERSION     "1.4.0"
#define OSI_STACK_SIZE          1024

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
tCircularBuffer *pRecordBuffer;
tCircularBuffer *pPlayBuffer;
tUDPSocket g_UdpSock;
OsiTaskHandle g_SpeakerTask = NULL ;
OsiTaskHandle g_MicTask = NULL ;
OsiTaskHandle g_NetworkTask = NULL ;

unsigned char g_loopback=1;

#if defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif
//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************


//******************************************************************************
//                    FUNCTION DECLARATIONS
//******************************************************************************
extern void Speaker( void *pvParameters );
extern void Microphone( void *pvParameters );
extern void Network( void *pvParameters );

//*****************************************************************************
//
//! Application defined hook (or callback) function - the tick hook.
//! The tick interrupt can optionally call this
//!
//! \param  none
//! 
//! \return none
//!
//*****************************************************************************
void
vApplicationTickHook( void )
{
}

//*****************************************************************************
//
//! Application defined hook (or callback) function - assert
//!
//! \param  none
//! 
//! \return none
//!
//*****************************************************************************
void
vAssertCalled( const char *pcFile, unsigned long ulLine )
{
    while(1)
    {

    }
}

//*****************************************************************************
//
//! Application defined idle task hook
//! 
//! \param  none
//! 
//! \return none
//!
//*****************************************************************************
void
vApplicationIdleHook( void )
{

}

//*****************************************************************************
//
//! Application provided stack overflow hook function.
//!
//! \param  handle of the offending task
//! \param  name  of the offending task
//! 
//! \return none
//!
//*****************************************************************************
void
vApplicationStackOverflowHook( OsiTaskHandle *pxTask, signed char *pcTaskName)
{
    ( void ) pxTask;
    ( void ) pcTaskName;

    for( ;; );
}

void vApplicationMallocFailedHook()
{
    while(1)
    {
        // Infinite loop;
    }
}

//*****************************************************************************
//
//! Board Initialization & Configuration
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************
void
BoardInit(void)
{
    /* In case of TI-RTOS vector table is initialize by OS itself */
#ifndef USE_TIRTOS
    //
    // Set vector table base
    //
#if defined(ccs)
    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
#endif
#if defined(ewarm)
    MAP_IntVTableBaseSet((unsigned long)&__vector_table);
#endif
#endif
    //
    // Enable Processor
    //
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}
//******************************************************************************
//                            MAIN FUNCTION
//******************************************************************************
int main()
{   
    long lRetVal = -1;
    unsigned char	RecordPlay;

    BoardInit();

    //
    // Pinmux Configuration
    //
    PinMuxConfig();

    //
    // Initialising the UART terminal
    //
    InitTerm();


    //
    // Initialising the I2C Interface
    //    
    lRetVal = I2C_IF_Open(1);
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }

    RecordPlay = I2S_MODE_RX_TX;
    g_loopback = 1;


    //
    // Create RX and TX Buffer
    //
    if(RecordPlay == I2S_MODE_RX_TX)
    {
        pRecordBuffer = CreateCircularBuffer(RECORD_BUFFER_SIZE);
        if(pRecordBuffer == NULL)
        {
            UART_PRINT("Unable to Allocate Memory for Tx Buffer\n\r");
            LOOP_FOREVER();
        }
    }

    /* Play */
    if(RecordPlay & I2S_MODE_TX)
    {
        pPlayBuffer = CreateCircularBuffer(PLAY_BUFFER_SIZE);
        if(pPlayBuffer == NULL)
        {
            UART_PRINT("Unable to Allocate Memory for Rx Buffer\n\r");
            LOOP_FOREVER();
        }
    }


    //
    // Configure Audio Codec
    //     
    AudioCodecReset(AUDIO_CODEC_TI_3254, NULL);
    AudioCodecConfig(AUDIO_CODEC_TI_3254, AUDIO_CODEC_16_BIT, 16000,
                      AUDIO_CODEC_STEREO, AUDIO_CODEC_SPEAKER_ALL,
                      AUDIO_CODEC_MIC_ALL);

    AudioCodecSpeakerVolCtrl(AUDIO_CODEC_TI_3254, AUDIO_CODEC_SPEAKER_ALL, 50);
    AudioCodecMicVolCtrl(AUDIO_CODEC_TI_3254, AUDIO_CODEC_SPEAKER_ALL, 50);


    GPIO_IF_LedConfigure(LED2|LED3);

    GPIO_IF_LedOff(MCU_RED_LED_GPIO);
    GPIO_IF_LedOff(MCU_GREEN_LED_GPIO);    

    //
    // Configure PIN_01 for GPIOOutput
    //
    MAP_PinTypeGPIO(PIN_01, PIN_MODE_0, false);
    MAP_GPIODirModeSet(GPIOA1_BASE, 0x4, GPIO_DIR_MODE_OUT);

    //
    // Configure PIN_02 for GPIOOutput
    //
    MAP_PinTypeGPIO(PIN_02, PIN_MODE_0, false);
    MAP_GPIODirModeSet(GPIOA1_BASE, 0x8, GPIO_DIR_MODE_OUT);


    //Turning off Green,Orange LED after i2c writes completed - First Time
    GPIO_IF_LedOff(MCU_GREEN_LED_GPIO);
    GPIO_IF_LedOff(MCU_ORANGE_LED_GPIO);

    //
    // Initialize the Audio(I2S) Module
    //    

    AudioInit();

    //
    // Initialize the DMA Module
    //    
    UDMAInit();
    if(RecordPlay & I2S_MODE_TX)
    {
        UDMAChannelSelect(UDMA_CH5_I2S_TX, NULL);
        SetupPingPongDMATransferRx(pPlayBuffer);
    }
    if(RecordPlay == I2S_MODE_RX_TX)
    {
        UDMAChannelSelect(UDMA_CH4_I2S_RX, NULL);
        SetupPingPongDMATransferTx(pRecordBuffer);
    }

    //
    // Setup the Audio In/Out
    //     
    lRetVal = AudioSetupDMAMode(DMAPingPongCompleteAppCB_opt, \
                                 CB_EVENT_CONFIG_SZ, RecordPlay);
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }    
    AudioCaptureRendererConfigure(AUDIO_CODEC_16_BIT, 16000, AUDIO_CODEC_STEREO, RecordPlay, 1);

    //
    // Start Audio Tx/Rx
    //     
    Audio_Start(RecordPlay);

    //
    // Start the simplelink thread
    //
    lRetVal = VStartSimpleLinkSpawnTask(9);  
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }


    //
    // Start the Network Task
    //    
    lRetVal = osi_TaskCreate( Network, (signed char*)"NetworkTask",\
                               OSI_STACK_SIZE, NULL,
                               1, &g_NetworkTask );
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }

    //
    // Start the Control Task
    //     
    lRetVal = ControlTaskCreate();
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }    

    //
    // Start the Microphone Task
    //       
    lRetVal = osi_TaskCreate( Microphone,(signed char*)"MicroPhone", \
                               OSI_STACK_SIZE, NULL,
                               1, &g_MicTask );
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }

    //
    // Start the Speaker Task
    //
    lRetVal = osi_TaskCreate( Speaker, (signed char*)"Speaker",OSI_STACK_SIZE, \
                               NULL, 1, &g_SpeakerTask );
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }

    //
    // Start the task scheduler
    //
    osi_start();      
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
