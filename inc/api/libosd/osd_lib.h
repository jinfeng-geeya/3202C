#ifndef _OSD_LIB_H_
#define _OSD_LIB_H_

#include <basic_types.h>
#include <mediatypes.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/osd/osddrv.h>
#include <hld/ge/ge.h>
#include <api/libchar/lib_char.h>

#include "osd_common.h"
#include "osd_vkey.h"

#define OSD_MULTI_REGION	0
#define OSD_VSCR_SUPPORT	1

#define V_KEY_NULL	0xFF
#define OSD_FONT_HEIGHT 24 //???

/////////////////////////////////////////////////////////////
//********** Message type Definiton **********//
/////////////////////////////////////////////////////////////
#define	MSG_TYPE_KEY		0
#define	MSG_TYPE_EVNT		1
#define	MSG_TYPE_MSG		2

/////////////////////////////////////////////////////////////
//********** VACTION Definiton **********//
/////////////////////////////////////////////////////////////
// Common Basic Key
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

/////////////////////////////////////////////////////////////
//********** VEVENT Definiton **********//
/////////////////////////////////////////////////////////////


// Param1 - Drawing mode
// Param2 - only for MatrixBox, specifies the cell rect.
// Return - PROC_LOOP: terminate the default drawing execution
//          PROC_PASS: continue drawing
#define EVN_PRE_DRAW        1
#define EVN_POST_DRAW       2

// Param1 - Determined by component type
//          TextField: control ID
//          EditField: the current value, pointer to a string or a numeric value 
//                     depending on editfield style
//          Multisel, List, MatrixBox: the current selection
// Return - Determined by component type
//          TextField: PROC_LEAVE to exit the applicaton.
//                     PROC_LOOP: continue done.
#define EVN_ENTER           11

#define EVN_FOCUS_PRE_LOSE    21
#define EVN_FOCUS_POST_LOSE      22

#define EVN_FOCUS_PRE_GET   23
#define EVN_FOCUS_POST_GET       24

#define EVN_PARENT_FOCUS_PRE_LOSE	   25
#define EVN_PARENT_FOCUS_POST_LOSE	   26
#define EVN_PARENT_FOCUS_PRE_GET   27
#define EVN_PARENT_FOCUS_POST_GET       28

#define EVN_ITEM_PRE_CHANGE   29
#define EVN_ITEM_POST_CHANGE    30

/*
#define EVN_WIN_CLOSING       31
#define EVN_WIN_CLOSED        32
#define EVN_WIN_OPENING       33
#define EVN_WIN_OPENED        34
*/

#define EVN_PRE_OPEN			31
#define EVN_POST_OPEN			32
#define EVN_PRE_CLOSE			33
#define EVN_POST_CLOSE			34

// Only for EditField, List, MatrixBox
// Param1 - pointer to the new cursor position. (UINT8 *)
#define EVN_CURSOR_PRE_CHANGE 41   // The cursor will be changed
// Param1 - the current cursor position
#define EVN_CURSOR_POST_CHANGE  42   // The cursor has been changed

// Only for EditField, Multisel, List, and MatrixBox
// Param1 - Determined by component type
//          EditField: pointer to the new value. (UINT32 * or PWCHAR)
//          Multisel, List, MatrixBox: pointer to the new selection
#define EVN_PRE_CHANGE        51  // control will change its content
// Param1 - Determined by component type
//          EditFiled: the current value
//          Multisel, List, MatrixBox: the current selection
#define EVN_POST_CHANGE         52  // control has changed its content

#define EVN_DATA_INVALID        53  // control's data is invalid

// Only for List, and MatrixBox
// Param1 - Determined by component type
//          List:
//          MatrixBox: 
#define EVN_REQUEST_DATA    12  // Request data

//Only for Multisel
//Param1 - Determined by component type
//			Multisel: the current selection
#define EVN_REQUEST_STRING 13 // Request string

