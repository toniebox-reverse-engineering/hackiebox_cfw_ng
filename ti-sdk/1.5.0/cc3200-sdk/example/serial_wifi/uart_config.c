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

//Application includes
#include "serial_wifi.h"

//Common interface includes
#include "network_if.h"
#include "common.h"

//*****************************************************************************
//
//! \addtogroup serial_wifi
//! @{
//
//*****************************************************************************

//****************************************************************************
//                      LOCAL FUNCTION PROTOTYPES
//****************************************************************************
void PrintCommandMenu();
void basic_Interpreter(void *pvParameters);
unsigned int GETChar(unsigned char *ucBuffer);
//
//*****************************************************************************
//
//! PrintCommandMenu
//!
//!  \param  None
//!
//!  \return none
//!
//!  \brief   Prints Help menu with info on current commands
//
//*****************************************************************************
void PrintCommandMenu()
{
    UART_PRINT("\r\n=============================================================================");
    UART_PRINT("\r\n==                     SimpleLink Serial Wi-Fi Mode                        ==");
    UART_PRINT("\r\n=============================================================================");
    UART_PRINT("\r\nhelp                                          Show this menu.                    ");
    UART_PRINT("\r\nwlan_connect [ssid] [type] [Sec] [User] [key]   Connect to wlan AP.            ");
    UART_PRINT("\r\n                                              ssid:Name of the AP                ");
    UART_PRINT("\r\n                                              type: 0-Personal,1-ENT             ");
    UART_PRINT("\r\n                                              Sec: 0-OPEN,1-WEP,2-WPA            ");
    UART_PRINT("\r\n                                              User: user name in case of ENT     ");
    UART_PRINT("\r\n                                              Key: password in case of WEP and WPA");
    UART_PRINT("\r\nconnect [IP | name] [port] [TCP | UDP]        Connect to specified IP or named   ");
    UART_PRINT("\r\n                                              server; with an optionally         ");
    UART_PRINT("\r\n                                              specified port. If not specified,  ");
    UART_PRINT("\r\n                                              default port is used.              ");
    UART_PRINT("\r\n                                              Can optionally set transport       ");
    UART_PRINT("\r\n                                              protocol explicitly. If IP/name not");
    UART_PRINT("\r\n                                              specified use UDP as default       ");
    UART_PRINT("\r\n                                              transport; otherwise, TCP.         ");
    UART_PRINT("\r\ndisconnect                                    Disconnect connected end point.    ");
    UART_PRINT("\r\n\\\\<                                             Enter local control mode.          ");
    UART_PRINT("\r\n\\\\>                                             Enter remote control mode.         ");
    UART_PRINT("\r\nexit                                          Terminate control mode, and resume ");
    UART_PRINT("\r\n                                              interpreter mode operation.        ");
    UART_PRINT("\r\nquit                                          Terminate interpreter mode, and    ");
    UART_PRINT("\r\n                                              free up all related resources.     ");
    //UART_PRINT("\r\nmode [A | B | H]                         Set data interpretation mode:      ");
    //UART_PRINT("\r\n                                         A - ASCII; B - Binary; H - Hex.    ");
    //UART_PRINT("\r\nwrap [B | L]                             Set terminal wrap mode level:      ");
    //UART_PRINT("\r\n                                         B - Byte; L - Line.                ");
    UART_PRINT("\r\nsend [dest] [data]                            Send provided data to dest.        ");
    UART_PRINT("\r\n                                              If TCP is used, dest is ignored.   ");
    UART_PRINT("\r\nrecv [dest] [length] [timeout]                Receive data from dest.           ");
    UART_PRINT("\r\n                                              If TCP is used, dest is ignored.   ");
    UART_PRINT("\r\n                                              If length not specified, 1 is used.");
    UART_PRINT("\r\n                                              Will wait until length bytes       ");
    UART_PRINT("\r\n                                              received, the optional timeout [us] is");
    UART_PRINT("\r\n                                              reached, or ^C is received.        ");
    UART_PRINT("\r\nping [IP | name] [number]                     Ping the provided dest for         ");
    UART_PRINT("\r\n                                              an optionally specified number of  ");
    UART_PRINT("\r\n                                              times. If not specified, ping will ");
    UART_PRINT("\r\n                                              run until ^C is received.          ");
    UART_PRINT("\r\nmDNS [option]                                   mDNS configuration                   ");
    UART_PRINT("\r\n                                              option: 0-Advertise,1- Listen      ");
}

//
//*****************************************************************************
//
//! basic_Interpreter
//!
//!  \param  pvParameters
//!
//!  \return none
//!
//!  \brief   Task handler function to receive inputs from the UART
//
//*****************************************************************************

void basic_Interpreter(void *pvParameters)
{
    while(1)
    {
        if((g_UartCmdSent == 1) &&(g_UartHaveCmd ==0))
        {
            g_UartCmdSent = 0;
            memset(g_ucUARTBuffer, 0, sizeof(g_ucUARTBuffer));
        }
        g_UartHaveCmd=GETChar(&g_ucUARTRecvBuffer[0]);
    }
}


//*****************************************************************************
//
//! GETChar
//!
//!  \param  ucBuffer to which Command will be populated
//!
//!  \return Success or Failure
//!
//!  \brief   Get the char string from UART
//
//*****************************************************************************
unsigned int GETChar(unsigned char *ucBuffer)
{

    int i=0;
    char c;
    uiUartline=0;

    //
    // Wait to receive a character over UART
    //
    while(MAP_UARTCharsAvail(CONSOLE) == false)
    {
    	osi_Sleep(1);
    }
    c = MAP_UARTCharGetNonBlocking(CONSOLE);

    MAP_UARTCharPut(CONSOLE, c);
    ilength=0;
    //
    // Checking the end of line
    //
    while(c!='\r' && c!='\n' )
    {
        uiUartline=1;
        //
        // Copying Data from UART into a buffer
        //
        if(  c!='\b')
        {
            ilength++;
            *(ucBuffer+i)=c;
            i++;
        }
        //
        // Deleting last character when you hit backspace
        //
        if(c=='\b')
        {
            i--;
            ilength--;
        }
        while(MAP_UARTCharsAvail(CONSOLE) == false)
        {
        	osi_Sleep(1);
        }
        c = MAP_UARTCharGetNonBlocking(CONSOLE);
        MAP_UARTCharPut(CONSOLE, c);
    }

    strncpy((char*)g_ucUARTBuffer,(char *)ucBuffer,ilength);
    memset(g_ucUARTRecvBuffer, 0, sizeof(g_ucUARTRecvBuffer));

   return uiUartline;
}


//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
