#ifndef __LD_RSC_LIB_H__
#define __LD_RSC_LIB_H__

#include <api/libloader/ld_osd_lib.h>

typedef enum
{
	LIB_FONT = 0x1000,
	LIB_FONT_MASSCHAR = 0x1f00,
	LIB_STRING = 0x2000,
	LIB_PALLETE = 0x4000,
	LIB_ICON = 0x8000,
}GUI_RSC_TYPE,*PGUI_RSC_TYPE;

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
/*******************************************/

extern UINT16 OSD_GetLangEnvironment(void);
extern BOOL OSD_SetLangEnvironment(UINT16 wLangClass);
extern BOOL OSD_GetObjInfo(UINT16 wLibClass, UINT16 uIndex, lpOBJECTINFO pObjectInfo);
extern UINT8 *OSD_GetRscObjData(UINT16 wLibClass, UINT16 uIndex, lpOBJECTINFO objectInfo);
extern ID_RSC OSD_GetDefaultFontLib(UINT16 uChar);
extern ID_RSC OSDExt_GetMsgLibId(UINT16 EnvID);

extern UINT8 *RSC_expand(UINT8* addr, UINT32 offset, UINT8* zip_mode);

#endif /* __LD_RSC_LIB_H__ */