// Param1 - action code
// Param1 - action code
#define EVN_UNKNOWN_ACTION     61	//EVN_USER_ACTION

#define EVN_KEY_GOT            71
#define EVN_UNKNOWNKEY_GOT     72	//EVN_PASS_ACTION

#define EVN_MSG_GOT			   81	//A message gotted

//********** VEVENT parameter Definiton **********//

#define EVN_LIST_PAGE_PRE_CHANGE   82		// only for obj_objlist, to calculate the new focus when page changed(sometime the obj attr will be inactive, need re-load).


//********** PROC return Definiton **********//
#define	PROC_LOOP		0xFF
#define	PROC_PASS		0xFE
#define	PROC_LEAVE		0xFD

/////////////////////////////////////////////////////////
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


/////////////////////////////////////////////////////////////
//********** Object HEAD  Definiton **********//
/////////////////////////////////////////////////////////////



#define C_ALIGN_CENTER  0x00
#define C_ALIGN_LEFT    0x01
#define C_ALIGN_RIGHT   0x02
#define C_ALIGN_TOP     0x04
#define C_ALIGN_BOTTOM  0x08
#define C_ALIGN_VCENTER	0x0C
#define C_ALIGN_SCROLL	0x10



#define GET_HALIGN(_align)  ((UINT8)(_align) & 0x03)
#define GET_VALIGN(_align)  ((UINT8)(_align) & 0x0C)

#define C_FONT_DEFAULT  0
#define C_FONT_1		1
#define C_FONT_2		2
#define C_FONT_3		3
#define C_FONT_4		4
#define C_FONT_5		5

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
    OT_ANIMATION,
    OT_MAX
}OBJECT_TYPE;

#define 	C_ATTR_ACTIVE   	0x01
//#define	C_ATTR_HL		0x02
#define	C_ATTR_SELECT		0x03
#define 	C_ATTR_INACTIVE 	0x04
#define 	C_ATTR_HIDDEN   	0x05

typedef struct _COLOR_STYLE
{
    UINT8 bShowIdx;
    UINT8 bHLIdx;       // Style during HighLight
    UINT8 bSelIdx;      // Style during Selected, add by sunny 07/21, For Window Object, it's Close Attribute
    UINT8 bGrayIdx;     // Gray Style, It will be active while bAction == ACTION_GRAY. This Attribute is not included in Window Object
}COLOR_STYLE, *PCOLOR_STYLE;

#define PRESULT UINT8
#define VACTION UINT8
#define VEVENT  UINT32

typedef struct _OBJECT_HEAD     OBJECT_HEAD;
typedef struct _OBJECT_HEAD 	*POBJECT_HEAD;
typedef VACTION (*PFN_KEY_MAP)(POBJECT_HEAD pObj, UINT32 Key);
typedef PRESULT (*PFN_CALLBACK)(POBJECT_HEAD pObj, VEVENT Event, UINT32 Param1, UINT32 Param2);

struct _OBJECT_HEAD
{
    UINT8 bType;
    UINT8 bAttr;    // defined in form C_ATTR_XXX
    UINT8 bFont;    // defined in form C_FONT_XXX
    UINT8 bID;      // 0: for component without focus

    UINT8 bLeftID;
    UINT8 bRightID;
    UINT8 bUpID;
    UINT8 bDownID;

    OSD_RECT frame;
    COLOR_STYLE style;

    PFN_KEY_MAP pfnKeyMap;
    PFN_CALLBACK pfnCallback;
    
    POBJECT_HEAD pNext;
    POBJECT_HEAD pRoot;
};

/////////////////////////////////////////////////////////////
//********** some function return and parameter  Definiton **********//
/////////////////////////////////////////////////////////////

typedef struct tagVSCR_LIST
{
	VSCR    vscr;
	struct tagVSCR_LIST  *pNext;
	struct tagVSCR_LIST   *pCur;	
}VSCR_LIST,*PVSCR_LIST; 

