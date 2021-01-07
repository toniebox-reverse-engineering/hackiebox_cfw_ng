//*****************************************************************************
// control.c
//
// Audio Control APIs
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
//! \addtogroup wifi_audio_app
//! @{
//
//****************************************************************************
// Standard include 
#include <stdio.h>

//driverlib include
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "interrupt.h"
#include "rom.h"
#include "rom_map.h"
#include "gpio.h"
#include "gpio_if.h"

//Free_rtos/ti-rtos includes
#include "osi.h"

// simplelink include
#include "simplelink.h"

//Common include
#include "common.h"


// Apps include 
#include "utils.h"
#include "network.h"
#include "control.h"

#define CC3200_MDNS_NAME  "CC3200._audio._udp.local"

//*****************************************************************************
//                      GLOBAL VARIABLES
//*****************************************************************************
P_AUDIO_HANDLER g_pAudioInControlHdl;
P_AUDIO_HANDLER g_pAudioOutControlHdl;

OsiTaskHandle g_AudioControlTask;
OsiMsgQ_t g_ControlMsgQueue;

typedef struct
{
  //Queue_Elem _elem;
  P_OSI_SPAWN_ENTRY pEntry;
  void* pValue;
}tTxMsg;

volatile unsigned char g_ucMicStartFlag = 0;
volatile unsigned char g_ucSpkrStartFlag = 0;
extern unsigned char g_loopback;

//*****************************************************************************
//
//! MicroPhone Control Routine
//!
//! \param  pValue - pointer to a memory structure that is passed 
//!         to the interrupt handler.
//!
//! \return None
//
//*****************************************************************************
void MicroPhoneControl(void* pValue)
{
    int iCount=0;
    unsigned long ulPin5Val = 1; 
    
    //Check whether GPIO Level is Stable As No Debouncing Circuit in LP
    for(iCount=0;iCount<3;iCount++)
    {
        osi_Sleep(200);
        ulPin5Val = MAP_GPIOPinRead(GPIOA1_BASE,GPIO_PIN_5);
        if(ulPin5Val)
        {
            //False Alarm
            return;
        }
    }
    if (g_ucMicStartFlag ==  0)
    {
        for(iCount = 0; iCount<3; iCount++)
        {
            //Blink LED 3 times to Indicate ON
            GPIO_IF_LedOff(MCU_GREEN_LED_GPIO);
            osi_Sleep(50);
            GPIO_IF_LedOn(MCU_GREEN_LED_GPIO);
            osi_Sleep(50);
        }
         g_ucMicStartFlag = 1;
#ifdef MULTICAST
         g_loopback = 0;
#endif
        
     }
     else
     {
        //Blink LED 3 times to Indicate OFF
        for(iCount = 0; iCount<3; iCount++)
        {
            GPIO_IF_LedOn(MCU_GREEN_LED_GPIO);
            osi_Sleep(50);
            GPIO_IF_LedOff(MCU_GREEN_LED_GPIO);
            osi_Sleep(50);
        }
        g_ucMicStartFlag = 0;
     }
    
     //Enable GPIO Interrupt 
     MAP_GPIOIntClear(GPIOA1_BASE,GPIO_PIN_5);
     MAP_IntPendClear(INT_GPIOA1);
     MAP_IntEnable(INT_GPIOA1);
     MAP_GPIOIntEnable(GPIOA1_BASE,GPIO_PIN_5);

}

//*****************************************************************************
//
//! Speaker Control Routine
//!
//! \param  pValue - pointer to a memory structure that is passed to 
//!         the interrupt handler.
//!
//! \return None
//
//*****************************************************************************

