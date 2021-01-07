#
_XDCBUILDCOUNT = 0
ifneq (,$(findstring path,$(_USEXDCENV_)))
override XDCPATH = C:/ti/tirtos_cc32xx_2_15_00_17/packages;C:/ti/tirtos_cc32xx_2_15_00_17/products/uia_2_00_02_39/packages;C:/ti/tirtos_cc32xx_2_15_00_17/products/bios_6_45_00_20/packages
override XDCROOT = C:/ti/xdctools_3_32_00_06_core
override XDCBUILDCFG = ./config.bld
endif
ifneq (,$(findstring args,$(_USEXDCENV_)))
override XDCARGS = 
override XDCTARGETS = 
endif
#
ifeq (0,1)
PKGPATH = C:/ti/tirtos_cc32xx_2_15_00_17/packages;C:/ti/tirtos_cc32xx_2_15_00_17/products/uia_2_00_02_39/packages;C:/ti/tirtos_cc32xx_2_15_00_17/products/bios_6_45_00_20/packages;C:/ti/xdctools_3_32_00_06_core/packages;..
HOSTOS = Windows
endif
