/*
 *  Do not modify this file; it is automatically 
 *  generated and any modifications will be overwritten.
 *
 * @(#) xdc-B06
 */

/*
 * ======== GENERATED SECTIONS ========
 *     
 *     PROLOGUE
 *     INCLUDES
 *     
 *     MODULE-WIDE CONFIGS
 *     VIRTUAL FUNCTIONS
 *     FUNCTION DECLARATIONS
 *     CONVERTORS
 *     SYSTEM FUNCTIONS
 *     
 *     EPILOGUE
 *     PREFIX ALIASES
 */


/*
 * ======== PROLOGUE ========
 */

#ifndef ti_sysbios_hal_Seconds_SecondsProxy__include
#define ti_sysbios_hal_Seconds_SecondsProxy__include

#ifndef __nested__
#define __nested__
#define ti_sysbios_hal_Seconds_SecondsProxy__top__
#endif

#ifdef __cplusplus
#define __extern extern "C"
#else
#define __extern extern
#endif

#define ti_sysbios_hal_Seconds_SecondsProxy___VERS 160


/*
 * ======== INCLUDES ========
 */

#include <xdc/std.h>

#include <xdc/runtime/xdc.h>
#include <xdc/runtime/Types.h>
#include <ti/sysbios/hal/package/package.defs.h>

#include <ti/sysbios/interfaces/ISeconds.h>


/*
 * ======== AUXILIARY DEFINITIONS ========
 */

/* Time */
typedef ti_sysbios_interfaces_ISeconds_Time ti_sysbios_hal_Seconds_SecondsProxy_Time;


/*
 * ======== MODULE-WIDE CONFIGS ========
 */

/* Module__diagsEnabled */
typedef xdc_Bits32 CT__ti_sysbios_hal_Seconds_SecondsProxy_Module__diagsEnabled;
__extern __FAR__ const CT__ti_sysbios_hal_Seconds_SecondsProxy_Module__diagsEnabled ti_sysbios_hal_Seconds_SecondsProxy_Module__diagsEnabled__C;

/* Module__diagsIncluded */
typedef xdc_Bits32 CT__ti_sysbios_hal_Seconds_SecondsProxy_Module__diagsIncluded;
__extern __FAR__ const CT__ti_sysbios_hal_Seconds_SecondsProxy_Module__diagsIncluded ti_sysbios_hal_Seconds_SecondsProxy_Module__diagsIncluded__C;

/* Module__diagsMask */
typedef xdc_Bits16 *CT__ti_sysbios_hal_Seconds_SecondsProxy_Module__diagsMask;
__extern __FAR__ const CT__ti_sysbios_hal_Seconds_SecondsProxy_Module__diagsMask ti_sysbios_hal_Seconds_SecondsProxy_Module__diagsMask__C;

/* Module__gateObj */
typedef xdc_Ptr CT__ti_sysbios_hal_Seconds_SecondsProxy_Module__gateObj;
__extern __FAR__ const CT__ti_sysbios_hal_Seconds_SecondsProxy_Module__gateObj ti_sysbios_hal_Seconds_SecondsProxy_Module__gateObj__C;

/* Module__gatePrms */
typedef xdc_Ptr CT__ti_sysbios_hal_Seconds_SecondsProxy_Module__gatePrms;
__extern __FAR__ const CT__ti_sysbios_hal_Seconds_SecondsProxy_Module__gatePrms ti_sysbios_hal_Seconds_SecondsProxy_Module__gatePrms__C;

/* Module__id */
typedef xdc_runtime_Types_ModuleId CT__ti_sysbios_hal_Seconds_SecondsProxy_Module__id;
__extern __FAR__ const CT__ti_sysbios_hal_Seconds_SecondsProxy_Module__id ti_sysbios_hal_Seconds_SecondsProxy_Module__id__C;

