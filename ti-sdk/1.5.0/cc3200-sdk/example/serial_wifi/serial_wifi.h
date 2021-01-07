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
#ifndef extern
#include "simplelink.h"
#endif

//Driverlib includes
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "uart.h"
#include "prcm.h"
#include "rom.h"
#include "rom_map.h"
#include "interrupt.h"

//Common interface includes
#include "uart_if.h"

//
// typedef declarations
//
typedef enum
{
	START_INTERPRETER_SERVERS_MODE,
	START_INTERPRETER_TCP_CLIENT_MODE,
	START_INTERPRETER_UDP_CLIENT_MODE
}InterpreterRole_e;

typedef struct
{
	long imr_multiaddr;     /* The IPv4 multicast address to join. */
	long imr_interface;     /* The interface to use for this group. */
}IpMreq_t;

typedef struct
{
       /* time */
       unsigned long tm_sec;
       unsigned long tm_min;
       unsigned long tm_hour;
       /* date */
       unsigned long tm_day;
       unsigned long tm_mon;
       unsigned long tm_year;
       unsigned long tm_week_day; //not required
       unsigned long tm_year_day; //not required
       unsigned long reserved[3];
}SlDateTime;

typedef enum
{
	BYTE_LEVEL_TERMINAL_MODE,
	LINE_LEVEL_TERMINAL_MODE
}WrapMode_e;

typedef enum
{
	INTERPRETER_MODE,
	LOCAL_CONTROL_MODE,
	REMOTE_CONTROL_MODE,
	CONTROL_REMOTE_MODE
}OperationMode_e;

typedef enum
{
	ASCII_INTERPRETER_MODE,
	BINARY_INTERPRETER_MODE,
	HEXA_INTERPRETER_MODE
}InterpreterMode_e;




#define SECURE_SOCKETS
#define ENT_SEC

#define INTERPRETER_PORT 					(224)
#define INTERPRETER_IGMP_IP					"224.224.224.224"
#define INTERPRETER_IGMP_IP_HEXA            (0xe0e0e0e0)
#define INTERPRETER_RECV_LENGTH				(1)
#define INTERPRETER_RECV_TIMEOUT			(5000)
#define PING_INTERVAL_TIME					(1000)
#define PING_REQ_TIMEOUT					(3000)
#define INTERPRETER_PING_SIZE				(100)
#define MAX_LENGTH_COMMAND					(20)
#define TERMINATE_INTERPRETER_MODE          (-2)

#define DEFAULT_TEST_AP_SSID				"cc3200demo"
#define DEFAULT_TEST_SERVER_REMOTE_IP		"192.168.0.194"

#define COMMAND_PROMPT						"\r\n>>> "
#define ATTEMPTING_TO_CONNECT_TO_AP			"Attempting to auto connect to AP"
#define ATTEMPTING_TO_ACQUIRE_IP			"Attempting to acquire IP"
#define NOT_CONNECTED_TO_AP					"Not connected to AP"
#define ENTERED_LOCAL_CONTROL_MODE			"Entered LOCAL CONTROL mode"
#define EXITED_LOCAL_CONTROL_MODE			"Exited LOCAL CONTROL mode"
#define ENTERED_CONTROL_REMOTE_MODE			"Entered REMOTE CONTROL mode"
#define EXITED_CONTROL_REMOTE_MODE			"Exited REMOTE CONTROL mode"
#define ENTERED_CONTROLLED_BY_REMOTE_MODE	"Entered CONTROLLED BY REMOTE mode"
#define EXITED_CONTROLLED_BY_REMOTE_MODE	"Exited CONTROLLED BY REMOTE mode"
#define REOPENED_TCP_UDP_SERVERS        	"TCP and UDP servers are currently ACTIVE"
#define ILLEGAL_COMMAND						"Illegal command"
#define ILLEGAL_INPUT						"Illegal input"
#define EXIT_INTERPRETER_MODE				"Exited INTERPRETER mode"
#define ENTERED_INTERPRETER_MODE			"Entered INTERPRETER mode"
#define ENTERED_BYTE_LEVEL_TERMINAL_MODE	"Entered BYTE LEVEL terminal mode"
#define ENTERED_LINE_LEVEL_TERMINAL_MODE	"Entered LINE LEVEL terminal mode"
#define ENTERED_ASCII_INTERPRETER_MODE	    "Entered ASCII interpreter mode"
#define ENTERED_BINARY_INTERPRETER_MODE	    "Entered BINARY interpreter mode"
#define ENTERED_HEXA_INTERPRETER_MODE	    "Entered HEXA interpreter mode"
#define TCP_CONNECTION_ESTABLISHED			"TCP connection established"
#define REMOTE_CLOSED_TCP_CONNECTION		"Remote closed TCP connection"
#define PING_STOPPED                        "Ping stopped"
#define PING_SUCCESSFUL						"Ping successful"


#define SL_SSL_CA_CERT		20	/* CA certificate file ID */
#define SL_SSL_SRV_KEY		129	/* Server key file ID */
#define SL_SSL_SRV_CERT		130	/* Server certificate file ID */
#define SL_SSL_CLNT_CERT	131	/* Client certificate file ID */

#define SL_SSL_SRV_KEY_FILE_NAME		"/cert/129.der"
#define SL_SSL_SRV_CERT_FILE_NAME		"/cert/130.der"
#define SL_SSL_CLNT_CERT_FILE_NAME		"/cert/131.der"

#define N_HELP_ENTRIES 12
#define SL_USER_FILEID          199
#define SL_DEVICE_SFLASH        2
#define SL_MAX_FILE_SIZE        64L*1024L       /* 64KB file */
#define SL_STOP_TIMEOUT         200