/* update(draw) content */
// 0x0000 00xx
#define C_UPDATE_ALL        0x01
#define C_UPDATE_FOCUS      0x02
#define C_UPDATE_CONTENT    0x03
#define C_UPDATE_CURSOR     0x04

/*draw color style */
//0x0000 xx00
#define C_DRAW_TYPE_NORMAL      (0x01<<4)
#define C_DRAW_TYPE_HIGHLIGHT   (0x02<<4)
#define C_DRAW_TYPE_GRAY        (0x03<<4)
#define C_DRAW_TYPE_SELECT      (0x04<<4)
#define C_DRAW_TYPE_HIDE        (0x05<<4)

#define C_DRAW_SIGN_EVN_FLG     		0x80000000
#define C_CLOSE_CLRBACK_FLG			0x40000000	//only for OSD_Closeobject(...)


#define OSD_SetUpdateType(nCmdDraw,update_type)	\
	nCmdDraw = ( (nCmdDraw & 0xFFFFFFF0) | update_type)
#define OSD_GetUpdateType(nCmdDraw)	\
	((nCmdDraw) & 0x0F)
		
#define OSD_SetDrawType(nCmdDraw,drawType)	\
	nCmdDraw = ( (nCmdDraw & 0xFFFFFF0F) | drawType)
#define OSD_GetDrawType(nCmdDraw)	\
	( (nCmdDraw) & 0xF0)

/////////////////////////////////////////////////////////

#define	OSD_DIRDRAW			0
#define	OSD_REQBUF			1
#define OSD_GET_CORNER		2
#define OSD_GET_BACK      	4
	
/////////////////////////////////////////
//macro to access object header

#define OSD_SetColor(pObj, _bShowIdx, _bHLIdx, _bSelIdx, _bGrayIdx) \
    do{ \
        ((POBJECT_HEAD)(pObj))->style.bShowIdx = (UINT8)(_bShowIdx); \
        ((POBJECT_HEAD)(pObj))->style.bHLIdx = (UINT8)(_bHLIdx); \
        ((POBJECT_HEAD)(pObj))->style.bSelIdx = (UINT8)(_bSelIdx); \
        ((POBJECT_HEAD)(pObj))->style.bGrayIdx = (UINT8)(_bGrayIdx); \
    }while(0)
//PCOLOR_STYLE OSD_GetColor(POBJECT_HEAD pObj);
#define OSD_GetColor(pObj) (&(((POBJECT_HEAD)(pObj))->style))

//void OSD_SetID(POBJECT_HEAD pObj, UINT8 bID, UINT8 bLeftID, UINT8 bRightID, UINT8 bUpID, UINT8 bDownID);
#define OSD_SetID(pObj, _bID, _bLID, _bRID, _bUID, _bDID) \
    do{ \
        ((POBJECT_HEAD)(pObj))->bID = (UINT8)(_bID); \
        ((POBJECT_HEAD)(pObj))->bLeftID = (UINT8)(_bLID); \
        ((POBJECT_HEAD)(pObj))->bRightID = (UINT8)(_bRID); \
        ((POBJECT_HEAD)(pObj))->bUpID = (UINT8)(_bUID); \
        ((POBJECT_HEAD)(pObj))->bDownID = (UINT8)(_bDID); \
    }while(0)

//UINT8 OSD_GetObjID(POBJECT_HEAD pObj);
#define OSD_GetObjID(pObj)  ( ((POBJECT_HEAD)(pObj))->bID)

//void OSD_SetKeyMap(POBJECT_HEAD pObj, PFN_KEY_MAP pfnKeyMap);
#define OSD_SetKeyMap(pObj, _pfnKeyMap) \
    do{ \
        ((POBJECT_HEAD)(pObj))->pfnKeyMap = (PFN_KEY_MAP)(_pfnKeyMap); \
    }while(0)
