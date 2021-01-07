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
 *     CREATE ARGS
 *     MODULE-WIDE CONFIGS
 *     PER-INSTANCE TYPES
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

#ifndef xdc_runtime_knl_SemProcess_Proxy__include
#define xdc_runtime_knl_SemProcess_Proxy__include

#ifndef __nested__
#define __nested__
#define xdc_runtime_knl_SemProcess_Proxy__top__
#endif

#ifdef __cplusplus
#define __extern extern "C"
#else
#define __extern extern
#endif

#define xdc_runtime_knl_SemProcess_Proxy___VERS 160


/*
 * ======== INCLUDES ========
 */

#include <xdc/std.h>

#include <xdc/runtime/xdc.h>
#include <xdc/runtime/Types.h>
#include <xdc/runtime/IInstance.h>
#include <xdc/runtime/knl/package/package.defs.h>

#include <xdc/runtime/knl/ISemProcessSupport.h>


/*
 * ======== AUXILIARY DEFINITIONS ========
 */

/* PendStatus */
typedef xdc_runtime_knl_ISemaphore_PendStatus xdc_runtime_knl_SemProcess_Proxy_PendStatus;

/* FOREVER */
#define xdc_runtime_knl_SemProcess_Proxy_FOREVER (~(0))

/* Mode */
typedef xdc_runtime_knl_ISemaphore_Mode xdc_runtime_knl_SemProcess_Proxy_Mode;


/*
 * ======== CREATE ARGS ========
 */

/* Args__create */
typedef struct xdc_runtime_knl_SemProcess_Proxy_Args__create {
    xdc_Int count;
    xdc_Int key;
} xdc_runtime_knl_SemProcess_Proxy_Args__create;


/*
 * ======== MODULE-WIDE CONFIGS ========
 */

/* Module__diagsEnabled */
typedef xdc_Bits32 CT__xdc_runtime_knl_SemProcess_Proxy_Module__diagsEnabled;
__extern __FAR__ const CT__xdc_runtime_knl_SemProcess_Proxy_Module__diagsEnabled xdc_runtime_knl_SemProcess_Proxy_Module__diagsEnabled__C;

/* Module__diagsIncluded */
typedef xdc_Bits32 CT__xdc_runtime_knl_SemProcess_Proxy_Module__diagsIncluded;
__extern __FAR__ const CT__xdc_runtime_knl_SemProcess_Proxy_Module__diagsIncluded xdc_runtime_knl_SemProcess_Proxy_Module__diagsIncluded__C;

/* Module__diagsMask */
typedef xdc_Bits16 *CT__xdc_runtime_knl_SemProcess_Proxy_Module__diagsMask;
__extern __FAR__ const CT__xdc_runtime_knl_SemProcess_Proxy_Module__diagsMask xdc_runtime_knl_SemProcess_Proxy_Module__diagsMask__C;

/* Module__gateObj */
typedef xdc_Ptr CT__xdc_runtime_knl_SemProcess_Proxy_Module__gateObj;
__extern __FAR__ const CT__xdc_runtime_knl_SemProcess_Proxy_Module__gateObj xdc_runtime_knl_SemProcess_Proxy_Module__gateObj__C;

/* Module__gatePrms */
typedef xdc_Ptr CT__xdc_runtime_knl_SemProcess_Proxy_Module__gatePrms;
__extern __FAR__ const CT__xdc_runtime_knl_SemProcess_Proxy_Module__gatePrms xdc_runtime_knl_SemProcess_Proxy_Module__gatePrms__C;

/* Module__id */
typedef xdc_runtime_Types_ModuleId CT__xdc_runtime_knl_SemProcess_Proxy_Module__id;
__extern __FAR__ const CT__xdc_runtime_knl_SemProcess_Proxy_Module__id xdc_runtime_knl_SemProcess_Proxy_Module__id__C;

/* Module__loggerDefined */
typedef xdc_Bool CT__xdc_runtime_knl_SemProcess_Proxy_Module__loggerDefined;
__extern __FAR__ const CT__xdc_runtime_knl_SemProcess_Proxy_Module__loggerDefined xdc_runtime_knl_SemProcess_Proxy_Module__loggerDefined__C;

