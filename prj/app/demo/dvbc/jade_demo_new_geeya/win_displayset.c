#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>

//#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"

#include "string.id"
#include "images.id"
#include "osd_config.h"
#include "osd_rsc.h"

#include "copper_common/system_data.h"
#include "copper_common/menu_api.h"
//#include "win_com_menu_define.h"
#include "win_com.h"

#include "win_displayset_vega.h"


/*******************************************************************************
*	local variable define
*******************************************************************************/
static PRESULT changeDisplay(VACTION act, UINT8 FocusID);
static void setAlpha(VACTION act);
static void win_osd_set_alpha_string(UINT8 input);
static void setDisplayScale(VACTION act);
static void win_displayset_load_data();
static void win_osd_set_display_scale_string(UINT32 index);
UINT32 win_display_scale_get_index ( SYSTEM_DATA* p_sys_data );
void win_av_set_display_scale ( SYSTEM_DATA* p_sys_data, UINT16 result );

#define ALPHA_ID	1//3
#define DISPLAY_SCALE	2

#define VACT_SET_DECREASE	(VACT_PASS + 1)
#define VACT_SET_INCREASE	(VACT_PASS + 2)

#define MAX_OSD_TRANSPARENCY 4

char* tv_scale[] =
    {
        "4:3PS",
        "4:3LB",
        "16:9",
    };

static VACTION displayset_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

	switch (key)
	{
		case V_KEY_MENU:
		case V_KEY_EXIT:
			act = VACT_CLOSE;
			break;
		case V_KEY_ENTER:
			act = VACT_ENTER;
			break;
		default:
			break;
	}

	return act;
}

static PRESULT displayset_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	VACTION unact;
	switch (event)
	{
		case EVN_PRE_OPEN:
			win_clear_trash(display_strs[20],7);
			/*make data in system_data take effect */
			win_displayset_load_data();
			break;
		case EVN_PRE_CLOSE:
			*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
			break;
		case EVN_UNKNOWN_ACTION:
			unact = (VACTION)(param1 >> 16);
			if(unact == VACT_ENTER)
			{
				sys_data_save(1);
				ret = PROC_LEAVE;
			}
			break;
	}

	return ret;
}


static VACTION displayset_con_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

	switch (key)
	{
		case V_KEY_LEFT:
			act = VACT_SET_DECREASE;
			break;
		case V_KEY_RIGHT:
			act = VACT_SET_INCREASE;
			break;
		case V_KEY_UP:
			act = VACT_CURSOR_UP;
			break;
		case V_KEY_DOWN:
			act = VACT_CURSOR_DOWN;
			break;
		default:
			break;
	}
	return act;
}


static PRESULT displayset_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{

	PRESULT ret = PROC_PASS;
	VACTION act;
	UINT8 id = OSD_GetFocusID(pObj);

	switch (event)
	{
		case EVN_UNKNOWN_ACTION:
			act = (VACTION)(param1 >> 16);
			ret = changeDisplay(act, id);
			break;
		default:
			break;
	}
	return ret;
}

/* here we not use multitsel,so all the items value change is handed in this function*/
static PRESULT changeDisplay(VACTION act, UINT8 FocusID)
{
	PRESULT ret = PROC_LOOP;

	switch (FocusID)
	{
		case ALPHA_ID:
			setAlpha(act);
			break;
		case DISPLAY_SCALE:
			setDisplayScale(act);
			break;
		default:
			break;

	}
	return ret;

}

