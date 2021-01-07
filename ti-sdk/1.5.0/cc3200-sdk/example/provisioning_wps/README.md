# Overview

This example code demonstrates how to use WPS Wi-Fi provisioning with
the CC3200. It demonstrates both the Push button and Pin code options available to be used with WPS.

# Application details

## Program Flow

  - Initialize the device networking layer
  - Initiate connection to configured AP with WPS security using the Push button method.
  - Once connected, the red LED turns on.
  - After a delay, the application disconnects from the AP. The red
    LED is turned off.
  - Initiate connection to the same configured AP with WPS security using the Pin code
    method. The default pin code is 88664422.
  - Once connected, the red LED turns on.
  - After a delay, the application disconnects from the AP. The red
    LED is turned off.

## Source Files briefly explained

- **gpio\_if** Basic GPIO interface APIs. Used to control the red LED.  
- **main** Initializes the device, connects to a AP using two WPS methods
- **pinmux** Assigns a personality to the pins at the device boundary  
- **startup\_\*.c** - Initialize vector table and IDE related functions

# Usage

1.  Setup a serial communication application. Open a serial terminal on a PC with the following settings:
	- **Port: ** Enumerated COM port
	- **Baud rate: ** 115200
	- **Data: ** 8 bit
	- **Parity: ** None
	- **Stop: ** 1 bit
	- **Flow control: ** None
2. Modify the SSID_NAME macro in **common.h** for your AP that supports WPS provisioning using push button and pin code. You do not need to modify any other AP parameters.
```c
	#define SSID_NAME		"<ap_name>"		/* AP SSID */
```
3.  Run the reference application.
	- Open the project in CCS/IAR. Build the application and debug to load to the device, or flash the binary using [UniFlash](http://processors.wiki.ti.com/index.php/CC3100_%26_CC3200_UniFlash_Quick_Start_Guide).
4.  Choose an AP that supports WiFi provisioning using WPS (Push Button as well as Pin Code).
5. Connect to the AP from a laptop and open the AP's webpage in a browser. This is likely <http://192.168.1.1> and then enter the admin-password details.
6.  Once the application starts, the device will wait for a connection using WPS Push button mode.
7.  Press the WPS push button on the AP. This can also be simulated on
    the webpage of the AP (if supported).
8.  Once connected, the red LED turns on. After about 5 seconds, the application disconnects from the AP and the red LED turns off.
9.  Now the device will wait for a connection using WPS pin code mode. Enter the pin Code 88664422 in the webpage of AP.
10. Once connected, the red LED turns on. After about 5 seconds, the application disconnects from the AP and the red LED turns off.

## Limitations/Known Issues
