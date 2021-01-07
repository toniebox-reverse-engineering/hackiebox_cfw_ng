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
 *     INTERNAL DEFINITIONS
 *     MODULE-WIDE CONFIGS
 *     PER-INSTANCE TYPES
 *     VIRTUAL FUNCTIONS
 *     FUNCTION DECLARATIONS
 *     CONVERTORS
 *     SYSTEM FUNCTIONS
 *     
 *     EPILOGUE
 *     STATE STRUCTURES
 *     PREFIX ALIASES
 */


/*
 * ======== PROLOGUE ========
 */

#ifndef ti_sysbios_heaps_HeapMem__include
#define ti_sysbios_heaps_HeapMem__include

#ifndef __nested__
#define __nested__
#define ti_sysbios_heaps_HeapMem__top__
#endif

#ifdef __cplusplus
#define __extern extern "C"
#else
#define __extern extern
#endif

#define ti_sysbios_heaps_HeapMem___VERS 160


/*
 * ======== INCLUDES ========
 */

#include <xdc/std.h>

#include <xdc/runtime/xdc.h>
#include <xdc/runtime/Types.h>
#include <xdc/runtime/IInstance.h>
#include <ti/sysbios/heaps/package/package.defs.h>

#include <xdc/runtime/Memory.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/IHeap.h>
#include <xdc/runtime/Assert.h>
#include <xdc/runtime/IGateProvider.h>
#include <ti/sysbios/heaps/package/HeapMem_Module_GateProxy.h>


/*
 * ======== AUXILIARY DEFINITIONS ========
 */

/* ExtendedStats */
struct ti_sysbios_heaps_HeapMem_ExtendedStats {
    xdc_Ptr buf;
    xdc_SizeT size;
};


/*
 * ======== INTERNAL DEFINITIONS ========
 */

/* Header */
struct ti_sysbios_heaps_HeapMem_Header {
    ti_sysbios_heaps_HeapMem_Header *next;
    xdc_runtime_Memory_Size size;
};

/* Instance_State */
typedef xdc_Char __T1_ti_sysbios_heaps_HeapMem_Instance_State__buf;
typedef xdc_Char *__ARRAY1_ti_sysbios_heaps_HeapMem_Instance_State__buf;
typedef __ARRAY1_ti_sysbios_heaps_HeapMem_Instance_State__buf __TA_ti_sysbios_heaps_HeapMem_Instance_State__buf;


/*
 * ======== MODULE-WIDE CONFIGS ========
 */

/* Module__diagsEnabled */
typedef xdc_Bits32 CT__ti_sysbios_heaps_HeapMem_Module__diagsEnabled;
__extern __FAR__ const CT__ti_sysbios_heaps_HeapMem_Module__diagsEnabled ti_sysbios_heaps_HeapMem_Module__diagsEnabled__C;

/* Module__diagsIncluded */
typedef xdc_Bits32 CT__ti_sysbios_heaps_HeapMem_Module__diagsIncluded;
__extern __FAR__ const CT__ti_sysbios_heaps_HeapMem_Module__diagsIncluded ti_sysbios_heaps_HeapMem_Module__diagsIncluded__C;

/* Module__diagsMask */
typedef xdc_Bits16 *CT__ti_sysbios_heaps_HeapMem_Module__diagsMask;
__extern __FAR__ const CT__ti_sysbios_heaps_HeapMem_Module__diagsMask ti_sysbios_heaps_HeapMem_Module__diagsMask__C;

/* Module__gateObj */
typedef xdc_Ptr CT__ti_sysbios_heaps_HeapMem_Module__gateObj;
__extern __FAR__ const CT__ti_sysbios_heaps_HeapMem_Module__gateObj ti_sysbios_heaps_HeapMem_Module__gateObj__C;

/* Module__gatePrms */
typedef xdc_Ptr CT__ti_sysbios_heaps_HeapMem_Module__gatePrms;
__extern __FAR__ const CT__ti_sysbios_heaps_HeapMem_Module__gatePrms ti_sysbios_heaps_HeapMem_Module__gatePrms__C;

/* Module__id */
typedef xdc_runtime_Types_ModuleId CT__ti_sysbios_heaps_HeapMem_Module__id;
__extern __FAR__ const CT__ti_sysbios_heaps_HeapMem_Module__id ti_sysbios_heaps_HeapMem_Module__id__C;

/* Module__loggerDefined */
typedef xdc_Bool CT__ti_sysbios_heaps_HeapMem_Module__loggerDefined;
__extern __FAR__ const CT__ti_sysbios_heaps_HeapMem_Module__loggerDefined ti_sysbios_heaps_HeapMem_Module__loggerDefined__C;

