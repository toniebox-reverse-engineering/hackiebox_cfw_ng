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
// Application Name     - DMA Reference Application
// Application Overview - Demonstrates the use of the DMA controller to transfer 
//                        data between memory buffers using Auto Mode and 
//                        Scatter Gather Mode, and to transfer data to and from 
//                        UART using Ping Pong Mode.In Auto and Scatter Gather 
//                        mode correctness of Data transfer is verified once the 
//                        transfer is complete. In Ping Pong mode transfer rate 
//                        is calculated and displayed.
//
//*****************************************************************************


//*****************************************************************************
//
//! \addtogroup dmaReferenceApp
//! @{
//
//*****************************************************************************

//Standard includes
#include <stdlib.h>
#include <string.h>

// Driverlib includes
#include "hw_common_reg.h"
#include "hw_memmap.h"
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_uart.h"
#include "uart.h"
#include "prcm.h"
#include "rom.h"
#include "rom_map.h"
#include "systick.h"
#include "utils.h"
#include "udma.h"
#include "interrupt.h"

// Common interface includes
#include "udma_if.h"
#include "uart_if.h"
#include "pinmux.h"

#define APPLICATION_VERSION     "1.4.0"
#define APP_NAME                "uDMA Reference"
#define UART_BAUD_RATE          115200
#define SYSCLK                  80000000
#define CONSOLE                 UARTA0_BASE
#define CONSOLE_PERIPH          PRCM_UARTA0
#define SYSTICKS_PER_SECOND     100
#define BUFF_SIZE               20
#define SYSTICK_RELOAD_VALUE    0x0000C3500
#define UART_PRINT              Report
//*****************************************************************************
//
// The size of the memory transfer source and destination buffers.
//
//*****************************************************************************
#define MEM_BUFFER_SIZE         1024

//*****************************************************************************
//
// The size of the UART transmit and receive buffers.  They do not need to be
// the same size.
//
//*****************************************************************************
#define UART_TXBUF_SIZE         256
#define UART_RXBUF_SIZE         256

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************

// The source and destination buffers used for memory transfers.
static unsigned long g_ulSrcBuf[MEM_BUFFER_SIZE];
static unsigned long g_ulDstBuf[MEM_BUFFER_SIZE];

// Task List used in Scatter Gather DMA Transfer
static tDMAControlTable TaskList[2];

extern volatile unsigned char iDone;

// The transmit and receive buffers used for the UART transfers.  There is one
// transmit buffer and a pair of recieve ping-pong buffers.
static unsigned char g_ucTxBuf[UART_TXBUF_SIZE];
static unsigned char g_ucRxBufA[UART_RXBUF_SIZE];
static unsigned char g_ucRxBufB[UART_RXBUF_SIZE];

// The count of UART buffers filled, one for each ping-pong buffer.
static unsigned long g_ulRxBufACount = 0;
static unsigned long g_ulRxBufBCount = 0;

// The number of seconds elapsed since the start of the program.  This value is
// maintained by the SysTick interrupt handler.
static unsigned long g_ulSeconds = 0;
static unsigned long g_ulTxCount = 0;

unsigned long ulBytesTransferred[100];
unsigned int uiCount=0;
volatile int Done=0;
int UARTDone=0;

// vector table entry
#if defined(ewarm)
    extern uVectorEntry __vector_table;
#endif

#if defined(ccs)
    extern void (* const g_pfnVectors[])(void);
#endif
//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************


