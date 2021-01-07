//*****************************************************************************
// mmc_demo.h
//
// MMC Demo Application header file. 
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

#ifndef __MMC_DEMO_H__
#define __MMC_DEMO_H__

//*****************************************************************************
// Card Attribute structure                                 
//*****************************************************************************
typedef struct
{
  unsigned long  ulCardType;
  unsigned long  long ullCapacity;
  unsigned long  ulVersion;
  unsigned long  ulCapClass;
  unsigned short ulRCA;
}CardAttrib_t;

//*****************************************************************************
// MACROS                                
//*****************************************************************************
#define RETRY_TIMEOUT        2000

#define CARD_TYPE_UNKNOWN    0
#define CARD_TYPE_MMC        1
#define CARD_TYPE_SDCARD     2

#define CARD_CAP_CLASS_SDSC  0
#define CARD_CAP_CLASS_SDHC  1

#define CARD_VERSION_1       0
#define CARD_VERSION_2       1


#endif //__MMC_DEMO_H__
