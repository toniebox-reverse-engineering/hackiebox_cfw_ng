# invoke SourceDir generated makefile for app.pem4
app.pem4: .libraries,app.pem4
.libraries,app.pem4: package/cfg/app_pem4.xdl
	$(MAKE) -f D:\Prathyusha\CC31xx\CC31xx\My_Workspace\TI_RTOS_compilers\cc3200-sdk\ti_rtos\ti_rtos_config/src/makefile.libs

clean::
	$(MAKE) -f D:\Prathyusha\CC31xx\CC31xx\My_Workspace\TI_RTOS_compilers\cc3200-sdk\ti_rtos\ti_rtos_config/src/makefile.libs clean

