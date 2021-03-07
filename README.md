# HackieboxNG Custom Firmware

## Introduction
HackieboxNG is the next generation bootloader and firmware for your toniebox!

## HackieboxNG SD bootloader
The HackieboxNG SD bootloader consists of two parts. The bootloader itself that allows selecting + running different firmwares and a relocator to move the bootloaders memory to the upper end of the memory. In addition there is a sd bootloader with one fixed file to run.

### Features
* Nine firmware slots
* Loading any CC3200 standard firmware
* Loading the original firmware directly
* Patching binaries in memory for ex. enable SLIX tags or disable charger wakeup ([more](https://github.com/toniebox-reverse-engineering/hackiebox_cfw_ng/wiki/OFWPatches))
* Highly configurable via json files

## HackieboxNG SD firmware
TBD

## Technical introduction
The HackieboxNG CFW is based on a full open source gcc (arm/ti) toolchain. This allows building the new firmware generation without dependecies to micropython or Energia.
Makefiles + Visual Studio Code launch.json/tasks.json are provided to get started instantly on linux.
An openocd file for the cc3200-launchpad is preconfigured in the launch.json. You may alternativly use the provided j-link config file if you got this type of debugger.

## How to build
Currently building is only supported under Linux and tested under Debian and Ubuntu.
As a first step it is adviced to run the
[**/common/buildAndShip.bootloader.sh**](https://github.com/toniebox-reverse-engineering/hackiebox_cfw_ng/blob/master/common/buildAndShip.bootloader.sh) script once without parameters and once **/common/buildAndShip.bootloader.sh debug**. This will build all artefacts and needed dependency artefacts. (especially the driverlib and libsimplelink_nonos_opt)
For building just single artefacts just take a look into the [buildAndShip.bootloader.sh](https://github.com/toniebox-reverse-engineering/hackiebox_cfw_ng/blob/master/common/buildAndShip.bootloader.sh) or the provided [/.vscode/tasks.json](https://github.com/toniebox-reverse-engineering/hackiebox_cfw_ng/blob/master/.vscode/tasks.json)

[Chech out the wiki for more detailed information](https://github.com/toniebox-reverse-engineering/hackiebox_cfw_ng/wiki)