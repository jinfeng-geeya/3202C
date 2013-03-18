#include <types.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
//#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"

#include "osd_config.h"
#include "string.id"
#include "images.id"
#include "win_pause.h"


/*******************************************************************************
 * Objects definition
 *******************************************************************************/
extern CONTAINER g_win_volume;
extern BITMAP win_pause_bmp;

static BOOL cur_pause_state = UNPAUSE_STATE;
#ifndef USE_LIB_GE
static UINT8 *pause_back_buf = NULL;
static UINT32 pause_back_buf_size;
static UINT8 *pbuf_pause = NULL;
static UINT8 pause_save_back =0;
static VSCR pause_vscr;
#endif
DEF_BITMAP(win_pause_bmp, NULL, NULL, C_ATTR_ACTIVE, 0,  \
	1, 0, 0, 0, 0, PAUSE_L, PAUSE_T, PAUSE_W, PAUSE_H, WSTL_TRANS_IX, WSTL_TRANS_IX, 0, 0,  \
	NULL, NULL,  \
	C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, PAUSE_ICON)

/*******************************************************************************
 * Function definition
 *******************************************************************************/
static BOOL backup_back();
static void restore_back();


BOOL GetPauseState()
{
	return cur_pause_state;
}

void ShowPauseOSDOnOff(UINT8 flag)
{
	OBJECT_HEAD *obj;
	obj = (OBJECT_HEAD*) &win_pause_bmp;

	if(getVolumeState()&&flag)
    {
		OSD_ObjClose((POBJECT_HEAD)&g_win_volume, C_CLOSE_CLRBACK_FLG);
		menu_stack_pop();
	}

	if (flag)
	{
		backup_back();
		OSD_DrawObject(obj, C_UPDATE_ALL);
	}
	else
	{
		OSD_ClearObject(obj, 0);
		restore_back();
	}
}


void ShowPauseOnOff(void)
{
	ShowPauseOSDOnOff(cur_pause_state == PAUSE_STATE);
}

void SetPauseOnOff(BOOL showOsd)
{

	if (cur_pause_state == UNPAUSE_STATE)
	{
		cur_pause_state = PAUSE_STATE;
		api_video_set_pause(1);
	}
	else
	{
		cur_pause_state = UNPAUSE_STATE;
		api_video_set_pause(0);
	}

	if (showOsd)
		ShowPauseOnOff();
}

void GetPauseRect(OSD_RECT *rect)
{
	POBJECT_HEAD obj;

	obj = (POBJECT_HEAD) &win_pause_bmp;
	*rect = obj->frame;
}

void SetPauseStatus(BOOL status)
{
	cur_pause_state = status;
}



static BOOL backup_back()
{
#ifndef USE_LIB_GE
	CONTAINER *win; 
	UINT32 vscr_size;
	struct OSDRect *frame=NULL;
	VSCR *lpVscr;
	
	/*if(*pbuf!=NULL)
		FREE(*pbuf);
	*pbuf = NULL;*/
	
	win = (CONTAINER *)&win_pause_bmp;//&win_mute_on_title;
	frame = &win->head.frame;
    
	lpVscr = OSD_GetTaskVscr(osal_task_get_current_id());
	OSD_UpdateVscr(lpVscr);
	lpVscr->lpbScr = NULL;

	vscr_size = (((frame->uWidth) >> FACTOR) + 1) *frame->uHeight;

	if (pause_back_buf != NULL && pause_back_buf_size >= vscr_size)
		pbuf_pause = pause_back_buf;
	else
	{
		if (pause_back_buf != NULL)
			FREE(pause_back_buf);
		pbuf_pause = (UINT8*)MALLOC(vscr_size);
		pause_back_buf = pbuf_pause;
		if (pause_back_buf != NULL)
			pause_back_buf_size = vscr_size;
	}
//to avoid the Pause bmp showing when changing program
/*
	if (pbuf_pause != NULL)
		pause_save_back = 1;
	else
		pause_save_back = 0;

	if (pause_save_back)
	{
		OSD_SetRect2(&pause_vscr.vR, frame);
		pause_vscr.lpbScr = pbuf_pause;
		OSD_GetRegionData(&pause_vscr, &pause_vscr.vR);
		return TRUE;
	}
	else
		return FALSE;
*/
	if (pause_save_back == 0)
	{
	    pause_save_back =1;
        
		OSD_SetRect2(&pause_vscr.vR, frame);
		pause_vscr.lpbScr = pbuf_pause;
		OSD_GetRegionData(&pause_vscr, &pause_vscr.vR);
		return TRUE;
	}
	else
		return FALSE;
#endif
}


static void restore_back()
{
#ifndef USE_LIB_GE
	if (pause_save_back)
	{
		OSD_RegionWrite(&pause_vscr, &pause_vscr.vR);
		if (pause_back_buf != NULL)
		{
			FREE(pause_back_buf);
			pause_back_buf = NULL;
			pause_save_back = 0;
		}
	}
#endif
}




