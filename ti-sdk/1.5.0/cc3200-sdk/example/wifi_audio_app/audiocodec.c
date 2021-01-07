//*****************************************************************************
// audiocodec.c
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

#include <stdint.h>
#include <stdbool.h>
#include "hw_types.h"
#include "i2c.h"
#include "hw_memmap.h"
#include "hw_ints.h"
#include "hw_i2c.h"
#include "rom.h"
#include "rom_map.h"
#include "stdlib.h"
#include "prcm.h"
#include "string.h"
#include "hw_common_reg.h"
#include "utils.h"
#include "pin.h"
#include "i2c_if.h"
#include "uart_if.h"
#include "audiocodec.h"
#include "ti3254.h"

// User define


static unsigned char val[2],I,J=0;

static unsigned long AudioCodecPageSelect(unsigned char ulPageAddress);
static unsigned long AudioCodecRegWrite(unsigned char ulRegAddr,unsigned char ucRegValue);


//*****************************************************************************
//
//! Reset or initialize code
//!
//! \param[in] codecId 	- Device id that need to reset/init
//! \param[in] arg 		- Pointer to user define structure/data type else NULL
//!
//! \return 0 on success else -ve.
//
//*****************************************************************************
int AudioCodecReset(unsigned char codecId, void *arg)
{
    if(codecId == AUDIO_CODEC_TI_3254)
    {

        //
        // Select page 0
        //
        AudioCodecPageSelect(TI3254_PAGE_0);

        //
        // Soft RESET
        //
        AudioCodecRegWrite(TI3254_SW_RESET_REG, 0x01);
    }

    return 0;
}

