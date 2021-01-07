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
// Application Name     - SDHost FatFS
// Application Overview - This application uses the FatFS to provide the block
//                        level read/write access to SD card, using the SD Host
//                        controller on CC3200.
//
//*****************************************************************************

// driverlib includes
#include "hw_types.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "hw_ints.h"
#include "pin.h"
#include "rom.h"
#include "rom_map.h"
#include "prcm.h"
#include "sdhost.h"
#include "ff.h"
#include "utils.h"
#include "interrupt.h"

// common interface includes
#include "uart_if.h"
#include "pinmux.h"

#define APPLICATION_VERSION  "1.4.0"
#define USERFILE        "userfile.txt"
#define SYSFILE         "sysfile.txt"
#define SYSTEXT         "The quick brown fox jumps over the lazy dog"

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
unsigned char pBuffer[100];

#if defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif
//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************


//****************************************************************************
//
//! List the files in the given directory
//!
//! \param pointer to directory
//!
//! \return None.
//
//****************************************************************************
static void
ListDirectory(DIR *dir)
{
    FILINFO fno;
    FRESULT res;
    unsigned long ulSize;
    tBoolean bIsInKB;

    for(;;)
    {
        res = f_readdir(dir, &fno);           // Read a directory item
        if (res != FR_OK || fno.fname[0] == 0)
        {
        break;                                // Break on error or end of dir
        }
        ulSize = fno.fsize;
        bIsInKB = false;
        if(ulSize > 1024)
        {
            ulSize = ulSize/1024;
            bIsInKB = true;
        }
        Report("->%-15s%5d %-2s    %-5s\n\r",fno.fname,ulSize,\
                (bIsInKB == true)?"KB":"B",(fno.fattrib&AM_DIR)?"Dir":"File");
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
//!
//! \return None.
//
//****************************************************************************
void main()
{

    FIL fp;
    FATFS fs;
    FRESULT res;
    DIR dir;
    UINT Size;

    //
    // Initialize Board configurations
    //
    BoardInit();

    //
    // Muxing for Enabling UART_TX and UART_RX.
    //
    PinMuxConfig();

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
    // Initialising the Terminal.
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
    Message("\t\t   ********************************************\n\r");
    Message("\t\t        CC3200 SDHost Fatfs Demo Application  \n\r");
    Message("\t\t   ********************************************\n\r");
    Message("\n\n\n\r");

    //
    // Enable MMCHS
    //
    MAP_PRCMPeripheralClkEnable(PRCM_SDHOST,PRCM_RUN_MODE_CLK);

    //
    // Reset MMCHS
    //
    MAP_PRCMPeripheralReset(PRCM_SDHOST);

    //
    // Configure MMCHS
    //
    MAP_SDHostInit(SDHOST_BASE);

    //
    // Configure card clock
    //
    MAP_SDHostSetExpClk(SDHOST_BASE,
                            MAP_PRCMPeripheralClockGet(PRCM_SDHOST),15000000);

    f_mount(&fs,"0",1);
    res = f_opendir(&dir,"/");
    if( res == FR_OK)
    {
        Message("Opening root directory.................... [ok]\n\n\r");
        Message("/\n\r");
        ListDirectory(&dir);
    }
    else
    {
        Message("Opening root directory.................... [Failed]\n\n\r");
    }

    Message("\n\rReading user file...\n\r");
    res = f_open(&fp,USERFILE,FA_READ);
    if(res == FR_OK)
    {
        f_read(&fp,pBuffer,100,&Size);
        Report("Read : %d Bytes\n\n\r",Size);
        Report("%s",pBuffer);
        f_close(&fp);
    }
    else
    {
        Report("Failed to open %s\n\r",USERFILE);
    }

    Message("\n\n\rWriting system file...\n\r");
    res = f_open(&fp,SYSFILE,FA_CREATE_ALWAYS|FA_WRITE);
    if(res == FR_OK)
    {
        f_write(&fp,SYSTEXT,sizeof(SYSTEXT),&Size);
        Report("Wrote : %d Bytes",Size);
        res = f_close(&fp);
    }
    else
    {
        Message("Failed to create a new file\n\r");
    }

    while(1)
    {

    }
}
