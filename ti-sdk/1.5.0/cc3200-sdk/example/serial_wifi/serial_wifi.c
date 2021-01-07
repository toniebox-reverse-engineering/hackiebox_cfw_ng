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

// Simplelink includes
#include "simplelink.h"

//Driverlib includes
#include "utils.h"

//Free_rtos/ti-rtos includes
#include "osi.h"

//Common interface includes
#include "network_if.h"
#include "common.h"

//Application Includes
#define extern
#include "serial_wifi.h"
#undef extern

//*****************************************************************************
//
//! \addtogroup serial_wifi
//! @{
//
//*****************************************************************************

/* Application specific status/error codes */
typedef enum{
    /* Choosing this number to avoid overlap w/ host-driver's error codes */
    LAN_CONNECTION_FAILED = -0x7D0,
    CLIENT_CONNECTION_FAILED = LAN_CONNECTION_FAILED - 1,
    DEVICE_NOT_IN_STATION_MODE = CLIENT_CONNECTION_FAILED - 1,

    STATUS_CODE_MAX = -0xBB8
}e_AppStatusCodes;

#define CC3200_DNS_NAME "CC3200._uart._tcp.local"

//****************************************************************************
// GLOBAL VARIABLES -- Start
//****************************************************************************
extern unsigned long  g_ulStatus;
//****************************************************************************
// GLOBAL VARIABLES -- End
//****************************************************************************


//                      LOCAL FUNCTION PROTOTYPES
//****************************************************************************
void Interpreter_Task(void *pvParameters);
long StartInterpreterMode(InterpreterRole_e role);
long ParseWlanConnectString();
long ParsemDNSString();
long ParsePingString();
long ParseRecvString();
long ParseSendString();
long ParseConnectString();
long ExecuteCommand(OperationMode_e source);
long StartUdpServer();
long StartTcpServer();
long InitialiseCmdMenu();
int GetCmdIndex( char *str);
long  InitialConfiguration(void);
static long InitDevice();
static long ConfigureSimpleLinkForSerialWifi(unsigned int uiMode);
static int set_time();
extern void InitializeAppVariables();


//*****************************************************************************
//
//! Interpreter_Task
//!
//!  \param  pvParameters
//!
//!  \return none
//!
//!  \brief Task handler function to handle the Serial-WiFi functionality
//
//*****************************************************************************

