/*-----------------------------------------------------------------------------
*	Copyright (C) 2010 ALI Corp. All rights reserved.
*	GUI object library with GE HW accerlaration by Shine Zhou.
*-----------------------------------------------------------------------------*/
#ifndef _OSD_LIB_H_
#define _OSD_LIB_H_

#include <basic_types.h>
#include <mediatypes.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/ge/ge.h>
#include <api/libchar/lib_char.h>

#include "osd_common_draw.h"
#include "osd_primitive.h"
#include "osd_common.h"
#include "osd_vkey.h"

#define OSD_MULTI_REGION	0
#define OSD_VSCR_SUPPORT	1

#define V_KEY_NULL	0xFF
#define OSD_FONT_HEIGHT 24 //???

#define OSD_SIGNAL	OSD_NotifyEvent

typedef UINT8 PRESULT;
typedef UINT8 VACTION;
typedef UINT32 VEVENT;

//********** Message type Definiton **********//
typedef enum
{
	MSG_TYPE_KEY = 0,
	MSG_TYPE_EVNT,
	MSG_TYPE_MSG,
	MSG_TYPE_RSVD,
}APP_MSG_TYPE,*PAPP_MSG_TYPE;

//********** VACTION Definiton **********//
typedef enum
{
	VACT_NUM_0 = 1,
	VACT_NUM_1,
	VACT_NUM_2,
	VACT_NUM_3,
	VACT_NUM_4,
	VACT_NUM_5,
	VACT_NUM_6,
	VACT_NUM_7,
	VACT_NUM_8,
	VACT_NUM_9,

	VACT_CLOSE,

	VACT_CURSOR_UP,
	VACT_CURSOR_DOWN,
	VACT_CURSOR_LEFT,
	VACT_CURSOR_RIGHT,
	VACT_CURSOR_PGUP,
	VACT_CURSOR_PGDN,	

	VACT_SELECT,

	VACT_INCREASE,//progress bar/multisel/editfield whole data +
	VACT_DECREASE,//progress bar/multisel/editfield,whole data -

	VACT_ENTER,

	/* For Edit Field only*/
	VACT_EDIT_LEFT,
	VACT_EDIT_RIGHT,
	VACT_EDIT_ENTER,
	VACT_EDIT_CANCEL_EXIT,
	VACT_EDIT_SAVE_EXIT,
	VACT_EDIT_INCREASE_,
	VACT_EDIT_DECREASE_,
	VACT_NO_RESPOND,
	VACT_EDIT_PASSWORD,
	VACT_PASS	= 0x30,
}VACT_t;

//********** VEVENT Definiton **********//
typedef enum
{
	EVN_PRE_DRAW = 1,
	EVN_POST_DRAW,
	
	EVN_ENTER = 11,
	EVN_REQUEST_DATA,
	EVN_REQUEST_STRING,

	EVN_FOCUS_PRE_LOSE = 21,
	EVN_FOCUS_POST_LOSE,
	EVN_FOCUS_PRE_GET,
	EVN_FOCUS_POST_GET,
	EVN_PARENT_FOCUS_PRE_LOSE,
	EVN_PARENT_FOCUS_POST_LOSE,
	EVN_PARENT_FOCUS_PRE_GET,
	EVN_PARENT_FOCUS_POST_GET,
	EVN_ITEM_PRE_CHANGE,
	EVN_ITEM_POST_CHANGE,

	EVN_PRE_OPEN = 31,
	EVN_POST_OPEN,
	EVN_PRE_CLOSE,
	EVN_POST_CLOSE,

	EVN_CURSOR_PRE_CHANGE = 41,
	EVN_CURSOR_POST_CHANGE,

	EVN_PRE_CHANGE = 51,
	EVN_POST_CHANGE,
    EVN_DATA_INVALID,
    
	EVN_UNKNOWN_ACTION = 61,

	EVN_KEY_GOT = 71,
	EVN_UNKNOWNKEY_GOT,

	EVN_MSG_GOT = 81,

	EVN_VSCR_PREUPDATE = 91,
	EVN_VSCR_POSTUPDATE,
	//TO ADD:
	EVN_FOCUS_SLIDE,
	EVN_MSG_RSVD,
	
	EVN_MSG_MAX = 0xff,
}GUI_EVN_TYPE,*PGUI_EVN_TYPE;

//********** PROC return Definiton **********//
typedef enum
{
	PROC_SKIP = 0xfc,
	PROC_LEAVE = 0xfd,
	PROC_PASS = 0xfe,
	PROC_LOOP = 0xff,
}GUI_PROC_TYPE,*PGUI_PROC_TYPE;