/* Module__loggerObj */
typedef xdc_Ptr CT__xdc_runtime_knl_SemProcess_Proxy_Module__loggerObj;
__extern __FAR__ const CT__xdc_runtime_knl_SemProcess_Proxy_Module__loggerObj xdc_runtime_knl_SemProcess_Proxy_Module__loggerObj__C;

/* Module__loggerFxn0 */
typedef xdc_runtime_Types_LoggerFxn0 CT__xdc_runtime_knl_SemProcess_Proxy_Module__loggerFxn0;
__extern __FAR__ const CT__xdc_runtime_knl_SemProcess_Proxy_Module__loggerFxn0 xdc_runtime_knl_SemProcess_Proxy_Module__loggerFxn0__C;

/* Module__loggerFxn1 */
typedef xdc_runtime_Types_LoggerFxn1 CT__xdc_runtime_knl_SemProcess_Proxy_Module__loggerFxn1;
__extern __FAR__ const CT__xdc_runtime_knl_SemProcess_Proxy_Module__loggerFxn1 xdc_runtime_knl_SemProcess_Proxy_Module__loggerFxn1__C;

/* Module__loggerFxn2 */
typedef xdc_runtime_Types_LoggerFxn2 CT__xdc_runtime_knl_SemProcess_Proxy_Module__loggerFxn2;
__extern __FAR__ const CT__xdc_runtime_knl_SemProcess_Proxy_Module__loggerFxn2 xdc_runtime_knl_SemProcess_Proxy_Module__loggerFxn2__C;

/* Module__loggerFxn4 */
typedef xdc_runtime_Types_LoggerFxn4 CT__xdc_runtime_knl_SemProcess_Proxy_Module__loggerFxn4;
__extern __FAR__ const CT__xdc_runtime_knl_SemProcess_Proxy_Module__loggerFxn4 xdc_runtime_knl_SemProcess_Proxy_Module__loggerFxn4__C;

/* Module__loggerFxn8 */
typedef xdc_runtime_Types_LoggerFxn8 CT__xdc_runtime_knl_SemProcess_Proxy_Module__loggerFxn8;
__extern __FAR__ const CT__xdc_runtime_knl_SemProcess_Proxy_Module__loggerFxn8 xdc_runtime_knl_SemProcess_Proxy_Module__loggerFxn8__C;

/* Module__startupDoneFxn */
typedef xdc_Bool (*CT__xdc_runtime_knl_SemProcess_Proxy_Module__startupDoneFxn)(void);
__extern __FAR__ const CT__xdc_runtime_knl_SemProcess_Proxy_Module__startupDoneFxn xdc_runtime_knl_SemProcess_Proxy_Module__startupDoneFxn__C;

/* Object__count */
typedef xdc_Int CT__xdc_runtime_knl_SemProcess_Proxy_Object__count;
__extern __FAR__ const CT__xdc_runtime_knl_SemProcess_Proxy_Object__count xdc_runtime_knl_SemProcess_Proxy_Object__count__C;

/* Object__heap */
typedef xdc_runtime_IHeap_Handle CT__xdc_runtime_knl_SemProcess_Proxy_Object__heap;
__extern __FAR__ const CT__xdc_runtime_knl_SemProcess_Proxy_Object__heap xdc_runtime_knl_SemProcess_Proxy_Object__heap__C;

/* Object__sizeof */
typedef xdc_SizeT CT__xdc_runtime_knl_SemProcess_Proxy_Object__sizeof;
__extern __FAR__ const CT__xdc_runtime_knl_SemProcess_Proxy_Object__sizeof xdc_runtime_knl_SemProcess_Proxy_Object__sizeof__C;

/* Object__table */
typedef xdc_Ptr CT__xdc_runtime_knl_SemProcess_Proxy_Object__table;
__extern __FAR__ const CT__xdc_runtime_knl_SemProcess_Proxy_Object__table xdc_runtime_knl_SemProcess_Proxy_Object__table__C;


/*
 * ======== PER-INSTANCE TYPES ========
 */

/* Params */
struct xdc_runtime_knl_SemProcess_Proxy_Params {
    size_t __size;
    const void *__self;
    void *__fxns;
    xdc_runtime_IInstance_Params *instance;
    xdc_runtime_knl_ISemaphore_Mode mode;
    xdc_runtime_IInstance_Params __iprms;
};