/* Module__loggerObj */
typedef xdc_Ptr CT__ti_sysbios_heaps_HeapMem_Module__loggerObj;
__extern __FAR__ const CT__ti_sysbios_heaps_HeapMem_Module__loggerObj ti_sysbios_heaps_HeapMem_Module__loggerObj__C;

/* Module__loggerFxn0 */
typedef xdc_runtime_Types_LoggerFxn0 CT__ti_sysbios_heaps_HeapMem_Module__loggerFxn0;
__extern __FAR__ const CT__ti_sysbios_heaps_HeapMem_Module__loggerFxn0 ti_sysbios_heaps_HeapMem_Module__loggerFxn0__C;

/* Module__loggerFxn1 */
typedef xdc_runtime_Types_LoggerFxn1 CT__ti_sysbios_heaps_HeapMem_Module__loggerFxn1;
__extern __FAR__ const CT__ti_sysbios_heaps_HeapMem_Module__loggerFxn1 ti_sysbios_heaps_HeapMem_Module__loggerFxn1__C;

/* Module__loggerFxn2 */
typedef xdc_runtime_Types_LoggerFxn2 CT__ti_sysbios_heaps_HeapMem_Module__loggerFxn2;
__extern __FAR__ const CT__ti_sysbios_heaps_HeapMem_Module__loggerFxn2 ti_sysbios_heaps_HeapMem_Module__loggerFxn2__C;

/* Module__loggerFxn4 */
typedef xdc_runtime_Types_LoggerFxn4 CT__ti_sysbios_heaps_HeapMem_Module__loggerFxn4;
__extern __FAR__ const CT__ti_sysbios_heaps_HeapMem_Module__loggerFxn4 ti_sysbios_heaps_HeapMem_Module__loggerFxn4__C;

/* Module__loggerFxn8 */
typedef xdc_runtime_Types_LoggerFxn8 CT__ti_sysbios_heaps_HeapMem_Module__loggerFxn8;
__extern __FAR__ const CT__ti_sysbios_heaps_HeapMem_Module__loggerFxn8 ti_sysbios_heaps_HeapMem_Module__loggerFxn8__C;

/* Module__startupDoneFxn */
typedef xdc_Bool (*CT__ti_sysbios_heaps_HeapMem_Module__startupDoneFxn)(void);
__extern __FAR__ const CT__ti_sysbios_heaps_HeapMem_Module__startupDoneFxn ti_sysbios_heaps_HeapMem_Module__startupDoneFxn__C;

/* Object__count */
typedef xdc_Int CT__ti_sysbios_heaps_HeapMem_Object__count;
__extern __FAR__ const CT__ti_sysbios_heaps_HeapMem_Object__count ti_sysbios_heaps_HeapMem_Object__count__C;

/* Object__heap */
typedef xdc_runtime_IHeap_Handle CT__ti_sysbios_heaps_HeapMem_Object__heap;
__extern __FAR__ const CT__ti_sysbios_heaps_HeapMem_Object__heap ti_sysbios_heaps_HeapMem_Object__heap__C;

/* Object__sizeof */
typedef xdc_SizeT CT__ti_sysbios_heaps_HeapMem_Object__sizeof;
__extern __FAR__ const CT__ti_sysbios_heaps_HeapMem_Object__sizeof ti_sysbios_heaps_HeapMem_Object__sizeof__C;

/* Object__table */
typedef xdc_Ptr CT__ti_sysbios_heaps_HeapMem_Object__table;
__extern __FAR__ const CT__ti_sysbios_heaps_HeapMem_Object__table ti_sysbios_heaps_HeapMem_Object__table__C;

/* A_zeroBlock */
#define ti_sysbios_heaps_HeapMem_A_zeroBlock (ti_sysbios_heaps_HeapMem_A_zeroBlock__C)
typedef xdc_runtime_Assert_Id CT__ti_sysbios_heaps_HeapMem_A_zeroBlock;
__extern __FAR__ const CT__ti_sysbios_heaps_HeapMem_A_zeroBlock ti_sysbios_heaps_HeapMem_A_zeroBlock__C;

/* A_heapSize */
#define ti_sysbios_heaps_HeapMem_A_heapSize (ti_sysbios_heaps_HeapMem_A_heapSize__C)
typedef xdc_runtime_Assert_Id CT__ti_sysbios_heaps_HeapMem_A_heapSize;
__extern __FAR__ const CT__ti_sysbios_heaps_HeapMem_A_heapSize ti_sysbios_heaps_HeapMem_A_heapSize__C;