typedef enum
{
	STRING_NULL = 0,
	STRING_ID,          // string id, data type is UINT16
	STRING_ID_UINT8,    // string id, data type is UINT8
	STRING_ANSI,        // ANSI string
	STRING_UTF8,        // UTF8 string
	STRING_UNICODE,     // Unicode string or String ID
	STRING_MBCS,        // MBCS
	STRING_NUM_TOTAL,   // num/total format
	STRING_NUMBER,      // decimal number
	STRING_NUM_PERCENT, // decimal number + %
	STRING_PROC,        // call proc to get string 
	STRING_REVERSE,	//for arabic
}STRING_TYPE;

//********** Object HEAD  Definiton **********//

#define GET_HALIGN(_align)  ((UINT8)(_align) & 0x03)
#define GET_VALIGN(_align)  ((UINT8)(_align) & 0x0C)
typedef enum
{
	C_ALIGN_CENTER = 0x0,
	C_ALIGN_LEFT = 0x01,
	C_ALIGN_RIGHT = 0x02,
	C_ALIGN_TOP = 0x04,
	C_ALIGN_BOTTOM = 0x08,
	C_ALIGN_VCENTER = 0x0c,
	C_ALIGN_SCROLL	 = 0x10,
}GUI_ALIGN_TYPE,*PGUI_ALIGN_TYPE;


typedef enum
{
	C_FONT_DEFAULT = 0,
	C_FONT_1,
	C_FONT_2,
	C_FONT_3,
	C_FONT_4,
	C_FONT_5,
	C_FONT_SLVMASK = 0xf0,
}GUI_FONT_SIZE,*PGUI_FONT_SIZE;

typedef enum
{
	OT_TEXTFIELD = 0, 
	OT_EDITFIELD, 
	OT_BITMAP, 
	OT_MULTISEL, 
	OT_PROGRESSBAR, 
	OT_SCROLLBAR, 
	OT_MULTITEXT, 
	OT_LIST, 
	OT_MATRIXBOX, 
	OT_OBJLIST, 
	OT_CONTAINER, 
	OT_MAX
}GUI_OBJ_TYPE,*PGUI_OBJ_TYPE;

typedef enum
{
	C_ATTR_ACTIVE = 0x01,
	C_ATTR_HIGHLIGHT,
	C_ATTR_SELECT,
	C_ATTR_INACTIVE,
	C_ATTR_HIDDEN,
	
	C_ATTR_LMASK = 0xf,
	
	C_ATTR_SLAVE = 0x10,
	C_ATTR_HMASK = 0xf0,
}GUI_OBJ_ATTR,*PGUI_OBJ_ATTR;

typedef struct _COLOR_STYLE
{
	UINT8	bShowIdx;
	UINT8	bHLIdx;       // Style during HighLight
	UINT8	bSelIdx;      // Style during Selected, add by sunny 07/21, For Window Object, it's Close Attribute
	UINT8	bGrayIdx;     // Gray Style, It will be active while bAction == ACTION_GRAY. This Attribute is not included in Window Object
}COLOR_STYLE, *PCOLOR_STYLE;

typedef struct _OBJECT_HEAD     OBJECT_HEAD;
typedef struct _OBJECT_HEAD 	*POBJECT_HEAD;
typedef VACTION (*PFN_KEY_MAP)(POBJECT_HEAD pObj, UINT32 Key);
typedef PRESULT (*PFN_CALLBACK)(POBJECT_HEAD pObj, VEVENT Event, UINT32 Param1, UINT32 Param2);
struct _OBJECT_HEAD
{
	UINT8 bType;
	UINT8 bAttr;// defined in form C_ATTR_XXX
	UINT8 bFont;// defined in form C_FONT_XXX
	UINT8 bID;// 0: for component without focus

	UINT8 bLeftID;
	UINT8 bRightID;
	UINT8 bUpID;
	UINT8 bDownID;

	GUI_RECT  frame;
	COLOR_STYLE style;

	PFN_KEY_MAP pfnKeyMap;
	PFN_CALLBACK pfnCallback;

	POBJECT_HEAD pNext;
	POBJECT_HEAD pRoot;
};

typedef struct SLAVE_OBJHEAD
{
	GUI_RECT  frame;
	COLOR_STYLE style;
}SDHEAD,*PSDHEAD;

typedef enum
{
	C_UPDATE_ALL = 0x01,
	C_UPDATE_FOCUS,
	C_UPDATE_CONTENT,
	C_UPDATE_FRAME,
	C_UPDATE_CURSOR,
	C_UPDATE_MASK = 0xf,
}GUI_OBJ_UPDATE,*PGUI_OBJ_UPDATE;

