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
// Application Name     - Hibernate
// Application Overview - The objective of this application is to showcase the
//                        hibernate as a power saving tool in a networking
//                        (this case a UDP client). In this example the device
//                        wakes up periodically from hibernate and broadcasts a
//                        message and then enters hibernate again.
//
//*****************************************************************************


//****************************************************************************
//
//! \addtogroup hib
//! @{
//
//****************************************************************************

#include <stdio.h>
#include <string.h>

// simplelink includes
#include "device.h"

// driverlib includes
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_hib1p2.h"
#include "timer.h"
#include "interrupt.h"
#include "prcm.h"
#include "rom.h"
#include "rom_map.h"
#include "utils.h"

// common interface includes
#include "network_if.h"
#include "gpio_if.h"
#include "common.h"
#ifndef NOTERM
#include "uart_if.h"
#endif
#include "utils_if.h."
#include "timer_if.h"
#include "pinmux.h"


#define APPLICATION_VERSION     "1.4.0"
#define APPNAME                 "Hibernate"
#define APP_UDP_PORT            5001
#define SYS_CLK                 80000000
#define MAX_BUF                 512
#define OSI_STACK_SIZE          3000


//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
SlSecParams_t SecurityParams = {0}; // AP Security Parameters
unsigned short g_usTimerInts = 0;   // Variable used in Timer Interrupt Handler

#if defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif
//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************


/****************************************************************************/
/*                      LOCAL FUNCTION PROTOTYPES                           */
/****************************************************************************/
void HIBUDPBroadcastTask(void *pvParameters);
void EnterHIBernate();


//*****************************************************************************
//
//! Periodic Timer Interrupt Handler
//!
//! \param None
//!
//! \return None
//
//*****************************************************************************
void
TimerPeriodicIntHandler(void)
{
    unsigned long ulInts;
    //
    // Clear all pending interrupts from the timer we are
    // currently using.
    //
    ulInts = MAP_TimerIntStatus(TIMERA0_BASE, true);
    MAP_TimerIntClear(TIMERA0_BASE, ulInts);
    //
    // Increment our interrupt counter.
    //
    g_usTimerInts++;
    if(!(g_usTimerInts & 0x1))
    {
        //
        // Off Led
        //
        GPIO_IF_LedOff(MCU_RED_LED_GPIO);
    }
    else
    {
        //
        // On Led
        //
        GPIO_IF_LedOn(MCU_RED_LED_GPIO);
    }
}

//****************************************************************************
//
//! Function to configure and start timer to blink the LED while device is
//! trying to connect to an AP
//!
//! \param none
//!
//! return none
//
//****************************************************************************
void LedTimerConfigNStart()
{
    //
    // Configure Timer for blinking the LED for IP acquisition
    //
    Timer_IF_Init(PRCM_TIMERA0,TIMERA0_BASE,TIMER_CFG_PERIODIC,TIMER_A,0);
    Timer_IF_IntSetup(TIMERA0_BASE,TIMER_A,TimerPeriodicIntHandler);
    Timer_IF_Start(TIMERA0_BASE,TIMER_A,100); // time value is in mSec
}

