#ifndef __TDS_DATA_TYPES_H__
#define __TDS_DATA_TYPES_H__

#ifdef	ALi_x86_Module
//#include "windows.h"
//#include "wintype.h"
#define __FUNCTION__	__FILE__

#undef LOBYTE
#undef HIBYTE
#undef LOWORD
#undef HIWORD
#undef ASSERT

#else

#define INLINE inline

#include <types.h>
	//************************************************************
	// Data types that has been defined by MS Windows
	//************************************************************
//	typedef long	INT32;
	typedef	long	INT;
	typedef INT32*	LPINT32;

//	typedef unsigned long	UINT32;
	typedef	unsigned long	UINT;
	typedef UINT32*	LPUINT32;

//	typedef UINT32	BOOL;
//	#define	TRUE	1
//	#define	FALSE	0

//	#define NULL 	(0)

	typedef unsigned char	BYTE;
	typedef unsigned short	WORD;
	typedef unsigned long	DWORD;
	typedef struct tagRECT
	{
		WORD left;
		WORD top;
		WORD right;
		WORD bottom;
	}
	*LPRECT, RECT;

#endif



//typedef char		INT8;
//typedef short		INT16;
typedef INT8*	LPINT8;
typedef INT16*	LPINT16;

//typedef unsigned char	UINT8;
//typedef unsigned short	UINT16;
typedef UINT8*	LPUINT8;
typedef UINT16*	LPUINT16;

//************************************************************
// Common Type Definition
//************************************************************
typedef BYTE*	LPBYTE;
typedef WORD*	LPWORD;
typedef DWORD*	LPDWORD;
//typedef void*	HANDLE;
#define	INVALID_HANDLE	((HANDLE)0xFFFFFFFF)
#define	MAX_DWORD			0xFFFFFFFF
#define	MAX_WORD			0xFFFF


//************************************************************
#define C_OFF                   0
#define C_ON                    1

// General Time Definition;
typedef struct _T_TIME
{
	UINT16	Year;
	UINT8	Month;
	UINT8	Day;
	UINT8	Hour;
	UINT8	Minute;
	UINT8	Second;
}T_TIME;

// Include legacy types to satisfy legacy modules
//#include "Legacy.h"

#endif