void Interpreter_Task(void *pvParameters)
{
    unsigned short UartBufLen;
    unsigned short HandleNow = 1;
    long counter = 0;
    long optvalue = 0;
    SlSocklen_t optlen = sizeof(long);
    SlrxFilterPrePreparedFiltersMask_t  FilterPrePreparedFiltersMask;
    char TempBuf[8];
    unsigned long ulAddr;
    unsigned long usPort;
    unsigned short     ulTextLen = 200;
    char pText[200];
    unsigned long ulToken;
    long lRetVal = -1;


    lRetVal = InitialiseCmdMenu();
    if(lRetVal < 0)
    {
        LOOP_FOREVER();
    }
    
    g_timeout.tv_sec = 0;
    g_timeout.tv_usec = 0;

    g_WlanRxMode = INTERPRETER_MODE;
    g_Protocol = SL_IPPROTO_UDP;
    g_Type = SL_SOCK_DGRAM;
    
    //powering on the CC3200 NWP
    lRetVal = ConfigureSimpleLinkForSerialWifi(ROLE_STA);
    if(lRetVal < 0)
    {
        LOOP_FOREVER();
    }

    //remove all filters
    memset(FilterPrePreparedFiltersMask, 0, \
             sizeof(FilterPrePreparedFiltersMask));
    FilterPrePreparedFiltersMask[0] = 0x00;

    lRetVal = sl_WlanPolicySet(SL_POLICY_CONNECTION,\
                               SL_CONNECTION_POLICY(1,0,0,0,0),NULL,0);
    if(lRetVal < 0)
    {
        UART_PRINT("Failed to set AUTO connect mode\n\r");
        LOOP_FOREVER();
    }

    //printing the command prompt
    UART_PRINT(COMMAND_PROMPT);
    UART_PRINT(COMMAND_PROMPT);
    UART_PRINT(COMMAND_PROMPT);


    if((!IS_CONNECTED(g_ulStatus)))
        UART_PRINT(ATTEMPTING_TO_CONNECT_TO_AP);
    UART_PRINT(COMMAND_PROMPT);
    if(!IS_IP_ACQUIRED(g_ulStatus))
            UART_PRINT(ATTEMPTING_TO_ACQUIRE_IP);
    UART_PRINT(COMMAND_PROMPT);

    MAP_UtilsDelay(48000000);//waiting for 6 secs

    if((!IS_CONNECTED(g_ulStatus)) || (!IS_IP_ACQUIRED(g_ulStatus)))
    {
        UART_PRINT(COMMAND_PROMPT);
        UART_PRINT("Connection Unsuccessful.");
        UART_PRINT(COMMAND_PROMPT);
        UART_PRINT("Connect to the AP using local control mode(\\\\<)");
    }


    UART_PRINT(COMMAND_PROMPT);

    //Set the time in the device.
    lRetVal = set_time();
    if(lRetVal < 0)
    {
         UART_PRINT("Unable to set time in the device\n\r");
         LOOP_FOREVER();
    }

    counter = 0;

    while (1)
    {
        osi_Sleep(1);
        if((IS_IP_ACQUIRED(g_ulStatus)) && (g_ConfigurationDone == 0))
        {
            lRetVal = InitialConfiguration();
            if(lRetVal < 0)
            {
                LOOP_FOREVER();
            }
        }

        if (g_mDNSStatus && (IS_IP_ACQUIRED(g_ulStatus)))
        {
            g_mDNSStatus = sl_NetAppDnsGetHostByService(\
                           (signed char *)CC3200_DNS_NAME,\
                           (unsigned char)strlen((const char *)CC3200_DNS_NAME),\
                           SL_AF_INET,&ulAddr,&usPort,\
                           &ulTextLen,(signed char *)&pText[0]);
            if(g_mDNSStatus == 0)
            {

                {
                    g_imDNSMode = 1;
                    lRetVal = sl_FsOpen("mDNSStatusFile.txt",\
                                           FS_MODE_OPEN_WRITE,\
                                           &ulToken,&g_isFileHandle);
                    if(lRetVal < 0)
                    {
                        UART_PRINT("Unable to open file\n\r");
                        LOOP_FOREVER();
                    }
                    lRetVal = sl_FsWrite(g_isFileHandle, 0, \
                                         (unsigned char *)&g_imDNSMode,\
                                          sizeof(g_imDNSMode));
                    if(lRetVal < 0)
                    {
                        UART_PRINT("Unable to write file\n\r");
                        LOOP_FOREVER();
                    }
                    lRetVal = sl_FsClose(g_isFileHandle,0,0,NULL);
                    if(lRetVal < 0)
                    {
                        UART_PRINT("Unable to close file\n\r");
                        LOOP_FOREVER();
                    }

                }

                g_RemoteAddr.sa_data[0] = ((usPort >> 8) & 0xFF);
                g_RemoteAddr.sa_data[1] = (usPort & 0xFF);

                g_RemoteAddr.sa_data[2] = (ulAddr & (0xFF000000)) >> 24;
                g_RemoteAddr.sa_data[3] = (ulAddr & (0x00FF0000)) >> 16;
                g_RemoteAddr.sa_data[4] = (ulAddr & (0x0000FF00)) >> 8;
                g_RemoteAddr.sa_data[5] = (ulAddr & (0x000000FF)) >> 0;

                g_RemoteAddr.sa_family = SL_AF_INET;


        #ifndef SECURE_SOCKETS
                g_Socket = sl_Socket(SL_AF_INET, SL_SOCK_STREAM, SL_IPPROTO_TCP);
        #else
                g_Socket = sl_Socket(SL_AF_INET, SL_SOCK_STREAM, SL_SEC_SOCKET);

                if( g_Socket < 0 )
                {
                    UART_PRINT("Failed to create socket\n\r");
                    LOOP_FOREVER();
                }

                //
                // configure the socket as SSLV3.0
                //
                lRetVal = sl_SetSockOpt(g_Socket, SL_SOL_SOCKET, \
                                           SL_SO_SECMETHOD, &g_ucMethod, \
                                           sizeof(g_ucMethod));
                if( lRetVal < 0 )
                {
                   UART_PRINT("Failed to set socket options\n\r");
                   sl_Close(g_Socket);
                   LOOP_FOREVER();
                }
                //
                //configure the socket as RSA with RC4 128 SHA
                //
                lRetVal = sl_SetSockOpt(g_Socket, SL_SOL_SOCKET, \
                                        SL_SO_SECURE_MASK, &g_uiCipher, \
                                        sizeof(g_uiCipher));
                if( lRetVal < 0 )
                {
                    UART_PRINT("Failed to set socket options\n\r");
                    sl_Close(g_Socket);
                    LOOP_FOREVER();
                }

                lRetVal = sl_SetSockOpt(g_Socket, SL_SOL_SOCKET, \
                                      SL_SO_SECURE_FILES_PRIVATE_KEY_FILE_NAME,\
                                      SL_SSL_SRV_KEY_FILE_NAME, \
                                      strlen(SL_SSL_SRV_KEY_FILE_NAME));
                  if( lRetVal < 0 )
                  {
                    UART_PRINT("Failed to set socket options\r\n");
                    sl_Close(g_Socket);
                    LOOP_FOREVER();
                  }

                lRetVal = sl_SetSockOpt(g_Socket, SL_SOL_SOCKET, \
                                     SL_SO_SECURE_FILES_CERTIFICATE_FILE_NAME,\
                                     SL_SSL_SRV_CERT_FILE_NAME, \
                                     strlen(SL_SSL_SRV_CERT_FILE_NAME));
                if( lRetVal < 0 )
                {
                    UART_PRINT("Failed to set socket options\r\n");
                    sl_Close(g_Socket);
                    LOOP_FOREVER();
                }
                lRetVal = sl_SetSockOpt(g_Socket, SL_SOL_SOCKET, \
                                       SL_SO_SECURE_FILES_CA_FILE_NAME, \
                                       SL_SSL_CLNT_CERT_FILE_NAME, \
                                       strlen(SL_SSL_CLNT_CERT_FILE_NAME));
                if( lRetVal < 0 )
                {
                    UART_PRINT("Failed to set socket options\r\n");
                    sl_Close(g_Socket);
                    LOOP_FOREVER();
                }
        #endif

                g_RetVal = sl_Connect(g_Socket, &g_RemoteAddr, \
                                        sizeof(g_RemoteAddr));
                if(g_RetVal >= 0)
                {
                    g_NonBlocking = 1;
                    g_Status = sl_SetSockOpt(g_Socket, SL_SOL_SOCKET,\
                                         SL_SO_NONBLOCKING, \
                                         &g_NonBlocking, sizeof(g_NonBlocking));
                    if( g_Status < 0 )
                    {
                        UART_PRINT("Failed to set socket options\r\n");
                        sl_Close(g_Socket);
                        LOOP_FOREVER();
                    }
                    
                    g_TcpConnected = 1;
                    UART_PRINT(TCP_CONNECTION_ESTABLISHED);
                    UART_PRINT(COMMAND_PROMPT);
                    g_Protocol = SL_IPPROTO_TCP;
                }
                else
                {
                  g_mDNSStatus = 192;//retry the mDNS connection
                  sl_Close(g_Socket);
                }
            }
        }

        if ((g_TcpConnected == 0) && (IS_IP_ACQUIRED(g_ulStatus)))
        {
            // if no  TCP connection, server should still be active -> check for
            // incoming requests
            g_SocketChild = sl_Accept(g_SocketTcpServer, &g_RemoteAddr, \
                                                    &g_AddrLen);

            if (g_SocketChild >= 16)
            {
                g_TcpConnected = 1;
                g_Protocol = SL_IPPROTO_TCP;
                g_Type = SL_SOCK_STREAM;

            #ifdef SECURE_SOCKETS
                //
                // configure the socket as SSLV3.0
                //
                lRetVal = sl_SetSockOpt(g_SocketChild, SL_SOL_SOCKET, \
                                          SL_SO_SECMETHOD, &g_ucMethod,\
                                          sizeof(g_ucMethod));
                if( lRetVal < 0 )
                {
                    UART_PRINT("Failed to set socket options\n\r");
                    sl_Close(g_Socket);
                    LOOP_FOREVER();
                }
                
                //
                //configure the socket as RSA with RC4 128 SHA
                //
                lRetVal = sl_SetSockOpt(g_SocketChild, SL_SOL_SOCKET,\
                                       SL_SO_SECURE_MASK, &g_uiCipher,\
                                       sizeof(g_uiCipher));
                if( lRetVal < 0 )
                {
                    UART_PRINT("Failed to set socket options\n\r");
                    sl_Close(g_Socket);
                    LOOP_FOREVER();
                }

                //
                //configure the socket with self certificate 
                // - for server verification
                //
                lRetVal = sl_SetSockOpt(g_Socket, SL_SOL_SOCKET, \
                                      SL_SO_SECURE_FILES_PRIVATE_KEY_FILE_NAME,\
                                      SL_SSL_SRV_KEY_FILE_NAME, \
                                      strlen(SL_SSL_SRV_KEY_FILE_NAME));
                if( lRetVal < 0 )
                {
                    UART_PRINT("Failed to set socket options\r\n");
                    sl_Close(g_Socket);
                    LOOP_FOREVER();
                }

                lRetVal = sl_SetSockOpt(g_Socket, SL_SOL_SOCKET, \
                                     SL_SO_SECURE_FILES_CERTIFICATE_FILE_NAME, \
                                     SL_SSL_SRV_CERT_FILE_NAME,\
                                     strlen(SL_SSL_SRV_CERT_FILE_NAME));
                if( lRetVal < 0 )
                {
                    UART_PRINT("Failed to set socket options\r\n");
                    sl_Close(g_Socket);
                    LOOP_FOREVER();
                }
                
                lRetVal = sl_SetSockOpt(g_Socket, SL_SOL_SOCKET, \
                                        SL_SO_SECURE_FILES_CA_FILE_NAME,\
                                        SL_SSL_CLNT_CERT_FILE_NAME, \
                                        strlen(SL_SSL_CLNT_CERT_FILE_NAME));
                if( lRetVal < 0 )
                {
                    UART_PRINT("Failed to set socket options\r\n");
                    sl_Close(g_Socket);
                    LOOP_FOREVER();
                }
            #endif
            
                //always work with one active socket
                g_Socket = g_SocketChild;
                //close server sockets
                sl_Close(g_SocketTcpServer);
                sl_Close(g_SocketUdpServer);

                UART_PRINT(TCP_CONNECTION_ESTABLISHED);
                UART_PRINT(COMMAND_PROMPT);
            }
        }

        g_Status = sl_GetSockOpt(g_Socket, SL_SOL_SOCKET, \
                                 SL_SO_NONBLOCKING, &optvalue, \
                                 &optlen);

        if (g_Status == 0)
        {
            memset(uBufRx.g_RecvBuf, 0, sizeof(uBufRx.g_RecvBuf));
            if (g_Protocol == SL_IPPROTO_TCP)
            {
                //TCP
                g_BytesRx = sl_Recv(g_Socket, uBufRx.g_RecvBuf, \
                                     sizeof(uBufRx.g_RecvBuf), 0);
            }
            else
            {
                //UDP
                g_AddrLen = sizeof(SlSockAddrIn_t);

                g_RemoteAddr.sa_data[0] = ((INTERPRETER_PORT >> 8) & 0xFF);
                g_RemoteAddr.sa_data[1] = (INTERPRETER_PORT & 0xFF);
                g_RemoteAddr.sa_family = SL_AF_INET;

                g_ActiveReadFds = g_ReadFds;

                g_RetVal = sl_Select(g_Socket + 1, &g_ActiveReadFds, NULL,\
                                                            NULL, &g_timeout);

                if (g_RetVal > 0)
                {
                    g_BytesRx = sl_RecvFrom(g_Socket, uBufRx.g_RecvBuf, \
                                               sizeof(uBufRx.g_RecvBuf), 0, \
                                               &g_RemoteAddr, &g_AddrLen);
                }
            }
            if (g_BytesRx > 0)
            {
                if (g_WrapMode == BYTE_LEVEL_TERMINAL_MODE)
                {
                    //binary mode
                    if (g_InterpreterMode == BINARY_INTERPRETER_MODE)
                    {
                        if (counter == 7) //end of byte
                        {
                            TempBuf[counter] = uBufRx.g_RecvBuf[0];
                            strncpy(uBufRx.g_RecvBuf, TempBuf, 8);
                            uBufRx.g_RecvBuf[8] = 0; //mark end of string
                            counter = 0;
                            g_BytesRx = 8;
                            HandleNow = 1;
                        }
                        else
                        {
                            TempBuf[counter ++] = uBufRx.g_RecvBuf[0];
                        }
                    }
                    //hexa mode
                    else if (g_InterpreterMode == HEXA_INTERPRETER_MODE)
                    {
                        if (counter == 1) //end of byte
                        {
                            TempBuf[counter] = uBufRx.g_RecvBuf[0];
                            strncpy(uBufRx.g_RecvBuf, TempBuf, 2);
                            uBufRx.g_RecvBuf[2] = 0; //mark end of string
                            counter = 0;
                            g_BytesRx = 2;
                            HandleNow = 1;
                        }
                        else
                        {
                            TempBuf[counter ++] = uBufRx.g_RecvBuf[0];
                        }
                    }
                    else
                    {
                        //ascii mode
                        counter = 0;
                        g_BytesRx = 1;
                        HandleNow = 1;
                    }

                }
                else
                {
                    //line level mode
                    HandleNow = 1;
                }

                if (HandleNow == 1)
                {
                    if (g_InterpreterMode == BINARY_INTERPRETER_MODE)
                    {
                        lRetVal = btoa();
                        if( lRetVal < 0 )
                        {
                            UART_PRINT("Failed to convert binary string to "
                                        "ascii\r\n");
                            sl_Close(g_Socket);
                            LOOP_FOREVER();
                        }
                        g_BytesRx /= 8;
                    }
                    else if (g_InterpreterMode == HEXA_INTERPRETER_MODE)
                    {
                        lRetVal = htoa();
                        if( lRetVal < 0 )
                        {
                            UART_PRINT("Failed to convert hexa string to"
                                        "ascii\r\n");
                            sl_Close(g_Socket);
                            LOOP_FOREVER();
                        }
                        g_BytesRx /= 2;
                    }

                    //two states are possible: (1) controlled by remote;
                    // (2) interpreter mode
                    UART_PRINT(uBufRx.g_RecvBuf);
                    UART_PRINT(COMMAND_PROMPT);

                    if (g_WlanRxMode == REMOTE_CONTROL_MODE)
                    {
                        //this state means controlled by remote
                        strncpy((char*)g_ucUARTBuffer, uBufRx.g_RecvBuf,\
                                    g_BytesRx);
                        g_ucUARTBuffer[g_BytesRx] = 0;
                        lRetVal = ExecuteCommand(REMOTE_CONTROL_MODE);                    
                        /*if(lRetVal < 0)
                        {
                            UART_PRINT("Failed to execute command\n\r");
                            LOOP_FOREVER();
                        }*/
                    }

                    if ((strstr(uBufRx.g_RecvBuf, "\\>")) ||\
                            (strstr(uBufRx.g_RecvBuf, "\\>\n")))
                    {
                        //enter controlled by remote mode
                        g_WlanRxMode = REMOTE_CONTROL_MODE;
                        UART_PRINT(ENTERED_CONTROLLED_BY_REMOTE_MODE);
                        UART_PRINT(COMMAND_PROMPT);
                    }

                    HandleNow = 0;
                }
                g_BytesRx = 0;
            }
        }

        if (1 == g_UartHaveCmd)
        {
            for (UartBufLen = 0; UartBufLen < UART_IF_BUFFER; UartBufLen++)
            {
                if (g_ucUARTBuffer[UartBufLen] == 0x0D)
                {
                    g_ucUARTBuffer[UartBufLen] = 0;
                    break;
                }
            }

            g_UartHaveCmd = 0;

            if (g_UartMode == INTERPRETER_MODE || g_UartMode == \
                                                    CONTROL_REMOTE_MODE)
            {
                //terminate interpreter mode
                if (strstr((char*)g_ucUARTBuffer, "quit"))
                {
                    lRetVal = sl_Stop(SL_STOP_TIMEOUT);                
                    goto quit;
                }
                //enter local control mode, comparing with \\<\0 as previously
                else if (strstr((char*)g_ucUARTBuffer, "\\<\0"))
                {
                    g_UartMode = LOCAL_CONTROL_MODE;
                    UART_PRINT(COMMAND_PROMPT);
                    UART_PRINT(ENTERED_LOCAL_CONTROL_MODE);
                    UART_PRINT(COMMAND_PROMPT);
                }

                //sl_Send stream of data to remote
                else
                {
                    if (strstr((char*)g_ucUARTBuffer, "\\>\0"))
                    {
                        g_UartMode = CONTROL_REMOTE_MODE;
                        UART_PRINT(COMMAND_PROMPT);
                        UART_PRINT(ENTERED_CONTROL_REMOTE_MODE);
                    }
                    else if(g_UartMode == CONTROL_REMOTE_MODE)
                    {
                        //help
                        if (strstr((char*)g_ucUARTBuffer, g_cmd_help[0]))
                        {
                            PrintCommandMenu();
                            UART_PRINT(COMMAND_PROMPT);
                        }
                        if (strstr((char*)g_ucUARTBuffer,g_cmd_help[2]))
                        {
                            g_UartMode = INTERPRETER_MODE;
                            UART_PRINT(COMMAND_PROMPT);
                            UART_PRINT(EXITED_CONTROL_REMOTE_MODE);
                            UART_PRINT(COMMAND_PROMPT);
                        }

                    }

                    if (g_Protocol == SL_IPPROTO_TCP)
                    {
                        g_BytesTx = sl_Send(g_Socket, g_ucUARTBuffer, \
                                             strlen((char*)g_ucUARTBuffer), 0);
                    }
                    else
                    {
                        //UDP
                        g_RemoteAddr.sa_data[0] = ((INTERPRETER_PORT >> 8) & \
                                                                        0xFF);
                        g_RemoteAddr.sa_data[1] = (INTERPRETER_PORT & 0xFF);
                        g_RemoteAddr.sa_family = SL_AF_INET;
                        g_RemoteAddr.sa_data[2] = ((INTERPRETER_IGMP_IP_HEXA >> \
                                                                    24) & 0xFF);
                        g_RemoteAddr.sa_data[3] = ((INTERPRETER_IGMP_IP_HEXA >> \
                                                                    16) & 0xFF);
                        g_RemoteAddr.sa_data[4] = ((INTERPRETER_IGMP_IP_HEXA >> \
                                                                    8) & 0xFF);
                        g_RemoteAddr.sa_data[5] = ((INTERPRETER_IGMP_IP_HEXA) & \
                                                                        0xFF);
                        g_BytesTx = sl_SendTo(g_Socket, g_ucUARTBuffer,  \
                                          strlen((char*)g_ucUARTBuffer), 0,\
                                          &g_RemoteAddr, sizeof(SlSockAddr_t));
                    }
                    UART_PRINT(COMMAND_PROMPT);
                }

            }

            else if (g_UartMode == LOCAL_CONTROL_MODE)
            {
                UART_PRINT(COMMAND_PROMPT);
                lRetVal = ExecuteCommand(LOCAL_CONTROL_MODE);            
                /*if(lRetVal < 0)
                {
                    UART_PRINT("Unable to execute command\n\r");
                    LOOP_FOREVER();
                }*/
            }
            g_UartCmdSent = 1;
            memset(g_ucUARTBuffer, 0, sizeof(g_ucUARTBuffer));
        }
    }

    quit:
        UART_PRINT(COMMAND_PROMPT);
        UART_PRINT(EXIT_INTERPRETER_MODE);
        UART_PRINT(COMMAND_PROMPT);
}