/* A_align */
#define ti_sysbios_heaps_HeapMem_A_align (ti_sysbios_heaps_HeapMem_A_align__C)
typedef xdc_runtime_Assert_Id CT__ti_sysbios_heaps_HeapMem_A_align;
__extern __FAR__ const CT__ti_sysbios_heaps_HeapMem_A_align ti_sysbios_heaps_HeapMem_A_align__C;

/* E_memory */
#define ti_sysbios_heaps_HeapMem_E_memory (ti_sysbios_heaps_HeapMem_E_memory__C)
typedef xdc_runtime_Error_Id CT__ti_sysbios_heaps_HeapMem_E_memory;
__extern __FAR__ const CT__ti_sysbios_heaps_HeapMem_E_memory ti_sysbios_heaps_HeapMem_E_memory__C;

/* A_invalidFree */
#define ti_sysbios_heaps_HeapMem_A_invalidFree (ti_sysbios_heaps_HeapMem_A_invalidFree__C)
typedef xdc_runtime_Assert_Id CT__ti_sysbios_heaps_HeapMem_A_invalidFree;
__extern __FAR__ const CT__ti_sysbios_heaps_HeapMem_A_invalidFree ti_sysbios_heaps_HeapMem_A_invalidFree__C;

/* primaryHeapBaseAddr */
#define ti_sysbios_heaps_HeapMem_primaryHeapBaseAddr (ti_sysbios_heaps_HeapMem_primaryHeapBaseAddr__C)
typedef xdc_Char *CT__ti_sysbios_heaps_HeapMem_primaryHeapBaseAddr;
__extern __FAR__ const CT__ti_sysbios_heaps_HeapMem_primaryHeapBaseAddr ti_sysbios_heaps_HeapMem_primaryHeapBaseAddr__C;

/* primaryHeapEndAddr */
#define ti_sysbios_heaps_HeapMem_primaryHeapEndAddr (ti_sysbios_heaps_HeapMem_primaryHeapEndAddr__C)
typedef xdc_Char *CT__ti_sysbios_heaps_HeapMem_primaryHeapEndAddr;
__extern __FAR__ const CT__ti_sysbios_heaps_HeapMem_primaryHeapEndAddr ti_sysbios_heaps_HeapMem_primaryHeapEndAddr__C;

/* reqAlign */
#ifdef ti_sysbios_heaps_HeapMem_reqAlign__D
#define ti_sysbios_heaps_HeapMem_reqAlign (ti_sysbios_heaps_HeapMem_reqAlign__D)
#else
#define ti_sysbios_heaps_HeapMem_reqAlign (ti_sysbios_heaps_HeapMem_reqAlign__C)
typedef xdc_SizeT CT__ti_sysbios_heaps_HeapMem_reqAlign;
__extern __FAR__ const CT__ti_sysbios_heaps_HeapMem_reqAlign ti_sysbios_heaps_HeapMem_reqAlign__C;
#endif


/*
 * ======== PER-INSTANCE TYPES ========
 */

/* Params */
struct ti_sysbios_heaps_HeapMem_Params {
    size_t __size;
    const void *__self;
    void *__fxns;
    xdc_runtime_IInstance_Params *instance;
    xdc_SizeT minBlockAlign;
    xdc_Ptr buf;
    xdc_runtime_Memory_Size size;
    xdc_runtime_IInstance_Params __iprms;
};

/* Struct */
struct ti_sysbios_heaps_HeapMem_Struct {
    const ti_sysbios_heaps_HeapMem_Fxns__ *__fxns;
    xdc_runtime_Memory_Size __f0;
    __TA_ti_sysbios_heaps_HeapMem_Instance_State__buf __f1;
    ti_sysbios_heaps_HeapMem_Header __f2;
    xdc_SizeT __f3;
    xdc_runtime_Types_CordAddr __name;
};


/*
 * ======== VIRTUAL FUNCTIONS ========
 */

/* Fxns__ */
struct ti_sysbios_heaps_HeapMem_Fxns__ {
    xdc_runtime_Types_Base* __base;
    const xdc_runtime_Types_SysFxns2 *__sysp;
    xdc_Ptr (*alloc)(ti_sysbios_heaps_HeapMem_Handle, xdc_SizeT, xdc_SizeT, xdc_runtime_Error_Block*);
    xdc_Void (*free)(ti_sysbios_heaps_HeapMem_Handle, xdc_Ptr, xdc_SizeT);
    xdc_Bool (*isBlocking)(ti_sysbios_heaps_HeapMem_Handle);
    xdc_Void (*getStats)(ti_sysbios_heaps_HeapMem_Handle, xdc_runtime_Memory_Stats*);
    xdc_runtime_Types_SysFxns2 __sfxns;
};

