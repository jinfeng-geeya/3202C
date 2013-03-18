/*-----------------------------------------------------------------------------
*
*	Copyright (C) 2005 ALI Corp. All rights reserved.
*
*	File: obj_list.h
*
*	Content: 
*		define the struct of list
*	History: 
*		2005/12/29 by eric
*		Created this file. Inherited from new osd lib
*-----------------------------------------------------------------------------*/
#ifndef  _OBJ_LIST_H_
#define _OBJ_LIST_H_

#include <basic_types.h>
#include <api/libosd/osd_lib.h>

#define LIST_NOSCROLL		0x0000
#define LIST_SCROLL			0x0040

#define LIST_NOGRID			0x0000
#define LIST_GRID			0x0080

#define LIST_PAGE_NOKEEP_CURITEM	0x0000
#define LIST_PAGE_KEEP_CURITEM	0x0200

#define LIST_NOFULL_PAGE	0x0000
#define LIST_FULL_PAGE		0x0400

enum
{
    FIELD_IS_ICON, 
    FIELD_IS_TEXT, 
    FIELD_IS_STRING_ID, 
    FIELD_IS_COUNT, 
    FIELD_IS_NUMBER
};

/////////////////	LIST 	////////////////////////////////
typedef struct tag_ListField{
	UINT16		nWidth;
	UINT8		bIntervalT;
	UINT8		bIntervalL;	
	UINT8		bSHStyle;		
	UINT8		bHLStyle;		
	UINT8 		bAssignType;
	UINT8		bType;			// 0: Icon, 1: Text, 2: Number Acount
//	ID_RSC		RscId;		// Icon Lib
	void* 		pContent; 
}LISTFIELD,*lpLISTFIELD;

typedef struct {
OBJECT_HEAD	Head;
UINT16			bListStyle;		
lpLISTFIELD		pFieldTable;
UINT8			bNumField;    // Field Number
UINT8			bPage;        // page size
SCROLL_BAR 	*	scrollBar;
INT8				bIntervalY;   // Interval Height between list items.
UINT32 *			item_status;   // 2bit for an item, 11=sel,10=hide,00=normal
UINT16			nCount;
UINT16			nTop;
UINT16			nSel;
UINT16			nPos;
}LIST, *PLIST;

/////////////////////////////////////////////////
// attribute set functions
#define OSD_SetListSelectArray(l,dwselectarray)	\
	(l)->item_status = dwselectarray		
//public interface added by Xian_Zhou
#define OSD_SetListCurPos(l,pos)	\
	(l)->nPos =(UINT16) (pos)
#define OSD_SetListTop(l,top)	\
	(l)->nTop =(UINT16) (top)

VACTION OSD_ListKeyMap(POBJECT_HEAD pObj, UINT32 vkey);
PRESULT OSD_ListProc(POBJECT_HEAD pObjHead, UINT32 msg_type, UINT32 msg,UINT32 param1);
void OSD_DrawListCell(LIST *l, UINT8 bStyleIdx, UINT32 nCmdDraw);
UINT16 OSD_GetListTopPoint(LIST *l);
void OSD_SetListTopPoint(LIST *l, UINT16 wTop);
void OSD_SetListContent(LIST * l, UINT8 bFieldN, lpLISTFIELD pListField);
void OSD_SetListStyle(LIST *l, UINT16  bListStyle);
void OSD_SetListVerScrollBar(LIST * l, SCROLL_BAR* sb);
void OSD_SetListCurPoint(LIST *l, UINT16 cur_pos);
UINT16 OSD_GetListCurPoint(LIST *l);
PRESULT  OSD_SetListCurSel(LIST *l, UINT16 uSelPoint);
void OSD_SetListCurHide(LIST *l, UINT16 uHidePoint);
BOOL OSD_CheckListSelect(LIST *l, UINT16 wSelectIdx);
UINT16 OSD_GetListPageSize(LIST *l);
void OSD_SetListPageSize(LIST *l, UINT8 cur_page);
UINT16 OSD_GetListCount(LIST *l);
void OSD_SetListCount(LIST *l, UINT16 count);
PRESULT OSD_ListMoveCursor(PLIST l, INT16 nStep);

#endif
