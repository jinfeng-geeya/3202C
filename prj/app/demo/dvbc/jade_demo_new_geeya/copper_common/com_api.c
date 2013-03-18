/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2003 Copyright (C)
 *
 *  File: com_api.c
 *
 *  Description:    Common APIs:
 *                  1.Routines LNB and Scart GPIO control
 *                  2.audio & video control
 *
 *  History:
 *      Date        Author      Version  Comment
 *      ====        ======      =======  =======
 *  1.  2006.02.17  Xianyu Nie   0.1.000  Initial
 *
 ****************************************************************************/
#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <osal/osal.h>

#include <hal/hal_gpio.h>
#include <hld/hld_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/snd/snd_dev.h>
#include <hld/snd/snd.h>
#include <hld/deca/deca.h>
#include <hld/decv/vdec_driver.h>
#include <hld/dis/vpo.h>
#include <hld/osd/osddrv.h>

#include <math.h>

#include "dev_handle.h"
#include "system_data.h"
#include "com_api.h"

#include "../win_com.h"
//#include <api/libge/osd_primitive.h>
//#include <api/libge/ge_draw_primitive.h>
//#include <api/libge/osd_plugin.h>
#include <api/libchunk/chunk.h>
#include <api/libpub/lib_frontend.h>

#include <api/libstbinfo/stb_info_data.h>
#include "stb_hwinfo.h"
#ifdef AD_SANZHOU
#include "../sanzhou_ad/adprocess.h"
#endif
extern sys_state_t system_state;

#if ((SUBTITLE_ON == 1)||(TTX_ON == 1)) 
UINT8 api_ttxsub_getlang(struct t_ttx_lang *lang_ttx_list,struct t_subt_lang *lang_sub_list,UINT8 select,UINT8 lang_num);
#endif

/*
sw ---- 0:   disable
1:   enable
 */
void api_feed_power_onoff(UINT8 sw)
{
#ifdef ENABLE_FEED_POWER

#endif
}

/*------------------------------------------------------------

Audio & video control function
------------------------------------------------------------*/

void api_audio_set_volume(UINT8 volume)
{
	snd_set_volume(g_snd_dev, SND_SUB_OUT, volume);
}

void api_audio_set_mute(UINT8 flag)
{
	snd_set_mute(g_snd_dev, SND_SUB_OUT, flag);
}

#ifdef DVBT_BEE_TONE
void api_audio_gen_tonevoice(UINT8 level, UINT8 init)
{
	deca_tone_voice(g_deca_dev, level, init);
}

void api_audio_stop_tonevoice(void)
{
	deca_stop_tone_voice(g_deca_dev);
}

#endif


#ifdef AUDIO_SPECIAL_EFFECT
void api_audio_beep_start(UINT8 *data, UINT32 data_len)
{
	struct ase_str_play_param param;
	UINT32 interval; //interval value, from 0 to 100;

	MEMSET(&param, 0, sizeof(struct ase_str_play_param));
	param.loop_cnt = 0xffffffff;
	param.src = data; /*your audio bit stream source start address, must start from the first frame.*/
	param.len = data_len; /*your audio bit stream length in byte*/

	deca_io_control(g_deca_dev, DECA_STR_PLAY, (UINT32)(&param)); //start beep
	deca_io_control(g_deca_dev, DECA_BEEP_INTERVAL, 0);
}

void api_audio_beep_set_interval(UINT32 interval)
{
	deca_io_control(g_deca_dev, DECA_BEEP_INTERVAL, interval);
}

void api_audio_beep_stop(void)
{
	deca_io_control(g_deca_dev, DECA_STR_STOP, 0);
}

#endif


int api_video_get_srcmode(UINT8 *video_Src)
{
	enum TVSystem src_sys;

	if (vdec_io_control(g_decv_dev, VDEC_IO_GET_MODE, (UINT32)(&src_sys)) == RET_SUCCESS)
	{
		if (src_sys == PAL)
			*video_Src = TV_MODE_PAL;
		else
			*video_Src = TV_MODE_NTSC358;

		return 1;
	}

	return 0;
}

UINT8 api_video_get_tvout(void)
{
	enum TVSystem out_sys;
	UINT8 tv_out;

	vpo_ioctl(g_vpo_dev, VPO_IO_GET_OUT_MODE, (UINT32)(&out_sys));

	switch (out_sys)
	{
		case PAL:
			tv_out = TV_MODE_PAL;
			break;
		case NTSC:
			tv_out = TV_MODE_NTSC358;
			break;
		case NTSC_443:
			tv_out = TV_MODE_NTSC443;
			break;
		case PAL_M:
			tv_out = TV_MODE_PAL_M;
			break;
		default:
			tv_out = TV_MODE_PAL;
	}

	return tv_out;
}

/*------------------------------------------------------------

Other Common functions
------------------------------------------------------------*/

static BOOL preNtsc = 0;

void api_osd_set_tv_system(BOOL ntsc)
{
	struct OSDRect rect;
	UINT32 eOSDSys;

	if ( ntsc == preNtsc )	return;

	OSD_GetRectOnScreen(&rect);
	if (ntsc)
	{
		rect.uTop = OSD_STARTROW_N;
	}
	else
	{
		rect.uTop = OSD_STARTROW_P;
	}
	OSD_ShowOnOff(FALSE);
#if 0
	OSD_SetRectOnScreen(&rect);
#else
	if (!ntsc)
	{
		eOSDSys = OSD_PAL;
		OSD_Scale(OSD_VSCALE_OFF, (UINT32)&eOSDSys);
	}
	else
	// if(NTSC == eTVMode)
	{
		eOSDSys = OSD_NTSC;
		OSD_Scale(OSD_VSCALE_OFF, (UINT32)&eOSDSys);
	}
#endif
	OSD_ShowOnOff(TRUE);
	preNtsc = ntsc;
}

UINT8 vdac_num_get(UINT8 vdac_name)
{
	UINT8 i;

	for (i=0; i<VDAC_NUM_MAX; i++)
	{
		if (vdac_name & 1)
			break;
		else
			vdac_name >>= 1;
	}
	if (i >= VDAC_NUM_MAX)
		return 0;
	return i;
}


/*
void vdac_copy_to_boot_info(SYSTEM_DATA* sys_data)
{
	int i;

	for (i=0; i<VDAC_NUM_MAX; i++)
	{
		sys_data->boot_info.c_info.vdac_out[i] = sys_data->avset.vdac_out[i];
	}
}
*/

void boot_set_vdec_out(SYSTEM_DATA* sys_data)
{	
    union UPG_INFO *upg_info = NULL;
    UINT8 i;
    upg_info =(union UPG_INFO * )stb_info_data_get_upginfo();
	for (i=0; i<VDAC_NUM_MAX; i++)
	{
		upg_info->c_info.vdac_out[i] = sys_data->avset.vdac_out[i];
	}
    stb_info_data_save();
}
void api_Svideo_OnOff(unsigned int sw)//sw:1 open,0 close
{
	SYSTEM_DATA* sys_data = sys_data_get();
	struct vp_io_reg_dac_para dac_reg_para;
	UINT8 cur_sv_y = (sys_data->avset.vdac_out[vdac_num_get(SVIDEO_DAC_Y)] >> 2) & 0x3f;
	UINT8 cur_sv_c = (sys_data->avset.vdac_out[vdac_num_get(SVIDEO_DAC_C)] >> 2) & 0x3f;

	if(sw == 1)//open SVIDEO
	{
		if (cur_sv_y != VDAC_TYPE_SVIDEO || cur_sv_c != VDAC_TYPE_SVIDEO)
		{
			dac_reg_para.eDacType = VDAC_USE_SVIDEO_TYPE;
			dac_reg_para.DacInfo.bEnable = 1;
			dac_reg_para.DacInfo.tDacIndex.uDacFirst    = SVIDEO_DAC_Y;
			dac_reg_para.DacInfo.tDacIndex.uDacSecond   = SVIDEO_DAC_C;
			dac_reg_para.DacInfo.eVGAMode = VGA_NOT_USE;
			dac_reg_para.DacInfo.bProgressive = FALSE;
			vpo_ioctl(g_vpo_dev,VPO_IO_REG_DAC,(UINT32)&dac_reg_para);

			sys_data->avset.vdac_out[vdac_num_get(SVIDEO_DAC_Y)] = VDAC_SVIDEO_Y;
			sys_data->avset.vdac_out[vdac_num_get(SVIDEO_DAC_C)] = VDAC_SVIDEO_C;
			//vdac_copy_to_boot_info(sys_data);
			boot_set_vdec_out(sys_data);
		}
	}
	else
	{
		vpo_ioctl(g_vpo_dev,VPO_IO_UNREG_DAC,VDAC_USE_SVIDEO_TYPE);//close SVIDEO DAC
		sys_data->avset.vdac_out[vdac_num_get(SVIDEO_DAC_Y)] = VDAC_NULL;
		sys_data->avset.vdac_out[vdac_num_get(SVIDEO_DAC_C)] = VDAC_NULL;
		//vdac_copy_to_boot_info(sys_data);
		boot_set_vdec_out(sys_data);
	}
}

