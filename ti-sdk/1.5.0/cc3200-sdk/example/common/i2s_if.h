//*****************************************************************************
// mcasp_if.h
//
//  MCASP interface APIs.
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
// mcasp_if.h - Defines and prototypes for MultiChannel Audio Serial Port.
//
//*****************************************************************************

#ifndef __MCASP_IF_H__
#define __MCASP_IF_H__

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
  
#define WFIFO_NUMDMA_WORDS      1
#define MCASP_DMA_PORT_ADDR     0x4401E200

//
// Update the sampling freq, channels, sample length appropriately
//
typedef enum {
	SAMPLING_FREQ_8_KHZ = 8000,
	SAMPLING_FREQ_11_X_KHZ = 11025,
	SAMPLING_FREQ_16_KHZ = 16000,
    SAMPLING_FREQ_22_X_KHZ = 22050,
    SAMPLING_FREQ_32_KHZ = 32000,
	SAMPLING_FREQ_44_1_KHZ = 44100,
	SAMPLING_FREQ_48_KHZ = 48000
} tESamplFreq;

typedef enum {
	CH_MONO = 1,
	CH_STEREO = 2
} tENumChannels;

typedef enum {
	BIT_8_PCM = 8,
	BIT_16_PCM = 16,
	BIT_24_PCM = 24,
    BIT_32_PCM = 32
} tESampleLen;

#define I2S_MODE_RX		0x01
#define I2S_MODE_TX		0x02
#define I2S_MODE_RX_TX	0x03

//
// Audio Renderer Interface APIs
//
extern void AudioRendererInit();
extern void AudioCapturerInit();
extern void AudioInit();
extern void McASPInit();
extern void AudioRendererSetupCPUMode(void (*pfnAppCbHndlr)(void));
extern void AudioRendererSetupDMAMode(void (*pfnAppCbHndlr)(void), 
                                      unsigned long ulCallbackEvtSamples);
extern void AudioCapturerSetupCPUMode(void (*pfnAppCbHndlr)(void));
extern long AudioCapturerSetupDMAMode(void (*pfnAppCbHndlr)(void), 
                                      unsigned long ulCallbackEvtSamples);

extern long AudioSetupDMAMode(void (*pfnAppCbHndlr)(void),
                                      unsigned long ulCallbackEvtSamples,
                                      unsigned char RxTx);


extern unsigned int* AudioRendererGetDMADataPtr();
extern unsigned int* AudioCapturerGetDMADataPtr();
extern void AudioCapturerConfigure(int iSamplingFrequency,
                                   short sNumOfChannels,
                                   short sBitsPerSample);
extern void AudioRendererConfigure(int iSamplingFrequency,
                                   short sNumOfChannels,
                                   short sBitsPerSample);
extern void AudioRendererStart();
extern void AudioCapturerStart();
extern void AudioRendererStop();
extern void AudioRendererDeInit();
extern unsigned int BitClockConfigure(int iSamplingFrequency,
                                      short sNumOfChannels,
                                      short sBitsPerSample);

extern void AudioCaptureRendererConfigure(unsigned char bitsPerSample,
											unsigned short bitRate,
											unsigned char noOfChannels,
											unsigned char RxTx,
											unsigned char	dma);

extern void Audio_Start(unsigned char RxTx);
//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif //  __MCASP_IF_H__
