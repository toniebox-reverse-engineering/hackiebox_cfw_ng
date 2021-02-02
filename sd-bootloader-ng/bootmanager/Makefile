#******************************************************************************
#
# Makefile - Rules for building the bootmgr application.
#
#  Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
#
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#
#    Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#
#    Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the
#    distribution.
#
#    Neither the name of Texas Instruments Incorporated nor the names of
#    its contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
#  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
#  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
#  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
#  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#*****************************************************************************

#VERBOSE=x

#
# The base directory.
#
ROOT=../..

#
# Set optimization level and linkscript
#
#OPT_LEVEL=s
ADD_LINK_SCRIPT=${PWD}/concatBootmanagerRelocator.sh

ifndef LINK_FILE
LINK_FILE=bootmgr.ld
endif

#
# Include the common make definitions.
#
include ${ROOT}/common/make/makeCommon

#
# Where to find source files that do not live in this directory.
#
#VPATH=..
VPATH+=${SDKROOT}/example/common

VPATH+=lib/FatFs
VPATH+=lib/jsmn-stream

#
# Where to find header files that do not live in the source directory.
#
#IPATH=..
#IPATH+=../../..
IPATH+=${SDKROOT}/inc
IPATH+=${SDKROOT}/driverlib
IPATH+=${SDKROOT}/simplelink
IPATH+=${SDKROOT}/simplelink/source
IPATH+=${SDKROOT}/simplelink/include
IPATH+=${SDKROOT}/simplelink_extlib/flc
IPATH+=${SDKROOT}/simplelink_extlib/provisioninglib

IPATH+=lib/FatFs
IPATH+=lib/jsmn-stream

#
# The default rule, which causes the driver library to be built.
#
all: ${OBJDIR} ${BINDIR}
all: ${BINDIR}/bootmgr.axf

#
# The rule to clean out all the build products.
#
clean:
	@rm -rf ${OBJDIR} ${wildcard *~}
	@rm -rf ${BINDIR} ${wildcard *~}


#
# The rule to create the target directories.
#
${OBJDIR}:
	@mkdir -p ${OBJDIR}

${BINDIR}:
	@mkdir -p ${BINDIR}

#
# Rules for building the bootmgr example.
#
${BINDIR}/bootmgr.axf: ${OBJDIR}/main.o
${BINDIR}/bootmgr.axf: ${OBJDIR}/helper.o
${BINDIR}/bootmgr.axf: ${OBJDIR}/config.o
${BINDIR}/bootmgr.axf: ${OBJDIR}/patch.o
${BINDIR}/bootmgr.axf: ${OBJDIR}/watchdog.o

${BINDIR}/bootmgr.axf: ${OBJDIR}/wiring.o

${BINDIR}/bootmgr.axf: ${OBJDIR}/udma_if.o

${BINDIR}/bootmgr.axf: ${OBJDIR}/ff.o
${BINDIR}/bootmgr.axf: ${OBJDIR}/diskio.o
${BINDIR}/bootmgr.axf: ${OBJDIR}/jsmn_stream.o

${BINDIR}/bootmgr.axf: ${OBJDIR}/ccsbcs.o
${BINDIR}/bootmgr.axf: ${OBJDIR}/network_common.o
${BINDIR}/bootmgr.axf: ${OBJDIR}/startup_${COMPILER}.o
${BINDIR}/bootmgr.axf: ${SDKROOT}/simplelink/${COMPILER}/${BINDIR}/libsimplelink_nonos_opt.a
${BINDIR}/bootmgr.axf: ${SDKROOT}/driverlib/${COMPILER}/${BINDIR}/libdriver.a

SCATTERgcc_bootmgr=${LINK_FILE}
ENTRY_bootmgr=ResetISR

#
# Include the automatically generated dependency files.
#
ifneq (${MAKECMDGOALS},clean)
-include ${wildcard ${COMPILER}/*.d} __dummy__
endif