void api_YUV_OnOff(unsigned int sw)//sw:1 open,0 close
{
	SYSTEM_DATA* sys_data = sys_data_get();
	struct vp_io_reg_dac_para dac_reg_para;
	UINT8 cur_yuv_y = (sys_data->avset.vdac_out[vdac_num_get(YUV_DAC_Y)] >> 2) & 0x3f;
	UINT8 cur_yuv_u = (sys_data->avset.vdac_out[vdac_num_get(YUV_DAC_U)] >> 2) & 0x3f;
	UINT8 cur_yuv_v = (sys_data->avset.vdac_out[vdac_num_get(YUV_DAC_V)] >> 2) & 0x3f;


	if(sw == 1)//open SVIDEO
	{
		if (cur_yuv_y != VDAC_TYPE_YUV || cur_yuv_u != VDAC_TYPE_YUV || cur_yuv_v != VDAC_TYPE_YUV)
		{
			dac_reg_para.eDacType = VDAC_USE_YUV_TYPE;
			dac_reg_para.DacInfo.bEnable = 1;
			dac_reg_para.DacInfo.tDacIndex.uDacFirst    = YUV_DAC_Y;
			dac_reg_para.DacInfo.tDacIndex.uDacSecond   = YUV_DAC_U;
			dac_reg_para.DacInfo.tDacIndex.uDacThird = YUV_DAC_V;
			dac_reg_para.DacInfo.eVGAMode = VGA_NOT_USE;
			dac_reg_para.DacInfo.bProgressive = FALSE;
			vpo_ioctl(g_vpo_dev,VPO_IO_REG_DAC,(UINT32)&dac_reg_para);

			sys_data->avset.vdac_out[vdac_num_get(YUV_DAC_Y)] = VDAC_YUV_Y;
			sys_data->avset.vdac_out[vdac_num_get(YUV_DAC_U)] = VDAC_YUV_U;
			sys_data->avset.vdac_out[vdac_num_get(YUV_DAC_V)] = VDAC_YUV_V;
			//vdac_copy_to_boot_info(sys_data);
			boot_set_vdec_out(sys_data);
		}
	}
	else
	{
		vpo_ioctl(g_vpo_dev,VPO_IO_UNREG_DAC,VDAC_USE_YUV_TYPE);//close YUV DAC
		sys_data->avset.vdac_out[vdac_num_get(YUV_DAC_Y)] = VDAC_NULL;
		sys_data->avset.vdac_out[vdac_num_get(YUV_DAC_U)] = VDAC_NULL;
		sys_data->avset.vdac_out[vdac_num_get(YUV_DAC_V)] = VDAC_NULL;
		//vdac_copy_to_boot_info(sys_data);
		boot_set_vdec_out(sys_data);
	}
}

void api_video_set_tvout(UINT8 tv_mode)
{
	UINT8 TV_Src, TV_Out;
	enum TVSystem tvsys, out_sys;
	BOOL ntsc;
	BOOL get_fail = FALSE;
	
	if (tv_mode == TV_MODE_AUTO)
	{
		if (SUCCESS != vdec_io_control(g_decv_dev, VDEC_IO_GET_MODE, (UINT32)(&tv_mode)))
			get_fail = TRUE;
	}

	if (get_fail == FALSE)
	{
		switch (tv_mode)
		{
			case TV_MODE_PAL:
				tvsys = PAL;
				break;
			case TV_MODE_PAL_M:
				tvsys = PAL_M;
				break;
			case TV_MODE_PAL_N:
				tvsys = PAL_N;
				break;
			case TV_MODE_NTSC358:
				tvsys = NTSC;
				break;
			case TV_MODE_SECAM:
				tvsys = SECAM;
				break;
			default:
				tvsys = PAL;
		}
		//if( eTVSys != NTSC_443)
		vpo_tvsys(g_vpo_dev, tvsys);
	}


	vpo_ioctl(g_vpo_dev, VPO_IO_GET_OUT_MODE, (UINT32)(&out_sys));


	if (out_sys == NTSC || out_sys == PAL_M || out_sys == PAL_60 || out_sys == NTSC_443)
		ntsc = TRUE;
	else
		ntsc = FALSE;

	api_osd_set_tv_system(ntsc);
}

void api_video_set_pause(UINT8 flag)
{
    struct vdec_device *vdec = dev_get_by_id(HLD_DEV_TYPE_DECV, 0);

    if ( flag )
		chchg_pause_video(vdec);
	else
		chchg_resume_video(vdec);
}


ID api_start_timer(char *name, UINT32 interval, OSAL_T_TIMER_FUNC_PTR handler)
{
	OSAL_T_CTIM t_dalm;
	ID alarmID;

	t_dalm.callback = handler;
	t_dalm.type = TIMER_ALARM;
	t_dalm.time = interval;

	alarmID = osal_timer_create(&t_dalm);
	if (OSAL_INVALID_ID != alarmID)
	{
		//libc_printf("----------------start %s timer succeed\n", name);
		return alarmID;

	}
	else
	{
		//libc_printf("----------------start %s timer failed\n",name);
		return OSAL_INVALID_ID;

	}

}

ID api_start_cycletimer(char* name, UINT32 interval, OSAL_T_TIMER_FUNC_PTR handler)
{
	OSAL_T_CTIM 	t_dalm;
	ID				alarmID;

	t_dalm.callback = handler;
	t_dalm.type = OSAL_TIMER_CYCLE;
	t_dalm.time  = interval;
	
	alarmID = osal_timer_create(&t_dalm);
	if(OSAL_INVALID_ID != alarmID)
	{
		//libc_printf("----------------start %s timer succeed\n", name);
		osal_timer_activate(alarmID, 1);	
		return alarmID;
	}
	else
	{
		//libc_printf("----------------start %s timer failed\n",name);
		return OSAL_INVALID_ID;
	}
}

void api_stop_timer(ID *pTimerID)
{
	ID timerID =  *pTimerID;
	if (OSAL_INVALID_ID != timerID)
		osal_timer_delete(timerID);
	*pTimerID = OSAL_INVALID_ID;
}

enum OSD_MODE api_osd_mode_change(enum OSD_MODE mode)
{
	struct osd_device *osd_dev;
	enum OSDSys eOSDSys;
	struct OSDPara tOpenPara;
	struct OSDRect tOpenRect, r;
	enum TVSystem tvsys;
    SYSTEM_DATA *sys_data;
    enum OSD_MODE pre_mode;
    
