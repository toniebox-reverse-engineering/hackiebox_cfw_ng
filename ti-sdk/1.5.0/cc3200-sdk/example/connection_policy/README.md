## Overview

SimpleLink Wi-Fi's connection policy determines how the device connects to an access point (AP) when it is cofigured to station (STA) mode. The three connection policies demonstrated in this example are:
  
- **Auto**: When the **Auto** connect policy is set, the device tries to automatically reconnect to one of its stored profiles based on priority.
- **Fast**: When the **Fast** connect policy is set, the device attempts to re-establish its connection with the last AP it was connected to. This mode works even if there is no stored profile for the AP because the device remembers the SSID and channel from the previous connection (e.g. if sl_WlanConnect() was used to establish the previous connection).
- **Auto Smart-Config**: When **Auto Smart-Config** connection policy is enabled, the device will automatically start-up the smart config process to allow a new AP profile to be configured using the SmartConfig provisioning process in the SimpleLink Starter Pro mobile App.

## Application details

This application illustrates using each of the connection policies described above one-by-one. In order to verify that each policy works as expected, the user needs to put
breakpoints in the code which will be triggered when each connection attempt successfully completes. The location where each breakpoint must be placed is
mentioned in source code comments.

### Source Files briefly explained

- **main.c**: Illustrates the use of API for setting the connection policies  
- **startup\_\*.c** - Initialize vector table and IDE related functions

## Usage

1.  Setup a serial communication application. Open a serial terminal on a PC with the following settings:
	- **Port: ** Enumerated COM port
	- **Baud rate: ** 115200
	- **Data: ** 8 bit
	- **Parity: ** None
	- **Stop: ** 1 bit
	- **Flow control: ** None
2. Edit **common.h** and modify the values of the below macros for the device to connect to the AP:
```c
	#define SSID_NAME           "<ap_name>"    		/* AP SSID */
	#define SECURITY_TYPE       SL_SEC_TYPE_OPEN 	/* Security type (OPEN or WEP or WPA*/
	#define SECURITY_KEY        ""              	/* Password of the secured AP */
```
3. Open the project in CCS/IAR. Build the application and start a debug session.
4. Place breakpoints in main.c as directed. These breakpoints places are shown as:
```c
	// ****** Put breakpoint here ******
	// If control comes here - this means the device connected to the AP in Auto mode
```
5. Run the example until it hits the first breakpoint and successfully connects to the AP using the **Auto** connect policy.
6. Run again to delete the created profile and run the **Auto SmartConfig** policy.
	- For details on SmartConfig, refer to the Provisioning SmartConfig example.
7. Wait for the next breakpoint to verify that the connection succeeded.
8. Run again to delete the stored profile and test the connection with the **Fast** connect policy.

## Limitations/Known Issues