/* Module__loggerDefined */
typedef xdc_Bool CT__ti_sysbios_hal_Seconds_SecondsProxy_Module__loggerDefined;
__extern __FAR__ const CT__ti_sysbios_hal_Seconds_SecondsProxy_Module__loggerDefined ti_sysbios_hal_Seconds_SecondsProxy_Module__loggerDefined__C;

/* Module__loggerObj */
typedef xdc_Ptr CT__ti_sysbios_hal_Seconds_SecondsProxy_Module__loggerObj;
__extern __FAR__ const CT__ti_sysbios_hal_Seconds_SecondsProxy_Module__loggerObj ti_sysbios_hal_Seconds_SecondsProxy_Module__loggerObj__C;

/* Module__loggerFxn0 */
typedef xdc_runtime_Types_LoggerFxn0 CT__ti_sysbios_hal_Seconds_SecondsProxy_Module__loggerFxn0;
__extern __FAR__ const CT__ti_sysbios_hal_Seconds_SecondsProxy_Module__loggerFxn0 ti_sysbios_hal_Seconds_SecondsProxy_Module__loggerFxn0__C;

/* Module__loggerFxn1 */
typedef xdc_runtime_Types_LoggerFxn1 CT__ti_sysbios_hal_Seconds_SecondsProxy_Module__loggerFxn1;
__extern __FAR__ const CT__ti_sysbios_hal_Seconds_SecondsProxy_Module__loggerFxn1 ti_sysbios_hal_Seconds_SecondsProxy_Module__loggerFxn1__C;

/* Module__loggerFxn2 */
typedef xdc_runtime_Types_LoggerFxn2 CT__ti_sysbios_hal_Seconds_SecondsProxy_Module__loggerFxn2;
__extern __FAR__ const CT__ti_sysbios_hal_Seconds_SecondsProxy_Module__loggerFxn2 ti_sysbios_hal_Seconds_SecondsProxy_Module__loggerFxn2__C;

/* Module__loggerFxn4 */
typedef xdc_runtime_Types_LoggerFxn4 CT__ti_sysbios_hal_Seconds_SecondsProxy_Module__loggerFxn4;
__extern __FAR__ const CT__ti_sysbios_hal_Seconds_SecondsProxy_Module__loggerFxn4 ti_sysbios_hal_Seconds_SecondsProxy_Module__loggerFxn4__C;

/* Module__loggerFxn8 */
typedef xdc_runtime_Types_LoggerFxn8 CT__ti_sysbios_hal_Seconds_SecondsProxy_Module__loggerFxn8;
__extern __FAR__ const CT__ti_sysbios_hal_Seconds_SecondsProxy_Module__loggerFxn8 ti_sysbios_hal_Seconds_SecondsProxy_Module__loggerFxn8__C;

/* Module__startupDoneFxn */
typedef xdc_Bool (*CT__ti_sysbios_hal_Seconds_SecondsProxy_Module__startupDoneFxn)(void);
__extern __FAR__ const CT__ti_sysbios_hal_Seconds_SecondsProxy_Module__startupDoneFxn ti_sysbios_hal_Seconds_SecondsProxy_Module__startupDoneFxn__C;

/* Object__count */
typedef xdc_Int CT__ti_sysbios_hal_Seconds_SecondsProxy_Object__count;
__extern __FAR__ const CT__ti_sysbios_hal_Seconds_SecondsProxy_Object__count ti_sysbios_hal_Seconds_SecondsProxy_Object__count__C;

/* Object__heap */
typedef xdc_runtime_IHeap_Handle CT__ti_sysbios_hal_Seconds_SecondsProxy_Object__heap;
__extern __FAR__ const CT__ti_sysbios_hal_Seconds_SecondsProxy_Object__heap ti_sysbios_hal_Seconds_SecondsProxy_Object__heap__C;

/* Object__sizeof */
typedef xdc_SizeT CT__ti_sysbios_hal_Seconds_SecondsProxy_Object__sizeof;
__extern __FAR__ const CT__ti_sysbios_hal_Seconds_SecondsProxy_Object__sizeof ti_sysbios_hal_Seconds_SecondsProxy_Object__sizeof__C;

