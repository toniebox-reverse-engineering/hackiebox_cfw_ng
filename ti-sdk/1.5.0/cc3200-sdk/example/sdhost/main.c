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
// Application Name     - SDHost
// Application Overview - This application showcases the basic use case of
//                        initializing the controller to communicate with the
//                        attached card, reading and writing SD card block using
//                        the internal controller buffer via DriverLib API(s).
//                        The demo includes extracting the card capacity,
//                        capacity class and SD version information and display
//                        it on the UART terminal.
//*****************************************************************************

//*****************************************************************************
//
//! \addtogroup SDHost
//! @{
//
//*****************************************************************************

// Driverlib includes
#include "hw_common_reg.h"
#include "hw_types.h"
#include "hw_memmap.h"
#include "hw_ints.h"
#include "sdhost.h"
#include "rom.h"
#include "rom_map.h"
#include "prcm.h"
#include "pin.h"
#include "interrupt.h"

// Common interface includes
#include "uart_if.h"

#include "sdhost_demo.h"
#include "utils.h"
#include "pinmux.h"
#include "stdcmd.h"

#define APPLICATION_VERSION  "1.4.0"
#define WRITE_TEST_EN        0

#if WRITE_TEST_EN
#warning Enabling WRITE_TEST_EN will corrupt the existing filesystem on the card
#endif

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
static unsigned char g_ucDataBuff[512];

#if defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif
//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************



//*****************************************************************************
//
//! Send Command to card
//!
//! \param ulCmd is the command to be send
//! \paran ulArg is the command argument
//!
//! This function sends command to attached card and check the response status
//! if any.
//!
//! \return Returns 0 on success, 1 otherwise
//
//*****************************************************************************
static unsigned long
SendCmd(unsigned long ulCmd, unsigned long ulArg)
{
    unsigned long ulStatus;

    //
    // Clear interrupt status
    //
    MAP_SDHostIntClear(SDHOST_BASE,0xFFFFFFFF);

    //
    // Send command
    //
    MAP_SDHostCmdSend(SDHOST_BASE,ulCmd,ulArg);

    //
    // Wait for command complete or error
    //
    do
    {
        ulStatus = MAP_SDHostIntStatus(SDHOST_BASE);
        ulStatus = (ulStatus & (SDHOST_INT_CC|SDHOST_INT_ERRI));
    }
    while( !ulStatus );

    //
    // Check error status
    //
    if(ulStatus & SDHOST_INT_ERRI)
    {
        //
        // Reset the command line
        //
        MAP_SDHostCmdReset(SDHOST_BASE);
        return 1;
    }
    else
    {
        return 0;
    }
}

//*****************************************************************************
//
//! Get the capacity of specified card
//!
//! \param ulRCA is the Relative Card Address (RCA)
//!
//! This function gets the capacity of card addressed by \e ulRCA paramaeter.
//!
//! \return Returns card capacity on success, 0 otherwise.
//
//*****************************************************************************
static unsigned long long
CardCapacityGet(unsigned short ulRCA)
{
    unsigned long ulRet;
    unsigned long ulResp[4];
    unsigned long long ullCapacity;
    unsigned long ulBlockSize;
    unsigned long ulBlockCount;
    unsigned long ulCSizeMult;
    unsigned long ulCSize;

    //
    // Read the CSD register
    //
    ulRet = SendCmd(CMD_SEND_CSD,(ulRCA << 16));
    if(ulRet == 0)
    {
        //
        // Read the response
        //
        MAP_SDHostRespGet(SDHOST_BASE,ulResp);
    }
    else
    {
        return 0;
    }

    //
    // 136 bit CSD register is read into an array of 4 words.
    // ulResp[0] = CSD[31:0]
    // ulResp[1] = CSD[63:32]
    // ulResp[2] = CSD[95:64]
    // ulResp[3] = CSD[127:96]
    //
    if( ((ulResp[3] >> 30) & 0x1) == 1)
    {
        ulBlockSize = 512 * 1024;
        ulBlockCount = (ulResp[1] >> 16 | ((ulResp[2] & 0x3F) << 16)) + 1;
    }
    else
    {
        ulBlockSize  = 1 << ((ulResp[2] >> 16) & 0xF);
        ulCSizeMult  = ((ulResp[1] >> 15) & 0x7);
        ulCSize      = ((ulResp[1] >> 30) | (ulResp[2] & 0x3FF) << 2);
        ulBlockCount = (ulCSize + 1) * (1<<(ulCSizeMult + 2));
    }

    //
    // Calculate the card capacity in bytes
    //
    ullCapacity = (unsigned long long)ulBlockSize * (ulBlockCount );

    //
    // Return the capacity
    //
    return ullCapacity;
}

