#!/bin/bash

ROOT=$(cd -P -- "$(dirname -- "$0")" && pwd -P)/../..
export PATH="${ROOT}/arm-sdk/xpack-arm-none-eabi-gcc-10.2.1-1.1/bin:$PATH"
