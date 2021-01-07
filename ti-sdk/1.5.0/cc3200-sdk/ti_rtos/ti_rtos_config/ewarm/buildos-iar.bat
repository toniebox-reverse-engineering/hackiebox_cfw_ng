@echo off
::******************************************************************************
::
:: Makefile - Rules for building the FreeRTOS Library.
::
::  Copyright (C) 2014 Texas Instruments Incorporated - http:::www.ti.com/
::
::
::  Redistribution and use in source and binary forms, with or without
::  modification, are permitted provided that the following conditions
::  are met:
::
::    Redistributions of source code must retain the above copyright
::    notice, this list of conditions and the following disclaimer.
::
::    Redistributions in binary form must reproduce the above copyright
::    notice, this list of conditions and the following disclaimer in the
::    documentation and/or other materials provided with the
::    distribution.
::
::    Neither the name of Texas Instruments Incorporated nor the names of
::    its contributors may be used to endorse or promote products derived
::    from this software without specific prior written permission.
::
::  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
::  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
::  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
::  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
::  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
::  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
::  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
::  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
::  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
::  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
::  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
::
::*****************************************************************************

:: Set the IAR installation path as per user's PC
set CGTOOLS=C:/Program Files (x86)/IAR Systems/Embedded Workbench 7.0/arm
:: Set the Latest XDC Tool path
set XDCTOOLS=C:/ti/xdctools_3_32_00_06_core
:: Set the latest TI-RTOS for SimpleLink path
set SYSBIOS=C:/ti/tirtos_cc32xx_2_15_00_17/products/bios_6_45_00_20

set BASE=%CD%

cd..

echo making 'app' baseimage ...

rmdir /s /q app

%XDCTOOLS%\xs --xdcpath="%SYSBIOS%/packages" xdc.tools.configuro -c "%CGTOOLS%" -t iar.targets.arm.M4 -p ti.platforms.simplelink:CC3200 -r release app.cfg

cd ewarm/iar
rmdir /s /q tirtos >nul
cd ..

set OSIMAGE=iar/tirtos
%XDCTOOLS%\xs --xdcpath="%SYSBIOS%/packages" xdc.tools.closure -f -d "%OSIMAGE%" ../app


cd ..
rmdir /s /q app