//void OSD_SetCallback(POBJECT_HEAD pObj, PFN_CALLBACK pfnCallback);
#define OSD_SetCallback(pObj, _pfnCallback) \
    do{ \
        ((POBJECT_HEAD)(pObj))->pfnCallback = (PFN_CALLBACK)(_pfnCallback); \
    }while(0)

//BOOL OSD_CheckAttr(POBJECT_HEAD pObj, UINT8 bAttr)
#define OSD_CheckAttr(pObj, _bAttr) \
    (((POBJECT_HEAD)(pObj))->bAttr == (UINT8)(_bAttr))
//void OSD_SetAttr(POBJECT_HEAD pObj, UINT8 bAttr);
#define OSD_SetAttr(pObj, _bAttr) \
    do{((POBJECT_HEAD)(pObj))->bAttr = (UINT8)(_bAttr);}while(0)
UINT8 OSD_GetShowIndex(const PCOLOR_STYLE pColorStyle, UINT8 bAttr);

//void OSD_SetRect(POSD_RECT pRect, UINT16 left, UINT16 top, UINT16 width, UINT16 height);
#define OSD_SetRect(r, _l, _t, _w, _h) \
    do{ \
        ((POSD_RECT)(r))->uLeft = (UINT16)(_l); \
        ((POSD_RECT)(r))->uTop = (UINT16)(_t); \
        ((POSD_RECT)(r))->uWidth = (UINT16)(_w); \
        ((POSD_RECT)(r))->uHeight = (UINT16)(_h); \
    }while(0)

//void OSD_CopyRect(POSD_RECT pDest, POSD_RECT pSrc);
#define OSD_CopyRect(dest, src) \
    do{ \
        ((UINT32 *)(dest))[0] = ((UINT32 *)(src))[0]; \
        ((UINT32 *)(dest))[1] = ((UINT32 *)(src))[1]; \
    }while(0)
#define OSD_SetRect2    OSD_CopyRect

//void OSD_ZeroRect(POSD_RECT pRect)
#define OSD_ZeroRect(r) \
    do{ \
        ((UINT32 *)(r))[0] = 0; \
        ((UINT32 *)(r))[1] = 0; \
    }while(0);

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
	
//common get object operation w functions
#define OSD_SetOSDMessage(msg,msgtype,msgcode)	\
	((msg) = ((msgtype) << 16) | (msgcode))

#define OSD_GetOSDMessage(msgtype,msgcode)	\
	( ((msgtype) << 16) | (msgcode))
		
#define OSD_GetOSDMessageType(msg)	\
	((msg)>>16)
		
#define	OSD_GetOSDMessageCode(msg)	\
	((msg) & 0xFFFF)

#define OSD_SetObjFont(pObj,font) \
    do{ \
        ((POBJECT_HEAD)(pObj))->bFont = font; \
    }while(0)

#define OSD_GetObjFont(pObj)    (((POBJECT_HEAD)(pObj))->bFont)
    

//rectangle operation functions
BOOL OSD_RectInRect(const struct OSDRect* R,const struct OSDRect* r);
void OSD_GetRectsCross(const struct OSDRect *backR,const struct OSDRect *forgR, struct OSDRect* relativR);

//Virtual screen operation functions
void  OSD_UpdateVscr(VSCR* pVscr);
lpVSCR OSD_GetVscr(struct OSDRect* pRect, UINT32 bFlag);
lpVSCR OSD_DrawObjectFrame(struct OSDRect *pRect,UINT8 bStyleIdx);
#define OSD_SetVscrModified(pVscr)	\
	(pVscr)->updatePending = 1


