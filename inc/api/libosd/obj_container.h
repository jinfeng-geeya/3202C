/*-----------------------------------------------------------------------------
*
*	Copyright (C) 2005 ALI Corp. All rights reserved.
*
*	File: obj_container.h
*
*	Content: 
*		define the struct of container
*	History: 
*		2005/12/29 by benjamin
*		Created this file. Inherited from new osd lib
*-----------------------------------------------------------------------------*/
#ifndef _OBJ_CONTAINER_H_
#define _OBJ_CONTAINER_H_
typedef enum
{
    DRAW_STYLE_ONE = 0,
    DRAW_STYLE_FADE,
    DRAW_STYLE_SPHERE,
    DRAW_STYLE_ROTATE,
    DRAW_STYLE_SLIDE,
    DRAW_STYLE_ZOOM,
    DRAW_STYLE_FLIP,
    DRAW_STYLE_MOVE
}DRAW_STYLE;
/////////////////	CONTAINER 	////////////////////////////////
typedef struct _CONTAINER_{
	OBJECT_HEAD		head;
	POBJECT_HEAD	pNextInCntn;		//point to objects included in this container; container<-obj1<-obj2
	UINT8			FocusObjectID;		//which object in container which is focused
	UINT8			bHiliteAsWhole;		//when hl is on container,wheather to highlight all objects included in this container
//    UINT8           effect_type;      //Alan extend bHiliteAsWhole high 6bit using 3D OSD effect    
//    DRAW_STYLE      effect_type;
} CONTAINER,*PCONTAINER;


//draw container
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