//*****************************************************************************
//
//! Start a DMA transfer and wait for it to finish.
//!
//! This function will perform the steps necessary to initiate a DMA transfer
//! It waits until the transfer is complete before returning to the caller.
//!
//! \param ulChannel DMA Channel
//!
//! \return Zero if there is no error 
//
//*****************************************************************************
unsigned long
StartAndCompleteSWTransfer(unsigned long ulChannel)
{
    unsigned long ulIdx;
    //
    // Clear interrupt status
    //
    MAP_uDMAIntClear(0xffffffff);
    MAP_uDMAChannelRequest(ulChannel);
    //
    // Wait for the mode to change to stopped
    //
    ulIdx = 0;
    while(MAP_uDMAChannelModeGet(ulChannel) != UDMA_MODE_STOP)
    {
        ulIdx++;
        if(ulIdx > 200000)
        {
            break;
        }
    }
    if(MAP_uDMAChannelIsEnabled(ulChannel))
    {
        UART_PRINT("Error, channel was enabled at end of transfer\n");
        return 1;
    }
    UART_PRINT("Transfer complete \n\r");
    return(0);
}

//*****************************************************************************
//!
//! Initializes the uDMA software channel to perform a memory to memory uDMA
//! transfer.
//!
//! \param None
//!
//! \return None
//*****************************************************************************
void
InitSWTransfer(void)
{
    unsigned int uIdx;

    //
    // Fill the source memory buffer with a simple incrementing pattern.
    //
    for(uIdx = 0; uIdx < MEM_BUFFER_SIZE; uIdx++)
    {
        g_ulSrcBuf[uIdx] = uIdx;
    }

    //
    // Configure the control parameters for the SW channel.  The SW channel
    // will be used to transfer between two memory buffers, 32 bits at a time.
    // Therefore the data size is 32 bits, and the address increment is 32 bits
    // for both source and destination.  The arbitration size will be set to 8,
    // which causes the uDMA controller to rearbitrate after 8 items are
    // transferred.  This keeps this channel from hogging the uDMA controller
    // once the transfer is started, and allows other channels cycles if they
    // are higher priority.
    //

    UDMASetupTransfer(UDMA_CH0_SW, UDMA_MODE_AUTO, MEM_BUFFER_SIZE,
                      UDMA_SIZE_32, UDMA_ARB_8,g_ulSrcBuf, UDMA_SRC_INC_32,
                                            g_ulDstBuf, UDMA_DST_INC_32);

    if(StartAndCompleteSWTransfer(UDMA_CH0_SW))
    {
      UART_PRINT("Memory to Memory Transfer Error \n\r");
    }
    //
    // Verifying the transfered Data
    //
    for(uIdx = 0; uIdx < MEM_BUFFER_SIZE; uIdx++)
    {
      if(uIdx!=g_ulDstBuf[uIdx])
        {
          UART_PRINT("Data transfered in Auto mode is Incorrect at %d th "
                        "location \n\r\n\r",uIdx);
          Done=1;
          return;

        }
    }
    UART_PRINT("Data transfered in Auto mode is verified \n\r\n\r");
    Done=1;

}