void SpeakerControl(void* pValue)
{
    int iCount=0;
    unsigned long ulPin6Val = 1;
    long lRetVal = -1;
    
    //Check whether GPIO Level is Stable As No Debouncing Circuit in LP
    for(iCount=0;iCount<3;iCount++)
    {
        osi_Sleep(200);
        ulPin6Val = MAP_GPIOPinRead(GPIOA2_BASE,GPIO_PIN_6);
        if(ulPin6Val)
        {
            //False Alarm
            return;
        }
    }
    
    if (g_ucSpkrStartFlag ==  0)
    {
#ifndef MULTICAST   
        //Un Register mDNS Service.
    lRetVal = sl_NetAppMDNSUnRegisterService((signed char *)CC3200_MDNS_NAME,\
                          (unsigned char)strlen((const char *)CC3200_MDNS_NAME));
    if(lRetVal < 0)
    {
      UART_PRINT("Unable to unregister MDNS service\n\r");
    }
    //Registering for the mDNS service.              
    lRetVal = sl_NetAppMDNSRegisterService((signed char *)CC3200_MDNS_NAME, \
                          (unsigned char)strlen((const char *)CC3200_MDNS_NAME),\
                          (signed char *)"multicast",\
                          (unsigned char)strlen((const char *)"multicast"),\
                           AUDIO_PORT,1000,0);    
    if(lRetVal < 0)
    {
      UART_PRINT("Unable to register MDNS service\n\r");
      LOOP_FOREVER();
    }
#endif 
        
        //Blink LED 3 times to Indicate ON
        for(iCount = 0; iCount<3; iCount++)
        {
            GPIO_IF_LedOff(MCU_ORANGE_LED_GPIO);
            osi_Sleep(50);
            GPIO_IF_LedOn(MCU_ORANGE_LED_GPIO);
            osi_Sleep(50);
        }                     
        g_ucSpkrStartFlag = 1;
        g_loopback = 0;
    }
    else
    {
        //Un Register mDNS Service.
    lRetVal = sl_NetAppMDNSUnRegisterService((signed char *)CC3200_MDNS_NAME,\
                           (unsigned char)strlen((const char *)CC3200_MDNS_NAME));        
    if(lRetVal < 0)
    {
      UART_PRINT("Unable to unregister MDNS service\n\r");
    }
        //Blink LED 3 times to Indicate OFF
        for(iCount = 0; iCount<3; iCount++)
        {
            GPIO_IF_LedOn(MCU_ORANGE_LED_GPIO);
            osi_Sleep(50);
            GPIO_IF_LedOff(MCU_ORANGE_LED_GPIO);
            osi_Sleep(50);
        }
        g_ucSpkrStartFlag = 0;            
    }
    
    //Enable GPIO Interrupt
    MAP_GPIOIntClear(GPIOA2_BASE,GPIO_PIN_6);
    MAP_IntPendClear(INT_GPIOA2);
    MAP_IntEnable(INT_GPIOA2);
    MAP_GPIOIntEnable(GPIOA2_BASE,GPIO_PIN_6);
}

//*****************************************************************************
//
//! Control for Wifi Audio Player Microphone
//!
//! \param None
//!
//! \return None
//
//*****************************************************************************
static void MICStartStopControl()
{    
  long lRetVal = -1;
  tTxMsg sMsg;
  sMsg.pEntry = (P_OSI_SPAWN_ENTRY)&MicroPhoneControl;
  sMsg.pValue = NULL;
  lRetVal = osi_MsgQWrite(&g_ControlMsgQueue,&sMsg,OSI_NO_WAIT);      
  if(lRetVal < 0)
  {
    UART_PRINT("Unable to write to message queue\n\r");
    LOOP_FOREVER();
  }
}             

//*****************************************************************************
//
//! Control for Wifi Audio Player Speaker
//!
//! \param None
//!
//! \return None
//
//*****************************************************************************
static void SpeakerStartStopControl()
{  
  long lRetVal = -1;
  tTxMsg sMsg;
  sMsg.pEntry = (P_OSI_SPAWN_ENTRY)&SpeakerControl;
  sMsg.pValue = NULL;
  lRetVal = osi_MsgQWrite(&g_ControlMsgQueue,&sMsg,OSI_NO_WAIT);  
  if(lRetVal < 0)
  {
    UART_PRINT("Unable to write to message queue\n\r");
    LOOP_FOREVER();
  }
}
       

//*****************************************************************************
//
//! GPIO Interrupt Handler for S3 button
//!
//! \param None
//!
//! \return None
//
//*****************************************************************************
static void MICButtonHandler()
{
    unsigned long ulPinState =  GPIOIntStatus(GPIOA1_BASE,1);
    if(ulPinState & GPIO_PIN_5)
    {  
        //Clear and Disable GPIO Interrupt
        MAP_GPIOIntDisable(GPIOA1_BASE,GPIO_PIN_5);
        MAP_GPIOIntClear(GPIOA1_BASE,GPIO_PIN_5);        
        MAP_IntDisable(INT_GPIOA1);
        
        //Call Speaker Handler
        if(g_pAudioInControlHdl)
        {
            g_pAudioInControlHdl();
        }
    }
              
}
//*****************************************************************************
//
//! GPIO Interrupt Handler for S2 button
//!
//! \param None
//!
//! \return None
//
//*****************************************************************************
static void SpeakerButtonHandler()
{
    unsigned long ulPinState =  GPIOIntStatus(GPIOA2_BASE,1);
    if(ulPinState & GPIO_PIN_6)
    {
        //Clear and Disable GPIO Interrupt
        MAP_GPIOIntDisable(GPIOA2_BASE,GPIO_PIN_6);
        MAP_GPIOIntClear(GPIOA2_BASE,GPIO_PIN_6);
        MAP_IntDisable(INT_GPIOA2);
        
        //Call Speaker Handler
        if(g_pAudioOutControlHdl)
        {
            g_pAudioOutControlHdl();
        }
    }
}

