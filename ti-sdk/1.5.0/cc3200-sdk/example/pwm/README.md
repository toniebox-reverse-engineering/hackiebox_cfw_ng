## Overview  

The general purpose timers (GPTs) support a 16-bit pulse-width
modulation (PWM) mode with software-programmable output inversion of the
PWM signal. As there are 4 GPTs, there are 8 possible PWM output
options (Timer A of TIMERA2 is not available).

## Application details  

The application uses the following pins to drive the PWM output:

  - PIN\_64 for TIMERPWM5
  - PIN\_01 for TIMERPWM6
  - PIN\_02 for TIMERPWM7

On this LaunchPad, the pins that drive the LEDs are capable of being
muxed to drive a PWM output. In this example, the brightness of the
LEDs are varied from off to on by varying the duty cycle of the PWM output.

The example generates a PWM output with ~0.5 ms time period. The duty
cycle is continuously varied in order to change the brightness and this
is done iteratively.

## Source Files briefly explained  

- **main.c** - Configures the timer to operate in PWM mode. Continuously changes
the duty cycle in order to change the brightness of the LEDs.
- **pinmux.c** - Assigns a personality to the pins at the device boundary  
- **startup\_\*.c** - Initialize vector table and IDE related functions

## Usage  

1.  Run the reference application.
      - Open the project in CCS/IAR. Build the application and debug to load to the device, or flash the binary using [UniFlash](http://processors.wiki.ti.com/index.php/CC3100_%26_CC3200_UniFlash_Quick_Start_Guide).
2.  The brightness of the 3 LEDs (red, green, yellow) incrementally
    changes from low (off) to high (on). This keeps
    repeating.

# Limitations/Known Issues
