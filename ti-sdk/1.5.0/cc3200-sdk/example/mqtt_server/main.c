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

//*****************************************************************************
//
// Application Name     -   MQTT Server
// Application Overview -   The device is running a MQTT server which allows
//                          local MQTT client to communicate with each other.
//                          Simultaneously, it is also running a client which is
//                          connected to the online broker. The interface
//                          between the on-board client and the server is such
//                          that the local clients can also communicate with the
//                          remote MQTT clients, which are connected to the same
//                          online broker as the on-board client.
//
//*****************************************************************************

//*****************************************************************************
//
//! \addtogroup mqtt_server
//! @{
//
//*****************************************************************************

// Standard includes
#include <stdlib.h>

// simplelink includes
#include "simplelink.h"

// driverlib includes
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "interrupt.h"
#include "rom_map.h"
#include "prcm.h"
#include "uart.h"
#include "timer.h"
#include "utils.h"

#include "network_if.h"
#ifndef NOTERM
#include "uart_if.h"
#endif
#include "button_if.h"
#include "gpio_if.h"
#include "timer_if.h"
#include "common.h"

//MQTT Library includes
#include "sl_mqtt_client.h"
#include "sl_mqtt_server.h"

// application specific includes
#include "pinmux.h"
#include "server_client_cbs.h"

#define APPLICATION_VERSION 	"1.4.0"

/*Operate Lib in MQTT 3.1 mode.*/
#define MQTT_3_1_1              false /*MQTT 3.1.1 */
#define MQTT_3_1                true /*MQTT 3.1*/

#define WILL_TOPIC              "Client"
#define WILL_MSG                "Client Stopped"
#define WILL_QOS                QOS2
#define WILL_RETAIN             false

/*Defining Broker IP address and port Number*/
#define SERVER_ADDRESS           "m2m.eclipse.org"
#define SERVER_IP_ADDRESS        "192.168.178.67"
#define PORT_NUMBER              1883
#define SECURED_PORT_NUMBER      8883
#define LOOPBACK_PORT            1882

/*Specifying Receive time out for the Receive task*/
#define RCV_TIMEOUT             30

/*Background receive task priority*/
#define TASK_PRIORITY           3

/* Keep Alive Timer value*/
#define KEEP_ALIVE_TIMER        25

/*Clean session flag*/
#define CLEAN_SESSION           true

/*Retain Flag. Used in publish message. */
#define RETAIN                  1

/*Defining Publish Topic*/
#define PUB_TOPIC               "/cc32xx/ButtonPressEvtSw2"

/*Defining Number of topics*/
#define SUB_TOPIC_COUNT         1

/*Defining Subscription Topic Values*/
#define SUB_TOPIC               "/Broker/To/cc32xx"

/*Defining Enrolled Topic Values*/
#define ENROLLED_TOPIC          "/cc32xx/To/Broker"

/*Defining QOS levels*/
#define QOS0                    0
#define QOS1                    1
#define QOS2                    2

/*Spawn task priority and OSI Stack Size*/
#define OSI_STACK_SIZE          2048

//*****************************************************************************
//                      LOCAL FUNCTION PROTOTYPES
//*****************************************************************************
void pushButtonInterruptHandler2();
void pushButtonInterruptHandler3();
void TimerPeriodicIntHandler(void);
void LedTimerConfigNStart();
void LedTimerDeinitStop();
void BoardInit(void);
static void DisplayBanner(char * AppName);
void MqttClientServer(void *pvParameters);

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif
#if defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif

unsigned short g_usTimerInts;
/* AP Security Parameters */
SlSecParams_t SecurityParams = {0};

struct BridgeHandle {
    /* Handle to the client context */
    void *ClientCtxHndl;
}BridgeHndl;

/*Client ID,User Name and Password*/
unsigned char *ClientId="mnb";
unsigned char *Username="username1";
unsigned char *Password="pwd1";

/*Subscription topics and qos values*/
char  *topic[SUB_TOPIC_COUNT]={SUB_TOPIC};

unsigned char qos[SUB_TOPIC_COUNT]={ QOS2};

/*Message Queue*/
OsiMsgQ_t g_PBQueue;

extern SlMqttServerCbs_t server_callbacks;
extern SlMqttClientCbs_t client_callbacks;
/*Publishing topics and messages*/
unsigned char *pub_topic=PUB_TOPIC;
unsigned char *data={"Push Button SW2 has been pressed on CC32XX device"};

