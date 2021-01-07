## Overview

Every GPTM (General purpose timer module) block can be used as two 16-bit
timers/counters (referred to as Timer A and Timer B) that can be
configured to operate independently as timers or event counters, or
concatenated to operate as one 32-bit timer. Timers can also be used to
trigger µDMA transfers. 

Following operating modes are supported:

1.  16 or 32-bit programmable one-shot timer
2.  16 or 32-bit programmable periodic timer
3.  16-bit general-purpose timer with an 8-bit prescaler

## Application details

This application showcases the usage of Timer DriverLib APIs. The
objective of this application is to showcase the usage of 16 bit timers
to generate interrupts which in turn toggle the state of the GPIO
(driving LEDs).

Two timers with different timeout value (one is twice the other) are set
to toggle two different GPIOs which drive LEDs on the LaunchPad.

### Source Files briefly explained

- **main.c** - contains main function implementing LED blinking using
    timers.
- **gpio\_if.c** - APIs to get pin number from GPIO number and set them.
- **pinmux.c** - Pinmux configurations as required by the application.
- **timer\_if.c** - APIs for timer.

## Usage

1.  Run the reference application.
      - Open the project in CCS/IAR. Build the application and debug to load to the device, or flash the binary using [UniFlash](http://processors.wiki.ti.com/index.php/CC3100_%26_CC3200_UniFlash_Quick_Start_Guide).
2.  Observe the two LEDs blinking with diffent frequencies (one is twice the other).

## Limitations/Known Issues