typedef enum
{
	C_DRAW_TYPE_NORMAL = (0x01<<4),
	C_DRAW_TYPE_HIGHLIGHT = (0x02<<4),
	C_DRAW_TYPE_GRAY = (0x03<<4),
	C_DRAW_TYPE_SELECT = (0x04<<4),
	C_DRAW_TYPE_HIDE = (0x05<<4),
	C_DRAW_TYPE_MASK = (0xf<<4),
}GUI_OBJ_DRAW_TYPE,*PGUI_OBJ_DRAW_TYPE;

typedef enum
{
	C_DRAW_SIGN_EVN_FLG = 0x80000000UL,
	C_CLOSE_CLRBACK_FLG = 0x40000000UL,
}GUI_OBJ_DRAW_PARAM,*PGUI_OBJ_DRAW_PARAM;

#define OSD_SetUpdateType(nCmdDraw,update_type) \
	nCmdDraw = ( (nCmdDraw & 0xFFFFFFF0) | update_type)
#define OSD_GetUpdateType(nCmdDraw)	\
	((nCmdDraw) & 0x0F)
#define OSD_SetDrawType(nCmdDraw,drawType) \
	nCmdDraw = ( (nCmdDraw & 0xFFFFFF0F) | drawType)
#define OSD_GetDrawType(nCmdDraw) \
	( (nCmdDraw) & 0xF0)

#define OSD_SetColor(pObj, _bShowIdx, _bHLIdx, _bSelIdx, _bGrayIdx) \
    do{ \
        ((POBJECT_HEAD)(pObj))->style.bShowIdx = (UINT8)(_bShowIdx); \
        ((POBJECT_HEAD)(pObj))->style.bHLIdx = (UINT8)(_bHLIdx); \
        ((POBJECT_HEAD)(pObj))->style.bSelIdx = (UINT8)(_bSelIdx); \
        ((POBJECT_HEAD)(pObj))->style.bGrayIdx = (UINT8)(_bGrayIdx); \
    }while(0)
#define OSD_GetColor(pObj) (&(((POBJECT_HEAD)(pObj))->Style))

#define OSD_SetID(pObj, _bID, _bLID, _bRID, _bUID, _bDID) \
    do{ \
        ((POBJECT_HEAD)(pObj))->bID = (UINT8)(_bID); \
        ((POBJECT_HEAD)(pObj))->bLeftID = (UINT8)(_bLID); \
        ((POBJECT_HEAD)(pObj))->bRightID = (UINT8)(_bRID); \
        ((POBJECT_HEAD)(pObj))->bUpID = (UINT8)(_bUID); \
        ((POBJECT_HEAD)(pObj))->bDownID = (UINT8)(_bDID); \
    }while(0)
#define OSD_GetObjID(pObj)  ( ((POBJECT_HEAD)(pObj))->bID)

#define OSD_SetKeyMap(pObj, _pfnKeyMap) \
    do{ \
        ((POBJECT_HEAD)(pObj))->pfnKeyMap = (PFN_KEY_MAP)(_pfnKeyMap); \
    }while(0)
#define OSD_SetCallback(pObj, _pfnCallback) \
    do{ \
        ((POBJECT_HEAD)(pObj))->pfnCallback = (PFN_CALLBACK)(_pfnCallback); \
    }while(0)
    
#define OSD_GetAttrSlave(pObj) \
			(((((POBJECT_HEAD)(pObj))->bAttr)&C_ATTR_SLAVE) == C_ATTR_SLAVE)

#define OSD_SetAttrSlave(pObj)\
   do{((POBJECT_HEAD)(pObj))->bAttr = (UINT8)(((POBJECT_HEAD)(pObj))|C_ATTR_SLAVE);}while(0)
   	
#define OSD_CheckAttr(pObj, _bAttr) \
    (((((POBJECT_HEAD)(pObj))->bAttr)&C_ATTR_LMASK) == (UINT8)(_bAttr))
#define OSD_SetAttr(pObj, _bAttr) \
    do{((POBJECT_HEAD)(pObj))->bAttr = (UINT8)(_bAttr);}while(0)

#define OSD_SetRect(r, _l, _t, _w, _h) \
    do{ \
        ((PGUI_RECT)(r))->uStartX  = (UINT16)(_l); \
        ((PGUI_RECT)(r))->uStartY = (UINT16)(_t); \
        ((PGUI_RECT)(r))->uWidth = (UINT16)(_w); \
        ((PGUI_RECT)(r))->uHeight = (UINT16)(_h); \
    }while(0)
    