//*****************************************************************************
//
//! StartInterpreterMode
//!
//!  \param  role
//!
//!  \return 0 on success else error code 
//!
//!  \brief StartInterpreterMode to create the TCP and UDP sockets
//
//*****************************************************************************
long StartInterpreterMode(InterpreterRole_e role)
{   
    long lRetVal = -1;
    
    if  (role == START_INTERPRETER_SERVERS_MODE)
    {
        lRetVal = StartTcpServer();
        ASSERT_ON_ERROR(lRetVal);
        lRetVal = StartUdpServer();
        ASSERT_ON_ERROR(lRetVal);

        //set udp server's socket as the single "active"
        g_Socket =  g_SocketUdpServer;
    }
    UART_PRINT(ENTERED_INTERPRETER_MODE);
    UART_PRINT(COMMAND_PROMPT);
    return SUCCESS;
}

//*****************************************************************************
//
//! ParseWlanConnectString
//!
//! @param  none
//!
//! @return 0 on success else error code
//!
//! @brief  Parse the received WlanConnect String
//
//*****************************************************************************
long ParseWlanConnectString()
{
    char *pcParse;
    unsigned long iLantype;
    long lRetVal = -1;
    
    // discarding the command
    g_pcSsid = (signed char *)strtok((char*)g_ucUARTBuffer," ");
    g_pcSsid ++;
    g_pcSsid = (signed char *)strtok(NULL," ");
    g_ulSsidLen = (unsigned char)strlen((const char *)g_pcSsid);

    pcParse = strtok(NULL," ");
    lRetVal = atolong(pcParse, &iLantype);
    ASSERT_ON_ERROR(lRetVal);

    pcParse = strtok(NULL," ");
    lRetVal = atolong(pcParse,(unsigned long *) &g_SecParams.Type);
    ASSERT_ON_ERROR(lRetVal);
    
    switch(g_SecParams.Type)
    {
        case SL_SEC_TYPE_OPEN:
            g_SecParams.Key = "";
            g_SecParams.KeyLen = 0;
            g_SecParams.Type = SL_SEC_TYPE_OPEN;
            break;
            
        case SL_SEC_TYPE_WEP:

            g_SecParams.Key = (signed char *)strtok(NULL," ");
            g_SecParams.KeyLen = (unsigned char)strlen((const char *) \
                                                        g_SecParams.Key);
            g_SecParams.Type = SL_SEC_TYPE_WEP;
            break;

        case SL_SEC_TYPE_WPA:
            if(iLantype == 1)
            {
                eapParams.User = (signed char *)strtok(NULL," ");
                eapParams.UserLen = (unsigned char)strlen((const char *) \
                                                            eapParams.User);
                eapParams.EapMethod = SL_ENT_EAP_METHOD_PEAP1_MSCHAPv2;
                eapParams.AnonUserLen = 0;
            }
            g_SecParams.Key = (signed char *)strtok(NULL," ");
            g_SecParams.KeyLen = (unsigned char)strlen((const char *) \
                                                            g_SecParams.Key);
            g_SecParams.Type = SL_SEC_TYPE_WPA;

            break;
        

    }

    return SUCCESS;
}