/* Struct */
struct xdc_runtime_knl_SemProcess_Proxy_Struct {
    const xdc_runtime_knl_SemProcess_Proxy_Fxns__ *__fxns;
    xdc_runtime_Types_CordAddr __name;
};


/*
 * ======== VIRTUAL FUNCTIONS ========
 */

/* Fxns__ */
struct xdc_runtime_knl_SemProcess_Proxy_Fxns__ {
    xdc_runtime_Types_Base* __base;
    const xdc_runtime_Types_SysFxns2 *__sysp;
    xdc_Int (*pend)(xdc_runtime_knl_SemProcess_Proxy_Handle, xdc_UInt, xdc_runtime_Error_Block*);
    xdc_Bool (*post)(xdc_runtime_knl_SemProcess_Proxy_Handle, xdc_runtime_Error_Block*);
    xdc_runtime_Types_SysFxns2 __sfxns;
};

/* Module__FXNS__C */
__extern const xdc_runtime_knl_SemProcess_Proxy_Fxns__ xdc_runtime_knl_SemProcess_Proxy_Module__FXNS__C;


/*
 * ======== FUNCTION DECLARATIONS ========
 */

/* Module_startup */
#define xdc_runtime_knl_SemProcess_Proxy_Module_startup( state ) (-1)

/* Handle__label__S */
xdc__CODESECT(xdc_runtime_knl_SemProcess_Proxy_Handle__label__S, "xdc_runtime_knl_SemProcess_Proxy_Handle__label__S")
__extern xdc_runtime_Types_Label *xdc_runtime_knl_SemProcess_Proxy_Handle__label__S( xdc_Ptr obj, xdc_runtime_Types_Label *lab );

/* Module__startupDone__S */
xdc__CODESECT(xdc_runtime_knl_SemProcess_Proxy_Module__startupDone__S, "xdc_runtime_knl_SemProcess_Proxy_Module__startupDone__S")
__extern xdc_Bool xdc_runtime_knl_SemProcess_Proxy_Module__startupDone__S( void );

/* Object__create__S */
xdc__CODESECT(xdc_runtime_knl_SemProcess_Proxy_Object__create__S, "xdc_runtime_knl_SemProcess_Proxy_Object__create__S")
__extern xdc_Ptr xdc_runtime_knl_SemProcess_Proxy_Object__create__S( xdc_Ptr __oa, xdc_SizeT __osz, xdc_Ptr __aa, const xdc_UChar *__pa, xdc_SizeT __psz, xdc_runtime_Error_Block *__eb );

/* create */
xdc__CODESECT(xdc_runtime_knl_SemProcess_Proxy_create, "xdc_runtime_knl_SemProcess_Proxy_create")
__extern xdc_runtime_knl_SemProcess_Proxy_Handle xdc_runtime_knl_SemProcess_Proxy_create( xdc_Int count, xdc_Int key, const xdc_runtime_knl_SemProcess_Proxy_Params *__prms, xdc_runtime_Error_Block *__eb );

/* Object__delete__S */
xdc__CODESECT(xdc_runtime_knl_SemProcess_Proxy_Object__delete__S, "xdc_runtime_knl_SemProcess_Proxy_Object__delete__S")
__extern xdc_Void xdc_runtime_knl_SemProcess_Proxy_Object__delete__S( xdc_Ptr instp );

/* delete */
xdc__CODESECT(xdc_runtime_knl_SemProcess_Proxy_delete, "xdc_runtime_knl_SemProcess_Proxy_delete")
__extern void xdc_runtime_knl_SemProcess_Proxy_delete(xdc_runtime_knl_SemProcess_Proxy_Handle *instp);

/* Object__destruct__S */
xdc__CODESECT(xdc_runtime_knl_SemProcess_Proxy_Object__destruct__S, "xdc_runtime_knl_SemProcess_Proxy_Object__destruct__S")
__extern xdc_Void xdc_runtime_knl_SemProcess_Proxy_Object__destruct__S( xdc_Ptr objp );

