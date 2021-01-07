/*
 *  ======== C28_float.h ========
 *  DO NOT MODIFY: This header is generated from stddef.xdt
 *
 *  This header contains target-specific definitions of target-independent
 *  macros required by the ITarget interface.  These definitions allow C/C++
 *  sources to portably compile for multiple targets (using #ifdef ...).
 */
#ifndef xdc_target_macros_include__
#define xdc_target_macros_include__    1

/*
 *  ======== ti_targets_C28_float ========
 *  This macro identifies the specific target being used.  This macro should
 *  probably be avoided in portable sources.
 */
#define ti_targets_C28_float  1

/*
 *  ======== xdc_target__arraytype_VaList ========
 *  The following macro specifies whether or not a VaList is an 
 *  array type; e.g., struct __va_list_tag (*)[1] verses a simple char *.  
 *  This affects how va_list variables are passed to functions (by value
 *  or by address).
 */
#define xdc_target__arraytype_VaList 0

/*
 *  ======== xdc_target__isaCompatible_* macros ========
 *  The following definitions enable clients to conditionally compile for any
 *  compatible subset of the actual target ISA.
 */
#define xdc_target__isaCompatible_28  1
#define xdc_target__isaCompatible_28FP  1

/*
 *  ======== xdc_target__isa_28FP ========
 *  This macro identifies the specific target ISA for which we are being
 *  compiled.
 */
#define xdc_target__isa_28FP  1

/*
 *  ======== xdc_target__{big|little}Endian ========
 *  The following macro enables clients to portably compile for big or little
 *  endian targets.
 */
#define xdc_target__littleEndian   1

/*
 *  ======== xdc_target__{...}Data ========
 *  The following macro enables clients to portably compile for target specific
 *  data models; e.g., near, far, ...
 */
#define xdc_target__largeData  1

/*
 *  ======== xdc_target__os_undefined ========
 *  The following macro enables clients to portably compile for target specific
 *  OS; e.g., Linux, Solaris, Windows, undefined.
 */
#define xdc_target__os_undefined    1

/*
 *  ======== xdc_target__sizeof_ ========
 *  The following macros enable clients to portably determine type sizes
 *  within #ifdef blocks; sizeof() can't be used and the definitions in
 *  stdint.h are not available to C++ clients (unless the special macro
 *  __STDC_LIMIT_MACROS is defined).
 */
#define  xdc_target__sizeof_IArg 2
#define  xdc_target__sizeof_Char 1
#define  xdc_target__sizeof_Double 2
#define  xdc_target__sizeof_Float 2
#define  xdc_target__sizeof_Fxn 2
#define  xdc_target__sizeof_Int 1
#define  xdc_target__sizeof_Int8 1
#define  xdc_target__sizeof_Int16 1
#define  xdc_target__sizeof_Int32 2
#define  xdc_target__sizeof_Int64 4
#define  xdc_target__sizeof_Long 2
#define  xdc_target__sizeof_LDouble 2
#define  xdc_target__sizeof_LLong 4
#define  xdc_target__sizeof_Ptr 2
#define  xdc_target__sizeof_Short 1
#define  xdc_target__sizeof_Size 2

/*
 *  ======== xdc_target__alignof_ ========
 *  The following macros enable clients to portably determine type alignment
 *  within #ifdef blocks; even if provided by the compiler, alignof() can't
 *  be used in pre-processor statements.
 */
#define  xdc_target__alignof_IArg 2
#define  xdc_target__alignof_Char 1
#define  xdc_target__alignof_Double 2
#define  xdc_target__alignof_Float 2
#define  xdc_target__alignof_Fxn 2
#define  xdc_target__alignof_Int 1
#define  xdc_target__alignof_Int8 1
#define  xdc_target__alignof_Int16 1
#define  xdc_target__alignof_Int32 2
#define  xdc_target__alignof_Int64 2
#define  xdc_target__alignof_Long 2
#define  xdc_target__alignof_LDouble 2
#define  xdc_target__alignof_LLong 2
#define  xdc_target__alignof_Ptr 2
#define  xdc_target__alignof_Short 1
#define  xdc_target__alignof_Size 2

/*
 *  ======== xdc_target__bitsPerChar ========
 *  The number of bits in a char.  This macro allow one to determine the
 *  precise number of bits in any of the standard types (whose sizes are
 *  expressed as a number of chars).
 */
#define  xdc_target__bitsPerChar 16

#endif  /* end multi-include guard */