//*****************************************************************************
//
//! ParsemDNSString
//!
//! @param  none
//!
//! @return 0 on success else error code
//!
//! @brief  Parse the received mDNS String
//
//*****************************************************************************
long ParsemDNSString()
{
    long lRetVal = -1;
    
    g_mDNSMode = strchr((char*)g_ucUARTBuffer, ' ');
    g_mDNSMode++;
    lRetVal = atolong(g_mDNSMode, &g_imDNSMode);
    ASSERT_ON_ERROR(lRetVal);
    return SUCCESS;
}

//*****************************************************************************
//
//! ParsePingString
//!
//! @param  none
//!
//! @return 0 on success else error code
//!
//! @brief  Parse the received Ping String
//
//*****************************************************************************
long ParsePingString()
{
    SlPingStartCommand_t PingParams;
    SlPingReport_t PingReport;
    long lRetVal = -1;

    //IP  handling
    g_pcIp = strchr((char*)g_ucUARTBuffer, ' '); 
    g_pcIp ++;
    lRetVal = atolong(g_pcIp, &g_ulIp);    
    ASSERT_ON_ERROR(lRetVal);
    
    g_ulPingIp |= g_ulIp << 24;
    g_pcIp = strchr(g_pcIp, '.');
    g_pcIp ++;
    lRetVal = atolong(g_pcIp, &g_ulIp);
    ASSERT_ON_ERROR(lRetVal);
    
    g_ulPingIp |= g_ulIp << 16;
    g_pcIp = strchr(g_pcIp, '.');
    g_pcIp ++;
    lRetVal = atolong(g_pcIp, &g_ulIp);    
    ASSERT_ON_ERROR(lRetVal);
    
    g_ulPingIp |= g_ulIp << 8;
    g_pcIp = strchr(g_pcIp, '.');
    g_pcIp ++;
    lRetVal = atolong(g_pcIp, &g_ulIp);    
    ASSERT_ON_ERROR(lRetVal);
    
    g_ulPingIp |= g_ulIp;

    //number of attempts handling              
    g_pcPingAttempts = strchr(g_pcIp, ' ');
    if (g_pcPingAttempts)
    {
        g_pcPingAttempts ++;
        lRetVal = atolong(g_pcPingAttempts, &g_ulPingAttempts);
        ASSERT_ON_ERROR(lRetVal);    
    }
    else
    {
        //use default number of ping attempts
        g_ulPingAttempts = 32768 - 1;
    }

    PingParams.Flags                  = NULL;
    PingParams.Ip                     = g_ulPingIp;
    PingParams.PingIntervalTime       = PING_INTERVAL_TIME;
    PingParams.PingRequestTimeout     = PING_REQ_TIMEOUT;
    PingParams.PingSize               = INTERPRETER_PING_SIZE;
    PingParams.TotalNumberOfAttempts  = g_ulPingAttempts;

    lRetVal = sl_NetAppPingStart(&PingParams,SL_AF_INET,&PingReport,NULL);
    ASSERT_ON_ERROR(lRetVal);    

    if(PingReport.PacketsSent == PingReport.PacketsReceived)
    {
        UART_PRINT(PING_SUCCESSFUL);
        UART_PRINT(COMMAND_PROMPT);
    }
    UART_PRINT(PING_STOPPED);
    UART_PRINT(COMMAND_PROMPT);
    return SUCCESS;
}

//*****************************************************************************
//
//! ParseRecvString
//!
//! @param  none
//!
//! @return 0 on success else error code
//!
//! @brief  Parse the Recv String
//
//*****************************************************************************
long ParseRecvString()
{ 
    long lRetVal = -1;
    SlTimeval_t     RecTimeout;
    RecTimeout.tv_sec = 0;
    RecTimeout.tv_usec = 0;

    g_pcIp = strchr((char*)g_ucUARTBuffer, ' '); 
    if (g_pcIp)
    {
        g_pcIp ++;
    }
    else
    {
        UART_PRINT("Ip incorrect\n\r");
        return - 1;
    }

    //IP  handling
    lRetVal = atolong(g_pcIp, &g_ulIp);
    ASSERT_ON_ERROR(lRetVal);
    
    g_RemoteAddr.sa_data[2] = g_ulIp;
    g_pcIp = strchr(g_pcIp, '.');
    g_pcIp ++;
    lRetVal = atolong(g_pcIp, &g_ulIp);
    ASSERT_ON_ERROR(lRetVal);
    
    g_RemoteAddr.sa_data[3] = g_ulIp;
    g_pcIp = strchr(g_pcIp, '.');
    g_pcIp ++;
    lRetVal = atolong(g_pcIp, &g_ulIp);
    ASSERT_ON_ERROR(lRetVal);
    
    g_RemoteAddr.sa_data[4] = g_ulIp;
    g_pcIp = strchr(g_pcIp, '.');
    g_pcIp ++;
    lRetVal = atolong(g_pcIp, &g_ulIp);
    ASSERT_ON_ERROR(lRetVal);
    
    g_RemoteAddr.sa_data[5] = g_ulIp;

    //length handling              
    g_pcLength = strchr(g_pcIp, ' ');
    if (g_pcLength)
    {
        g_pcLength ++;
        lRetVal = atolong(g_pcLength, &g_ulLength);
        ASSERT_ON_ERROR(lRetVal);

        //timeout handling
		g_pcTimeout = strchr(g_pcLength, ' ');
		if (g_pcTimeout)
		{
		   g_pcTimeout ++;
		   lRetVal = atolong(g_pcTimeout, &RecTimeout.tv_usec);
		   ASSERT_ON_ERROR(lRetVal);
		}
		else
		{
			//use default Timeout
			RecTimeout.tv_usec = INTERPRETER_RECV_TIMEOUT;
		}
    }
    else
    {
        //use default Length
        g_ulLength = INTERPRETER_RECV_LENGTH;
        //use default Timeout
        RecTimeout.tv_usec = INTERPRETER_RECV_TIMEOUT;
    }

    g_Status = sl_SetSockOpt(g_Socket, SL_SOL_SOCKET, SL_SO_RCVTIMEO,\
                                 &RecTimeout, sizeof(RecTimeout));
    if(g_Status < 0 )
    {
        UART_PRINT("Failed to set socket options\r\n");
        sl_Close(g_Socket);
        return g_Status;
    }

    return SUCCESS;
}

//*****************************************************************************
//
//! ParseSendString
//!
//! @param  none
//!
//! @return 0 on success else error code 
//!
//! @brief  Parse the Send String
//
//*****************************************************************************
long ParseSendString()
{
    long lRetVal = -1;
    g_pcIp = strchr((char*)g_ucUARTBuffer, ' '); 

    if (g_pcIp)
    {
        g_pcIp ++;
    }
    else
    {
        UART_PRINT("IP incorrect\n\r");
        return -1;
    }

    //IP  handling        
    lRetVal = atolong(g_pcIp, &g_ulIp);
    ASSERT_ON_ERROR(lRetVal);
    
    g_RemoteAddr.sa_data[2] = g_ulIp;
    g_pcIp = strchr(g_pcIp, '.');
    g_pcIp ++;
    lRetVal = atolong(g_pcIp, &g_ulIp);
    ASSERT_ON_ERROR(lRetVal);
    
    g_RemoteAddr.sa_data[3] = g_ulIp;
    g_pcIp = strchr(g_pcIp, '.');
    g_pcIp ++;
    lRetVal = atolong(g_pcIp, &g_ulIp);
    ASSERT_ON_ERROR(lRetVal);
    
    g_RemoteAddr.sa_data[4] = g_ulIp;
    g_pcIp = strchr(g_pcIp, '.');
    g_pcIp ++;
    lRetVal = atolong(g_pcIp, &g_ulIp);
    ASSERT_ON_ERROR(lRetVal);
    
    g_RemoteAddr.sa_data[5] = g_ulIp;

    g_pcData = strchr(g_pcIp, ' ');
    g_pcData ++;
    return SUCCESS;
}

