# HackieboxNG Custom Firmware

## Introduction
The HackieboxNG CFW is based on a full open source gcc (arm/ti) toolchain. This allows building the new firmware generation without dependecies to micropython or Energia.
Makefiles + Visual Studio Code launch.json/tasks.json are provided to get started instantly (on linux)

## HackieboxNG SD bootloader
The HackieboxNG SD bootloader consists of two parts. The bootloader itself that allows selecting + running different firmwares and a relocator to move the bootloaders memory to the upper end of the memory. In addition there is a sd bootloader with one fixed file to run.

### Features
* Nine firmware slots
* Loading any CC3200 standard firmware
* Loading the original firmware directly (special runtime patch needed)
* Patching binaries in memory
* Configuration files

## HackieboxNG SD firmware
TBD