	sys_data = sys_data_get();
	static enum OSD_MODE current_mode = OSD_WINDOW;  
    
#if ((SUBTITLE_ON == 1 )||(TTX_ON ==1))   
	static UINT8 current_sub_mode = 0xFF; 
	pre_mode = sys_data->osd_set.second_osd_mode;
   #if(defined(MULTI_CAS) && (CAS_TYPE== CAS_ABEL))
	if(is_fp_displaying() == TRUE)
		return pre_mode;
   #endif
	
#else
	pre_mode = current_mode;
	if (mode == current_mode)
		return pre_mode;
#endif
/*
	if (mode == current_mode)
		return pre_mode;
*/
	if (mode == OSD_WINDOW)
	{
		PRINTF("WINDOW\n");

#if ((SUBTITLE_ON == 1) ||(TTX_ON ==1))  
#if (SUBTITLE_ON == 1 )
		if( pre_mode == OSD_SUBTITLE && current_sub_mode == 0)
			subt_show_onoff(FALSE);
#endif
#if (TTX_ON ==1)
		if((pre_mode == OSD_SUBTITLE && current_sub_mode == 1) || pre_mode == OSD_TELETEXT)
			TTXEng_ShowOnOff(FALSE);			
#endif 
#endif

		vpo_ioctl(g_vpo_dev, VPO_IO_GET_OUT_MODE, (UINT32)(&tvsys));
		if (tvsys == NTSC || tvsys == PAL_M || tvsys == PAL_60 || tvsys == NTSC_443)
			eOSDSys = OSD_NTSC;
		else
			eOSDSys = OSD_PAL;

#if (COLOR_N==4)
		tOpenPara.eMode = OSD_4_COLOR;
#elif (COLOR_N==16)
		tOpenPara.eMode = OSD_16_COLOR;
#elif (COLOR_N==256)
		tOpenPara.eMode = OSD_256_COLOR;
#endif
		tOpenPara.uGAlphaEnable = 0;
		tOpenPara.uGAlpha = 0x0f;


		tOpenRect.uLeft = (720 - OSD_MAX_WIDTH ) >> 1;
		tOpenRect.uWidth = OSD_MAX_WIDTH;
		tOpenRect.uTop = (576 - OSD_MAX_HEIGHT ) >> 1;
		tOpenRect.uHeight = OSD_MAX_HEIGHT;

		r.uLeft = 0;
		r.uTop = 0;
		r.uWidth = OSD_MAX_WIDTH;
		r.uHeight = OSD_MAX_HEIGHT;

#ifndef USE_LIB_GE
		osd_dev = g_osd_dev;

		OSDDrv_Close((HANDLE)osd_dev);
		OSDDrv_Open((HANDLE)osd_dev, &tOpenPara);
#endif
		osal_task_sleep(20);
		sys_data = sys_data_get();
#ifdef USE_LIB_GE        
        OSD_ClearScrn(NULL);
#else
		OSDDrv_CreateRegion((HANDLE)osd_dev, 0, &(tOpenRect), NULL);
		OSDDrv_RegionFill((HANDLE)osd_dev, 0, &r, OSD_TRANSPARENT_COLOR);
#endif
        
		OSD_Scale(OSD_VSCALE_OFF, (UINT32) &eOSDSys);
		sys_data_set_palette(0);
		OSD_ShowOnOff(OSDDRV_ON);
		current_mode = OSD_WINDOW;     
#if((TTX_ON == 1)||(SUBTITLE_ON == 1))
		sys_data->osd_set.second_osd_mode = OSD_WINDOW;
#endif
	}
#if(defined(MULTI_CAS) && (CAS_TYPE== CAS_ABEL) && ((TTX_ON == 1)||(SUBTITLE_ON == 1)))
	else if(mode == OSD_NO_SHOW)
	{
		if( pre_mode == OSD_SUBTITLE && current_sub_mode == 0)
		{
	#if(SUBTITLE_ON == 1)	
			subt_show_onoff(FALSE);
	#endif	
		}
		else if((pre_mode == OSD_SUBTITLE && current_sub_mode == 1) || pre_mode == OSD_TELETEXT)
		{
	#if(TTX_ON == 1)	
			TTXEng_SetInitLang(0xFF);//for exit TTXEng
			TTXEng_ShowOnOff(FALSE);
	#endif		
		}
		sys_data->osd_set.second_osd_mode = OSD_NO_SHOW;
	}
#endif	
#if ((TTX_ON == 1)||(SUBTITLE_ON == 1)) 
	else if ( mode == OSD_SUBTITLE )
	{
		if(sys_data->osd_set.subtitle_display == 1)
		{
			struct t_ttx_lang *ttxlang_list;
			struct t_subt_lang	*sublang_list;
			UINT8 lang_num,lang_numttx,sel_lang,sub_mode;

			sub_mode = 0;
			sel_lang = 0xFF;
#if (SUBTITLE_ON == 1)
			subt_get_language(&sublang_list,&lang_num);
			sel_lang = api_ttxsub_getlang(NULL,sublang_list,sys_data->lang.sub_lang,lang_num);
#endif

#if (TTX_ON ==1)			
			if(sel_lang == 0xFF)
			{	
				sub_mode = 1;
				TTXEng_GetSubtLang(&ttxlang_list, &lang_num);
				sel_lang = api_ttxsub_getlang(ttxlang_list,NULL,sys_data->lang.sub_lang,lang_num);				
			}
#endif
			if(sys_data->osd_set.subtitle_lang != 0xFF) //SUBTITLE_LANGUAGE_INVALID, 0xFF
			{
#if (SUBTITLE_ON == 1)
				subt_get_language(&sublang_list,&lang_num);
#else
				lang_num = 0;
#endif
#if (TTX_ON ==1)
				TTXEng_GetSubtLang(&ttxlang_list, &lang_numttx);
#else
				lang_numttx = 0;
#endif
				
				if(((lang_num + lang_numttx) > 0) && (sys_data->osd_set.subtitle_lang < (lang_num + lang_numttx)))
				{
					if(lang_numttx>0 && sys_data->osd_set.subtitle_lang <lang_numttx)
					{
						sub_mode = 1;//ttx-subtitle
						sel_lang = sys_data->osd_set.subtitle_lang;
					}
					else if((lang_num>0) && (sys_data->osd_set.subtitle_lang <(lang_num + lang_numttx)))
					{
						sub_mode = 0;//normal-subtitle
						sel_lang = (sys_data->osd_set.subtitle_lang - lang_numttx);
					}
				}
				else
				{		
					PRINTF("NO SUBTITLE DATA. \n");
				}
					
			}
			
			if(sel_lang != 0XFF)/*subt languages exist*/
			{
				if(sub_mode == 0)
				{
#if (TTX_ON ==1)
					TTXEng_ShowOnOff(FALSE);
#endif

#if (SUBTITLE_ON == 1)
					subt_show_onoff(TRUE);
					subt_set_language(sel_lang);
//                    subt_enable(TRUE);	
#endif				
				}
				else
				{
#if (SUBTITLE_ON == 1 )
					subt_show_onoff(FALSE);
//                   	subt_enable(FALSE);
#endif

#if (TTX_ON ==1)
//                    if(current_mode!=OSD_SUBTITLE)
//                        TTXEng_ShowOnOff(TRUE);
					TTXEng_ShowOnOff(TRUE); 
					TTXEng_SetSubtLang(sel_lang);
#endif
				}
				//sys_data->osd_set.second_osd_mode = OSD_SUBTITLE;
			}

//			api_osd_layer2_onoff(OSDDRV_ON);
			current_sub_mode = sub_mode;
            current_mode = OSD_SUBTITLE; 
		}
		else
		{
//Clear the subtitle or teletext content of the last program
#if (SUBTITLE_ON == 1 )
			subt_show_onoff(FALSE);
#endif
#if (TTX_ON ==1)
//			TTXEng_SetInitLang(0xFF);//for exit TTXEng
			TTXEng_ShowOnOff(FALSE);			
#endif  
		}
		sys_data->osd_set.second_osd_mode = OSD_SUBTITLE;
	}
	else if ( mode == OSD_TELETEXT )
	{
	
#if (TTX_ON ==1)
		PRINTF("TELETEXT\n");
#if (SUBTITLE_ON == 1 )
		subt_show_onoff(FALSE);
#endif
		TTXEng_ShowOnOff(TRUE);
		sys_data->osd_set.second_osd_mode = OSD_TELETEXT;
#endif

	}  
#endif    
	else if (mode == 0xFF)
	{
		current_mode = 0xFF;
#if((TTX_ON == 1)||(SUBTITLE_ON == 1))		
		sys_data->osd_set.second_osd_mode = OSD_INVALID_MODE;	
#endif
	}
    
    sys_data_save(1);
	return pre_mode;

}

/******************************************************************************************************/
#if ((SUBTITLE_ON == 1)||(TTX_ON == 1)) 
UINT8 api_ttxsub_getlang(struct t_ttx_lang *lang_ttx_list,struct t_subt_lang *lang_sub_list,UINT8 select,UINT8 lang_num);

UINT8 api_ttxsub_getlang(struct t_ttx_lang *lang_ttx_list,struct t_subt_lang *lang_sub_list,UINT8 select,UINT8 lang_num)
{
	UINT8 i;
	UINT8 stream_lang_num,stream_langmucode_num;
	struct t_ttx_lang 	*lang_txt;
	struct t_subt_lang	*lang_sub;
	UINT8 lang_code[3],lang_codem[3],*code;
	extern char *stream_iso_639lang_abbr[];
	extern char *iso_639lang_multicode[][2];
#ifdef DTG_SUB_SELECT
	UINT8 sub_type = 0x10;
#endif	
	stream_lang_num = get_stream_lang_cnt();
	stream_langmucode_num = get_stream_langm_cnt();
	
	
	if(lang_num==0)
		return 0xFF;
	if(lang_ttx_list == NULL && lang_sub_list==NULL)
		return 0xFF;
	
	if(lang_num == 1)
		return 0;
	if(select < stream_lang_num)
	{
		MEMCPY(lang_code,stream_iso_639lang_abbr[select],3);
		MEMCPY(lang_codem,lang_code,3);
		for(i=0;i<stream_langmucode_num;i++)
		{
			if(MEMCMP(iso_639lang_multicode[i][0],lang_code,3) == 0)
			{
				MEMCPY(lang_codem,iso_639lang_multicode[i][1],3);
				break;
			}
		}
	}
	else
		return 0;
	
	for(i=0;i<lang_num;i++)
	{
        if(lang_ttx_list != NULL)
		{
			code = lang_ttx_list[i].lang;
		}
		else
		{
			code = lang_sub_list[i].lang;
#ifdef DTG_SUB_SELECT
			sub_type = lang_sub_list[i].subt_type;
#endif
		}
		
		if(MEMCMP(lang_code,code,3) == 0
			|| MEMCMP(lang_codem,code,3) == 0)
#ifdef DTG_SUB_SELECT
			if((sub_type>>4)==1)
#endif
			return i;
	}
#ifdef DTG_SUB_SELECT
	if(lang_sub_list != NULL)
	{
		for(i=0;i<lang_num;i++)
		{
			lang_sub = &lang_sub_list[i];
			code = lang_sub->lang;
			sub_type = lang_sub->subt_type;

			if(MEMCMP("und",code,3) == 0
				&& (sub_type>>4)==1)
					return i;
		}
	}
#endif
	return 0;
}

#endif

/**************************************************************
id:  logo  id
addr: return address
len: retrun data length
return : false , true

 ****************************************************************/
//#ifdef LOGO_SUPPORT
UINT8 api_get_chuck_addlen(UINT32 id, UINT32 *addr, UINT32 *len)
{
	INT32 ret;
	CHUNK_HEADER chuck_hdr;

	ret = sto_get_chunk_header(id, &chuck_hdr);
	if (ret == 0)
		return 0;

	*addr = (UINT32)sto_chunk_goto(&id, 0xFFFFFFFF, 1) + CHUNK_HEADER_SIZE;
	*len = chuck_hdr.len - CHUNK_HEADER_SIZE + CHUNK_NAME;

	return 1;
}


