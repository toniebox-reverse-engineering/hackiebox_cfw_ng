//*****************************************************************************
// circ_buffer.h
//
// APIs for Implementing circular buffer
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

#ifndef __CIRCULAR_BUFFER_API_H__
#define __CIRCULAR_BUFFER_API_H__

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
  
/****************************************************************************/
/*				TYPEDEFS										*/
/****************************************************************************/
typedef struct CircularBuffer
{
    unsigned char *pucReadPtr;
    unsigned char *pucWritePtr;
    unsigned char *pucBufferStartPtr;
    unsigned long ulBufferSize;
    unsigned char *pucBufferEndPtr;
}tCircularBuffer;

#ifndef TRUE
#define TRUE                    1
#endif

#ifndef FALSE
#define FALSE                   0
#endif

//*****************************************************************************
//
// Define a boolean type, and values for true and false.
//
//*****************************************************************************
typedef unsigned int tboolean;

/****************************************************************************/
/*		        FUNCTION PROTOTYPES							*/
/****************************************************************************/
extern tCircularBuffer* CreateCircularBuffer(unsigned long ulBufferSize);
extern void DestroyCircularBuffer(tCircularBuffer *pCircularBuffer);
extern unsigned char* GetReadPtr(tCircularBuffer *pCircularBuffer);
extern unsigned char* GetWritePtr(tCircularBuffer *pCircularBuffer);
extern long FillBuffer(tCircularBuffer *pCircularBuffer,
                       unsigned char *pucBuffer, unsigned int uiBufferSize);
extern void UpdateReadPtr(tCircularBuffer *pBuffer, unsigned int uiDataSize);
extern void UpdateWritePtr(tCircularBuffer *pCircularBuffer,
                           unsigned int uiPacketSize);
extern long ReadBuffer(tCircularBuffer *pCircularBuffer,
                       unsigned char *pucBuffer, unsigned int uiDataSize);
extern long FillZeroes(tCircularBuffer *pCircularBuffer,
					   unsigned int uiPacketSize);
extern unsigned int GetBufferSize(tCircularBuffer *pCircularBuffer);
extern unsigned int GetBufferEmptySize(tCircularBuffer *pCircularBuffer);
extern tboolean IsBufferEmpty(tCircularBuffer *pCircularbuffer);
extern tboolean IsBufferSizeFilled(tCircularBuffer *pCircularBuffer,
                             unsigned long ulThresholdHigh);
extern tboolean IsBufferVacant(tCircularBuffer *pCircularBuffer,
                               unsigned long ulThresholdLow);

#ifdef __cplusplus
}
#endif

#endif // __CIRCULAR_BUFFER_API_H__

