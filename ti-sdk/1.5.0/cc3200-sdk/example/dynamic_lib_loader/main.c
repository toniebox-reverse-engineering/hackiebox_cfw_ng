//*****************************************************************************
//
//  Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
//
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions
//  are met:
//
//  Redistributions of source code must retain the above copyright
//  notice, this list of conditions and the following disclaimer.
//
//  Redistributions in binary form must reproduce the above copyright
//  notice, this list of conditions and the following disclaimer in the
//  documentation and/or other materials provided with the
//  distribution.
//
//  Neither the name of Texas Instruments Incorporated nor the names of
//  its contributors may be used to endorse or promote products derived
//  from this software without specific prior written permission.
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
// Application Name     - Dynamic Library Loader
// Application Overview - This particular application illustrates how to load
//						  different libraries having same Interface APIs can be
//						  hooked at the run time
//
//*****************************************************************************

//****************************************************************************
//
//! \addtogroup loader
//! @{
//
//****************************************************************************

#include "hw_types.h"
#include "hw_ints.h"
#include "rom.h"
#include "rom_map.h"
#include "interrupt.h"
#include "prcm.h"
#include "utils.h"

#ifndef NOTERM
#include "uart_if.h"
#endif

#include "simplelink.h"
#include "osi.h"

#include "common.h"
#include "pinmux.h"
#include "app.h"

#define APPLICATION_VERSION     "1.4.0"
#define APPLICATION_NAME        "Dynamic Lib Loader"

#define LIBRARY_1           "/tmp/app1.bin"
#define LIBRARY_2           "/tmp/app2.bin"

const unsigned char *libraries [] = {LIBRARY_1, LIBRARY_2};

#define OSI_STACK_SIZE      2048

#define START_OF_DYNAMIC_LIB_SECTION        0x20030000  /* this address onward App1 / App2 will reside */
#define START_OF_APPLICATION_RAM            0x20004000  /* loader app starting RAM address */
#define END_OF_APPLICATION_RAM              0x2003FFFF  /* End of RAM for CC3200 device */
#define OFFSET_TO_DYNAMIC_LIB_SECTION       0 /* w.r.t the file being read */

#define TOTAL_SIZE_OF_DYNAMIC_LIB_SECTION   ((END_OF_APPLICATION_RAM - START_OF_DYNAMIC_LIB_SECTION))

// Wrapper function for printing the library name 
int w_Print(char *p_lib_name);

#ifndef USE_TIRTOS
#if defined(ccs) || defined(gcc)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif
#endif

/* Wrapper function pointer table */

//*****************************************************************************
// There is no way for the libraries (that were loaded dynamically) to know
// the address of functions (and other global variables) defined in ‘loader’.
// Hence, to allow these library functions to access ‘loader’s’ functions
// (and other global variables), ‘loader shall have all such functions
// wrapped, and the address of the wrapper function(s) shall be registered
// with the library
//
//*****************************************************************************
s_fptr *gp_ftable = NULL;
s_wptr wptr_table =
{
    w_Print
};

#ifdef USE_FREERTOS
//*****************************************************************************
//
//! \brief Application defined hook (or callback) function - assert
//!
//! \param[in]  pcFile - Pointer to the File Name
//! \param[in]  ulLine - Line Number
//!
//! \return none
//!
//*****************************************************************************
void vAssertCalled( const char *pcFile, unsigned long ulLine )
{
    /* Handle Assert here */
    LOOP_FOREVER();
}

//*****************************************************************************
//
//! \brief Application defined idle task hook
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
void vApplicationIdleHook( void)
{
    /* Handle Idle Hook for Profiling, Power Management etc */
    LOOP_FOREVER();
}

//*****************************************************************************
//
//! \brief Application defined malloc failed hook
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
void vApplicationMallocFailedHook()
{
    /* Handle Memory Allocation Errors */
    LOOP_FOREVER();
}

