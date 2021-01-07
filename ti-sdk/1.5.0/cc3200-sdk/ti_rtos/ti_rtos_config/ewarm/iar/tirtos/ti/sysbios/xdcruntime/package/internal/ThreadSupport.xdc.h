/*
 *  Do not modify this file; it is automatically 
 *  generated and any modifications will be overwritten.
 *
 * @(#) xdc-B06
 */

#ifndef ti_sysbios_xdcruntime_ThreadSupport__INTERNAL__
#define ti_sysbios_xdcruntime_ThreadSupport__INTERNAL__

#ifndef ti_sysbios_xdcruntime_ThreadSupport__internalaccess
#define ti_sysbios_xdcruntime_ThreadSupport__internalaccess
#endif

#include <ti/sysbios/xdcruntime/ThreadSupport.h>

#undef xdc_FILE__
#ifndef xdc_FILE
#define xdc_FILE__ NULL
#else
#define xdc_FILE__ xdc_FILE
#endif

/* self */
#undef ti_sysbios_xdcruntime_ThreadSupport_self
#define ti_sysbios_xdcruntime_ThreadSupport_self ti_sysbios_xdcruntime_ThreadSupport_self__E

/* start */
#undef ti_sysbios_xdcruntime_ThreadSupport_start
#define ti_sysbios_xdcruntime_ThreadSupport_start ti_sysbios_xdcruntime_ThreadSupport_start__E

/* yield */
#undef ti_sysbios_xdcruntime_ThreadSupport_yield
#define ti_sysbios_xdcruntime_ThreadSupport_yield ti_sysbios_xdcruntime_ThreadSupport_yield__E

/* compareOsPriorities */
#undef ti_sysbios_xdcruntime_ThreadSupport_compareOsPriorities
#define ti_sysbios_xdcruntime_ThreadSupport_compareOsPriorities ti_sysbios_xdcruntime_ThreadSupport_compareOsPriorities__E

/* sleep */
#undef ti_sysbios_xdcruntime_ThreadSupport_sleep
#define ti_sysbios_xdcruntime_ThreadSupport_sleep ti_sysbios_xdcruntime_ThreadSupport_sleep__E

/* join */
#undef ti_sysbios_xdcruntime_ThreadSupport_join
#define ti_sysbios_xdcruntime_ThreadSupport_join ti_sysbios_xdcruntime_ThreadSupport_join__E

/* getPriority */
#undef ti_sysbios_xdcruntime_ThreadSupport_getPriority
#define ti_sysbios_xdcruntime_ThreadSupport_getPriority ti_sysbios_xdcruntime_ThreadSupport_getPriority__E

/* setPriority */
#undef ti_sysbios_xdcruntime_ThreadSupport_setPriority
#define ti_sysbios_xdcruntime_ThreadSupport_setPriority ti_sysbios_xdcruntime_ThreadSupport_setPriority__E

/* getOsPriority */
#undef ti_sysbios_xdcruntime_ThreadSupport_getOsPriority
#define ti_sysbios_xdcruntime_ThreadSupport_getOsPriority ti_sysbios_xdcruntime_ThreadSupport_getOsPriority__E

/* setOsPriority */
#undef ti_sysbios_xdcruntime_ThreadSupport_setOsPriority
#define ti_sysbios_xdcruntime_ThreadSupport_setOsPriority ti_sysbios_xdcruntime_ThreadSupport_setOsPriority__E

/* getOsHandle */
#undef ti_sysbios_xdcruntime_ThreadSupport_getOsHandle
#define ti_sysbios_xdcruntime_ThreadSupport_getOsHandle ti_sysbios_xdcruntime_ThreadSupport_getOsHandle__E

/* getTls */
#undef ti_sysbios_xdcruntime_ThreadSupport_getTls
#define ti_sysbios_xdcruntime_ThreadSupport_getTls ti_sysbios_xdcruntime_ThreadSupport_getTls__E

/* setTls */
#undef ti_sysbios_xdcruntime_ThreadSupport_setTls
#define ti_sysbios_xdcruntime_ThreadSupport_setTls ti_sysbios_xdcruntime_ThreadSupport_setTls__E

