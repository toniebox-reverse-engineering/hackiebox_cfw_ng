//*****************************************************************************
// des_vector.h
//
// Test Vector data for DES Module
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


#ifndef __DES_VECTOR_H__
#define __DES_VECTOR_H__

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
typedef struct DESTestVectorStruct
{
    uint32_t ui32TestNumber;
    uint32_t ui32KeySize;
    uint8_t * pui32Key;
    uint8_t pui32IV[8];
    uint32_t ui32DataLength;
    uint8_t * pui32PlainText;
    uint8_t pui32CipherText[64];
} sDESTestVector;

uint8_t pui32DESPlainText[64] = {
    0xe2, 0xbe, 0xc1, 0x6b, 0x96, 0x9f, 0x40, 0x2e, 0x11, 0x7e, 0x3d, 0xe9, 0x2a, 0x17, 0x93, 0x73,
    0x57, 0x8a, 0x2d, 0xae, 0x9c, 0xac, 0x03, 0x1e, 0xac, 0x6f, 0xb7, 0x9e, 0x51, 0x8e, 0xaf, 0x45,
    0x46, 0x1c, 0xc8, 0x30, 0x11, 0xe4, 0x5c, 0xa3, 0x19, 0xc1, 0xfb, 0xe5, 0xef, 0x52, 0x0a, 0x1a,
    0x45, 0x24, 0x9f, 0xf6, 0x17, 0x9b, 0x4f, 0xdf, 0x7b, 0x41, 0x2b, 0xad, 0x10, 0x37, 0x6c, 0xe6
};
uint8_t pui32DESKey[8] = {
    0xc7, 0xf5, 0x1c, 0x87, 0x80, 0x76, 0x21, 0x1f
};
uint8_t pui32TDESKey[24] = {
    0xc7, 0xf5, 0x1c, 0x87, 0x80, 0x76, 0x21, 0x1f, 0x5d, 0xe5, 0xc8, 0x71, 0xa2, 0x43, 0xcf, 0x7e,
    0xd2, 0x5f, 0xdb, 0x75, 0xad, 0x73, 0x06, 0x8f
};

sDESTestVector psDESTestVectors =
{
      
        0,
        8,
        pui32DESKey,
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        64,
        pui32DESPlainText,
        {0x3e, 0x5c, 0x72, 0x7d, 0x50, 0xc1, 0xc9, 0x5a, 0xd6, 0xaa, 0xee, 0x2d, 0xf7, 0xbe, 0xb1, 0x5e,
         0xaf, 0x8c, 0xc1, 0x1e, 0x2d, 0x71, 0x83, 0x4a, 0x30, 0xce, 0xf9, 0x4a, 0xc0, 0x28, 0x05, 0xce,
         0xfd, 0x0f, 0xb2, 0x55, 0x2d, 0x1e, 0xb2, 0x8a, 0x82, 0x46, 0xa2, 0x6c, 0x51, 0x00, 0xbd, 0xa5,
         0x6e, 0x93, 0xc4, 0x98, 0x4a, 0xf2, 0x05, 0xc1, 0x30, 0xf7, 0x2c, 0x00, 0x72, 0xf9, 0x36, 0xf9}

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
