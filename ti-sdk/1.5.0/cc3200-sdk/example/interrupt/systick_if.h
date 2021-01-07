//*****************************************************************************
// systick_if.h 
//
// Systick interface header file
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

#ifndef __UT_UTILS_H__
#define __UT_UTILS_H__

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// Macros to convert back and forward between ticks and real time.
//
//*****************************************************************************

  //*************** clock frequency 80Mhz   ******************//
#define MAP_SysCtlClockGet 80000000
extern unsigned long g_ulClockRate;
#define MILLISECONDS_TO_TICKS(ms) ((g_ulClockRate / 1000) * (ms))
#define MICROSECONDS_TO_TICKS(us) ((g_ulClockRate / 1000000) * (us))
#define TICKS_TO_MILLISECONDS(ticks)                                          \
    (((ticks) / g_ulClockRate) * 1000 +                                       \
     ((((ticks)%g_ulClockRate) * 1000) / g_ulClockRate))

//*****************************************************************************
//
// The largest value of ulTicks%g_ulClockRate that can be converted to
// milliseconds using the macro above. Values larger than this will cause
// overflow and result in incorrect results. This value is (2^32)/1000.
//
//*****************************************************************************
#define MAX_TICKS_FOR_MS_CONVERSION 4294967

//*****************************************************************************
//
// Test result. This enum is used to pass the final  result from each
// case to the framework which will then report the result to the host.
// TEST_SKIPPED may be used by the testcase if it is not possible to run the
// test given the parameters provided, for example a test which requires
// user interaction and has no automatic alternative may report TEST_SKIPPED
// if passed tTestParams->bInteractive set to FALSE.
//
//*****************************************************************************
typedef enum
{
    TEST_PASSED,
    TEST_FAILED,
    TEST_SKIPPED
}
tTestResult;

//*****************************************************************************
//
// Function prototypes
//
//*****************************************************************************
extern tBoolean SysTickInit(void);

extern unsigned long UTUtilsGetSysTime(void);

extern tBoolean UTUtilsWaitForCount(volatile unsigned long *pulCount,
                                    unsigned long ulTarget,
                                    unsigned long ulTimeoutMs);
extern void SysCtlDelay(unsigned long ulCount);
extern void UTUtilsDelay(unsigned long ulTimeoutMicroS);
extern void UTUtilsDelayCycles(unsigned long ulTimeoutCycles);
extern unsigned long UTUtilsSysTickValueGet(void);

#ifdef __cplusplus
}
#endif

#endif // __UT_UTILS_H__