/* Object__get__S */
xdc__CODESECT(xdc_runtime_knl_SemProcess_Proxy_Object__get__S, "xdc_runtime_knl_SemProcess_Proxy_Object__get__S")
__extern xdc_Ptr xdc_runtime_knl_SemProcess_Proxy_Object__get__S( xdc_Ptr oarr, xdc_Int i );

/* Object__first__S */
xdc__CODESECT(xdc_runtime_knl_SemProcess_Proxy_Object__first__S, "xdc_runtime_knl_SemProcess_Proxy_Object__first__S")
__extern xdc_Ptr xdc_runtime_knl_SemProcess_Proxy_Object__first__S( void );

/* Object__next__S */
xdc__CODESECT(xdc_runtime_knl_SemProcess_Proxy_Object__next__S, "xdc_runtime_knl_SemProcess_Proxy_Object__next__S")
__extern xdc_Ptr xdc_runtime_knl_SemProcess_Proxy_Object__next__S( xdc_Ptr obj );

/* Params__init__S */
xdc__CODESECT(xdc_runtime_knl_SemProcess_Proxy_Params__init__S, "xdc_runtime_knl_SemProcess_Proxy_Params__init__S")
__extern xdc_Void xdc_runtime_knl_SemProcess_Proxy_Params__init__S( xdc_Ptr dst, const xdc_Void *src, xdc_SizeT psz, xdc_SizeT isz );

/* Proxy__abstract__S */
xdc__CODESECT(xdc_runtime_knl_SemProcess_Proxy_Proxy__abstract__S, "xdc_runtime_knl_SemProcess_Proxy_Proxy__abstract__S")
__extern xdc_Bool xdc_runtime_knl_SemProcess_Proxy_Proxy__abstract__S( void );

/* Proxy__delegate__S */
xdc__CODESECT(xdc_runtime_knl_SemProcess_Proxy_Proxy__delegate__S, "xdc_runtime_knl_SemProcess_Proxy_Proxy__delegate__S")
__extern xdc_Ptr xdc_runtime_knl_SemProcess_Proxy_Proxy__delegate__S( void );

/* pend__E */
#define xdc_runtime_knl_SemProcess_Proxy_pend xdc_runtime_knl_SemProcess_Proxy_pend__E
xdc__CODESECT(xdc_runtime_knl_SemProcess_Proxy_pend__E, "xdc_runtime_knl_SemProcess_Proxy_pend")
__extern xdc_Int xdc_runtime_knl_SemProcess_Proxy_pend__E( xdc_runtime_knl_SemProcess_Proxy_Handle __inst, xdc_UInt timeout, xdc_runtime_Error_Block *eb );

/* post__E */
#define xdc_runtime_knl_SemProcess_Proxy_post xdc_runtime_knl_SemProcess_Proxy_post__E
xdc__CODESECT(xdc_runtime_knl_SemProcess_Proxy_post__E, "xdc_runtime_knl_SemProcess_Proxy_post")
__extern xdc_Bool xdc_runtime_knl_SemProcess_Proxy_post__E( xdc_runtime_knl_SemProcess_Proxy_Handle __inst, xdc_runtime_Error_Block *eb );


/*
 * ======== CONVERTORS ========
 */

/* Module_upCast */
static inline xdc_runtime_knl_ISemProcessSupport_Module xdc_runtime_knl_SemProcess_Proxy_Module_upCast( void )
{
    return (xdc_runtime_knl_ISemProcessSupport_Module)xdc_runtime_knl_SemProcess_Proxy_Proxy__delegate__S();
}

/* Module_to_xdc_runtime_knl_ISemProcessSupport */
#define xdc_runtime_knl_SemProcess_Proxy_Module_to_xdc_runtime_knl_ISemProcessSupport xdc_runtime_knl_SemProcess_Proxy_Module_upCast

/* Handle_upCast */
static inline xdc_runtime_knl_ISemProcessSupport_Handle xdc_runtime_knl_SemProcess_Proxy_Handle_upCast( xdc_runtime_knl_SemProcess_Proxy_Handle i )
{
    return (xdc_runtime_knl_ISemProcessSupport_Handle)i;
}

/* Handle_to_xdc_runtime_knl_ISemProcessSupport */
#define xdc_runtime_knl_SemProcess_Proxy_Handle_to_xdc_runtime_knl_ISemProcessSupport xdc_runtime_knl_SemProcess_Proxy_Handle_upCast

