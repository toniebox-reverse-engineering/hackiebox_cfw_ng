/******************************************************************************
 *
 *   Copyright (C) 2014 Texas Instruments Incorporated
 *
 *   All rights reserved. Property of Texas Instruments Incorporated.
 *   Restricted rights to use, duplicate or disclose this code are
 *   granted through contract.
 *
 *   The program may not be used without the written permission of
 *   Texas Instruments Incorporated or against the terms and conditions
 *   stipulated in the agreement under which this program has been supplied,
 *   and under no circumstances can it be used with non-TI connectivity device.
 *
 ******************************************************************************/

#include "platform.h"
#include <hw_types.h>
#include <prcm.h>
#include <rom_map.h>
#include <rom.h>

void platform_timer_init()
{
    MAP_PRCMRTCInUseSet();
}

u32 platform_get_time_in_secs()
{
    u32 Secs = 0;
    u16 Msec = 0;

    MAP_PRCMRTCGet((u32*) &Secs, &Msec);
    return (Secs);
}