//*****************************************************************************
//
//! \brief Application defined stack overflow hook
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
void vApplicationStackOverflowHook(OsiTaskHandle *pxTask, signed char *pcTaskName)
{
    /* Handle FreeRTOS Stack Overflow */
    LOOP_FOREVER();
}
#endif /* USE_FREERTOS */

//*****************************************************************************
//
//! \brief The Function Handles WLAN Events
//!
//! \param[in]  pWlanEvent - Pointer to WLAN Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkWlanEventHandler(SlWlanEvent_t *pWlanEvent)
{
    /* Unused in this application */
}

//*****************************************************************************
//
//! \brief This function handles network events such as IP acquisition, IP
//!           leased, IP released etc.
//!
//! \param[in]  pNetAppEvent - Pointer to NetApp Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkNetAppEventHandler(SlNetAppEvent_t *pNetAppEvent)
{
    /* Unused in this application */
}

//*****************************************************************************
//
//! \brief This function handles HTTP server events
//!
//! \param[in]  pServerEvent - Contains the relevant event information
//! \param[in]    pServerResponse - Should be filled by the user with the
//!                                      relevant response information
//!
//! \return None
//!
//****************************************************************************
void SimpleLinkHttpServerCallback(SlHttpServerEvent_t *pHttpEvent,
                                  SlHttpServerResponse_t *pHttpResponse)
{
    /* Unused in this application */
}

//*****************************************************************************
//
//! \brief This function handles General Events
//!
//! \param[in]     pDevEvent - Pointer to General Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkGeneralEventHandler(SlDeviceEvent_t *pDevEvent)
{
    /* Unused in this application */
}

//*****************************************************************************
//
//! This function handles socket events indication
//!
//! \param[in]      pSock - Pointer to Socket Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkSockEventHandler(SlSockEvent_t *pSock)
{
    /* Unused in this application */
}

//*****************************************************************************
//
//! \brief  Wrapper function
//!
//! \param[in]      p_lib_name - Library name
//!
//! \return None
//
//*****************************************************************************
int w_Print(char *p_lib_name)
{
    UART_PRINT("%s\n\r", p_lib_name);
    return 0;
}

//*****************************************************************************
//
//! \brief  Function display the application banner
//!
//! \param[in]      p_app_name - Application name
//!
//! \return None
//
//*****************************************************************************
static void DisplayBanner(char *p_app_name)
{
    UART_PRINT("\n\n\n\r");
    UART_PRINT("\t\t *************************************************\n\r");
    UART_PRINT("\t\t\t Application Name: %s       \n\r", p_app_name);
    UART_PRINT("\t\t *************************************************\n\r");
    UART_PRINT("\n\n\n\r");
}

//*****************************************************************************
//
//! \brief  Board Initialization & Configuration
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
    /* Set vector table base */
    #if defined(ccs) || defined(gcc)
        MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
    #endif

    #if defined(ewarm)
        MAP_IntVTableBaseSet((unsigned long)&__vector_table);
    #endif
#endif /* USE_TIRTOS */

    /* Enable Processor */
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}