//****************************************************************************
//
//! Disable the LED blinking Timer as Device is connected to AP
//!
//! \param none
//!
//! return none
//
//****************************************************************************
void LedTimerDeinitStop()
{
    //
    // Disable the LED blinking Timer as Device is connected to AP
    //
    Timer_IF_Stop(TIMERA0_BASE,TIMER_A);
    Timer_IF_DeInit(TIMERA0_BASE,TIMER_A);

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
static void
DisplayBanner(char * AppName)
{

    DBG_PRINT("\n\n\n\r");
    DBG_PRINT("\t\t *************************************************\n\r");
    DBG_PRINT("\t\t       CC3200 %s Application       \n\r", AppName);
    DBG_PRINT("\t\t *************************************************\n\r");
    DBG_PRINT("\n\n\n\r");
}

//****************************************************************************
//
//! Task function implementing the UDP client and showcasing the hibernate 
//! functionality
//!
//! \param none
//! 
//! This function  
//!    1. Creates a UDP socket
//!    2. Broadcasts a packet on the socket
//!    3. Closes the socket
//!    4. Enters the HIBernate mode
//!
//! \return None.
//
//****************************************************************************
void HIBUDPBroadcastTask(void *pvParameters)
{
    int iSocketDesc;
    long lRetVal;
    struct sockaddr_in sBroadcastAddr;
    char pcBroadcastMessage[]="32xx HIB example application";

    //
    // Check the wakeup source. If first itme entry or wakeup from HIB
    //
    if(MAP_PRCMSysResetCauseGet() == 0)
    {
        DisplayBanner(APPNAME);
        DBG_PRINT("HIB: Wake up on Power ON\n\r");
    }
    else if(MAP_PRCMSysResetCauseGet() == PRCM_HIB_EXIT)
    {
        DBG_PRINT("HIB: Woken up from Hibernate\n\r");
    }
    else
    {

    }
    GPIO_IF_LedConfigure(LED1);

    GPIO_IF_LedOff(MCU_RED_LED_GPIO);


    //
    // Configure Timer for blinking the LED for IP acquisition
    //
    LedTimerConfigNStart();

    //
    //
    // Reset The state of the machine
    //
    Network_IF_ResetMCUStateMachine();

    //
    // Start the driver
    //
    lRetVal = Network_IF_InitDriver(ROLE_STA);
    if(lRetVal < 0)
    {
       UART_PRINT("Failed to start SimpleLink Device\n\r");
       LOOP_FOREVER();
    }

    // Initialize AP security params
    SecurityParams.Key = (signed char*)SECURITY_KEY;
    SecurityParams.KeyLen = strlen(SECURITY_KEY);
    SecurityParams.Type = SECURITY_TYPE;

    //
    // Connect to the Access Point
    //
    lRetVal = Network_IF_ConnectAP(SSID_NAME, SecurityParams);
    if(lRetVal < 0)
    {
       UART_PRINT("Connection to AP failed\n\r",lRetVal);
       LOOP_FOREVER();
    }

    //
    // Disable the LED blinking Timer as Device is connected to AP
    //
    LedTimerDeinitStop();

    //
    // Switch ON RED LED to indicate that Device acquired an IP
    //
    GPIO_IF_LedOn(MCU_IP_ALLOC_IND);

    //
    // Create UDP socket
    //
    iSocketDesc = sl_Socket(AF_INET, SOCK_DGRAM, 0);
    if(iSocketDesc < 0)
    {
        DBG_PRINT("HIB: Socket create failed\n\r");
        goto end;
    }
    DBG_PRINT("HIB: Socket created\n\r");

    //
    // Assign socket structure values for a braodcast message
    //
    sBroadcastAddr.sin_family = AF_INET;
    sBroadcastAddr.sin_addr.s_addr= sl_Htonl(0xFFFFFFFF);
    sBroadcastAddr.sin_port= sl_Htons(APP_UDP_PORT);

    //
    // Broadcast message
    //
    lRetVal = sendto(iSocketDesc, (char *)&pcBroadcastMessage[0],
                        sizeof(pcBroadcastMessage), 0,
                     (struct sockaddr *)&sBroadcastAddr,sizeof(sBroadcastAddr));
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }

    UNUSED(lRetVal);
    DBG_PRINT("HIB: sent message\n\r");

    //
    // Close the socket
    //
    close(iSocketDesc);
    DBG_PRINT("HIB: Socket closed\n\r");

    //
    // Stop the driver
    //
    lRetVal = Network_IF_DeInitDriver();
    if(lRetVal < 0)
    {
       UART_PRINT("Failed to stop SimpleLink Device\n\r");
       LOOP_FOREVER();
    }

    //
    // Switch Off RED & Green LEDs to indicate that Device is
    // disconnected from AP and Simplelink is shutdown
    //
    GPIO_IF_LedOff(MCU_IP_ALLOC_IND);
    GPIO_IF_LedOff(MCU_GREEN_LED_GPIO);

    //
    // Enter HIB here configuring the wakeup-timer
    //
    EnterHIBernate();
end:
    DBG_PRINT("HIB: Test Complete\n\r");

    //
    // Loop here
    //
    LOOP_FOREVER();
}

//****************************************************************************
//
//! Enter the HIBernate mode configuring the wakeup timer
//!
//! \param none
//! 
//! This function  
//!    1. Sets up the wakeup RTC timer
//!    2. Enables the RTC
//!    3. Enters into HIBernate 
//!
//! \return None.
//
//****************************************************************************
void EnterHIBernate()
{
#define SLOW_CLK_FREQ           (32*1024)
    //
    // Configure the HIB module RTC wake time
    //
    MAP_PRCMHibernateIntervalSet(5 * SLOW_CLK_FREQ);

    //
    // Enable the HIB RTC
    //
    MAP_PRCMHibernateWakeupSourceEnable(PRCM_HIB_SLOW_CLK_CTR);

    DBG_PRINT("HIB: Entering HIBernate...\n\r");
    MAP_UtilsDelay(80000);
    
    //
    // powering down SPI Flash to save power 
    //
    Utils_SpiFlashDeepPowerDown();
    //
    // Enter HIBernate mode
    //
    MAP_PRCMHibernateEnter();
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
// In case of TI-RTOS vector table is initialize by OS itself
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
//! This function  
//!    1. Invokes the SLHost task
//!    2. Invokes the HIBUDPBroadcastTask
//!
//! \return None.
//
//****************************************************************************
void main()
{
    long lRetVal = -1;

    //
    // Initialize board confifurations
    //
    BoardInit();

    //
    // Pinmux for UART & LED
    //
    PinMuxConfig();
#ifndef NOTERM
    //
    // Configuring UART
    //
    InitTerm();
#endif
    //
    // Start the SimpleLink Host
    //
    lRetVal = VStartSimpleLinkSpawnTask(SPAWN_TASK_PRIORITY);
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }

    //
    // Start the HIBUDPBroadcast task
    //
    lRetVal = osi_TaskCreate(HIBUDPBroadcastTask, (const signed char *)"HIB UDP Broadcast",
                OSI_STACK_SIZE, NULL, 1, NULL );
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
