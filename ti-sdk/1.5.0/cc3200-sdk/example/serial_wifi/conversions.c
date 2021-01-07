//*****************************************************************************
// Copyright (C) 2014 Texas Instruments Incorporated
//
// All rights reserved. Property of Texas Instruments Incorporated.
// Restricted rights to use, duplicate or disclose this code are
// granted through contract.
// The program may not be used without the written permission of
// Texas Instruments Incorporated or against the terms and conditions
// stipulated in the agreement under which this program has been supplied,
// and under no circumstances can it be used with non-TI connectivity device.
//
//*****************************************************************************

#include <string.h>
#include <stdlib.h>
#include "serial_wifi.h"

unsigned char atoc(char data);
unsigned char atod(char data);
long atolong(char *data, unsigned long *retLong);
unsigned char ascii_to_char(char b1, char b2);

//*****************************************************************************
//
//! atoc
//!
//! @param  none
//!
//! @return hexadecimal equivalent
//!
//! @brief  Convert nibble to hexdecimal from ASCII
//
//*****************************************************************************
unsigned char atoc(char data)
{
    unsigned char ucRes;

    if ((data >= 0x30) && (data <= 0x39))
    {
        ucRes = data - 0x30;
    }
    else
    {
        if (data == 'a')
        {
            ucRes = 0x0a;;
        }
        else if (data == 'b')
        {
            ucRes = 0x0b;
        }
        else if (data == 'c')
        {
            ucRes = 0x0c;
        }
        else if (data == 'd')
        {
            ucRes = 0x0d;
        }
        else if (data == 'e')
        {
            ucRes = 0x0e;
        }
        else if (data == 'f')
        {
            ucRes = 0x0f;
        }
    }


    return ucRes;
}

//*****************************************************************************
//
//! atod
//!
//! \param  none
//!
//! \return Decimal value of ASCII char
//!
//! \brief  Convert ASCII char to decimal
//
//*****************************************************************************
unsigned char atod(char data)
{
    unsigned char retVal = 0xff;

    if ((data >= 0x30) && (data <= 0x39))
    {
        retVal = data - 0x30;
    }

    return retVal;
}

//*****************************************************************************
//
//! atolong
//!
//! \param  none
//!
//! \return Return long value else -1 as error
//!
//! \brief  Convert ASCII string to long
//
//*****************************************************************************
long atolong(char *data, unsigned long *retLong)
{
    unsigned char cycleCount = 0;
    unsigned char digit;

    if((data == NULL) || (retLong == NULL))
    {
        return (-1);
    }

    *retLong = 0;
    while ((digit = atod(*data)) != 0xff)
    {
        *retLong *= 10;
        *retLong += digit;
        data++;
        cycleCount++;
    }

    return cycleCount;
}

//*****************************************************************************
//
//! ascii_to_char
//!
//! @param  b1 first byte
//! @param  b2 second byte
//!
//! @return The converted character
//!
//! @brief  Convert 2 bytes in ASCII into one character
//
//*****************************************************************************

unsigned char ascii_to_char(char b1, char b2)
{
    unsigned char ucRes;

    ucRes = (atoc(b1)) << 4 | (atoc(b2));

    return ucRes;
}

//*****************************************************************************
//
//! htoa
//!
//! @param  none
//!
//! @return status
//!
//! @brief  Converts hexa string to ascii
//
//*****************************************************************************
int htoa()
{
    int i, len;
    char Byte[8];

    len = strlen(uBufRx.g_RecvBuf);

    //check data validity
    for (i = 0; i < len; i++)
    {
        if(((uBufRx.g_RecvBuf[i] >= 0x30) && (uBufRx.g_RecvBuf[i] <= 0x39)) || \
        ((uBufRx.g_RecvBuf[i] >= 0x41) && (uBufRx.g_RecvBuf[i] <= 0x46)) || \
        ((uBufRx.g_RecvBuf[i] >= 0x61) && (uBufRx.g_RecvBuf[i] <= 0x66)))
        {
            continue;
        }
        else
        {
            return -1;
        }
    }

    for (i = 0; i < (len / 2); i++)
    {
        strncpy(Byte, &uBufRx.g_RecvBuf[i * 2], 2);
        uBufRx.g_RecvBuf[i] = strtol(Byte, NULL, 16);
    }

    return 0;
}

//*****************************************************************************
//
//! btoa
//!
//! @param  none
//!
//! @return status
//!
//! @brief  Converts binary string to ascii
//
//*****************************************************************************
int btoa()
{
    int i, len;
    char Byte[8];

    len = strlen(uBufRx.g_RecvBuf);

    //check data validity
    for (i = 0; i < len; i++)
    {
        if ((uBufRx.g_RecvBuf[i] != 0x30) && (uBufRx.g_RecvBuf[i] != 0x31))
        {
            return -1;
        }
    }

    for (i = 0; i < (len / 8); i++)
    {
        strncpy(Byte, &uBufRx.g_RecvBuf[i * 8], 8);
        uBufRx.g_RecvBuf[i] = strtol(Byte, NULL, 2);
    }

    return 0;
}