//*****************************************************************************
//
//! ParseConnectString
//!
//! @param  none
//!
//! @return 0 on success else error code
//!
//! @brief  Parse the Connect String
//
//*****************************************************************************
long ParseConnectString()
{
    int lRetVal;

    //IP  handling
    g_pcIp = strchr((char*)g_ucUARTBuffer, ' '); 
    g_pcIp ++;
    lRetVal = atolong(g_pcIp, &g_ulIp);
    ASSERT_ON_ERROR(lRetVal);
    
    g_RemoteAddr.sa_data[2] = g_ulIp;
    g_pcIp = strchr(g_pcIp, '.');
    g_pcIp ++;
    lRetVal = atolong(g_pcIp, &g_ulIp);
    ASSERT_ON_ERROR(lRetVal);
    
    g_RemoteAddr.sa_data[3] = g_ulIp;
    g_pcIp = strchr(g_pcIp, '.');
    g_pcIp ++;
    lRetVal = atolong(g_pcIp, &g_ulIp);
    ASSERT_ON_ERROR(lRetVal);
    
    g_RemoteAddr.sa_data[4] = g_ulIp;
    g_pcIp = strchr(g_pcIp, '.');
    g_pcIp ++;
    lRetVal = atolong(g_pcIp, &g_ulIp);
    ASSERT_ON_ERROR(lRetVal);
    
    g_RemoteAddr.sa_data[5] = g_ulIp;

    //port handling              
    g_pcPort = strchr(g_pcIp, ' ');
    if (g_pcPort)
    {
        g_pcPort ++;
        lRetVal = atolong(g_pcPort, &g_ulPort);
        ASSERT_ON_ERROR(lRetVal);
        //protocol handling
		g_pcProtocol = strchr(g_pcPort, ' ');
		if(g_pcProtocol)
		{
			g_pcProtocol ++;
		}else
		{
			//UDP is also the default in case no protocol is inserted
			g_Protocol = SL_IPPROTO_UDP;
			g_Socket = sl_Socket(SL_AF_INET, SL_SOCK_DGRAM, SL_IPPROTO_UDP);
			goto parse_return;
		}
    }
    else
    {
        //Use default port
        g_ulPort = INTERPRETER_PORT;

        //UDP is also the default in case no protocol is inserted
		g_Protocol = SL_IPPROTO_UDP;
		g_Socket = sl_Socket(SL_AF_INET, SL_SOCK_DGRAM, SL_IPPROTO_UDP);
		goto parse_return;
    }

    //the destination port
    g_RemoteAddr.sa_data[0] = ((g_ulPort >> 8) & 0xFF);
    g_RemoteAddr.sa_data[1] = (g_ulPort & 0xFF);

    if ((strstr(g_pcProtocol, "TCP")) || (strstr(g_pcProtocol, "tcp"))) 
    {
        g_Protocol = SL_IPPROTO_TCP;
#ifndef SECURE_SOCKETS
        g_Socket = sl_Socket(SL_AF_INET, SL_SOCK_STREAM, SL_IPPROTO_TCP);
#else
        sl_Close(g_Socket);
        g_Socket = sl_Socket(SL_AF_INET, SL_SOCK_STREAM, SL_SEC_SOCKET);
        //SL_SEC_SOCKET
        if( g_Socket < 0 )
        {
           UART_PRINT("Failed to create socket\r\n");
           sl_Close(g_Socket);
           return g_Socket;
        }

      //
      // configure the socket as SSLV3.0
      //
      lRetVal = sl_SetSockOpt(g_Socket, SL_SOL_SOCKET, SL_SO_SECMETHOD,\
                                            &g_ucMethod, sizeof(g_ucMethod));
      if( lRetVal < 0 )
      {
         UART_PRINT("Failed to set socket options\r\n");
         sl_Close(g_Socket);
         return lRetVal;
      }
      //
      //configure the socket as RSA with RC4 128 SHA
      //
      lRetVal = sl_SetSockOpt(g_Socket, SL_SOL_SOCKET, SL_SO_SECURE_MASK, \
                                              &g_uiCipher, sizeof(g_uiCipher));
      if( lRetVal < 0 )
      {
         UART_PRINT("Failed to set socket options\r\n");
         sl_Close(g_Socket);
         return lRetVal;
      }

      //
      //configure the socket with GOOGLE CA certificate - for server verification
      //
      lRetVal = sl_SetSockOpt(g_Socket, SL_SOL_SOCKET, \
                               SL_SO_SECURE_FILES_PRIVATE_KEY_FILE_NAME, \
                               SL_SSL_SRV_KEY_FILE_NAME, \
                               strlen(SL_SSL_SRV_KEY_FILE_NAME));
      if( lRetVal < 0 )
      {
         UART_PRINT("Failed to set socket options\r\n");
         sl_Close(g_Socket);
         return lRetVal;
      }

        lRetVal = sl_SetSockOpt(g_Socket, SL_SOL_SOCKET, \
                                 SL_SO_SECURE_FILES_CERTIFICATE_FILE_NAME, \
                                 SL_SSL_SRV_CERT_FILE_NAME, \
                                 strlen(SL_SSL_SRV_CERT_FILE_NAME));
      if( lRetVal < 0 )
      {
         UART_PRINT("Failed to set socket options\r\n");
         sl_Close(g_Socket);
         return lRetVal;
      }

        lRetVal = sl_SetSockOpt(g_Socket, SL_SOL_SOCKET, \
                                      SL_SO_SECURE_FILES_CA_FILE_NAME, \
                                      SL_SSL_CLNT_CERT_FILE_NAME, \
                                      strlen(SL_SSL_CLNT_CERT_FILE_NAME));
      if( lRetVal < 0 )
      {
         UART_PRINT("Failed to set socket options\r\n");
         sl_Close(g_Socket);
         return lRetVal;
      }

#endif
    }
    else
    {
    	//UDP is also the default in case no protocol is inserted
		g_Protocol = SL_IPPROTO_UDP;
		g_Socket = sl_Socket(SL_AF_INET, SL_SOCK_DGRAM, SL_IPPROTO_UDP);
    }
parse_return:
    g_RemoteAddr.sa_family = SL_AF_INET;
    return SUCCESS;
}