//*****************************************************************************
//
//!  \brief Initialize Push Button GPIO
//!
//! \param[in] pAudioInControl               GPIO Interrupt Handler
//! \param[in] pAudioOutControl              GPIO Interrupt Handler

//!
//! \return                     0 - Success
//!                               -1 - Error
//!
//! \brief  Initializes Audio Player Push Button Controls
//
//*****************************************************************************
long InitControl(P_AUDIO_HANDLER pAudioInControl,P_AUDIO_HANDLER pAudioOutControl)
{
  long lRetVal = -1;
  //
  // Set Interrupt Type for GPIO
  // 
  MAP_GPIOIntTypeSet(GPIOA1_BASE,GPIO_PIN_5,GPIO_FALLING_EDGE);
  MAP_GPIOIntTypeSet(GPIOA2_BASE,GPIO_PIN_6,GPIO_FALLING_EDGE);
  
  //
  // Store Interrupt handlers
  //
  g_pAudioInControlHdl = pAudioInControl;
  g_pAudioOutControlHdl = pAudioOutControl;
  
  //
  // Register Interrupt handler
  //
  lRetVal = osi_InterruptRegister(INT_GPIOA1,(P_OSI_INTR_ENTRY)MICButtonHandler,\
                                   INT_PRIORITY_LVL_1);
  ASSERT_ON_ERROR(lRetVal);

  lRetVal = osi_InterruptRegister(INT_GPIOA2,(P_OSI_INTR_ENTRY) \
                                      SpeakerButtonHandler,INT_PRIORITY_LVL_1);
  ASSERT_ON_ERROR(lRetVal);
  
  //
  // Enable Interrupt
  //
  MAP_GPIOIntClear(GPIOA1_BASE,GPIO_PIN_5);
  MAP_GPIOIntEnable(GPIOA1_BASE,GPIO_INT_PIN_5);
  MAP_GPIOIntClear(GPIOA2_BASE,GPIO_PIN_6);
  MAP_GPIOIntEnable(GPIOA2_BASE,GPIO_INT_PIN_6);
  
  return SUCCESS;

}

//*****************************************************************************
//
//! AudioControlTask Task Routine
//!
//! \param pvParameters     Parameters to the task's entry function
//!
//! \return None
//
//*****************************************************************************
void AudioControlTask(void *pvParameters)
{
    tTxMsg Msg; 
    int ret=0;
    
    for(;;)
    {
        ret = osi_MsgQRead( &g_ControlMsgQueue, &Msg, OSI_WAIT_FOREVER );
        if(OSI_OK==ret)
        {
            Msg.pEntry(Msg.pValue);
        }
    }
}

//*****************************************************************************
//
//! Control Create Routine
//!
//! \param None
//!
//! \return                     0 - Success
//!                               -1 - Error
//
//*****************************************************************************

long ControlTaskCreate()
{
    long lRetVal = -1;
    lRetVal = InitControl(MICStartStopControl,SpeakerStartStopControl);
    ASSERT_ON_ERROR(lRetVal);  

    lRetVal = osi_MsgQCreate(&g_ControlMsgQueue,"g_ControlMsgQueue",\
                                  sizeof(tTxMsg),1);
    ASSERT_ON_ERROR(lRetVal);

    lRetVal = osi_TaskCreate(AudioControlTask, \
                          (signed char*)"AudioControlTask",\
                            2048, NULL, 1, &g_AudioControlTask );
    ASSERT_ON_ERROR(lRetVal);

    return SUCCESS;

}

//*****************************************************************************
//
//! Control Destroy Routine
//!
//! \param None
//!
//! \return                     0 - Success
//!                               -1 - Error
//
//*****************************************************************************
long ControlTaskDestroy()
{
    long lRetVal = -1;

    lRetVal = osi_MsgQDelete(&g_ControlMsgQueue);
    ASSERT_ON_ERROR(lRetVal);

    osi_TaskDelete(&g_AudioControlTask);

    g_ControlMsgQueue = NULL;
    g_AudioControlTask = NULL;

    return SUCCESS;
}


//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

