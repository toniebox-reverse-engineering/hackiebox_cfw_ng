## Overview

This application demonstrates the CC3200 webserver library using websockets. The application completes the upgrade  handshake, starts a counter, and continuously sends the value over a websocket to the browser acting as a web client.

## Application details

In this application, the SimpleLink device runs in Access Point mode. The user
must connect a Wi-Fi enabled device (PC/Smartphone) to the CC3200 AP. The default SSID is **mysimplelink-xxxxxx**, where the last six digits correspond to the CC3200's MAC address.

Using a web browser, the user can navigate to http://mysimplelink.net/websocket_demo.html to display the static pages stored on the device's serial flash.

The web page will ask for the URL of the CC3200 websocket server.
The default value is 192.168.1.1. Send "Connect" and wait
for the alert window to confirm connection. The user will see the counter start and update every second.


### Websocket Sequence

1.  Client makes TCP Connection with Websocket Server at port 80/443.
2.  The client initiates a websocket handshake by sending an upgrade request.
3.  Websocket Server responds with Response code 101 if handshake is successful.
4.  Client sees an alert on web page saying connection is established.
5.  The webserver starts sending the counter values without any polling from client.
6.  Client closes connection by sending a close frame.  

## Source Files briefly explained

- **main.c** - System Initialization, Spawns the HTTPServer Task
- **httpserverapp.c** – Starts simplelink. Configures CC3200 in AP mode.
Starts the Http server. Handles all websocket callbacks.
- **gpio\_if.c** - Handles GPIO related operations
- **pinmux.c** - Configure UART and GPIO
- **network\_if.c**- Common network interface APIs
- **startup\_\*.c** - Initialize vector table and IDE related functions

  
**WebSocket library is available in the \<cc3200\_sdk\>\\netapps\\http\\server folder.**

## Usage

1.  Open **<cc3200-sdk>\\examples\\websock\_camera\\html\\websocket\_simple.usf**
	    session file in Uniflash.
2.  Program the image.
3.  Remove SOP jumpers on LaunchPad and reset.
4. The CC3200 should start or switch to AP mode, Connect your PC/SmartPhone to **mysimplelink-xxxxxx**
5. Open a browser and navigate to **mysimplelink.net/websocket\_demo.html** to connect to the webserver.
6. Click on **Connect** button and wait for a notification pop-up message saying "WebSocket Connected" and the counter will begin.

To see the webserver debug terminal messages, import the webserver library from *<cc3200-sdk>\\netapps\\http\\server* and remove the predefined symbol `NOTERM` from the project properties.

## Limitation/Known Issues