#define OSD_CopyRect(dest, src) OSD_SetRect(dest,(src)->uStartX, (src)->uStartY, (src)->uWidth,(src)->uHeight)
    
#define OSD_SetRect2    OSD_CopyRect
#define OSD_ZeroRect(rct) OSD_SetRect(rct,0,0,0,0)

#define OSD_SetObjRect(pObj,_l, _t, _w, _h)	\
	OSD_SetRect(& (((POBJECT_HEAD)(pObj))->frame),_l, _t, _w, _h)

#define OSD_SetObjpNext(pObj,pNextObj)	\
	((POBJECT_HEAD)(pObj))->pNext = (POBJECT_HEAD)(pNextObj)
#define OSD_GetObjpNext(pObj)	\
	((POBJECT_HEAD)(pObj))->pNext
#define OSD_SetObjRoot(pObj,pRootObj)	\
	((POBJECT_HEAD)(pObj))->pRoot = (POBJECT_HEAD)(pRootObj)
#define OSD_GetObjRoot(pObj)	\
	((POBJECT_HEAD)(pObj))->pRoot

#define OSD_SetOSDMessage(msg,msgtype,msgcode)	\
	((msg) = ((msgtype) << 16) | (msgcode))
#define OSD_GetOSDMessage(msgtype,msgcode)	\
	( ((msgtype) << 16) | (msgcode))
#define OSD_GetOSDMessageType(msg)	\
	((msg)>>16)
#define	OSD_GetOSDMessageCode(msg)	\
	((msg) & 0xFFFF)
	
BOOL	OSD_RectInRect(const PGUI_RECT R,const PGUI_RECT r);
void	OSD_GetRectsCross(const PGUI_RECT backR,const PGUI_RECT forgR,PGUI_RECT relativR);

POBJECT_HEAD	OSD_GetTopRootObject(POBJECT_HEAD pObj);
UINT8	OSD_GetFocusID(POBJECT_HEAD pObj);
void OSD_SetWholeObjAttr(POBJECT_HEAD	pObj, UINT8 bAttr);
POBJECT_HEAD	OSD_GetObject(POBJECT_HEAD pObj, UINT8 bID);
POBJECT_HEAD	OSD_GetFocusObject(POBJECT_HEAD pObj);
UINT8	OSD_GetAdjacentObjID(POBJECT_HEAD pObj, VACTION action);
POBJECT_HEAD	OSD_GetAdjacentObject(POBJECT_HEAD pObj, VACTION action);
PRESULT 	OSD_ChangeFocus(POBJECT_HEAD pObj, UINT16 bNewFocusID,UINT32 parm);

PGUI_VSCR OSD_DrawSlaveFrame(PGUI_RECT prct,UINT32 bStyleIdx);
UINT8 OSD_GetSlaveWstyle(PCOLOR_STYLE pstyle,POBJECT_HEAD pobj,UINT8 colorstyle);
UINT8	OSD_GetShowIndex(const PCOLOR_STYLE pColorStyle, UINT8 bAttr);
PGUI_VSCR	OSD_DrawObjectFrame(PGUI_RECT pRect,UINT32 bStyleIdx);
void	OSD_DrawObjectCell(POBJECT_HEAD pObj, UINT8 bStyleIdx,	UINT32 nCmdDraw);
PRESULT	OSD_DrawObject(POBJECT_HEAD pObj, UINT32 nCmdDraw );
PRESULT	OSD_TrackObject(POBJECT_HEAD pObj,UINT32 nCmdDraw );
PRESULT	OSD_SelObject(POBJECT_HEAD pObj,  UINT32 nCmdDraw );
void	OSD_HideObject(POBJECT_HEAD pObj, UINT32 nCmdDraw );
void	OSD_ClearObject(POBJECT_HEAD pObj, UINT32 nCmdDraw);

PRESULT	OSD_NotifyEvent(POBJECT_HEAD pObj, VEVENT Event, UINT32 Param1, UINT32 Param2);
PRESULT	OSD_ObjOpen(POBJECT_HEAD pObj, UINT32 param);
PRESULT	OSD_ObjClose(POBJECT_HEAD pObj,UINT32 param);
PRESULT	OSD_ObjProc(POBJECT_HEAD pObj, UINT32 msgType,UINT32 msg,UINT32 param1);
//***********************************************************************************************//