//Get top root object, focus object ID, object by ID, focus object
POBJECT_HEAD OSD_GetTopRootObject(POBJECT_HEAD pObj);
UINT8 		 OSD_GetFocusID(POBJECT_HEAD pObj);
POBJECT_HEAD OSD_GetObject(POBJECT_HEAD pObj, UINT8 bID);
POBJECT_HEAD OSD_GetFocusObject(POBJECT_HEAD pObj);
UINT8 OSD_GetAdjacentObjID(POBJECT_HEAD pObj, VACTION action);
POBJECT_HEAD OSD_GetAdjacentObject(POBJECT_HEAD pObj, VACTION action);
PRESULT 	 OSD_ChangeFocus(POBJECT_HEAD pObj, UINT16 bNewFocusID,UINT32 parm);

//OSD object draw functions
void OSD_DrawObjectCell(POBJECT_HEAD pObj, UINT8 bStyleIdx,	UINT32 nCmdDraw);

//nCmdDraw : C_DRAW_SIGN_EVN_FLG | C_UPDATE_XXX
PRESULT OSD_DrawObject(POBJECT_HEAD pObj, UINT32 nCmdDraw );
PRESULT OSD_TrackObject(POBJECT_HEAD pObj,UINT32 nCmdDraw );
PRESULT OSD_SelObject(POBJECT_HEAD pObj,  UINT32 nCmdDraw );
void	OSD_HideObject(POBJECT_HEAD pObj, UINT32 nCmdDraw );
void    OSD_ClearObject(POBJECT_HEAD pObj, UINT32 nCmdDraw);

//call callback functions
PRESULT OSD_NotifyEvent(POBJECT_HEAD pObj, VEVENT Event, UINT32 Param1, UINT32 Param2);
#define OSD_SIGNAL	OSD_NotifyEvent


//Open / close a the menu with specifed object
PRESULT OSD_ObjOpen(POBJECT_HEAD pObj, UINT32 param);
PRESULT OSD_ObjClose(POBJECT_HEAD pObj,UINT32 param);

//object proc function
PRESULT OSD_ObjProc(POBJECT_HEAD pObj, UINT32 msgType,UINT32 msg,UINT32 param1);

#ifdef TXT_RUNNER_SUPPORT

struct RUNTXTMAP
{
   // struct _TEXT_FIELD *run_head;
  //  struct _TEXT_FIELD org_head;
    POBJECT_HEAD run_head;
    UINT8 run_obj_mode;	
    UINT8 offset_step;
    UINT16 offset_value;
    UINT16 total_strlen;
};

void Set_OSD_TXT_Runner_Obj(struct RUNTXTMAP  pObj1,struct RUNTXTMAP  pObj2,struct RUNTXTMAP pObj3);
 void Get_OSD_TXT_Runner_TextField_width(UINT8 index,struct OSDRect *pRect1);

void Start_TXT_Runner();
#endif


#define LIB_FONT					0x1000
#define LIB_STRING				0x2000
#define LIB_PALLETE				0x4000
#define LIB_ICON					0x8000
#define LIB_FONT_MASSCHAR		0x1F00

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
#include "obj_animation.h"

#include "osd_common_draw.h"

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
#ifndef _UNCOMPRESSED_ARRAY_METHOD
	BOOL	m_fFlag;			//flag to show the data being compressed,structure so----
#endif 
#ifdef BIDIRECTIONAL_OSD_STYLE
	BOOL	m_mirrorFlag;
#endif
}FONTICONPROP;

typedef struct tag_ObjectInfo
{
	UINT16	m_wObjClass;			// the class ID of FONT,STRING the object belongs to
									// the object ID of ICON,PALLETE
	FONTICONPROP 	m_objAttr;		// the attribute of this object
}OBJECTINFO,*lpOBJECTINFO;

typedef enum 
{
	GE_DRAW_MODE = 0,
	OSD_DRAW_MODE,
}OSD_DRAW_DEV_MODE;
	
