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
RELOCATOR=../relocator/exe

#Boot Manager Path
BOOTMGR=exe

#clean all
echo "Clean all.."
rm -rf Release/$BUILD

#RELOC_ADDR=0x20034000
RELOC_ADDR=$1
RELOCATOR_PATH=$RELOCATOR/${RELOC_ADDR}/relocator.bin
BOOTMGR_SUBDIR=$2
BOOTMGR_PATH=$BOOTMGR/${BOOTMGR_SUBDIR}/bootmgr.bin
BOOTMGR_RELOC_PATH=$BOOTMGR/${BOOTMGR_SUBDIR}/bootmgr.relocator.bin
#Check for re-locator binary
if [ ! -f $RELOCATOR_PATH ]; then

	echo "Error: Relocator Not found: $RELOCATOR_PATH"
	exit
else
	echo "Relocator found..."
fi

#Check for boot manager binary
if [ ! -f $BOOTMGR_PATH ]; then

	echo "Error: Boot Manager Not found: $BOOTMGR_PATH"
	exit
else
	echo "Boot Manager found..."
fi

#echo
echo "Generating Application bootloader..."

# Generate all 0 bin file
dd if=/dev/zero of=__tmp.bin ibs=1 count=256 conv=notrunc >/dev/null 2>&1

# Generate 0 padded version of relocator 
dd if=$RELOCATOR_PATH of=__tmp.bin ibs=1 conv=notrunc >/dev/null 2>&1

# Concatenate re-locator and boot-manager
cat __tmp.bin $BOOTMGR_PATH > $BOOTMGR_RELOC_PATH

# Remove tmp file
rm -f __tmp.bin
