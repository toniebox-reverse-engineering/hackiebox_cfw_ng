## Overview

This example demonstrates file operations performed in application code. This application uses the serial flash for file storage.

CC3200 LaunchPad has 1 MB serial flash.

Page, Sector, and Block size is provided below:
  
- Page: 256 Bytes  
- Sector: 4 Kb  
- Block: 64 Kb

The network processor maintains a file system iin the serial flash and the SimpleLink library
exposes APIs to perform file operations in application code.

## Application details

This application will:

  - Open a user file for writing
  - Write the "Old MacDonald" child song several times to create a ~64KB file
  - Close the user file
  - Open the same file for reading
  - Read the data and compare it with the stored buffer
  - Close the user file
  - Turns on the green LED on success or the red LED on failure. The yellow LED will be on all the time.

### Source Files briefly explained

- **gpio\_if.c** - Basic GPIO interface APIs. Used to control the RED/GREEN
LED.  
- **main.c** - Initializes the device, exercises the file operation APIs
supported by the SimpleLink library
- **pinmux.c** - Assigns the pin muxing  
- **startup\_\*.c** - Initialize vector table and IDE related functions

## Usage
1.  Run the reference application.
      - Open the project in CCS/IAR. Build the application and debug to load to the device, or flash the binary using [UniFlash](http://processors.wiki.ti.com/index.php/CC3100_%26_CC3200_UniFlash_Quick_Start_Guide).
2.  If the file operations succeed, the green LED will turn on. If there is a failure, the red LED will turn on.

# Limitations/Known Issues