void api_show_bootup_logo(void)
{
	int i;
	void *pWriteBuffer;
	UINT32 uSizeGot;
	RET_CODE ret_code;
	UINT32 logo_id, addr, len;
	struct sto_device *sto_flash_dev;
	
#ifdef AD_SANZHOU
	if(szxc_ad_show_mpg(AD_STARTUP)==TRUE)
	{
		return;
	}
#endif
	logo_id = MENU_LOGO_ID;
	if (api_get_chuck_addlen(logo_id, &addr, &len) == 0)
		return ;
	//screen_back_state = SCREEN_BACK_MENU;

	ret_code = vdec_start((struct vdec_device*)dev_get_by_id(HLD_DEV_TYPE_DECV, 0));
	sto_flash_dev = (struct sto_device*)dev_get_by_id(HLD_DEV_TYPE_STO, 0);

	for (i = 0; i < 2; i++)
	{
		ret_code = vdec_vbv_request(g_decv_dev, len, &pWriteBuffer, &uSizeGot, NULL);
		//MEMCPY(pWriteBuffer,(void*)addr,uSizeGot);
		sto_get_data(sto_flash_dev, (UINT8*)pWriteBuffer, addr, uSizeGot);
		vdec_vbv_update(g_decv_dev, uSizeGot);
	}
}

void api_show_radio_logo(void)
{
	UINT32 addr, len;
    struct cc_logo_info logo;
    struct cc_device_list dev_list;
#ifdef AD_SANZHOU
	if(szxc_ad_show_mpg(AD_RADIO)==TRUE)
	{
		return;
	}
#endif
	if (api_get_chuck_addlen(RADIO_LOGO_ID, &addr, &len))
	{
            MEMSET(&logo, 0, sizeof(logo));
            MEMSET(&dev_list, 0, sizeof(struct cc_device_list));
            logo.addr = (UINT8 *)addr;
            logo.size = len;
            logo.sto = dev_get_by_id(HLD_DEV_TYPE_STO, 0);
            dev_list.vdec = dev_get_by_id(HLD_DEV_TYPE_DECV, 0);
            dev_list.deca = dev_get_by_id(HLD_DEV_TYPE_DECA, 0);
            dev_list.snd_dev = dev_get_by_id(HLD_DEV_TYPE_SND, 0);
            dev_list.dmx = dev_get_by_id(HLD_DEV_TYPE_DMX, 0);

            chchg_show_logo(&logo, &dev_list);
	}
}

void api_show_menu_logo(void)
{
	UINT32 addr, len;

    struct cc_logo_info logo;
    struct cc_device_list dev_list;
#ifdef AD_SANZHOU
	if(szxc_ad_show_mpg(AD_MAINMENU)==TRUE)
	{
		return;
	}
#endif    
	if ( api_get_chuck_addlen ( MENU_LOGO_ID, &addr, &len ) )
	{
		MEMSET(&logo, 0, sizeof(logo));
            logo.addr = (UINT8 *)addr;
            logo.size = len;
            logo.sto = dev_get_by_id(HLD_DEV_TYPE_STO, 0);
            dev_list.vdec = dev_get_by_id(HLD_DEV_TYPE_DECV, 0);
            dev_list.deca = dev_get_by_id(HLD_DEV_TYPE_DECA, 0);
            dev_list.snd_dev = dev_get_by_id(HLD_DEV_TYPE_SND, 0);
            dev_list.dmx = dev_get_by_id(HLD_DEV_TYPE_DMX, 0);

            chchg_show_logo(&logo, &dev_list);
	}
}

void api_show_row_logo(UINT32 logo_id)
{
	int i;    
	void* pWriteBuffer;
	UINT32 uSizeGot;
	RET_CODE ret_code;
	UINT32 addr,len;
	struct VDec_StatusInfo CurStatus;

	struct sto_device* sto_flash_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);

	CHUNK_HEADER chuck_hdr;

	if(sto_get_chunk_header(logo_id, &chuck_hdr) == 0)
		return;

	addr = (UINT32)sto_chunk_goto(&logo_id, 0xFFFFFFFF, 1) + CHUNK_HEADER_SIZE;
	len = chuck_hdr.len - CHUNK_HEADER_SIZE + CHUNK_NAME;
    
    ret_code = vdec_stop(g_decv_dev,FALSE,FALSE);
    ret_code = vdec_start(g_decv_dev);
	for(i=0; i<2; i++)
	{
		ret_code = vdec_vbv_request(g_decv_dev, len, &pWriteBuffer,&uSizeGot,NULL);

		sto_get_data(sto_flash_dev, (UINT8 *)pWriteBuffer, addr, uSizeGot);
		vdec_vbv_update(g_decv_dev, uSizeGot);
	} 
    
    MEMSET(&CurStatus,0,sizeof(struct VDec_StatusInfo));
    for(i=0;i<100;i++) //waiting
    {
        osal_task_sleep(3);
	    vdec_io_control(g_decv_dev, VDEC_IO_GET_STATUS, (UINT32)&CurStatus);
        if(CurStatus.uFirstPicShowed)
            break;
    }
    ret_code = vdec_stop(g_decv_dev,FALSE,FALSE);
	return;
}

int api_parse_defaultset_subblock(void)
{
	unsigned chid;
	int i, sub_cnt;
	unsigned char sb_type;
	unsigned long addr, end_addr;
	SUBBLOCK_HEADER hd;
	struct sto_device *sto;

	unsigned char item_type, byte, byte2;
	unsigned long item_datalen, n;
	SYSTEM_DATA *sys_data;

	sto = (struct sto_device*)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
	if (sto == NULL)
		return 0;
	sys_data = sys_data_get();


	chid = STB_CHUNK_DEFAULT_DB_ID;
	sb_type = 0x02;

	sub_cnt = sto_subblock_count(chid);
	if (sub_cnt == 0)
		return 0;

	addr = sto_subblock_goto(chid, &sb_type, 0xFF, 1);
	if (addr &0x80000000)
		return 0;
	sto_get_subblock_header(STB_CHUNK_DEFAULT_DB_ID, sb_type, &hd);
	addr += SUBBLOCK_HEADER_SIZE;

	end_addr = addr + hd.len;
	while (addr < end_addr)
	{
		sto_get_data(sto, &item_type, addr++, 1);
		if (item_type == 0x00 || item_type == 0xFF)
			break;

		n = 0;
		item_datalen = 0;
		while (n++ < 4)
		{
			item_datalen <<= 7;
			sto_get_data(sto, &byte, addr++, 1);
			item_datalen += byte &0x7F;
			if ((byte &0x80) == 0x00)
			 /* length field byte end. */
				break;
		}

		if ( byte & 0x80 )	break;	/* item data length field error (>4Bs) */
		 /* item data length field error (>4Bs) */

		switch (item_type)
		{
			case 0x01:
				 /* Fav group name */
#ifdef FAV_GROP_RENAME
				{
					unsigned char favgrp_cnt, favgrp_namelen, favgrp_code;
					sto_get_data(sto, &favgrp_cnt, addr + 0, 1);
					sto_get_data(sto, &favgrp_namelen, addr + 1, 1);
					sto_get_data(sto, &favgrp_code, addr + 2, 1);
					if (favgrp_cnt > MAX_FAVGROUP_NUM)
						favgrp_cnt = MAX_FAVGROUP_NUM;
					if (favgrp_cnt == 0xFF || favgrp_namelen == 0xFF || favgrp_code == 0xFF)
						break;
					if (favgrp_code != 0x00 && favgrp_code != 0x01)
						break;

					byte = favgrp_namelen;
					if (byte > FAV_GRP_NAME_LEN)
						byte = FAV_GRP_NAME_LEN;

					for (i = 0; i < favgrp_cnt; i++)
					{
						sto_get_data(sto, sys_data->favgrp_names[i], addr + 3+favgrp_namelen * i, byte);

						if (favgrp_code == 0)
							sys_data->favgrp_names[i][favgrp_namelen] = '\0';
					}
				}
#endif
				break;
			default:
				;
		}

		addr += item_datalen;
	}

}

BOOL api_tv_radio_switch()
{
	UINT16 channel;
	UINT8 uAVMode;
	uAVMode = sys_data_get_cur_chan_mode();

	//1Switch TV to Audio
	if (PROG_TV_MODE == uAVMode)
	{
		sys_data_set_cur_chan_mode(PROG_RADIO_MODE);
	}
	else
	{
		sys_data_set_cur_chan_mode(PROG_TV_MODE);
	}

	//channel = get_node_num(TYPE_PROG_NODE, NULL);
	channel = get_prog_num(VIEW_ALL | uAVMode, 0);

	//no program
	if (0 == channel)
	{
		//popup no program message
		sys_data_set_cur_chan_mode(uAVMode);
		return FALSE;
	}
	else
	{
		if (PROG_TV_MODE == uAVMode)
			api_show_radio_logo();
		sys_data_change_group(sys_data_get_cur_group_index());
		return TRUE;
	}
}

/**************************************************************
check leap year
 ****************************************************************/
UINT8 api_check_is_leap_year(UINT16 yy)
{
	if ((yy % 4 == 0 && yy % 100 != 0) || yy % 400 == 0)
		return 1;
	else
		return 0;
}

/*******************************************************
yy,mm,dd,valid --(I/O) Input year,month,day time to check is valid
type           --(I/O) check day valid,
(when daily,weekly type = 0,
else if monthly type  = 1
else if yearly type = 2)
 ***************************************************************/

