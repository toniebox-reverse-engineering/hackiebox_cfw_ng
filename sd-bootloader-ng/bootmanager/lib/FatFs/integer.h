/*-------------------------------------------*/
/* Integer type definitions for FatFs module */
/*-------------------------------------------*/

#ifndef _FF_INTEGER
#define _FF_INTEGER

#ifdef _WIN32	/* FatFs development platform */

#include <windows.h>
#include <tchar.h>
typedef unsigned __int64 QWORD;

#else			/* Embedded platform */

//#include <arduino.h>

/* This type MUST be 8 bit */
typedef unsigned char   BYTE;

/* These types MUST be 16 bit */
typedef short           SHORT;
typedef unsigned short  WORD;
typedef unsigned short  WCHAR;

#ifdef ESP8266
/* These types MUST be 16 bit or 32 bit */
typedef int             INT;
typedef unsigned int    UINT;
#else
typedef long            INT;
typedef unsigned long   UINT;
#endif

/* These types MUST be 32 bit */
typedef long            LONG;
typedef unsigned long   DWORD;

/* This type MUST be 64-bit (Remove this for ANSI C (C89) compatibility) */
typedef unsigned long long QWORD;

#endif

#endif
