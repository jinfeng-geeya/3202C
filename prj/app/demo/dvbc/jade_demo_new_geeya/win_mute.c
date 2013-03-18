#include <types.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
//#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"

#include "osd_config.h"
#include "string.id"
#include "images.id"

#include "win_com.h"
#include "win_mute.h"


/*******************************************************************************
 * WINDOW's objects declaration
 *******************************************************************************/

extern CONTAINER g_win_volume;
extern BITMAP win_mute_bmp;

BOOL cur_mute_state = UNMUTE_STATE;
static UINT8 *pbuf = NULL;

/*******************************************************************************
 * Objects definition
 *******************************************************************************/

DEF_BITMAP(win_mute_bmp, NULL, NULL, C_ATTR_ACTIVE, 0,  \
	1, 0, 0, 0, 0, MUTE_L, MUTE_T, MUTE_W, MUTE_H, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, 0, 0,  \
	NULL, NULL,  \
	C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, MUTE_ICON)
#ifndef USE_LIB_GE
static UINT8 mute_save_back =0;

static UINT8 *mute_back_buf = NULL;
static UINT32 mute_back_buf_size;
static POBJECT_HEAD top_menu = NULL;
static VSCR mute_vscr;
//static uint8 back_ground[(MUTE_W+1)*MUTE_H];

extern UINT8 *osd_vscr_buffer[];
#endif

/*******************************************************************************
 * Function definition
 *******************************************************************************/

/***********************************************/

BOOL GetMuteState()
{
	return cur_mute_state;
}

void MuteOSDClear()
{
	OBJECT_HEAD *obj;
	obj = (OBJECT_HEAD*) &win_mute_bmp;
	OSD_ClearObject(obj, 0);
}

void ShowMuteOSDOnOff(UINT8 flag)
{
	OBJECT_HEAD *obj;
	obj = (OBJECT_HEAD*) &win_mute_bmp;
	if(getVolumeState()&&flag)
    {
		OSD_ObjClose((POBJECT_HEAD)&g_win_volume, C_CLOSE_CLRBACK_FLG);
		menu_stack_pop();
		//setVolumeOn(FALSE);
		//ap_send_msg(CTRL_MSG_SUBTYPE_CMD_EXIT_VOLUME, 1, TRUE);
	}
    if (flag)
    {
		backup_back();
		OSD_DrawObject(obj, C_UPDATE_ALL);
	}
    else
    {
		OSD_ClearObject(obj, C_UPDATE_ALL);
		restore_back();
	}
}

void ShowMuteOnOff(void)
{
	ShowMuteOSDOnOff(cur_mute_state == MUTE_STATE);
}

void SetMuteOnOff(BOOL showOsd)
{
	SYSTEM_DATA *sys_data;
	sys_data = sys_data_get();
	P_NODE p_node;
	UINT16 channel;
	
	channel= sys_data_get_cur_group_cur_mode_channel();
	
//	get_prog_at(channel, &p_node);
	
	if (cur_mute_state == UNMUTE_STATE)
		cur_mute_state = MUTE_STATE;
	else
		cur_mute_state = UNMUTE_STATE;

#ifdef AUDIO_MUTE_SAVE
	sys_data->bMuteSate = cur_mute_state;
#endif
#ifdef MIS_AD
	Mis_Set_Mute_Flag(cur_mute_state);
#endif
	api_audio_set_mute(cur_mute_state);
	if(!cur_mute_state)
//		api_audio_set_volume(p_node.audio_volume);
        api_audio_set_volume(system_config.volume);
	
	if (showOsd)
		ShowMuteOnOff();
}

