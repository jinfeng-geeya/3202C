/*-----------------------------------------------------------------------------
*	Copyright (C) 2010 ALI Corp. All rights reserved.
*	GUI object library with GE HW accerlaration by Shine Zhou.
*-----------------------------------------------------------------------------*/
#ifndef _OBJ_CONTAINER_H_
#define _OBJ_CONTAINER_H_

typedef struct _CONTAINER_{
	OBJECT_HEAD		head;
	POBJECT_HEAD	pNextInCntn;		//point to objects included in this container; container<-obj1<-obj2
	UINT8			FocusObjectID;		//which object in container which is focused
	UINT8			bHiliteAsWhole;		//when hl is on container,wheather to highlight all objects included in this container
	UINT32			slave;
} CONTAINER,*PCONTAINER;

typedef struct SLAVE_CONTAINER_{
	GUI_RECT  frame;
	COLOR_STYLE style;
} SDCON,*PSDCON;

void OSD_DrawContainerCell(PCONTAINER pCtrl,UINT8 bStyleIdx,UINT32 nCmdDraw);

//key map
VACTION OSD_ContainerKeyMap(POBJECT_HEAD pObj,	UINT32 vkey);

//proc
PRESULT OSD_ContainerProc(POBJECT_HEAD pObj, UINT32 msg_type, UINT32 msg,UINT32 param1);

PRESULT OSD_ContainerChgFocus(CONTAINER	*c, UINT8 bNewFocusID,UINT32 parm);

//set contianer foucus
#define OSD_SetContainerFocus(c, id)	\
	(c)->FocusObjectID = (id)
//get focus id
#define OSD_GetContainerFocus(c)	\
	((c)->FocusObjectID)

#define OSD_GetContainerNextObj(c)	\
	(c)->pNextInCntn

#define OSD_SetContainerNextObj(c,pNextInCont)	\
	(c)->pNextInCntn = (POBJECT_HEAD)pNextInCont;

#endif//_OBJ_CONTAINER_H_

