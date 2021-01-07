#******************************************************************************
#
# Makefile - Rules for building the ota_update_nonos application.
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

#
# The base directory.
#
ROOT=../../../..

#
# Include the common make definitions.
#
include ${ROOT}/tools/gcc_scripts/makedefs

#
# Where to find source files that do not live in this directory.
#
VPATH=..
VPATH+=../../../common


#
# Where to find header files that do not live in the source directory.
#
IPATH=..
IPATH+=../../..
IPATH+=../../../common
IPATH+=../../../../inc
IPATH+=../../../../oslib
IPATH+=../../../../driverlib
IPATH+=../../../../simplelink/source
IPATH+=../../../../simplelink/include
IPATH+=../../../../simplelink_extlib/flc
IPATH+=../../../../simplelink_extlib/include
IPATH+=../../../../third_party/FreeRTOS
IPATH+=../../../../third_party/FreeRTOS/source
IPATH+=../../../../third_party/FreeRTOS/source/portable/GCC/ARM_CM4
IPATH+=../../../../third_party/FreeRTOS/source/include
IPATH+=../../../../simplelink_extlib/provisioninglib

#
# The default rule, which causes the driver library to be built.
#
all: ${OBJDIR} ${BINDIR}
all: ${BINDIR}/ota_update_nonos.axf

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
# Rules for building the ota_update_nonos example.
#
${BINDIR}/ota_update_nonos.axf: ${OBJDIR}/main.o
${BINDIR}/ota_update_nonos.axf: ${OBJDIR}/pinmux.o
${BINDIR}/ota_update_nonos.axf: ${OBJDIR}/net.o
${BINDIR}/ota_update_nonos.axf: ${OBJDIR}/task.o
${BINDIR}/ota_update_nonos.axf: ${OBJDIR}/button_if.o
${BINDIR}/ota_update_nonos.axf: ${OBJDIR}/uart_if.o
${BINDIR}/ota_update_nonos.axf: ${OBJDIR}/udma_if.o
${BINDIR}/ota_update_nonos.axf: ${OBJDIR}/network_common.o
${BINDIR}/ota_update_nonos.axf: ${OBJDIR}/startup_${COMPILER}.o
${BINDIR}/ota_update_nonos.axf: ${ROOT}/simplelink_extlib/ota/${COMPILER}/${BINDIR}/ota.a
${BINDIR}/ota_update_nonos.axf: ${ROOT}/simplelink_extlib/flc/${COMPILER}/${BINDIR}/flc.a
${BINDIR}/ota_update_nonos.axf: ${ROOT}/simplelink/${COMPILER}/${BINDIR}/libsimplelink_nonos.a
${BINDIR}/ota_update_nonos.axf: ${ROOT}/driverlib/${COMPILER}/${BINDIR}/libdriver.a
SCATTERgcc_ota_update_nonos=ota_update_nonos.ld
ENTRY_ota_update_nonos=ResetISR

#
# Include the automatically generated dependency files.
#
ifneq (${MAKECMDGOALS},clean)
-include ${wildcard ${COMPILER}/*.d} __dummy__
endif
