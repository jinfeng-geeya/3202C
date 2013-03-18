/**********************************************************************
 *
 *  Copyright (C) 2003 ALi.  All Rights Reserved.
 *
 *  File:   rsc.h
 *
 *  Contents:    module to analysis the raw data of special resource .
 *
 *  History:
 *      Date	     	By	       Reason
 *   ========	   =============   =============
 *   10/14/2003      Linda Liu	       Create
 *
 *********************************************************************/
#ifndef _RSC_LIB_H
#define _RSC_LIB_H

#include <types.h>
//#include <api/libge/osd_lib.h>
//#include <api/libge/osd_common_draw.h>
#include "win_com.h"
///////////////////////////////////////////////////////////////
// Language definition

#if(TTX_ON ==1)

typedef enum
{
//NULL_ENV,
ENGLISH_ENV,
CHINESE_ENV,
FRANCE_ENV,
GERMANY_ENV,
ITALY_ENV,
RUSSIAN_ENV,
SPANISH_ENV,
TURKISH_ENV,
PORTUGUESE_ENV,
ARABIC_ENV,
POLISH_ENV,
//CHINESE_ENV,
INDONESIAN_ENV,
IRAN_EVN,
GREECE_EVN,
SWEDISH_EVN,
SLAVIC_EVN,
DUTCH_EVN,
SLOVAK_EVN,
ROMANIAN_EVN,
HUNGARIAN_EVN,
CZECH_EVN,
BULGARIAN_EVN,
DANISH_EVN,
FINNISH_EVN,
ESTONIAN_EVN,
CATANIAN_EVN,
PERSIAN_EVN,
LATIN_EVN,
}LANG_GROUP_t;

#else

typedef enum
{
    ENGLISH_ENV = 0,
    CHINESE_ENV,
} LANG_GROUP_t;

#endif

#define DEFAULT_LANG_ENV    GERMANY_ENV//GERMANY_ENV


///////////////////////////////////////////////////////////////
// Font definition
#define LIB_FONT_NULL		0x1000
#define LIB_FONT_DEFAULT    0x1100
#define LIB_FONT_LATIN1     0x1200
#define LIB_FONT_LATINE     0x1300
#define LIB_FONT_GREEK	      0x1400
#define LIB_FONT_CYRILLIC	    0x1500
#define LIB_FONT_ARABIC	    0x1600
#define LIB_FONT_CHN	    0x1700
#define LIB_FONT_MASSCHAR1		0x1D00
#define LIB_FONT_MASSCHAR		0x1F00

///////////////////////////////////////////////////////////////
// String definition

#define LIB_STRING_ENGLISH	0x2100
#define LIB_STRING_CHINESE	0x2200

///////////////////////////////////////////////////////////////
// Pallete definition

#define LIB_PALLETE_8BPP	0x4080
#define LIB_PALLETE_4BPP	0x4040
#define LIB_PALLETE_2BPP	0x4020



#define LIB_CHAR_ASCII 0
/***************************************************************************************/
typedef struct tagLANG_TABLE
{
	UINT16 LangId;
	ID_RSC StrId;
	ID_RSC CharType;
	ID_RSC FontId;
} LANG_TABLE,  *lpLANG_TABLE;
typedef struct
{
	unsigned long	w;
	unsigned long	h;
	unsigned long	bit_count;
	unsigned long	bmp_size;
	unsigned long	data_size;
	unsigned long	data;
}bitmap_infor_t;

typedef struct
{
	unsigned short code;
	unsigned short width;
	unsigned short height;
	unsigned short stride;
	unsigned long data_offset;
}font_info_t;

typedef struct
{
	unsigned char lang[4];
	unsigned long count;
	unsigned short fontinfo_size;
	unsigned short bit_count;
	unsigned long len;
	unsigned long offset;
}font_array_t;

/*******************************************
 ***-the functions the client can visit***-*/

UINT16 OSD_GetLangEnvironment(void);
BOOL OSD_SetLangEnvironment(UINT16 wLangClass);

BOOL OSD_GetObjInfo(UINT16 wLibClass, UINT16 uIndex, lpOBJECTINFO pObjectInfo);
UINT8 *OSD_GetRscObjData(UINT16 wLibClass, UINT16 uIndex, lpOBJECTINFO objectInfo);
UINT8 *OSD_GetRscObjData_ext(UINT16 wLibClass, UINT16 uIndex , OBJECTINFO *pObjectInfo, UINT8 *font_bitmap_ext, UINT32 font_bitmap_size);

BOOL OSD_ReleaseObjData(UINT8 *lpDataBuf, lpOBJECTINFO pObjectInfo);

ID_RSC OSD_GetDefaultFontLib(UINT16 uChar);


PWINSTYLE OSDExt_GetWinStyle(UINT8 bStyleIdx);
ID_RSC OSDExt_GetMsgLibId(UINT16 EnvID);


/*******************************************************************************/

#endif