//*****************************************************************************
//
//! ExecuteCommand
//!
//! @param  source
//!
//! @return 0 on success else Error Code
//!
//! @brief  Execute Commands received in local control/remote control modes
//
//*****************************************************************************
long ExecuteCommand(OperationMode_e source)
{
    char *pMode;
    SlFdSet_t ReadFds;
    SlFdSet_t ActiveReadFds;
    unsigned long ulToken;
    long lRetVal;


    if (strstr((char*)g_ucUARTBuffer, g_cmd_help[1]))
    {
        char str[3][20],*pch;
        int i = 0,CmdIdx;
        for(i = 0;i < 3;i++)
        {
            memset(str[i],0,strlen(str[i]));
        }
        i= 0;
        pch = strtok ((char*)g_ucUARTBuffer," ");
        while (pch != NULL)
        {
           strcpy(str[i],pch);
           i++;
           pch = strtok (NULL, " ");
        }
        CmdIdx = GetCmdIndex(str[1]);
        if(CmdIdx > (N_HELP_ENTRIES - 2))
        {
            return(-1);
        }
        memset(g_cmd_help[CmdIdx],0,20);
        strncpy((char*)g_cmd_help[CmdIdx], str[2], strlen(str[2]));
        g_cmd_help[CmdIdx][strlen(str[2])] = 0;
        return(0);

    }

    //help
    if (strstr((char*)g_ucUARTBuffer, g_cmd_help[0]))
    {
        PrintCommandMenu();
        UART_PRINT(COMMAND_PROMPT);
        return(0);
    }

    //resume interpreter mode
    if (strstr((char*)g_ucUARTBuffer,g_cmd_help[2]))
    {
        if (source == LOCAL_CONTROL_MODE)
        {
            g_UartMode = INTERPRETER_MODE;            
            UART_PRINT(EXITED_LOCAL_CONTROL_MODE);
        }
        else if (source == REMOTE_CONTROL_MODE)
        {
            g_WlanRxMode = INTERPRETER_MODE;             
            UART_PRINT(EXITED_CONTROLLED_BY_REMOTE_MODE);
        }

        UART_PRINT(COMMAND_PROMPT);

        return(0);
    }          

    //mode
    if (strstr((char*)g_ucUARTBuffer, g_cmd_help[3]))
    {
        pMode = strchr((char*)g_ucUARTBuffer, ' '); 

        if (pMode)
        {
            pMode ++;
            if (*pMode == 'B')
			{
				g_InterpreterMode = BINARY_INTERPRETER_MODE;
				UART_PRINT(ENTERED_BINARY_INTERPRETER_MODE);
			}
			else if (*pMode == 'H')
			{
				g_InterpreterMode = HEXA_INTERPRETER_MODE;
				UART_PRINT(ENTERED_HEXA_INTERPRETER_MODE);
			}
			else
			{
				g_InterpreterMode = ASCII_INTERPRETER_MODE;
				UART_PRINT(ENTERED_ASCII_INTERPRETER_MODE);
			}
        }
        else
        {
        	g_InterpreterMode = ASCII_INTERPRETER_MODE;
        	UART_PRINT(ENTERED_ASCII_INTERPRETER_MODE);
        }

        UART_PRINT(COMMAND_PROMPT);

        return(0);
    }

    //wrap
    if (strstr((char*)g_ucUARTBuffer, g_cmd_help[4]))
    {
        pMode = strchr((char*)g_ucUARTBuffer, ' '); 

        if (pMode)
        {
            pMode ++;
        }

        if (*pMode == 'B')
        {
            g_WrapMode = BYTE_LEVEL_TERMINAL_MODE;
            UART_PRINT(ENTERED_BYTE_LEVEL_TERMINAL_MODE);
        }    
        else
        {
            g_WrapMode = LINE_LEVEL_TERMINAL_MODE;
            UART_PRINT(ENTERED_LINE_LEVEL_TERMINAL_MODE);
        }

        UART_PRINT(COMMAND_PROMPT);

        return(0);
    }

    //wlan_connect [ssid] 
    if (strstr((char*)g_ucUARTBuffer, g_cmd_help[5]))
    {
        ParseWlanConnectString();
        if(eapParams.User != NULL)
        {
            lRetVal = sl_WlanConnect(g_pcSsid, g_ulSsidLen, 0, \
                                              &g_SecParams, &eapParams);
            ASSERT_ON_ERROR(lRetVal);
            lRetVal = sl_WlanProfileAdd(g_pcSsid, g_ulSsidLen, NULL,\
                                       &g_SecParams , &eapParams, 6, 0);
            ASSERT_ON_ERROR(lRetVal);
        }
        else
        {
            lRetVal = sl_WlanConnect(g_pcSsid, g_ulSsidLen, 0, &g_SecParams, 0);
            ASSERT_ON_ERROR(lRetVal);
            lRetVal = sl_WlanProfileAdd(g_pcSsid, g_ulSsidLen, NULL, \
                                              &g_SecParams , NULL, 6, 0);
            ASSERT_ON_ERROR(lRetVal);
        }

        return(0);
    }

    //disonnect
    if (strstr((char*)g_ucUARTBuffer, g_cmd_help[6]))
    {
        lRetVal = sl_Close(g_Socket);
        ASSERT_ON_ERROR(lRetVal);
    }

    //connect [IP | name] [port] [TCP | UDP] 
    if (strstr((char*)g_ucUARTBuffer,g_cmd_help[7]))
    {
        if ((!IS_CONNECTED(g_ulStatus)))
        {
            UART_PRINT(NOT_CONNECTED_TO_AP);
            UART_PRINT(COMMAND_PROMPT);
            return(0);
        }

        lRetVal = ParseConnectString();
        ASSERT_ON_ERROR(lRetVal);
        
        //by default, define the socket as non-blocking
        if (g_Protocol == SL_IPPROTO_TCP)
        {
            g_Status = sl_Connect(g_Socket, &g_RemoteAddr, sizeof(g_RemoteAddr));
            if( g_Status < 0 )
            {
                    UART_PRINT("Failed to set socket options\r\n");
                    sl_Close(g_Socket);
                    return g_Status;
            }
            g_NonBlocking = 1;
            g_Status = sl_SetSockOpt(g_Socket, SL_SOL_SOCKET, \
                                        SL_SO_NONBLOCKING, &g_NonBlocking, \
                                        sizeof(g_NonBlocking));
            if( g_Status < 0 )
            {
                UART_PRINT("Failed to set socket options\r\n");
                sl_Close(g_Socket);
                return g_Status;
            }
        }
        return(0);
    }

    //send [dest] [data] 
    if (strstr((char*)g_ucUARTBuffer,g_cmd_help[8]))
    {
        if ((!IS_CONNECTED(g_ulStatus)))
        {
            UART_PRINT(NOT_CONNECTED_TO_AP);
            UART_PRINT(COMMAND_PROMPT);
            return(0);
        }

        lRetVal = ParseSendString();
        ASSERT_ON_ERROR(lRetVal);

        if (g_Protocol == SL_IPPROTO_TCP)
        {    
            g_BytesTx = sl_Send(g_Socket, g_pcData, strlen(g_pcData), 0);                   
        }
        else
        {
            g_BytesTx = sl_SendTo(g_Socket, g_pcData,  strlen(g_pcData), 0, \
                                          &g_RemoteAddr, sizeof(SlSockAddr_t));
        }

        return(0);
    }

    //recv [dest] [length] [timeout] 
    if (strstr((char*)g_ucUARTBuffer,g_cmd_help[9]))
    {
        if ((!IS_CONNECTED(g_ulStatus)))
        {
            UART_PRINT(NOT_CONNECTED_TO_AP);
            UART_PRINT(COMMAND_PROMPT);
            return(0);
        }

        lRetVal = ParseRecvString();
        ASSERT_ON_ERROR(lRetVal);

        if (g_Protocol == SL_IPPROTO_TCP)
        {    
            g_BytesRx = sl_Recv(g_Socket, uBufRx.g_RecvBuf, g_ulLength, 0);
        }
        else
        {
            SL_FD_ZERO(&ReadFds);
            SL_FD_SET(g_Socket, &ReadFds);

            ActiveReadFds = ReadFds; 
            g_RetVal = sl_Select(g_Socket + 1, &ActiveReadFds, NULL, \
                                       NULL, &g_timeout);

            if (g_RetVal > 0)
            {
                g_BytesRx = sl_RecvFrom(g_Socket, uBufRx.g_RecvBuf, g_ulLength,\
                                          0, &g_RemoteAddr, &g_AddrLen);
            }
        }

        if (g_BytesRx > 0)
        {
            UART_PRINT(uBufRx.g_RecvBuf);
        }

        return(0);
    }         

    //ping [IP | name] [number] 
    if (strstr((char*)g_ucUARTBuffer,g_cmd_help[10]))
    {
        if ((!IS_CONNECTED(g_ulStatus)))
        {
            UART_PRINT(NOT_CONNECTED_TO_AP);
            UART_PRINT(COMMAND_PROMPT);
            return(0);
        }
        lRetVal = ParsePingString();        
        ASSERT_ON_ERROR(lRetVal);
        return(0);
    }

    //mDNS [server(0)/client(1)]
    if (strstr((char*)g_ucUARTBuffer, g_cmd_help[11]))
    {
        ParsemDNSString();
        if(g_imDNSMode == 0)
        {
            lRetVal = sl_NetAppMDNSUnRegisterService(CC3200_DNS_NAME,\
                                        (unsigned char)strlen(CC3200_DNS_NAME));
            //ASSERT_ON_ERROR(lRetVal);
            lRetVal = sl_NetAppMDNSRegisterService(CC3200_DNS_NAME,\
                                   (unsigned char)strlen(CC3200_DNS_NAME),\
                                   "multicast",\
                                   (unsigned char)strlen("multicast"),\
                                    INTERPRETER_PORT,2000,1);
            ASSERT_ON_ERROR(lRetVal);
            g_mDNSStatus = 0;

            lRetVal = sl_FsOpen("mDNSStatusFile.txt",FS_MODE_OPEN_WRITE, \
                                     &ulToken,&g_isFileHandle);
            ASSERT_ON_ERROR(lRetVal);
            lRetVal = sl_FsWrite(g_isFileHandle, 0, \
                                  (unsigned char *)&g_imDNSMode, \
                                  sizeof(g_imDNSMode));
            ASSERT_ON_ERROR(lRetVal);
            lRetVal = sl_FsClose(g_isFileHandle,0,0,NULL);
            ASSERT_ON_ERROR(lRetVal);
        }
        else if(g_imDNSMode == 1)
        {
            g_mDNSStatus = 1;
            lRetVal = sl_NetAppMDNSUnRegisterService(CC3200_DNS_NAME,\
                                       (unsigned char)strlen(CC3200_DNS_NAME));
            //ASSERT_ON_ERROR(lRetVal);

            lRetVal = sl_FsOpen("mDNSStatusFile.txt",FS_MODE_OPEN_WRITE, \
                                                   &ulToken,&g_isFileHandle);
            ASSERT_ON_ERROR(lRetVal);
            lRetVal = sl_FsWrite(g_isFileHandle, 0, \
                                 (unsigned char *)&g_imDNSMode, \
                                 sizeof(g_imDNSMode));
            ASSERT_ON_ERROR(lRetVal);
            lRetVal = sl_FsClose(g_isFileHandle,0,0,NULL);
            ASSERT_ON_ERROR(lRetVal);
        }
        return(0);
    }

    UNUSED(lRetVal);

    //if here, the command is illegal
    UART_PRINT(ILLEGAL_COMMAND);
    UART_PRINT(COMMAND_PROMPT);

    return(-1);
}