//*****************************************************************************
//
//! Configure audio codec for smaple rate, bits and number of channels
//!
//! \param[in] codecId 			- Device id
//! \param[in] bitsPerSample 	- Bits per sample (8, 16, 24 etc..)
//!									Please ref Bits per sample Macro section
//! \param[in] bitRate 			- Sampling rate in Hz. (8000, 16000, 44100 etc..)
//! \param[in] noOfChannels 	- Number of channels. (Mono, stereo etc..)
//!									Please refer Number of Channels Macro section
//! \param[in] speaker 			- Audio out that need to configure. (headphone, line out, all etc..)
//!									Please refer Audio Out Macro section
//! \param[in] mic 				- Audio in that need to configure. (line in, mono mic, all etc..)
//!									Please refer Audio In Macro section
//!
//! \return 0 on success else -ve.
//
//*****************************************************************************
int AudioCodecConfig(unsigned char codecId, unsigned char bitsPerSample, unsigned short bitRate,
                      unsigned char noOfChannels, unsigned char speaker,  unsigned char mic)
{
    unsigned int	bitClk = 0;

    if(codecId == AUDIO_CODEC_TI_3254)
    {
        AudioCodecPageSelect(TI3254_PAGE_0);

        if(bitsPerSample == AUDIO_CODEC_16_BIT)
        {
            // Set I2S Mode and Word Length
            AudioCodecRegWrite(TI3254_AUDIO_IF_1_REG, 0x00); 	// 0x00 	16bit, I2S, BCLK is input to the device
                                                                // WCLK is input to the device,
        }
        else
        {
            return -1;
        }

        bitClk = bitsPerSample * bitRate * noOfChannels;
        if(bitClk == 512000)
        {
            AudioCodecPageSelect(TI3254_PAGE_0);

            AudioCodecRegWrite(TI3254_CLK_MUX_REG, 0x03);		// PLL Clock is CODEC_CLKIN
            AudioCodecRegWrite(TI3254_CLK_PLL_P_R_REG, 0x94);	// PLL is powered up, P=1, R=4
            AudioCodecRegWrite(TI3254_CLK_PLL_J_REG, 0x2A);		// J=42
            AudioCodecRegWrite(TI3254_CLK_PLL_D_MSB_REG, 0x00);	// D = 0

            AudioCodecRegWrite(TI3254_CLK_NDAC_REG, 0x8E);		// NDAC divider powered up, NDAC = 14
            AudioCodecRegWrite(TI3254_CLK_MDAC_REG, 0x81);		// MDAC divider powered up, MDAC = 1
            AudioCodecRegWrite(TI3254_DAC_OSR_MSB_REG, 0x01);	// DOSR = 0x0180 = 384
            AudioCodecRegWrite(TI3254_DAC_OSR_LSB_REG, 0x80);	// DOSR = 0x0180 = 384

            AudioCodecRegWrite(TI3254_CLK_NADC_REG, 0x95);    	// NADC divider powered up, NADC = 21
            AudioCodecRegWrite(TI3254_CLK_MADC_REG, 0x82);      // MADC divider powered up, MADC = 2
            AudioCodecRegWrite(TI3254_ADC_OSR_REG, 0x80);    	// AOSR = 128 ((Use with PRB_R1 to PRB_R6, ADC Filter Type A)
        }
        else
        {
            return -1;
        }


        // Configure Power Supplies
        AudioCodecPageSelect(TI3254_PAGE_1);		//Select Page 1

        AudioCodecRegWrite(TI3254_PWR_CTRL_REG, 0x08);	// Disabled weak connection of AVDD with DVDD
        AudioCodecRegWrite(TI3254_LDO_CTRL_REG, 0x01);	// Over Current detected for AVDD LDO
        AudioCodecRegWrite(TI3254_ANALOG_IP_QCHRG_CTRL_REG, 0x32); // Analog inputs power up time is 6.4 ms
        AudioCodecRegWrite(TI3254_REF_PWR_UP_CTRL_REG, 0x01);	// Reference will power up in 40ms when analog blocks are powered up


        if(speaker)
        {
            unsigned char	reg1;

            AudioCodecPageSelect(TI3254_PAGE_0);	//Select Page 0


            // ##Configure Processing Blocks
            AudioCodecRegWrite(TI3254_DAC_SIG_P_BLK_CTRL_REG, 0x2);  // DAC Signal Processing Block PRB_P2


            AudioCodecPageSelect(TI3254_PAGE_44);	// Select Page 44

            AudioCodecRegWrite(TI3254_DAC_ADP_FILTER_CTRL_REG, 0x04);   // Adaptive Filtering enabled for DAC


            AudioCodecPageSelect(TI3254_PAGE_1);	// Select Page 1

            reg1 = 0x00;

            if(speaker & AUDIO_CODEC_SPEAKER_HP)
            {
                //De-pop: 5 time constants, 6k resistance
                AudioCodecRegWrite(TI3254_HP_DRV_START_UP_CTRL_REG, 0x25);	// Headphone ramps power up time is determined with 6k resistance,
                                                                            // Headphone ramps power up slowly in 5.0 time constants

                //Route LDAC/RDAC to HPL/HPR
                AudioCodecRegWrite(TI3254_HPL_ROUTING_SEL_REG, 0x08);	// Left Channel DAC reconstruction filter's positive terminal is routed to HPL
                AudioCodecRegWrite(TI3254_HPR_ROUTING_SEL_REG, 0x08);	// Left Channel DAC reconstruction filter's negative terminal is routed to HPR

                reg1 |= 0x30;	// HPL and HPR is powered up
            }

            if(speaker & AUDIO_CODEC_SPEAKER_LO)
            {
                //Route LDAC/RDAC to LOL/LOR
                AudioCodecRegWrite(TI3254_LOL_ROUTING_SEL_REG, 0x08);	// Left Channel DAC reconstruction filter output is routed to LOL
                AudioCodecRegWrite(TI3254_LOR_ROUTING_SEL_REG, 0x08);	// Right Channel DAC reconstruction filter output is routed to LOR

                reg1 |= 0x0C;	// LOL and LOR is powered up
            }

            //Power up HPL/HPR and LOL/LOR drivers
            AudioCodecRegWrite(TI3254_OP_DRV_PWR_CTRL_REG, reg1);

            if(speaker & AUDIO_CODEC_SPEAKER_HP)
            {
                //Unmute HPL/HPR driver, 0dB Gain
                AudioCodecRegWrite(TI3254_HPL_DRV_GAIN_CTRL_REG, 0x00);		// HPL driver is not muted, HPL driver gain is 0dB
                AudioCodecRegWrite(TI3254_HPR_DRV_GAIN_CTRL_REG, 0x00);		// HPR driver is not muted, HPL driver gain is 0dB
            }

            if(speaker & AUDIO_CODEC_SPEAKER_HP)
            {
                //Unmute LOL/LOR driver, 0dB Gain
                AudioCodecRegWrite(TI3254_LOL_DRV_GAIN_CTRL_REG, 0x0E);	// LOL driver gain is 11dB
                AudioCodecRegWrite(TI3254_LOR_DRV_GAIN_CTRL_REG, 0x0E);	// LOL driver gain is 11dB
            }



            AudioCodecPageSelect(TI3254_PAGE_0);		//Select Page 0

            //DAC => 64dB
            AudioCodecRegWrite(TI3254_LEFT_DAC_VOL_CTRL_REG, 0x80);		// Digital Volume Control = 64.0dB silent Note: As per data sheet its reserved but on setting this value there is silent
            AudioCodecRegWrite(TI3254_RIGHT_DAC_VOL_CTRL_REG, 0x80);	// Digital Volume Control = 64.0dB silent Note: As per data sheet its reserved  but on setting this value there is silent


            AudioCodecPageSelect(TI3254_PAGE_0);		//Select Page 0

            //Power up LDAC/RDAC
            AudioCodecRegWrite(TI3254_DAC_CHANNEL_SETUP_1_REG, 0xD6);	// Left and Right DAC Channel Powered Up
            // Left DAC data Left Channel Audio Interface Data
            // Right DAC data is Left Channel Audio Interface Data
            // Soft-Stepping is disabled

            //Unmute LDAC/RDAC
            AudioCodecRegWrite(TI3254_DAC_CHANNEL_SETUP_2_REG, 0x00);	// When Right DAC Channel is powered down, the data is zero.
            // Auto Mute disabled
            // Left and Right DAC Channel not muted
            // Left and Right Channel have independent volume control
        }


        if(mic)
        {
            unsigned char reg1 = 0x00;	// TI3254_MICBIAS_CTRL_REG
            unsigned char reg2 = 0x00;	// TI3254_LEFT_MICPGA_P_CTRL_REG
            unsigned char reg3 = 0x00;	// TI3254_LEFT_MICPGA_N_CTRL_REG
            unsigned char reg4 = 0x00;	// TI3254_RIGHT_MICPGA_P_CTRL_REG
            unsigned char reg5 = 0x00;	// TI3254_RIGHT_MICPGA_N_CTRL_REG
            unsigned char reg6 = 0x00;	// TI3254_FLOAT_IP_CTRL_REG


            AudioCodecPageSelect(TI3254_PAGE_8);	// Select Page 8

            AudioCodecRegWrite(TI3254_ADC_ADP_FILTER_CTRL_REG, 0x04);   // Adaptive Filtering enabled for ADC


            AudioCodecPageSelect(TI3254_PAGE_0);		//Select Page 0

            AudioCodecRegWrite(TI3254_ADC_SIG_P_BLK_CTRL_REG, 0x2);	// ADC Signal Processing Block PRB_P2

            if(mic & AUDIO_CODEC_MIC_LINE_IN)
            {
                reg1 |= 0x40;	// MICBIAS powered up
                reg2 |= 0x40;	// IN1L is routed to Left MICPGA with 10k resistance
                reg3 |= 0x40;	// CM is routed to Left MICPGA via CM1L with 10k resistance
                reg4 |= 0x40;	// IN1R is routed to Right MICPGA with 10k resistance
                reg5 |= 0x40;	// CM is routed to Right MICPGA via CM1R with 10k resistance
                reg6 |= 0xC0;	// IN1L input is weakly driven to common mode. Use when not routing IN1L to Left and Right MICPGA and HPL, HPR
            }

            if(mic & AUDIO_CODEC_MIC_MONO)
            {
                reg1 |= 0x40;	// MICBIAS powered up
                reg2 |= 0x00;
                reg3 |= 0x10;
                reg4 |= 0x10;	// IN2R is routed to Right MICPGA with 10k resistance
                reg5 |= 0x40;	// CM is routed to Right MICPGA via CM1R with 10k resistance
                reg6 |= 0x10;	// IN2R input is weakly driven to common mode. Use when not routing IN2R to Left and Right MICPGA
            }

            if(mic & AUDIO_CODEC_MIC_ONBOARD)
            {
                reg1 |= 0x40;	// MICBIAS powered up
                reg2 |= 0x00;
                reg3 |= 0x04;
                reg4 |= 0x04;	// IN3R is routed to Right MICPGA with 10k resistance
                reg5 |= 0x40;	// CM is routed to Right MICPGA via CM1R with 10k resistance
                reg6 |= 0x04;	// IN3R input is weakly driven to common mode. Use when not routing IN3R to Left and Right MICPGA
            }

            AudioCodecPageSelect(TI3254_PAGE_1);     //Select Page 1

            AudioCodecRegWrite(TI3254_MICBIAS_CTRL_REG, reg1);

            //Route IN2L not routed
            AudioCodecRegWrite(TI3254_LEFT_MICPGA_P_CTRL_REG, reg2);

            //Route IN2R CM1L to LEFT_N with 10K input impedance
            AudioCodecRegWrite(TI3254_LEFT_MICPGA_N_CTRL_REG, reg3);

            //Route IN2R to RIGHT_P with 10K input impedance
            AudioCodecRegWrite(TI3254_RIGHT_MICPGA_P_CTRL_REG, reg4);

            //Route CM1R to RIGHT_M with 10K input impedance
            AudioCodecRegWrite(TI3254_RIGHT_MICPGA_N_CTRL_REG, reg5);

            AudioCodecRegWrite(TI3254_FLOAT_IP_CTRL_REG, reg6);


            //make channel gain 0dB, since 20K input
            //impedance is used single ended
            AudioCodecRegWrite(TI3254_LEFT_MICPGA_VOL_CTRL_REG, 0x00);	// 0.0dB

            //Unmute Right MICPGA, Gain selection of 6dB to
            //make channel gain 0dB, since 20K input
            //impedance is used single ended
            AudioCodecRegWrite(TI3254_RIGHT_MICPGA_VOL_CTRL_REG, 0x00);	// 0.0dB


            AudioCodecRegWrite(TI3254_LEFT_ADC_VOL_CTRL_REG, 0x68);		// -12dB
            AudioCodecRegWrite(TI3254_RIGHT_ADC_VOL_CTRL_REG, 0x68);	// -12dB



            AudioCodecPageSelect(TI3254_PAGE_0);	// Select Page 0

            //Power up LADC/RADC
            AudioCodecRegWrite(TI3254_ADC_CHANNEL_SETUP_REG, 0xC0);	// Left and Right Channel ADC is powered up

            //Unmute LADC/RADC
            AudioCodecRegWrite(TI3254_ADC_FINE_GAIN_ADJ_REG, 0x00);	// Left and Right ADC Channel Un-muted. Left and Right ADC Channel Fine Gain = 0dB,
        }

    }

    return 0;
}

