//*****************************************************************************
// diskio.c
//
// Low level SD Card access hookup for FatFS
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

#include "hw_types.h"
#include "hw_memmap.h"
#include "hw_ints.h"
#include "rom.h"
#include "rom_map.h"
#include "diskio.h"
#include "sdhost.h"
#include "stdcmd.h"
#include "utils.h"

//*****************************************************************************
// Macros
//*****************************************************************************
#define  DISKIO_RETRY_TIMEOUT 0xFFFFFFFF
#define  DISKIO_SECTOR_SIZE   512;

#define CARD_TYPE_UNKNOWN    0
#define CARD_TYPE_MMC        1
#define CARD_TYPE_SDCARD     2

#define CARD_CAP_CLASS_SDSC  0
#define CARD_CAP_CLASS_SDHC  1

#define CARD_VERSION_1       0
#define CARD_VERSION_2       1

//*****************************************************************************
// Disk Info Structure definition
//*****************************************************************************
typedef struct
{
  unsigned char ucCardType;
  unsigned long  ulVersion;
  unsigned long  ulCapClass;
  unsigned long ulNofBlock;
  unsigned long ulBlockSize;
  DSTATUS bStatus;
  unsigned short usRCA;
}DiskInfo_t;

//*****************************************************************************
// Disk Info for attached disk
//*****************************************************************************
static DiskInfo_t g_sDisk=
{
  CARD_TYPE_UNKNOWN,
  CARD_VERSION_1,
  CARD_CAP_CLASS_SDSC,
  0,0,
  STA_NOINIT,
  0
};

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
CardSendCmd(unsigned long ulCmd, unsigned long ulArg)
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
//! \return Returns 0 on success, 1 otherwise.
//
//*****************************************************************************
static unsigned long
CardCapacityGet(DiskInfo_t *psDiskInfo)
{
  unsigned long ulRet;
  unsigned long ulResp[4];
  unsigned long ulBlockSize;
  unsigned long ulBlockCount;
  unsigned long ulCSizeMult;
  unsigned long ulCSize;

  //
  // Read the CSD register
  //
  ulRet = CardSendCmd(CMD_SEND_CSD,(psDiskInfo->usRCA << 16 ));

  if(ulRet == 0)
  {
    //
    // Read the response
    //
    MAP_SDHostRespGet(SDHOST_BASE,ulResp);

    //
    // 136 bit CSD register is read into an array of 4 words.
    // ulResp[0] = CSD[31:0]
    // ulResp[1] = CSD[63:32]
    // ulResp[2] = CSD[95:64]
    // ulResp[3] = CSD[127:96]
    //
    if(ulResp[3] >> 30)
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
    psDiskInfo->ulBlockSize = ulBlockSize;
    psDiskInfo->ulNofBlock  = ulBlockCount;
  }

  //
  // return
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
CardSelect(DiskInfo_t *sDiskInfo)
{
  unsigned long ulRCA;
  unsigned long ulRet;

  ulRCA = sDiskInfo->usRCA;

  //
  // Send select command with card's RCA.
  //
  ulRet = CardSendCmd(CMD_SELECT_CARD, (ulRCA << 16));

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
//! Initializes physical drive
//!
//! This function initializes a physical drive. given by \e bDrive.
//!
//! \return Returns 0 on succeeded.
//*****************************************************************************
DSTATUS disk_initialize ( BYTE bDrive )
{
  unsigned long ulRet;
  unsigned long ulResp[4];

  //
  // Check the drive No.
  // Only 1 drive is supported
  //
  if (bDrive == 0)
  {
    if( g_sDisk.bStatus == 0)
    {
      return g_sDisk.bStatus;
    }

    //
    // Send std GO IDLE command
    //
    if( CardSendCmd(CMD_GO_IDLE_STATE, 0) == 0)
    {

      //
      // Get interface operating condition for the card
      //
      ulRet = CardSendCmd(CMD_SEND_IF_COND,0x000001A5);
      MAP_SDHostRespGet(SDHOST_BASE,ulResp);

      //
      // It's a SD ver 2.0 or higher card
      //
      if(ulRet == 0 && ((ulResp[0] & 0xFF) == 0xA5) )
      {
        //
        // Version 1 card do not respond to this
        // command
        //
        g_sDisk.ulVersion = CARD_VERSION_2;
        g_sDisk.ucCardType = CARD_TYPE_SDCARD;

        //
        // Wait for card to become ready.
        //
        do
        {
            //
            // Send ACMD41
            //
            CardSendCmd(CMD_APP_CMD,0);
            ulRet = CardSendCmd(CMD_SD_SEND_OP_COND,0x40E00000);

            //
            // Response contains 32-bit OCR register
            //
            MAP_SDHostRespGet(SDHOST_BASE,ulResp);

        }while(((ulResp[0] >> 31) == 0));

        if(ulResp[0] & (1UL<<30))
        {
          g_sDisk.ulCapClass = CARD_CAP_CLASS_SDHC;
        }

        g_sDisk.bStatus = 0;
      }
      else //It's a MMC or SD 1.x card
      {
        //
        // Wait for card to become ready.
        //
        do
        {
            CardSendCmd(CMD_APP_CMD,0);
            ulRet = CardSendCmd(CMD_SD_SEND_OP_COND,0x00E00000);
            if(ulRet == 0)
            {
              //
              // Response contains 32-bit OCR register
              //
              MAP_SDHostRespGet(SDHOST_BASE,ulResp);
            }
        }while(((ulRet == 0) && (ulResp[0] >> 31) == 0));

        if(ulRet == 0)
        {
          g_sDisk.ucCardType = CARD_TYPE_SDCARD;
          g_sDisk.bStatus = 0;
        }
        else
        {
          if( CardSendCmd(CMD_SEND_OP_COND,0) == 0)
          {
            //
            // MMC not supported by the controller
            //
            g_sDisk.ucCardType = CARD_TYPE_MMC;
          }
        }
      }
    }
  }

  //
  // Get the RCA of the attached card
  //
  if(g_sDisk.bStatus == 0)
  {

    ulRet = CardSendCmd(CMD_ALL_SEND_CID,0);

    if( ulRet == 0)
    {
      CardSendCmd(CMD_SEND_REL_ADDR,0);
      MAP_SDHostRespGet(SDHOST_BASE,ulResp);

      //
      //  Fill in the RCA
      //
      g_sDisk.usRCA = (ulResp[0] >> 16);

      //
      // Get tha card capacity
      //
      CardCapacityGet(&g_sDisk);
    }

    //
    // Select the card.
    //
    ulRet = CardSelect(&g_sDisk);
    if(ulRet == 0)
    {
      g_sDisk.bStatus = 0;
    }
  }

  //
  // Set card rd/wr block len
  //
  MAP_SDHostBlockSizeSet(SDHOST_BASE,512);

  return g_sDisk.bStatus;
}

//*****************************************************************************
//
//! Gets the disk status.
//!
//! This function gets the current status of the drive.
//!
//! \return Returns the current status of the specified drive
//
//*****************************************************************************
DSTATUS disk_status ( BYTE bDrive )
{
  if(bDrive == 0)
  {
    return g_sDisk.bStatus;
  }
  else
  {
    return STA_NOINIT;
  }
}

//*****************************************************************************
//
//! Reads sector(s) from the disk drive.
//!
//!
//! This function reads specified number of sectors from the drive
//!
//! \return Returns RES_OK on success.
//
//*****************************************************************************
DRESULT disk_read ( BYTE bDrive, BYTE* pBuffer, DWORD ulSectorNumber,
                   UINT bSectorCount )
{
  DRESULT Res;
  unsigned long ulSize;

  Res = RES_ERROR;

  //
  // Return if disk not initialized
  //
  if((g_sDisk.bStatus & STA_NOINIT))
  {
    return RES_PARERR;
  }

  //
  // SDSC uses linear address, SDHC uses block address
  //
  if(g_sDisk.ulCapClass == CARD_CAP_CLASS_SDSC)
  {
    ulSectorNumber = ulSectorNumber * DISKIO_SECTOR_SIZE;
  }

  //
  // Set the block count
  //
  MAP_SDHostBlockCountSet(SDHOST_BASE,bSectorCount);

  //
  // Compute the number of words
  //
  ulSize = (512*bSectorCount)/4;

  //
  // Check if 1 block or multi block transfer
  //
  if (bSectorCount == 1)
  {
    //
    // Send single block read command
    //
    if( CardSendCmd(CMD_READ_SINGLE_BLK, ulSectorNumber) == 0 )
    {
      //
      // Read the block of data
      //
      while(ulSize--)
      {
        MAP_SDHostDataRead(SDHOST_BASE,(unsigned long *)pBuffer);
        pBuffer+=4;
      }
      Res = RES_OK;
    }
  }
  else
  {
    //
    // Send multi block read command
    //
    if( CardSendCmd(CMD_READ_MULTI_BLK, ulSectorNumber) == 0 )
    {
      //
      // Read the data
      //
      while(ulSize--)
      {
        MAP_SDHostDataRead(SDHOST_BASE,(unsigned long *)pBuffer);
        pBuffer+=4;
      }
      CardSendCmd(CMD_STOP_TRANS,0);

      //
      // Wait for command to complete
      //
      while( !(MAP_SDHostIntStatus(SDHOST_BASE) & SDHOST_INT_TC) )
      {

      }

      Res = RES_OK;
    }
  }

  //
  // return status
  //
  return Res;
}

//*****************************************************************************
//
//! Wrties sector(s) to the disk drive.
//!
//!
//! This function writes specified number of sectors to the drive
//!
//! \return Returns RES_OK on success.
//
//*****************************************************************************
DRESULT disk_write ( BYTE bDrive,const BYTE* pBuffer, DWORD ulSectorNumber,
                    UINT bSectorCount)
{
  DRESULT Res;
  unsigned long ulSize;

  Res = RES_ERROR;

  if (bDrive || !bSectorCount)
  {
    return RES_PARERR;
  }

  //
  // Return if disk not initialized
  //
  if((g_sDisk.bStatus & STA_NOINIT))
  {
    return RES_NOTRDY;
  }

  //
  // SDSC uses linear address, SDHC uses block address
  //
  if(g_sDisk.ulCapClass == CARD_CAP_CLASS_SDSC)
  {
    ulSectorNumber = ulSectorNumber * DISKIO_SECTOR_SIZE;
  }

  //
  // Set the block count
  //
  MAP_SDHostBlockCountSet(SDHOST_BASE,bSectorCount);

  //
  // Compute the number of words
  //
  ulSize = (512*bSectorCount)/4;

  //
  // Check if 1 block or multi block transfer
  //
  if (bSectorCount == 1)
  {
    //
    // Send single block write command
    //
    if( CardSendCmd(CMD_WRITE_SINGLE_BLK, ulSectorNumber) == 0 )
    {
      //
      // Write the data
      //
      while(ulSize--)
      {
        MAP_SDHostDataWrite(SDHOST_BASE,(*(unsigned long *)pBuffer));
        pBuffer+=4;
      }

      //
      // Wait for data transfer complete
      //
      while( !(MAP_SDHostIntStatus(SDHOST_BASE) & SDHOST_INT_TC) )
      {

      }
      Res = RES_OK;
    }
  }
  else
  {

    //
    // Set the card write block count
    //
    if(g_sDisk.ucCardType == CARD_TYPE_SDCARD)
    {
      CardSendCmd(CMD_APP_CMD,g_sDisk.usRCA << 16);
      CardSendCmd(CMD_SET_BLK_CNT, bSectorCount);
    }

    //
    // Send single block write command
    //
    if( CardSendCmd(CMD_WRITE_MULTI_BLK, ulSectorNumber) == 0 )
    {
      //
      // Write the data buffer
      //
      while(ulSize--)
      {
        MAP_SDHostDataWrite(SDHOST_BASE,(*(unsigned long *)pBuffer));
        pBuffer+=4;
      }

      //
      // Wait for transfer complete
      //
      while( !(MAP_SDHostIntStatus(SDHOST_BASE) & SDHOST_INT_TC) )
      {

      }
      CardSendCmd(CMD_STOP_TRANS,0);

      //
      // Wait for command to complete
      //
      while( !(MAP_SDHostIntStatus(SDHOST_BASE) & SDHOST_INT_TC) )
      {

      }

      Res = RES_OK;
    }
  }

  //
  // return status
  //
  return Res;
}


//*****************************************************************************
//
//! Miscellaneous function
//!
//! This function controls device specified features nd miscellaneous functions
//! other than disk read/write.
//!
//! \returns RES_OK on success.
//
//*****************************************************************************
DRESULT disk_ioctl (BYTE bDrive,BYTE bCommand,void* Buffer )
{
  if( bDrive != 0)
  {
    return RES_PARERR;
  }

  switch(bCommand)
  {
    case GET_SECTOR_COUNT:
         *(WORD*)Buffer = g_sDisk.ulNofBlock;
         break;

    case GET_SECTOR_SIZE :
         *(WORD*)Buffer = 512;
         break;

    case CTRL_SYNC:
         break;

  default:
    while(1);
  }

  return RES_OK;
}

//*****************************************************************************
//
//! The get_fattime function gets current time.
//
//*****************************************************************************
DWORD get_fattime()
{
  return 0;
}