static UINT8 month_days[12] =
{
	/*1  2  3 4  5  6   7  8  9 10 11 12  */
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

/*
return:
0  	-- OK
-1 	-- year < 2000 || year>2100)
-2     -- month==0 || month>12
-3     -- day==0 || day>max days of this month
 */
INT32 api_check_valid_date(UINT16 year, UINT8 month, UINT8 day)
{
	UINT8 mm_days;

	mm_days = month_days[month - 1];
	if (month == 2 && api_check_is_leap_year(year))
		mm_days = 29;
	if (year < 2000 || year > 2100)
		return  - 1;
	if (month == 0 || month > 12)
		return  - 2;
	if (day == 0 || day > mm_days)
		return  - 3;

	return 0;
}

BOOL api_correct_yy_mm_dd(UINT16 *yy, UINT8 *mm, UINT8 *dd, UINT8 type)
{
	UINT16 year;
	UINT8 month, day, mm_days;
	BOOL b;
	year =  *yy;
	month =  *mm;
	day =  *dd;

INIT_DATETIME_CHECK:
	if ( day <= 0 )  // hong zhang 050430
	{
		month -= 1;
	}

	/* For monthly */
	if (month > 12)
	{
		year += 1;
		month = 1;
	}
	else if (month <= 0)
	// hong zhang 050430
	{
		year -= 1;
		month = 12;
	}

	mm_days = month_days[month - 1];
	if (month == 2 && api_check_is_leap_year(year))
		mm_days = 29;

	if (type == 0)
	//daily,weekly
	{
		if (day > mm_days)
		{
			day -= mm_days;
			month += 1;
			if (month > 12)
			{
				month = 1;
				year += 1;
			}
		}
		else if (day <= 0)
		// hong zhang 050430
		{
			day = mm_days;
		}

		b = TRUE;
	}
	else
	//monthly or yearly
	{

		if (day <= mm_days)
		{
			b = TRUE;
		}
		else if (type == 1)
		//monthly
		{
			month += 1;
			goto INIT_DATETIME_CHECK;
		}
		else if (type == 2)
		//yearly
		{
			year += 1;
			goto INIT_DATETIME_CHECK;
		}
	}

	*yy = year;
	*mm = month;
	*dd = day;

	b = TRUE;

	return TRUE;

}

/*************************************
return:
0	- dt1 = dt2
1   	- dt1 > dt2
-1  	- dt1 < dt2
 **************************************/
INT32 api_compare_time(date_time *dt1, date_time *dt2)
{
	if (dt1->hour > dt2->hour)
		return 1;
	else if (dt1->hour < dt2->hour)
		return  - 1;

	if (dt1->min > dt2->min)
		return 1;
	else if (dt1->min < dt2->min)
		return  - 1;

	return 0;
}

INT32 api_compare_time_ext(date_time *dt1, date_time *dt2)
{
	if (dt1->hour > dt2->hour)
		return 1;
	else if (dt1->hour < dt2->hour)
		return  - 1;

	if (dt1->min > dt2->min)
		return 1;
	else if (dt1->min < dt2->min)
		return  - 1;

	if (dt1->sec > dt2->sec)
		return 1;
	else if (dt1->sec < dt2->sec)
		return  - 1;


	return 0;
}


INT32 api_compare_day(date_time *dt1, date_time *dt2)
{
	if (dt1->year > dt2->year)
		return 1;
	else if (dt1->year < dt2->year)
		return  - 1;

	if (dt1->month > dt2->month)
		return 1;
	else if (dt1->month < dt2->month)
		return  - 1;

	if (dt1->day > dt2->day)
		return 1;
	else if (dt1->day < dt2->day)
		return  - 1;

	return 0;
}

INT32 api_compare_day_time(date_time *dt1, date_time *dt2)
{
	INT32 nRet = api_compare_day(dt1, dt2);
	if ( nRet != 0 ) return nRet;
	return api_compare_time(dt1, dt2);
}


INT32 api_compare_day_time_ext(date_time *dt1, date_time *dt2)
{
	INT32 nRet = api_compare_day(dt1, dt2);
	if ( nRet != 0 ) return nRet;
	return api_compare_time_ext(dt1, dt2);
}

//*len>=0 when dt2>=dt1;*len<0 when dt2<dt1
void api_get_time_len(date_time* dt1,date_time* dt2,INT32* len)
{
	INT32 day =0;
	INT32 tmp=0;
	if(eit_compare_time(dt1, dt2)>=0)//dt2>dt1
	{
		day=relative_day(dt1, dt2);
		*len=3600*(24*day+dt2->hour)+60*dt2->min+dt2->sec-(3600*(dt1->hour)+60*dt1->min+dt1->sec);
	}
	else
	{
		day=relative_day(dt2, dt1);
		*len=-(3600*(24*day+dt1->hour)+60*dt1->min+dt1->sec-(3600*(dt2->hour)+60*dt2->min+dt2->sec));
	}
}

BOOL set_next_wakeup_datetime(TIMER_SET_CONTENT *timer)
{
	UINT16 next_yy;
	UINT8 next_mm, next_dd;
	UINT8 type;

	next_yy = timer->wakeup_year;
	next_mm = timer->wakeup_month;
	next_dd = timer->wakeup_day;

	type = 0xFF;

	if (timer->timer_mode == TIMER_MODE_ONCE)
	{
		timer->timer_mode = TIMER_MODE_OFF;
	}
	else if (timer->timer_mode == TIMER_MODE_DAILY)
	{
		next_dd += 1;
		type = 0;
	}
	else if (timer->timer_mode == TIMER_MODE_WEEKLY)
	{
		next_dd += 7;
		type = 0;
	}
	else if (timer->timer_mode == TIMER_MODE_MONTHLY)
	{
		next_mm += 1;
		type = 1;
	}
	else if (timer->timer_mode == TIMER_MODE_YEARLY)
	{
		next_yy += 1;
		type = 2;
	}

	if (type != 0xFF)
	{
		api_correct_yy_mm_dd(&next_yy, &next_mm, &next_dd, type);
		timer->wakeup_year = next_yy;
		timer->wakeup_month = next_mm;
		timer->wakeup_day = next_dd;

		return TRUE;
	}
	else
		return FALSE;

}


void update_yy_mm_dd(UINT16 *yy, UINT8 *mm, UINT8 *dd, UINT8 type)
{
	date_time dt;

	UINT16 year, cur_year;
	UINT8 month, day, mm_days, cur_month, cur_day;
	BOOL b;


	get_local_time(&dt);

	cur_year = dt.year;
	cur_month = dt.month;
	cur_day = dt.day;

	year =  *yy;
	month =  *mm;
	day =  *dd;

	while ( ( year < cur_year )
	        || ( year == cur_year && month < cur_month )
	        || ( year == cur_year && month == cur_month && day < cur_day )
	      )
	{

		if (type == TIMER_MODE_DAILY)
			day += 1;
		else if (type == TIMER_MODE_WEEKLY)
			day += 7;
		else if (type == TIMER_MODE_MONTHLY)
			month += 1;
		else
		//TIMER_MODE_YEARLY
			year += 1;


		/* For monthly */
		if (month > 12)
		{
			year += 1;
			month = 1;
		}

		mm_days = month_days[month - 1];
		if (month == 2 && api_check_is_leap_year(year))
			mm_days = 29;

		if (day > mm_days)
		{
			day -= mm_days;
			month += 1;
			if (month > 12)
			{
				month = 1;
				year += 1;
			}
		}
	}

	*yy = year;
	*mm = month;
	*dd = day;

}

/*****************************************
function: api_compare_timer
parameter:
timer1:[a,b], wakeup time on a, and duration time is (b-a)
timer2:[c,d], wakeup time on c, and duration time is (c-d)
return:
0 	- [a,b]=[c,d]
1 	- [a,b] ^[c,d] = NULL, b<c
2	- [a,b] ^[c,d] = NULL, d<a
-1 	- other
 ******************************************/
INT32 api_compare_timer(TIMER_SET_CONTENT *timer1, TIMER_SET_CONTENT *timer2)
{
	date_time time_a, time_b, time_c, time_d;

	MEMSET(&time_a, 0, sizeof(date_time));
	MEMSET(&time_b, 0, sizeof(date_time));
	MEMSET(&time_c, 0, sizeof(date_time));
	MEMSET(&time_d, 0, sizeof(date_time));

	time_a.year = time_b.year = timer1->wakeup_year;
	time_a.month = time_b.month = timer1->wakeup_month;
	time_a.day = time_b.day = timer1->wakeup_day;
	time_a.hour = timer1->wakeup_time / 3600;
	time_a.min = timer1->wakeup_time / 60 % 60;
	time_a.sec = timer1->wakeup_time % 60;

	time_b.hour = (timer1->wakeup_time + timer1->wakeup_duration_time) / 3600;
	time_b.min = (timer1->wakeup_time + timer1->wakeup_duration_time) / 60 % 60;
	time_b.sec = (timer1->wakeup_time + timer1->wakeup_duration_time)%60;

	if (time_b.hour > 23)
	{
		time_b.hour %= 24;
		time_b.day++;
		api_correct_yy_mm_dd(&time_b.year, &time_b.month, &time_b.day, 0);
	}

	time_c.year = time_d.year = timer2->wakeup_year;
	time_c.month = time_d.month = timer2->wakeup_month;
	time_c.day = time_d.day = timer2->wakeup_day;
	time_c.hour = timer2->wakeup_time / 3600;
	time_c.min = timer2->wakeup_time / 60 % 60;
	time_c.sec = timer2->wakeup_time % 60;

	time_d.hour = (timer2->wakeup_time + timer2->wakeup_duration_time) / 3600;
	time_d.min = (timer2->wakeup_time + timer2->wakeup_duration_time) / 60 % 60;
	time_d.sec = (timer2->wakeup_time + timer2->wakeup_duration_time) % 60;

	if (time_d.hour > 23)
	{
		time_d.hour %= 24;
		time_d.day++;
		api_correct_yy_mm_dd(&time_d.year, &time_d.month, &time_d.day, 0);
	}

	if (api_compare_day_time_ext(&time_a, &time_c) == 0 && api_compare_day_time_ext(&time_b, &time_d) == 0)
		return 0;

	if (api_compare_day_time_ext(&time_b, &time_c) <= 0)
		return 1;

	if (api_compare_day_time_ext(&time_a, &time_d) >= 0)
		return 2;

	return  - 1;
}

/*****************************************
function: api_compare_timer_time
parameter:
timer1:[a,b], wakeup time on a, and duration time is (b-a)
timer2:[c,d], wakeup time on c, and duration time is (c-d)
return:
0 	- [a,b]=[c,d]
1 	- [a,b] ^[c,d] = NULL, b<c
2	- [a,b] ^[c,d] = NULL, d<a
-1 	- other
 ******************************************/
INT32 api_compare_timer_time(TIMER_SET_CONTENT *timer1, TIMER_SET_CONTENT *timer2)
{
	date_time time_a, time_b, time_c, time_d;

	MEMSET(&time_a, 0, sizeof(date_time));
	MEMSET(&time_b, 0, sizeof(date_time));
	MEMSET(&time_c, 0, sizeof(date_time));
	MEMSET(&time_d, 0, sizeof(date_time));

	time_a.hour = timer1->wakeup_time / 3600;
	time_a.min = timer1->wakeup_time / 60 % 60;

	time_b.hour = (timer1->wakeup_time + timer1->wakeup_duration_time) / 3600;
	time_b.min = (timer1->wakeup_time + timer1->wakeup_duration_time) / 60 % 60;

	if (time_b.hour > 23)
	{
		time_b.hour %= 24;
		time_b.day++;
	}

	time_c.hour = timer2->wakeup_time / 3600;
	time_c.min = timer2->wakeup_time / 60 % 60;

	time_d.hour = (timer2->wakeup_time + timer2->wakeup_duration_time) / 3600;
	time_d.min = (timer2->wakeup_time + timer2->wakeup_duration_time) / 60 % 60;

	if (time_d.hour > 23)
	{
		time_d.hour %= 24;
		time_d.day++;
	}

	if (api_compare_day_time(&time_a, &time_c) == 0 && api_compare_day_time(&time_b, &time_d) == 0)
		return 0;

	if (api_compare_day_time(&time_b, &time_c) <= 0)
		return 1;

	if (api_compare_day_time(&time_a, &time_d) >= 0)
		return 2;

	return  - 1;
}

/*****************************************
function: api_check_timer
parameter:
timer1:[a,b], wakeup time on a, and duration time is (b-a)
timer2:[c,d], wakeup time on c, and duration time is (c-d)
return:
0 	- [a,b] ^ [c,d] != NULL
1 	- [a,b] ^ [c,d] = NULL
 ******************************************/
INT32 api_check_timer(TIMER_SET_CONTENT *timer1, TIMER_SET_CONTENT *timer2)
{
	TIMER_SET_CONTENT timer_1;
	INT32 ret;
	UINT8 next_mm, next_dd;
	UINT16 next_yy;
	UINT8 type;

	ret = api_compare_timer(timer1, timer2);

	if ( ret <= 0 ) return 0;

	if ( ret == 2 ) return 1;

	timer_1 =  *timer1;
	next_yy = timer1->wakeup_year;
	next_mm = timer1->wakeup_month;
	next_dd = timer1->wakeup_day;
	type = 0xFF;

	switch (timer1->timer_mode)
	{
		case TIMER_MODE_OFF:
		case TIMER_MODE_ONCE:
			return 1;
		case TIMER_MODE_DAILY:
COMPARE_TIME_DURATION:
			if ( api_compare_timer_time ( timer1, timer2 ) > 0 )
				return 1;
			else
				return 0;
		case TIMER_MODE_WEEKLY:
			next_dd += 7;
			type = 0;
			break;
		case TIMER_MODE_MONTHLY:
			if (timer2->timer_mode == TIMER_MODE_MONTHLY)
			{
				if (timer1->wakeup_day >= 28 && timer2->wakeup_day >= 28)
				{
					goto COMPARE_TIME_DURATION;
				}
			}
			else if (timer2->timer_mode == TIMER_MODE_YEARLY)
			{
				if ( timer1->wakeup_day > 28
				        && timer2->wakeup_day >= 28
				        && timer2->wakeup_day <= timer1->wakeup_day )
				{
					goto COMPARE_TIME_DURATION;
				}
			}
			next_mm += 1;
			type = 1;
			break;
		case TIMER_MODE_YEARLY:
			if (timer2->timer_mode == TIMER_MODE_MONTHLY)
			{
				do
				{
					next_yy += 1;
					type = 2;
					api_correct_yy_mm_dd(&next_yy, &next_mm, &next_dd, type);
					timer_1.wakeup_year = next_yy;
					timer_1.wakeup_month = next_mm;
					timer_1.wakeup_day = next_dd;
				}
				while (api_compare_timer(&timer_1, timer2) == 1);

				return api_check_timer ( timer2, &timer_1 );
			}
			next_yy += 1;
			type = 2;
			break;
	}

	if (timer2->timer_mode == TIMER_MODE_DAILY)
	{
		goto COMPARE_TIME_DURATION;
	}

	if (type != 0xFF)
	{
		api_correct_yy_mm_dd(&next_yy, &next_mm, &next_dd, type);
		timer_1.wakeup_year = next_yy;
		timer_1.wakeup_month = next_mm;
		timer_1.wakeup_day = next_dd;
	}

	return api_check_timer(&timer_1, timer2);
}

BOOL api_timers_running(void)
{
	UINT16 i;
	SYSTEM_DATA *sys_data;
	TIMER_SET_T* timer_set;
	TIMER_SET_CONTENT* timer;
	
	sys_data = sys_data_get();
	timer_set = &(sys_data->timer_set);
	for(i=0;i<MAX_TIMER_NUM;i++)
	{
		timer = &(timer_set->TimerContent[i]);
		if(timer->wakeup_state == TIMER_STATE_RUNING)
			return TRUE;
	}

	return FALSE;
}

UINT8 api_get_first_timer(void)
{
	UINT32 i;
	SYSTEM_DATA *sys_data;
	TIMER_SET_T *timer_set;
	TIMER_SET_CONTENT *timer;
	date_time dt, firsttime, timertime;

	UINT8 timer_idx = 0xff;


	firsttime.sec = 0;

	get_local_time(&dt);
	sys_data = sys_data_get();
	timer_set = &sys_data->timer_set;

	//    dt.sec = timertime.sec = 0; /* Ignore second*/

	for (i = 0; i < MAX_TIMER_NUM; i++)
	{
		timer = &timer_set->TimerContent[i];

		if (timer->timer_mode == TIMER_MODE_OFF)
			continue;
		if ( timer->timer_service != TIMER_SERVICE_CHANNEL
		        && timer->timer_service != TIMER_SERVICE_MESSAGE
		   )
			continue;
		if (timer->wakeup_state == TIMER_STATE_RUNING)
		{
			if (timer->timer_mode == TIMER_MODE_ONCE)
			{
				timer->timer_mode = TIMER_MODE_OFF;
				continue;
			}
			else
				set_next_wakeup_datetime(timer);
		}


		timertime.year = timer->wakeup_year;
		timertime.month = timer->wakeup_month;
		timertime.day = timer->wakeup_day;
		timertime.hour = timer->wakeup_time / 3600;
		timertime.min = (timer->wakeup_time) / 60 % 60;
		timertime.sec = timer->wakeup_time % 60;

		if (api_compare_day_time_ext(&timertime, &dt) < 0)
		{
			set_next_wakeup_datetime(timer);

			timertime.year = timer->wakeup_year;
			timertime.month = timer->wakeup_month;
			timertime.day = timer->wakeup_day;
			timertime.hour = timer->wakeup_time / 3600;
			timertime.min = timer->wakeup_time / 60 % 60;
			timertime.sec = timer->wakeup_time % 60;
		}

		if (api_compare_day_time_ext(&timertime, &dt) < 0)
			continue;


		if (timer_idx == 0xff)
		{
			firsttime = timertime;
			timer_idx = i;
		}
		else
		{
			if (api_compare_day_time_ext(&timertime, &firsttime) < 0)
			{
				firsttime = timertime;
				timer_idx = i;
			}
		}

	}

	return timer_idx;
}

void api_timers_proc(void)
{
	int i;

	SYSTEM_DATA *sys_data;
	TIMER_SET_T *timer_set;
	TIMER_SET_CONTENT *timer;
	date_time dt, start_dt, end_dt,warning_dt;
	UINT32 localtime;
	UINT16 next_yy;
	UINT8 next_mm, next_dd;
	P_NODE p_node;

	get_local_time(&dt);
	sys_data = sys_data_get();
	timer_set = &sys_data->timer_set;

	for (i = 0; i < MAX_TIMER_NUM/*+AUTO_START_TIMER_NUM*/; i++)
	{
		timer = &timer_set->TimerContent[i];
		/*Auto power up timer*/		
		/*if(i==36&&sys_data->auto_power_on!=0)
		{
			localtime=dt.hour*3600+dt.min*60+dt.sec;
			if((localtime<timer->wakeup_time+3)&&(localtime>timer->wakeup_time-3)&&system_state == SYS_STATE_POWER_OFF)
				power_on_process();//ap_send_msg(CTRL_MSG_SUBTYPE_CMD_TIMER_AUTOSTART, 36, TRUE);
			continue;
		}
		//Auto power down timer
		if(i==37&&sys_data->auto_power_off!=0)\
		{
			localtime=dt.hour*3600+dt.min*60+dt.sec;
			if((localtime<timer->wakeup_time+3)&&(localtime>timer->wakeup_time-3))
			{
				if(system_state == SYS_STATE_DATACAST)
					enr_send_msg(CTRL_MSG_SUBTYPE_CMD_TIMER_AUTOSHUTDOWN, 37, TRUE);
				else
					ap_send_msg(CTRL_MSG_SUBTYPE_CMD_TIMER_AUTOSHUTDOWN, 37, TRUE);
			}
			continue;
		}
		*/
		if (timer->timer_mode == TIMER_MODE_OFF)
		{
			timer->wakeup_state = TIMER_STATE_READY;
			continue;
		}


		if (!(timer->timer_service <= TIMER_SERVICE_MESSAGE))
		{
			timer->timer_mode = TIMER_MODE_OFF;
			timer->wakeup_state = TIMER_STATE_READY;
			continue;
		}

		start_dt.year = timer->wakeup_year;
		start_dt.month = timer->wakeup_month;
		start_dt.day = timer->wakeup_day;
		start_dt.hour = timer->wakeup_time / 3600;
		start_dt.min = timer->wakeup_time / 60 % 60;
		start_dt.sec = timer->wakeup_time % 60;

		convert_time_by_offset2(&warning_dt,&start_dt,0,-1,0);

		/* Timer reach to wakeup time */
		if((api_compare_day_time_ext(&warning_dt,&dt) <= 0)&&(api_compare_day_time_ext(&dt,&start_dt) <= 0))
		{
			if (timer->wakeup_state == TIMER_STATE_RUNING)
			{
				if (system_state == SYS_STATE_POWER_OFF)
				{
					set_next_wakeup_datetime(timer);
					timer->wakeup_state = TIMER_STATE_READY;
				}
				continue;
			}
			else
			 /* Wakeup a timer */
			{
				/*For message or channel of duration is zero,immediatelly update next wakeup date*/
				if ( timer->timer_service == TIMER_SERVICE_MESSAGE
				        || timer->wakeup_duration_time == 0 )
				{
					if (system_state != SYS_STATE_POWER_OFF)
					 /* To avoid not execuate the timer when system wakeup */
						set_next_wakeup_datetime(timer);
				}

//1 Dispatch off in timer callback,so can not call the get_prog_by_id(include mutex,semaphore or flag)
//2 Check the prog can be found or not in the ctrl task(when receive the wakeup msg.)
#if 0
				/* If it's channel timer and specified channel not exit, turn off the timer */
				if (TIMER_SERVICE_CHANNEL == timer->timer_service)
				{
					if (get_prog_by_id(timer->wakeup_channel, &p_node) != DB_SUCCES)
					{
						/* The channel not exit, turn of the timer*/
						timer->timer_mode = TIMER_MODE_OFF;
						continue;
					}
				}
#endif
				if (system_state == SYS_STATE_POWER_OFF)
				{
					sys_data->wakeup_timer = i + 1;
					power_on_process();
				}
				else
				{
					if(i>=32)
					{
						if(system_state == SYS_STATE_DATACAST)
							generic_dbcast_message_send(OSAL_INVALID_ID,CTRL_MSG_SUBTYPE_STATUS_NVOD_TIMER_WAKEUP, i, FALSE);
						else
							ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_NVOD_TIMER_WAKEUP, i, FALSE);
					}
					else
					{
						if(system_state == SYS_STATE_DATACAST)
							generic_dbcast_message_send(OSAL_INVALID_ID,CTRL_MSG_SUBTYPE_CMD_TIMER_WAKEUP, i, FALSE);
						else
							ap_send_msg(CTRL_MSG_SUBTYPE_CMD_TIMER_WAKEUP, i, FALSE);
					}
				}

				/* Make the timer a running status. */
				if (timer->timer_mode != TIMER_MODE_OFF)
					timer->wakeup_state = TIMER_STATE_RUNING;
			}
		}
		else
		 /* Timer not reach to wakeup time */
		{
			/* Message timer or duration==0 timer in running state will not hold other than the wakeup minute */

			if (timer->wakeup_state == TIMER_STATE_RUNING)
			{
				if ( timer->timer_service == TIMER_SERVICE_MESSAGE
				        || timer->wakeup_duration_time == 0 )
					timer->wakeup_state = TIMER_STATE_READY;
			}


			/* If a timer is running. */
			if (timer->wakeup_state == TIMER_STATE_RUNING)
			{
				/* Get timer expire time.*/
				convert_time_by_offset ( &end_dt, &start_dt,
				                         timer->wakeup_duration_time / 3600, timer->wakeup_duration_time /60 % 60 );

				if (api_compare_day_time_ext(&end_dt, &dt) <= 0)
				 /* Reach to end time */
				{
					timer->wakeup_state = TIMER_STATE_READY;
					set_next_wakeup_datetime(timer);

					/* Stop the timer immediatelly */
					if (system_state != SYS_STATE_POWER_OFF)
					{
						//ap_send_msg(CTRL_MSG_SUBTYPE_CMD_TIMER_EXPIRE, i, FALSE);
					}
				}
			}
			else
			 /*The timer is not running*/
			{
				if (api_compare_day_time(&start_dt, &dt) < 0)
				{
					/*Check timers date&time setting*/
					/* If date date&time is previous that current date&time, update date&time */
					set_next_wakeup_datetime(timer);
				}
			}
		}
	}

	/*for special auto start timer only*/
	//for(i=MAX_TIMER_NUM;i<MAX_TIMER_NUM+2;i++)
	//{
	/*timer = &timer_set->TimerContent[36];
	if(timer->timer_mode!=TIMER_MODE_OFF&&sys_data->auto_power_on!=0){
		localtime=dt.hour*3600+dt.min*60+dt.sec;
		if(localtime==timer->wakeup_time)
			power_on_process();//ap_send_msg(CTRL_MSG_SUBTYPE_CMD_TIMER_AUTOSTART, 36, TRUE);
	}
	
	timer = &timer_set->TimerContent[37];
	if(timer->timer_mode!=TIMER_MODE_OFF&&sys_data->auto_power_off!=0){
		localtime=dt.hour*3600+dt.min*60+dt.sec;
		if(localtime==timer->wakeup_time)
			ap_send_msg(CTRL_MSG_SUBTYPE_CMD_TIMER_AUTOSHUTDOWN, 37, TRUE);
	}*/
	//}
	
}

