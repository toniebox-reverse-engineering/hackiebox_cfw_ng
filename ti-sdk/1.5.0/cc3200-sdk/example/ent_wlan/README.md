## Overview

This simple example code connects to an enterprise network using a certificate flashed to the serial flash.

## Application details

Example demonstrates connection to an enterprise network. The red LED will keep blinking until the connection is established.

### Source Files briefly explained

- **main.c** - Initializes the device, connects to a enterprise network

## Usage

1. Download the required enterprise certificate file on sFLASH using 'Uniflash' utility. For detailed instructions about using Uniflash, refer to the [UniFlash User Guide](http://processors.wiki.ti.com/index.php/CC3100_%26_CC3200_UniFlash_Quick_Start_Guide).
	1. Select the "/cert/ca.pem" tab.
	2. In the Url field, browse to the location of your enterprise certificate. Check the Erase and Update check boxes.
	3. Select **Program**
2. Open `main.c` and modify `ENT_NAME`, `USER_NAME`, and `PASSWORD` accordingly.
3. Rebuild the application and run.
	- Debug to load the application to the device, or flash the binary using [UniFlash](http://processors.wiki.ti.com/index.php/CC3100_%26_CC3200_UniFlash_Quick_Start_Guide).

## Limitations/Known Issues