/* Handle_downCast */
static inline xdc_runtime_knl_SemProcess_Proxy_Handle xdc_runtime_knl_SemProcess_Proxy_Handle_downCast( xdc_runtime_knl_ISemProcessSupport_Handle i )
{
    xdc_runtime_knl_ISemProcessSupport_Handle i2 = (xdc_runtime_knl_ISemProcessSupport_Handle)i;
if (xdc_runtime_knl_SemProcess_Proxy_Proxy__abstract__S()) return (xdc_runtime_knl_SemProcess_Proxy_Handle)i;
    return (void*)i2->__fxns == (void*)xdc_runtime_knl_SemProcess_Proxy_Proxy__delegate__S() ? (xdc_runtime_knl_SemProcess_Proxy_Handle)i : 0;
}

/* Handle_from_xdc_runtime_knl_ISemProcessSupport */
#define xdc_runtime_knl_SemProcess_Proxy_Handle_from_xdc_runtime_knl_ISemProcessSupport xdc_runtime_knl_SemProcess_Proxy_Handle_downCast

/* Module_upCast2 */
static inline xdc_runtime_knl_ISemaphore_Module xdc_runtime_knl_SemProcess_Proxy_Module_upCast2( void )
{
    return (xdc_runtime_knl_ISemaphore_Module)xdc_runtime_knl_SemProcess_Proxy_Proxy__delegate__S();
}

/* Module_to_xdc_runtime_knl_ISemaphore */
#define xdc_runtime_knl_SemProcess_Proxy_Module_to_xdc_runtime_knl_ISemaphore xdc_runtime_knl_SemProcess_Proxy_Module_upCast2

/* Handle_upCast2 */
static inline xdc_runtime_knl_ISemaphore_Handle xdc_runtime_knl_SemProcess_Proxy_Handle_upCast2( xdc_runtime_knl_SemProcess_Proxy_Handle i )
{
    return (xdc_runtime_knl_ISemaphore_Handle)i;
}

/* Handle_to_xdc_runtime_knl_ISemaphore */
#define xdc_runtime_knl_SemProcess_Proxy_Handle_to_xdc_runtime_knl_ISemaphore xdc_runtime_knl_SemProcess_Proxy_Handle_upCast2

/* Handle_downCast2 */
static inline xdc_runtime_knl_SemProcess_Proxy_Handle xdc_runtime_knl_SemProcess_Proxy_Handle_downCast2( xdc_runtime_knl_ISemaphore_Handle i )
{
    xdc_runtime_knl_ISemaphore_Handle i2 = (xdc_runtime_knl_ISemaphore_Handle)i;
if (xdc_runtime_knl_SemProcess_Proxy_Proxy__abstract__S()) return (xdc_runtime_knl_SemProcess_Proxy_Handle)i;
    return (void*)i2->__fxns == (void*)xdc_runtime_knl_SemProcess_Proxy_Proxy__delegate__S() ? (xdc_runtime_knl_SemProcess_Proxy_Handle)i : 0;
}

/* Handle_from_xdc_runtime_knl_ISemaphore */
#define xdc_runtime_knl_SemProcess_Proxy_Handle_from_xdc_runtime_knl_ISemaphore xdc_runtime_knl_SemProcess_Proxy_Handle_downCast2


/*
 * ======== SYSTEM FUNCTIONS ========
 */

/* Module_startupDone */
#define xdc_runtime_knl_SemProcess_Proxy_Module_startupDone() xdc_runtime_knl_SemProcess_Proxy_Module__startupDone__S()

/* Object_heap */
#define xdc_runtime_knl_SemProcess_Proxy_Object_heap() xdc_runtime_knl_SemProcess_Proxy_Object__heap__C

/* Module_heap */
#define xdc_runtime_knl_SemProcess_Proxy_Module_heap() xdc_runtime_knl_SemProcess_Proxy_Object__heap__C

/* Module_id */
static inline CT__xdc_runtime_knl_SemProcess_Proxy_Module__id xdc_runtime_knl_SemProcess_Proxy_Module_id( void ) 
{
    return xdc_runtime_knl_SemProcess_Proxy_Module__id__C;
}