#define	ID_RSC	UINT16
typedef	UINT16	(*OSD_GET_LANG_ENV)(void);
typedef	BOOL	(*OSD_GET_OBJ_INFO)(UINT16 wLibClass,UINT16 uIndex,lpOBJECTINFO pObjectInfo);
typedef	UINT8* 	(*OSD_GET_RSC_DATA)(UINT16 wLibClass,UINT16 uIndex,lpOBJECTINFO objectInfo);
typedef	UINT8* 	(*OSD_GET_THAI_FONT_DATA)(UINT16 wLibClass,struct thai_cell *cell,lpOBJECTINFO objectInfo);
typedef	BOOL   	(*OSD_REL_RSC_DATA)(UINT8* lpDataBuf,lpOBJECTINFO pObjectInfo);
typedef 	ID_RSC 	(*OSD_GET_FONT_LIB)(UINT16 uChar);
typedef 	PWINSTYLE (*OSD_GET_WIN_STYLE)(UINT8 bStyleIdx);
typedef 	ID_RSC    (*OSD_GET_STR_LIB_ID)(UINT16 EnvID);
typedef	UINT32 (*AP_HK_TO_VK)(UINT32 start, UINT32 key_info, UINT32* vkey );
#ifdef BIDIRECTIONAL_OSD_STYLE
typedef BOOL	(*OSD_GET_MIRROR_FLAG)(void);
#endif

struct OSD_RscInfo
{/*Split from old struct,to keep OSD_LIB maintanace,UI resource is unique*/
	OSD_GET_LANG_ENV	osd_get_lang_env;
	OSD_GET_OBJ_INFO		osd_get_obj_info;
	OSD_GET_RSC_DATA	osd_get_rsc_data;
	OSD_GET_THAI_FONT_DATA osd_get_thai_font_data;
	OSD_REL_RSC_DATA	osd_rel_rsc_data;
	OSD_GET_FONT_LIB 	osd_get_font_lib;
	OSD_GET_WIN_STYLE	osd_get_win_style;
	OSD_GET_STR_LIB_ID	osd_get_str_lib_id;
	AP_HK_TO_VK			ap_hk_to_vk;
#ifdef BIDIRECTIONAL_OSD_STYLE
	OSD_GET_MIRROR_FLAG	osd_get_mirror_flag;
#endif
};
struct OSD_RegionInfo	
{/*Inherit from OSD hld driver,eg. 29,29C serial*/
	HANDLE		osddev_handle;
	struct OSDPara	tOpenPara;
	struct OSDRect	osdrect;
	UINT8		region_id;
};

struct OSD_SurfaceInfo
{/*New build for Ge hld driver*/
	struct ge_device	*ge_dev;
	ge_surface_desc_t	*ge_dissf,*ge_virsf;
	ge_region_t		ge_region_para;
	UINT8			dissf_region_id;
	UINT8			virsf_region_id;
};

extern struct OSD_RscInfo	g_osd_rsc_info;
extern struct OSD_RegionInfo	g_osd_region_info;
extern struct OSD_SurfaceInfo	g_ge_surface_info;
extern PVSCR_LIST	 pVscrHead;
typedef void(*OSD_NotifyFunc)(POBJECT_HEAD pobj);
void OSD_RegOpenNotifyFunc(OSD_NotifyFunc func);
void OSD_RegCloseNotifyFunc(OSD_NotifyFunc func);
void OSD_RegionInit(struct OSD_RegionInfo *p_region_info,struct OSD_RscInfo *p_rsc_info);
void OSD_SurfaceInit(struct OSD_SurfaceInfo *p_surface_info,struct OSD_RscInfo *p_rsc_info);
OSD_DRAW_DEV_MODE OSD_GetDrawMode();
void OSD_SetDrawMode(OSD_DRAW_DEV_MODE mode_chg);
void OSD_SetWholeObjAttr(POBJECT_HEAD	pObj, UINT8 bAttr);

void OSD_SetPaletteBoardEx(UINT32 wPalIdx, UINT32 nColor);
void OSD_SetPaletteBoard(UINT32 wPalIdx, UINT32 nColor);

#endif//_OSD_LIB_H_
