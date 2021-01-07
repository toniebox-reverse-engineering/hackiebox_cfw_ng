//*****************************************************************************
// BMA222drv.h - Defines and Macros for the BMA222 accelerometer interface.
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


#ifndef __BMA222_H__
#define __BMA222_H__

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
// BMA222 Device I2C address
//*****************************************************************************
#define BMA222_DEV_ADDR          0x18

#define BMA222_CHID_ID_NUM       0x00

//*****************************************************************************
// BMA222 Acc Data Register related macros
//*****************************************************************************
#define BMA222_ACC_DATA_X_NEW   (0x2)
#define BMA222_ACC_DATA_X   	(0x3)
#define BMA222_ACC_DATA_Y_NEW  	(0x4)
#define BMA222_ACC_DATA_Y   	(0x5)
#define BMA222_ACC_DATA_Z_NEW  	(0x6)
#define BMA222_ACC_DATA_Z   	(0x7)



//*****************************************************************************
// BMA222 Data Interpretation macros
//*****************************************************************************
#define RESOLUTION_8BIT         ((float)(1.999 / 127))  //+-2g
#define G_VAL                   ((float)9.7798)

//*****************************************************************************
//
// API Function prototypes
//
//*****************************************************************************
int BMA222Open();
int BMA222Read(signed char *pcAccX, signed char *pcAccY, signed char *pcAccZ);
int BMA222ReadNew(signed char *pcAccX, signed char *pcAccY, signed char *pcAccZ);
int BMA222Close();

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif //  __BMA222_H__
