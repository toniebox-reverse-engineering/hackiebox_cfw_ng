#!/bin/bash
set -e 

#Change into script directory
cd $(cd -P -- "$(dirname -- "$0")" && pwd -P)
#Change to repository root
cd ..
ROOT=$PWD

source ${ROOT}/common/make/sdkPath
source ${ROOT}/common/make/armGccPath
export PATH="${ARMGCC_BIN}:$PATH"

if [ "$1" == "debug" ]; then
    DEBUG_APPENDIX_RELOC=Debug
    DEBUG_APPENDIX=Log
    DEBUG_APPENDIX_ZIP=_debug
    DEBUG_PATH_APPENDIX=debug/
else
    DEBUG_APPENDIX=""
    DEBUG_APPENDIX_ZIP=""
    DEBUG_PATH_APPENDIX="build/"
fi

OUT_DIR=${ROOT}/exe/sd-bootloader-ng
OUT_FILE=${ROOT}/exe/sd-bootloader-ng${DEBUG_APPENDIX_ZIP}.zip

DRIVELIB_DIR=${SDKROOT}/driverlib/gcc
SIMPLELINK_DIR=${SDKROOT}/simplelink/gcc
RELOC_DIR=${ROOT}/sd-bootloader-ng/relocator
BOOTMGR_DIR=${ROOT}/sd-bootloader-ng/bootmanager

BOOTMGR_SRC_BIN=${BOOTMGR_DIR}/exe/${DEBUG_PATH_APPENDIX}/bootloader/bootmgr.relocator.bin
BOOTMGR_SRC_SD=${BOOTMGR_DIR}/sd/revvox/boot

PRELOAD_SRC_BIN=${BOOTMGR_DIR}/exe/${DEBUG_PATH_APPENDIX}/preloader/bootmgr.relocator.bin

PRELOAD_DES_DIR=${OUT_DIR}/flash/sys
BOOTMGR_DES_DIR=${OUT_DIR}/sd/revvox/boot

PRELOAD_DES_BIN=${PRELOAD_DES_DIR}/mcuimg.bin
BOOTMGR_DES_BIN=${BOOTMGR_DES_DIR}/ngbootloader.bin

if [ -d "$RELOC_DIR" ] && [ -d "$BOOTMGR_DIR" ]; then
    echo Clean output directory
    rm -f ${OUT_FILE}
    rm -rf ${OUT_DIR}

    echo 
    echo Create output directory
    mkdir -p ${OUT_DIR}
    mkdir -p ${PRELOAD_DES_DIR}
    mkdir -p ${BOOTMGR_DES_DIR}/patch

    echo 
    echo Build driverlib
    cd ${DRIVELIB_DIR}
    make -f ${ROOT}/common/make/MakefileDriverlib clean all

    echo 
    echo Build libsimplelink_nonos_opt
    cd ${SIMPLELINK_DIR}
    make -f ${ROOT}/common/make/MakefileSimplelink_opt target=NONOS clean all

    echo 
    echo Build relocator
    cd ${RELOC_DIR}
    make -f Makefile${DEBUG_APPENDIX_RELOC} clean all

    echo 
    echo Build Preloader
    cd ${BOOTMGR_DIR}
    make -f Makefile${DEBUG_APPENDIX}Preloader clean all
    cp ${PRELOAD_SRC_BIN} ${PRELOAD_DES_BIN}

    echo 
    echo Build Bootmanager

    cd ${BOOTMGR_DIR}
    make -f Makefile${DEBUG_APPENDIX} clean all
    cp ${BOOTMGR_SRC_BIN} ${BOOTMGR_DES_BIN}

    echo 
    echo Copy additional files
    cp ${BOOTMGR_SRC_SD}/ngCfg.json ${BOOTMGR_DES_DIR}/ngCfg.json
    cp ${BOOTMGR_SRC_SD}/ng-ofw1.sha ${BOOTMGR_DES_DIR}/ng-ofw1.sha
    cp ${BOOTMGR_SRC_SD}/patch/* ${BOOTMGR_DES_DIR}/patch/

    cd ${OUT_DIR}
    zip -r ${OUT_FILE} *
else
    echo Wrong directory, missing subdirectories...
    echo ROOT=${ROOT}
    exit 1
fi
