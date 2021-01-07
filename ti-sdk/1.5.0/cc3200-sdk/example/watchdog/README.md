## Overview

A watchdog timer generates an interrupt or a reset when a time-out value
is reached. The watchdog timer is used to regain control when a system
has failed due to a software error or due to the failure of an external
device to respond in the expected way.

The watchdog timer has following features:

1.  32-bit down counter with a programmable load register.
2.  Programmable interrupt generation logic with interrupt masking.
3.  User-enabled stalling when the microcontroller asserts the CPU Halt
    flag during debug.

## Application details

This application showcases the usage of the Watchdog timer (WDT)
DriverLib APIs. The awatchdog resets the system whenever the system
fails after a set period of time. In this application, if the watchdog interrupt is not cleared after 10 counts, the system will reset to simulate the condition of a failed system.

Whenever the watchdog timer expires, the interrupt is cleared, which be indicated by the red LED
blinking. After ten times, the interrupt will not be cleared and the LED will be on to indicate the system reset. System reset will restart the application.

### Source Files briefly explained

1.  **main.c** - Main file that showcases the watchdog functionality with
    LED blinking for 10 times and then remain in ON state.
2.  **gpio\_if.c** - APIs to get pin number from GPIO number and set them.
3.  **pinmux.c** - Pinmux configurations as required by the application.

## Usage

1.  Run the reference application.
      - Open the project in CCS/IAR. Build the application and flash the binary using [UniFlash](http://processors.wiki.ti.com/index.php/CC3100_%26_CC3200_UniFlash_Quick_Start_Guide).
2. The red LED displays when the CC3200 is clearing the watchdog interrupt or resetting.

## Limitations/Known Issues

For details on the correct watchdog recovery sequence, refer to the [CC3200 Technical Reference Manual](http://www.ti.com/lit/swru367)
	