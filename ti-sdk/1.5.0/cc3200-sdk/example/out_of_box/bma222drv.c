//*****************************************************************************
// BMA222drv.c - Accelerometer sensor driver APIs.
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

//*****************************************************************************
//
//! \addtogroup oob
//! @{
//
//*****************************************************************************
#include <stdio.h>
#include <math.h>
#include "bma222drv.h"
// Common interface includes
#include "i2c_if.h"
#include "uart_if.h"


//*****************************************************************************
//                      MACRO DEFINITIONS
//*****************************************************************************
#define FAILURE                 -1
#define SUCCESS                 0
#define RET_IF_ERR(Func)        {int iRetVal = (Func); \
                                 if (SUCCESS != iRetVal) \
                                     return  iRetVal;}
#define DBG_PRINT               Report

//****************************************************************************
//                      LOCAL FUNCTION DEFINITIONS                          
//****************************************************************************


//****************************************************************************
//
//! Returns the value in the specified register
//!
//! \param ucRegAddr is the offset register address
//! \param pucRegValue is the pointer to the register value store
//! 
//! This function  
//!    1. Returns the value in the specified register
//!
//! \return 0: Success, < 0: Failure.
//
//****************************************************************************
int
GetRegisterValue(unsigned char ucRegAddr, unsigned char *pucRegValue)
{
    //
    // Invoke the readfrom  API to get the required byte
    //
    if(I2C_IF_ReadFrom(BMA222_DEV_ADDR, &ucRegAddr, 1,
                   pucRegValue, 1) != 0)
    {
        DBG_PRINT("BMA222 sensor I2C ReadFrom failed\n\r");
        return FAILURE;
    }

    return SUCCESS;
}

//****************************************************************************
//
//! Sets the value in the specified register
//!
//! \param ucRegAddr is the offset register address
//! \param ucRegValue is the register value to be set
//! 
//! This function  
//!    1. Returns the value in the specified register
//!
//! \return 0: Success, < 0: Failure.
//
//****************************************************************************
int
SetRegisterValue(unsigned char ucRegAddr, unsigned char ucRegValue)
{
    unsigned char ucData[2];
    //
    // Select the register to be written followed by the value.
    //
    ucData[0] = ucRegAddr;
    ucData[1] = ucRegValue;
    //
    // Initiate the I2C write
    //
    if(I2C_IF_Write(BMA222_DEV_ADDR,ucData,2,1) == 0)
    {
        return SUCCESS;
    }
    else
    {
        DBG_PRINT("I2C write failed\n\r");
    }

    return FAILURE;
}

//****************************************************************************
//
//! Reads a block of continuous data
//!
//! \param ucRegAddr is the start offset register address
//! \param pucBlkData is the pointer to the data value store
//! \param ucBlkDataSz is the size of data to be read
//! 
//! This function  
//!    1. Returns the data values in the specified store
//!
//! \return 0: Success, < 0: Failure.
//
//****************************************************************************
int
BlockRead(unsigned char ucRegAddr, 
          unsigned char *pucBlkData,
          unsigned char ucBlkDataSz)
{
    //
    // Invoke the readfrom I2C API to get the required bytes
    //
    if(I2C_IF_ReadFrom(BMA222_DEV_ADDR, &ucRegAddr, 1,
                   pucBlkData, ucBlkDataSz) != 0)
    {        
        DBG_PRINT("I2C readfrom failed\n");
        return FAILURE;
    }

    return SUCCESS;
}

//****************************************************************************
//
//! Initialize the BMA222 accelerometer device with defaults
//!
//! \param None
//! 
//! This function  
//!    1. Reads the CHIP ID.
//!
//! \return 0: Success, < 0: Failure.
//
//****************************************************************************
int
BMA222Open()
{
    unsigned char ucRegVal;
    //
    // Read the CHIP ID NUM
    //
    RET_IF_ERR(GetRegisterValue(BMA222_CHID_ID_NUM, &ucRegVal));
    DBG_PRINT("CHIP ID: 0x%x\n\r", ucRegVal);

    return SUCCESS;
}

//****************************************************************************
//
//! Place the BMA222 accelerometer device to standby
//!
//! \param None
//! 
//! This function  
//!    1. Sets the device to standby mode.
//!
//! \return 0: Success, < 0: Failure.
//
//****************************************************************************
int
BMA222Close()
{
    return SUCCESS;
}

//****************************************************************************
//
//! Get the accelerometer data readings
//!
//! \param pfAccX pointer to the AccX store
//! \param pfAccY pointer to the AccY store
//! \param pfAccZ pointer to the AccZ store
//! 
//! This function  
//!    1. Reads the data registers over I2C.
//!    2. Applies the range conversion to the raw values
//!
//! \return 0: Success, < 0: Failure.
//
//****************************************************************************
int
BMA222Read(signed char *pcAccX, signed char *pcAccY, signed char *pcAccZ)
{
    char cAccX = 0;
    char cAccY = 0;
    char cAccZ = 0;
    //
    // Read the acclerometer output registers LSB and MSB
    //
    RET_IF_ERR(BlockRead(BMA222_ACC_DATA_X, (unsigned char *)&cAccX,
                     sizeof(cAccX)));

    RET_IF_ERR(BlockRead(BMA222_ACC_DATA_Y, (unsigned char *)&cAccY,
             sizeof(cAccY)));

    RET_IF_ERR(BlockRead(BMA222_ACC_DATA_Z, (unsigned char *)&cAccZ,
             sizeof(cAccZ)));

    *pcAccX = cAccX;
    *pcAccY = cAccY;
    *pcAccZ = cAccZ;

    return SUCCESS;
}

//****************************************************************************
//
//! Get the raw accelerometer data register readings
//!
//! \param psAccX pointer to the raw AccX store
//! \param psAccY pointer to the raw AccY store
//! \param psAccZ pointer to the raw AccZ store
//! 
//! This function  
//!    1. Reads the data registers over I2C.
//!    2. Returns the accelerometer readings
//!
//! \return 0: Success, < 0: Failure.
//
//****************************************************************************
int
BMA222ReadNew(signed char *pcAccX, signed char *pcAccY, signed char *pcAccZ)
{
    char cAccX[6];
   
    //
    // Read the acclerometer output registers LSB and MSB
    //
    RET_IF_ERR(BlockRead(BMA222_ACC_DATA_X_NEW, (unsigned char *)cAccX,6));

     //
    // Check whether new Sensor Data is available
    //
    if((cAccX[0] & 0x1) && (cAccX[2] & 0x1) && (cAccX[4] & 0x1))
    {
        *pcAccX = cAccX[1];
        *pcAccY = cAccX[3];
        *pcAccZ = cAccX[5];
        return SUCCESS;
    }

    //New Sensor Data Not Available
    return FAILURE;

}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