/* Proxy_abstract */
#define xdc_runtime_knl_SemProcess_Proxy_Proxy_abstract() xdc_runtime_knl_SemProcess_Proxy_Proxy__abstract__S()

/* Proxy_delegate */
#define xdc_runtime_knl_SemProcess_Proxy_Proxy_delegate() ((xdc_runtime_knl_ISemProcessSupport_Module)xdc_runtime_knl_SemProcess_Proxy_Proxy__delegate__S())

/* Params_init */
static inline void xdc_runtime_knl_SemProcess_Proxy_Params_init( xdc_runtime_knl_SemProcess_Proxy_Params *prms ) 
{
    if (prms) {
        xdc_runtime_knl_SemProcess_Proxy_Params__init__S(prms, 0, sizeof(xdc_runtime_knl_SemProcess_Proxy_Params), sizeof(xdc_runtime_IInstance_Params));
    }
}

/* Params_copy */
static inline void xdc_runtime_knl_SemProcess_Proxy_Params_copy(xdc_runtime_knl_SemProcess_Proxy_Params *dst, const xdc_runtime_knl_SemProcess_Proxy_Params *src) 
{
    if (dst) {
        xdc_runtime_knl_SemProcess_Proxy_Params__init__S(dst, (const void *)src, sizeof(xdc_runtime_knl_SemProcess_Proxy_Params), sizeof(xdc_runtime_IInstance_Params));
    }
}


/*
 * ======== EPILOGUE ========
 */

#ifdef xdc_runtime_knl_SemProcess_Proxy__top__
#undef __nested__
#endif

#endif /* xdc_runtime_knl_SemProcess_Proxy__include */


/*
 * ======== PREFIX ALIASES ========
 */

#if !defined(__nested__) && !defined(xdc_runtime_knl_SemProcess_Proxy__nolocalnames)

#ifndef xdc_runtime_knl_SemProcess_Proxy__localnames__done
#define xdc_runtime_knl_SemProcess_Proxy__localnames__done

