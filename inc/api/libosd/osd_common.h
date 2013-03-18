/*====================================================================
 *
 *  Copyright (C) 2003 ALi.  All Rights Reserved.
 *
 *  File:   osdobj_common.h
 *
 *  Contents:    Provide OSDLib Common Function
 *
 *  History:
 *      Date          		By        	Reason
 *   ===========    	=======  	=============
 *	 mm/dd/yyyy
 *   09/16/2004     		Sunny Yin   Create
 *********************************************************************/
#ifndef _OSDOBJ_COMMON_H_
#define _OSDOBJ_COMMON_H_

#include <basic_types.h>
#include <api/libc/string.h>

#define OSD_ASSERT	ASSERT

typedef struct OSDRect  OSD_RECT, *POSD_RECT;
typedef const OSD_RECT *PCOSD_RECT;

#define IS_NUMBER_CHAR(ch)  ((INT8)(ch) >= '0' && (INT8)(ch) <= '9')
#define MAKEWORD(bLow, bHigh) \
    ((UINT16)(((UINT8)(bLow)) | ((UINT16)((UINT8)(bHigh))) << 8))
#define MAKELONG(wLow, wHigh) \
    ((UINT32)(((UINT16)(wLow)) | ((UINT32)((UINT16)(wHigh))) << 16))
#define FIELD_OFFSET(type, field) \
    ((INT32)(UINT32)&(((type *)0)->field))
#define CONTAINING_RECORD(address, type, field)	\
	((type *)((UINT32)(address) - (UINT32)(&((type *)0)->field)))
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(_array)      (sizeof(_array)/sizeof(_array[0]))
#endif

#define LOBYTE(_val)    ((UINT8)(_val))
#define HIBYTE(_val)    ((UINT8)((UINT16)(_val)>>8))
#define LOWORD(_val)    ((UINT16)(_val))
#define HIWORD(_val)    ((UINT16)((UINT32)(_val)>>16))


#define ComMemCopy3 libc_memcpy


UINT16 ComMB16ToUINT16(UINT8* pVal);

/*
UINT8 ComUniStrLen(const UINT16* string);
UINT8 ComUniStrCat(UINT16 * Target, const UINT16* Source);
UINT8 ComStrCat(UINT8 * Target, const UINT8* Source);
void  ComInt2Str(UINT8* lpStr, UINT32 dwValue, UINT8 bLen);
*/

UINT32  ComAscStr2Uni(UINT8* Ascii_str,UINT16* Uni_str);
void   ComUniStrToAsc(UINT8 * unicode,char* asc);
void   ComUniStrCopy(UINT16* Target,const UINT16* Source);
UINT32 ComUniStr2Int(const UINT16 *String);
UINT32  ComInt2UniStr(UINT16 *pBuffer, UINT32 num, UINT32 len);

void   ComUniStrCopyEx(UINT16* Target,const UINT16* Source,UINT32 len);
int    ComUniStrCopyChar(UINT8 *dest, UINT8 *src);
int ComUniStrCopyCharN(UINT8 *dest, UINT8 *src, UINT32 len);

void   ComUniStrMemSet(UINT16* Target,UINT8 c,UINT32 len);
char   GetUniStrCharAt(const UINT16* string,UINT32 pos);
void   SetUniStrCharAt(UINT16* string, char c, UINT32 pos);
BOOL   ComUniStr2IntExt(const UINT16* string,UINT8 cnt,UINT32* val);

#define ComMB16ToWord   ComMB16ToUINT16
BOOL  IS_NEWLINE(UINT16  wc);
UINT32 ComUniStrToMB(UINT16* pwStr);
UINT32 ComUniStrCat(UINT16 * Target, const UINT16* Source);

INT32 ComUniStrCmp(const UINT16 * dest, const UINT16* src);
INT32 ComUniStrCmpExt(const UINT8 * dest, const UINT8* src);

UINT16 *ComStr2UniStrExt(UINT16* uni, char* str, UINT16 maxcount);


#endif