/* Module__FXNS__C */
__extern const ti_sysbios_heaps_HeapMem_Fxns__ ti_sysbios_heaps_HeapMem_Module__FXNS__C;


/*
 * ======== FUNCTION DECLARATIONS ========
 */

/* Module_startup */
#define ti_sysbios_heaps_HeapMem_Module_startup( state ) (-1)

/* Instance_init__E */
xdc__CODESECT(ti_sysbios_heaps_HeapMem_Instance_init__E, "ti_sysbios_heaps_HeapMem_Instance_init")
__extern xdc_Void ti_sysbios_heaps_HeapMem_Instance_init__E(ti_sysbios_heaps_HeapMem_Object *, const ti_sysbios_heaps_HeapMem_Params *);

/* Handle__label__S */
xdc__CODESECT(ti_sysbios_heaps_HeapMem_Handle__label__S, "ti_sysbios_heaps_HeapMem_Handle__label__S")
__extern xdc_runtime_Types_Label *ti_sysbios_heaps_HeapMem_Handle__label__S( xdc_Ptr obj, xdc_runtime_Types_Label *lab );

/* Module__startupDone__S */
xdc__CODESECT(ti_sysbios_heaps_HeapMem_Module__startupDone__S, "ti_sysbios_heaps_HeapMem_Module__startupDone__S")
__extern xdc_Bool ti_sysbios_heaps_HeapMem_Module__startupDone__S( void );

/* Object__create__S */
xdc__CODESECT(ti_sysbios_heaps_HeapMem_Object__create__S, "ti_sysbios_heaps_HeapMem_Object__create__S")
__extern xdc_Ptr ti_sysbios_heaps_HeapMem_Object__create__S( xdc_Ptr __oa, xdc_SizeT __osz, xdc_Ptr __aa, const xdc_UChar *__pa, xdc_SizeT __psz, xdc_runtime_Error_Block *__eb );

/* create */
xdc__CODESECT(ti_sysbios_heaps_HeapMem_create, "ti_sysbios_heaps_HeapMem_create")
__extern ti_sysbios_heaps_HeapMem_Handle ti_sysbios_heaps_HeapMem_create( const ti_sysbios_heaps_HeapMem_Params *__prms, xdc_runtime_Error_Block *__eb );

/* construct */
xdc__CODESECT(ti_sysbios_heaps_HeapMem_construct, "ti_sysbios_heaps_HeapMem_construct")
__extern void ti_sysbios_heaps_HeapMem_construct( ti_sysbios_heaps_HeapMem_Struct *__obj, const ti_sysbios_heaps_HeapMem_Params *__prms );

/* Object__delete__S */
xdc__CODESECT(ti_sysbios_heaps_HeapMem_Object__delete__S, "ti_sysbios_heaps_HeapMem_Object__delete__S")
__extern xdc_Void ti_sysbios_heaps_HeapMem_Object__delete__S( xdc_Ptr instp );

/* delete */
xdc__CODESECT(ti_sysbios_heaps_HeapMem_delete, "ti_sysbios_heaps_HeapMem_delete")
__extern void ti_sysbios_heaps_HeapMem_delete(ti_sysbios_heaps_HeapMem_Handle *instp);

/* Object__destruct__S */
xdc__CODESECT(ti_sysbios_heaps_HeapMem_Object__destruct__S, "ti_sysbios_heaps_HeapMem_Object__destruct__S")
__extern xdc_Void ti_sysbios_heaps_HeapMem_Object__destruct__S( xdc_Ptr objp );

/* destruct */
xdc__CODESECT(ti_sysbios_heaps_HeapMem_destruct, "ti_sysbios_heaps_HeapMem_destruct")
__extern void ti_sysbios_heaps_HeapMem_destruct(ti_sysbios_heaps_HeapMem_Struct *obj);

/* Object__get__S */
xdc__CODESECT(ti_sysbios_heaps_HeapMem_Object__get__S, "ti_sysbios_heaps_HeapMem_Object__get__S")
__extern xdc_Ptr ti_sysbios_heaps_HeapMem_Object__get__S( xdc_Ptr oarr, xdc_Int i );

/* Object__first__S */
xdc__CODESECT(ti_sysbios_heaps_HeapMem_Object__first__S, "ti_sysbios_heaps_HeapMem_Object__first__S")
__extern xdc_Ptr ti_sysbios_heaps_HeapMem_Object__first__S( void );

/* Object__next__S */
xdc__CODESECT(ti_sysbios_heaps_HeapMem_Object__next__S, "ti_sysbios_heaps_HeapMem_Object__next__S")
__extern xdc_Ptr ti_sysbios_heaps_HeapMem_Object__next__S( xdc_Ptr obj );