/*Initialization structure to be used with sl_ExtMqtt_Init API*/
SlMqttClientCtxCfg_t Mqtt_ClientCtx ={
{
    SL_MQTT_NETCONN_URL,
    SERVER_ADDRESS,
    PORT_NUMBER,
    0,
    0,
    0,
    NULL
    },
    MQTT_3_1,
    true
};

SlMqttClientLibCfg_t Mqtt_ClientCfg ={
    0,
    TASK_PRIORITY,
    RCV_TIMEOUT,
    true,
    (long(*)(const char *, ...))UART_PRINT
};

SlMqttServerCfg_t Mqtt_Server={
    {
        0,
        NULL,
        PORT_NUMBER,
        0,
        0,
        0,
        NULL
    },
    LOOPBACK_PORT,
    TASK_PRIORITY,
    RCV_TIMEOUT,
    true,
    (long(*)(const char *, ...))UART_PRINT
};

SlMqttWill_t will_param={
    WILL_TOPIC,
    WILL_MSG,
    WILL_QOS,
    WILL_RETAIN
};
//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************

//****************************************************************************
//
//! Push Button Handler1(GPIOSW2). Press push button1 (GPIOSW2) Whenever user
//! wants to publish a message. Write message into message queue signaling the
//! event publish messages
//!
//! \param none
//!
//! return none
//
//****************************************************************************
void 
pushButtonInterruptHandler2()
{
    struct msg_queue queue_element;
    queue_element.event = PUB_PUSH_BUTTON_PRESSED;
    queue_element.msg_ptr = NULL;
    //
    // write message indicating publish message
    //
    osi_MsgQWrite(&g_PBQueue,&queue_element,OSI_NO_WAIT);
}

//****************************************************************************
//
//! Push Button Handler2(GPIOSW3). Press push button3 Whenever user wants to discoonect
//! from the remote broker. Write message into message queue indicating
//! disconnect from broker.
//!
//! \param none
//!
//! return none
//
//****************************************************************************
void 
pushButtonInterruptHandler3()
{
    struct msg_queue queue_element;
    queue_element.event = DISC_PUSH_BUTTON_PRESSED;
    queue_element.msg_ptr = NULL;
    //
    // write message indicating publish message
    //
    osi_MsgQWrite(&g_PBQueue,&queue_element,OSI_NO_WAIT);
}

//*****************************************************************************
//
//! Periodic Timer Interrupt Handler
//!
//! \param None
//!
//! \return None
//
//*****************************************************************************
void
TimerPeriodicIntHandler(void)
{
    unsigned long ulInts;

    //
    // Clear all pending interrupts from the timer we are
    // currently using.
    //
    ulInts = MAP_TimerIntStatus(TIMERA0_BASE, true);
    MAP_TimerIntClear(TIMERA0_BASE, ulInts);

    //
    // Increment our interrupt counter.
    //
    g_usTimerInts++;
    if(!(g_usTimerInts & 0x1))
    {
        //
        // Off Led
        //
        GPIO_IF_LedOff(MCU_RED_LED_GPIO);
    }
    else
    {
        //
        // On Led
        //
        GPIO_IF_LedOn(MCU_RED_LED_GPIO);
    }
}

//****************************************************************************
//
//! Function to configure and start timer to blink the LED while device is
//! trying to connect to an AP
//!
//! \param none
//!
//! return none
//
//****************************************************************************
void LedTimerConfigNStart()
{
    //
    // Configure Timer for blinking the LED for IP acquisition
    //
    Timer_IF_Init(PRCM_TIMERA0,TIMERA0_BASE,TIMER_CFG_PERIODIC,TIMER_A,0);
    Timer_IF_IntSetup(TIMERA0_BASE,TIMER_A,TimerPeriodicIntHandler);
    Timer_IF_Start(TIMERA0_BASE,TIMER_A,100);
}

//****************************************************************************
//
//! Disable the LED blinking Timer as Device is connected to AP
//!
//! \param none
//!
//! return none
//
//****************************************************************************
void LedTimerDeinitStop()
{
    //
    // Disable the LED blinking Timer as Device is connected to AP
    //
    Timer_IF_Stop(TIMERA0_BASE,TIMER_A);
    Timer_IF_DeInit(TIMERA0_BASE,TIMER_A);

}