static void setAlpha(VACTION act)
{
	SYSTEM_DATA *pSysData = NULL;
	pSysData = sys_data_get();
	TEXT_FIELD *txtTime = &bg_alpha;
#ifdef USE_LIB_GE
	ge_global_alpha_t global_A;
	struct ge_device *ge_dev;

	ge_dev = (struct ge_device *)dev_get_by_id(HLD_DEV_TYPE_GE, 0);
	if(RET_SUCCESS!=ge_open(ge_dev))
	{
		PRINTF("ge_open failed!!\n");
		ASSERT(0);
	}
#endif
	if (VACT_SET_DECREASE == act)
	{
		if (pSysData->osd_set.osd_trans == 0)
		{
			pSysData->osd_set.osd_trans = MAX_OSD_TRANSPARENCY;
		}
		else
		{
			pSysData->osd_set.osd_trans--;
		}
		win_osd_set_alpha_string(pSysData->osd_set.osd_trans);
	}
	else
	{
		if (pSysData->osd_set.osd_trans == MAX_OSD_TRANSPARENCY)
		{
			pSysData->osd_set.osd_trans = 0;
		}
		else
		{
			pSysData->osd_set.osd_trans++;
		}
		win_osd_set_alpha_string(pSysData->osd_set.osd_trans);
	}
    
#ifdef USE_LIB_GE
    global_A.layer = GUI_GMA2_SURF;
	global_A.valid = 1;		
    global_A.value = 0xff*(10-pSysData->osd_set.osd_trans)/10;
	ge_io_ctrl(ge_dev,GE_IO_GLOBAL_ALPHA_29E,(UINT32)&global_A);
#else    
	sys_data_set_palette(0);
#endif

#if(defined(MIS_AD) || defined(MIS_AD_NEW))
	MIS_SetOSDAlpha(0x00);
#endif
	OSD_TrackObject((POBJECT_HEAD)txtTime, C_UPDATE_ALL/*C_UPDATE_CONTENT*/);
	return ;
}


static void setDisplayScale(VACTION act)
{
	SYSTEM_DATA *pSysData = NULL;
	pSysData = sys_data_get();
	
	UINT32 index = win_display_scale_get_index(pSysData);
	if (VACT_SET_DECREASE == act)
	{
		if(index == 0) 
			index = 2; 
		else
			index--;
	}
	else
	{
		if(index == 2)
			index = 0;
		else
			index++;
	}
	win_osd_set_display_scale_string(index);
	OSD_TrackObject((POBJECT_HEAD)&display_scale, C_UPDATE_ALL/*C_UPDATE_CONTENT*/);
	win_av_set_display_scale(pSysData,index);
	sys_data_set_display_mode ( & ( pSysData->avset ) );
}


/*set transparent string on osd*/
static void win_osd_set_alpha_string(UINT8 input)
{
	UINT8 temp[5];
	if (input)
	{
		sprintf(temp, "%d0%%", input);
		ComAscStr2Uni(temp, display_strs[21]);
	}
	else
	{
		sprintf(temp, "Off", input);
		ComAscStr2Uni(temp, display_strs[21]);
	}
}



/*set av string on osd*/
static void win_osd_set_display_scale_string(UINT32 index)
{
	UINT8 temp[10]={0};
	TEXT_FIELD *txtTime = &display_scale;
	
	sprintf(temp,"%s",tv_scale[index]);
	OSD_SetTextFieldContent(txtTime,STRING_ANSI,(UINT32)temp);
	//OSD_TrackObject((POBJECT_HEAD)txtTime, C_UPDATE_ALL/*C_UPDATE_CONTENT*/);

}

static void win_displayset_load_data()
{
	SYSTEM_DATA *pSysData = NULL;
	pSysData = sys_data_get();
	UINT32 index = 0;

	win_osd_set_alpha_string(pSysData->osd_set.osd_trans);//获取系统数据--背景透明度
		
	index = win_display_scale_get_index(pSysData);//获取高宽比INDEX
	win_osd_set_display_scale_string(index);
}


UINT32 win_display_scale_get_index ( SYSTEM_DATA* p_sys_data )
{
	if ( p_sys_data->avset.tv_ratio == TV_ASPECT_RATIO_169 )
		return 2;
	else
	{
		if ( p_sys_data->avset.display_mode == DISPLAY_MODE_PANSCAN )
			return 0;
		else
			return 1;
	}
}

void win_av_set_display_scale ( SYSTEM_DATA* p_sys_data, UINT16 result )
{
	if ( result == 0 )
	{
		p_sys_data->avset.tv_ratio = TV_ASPECT_RATIO_43;
		p_sys_data->avset.display_mode = DISPLAY_MODE_PANSCAN;
	}
	else if ( result == 1 )
	{
		p_sys_data->avset.tv_ratio = TV_ASPECT_RATIO_43;
		p_sys_data->avset.display_mode = DISPLAY_MODE_LETTERBOX;
	}
	else
	{
		p_sys_data->avset.tv_ratio = TV_ASPECT_RATIO_169;
		p_sys_data->avset.display_mode = DISPLAY_MODE_LETTERBOX;
	}
}

