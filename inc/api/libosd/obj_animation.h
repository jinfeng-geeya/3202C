/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2007 Copyright (C)
 *
 *  File: obj_animation.h
 *
 *  Description: the struct of animation object
 *
 *  History:
 *  Date            Author  		Version  Comment
 *  ====            ======  		=======  =======
 *  1.  2007.11.16   Sharon Zhang 	0.1.000  Initial
 *
 ****************************************************************************/
#ifndef _OBJ_ANIMATION_H_
#define _OBJ_ANIMATION_H_

/////////////////	ANIMATION 	////////////////////////////////
#define OSD_ANM_REFRESH			0

typedef void(*anm_callback_func)(unsigned long type, unsigned long param);

typedef struct _ANIMATION_{
	OBJECT_HEAD		head;
    UINT8 bAlign;
    UINT8 bX;
    UINT8 bY;
	UINT8 bMovingFlag; //1:move, 0:still
    UINT16 *pFrame;	//pointer of frameID array
    UINT8 bCurFrameNum;	//0~(bAllFrameNum-1)
    UINT8 bAllFrameNum;
	UINT32 dwInterval;	//bFPS;		//frame per second
	ID TimerID;
	anm_callback_func anm_callback;
} ANIMATION,*PANIMATION;


#define OSD_Animation_SetMoving(pCtrl) \
 	(pCtrl)->bMovingFlag = TRUE

#define OSD_Animation_ClearMoving(pCtrl) \
 	(pCtrl)->bMovingFlag = FALSE
   
#define OSD_Animation_SetCurFrame(pCtrl, bCurFrame) \
 	(pCtrl)->bCurFrameNum = (bCurFrame)

void OSD_Animation_Start_Timer(PANIMATION pCtrl);
void OSD_Animation_Stop_Timer(PANIMATION pCtrl);

//draw animation
void OSD_DrawAnimationCell(PANIMATION pCtrl,UINT8 bStyleIdx,UINT32 nCmdDraw);

//key map
VACTION OSD_AnimationKeyMap(POBJECT_HEAD pObj,	UINT32 vkey);

//proc
PRESULT OSD_AnimationProc(POBJECT_HEAD pObj, UINT32 msg_type, UINT32 msg,UINT32 param1);

#endif//_OBJ_ANIMATIN_H_


