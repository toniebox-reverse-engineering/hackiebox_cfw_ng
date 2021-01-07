//*****************************************************************************
// tmp006drv.c - Temperature sensor driver APIs.
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
//! \addtogroup home_automation
//! @{
//
//*****************************************************************************
#include <stdio.h>
#include <math.h>
#include "tmp006drv.h"
#include "i2c_if.h"
#include "uart_if.h"

//****************************************************************************
//                      GLOBAL VARIABLES                                   
//****************************************************************************

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
static int GetRegisterValue(unsigned char ucRegAddr, 
                            unsigned short *pusRegValue);
static double ComputeTemperature(double dVobject, double dTAmbient);


//****************************************************************************
//
//! Returns the value in the specified register
//!
//! \param ucRegAddr is the offset register address
//! \param pusRegValue is the pointer to the register value store
//! 
//! This function  
//!    1. Returns the value in the specified register
//!
//! \return 0: Success, < 0: Failure.
//
//****************************************************************************
int
GetRegisterValue(unsigned char ucRegAddr, unsigned short *pusRegValue)
{
    unsigned char ucRegData[2];
    //
    // Invoke the readfrom I2C API to get the required byte
    //
    if(I2C_IF_ReadFrom(TMP006_DEV_ADDR, &ucRegAddr, 1, 
                   &ucRegData[0], 2) != 0)
    {
        DBG_PRINT("TMP006 sensor I2C ReadFrom failed\n\r");
        return FAILURE;
    }

    *pusRegValue = (unsigned short)(ucRegData[0] << 8) | ucRegData[1];

    return SUCCESS;
}

//****************************************************************************
//
//! Initialize the temperature sensor
//!
//! \param None
//! 
//! This function  
//!    1. Get the device manufacturer and version
//!    2. Add any initialization here
//!
//! \return 0: Success, < 0: Failure.
//
//****************************************************************************
int 
TMP006DrvOpen()
{
    unsigned short usManufacID, usDevID, usConfigReg;

    //
    // Get the manufacturer ID
    //
    RET_IF_ERR(GetRegisterValue(TMP006_MANUFAC_ID_REG_ADDR, &usManufacID));
    DBG_PRINT("Manufacturer ID: 0x%x\n\r", usManufacID);
    if(usManufacID != TMP006_MANUFAC_ID)
    {
        DBG_PRINT("Error in Manufacturer ID\n\r");
        return FAILURE;
    }

    //
    // Get the device ID
    //
    RET_IF_ERR(GetRegisterValue(TMP006_DEVICE_ID_REG_ADDR, &usDevID));
    DBG_PRINT("Device ID: 0x%x\n\r", usDevID);
    if(usDevID != TMP006_DEVICE_ID)
    {
        DBG_PRINT("Error in Device ID\n");
        return FAILURE;
    }

    //
    // Get the configuration register value
    //
    RET_IF_ERR(GetRegisterValue(TMP006_CONFIG_REG_ADDR, &usConfigReg));
    DBG_PRINT("Configuration register value: 0x%x\n\r", usConfigReg);

    return SUCCESS;
}
//****************************************************************************
//
//! Compute the temperature value from the sensor voltage and die temp.
//!
//! \param dVobject is the sensor voltage value
//! \param dTAmbient is the local die temperature
//! 
//! This function  
//!    1. Computes the temperature from the VObject and TAmbient values
//!
//! \return 0: Success, < 0: Failure.
//
//****************************************************************************
double ComputeTemperature(double dVobject, double dTAmbient)
{
    //
    // This algo is obtained from 
    // http://processors.wiki.ti.com/index.php/SensorTag_User_Guide
    // #IR_Temperature_Sensor
    //
    double Tdie2 = dTAmbient + 273.15;
    const double S0 = 6.4E-14;            // Calibration factor
    const double a1 = 1.75E-3;
    const double a2 = -1.678E-5;
    const double b0 = -2.94E-5;
    const double b1 = -5.7E-7;
    const double b2 = 4.63E-9;
    const double c2 = 13.4;
    const double Tref = 298.15;
    double S = S0*(1+a1*(Tdie2 - Tref)+a2*pow((Tdie2 - Tref),2));
    double Vos = b0 + b1*(Tdie2 - Tref) + b2*pow((Tdie2 - Tref),2);
    double fObj = (dVobject - Vos) + c2*pow((dVobject - Vos),2);
    double tObj = pow(pow(Tdie2,4) + (fObj/S),.25);
    tObj = (tObj - 273.15);
    return tObj;
}

//****************************************************************************
//
//! Get the temperature value
//!
//! \param pfCurrTemp is the pointer to the temperature value store
//! 
//! This function  
//!    1. Get the sensor voltage reg and ambient temp reg values
//!    2. Compute the temperature from the read values
//!
//! \return 0: Success, < 0: Failure.
//
//****************************************************************************
int 
TMP006DrvGetTemp(float *pfCurrTemp)
{
    unsigned short usVObjectRaw, usTAmbientRaw;
    double dVObject, dTAmbient;
    //
    // Get the sensor voltage register value
    //
    RET_IF_ERR(GetRegisterValue(TMP006_VOBJECT_REG_ADDR, &usVObjectRaw));
    //
    // Get the ambient temperature register value
    //
    RET_IF_ERR(GetRegisterValue(TMP006_TAMBIENT_REG_ADDR, &usTAmbientRaw));
    //
    // Apply the format conversion
    //
    dVObject = ((short)usVObjectRaw) * 156.25e-9;
    dTAmbient = ((short)usTAmbientRaw) / 128;

    *pfCurrTemp = ComputeTemperature(dVObject, dTAmbient);
    
    //
    // Convert to Farenheit
    //
    *pfCurrTemp = ((*pfCurrTemp * 9) / 5) + 32;

    return SUCCESS;
}
//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
