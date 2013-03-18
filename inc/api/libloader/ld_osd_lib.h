#ifndef __LD_OSD_LIB_H__
#define __LD_OSD_LIB_H__

#include <basic_types.h>
#include <mediatypes.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/osd/osddrv.h>
#include <api/libchar/lib_char.h>

#define C_ALIGN_CENTER		0x00
#define C_ALIGN_LEFT		0x01
#define C_ALIGN_RIGHT		0x02
#define C_ALIGN_TOP			0x04
#define C_ALIGN_BOTTOM		0x08
#define C_ALIGN_VCENTER	0x0C

#define GET_HALIGN(_align)  ((UINT8)(_align) & 0x03)
#define GET_VALIGN(_align)  ((UINT8)(_align) & 0x0C)

#define	C_NOSHOW				0x1000
#define 	C_NOSHOW_TEXT		0x1000		// Only effect Fg
#define	C_MIXER				0x2000		// Only effect Bg/Fg
#define	C_DOUBLE_LINE			0x4000		// Only effect Line
#define	C_MULTI_LINE			0x8000		// only effect Line

#define	OSD_DIRDRAW			0
#define	OSD_REQBUF			1
#define 	OSD_GET_CORNER		2
#define 	OSD_GET_BACK      		4

typedef UINT16 ID_RSC;
#define	OSD_INVALID_REGION	0xFF

/*****************************************
***-the property of a certain object-****
NOTE:members of this struct is meaningful 
only to font and icon object,except m_fFlag,
which is usefule to all kinds of object*/ 
typedef struct tag_FontIcon_Prop
{
	UINT16	m_wHeight;
	UINT16	m_wWidth;
	UINT16	m_wActualWidth;			//the actual width the data was stored,because filling happened
						//to match with the unit of 8bytes
						//actualwidth data with the unit of UINT8,not BITE
						//considering you can calculate from width and color,why give it???
	UINT8	m_bColor;			// the color BPP of this object
}FONTICONPROP,*lpFONTICONPROP;

typedef struct tag_ObjectInfo
{
	UINT16	m_wObjClass;			// the class ID of FONT,STRING the object belongs to
									// the object ID of ICON,PALLETE
	FONTICONPROP 	m_objAttr;		// the attribute of this object
}OBJECTINFO,*lpOBJECTINFO;

typedef struct
{
	UINT16 uLeft;
	UINT16 uTop;
	UINT16 uWidth;       /* Width of the pixmap */
	UINT16 uHeight;      /* Height of the pixmap */
	UINT16 stride;      /* Number of bytes per line in the pixmap */
	UINT16 bitsPerPix;	//
	UINT8  *pData;
}BitMap_t;

struct OSD_InitInfo	
{
	struct osd_device*		p_handle;
	struct OSDPara			tOpenPara;
	struct OSDRect 			osdrect;
};

typedef struct OSDRect  OSD_RECT, *POSD_RECT;

#define OSD_SetRect(r, _l, _t, _w, _h) \
    do{ \
        ((POSD_RECT)(r))->uLeft = (UINT16)(_l); \
        ((POSD_RECT)(r))->uTop = (UINT16)(_t); \
        ((POSD_RECT)(r))->uWidth = (UINT16)(_w); \
        ((POSD_RECT)(r))->uHeight = (UINT16)(_h); \
    }while(0)

#define OSD_CopyRect(dest, src) \
    do{ \
        ((UINT32 *)(dest))[0] = ((UINT32 *)(src))[0]; \
        ((UINT32 *)(dest))[1] = ((UINT32 *)(src))[1]; \
    }while(0)
#define OSD_SetRect2    OSD_CopyRect

UINT16 ComMB16ToUINT16(UINT8* pVal);
#define ComMB16ToWord   ComMB16ToUINT16

void OSD_DrawFill(UINT16 x, UINT16 y, UINT16 w, UINT16 h, UINT8 color, lpVSCR pVscr);
void OSD_DrawFrame(struct OSDRect* pFrame, UINT8 color, lpVSCR pVscr);
void OSD_DrawClut2(UINT16 x, UINT16 y,UINT8 *pImg,lpFONTICONPROP imgInfo,UINT16 Style, lpVSCR pVscr);
VSCR* OSD_GetVscr(struct OSDRect* pRect, UINT32 bFlag);

#endif /* __LD_OSD_LIB_H__ */

