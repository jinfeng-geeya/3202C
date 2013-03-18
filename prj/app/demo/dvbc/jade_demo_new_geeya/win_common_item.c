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

#include "win_common_item_obj.h"


/*******************************************************************************
*	local variable define
*******************************************************************************/
struct help_item_resource win_com_item_help[] = 
{
    {1,IM_HELP_ICON_LR,RS_CHANGTYPE},
    {1,IM_HELP_ICON_TB,RS_SELECT_EDIT_ITEM},
    {0,RS_MENU,RS_HELP_BACK},
    {0,RS_HELP_EXIT,RS_HELP_EXIT},          
};

static PRESULT changeDisplay(VACTION act, UINT8 FocusID);
static void setTimeZone(VACTION act);
static void setMenuLanguage(VACTION act);
static void setTransparency(VACTION act);
static void win_time_set_GMT_Offset_string(UINT8 input);
static void win_osd_set_trans_string(UINT8 input);
static void ap_language_setting(UINT16 audio_lang_1, UINT16 audio_lang_2);
static void win_osd_set_language_string(UINT8 input);
static void setBrightness(VACTION act);
static void setContrast(VACTION act);
static void setSaturation(VACTION act);
static void setTVmode(VACTION act);
static void setVideooutput(VACTION act);
static void setTvaspect(VACTION act);
static void win_set_display_string(UINT8 type,UINT8 input);
void set_display_effect_param_scaling(UINT8 type, UINT8  value);
static void win_commom_load_data();
static void win_osd_set_tvratio_string(UINT32 index);
static void win_osd_set_tvmode_string(UINT32 index);
static win_osd_set_videooutput_string(UINT32 index);
UINT32 win_avratio_get_index ( SYSTEM_DATA* p_sys_data );
void win_av_set_ratio ( SYSTEM_DATA* p_sys_data, UINT16 result );
UINT32 win_tvmode_get_index ( UINT32 set );
UINT32 tvmode_osd_to_setting ( UINT32 osd );
void set_video_output( UINT32 index );
static PRESULT commom_item_message_proc(UINT32 msg_type, UINT32 msg_code);

#define TIME_ZONE_ID     1
#define LANGUAGE_ID      2
#define TRANSPRENCY_ID   3
#define BRIGHTNESS_ID    4
#define CONTRAST_ID      5
#define SATURATION_ID    6
#define TVASPECT_ID		 7 
#define TVMODE_ID		 8
#define VIDEOOUTPUT_ID	9


#define VACT_SET_DECREASE	(VACT_PASS + 1)
#define VACT_SET_INCREASE	(VACT_PASS + 2)

#define MAX_OSD_TRANSPARENCY 4
#define MENU_LANGUAGE_STREAM_NUMBER 2 //just support two language

#define FIXED_EFFECT  1
#undef  FIXED_EFFECT



#define TV_MODE_NCNT	(sizeof(tv_mode) / sizeof(tv_mode[0]))
char* tv_mode[] =
    {
        "Auto",
        "PAL",
        "NTSC",
    };

UINT8 tv_mode_set[] =
    {
        TV_MODE_AUTO,
        TV_MODE_PAL,
        TV_MODE_NTSC358,
    };

#define TV_RATIO_NCNT	(sizeof(tv_ratio) / sizeof(tv_ratio[0]));
char* tv_ratio[] =
    {
        "4:3PS",
        "4:3LB",
        "16:9",
    };

#define VIDEO_OUTPUT_NCNT	(sizeof(video_output_set) / sizeof(video_output_set[0]))
char* video_output_set[] = 
	{
		"YCrCb",
		"S-Video",
	};
/*******************************************************************************
 *	helper functions define
 *******************************************************************************/

static VACTION win_common_item_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

	switch (key)
	{
		case V_KEY_MENU:
			act = VACT_CLOSE;
			break;
		case V_KEY_EXIT:
			sys_data_save(1);
			BackToFullScrPlay();			
			break;
		default:
			break;
	}

	return act;
}