/* Params__init__S */
xdc__CODESECT(ti_sysbios_heaps_HeapMem_Params__init__S, "ti_sysbios_heaps_HeapMem_Params__init__S")
__extern xdc_Void ti_sysbios_heaps_HeapMem_Params__init__S( xdc_Ptr dst, const xdc_Void *src, xdc_SizeT psz, xdc_SizeT isz );

/* enter__E */
#define ti_sysbios_heaps_HeapMem_enter ti_sysbios_heaps_HeapMem_enter__E
xdc__CODESECT(ti_sysbios_heaps_HeapMem_enter__E, "ti_sysbios_heaps_HeapMem_enter")
__extern xdc_IArg ti_sysbios_heaps_HeapMem_enter__E( void );

/* leave__E */
#define ti_sysbios_heaps_HeapMem_leave ti_sysbios_heaps_HeapMem_leave__E
xdc__CODESECT(ti_sysbios_heaps_HeapMem_leave__E, "ti_sysbios_heaps_HeapMem_leave")
__extern xdc_Void ti_sysbios_heaps_HeapMem_leave__E( xdc_IArg key );

/* alloc__E */
#define ti_sysbios_heaps_HeapMem_alloc ti_sysbios_heaps_HeapMem_alloc__E
xdc__CODESECT(ti_sysbios_heaps_HeapMem_alloc__E, "ti_sysbios_heaps_HeapMem_alloc")
__extern xdc_Ptr ti_sysbios_heaps_HeapMem_alloc__E( ti_sysbios_heaps_HeapMem_Handle __inst, xdc_SizeT size, xdc_SizeT align, xdc_runtime_Error_Block *eb );

/* free__E */
#define ti_sysbios_heaps_HeapMem_free ti_sysbios_heaps_HeapMem_free__E
xdc__CODESECT(ti_sysbios_heaps_HeapMem_free__E, "ti_sysbios_heaps_HeapMem_free")
__extern xdc_Void ti_sysbios_heaps_HeapMem_free__E( ti_sysbios_heaps_HeapMem_Handle __inst, xdc_Ptr block, xdc_SizeT size );

/* isBlocking__E */
#define ti_sysbios_heaps_HeapMem_isBlocking ti_sysbios_heaps_HeapMem_isBlocking__E
xdc__CODESECT(ti_sysbios_heaps_HeapMem_isBlocking__E, "ti_sysbios_heaps_HeapMem_isBlocking")
__extern xdc_Bool ti_sysbios_heaps_HeapMem_isBlocking__E( ti_sysbios_heaps_HeapMem_Handle __inst );

/* getStats__E */
#define ti_sysbios_heaps_HeapMem_getStats ti_sysbios_heaps_HeapMem_getStats__E
xdc__CODESECT(ti_sysbios_heaps_HeapMem_getStats__E, "ti_sysbios_heaps_HeapMem_getStats")
__extern xdc_Void ti_sysbios_heaps_HeapMem_getStats__E( ti_sysbios_heaps_HeapMem_Handle __inst, xdc_runtime_Memory_Stats *stats );

/* restore__E */
#define ti_sysbios_heaps_HeapMem_restore ti_sysbios_heaps_HeapMem_restore__E
xdc__CODESECT(ti_sysbios_heaps_HeapMem_restore__E, "ti_sysbios_heaps_HeapMem_restore")
__extern xdc_Void ti_sysbios_heaps_HeapMem_restore__E( ti_sysbios_heaps_HeapMem_Handle __inst );

/* getExtendedStats__E */
#define ti_sysbios_heaps_HeapMem_getExtendedStats ti_sysbios_heaps_HeapMem_getExtendedStats__E
xdc__CODESECT(ti_sysbios_heaps_HeapMem_getExtendedStats__E, "ti_sysbios_heaps_HeapMem_getExtendedStats")
__extern xdc_Void ti_sysbios_heaps_HeapMem_getExtendedStats__E( ti_sysbios_heaps_HeapMem_Handle __inst, ti_sysbios_heaps_HeapMem_ExtendedStats *stats );

/* init__I */
#define ti_sysbios_heaps_HeapMem_init ti_sysbios_heaps_HeapMem_init__I
xdc__CODESECT(ti_sysbios_heaps_HeapMem_init__I, "ti_sysbios_heaps_HeapMem_init")
__extern xdc_Void ti_sysbios_heaps_HeapMem_init__I( void );

