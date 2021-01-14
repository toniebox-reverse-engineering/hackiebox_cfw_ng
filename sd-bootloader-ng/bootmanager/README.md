## Overview

This application demonstrates the secondary bootloader operations to manage
updates to the application image using Over-The-Air (OTA) Updates.

The CC3200's primary bootloader executes from the ROM (hence cannot be
modified) and loads the user’s application image residing in serial flash to
the microcontroller's RAM and transfers the execution control. The user's application image saved to the CC3200 file system in serial flash is named **/sys/mcuimg.bin**. For the OTA application, we require two stage bootloading. In this case, the primary bootloader would load /sys/mcuimg.bin which would then act as the secondary boot loader.

The secondary bootloader could be used as an effective tool to load and execute binary images from the serial flash after making a choice from multiple binary images available in the storage.

**Normal boot** is meant as any boot like POR, hibernate, WDT
reset, MCU reset etc. The only exception being one boot cycle just after
an OTA update. The trade-off here is the application bootloader run
time size which increases from 16KB to 32KB when `FAST_BOOT` is
defined. 

**Fast boot** does not turn on the networking subsystem for loading the application image during the normal boot, reduces the overall loading time of the application image during normal boot, and improves the overall initialization time of the application.

## Application Details

Focus of this specific example is to enable the developer to create a
roll-back option. This application will be flashed as
**/sys/mcuimg.bin** which in turn can manage three user images in serial flash and would be responsible for selecting one of the following
three application images.

1.  **/sys/mcuimg1.bin** - Factory Default (This would be flashed to the system during production flow)
2.  **/sys/mcuimg2.bin** - Application Image (Updated)
3.  **/sys/mcuimg3.bin** - Application Image (Updated)

After loading the images, they can be tested to make sure that the
updated image is not corrupt. In case of any corruption, the system can
roll back to the last known working application image. (This step is not
part of this application. Please refer to the OTA Update example.)

For more details on the application bootloader and the OTA Update process, see the OTA Application Note at cc3200_sdk/docs/CC3200_OTA_Update_Application_Note.pdf.

## Source Files briefly explained

- **main.c** : Contains the core logic for application bootloader
- **startup\_\*.c** - Initialize vector table and IDE related functions
- **udma_if.c** : UDMA driver wrapper API implementation

## Usage

This application uses an optimizedo version of the SimpleLink library for network functionality. The following section lists the steps for recompiling this application.

### CCS

1. Import the SimpleLink project from  cc3200\_sdk/simplelink/ccs
2. Go to Build Configurations-\>Set Active and select NON\_OS.
3. In the project properties, go to CCS build-\>ARM
Compiler-\>Optimization and set optimization level to “3 Interprocedure
Optimizations”.
4. Go to CCS build-\>ARM Archiver-\>Basic Options. Set output file name
to “${ProjName}\_opt.a”
5. Rebuild the simplelink project.
6. Import the bootmgr project from cc3200\_sdk\example\application_bootloader\bootmgr\ccs
7. Rebuild the bootmgr project to generate application_bootloader.bin.

### IAR

1. Open the SimpleLink project from cc3200_sdk\simplelink\ewarm\simplelink.ewp
2. Select the NON\_OS project configuration
3. Go to project-\>Options-\>C/C++ Compiler-\>Optimizations-\>Level and set to “Medium”
4. Go to project-\>options-\>Library Builder and make sure the output file name is changed to "simplelink\_opt.a”.
5. Rebuild the simplelink project.
6. Open the bootmgr project from cc3200_sdk\example\application_bootloader\bootmgr\ewarm\bootmgr.ewp
7. Rebuild the bootmgr project. This will generate application_bootloader.bin.

### GCC

Execute the below command on Cygwin in cc3200_sdk/simplelink/gcc:

`make -f Makefile\_opt target=NONOS`

## Limitations