//*****************************************************************************
//
//! \brief    This funtion:
//!             - opens the user file for reading
//!             - reads the data into application's 'DYNAMIC LIB' section
//!             - closes the user file
//!
//!  \param[in] p_library_name : Library's name to be loaded
//!
//!  \return 0: Success, -ve: Failure
//
//*****************************************************************************
static signed long load_library(const unsigned char *p_library_name)
{
    unsigned char *p_dynamic_lib_section = NULL;
    signed long file_handle = -1;
    unsigned long token = 0;
    signed long ret_val = -1;

    p_dynamic_lib_section = (unsigned char *)START_OF_DYNAMIC_LIB_SECTION;

    // Erase contents @ START_OF_DYNAMIC_LIB_SECTION 
    memset(p_dynamic_lib_section, '\0', TOTAL_SIZE_OF_DYNAMIC_LIB_SECTION);

    // Open the library 
    ret_val = sl_FsOpen((unsigned char *)p_library_name, FS_MODE_OPEN_READ,\
                            &token, &file_handle);
    if(ret_val < 0)
    {
        sl_FsClose(file_handle, 0, 0, 0);
        ASSERT_ON_ERROR(ret_val);
    }

    // Load the contents @ START_OF_DYNAMIC_LIB_SECTION 
    ret_val = sl_FsRead(file_handle, OFFSET_TO_DYNAMIC_LIB_SECTION,\
                        p_dynamic_lib_section,  TOTAL_SIZE_OF_DYNAMIC_LIB_SECTION);
    if ((ret_val < 0))
    {
        sl_FsClose(file_handle, 0, 0, 0);
        ASSERT_ON_ERROR(ret_val);
    }

    /* The first few bytes in this section has the function-table */
    gp_ftable = (s_fptr *)p_dynamic_lib_section;
    if((NULL == gp_ftable->p_add) ||
        (NULL == gp_ftable->p_sub) ||
         (NULL == gp_ftable->p_init) ||
          (NULL == gp_ftable->p_display))
    {
        /* Not all required functions are defined by the library - Assert */
    	ASSERT_ON_ERROR(-1);
    }

    /* Call functions of the library that was loaded dynamically */

    /** Registering the wrapper-functions w/ the library
      * There is no way for the libraries (that were loaded dynamically) to know
      * the address of functions (and other global variables) defined in ‘loader’.
      * Hence, to allow these library functions to access ‘loader’s’ functions
      * (and other global variables), ‘loader shall have all such functions
      * wrapped, and the address of the wrapper function(s) shall be registered
      * with the library
      */
    gp_ftable->p_init(&wptr_table);

    sl_FsClose(file_handle, 0, 0, 0);
    return 0;
}

//*****************************************************************************
//
//! This task will load two different libraries one by one and call
//! common APIs having different implementation.
//!
//! \param pvParameters is the pointer to the the argument for the task
//!
//! \return none
//
//*****************************************************************************
void Loader(void *pvParameters)
{
    unsigned int uiNumOfLibs = 0;
    unsigned int uiCounter = 0;
    int first_num, second_num;
    
    first_num = 40;
    second_num = 10;
    // Start SimpleLink device  to access file System
    if(sl_Start(NULL, NULL, NULL) < 0)
    {
        LOOP_FOREVER();
    }

    uiNumOfLibs = sizeof(libraries)/sizeof(libraries[0]);
    while(uiCounter < uiNumOfLibs)
    {
        UART_PRINT("Loading library %u.. ",uiCounter+1);
        if(load_library(libraries[uiCounter]) < 0)
        {
            UART_PRINT("could not load the library\n\r");
            LOOP_FOREVER();
        }
        
        /* calling common API for both the libraries */
        gp_ftable->p_display((char *) libraries[uiCounter]);
        UART_PRINT("Library loaded successfully\n\r");
        
        UART_PRINT("Calling common APIs..\n\r");
        UART_PRINT("AddNums(%u, %u): %d \n\r", first_num, second_num, 
                   gp_ftable->p_add(first_num, second_num));
        UART_PRINT("SubNums(%u, %u): %d \n\r", first_num, second_num, 
                   gp_ftable->p_sub(first_num, second_num));
        UART_PRINT("\n\r");
        uiCounter++;
    }
    UART_PRINT("Application Tested Successfully\n\r");
    UART_PRINT("Exiting Application\n\r");
    LOOP_FOREVER();
}

//****************************************************************************
//
//! Main function
//!
//! \param none
//!
//! This function
//!    1. Invokes the Loader Task
//!
//! \return None.
//
//****************************************************************************
void main()
{
    BoardInit();
    PinMuxConfig();

    InitTerm();

    DisplayBanner(APPLICATION_NAME);

    // Start the SimpleLink Host 
    VStartSimpleLinkSpawnTask(SPAWN_TASK_PRIORITY);

    // Start the Loader task 
    osi_TaskCreate(Loader, (const signed char*)"Loader Task", \
                                OSI_STACK_SIZE, NULL, 1, NULL );

    // Start the task scheduler 
    osi_start();
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