UINT16 Hex2Dec(UINT16 x)
{
	//0x0000 - 0x0799    =>  100 - 899
	UINT16 y;
	UINT8 units = (x &0x000f);
	UINT8 tens = (x &0x00f0) >> 4;
	UINT8 hunds = (x &0x0f00) >> 8;
	if (tens > 9 || units > 9)
	{
		return 0xffff;
	}
	y = units + tens * 10+hunds * 100;
	if (y < 100)
		y += 800;
	if (y > 899)
		y = 0xffff;
	return y;
}







/**************************************************************
    select channel audio language
**************************************************************/
 #define MAX_LANG_GROUP_SIZE    6


static char *iso639_lang_multicode[][2] = 
{
    {"fre","fra"},
    {"ger","deu"},
    {"spa","esl"}
};

static UINT8 global_lang_group[MAX_LANG_GROUP_SIZE][4];
static UINT32 global_lang_num;


void get_ch_pids(P_NODE* p_node,UINT16* audioPID,UINT16* ttxPID,UINT16* subtPID,UINT32* audioidx)
{
	UINT32 i,j,k;
	UINT8 lang_in_db[MAX_AUDIO_CNT][4];
    	UINT32 audio_cnt;
    	UINT32 iso639_cnt = sizeof(iso639_lang_multicode)/sizeof(iso639_lang_multicode[0]);

    
	MEMSET(lang_in_db,0,sizeof(lang_in_db));

    audio_cnt = p_node->audio_count;
    if(audio_cnt>MAX_AUDIO_CNT)
        audio_cnt = MAX_AUDIO_CNT;

    if(audio_cnt == 0)
    {
        *audioPID = 0x1FFF;
        *audioidx = 0;
    }
    else if(p_node->cur_audio<audio_cnt)
    {
        *audioPID = p_node->audio_pid[p_node->cur_audio];
        *audioidx = p_node->cur_audio;
    }
    else
    {
        *audioPID = p_node->audio_pid[0];
        *audioidx = 0;
    }

    /* If this channel's audio PID has not been selected by user,then decide
      the audio PID by audio language setting */
    if( !p_node->audio_select)
    {
    	for(i=0;i<audio_cnt;i++)
    		get_audio_lang3b((UINT8 *)&p_node->audio_lang[i],lang_in_db[i]);		
    	
    	for(i=0;i<global_lang_num;i++)
    	{
    		for(j=0;j<audio_cnt;j++)
    		{
    			if( STRCMP(global_lang_group[i],lang_in_db[j]) == 0)
    			{
    				*audioPID = p_node->audio_pid[j];
                    *audioidx = j;
                    goto GET_AUDIO_PID_DONE;
    			}
                else
                {
                    for(k=0;k<iso639_cnt;k++)
                    {
                        if(STRCMP(iso639_lang_multicode[k][0],global_lang_group[i]) == 0
                            && STRCMP(iso639_lang_multicode[k][1],lang_in_db[j]) == 0)
                        {
                            *audioPID = p_node->audio_pid[j];
                            *audioidx = j;
                            goto GET_AUDIO_PID_DONE;
                        }
                    }
                }
    		}
    	}
    }

GET_AUDIO_PID_DONE:    

    *ttxPID     = p_node->teletext_pid;
    *subtPID    = p_node->subtitle_pid;
}


