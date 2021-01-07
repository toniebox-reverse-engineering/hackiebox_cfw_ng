//*****************************************************************************
// gpio_if.c
//
// GPIO interface APIs, this common interface file helps to configure,
// set/toggle only 3 GPIO pins which are connected to 3 LEDs of CC32xx Launchpad
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

// Standard includes
#include <stdio.h>

// Driverlib includes
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_apps_rcm.h"
#include "interrupt.h"
#include "pin.h"
#include "gpio.h"
#include "prcm.h"
#include "rom.h"
#include "rom_map.h"

// OS includes
#if defined(USE_TIRTOS) || defined(USE_FREERTOS) || defined(SL_PLATFORM_MULTI_THREADED)
#include <stdlib.h>
#include "osi.h"
#endif

// Common interface include
#include "gpio_if.h"

//****************************************************************************
//                      GLOBAL VARIABLES                                   
//****************************************************************************
static unsigned long ulReg[]=
{
    GPIOA0_BASE,
    GPIOA1_BASE,
    GPIOA2_BASE,
    GPIOA3_BASE,
    GPIOA4_BASE
};

//*****************************************************************************
// Variables to store TIMER Port,Pin values
//*****************************************************************************
unsigned int g_uiLED1Port = 0,g_uiLED2Port = 0,g_uiLED3Port = 0;
unsigned char g_ucLED1Pin,g_ucLED2Pin,g_ucLED3Pin;

#define GPIO_LED1 9
#define GPIO_LED2 10
#define GPIO_LED3 11


//****************************************************************************
//                      LOCAL FUNCTION DEFINITIONS                          
//****************************************************************************

static unsigned char
GetPeripheralIntNum(unsigned int uiGPIOPort)
{

    switch(uiGPIOPort)
    {
       case GPIOA0_BASE:
          return INT_GPIOA0;
       case GPIOA1_BASE:
          return INT_GPIOA1;
       case GPIOA2_BASE:
          return INT_GPIOA2;
       case GPIOA3_BASE:
          return INT_GPIOA3;
       default:
          return INT_GPIOA0;
    }

}

//*****************************************************************************
//
//! GPIO Enable & Configuration
//!
//! \param  ucPins is the bit-pack representation of 3 LEDs
//!         LSB:GP09-GP10-GP11:MSB
//!
//! \return None
//
//*****************************************************************************
void
GPIO_IF_LedConfigure(unsigned char ucPins)
{

  if(ucPins & LED1)
  {
    GPIO_IF_GetPortNPin(GPIO_LED1,
                        &g_uiLED1Port,
                        &g_ucLED1Pin);
  }

  if(ucPins & LED2)
  {
    GPIO_IF_GetPortNPin(GPIO_LED2,
                  &g_uiLED2Port,
          &g_ucLED2Pin);
  }

  if(ucPins & LED3)
  {
    GPIO_IF_GetPortNPin(GPIO_LED3,
                      &g_uiLED3Port,
                      &g_ucLED3Pin);

  }

}

//*****************************************************************************
//
//! Turn LED On
//!
//! \param  ledNum is the LED Number
//!
//! \return none
//!
//! \brief  Turns a specific LED Off
//
//*****************************************************************************
void
GPIO_IF_LedOn(char ledNum)
{
    switch(ledNum)
    {
        case MCU_ON_IND:
        case MCU_EXECUTE_SUCCESS_IND:
        case MCU_GREEN_LED_GPIO:
        {
          /* Switch ON GREEN LED */
          GPIO_IF_Set(GPIO_LED3, g_uiLED3Port, g_ucLED3Pin, 1);
          break;
        }
        case MCU_SENDING_DATA_IND:
        case MCU_EXECUTE_FAIL_IND:
        case MCU_ORANGE_LED_GPIO:
        {
          /* Switch ON ORANGE LED */
          GPIO_IF_Set(GPIO_LED2, g_uiLED2Port, g_ucLED2Pin, 1);
          break;
        }
        case MCU_ASSOCIATED_IND:
        case MCU_IP_ALLOC_IND:
        case MCU_SERVER_INIT_IND:
        case MCU_CLIENT_CONNECTED_IND:
        case MCU_RED_LED_GPIO:
        {
          /* Switch ON RED LED */
          GPIO_IF_Set(GPIO_LED1, g_uiLED1Port, g_ucLED1Pin, 1);
          break;
        }
        case MCU_ALL_LED_IND:
        {
          /* Switch ON ALL LEDs LED */
          GPIO_IF_Set(GPIO_LED3, g_uiLED3Port, g_ucLED3Pin, 1);
          GPIO_IF_Set(GPIO_LED2, g_uiLED2Port, g_ucLED2Pin, 1);
          GPIO_IF_Set(GPIO_LED1, g_uiLED1Port, g_ucLED1Pin, 1);
          break;
        }
        default:
          break;
    }
}