#include "obj_textfield.h"
#include "obj_bitmap.h"
#include "obj_scrollbar.h"
#include "obj_progressbar.h"
#include "obj_objectlist.h"
#include "obj_container.h"
#include "obj_editfield.h"
#include "obj_multisel.h"
#include "obj_multitext.h"
#include "obj_list.h"
#include "obj_matrixbox.h"
#include "osd_common_draw.h"

typedef enum
{
	LIB_FONT = 0x1000,
	LIB_FONT_MASSCHAR = 0x1f00,
	LIB_STRING = 0x2000,
	LIB_PALLETE = 0x4000,
	LIB_ICON = 0x8000,
}GUI_RSC_TYPE,*PGUI_RSC_TYPE;

typedef struct tag_FontIcon_Prop
{
	UINT16	m_wHeight;
	UINT16	m_wWidth;
	UINT16	m_wActualWidth;
	UINT8	m_bColor;	
#ifndef _UNCOMPRESSED_ARRAY_METHOD
	BOOL	m_fFlag;//flag to show the data being compressed,structure so----
#endif 
}FONTICONPROP;

typedef struct tag_ObjectInfo
{
	UINT16	m_wObjClass;			// the class ID of FONT,STRING the object belongs to
	FONTICONPROP 	m_objAttr;		// the attribute of this object
}OBJECTINFO,*lpOBJECTINFO;

typedef	UINT16	(*OSD_GET_LANG_ENV)(void);
typedef	BOOL	(*OSD_GET_OBJ_INFO)(UINT16 wLibClass,UINT16 uIndex,lpOBJECTINFO pObjectInfo);
typedef	UINT8* 	(*OSD_GET_RSC_DATA)(UINT16 wLibClass,UINT16 uIndex,lpOBJECTINFO objectInfo);
typedef	UINT8* 	(*OSD_GET_THAI_FONT_DATA)(UINT16 wLibClass,struct thai_cell *cell,lpOBJECTINFO objectInfo);
typedef	BOOL   	(*OSD_REL_RSC_DATA)(UINT8* lpDataBuf,lpOBJECTINFO pObjectInfo);
typedef 	ID_RSC 	(*OSD_GET_FONT_LIB)(UINT16 uChar);
typedef 	PWINSTYLE (*OSD_GET_WIN_STYLE)(UINT8 bStyleIdx);
typedef 	ID_RSC    (*OSD_GET_STR_LIB_ID)(UINT16 EnvID);
typedef	UINT32 (*AP_HK_TO_VK)(UINT32 start, UINT32 key_info, UINT32* vkey );

typedef struct OSD_RscFunc
{/*Split from old struct,to keep OSD_LIB maintanace*/
	OSD_GET_LANG_ENV	osd_get_lang_env;
	OSD_GET_OBJ_INFO		osd_get_obj_info;
	OSD_GET_RSC_DATA	osd_get_rsc_data;
	OSD_GET_THAI_FONT_DATA osd_get_thai_font_data;
	OSD_REL_RSC_DATA	osd_rel_rsc_data;
	OSD_GET_FONT_LIB 	osd_get_font_lib;
	OSD_GET_WIN_STYLE	osd_get_win_style;
	OSD_GET_STR_LIB_ID	osd_get_str_lib_id;
	AP_HK_TO_VK			ap_hk_to_vk;
}GUI_RSC_FNC,*PGUI_RSC_FNC;

extern GUI_RSC_FNC g_gui_rscfunc;
UINT8 app_rsc_init(PGUI_RSC_FNC p_rsc_info);
BOOL    OSD_GetCharWidthHeight(UINT16 uString,UINT8 font, UINT16* width, UINT16* height);
BOOL OSD_GetThaiCellWidthHeight(struct thai_cell *cell, UINT8 font, UINT16* width, UINT16* height);
UINT16  OSD_MultiFontLibStrMaxHW(UINT8* pString,UINT8 font, UINT16 *wH, UINT16 *wW,UINT16 strLen);
typedef void(*OSD_NotifyFunc)(POBJECT_HEAD pobj);
void OSD_RegOpenNotifyFunc(OSD_NotifyFunc func);
void OSD_RegCloseNotifyFunc(OSD_NotifyFunc func);
UINT8*  OSD_GetUnicodeString(UINT16 uIndex);
BOOL OSD_GetLibInfoByWordIdx(UINT16 wIdx, ID_RSC RscLibId, UINT16 *wWidth, UINT16 *uHeight);
UINT8* OSD_GetRscPallette(UINT16 wPalIdx);
void OSD_SetCloseClrbackFlag(BOOL bClear);
void OSD_ResumeCloseClrbackFlag(void);
#endif//_OSD_LIB_H_