/* stat */
#undef ti_sysbios_xdcruntime_ThreadSupport_stat
#define ti_sysbios_xdcruntime_ThreadSupport_stat ti_sysbios_xdcruntime_ThreadSupport_stat__E

/* runStub */
#define ThreadSupport_runStub ti_sysbios_xdcruntime_ThreadSupport_runStub__I

/* Module_startup */
#undef ti_sysbios_xdcruntime_ThreadSupport_Module_startup
#define ti_sysbios_xdcruntime_ThreadSupport_Module_startup ti_sysbios_xdcruntime_ThreadSupport_Module_startup__E

/* Instance_init */
#undef ti_sysbios_xdcruntime_ThreadSupport_Instance_init
#define ti_sysbios_xdcruntime_ThreadSupport_Instance_init ti_sysbios_xdcruntime_ThreadSupport_Instance_init__E

/* Instance_finalize */
#undef ti_sysbios_xdcruntime_ThreadSupport_Instance_finalize
#define ti_sysbios_xdcruntime_ThreadSupport_Instance_finalize ti_sysbios_xdcruntime_ThreadSupport_Instance_finalize__E

/* per-module runtime symbols */
#undef Module__MID
#define Module__MID ti_sysbios_xdcruntime_ThreadSupport_Module__id__C
#undef Module__DGSINCL
#define Module__DGSINCL ti_sysbios_xdcruntime_ThreadSupport_Module__diagsIncluded__C
#undef Module__DGSENAB
#define Module__DGSENAB ti_sysbios_xdcruntime_ThreadSupport_Module__diagsEnabled__C
#undef Module__DGSMASK
#define Module__DGSMASK ti_sysbios_xdcruntime_ThreadSupport_Module__diagsMask__C
#undef Module__LOGDEF
#define Module__LOGDEF ti_sysbios_xdcruntime_ThreadSupport_Module__loggerDefined__C
#undef Module__LOGOBJ
#define Module__LOGOBJ ti_sysbios_xdcruntime_ThreadSupport_Module__loggerObj__C
#undef Module__LOGFXN0
#define Module__LOGFXN0 ti_sysbios_xdcruntime_ThreadSupport_Module__loggerFxn0__C
#undef Module__LOGFXN1
#define Module__LOGFXN1 ti_sysbios_xdcruntime_ThreadSupport_Module__loggerFxn1__C
#undef Module__LOGFXN2
#define Module__LOGFXN2 ti_sysbios_xdcruntime_ThreadSupport_Module__loggerFxn2__C
#undef Module__LOGFXN4
#define Module__LOGFXN4 ti_sysbios_xdcruntime_ThreadSupport_Module__loggerFxn4__C
#undef Module__LOGFXN8
#define Module__LOGFXN8 ti_sysbios_xdcruntime_ThreadSupport_Module__loggerFxn8__C
#undef Module__G_OBJ
#define Module__G_OBJ ti_sysbios_xdcruntime_ThreadSupport_Module__gateObj__C
#undef Module__G_PRMS
#define Module__G_PRMS ti_sysbios_xdcruntime_ThreadSupport_Module__gatePrms__C
#undef Module__GP_create
#define Module__GP_create ti_sysbios_xdcruntime_ThreadSupport_Module_GateProxy_create
#undef Module__GP_delete
#define Module__GP_delete ti_sysbios_xdcruntime_ThreadSupport_Module_GateProxy_delete
#undef Module__GP_enter
#define Module__GP_enter ti_sysbios_xdcruntime_ThreadSupport_Module_GateProxy_enter
#undef Module__GP_leave
#define Module__GP_leave ti_sysbios_xdcruntime_ThreadSupport_Module_GateProxy_leave
#undef Module__GP_query
#define Module__GP_query ti_sysbios_xdcruntime_ThreadSupport_Module_GateProxy_query

/* Object__sizingError */
#line 1 "Error_inconsistent_object_size_in_ti.sysbios.xdcruntime.ThreadSupport"
typedef char ti_sysbios_xdcruntime_ThreadSupport_Object__sizingError[sizeof(ti_sysbios_xdcruntime_ThreadSupport_Object) > sizeof(ti_sysbios_xdcruntime_ThreadSupport_Struct) ? -1 : 1];


#endif /* ti_sysbios_xdcruntime_ThreadSupport__INTERNAL____ */
