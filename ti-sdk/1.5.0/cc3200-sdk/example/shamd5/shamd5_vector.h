//*****************************************************************************
// shamd5_vector.h
//
// Test Vector data for ShaMD5 Module
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

#ifndef __SHAMD5_VECTOR_H__
#define __SHAMD5_VECTOR_H__

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

typedef struct ShaMD5TestVectorStruct
{
    uint8_t * puiHMACKey;
    uint32_t uiDataLength;
    uint8_t * puiPlainText;
    uint8_t *puiExpectedHash;
    uint32_t uiHashLength;
} sShaMD5TestVector;

//*****************************************************************************
//
// Sample key for generating HMAC.  This array contains 64 bytes (512 bits)
// of randomly generated data.
//
//*****************************************************************************
#if 0
uint8_t puiHMACKey[64] =
{
    0x8a, 0x5f, 0x1b, 0x22, 0xcb, 0x93, 0x5d, 0x29, 0xcc, 0x1a, 0xc0, 0x92, 0x5d, 0xad, 0x8c, 0x9e, 0x6a, 0x83, 0xb3, 0x9f, 0x86, 0x07, 0xdc, 0x60,
    0xda, 0x0b, 0xa4, 0xd2, 0xf4, 0x9b, 0x0f, 0xa2, 0xaf, 0x35, 0xd5, 0x24, 0xff, 0xa8, 0x00, 0x1d, 0xbc, 0xc9, 0x31, 0xe8, 0x4a, 0x2c, 0x99, 0xef,
    0x7f, 0xa2, 0x97, 0xab, 0xab, 0x94, 0x3b, 0xae, 0x07, 0xc6, 0x1c, 0xc4, 0x47, 0xc8, 0x62, 0x7d
};


uint8_t pui32MD5HMACResult[16] =
{
    0x6a, 0x14, 0x03, 0xa1, 0xd4, 0x08, 0xe1, 0x03, 0x8d, 0x23, 0x25, 0x98, 0xfe, 0xe3, 0x4b, 0x16
};

uint8_t pui32SHA224HMACResult[28] =
{
    0x78, 0xda, 0x29, 0x14, 0x71, 0x88, 0x03, 0xaa, 0x4e, 0x6e, 0xa2, 0x5e, 0x0b, 0x38, 0xd8, 0x56, 0x86, 0x98, 0x2b, 0xa9, 0x9f, 0x7f, 0x18, 0x40,
    0xc6, 0x9f, 0xdf, 0x04
};
#else
uint32_t puiHMACKey[16] =
{
    0x8a5f1b22, 0xcb935d29, 0xcc1ac092, 0x5dad8c9e, 0x6a83b39f, 0x8607dc60,
    0xda0ba4d2, 0xf49b0fa2, 0xaf35d524, 0xffa8001d, 0xbcc931e8, 0x4a2c99ef,
    0x7fa297ab, 0xab943bae, 0x07c61cc4, 0x47c8627d
};


uint32_t pui32MD5HMACResult[4] =
{
    0x6a1403a1, 0xd408e103, 0x8d232598, 0xfee34b16
};

uint32_t pui32SHA224HMACResult[7] =
{
    0x78da2914, 0x718803aa, 0x4e6ea25e, 0x0b38d856, 0x86982ba9, 0x9f7f1840,
    0xc69fdf04
};
#endif