void GetMuteRect(OSD_RECT *rect)
{
	POBJECT_HEAD obj;

	obj = (POBJECT_HEAD) &win_mute_bmp;
	*rect = obj->frame;
}
void SetMuteState(BOOL state)
{
	cur_mute_state = state;
}
/*this is used in menu background*/
void mute_onoff_onmenu()
{
	/*BITMAP * bmp = &win_mute_on_title;

	if(GetMuteState())
	{		
		//restore_back();
		//bmp->wIconID = 0;
		//OSD_DrawObject((POBJECT_HEAD)&win_mute_on_title, C_UPDATE_ALL);
		//SetMuteState(FALSE);	
		SetMuteNVODOnOff(TRUE);	
		return;
	}

	if(menu_stack_get_top()!=top_menu)
	{
		top_menu = menu_stack_get_top();
		if(!backup_back())
			libc_printf("BACK UP FAILED %s%d\n", __FILE__, __LINE__);
	}*/

	//bmp->wIconID = IM_063MUTE;
	//OSD_DrawObject((POBJECT_HEAD)&win_mute_on_title, C_UPDATE_ALL);
	//SetMuteState(TRUE);
	SetMuteOnOff(TRUE);//SetMuteNVODOnOff(TRUE);//

}

BOOL backup_back()
{
	CONTAINER *win;  
#ifndef USE_LIB_GE
	UINT32 vscr_size;
	struct OSDRect *frame=NULL;
	VSCR *lpVscr;
	
	/*if(*pbuf!=NULL)
		FREE(*pbuf);
	*pbuf = NULL;*/
	
	win = (CONTAINER *)&win_mute_bmp;//&win_mute_on_title; 
	frame = &win->head.frame; 
	
	lpVscr = OSD_GetTaskVscr(osal_task_get_current_id());
	OSD_UpdateVscr(lpVscr);
	lpVscr->lpbScr = NULL;

	vscr_size = (((frame->uWidth) >> FACTOR) + 1) *frame->uHeight;
	/*if (vscr_size < OSD_VSRC_MEM_MAX_SIZE / 2)
	{
		//pbuf = (UINT8*)MALLOC(vscr_size);
		//MEMCPY(&pbuf,&osd_vscr_buffer[osal_task_get_current_id() - 1][vscr_size],vscr_size);
		pbuf = &osd_vscr_buffer[osal_task_get_current_id() - 1][vscr_size];
		mute_save_back = 1;
	}
	else
	{*/
	if (mute_back_buf != NULL && mute_back_buf_size >= vscr_size)
		pbuf = mute_back_buf;
	else
	{
		if (mute_back_buf != NULL)
			FREE(mute_back_buf);
		pbuf = (UINT8*)MALLOC(vscr_size);
		mute_back_buf = pbuf;
		if (mute_back_buf != NULL)
			mute_back_buf_size = vscr_size;
	}
//There is something wrong with the function, 
//when changing the Mute state betteen the preview window and video widow 
/*    
	if (pbuf != NULL)
		mute_save_back = 1;
	else
		mute_save_back = 0;
	//}
	if (mute_save_back )
	{
		OSD_SetRect2(&mute_vscr.vR, frame);
		mute_vscr.lpbScr = pbuf;
		OSD_GetRegionData(&mute_vscr, &mute_vscr.vR);
		return TRUE;
	}
	else
		return FALSE;
*/

	if (mute_save_back == 0 )
	{
	    mute_save_back = 1;
		OSD_SetRect2(&mute_vscr.vR, frame);
		mute_vscr.lpbScr = pbuf;
		OSD_GetRegionData(&mute_vscr, &mute_vscr.vR);
		return TRUE;
	}
	else
		return FALSE;
#endif
}


void restore_back()
{
#ifndef USE_LIB_GE
	if (mute_save_back)
	{
		OSD_RegionWrite(&mute_vscr, &mute_vscr.vR);
		if (mute_back_buf != NULL)
		{
			FREE(mute_back_buf);
			mute_back_buf = NULL;
			mute_save_back = 0;
		}
	}
 #endif
}


void SetMuteOnOff_ext(BOOL showOsd)
{
	if (cur_mute_state == UNMUTE_STATE)
		cur_mute_state = MUTE_STATE;
	else
		cur_mute_state = UNMUTE_STATE;

	api_audio_set_mute(cur_mute_state);
	if(!cur_mute_state)
	{	
      //  enreach_set_volumn(enreach_get_volumn());
	}
	
	if (showOsd)
		ShowMuteOnOff();
}

