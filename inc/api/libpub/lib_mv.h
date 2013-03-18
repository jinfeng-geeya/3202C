#ifndef __MID27_MULTIVIEW_HEAD_FILE_H__
#define __MID27_MULTIVIEW_HEAD_FILE_H__

#include"lib_frontend.h"

#define BOX_FINISHED	        0x100
#define BOX_FOCUS		0x200
#define BOX_SCAN_START          0x1000
#define BOX_PLAY_ABORT		0xFFFF
#define BOX_PLAY_NORMAL	        0x0001
#define MV_SCAN_BOX		0x40
#define MV_SCAN_SCREEN		0x20
#define MV_ACTIVE		0x10
#define MV_IDLE			0x0

#define MV_BOX_NUM		25 //max number of boxes in one frame.

enum MVMode{
	MV_2_BOX=0,
	MV_4_BOX,
	MV_9_BOX,
	MV_16_BOX,
	MV_25_BOX,
	MV_AUTO_BOX
};

typedef unsigned long (* MVCallback)(UINT16 uIndex);

struct MVInitPara
{
	struct RectSize	tMpSize,tPipSize;
	UINT8 			uBoxNum;//total number of boxes in the whole picture;
	struct Position	puBoxPosi[MV_BOX_NUM];//pointer of the coordination array for all boxes;(placement sequence index is left to right, up to down)
	MVCallback	callback;
	struct Rect 	PlayWin;
};

struct mv_param
{
	struct cc_es_info		es;
	struct cc_device_list	device_list; 
	struct ft_frontend		ft;
};

struct MultiViewPara
{
	UINT8 	uBoxActiveIdx;//active box index
	UINT8 	uBoxNum;//total number of boxes in the whole picture;
	UINT16	uProgInfo[MV_BOX_NUM];
	struct mv_param mv_param[MV_BOX_NUM];
};


void mv_init(void);

void UIMultiviewInit(struct MVInitPara *tMVinitPara, enum MVMode mvMode);
void UIMultiviewEnter(struct MultiViewPara *mvPara);
BOOL UIMultiviewDrawOneBox(UINT16 uActiveIndex);//, UINT8 bEnableSanDraw);
void UIMultiviewExit();
UINT8 UIMultiviewGetStatus(void);

#endif

