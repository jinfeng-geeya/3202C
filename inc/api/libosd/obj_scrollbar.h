/*-----------------------------------------------------------------------------
*
*	Copyright (C) 2005 ALI Corp. All rights reserved.
*
*	File: obj_scroll.h
*
*	Content: 
*		define the struct of progressbar
*	History: 
*		2005/12/30 by benjamin
*		Created this file. Inherited from new osd lib
*-----------------------------------------------------------------------------*/
#ifndef  _OBJ_SCROLLBAR_H_
#define  _OBJ_SCROLLBAR_H_


#define 	BAR_HORI_ORIGINAL	1	//only support PIC.thumb is a pic,with the original pic's width&height
#define 	BAR_HORI_AUTO		2	//resize the thumb's width&height to fit the rect area
#define 	BAR_VERT_ORIGINAL 	4	//only support PIC.
#define 	BAR_VERT_AUTO		8
#define	SBAR_STYLE_RECT_STYLE		0x80
#define	BAR_SCROLL_COMPLETE	0x10
#define BAR_THUMB_RECT_SUPPORT_OVERFLOW   0x20

typedef struct _SCROLL_BAR
{
	OBJECT_HEAD 	head;
	UINT8 			bStyle;   // Horizontal or Vertical
	UINT8			bPage;
	UINT16 			wThumbID;    //Thumb area's style
	UINT16 			wTickBg;	//rect area's style
	OSD_RECT 		rcBar;
	UINT16 			nMax;
	UINT16 			nPos;
}SCROLL_BAR, *PSCROLL_BAR;


#define OSD_SetScrollBarStyle(b,s)	\
	(b)->bStyle=s

#define GetScrollBarStyle(b)	\
	(b)->bStyle


#define OSD_SetScrollBarPage(b,n)\
	(b)->bPage = n
#define OSD_SetScrollBarMax(b,n)	\
	(b)->nMax = n
#define OSD_SetScrollBarPos(b,n)	\
	(b)->nPos = n



void OSD_DrawScrollBarCell(PSCROLL_BAR b,UINT8 bStyleIdx,UINT32 nCmdDraw);
//key map
VACTION OSD_ScrollBarKeyMap(POBJECT_HEAD pObj,	UINT32 vkey);

//proc
PRESULT OSD_ScrollBarProc(POBJECT_HEAD pObj, UINT32 msg_type, UINT32 msg,UINT32 param1);

#endif