//*****************************************************************************
//!
//! The interrupt handler for UART0.  This interrupt will occur when a DMA
//! transfer is complete using the UART0 uDMA channel.This interrupt handler will
//! switch between receive ping-pong buffers A and B.  It will also restart a TX
//! uDMA transfer if the prior transfer is complete.  This will keep the UART
//! running continuously (looping TX data back to RX).
//!
//! \param None
//!
//! \return None
//*****************************************************************************
void
UART0IntHandler(void)
{
    unsigned long ulStatus;
    unsigned long ulMode;
    //
    // Read the interrupt status of the UART.
    //
    ulStatus = MAP_UARTIntStatus(UARTA0_BASE, 1);
    //
    // Clear any pending status, even though there should be none since no UART
    // interrupts were enabled.  
    //
    MAP_UARTIntClear(UARTA0_BASE, ulStatus);
    if(uiCount<6)
    {
    //
    // Check the DMA control table to see if the ping-pong "A" transfer is
    // complete.  The "A" transfer uses receive buffer "A", and the primary
    // control structure.
    //
    ulMode = MAP_uDMAChannelModeGet(UDMA_CH8_UARTA0_RX | UDMA_PRI_SELECT);

    //
    // If the primary control structure indicates stop, that means the "A"
    // receive buffer is done.  The uDMA controller should still be receiving
    // data into the "B" buffer.
    //
    if(ulMode == UDMA_MODE_STOP)
    {
        //
        // Increment a counter to indicate data was received into buffer A.  
        //
        g_ulRxBufACount++;

        //
        // Set up the next transfer for the "A" buffer, using the primary
        // control structure.  When the ongoing receive into the "B" buffer is
        // done, the uDMA controller will switch back to this one.  
        //
        UDMASetupTransfer(UDMA_CH8_UARTA0_RX | UDMA_PRI_SELECT, UDMA_MODE_PINGPONG,
                          sizeof(g_ucRxBufA),UDMA_SIZE_8, UDMA_ARB_4,
                          (void *)(UARTA0_BASE + UART_O_DR), UDMA_SRC_INC_NONE,
                                            g_ucRxBufA, UDMA_DST_INC_8);
    }

    //
    // Check the DMA control table to see if the ping-pong "B" transfer is
    // complete.  The "B" transfer uses receive buffer "B", and the alternate
    // control structure.
    //
    ulMode = MAP_uDMAChannelModeGet(UDMA_CH8_UARTA0_RX | UDMA_ALT_SELECT);

    //
    // If the alternate control structure indicates stop, that means the "B"
    // receive buffer is done.  The uDMA controller should still be receiving
    // data into the "A" buffer.
    //
    if(ulMode == UDMA_MODE_STOP)
    {
        //
        // Increment a counter to indicate data was received into buffer A. 
        //
        g_ulRxBufBCount++;

        //
        // Set up the next transfer for the "B" buffer, using the alternate
        // control structure.  When the ongoing receive into the "A" buffer is
        // done, the uDMA controller will switch back to this one. 
        //
        UDMASetupTransfer(UDMA_CH8_UARTA0_RX | UDMA_ALT_SELECT,
                          UDMA_MODE_PINGPONG, sizeof(g_ucRxBufB),UDMA_SIZE_8,
                          UDMA_ARB_4,(void *)(UARTA0_BASE + UART_O_DR), 
                          UDMA_SRC_INC_NONE, g_ucRxBufB, UDMA_DST_INC_8);
    }

    //
    // If the UART0 DMA TX channel is disabled, that means the TX DMA transfer
    // is done.
    //
    if(!MAP_uDMAChannelIsEnabled(UDMA_CH9_UARTA0_TX))
    {
        g_ulTxCount++;
        //
        // Start another DMA transfer to UART0 TX.
        //
        UDMASetupTransfer(UDMA_CH9_UARTA0_TX| UDMA_PRI_SELECT, UDMA_MODE_BASIC,
           sizeof(g_ucTxBuf),UDMA_SIZE_8, UDMA_ARB_4,g_ucTxBuf, UDMA_SRC_INC_8,
                          (void *)(UARTA0_BASE + UART_O_DR), UDMA_DST_INC_NONE);
        //
        // The uDMA TX channel must be re-enabled.
        //
        MAP_uDMAChannelEnable(UDMA_CH9_UARTA0_TX);
    }
    }
    else
    {
        UARTDone=1;
        MAP_UARTIntUnregister(UARTA0_BASE);
    }
}

