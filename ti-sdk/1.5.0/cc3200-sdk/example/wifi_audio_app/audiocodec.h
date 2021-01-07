//*****************************************************************************
// audiocodec.h
//
// Interface for Audio codec
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

#ifndef AUDIOCODEC_H_
#define AUDIOCODEC_H_

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

// Define 8-bit codec id to identify codec device
#define AUDIO_CODEC_TI_3254		0
#define AUDIO_CODEC_TI_3110		1


// Bits per sample Macro
#define AUDIO_CODEC_8_BIT		8
#define AUDIO_CODEC_16_BIT		16
#define AUDIO_CODEC_20_BIT		20
#define AUDIO_CODEC_24_BIT		24



// Number of Channels Macro
#define AUDIO_CODEC_DEFAULT			0
#define AUDIO_CODEC_MONO			1
#define AUDIO_CODEC_STEREO			2
#define AUDIO_CODEC_REVERSE_MONO	3
#define AUDIO_CODEC_REVERSE_STEREO	4

// Audio out Macro
// Note: Max 8 line out per codec device supported
#define AUDIO_CODEC_SPEAKER_NONE		(0x00)
#define AUDIO_CODEC_SPEAKER_HP			(0x01)		// Headphone line
#define AUDIO_CODEC_SPEAKER_LO			(0x02)		// Line out
#define AUDIO_CODEC_SPEAKER_RESERVED1	(0x04)		// Onboard mic device
#define AUDIO_CODEC_SPEAKER_RESERVED2	(0x08)		// Onboard mic device
#define AUDIO_CODEC_SPEAKER_RESERVED3	(0x10)		// Onboard mic device
#define AUDIO_CODEC_SPEAKER_RESERVED4	(0x20)		// Onboard mic device
#define AUDIO_CODEC_SPEAKER_RESERVED5	(0x40)		// Onboard mic device
#define AUDIO_CODEC_SPEAKER_RESERVED6	(0x80)		// Onboard mic device
#define AUDIO_CODEC_SPEAKER_ALL			(0xFF)

// Audio in macro.
// Max 8 line in per codec device supported
#define AUDIO_CODEC_MIC_NONE		(0x00)
#define AUDIO_CODEC_MIC_MONO		(0x01)		// Mono mic
#define AUDIO_CODEC_MIC_LINE_IN		(0x02)		// Line in
#define AUDIO_CODEC_MIC_ONBOARD		(0x04)		// Onboard mic device
#define AUDIO_CODEC_MIC_RESERVED2	(0x08)		// Mono mic
#define AUDIO_CODEC_MIC_RESERVED3	(0x10)		// Line in
#define AUDIO_CODEC_MIC_RESERVED4	(0x20)		// Onboard mic device
#define AUDIO_CODEC_MIC_RESERVED5	(0x40)		// Mono mic
#define AUDIO_CODEC_MIC_RESERVED6	(0x80)		// Line in
#define AUDIO_CODEC_MIC_ALL			(0xFF)


/*
 * 	Reset or initialize code
 * 	codecId:	Device id that need to reset/init
 * 	arg:		Pointer to user define structure/data type else NULL
 *
 */
int AudioCodecReset(unsigned char codecId, void *arg);

/*
 *	Configure audio codec for smaple rate, bits and number of channels
 *	codecId:	Device id
 *
 *	bitsPerSample:	Bits per sample (8, 16, 24 etc..)
 *						Please ref Bits per sample Macro section
 *	bitsRate:		Sampling rate in Hz. (8000, 16000, 44100 etc..)
 *	noOfChannels:	Number of channels. (Mono, stereo etc..)
 						Please refer Number of Channels Macro section
 *	speaker:		Audio out that need to configure. (headphone, line out, all etc..)
 *						Please refer Audio Out Macro section
 *	mic:			Audio in that need to configure. (line in, mono mic, all etc..)
 *						Please refer Audio In Macro section
 *
 */
int AudioCodecConfig(unsigned char codecId, unsigned char bitsPerSample, unsigned short bitRate,
						unsigned char noOfChannels, unsigned char speaker,  unsigned char mic);

/*
 * 	Configure volume level for specific audio out on a codec device
 * 	codecId:	Device id
 * 	speaker:	Audio out id. (headphone, line out, all etc..)
 *					Please refer Audio out Macro section
 *	volumeLevel: Volume level. 0-100
 *
 */
int AudioCodecSpeakerVolCtrl(unsigned char codecId, unsigned char speaker, signed char volumeLevel);


/*
 * 	Mute Audio line out
 * 	codecId:	Device id
 * 	speaker:	Audio out id. (headphone, line out, all etc..)
 *					Please refer Audio out Macro section
 *
 */
int AudioCodecSpeakerMute(unsigned char codecId, unsigned char speaker);

/*
 * 	Unmute audio line out
 * 	codecId:	Device id
 * 	speaker:	Audio out id. (headphone, line out, all etc..)
 *					Please refer Audio out Macro section
 *
 */
int AudioCodecSpeakerUnmute(unsigned char codecId, unsigned char speaker);

/*
 * 	Configure volume level for specific audio in on a codec device
 * 	codecId:	Device id
 * 	mic:		Audio in id. (line in, mono mic, all etc..)
 *					Please refer Audio In Macro section
 *	volumeLevel: Volume level 0-100
 *
 */
int AudioCodecMicVolCtrl(unsigned char codecId, unsigned char mic, signed char volumeLevel);

/*
 * 	Mute Audio line in
 * 	codecId:	Device id
 * 	mic:		Audio in id. (line in, mono mic, all etc..)
 *					Please refer Audio In Macro section
 *
 */
int AudioCodecMicMute(unsigned char codecId, unsigned char mic);

/*
 * 	Unmute audio line
 * 	codecId:	Device id
 * 	mic:		Audio in id. (line in, mono mic, all etc..)
 *					Please refer Audio In Macro section
 *
 */
int AudioCodecMicUnmute(unsigned char codecId, unsigned char mic);

#ifdef __cplusplus
}
#endif

#endif /* AUDIOCODEC_H_ */
