//*****************************************************************************
// pcm_handler.c
//
// PCM Handler Interface
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
//! \addtogroup WifiAudioPlayer
//! @{
//
//*****************************************************************************
/* Standard includes. */
#include <stdio.h>
#include <string.h>


/* Hardware & DriverLib library includes. */
#include "hw_types.h"
#include "hw_mcasp.h"
#include "hw_udma.h"
#include "hw_ints.h"
#include "rom.h"
#include "rom_map.h"
#include "debug.h"
#include "interrupt.h"
#include "hw_memmap.h"
#include "i2s.h"
#include "udma.h"
#include "pin.h"
#include "circ_buff.h"
#include "pcm_handler.h"
#include "udma_if.h"
#include "i2c_if.h"
#include "uart_if.h"



//*****************************************************************************
//                          LOCAL DEFINES
//*****************************************************************************
#define UDMA_DSTSZ_32_SRCSZ_16          0x21000000
#define mainQUEUE_SIZE                3
#define PLAY_WATERMARK		30*1024

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
tCircularBuffer *g_pRecordBuffer;
tCircularBuffer *g_pPlayBuffer;
extern unsigned int g_uiPlayWaterMark;

unsigned char gaucZeroBuffer[(CB_TRANSFER_SZ * HALF_WORD_SIZE)];
unsigned int g_uiPlayWaterMark = 0;

//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************


//*****************************************************************************
//
//! Callback function implementing ping pong mode DMA transfer
//!
//! \param None
//!
//! This function
//!        1. sets the primary and the secondary buffer for ping pong transfer.
//!           2. sets the control table.
//!           3. transfer the appropriate data according to the flags.
//!
//! \return None.
//
//*****************************************************************************
void DMAPingPongCompleteAppCB_opt()
{
    unsigned long ulPrimaryIndexTx = 0x4;
    unsigned long ulAltIndexTx = 0x24;
    unsigned long ulPrimaryIndexRx = 0x5;
    unsigned long ulAltIndexRx = 0x25;
    tDMAControlTable *pControlTable;

    if(MAP_uDMAIntStatus() & 0x00000010)
    {
    	I2SIntClear(I2S_BASE, I2S_INT_RDMA);

        pControlTable = MAP_uDMAControlBaseGet();

        if((pControlTable[ulPrimaryIndexTx].ulControl & UDMA_CHCTL_XFERMODE_M) \
                                                                                == 0)
        {
                MAP_uDMAChannelTransferSet(UDMA_CH4_I2S_RX,
                                             UDMA_MODE_PINGPONG,
                                             (void *)I2S_RX_DMA_PORT,
                                             (void *)GetWritePtr(g_pRecordBuffer),
                                             CB_TRANSFER_SZ);
                 MAP_uDMAChannelEnable(UDMA_CH4_I2S_RX);
                 UpdateWritePtr(g_pRecordBuffer, (2*CB_TRANSFER_SZ));
        }
        else if((pControlTable[ulAltIndexTx].ulControl & UDMA_CHCTL_XFERMODE_M) \
                                                                        == 0)
        {
                MAP_uDMAChannelTransferSet(UDMA_CH4_I2S_RX|UDMA_ALT_SELECT,
                                             UDMA_MODE_PINGPONG,
                                             (void *)I2S_RX_DMA_PORT,
                                             (void *)GetWritePtr(g_pRecordBuffer),
                                             CB_TRANSFER_SZ );
                 MAP_uDMAChannelEnable(UDMA_CH4_I2S_RX|UDMA_ALT_SELECT);
                 UpdateWritePtr(g_pRecordBuffer, (2*CB_TRANSFER_SZ));
         }
    }

    if(MAP_uDMAIntStatus() & 0x00000020)
    {
        //Clear the MCASP write interrupt
		I2SIntClear(I2S_BASE, I2S_INT_XDMA);
        pControlTable = MAP_uDMAControlBaseGet();
        if((pControlTable[ulPrimaryIndexRx].ulControl & UDMA_CHCTL_XFERMODE_M) \
                                                                                == 0)
        {

            if( IsBufferEmpty(g_pPlayBuffer) || !g_uiPlayWaterMark )
            {
                g_uiPlayWaterMark = IsBufferSizeFilled(g_pPlayBuffer,PLAY_WATERMARK);
                MAP_uDMAChannelTransferSet(UDMA_CH5_I2S_TX,
                                            UDMA_MODE_PINGPONG,
                                            (void *)&gaucZeroBuffer[0],
                                            (void *)I2S_TX_DMA_PORT,
                                            CB_TRANSFER_SZ );
            }
            else
            {
                MAP_uDMAChannelTransferSet(UDMA_CH5_I2S_TX,
                                            UDMA_MODE_PINGPONG,
                                            (void *)GetReadPtr(g_pPlayBuffer),
                                            (void *)I2S_TX_DMA_PORT,
                                            CB_TRANSFER_SZ );
                 UpdateReadPtr(g_pPlayBuffer, (2*CB_TRANSFER_SZ));
            }
            MAP_uDMAChannelEnable(UDMA_CH5_I2S_TX);
        }
        else if((pControlTable[ulAltIndexRx].ulControl & UDMA_CHCTL_XFERMODE_M) \
                == 0)
        {
            if( IsBufferEmpty(g_pPlayBuffer) || !g_uiPlayWaterMark )
            {
                 g_uiPlayWaterMark = IsBufferSizeFilled(g_pPlayBuffer,PLAY_WATERMARK);
                 MAP_uDMAChannelTransferSet(UDMA_CH5_I2S_TX|UDMA_ALT_SELECT,
                                              UDMA_MODE_PINGPONG,
                                              (void *)&gaucZeroBuffer[0],
                                              (void *)I2S_TX_DMA_PORT,
                                              CB_TRANSFER_SZ );
            }
            else
            {
                MAP_uDMAChannelTransferSet(UDMA_CH5_I2S_TX|UDMA_ALT_SELECT,
                                             UDMA_MODE_PINGPONG,
                                             (void *)GetReadPtr(g_pPlayBuffer),
                                             (void *)I2S_TX_DMA_PORT,
                                             CB_TRANSFER_SZ );
                 UpdateReadPtr(g_pPlayBuffer, (2*CB_TRANSFER_SZ));

            }
            MAP_uDMAChannelEnable(UDMA_CH5_I2S_TX|UDMA_ALT_SELECT);
        }
    }

}