/* Object__table */
typedef xdc_Ptr CT__ti_sysbios_hal_Seconds_SecondsProxy_Object__table;
__extern __FAR__ const CT__ti_sysbios_hal_Seconds_SecondsProxy_Object__table ti_sysbios_hal_Seconds_SecondsProxy_Object__table__C;


/*
 * ======== VIRTUAL FUNCTIONS ========
 */

/* Fxns__ */
struct ti_sysbios_hal_Seconds_SecondsProxy_Fxns__ {
    xdc_runtime_Types_Base* __base;
    const xdc_runtime_Types_SysFxns2 *__sysp;
    xdc_UInt32 (*get)(void);
    xdc_UInt32 (*getTime)(ti_sysbios_interfaces_ISeconds_Time*);
    xdc_Void (*set)(xdc_UInt32);
    xdc_runtime_Types_SysFxns2 __sfxns;
};

/* Module__FXNS__C */
__extern const ti_sysbios_hal_Seconds_SecondsProxy_Fxns__ ti_sysbios_hal_Seconds_SecondsProxy_Module__FXNS__C;


/*
 * ======== FUNCTION DECLARATIONS ========
 */

/* Module_startup */
#define ti_sysbios_hal_Seconds_SecondsProxy_Module_startup( state ) (-1)

/* Handle__label__S */
xdc__CODESECT(ti_sysbios_hal_Seconds_SecondsProxy_Handle__label__S, "ti_sysbios_hal_Seconds_SecondsProxy_Handle__label__S")
__extern xdc_runtime_Types_Label *ti_sysbios_hal_Seconds_SecondsProxy_Handle__label__S( xdc_Ptr obj, xdc_runtime_Types_Label *lab );

/* Module__startupDone__S */
xdc__CODESECT(ti_sysbios_hal_Seconds_SecondsProxy_Module__startupDone__S, "ti_sysbios_hal_Seconds_SecondsProxy_Module__startupDone__S")
__extern xdc_Bool ti_sysbios_hal_Seconds_SecondsProxy_Module__startupDone__S( void );

/* Object__create__S */
xdc__CODESECT(ti_sysbios_hal_Seconds_SecondsProxy_Object__create__S, "ti_sysbios_hal_Seconds_SecondsProxy_Object__create__S")
__extern xdc_Ptr ti_sysbios_hal_Seconds_SecondsProxy_Object__create__S( xdc_Ptr __oa, xdc_SizeT __osz, xdc_Ptr __aa, const xdc_UChar *__pa, xdc_SizeT __psz, xdc_runtime_Error_Block *__eb );

/* Object__delete__S */
xdc__CODESECT(ti_sysbios_hal_Seconds_SecondsProxy_Object__delete__S, "ti_sysbios_hal_Seconds_SecondsProxy_Object__delete__S")
__extern xdc_Void ti_sysbios_hal_Seconds_SecondsProxy_Object__delete__S( xdc_Ptr instp );

/* Object__destruct__S */
xdc__CODESECT(ti_sysbios_hal_Seconds_SecondsProxy_Object__destruct__S, "ti_sysbios_hal_Seconds_SecondsProxy_Object__destruct__S")
__extern xdc_Void ti_sysbios_hal_Seconds_SecondsProxy_Object__destruct__S( xdc_Ptr objp );

/* Object__get__S */
xdc__CODESECT(ti_sysbios_hal_Seconds_SecondsProxy_Object__get__S, "ti_sysbios_hal_Seconds_SecondsProxy_Object__get__S")
__extern xdc_Ptr ti_sysbios_hal_Seconds_SecondsProxy_Object__get__S( xdc_Ptr oarr, xdc_Int i );

/* Object__first__S */
xdc__CODESECT(ti_sysbios_hal_Seconds_SecondsProxy_Object__first__S, "ti_sysbios_hal_Seconds_SecondsProxy_Object__first__S")
__extern xdc_Ptr ti_sysbios_hal_Seconds_SecondsProxy_Object__first__S( void );

