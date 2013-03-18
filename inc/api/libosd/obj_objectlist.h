#ifndef _OBJ_OBJECTLIST_H_
#define _OBJ_OBJECTLIST_H_

#define LIST_HOR			0x0000
#define LIST_VER 			0x0001

#define LIST_NO_SLECT		0x0000
#define LIST_SINGLE_SLECT 	0x0002
#define LIST_MULTI_SLECT 	0x0004


//When list is not the focus object in the window
#define LIST_NOKEEP_FOUCS	0x0000
#define LIST_KEEP_FOUCS		0x0008

#define LIST_NOKEEP_SELECT	0x0000
#define LIST_KEEP_SELECT	0x0010

#define LIST_ITEMS_NOCOMPLETE	0x0000
#define LIST_ITEMS_COMPLETE		0x0020

#define LIST_NOSCROLL		0x0000
#define LIST_SCROLL			0x0040

#define LIST_NOGRID			0x0000
#define LIST_GRID			0x0080

#define LIST_FOCUS_FIRST    0x0000
#define LIST_SELECT_FIRST   0x0100

#define LIST_PAGE_NOKEEP_CURITEM  0x0000
#define LIST_PAGE_KEEP_CURITEM    0x0200

#define LIST_NOFULL_PAGE    0x0000
#define LIST_FULL_PAGE      0x0400

#define LIST_CLIP_ENABLE    0x0800
#define LIST_FIX_PAGE       0x1000

#define LIST_BARSCROLL_NOCOMPLETE	0x0000
#define LIST_BARSCROLL_COMPLETE	0x2000

#define LIST_INVALID_SEL_IDX	0xFFFF


typedef struct {
	OBJECT_HEAD	head;
				
	OBJECT_HEAD**	pListField;
	/*
		scrool bar
	*/
	SCROLL_BAR 		*scrollBar;
	
//	UINT8		bFlashCtrl;			// It's for private use in Draw/Track func to check if required flash all data 
	
	UINT16		bListStyle;	
	/*
	wDep: 			a page of list's count
	wCount:			
			When LIST_ITEMS_COMPLETE:	count of objects in pListField
			else(LIST_ITEMS_NOCOMPLETE)	count of objects "in" list
	*/
	
	UINT16		wDep, wCount;
	/*wTop:		the first item of the page "in" the list
	
	When move focus in the list
			wCurPoint	the old focus item index
			wNewPoint	the new foucs item index to be to moved to 
	*/
	
	UINT16		wTop,wCurPoint, wNewPoint;

	

	/*
		wSelect : When LIST_SINGLE_SLECT,select item index 
		dwSelect: When LIST_MULTI_SLECT, select items
	*/
	UINT16      wSelect;
	UINT32*		dwSelect;

	/*
		Select mark object
	*/	
	OBJECT_HEAD*	pSelMarkObj;
				
} OBJLIST,*POBJLIST;


/////////////////////////////////////////////////
// attribute set functions
#define OSD_SetObjListStyle(ol,style)	\
	(ol)->bListStyle = (UINT16)(style)

#define OSD_SetObjListTop(ol,top)	\
	(ol)->wTop = (UINT16)(top)
	
#define OSD_SetObjListPage(ol,page)	\
	(ol)->wDep = (UINT16)(page)
		
#define OSD_SetObjListCount(ol,count)	\
	(ol)->wCount = (UINT16)(count)

#define OSD_SetObjListCurPoint(ol,curpoint)	\
	(ol)->wCurPoint = (UINT16)(curpoint)
	
#define OSD_SetObjListNewPoint(ol,newpoint)	\
	(ol)->wNewPoint = (UINT16)(newpoint)	
	
#define OSD_SetObjListSingleSelect(ol,select)	\
	(ol)->wSelect = (UINT16)(select)
	
#define OSD_SetObjListMultiSelectArray(ol,dwselectarray)	\
	(ol)->dwSelect = dwselectarray		
	
#define OSD_SetObjListContent(ol,listfield)	\
	(ol)->pListField = (OBJECT_HEAD**)(listfield)
	
/////////////////////////////////////////////////
// attribute get functions
#define OSD_GetObjListStyle(ol)	\
	(ol)->bListStyle
		
#define  OSD_GetObjListTop(ol)	\
	(ol)->wTop
	
#define OSD_GetObjListPage(ol)	\
	(ol)->wDep
			
#define OSD_GetObjListCount(ol)	\
	(ol)->wCount
	
#define OSD_GetObjListCurPoint(ol)	\
	(ol)->wCurPoint
	
#define OSD_GetObjListNewPoint(ol)	\
	(ol)->wNewPoint
	
#define OSD_GetObjListSingleSelect(ol)	\
	(ol)->wSelect

UINT16 	OSD_GetObjListMultiSelItemNum(OBJLIST *ol);
BOOL 	OSD_CheckObjListMultiSelect(OBJLIST *ol, UINT16 wSelectIdx);
void 	OSD_SwitchObjListMultiSelect(OBJLIST *ol, UINT16 wSwitchIdx);


void    OSD_SetObjListVerScrollBar(OBJLIST * ol, SCROLL_BAR* sb);
PRESULT OSD_ObjListChgFocus(OBJLIST *ol, UINT16 newItem,UINT32 parm);
PRESULT OSD_ObjListChgTopFocus(OBJLIST *ol, UINT16 newTop, UINT16 newPoint, UINT32 parm);

void    OSD_SetObjListAspect(OBJLIST *ol,UINT16 cnt, UINT16 page,UINT16 top,  UINT16 focus);

void 	OSD_ObjListDelItemUpdateSelect(OBJLIST *ol, UINT16 item);

void OSD_DrawObjectListCell(OBJLIST *ol,UINT8 bStyleIdx,UINT32 nCmdDraw);
VACTION OSD_ObjListDefMap(OBJECT_HEAD* ObjHead, UINT32 dInput);
PRESULT OSD_ObjectListProc(POBJECT_HEAD pObj, UINT32 msg_type, UINT32 msg,UINT32 param1);

#endif//_OBJ_OBJECTLIST_H_