//*****************************************************************************
//
//! Configure volume level for specific audio out on a codec device
//!
//! \param[in] codecId 		- Device id
//! \param[in] speaker	 	- Audio out id. (headphone, line out, all etc..)
//!								Please refer Audio out Macro section
//! \param[in] volumeLevel 	-  Volume level. 0-100
//!
//! \return 0 on success else -ve.
//
//*****************************************************************************
int AudioCodecSpeakerVolCtrl(unsigned char codecId, unsigned char speaker, signed char volumeLevel)
{
    short  vol = 0;

    if(volumeLevel < 4)
    {
        vol = 128;
    }
    else if (volumeLevel > 97)
    {
        vol = 316;
    }
    else
    {
        vol = 122 + (volumeLevel << 1);
    }

    Report("Volume: %d reg: %d %d\n\r", volumeLevel, vol, (unsigned char )(vol&0x00FF));

    AudioCodecPageSelect(TI3254_PAGE_0);
    AudioCodecRegWrite(TI3254_LEFT_DAC_VOL_CTRL_REG, (unsigned char )(vol&0x00FF));
    AudioCodecRegWrite(TI3254_RIGHT_DAC_VOL_CTRL_REG, (unsigned char )(vol&0x00FF));
    return 0;
}

//*****************************************************************************
//
//! Mute Audio line out
//!
//! \param[in] codecId 		- Device id
//! \param[in] speaker	 	- Audio out id. (headphone, line out, all etc..)
//!								Please refer Audio out Macro section
//!
//! \return 0 on success else -ve.
//
//*****************************************************************************
int AudioCodecSpeakerMute(unsigned char codecId, unsigned char speaker)
{
    Report("Speaker Mute\n\r");
    AudioCodecPageSelect(TI3254_PAGE_0);

    //Unmute LDAC/RDAC
    AudioCodecRegWrite(TI3254_DAC_CHANNEL_SETUP_2_REG, 0x0C);	// Left and Right DAC Channel muted
    return 0;
}