//*****************************************************************************
//
//! Initializes the UART0 peripheral and sets up the TX and RX uDMA channels.
//! The UART is configured for loopback mode so that any data sent on TX will be
//! received on RX.  The uDMA channels are configured so that the TX channel
//! will copy data from a buffer to the UART TX output.  And the uDMA RX channel
//! will receive any incoming data into a pair of buffers in ping-pong mode.
//!
//! \param None
//!
//! \return None
//!
//*****************************************************************************
void
InitUART0Transfer(void)
{
    unsigned int uIdx;

    //
    // Fill the TX buffer with a simple data pattern.
    //
    for(uIdx = 0; uIdx < UART_TXBUF_SIZE; uIdx++)
    {
        g_ucTxBuf[uIdx] = 65;
    }
    MAP_PRCMPeripheralReset(PRCM_UARTA0);
    MAP_PRCMPeripheralClkEnable(PRCM_UARTA0,PRCM_RUN_MODE_CLK);

    MAP_UARTConfigSetExpClk(CONSOLE,SYSCLK, UART_BAUD_RATE,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                                        UART_CONFIG_PAR_NONE));
    MAP_uDMAChannelAssign(UDMA_CH8_UARTA0_RX);
    MAP_uDMAChannelAssign(UDMA_CH9_UARTA0_TX);
    MAP_UARTIntRegister(UARTA0_BASE,UART0IntHandler);
    
    //
    // Set both the TX and RX trigger thresholds to 4.  This will be used by
    // the uDMA controller to signal when more data should be transferred.  The
    // uDMA TX and RX channels will be configured so that it can transfer 4
    // bytes in a burst when the UART is ready to transfer more data.
    //
    MAP_UARTFIFOLevelSet(UARTA0_BASE, UART_FIFO_TX4_8, UART_FIFO_RX4_8);

    //
    // Enable the UART for operation, and enable the uDMA interface for both TX
    // and RX channels.
    //
    MAP_UARTEnable(UARTA0_BASE);

    //
    // This register write will set the UART to operate in loopback mode.  Any
    // data sent on the TX output will be received on the RX input.
    //
    HWREG(UARTA0_BASE + UART_O_CTL) |= UART_CTL_LBE;

    //
    // Enable the UART peripheral interrupts. uDMA controller will cause an 
    // interrupt on the UART interrupt signal when a uDMA transfer is complete.
    //
   
    MAP_UARTIntEnable(UARTA0_BASE,UART_INT_DMATX);
    MAP_UARTIntEnable(UARTA0_BASE,UART_INT_DMARX);

    //
    // Configure the control parameters for the UART TX.  The uDMA UART TX
    // channel is used to transfer a block of data from a buffer to the UART.
    // The data size is 8 bits.  The source address increment is 8-bit bytes
    // since the data is coming from a buffer.  The destination increment is
    // none since the data is to be written to the UART data register.  The
    // arbitration size is set to 4, which matches the UART TX FIFO trigger
    // threshold.
    //
    UDMASetupTransfer(UDMA_CH8_UARTA0_RX | UDMA_PRI_SELECT, UDMA_MODE_PINGPONG,
            sizeof(g_ucRxBufA),UDMA_SIZE_8, UDMA_ARB_4,
            (void *)(UARTA0_BASE + UART_O_DR), UDMA_SRC_INC_NONE,
                                            g_ucRxBufA, UDMA_DST_INC_8);

    UDMASetupTransfer(UDMA_CH8_UARTA0_RX | UDMA_ALT_SELECT, UDMA_MODE_PINGPONG,
            sizeof(g_ucRxBufB),UDMA_SIZE_8, UDMA_ARB_4,
              (void *)(UARTA0_BASE + UART_O_DR), UDMA_SRC_INC_NONE,
                                            g_ucRxBufB, UDMA_DST_INC_8);

    UDMASetupTransfer(UDMA_CH9_UARTA0_TX| UDMA_PRI_SELECT,
               UDMA_MODE_BASIC,sizeof(g_ucTxBuf),UDMA_SIZE_8, UDMA_ARB_4,
               g_ucTxBuf, UDMA_SRC_INC_8,(void *)(UARTA0_BASE + UART_O_DR), 
                                                    UDMA_DST_INC_NONE);
    
    MAP_UARTDMAEnable(UARTA0_BASE, UART_DMA_RX | UART_DMA_TX);
}


