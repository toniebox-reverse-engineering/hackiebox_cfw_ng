## Overview

The device can be brought up in Station, AP, or P2P mode, and can switch between these modes using APIs. Networking parameters can also be adjusted by modifying files stored in the serial flash.

## Application details

This objective of the application is to enable the user to switch between
the networking modes (station, AP, or P2P) and configure the SSID name
(in the case of AP mode) using APIs.

## Source Files briefly explained

- **main.c** – starts simplelink and call APIs for pin muxing, UART
terminal initialization, getting user input and configuring
networking mode.  
- **pinmux.c** – contains pin muxing configuration for UART
communication .
- **startup\_\*.c** - Initialize vector table and IDE related functions
- **uart\_if.c** – APIs for communication over UART.

## Usage

1.  Setup a serial communication application. Open a serial terminal on a PC with the following settings:
	- **Port: ** Enumerated COM port
	- **Baud rate: ** 115200
	- **Data: ** 8 bit
	- **Parity: ** None
	- **Stop: ** 1 bit
	- **Flow control: ** None
2.  Run the reference application.
      - Open the project in CCS/IAR. Build the application and debug to load to the device, or flash the binary using [UniFlash](http://processors.wiki.ti.com/index.php/CC3100_%26_CC3200_UniFlash_Quick_Start_Guide).
3. Provide input on terminal to configure the networking mode and SSID name (in case of AP mode).

**Note:** Option #3 will switch device into P2P mode. To verify the P2P feature on CC3200, please refer the p2p example.

## Limitations/Known Issues