UINT32 api_set_audio_language(BYTE* szLangGroup,UINT32 uLangNum)
{
	UINT32 i,j;

	MEMSET(global_lang_group,0,sizeof(global_lang_group));
	if (uLangNum > MAX_LANG_GROUP_SIZE)
		uLangNum = MAX_LANG_GROUP_SIZE;
	for(i=0; i<uLangNum; i++)
	{
		for (j=0;j<3;j++)
		{
			global_lang_group[i][j] = szLangGroup[i*4+j];
		}
	}

	global_lang_num = uLangNum;
	return uLangNum;
}



UINT32 api_get_recently_timer()
 {
 	int i;

 	SYSTEM_DATA* sys_data;
 	TIMER_SET_T* timer_set;
 	TIMER_SET_CONTENT* timer;

 	UINT16  yy = 0xFFFF;
    UINT32 hm =0xFFFFFFFF;
 	UINT8  mm = 0xFF, dd = 0xFF;
 	UINT32	wakeup_time;
	UINT16  hour = 0;
	UINT16  minter = 0;
	UINT16  sec = 0;

 	sys_data = sys_data_get();
 	timer_set = &sys_data->timer_set;

 	for(i=0;i<MAX_TIMER_NUM;i++)
 	{
 		timer = &timer_set->TimerContent[i];
 		// if(timer->timer_mode == TIMER_MODE_OFF)
 		if(timer->timer_mode == TIMER_MODE_OFF )
 		{
 			timer->wakeup_state = TIMER_STATE_READY;
 			continue;
 		}

/* 		if(TIMER_STATE_RUNING == timer->wakeup_state)
 		{
 			set_next_wakeup_datetime(timer);
 			timer->wakeup_state = TIMER_STATE_READY;
 		}
*/ 		if( (timer->timer_mode != TIMER_MODE_OFF) && ((timer->wakeup_year < yy) ||
 			((timer->wakeup_year == yy) && (timer->wakeup_month < mm)) ||
 			((timer->wakeup_year == yy) && (timer->wakeup_month == mm) && (timer->wakeup_day < dd)) ||
 			((timer->wakeup_year == yy) && (timer->wakeup_month == mm) && (timer->wakeup_day == dd) && (timer->wakeup_time < hm))))
 		{
 			yy = timer->wakeup_year;
 			mm = timer->wakeup_month;
 			dd = timer->wakeup_day;
 			hm = timer->wakeup_time;
 			sys_data->wakeup_timer = i+1; // base 1

			// find the first timer
			//break;
 		}
 	}
 	if ((hm == 0xFFFF) && (dd == 0xFF) && (mm == 0xFF) && (yy == 0xFFFF))
 	{
 		wakeup_time = 0;
 		sys_data->wakeup_timer = 0x0;
 	}
 	else
 	{
		hour = hm/3600;
		minter = (hm-hour*3600)/60;
		sec = hm -hour*3600 -minter*60;
		wakeup_time = (sec &0x3F) | ((minter & 0x3F)<<6) | ((hour & 0x1F)<<12) | ((dd & 0x1F)<<17)
 			| ((mm & 0xF) << 22) |  (((yy % 100) & 0x3F)<<26);
  		//set_next_wakeup_datetime(&timer_set->TimerContent[sys_data->wakeup_timer-1]);
	}
 	sys_data_save(1);
 	return wakeup_time;
 }