/* Object__next__S */
xdc__CODESECT(ti_sysbios_hal_Seconds_SecondsProxy_Object__next__S, "ti_sysbios_hal_Seconds_SecondsProxy_Object__next__S")
__extern xdc_Ptr ti_sysbios_hal_Seconds_SecondsProxy_Object__next__S( xdc_Ptr obj );

/* Params__init__S */
xdc__CODESECT(ti_sysbios_hal_Seconds_SecondsProxy_Params__init__S, "ti_sysbios_hal_Seconds_SecondsProxy_Params__init__S")
__extern xdc_Void ti_sysbios_hal_Seconds_SecondsProxy_Params__init__S( xdc_Ptr dst, const xdc_Void *src, xdc_SizeT psz, xdc_SizeT isz );

/* Proxy__abstract__S */
xdc__CODESECT(ti_sysbios_hal_Seconds_SecondsProxy_Proxy__abstract__S, "ti_sysbios_hal_Seconds_SecondsProxy_Proxy__abstract__S")
__extern xdc_Bool ti_sysbios_hal_Seconds_SecondsProxy_Proxy__abstract__S( void );

/* Proxy__delegate__S */
xdc__CODESECT(ti_sysbios_hal_Seconds_SecondsProxy_Proxy__delegate__S, "ti_sysbios_hal_Seconds_SecondsProxy_Proxy__delegate__S")
__extern xdc_Ptr ti_sysbios_hal_Seconds_SecondsProxy_Proxy__delegate__S( void );

/* get__E */
#define ti_sysbios_hal_Seconds_SecondsProxy_get ti_sysbios_hal_Seconds_SecondsProxy_get__E
xdc__CODESECT(ti_sysbios_hal_Seconds_SecondsProxy_get__E, "ti_sysbios_hal_Seconds_SecondsProxy_get")
__extern xdc_UInt32 ti_sysbios_hal_Seconds_SecondsProxy_get__E( void );

/* getTime__E */
#define ti_sysbios_hal_Seconds_SecondsProxy_getTime ti_sysbios_hal_Seconds_SecondsProxy_getTime__E
xdc__CODESECT(ti_sysbios_hal_Seconds_SecondsProxy_getTime__E, "ti_sysbios_hal_Seconds_SecondsProxy_getTime")
__extern xdc_UInt32 ti_sysbios_hal_Seconds_SecondsProxy_getTime__E( ti_sysbios_interfaces_ISeconds_Time *ts );

/* set__E */
#define ti_sysbios_hal_Seconds_SecondsProxy_set ti_sysbios_hal_Seconds_SecondsProxy_set__E
xdc__CODESECT(ti_sysbios_hal_Seconds_SecondsProxy_set__E, "ti_sysbios_hal_Seconds_SecondsProxy_set")
__extern xdc_Void ti_sysbios_hal_Seconds_SecondsProxy_set__E( xdc_UInt32 seconds );


/*
 * ======== CONVERTORS ========
 */

/* Module_upCast */
static inline ti_sysbios_interfaces_ISeconds_Module ti_sysbios_hal_Seconds_SecondsProxy_Module_upCast( void )
{
    return (ti_sysbios_interfaces_ISeconds_Module)ti_sysbios_hal_Seconds_SecondsProxy_Proxy__delegate__S();
}

/* Module_to_ti_sysbios_interfaces_ISeconds */
#define ti_sysbios_hal_Seconds_SecondsProxy_Module_to_ti_sysbios_interfaces_ISeconds ti_sysbios_hal_Seconds_SecondsProxy_Module_upCast


/*
 * ======== SYSTEM FUNCTIONS ========
 */

/* Module_startupDone */
#define ti_sysbios_hal_Seconds_SecondsProxy_Module_startupDone() ti_sysbios_hal_Seconds_SecondsProxy_Module__startupDone__S()

/* Object_heap */
#define ti_sysbios_hal_Seconds_SecondsProxy_Object_heap() ti_sysbios_hal_Seconds_SecondsProxy_Object__heap__C