/* initPrimary__I */
#define ti_sysbios_heaps_HeapMem_initPrimary ti_sysbios_heaps_HeapMem_initPrimary__I
xdc__CODESECT(ti_sysbios_heaps_HeapMem_initPrimary__I, "ti_sysbios_heaps_HeapMem_initPrimary")
__extern xdc_Void ti_sysbios_heaps_HeapMem_initPrimary__I( void );


/*
 * ======== CONVERTORS ========
 */

/* Module_upCast */
static inline xdc_runtime_IHeap_Module ti_sysbios_heaps_HeapMem_Module_upCast( void )
{
    return (xdc_runtime_IHeap_Module)&ti_sysbios_heaps_HeapMem_Module__FXNS__C;
}

/* Module_to_xdc_runtime_IHeap */
#define ti_sysbios_heaps_HeapMem_Module_to_xdc_runtime_IHeap ti_sysbios_heaps_HeapMem_Module_upCast

/* Handle_upCast */
static inline xdc_runtime_IHeap_Handle ti_sysbios_heaps_HeapMem_Handle_upCast( ti_sysbios_heaps_HeapMem_Handle i )
{
    return (xdc_runtime_IHeap_Handle)i;
}

/* Handle_to_xdc_runtime_IHeap */
#define ti_sysbios_heaps_HeapMem_Handle_to_xdc_runtime_IHeap ti_sysbios_heaps_HeapMem_Handle_upCast

/* Handle_downCast */
static inline ti_sysbios_heaps_HeapMem_Handle ti_sysbios_heaps_HeapMem_Handle_downCast( xdc_runtime_IHeap_Handle i )
{
    xdc_runtime_IHeap_Handle i2 = (xdc_runtime_IHeap_Handle)i;
    return (void*)i2->__fxns == (void*)&ti_sysbios_heaps_HeapMem_Module__FXNS__C ? (ti_sysbios_heaps_HeapMem_Handle)i : 0;
}

/* Handle_from_xdc_runtime_IHeap */
#define ti_sysbios_heaps_HeapMem_Handle_from_xdc_runtime_IHeap ti_sysbios_heaps_HeapMem_Handle_downCast


/*
 * ======== SYSTEM FUNCTIONS ========
 */

/* Module_startupDone */
#define ti_sysbios_heaps_HeapMem_Module_startupDone() ti_sysbios_heaps_HeapMem_Module__startupDone__S()

/* Object_heap */
#define ti_sysbios_heaps_HeapMem_Object_heap() ti_sysbios_heaps_HeapMem_Object__heap__C

/* Module_heap */
#define ti_sysbios_heaps_HeapMem_Module_heap() ti_sysbios_heaps_HeapMem_Object__heap__C

/* Module_id */
static inline CT__ti_sysbios_heaps_HeapMem_Module__id ti_sysbios_heaps_HeapMem_Module_id( void ) 
{
    return ti_sysbios_heaps_HeapMem_Module__id__C;
}

/* Module_hasMask */
static inline xdc_Bool ti_sysbios_heaps_HeapMem_Module_hasMask( void ) 
{
    return ti_sysbios_heaps_HeapMem_Module__diagsMask__C != NULL;
}

/* Module_getMask */
static inline xdc_Bits16 ti_sysbios_heaps_HeapMem_Module_getMask( void ) 
{
    return ti_sysbios_heaps_HeapMem_Module__diagsMask__C != NULL ? *ti_sysbios_heaps_HeapMem_Module__diagsMask__C : 0;
}

/* Module_setMask */
static inline xdc_Void ti_sysbios_heaps_HeapMem_Module_setMask( xdc_Bits16 mask ) 
{
    if (ti_sysbios_heaps_HeapMem_Module__diagsMask__C != NULL) *ti_sysbios_heaps_HeapMem_Module__diagsMask__C = mask;
}

/* Params_init */
static inline void ti_sysbios_heaps_HeapMem_Params_init( ti_sysbios_heaps_HeapMem_Params *prms ) 
{
    if (prms) {
        ti_sysbios_heaps_HeapMem_Params__init__S(prms, 0, sizeof(ti_sysbios_heaps_HeapMem_Params), sizeof(xdc_runtime_IInstance_Params));
    }
}

/* Params_copy */
static inline void ti_sysbios_heaps_HeapMem_Params_copy(ti_sysbios_heaps_HeapMem_Params *dst, const ti_sysbios_heaps_HeapMem_Params *src) 
{
    if (dst) {
        ti_sysbios_heaps_HeapMem_Params__init__S(dst, (const void *)src, sizeof(ti_sysbios_heaps_HeapMem_Params), sizeof(xdc_runtime_IInstance_Params));
    }
}

