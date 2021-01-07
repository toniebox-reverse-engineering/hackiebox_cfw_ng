/*
 *  Do not modify this file; it is automatically 
 *  generated and any modifications will be overwritten.
 *
 * @(#) xdc-B06
 */

#ifndef ti_sysbios_hal_Timer__INTERNAL__
#define ti_sysbios_hal_Timer__INTERNAL__

#ifndef ti_sysbios_hal_Timer__internalaccess
#define ti_sysbios_hal_Timer__internalaccess
#endif

#include <ti/sysbios/hal/Timer.h>

#undef xdc_FILE__
#ifndef xdc_FILE
#define xdc_FILE__ NULL
#else
#define xdc_FILE__ xdc_FILE
#endif

/* getNumTimers */
#undef ti_sysbios_hal_Timer_getNumTimers
#define ti_sysbios_hal_Timer_getNumTimers ti_sysbios_hal_Timer_getNumTimers__E

/* getStatus */
#undef ti_sysbios_hal_Timer_getStatus
#define ti_sysbios_hal_Timer_getStatus ti_sysbios_hal_Timer_getStatus__E

/* startup */
#undef ti_sysbios_hal_Timer_startup
#define ti_sysbios_hal_Timer_startup ti_sysbios_hal_Timer_startup__E

/* getMaxTicks */
#undef ti_sysbios_hal_Timer_getMaxTicks
#define ti_sysbios_hal_Timer_getMaxTicks ti_sysbios_hal_Timer_getMaxTicks__E

/* setNextTick */
#undef ti_sysbios_hal_Timer_setNextTick
#define ti_sysbios_hal_Timer_setNextTick ti_sysbios_hal_Timer_setNextTick__E

/* start */
#undef ti_sysbios_hal_Timer_start
#define ti_sysbios_hal_Timer_start ti_sysbios_hal_Timer_start__E

/* stop */
#undef ti_sysbios_hal_Timer_stop
#define ti_sysbios_hal_Timer_stop ti_sysbios_hal_Timer_stop__E

/* setPeriod */
#undef ti_sysbios_hal_Timer_setPeriod
#define ti_sysbios_hal_Timer_setPeriod ti_sysbios_hal_Timer_setPeriod__E

/* setPeriodMicroSecs */
#undef ti_sysbios_hal_Timer_setPeriodMicroSecs
#define ti_sysbios_hal_Timer_setPeriodMicroSecs ti_sysbios_hal_Timer_setPeriodMicroSecs__E

/* getPeriod */
#undef ti_sysbios_hal_Timer_getPeriod
#define ti_sysbios_hal_Timer_getPeriod ti_sysbios_hal_Timer_getPeriod__E

/* getCount */
#undef ti_sysbios_hal_Timer_getCount
#define ti_sysbios_hal_Timer_getCount ti_sysbios_hal_Timer_getCount__E

/* getFreq */
#undef ti_sysbios_hal_Timer_getFreq
#define ti_sysbios_hal_Timer_getFreq ti_sysbios_hal_Timer_getFreq__E

/* getFunc */
#undef ti_sysbios_hal_Timer_getFunc
#define ti_sysbios_hal_Timer_getFunc ti_sysbios_hal_Timer_getFunc__E

/* setFunc */
#undef ti_sysbios_hal_Timer_setFunc
#define ti_sysbios_hal_Timer_setFunc ti_sysbios_hal_Timer_setFunc__E

/* trigger */
#undef ti_sysbios_hal_Timer_trigger
#define ti_sysbios_hal_Timer_trigger ti_sysbios_hal_Timer_trigger__E

/* getExpiredCounts */
#undef ti_sysbios_hal_Timer_getExpiredCounts
#define ti_sysbios_hal_Timer_getExpiredCounts ti_sysbios_hal_Timer_getExpiredCounts__E

/* getExpiredTicks */
#undef ti_sysbios_hal_Timer_getExpiredTicks
#define ti_sysbios_hal_Timer_getExpiredTicks ti_sysbios_hal_Timer_getExpiredTicks__E

/* getCurrentTick */
#undef ti_sysbios_hal_Timer_getCurrentTick
#define ti_sysbios_hal_Timer_getCurrentTick ti_sysbios_hal_Timer_getCurrentTick__E

/* Module_startup */
#undef ti_sysbios_hal_Timer_Module_startup
#define ti_sysbios_hal_Timer_Module_startup ti_sysbios_hal_Timer_Module_startup__E

/* Instance_init */
#undef ti_sysbios_hal_Timer_Instance_init
#define ti_sysbios_hal_Timer_Instance_init ti_sysbios_hal_Timer_Instance_init__E

/* Instance_finalize */
#undef ti_sysbios_hal_Timer_Instance_finalize
#define ti_sysbios_hal_Timer_Instance_finalize ti_sysbios_hal_Timer_Instance_finalize__E

/* per-module runtime symbols */
#undef Module__MID
#define Module__MID ti_sysbios_hal_Timer_Module__id__C
#undef Module__DGSINCL
#define Module__DGSINCL ti_sysbios_hal_Timer_Module__diagsIncluded__C
#undef Module__DGSENAB
#define Module__DGSENAB ti_sysbios_hal_Timer_Module__diagsEnabled__C
#undef Module__DGSMASK
#define Module__DGSMASK ti_sysbios_hal_Timer_Module__diagsMask__C
#undef Module__LOGDEF
#define Module__LOGDEF ti_sysbios_hal_Timer_Module__loggerDefined__C
#undef Module__LOGOBJ
#define Module__LOGOBJ ti_sysbios_hal_Timer_Module__loggerObj__C
#undef Module__LOGFXN0
#define Module__LOGFXN0 ti_sysbios_hal_Timer_Module__loggerFxn0__C
#undef Module__LOGFXN1
#define Module__LOGFXN1 ti_sysbios_hal_Timer_Module__loggerFxn1__C
#undef Module__LOGFXN2
#define Module__LOGFXN2 ti_sysbios_hal_Timer_Module__loggerFxn2__C
#undef Module__LOGFXN4
#define Module__LOGFXN4 ti_sysbios_hal_Timer_Module__loggerFxn4__C
#undef Module__LOGFXN8
#define Module__LOGFXN8 ti_sysbios_hal_Timer_Module__loggerFxn8__C
#undef Module__G_OBJ
#define Module__G_OBJ ti_sysbios_hal_Timer_Module__gateObj__C
#undef Module__G_PRMS
#define Module__G_PRMS ti_sysbios_hal_Timer_Module__gatePrms__C
#undef Module__GP_create
#define Module__GP_create ti_sysbios_hal_Timer_Module_GateProxy_create
#undef Module__GP_delete
#define Module__GP_delete ti_sysbios_hal_Timer_Module_GateProxy_delete
#undef Module__GP_enter
#define Module__GP_enter ti_sysbios_hal_Timer_Module_GateProxy_enter
#undef Module__GP_leave
#define Module__GP_leave ti_sysbios_hal_Timer_Module_GateProxy_leave
#undef Module__GP_query
#define Module__GP_query ti_sysbios_hal_Timer_Module_GateProxy_query

/* Object__sizingError */
#line 1 "Error_inconsistent_object_size_in_ti.sysbios.hal.Timer"
typedef char ti_sysbios_hal_Timer_Object__sizingError[sizeof(ti_sysbios_hal_Timer_Object) > sizeof(ti_sysbios_hal_Timer_Struct) ? -1 : 1];


#endif /* ti_sysbios_hal_Timer__INTERNAL____ */