//*****************************************************************************
//
//! Board Initialization & Configuration
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************
void BoardInit(void)
{
    /* In case of TI-RTOS vector table is initialize by OS itself */
    #ifndef USE_TIRTOS
    //
    // Set vector table base
    //
    #if defined(ccs)
        IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
    #endif
    #if defined(ewarm)
        IntVTableBaseSet((unsigned long)&__vector_table);
    #endif
    #endif
    //
    // Enable Processor
    //
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}

//*****************************************************************************
//
//! Application startup display on UART
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
static void
DisplayBanner(char * AppName)
{

    UART_PRINT("\n\n\n\r");
    UART_PRINT("\t\t *************************************************\n\r");
    UART_PRINT("\t\t    CC3200 %s Application       \n\r", AppName);
    UART_PRINT("\t\t *************************************************\n\r");
    UART_PRINT("\n\n\n\r");
}

//*****************************************************************************
//
//! Task implementing MQTT Server plus client bridge
//!
//! \param  none
//!
//! This function
//!    1. Initializes network driver and connects to the default AP
//!    2. Initializes the mqtt client ans server libraries and set up MQTT
//          with the remote broker.
//!    3. set up the button events and their callbacks(for publishing)
//!       4. handles the callback signals
//!
//! \return None
//!
//*****************************************************************************
void
MqttClientServer(void *pvParameters)
{
    
    struct msg_queue que_elem_recv;
    struct publish_msg_header msg_head;
    unsigned int uiTopOffset = 0;
    unsigned int uiPayloadOffset = 0;
    long lRetVal = -1;
    int iCount = 0;
    unsigned int uiConnFlag = 0;
    //
    // Configure LED
    //
    GPIO_IF_LedConfigure(LED1|LED2|LED3);

    GPIO_IF_LedOff(MCU_RED_LED_GPIO);
    GPIO_IF_LedOff(MCU_GREEN_LED_GPIO);

    //
    // Reset The state of the machine
    //
    Network_IF_ResetMCUStateMachine();

    //
    // Start the driver
    //
    lRetVal = Network_IF_InitDriver(ROLE_STA);
    if(lRetVal < 0)
    {
       UART_PRINT("Failed to start SimpleLink Device\n\r",lRetVal);
       LOOP_FOREVER();
    }

    // switch on Green LED to indicate Simplelink is properly up
    GPIO_IF_LedOn(MCU_ON_IND);

    // Start Timer to blink Red LED till AP connection
    LedTimerConfigNStart();

    // Initialize AP security params
    SecurityParams.Key = (signed char *)SECURITY_KEY;
    SecurityParams.KeyLen = strlen(SECURITY_KEY);
    SecurityParams.Type = SECURITY_TYPE;

    //
    // Connect to the Access Point
    //
    lRetVal = Network_IF_ConnectAP(SSID_NAME, SecurityParams);
    if(lRetVal < 0)
    {
       UART_PRINT("Connection to an AP failed\n\r");
       LOOP_FOREVER();
    }

    //
    // Disable the LED blinking Timer as Device is connected to AP
    //
    LedTimerDeinitStop();

    //
    // Switch ON RED LED to indicate that Device acquired an IP
    //
    GPIO_IF_LedOn(MCU_IP_ALLOC_IND);

    UtilsDelay(20000000);

    GPIO_IF_LedOff(MCU_RED_LED_GPIO);
    GPIO_IF_LedOff(MCU_ORANGE_LED_GPIO);
    GPIO_IF_LedOff(MCU_GREEN_LED_GPIO);

    //
    // Register Push Button Handlers
    //
    Button_IF_Init(pushButtonInterruptHandler2,pushButtonInterruptHandler3);

/******************************************************************************/
/*    Initialiszing server and registreing callbacks                          */
/******************************************************************************/
    sl_ExtLib_MqttServerInit( &Mqtt_Server,&server_callbacks);

/******************************************************************************/
/*    Activate the MQTT Server Network Resources                              */
/******************************************************************************/
    sl_ExtLib_MqttServerActivate();

/******************************************************************************/
/* Initialising Client and Subscribing to the Broker                          */
/******************************************************************************/

    //
    // Initialze MQTT client lib
    //
    lRetVal = sl_ExtLib_MqttClientInit(&Mqtt_ClientCfg);
    if(lRetVal != 0)
    {
        // lib initialization failed
        UART_PRINT("MQTT Client lib initialization failed\n\r");
        LOOP_FOREVER();
    }

    //
    // Create MQTT client context
    //
    BridgeHndl.ClientCtxHndl = sl_ExtLib_MqttClientCtxCreate(&Mqtt_ClientCtx,
                                    &client_callbacks,
                                    &BridgeHndl);
    //
    // Set Client ID
    //
    sl_ExtLib_MqttClientSet(BridgeHndl.ClientCtxHndl, SL_MQTT_PARAM_CLIENT_ID,
                        ClientId,strlen((char*)ClientId));
    
    
    //
    // setting will parameters
    //
    sl_ExtLib_MqttClientSet(BridgeHndl.ClientCtxHndl, SL_MQTT_PARAM_WILL_PARAM,
                        &will_param,sizeof(will_param));

#ifdef CLNT_USR_PWD
    //
    // Set username for client connection
    //
    sl_ExtLib_MqttClientSet(BridgeHndl.ClientCtxHndl, SL_MQTT_PARAM_USER_NAME,
                        Username,strlen((char*)Username));

    //
    // Set password
    //
    sl_ExtLib_MqttClientSet(BridgeHndl.ClientCtxHndl, SL_MQTT_PARAM_PASS_WORD,
                        Password,strlen((char*)Password));
#endif
    // 
    // Initiate MQTT Connect
    //
    lRetVal = sl_ExtLib_MqttClientConnect(BridgeHndl.ClientCtxHndl,
                                          CLEAN_SESSION,KEEP_ALIVE_TIMER);
    if(lRetVal != 0)
    {
        // lib initialization failed
        UART_PRINT("Connection to broker failed\n\r");

        uiConnFlag = 0;
        goto skip_subscription;
    }
    else
    {
        uiConnFlag = 1;
    }
    
    //
    // Subscribe to topics
    //
    if(sl_ExtLib_MqttClientSub(BridgeHndl.ClientCtxHndl, topic,qos,
                               SUB_TOPIC_COUNT)<0)
    {
        UART_PRINT("\n\r Subscription Error \n\r");
        UART_PRINT("Disconnecting from the broker\r\n");
        sl_ExtLib_MqttClientDisconnect(BridgeHndl.ClientCtxHndl);
        uiConnFlag = 0;
    }
    else
    {
        UART_PRINT("CC3200 MQTT Client subscribed on following topics:\n\r");
        for(iCount = 0; iCount < SUB_TOPIC_COUNT; iCount++)
        {
            UART_PRINT("%s\n\r,",topic[iCount]);
        }
    }

    //
    // on-board client is enrolling to server. Server will treated this
    // enrollment as a subscription.
    //
    sl_ExtLib_MqttTopicEnroll(ENROLLED_TOPIC);
skip_subscription:
    //
    // handling the signals from various callbacks including the push button
    // prompting the client to publish a msg on PUB_TOPIC OR msg recieved by the
    // server on enrolled topic(for which the on-board clinet ha enrolled) from
    // a local client(will be published to the remote broker by the client) OR
    // msg received by the client from the remote broker(need to be sent to the
    // server to see if any local client has subscribed on the same topic).
    //
    for(;;)
    {
        //
        // waiting for signals
        //
        osi_MsgQRead( &g_PBQueue, &que_elem_recv, OSI_WAIT_FOREVER);

        switch(que_elem_recv.event)
        {
        case PUB_PUSH_BUTTON_PRESSED:
            //
            // send publish message
            //
            lRetVal =
            sl_ExtLib_MqttClientSend(BridgeHndl.ClientCtxHndl,
                                     (const char *)pub_topic,data,
                                      strlen((char*)data), QOS2,
                                      RETAIN );
            UART_PRINT("\n\r CC3200 Publishes the following message \n\r");
            UART_PRINT("Topic: %s\n\r",pub_topic);
            UART_PRINT("Data: %s\n\r",data);
            Button_IF_EnableInterrupt(SW2);
            break;
                
            //
            // msg received by server (on the enrolled topic by on-board client)
            // publish it to the remote broker
            //
        case MSG_RECV_BY_SERVER:
            if(uiConnFlag == 0)
            {
                free(que_elem_recv.msg_ptr);
                break;
            }

            memcpy(&msg_head, que_elem_recv.msg_ptr, sizeof(msg_head));
            uiTopOffset = sizeof(msg_head);
            uiPayloadOffset = uiTopOffset + msg_head.topic_len + 1;
            lRetVal =
            sl_ExtLib_MqttClientSend(BridgeHndl.ClientCtxHndl,
                      ((const char *)(que_elem_recv.msg_ptr)+uiTopOffset),
                      ((unsigned char*)(que_elem_recv.msg_ptr)+uiPayloadOffset),
                      msg_head.pay_len,
                      msg_head.qos,
                      msg_head.retain);
            free(que_elem_recv.msg_ptr);
            break;
            
            //
            // msg received by client from remote broker (on a topic subscribed
            // by local client)
            //
        case MSG_RECV_BY_CLIENT:
            memcpy(&msg_head, que_elem_recv.msg_ptr, sizeof(msg_head));
            uiTopOffset = sizeof(msg_head);
            uiPayloadOffset = uiTopOffset + msg_head.topic_len + 1;
            lRetVal =
            sl_ExtLib_MqttServerSend(((const char*)(que_elem_recv.msg_ptr)+
                      uiTopOffset),
                      ((unsigned char*)(que_elem_recv.msg_ptr)+uiPayloadOffset),
                      msg_head.pay_len,
                      msg_head.qos,
                      msg_head.retain,
                      0);
            free(que_elem_recv.msg_ptr);
            break;
            
            //
            // on-board client disconnected from remote broker, only local MQTT
            // network will work
            //
        case BROKER_DISCONNECTION:
            uiConnFlag = 0;
            break;

            //
            // push button for broker disconneciotn is received, must disconnect
            // from the remote broker
            //
        case DISC_PUSH_BUTTON_PRESSED:
            if(uiConnFlag == 0)
            {
                break;
            }
            sl_ExtLib_MqttClientDisconnect(BridgeHndl.ClientCtxHndl);
            uiConnFlag = 0;
            break;
        default:
            break;
        }
        if(lRetVal < 0)
        {
            break;
        }
    }
   
    //
    // unsubscribing the topics
    //
    sl_ExtLib_MqttClientUnsub(BridgeHndl.ClientCtxHndl, topic, SUB_TOPIC_COUNT);
    UART_PRINT("\n\r CC3200 MQTT Client Unsubscribed from the topics");
    for(iCount = 0; iCount < SUB_TOPIC_COUNT; iCount++)
    {
        UART_PRINT(",%s",topic[iCount]);
    }
    UART_PRINT("\b\n\r");
    
    //
    // disconnecting from the remote broker
    //
    sl_ExtLib_MqttClientDisconnect(BridgeHndl.ClientCtxHndl);
    UART_PRINT("\n\r Disconnected from the broker\n\r");

    //
    // deleting the context for the connection
    //
    sl_ExtLib_MqttClientCtxDelete(BridgeHndl.ClientCtxHndl);

    //
    // exiting the Client library
    //
    sl_ExtLib_MqttClientExit();

    UART_PRINT("\n\r Exiting the Application\n\r");
    
    LOOP_FOREVER();
}

//*****************************************************************************
//
//! Main 
//!
//! \param  none
//!
//! This function
//!    1. Invokes the SLHost task
//!    2. Invokes the MqttServer
//!
//! \return None
//!
//*****************************************************************************
void
main()
{ 
    long lRetVal = -1;
    //
    // Initialize the board configurations
    //
    BoardInit();

    //
    // Pinmux for UART
    //
    PinMuxConfig();

    //
    // Configuring UART
    //
    InitTerm();

    //
    // Display Application Banner
    //
    DisplayBanner("MQTT_Server");

    //
    // Start the SimpleLink Host
    //
    lRetVal = VStartSimpleLinkSpawnTask(SPAWN_TASK_PRIORITY);
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }

    //
    // Start the MQTT Client task
    //
    osi_MsgQCreate(&g_PBQueue,"PBQueue",sizeof(struct msg_queue),10);
    lRetVal = osi_TaskCreate(MqttClientServer,
                            (const signed char *)"Mqtt Server Client App",
                            OSI_STACK_SIZE, NULL, 2, NULL );

    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }
    //
    // Start the task scheduler
    //
    osi_start();
}

