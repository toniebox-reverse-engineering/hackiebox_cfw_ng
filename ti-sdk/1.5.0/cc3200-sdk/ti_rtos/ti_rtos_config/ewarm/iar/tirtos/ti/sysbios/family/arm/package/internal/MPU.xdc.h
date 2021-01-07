/*
 *  Do not modify this file; it is automatically 
 *  generated and any modifications will be overwritten.
 *
 * @(#) xdc-B06
 */

#ifndef ti_sysbios_family_arm_MPU__INTERNAL__
#define ti_sysbios_family_arm_MPU__INTERNAL__

#ifndef ti_sysbios_family_arm_MPU__internalaccess
#define ti_sysbios_family_arm_MPU__internalaccess
#endif

#include <ti/sysbios/family/arm/MPU.h>

#undef xdc_FILE__
#ifndef xdc_FILE
#define xdc_FILE__ NULL
#else
#define xdc_FILE__ xdc_FILE
#endif

/* disable */
#undef ti_sysbios_family_arm_MPU_disable
#define ti_sysbios_family_arm_MPU_disable ti_sysbios_family_arm_MPU_disable__E

/* enable */
#undef ti_sysbios_family_arm_MPU_enable
#define ti_sysbios_family_arm_MPU_enable ti_sysbios_family_arm_MPU_enable__E

/* disableBR */
#undef ti_sysbios_family_arm_MPU_disableBR
#define ti_sysbios_family_arm_MPU_disableBR ti_sysbios_family_arm_MPU_disableBR__E

/* enableBR */
#undef ti_sysbios_family_arm_MPU_enableBR
#define ti_sysbios_family_arm_MPU_enableBR ti_sysbios_family_arm_MPU_enableBR__E

/* initRegionAttrs */
#undef ti_sysbios_family_arm_MPU_initRegionAttrs
#define ti_sysbios_family_arm_MPU_initRegionAttrs ti_sysbios_family_arm_MPU_initRegionAttrs__E

/* isEnabled */
#undef ti_sysbios_family_arm_MPU_isEnabled
#define ti_sysbios_family_arm_MPU_isEnabled ti_sysbios_family_arm_MPU_isEnabled__E

/* setRegion */
#undef ti_sysbios_family_arm_MPU_setRegion
#define ti_sysbios_family_arm_MPU_setRegion ti_sysbios_family_arm_MPU_setRegion__E

/* startup */
#undef ti_sysbios_family_arm_MPU_startup
#define ti_sysbios_family_arm_MPU_startup ti_sysbios_family_arm_MPU_startup__E

/* disableAsm */
#define MPU_disableAsm ti_sysbios_family_arm_MPU_disableAsm__I

/* enableAsm */
#define MPU_enableAsm ti_sysbios_family_arm_MPU_enableAsm__I

/* disableBRAsm */
#define MPU_disableBRAsm ti_sysbios_family_arm_MPU_disableBRAsm__I

/* enableBRAsm */
#define MPU_enableBRAsm ti_sysbios_family_arm_MPU_enableBRAsm__I

/* isEnabledAsm */
#define MPU_isEnabledAsm ti_sysbios_family_arm_MPU_isEnabledAsm__I

/* setRegionAsm */
#define MPU_setRegionAsm ti_sysbios_family_arm_MPU_setRegionAsm__I

/* Module_startup */
#undef ti_sysbios_family_arm_MPU_Module_startup
#define ti_sysbios_family_arm_MPU_Module_startup ti_sysbios_family_arm_MPU_Module_startup__E

/* Instance_init */
#undef ti_sysbios_family_arm_MPU_Instance_init
#define ti_sysbios_family_arm_MPU_Instance_init ti_sysbios_family_arm_MPU_Instance_init__E

/* Instance_finalize */
#undef ti_sysbios_family_arm_MPU_Instance_finalize
#define ti_sysbios_family_arm_MPU_Instance_finalize ti_sysbios_family_arm_MPU_Instance_finalize__E

/* module */
#define MPU_module ((ti_sysbios_family_arm_MPU_Module_State *)(xdc__MODOBJADDR__(ti_sysbios_family_arm_MPU_Module__state__V)))
#if !defined(__cplusplus) || !defined(ti_sysbios_family_arm_MPU__cplusplus)
#define module ((ti_sysbios_family_arm_MPU_Module_State *)(xdc__MODOBJADDR__(ti_sysbios_family_arm_MPU_Module__state__V)))
#endif
/* per-module runtime symbols */
#undef Module__MID
#define Module__MID ti_sysbios_family_arm_MPU_Module__id__C
#undef Module__DGSINCL
#define Module__DGSINCL ti_sysbios_family_arm_MPU_Module__diagsIncluded__C
#undef Module__DGSENAB
#define Module__DGSENAB ti_sysbios_family_arm_MPU_Module__diagsEnabled__C
#undef Module__DGSMASK
#define Module__DGSMASK ti_sysbios_family_arm_MPU_Module__diagsMask__C
#undef Module__LOGDEF
#define Module__LOGDEF ti_sysbios_family_arm_MPU_Module__loggerDefined__C
#undef Module__LOGOBJ
#define Module__LOGOBJ ti_sysbios_family_arm_MPU_Module__loggerObj__C
#undef Module__LOGFXN0
#define Module__LOGFXN0 ti_sysbios_family_arm_MPU_Module__loggerFxn0__C
#undef Module__LOGFXN1
#define Module__LOGFXN1 ti_sysbios_family_arm_MPU_Module__loggerFxn1__C
#undef Module__LOGFXN2
#define Module__LOGFXN2 ti_sysbios_family_arm_MPU_Module__loggerFxn2__C
#undef Module__LOGFXN4
#define Module__LOGFXN4 ti_sysbios_family_arm_MPU_Module__loggerFxn4__C
#undef Module__LOGFXN8
#define Module__LOGFXN8 ti_sysbios_family_arm_MPU_Module__loggerFxn8__C
#undef Module__G_OBJ
#define Module__G_OBJ ti_sysbios_family_arm_MPU_Module__gateObj__C
#undef Module__G_PRMS
#define Module__G_PRMS ti_sysbios_family_arm_MPU_Module__gatePrms__C
#undef Module__GP_create
#define Module__GP_create ti_sysbios_family_arm_MPU_Module_GateProxy_create
#undef Module__GP_delete
#define Module__GP_delete ti_sysbios_family_arm_MPU_Module_GateProxy_delete
#undef Module__GP_enter
#define Module__GP_enter ti_sysbios_family_arm_MPU_Module_GateProxy_enter
#undef Module__GP_leave
#define Module__GP_leave ti_sysbios_family_arm_MPU_Module_GateProxy_leave
#undef Module__GP_query
#define Module__GP_query ti_sysbios_family_arm_MPU_Module_GateProxy_query


#endif /* ti_sysbios_family_arm_MPU__INTERNAL____ */