//*****************************************************************************
//
//! Initializes the card and fills the card attribute structure.
//!
//! \param CardAttrib is the pointer to card attribute structure.
//!
//! This function initializes the card and fills in the card attribute
//! structure.
//!
//! \return Returns 0 success, 1 otherwise.
//
//*****************************************************************************
static unsigned long
CardInit(CardAttrib_t *CardAttrib)
{
    unsigned long ulRet;
    unsigned long ulResp[4];

    //
    // Initialize the attributes.
    //
    CardAttrib->ulCardType = CARD_TYPE_UNKNOWN;
    CardAttrib->ulCapClass = CARD_CAP_CLASS_SDSC;
    CardAttrib->ulRCA      = 0;
    CardAttrib->ulVersion  = CARD_VERSION_1;

    //
    // Send std GO IDLE command
    //
    if( SendCmd(CMD_GO_IDLE_STATE, 0) == 0)
    {

        ulRet = SendCmd(CMD_SEND_IF_COND,0x00000100);

        //
        // It's a SD ver 2.0 or higher card
        //
        if(ulRet == 0)
        {
            CardAttrib->ulVersion = CARD_VERSION_2;
            CardAttrib->ulCardType = CARD_TYPE_SDCARD;

            //
            // Wait for card to become ready.
            //
            do
            {
                //
                // Send ACMD41
                //
                SendCmd(CMD_APP_CMD,0);
                ulRet = SendCmd(CMD_SD_SEND_OP_COND,0x40E00000);

                //
                // Response contains 32-bit OCR register
                //
                MAP_SDHostRespGet(SDHOST_BASE,ulResp);

            }while(((ulResp[0] >> 31) == 0));

            if(ulResp[0] & (1UL<<30))
            {
            CardAttrib->ulCapClass = CARD_CAP_CLASS_SDHC;
            }

        }
        else //It's a MMC or SD 1.x card
        {

            //
            // Wait for card to become ready.
            //
            do
            {
                if( (ulRet = SendCmd(CMD_APP_CMD,0)) == 0 )
                {
                    ulRet = SendCmd(CMD_SD_SEND_OP_COND,0x00E00000);

                    //
                    // Response contains 32-bit OCR register
                    //
                    MAP_SDHostRespGet(SDHOST_BASE,ulResp);
                }
            }while((ulRet == 0) && ((ulResp[0] >> 31) == 0));

            //
            // Check the response
            //
            if(ulRet == 0)
            {
                CardAttrib->ulCardType = CARD_TYPE_SDCARD;
            }
            else // CMD 55 is not recognised by SDHost cards.
            {
                //
                // Confirm if its a SDHost card
                //
                ulRet = SendCmd(CMD_SEND_OP_COND,0);
                if( ulRet == 0)
                {
                    CardAttrib->ulCardType = CARD_TYPE_MMC;
                }
            }
        }
    }

    //
    // Get the RCA of the attached card
    //
    if(ulRet == 0)
    {
        ulRet = SendCmd(CMD_ALL_SEND_CID,0);
        if( ulRet == 0)
        {
            SendCmd(CMD_SEND_REL_ADDR,0);
            MAP_SDHostRespGet(SDHOST_BASE,ulResp);

            //
            //  Fill in the RCA
            //
            CardAttrib->ulRCA = (ulResp[0] >> 16);

            //
            // Get tha card capacity
            //
            CardAttrib->ullCapacity = CardCapacityGet(CardAttrib->ulRCA);
        }
    }

    //
    // return status.
    //
    return ulRet;
}

