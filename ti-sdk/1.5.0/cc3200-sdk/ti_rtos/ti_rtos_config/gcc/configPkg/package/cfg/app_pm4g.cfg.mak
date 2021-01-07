# invoke SourceDir generated makefile for app.pm4g
app.pm4g: .libraries,app.pm4g
.libraries,app.pm4g: package/cfg/app_pm4g.xdl
	$(MAKE) -f D:\git\a0132167_cc3200_sdk_1.2.0\source-1p2-rahul\ti_rtos\ti_rtos_config/src/makefile.libs

clean::
	$(MAKE) -f D:\git\a0132167_cc3200_sdk_1.2.0\source-1p2-rahul\ti_rtos\ti_rtos_config/src/makefile.libs clean