//*****************************************************************************
//
//! Turn LED Off
//!
//! \param  ledNum is the LED Number
//!
//! \return none
//!
//! \brief  Turns a specific LED Off
//
//*****************************************************************************
void
GPIO_IF_LedOff(char ledNum)
{
  switch(ledNum)
  {
    case MCU_ON_IND:
    case MCU_EXECUTE_SUCCESS_IND:
    case MCU_GREEN_LED_GPIO:
    {
      /* Switch OFF GREEN LED */
      GPIO_IF_Set(GPIO_LED3, g_uiLED3Port, g_ucLED3Pin, 0);
      break;
    }
    case MCU_SENDING_DATA_IND:
    case MCU_EXECUTE_FAIL_IND:
    case MCU_ORANGE_LED_GPIO:
    {
      /* Switch OFF ORANGE LED */
      GPIO_IF_Set(GPIO_LED2, g_uiLED2Port, g_ucLED2Pin, 0);
      break;
    }
    case MCU_ASSOCIATED_IND:
    case MCU_IP_ALLOC_IND:
    case MCU_SERVER_INIT_IND:
    case MCU_CLIENT_CONNECTED_IND:
    case MCU_RED_LED_GPIO:
    {
      /* Switch OFF RED LED */
      GPIO_IF_Set(GPIO_LED1, g_uiLED1Port, g_ucLED1Pin, 0);
      break;
    }
    case MCU_ALL_LED_IND:
    {
      /* Switch OFF ALL LEDs LED */
      GPIO_IF_Set(GPIO_LED3, g_uiLED3Port, g_ucLED3Pin, 0);
      GPIO_IF_Set(GPIO_LED2, g_uiLED2Port, g_ucLED2Pin, 0);
      GPIO_IF_Set(GPIO_LED1, g_uiLED1Port, g_ucLED1Pin, 0);
      break;
    }
    default:
      break;
  }
}

//*****************************************************************************
//
//!  \brief This function returns LED current Status
//!
//!  \param[in] ucGPIONum is the GPIO to which the LED is connected
//!                MCU_GREEN_LED_GPIO\MCU_ORANGE_LED_GPIO\MCU_RED_LED_GPIO
//!
//!
//!  \return 1: LED ON, 0: LED OFF
//
//*****************************************************************************
unsigned char
GPIO_IF_LedStatus(unsigned char ucGPIONum)
{
  unsigned char ucLEDStatus;
  switch(ucGPIONum)
  {
    case MCU_GREEN_LED_GPIO:
    {
      ucLEDStatus = GPIO_IF_Get(ucGPIONum, g_uiLED3Port, g_ucLED3Pin);
      break;
    }
    case MCU_ORANGE_LED_GPIO:
    {
      ucLEDStatus = GPIO_IF_Get(ucGPIONum, g_uiLED2Port, g_ucLED2Pin);
      break;
    }
    case MCU_RED_LED_GPIO:
    {
      ucLEDStatus = GPIO_IF_Get(ucGPIONum, g_uiLED1Port, g_ucLED1Pin);
      break;
    }
    default:
        ucLEDStatus = 0;
  }
  return ucLEDStatus;
}

//*****************************************************************************
//
//! Toggle the Led state
//!
//! \param  ledNum is the LED Number
//!
//! \return none
//!
//! \brief  Toggles a board LED
//
//*****************************************************************************
void GPIO_IF_LedToggle(unsigned char ucLedNum)
{

    unsigned char ucLEDStatus = GPIO_IF_LedStatus(ucLedNum);
    if(ucLEDStatus == 1)
    {
        GPIO_IF_LedOff(ucLedNum);
    }
    else
    {
        GPIO_IF_LedOn(ucLedNum);
    }
}

