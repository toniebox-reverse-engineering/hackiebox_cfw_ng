# invoke SourceDir generated makefile for ti_rtos_config.pm4g
ti_rtos_config.pm4g: .libraries,ti_rtos_config.pm4g
.libraries,ti_rtos_config.pm4g: package/cfg/ti_rtos_config_pm4g.xdl
	$(MAKE) -f D:\Prathyusha\CC31xx\CC31xx\My_Workspace\SDK0.5_rel\cc3200-sdk\ti_rtos\ti_rtos_config\GCC/src/makefile.libs

clean::
	$(MAKE) -f D:\Prathyusha\CC31xx\CC31xx\My_Workspace\SDK0.5_rel\cc3200-sdk\ti_rtos\ti_rtos_config\GCC/src/makefile.libs clean