//*****************************************************************************
//
//! Select a card for reading or writing
//!
//! \param Card is the pointer to card attribute structure.
//!
//! This function selects a card for reading or writing using its RCA from
//! \e Card parameter.
//!
//! \return Returns 0 success, 1 otherwise.
//
//*****************************************************************************
static unsigned long
CardSelect(CardAttrib_t *Card)
{
    unsigned long ulRet;

    //
    // Send select command with card's RCA.
    //
    ulRet = SendCmd(CMD_SELECT_CARD, (Card->ulRCA << 16));

    if(ulRet == 0)
    {
        while( !(MAP_SDHostIntStatus(SDHOST_BASE) & SDHOST_INT_TC) )
        {

        }
    }

    //
    // Delay for card to become ready
    //
    MAP_UtilsDelay(80000000/12);

    return ulRet;
}

//*****************************************************************************
//
//! Deselects previously selected card.
//!
//! This function deselects previously selected card.
//!
//! \return None.
//
//*****************************************************************************
static void
CardDeselect()
{
    //
    // Send deselect command
    //
    SendCmd(CMD_DESELECT_CARD,0);
}

//*****************************************************************************
//
//! Reads a block of data.
//!
//! \param Card is pointer to a valid card attrib structure.
//! \param pBuffer is pointer to the buffer to be read into.
//! \param ulBlockNo is stating block number.
//! \param ulBlockCount is number of block to be read.
//!
//! This function reads specified number of block into \e pBuffer. Each block
//! is of 512 byte.
//!
//! \return Returns 0 on success, 1 otherwise.
//
//*****************************************************************************
static unsigned long
CardReadBlock(CardAttrib_t *Card, unsigned char *pBuffer,
              unsigned long ulBlockNo, unsigned long ulBlockCount)
{

    unsigned long ulSize;


    //
    // SDSC linear address instead of block address
    //
    if(Card->ulCapClass == CARD_CAP_CLASS_SDSC)
    {
    ulBlockNo = ulBlockNo * 512;
    }

    //
    // Set the number of block on the host
    //
    MAP_SDHostBlockCountSet(SDHOST_BASE,ulBlockCount);

    //
    // Compute total size in words.
    //
    ulSize = (512*ulBlockCount)/4;

    //
    // Send multi block read command to the card
    //
    if( SendCmd(CMD_READ_MULTI_BLK, ulBlockNo) == 0 )
    {
        //
        // Read out the data.
        //
        while(ulSize--)
        {
            MAP_SDHostDataRead(SDHOST_BASE,(unsigned long *)pBuffer);
            pBuffer+=4;
        }

        //
        // Send multi block read stop command to the card
        //
        SendCmd(CMD_STOP_TRANS,0);

        //
        // return success.
        //
        return 0;
    }

    //
    // Return error
    //
    return 1;
}

//*****************************************************************************
//
//! Write a block of data.
//!
//! \param Card is pointer to a valid card attrib structure.
//! \param pBuffer is pointer to the buffer to be written
//! \param ulBlockNo is stating block number.
//! \param ulBlockCount is number of block to be read.
//!
//! This function write specified number of block into \e pBuffer. Each block
//! is of 512 byte.
//!
//! \return Returns 0 on success, 1 otherwise.
//
//*****************************************************************************
#if WRITE_TEST_EN
static unsigned long
CardWriteBlock(CardAttrib_t *Card, unsigned char *pBuffer,
              unsigned long ulBlockNo, unsigned long ulBlockCount)
{
    unsigned long ulSize;

    //
    // SDSC linear address instead of block address
    //
    if(Card->ulCapClass == CARD_CAP_CLASS_SDSC)
    {
        ulBlockNo = ulBlockNo * 512;
    }

    //
    // Set the number of block on the host
    //
    MAP_SDHostBlockCountSet(SDHOST_BASE,ulBlockCount);

    //
    // Compute total size in words.
    //
    ulSize = (512 * ulBlockCount)/4;

    //
    // Send multi block read command to the card
    //
    if( SendCmd(CMD_WRITE_MULTI_BLK, ulBlockNo) == 0 )
    {
        //
        // Write the data
        //
        while(ulSize--)
        {
            MAP_SDHostDataWrite(SDHOST_BASE,*(unsigned long *)pBuffer);
            pBuffer += 4;
        }

        //
        // Wait for transfer completion.
        //
        while( !(MAP_SDHostIntStatus(SDHOST_BASE) & SDHOST_INT_TC) )
        {

        }

        //
        // Send multi block write stop command to the card
        //
        SendCmd(CMD_STOP_TRANS,0);

        //
        // return
        //
        return 0;
    }

    //
    // Return error
    //
    return 1;
}
#endif


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