/* Object_count */
#define ti_sysbios_heaps_HeapMem_Object_count() ti_sysbios_heaps_HeapMem_Object__count__C

/* Object_sizeof */
#define ti_sysbios_heaps_HeapMem_Object_sizeof() ti_sysbios_heaps_HeapMem_Object__sizeof__C

/* Object_get */
static inline ti_sysbios_heaps_HeapMem_Handle ti_sysbios_heaps_HeapMem_Object_get(ti_sysbios_heaps_HeapMem_Instance_State *oarr, int i) 
{
    return (ti_sysbios_heaps_HeapMem_Handle)ti_sysbios_heaps_HeapMem_Object__get__S(oarr, i);
}

/* Object_first */
static inline ti_sysbios_heaps_HeapMem_Handle ti_sysbios_heaps_HeapMem_Object_first( void )
{
    return (ti_sysbios_heaps_HeapMem_Handle)ti_sysbios_heaps_HeapMem_Object__first__S();
}

/* Object_next */
static inline ti_sysbios_heaps_HeapMem_Handle ti_sysbios_heaps_HeapMem_Object_next( ti_sysbios_heaps_HeapMem_Object *obj )
{
    return (ti_sysbios_heaps_HeapMem_Handle)ti_sysbios_heaps_HeapMem_Object__next__S(obj);
}

/* Handle_label */
static inline xdc_runtime_Types_Label *ti_sysbios_heaps_HeapMem_Handle_label( ti_sysbios_heaps_HeapMem_Handle inst, xdc_runtime_Types_Label *lab )
{
    return ti_sysbios_heaps_HeapMem_Handle__label__S(inst, lab);
}

/* Handle_name */
static inline xdc_String ti_sysbios_heaps_HeapMem_Handle_name( ti_sysbios_heaps_HeapMem_Handle inst )
{
    xdc_runtime_Types_Label lab;
    return ti_sysbios_heaps_HeapMem_Handle__label__S(inst, &lab)->iname;
}

/* handle */
static inline ti_sysbios_heaps_HeapMem_Handle ti_sysbios_heaps_HeapMem_handle( ti_sysbios_heaps_HeapMem_Struct *str )
{
    return (ti_sysbios_heaps_HeapMem_Handle)str;
}

/* struct */
static inline ti_sysbios_heaps_HeapMem_Struct *ti_sysbios_heaps_HeapMem_struct( ti_sysbios_heaps_HeapMem_Handle inst )
{
    return (ti_sysbios_heaps_HeapMem_Struct*)inst;
}


/*
 * ======== EPILOGUE ========
 */

#ifdef ti_sysbios_heaps_HeapMem__top__
#undef __nested__
#endif

#endif /* ti_sysbios_heaps_HeapMem__include */


/*
 * ======== STATE STRUCTURES ========
 */

#if defined(__config__) || (!defined(__nested__) && defined(ti_sysbios_heaps_HeapMem__internalaccess))

#ifndef ti_sysbios_heaps_HeapMem__include_state
#define ti_sysbios_heaps_HeapMem__include_state

/* Object */
struct ti_sysbios_heaps_HeapMem_Object {
    const ti_sysbios_heaps_HeapMem_Fxns__ *__fxns;
    xdc_runtime_Memory_Size align;
    __TA_ti_sysbios_heaps_HeapMem_Instance_State__buf buf;
    ti_sysbios_heaps_HeapMem_Header head;
    xdc_SizeT minBlockAlign;
};

#endif /* ti_sysbios_heaps_HeapMem__include_state */

#endif


/*
 * ======== PREFIX ALIASES ========
 */

#if !defined(__nested__) && !defined(ti_sysbios_heaps_HeapMem__nolocalnames)

#ifndef ti_sysbios_heaps_HeapMem__localnames__done
#define ti_sysbios_heaps_HeapMem__localnames__done

