//*****************************************************************************
// bootmgr.h
//
// Boot Manager header file
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

#ifndef __BOOTMGR_H__
#define __BOOTMGR_H__

//****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
// User image tokens
//*****************************************************************************
#ifndef FAST_BOOT
#define FACTORY_IMG_TOKEN       0x5555AAAA
#endif
  
#define USER_IMG_1_TOKEN        0xAA5555AA
#define USER_IMG_2_TOKEN        0x55AAAA55
#define USER_BOOT_INFO_TOKEN    0xA5A55A5A



//*****************************************************************************
// Macros
//*****************************************************************************
#ifndef FAST_BOOT
#define APP_IMG_SRAM_OFFSET     0x20004000
#else
#define APP_IMG_SRAM_OFFSET     0x20008000
#endif
  
#define DEVICE_IS_CC3101RS      0x18
#define DEVICE_IS_CC3101S       0x1B


//*****************************************************************************
// Fucntion prototype
//*****************************************************************************
extern void Run(unsigned long);


//****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//****************************************************************************
#ifdef __cplusplus
}
#endif

#endif //__BOOTMGR_H__