//*****************************************************************************
//
//! configuring the DMA transfer
//!
//! \param psAudPlayerCtrl is the control structure managing the input data.
//!
//! This function
//!        1. setting the source and the destination for the DMA transfer.
//!           2. setting the uDMA registers to control actual transfer.
//!
//! \return None.
//
//*****************************************************************************
void SetupPingPongDMATransferTx(tCircularBuffer *pRecordBuffer)
{
    
    g_pRecordBuffer = pRecordBuffer;

    UDMASetupTransfer(UDMA_CH4_I2S_RX,
                      UDMA_MODE_PINGPONG,
                      CB_TRANSFER_SZ,
                      UDMA_SIZE_16,
                      UDMA_ARB_8,
                      (void *)I2S_RX_DMA_PORT,
                      UDMA_CHCTL_SRCINC_NONE,
                      (void *)GetWritePtr(pRecordBuffer),
                      UDMA_CHCTL_DSTINC_16);


    UDMASetupTransfer(UDMA_CH4_I2S_RX|UDMA_ALT_SELECT,
                      UDMA_MODE_PINGPONG,
                      CB_TRANSFER_SZ,
                      UDMA_SIZE_16,
                      UDMA_ARB_8,
                      (void *)I2S_RX_DMA_PORT,
                      UDMA_CHCTL_SRCINC_NONE,
                      (void *)(GetWritePtr(pRecordBuffer)+(2*CB_TRANSFER_SZ)),
                      UDMA_CHCTL_DSTINC_16);
}

void SetupPingPongDMATransferRx(tCircularBuffer *pPlayBuffer)
{
	g_pPlayBuffer = pPlayBuffer;
	
    UDMASetupTransfer(UDMA_CH5_I2S_TX,
                      UDMA_MODE_PINGPONG,
                      CB_TRANSFER_SZ,
                      UDMA_SIZE_16,
                      UDMA_ARB_8,
                      (void *)GetReadPtr(pPlayBuffer),
                      UDMA_CHCTL_SRCINC_16,
                      (void *)I2S_TX_DMA_PORT,
                      UDMA_DST_INC_NONE);

    UDMASetupTransfer(UDMA_CH5_I2S_TX|UDMA_ALT_SELECT,
                      UDMA_MODE_PINGPONG,
                      CB_TRANSFER_SZ,
                      UDMA_SIZE_16,
                      UDMA_ARB_8,
                      (void *)(GetReadPtr(pPlayBuffer)+(2*CB_TRANSFER_SZ)),
                      UDMA_CHCTL_SRCINC_16,
                      (void *)I2S_TX_DMA_PORT,
                      UDMA_DST_INC_NONE);
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