static PRESULT win_common_item_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;

	switch (event)
	{
		case EVN_PRE_OPEN:
			wincom_open_title_ext(RS_COMMON_ITEM, IM_TITLE_ICON_SYSTEM);
			wincom_open_help(win_com_item_help, 4);
			win_clear_trash(display_strs[20],7);
			/*make data in system_data take effect */
			win_commom_load_data();
			break;
		case EVN_POST_OPEN:
			break;
		case EVN_PRE_CLOSE:
			*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
			break;
		case EVN_POST_CLOSE:
			sys_data_save(1);
			break;
		case EVN_MSG_GOT:
			ret = commom_item_message_proc( param1, param2 );
			break;
	}

	return ret;
}


static VACTION con_keymap(POBJECT_HEAD pObj, UINT32 key)
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
		default:
			break;
	}
	return act;
}


static PRESULT con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
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

static PRESULT commom_item_message_proc(UINT32 msg_type, UINT32 msg_code)
{
	PRESULT ret = PROC_LOOP;
	switch (msg_type)
	{
		case CTRL_MSG_SUBTYPE_CMD_TIMEDISPLAYUPDATE:
			if (is_time_inited())
			{
				wincom_draw_title_time();
			}
            break;
		default:
			break;
	}
}


/* here we not use multitsel,so all the items value change is handed in this function*/
static PRESULT changeDisplay(VACTION act, UINT8 FocusID)
{
	PRESULT ret = PROC_LOOP;

	switch (FocusID)
	{
		case TIME_ZONE_ID:
			//time set
			setTimeZone(act);
			break;
		case LANGUAGE_ID:
			//language set
			setMenuLanguage(act);
			break;
		case TRANSPRENCY_ID:
			// transparency set
			setTransparency(act);
			break;
		case BRIGHTNESS_ID:
			// brightness set
			setBrightness(act);
			break;
		case CONTRAST_ID:
			// contrast set
			setContrast(act);
			break;
		case SATURATION_ID:
			// saturation set
			setSaturation(act);
			break;
		case TVASPECT_ID:
			setTvaspect(act);
			break;
		case TVMODE_ID:
			setTVmode(act);
			break;
		case VIDEOOUTPUT_ID:
			setVideooutput(act);
			break;
		default:
			break;

	}
	return ret;

}

static void setTimeZone(VACTION act)
{
	SYSTEM_DATA *pSysData = NULL;
	pSysData = sys_data_get();
	TEXT_FIELD *txtTime = &txt_item_value1;
	INT32 hoffset, moffset;


	if (VACT_SET_DECREASE == act)
	{
		//cause GMToffset_count is between 0-47
		if (pSysData->local_time.GMToffset_count >= 2)
		{
			pSysData->local_time.GMToffset_count -= 2;
			timer_InOrDecrease(2,1);
		}
		else if(1==pSysData->local_time.GMToffset_count)
		{
			pSysData->local_time.GMToffset_count = 47;
			timer_InOrDecrease(1,23);
		}
		else if(0==pSysData->local_time.GMToffset_count)
		{
			pSysData->local_time.GMToffset_count = 46;
			timer_InOrDecrease(1,23);
		}        
	}
	else
	{
		if (pSysData->local_time.GMToffset_count <= 45)
		{
			pSysData->local_time.GMToffset_count += 2;
			timer_InOrDecrease(1,1);
		}
		else if(46==pSysData->local_time.GMToffset_count)
		{
			pSysData->local_time.GMToffset_count = 0;
			timer_InOrDecrease(2,23);
		}
		else if(47==pSysData->local_time.GMToffset_count)
		{
			pSysData->local_time.GMToffset_count = 1;
			timer_InOrDecrease(2,23);
		} 
	}
	win_time_set_GMT_Offset_string(pSysData->local_time.GMToffset_count);
	pSysData->local_time.buseGMT = TRUE;
	OSD_TrackObject((POBJECT_HEAD)txtTime, C_UPDATE_ALL/*C_UPDATE_CONTENT*/);
	//sys_data_save(1);

	sys_data_gmtoffset_2_hmoffset(&hoffset, &moffset);
	set_STC_offset(hoffset, 0, 0);

	wincom_open_title_ext(RS_COMMON_ITEM, IM_TITLE_ICON_SYSTEM);
	return ;
}