//*****************************************************************************
//
//! Application startup display on UART
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
void
DisplayBanner()
{
    UART_PRINT("\n\n\n\r");
    UART_PRINT("\t\t   *******************************************\n\r");
    UART_PRINT("\t\t        CC3200 %s Application        \n\r", APP_NAME);
    UART_PRINT("\t\t   *******************************************\n\r");
    UART_PRINT("\n\n\n\r");

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
static void
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

//*****************************************************************************
//!
//! The interrupt handler for the SysTick timer.  This handler will increment a
//! seconds counter whenever the appropriate number of ticks has occurred. 
//!
//! \param None
//! 
//! \return None
//!
//*****************************************************************************
void
SysTickHandler(void)
{
    static unsigned long ulTickCount = 0;
    
    //
    // Increment the tick counter.
    //
    ulTickCount++;
    
    //
    // If the number of ticks per second has occurred, then increment the
    // seconds counter.
    //
    if(!(ulTickCount % SYSTICKS_PER_SECOND))
    {
        g_ulSeconds++;
    }

}

//*****************************************************************************
//!
//! Set up Scatter Gather DMA Transfer
//!
//! \param ulChannel DMA channel to be used.
//! \param pvSrcBuf0 Source of Task1
//! \param pvSrcBuf1 Source of Task2
//! \param pvDstBuf  Destination Buffer
//! \param size  Size to be transfered
//! 
//! \return None
//!
//*****************************************************************************

static void  
SetupSGMemTransfer(unsigned long ulChannel,void *pvSrcBuf0,
                             void *pvSrcBuf1,void *pvDstBuf,unsigned long size)
{
    //
    // Create task0
    //
    tDMAControlTable task0=uDMATaskStructEntry(size, UDMA_SIZE_8,
                            UDMA_SRC_INC_8, pvSrcBuf0,
                             UDMA_DST_INC_8,pvDstBuf,
                            UDMA_ARB_8, UDMA_MODE_MEM_SCATTER_GATHER);

    //
    // create task1
    //
    tDMAControlTable task1=uDMATaskStructEntry(size, UDMA_SIZE_8,
                            UDMA_SRC_INC_8, pvSrcBuf1,
                             UDMA_DST_INC_8, (char *)pvDstBuf+size,
                            UDMA_ARB_8, UDMA_MODE_AUTO);

    //
    // Copy to TaskList
    //
    memcpy(&TaskList[0],&task0,sizeof(tDMAControlTable));
    memcpy(&TaskList[1],&task1,sizeof(tDMAControlTable));
    memset(pvDstBuf,0,size);
    //
    // Setup Scatter Gather with two tasks
    //
    MAP_uDMAChannelScatterGatherSet(ulChannel, 2,TaskList,0);
    MAP_uDMAChannelEnable(ulChannel);
}

//*****************************************************************************
//!
//! Initialize Scatter Gather DMA Transfer
//!
//! \param None
//! 
//! \return None
//!
//*****************************************************************************
void
InitSGTransfer()
{
    int i;
    char * pvSrcBuf,*pvDstBuf;
    
    //
    // Source Buffer
    //
    pvSrcBuf=malloc(BUFF_SIZE);
    if(pvSrcBuf == NULL)
    {
        UART_PRINT("Failed to allocate Src buffer\n\r");
        return ;
    }
    for(i=0;i<BUFF_SIZE;i++)
        *(pvSrcBuf+i)=i;

    MAP_uDMAChannelAssign(UDMA_CH0_SW);
    MAP_uDMAChannelAttributeDisable(UDMA_CH0_SW,UDMA_ATTR_ALTSELECT);
    iDone=0;

    pvDstBuf=malloc(BUFF_SIZE);
    if(pvDstBuf == NULL)
    {
        UART_PRINT("Failed to allocate Dst buffer\n\r");
        return ;
    }
    
    //
    // Call to Set Up Scatter Gather Mode Transfer
    //
    SetupSGMemTransfer(UDMA_CH0_SW,pvSrcBuf,pvSrcBuf+BUFF_SIZE/2,
                       pvDstBuf,BUFF_SIZE/2);
    MAP_uDMAChannelRequest(UDMA_CH0_SW);
    
    //
    // Checking for the Completion
    //
    while(!iDone);
    
    //
    // Checking Correctness of SG Transfer
    //
    if(memcmp(pvSrcBuf,pvDstBuf,BUFF_SIZE)==0)
        UART_PRINT("SG Mode Memory to Memory Transfer Completed \n\r\n\r");
    else
        UART_PRINT("SG Mode Memory to Memory Transfer Failed! \n\r\n\r");

    free(pvDstBuf);

}

//*****************************************************************************
//
//! Main function for uDMA Application 
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
void
main()
{
    static unsigned long ulPrevSeconds;
    static unsigned long ulPrevUARTCount = 0;
    unsigned long ulXfersCompleted;
    unsigned long ulBytesAvg=0;
    int iCount=0;
    
	//
    // Initailizing the board
    //
    BoardInit();
    //
    // Muxing for Enabling UART_TX and UART_RX.
    //
    PinMuxConfig();
    
    //
    // Initialising the Terminal.
    //
    InitTerm();
    
    //
    // Display Welcome Message
    //
    DisplayBanner();
    
    //
    // SysTick Enabling
    //
    SysTickIntRegister(SysTickHandler);
    SysTickPeriodSet(SYSTICK_RELOAD_VALUE);
    SysTickEnable();

    //
    // uDMA Initialization
    //
    UDMAInit();
    
    //
    // Register interrupt handler for UART
    //
    MAP_UARTIntRegister(UARTA0_BASE,UART0IntHandler);
    
    UART_PRINT("Completed DMA Initialization \n\r\n\r");
    
    //
    // Auto DMA Transfer
    //
    UART_PRINT("Starting Auto DMA Transfer  \n\r");
    InitSWTransfer();
    
    //
    // Scatter Gather DMA Transfer
    //
    UART_PRINT("Starting Scatter Gather DMA Operation\n\r");
    InitSGTransfer();
    while(!Done){}
    MAP_UtilsDelay(80000000);
    
    //
    // Ping Pong UART Transfer
    //
    InitUART0Transfer();
    
    //
    // Remember the current SysTick seconds count.
    //
    ulPrevSeconds = g_ulSeconds;

    while(1)
    {
        //
        // Check to see if one second has elapsed.  If so, the make some
        // updates.
        //
        if(g_ulSeconds != ulPrevSeconds)
        {

            uiCount++;
    
            //
            // Remember the new seconds count.
            //
            ulPrevSeconds = g_ulSeconds;
            
            //
            // Calculate how many UART transfers have occurred since the last
            // second.
            //
            ulXfersCompleted = (g_ulRxBufACount + g_ulRxBufBCount -
                                ulPrevUARTCount);

            //
            // Remember the new UART transfer count.
            //
            ulPrevUARTCount = g_ulRxBufACount + g_ulRxBufBCount;
            
            //
            // Compute how many bytes were transferred by the UART.
            //
            ulBytesTransferred[iCount] = (ulXfersCompleted * UART_RXBUF_SIZE );
            iCount++;
            
            //
            // Print a message to the display showing the memory transfer rate.
            //
            if(UARTDone)
            {
                MAP_PRCMPeripheralReset(PRCM_UARTA0);
                MAP_PRCMPeripheralClkEnable(PRCM_UARTA0,PRCM_RUN_MODE_CLK);
                UARTConfigSetExpClk(CONSOLE,SYSCLK, UART_BAUD_RATE,
                                    (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                                                        UART_CONFIG_PAR_NONE));
            }

        }
        
        //
        // See if we have run long enough and exit the loop if so.
        //
        if(g_ulSeconds >= 6 && UARTDone)
        {
            break;
        }
    }
    
    //
    // Compute average Bytes Transfer Rate for the past 5 seconds
    //
    for(iCount=1;iCount<=5;iCount++)
    {
        ulBytesAvg += ulBytesTransferred[iCount];
    }
    ulBytesAvg=ulBytesAvg/5;

    UART_PRINT("\n\r");
    UART_PRINT("\n\r");

    UART_PRINT("\n\rCompleted Ping Pong Transfer from Memory to UART "
                "Peripheral \n\r");
    UART_PRINT(" \n\rTransfer Rate is %lu Bytes/Sec \n\r", ulBytesAvg);
    UART_PRINT("\n\rTest Ended\n\r");
    return ;

}


//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************