//*****************************************************************************
//
//! Unmute audio line out
//!
//! \param[in] codecId 		- Device id
//! \param[in] speaker	 	- Audio out id. (headphone, line out, all etc..)
//!								Please refer Audio out Macro section
//!
//! \return 0 on success else -ve.
//
//*****************************************************************************
int AudioCodecSpeakerUnmute(unsigned char codecId, unsigned char speaker)
{
    Report("Speaker Unmute\n\r");

    AudioCodecPageSelect(TI3254_PAGE_0);

    //Unmute LDAC/RDAC
    AudioCodecRegWrite(TI3254_DAC_CHANNEL_SETUP_2_REG, 0x00);	// Left and Right DAC Channel not muted
    return 0;
}

//*****************************************************************************
//
//! Configure volume level for specific audio in on a codec device
//!
//! \param[in] codecId 		- Device id
//! \param[in] mic		 	- Audio in id. (line in, mono mic, all etc..)
//!								Please refer Audio In Macro section
//! \param[in] volumeLevel 	- Volume level (0 - 100)
//!
//! \return 0 on success else -ve.
//
//*****************************************************************************
int AudioCodecMicVolCtrl(unsigned char codecId, unsigned char mic, signed char volumeLevel)
{
    static unsigned char vol = 0x00;

    //Note: Volume level 0 will not mute the ADC

    if(volumeLevel < 2)
    {
        vol = 104;
    }
    else
    {
        vol = 103 + (volumeLevel >> 1);
    }


    Report("Volume: %d reg: %d %x\n\r", volumeLevel, vol, (unsigned char )(vol&0x7F));

    AudioCodecPageSelect(TI3254_PAGE_0);	// Select Page 0


    //Unmute LADC/RADC
    AudioCodecRegWrite(TI3254_LEFT_ADC_VOL_CTRL_REG, (unsigned char )(vol&0x7F));
    AudioCodecRegWrite(TI3254_RIGHT_ADC_VOL_CTRL_REG, (unsigned char )(vol&0x7F));
    return 0;
}