void api_timer_resort(void)
{
    INT8 timer_num;
    UINT8 i,j;
    date_time dt1,dt2;
    UINT32 wake_time1,wake_time2;
    INT8 Result=0;    
    TIMER_SET_CONTENT timer_temp;
    SYSTEM_DATA *sys_data = sys_data_get();    
    timer_num = find_available_timer() - 1;
    
    for (i=0; i<timer_num; i++)    
    {
           memset(&timer_temp,0,sizeof(TIMER_SET_CONTENT));
	    memcpy(&timer_temp,&sys_data->timer_set.TimerContent[i],sizeof(TIMER_SET_CONTENT));
           memset(&dt1,0,sizeof(date_time));
           dt1.year = sys_data->timer_set.TimerContent[i].wakeup_year;
	    dt1.month = sys_data->timer_set.TimerContent[i].wakeup_month;
	    dt1.day = sys_data->timer_set.TimerContent[i].wakeup_day;
	    wake_time1 = sys_data->timer_set.TimerContent[i].wakeup_time;
           for (j=i+1; j<timer_num; j++)
           {
                  memset(&dt2,0,sizeof(date_time));
		    dt2.year = sys_data->timer_set.TimerContent[j].wakeup_year;
		    dt2.month = sys_data->timer_set.TimerContent[j].wakeup_month;
		    dt2.day = sys_data->timer_set.TimerContent[j].wakeup_day;
		    wake_time2 = sys_data->timer_set.TimerContent[j].wakeup_time;
		    Result = api_compare_day(&dt1, &dt2);
		    if((Result==1) || ((Result==0)&&(wake_time1>wake_time2)))
		    { 
                         memcpy(&sys_data->timer_set.TimerContent[i],&sys_data->timer_set.TimerContent[j],sizeof(TIMER_SET_CONTENT));
			    memcpy(&sys_data->timer_set.TimerContent[j],&timer_temp,sizeof(TIMER_SET_CONTENT));
		    }                 
           }		 
    } 
    sys_data_save(TRUE);
}

UINT8 api_cur_channel_is_scrambled(UINT8 *type)
{
	UINT8 ret = 0, temp = 0;
	UINT8 scramble_type = 0xff;
	UINT16 i;
	struct dmx_device *dmx_dev = g_dmx_dev;
//	struct dmx_device *dmx_dev = (struct dmx_device*)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);

	osal_task_sleep(30);
	temp = dmx_io_control(dmx_dev, IS_AV_SOURCE_SCRAMBLED, (UINT32)(&scramble_type));
	if (temp == RET_SUCCESS)
	{
		ret = 1;
		*type = scramble_type;
	}
	else
	{
		ret = 0;
	}

	return ret;
}

void api_hwsw_Info_get(HW_SW_INFO *info)
{
	STB_INFO_DATA *stb_data;

	if (info == NULL)
		return ;

	stb_data= stb_info_data_get();
	get_stb_hwinfo(&info->oui, &info->hw_model, &info->hw_version);
	info->sw_model = stb_data->sw_model;
	info->sw_version = stb_data->sw_version;
}

void api_sn_get(UINT8 *sn)
{
	if (sn != NULL)
	{
		get_stb_serial(sn);
	}
}

void api_mac_addr_get(UINT8 *mac)
{
	if (mac != NULL)
	{
		get_stb_mac(mac);
	}

}

void api_fill_black_screen()
{
	struct VDec_StatusInfo CurStatus;
	UINT8 resume_flag = FALSE;
	RET_CODE ret_code;

	struct vdec_device * vdec_handle = (struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0);

	vdec_io_control(vdec_handle, VDEC_IO_GET_STATUS, (UINT32) &CurStatus);

	if(CurStatus.uCurStatus == VDEC_STOPPED || (CurStatus.uCurStatus != VDEC_STOPPED && !CurStatus.uFirstPicShowed))
	{
		if(CurStatus.uCurStatus != VDEC_STOPPED)
		{
			resume_flag = TRUE;
			ret_code = vdec_stop(vdec_handle,TRUE,FALSE);
		}

		struct YCbCrColor tColor;
		tColor.uY = 0x10;
		tColor.uCb = 0x80;
		tColor.uCr = 0x80;
		vdec_io_control(vdec_handle, VDEC_IO_FILL_FRM, (UINT32)(&tColor));
		vpo_win_onoff((struct vpo_device*)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS), TRUE);
		if(resume_flag)
		{
			ret_code = vdec_start(vdec_handle);
		}
	}
}

