/*
 ************************************************************************
 *	wiring.c
 *
 *	Energia core files for cc3200
 *		Copyright (c) 2014 Robert Wessels. All right reserved.
 *
 *
 ***********************************************************************
  Derived from:
  wiring.c - Partial implementation of the Wiring API for the ATmega8.
  Part of Arduino - http://www.arduino.cc/

  Copyright (c) 2005-2006 David A. Mellis

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this library; if not, write to the
  Free Software Foundation, Inc., 59 Temple Place, Suite 330,
  Boston, MA  02111-1307  USA
 */


#include <stdbool.h>
#include <stdint.h>

#include "hw_ints.h"
#include "hw_nvic.h"
#include "hw_timer.h"
#include "hw_types.h"
#include "rom_map.h"
#include "systick.h"
#include "timer.h"

#include "globalDefines.h"


static void (*SysTickCbFuncs[8])(uint32_t ui32TimeMS);

#define SYSTICKHZ               1000
#define SYSTICKMS               (1000 / SYSTICKHZ)

static unsigned long milliseconds = 0;
#define SYSTICK_INT_PRIORITY    0x80


unsigned long micros(void)
{
	return (milliseconds * 1000) + ( ((HAL_FCPU_HZ / SYSTICKHZ) - MAP_SysTickValueGet()) / (HAL_FCPU_HZ/1000000));
}

unsigned long millis(void)
{
	return milliseconds;
}
/*
void delayMicroseconds(unsigned int us)
{
	// Systick timer rolls over every 1000000/SYSTICKHZ microseconds 
	if (us > (1000000UL / SYSTICKHZ - 1)) {
		delay(us / 1000);  // delay milliseconds
		us = us % 1000;     // handle remainder of delay
	};

	// 24 bit timer - mask off undefined bits
	unsigned long startTime = HWREG(NVIC_ST_CURRENT) & NVIC_ST_CURRENT_M;

	unsigned long ticks = (unsigned long)us * (HAL_FCPU_HZ/1000000UL);
	volatile unsigned long elapsedTime;

	if (ticks > startTime) {
		ticks = (ticks + (NVIC_ST_CURRENT_M - (unsigned long)HAL_FCPU_HZ / SYSTICKHZ)) & NVIC_ST_CURRENT_M;
	}

	do {
		elapsedTime = (startTime-(HWREG(NVIC_ST_CURRENT) & NVIC_ST_CURRENT_M )) & NVIC_ST_CURRENT_M;
	} while(elapsedTime <= ticks);
}

void delay(uint32_t millis)
{
	unsigned long i;
	for(i=0; i<millis*2; i++){
		delayMicroseconds(500);
	}
}


volatile bool stay_asleep = false;
*/
/* TODO: Replace sleep, sleepSeconds and suspend with actual RTC+Hibernate module implementation */
/*
void sleep(uint32_t ms)
{
	unsigned long i;

	stay_asleep = true;
	for(i=0; i<ms*2 && stay_asleep; i++) {
		delayMicroseconds(500);
	}
	stay_asleep = false;
}

void sleepSeconds(uint32_t seconds)
{
	unsigned long i, j;

	stay_asleep = true;
	for(i=0; i<seconds && stay_asleep; i++) {
		for(j=0; j<2000 && stay_asleep; j++) {
			delayMicroseconds(500);
		}
	}
	stay_asleep = false;
}

void suspend(void)
{
	stay_asleep = true;

	while(stay_asleep)
		;
}

void registerSysTickCb(void (*userFunc)(uint32_t))
{
	uint8_t i;
	for (i=0; i<8; i++) {
		if(!SysTickCbFuncs[i]) {
			SysTickCbFuncs[i] = userFunc;
			break;
		}
	}
}
*/
void xPortSysTickHandler(void) {
	milliseconds++;

	uint8_t i;
	for (i=0; i<8; i++) {
		if (SysTickCbFuncs[i])
			SysTickCbFuncs[i](SYSTICKMS);
	}
}
