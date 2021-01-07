//*****************************************************************************
// crc_vector.h
//
// Test Vector data for CRC Module
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


#ifndef __CRC_VECTOR_H__
#define __CRC_VECTOR_H__

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// Test Vector Structure
//
//*****************************************************************************

uint32_t g_pui32RandomData[] =
{
    0x8a5f1b22, 0xcb935d29, 0xcc1ac092, 0x5dad8c9e, 0x6a83b39f, 0x8607dc60,
    0xda0ba4d2, 0xf49b0fa2, 0xaf35d524, 0xffa8001d, 0xbcc931e8, 0x4a2c99ef,
    0x7fa297ab, 0xab943bae, 0x07c61cc4, 0x47c8627d
};

//*****************************************************************************
//
// Test Vector Structure
//
//*****************************************************************************
typedef struct CRCTestVectorStruct
{

    uint32_t ui32Seed;
    uint32_t ui32Result;
    uint32_t *ui32Data;
    uint32_t ui32Length;
} sCRCTestVector;

//*****************************************************************************
//
// Test vectors
//
//*****************************************************************************
sCRCTestVector g_psCRC8005TestVectors =
{

        0x0000ffff,
        0x0000eff9,
        g_pui32RandomData,
        16

};

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif
