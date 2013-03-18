#include <sys_config.h>
#include <types.h>
#include <mediatypes.h>
#include <math.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libtsi/db_3l.h>
#include <api/libtsi/p_search.h>
#include <api/libdb/tmp_info.h>
#include <hld/dis/vpo.h>
#include <hld/rfm/rfm.h>
#include <hld/rfm/rfm_dev.h>
#include <hld/sto/sto.h>
#include <hld/snd/snd_dev.h>
#include <hld/snd/snd.h>
#include <hld/osd/osddrv.h>
#include <hal/hal_gpio.h>
#include <api/libtsi/sec_tdt2.h>

//#include <api/libpub/lib_nim_manage.h>

#include "../osd_rsc.h"
#include "com_api.h"

#include "system_data.h"

#define SYS_PRINTF	PRINTF

SYSTEM_DATA system_config;
static SYSTEM_DATA system_config_bak;


#define CUR_CHAN_MODE			(system_config.cur_chan_mode)
#define CUR_CHAN_GROUP_INDEX	(system_config.cur_chan_group_index)

#define DEFAULT_DISPLAY_EFFECT       4// for   brightness & contrast & saturation 50%

/////////////////////////////////////
static BOOL get_cur_group_idx(UINT8 *group_idx);
void set_special_group_channel(group_channel_t *p_grp, UINT16 channel_num);

char *stream_iso_639lang_abbr[] =
{
        "eng",
        "fre",
        "deu",
        "ita",
        "spa",
        "por",
        "rus",
        "tur",
        "pol",
        "ara",
    };

char *iso_639lang_multicode[][2] =
	{
        {"fre", "fra"},
        {"deu", "ger"},
        {"spa", "esl"},
        {"cze", "ces"},
        {"gre", "ell"},
        {"swe", "sve"},
};

#define STREAM_ISO_639_NUM  (sizeof(stream_iso_639lang_abbr)/sizeof(char*))
#define STREAM_ISO_639_LANG_MULTICODE_NUM  (sizeof(iso_639lang_multicode)/2/sizeof(char*))
#define AUDIO_CODE_CNT 3
UINT32 get_stream_lang_cnt(void)
{
	return STREAM_ISO_639_NUM;
}

UINT32 get_stream_langm_cnt(void)
{
	return STREAM_ISO_639_LANG_MULTICODE_NUM;
}

/*******************************************************************************
 * system data: load / save / get / init APIs
 ********************************************************************************/

void sys_data_factroy_init()
{
	int i;
	TIMER_SET_CONTENT *timer;
	UINT8 menuPwd[4] = {'0','0','0','0'};
	MEMSET(&system_config, 0, sizeof(system_config));

	sys_data_set_menu_password(menuPwd);
	sys_data_set_chan_password(0);

	system_config.lang.OSD_lang = CHINESE_ENV; //ENGLISH_ENV;//Change default language to chinese.
	system_config.lang.Audio_lang_1 = ENGLISH_ENV;
	system_config.lang.Audio_lang_2 = CHINESE_ENV;
	system_config.lang.sub_lang = ENGLISH_ENV;
	system_config.lang.ttx_lang = ENGLISH_ENV;
	//system_config.bLNB_power = LNB_POWER_ON;
	system_config.poweroffmode = STANDBY_MODE_UPGRADE; /* 0 -STANDBY_MODE_LOWPOWER, 1 -  STANDBY_MODE_UPGRADE*/
	//system_config.vcrloopmode  = VCRLOOP_STB_MASTER; /* VCRLOOP_VCRMASTER = 0, VCRLOOP_RECEIVERMASTER */

	system_config.factory_reset = TRUE;

	system_config.local_time.buseGMT = TRUE;
	system_config.local_time.GMToffset_count = 39;

	system_config.channel_lock = TRUE;
	system_config.channel_lock_level = CHAN_LOCK_NO;
	system_config.menu_lock = TRUE;

	system_config.avset.tv_mode = TV_MODE_AUTO;
	system_config.avset.tv_ratio = TV_ASPECT_RATIO_43;
	system_config.avset.display_mode = DISPLAY_MODE_PANSCAN;
	system_config.avset.scart_out = SCART_CVBS;
	system_config.avset.rf_mode = RF_MODE_PAL_BG;
	system_config.avset.rf_channel = 38;

	extern UINT8 board_vdac_config[VDAC_NUM_MAX];
	for (i=0; i<VDAC_NUM_MAX; i++)
	{
		system_config.avset.vdac_out[i] = board_vdac_config[i];
	}

	system_config.volume = 66;//set default volumn to 2/3

	system_config.osd_set.osd_trans = 0;
	system_config.osd_set.pallete = 0;
	system_config.osd_set.prog_position = 0;
	system_config.osd_set.time_display = 1;
	system_config.osd_set.time_out = 5;
	system_config.osd_set.subtitle_display = 1;
#if((SUBTITLE_ON==1)||(TTX_ON==1))
	system_config.osd_set.subtitle_lang = 0xFF;
	system_config.osd_set.second_osd_mode = OSD_WINDOW;
#endif
	system_config.chan_sw = CHAN_SWITCH_ALL;
	
	system_config.main_frequency = MAIN_FREQ_DEFAULT;
	system_config.main_symbol = MAIN_SYM_DEFAULT;
	system_config.main_qam = MAIN_QAM_DEFAULT;
	system_config.force_ota = 0;
	
	system_config.feed_power = 0; // set feed power to disable

	/* Default: Disable/TV/No Channel */
	system_config.startup_switch = 0;
	system_config.startup_mode = TV_CHAN;
	system_config.startup_group_index[0] = system_config.startup_group_index[1] = 0; /* All sate group*/
	MEMSET(system_config.statup_chsid, 0xFF, sizeof(system_config.statup_chsid));

#ifdef DATA_BROADCAST
#if(DATA_BROADCAST==DATA_BROADCAST_ENREACH)
    system_config.enreach_scr_size = 1;//Set enreach screen 640*526 as default.
#endif
#endif

	//InitTimerSetting(system_config.timer_set.TimerContent);
	system_config.timer_set.timer_num = 1;
	for (i = 0; i < MAX_TIMER_NUM; i++)
	{
		timer = &system_config.timer_set.TimerContent[i];
		timer->timer_mode = TIMER_MODE_OFF;
		timer->timer_service = TIMER_SERVICE_CHANNEL;
		timer->wakeup_year = DEFAULT_YEAR;
		timer->wakeup_month = DEFAULT_MONTH;
		timer->wakeup_day = DEFAULT_DAY;
		//timer->wakeup_time_hour      = 0;
		//timer->wakeup_time_min      = 0;
		timer->wakeup_time = 0;
		//timer->wakeup_duration_hour = 0;
		//timer->wakeup_duration_min      = 0;
		timer->wakeup_duration_time = 0;
		timer->wakeup_channel = 0xFFFFFFFF;
		timer->wakeup_message = TIMER_MSG_BIRTHDAY;
		timer->wakeup_state = TIMER_STATE_READY;
	}

	system_config.sys_dt.year = DEFAULT_YEAR;
	system_config.sys_dt.month = DEFAULT_MONTH;
	system_config.sys_dt.day = DEFAULT_DAY;
	system_config.sys_dt.hour = 0;
	system_config.sys_dt.min = 0;
	system_config.sys_dt.sec = 0;


	system_config.bstandmode = 0;
	system_config.poweroffmode = 0;

	system_config.chchgvideo_type = CHAN_CHG_VIDEO_BLACK;
	system_config.install_beep = 0;
	//system_config.motor_lmt = DISABLE_LMT;
	system_config.cur_sat_idx = 0;
	system_config.cur_chan_group_index = 0; //all program.
	/*set brightness,contrast,saturation to default value 50%*/
	system_config.display_effect.brightness = 4;
	system_config.display_effect.contrast = 4;
	system_config.display_effect.saturation = 4;

#ifdef FAV_GROP_RENAME
	for (i = 0; i < MAX_FAVGROUP_NUM; i++)
		sprintf(system_config.favgrp_names[i], "Fav group %d", i + 1);
#endif

	api_parse_defaultset_subblock();
	//NVODTimerReset();

#ifdef MULTI_CAS
#if(CAS_TYPE==CAS_DVN)
    MEMSET(&system_config.dvn_emails,0,sizeof(dvn_email));
	system_config.rating_control=5;
#endif
#endif

#ifdef AD_SANZHOU
	system_config.ad_frequency = MAIN_FREQ_DEFAULT;
	system_config.ad_symbol = MAIN_SYM_DEFAULT;
	system_config.ad_qam = MAIN_QAM_DEFAULT;
#endif
#ifdef MIS_AD
	system_config.mis_frequency = MAIN_FREQ_DEFAULT;
	system_config.mis_symbol = MAIN_SYM_DEFAULT;
	system_config.mis_qam = MAIN_QAM_DEFAULT;
	system_config.area_code = 0;
	MEMSET(system_config.mis_para, 0, 16);
#endif
#ifdef AD_TYPE
	AD_InitSysData(&system_config.ad_data);
#endif

}


SYSTEM_DATA *sys_data_get(void)
{
	return  &system_config;
}

extern UINT8 rcu_sw_pos;

void sys_data_load(void)
{
	AV_Set *av_set;
	//	tDspMode 			aspect_mode;
	BOOL create_view, grp_exst;
	S_NODE s_node;
	UINT8 group_idx, group_type, group_pos;
	UINT16 channel;
	UINT16 langclass;
	INT32 hoffset, moffset;
	UINT8 GMTOffset;
	date_time dt;
	enum TVSystem eTVSys;
	prog_id_t *prog_id;
	UINT8 av_mode;
	UINT8 pre_chan_mode, boot_chan_node;
	UINT16 chan_idx[2], boot_group_idx;

	
	/* Load data here */
	if(load_tmp_data((UINT8*)&system_config,sizeof(system_config))!=SUCCESS)
	{
		osal_task_sleep(50);
		if(load_tmp_data((UINT8*)&system_config,sizeof(system_config))!=SUCCESS)
		{
			osal_task_sleep(50);
			erase_tmp_sector();
			//if load tmp data fail, set to default 
			sys_data_factroy_init();
			save_tmp_data((UINT8*)&system_config,sizeof(system_config));
		}
	}

#ifdef NVOD_FEATURE
	nvod_release();
	nvod_init(system_config.main_frequency, system_config.main_symbol, system_config.main_qam);
#endif
	//SetNvodScanOverFlag(FALSE);
  
	if (CUR_CHAN_MODE > TV_CHAN)
		CUR_CHAN_MODE = TV_CHAN;

	if (get_prog_num(VIEW_ALL | CUR_CHAN_MODE, 0) != 0)
		create_view = TRUE;
	else
		create_view = FALSE;

	if (system_config.startup_switch && system_config.startup_mode <= 2)
	{
		boot_chan_node = pre_chan_mode = CUR_CHAN_MODE;
		if (system_config.startup_mode == RADIO_CHAN)
		{
			CUR_CHAN_MODE = RADIO_CHAN;
			pre_chan_mode = CUR_CHAN_MODE;
		}
		else if (system_config.startup_mode == TV_CHAN)
		{
			CUR_CHAN_MODE = TV_CHAN;
			pre_chan_mode = CUR_CHAN_MODE;
		}
		
		if (CUR_CHAN_GROUP_INDEX >= MAX_GROUP_NUM+MAX_FAVGROUP_NUM)
			CUR_CHAN_GROUP_INDEX = 0;
		boot_group_idx = CUR_CHAN_GROUP_INDEX;

		CUR_CHAN_GROUP_INDEX = 0;/* Force to all sate group*/

		if ( system_config.startup_mode == RADIO_CHAN
		        || system_config.startup_mode == 2 ) //both
		{

			sys_data_set_cur_chan_mode(RADIO_CHAN);
			recreate_prog_view(VIEW_ALL | RADIO_CHAN, 0);
			prog_id = &system_config.statup_chsid[0];
			//chan_idx[0] = get_prog_pos_ext(prog_id->sat_id,prog_id->tp_id,prog_id->prog_number);
			if (get_prog_num(VIEW_ALL | RADIO_CHAN, 0) > 0)
				chan_idx[0] = 0;
			else
				chan_idx[0] = 0xFFFF;
			if (chan_idx[0] != P_INVALID_ID)
				system_config.cur_chan_group[CUR_CHAN_GROUP_INDEX].radio_channel = chan_idx[0];
		}

		if ( system_config.startup_mode == TV_CHAN
		        || system_config.startup_mode == 2 ) //both

		{
			sys_data_set_cur_chan_mode(TV_CHAN);
			recreate_prog_view(VIEW_ALL | TV_CHAN, 0);
			prog_id = &system_config.statup_chsid[1];
			//chan_idx[1]  = get_prog_pos_ext(prog_id->sat_id,prog_id->tp_id,prog_id->prog_number);
			if (get_prog_num(VIEW_ALL | TV_CHAN, 0) > 0)
				chan_idx[0] = 0;
			else
				chan_idx[0] = 0xFFFF;
			if (chan_idx[1] != P_INVALID_ID)
				system_config.cur_chan_group[CUR_CHAN_GROUP_INDEX].tv_channel = chan_idx[1];
		}

		if (chan_idx[pre_chan_mode] != P_INVALID_ID)
		{
			sys_data_set_cur_chan_mode(pre_chan_mode);
			//recreate_prog_view(CREATE_VIEW_ALL_SAT,0);
			create_view = TRUE;
		}
		else
		{
			CUR_CHAN_GROUP_INDEX = boot_group_idx;
			sys_data_set_cur_chan_mode(boot_chan_node);
			system_config.startup_switch = 0;
		}

	}

	api_feed_power_onoff(system_config.feed_power);

	sys_data_set_cur_chan_mode(CUR_CHAN_MODE);

	if (CUR_CHAN_GROUP_INDEX >= MAX_GROUP_NUM+MAX_FAVGROUP_NUM)
		CUR_CHAN_GROUP_INDEX = 0;

	sys_data_check_channel_groups();

	group_idx = sys_data_get_cur_group_index();
	if (group_idx >= sys_data_get_group_num()&&group_idx<MAX_GROUP_NUM)//当group_idx为fav的时候不需要重新设置为0
		sys_data_set_cur_group_index(0);

	if (sys_data_get_group_num() > 0)
		create_view = TRUE;

	if (create_view)
	{
		group_idx = sys_data_get_cur_group_index();

		grp_exst = sys_data_get_cur_mode_group_infor(group_idx, &group_type, &group_pos, &channel);

		if (!grp_exst)
		{
			CUR_CHAN_GROUP_INDEX = 0;
			group_type = ALL_GROUP_TYPE;
		}

		if (group_type == ALL_GROUP_TYPE)
		{
			recreate_prog_view(VIEW_ALL | CUR_CHAN_MODE, 0);
		}
/*
		else if (group_type == SATE_GROUP_TYPE)
		{
			get_sat_at(group_pos, VIEW_ALL, &s_node);
			recreate_prog_view(VIEW_SINGLE_SAT | CUR_CHAN_MODE, s_node.sat_id);
		}
*/
		else
		{
			recreate_prog_view(VIEW_FAV_GROUP | CUR_CHAN_MODE, group_pos);
		}

	}

	MEMCPY(&system_config_bak, &system_config, sizeof(system_config));

    /*display effect setting*/
    sys_data_load_display_effect(TRUE);
	
	/* Load system time*/
	dt = system_config.sys_dt;
	if ( dt.year < 1900 || dt.year > 2100 || dt.month < 1 || dt.month > 12 || dt.day < 1 || dt.day > 31
	        || dt.hour > 23 || dt.min > 59 || dt.sec > 59 )
	{
		/* If system is invalid,set system to initial time*/
		dt.year = DEFAULT_YEAR;
		dt.month = DEFAULT_MONTH;
		dt.day = DEFAULT_DAY;
		dt.hour = 0;
		dt.min = 0;
		dt.sec = 0;
	}
	set_STC_time(&dt);
	init_UTC(&dt);

	if (system_config.local_time.buseGMT)
	{
		sys_data_gmtoffset_2_hmoffset(&hoffset, &moffset);
		set_STC_offset(hoffset, moffset, 0);
		enable_summer_time(system_config.local_time.SummerTime);
	}
	else
	{

		disable_time_parse();
	}

	/* Video Setting */
	av_set = &system_config.avset;
	//	if ( !firsttime )
	api_video_set_tvout(av_set->tv_mode);

	sys_data_set_display_mode(av_set);
    #if(SYS_DEFINE_NULL != SYS_RFM_MODULE)
	sys_data_set_rf(av_set->rf_mode, av_set->rf_channel);
    #endif


	/* Audio Setting */
	api_audio_set_volume(system_config.volume);


	OSD_SetLangEnvironment(system_config.lang.OSD_lang);
	sys_data_select_audio_language(system_config.lang.Audio_lang_1, system_config.lang.Audio_lang_2);

	sys_data_set_palette(0);

	if (system_config.rcupos > MAX_RCU_POSNUM)
		system_config.rcupos = 0;


#ifdef CHANCHG_VIDEOTYPE_SUPPORT
	UIChChgSetVideoType(system_config.chchgvideo_type);
#endif

}

void sys_data_save(INT32 block)
{
	//Save system_config here
	if (MEMCMP(&system_config_bak, &system_config, sizeof(system_config)))
	{
		save_tmp_data((UINT8*) &system_config, sizeof(system_config));
		MEMCPY(&system_config_bak, &system_config, sizeof(system_config));
	}
}


/*******************************************************************************
 * system data: Channel group operation APIs
 ********************************************************************************/

/* Get & Set current channel mode */
UINT8 sys_data_get_cur_chan_mode(void)
{
	return CUR_CHAN_MODE;
}

void sys_data_set_cur_chan_mode(UINT8 chan_mode)
{
	if (chan_mode > 1)
		chan_mode = 1;
	CUR_CHAN_MODE = chan_mode;

	//set_av_mode(CUR_CHAN_MODE);
}


/* Sync group information with database  */
void sys_data_check_channel_groups(void)
{
	UINT8 i, j;
	BOOL b;
	UINT16 total_v_prog_num, total_a_prog_num;
	SYSTEM_DATA *p_sys_data;
	group_channel_t *p_grp;
	UINT16 sat_v_prog_num[1];
	UINT16 sat_a_prog_num[1];

	UINT16 fav_v_prog_num[MAX_FAVGROUP_NUM];
	UINT16 fav_a_prog_num[MAX_FAVGROUP_NUM];
	UINT16 prog_num=0;

	p_sys_data = sys_data_get();


	total_v_prog_num = 0;
	total_a_prog_num = 0;
	for (i = 0; i < 1; i++)
	{
		sat_v_prog_num[i] = 0;
		sat_a_prog_num[i] = 0;
	}
	for (i = 0; i < MAX_FAVGROUP_NUM; i++)
	{
		fav_v_prog_num[i] = 0;
		fav_a_prog_num[i] = 0;
	}


	recreate_prog_view(VIEW_ALL | TV_CHAN, 0);
	total_v_prog_num = get_prog_num(VIEW_ALL | TV_CHAN, 0);

	recreate_prog_view(VIEW_ALL |RADIO_CHAN, 0);
	total_a_prog_num = get_prog_num(VIEW_ALL | RADIO_CHAN, 0);

//	if(!total_v_prog_num && !total_a_prog_num)
//		goto INIT_GROUP;
	
	/* For all  group */
	p_grp = &p_sys_data->cur_chan_group[0];
	if (total_v_prog_num == 0)
		p_grp->tv_channel = P_INVALID_ID;
	else if (p_grp->tv_channel >= total_v_prog_num)
		p_grp->tv_channel = total_v_prog_num - 1;

	if (total_a_prog_num == 0)
		p_grp->radio_channel = P_INVALID_ID;
	else if (p_grp->radio_channel >= total_a_prog_num)
		p_grp->radio_channel = total_a_prog_num - 1;

	/*nvod & data wil be ignored*/
	recreate_prog_view(VIEW_ALL|PROG_DATA_MODE, 0);
	prog_num = get_prog_num(VIEW_ALL | PROG_DATA_MODE, 0);
	p_grp = &p_sys_data->cur_chan_group[DATA_GROUP_TYPE];
	set_special_group_channel(p_grp,prog_num);

	recreate_prog_view(VIEW_ALL|PROG_NVOD_MODE, 0);
	prog_num = get_prog_num(VIEW_ALL | PROG_NVOD_MODE, 0);
	p_grp = &p_sys_data->cur_chan_group[NVOD_GROUP_TYPE];
	set_special_group_channel(p_grp,prog_num);

	
	/*local group in BAT*/
	for(i=0;i<MAX_LOCAL_GROUP_NUM;i++)
	{
		p_grp = &p_sys_data->cur_chan_group[LOCAL_GROUP_TYPE+i];
		prog_num = 0;
		if(p_sys_data->local_group_id[i])
		{
			recreate_prog_view(VIEW_BOUQUIT_ID|PROG_ALL_MODE,p_sys_data->local_group_id[i]);
			prog_num = get_prog_num(VIEW_ALL|PROG_TV_MODE, 0);
			if(prog_num==0)
				p_grp->tv_channel = P_INVALID_ID;
			else
				//p_grp->tv_channel = prog_num - 1;
				p_grp->tv_channel = 0;
			
			prog_num = get_prog_num(VIEW_ALL|PROG_RADIO_MODE, 0);
			if(prog_num==0)
				p_grp->radio_channel = P_INVALID_ID;
			else
				//p_grp->radio_channel = prog_num - 1;
                p_grp->radio_channel = 0;
		}
		else
		{
			p_grp->tv_channel = P_INVALID_ID;
			p_grp->radio_channel = P_INVALID_ID;
		}
	}
	
	/*fav group*/
	get_specific_prog_num ( SAT_PROG_NUM | FAV_PROG_NUM,
	                        sat_v_prog_num, sat_a_prog_num, fav_v_prog_num, fav_a_prog_num );

	for (i = 0; i < MAX_FAVGROUP_NUM; i++)
	{
		p_grp = &p_sys_data->cur_chan_group[MAX_GROUP_NUM+i];

		if (fav_v_prog_num[i] == 0)
			p_grp->tv_channel = P_INVALID_ID;
		else if (p_grp->tv_channel >= fav_v_prog_num[i])
			//p_grp->tv_channel = fav_v_prog_num[i] - 1;
			p_grp->tv_channel = 0;

		if (fav_a_prog_num[i] == 0)
			p_grp->radio_channel = P_INVALID_ID;
		else if (p_grp->radio_channel >= fav_a_prog_num[i])
			//p_grp->radio_channel = fav_a_prog_num[i] - 1;
			p_grp->radio_channel = 0;
		if (fav_v_prog_num[i] > 0 || fav_a_prog_num[i] > 0)
			SYS_PRINTF("fav=%d,v_prog_num=%d,a_prog_num=%d\n", i, fav_v_prog_num[i], fav_a_prog_num[i]);
	}

	SYS_PRINTF("All Sate %d,total_v_prog_num=%d,total_a_prog_num=%d\n", i, total_v_prog_num, total_a_prog_num);

}

/* Get channel group count */
UINT8 sys_data_get_group_num()
{
	UINT8 i, group_num;
	UINT16 channel;
	BOOL b;
	UINT8 av_flag = CUR_CHAN_MODE;
	SYSTEM_DATA *p_sys_data;
	group_channel_t *p_grp;
	
	p_sys_data = sys_data_get();

#if 0
	group_num = 0;

	for (i = 0; i < MAX_SAT_NUM; i++)
	{
		if (sys_data_get_sate_group_channel(i, &channel, av_flag))
			group_num++;
	}
	
	/* group_num must be 1 */
	for(i=1;i<3;i++)
	{
		p_grp = &(p_sys_data->cur_chan_group[i]);
		if ( ( av_flag == TV_CHAN && p_grp->tv_channel != P_INVALID_ID )
		        || ( av_flag == RADIO_CHAN && p_grp->radio_channel != P_INVALID_ID ) )
		{
			group_num++;
		}
	}

	group_num += p_sys_data->local_group_cnt;
#else
	group_num = 3+ p_sys_data->local_group_cnt;
#endif

	PRINTF("group_num=%d\n", group_num);

	return group_num;
}

UINT8 sys_data_get_sate_group_num(UINT8 av_flag)
{
	UINT8 i, group_num;
	UINT16 channel;

	group_num = 0;

	for (i = 0; i < MAX_SAT_NUM; i++)
	{
		if (sys_data_get_sate_group_channel(i, &channel, av_flag))
			group_num++;
	}

	return group_num;
}

UINT8 sys_data_get_local_group_num()
{
	SYSTEM_DATA *p_sys_data;
	p_sys_data = sys_data_get();
	return p_sys_data->local_group_cnt;;
}

UINT8 sys_data_get_fav_group_num(UINT8 av_flag)
{
	UINT8 i, group_num;
	UINT16 channel;

	group_num = 0;

	for (i = 0; i < MAX_FAVGROUP_NUM; i++)
	{
		if (sys_data_get_fav_group_channel(i, &channel, av_flag))
			group_num++;
	}

	return group_num;
}


/* Get & Set current group index */
UINT8 sys_data_get_cur_group_index(void)
{
	UINT8 i, group_num;
	BOOL b, checkGroup = FALSE;
	UINT8 pre_chan_group_index;


	FIND_GROUP_IDX:

	pre_chan_group_index = CUR_CHAN_GROUP_INDEX;

	for (; CUR_CHAN_GROUP_INDEX <= (MAX_GROUP_NUM+MAX_FAVGROUP_NUM- 1); CUR_CHAN_GROUP_INDEX++)
	{
		b = get_cur_group_idx(&group_num);
		if ( b )	return group_num;
	}

	CUR_CHAN_GROUP_INDEX = pre_chan_group_index;
	while (CUR_CHAN_GROUP_INDEX > 0)
	{
		CUR_CHAN_GROUP_INDEX--;
		b = get_cur_group_idx(&group_num);
		if ( b )	return group_num;
	}
	if (!checkGroup)
	{
		sys_data_check_channel_groups();
		checkGroup = TRUE;
		goto FIND_GROUP_IDX;
	}

	SYS_PRINTF("%s: No program!!!", __FUNCTION__);

	return 0;
}

void sys_data_set_cur_group_index(UINT8 group_idx)
{
	SYSTEM_DATA *p_sys_data;
	group_channel_t *p_grp;

	UINT8 i, group_num;
	UINT8 av_flag;
	BOOL b = TRUE;

	av_flag = CUR_CHAN_MODE;
	p_sys_data = sys_data_get();

	group_num = 0;

	for (i = 0; i < MAX_GROUP_NUM+MAX_FAVGROUP_NUM; i++)
	{
		p_grp = &p_sys_data->cur_chan_group[i];
//		if ( ( av_flag == TV_CHAN && p_grp->tv_channel != P_INVALID_ID )
//		        || ( av_flag == RADIO_CHAN && p_grp->radio_channel != P_INVALID_ID ) )
		{

			if (group_num == group_idx)
			{
				CUR_CHAN_GROUP_INDEX = i;
				return ;
			}
			else
				group_num++;
		}
	}

//	CUR_CHAN_GROUP_INDEX = group_idx - group_num + 3;
}


UINT8 sys_data_get_cur_intgroup_index(void)
{
	return CUR_CHAN_GROUP_INDEX;
}

void sys_data_set_cur_intgroup_index(UINT8 intgroup_idx)
{
	if (intgroup_idx < MAX_GROUP_NUM+MAX_FAVGROUP_NUM)
		CUR_CHAN_GROUP_INDEX = intgroup_idx;
}


/* Get & set current group infor(channel index)  */
UINT16 sys_data_get_cur_group_cur_mode_channel(void)
{
	UINT16 cur_channel;
	sys_data_get_cur_group_channel(&cur_channel, CUR_CHAN_MODE);

	return cur_channel;
}

void sys_data_set_group_channel(UINT8 group_idx, UINT16 channel)
{
	SYSTEM_DATA *p_sys_data;
	group_channel_t *p_grp;
	UINT8 group_type, group_pos;
	UINT16 cur_channel;

	BOOL b = sys_data_get_cur_mode_group_infor(group_idx, &group_type, &group_pos, &cur_channel);

	if (b)
	{
		p_sys_data = sys_data_get();
		p_grp = &p_sys_data->cur_chan_group[group_idx];
		if (CUR_CHAN_MODE == TV_CHAN)
			p_grp->tv_channel = channel;
		else
			p_grp->radio_channel = channel;
	}
}

void sys_data_get_group_channel(UINT8 group_idx, UINT16 *channel, UINT8 av_flag)
{
	SYSTEM_DATA *p_sys_data;
	group_channel_t *p_grp;
	UINT8 group_type, group_pos;
	UINT16 cur_channel;

	BOOL b = sys_data_get_cur_mode_group_infor(group_idx, &group_type, &group_pos, &cur_channel);

	if (b)
	{
		p_sys_data = sys_data_get();
		p_grp = &p_sys_data->cur_chan_group[group_idx];

		if (av_flag == TV_CHAN)
			*channel = p_grp->tv_channel;
		else if(av_flag == RADIO_CHAN)
			*channel = p_grp->radio_channel;
	}
}

void sys_data_set_cur_group_channel(UINT16 channel)
{
	sys_data_set_group_channel(CUR_CHAN_GROUP_INDEX, channel);
}

void sys_data_get_cur_group_channel(UINT16 *channel, UINT8 av_flag)
{
	sys_data_get_group_channel(CUR_CHAN_GROUP_INDEX, channel, av_flag);
}

BOOL sys_data_get_sate_group_channel(UINT16 sate_pos, UINT16 *channel, UINT8 av_flag) //UINT16* radio_channel)
{
	SYSTEM_DATA *p_sys_data;
	group_channel_t *p_grp;

	p_sys_data = sys_data_get();

	p_grp = &p_sys_data->cur_chan_group[0];

	if (av_flag == TV_CHAN)
	{
		if (p_grp->tv_channel == P_INVALID_ID)
			return FALSE;
		else
			*channel = p_grp->tv_channel;
	}
	else
	{
		if (p_grp->radio_channel == P_INVALID_ID)
			return FALSE;
		else
			*channel = p_grp->radio_channel;
	}

	return TRUE;
}

BOOL sys_data_get_fav_group_channel(UINT8 fav_group, UINT16 *channel, UINT8 av_flag) //UINT16* radio_channel)
{
	SYSTEM_DATA *p_sys_data;
	group_channel_t *p_grp;

	if (fav_group >= MAX_FAVGROUP_NUM)
		return FALSE;

	p_sys_data = sys_data_get();
	p_grp = &p_sys_data->cur_chan_group[MAX_GROUP_NUM+fav_group];

	if (p_grp->tv_channel == P_INVALID_ID && p_grp->radio_channel == P_INVALID_ID)
		return FALSE;

	if (av_flag == TV_CHAN)
	{
		if (p_grp->tv_channel == P_INVALID_ID)
			return FALSE;
		else
			*channel = p_grp->tv_channel;
	}
	else
	{
		if (p_grp->radio_channel == P_INVALID_ID)
			return FALSE;
		else
			*channel = p_grp->radio_channel;
	}

	return TRUE;
}



BOOL sys_data_get_cur_mode_group_infor(UINT8 group_idx, UINT8 *group_type, UINT8 *group_pos, UINT16 *channel)
{
	SYSTEM_DATA *p_sys_data;
	group_channel_t *p_grp;
	UINT8 i, group_num;
	UINT8 av_flag;

	av_flag = CUR_CHAN_MODE;
	p_sys_data = sys_data_get();

	group_num = 0;

	for (i = 0; i < MAX_GROUP_NUM+MAX_FAVGROUP_NUM; i++)
	{
		p_grp = &p_sys_data->cur_chan_group[i];
//		if ( ( av_flag == 1 && p_grp->tv_channel != P_INVALID_ID )
//		        || ( av_flag == 0 && p_grp->radio_channel != P_INVALID_ID ) )
		{

			if (group_num == group_idx)
			{
				if (i == 0)
					*group_type = ALL_GROUP_TYPE;
				else if(i==DATA_GROUP_TYPE)
					*group_type = DATA_GROUP_TYPE;
				else if(i==NVOD_GROUP_TYPE)
					*group_type = NVOD_GROUP_TYPE;
				else if(i<MAX_GROUP_NUM)
					*group_type = LOCAL_GROUP_TYPE;
				else
					*group_type = FAV_GROUP_TYPE;
				
				*group_pos = i;
				
				if (av_flag == TV_CHAN)
					*channel = p_grp->tv_channel;
				else
					*channel = p_grp->radio_channel;
				return TRUE;
			}
			else
				group_num++;
		}

	}

	return FALSE;
}


/* Change group index */
INT32 sys_data_change_group(UINT8 group_idx)
{
	UINT8 group_type, group_pos;
	S_NODE s_node;
	UINT16 cur_channel;
	SYSTEM_DATA * psys_data = sys_data_get();

	BOOL b = sys_data_get_cur_mode_group_infor(group_idx, &group_type, &group_pos, &cur_channel);

	if (b)
	{
		if (group_type == ALL_GROUP_TYPE)
		{
			sys_data_set_cur_intgroup_index(0);
			return recreate_prog_view(VIEW_ALL | CUR_CHAN_MODE, 0);
		}
		else if (group_type == DATA_GROUP_TYPE)
		{
			sys_data_set_cur_intgroup_index(group_pos);
			return recreate_prog_view(VIEW_ALL|PROG_DATA_MODE , 0);
			SYS_PRINTF("========Change to DATA_GROUP_TYPE(%d)\n", group_pos);
		}
		else if (group_type == NVOD_GROUP_TYPE)
		{
			sys_data_set_cur_intgroup_index(group_pos);
			return recreate_prog_view(VIEW_ALL | PROG_NVOD_MODE, 0);
			SYS_PRINTF("========Change to NVOD_GROUP_TYPE(%d)\n", group_pos);
		}
		else if (group_type == LOCAL_GROUP_TYPE)
		{
			/*we change to the specified local group*/
			sys_data_set_cur_intgroup_index(group_pos);
			return recreate_prog_view(VIEW_BOUQUIT_ID|CUR_CHAN_MODE, psys_data->local_group_id[group_pos-1]);
			SYS_PRINTF("========Change to LOCAL_GROUP_TYPE(%d,%s)\n", group_pos-1, psys_data->local_group_name[group_pos-1]);
		}
		else if (group_type == FAV_GROUP_TYPE)
		{
			sys_data_set_cur_intgroup_index(group_pos);
			return recreate_prog_view(VIEW_FAV_GROUP | CUR_CHAN_MODE, group_pos-MAX_GROUP_NUM);
		}
		
	}
	else
	{
		sys_data_set_cur_intgroup_index(0);
		return recreate_prog_view(VIEW_ALL|CUR_CHAN_MODE,0);
	}

	return E_FAILURE;
}

INT32 sys_data_change_fav_group(UINT8 group_idx)
{
	return recreate_prog_view(VIEW_FAV_GROUP | CUR_CHAN_MODE, group_idx);
}

/*
BOOL sys_data_get_curprog_info(P_NODE *cur_prog_node)
{
	UINT8 cur_grp_idx;
	UINT16 cur_prog;

	cur_prog = sys_data_get_cur_group_cur_mode_channel();
	cur_grp_idx = sys_data_get_cur_group_index();
	sys_data_change_group(cur_grp_idx);
	get_prog_at(cur_prog, cur_prog_node);

	return TRUE; //Return value returned for Debug error code
}
*/
static BOOL get_cur_group_idx(UINT8 *group_idx)
{
	SYSTEM_DATA *p_sys_data;
	group_channel_t *p_grp;
	UINT8 i, group_num;
	UINT8 av_flag;

	av_flag = CUR_CHAN_MODE;
	p_sys_data = sys_data_get();

	group_num = 0;

	for (i = 0; i < MAX_GROUP_NUM+MAX_FAVGROUP_NUM; i++)
	//For newland ,we search from the first group(all group).
	{
		p_grp = &p_sys_data->cur_chan_group[i];
//		if ( ( av_flag == TV_CHAN && p_grp->tv_channel != P_INVALID_ID )
//		        || ( av_flag == RADIO_CHAN && p_grp->radio_channel != P_INVALID_ID ) )
		{
			if (i == CUR_CHAN_GROUP_INDEX)
			{
				*group_idx = group_num;
				return TRUE;
			}
			else
				group_num++;
		}
	}

	return FALSE;
}


void set_special_group_channel(group_channel_t *p_grp, UINT16 channel_num)
{
	if(!channel_num)
	{
		p_grp->tv_channel = P_INVALID_ID;
		p_grp->radio_channel = P_INVALID_ID;
	}
	else
	{
		//p_grp->tv_channel = channel_num-1;
		p_grp->tv_channel = 0;
	}
}



UINT8 sys_data_cur_group_inc_dec(BOOL inc, UINT32 flag)
{
	sys_data_group_inc_dec(sys_data_get_cur_intgroup_index(), inc, flag);
}

UINT8 sys_data_group_inc_dec(UINT8 grp_idx, BOOL inc, UINT32 flag)
{
	UINT8 group_type, group_pos;
	UINT8 av_mode;
	UINT16 channel = P_INVALID_ID;
	SYSTEM_DATA * psys_data = sys_data_get();

	BOOL b = sys_data_get_cur_mode_group_infor(grp_idx, &group_type, &group_pos, &channel);

	if (!b)
		return ALL_GROUP_TYPE;

	av_mode = sys_data_get_cur_chan_mode();

	if (inc)
	{
		if (grp_idx == psys_data->local_group_cnt)
		{
			grp_idx = DATA_GROUP_TYPE;
			sys_data_get_group_channel(grp_idx, &channel, av_mode);
			if ((flag & GRP_WITH_DATA) && (channel != P_INVALID_ID))
				return grp_idx;
		}
		if (grp_idx == DATA_GROUP_TYPE)
		{
			grp_idx = NVOD_GROUP_TYPE;
			sys_data_get_group_channel(grp_idx, &channel, av_mode);
			if ((flag & GRP_WITH_NVOD) && (channel != P_INVALID_ID))
				return grp_idx;
		}
		if (grp_idx == NVOD_GROUP_TYPE)
		{
			grp_idx = FAV_GROUP_TYPE;
			sys_data_get_group_channel(grp_idx, &channel, av_mode);
			if ((flag & GRP_WITH_FAV) && (channel != P_INVALID_ID))
				return grp_idx;
		}
		if (grp_idx == FAV_GROUP_TYPE)
		{
			return ALL_GROUP_TYPE;
		}
		if (psys_data->local_group_cnt != 0)
		{
			grp_idx++;
			return grp_idx;
		}
		else
			return ALL_GROUP_TYPE;
	}
	else
	{
		if (grp_idx == ALL_GROUP_TYPE)
		{
			grp_idx = FAV_GROUP_TYPE;
			sys_data_get_group_channel(grp_idx, &channel, av_mode);
			if ((flag & GRP_WITH_FAV) && (channel != P_INVALID_ID))
				return grp_idx;
		}
		if (grp_idx == FAV_GROUP_TYPE)
		{
			grp_idx = NVOD_GROUP_TYPE;
			sys_data_get_group_channel(grp_idx, &channel, av_mode);
			if ((flag & GRP_WITH_NVOD) && (channel != P_INVALID_ID))
				return grp_idx;		
		}
		if (grp_idx == NVOD_GROUP_TYPE)
		{
			grp_idx = DATA_GROUP_TYPE;
			sys_data_get_group_channel(grp_idx, &channel, av_mode);
			if ((flag & GRP_WITH_DATA) && (channel != P_INVALID_ID))
				return grp_idx;
		}
		if (grp_idx == DATA_GROUP_TYPE)
		{
			return  psys_data->local_group_cnt;
		}
		if (psys_data->local_group_cnt != 0)
		{
			grp_idx--;
			return grp_idx;
		}
		else
			return ALL_GROUP_TYPE;
	}
}



/*
UINT8 sys_data_get_interal_group_index(UINT8 cur_grp_int_idx, int shift)
{
UINT8  i,n;
SYSTEM_DATA* p_sys_data;
group_channel_t* p_grp;
UINT8 av_flag;

av_flag = CUR_CHAN_MODE;
p_sys_data = sys_data_get();

if(cur_grp_int_idx>=MAX_GROUP_NUM)
cur_grp_int_idx = CUR_CHAN_GROUP_INDEX;

av_flag = CUR_CHAN_MODE;
p_sys_data = sys_data_get();

if(shift!=0)
{
if(shift>0)
shift = 1;
else
shift = -1;

n = 0;
do{
n++;
cur_grp_int_idx = (MAX_GROUP_NUM + cur_grp_int_idx + shift)%MAX_GROUP_NUM;

p_grp = &p_sys_data->cur_chan_group[cur_grp_int_idx];
if( (av_flag==TV_CHAN && p_grp->tv_channel!=P_INVALID_ID)
|| (av_flag==RADIO_CHAN && p_grp->radio_channel!=P_INVALID_ID) )
{
break;
}
}while(n<MAX_GROUP_NUM);
}
else
cur_grp_int_idx = CUR_CHAN_GROUP_INDEX;

return cur_grp_int_idx;


}
 */


/*******************************************************************************
 * system data: Extend APIs
 ********************************************************************************/

/* Get & Set Menu/Chan password & lock*/

BOOL sys_data_get_menu_lock(void)
{
	return system_config.menu_lock;
}


BOOL sys_data_get_channel_lock(void)
{
	return system_config.channel_lock;
}


UINT8 *sys_data_get_menu_password(void)
{
	return system_config.menu_password;
}

UINT32 sys_data_get_chan_password(void)
{
	return system_config.chan_password;
}


void sys_data_set_menu_lock(BOOL lock)
{
	system_config.menu_lock = lock;
}

void sys_data_set_channel_lock(BOOL lock)
{
	system_config.channel_lock = lock;
}

void sys_data_set_menu_password(UINT8 *password)
{
	//system_config.menu_password = password;
	MEMCPY(system_config.menu_password, password, sizeof(system_config.menu_password));
}

void sys_data_set_chan_password(UINT32 password)
{
	system_config.chan_password = password;
}


UINT8 sys_data_get_utcoffset(void)
{
	return system_config.local_time.GMToffset_count;
}

UINT8 sys_data_set_utcoffset(UINT8 offset)
{
	if (offset <= 47)
		system_config.local_time.GMToffset_count = offset;
}

void sys_data_gmtoffset_2_hmoffset(INT32 *hoffset, INT32 *moffset)
{
	//INT8 hoffset,moffset;
	UINT8 GMTOffset;

	GMTOffset = system_config.local_time.GMToffset_count;
	*hoffset = (INT32)(GMTOffset - 23) / 2;
	*moffset = (INT32)((GMTOffset - 23) % 2 * 30);
}


/* Set OSD language */
void sys_data_select_language(UINT8 langid)
{
	OSD_SetLangEnvironment(langid);
	system_config.lang.OSD_lang = langid;
}

void sys_data_select_audio_language(UINT8 langid1, UINT8 langid2)
{
	UINT8 szLangGroup[AUDIO_CODE_CNT][4]; /*default is 2,for support multicode*/
	UINT8 i, multi_audio_cnt;
	UINT8 strm_lang_num, strm_multicode_num;
	UINT8 lang_code[4], lang_codem[4];

	if (langid1 >= STREAM_ISO_639_NUM)
		langid1 = 0;
	if (langid2 >= STREAM_ISO_639_NUM)
		langid2 = langid1;

	multi_audio_cnt = 2;
	strm_lang_num = get_stream_lang_cnt();
	strm_multicode_num = get_stream_langm_cnt();
	MEMSET(lang_code,0,4);	// add lang_code init
	MEMSET(lang_codem,0,4);
	MEMCPY(lang_code, stream_iso_639lang_abbr[langid1], 3);
	MEMCPY(lang_codem, lang_code, 3);
	for (i = 0; i < strm_multicode_num; i++)
	{
		if (MEMCMP(iso_639lang_multicode[i][0], lang_code, 3) == 0)
		{
			MEMCPY(lang_codem, iso_639lang_multicode[i][1], 3);
			multi_audio_cnt = 3;
			break;
		}
	}


	if (multi_audio_cnt == 2)
	{
		STRCPY(szLangGroup[0], lang_code); /*copy Country abrev.*/
		STRCPY(szLangGroup[1], stream_iso_639lang_abbr[langid2]);
	}
	else if (multi_audio_cnt == 3)
	{
		STRCPY(szLangGroup[0], lang_code);
		STRCPY(szLangGroup[1], lang_codem);
		STRCPY(szLangGroup[2], stream_iso_639lang_abbr[langid2]);
	}

	system_config.lang.Audio_lang_1 = langid1;
	system_config.lang.Audio_lang_2 = langid2;
	//UIChChgSetAudLanguage ( szLangGroup, multi_audio_cnt );
	api_set_audio_language( szLangGroup, multi_audio_cnt );
}


/*Get & set factory reset */
BOOL sys_data_get_factory_reset()
{
	if (system_config.factory_reset)
		return TRUE;
	else
		return FALSE;
}

void sys_data_set_factory_reset(BOOL bset)
{
	if (bset == TRUE)
		system_config.factory_reset = 1;
	else
		system_config.factory_reset = 0;
}

//////////////////////////////////////////////

void sys_data_set_palette(UINT8 flag)
{
	static UINT8 *pal = NULL;
	UINT8 *pal_data;
	INT32 i;
	UINT8 val;
	struct osd_device *osd_dev;

	if (pal == NULL)
		pal = (UINT8*)MALLOC(COLOR_N *4);
	pal_data = (UINT8*)OSD_GetRscPallette(0x4080 /*LIB_PALLETE_8BPP*/ | system_config.osd_set.pallete);
	switch (system_config.osd_set.osd_trans)
	{
		case 1:
			//"10%",
			val = 1;
			break;
		case 2:
			//"20%",
			val = 2;
			break;
		case 3:
			//"40%",
			val = 3;
			break;
		case 4:
			//"40%",
			val = 4;
			break;
		case 0:
			//  "No"
		default:
			val = 0;
	}
	MEMCPY(pal, pal_data, COLOR_N *4);
	if (!flag)
	{
		for (i = 0; i < COLOR_N; i++)
		{
			pal[4 *i + 3] = pal[4 *i + 3]*(10-val) / 10;
		}
	}
	osd_dev = (struct osd_device*)dev_get_by_id(HLD_DEV_TYPE_OSD, 0);
	OSDDrv_SetPallette((HANDLE)osd_dev, pal, COLOR_N, OSDDRV_YCBCR);
}

void sys_data_set_display_mode(AV_Set *av_set)
{
	enum TVMode eTVAspect;
	enum DisplayMode e169DisplayMode;

	if (av_set->tv_ratio == TV_ASPECT_RATIO_169)
		eTVAspect = TV_16_9;
	else
	// if(av_set->tv_ratio==TV_ASPECT_RATIO_43)
		eTVAspect = TV_4_3;


	if (av_set->tv_ratio == TV_ASPECT_RATIO_AUTO)
		e169DisplayMode = NORMAL_SCALE;
	else if (av_set->display_mode == DISPLAY_MODE_LETTERBOX)
		e169DisplayMode = LETTERBOX;
	else if (av_set->display_mode == DISPLAY_MODE_PANSCAN)
		e169DisplayMode = PANSCAN;
	else
		e169DisplayMode = NORMAL_SCALE;

	//if(eTVAspect == TV_16_9)
	if (av_set->tv_ratio == TV_ASPECT_RATIO_169)
		e169DisplayMode = PILLBOX;

	vpo_aspect_mode((struct vpo_device*)dev_get_by_id(HLD_DEV_TYPE_DIS, 0),  \
		eTVAspect, e169DisplayMode);

}

#if(SYS_DEFINE_NULL != SYS_RFM_MODULE)
void sys_data_set_rf(UINT8 rf_mode, UINT8 rf_channel)
{
	struct rfm_device *rfm_dev;
	UINT32 set_rf_mode;

	rfm_dev = (struct rfm_device*)dev_get_by_type(NULL, HLD_DEV_TYPE_RFM);
	if (rfm_dev == NULL)
		return ;

	switch (rf_mode)
	{
		case RF_MODE_NTSC_M:
			set_rf_mode = RFM_SYSTEM_NTSC_M;
			break;
		case RF_MODE_PAL_I:
			set_rf_mode = RFM_SYSTEM_PAL_I;
			break;
		case RF_MODE_PAL_DK:
			set_rf_mode = RFM_SYSTEM_PAL_DK;
			break;
		case RF_MODE_PAL_BG:
		default:
			set_rf_mode = RFM_SYSTEM_PAL_BG;
			break;
	}


	rfm_system_set(rfm_dev, set_rf_mode);
	rfm_channel_set(rfm_dev, rf_channel);
}
#endif

UINT16 sys_data_get_cur_satidx(void)
{
	return (system_config.cur_sat_idx);
}

void sys_data_set_cur_satidx(UINT16 sat_idx)
{
	system_config.cur_sat_idx = sat_idx;
}

void do_set_display_effect(UINT8 type,UINT8 value)
{
	struct vpo_io_video_enhance vp_ve_par;
    struct vpo_device * vp_dev;

	switch ( type )
	{
		case ENUM_DISP_BRIGHTNESS:
			/* this value is for driver to take effect*/
			vp_ve_par.grade=value;  
			/* this value is for system_data to record,
			 * here we can always get  positive number when we  do scaling
			 */
            system_config.display_effect.brightness=(value-42)/2; 
            vp_ve_par.changed_flag=VPO_IO_SET_ENHANCE_BRIGHTNESS;
			break;
		case ENUM_DISP_CONTRAST:
			vp_ve_par.grade=value; 
            system_config.display_effect.contrast=(value-30)/5;
            vp_ve_par.changed_flag=VPO_IO_SET_ENHANCE_CONTRAST;
			break;
		case ENUM_DISP_SATURATION:
			vp_ve_par.grade=value;
            system_config.display_effect.saturation=(value-10)/10;
            vp_ve_par.changed_flag=VPO_IO_SET_ENHANCE_SATURATION;
			break;
		default:
			break;
	}

     vp_dev= (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0);
     if(NULL==vp_dev)
     {
           return;
     }
     else
     {
           vpo_ioctl(vp_dev,VPO_IO_VIDEO_ENHANCE, (UINT32)&vp_ve_par);
     }
}


void sys_data_load_display_effect(BOOL b_scal)
{
    st_display_effect * p_disp_eff=&system_config.display_effect;
    UINT8 res=0;

    if(p_disp_eff->brightness>=10)
            p_disp_eff->brightness=DEFAULT_DISPLAY_EFFECT;
    if(p_disp_eff->contrast>=10)
            p_disp_eff->contrast=DEFAULT_DISPLAY_EFFECT;
    if(p_disp_eff->saturation>=10)
            p_disp_eff->saturation=DEFAULT_DISPLAY_EFFECT;         
    
    if(b_scal)
    {
            res=40+(p_disp_eff->brightness+1)*2;  //  from 42 -60
            do_set_display_effect(ENUM_DISP_BRIGHTNESS,res);

            res=25+(p_disp_eff->contrast+1)*5;  //from 30-75
            do_set_display_effect(ENUM_DISP_CONTRAST,res);

            res=(p_disp_eff->saturation+1)*10;
            do_set_display_effect(ENUM_DISP_SATURATION,res);
    }
    return;
          
}

#if(TTX_ON ==1)
UINT8 sys_data_get_ttxpatch()
{	
	UINT8 g0_set;
	switch(system_config.lang.ttx_lang)
		{
			case ENGLISH_ENV:
			case FRANCE_ENV:
			case ITALY_ENV:
			case PORTUGUESE_ENV:
			case SPANISH_ENV:
			case GERMANY_ENV:
				g0_set = LATIN_0;
				break;					
			case TURKISH_ENV:
				g0_set = LATIN_2;					
				break;					
			case POLISH_ENV:
				g0_set = LATIN_1;					
				break;					
			case RUSSIAN_ENV:
				g0_set = LATIN_CYRILLIC;					
				break;					
			case ARABIC_ENV:
				g0_set = LATIN_ARABIC;//ARABIC_HEBREW;					
				break;
#ifdef GREEK_ENV
			case GREEK_ENV:
				g0_set = LATIN_GREEK;
				break;
#endif
			default:
				g0_set = LATIN_0;
				break;
		}
	return g0_set;
}

void TTXEng_set_g0_set()
{
    UINT8 lang_font=0;
	lang_font = sys_data_get_ttxpatch();
	TTXEng_default_g0_set(lang_font);
}

#endif
#ifdef AD_TYPE
void sys_data_get_advdata(ad_sys_data *pData)
{
	MEMCPY(pData, &system_config.ad_data, sizeof(ad_sys_data));
}

void sys_data_set_advdata(ad_sys_data *pData)
{
	if(MEMCMP(&system_config.ad_data, pData, sizeof(ad_sys_data)))
	{
		MEMCPY(&system_config.ad_data, pData, sizeof(ad_sys_data));
		sys_data_save(1);
	}
}
#endif
//////////////////////////////////