/* module prefix */
#define SemProcess_Proxy_Instance xdc_runtime_knl_SemProcess_Proxy_Instance
#define SemProcess_Proxy_Handle xdc_runtime_knl_SemProcess_Proxy_Handle
#define SemProcess_Proxy_Module xdc_runtime_knl_SemProcess_Proxy_Module
#define SemProcess_Proxy_Object xdc_runtime_knl_SemProcess_Proxy_Object
#define SemProcess_Proxy_Struct xdc_runtime_knl_SemProcess_Proxy_Struct
#define SemProcess_Proxy_PendStatus xdc_runtime_knl_SemProcess_Proxy_PendStatus
#define SemProcess_Proxy_FOREVER xdc_runtime_knl_SemProcess_Proxy_FOREVER
#define SemProcess_Proxy_Mode xdc_runtime_knl_SemProcess_Proxy_Mode
#define SemProcess_Proxy_Params xdc_runtime_knl_SemProcess_Proxy_Params
#define SemProcess_Proxy_pend xdc_runtime_knl_SemProcess_Proxy_pend
#define SemProcess_Proxy_post xdc_runtime_knl_SemProcess_Proxy_post
#define SemProcess_Proxy_Module_name xdc_runtime_knl_SemProcess_Proxy_Module_name
#define SemProcess_Proxy_Module_id xdc_runtime_knl_SemProcess_Proxy_Module_id
#define SemProcess_Proxy_Module_startup xdc_runtime_knl_SemProcess_Proxy_Module_startup
#define SemProcess_Proxy_Module_startupDone xdc_runtime_knl_SemProcess_Proxy_Module_startupDone
#define SemProcess_Proxy_Module_hasMask xdc_runtime_knl_SemProcess_Proxy_Module_hasMask
#define SemProcess_Proxy_Module_getMask xdc_runtime_knl_SemProcess_Proxy_Module_getMask
#define SemProcess_Proxy_Module_setMask xdc_runtime_knl_SemProcess_Proxy_Module_setMask
#define SemProcess_Proxy_Object_heap xdc_runtime_knl_SemProcess_Proxy_Object_heap
#define SemProcess_Proxy_Module_heap xdc_runtime_knl_SemProcess_Proxy_Module_heap
#define SemProcess_Proxy_construct xdc_runtime_knl_SemProcess_Proxy_construct
#define SemProcess_Proxy_create xdc_runtime_knl_SemProcess_Proxy_create
#define SemProcess_Proxy_handle xdc_runtime_knl_SemProcess_Proxy_handle
#define SemProcess_Proxy_struct xdc_runtime_knl_SemProcess_Proxy_struct
#define SemProcess_Proxy_Handle_label xdc_runtime_knl_SemProcess_Proxy_Handle_label
#define SemProcess_Proxy_Handle_name xdc_runtime_knl_SemProcess_Proxy_Handle_name
#define SemProcess_Proxy_Instance_init xdc_runtime_knl_SemProcess_Proxy_Instance_init
#define SemProcess_Proxy_Object_count xdc_runtime_knl_SemProcess_Proxy_Object_count
#define SemProcess_Proxy_Object_get xdc_runtime_knl_SemProcess_Proxy_Object_get
#define SemProcess_Proxy_Object_first xdc_runtime_knl_SemProcess_Proxy_Object_first
#define SemProcess_Proxy_Object_next xdc_runtime_knl_SemProcess_Proxy_Object_next
#define SemProcess_Proxy_Object_sizeof xdc_runtime_knl_SemProcess_Proxy_Object_sizeof
#define SemProcess_Proxy_Params_copy xdc_runtime_knl_SemProcess_Proxy_Params_copy
#define SemProcess_Proxy_Params_init xdc_runtime_knl_SemProcess_Proxy_Params_init
#define SemProcess_Proxy_Instance_State xdc_runtime_knl_SemProcess_Proxy_Instance_State
#define SemProcess_Proxy_Proxy_abstract xdc_runtime_knl_SemProcess_Proxy_Proxy_abstract
#define SemProcess_Proxy_Proxy_delegate xdc_runtime_knl_SemProcess_Proxy_Proxy_delegate
#define SemProcess_Proxy_delete xdc_runtime_knl_SemProcess_Proxy_delete
#define SemProcess_Proxy_destruct xdc_runtime_knl_SemProcess_Proxy_destruct
#define SemProcess_Proxy_Module_upCast xdc_runtime_knl_SemProcess_Proxy_Module_upCast
#define SemProcess_Proxy_Module_to_xdc_runtime_knl_ISemProcessSupport xdc_runtime_knl_SemProcess_Proxy_Module_to_xdc_runtime_knl_ISemProcessSupport
#define SemProcess_Proxy_Handle_upCast xdc_runtime_knl_SemProcess_Proxy_Handle_upCast
#define SemProcess_Proxy_Handle_to_xdc_runtime_knl_ISemProcessSupport xdc_runtime_knl_SemProcess_Proxy_Handle_to_xdc_runtime_knl_ISemProcessSupport
#define SemProcess_Proxy_Handle_downCast xdc_runtime_knl_SemProcess_Proxy_Handle_downCast
#define SemProcess_Proxy_Handle_from_xdc_runtime_knl_ISemProcessSupport xdc_runtime_knl_SemProcess_Proxy_Handle_from_xdc_runtime_knl_ISemProcessSupport
#define SemProcess_Proxy_Module_upCast2 xdc_runtime_knl_SemProcess_Proxy_Module_upCast2
#define SemProcess_Proxy_Module_to_xdc_runtime_knl_ISemaphore xdc_runtime_knl_SemProcess_Proxy_Module_to_xdc_runtime_knl_ISemaphore
#define SemProcess_Proxy_Handle_upCast2 xdc_runtime_knl_SemProcess_Proxy_Handle_upCast2
#define SemProcess_Proxy_Handle_to_xdc_runtime_knl_ISemaphore xdc_runtime_knl_SemProcess_Proxy_Handle_to_xdc_runtime_knl_ISemaphore
#define SemProcess_Proxy_Handle_downCast2 xdc_runtime_knl_SemProcess_Proxy_Handle_downCast2
#define SemProcess_Proxy_Handle_from_xdc_runtime_knl_ISemaphore xdc_runtime_knl_SemProcess_Proxy_Handle_from_xdc_runtime_knl_ISemaphore

#endif /* xdc_runtime_knl_SemProcess_Proxy__localnames__done */
#endif