//****************************************************************************
//
//! Get the port and pin of a given GPIO
//!
//! \param ucPin is the pin to be set-up as a GPIO (0:39)
//! \param puiGPIOPort is the pointer to store GPIO port address return value
//! \param pucGPIOPin is the pointer to store GPIO pin return value
//! 
//! This function  
//!    1. Return the GPIO port address and pin for a given external pin number
//!
//! \return None.
//
//****************************************************************************
void
GPIO_IF_GetPortNPin(unsigned char ucPin,
                unsigned int *puiGPIOPort,
                    unsigned char *pucGPIOPin)
{
    //
    // Get the GPIO pin from the external Pin number
    //
    *pucGPIOPin = 1 << (ucPin % 8);

    //
    // Get the GPIO port from the external Pin number
    //
    *puiGPIOPort = (ucPin / 8);
    *puiGPIOPort = ulReg[*puiGPIOPort];
}

//****************************************************************************
//
//! Configures the GPIO selected as input to generate interrupt on activity
//!
//! \param uiGPIOPort is the GPIO port address
//! \param ucGPIOPin is the GPIO pin of the specified port
//! \param uiIntType is the type of the interrupt (refer gpio.h)
//! \param pfnIntHandler is the interrupt handler to register
//! 
//! This function  
//!    1. Sets GPIO interrupt type
//!    2. Registers Interrupt handler
//!    3. Enables Interrupt
//!
//! \return None
//
//****************************************************************************
void
GPIO_IF_ConfigureNIntEnable(unsigned int uiGPIOPort,
                                  unsigned char ucGPIOPin,
                                  unsigned int uiIntType,
                                  void (*pfnIntHandler)(void))
{
    //
    // Set GPIO interrupt type
    //
    MAP_GPIOIntTypeSet(uiGPIOPort,ucGPIOPin,uiIntType);

    //
    // Register Interrupt handler
    //
#if defined(USE_TIRTOS) || defined(USE_FREERTOS) || defined(SL_PLATFORM_MULTI_THREADED) 
    // USE_TIRTOS: if app uses TI-RTOS (either networking/non-networking)
    // USE_FREERTOS: if app uses Free-RTOS (either networking/non-networking)
    // SL_PLATFORM_MULTI_THREADED: if app uses any OS + networking(simplelink)
    osi_InterruptRegister(GetPeripheralIntNum(uiGPIOPort),
                                        pfnIntHandler, INT_PRIORITY_LVL_1);
                
#else
	MAP_IntPrioritySet(GetPeripheralIntNum(uiGPIOPort), INT_PRIORITY_LVL_1);
    MAP_GPIOIntRegister(uiGPIOPort,pfnIntHandler);
#endif

    //
    // Enable Interrupt
    //
    MAP_GPIOIntClear(uiGPIOPort,ucGPIOPin);
    MAP_GPIOIntEnable(uiGPIOPort,ucGPIOPin);
}

//****************************************************************************
//
//! Set a value to the specified GPIO pin
//!
//! \param ucPin is the GPIO pin to be set (0:39)
//! \param uiGPIOPort is the GPIO port address
//! \param ucGPIOPin is the GPIO pin of the specified port
//! \param ucGPIOValue is the value to be set
//! 
//! This function  
//!    1. Sets a value to the specified GPIO pin
//!
//! \return None.
//
//****************************************************************************
void 
GPIO_IF_Set(unsigned char ucPin,
             unsigned int uiGPIOPort,
             unsigned char ucGPIOPin,
             unsigned char ucGPIOValue)
{
    //
    // Set the corresponding bit in the bitmask
    //
    ucGPIOValue = ucGPIOValue << (ucPin % 8);

    //
    // Invoke the API to set the value
    //
    MAP_GPIOPinWrite(uiGPIOPort,ucGPIOPin,ucGPIOValue);
}

//****************************************************************************
//
//! Set a value to the specified GPIO pin
//!
//! \param ucPin is the GPIO pin to be set (0:39)
//! \param uiGPIOPort is the GPIO port address
//! \param ucGPIOPin is the GPIO pin of the specified port
//!
//! This function
//!    1. Gets a value of the specified GPIO pin
//!
//! \return value of the GPIO pin
//
//****************************************************************************
unsigned char
GPIO_IF_Get(unsigned char ucPin,
             unsigned int uiGPIOPort,
             unsigned char ucGPIOPin)
{
    unsigned char ucGPIOValue;
    long lGPIOStatus;

    //
    // Invoke the API to Get the value
    //
    lGPIOStatus =  MAP_GPIOPinRead(uiGPIOPort,ucGPIOPin);

    //
    // Set the corresponding bit in the bitmask
    //
    ucGPIOValue = lGPIOStatus >> (ucPin % 8);
    return ucGPIOValue;
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
