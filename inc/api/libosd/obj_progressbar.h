/*-----------------------------------------------------------------------------
*
*	Copyright (C) 2005 ALI Corp. All rights reserved.
*
*	File: obj_progressbar.h
*
*	Content: 
*		define the struct of progressbar
*	History: 
*		2005/12/30 by benjamin
*		Created this file. Inherited from new osd lib
*-----------------------------------------------------------------------------*/
#ifndef  _OBJ_PROGRESSBAR_H_
#define _OBJ_PROGRESSBAR_H_

#include <basic_types.h>
#include <api/libosd/osd_lib.h>


#define PROGRESSBAR_HORI_NORMAL		1
#define PROGRESSBAR_HORI_REVERSE 	2
#define PROGRESSBAR_VERT_NORMAL		4
#define PROGRESSBAR_VERT_REVERSE	8
#define	PBAR_STYLE_RECT_STYLE		0x80

typedef struct _PROGRESS_BAR
{
	OBJECT_HEAD 	head;
	UINT8 			bStyle;    	// Horizontal or Vertical,and the 
	UINT8 			bX;   		// interval width between tick for hori progress
	UINT8 			bY;   		// interval height between tick for vert progress
	UINT16 			wTickBg;     	// color style,rect area
	UINT16 			wTickFg;     	// same as above,tick area
	OSD_RECT 		rcBar;
	INT16 			nMin;
	INT16 			nMax;
	INT16 			nBlocks;
	INT16 			nPos;
}PROGRESS_BAR, *PPROGRESS_BAR;

//private function
#define GetProgressBarStyle(b)	\
	((b)->bStyle)


//public
#define OSD_SetProgressBarPos(b,n)	\
	(b)->nPos = n;
#define OSD_SetProgressBarStep(b,n)	\
	(b)->nStep = n;
#define OSD_GetProgressBarPos(b)	\
	(b)->nPos

void OSD_SetProgressBarTick(PPROGRESS_BAR b, UINT16 wTickBg, UINT16 wTickFg);
void OSD_SetProgressBarRange(PPROGRESS_BAR b, INT16 nMin, INT16 nMax);
BOOL OSD_SetProgressBarValue(PPROGRESS_BAR b, INT16 value);


void OSD_DrawProgressBarCell(PPROGRESS_BAR b,UINT8 bStyleIdx,UINT32 nCmdDraw);
//key map
VACTION OSD_ProgressBarKeyMap(POBJECT_HEAD pObj,	UINT32 vkey);

//proc
PRESULT OSD_ProgressBarProc(POBJECT_HEAD pObj, UINT32 msg_type, UINT32 msg,UINT32 param1);

#endif

