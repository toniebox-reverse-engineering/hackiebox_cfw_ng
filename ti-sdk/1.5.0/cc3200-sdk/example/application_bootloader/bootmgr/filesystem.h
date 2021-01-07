//*****************************************************************************
// filesystem.h
//
// uDMA interface MACRO and function prototypes
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

#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__

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
// macros
//*****************************************************************************
#define MAX_FILE_NAME_LENGTH           180
#define MAX_CERTIFIC_FILE_NAME_LENGTH  (MAX_FILE_NAME_LENGTH)
#define SHA_SIZE_DIGEST                 20
#define FILE_SIGNATUE_SIZE              256
#define FAST_FILE_SIGNATURE_SIZE        20
#define MAX_TOKENS                      4

//*****************************************************************************
// typedef
//*****************************************************************************
typedef unsigned long  UINT32; 
typedef unsigned short UINT16;
typedef unsigned char  UINT8;
typedef unsigned long Token_t;
typedef long SlFileHandle_t;
typedef long NumberOfBytes_t;
typedef UINT16 InternalSecurityFlag_t;

//*****************************************************************************
// typedef struct
//*****************************************************************************
typedef struct
{
	Token_t TokenVal[ MAX_TOKENS ];
}TokensVal_t;


typedef struct
{
   char Signature[ FILE_SIGNATUE_SIZE ];
}Signature_t;


typedef struct
{
    char Cert[ MAX_CERTIFIC_FILE_NAME_LENGTH];
}CertificFileName_t;


typedef struct
{
  CertificFileName_t  CertificFileName;
  Signature_t         Signature;
  UINT8               InternalSignatureFlags;//See SignatureFlags_t
}SignatureAttr_t;


typedef struct
{
    UINT32           Size:24;
    UINT8            SecurityFlags;
    UINT32           AllocatedSize:24;
    UINT8            FileOptions; //Not in use
    TokensVal_t TokensVal;
}SlFsFileInfo2_t;


//*****************************************************************************
// Function pointers
//*****************************************************************************
#define fs_Open        ((SlFileHandle_t  (*)(UINT8 SecurityFlag,const unsigned char *pFileName , UINT32 Mode, Token_t* pToken ))(0x000020c5))
#define fs_Read        ((NumberOfBytes_t (*)(SlFileHandle_t FileHandle, UINT32 Offset, char* pData, UINT32 Len))(0x000022f3))
#define fs_GetFileInfo ((long            (*)(UINT8 SecurityFlag, const unsigned char *pFileName, const Token_t* pToken ,SlFsFileInfo2_t* pFsFileInfo))0x000023db)
#define fs_Close       ((long            (*)(InternalSecurityFlag_t InternalSecurityFlag, SlFileHandle_t FileHandle, SignatureAttr_t* pCertification))0x000022c5)

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif //  __FILESYSTEM_H__
