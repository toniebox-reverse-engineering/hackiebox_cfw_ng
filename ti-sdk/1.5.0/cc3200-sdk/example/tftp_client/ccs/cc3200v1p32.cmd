/******************************************************************************
 *
 * cc3200v1p32.cmd - CCS linker configuration file for cc3200 ES 1.32.
 *
 * Copyright (c) 2014 Texas Instruments Incorporated.  All rights reserved.
 * TI Information - Selective Disclosure
 *
 *****************************************************************************/

--retain=g_pfnVectors

/* The following command line options are set as part of the CCS project.    */
/* If you are building using the command line, or for some reason want to    */
/* define them here, you can uncomment and modify these lines as needed.     */
/* If you are using CCS for building, it is probably better to make any such */
/* modifications in your CCS project and leave this file alone.              */
/*                                                                           */

/* The starting address of the application.  Normally the interrupt vectors  */
/* must be located at the beginning of the application.                      */
#define RAM_BASE 0x20004000

/* System memory map */

MEMORY
{
    /* Application uses internal RAM for program and data */
    SRAM_CODE (RWX) : origin = 0x20004000, length = 0x14300
    SRAM_DATA (RWX) : origin = 0x20018300, length = 0x17D00
}

/* Section allocation in memory */

SECTIONS
{
    .intvecs:   > RAM_BASE
    .text   :   > SRAM_CODE
    .const  :   > SRAM_CODE
    .cinit  :   > SRAM_CODE
    .pinit  :   > SRAM_DATA
    .init_array : > SRAM_CODE
    .vtable :   > SRAM_CODE
    .data   :   > SRAM_DATA
    .bss    :   > SRAM_DATA
    .sysmem :   > SRAM_DATA
    .stack  :   > SRAM_DATA(HIGH)
}