#define BUF_SIZE 						(100)
#define MCU_RX_BUFFER_OVERHEAD_SIZE		(32)

#define UART_BAUD_RATE  				115200
#define SYSCLK          				80000000
#define CONSOLE        					UARTA0_BASE

#define DATE                10    /* Current Date */
#define MONTH               7     /* Month 1-12 */
#define YEAR                2014  /* Current year */
#define HOUR                12    /* Time - hours */
#define MINUTE              11    /* Time - minutes */
#define SECOND              0     /* Time - seconds */



/****************** Globals declaration ******************/

//*****************************************************************************
// global variables containing the string entered
//*****************************************************************************
extern unsigned char g_ucUARTBuffer[80];
extern unsigned char g_ucUARTRecvBuffer[80];

//*****************************************************************************
// global variable indicating a command has entered
//*****************************************************************************
extern unsigned int uiUartline;

//*****************************************************************************
// Global variable indicating input length
//*****************************************************************************
extern unsigned int ilength
#ifdef extern
=1
#endif
;
extern unsigned int g_ConfigurationDone
#ifdef extern
=0
#endif
;

extern union
{
	char g_RecvBuf[BUF_SIZE];
	unsigned long demobuf[BUF_SIZE/4];
} uBufRx;



extern union
{
	char g_sl_SendBuf[BUF_SIZE];
	unsigned long demobuf[BUF_SIZE/4];
} uBufTx;


extern union
{
	unsigned char g_ucRxBuffer[MCU_RX_BUFFER_OVERHEAD_SIZE];
	unsigned long demobuf[MCU_RX_BUFFER_OVERHEAD_SIZE/4];
} uBufuRx;




extern unsigned char g_ucDispBuffer[150];
extern unsigned long g_ulSsidLen;
extern unsigned long g_ulPort;
extern unsigned long g_ulIp;
extern unsigned long g_ulLength;
extern unsigned long g_ulPingAttempts;
extern unsigned long g_ulPingIp;
extern SlSecParams_t 		g_SecParams;
//#ifdef ENT_SEC
extern SlSecParamsExt_t eapParams;
//#endif

extern SlSocklen_t 		g_AddrLen;
extern SlSockAddr_t 		g_RemoteAddr;
extern SlSockAddr_t 		g_LocalAddr;
extern IpMreq_t 			g_MulticastIp;
extern SlDateTime g_time;
extern unsigned char    g_ucMethod
#ifdef extern
= SL_SO_SEC_METHOD_SSLV3
#endif
;
extern unsigned int g_uiCipher
#ifdef extern
= SL_SEC_MASK_SSL_RSA_WITH_RC4_128_SHA
#endif
;
extern SlSockSecureFiles_t	g_sockSecureFiles;
extern signed char *g_pcSsid;
extern char *g_pcIp, *g_pcPort, *g_pcProtocol, *g_pcData, *g_pcLength, *g_pcTimeout, *g_pcPingAttempts,*g_mDNSMode, *g_UserName,*g_Key;
extern OperationMode_e     g_WlanRxMode;
extern SlTimeval_t 		g_timeout;
extern SlFdSet_t 			g_ReadFds, g_WriteFds, g_ActiveReadFds;
extern int 				g_Protocol, g_Type, g_BytesTx
#ifdef extern
= 0
#endif
, g_BytesRx
#ifdef extern
= 0
#endif
, g_Status;
extern int 				g_TcpConnected
#ifdef extern
= 0
#endif
, g_RetVal, g_NonBlocking;
extern volatile int 		g_Socket, g_SocketChild, g_SocketTcpServer, g_SocketUdpServer;

//unsigned char 		g_ucRxBuffer[MCU_RX_BUFFER_OVERHEAD_SIZE];
extern const char 				g_Digits[]
#ifdef extern
= "0123456789"//, g_RecvBuf[BUF_SIZE];
#endif
;
extern volatile int 			g_UartHaveCmd
#ifdef extern
= 0
#endif
,g_UartCmdSent
#ifdef extern
= 1
#endif
;
extern unsigned long g_imDNSMode
#ifdef extern
= 0xFF
#endif
,g_mDNSStatus
#ifdef extern
= 0xFF
#endif
;
extern long g_isFileHandle;


extern WrapMode_e g_WrapMode
#ifdef extern
= LINE_LEVEL_TERMINAL_MODE
#endif
;
extern OperationMode_e g_UartMode
#ifdef extern
= INTERPRETER_MODE
#endif
;
extern InterpreterMode_e g_InterpreterMode
#ifdef extern
= ASCII_INTERPRETER_MODE
#endif
;
extern char g_cmd_help[N_HELP_ENTRIES][20];

extern unsigned char aucmDNSFileName
#ifdef extern
= 'D'
#endif
;

//****************************************************************************
//                       FUNCTION PROTOTYPES
//****************************************************************************

long StartInterpreterMode(InterpreterRole_e role);
long StartUdpServer();
long StartTcpServer();
long ParseWlanConnectString();
long ParsePingString();
long ParseRecvString();
long ParseSendString();
long ParseConnectString();
int btoa();
int htoa();
long ExecuteCommand(OperationMode_e local);
void DispatcherUartSendPacket(unsigned char *inBuff, unsigned short usLength);
long InitialiseCmdMenu();
int GetCmdIndex( char *str);
unsigned int GETChar(unsigned char *ucBuffer);
long atolong(char *data, unsigned long *retLong);
void PrintCommandMenu();
long  InitialConfiguration(void);