//*****************************************************************************
//
// Source data for producing hashes. This array contains 1024 bytes of
// randomly generated data.
//
//*****************************************************************************
uint32_t puiRandomData[] =
{
    0x7c68c9ec, 0x72af34b3, 0xca0edf2e, 0x60f4860d, 0x50cfa1dc, 0x9a2b538c,
    0x98450274, 0x60f5c272, 0x7317d78e, 0x2361ca0e, 0xfa4a52b1, 0x658f729b,
    0x5267f9d9, 0x1bccd3ca, 0x2f0bb993, 0x1be38a3d, 0x00bd2d2a, 0x97405e63,
    0xe3efd585, 0xb02d1588, 0xe55d71c8, 0x43a27ecf, 0x5fd275db, 0x73ad8f06,
    0x88f55495, 0x68922493, 0x03ea6039, 0xe40a678a, 0x052847ce, 0xf7a28b46,
    0x3b60c73e, 0x3f08dbd4, 0x2a66b3a6, 0xcf398b15, 0xacbfc6d8, 0x6c15a285,
    0x997d0e01, 0xbfd12e26, 0xa26bc485, 0xb8946d2f, 0x0f84742b, 0x5be82a2f,
    0x8d2e2cc7, 0xc7a1dea6, 0xcfaa6cb6, 0xe706434c, 0x079810d0, 0x5eca9400,
    0x7b92dd1c, 0x1ec552e8, 0xa74ae9c3, 0x2e859af5, 0x8d9d1a35, 0x07ff6040,
    0xc0b19670, 0x2e348aa8, 0xed89efea, 0x3262e8f0, 0x45093372, 0x8f8bae5c,
    0x505d64bb, 0x9a172079, 0x327b5f67, 0xa3a12ba8, 0x7f573054, 0xd3d5f778,
    0xbc1bd124, 0x0d0ad1c6, 0x24ac345b, 0x4f50084a, 0x302a5985, 0xfa3e8b86,
    0x2022c497, 0xd297e4b4, 0xd1c53c01, 0x6e541890, 0x93ec53c6, 0x24c5ce2b,
    0xdd38e334, 0x078a0334, 0x2a470b22, 0xadad86b4, 0x7b2041db, 0xc74ce30b,
    0x8e6dc4ca, 0x273b85c8, 0x339d2334, 0x86d1dacc, 0xd588e165, 0xcee15221,
    0x8e11a0a1, 0x9315a6c2, 0x53e9fa9a, 0xf4bb6d7a, 0x421cb9ec, 0x1f370567,
    0xfd8c880f, 0xd20797cd, 0x90aee852, 0x2a2f966a, 0x126ffcdd, 0x44a2f09a,
    0xbac72ac4, 0x77d588c5, 0x77b53c09, 0x275b8828, 0x778a2be5, 0x40167d1e,
    0x550c0c94, 0x14e070e7, 0x597ff5a3, 0xbef40dc2, 0x8306d119, 0x6a8d29a6,
    0xb5d8e740, 0x52a37fe2, 0xdf34ad27, 0x1bb885fd, 0x6dd352f8, 0x8b0d62b5,
    0x5c82d35f, 0x0eb84312, 0xd2c7823a, 0x494f7a00, 0x30680642, 0x01fa9460,
    0xdc63956f, 0x70fa0b53, 0xd0865e78, 0x3a52e983, 0x318a881c, 0x4d113947,
    0xc0f302df, 0x6b2027fb, 0x1078566d, 0xd71d39a6, 0xcdd00388, 0x119e3c4e,
    0x4ddbf1c6, 0xb371eb0f, 0xdcbd768f, 0x2fc5b5e8, 0xc67a2efe, 0x29d18630,
    0xb389d68f, 0x26a71f13, 0x43583b57, 0x56f5eae8, 0x2edc7cd5, 0xcc93d41e,
    0xab691f87, 0x51ab1d8e, 0x37c2966e, 0x19ccd9ec, 0xb782124a, 0xdefc2804,
    0xea3bde3c, 0x46d81e08, 0xf828d58e, 0x757a39d3, 0xc92f1b5f, 0x56a2b368,
    0x1bbbb9b9, 0x46086ac7, 0x8a343144, 0x1675157a, 0x28ac0cf1, 0xb8695178,
    0x25fc4cec, 0x3f23a44e, 0x0a697977, 0x525794ad, 0xf920e15c, 0x49a0a7a7,
    0x1f54cafb, 0x7357b64c, 0x6d3a19c6, 0x5efb526d, 0x3d37f6e2, 0xd4f5835b,
    0x6ff454ee, 0x4f2a311c, 0x83cc4a40, 0x003036e9, 0xd481bf33, 0x38868b3c,
    0x63ee4445, 0x58426a29, 0xa022ae59, 0x07deb8ce, 0xfe3e673d, 0x176aa368,
    0xf2b18641, 0xbadeccd8, 0xea7a72b4, 0x72ccf0a0, 0xcdee3b08, 0x1689c54f,
    0xd577085a, 0xd9d79bd1, 0x089fa69a, 0x03fdaf65, 0x855e5697, 0x5788c00c,
    0x1139e03e, 0x48f4305f, 0x2d8ad2fd, 0x71ab04b5, 0xf5c7871c, 0x76801f21,
    0x329a590e, 0xe8e982a2, 0xdb67783e, 0x26ebf88b, 0x13ac5de7, 0x69b07707,
    0x2bc54e92, 0xc2556f94, 0x6d21bc3b, 0x3a230d0c, 0x4e02eeec, 0x53605beb,
    0x3a31e796, 0x6e186887, 0x8f93356e, 0xfa2342e4, 0xfbf2f519, 0x7ae95455,
    0xad6e9d94, 0xd942c7ab, 0x624f7aed, 0xd4158624, 0x82a0c0a9, 0x6d79b262,
    0xa7b9c84d, 0x2015bfeb, 0x462c7267, 0x44a17743, 0x7d207f71, 0xc2ab7566,
    0xaa833e65, 0x0a6c385e, 0x3b2d85f1, 0x8a4821a8, 0x62bf5742, 0xf55cf0e1,
    0xfc07d0d9, 0x54910235, 0xe8ae66c9, 0x9beb7306, 0xe5671f9e, 0x3332ad03,
    0xdb2343b6, 0x124332ac, 0xf595c7fb, 0xda2c72b0
};




sShaMD5TestVector g_psHMACShaMD5TestVectors =
{

    (uint8_t *)puiHMACKey,
    1024,
    (uint8_t *)puiRandomData,
    (uint8_t *)pui32MD5HMACResult,
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
