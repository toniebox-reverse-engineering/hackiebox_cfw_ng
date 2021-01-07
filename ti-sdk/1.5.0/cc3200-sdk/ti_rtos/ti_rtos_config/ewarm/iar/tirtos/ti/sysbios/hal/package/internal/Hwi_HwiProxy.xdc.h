/*
 *  Do not modify this file; it is automatically 
 *  generated and any modifications will be overwritten.
 *
 * @(#) xdc-B06
 */

#ifndef ti_sysbios_hal_Hwi_HwiProxy__INTERNAL__
#define ti_sysbios_hal_Hwi_HwiProxy__INTERNAL__

#ifndef ti_sysbios_hal_Hwi_HwiProxy__internalaccess
#define ti_sysbios_hal_Hwi_HwiProxy__internalaccess
#endif

#include <ti/sysbios/hal/Hwi_HwiProxy.h>

#undef xdc_FILE__
#ifndef xdc_FILE
#define xdc_FILE__ NULL
#else
#define xdc_FILE__ xdc_FILE
#endif

/* getStackInfo */
#undef ti_sysbios_hal_Hwi_HwiProxy_getStackInfo
#define ti_sysbios_hal_Hwi_HwiProxy_getStackInfo ti_sysbios_hal_Hwi_HwiProxy_getStackInfo__E

/* getCoreStackInfo */
#undef ti_sysbios_hal_Hwi_HwiProxy_getCoreStackInfo
#define ti_sysbios_hal_Hwi_HwiProxy_getCoreStackInfo ti_sysbios_hal_Hwi_HwiProxy_getCoreStackInfo__E

/* startup */
#undef ti_sysbios_hal_Hwi_HwiProxy_startup
#define ti_sysbios_hal_Hwi_HwiProxy_startup ti_sysbios_hal_Hwi_HwiProxy_startup__E

/* disable */
#undef ti_sysbios_hal_Hwi_HwiProxy_disable
#define ti_sysbios_hal_Hwi_HwiProxy_disable ti_sysbios_hal_Hwi_HwiProxy_disable__E

/* enable */
#undef ti_sysbios_hal_Hwi_HwiProxy_enable
#define ti_sysbios_hal_Hwi_HwiProxy_enable ti_sysbios_hal_Hwi_HwiProxy_enable__E

/* restore */
#undef ti_sysbios_hal_Hwi_HwiProxy_restore
#define ti_sysbios_hal_Hwi_HwiProxy_restore ti_sysbios_hal_Hwi_HwiProxy_restore__E

/* switchFromBootStack */
#undef ti_sysbios_hal_Hwi_HwiProxy_switchFromBootStack
#define ti_sysbios_hal_Hwi_HwiProxy_switchFromBootStack ti_sysbios_hal_Hwi_HwiProxy_switchFromBootStack__E

/* post */
#undef ti_sysbios_hal_Hwi_HwiProxy_post
#define ti_sysbios_hal_Hwi_HwiProxy_post ti_sysbios_hal_Hwi_HwiProxy_post__E

/* getTaskSP */
#undef ti_sysbios_hal_Hwi_HwiProxy_getTaskSP
#define ti_sysbios_hal_Hwi_HwiProxy_getTaskSP ti_sysbios_hal_Hwi_HwiProxy_getTaskSP__E

/* disableInterrupt */
#undef ti_sysbios_hal_Hwi_HwiProxy_disableInterrupt
#define ti_sysbios_hal_Hwi_HwiProxy_disableInterrupt ti_sysbios_hal_Hwi_HwiProxy_disableInterrupt__E

/* enableInterrupt */
#undef ti_sysbios_hal_Hwi_HwiProxy_enableInterrupt
#define ti_sysbios_hal_Hwi_HwiProxy_enableInterrupt ti_sysbios_hal_Hwi_HwiProxy_enableInterrupt__E

/* restoreInterrupt */
#undef ti_sysbios_hal_Hwi_HwiProxy_restoreInterrupt
#define ti_sysbios_hal_Hwi_HwiProxy_restoreInterrupt ti_sysbios_hal_Hwi_HwiProxy_restoreInterrupt__E

/* clearInterrupt */
#undef ti_sysbios_hal_Hwi_HwiProxy_clearInterrupt
#define ti_sysbios_hal_Hwi_HwiProxy_clearInterrupt ti_sysbios_hal_Hwi_HwiProxy_clearInterrupt__E

/* getFunc */
#undef ti_sysbios_hal_Hwi_HwiProxy_getFunc
#define ti_sysbios_hal_Hwi_HwiProxy_getFunc ti_sysbios_hal_Hwi_HwiProxy_getFunc__E

/* setFunc */
#undef ti_sysbios_hal_Hwi_HwiProxy_setFunc
#define ti_sysbios_hal_Hwi_HwiProxy_setFunc ti_sysbios_hal_Hwi_HwiProxy_setFunc__E