//*****************************************************************************
//
//! StartUdpServer
//!
//! @param  none
//!
//! @return 0 on success else error code 
//!
//! @brief  Start Udp Server socket
//
//*****************************************************************************
long StartUdpServer()
{
    g_AddrLen = sizeof(SlSockAddrIn_t);

    g_LocalAddr.sa_family = SL_AF_INET;        
    g_LocalAddr.sa_data[0] = ((INTERPRETER_PORT >> 8) & 0xFF);
    g_LocalAddr.sa_data[1] = (INTERPRETER_PORT & 0xFF);

    //all 0 => Own IP address
    memset(&g_LocalAddr.sa_data[2], 0, 4);

    g_SocketUdpServer = sl_Socket(SL_AF_INET, SL_SOCK_DGRAM, SL_IPPROTO_UDP);
    ASSERT_ON_ERROR(g_SocketUdpServer);

    g_NonBlocking = 0;
    g_RetVal = sl_SetSockOpt(g_SocketUdpServer, SL_SOL_SOCKET, \
                                  SL_SO_NONBLOCKING, &g_NonBlocking, \
                                  sizeof(g_NonBlocking));
    ASSERT_ON_ERROR(g_RetVal);

    g_MulticastIp.imr_multiaddr = sl_Htonl(INTERPRETER_IGMP_IP_HEXA); 
    g_MulticastIp.imr_interface = 0x00000000;

    //65 is IP_ADD_MEMBERSHIP
    g_RetVal = sl_SetSockOpt(g_SocketUdpServer, SL_IPPROTO_IP, 65, \
                                    &g_MulticastIp, sizeof(IpMreq_t));
    ASSERT_ON_ERROR(g_RetVal);
    
    g_RetVal = sl_Bind(g_SocketUdpServer, (SlSockAddr_t *)&g_LocalAddr, \
                                  sizeof(g_LocalAddr));
    ASSERT_ON_ERROR(g_RetVal);

    g_RemoteAddr.sa_family = SL_AF_INET;
    g_RemoteAddr.sa_data[0] = ((INTERPRETER_PORT >> 8) & 0xFF);
    g_RemoteAddr.sa_data[1] = (INTERPRETER_PORT & 0xFF);

    SL_FD_ZERO(&g_ReadFds);
    SL_FD_SET(g_SocketUdpServer, &g_ReadFds);

    return SUCCESS;
}


//*****************************************************************************
//
//! StartTcpServer
//!
//! @param  none
//!
//! @return 0 on success else error code
//!
//! @brief  Start Tcp Server socket
//
//*****************************************************************************
long StartTcpServer()
{
#ifdef SECURE_SOCKETS
    int lRetVal;
#endif

    g_LocalAddr.sa_family = SL_AF_INET;        
    g_LocalAddr.sa_data[0] = ((INTERPRETER_PORT >> 8) & 0xFF);
    g_LocalAddr.sa_data[1] = (INTERPRETER_PORT & 0xFF);

    //all 0 => Own IP address
    memset(&g_LocalAddr.sa_data[2], 0, 4);

#ifndef SECURE_SOCKETS
    g_SocketTcpServer = sl_Socket(SL_AF_INET, SL_SOCK_STREAM, SL_IPPROTO_TCP); 

#else
    g_SocketTcpServer= sl_Socket(SL_AF_INET, SL_SOCK_STREAM,SL_SEC_SOCKET);
    if( g_SocketTcpServer < 0 )
    {
       UART_PRINT("Failed to create socket\r\n");
       sl_Close(g_Socket);
       return -1;
    }

      //
      // configure the socket as SSLV3.0
      //
      lRetVal = sl_SetSockOpt(g_SocketTcpServer, SL_SOL_SOCKET, \
                             SL_SO_SECMETHOD, &g_ucMethod, sizeof(g_ucMethod));
      if( lRetVal < 0 )
      {
         UART_PRINT("Failed to set socket options\r\n");
         sl_Close(g_Socket);
         return lRetVal;
      }
      //
      //configure the socket as RSA with RC4 128 SHA
      //
      lRetVal = sl_SetSockOpt(g_SocketTcpServer, SL_SOL_SOCKET, \
                               SL_SO_SECURE_MASK, &g_uiCipher, \
                               sizeof(g_uiCipher));
      if( lRetVal < 0 )
      {
         UART_PRINT("Failed to set socket options\r\n");
         sl_Close(g_Socket);
         return lRetVal;
      }

      //
      //configure the socket with CA certificate - for server verification
      //
      lRetVal = sl_SetSockOpt(g_Socket, SL_SOL_SOCKET, \
                                    SL_SO_SECURE_FILES_PRIVATE_KEY_FILE_NAME,\
                                    SL_SSL_SRV_KEY_FILE_NAME, \
                                    strlen(SL_SSL_SRV_KEY_FILE_NAME));
      if( lRetVal < 0 )
      {
         UART_PRINT("Failed to set socket options\r\n");
         sl_Close(g_Socket);
         return lRetVal;
      }     

      lRetVal = sl_SetSockOpt(g_Socket, SL_SOL_SOCKET, \
                                    SL_SO_SECURE_FILES_CERTIFICATE_FILE_NAME,\
                                    SL_SSL_SRV_CERT_FILE_NAME, \
                                    strlen(SL_SSL_SRV_CERT_FILE_NAME));
      if( lRetVal < 0 )
      {
         UART_PRINT("Failed to set socket options\r\n");
         sl_Close(g_Socket);
         return lRetVal;
      }

      lRetVal = sl_SetSockOpt(g_Socket, SL_SOL_SOCKET, \
                                       SL_SO_SECURE_FILES_CA_FILE_NAME, \
                                       SL_SSL_CLNT_CERT_FILE_NAME, \
                                       strlen(SL_SSL_CLNT_CERT_FILE_NAME));
      if( lRetVal < 0 )
      {
         UART_PRINT("Failed to set socket options\r\n");
         sl_Close(g_Socket);
         return lRetVal;
      }
#endif
    g_NonBlocking = 1;
    g_Status = sl_SetSockOpt(g_SocketTcpServer, SL_SOL_SOCKET, \
                               SL_SO_NONBLOCKING, &g_NonBlocking, \
                               sizeof(g_NonBlocking));
    if( g_Status < 0 )
    {
        UART_PRINT("Failed to set socket options\r\n");
        sl_Close(g_Socket);
        return FAILURE;
    }

    if (sl_Bind(g_SocketTcpServer, &g_LocalAddr, sizeof(g_LocalAddr)) < 0)
    {
        UART_PRINT(" Bind Error\n\r");
        sl_Close(g_SocketTcpServer);
        return FAILURE;
    }

    if (sl_Listen(g_SocketTcpServer, 0) < 0)
    {
        UART_PRINT(" Listen Error\n\r");
        sl_Close(g_SocketTcpServer);
        return FAILURE;
    }

    return SUCCESS;
}



//*****************************************************************************
//
//! InitialiseCmdMenu
//!
//! @param  none
//!
//! @return 0 on success else error code
//!
//! @brief  Initialize the Cmd Menu for supporting Rename
//
//*****************************************************************************

long InitialiseCmdMenu()
{
    strcpy(g_cmd_help[0],"help");
    strcpy(g_cmd_help[1],"rename");
    strcpy(g_cmd_help[2],"exit");
    strcpy(g_cmd_help[3],"mode");
    strcpy(g_cmd_help[4],"wrap");
    strcpy(g_cmd_help[5],"wlan_connect");
    strcpy(g_cmd_help[6],"disconnect");
    strcpy(g_cmd_help[7],"connect");
    strcpy(g_cmd_help[8],"send");
    strcpy(g_cmd_help[9],"recv");
    strcpy(g_cmd_help[10],"ping");
    strcpy(g_cmd_help[11],"mDNS");

    return SUCCESS;
}


//*****************************************************************************
//
//! GetCmdIndex
//!
//! @param  string
//!
//! @return return cmd index else number of enetries as error
//!
//! @brief  Get Cmd Index based on the string received
//
//*****************************************************************************


int GetCmdIndex( char *str)
{
    int i;
    if(str == NULL)
    {
        return N_HELP_ENTRIES;
    }
    for(i = 0;i<=N_HELP_ENTRIES;i++)
    {
        if(strcmp(g_cmd_help[i],str) == 0)
            return i;
    }
    return i;
}

//*****************************************************************************
//
//! InitialConfiguration
//!
//! @param  string
//!
//! @return 0 on success else error code 
//!
//! @brief  Configuring the Serial Wifi to the initial state based on stored
//!            configuration.
//
//*****************************************************************************