static void setMenuLanguage(VACTION act)
{
	SYSTEM_DATA *pSysData = NULL;
	pSysData = sys_data_get();
	TEXT_FIELD *txtTime = &txt_item_value2;

	/* this code just support two language, if need more to support,can change to switch*/
	if (RS_SYSTEM_LANGUAGE_CHINESE == txtTime->wStringID)
	{
		txtTime->wStringID = RS_SYSTEM_LANGUAGE_ENGLISH;
		pSysData->lang.OSD_lang = 0;
	}
	else
	{
		txtTime->wStringID = RS_SYSTEM_LANGUAGE_CHINESE;
		pSysData->lang.OSD_lang = 1;
	}

	sys_data_select_language(pSysData->lang.OSD_lang);
	/*we redraw all the objects to show the effects that language changed*/
	OSD_TrackObject((POBJECT_HEAD) &g_win_common_item, C_UPDATE_ALL);
	wincom_open_title_ext(RS_COMMON_ITEM, IM_TITLE_ICON_SYSTEM);
	wincom_open_help(win_com_item_help, 4);
	//sys_data_save(1);
	return ;

}

static void setTransparency(VACTION act)
{
	SYSTEM_DATA *pSysData = NULL;
	pSysData = sys_data_get();
	TEXT_FIELD *txtTime = &txt_item_value3;
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
		win_osd_set_trans_string(pSysData->osd_set.osd_trans);
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
		win_osd_set_trans_string(pSysData->osd_set.osd_trans);
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


static void setBrightness(VACTION act)
{
	SYSTEM_DATA *pSysData = NULL;
	pSysData = sys_data_get();
	TEXT_FIELD *txtTime = &txt_item_value4;
	
	if (VACT_SET_DECREASE == act)
	{
		pSysData->display_effect.brightness = (pSysData->display_effect.brightness >= 1) ? pSysData->display_effect.brightness-1 : 9;
	}
	else
	{
		pSysData->display_effect.brightness = (pSysData->display_effect.brightness <= 8) ? pSysData->display_effect.brightness+1 : 0;
	}	
	win_set_display_string(BRIGHTNESS_ID,pSysData->display_effect.brightness);
	set_display_effect_param_scaling(ENUM_DISP_BRIGHTNESS,pSysData->display_effect.brightness);
	OSD_TrackObject((POBJECT_HEAD)txtTime,C_UPDATE_ALL/* C_UPDATE_CONTENT*/);
	//sys_data_save(1);
}

static void setContrast(VACTION act)
{
	SYSTEM_DATA *pSysData = NULL;
	pSysData = sys_data_get();
	TEXT_FIELD *txtTime = &txt_item_value5;
	
	if (VACT_SET_DECREASE == act)
	{
		pSysData->display_effect.contrast = (pSysData->display_effect.contrast >= 1) ? pSysData->display_effect.contrast-1 : 9;
	}
	else
	{
		pSysData->display_effect.contrast = (pSysData->display_effect.contrast <= 8) ? pSysData->display_effect.contrast+1 : 0;
	}	
	win_set_display_string(CONTRAST_ID,pSysData->display_effect.contrast);
	set_display_effect_param_scaling(ENUM_DISP_CONTRAST,pSysData->display_effect.contrast);
	OSD_TrackObject((POBJECT_HEAD)txtTime, C_UPDATE_ALL/*C_UPDATE_CONTENT*/);
	//sys_data_save(1);
	
}

static void setSaturation(VACTION act)
{
	SYSTEM_DATA *pSysData = NULL;
	pSysData = sys_data_get();
	TEXT_FIELD *txtTime = &txt_item_value6;
	
	if (VACT_SET_DECREASE == act)
	{
		pSysData->display_effect.saturation = (pSysData->display_effect.saturation >= 1) ? pSysData->display_effect.saturation-1 : 9;
	}
	else
	{
		pSysData->display_effect.saturation = (pSysData->display_effect.saturation <= 8) ? pSysData->display_effect.saturation+1 : 0;
	}	
	win_set_display_string(SATURATION_ID,pSysData->display_effect.saturation);	
	set_display_effect_param_scaling(ENUM_DISP_SATURATION,pSysData->display_effect.saturation);
	OSD_TrackObject((POBJECT_HEAD)txtTime, C_UPDATE_ALL/*C_UPDATE_CONTENT*/);
	//sys_data_save(1);

}



static void setTvaspect(VACTION act)
{
	SYSTEM_DATA *pSysData = NULL;
	pSysData = sys_data_get();
	
	UINT32 index = win_avratio_get_index(pSysData);
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
	win_osd_set_tvratio_string(index);
	OSD_TrackObject((POBJECT_HEAD)&txt_item_value7, C_UPDATE_ALL/*C_UPDATE_CONTENT*/);
	win_av_set_ratio(pSysData,index);
	sys_data_set_display_mode ( & ( pSysData->avset ) );
}


static void setTVmode(VACTION act)
{
	SYSTEM_DATA *pSysData = NULL;
	pSysData = sys_data_get();
	UINT32 index = win_tvmode_get_index(pSysData->avset.tv_mode);
	
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
	
	win_osd_set_tvmode_string(index);
	OSD_TrackObject((POBJECT_HEAD)&txt_item_value8, C_UPDATE_ALL/*C_UPDATE_CONTENT*/);
	pSysData->avset.tv_mode = tvmode_osd_to_setting (index);
	api_video_set_tvout ( pSysData->avset.tv_mode );
}

static void setVideooutput(VACTION act)
{
	SYSTEM_DATA *pSysData = NULL;
	pSysData = sys_data_get();
	UINT32 index = pSysData->avset.scart_out;
	
	if (VACT_SET_DECREASE == act)
	{
		index = (index == 0) ? 1 : 0;
	}
	else
	{
		index = (index == 0) ? 1 : 0;
	}
	
	win_osd_set_videooutput_string(index);
	OSD_TrackObject((POBJECT_HEAD)&txt_item_value9, C_UPDATE_ALL/*C_UPDATE_CONTENT*/);
	pSysData->avset.scart_out = index;
	set_video_output(index);
}

static void win_time_set_GMT_Offset_string(UINT8 input)
{
	UINT8 hour, offset;
	UINT8 temp[15];

	if (input >= 23)
	{
		offset = input - 23;
		hour = offset / 2;
		//minute = offset % 2 * 30;
		sprintf(temp, "GMT+%02d", hour);
	}
	else
	{
		offset = 23-input;
		hour = offset / 2;
		//minute = offset % 2 * 30;
		sprintf(temp, "GMT-%02d", hour);
	}

	ComAscStr2Uni(temp, display_strs[20]);
	return ;
}




void ap_language_setting(UINT16 audio_lang_1, UINT16 audio_lang_2)
{
	sys_data_select_audio_language(audio_lang_1, audio_lang_2);
	//UIChChgApplyAudLanguage();
}

/*set transparent string on osd*/
static void win_osd_set_trans_string(UINT8 input)
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

/*set language string on osd*/
static void win_osd_set_language_string(UINT8 input)
{
	UINT16 lang_string_id = 0;

	TEXT_FIELD *txtTime = &txt_item_value2;
	lang_string_id = (input == 0) ? RS_SYSTEM_LANGUAGE_ENGLISH : RS_SYSTEM_LANGUAGE_CHINESE;
	OSD_SetTextFieldContent(txtTime, STRING_ID, lang_string_id);
}


/*set av string on osd*/
static void win_osd_set_tvratio_string(UINT32 index)
{
	UINT8 temp[10]={0};
	TEXT_FIELD *txtTime = &txt_item_value7;
	
	sprintf(temp,"%s",tv_ratio[index]);
	OSD_SetTextFieldContent(txtTime,STRING_ANSI,(UINT32)temp);
	//OSD_TrackObject((POBJECT_HEAD)txtTime, C_UPDATE_ALL/*C_UPDATE_CONTENT*/);

}


/*set tv mode string on osd*/
static void win_osd_set_tvmode_string(UINT32 index)
{
	UINT8 temp[10]={0};
	TEXT_FIELD *txtTime = &txt_item_value8;
	
	sprintf(temp,"%s",tv_mode[index]);
	OSD_SetTextFieldContent(txtTime,STRING_ANSI,(UINT32)temp);
	//OSD_TrackObject((POBJECT_HEAD)txtTime, C_UPDATE_ALL/*C_UPDATE_CONTENT*/);
}

static win_osd_set_videooutput_string(UINT32 index)
{
	UINT8 temp[10]={0};
	TEXT_FIELD *txtTime = &txt_item_value9;
	
	sprintf(temp,"%s",video_output_set[index]);
	OSD_SetTextFieldContent(txtTime,STRING_ANSI,(UINT32)temp);
	//OSD_TrackObject((POBJECT_HEAD)txtTime, C_UPDATE_ALL/*C_UPDATE_CONTENT*/);
}

static void win_set_display_string(UINT8 type,UINT8 input)
{
	UINT8 temp[5];
	
	sprintf(temp,"%d0%%",input+1);
	switch(type)
	{
		case BRIGHTNESS_ID:
			ComAscStr2Uni(temp, display_strs[22]);
			break;
		case CONTRAST_ID:
			ComAscStr2Uni(temp, display_strs[23]);
			break;
		case SATURATION_ID:
			ComAscStr2Uni(temp, display_strs[24]);
			break;
	}
	
}

static void win_commom_load_data()
{
	SYSTEM_DATA *pSysData = NULL;
	pSysData = sys_data_get();
	UINT32 index = 0;

	win_time_set_GMT_Offset_string(pSysData->local_time.GMToffset_count);
	win_osd_set_trans_string(pSysData->osd_set.osd_trans);
	win_osd_set_language_string(pSysData->lang.OSD_lang);
	win_set_display_string(BRIGHTNESS_ID,pSysData->display_effect.brightness);
	set_display_effect_param_scaling(ENUM_DISP_BRIGHTNESS,pSysData->display_effect.brightness);
	win_set_display_string(CONTRAST_ID,pSysData->display_effect.contrast);
	set_display_effect_param_scaling(ENUM_DISP_CONTRAST,pSysData->display_effect.contrast);
	win_set_display_string(SATURATION_ID,pSysData->display_effect.saturation);
	set_display_effect_param_scaling(ENUM_DISP_SATURATION,pSysData->display_effect.saturation);

	index = win_avratio_get_index(pSysData);
	win_osd_set_tvratio_string(index);

	index = win_tvmode_get_index ( pSysData->avset.tv_mode ); 
	win_osd_set_tvmode_string(index);

	win_osd_set_videooutput_string(pSysData->avset.scart_out);
}


UINT32 win_avratio_get_index ( SYSTEM_DATA* p_sys_data )
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

void win_av_set_ratio ( SYSTEM_DATA* p_sys_data, UINT16 result )
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

UINT32 win_tvmode_get_index ( UINT32 set )
{
	UINT32 i;

	for ( i = 0;i < TV_MODE_NCNT;i++ )
	{
		if ( set == tv_mode_set[i] )
				break;
	}
	if ( i == TV_MODE_NCNT )
		i = 0;

	return i;
}


UINT32 tvmode_osd_to_setting ( UINT32 osd )
{
	if ( osd >= TV_MODE_NCNT )
		osd = 0;

	return tv_mode_set[osd];
}

void set_video_output(UINT32 index)
{
	if (index == 0)
	{
		api_Svideo_OnOff(0);
		api_YUV_OnOff(1);
	}
	else if (index == 1)
	{
		api_YUV_OnOff(0);
		api_Svideo_OnOff(1);	
	}
}
void set_display_effect_param_scaling(UINT8 type, UINT8  value)
{
    UINT8 res=0;
    
#ifndef  FIXED_EFFECT
        switch(type)
        {
		case ENUM_DISP_BRIGHTNESS:
            res=40+(value+1)*2;  //  from 42 -60
			do_set_display_effect(type,res);
			break;
		case ENUM_DISP_CONTRAST:
            res=25+(value+1)*5;  //from 30-75
			do_set_display_effect(type,res);
			break;
		case ENUM_DISP_SATURATION:
            res=(value+1)*10;
			do_set_display_effect(type,res);
			break;
		default:
			break;
        }
#else
        res=50;//default value
        do_set_display_effect(type,res);
#endif
    return;

}