/* getHookContext */
#undef ti_sysbios_hal_Hwi_HwiProxy_getHookContext
#define ti_sysbios_hal_Hwi_HwiProxy_getHookContext ti_sysbios_hal_Hwi_HwiProxy_getHookContext__E

/* setHookContext */
#undef ti_sysbios_hal_Hwi_HwiProxy_setHookContext
#define ti_sysbios_hal_Hwi_HwiProxy_setHookContext ti_sysbios_hal_Hwi_HwiProxy_setHookContext__E

/* getIrp */
#undef ti_sysbios_hal_Hwi_HwiProxy_getIrp
#define ti_sysbios_hal_Hwi_HwiProxy_getIrp ti_sysbios_hal_Hwi_HwiProxy_getIrp__E

/* Module_startup */
#undef ti_sysbios_hal_Hwi_HwiProxy_Module_startup
#define ti_sysbios_hal_Hwi_HwiProxy_Module_startup ti_sysbios_hal_Hwi_HwiProxy_Module_startup__E

/* Instance_init */
#undef ti_sysbios_hal_Hwi_HwiProxy_Instance_init
#define ti_sysbios_hal_Hwi_HwiProxy_Instance_init ti_sysbios_hal_Hwi_HwiProxy_Instance_init__E

/* Instance_finalize */
#undef ti_sysbios_hal_Hwi_HwiProxy_Instance_finalize
#define ti_sysbios_hal_Hwi_HwiProxy_Instance_finalize ti_sysbios_hal_Hwi_HwiProxy_Instance_finalize__E

/* per-module runtime symbols */
#undef Module__MID
#define Module__MID ti_sysbios_hal_Hwi_HwiProxy_Module__id__C
#undef Module__DGSINCL
#define Module__DGSINCL ti_sysbios_hal_Hwi_HwiProxy_Module__diagsIncluded__C
#undef Module__DGSENAB
#define Module__DGSENAB ti_sysbios_hal_Hwi_HwiProxy_Module__diagsEnabled__C
#undef Module__DGSMASK
#define Module__DGSMASK ti_sysbios_hal_Hwi_HwiProxy_Module__diagsMask__C
#undef Module__LOGDEF
#define Module__LOGDEF ti_sysbios_hal_Hwi_HwiProxy_Module__loggerDefined__C
#undef Module__LOGOBJ
#define Module__LOGOBJ ti_sysbios_hal_Hwi_HwiProxy_Module__loggerObj__C
#undef Module__LOGFXN0
#define Module__LOGFXN0 ti_sysbios_hal_Hwi_HwiProxy_Module__loggerFxn0__C
#undef Module__LOGFXN1
#define Module__LOGFXN1 ti_sysbios_hal_Hwi_HwiProxy_Module__loggerFxn1__C
#undef Module__LOGFXN2
#define Module__LOGFXN2 ti_sysbios_hal_Hwi_HwiProxy_Module__loggerFxn2__C
#undef Module__LOGFXN4
#define Module__LOGFXN4 ti_sysbios_hal_Hwi_HwiProxy_Module__loggerFxn4__C
#undef Module__LOGFXN8
#define Module__LOGFXN8 ti_sysbios_hal_Hwi_HwiProxy_Module__loggerFxn8__C
#undef Module__G_OBJ
#define Module__G_OBJ ti_sysbios_hal_Hwi_HwiProxy_Module__gateObj__C
#undef Module__G_PRMS
#define Module__G_PRMS ti_sysbios_hal_Hwi_HwiProxy_Module__gatePrms__C
#undef Module__GP_create
#define Module__GP_create ti_sysbios_hal_Hwi_HwiProxy_Module_GateProxy_create
#undef Module__GP_delete
#define Module__GP_delete ti_sysbios_hal_Hwi_HwiProxy_Module_GateProxy_delete
#undef Module__GP_enter
#define Module__GP_enter ti_sysbios_hal_Hwi_HwiProxy_Module_GateProxy_enter
#undef Module__GP_leave
#define Module__GP_leave ti_sysbios_hal_Hwi_HwiProxy_Module_GateProxy_leave
#undef Module__GP_query
#define Module__GP_query ti_sysbios_hal_Hwi_HwiProxy_Module_GateProxy_query

/* Object__sizingError */
#line 1 "Error_inconsistent_object_size_in_ti.sysbios.hal.Hwi_HwiProxy"
typedef char ti_sysbios_hal_Hwi_HwiProxy_Object__sizingError[sizeof(ti_sysbios_hal_Hwi_HwiProxy_Object) > sizeof(ti_sysbios_hal_Hwi_HwiProxy_Struct) ? -1 : 1];


#endif /* ti_sysbios_hal_Hwi_HwiProxy__INTERNAL____ */