long  InitialConfiguration(void)
{
    int lRetVal;
    unsigned long ulToken;

    //start Interpreter Mode
    lRetVal = StartInterpreterMode(START_INTERPRETER_SERVERS_MODE);
    ASSERT_ON_ERROR(lRetVal);

    g_mDNSStatus = 1;

    lRetVal = sl_FsOpen("mDNSStatusFile.txt",FS_MODE_OPEN_READ, \
                                        &ulToken,&g_isFileHandle);
    if(lRetVal < 0) //file does'nt exist
    {

        //create a file
        lRetVal = sl_FsOpen("mDNSStatusFile.txt",\
                           FS_MODE_OPEN_CREATE(1024,_FS_FILE_OPEN_FLAG_COMMIT| \
                                     _FS_FILE_PUBLIC_WRITE), \
                                     &ulToken,&g_isFileHandle);
        ASSERT_ON_ERROR(lRetVal);
        
        lRetVal = sl_FsClose(g_isFileHandle,0,0,NULL);        
        ASSERT_ON_ERROR(lRetVal);
        g_imDNSMode = 1;
        lRetVal = sl_FsOpen("mDNSStatusFile.txt",\
                               FS_MODE_OPEN_WRITE,\
                               &ulToken,&g_isFileHandle);
        if(lRetVal < 0)
        {
            UART_PRINT("Unable to open file\n\r");
            LOOP_FOREVER();
        }
        lRetVal = sl_FsWrite(g_isFileHandle, 0, \
                             (unsigned char *)&g_imDNSMode,\
                              sizeof(g_imDNSMode));
        if(lRetVal < 0)
        {
            UART_PRINT("Unable to write file\n\r");
            LOOP_FOREVER();
        }
        lRetVal = sl_FsClose(g_isFileHandle,0,0,NULL);
        if(lRetVal < 0)
        {
            UART_PRINT("Unable to close file\n\r");
            LOOP_FOREVER();
        }
    }
    else
    {
        lRetVal = sl_FsRead(g_isFileHandle,0, (unsigned char *)&g_imDNSMode, \
                                  sizeof(g_imDNSMode));
        ASSERT_ON_ERROR(lRetVal);
        
        if(g_imDNSMode == 0)
        {
            lRetVal = sl_NetAppMDNSUnRegisterService(CC3200_DNS_NAME,\
                                       (unsigned char)strlen(CC3200_DNS_NAME));
            //ASSERT_ON_ERROR(lRetVal);

            lRetVal = sl_NetAppMDNSRegisterService(CC3200_DNS_NAME,\
                                      (unsigned char)strlen(CC3200_DNS_NAME),\
                                      "multicast",\
                                      (unsigned char)strlen("multicast"),\
                                      INTERPRETER_PORT,2000,1);
            ASSERT_ON_ERROR(lRetVal);

            g_mDNSStatus = 0;
        }

        lRetVal = sl_FsClose(g_isFileHandle,0,0,NULL);    
        ASSERT_ON_ERROR(lRetVal);
    }

    g_ConfigurationDone = 1;
    return SUCCESS;
}

//*****************************************************************************
//
//! InitDevice
//!
//! @param  none
//!
//! @return 0 on success else error code
//!
//! @brief  Initialise the device to default state without deleting the stored
//!            profiles.
//
//*****************************************************************************

static long InitDevice()
{
    SlVersionFull   ver = {0};

    unsigned char ucVal = 1;
    unsigned char ucConfigOpt = 0;
    unsigned char ucConfigLen = 0;
    unsigned char ucPower = 0;

    long lRetVal = -1;
    long lMode = -1;

    lMode = sl_Start(0, 0, 0);
    ASSERT_ON_ERROR(lMode);

    // Get the device's version-information
    ucConfigOpt = SL_DEVICE_GENERAL_VERSION;
    ucConfigLen = sizeof(ver);
    lRetVal = sl_DevGet(SL_DEVICE_GENERAL_CONFIGURATION, &ucConfigOpt, \
                                 &ucConfigLen, (unsigned char *)(&ver));
    ASSERT_ON_ERROR(lRetVal);

    UART_PRINT("Host Driver Version: %s\n\r",SL_DRIVER_VERSION);
    UART_PRINT("Build Version %d.%d.%d.%d.31.%d.%d.%d.%d.%d.%d.%d.%d\n\r",
    ver.NwpVersion[0],ver.NwpVersion[1],ver.NwpVersion[2],ver.NwpVersion[3],
    ver.ChipFwAndPhyVersion.FwVersion[0],ver.ChipFwAndPhyVersion.FwVersion[1],
    ver.ChipFwAndPhyVersion.FwVersion[2],ver.ChipFwAndPhyVersion.FwVersion[3],
    ver.ChipFwAndPhyVersion.PhyVersion[0],ver.ChipFwAndPhyVersion.PhyVersion[1],
    ver.ChipFwAndPhyVersion.PhyVersion[2],ver.ChipFwAndPhyVersion.PhyVersion[3]);

    // Set connection policy to Auto + SmartConfig 
    // (Device's default connection policy)
    lRetVal = sl_WlanPolicySet(SL_POLICY_CONNECTION, \
                             SL_CONNECTION_POLICY(1, 0, 0, 0, 1), NULL, 0);
    ASSERT_ON_ERROR(lRetVal);

    // If the device is not in station-mode, try putting it in staion-mode
    if (ROLE_STA != lMode)
    {
        if (ROLE_AP == lMode)
        {
            // If the device is in AP mode, we need to wait for this
            // event before doing anything
            while(!IS_IP_ACQUIRED(g_ulStatus))
            {
#ifndef SL_PLATFORM_MULTI_THREADED
              _SlNonOsMainLoopTask();
#endif
            }
        }

        // Switch to STA role and restart
        lRetVal = sl_WlanSetMode(ROLE_STA);
        ASSERT_ON_ERROR(lRetVal);

        lRetVal = sl_Stop(SL_STOP_TIMEOUT);
        ASSERT_ON_ERROR(lRetVal);

        lRetVal = sl_Start(0, 0, 0);
        ASSERT_ON_ERROR(lRetVal);

        // Check if the device is in station again
        if (ROLE_STA != lRetVal)
        {
            // We don't want to proceed if the device is not coming up in sta-mode
            return DEVICE_NOT_IN_STATION_MODE;
        }
    }

    //
    // Device in station-mode. Disconnect previous connection if any
    // The function returns 0 if 'Disconnected done', negative number if already disconnected
    // Wait for 'disconnection' event if 0 is returned, Ignore other return-codes
    //
    lRetVal = sl_WlanDisconnect();
    if(0 == lRetVal)
    {
        // Wait
        while(IS_CONNECTED(g_ulStatus))
        {
#ifndef SL_PLATFORM_MULTI_THREADED
              _SlNonOsMainLoopTask();
#endif
        }
    }

    // Enable DHCP client
    lRetVal = sl_NetCfgSet(SL_IPV4_STA_P2P_CL_DHCP_ENABLE,1,1,&ucVal);
    ASSERT_ON_ERROR(lRetVal);

    // Disable scan
    ucConfigOpt = SL_SCAN_POLICY(0);
    lRetVal = sl_WlanPolicySet(SL_POLICY_SCAN , ucConfigOpt, NULL, 0);
    ASSERT_ON_ERROR(lRetVal);

    // Set Tx power level for station mode
    // Number between 0-15, as dB offset from max power - 0 will set maximum power
    ucPower = 0;
    lRetVal = sl_WlanSet(SL_WLAN_CFG_GENERAL_PARAM_ID, \
                              WLAN_GENERAL_PARAM_OPT_STA_TX_POWER, 1, \
                              (unsigned char *)&ucPower);
    ASSERT_ON_ERROR(lRetVal);

    // Set PM policy to normal
    lRetVal = sl_WlanPolicySet(SL_POLICY_PM , SL_NORMAL_POLICY, NULL, 0);
    ASSERT_ON_ERROR(lRetVal);

    // Unregister mDNS services

    lRetVal = sl_Stop(SL_STOP_TIMEOUT);
    ASSERT_ON_ERROR(lRetVal);

    InitializeAppVariables();

    return lRetVal; // Success
}


//*****************************************************************************
//
//! ConfigureSimpleLinkForSerialWifi
//!
//! @param  uiMode
//!
//! @return 0 on success else error code 
//!
//! @brief  Configuring the state of the CC3200 simplelink device for Serial Wifi
//
//*****************************************************************************
static long ConfigureSimpleLinkForSerialWifi(unsigned int uiMode)
{
     long retVal = -1;

    // Reset CC3200 Network State Machine
    InitializeAppVariables();

    retVal = InitDevice();
    if(retVal < 0)
    {
        if (DEVICE_NOT_IN_STATION_MODE == retVal)
            UART_PRINT("Failed to configure the device in its default state\n\r");

        ASSERT_ON_ERROR(retVal);
    }

    UART_PRINT("Device is configured in default state \n\r");

    CLR_STATUS_BIT_ALL(g_ulStatus);

    //
    // Assumption is that the device is configured in station mode already
    // and it is in its default state
    //
    retVal = sl_Start(NULL,NULL,NULL);

    if (retVal < 0 || retVal != uiMode)
    {
        UART_PRINT("Failed to start the device \n\r");
        ASSERT_ON_ERROR(retVal);
    }

    UART_PRINT("Started SimpleLink Device: STA Mode\n\r");
    return SUCCESS;
}


//*****************************************************************************
//
//! This function updates the date and time of CC3200.
//!
//! \param None
//!
//! \return
//!     0 for success, negative otherwise
//!
//*****************************************************************************

static int set_time()
{
    long retVal;

    g_time.tm_day = DATE;
    g_time.tm_mon = MONTH;
    g_time.tm_year = YEAR;
    g_time.tm_sec = HOUR;
    g_time.tm_hour = MINUTE;
    g_time.tm_min = SECOND;

    retVal = sl_DevSet(SL_DEVICE_GENERAL_CONFIGURATION,
                          SL_DEVICE_GENERAL_CONFIGURATION_DATE_TIME,
                          sizeof(SlDateTime),(unsigned char *)(&g_time));

    ASSERT_ON_ERROR(retVal);
    return SUCCESS;
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