/* module prefix */
#define HeapMem_Instance ti_sysbios_heaps_HeapMem_Instance
#define HeapMem_Handle ti_sysbios_heaps_HeapMem_Handle
#define HeapMem_Module ti_sysbios_heaps_HeapMem_Module
#define HeapMem_Object ti_sysbios_heaps_HeapMem_Object
#define HeapMem_Struct ti_sysbios_heaps_HeapMem_Struct
#define HeapMem_ExtendedStats ti_sysbios_heaps_HeapMem_ExtendedStats
#define HeapMem_Header ti_sysbios_heaps_HeapMem_Header
#define HeapMem_Instance_State ti_sysbios_heaps_HeapMem_Instance_State
#define HeapMem_A_zeroBlock ti_sysbios_heaps_HeapMem_A_zeroBlock
#define HeapMem_A_heapSize ti_sysbios_heaps_HeapMem_A_heapSize
#define HeapMem_A_align ti_sysbios_heaps_HeapMem_A_align
#define HeapMem_E_memory ti_sysbios_heaps_HeapMem_E_memory
#define HeapMem_A_invalidFree ti_sysbios_heaps_HeapMem_A_invalidFree
#define HeapMem_primaryHeapBaseAddr ti_sysbios_heaps_HeapMem_primaryHeapBaseAddr
#define HeapMem_primaryHeapEndAddr ti_sysbios_heaps_HeapMem_primaryHeapEndAddr
#define HeapMem_reqAlign ti_sysbios_heaps_HeapMem_reqAlign
#define HeapMem_Params ti_sysbios_heaps_HeapMem_Params
#define HeapMem_enter ti_sysbios_heaps_HeapMem_enter
#define HeapMem_leave ti_sysbios_heaps_HeapMem_leave
#define HeapMem_alloc ti_sysbios_heaps_HeapMem_alloc
#define HeapMem_free ti_sysbios_heaps_HeapMem_free
#define HeapMem_isBlocking ti_sysbios_heaps_HeapMem_isBlocking
#define HeapMem_getStats ti_sysbios_heaps_HeapMem_getStats
#define HeapMem_restore ti_sysbios_heaps_HeapMem_restore
#define HeapMem_getExtendedStats ti_sysbios_heaps_HeapMem_getExtendedStats
#define HeapMem_Module_name ti_sysbios_heaps_HeapMem_Module_name
#define HeapMem_Module_id ti_sysbios_heaps_HeapMem_Module_id
#define HeapMem_Module_startup ti_sysbios_heaps_HeapMem_Module_startup
#define HeapMem_Module_startupDone ti_sysbios_heaps_HeapMem_Module_startupDone
#define HeapMem_Module_hasMask ti_sysbios_heaps_HeapMem_Module_hasMask
#define HeapMem_Module_getMask ti_sysbios_heaps_HeapMem_Module_getMask
#define HeapMem_Module_setMask ti_sysbios_heaps_HeapMem_Module_setMask
#define HeapMem_Object_heap ti_sysbios_heaps_HeapMem_Object_heap
#define HeapMem_Module_heap ti_sysbios_heaps_HeapMem_Module_heap
#define HeapMem_construct ti_sysbios_heaps_HeapMem_construct
#define HeapMem_create ti_sysbios_heaps_HeapMem_create
#define HeapMem_handle ti_sysbios_heaps_HeapMem_handle
#define HeapMem_struct ti_sysbios_heaps_HeapMem_struct
#define HeapMem_Handle_label ti_sysbios_heaps_HeapMem_Handle_label
#define HeapMem_Handle_name ti_sysbios_heaps_HeapMem_Handle_name
#define HeapMem_Instance_init ti_sysbios_heaps_HeapMem_Instance_init
#define HeapMem_Object_count ti_sysbios_heaps_HeapMem_Object_count
#define HeapMem_Object_get ti_sysbios_heaps_HeapMem_Object_get
#define HeapMem_Object_first ti_sysbios_heaps_HeapMem_Object_first
#define HeapMem_Object_next ti_sysbios_heaps_HeapMem_Object_next
#define HeapMem_Object_sizeof ti_sysbios_heaps_HeapMem_Object_sizeof
#define HeapMem_Params_copy ti_sysbios_heaps_HeapMem_Params_copy
#define HeapMem_Params_init ti_sysbios_heaps_HeapMem_Params_init
#define HeapMem_delete ti_sysbios_heaps_HeapMem_delete
#define HeapMem_destruct ti_sysbios_heaps_HeapMem_destruct
#define HeapMem_Module_upCast ti_sysbios_heaps_HeapMem_Module_upCast
#define HeapMem_Module_to_xdc_runtime_IHeap ti_sysbios_heaps_HeapMem_Module_to_xdc_runtime_IHeap
#define HeapMem_Handle_upCast ti_sysbios_heaps_HeapMem_Handle_upCast
#define HeapMem_Handle_to_xdc_runtime_IHeap ti_sysbios_heaps_HeapMem_Handle_to_xdc_runtime_IHeap
#define HeapMem_Handle_downCast ti_sysbios_heaps_HeapMem_Handle_downCast
#define HeapMem_Handle_from_xdc_runtime_IHeap ti_sysbios_heaps_HeapMem_Handle_from_xdc_runtime_IHeap

/* proxies */
#include <ti/sysbios/heaps/package/HeapMem_Module_GateProxy.h>

#endif /* ti_sysbios_heaps_HeapMem__localnames__done */
#endif
