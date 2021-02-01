#!/bin/bash

$(cd -P -- "$(dirname -- "$0")" && pwd -P)
cd ..
ROOT=$PWD
OUT_DIR=${ROOT}/exe/sd-bootloader-ng
OUT_FILE=${ROOT}/exe/sd-bootloader-ng.zip
RELOC_DIR=${ROOT}/sd-bootloader-ng/relocator
BOOTMGR_DIR=${ROOT}/sd-bootloader-ng/bootmanager

BOOTMGR_SRC_BIN=${BOOTMGR_DIR}/exe/bootmgr.relocator.bin
BOOTMGR_SRC_SD=${BOOTMGR_DIR}/sd/revvox/boot

PRELOAD_DES_DIR=${OUT_DIR}/flash/sys
BOOTMGR_DES_DIR=${OUT_DIR}/sd/revvox/boot

PRELOAD_DES_BIN=${PRELOAD_DES_DIR}/mcuimg.bin
BOOTMGR_DES_BIN=${BOOTMGR_DES_DIR}/ngbootloader.bin

if [ -d "$RELOC_DIR" ] && [ -d "$RELOC_DIR" ]; then
    echo Clean output directory
    rm -f ${OUT_FILE}
    rm -rf ${OUT_DIR}
    echo Create output directory
    mkdir ${OUT_DIR}
    mkdir -p ${PRELOAD_DES_DIR}
    mkdir -p ${BOOTMGR_DES_DIR}/patch

    echo Build relocator
    cd ${RELOC_DIR}
    make -f Makefile clean all

    echo 
    echo Build Preloader
    cd ${BOOTMGR_DIR}
    make -f MakefilePreloader clean all
    cp ${BOOTMGR_SRC_BIN} ${PRELOAD_DES_BIN}

    echo 
    echo Build Bootmanager

    cd ${BOOTMGR_DIR}
    make -f Makefile clean all
    cp ${BOOTMGR_SRC_BIN} ${BOOTMGR_DES_BIN}

    echo 
    echo Copy additional files
    cp ${BOOTMGR_SRC_SD}/ngCfg.json ${BOOTMGR_DES_DIR}/ngCfg.json
    cp ${BOOTMGR_SRC_SD}/ng-ofw1.sha ${BOOTMGR_DES_DIR}/ng-ofw1.sha
    cp ${BOOTMGR_SRC_SD}/patch/* ${BOOTMGR_DES_DIR}/patch/

    zip -r ${OUT_FILE} ${OUT_DIR}
else
    echo Wrong directory, missing subdirectories...
    echo ROOT=${ROOT}
    exit 1
fi