//*****************************************************************************
//
//! Mute Audio line in
//!
//! \param[in] codecId 		- Device id
//! \param[in] mic		 	- Audio in id. (line in, mono mic, all etc..)
//!								Please refer Audio In Macro section
//!
//! \return 0 on success else -ve.
//
//*****************************************************************************
int AudioCodecMicMute(unsigned char codecId, unsigned char mic)
{
    Report("Mic Mute\n\r");

    AudioCodecPageSelect(TI3254_PAGE_0);	// Select Page 0

    //Unmute LADC/RADC
    AudioCodecRegWrite(TI3254_ADC_FINE_GAIN_ADJ_REG, 0x88);	// Left and Right ADC Channel Un-muted. Left and Right ADC Channel Fine Gain = 0dB,
    return 0;
}

//*****************************************************************************
//
//! Unmute audio line
//!
//! \param[in] codecId	- Device id
//! \param[in] mic	 	- Audio in id. (line in, mono mic, all etc..)
//!							Please refer Audio In Macro section
//!
//! \return 0 on success else -ve.
//
//*****************************************************************************
int AudioCodecMicUnmute(unsigned char codecId, unsigned char mic)
{
    Report("Mic unmute\n\r");
    AudioCodecPageSelect(TI3254_PAGE_0);	// Select Page 0

    //Unmute LADC/RADC
    AudioCodecRegWrite(TI3254_ADC_FINE_GAIN_ADJ_REG, 0x00);	// Left and Right ADC Channel Un-muted. Left and Right ADC Channel Fine Gain = 0dB,
    return 0;
}

//*****************************************************************************
//
//! Select Codec page that need to configure
//!
//! \param[in] ulPageAddress	- page id
//!
//! \return 0 on success else -ve.
//
//*****************************************************************************
static unsigned long AudioCodecPageSelect(unsigned char ulPageAddress)
{
    return AudioCodecRegWrite(TI3254_PAGE_SEL_REG,ulPageAddress);
}


//******************************************************************************
//
// Writes to specfied register
// ulRegAddr - Register Address
// ucRegValue - 8 bit Register Value
//
//******************************************************************************
static unsigned long AudioCodecRegWrite(unsigned char ulRegAddr,unsigned char ucRegValue)
{
    unsigned char ucData[2];
    ucData[0] = (unsigned char)ulRegAddr;
    ucData[1] = ucRegValue;
    val[1]=ucData[1];
#if 0
    if(I2C_IF_ReadFrom(0x44,&ucRegAddr, 1, &ucRegData[0], 2) != 0)
        return -1;
#endif
    J=I2C_IF_Write(CODEC_I2C_SLAVE_ADDR, ucData, 2, 1);
    if(J !=0)
        return (1U<<31);
    MAP_UtilsDelay(27000);

    J=I2C_IF_ReadFrom(CODEC_I2C_SLAVE_ADDR, &ucData[0], 1,&val[0],1);
    if(J !=0)
        return (1U<<31);

    if(val[0] != val[1])
        ++I;

    return 0;
}


