#!/bin/bash
#*****************************************************************************
# Copyright (C) 2014 Texas Instruments Incorporated
#
# All rights reserved. Property of Texas Instruments Incorporated.
# Restricted rights to use, duplicate or disclose this code are
# granted through contract.
# The program may not be used without the written permission of
# Texas Instruments Incorporated or against the terms and conditions
# stipulated in the agreement under which this program has been supplied,
# and under no circumstances can it be used with non-TI connectivity device.
#
#*****************************************************************************

#Re-locator Path
RELOCATOR=../../relocator/gcc/exe

#Boot Manager Path
BOOTMGR=Exe/


#clean all
echo "Clean all.."
rm -rf Release/$BUILD


#Check for re-locator binary
if [ ! -f $RELOCATOR/relocator.bin ]; then

	echo "Error : Relocator Not found!"
	exit
else
	echo "Relocator found..."
fi

#Check for boot manager binary
if [ ! -f $BOOTMGR/bootmgr.bin ]; then

	echo "Error : Boot Manager Not found!"
	exit
else
	echo "Boot Manager found..."
fi

#echo
echo "Generating Application bootloader..."

# Generate all 0 bin file
dd if=/dev/zero of=__tmp.bin ibs=1 count=256 conv=notrunc >/dev/null 2>&1

# Generate 0 padded version of relocator 
dd if=$RELOCATOR/relocator.bin of=__tmp.bin ibs=1 conv=notrunc >/dev/null 2>&1

# Concatenate re-locator and boot-manager
cat __tmp.bin $BOOTMGR/bootmgr.bin > Exe/application_bootloader.bin

# Remove tmp file
rm -f __tmp.bin
