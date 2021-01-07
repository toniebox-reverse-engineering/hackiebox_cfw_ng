//*****************************************************************************
// Copyright (C) 2014 Texas Instruments Incorporated
//
// All rights reserved. Property of Texas Instruments Incorporated.
// Restricted rights to use, duplicate or disclose this code are
// granted through contract.
// The program may not be used without the written permission of
// Texas Instruments Incorporated or against the terms and conditions
// stipulated in the agreement under which this program has been supplied,
// and under no circumstances can it be used with non-TI connectivity device.
//
//*****************************************************************************

//*****************************************************************************
//
// Application Name     -   Serial Wi-Fi 
// Application Overview -   This application runs over the UART interface
//                          and behaves as a command line interpreter.
//                          This application demonstrates secure and robust end
//                          to end communcation leveraging the entire network
//                          stack and UART interface.
//
//*****************************************************************************


//*****************************************************************************
//
//! \addtogroup serial_wifi
//! @{
//
//*****************************************************************************

//Driverlib includes
#include "hw_ints.h"
#include "hw_types.h"
#include "interrupt.h"
#include "prcm.h"
#include "rom.h"
#include "rom_map.h"
#include "pinmux.h"

//Application Includes
#include "serial_wifi.h"

//Common interface includes
#include "network_if.h"
#include "common.h"

#define APPLICATION_NAME        "SERIAL WiFi"
#define APPLICATION_VERSION     "1.4.0"
#define OSI_STACK_SIZE         2048

//****************************************************************************
//                      FUNCTION PROTOTYPES
//****************************************************************************
extern void Interpreter_Task(void *pvParameters);
extern void basic_Interpreter(void *pvParameters);

//****************************************************************************
//                      LOCAL FUNCTION PROTOTYPES
//****************************************************************************
static void DisplayBanner(char * AppName);
static void BoardInit(void);

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
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
//! Application startup display on UART
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
static void DisplayBanner(char * AppName)
{

  UART_PRINT("\n\n\n\r");
  UART_PRINT("\t\t *************************************************\n\r");
  UART_PRINT("\t\t       CC3200 %s Application       \n\r", AppName);
  UART_PRINT("\t\t *************************************************\n\r");
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
static void BoardInit(void)
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
//
//! main
//!
//! @param  none
//!
//! @return none
//!
//! @brief  Main function
//
//*****************************************************************************
int main(void)
{
    long    lRetVal = -1;

    //board initializations
    BoardInit();

    //UART driver initialisations
    PinMuxConfig();
    InitTerm();


    ClearTerm();
    DisplayBanner(APPLICATION_NAME);

    //
    // Simplelinkspawntask
    //
    lRetVal = VStartSimpleLinkSpawnTask(9);
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }
    
    //
    //create an Interpreter task
    //
    lRetVal = osi_TaskCreate(Interpreter_Task, (signed char *)"Wi-Fi",\
                                OSI_STACK_SIZE, NULL,
                                1, NULL);
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }
    
    // create a separate thread for reading UART
    lRetVal = osi_TaskCreate(basic_Interpreter,(signed char *)"uart", \
                                OSI_STACK_SIZE, NULL,
                                1, NULL);
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }

    osi_start();

    while(1);

}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