//****************************************************************************
//
//! Main function
//!
//! \param none
//!
//! \return None.
//
//****************************************************************************
int main()
{
    CardAttrib_t sCard;
    unsigned long ulCapacity;
    unsigned long ulAddress;

    //
    // Initialize Board configurations
    //
    BoardInit();

    //
    // Muxing for Enabling UART_TX and UART_RX.
    //
    PinMuxConfig();

    //
    // Initializing the Terminal.
    //
    InitTerm();

    //
    // Clearing the Terminal.
    //
    ClearTerm();

    //
    // Display the Banner
    //
    Message("\n\n\n\r");
    Message("\t\t   *************************************\n\r");
    Message("\t\t        CC3200 SDHost Demo Application  \n\r");
    Message("\t\t   *************************************\n\r");
    Message("\n\n\n\r");

    //
    // Set the SD card clock as output pin
    //
    MAP_PinDirModeSet(PIN_07,PIN_DIR_MODE_OUT);

    //
    // Enable Pull up on data
    //
    MAP_PinConfigSet(PIN_06,PIN_STRENGTH_4MA, PIN_TYPE_STD_PU);

    //
    // Enable Pull up on CMD
    //
    MAP_PinConfigSet(PIN_08,PIN_STRENGTH_4MA, PIN_TYPE_STD_PU);

    //
    // Enable SDHOST
    //
    MAP_PRCMPeripheralClkEnable(PRCM_SDHOST,PRCM_RUN_MODE_CLK);

    //
    // Reset SDHOST
    //
    MAP_PRCMPeripheralReset(PRCM_SDHOST);

    //
    // Configure SDHOST
    //
    MAP_SDHostInit(SDHOST_BASE);

    //
    // Configure card clock to 15 Mhz
    //
    MAP_SDHostSetExpClk(SDHOST_BASE, MAP_PRCMPeripheralClockGet(PRCM_SDHOST),
                            15000000);

    //
    // Initialize the card
    //
    if( CardInit(&sCard) )
    {
        Message("Error : Failed to initialize the card. Check "
                    "if card is properly inserted \n\r");
    }
    else
    {
        //
        // Print Card Details
        //
        ulCapacity = (sCard.ullCapacity/(1024*1024));
        Message("********************\n\r");
        Message("Card Info \n\r");
        Message("********************\n\r");
        Report("Card Type   : %s \n\r",
                    (sCard.ulCardType == CARD_TYPE_MMC)?"MMC":"SD Card");
        Report("Class       : %s \n\r",
                    (sCard.ulCapClass == CARD_CAP_CLASS_SDHC)?"SDHC":"SDSC");
        Report("Capacity    : %d MB \n\r",ulCapacity);
        Report("Version     : Ver %s \n\r",
                        (sCard.ulVersion  == CARD_VERSION_2)?"2.0":"1.0");

        //
        // Select the card
        //
        CardSelect(&sCard);

        //
        // Set the block size on the controller.
        //
        MAP_SDHostBlockSizeSet(SDHOST_BASE,512);

        Message("\n\rReading first Block.....\n\r");

        //
        // Read 1 Block (512 Bytes) from the card
        //
        CardReadBlock(&sCard,g_ucDataBuff,0,1);

        //
        // Display the content
        //
        for(ulAddress=0; ulAddress < 512; ulAddress++)
        {
            if(ulAddress%20 == 0)
            {
                Report("\n\r%0.4x :", ulAddress);
            }
            Report("%2.2x ",g_ucDataBuff[ulAddress]);
        }

#if WRITE_TEST_EN

        Message("\n\n\rWriting first Block.....\n\n\r");
        Message("Info : This will corrupt the existing filesystem on the card\n\r");

        //
        // Initialize the buffer with pattern
        //
        for(ulAddress = 0; ulAddress < 512; ulAddress++)
        {
          g_ucDataBuff[ulAddress] = ulAddress;
        }

        //
        // Write 1 block (512 bytes) to the card
        //
        CardWriteBlock(&sCard,g_ucDataBuff,0,1);

#endif

        //
        // de-select the card
        //
        CardDeselect();
    }

    //
    // Infinite loop;
    //
    while(1)
    {

    }
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