/* Module_heap */
#define ti_sysbios_hal_Seconds_SecondsProxy_Module_heap() ti_sysbios_hal_Seconds_SecondsProxy_Object__heap__C

/* Module_id */
static inline CT__ti_sysbios_hal_Seconds_SecondsProxy_Module__id ti_sysbios_hal_Seconds_SecondsProxy_Module_id( void ) 
{
    return ti_sysbios_hal_Seconds_SecondsProxy_Module__id__C;
}

/* Proxy_abstract */
#define ti_sysbios_hal_Seconds_SecondsProxy_Proxy_abstract() ti_sysbios_hal_Seconds_SecondsProxy_Proxy__abstract__S()

/* Proxy_delegate */
#define ti_sysbios_hal_Seconds_SecondsProxy_Proxy_delegate() ((ti_sysbios_interfaces_ISeconds_Module)ti_sysbios_hal_Seconds_SecondsProxy_Proxy__delegate__S())


/*
 * ======== EPILOGUE ========
 */

#ifdef ti_sysbios_hal_Seconds_SecondsProxy__top__
#undef __nested__
#endif

#endif /* ti_sysbios_hal_Seconds_SecondsProxy__include */


/*
 * ======== PREFIX ALIASES ========
 */

#if !defined(__nested__) && !defined(ti_sysbios_hal_Seconds_SecondsProxy__nolocalnames)

#ifndef ti_sysbios_hal_Seconds_SecondsProxy__localnames__done
#define ti_sysbios_hal_Seconds_SecondsProxy__localnames__done

/* module prefix */
#define Seconds_SecondsProxy_Time ti_sysbios_hal_Seconds_SecondsProxy_Time
#define Seconds_SecondsProxy_get ti_sysbios_hal_Seconds_SecondsProxy_get
#define Seconds_SecondsProxy_getTime ti_sysbios_hal_Seconds_SecondsProxy_getTime
#define Seconds_SecondsProxy_set ti_sysbios_hal_Seconds_SecondsProxy_set
#define Seconds_SecondsProxy_Module_name ti_sysbios_hal_Seconds_SecondsProxy_Module_name
#define Seconds_SecondsProxy_Module_id ti_sysbios_hal_Seconds_SecondsProxy_Module_id
#define Seconds_SecondsProxy_Module_startup ti_sysbios_hal_Seconds_SecondsProxy_Module_startup
#define Seconds_SecondsProxy_Module_startupDone ti_sysbios_hal_Seconds_SecondsProxy_Module_startupDone
#define Seconds_SecondsProxy_Module_hasMask ti_sysbios_hal_Seconds_SecondsProxy_Module_hasMask
#define Seconds_SecondsProxy_Module_getMask ti_sysbios_hal_Seconds_SecondsProxy_Module_getMask
#define Seconds_SecondsProxy_Module_setMask ti_sysbios_hal_Seconds_SecondsProxy_Module_setMask
#define Seconds_SecondsProxy_Object_heap ti_sysbios_hal_Seconds_SecondsProxy_Object_heap
#define Seconds_SecondsProxy_Module_heap ti_sysbios_hal_Seconds_SecondsProxy_Module_heap
#define Seconds_SecondsProxy_Proxy_abstract ti_sysbios_hal_Seconds_SecondsProxy_Proxy_abstract
#define Seconds_SecondsProxy_Proxy_delegate ti_sysbios_hal_Seconds_SecondsProxy_Proxy_delegate
#define Seconds_SecondsProxy_Module_upCast ti_sysbios_hal_Seconds_SecondsProxy_Module_upCast
#define Seconds_SecondsProxy_Module_to_ti_sysbios_interfaces_ISeconds ti_sysbios_hal_Seconds_SecondsProxy_Module_to_ti_sysbios_interfaces_ISeconds

#endif /* ti_sysbios_hal_Seconds_SecondsProxy__localnames__done */
#endif
