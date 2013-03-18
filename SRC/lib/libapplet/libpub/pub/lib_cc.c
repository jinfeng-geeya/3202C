#include <sys_config.h>
#include <basic_types.h>
#include <retcode.h>
#include <mediatypes.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <hld/nim/nim_dev.h>
#include <hld/nim/nim.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/deca/deca_dev.h>
#include <hld/deca/deca.h>
#include <hld/snd/snd_dev.h>
#include <hld/snd/snd.h>
#include <hld/decv/vdec_driver.h>
#include <hld/dis/vpo.h>



#include <api/libpub/lib_pub.h>
#include <api/libpub/lib_hde.h>
#include <api/libpub/lib_frontend.h>
#include <api/libpub/lib_device_manage.h>

#include <bus/tsi/tsi.h>
#include<api/libsi/lib_epg.h>
#include <api/libsi/si_service_type.h>
#include <api/libsi/si_tdt.h>
#include <api/libsi/lib_nvod.h>
#include <api/libsi/sie_monitor.h>
#include <api/libsi/ts_monitor.h>
#ifdef CC_USE_TSG_PLAYER
#include <api/libpvr/lib_pvr.h>
#endif

/*******************************************************
* macro define
********************************************************/
#define CC_PRINTF   PRINTF//libc_printf
//#define CHANNEL_CHANGE_VIDEO_FAST_SHOW


/*******************************************************
* extern declare
********************************************************/
extern struct pub_module_config libpub_config;

/*******************************************************
* struct and variable
********************************************************/
extern UINT16 l_cc_sema_id;
#define ENTER_CC_API()  osal_semaphore_capture(l_cc_sema_id, TMO_FEVR)
#define LEAVE_CC_API()  osal_semaphore_release(l_cc_sema_id)

//ci device & service data
//static struct ci_device_list ci_dev_list;
//static struct ci_service_info ci_service;


static INT32 cc_cmd_cnt = 0;		//play channel cmd counter for CI

#ifdef MULTI_CAS
static BOOL mcas_xpond_stoped = FALSE;//a temp solution to make sure api_mcas_start_transponder()
										//is called pair with api_mcas_stop_transponder()
#endif
/********************************************************
*internal functions
/*******************************************************/
//#ifdef CHCHG_TICK_PRINTF
UINT32 cc_play_tick = 0;
UINT32 vdec_start_tick = 0;
extern UINT32 CHCHG_TICK_PRINTF_time;
//先用数组记录换台时间，最后再一次打印出来，因为打印信息很影响速度
/*enum
{
	CC_CHECK_NIM = 1,
	CC_PRE_PLAY,
	CC_CLOSE_VPO,
	CC_STOP_AUDIO,
	CC_STOP_VIDEO,
	CC_SELECT_DECV,
	CC_STOP_DMX,
	CC_PRE_TUNER,
	CC_SET_XPOND,
	CC_POST_TUNER,
	CC_VPO_CHANCHNG,
	CC_START_DMX,
	CC_START_VIDEO,
	CC_START_AUDIO,
	CC_STEP_NUM
};
static struct
{
	UINT32 start_tick;
	UINT32 end_tick;
	UINT32 step;
} g_cc_play_tick[CC_STEP_NUM];*/

extern char g_cc_step_name[][23];

extern UINT32 g_cc_play_step;
static UINT32 g_cc_play_last_tick;
static void dump_cc_steps()
{
#ifdef CHCHG_TICK_PRINTF
	int i;
	printf("Dump CC tick:\n");
	printf("%-23s%12s%12s%12s\n","Step","Start","End","Used");
	for (i=0;i<g_cc_play_step;i++)
	{
		printf("%02d-%-20s%12u%12u%12u\n",i,g_cc_step_name[g_cc_play_tick[i].step],
					g_cc_play_tick[i].start_tick,g_cc_play_tick[i].end_tick,
					g_cc_play_tick[i].end_tick-g_cc_play_tick[i].start_tick);
	}
	printf("Total: %4u\n",g_cc_play_tick[i-1].end_tick-g_cc_play_tick[0].start_tick);
#endif
}




/* Set hdmi audio output when cc play */
/* Peter Feng, 2008.06.13 */
static void set_hdmi_audio_output(BOOL enable)
{
#ifdef SUPPORT_HDMI_CAT
	// call CAT driver to enable/disable audio
	CC_PRINTF("HDMI --- Play channel enable/disable audio is %d.\n", enable);
	HDMI_CAT6611_Enable_AudioOutput(enable);
#endif

#ifdef SUPPORT_HDMI_CAT6613
	// call CAT driver to enable/disable audio
	CC_PRINTF("HDMI --- Play channel enable/disable audio is %d.\n", enable);
	HDMITX_CAT6613_Enable_AudioOutput(enable);
#endif
}

/* Set Audio Channel Mode: Left, Right, Mono, or Stereo */
static void set_audio_channel(struct snd_device *snd_dev, UINT16 audio_ch)
{
    	enum SndDupChannel chan;

	switch(audio_ch)
	{
	case AUDIO_CH_L:
		chan = SND_DUP_L;
		break;
	case AUDIO_CH_R:
        	chan = SND_DUP_R;
        	break;
       case AUDIO_CH_MONO:
        	chan = SND_DUP_MONO;
        	break;
       case AUDIO_CH_STEREO:
       default:
        	chan = SND_DUP_NONE;
        	break;
	}
	snd_set_duplicate(snd_dev, chan);
}

BOOL enable_blk_screen = TRUE;
void enable_fill_blk_screen(BOOL b_eanble)
{
	enable_blk_screen = b_eanble;
}

/* According to cmd_bits, Play All CC Command */
static INT32 cc_driver_act(UINT32 cmd_bits, struct cc_es_info *es, struct cc_device_list *dev_list, struct ft_frontend *ft)
{
	INT32 ret;
   	struct io_param io_parameter;		
	UINT16 pid_list[4];
	UINT16 prog_pos = 0xffff;		
#ifdef CHCHG_TICK_PRINTF
	UINT32 temp_tick = 0;
#endif

#ifdef DUAL_VIDEO_OUTPUT
	struct vpo_device *pDisDevice_O = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 1);
#endif
	BOOL close_vpo = FALSE;
#if 0//#ifdef CHCHG_TICK_PRINTF
	if(cc_play_tick)
	{
		temp_tick = osal_get_tick();
		libc_printf("####CHCHG %s():line%d start driver act, tick used=%d\n", __FUNCTION__,__LINE__, temp_tick-cc_play_tick);
		cc_play_tick = temp_tick;
	}
#endif
	MEMSET(&io_parameter, 0, sizeof(io_parameter));

	if(es->background_play == 1) //not to operate when start backgound play for dual-record timer
	{
		// set front for backgound play to fix BUG34959

		if ((cmd_bits & CC_CMD_SET_XPOND) && ft)
		{
			CC_PRINTF("set nim for (%d, %d, %d)\n", es->sat_id, es->tp_id, es->prog_id);
			frontend_set_nim(ft->nim, &ft->antenna, &ft->xpond, 1);
		}
		return 0;
	}
	
	//Close Vpo
	CC_PRINTF("CC Driver Act 1 --- Close VPO\n");
    	if(cmd_bits&CC_CMD_CLOSE_VPO)
    	{
    		if(dev_list->vpo)
		{
			g_cc_play_tick[g_cc_play_step].step = CC_CLOSE_VPO;
			g_cc_play_tick[g_cc_play_step].start_tick = osal_get_tick();
			//Choose which hardware layer to close: Main Layer or PIP Layer
			UINT32 vpo_layer = es->close_vpo_layer;
           	 	if((es->close_vpo_layer != 0) && (es->close_vpo_layer <= 2))
           	 	{
				vpo_ioctl(dev_list->vpo, VPO_IO_CHOOSE_HW_LAYER, (UINT32)&vpo_layer);
           	 	}
            		ret = vpo_win_onoff(dev_list->vpo, FALSE);
			g_cc_play_tick[g_cc_play_step++].end_tick = osal_get_tick();
     		}
#ifdef DUAL_VIDEO_OUTPUT
			if(pDisDevice_O)
				vpo_win_onoff(pDisDevice_O, FALSE);	
#endif
#if 0//#ifdef CHCHG_TICK_PRINTF
		if(cc_play_tick)
		{
			temp_tick = osal_get_tick();
			libc_printf("####CHCHG %s():line%d vpo closed, tick used=%d\n", __FUNCTION__,__LINE__, temp_tick-cc_play_tick);
			cc_play_tick = temp_tick;
		}
#endif
	}	

#if (SYS_MAIN_BOARD != BOARD_DB_M3603_01V01 && SYS_MAIN_BOARD != BOARD_DB_M3383_01V01 \
	&& SYS_MAIN_BOARD != BOARD_DB_M3603_02V01&& SYS_MAIN_BOARD !=BOARD_M3383_UM00_01V01)
   	//Stop Deca Immediately
   	CC_PRINTF("CC Driver Act 2 --- Stop Deca\n");
    	if((cmd_bits & CC_CMD_STOP_AUDIO) && (es->audio_no_act == 0))
    	{
    		if(dev_list->deca)
        	{
			g_cc_play_tick[g_cc_play_step].step = CC_STOP_AUDIO;
			g_cc_play_tick[g_cc_play_step].start_tick = osal_get_tick();
        		ret = deca_stop(dev_list->deca, 0, ADEC_STOP_IMM);
            		CC_PRINTF("%s(): deca stop ret = %d\n", __FUNCTION__, ret);
			g_cc_play_tick[g_cc_play_step++].end_tick = osal_get_tick();
		}
#if 0//#ifdef CHCHG_TICK_PRINTF
		if(cc_play_tick)
		{
			temp_tick = osal_get_tick();
			libc_printf("####CHCHG %s():line%d deca stop, tick used=%d\n", __FUNCTION__,__LINE__, temp_tick-cc_play_tick);
			cc_play_tick = temp_tick;
		}
#endif
	}
#endif	
    
    	//Stop Vdec
    	CC_PRINTF("CC Driver Act 3 --- Stop Vdec\n");
    	if(cmd_bits & CC_CMD_STOP_VIDEO)
    	{

			if(dev_list->vdec_stop == NULL)
				dev_list->vdec_stop = dev_list->vdec;
			if(cmd_bits&CC_CMD_CLOSE_VPO)
				close_vpo = TRUE;
        	if(dev_list->vdec_stop)
        	{    
			g_cc_play_tick[g_cc_play_step].step = CC_STOP_VIDEO;
			g_cc_play_tick[g_cc_play_step].start_tick = osal_get_tick();
       		ret = vdec_stop(dev_list->vdec_stop, close_vpo, FALSE);
	            	CC_PRINTF("%s(): vdec stop ret = %d\n", __FUNCTION__, ret);
			g_cc_play_tick[g_cc_play_step++].end_tick = osal_get_tick();
	       }
			
    		if(dev_list->vdec_hdtv)
			{
		    	if(((es->v_pid) & H264_VIDEO_PID_FLAG) == H264_VIDEO_PID_FLAG)
		    	{
					dev_list->vdec = dev_list->vdec_hdtv;
					h264_decoder_select(1, hde_get_mode() == VIEW_MODE_PREVIEW);
		    	}
		    	else
		    	{
			    	h264_decoder_select(0, hde_get_mode() == VIEW_MODE_PREVIEW);
		    	}
			}
#if 0//#ifdef CHCHG_TICK_PRINTF
			if(cc_play_tick)
			{
				temp_tick = osal_get_tick();
				libc_printf("####CHCHG %s():line%d vdec stop, tick used=%d\n", __FUNCTION__,__LINE__, temp_tick-cc_play_tick);
				cc_play_tick = temp_tick;
			}
#endif
		}		

    	//Pause Vdec
    	CC_PRINTF("CC Driver Act 3-1 --- pause Vdec\n");
    	if(cmd_bits & CC_CMD_PAUSE_VIDEO)
    	{
    		if(dev_list->vdec_hdtv)
			{
		    	if(((es->v_pid) & H264_VIDEO_PID_FLAG) == H264_VIDEO_PID_FLAG)
		    	{
					dev_list->vdec = dev_list->vdec_hdtv;
					h264_decoder_select(1, hde_get_mode() == VIEW_MODE_PREVIEW);
		    	}
		    	else
		    	{
			    	h264_decoder_select(0, hde_get_mode() == VIEW_MODE_PREVIEW);
		    	}
			}
			
        	if(dev_list->vdec)
        	{ 
				vdec_io_control(dev_list->vdec, VDEC_IO_KEEP_INPUT_PATH_INFO, TRUE);
        		ret = vdec_stop(dev_list->vdec, FALSE, FALSE);
	            	CC_PRINTF("%s(): vdec stop ret = %d\n", __FUNCTION__, ret);
	       }
		}
    
     	//Stop Dmx Audio and Video
     	CC_PRINTF("CC Driver Act 4 --- Stop Dmx\n");
    	if(cmd_bits & CC_CMD_STOP_DMX)
    	{
        	if(dev_list->dmx)
        	{    
            	ret = dmx_io_control(dev_list->dmx, es->dmx_2play ? IO_STREAM_DISABLE_EXT : IO_STREAM_DISABLE, (UINT32)&io_parameter);
            	CC_PRINTF("%s(): stop dmx av ret = %d\n", __FUNCTION__, ret);
        	}
#if 0//#ifdef CHCHG_TICK_PRINTF
			if(cc_play_tick)
			{
				temp_tick = osal_get_tick();
				libc_printf("####CHCHG %s():line%d dmx stop, tick used=%d\n", __FUNCTION__,__LINE__, temp_tick-cc_play_tick);
				cc_play_tick = temp_tick;
			}
#endif
    	}

        if(cmd_bits & CC_CMD_DMX_DISABLE)
        {           
        	if(dev_list->dmx)
        	{

			g_cc_play_tick[g_cc_play_step].step = CC_STOP_DMX;
			g_cc_play_tick[g_cc_play_step].start_tick = osal_get_tick();
            	ret = dmx_io_control(dev_list->dmx, es->dmx_2play ? IO_STREAM_DISABLE_EXT : IO_STREAM_DISABLE, 0);
			g_cc_play_tick[g_cc_play_step++].end_tick = osal_get_tick();
        	}
#if 0//#ifdef CHCHG_TICK_PRINTF
			if(cc_play_tick)
			{
				temp_tick = osal_get_tick();
				libc_printf("####CHCHG %s():line%d dmx disable, tick used=%d\n", __FUNCTION__,__LINE__, temp_tick-cc_play_tick);
				cc_play_tick = temp_tick;
			}
#endif
        }

		if(cmd_bits & CC_CMD_STOP_MCAS_SERVICE )
		{
#ifdef MULTI_CAS
#ifdef NEW_DEMO_FRAME
	    	api_mcas_stop_service(es->sim_id);
#else
		api_mcas_stop_service();
#endif
	    	if(cmd_bits & CC_CMD_SET_XPOND )
	    	{
				g_cc_play_tick[g_cc_play_step].step = CC_PRE_TUNER;
				g_cc_play_tick[g_cc_play_step].start_tick = osal_get_tick();
				#if (defined(GET_CAT_BY_SIM)) && (defined(NEW_DEMO_FRAME))
				api_mcas_stop_transponder(es->sim_id);
				#else
	    		api_mcas_stop_transponder();
				#endif
				g_cc_play_tick[g_cc_play_step++].end_tick = osal_get_tick();
	    	}
#endif
#if 0//#ifdef CHCHG_TICK_PRINTF
			if(cc_play_tick)
			{
				temp_tick = osal_get_tick();
				libc_printf("####CHCHG %s():line%d mcas stop, tick used=%d\n", __FUNCTION__,__LINE__, temp_tick-cc_play_tick);
				cc_play_tick = temp_tick;
			}
#endif
		}


	//Fill Frame Buffer with Color
	CC_PRINTF("CC Driver Act 5 --- Fill Frame Buffer\n");
    	if((cmd_bits & CC_CMD_FILL_FB) && enable_blk_screen)
    	{
        	if(dev_list->vdec)
        	{    
            		struct YCbCrColor tColor;
            		tColor.uY = 0x10;
            		tColor.uCb = 0x80;
            		tColor.uCr = 0x80;
            		ret = vdec_io_control(dev_list->vdec, VDEC_IO_FILL_FRM, (UINT32)&tColor);
            		CC_PRINTF("%s(): fill frame buffer ret = %d\n", __FUNCTION__, ret);
        	}
#if 0//#ifdef CHCHG_TICK_PRINTF
			if(cc_play_tick)
			{
				temp_tick = osal_get_tick();
				libc_printf("####CHCHG %s():line%d fill framebuffer, tick used=%d\n", __FUNCTION__,__LINE__, temp_tick-cc_play_tick);
				cc_play_tick = temp_tick;
			}
#endif
    	}

		if(cmd_bits & CC_CMD_SET_XPOND )
		{
			if(ft)
			{
			g_cc_play_tick[g_cc_play_step].step = CC_SET_XPOND;
			g_cc_play_tick[g_cc_play_step].start_tick = osal_get_tick();
				frontend_set_nim(ft->nim, &ft->antenna, &ft->xpond, 1);
			g_cc_play_tick[g_cc_play_step++].end_tick = osal_get_tick();
           		//After Set Front End, Enable TDT Parse, not to do under S
           		enable_time_parse();
  			}			
#if 0//#ifdef CHCHG_TICK_PRINTF
			if(cc_play_tick)
			{
				temp_tick = osal_get_tick();
				libc_printf("####CHCHG %s():line%d xpond set, tick=%d, tick used=%d\n", __FUNCTION__,__LINE__,temp_tick,temp_tick-cc_play_tick);
				cc_play_tick = temp_tick;
			}
#endif
		}
#if (SYS_MAIN_BOARD == BOARD_DB_M3603_01V01 || SYS_MAIN_BOARD == BOARD_DB_M3383_01V01 \
	|| SYS_MAIN_BOARD == BOARD_DB_M3603_02V01|| SYS_MAIN_BOARD ==BOARD_M3383_UM00_01V01)
   	//Stop Deca Immediately
   	CC_PRINTF("CC Driver Act 2 --- Stop Deca\n");
    	if((cmd_bits & CC_CMD_STOP_AUDIO) && (es->audio_no_act == 0))
    	{
    		if(dev_list->deca)
        	{
        		ret = deca_stop(dev_list->deca, 0, ADEC_STOP_IMM);
            		CC_PRINTF("%s(): deca stop ret = %d\n", __FUNCTION__, ret);
			}
#if 0//#ifdef CHCHG_TICK_PRINTF
			if(cc_play_tick)
			{
				temp_tick = osal_get_tick();
				libc_printf("####CHCHG %s():line%d deca stop, tick used=%d\n", __FUNCTION__,__LINE__, temp_tick-cc_play_tick);
				cc_play_tick = temp_tick;
			}
#endif
	}
#endif
	if(cmd_bits & CC_CMD_START_SI_MONITOR )
	{
				g_cc_play_tick[g_cc_play_step].step = CC_POST_TUNER;
				g_cc_play_tick[g_cc_play_step].start_tick = osal_get_tick();
#if defined(CONAX_NEW_PUB)
		prog_pos = get_prog_pos(es->prog_id);
	    	si_monitor_on(prog_pos);          
#endif

#if(CAS_TYPE==CAS_CDCA||CAS_TYPE == CAS_TF||CAS_TYPE==CAS_DVN||CAS_TYPE==CAS_DVT||CAS_TYPE==CAS_IRDETO || (CAS_TYPE==CAS_GY))
		prog_pos = get_prog_pos(es->prog_id);
	    si_monitor_on(prog_pos);
#endif

#ifdef TS_MONITOR_FEATURE
            prog_pos = get_prog_pos(es->prog_id);
            ts_monitor_start(es->prog_id);
#endif

#ifdef MULTI_CAS
		if((cmd_bits & CC_CMD_SET_XPOND ) || mcas_xpond_stoped)
		{
#if (defined(GET_CAT_BY_SIM)) && (defined(NEW_DEMO_FRAME))
			api_mcas_start_transponder(es->sim_id);
#else
			api_mcas_start_transponder();
#endif
            mcas_xpond_stoped = FALSE;
		}
#ifdef SUPPORT_HW_CONAX
		P_NODE tmp_cas_node;
		get_prog_by_id(es->prog_id, &tmp_cas_node);
		mcas_set_prog_info(&tmp_cas_node);
#endif
#ifdef NEW_DEMO_FRAME
		api_mcas_start_service(es->prog_id, es->sim_id);
#else
		api_mcas_start_service(es->prog_id);
#endif
#endif

#ifdef CAS_CM_ENABLE
		//if((cmd_bits & CC_CMD_SET_ANTENNA )||(cmd_bits & CC_CMD_SET_XPOND ))
		{
			api_osm_cas_start_transponder();
		}
		api_osm_cas_start_service(es->prog_id);	
#endif

#if (CAS_TYPE == CAS_IRDETO)
		if(cmd_bits & CC_CMD_SET_XPOND )
		{
#if (defined(GET_CAT_BY_SIM)) && (defined(NEW_DEMO_FRAME))
			api_mcas_start_transponder(es->sim_id);
#else
			api_mcas_start_transponder();
#endif
		}
#ifdef NEW_DEMO_FRAME
		api_mcas_start_service(es->prog_id, es->sim_id);
#else
		api_mcas_start_service(es->prog_id);
#endif

#endif

#if 0//#ifdef CHCHG_TICK_PRINTF
		if(cc_play_tick)
		{
			temp_tick = osal_get_tick();
			libc_printf("####CHCHG %s():line%d monitor and mcas start, tick=%d, tick used=%d\n", __FUNCTION__,__LINE__, temp_tick,temp_tick-cc_play_tick);
			cc_play_tick = temp_tick;
		}
#endif
				g_cc_play_tick[g_cc_play_step++].end_tick = osal_get_tick();
	}
	
 #ifdef CHANNEL_CHANGE_VIDEO_FAST_SHOW
	if(cmd_bits & CC_CMD_INFO_VPO_CHANCHNG)
	{
		g_cc_play_tick[g_cc_play_step].step = CC_VPO_CHANCHNG;
		g_cc_play_tick[g_cc_play_step].start_tick = osal_get_tick();
		if(hde_get_mode()==VIEW_MODE_MULTI)
			vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0),VPO_IO_CHANGE_CHANNEL, 0);
		else
			vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0),VPO_IO_CHANGE_CHANNEL, 1);
		g_cc_play_tick[g_cc_play_step++].end_tick = osal_get_tick();
#if 0//#ifdef CHCHG_TICK_PRINTF
		if(cc_play_tick)
		{
			temp_tick = osal_get_tick();
			libc_printf("####CHCHG %s():line%d DIS change channel, tick used=%d\n", __FUNCTION__,__LINE__, temp_tick-cc_play_tick);
			cc_play_tick = temp_tick;
		}
#endif
			
	}
#endif
   	//Start Dmx Es Stream
    	CC_PRINTF("CC Driver Act 6 --- Start Dmx\n");
    	if(cmd_bits & CC_CMD_START_DMX)
    	{
        	UINT32 create_stream = 0;
        	UINT32 enable_stream = 0;

		//Video PID, Audio PID and PCR PID
        	MEMSET(&pid_list[0], 0, sizeof(pid_list));
        	io_parameter.io_buff_in = (UINT8 *)pid_list;
			io_parameter.buff_in_len = sizeof(pid_list);
        	if(cmd_bits & CC_CMD_START_VIDEO)
        	{
        		pid_list[0] = es->v_pid;
        	}
        	if(cmd_bits & CC_CMD_START_AUDIO)
        	{
        		pid_list[1] = es->a_pid;
#ifdef AUDIO_DESCRIPTION_SUPPORT
				pid_list[3] = es->ad_pid;
#else
				pid_list[3] = INVALID_PID;
#endif
        	}
			
         	pid_list[2] = es->pcr_pid;

		//Create Audio Stream or/and Video Stream
        	if((cmd_bits & CC_CMD_START_AUDIO) && (cmd_bits & CC_CMD_START_VIDEO))
        	{
        		create_stream = es->dmx_2play ? IO_CREATE_AV_STREAM_EXT : IO_CREATE_AV_STREAM;
        		enable_stream = es->dmx_2play ? IO_STREAM_ENABLE_EXT : IO_STREAM_ENABLE;
        	}
        	else if(cmd_bits & CC_CMD_START_AUDIO)
        	{
        		create_stream = IO_CREATE_AUDIO_STREAM;
        		enable_stream = AUDIO_STREAM_ENABLE;
        	}
        	else if(cmd_bits & CC_CMD_START_VIDEO)
        	{
        		create_stream = IO_CREATE_VIDEO_STREAM;
        		enable_stream = IO_STREAM_ENABLE;
        	}

		g_cc_play_tick[g_cc_play_step].step = CC_START_DMX;
		g_cc_play_tick[g_cc_play_step].start_tick = osal_get_tick();
        	if((create_stream != 0) && (enable_stream != 0))
        	{
			//force to change slot for dynamic pid
			if (es->dynamic_pid_chgch == 1)
			{
				ret = dmx_io_control(dev_list->dmx,DMX_FORCE_CHANGE_SLOT,(UINT32)NULL);
				CC_PRINTF("DMX_FORCE_CHANGE_SLOT : %s :dmx force to cahnge stream slot ,ret = %d\n", __FUNCTION__,ret);
			}			
			
            		ret = dmx_io_control(dev_list->dmx, create_stream, (UINT32)&io_parameter);
            		CC_PRINTF("%s(): dmx create stream,v[%d]a[%d]p[%d], ret = %d\n", __FUNCTION__,
            			pid_list[0],pid_list[1],pid_list[2],ret);

			//Set Audio Synchronization mode in Deca
            		if(INVALID_PID != es->v_pid)
            		{
                		deca_set_sync_mode(dev_list->deca, ADEC_SYNC_PTS);
            		}
            		else 
            		{
                		deca_set_sync_mode(dev_list->deca, ADEC_SYNC_FREERUN);
            		}								
					
			//Enable Dmx Stream
					if((enable_stream==IO_STREAM_ENABLE) || (enable_stream==IO_STREAM_ENABLE_EXT)
						|| (enable_stream==VIDEO_STREAM_ENABLE) || (enable_stream==AUDIO_STREAM_ENABLE))
					{
    					struct dmx_device *dmx = dev_get_by_id(HLD_DEV_TYPE_DMX, 0);

						if(dev_list->dmx != dmx)
						{
							if(dmx_io_control(dmx, CHECK_VDEC_OCCUPATION, 0) == RET_SUCCESS
								|| dmx_io_control(dmx, CHECK_ADEC_OCCUPATION, 0) == RET_SUCCESS)
							{
								dmx_io_control(dev_get_by_id(HLD_DEV_TYPE_DMX, 0), IO_STREAM_DISABLE, (UINT32)NULL);
							}
						}
						else
						{
							if(dmx_io_control(dev_get_by_id(HLD_DEV_TYPE_DMX, 1), CHECK_VDEC_OCCUPATION, 0) == RET_SUCCESS
								||dmx_io_control(dev_get_by_id(HLD_DEV_TYPE_DMX, 1), CHECK_ADEC_OCCUPATION, 0) == RET_SUCCESS)
							{
 								dmx_io_control(dev_get_by_id(HLD_DEV_TYPE_DMX, 1), IO_STREAM_DISABLE, (UINT32)NULL);
							}
						}
					}
            		ret = dmx_io_control(dev_list->dmx, enable_stream, (UINT32)&io_parameter);
            		CC_PRINTF("%s(): dmx enable stream,ret = %d\n", __FUNCTION__,ret);
        	}
		g_cc_play_tick[g_cc_play_step++].end_tick = osal_get_tick();
#if 0//#ifdef CHCHG_TICK_PRINTF
        	if(cc_play_tick)
			{
				temp_tick = osal_get_tick();
				libc_printf("####CHCHG %s():line%d dmx start, tick=%d,tick used=%d\n", __FUNCTION__,__LINE__, temp_tick,temp_tick-cc_play_tick);
				cc_play_tick = temp_tick;
			}
#endif
    	}
    
    	//Start Vdec
    	CC_PRINTF("CC Driver Act 7 --- Start Vdec\n");
    	if(cmd_bits & CC_CMD_START_VIDEO)
    	{
       	if(dev_list->vdec)
        	{    
			g_cc_play_tick[g_cc_play_step].step = CC_START_VIDEO;
			g_cc_play_tick[g_cc_play_step].start_tick = osal_get_tick();
           		ret = vdec_start(dev_list->vdec);
            		CC_PRINTF("%s(): vdec start ret = %d\n", __FUNCTION__, ret);
			g_cc_play_tick[g_cc_play_step++].end_tick = osal_get_tick();
        	}
#if 0//#ifdef CHCHG_TICK_PRINTF
			if(cc_play_tick)
			{
				temp_tick = osal_get_tick();
				vdec_start_tick = temp_tick;
				libc_printf("####CHCHG %s():line%d vdec start, tick=%d, tick used=%d\n", __FUNCTION__,__LINE__, temp_tick, temp_tick-cc_play_tick);
				cc_play_tick = temp_tick;
			}
#endif
    	}

    	//Resume Vdec
    	CC_PRINTF("CC Driver Act 7-1 --- resume Vdec\n");
    	if(cmd_bits & CC_CMD_RESUME_VIDEO)
    	{			
        	if(dev_list->vdec)
        	{    
        		ret = vdec_dvr_resume(dev_list->vdec);
	            	CC_PRINTF("%s(): vdec resume ret = %d\n", __FUNCTION__, ret);
	       }
		}
    
    	//Start Deca
    	CC_PRINTF("CC Driver Act 8 --- Start Deca\n");
    	if((cmd_bits & CC_CMD_START_AUDIO) && (es->audio_no_act == 0))
    	{
        	if((dev_list->deca) && (dev_list->snd_dev))
        	{
			g_cc_play_tick[g_cc_play_step].step = CC_START_AUDIO;
			g_cc_play_tick[g_cc_play_step].start_tick = osal_get_tick();
        		//Set Audio Stream Type in Deca
            		deca_io_control(dev_list->deca, DECA_SET_STR_TYPE, es->a_type);
				
            		//Set Spdif Type in Snd
            		if(es->spdif_enable)
            		{                      
                		//if(es->a_type == AUDIO_AC3)
                		if( (es->a_type == AUDIO_AC3)||
                            (es->a_type == AUDIO_EC3) ||
                    		(es->a_type == AUDIO_MPEG_AAC) ||
                    		(es->a_type == AUDIO_MPEG_ADTS_AAC))

                		{
                    			snd_set_spdif_type(dev_list->snd_dev, SND_OUT_SPDIF_BS);
                		}
                		else if(es->a_type == AUDIO_MPEG2)
                		{
                    			snd_set_spdif_type(dev_list->snd_dev, SND_OUT_SPDIF_PCM);
                		}
            		}
					
			//Call CAT driver to Disable Audio
        		//if(es->a_type == AUDIO_AC3)
        		if( (es->a_type == AUDIO_AC3)||
                    (es->a_type == AUDIO_EC3) ||
            		(es->a_type == AUDIO_MPEG_AAC) ||
            		(es->a_type == AUDIO_MPEG_ADTS_AAC))
	     		{	     			
	     			CC_PRINTF("Play Channel Disable Audio.\n");
				set_hdmi_audio_output(FALSE);
	     		}
					
      			//Start Deca
            		ret = deca_start(dev_list->deca, 0);
            		CC_PRINTF("%s(): deca start ret = %d\n", __FUNCTION__, ret);
					
			//Call CAT driver to Enable Audio
	     		if(es->a_type == AUDIO_MPEG2)
	     		{	     			
	     			CC_PRINTF("Play Channel Enable Audio.\n");
				set_hdmi_audio_output(TRUE);
	     		}
				
	              //Set Audio Channel
            		set_audio_channel(dev_list->snd_dev, es->a_ch);  

			//set snd volumn
			/* If Current Mode is Mute, can not Set Volumn */
			if(SUCCESS !=  snd_io_control(dev_list->snd_dev, IS_SND_MUTE, 0))
			{
				snd_set_volume(dev_list->snd_dev, SND_SUB_OUT, es->a_volumn);
			}
			g_cc_play_last_tick = g_cc_play_tick[g_cc_play_step++].end_tick = osal_get_tick();
        	}
#if 0//#ifdef CHCHG_TICK_PRINTF
			if(cc_play_tick)
			{
				temp_tick = osal_get_tick();
				libc_printf("####CHCHG %s():line%d deca start, tick used=%d\n", __FUNCTION__,__LINE__, temp_tick-cc_play_tick);
				cc_play_tick = temp_tick;
			}
#endif
    	}
    
    	//Change Dmx Audio Stream
    	CC_PRINTF("CC Driver Act 9 --- Change Audio PID\n");
    	if(cmd_bits & CC_CMD_DMX_CHG_APID)
    	{
        	if(dev_list->dmx)
        	{    
            		MEMSET(&pid_list[0], 0, sizeof(pid_list));
            		io_parameter.io_buff_in = (UINT8 *)pid_list;
					io_parameter.buff_in_len = sizeof(pid_list);
            		pid_list[1] = es->a_pid;
#ifdef AUDIO_DESCRIPTION_SUPPORT
            		pid_list[3] = es->ad_pid;
#else
					pid_list[3] = INVALID_PID;
#endif
            		ret = dmx_io_control(dev_list->dmx, IO_CHANGE_AUDIO_STREAM, (UINT32)&io_parameter);
            		CC_PRINTF("%s(): change audio pid ret = %d\n", __FUNCTION__, ret);
        	}
#if 0//#ifdef CHCHG_TICK_PRINTF
			if(cc_play_tick)
			{
				temp_tick = osal_get_tick();
				libc_printf("####CHCHG %s():line%d dmx change audio pid, tick used=%d\n", __FUNCTION__,__LINE__, temp_tick-cc_play_tick);
				cc_play_tick = temp_tick;
			}
#endif
    	}    

	//Switch Audio Channel
	CC_PRINTF("CC Driver Act 10 --- Switch Audio Channel\n");
    	if(cmd_bits & CC_CMD_AUDIO_SWITCH_CHAN)
    	{
        	if(dev_list->snd_dev)
        	{
            		set_audio_channel(dev_list->snd_dev, es->a_ch);
        	}
#if 0//#ifdef CHCHG_TICK_PRINTF
			if(cc_play_tick)
			{
				temp_tick = osal_get_tick();
				libc_printf("####CHCHG %s():line%d switch audio channel, tick used=%d\n", __FUNCTION__,__LINE__, temp_tick-cc_play_tick);
				cc_play_tick = temp_tick;
			}
#endif
    	}

	CC_PRINTF("CC Driver Act 11 --- Leave Driver Act\n");
}

/* Switch Audio PID *
 * For example, one program may have several language streams, when switch another language stream,
 * this function will be called by upper layer. */
static void cc_switch_apid(UINT32 cmd, struct cc_es_info *es, struct cc_device_list *dev_list)
{
    	UINT32 cmd_bits = cmd;

    	if(es->a_pid != 0)
    	{
        	cmd_bits = CC_CMD_STOP_AUDIO|CC_CMD_DMX_CHG_APID|CC_CMD_START_AUDIO;
    	}
    	else
    	{
        	cmd_bits = CC_CMD_NULL;
	}
    	cc_driver_act(cmd_bits, es, dev_list, NULL);
}

/* Pause Video, don't Pause Audio */
static void cc_pause_video(UINT32 cmd, struct cc_es_info *es, struct cc_device_list *dev_list)
{
    	UINT32 cmd_bits = cmd;

    	cmd_bits = CC_CMD_PAUSE_VIDEO;//CC_CMD_STOP_VIDEO;
    	cc_driver_act(cmd_bits,es, dev_list, NULL);
}

/* Resume Video*/
static void cc_resume_video(UINT32 cmd, struct cc_es_info *es, struct cc_device_list *dev_list)
{
    	UINT32 cmd_bits = cmd;

    	cmd_bits = CC_CMD_START_VIDEO;
    	cc_driver_act(cmd_bits,es, dev_list, NULL);
}

static BOOL is_vdec_first_shown = FALSE;
/* First Video Frame Already Show */
static void cb_vdec_first_show(UINT32 uParam1, UINT32 uParam2)
{
	is_vdec_first_shown = TRUE;
#ifdef CHCHG_TICK_PRINTF
	//if(vdec_start_tick)
	{
		UINT32 temp_tick = osal_get_tick();
		libc_printf("####CHCHG %s():line%d vdec first show, tick=%d, tick used=%d\n", __FUNCTION__,__LINE__, temp_tick, temp_tick-CHCHG_TICK_PRINTF_time);
		vdec_start_tick = 0;
	}
#endif
}

/* Check whether First Vedio Frame Already Show or not */
static BOOL ck_vdec_first_show(void)
{
	return is_vdec_first_shown;
}

/* Reset First Vedio Frame not Show */
static void reset_vdec_first_show(void)
{
	is_vdec_first_shown = FALSE;
}

extern UINT32 CIPlus_case_idx;
/* Process Before Play Channel */
static void cc_pre_play_channel(struct dmx_device *dmx, struct cc_es_info *es,
					struct ft_ts_route *ts_route, UINT8 frontend_change)
{
	UINT16 prog_pos = 0xffff;
	T_NODE tp;
	//	struct ci_service_info service;
	//SI Monitor Off
    CC_PRINTF("CC Pre Play Channel 3 --- SI Monitor Off\n"); 
    #ifdef TS_MONITOR_FEATURE
	ts_monitor_stop();
    #endif
#ifndef NEW_DEMO_FRAME //Temp Solution For PIP PVR new SI Monitor
	prog_pos = get_prog_pos(es->prog_id);
    	si_monitor_off(prog_pos);
#if(defined(_SERVICE_NAME_UPDATE_ENABLE_)\
	||defined(_MULTILINGAL_SERVICE_NAME_ENABLE_)\
	||defined(_PREFERRED_NAME_ENABLE_)\
	||defined(_SHORT_SERVICE_NAME_ENABLE_))
#if (SYS_PROJECT_FE != PROJECT_FE_ATSC)
	sdt_monitor_off();
#endif // (SYS_PROJECT_FE != PROJECT_FE_ATSC)
#endif
#ifdef AUTO_OTA
	ota_monitor_off();
#endif // AUTO_OTA
#ifdef SUPPORT_FRANCE_HD
	nit_monitor_off();
#endif // SUPPORT_FRANCE_HD
#else

#if defined(CONAX_NEW_PUB)
	prog_pos = get_prog_pos(es->prog_id);
    si_monitor_off(prog_pos);
#endif
//PVR defined NEW_DEMO_FRAME, call cc_sim_stop_monitor(),but CAS need si_monitor_off()
#if(CAS_TYPE==CAS_CDCA||CAS_TYPE == CAS_TF||CAS_TYPE==CAS_DVN||CAS_TYPE==CAS_DVT||CAS_TYPE==CAS_IRDETO || (CAS_TYPE==CAS_GY))
	prog_pos = get_prog_pos(es->prog_id);
	si_monitor_off(prog_pos);
#endif

#endif
#ifdef MULTI_CAS
#ifdef NEW_DEMO_FRAME
    	api_mcas_stop_service(es->sim_id);
#else
	#if(CAS_TYPE == CAS_CONAX)
		ap_set_cas_dispstr(0x00);
                if(get_mmi_showed()!=5)
		clean_mmi_cur_msg();
	#endif
		api_mcas_stop_service();
#endif
    	if(frontend_change)
    	{
#if (defined(GET_CAT_BY_SIM)) && (defined(NEW_DEMO_FRAME))
			api_mcas_stop_transponder(es->sim_id);
#else
    		api_mcas_stop_transponder();
#endif
			mcas_xpond_stoped = TRUE;
    	}
#endif

#ifdef CAS_CM_ENABLE
	api_osm_cas_stop_service();
    	if(frontend_change)
    	{
    		api_osm_cas_stop_transponder();
    	}
#endif

    	//NVOD
    	CC_PRINTF("CC Pre Play Channel 4 --- NVOD Save\n");
    	if((es->service_type==SERVICE_TYPE_NVOD_TS)&&(frontend_change))
    	{
    		get_tp_by_id(es->tp_id, &tp);
    		nvod_saveorload_tp_eit(0, &tp);
    	}

	//EPG Off
	//recommend: after si_monitor, for it maybe spend much time
	CC_PRINTF("CC Pre Play Channel 2 --- EPG Off\n");
	epg_off();

	CC_PRINTF("CC Pre Play Channel 6 --- Leave Pre Play Channel\n");
}

/* Process After Play Channel */
static void cc_post_play_channel(struct nim_device *nim, struct dmx_device *dmx,
	struct ft_ts_route *ts_route, struct cc_es_info *es, UINT8 frontend_change)
{	
	T_NODE tp;
	P_NODE pnode;

    	//EPG On
    	CC_PRINTF("CC Post Play Channel 2 --- EPG On\n");
    	epg_on(es->sat_id, es->tp_id, es->service_id);
    
    	//NVOD
   		CC_PRINTF("CC Post Play Channel 4 --- NVOD Load\n");
    	if((es->service_type==SERVICE_TYPE_NVOD_TS)&&(frontend_change))
    	{
        	get_tp_by_id(es->tp_id, &tp);
        	nvod_saveorload_tp_eit(1, &tp);
    	}    

	get_prog_by_id(es->prog_id, &pnode);
	//free prog ts bypass ci slot
#if(MW_MODULE == MW_AVIT)
#ifdef AVIT_ENABLE_ETV
    //gavin 20110927
    j_set_etv_svrid(es->service_id);
#endif
#endif
	CC_PRINTF("CC Post Play Channel 6 --- Leave Post Play Channel\n");
}    

/* Play Channel */
void cc_play_channel(UINT32 cmd, struct ft_frontend *ft, struct cc_param *param)
{
    UINT32 cmd_bits = cmd;
    UINT32 channel_pos;
    UINT32 hde_mode;
    struct cc_es_info *es = &param->es;
    struct cc_device_list *dev_list = &param->dev_list;
    UINT16 prog_pos = 0xffff;
    UINT16 video_pid = es->v_pid & 0x1fff;
    UINT16 audio_pid = es->a_pid & 0x1fff;

#if 0//#ifdef CHCHG_TICK_PRINTF
	cc_play_tick = osal_get_tick();
	libc_printf("####CHCHG %s():line%d start play channel, tick =%d\n", __FUNCTION__,__LINE__, cc_play_tick);
#endif
	CC_PRINTF("CC Play Channel 1 --- Set Cmd Bits\n");

	osal_task_dispatch_off();
	cc_cmd_cnt = 0;
	osal_task_dispatch_on();

    	//Check Hde Mode
    	hde_mode = hde_get_mode();
#if defined(HDTV_SUPPORT) && !defined(CHANCHG_VIDEOTYPE_SUPPORT)
        if(hde_mode == VIEW_MODE_MULTI)
        {
       	    cmd_bits &= ~CC_CMD_CLOSE_VPO;
        }
        if (cmd_bits & CC_CMD_CLOSE_VPO)
            enable_fill_blk_screen(FALSE); // fill black screen is slow
        else
            enable_fill_blk_screen(TRUE);
#else
    	if(/*(hde_mode == VIEW_MODE_PREVIEW) ||*/ (hde_mode == VIEW_MODE_MULTI))
    	{
       	cmd_bits &= ~CC_CMD_CLOSE_VPO;
    	}
#endif

	if(es->blk_enable)
		cmd_bits |= CC_CMD_FILL_FB;
		
    	//Check Video ES
    cmd_bits |= CC_CMD_STOP_VIDEO;
    if(video_pid != 0 && video_pid != INVALID_PID)
    {
        cmd_bits |= CC_CMD_START_VIDEO;
#ifdef CHANNEL_CHANGE_VIDEO_FAST_SHOW
	    cmd_bits |= CC_CMD_INFO_VPO_CHANCHNG;
#endif
    }
    else
    {
	    P_NODE p_node;
	    if(SUCCESS==get_prog_by_id(es->prog_id, &p_node))
	    {
            if(p_node.av_flag)
                cmd_bits |= CC_CMD_STOP_VIDEO;
	    }
    }

		
    	//Check Audio ES
    cmd_bits |= CC_CMD_STOP_AUDIO;
    if(audio_pid != 0 && audio_pid != INVALID_PID)
        cmd_bits |= CC_CMD_START_AUDIO;
    //If Radio Channel, don't Close Vpo or Fill Video Frame Buffer
    if((video_pid == 0 || video_pid == 8191) && audio_pid != 0 && audio_pid < 8190)
    {
        cmd_bits &= ~(CC_CMD_CLOSE_VPO|CC_CMD_FILL_FB);
    }

    	cmd_bits |= CC_CMD_STOP_DMX|CC_CMD_START_DMX;

		cmd_bits |= CC_CMD_START_SI_MONITOR;
		
		cmd_bits |= CC_CMD_STOP_MCAS_SERVICE;
		
    	UINT8 antenna_change = 0;
    	UINT8 xpond_change = 0;
    	struct nim_config config;

    	if(ft->nim != NULL)
    	{
		g_cc_play_tick[g_cc_play_step].step = CC_CHECK_NIM;
		g_cc_play_tick[g_cc_play_step].start_tick = osal_get_tick();
    		//Judge whether antenna and transponder need change or not
        	dev_get_nim_config(ft->nim, ft->xpond.common.type, &config);        	
        	if(MEMCMP(&ft->xpond, &config.xpond, sizeof(union ft_xpond)))
        	{  
                xpond_change = 1; 
                if(LNB_CTRL_UNICABLE == ft->antenna.lnb_type) 
                   antenna_change = 1;   //need send channel change command to UNICABLE LNB 
        	}
    		else if(TRUE == frontend_check_nim_param(ft->nim, &ft->xpond))
    		{
    			xpond_change = 1;
    		}
		g_cc_play_tick[g_cc_play_step++].end_tick = osal_get_tick();
    	}

		//Process Before Play Channel
    	CC_PRINTF("CC Play Channel 2 --- Pre Play Channel\n");
		g_cc_play_tick[g_cc_play_step].step = CC_PRE_PLAY;
		g_cc_play_tick[g_cc_play_step].start_tick = osal_get_tick();
    	cc_pre_play_channel(param->dev_list.dmx, es, &ft->ts_route, (antenna_change|xpond_change));
		g_cc_play_tick[g_cc_play_step++].end_tick = osal_get_tick();

		//Set Antenna, Transponder
    	CC_PRINTF("CC Play Channel 3 --- Set Frontend\n");
    	CC_PRINTF("%s(): before set frontend, tick=%d\n",__FUNCTION__,osal_get_tick());
		if(ft->nim != NULL)
		{
		//Change Antenna and Transponder	
              if(xpond_change == 1)
        	{
           		cmd_bits |= CC_CMD_SET_XPOND;
        	}     
    	}    	


    	//Set TS Route
    	CC_PRINTF("CC Play Channel 4 --- Set TS Route\n");
    	frontend_set_ts_route(ft->nim, &ft->ts_route);
    	CC_PRINTF("%s(): before call cc_driver_act, tick=%d\n", __FUNCTION__, osal_get_tick());


	//Play Channel
	CC_PRINTF("CC Play Channel 5 --- Driver Act\n");
    	cc_driver_act(cmd_bits, es, dev_list, ft);
    	CC_PRINTF("%s(): after call cc_driver_act, tick=%d\n", __FUNCTION__, osal_get_tick());
#ifdef CC_POST_CALLBACK
	if(param->callback!=NULL)
		param->callback();
#endif
#if 1//#ifdef CHCHG_TICK_PRINTF
	cc_play_tick = 0;

	dump_cc_steps();
	g_cc_play_step = 0;
#endif	
	//Process After Play Channel
	CC_PRINTF("CC Play Channel 6 --- Post Play Channel\n");
    	cc_post_play_channel(ft->nim, param->dev_list.dmx, &ft->ts_route, es, (antenna_change|xpond_change));
    	CC_PRINTF("%s(): after post_play_channel, tick=%d\n", __FUNCTION__, osal_get_tick());
	CC_PRINTF("CC Play Channel 7 --- Leave Play Channel\n");


#if (SYS_PROJECT_FE == PROJECT_FE_DVBC)
#ifdef SUBTITLE_ON
	prog_pos = get_prog_pos(es->prog_id);
	si_monitor_on(prog_pos);
#endif
#endif
}

/* Process Before Stop Channel */
void cc_pre_stop_channel(struct dmx_device *dmx,struct cc_es_info *es,UINT8 frontend_change)
{
    	UINT16 prog_pos = 0xffff;
    	T_NODE tp;

     	//Stop CA
     	CC_PRINTF("CC Pre Stop Channel 1 --- Stop CA\n");
#ifdef MULTI_CAS
#ifdef NEW_DEMO_FRAME
    	api_mcas_stop_service(es->sim_id);
#else
		api_mcas_stop_service();
#endif
    	if(frontend_change)
    	{
#if (defined(GET_CAT_BY_SIM)) && (defined(NEW_DEMO_FRAME))
			api_mcas_stop_transponder(es->sim_id);
#else
			api_mcas_stop_transponder();
#endif
			mcas_xpond_stoped = TRUE;
    	}
#endif

#ifdef CAS_CM_ENABLE
	api_osm_cas_stop_service();
    	if(frontend_change)
    	{
    		api_osm_cas_stop_transponder();
    	}
#endif

    	//SI Monitor Off
    	CC_PRINTF("CC Pre Stop Channel 3 --- SI Monitor Off\n");
#ifdef TS_MONITOR_FEATURE
	ts_monitor_stop();
#endif
#ifndef NEW_DEMO_FRAME //temp solution for pip pvr new si monitor
	prog_pos = get_prog_pos(es->prog_id);
    	si_monitor_off(prog_pos);
#else
#if defined(CONAX_NEW_PUB)
    	prog_pos = get_prog_pos(es->prog_id);
    	si_monitor_off(prog_pos);
#endif
//PVR defined NEW_DEMO_FRAME, call cc_sim_stop_monitor(),but CAS need si_monitor_off()
	#if(CAS_TYPE==CAS_CDCA||CAS_TYPE == CAS_TF||CAS_TYPE==CAS_DVN||CAS_TYPE==CAS_DVT||CAS_TYPE==CAS_IRDETO || (CAS_TYPE==CAS_GY))
		prog_pos = get_prog_pos(es->prog_id);
	    si_monitor_off(prog_pos);
	#endif

#endif

    	//NVOD
    	CC_PRINTF("CC Pre Stop Channel 4 --- NVOD Save\n");
    	if((es->service_type == SERVICE_TYPE_NVOD_TS) && (frontend_change))
    	{
    		get_tp_by_id(es->tp_id, &tp);
    		nvod_saveorload_tp_eit(0, &tp);
    	}    

	//EPG Off
	//recommend: after si_monitor, for it maybe spend much time
	CC_PRINTF("CC Pre Stop Channel 2 --- EPG Off\n");
    	epg_off();

	CC_PRINTF("CC Pre Stop Channel 5 --- Leave Pre Stop Channel\n");
}

/* Stop Channel *
 * For Example, when enter main menu, this function will be called by upper layer */
void cc_stop_channel(UINT32 cmd, struct cc_es_info *es, struct cc_device_list *dev_list)
{
    	UINT32 cmd_bits = cmd;
    	UINT32 channel_pos;
    	UINT32 hde_mode;

	CC_PRINTF("CC Stop Channel 1 --- Set Cmd Bits\n");
    	//Check Hde Mode to Determine Whether Close Vpo
    	hde_mode = hde_get_mode();

#if defined(HDTV_SUPPORT) && !defined(CHANCHG_VIDEOTYPE_SUPPORT)
        if(hde_mode == VIEW_MODE_MULTI)
        {
       	    cmd_bits &= ~CC_CMD_CLOSE_VPO;
        }
        if (cmd_bits & CC_CMD_CLOSE_VPO)
            enable_fill_blk_screen(FALSE); // fill black screen is slow
        else
            enable_fill_blk_screen(TRUE);
#else
    	if(/*(hde_mode == VIEW_MODE_PREVIEW) || */(hde_mode == VIEW_MODE_MULTI))
    	{
    		cmd_bits &= ~CC_CMD_CLOSE_VPO;
    	}
#endif

	// For Invalid-Program, Audio and Video PID are both 0
	if((es->v_pid == 0) && (es->a_pid == 0))
	{
		cmd_bits |= (CC_CMD_STOP_VIDEO | CC_CMD_STOP_AUDIO);
	}
       
    	//Check Video ES
    	if(es->v_pid != 0)
    	{
    		cmd_bits |= CC_CMD_STOP_VIDEO;
    	}
    
    	//Check Audio ES
    	if(es->a_pid != 0)
    	{
        	cmd_bits |= CC_CMD_STOP_AUDIO;
        	//If Radio Channel, don't Close Vpo or Fill Video Frame Buffer
        	if(es->v_pid == 0)
        	{
            		cmd_bits &= ~(CC_CMD_CLOSE_VPO|CC_CMD_FILL_FB);
        	}
    	}
#ifndef SUPPORT_HW_CONAX
    	cmd_bits |= CC_CMD_STOP_DMX;

    	//Process Before Stop Channel
    	CC_PRINTF("CC Stop Channel 2 --- Pre Stop Channel\n");
    	cc_pre_stop_channel(dev_list->dmx, es, 1);
#else
    cmd_bits |= CC_CMD_DMX_DISABLE;
	cc_pre_stop_channel(dev_list->dmx, es, 0);
#endif
	//Stop Channel
	CC_PRINTF("CC Stop Channel 3 --- Driver Act\n");
    	cc_driver_act(cmd_bits, es, dev_list, NULL);
	CC_PRINTF("CC Stop Channel 4 --- Leave Stop Channel\n");

}

/* Show Background Logo */
static void cc_show_logo(UINT32 cmd, struct cc_logo_info *plogo, struct cc_device_list *dev_list)
{
    	struct cc_es_info es;
    	INT32 ret;
    	UINT8 i;
    	UINT32 cmd_bits;

	CC_PRINTF("CC Show Logo 1 --- Driver Act\n");
        vdec_start(dev_list->vdec);//fix pause issue
    	MEMSET(&es, 0, sizeof(es));
    	cmd_bits = cmd | CC_CMD_STOP_VIDEO | CC_CMD_STOP_DMX |CC_CMD_STOP_AUDIO;
    	cc_driver_act(cmd_bits, &es, dev_list, NULL);

	CC_PRINTF("CC Show Logo 2 --- Show Logo\n");
    	if((plogo != NULL) && (plogo->addr != NULL) && (plogo->size > 0))// && (plogo->sto != NULL))
    	{
       	struct vdec_io_reg_callback_para tpara;
        	void* pWriteBuffer;
        	UINT32 uSizeGot;
#ifdef DUAL_ENABLE
            UINT8 *logo_buf;
            logo_buf = (UINT8 *)MALLOC(plogo->size);
#endif

        	reset_vdec_first_show();
        	tpara.eCBType = VDEC_CB_FIRST_SHOWED;
        	tpara.pCB = cb_vdec_first_show;
        	vdec_io_control(dev_list->vdec, VDEC_IO_REG_CALLBACK, (UINT32)(&tpara));
        	ret = vdec_start(dev_list->vdec);
#ifdef ADJUST_SPECIAL_LOGO_QUALITY
		struct vpo_device *vpo_dev = (struct vpo_device*)dev_get_by_id(HLD_DEV_TYPE_DIS, 0);
		vpo_ioctl(vpo_dev, VPO_IO_CHANGE_YC_INIT_PHASE, 1);
#endif
        	for(i = 0;i < 2; i++)
        	{
			ret = vdec_vbv_request((void *)dev_list->vdec, plogo->size, &pWriteBuffer, &uSizeGot,
				(struct control_block *)NULL);
#ifndef HDTV_SUPPORT
#ifdef DUAL_ENABLE            
			if(((UINT32)plogo->addr&0x80000000) ||((UINT32)plogo->addr&0xA0000000))
			{
				MEMCPY(pWriteBuffer, plogo->addr,uSizeGot);
			}
			else
			{
            	    sto_get_data(plogo->sto, (UINT8 *)logo_buf, plogo->addr, uSizeGot);
                    MEMCPY(pWriteBuffer,logo_buf,uSizeGot);
			}
#else
            if(plogo->sto != NULL)
            {
			    sto_get_data(plogo->sto, (UINT8 *)pWriteBuffer, plogo->addr, uSizeGot);
            }
            else
            {
                MEMCPY(pWriteBuffer, plogo->addr, uSizeGot);
            }
#endif
#else
			if (i == 0)
			{
#ifdef DUAL_ENABLE            
            	    sto_get_data(plogo->sto, (UINT8 *)logo_buf, plogo->addr, uSizeGot);
                    MEMCPY(pWriteBuffer,logo_buf,uSizeGot);
#else
		    		sto_get_data(plogo->sto, (UINT8 *)pWriteBuffer, plogo->addr, uSizeGot);
#endif
			}
			else if (uSizeGot >= 32)
			{
#ifdef DUAL_ENABLE  
                MEMSET(logo_buf, 0, 32);
                MEMCPY(pWriteBuffer,logo_buf,32);
#else
				MEMSET(pWriteBuffer, 0, 32);
#endif
				uSizeGot = 32;
			}
#endif
			vdec_vbv_update((void *)dev_list->vdec, uSizeGot);
        	}
	
        	osal_task_sleep(10);
        	while(!ck_vdec_first_show())
        	{
        		osal_task_sleep(10);
        	}
#ifdef DUAL_ENABLE            
            FREE(logo_buf);
#endif
        	ret = vdec_stop(dev_list->vdec, FALSE, FALSE);
#ifdef ADJUST_SPECIAL_LOGO_QUALITY
		vpo_ioctl(vpo_dev, VPO_IO_CHANGE_YC_INIT_PHASE, 0);
#endif

    	}

	CC_PRINTF("CC Show Logo 3 --- Leave Show Logo\n");
}


/* Implement CC Command */
static void cc_cmd_implement(struct libpub_cmd *cmd)
{
    	UINT32 cmd_finish_bit = 0;
    	struct ft_frontend *ft = NULL;
    	struct ft_ts_route *ts_route = NULL;
    	struct cc_es_info *es = NULL;
    	struct cc_device_list *dev_list = NULL;
    	struct cc_logo_info* plogo = NULL;
    	struct cc_param *param = NULL;

    	switch(cmd->cmd_type)
    	{
	case LIBPUB_CMD_CC_PLAY:
        	ft = (struct ft_frontend*)cmd->data1;
        	param = (struct cc_param*)cmd->data2;
        	//Show Black Screen
        	if(cmd->para1 == 1)
        	{
            		cmd->cmd_bit |= (CC_CMD_CLOSE_VPO|CC_CMD_FILL_FB);
        	}
       
        	CC_PRINTF("%s(): call cc_play_channel(),tick=%d\n", __FUNCTION__, osal_get_tick());     
        	cc_play_channel(cmd->cmd_bit, ft, param);
        	cmd_finish_bit = LIBPUB_FLG_CC_PLAY_BIT;
        	break;
    	case LIBPUB_CMD_CC_STOP:
        	es = (struct cc_es_info *)cmd->data1;
        	dev_list = (struct cc_device_list *)cmd->data2;
        	//Show Black Screen
        	if(cmd->para1 == 1)
        	{
            		cmd->cmd_bit |= (CC_CMD_CLOSE_VPO|CC_CMD_FILL_FB);
        	}
   
        	cc_stop_channel(cmd->cmd_bit, es, dev_list);
        	cmd_finish_bit = LIBPUB_FLG_CC_STOP_BIT;
    		break;
    	case LIBPUB_CMD_CC_PAUSE_VIDEO:
        	es = (struct cc_es_info *)cmd->data1;
        	dev_list = (struct cc_device_list *)cmd->data2;
		cc_pause_video(cmd->cmd_bit, es, dev_list);
        	cmd_finish_bit = LIBPUB_FLG_CC_PAUSE_BIT;
    		break;
    	case LIBPUB_CMD_CC_RESUME_VIDEO:
        	es = (struct cc_es_info *)cmd->data1;
        	dev_list = (struct cc_device_list *)cmd->data2;
		cc_resume_video(cmd->cmd_bit,es, dev_list);
        	cmd_finish_bit = LIBPUB_FLG_CC_PAUSE_BIT;
    		break;
    	case LIBPUB_CMD_CC_SWITCH_APID:
        	es = (struct cc_es_info *)cmd->data1;
        	dev_list = (struct cc_device_list *)cmd->data2;
		cc_switch_apid(cmd->cmd_bit, es, dev_list);
        	cmd_finish_bit = LIBPUB_FLG_CC_SWITCH_APID_BIT;
    		break;
    	case LIBPUB_CMD_CC_SWITCH_ACH:
        	es = (struct cc_es_info *)cmd->data1;
        	dev_list = (struct cc_device_list *)cmd->data2;
        	cmd->cmd_bit = CC_CMD_AUDIO_SWITCH_CHAN;
        	cc_driver_act(cmd->cmd_bit, es, dev_list, NULL);
        	cmd_finish_bit = LIBPUB_FLG_CC_SWITCH_ACH_BIT;
    		break;
    	case LIBPUB_CMD_CC_SHOW_LOGO:
        	plogo = (struct cc_logo_info*)cmd->data1;
        	dev_list = (struct cc_device_list *)cmd->data2;
		cc_show_logo(cmd->cmd_bit, plogo, dev_list);
        	cmd_finish_bit = LIBPUB_FLG_CC_LOGO_BIT;
    		break;
    	}

	if(cmd->sync == 1)
	{
        	libpub_declare_cmd_finish(cmd_finish_bit);
	}
}


/********************************************************
*external APIs
/*******************************************************/
void chchg_init(void)
{
	BOOL ret_bool;
#ifdef CHCHG_TICK_PRINTF
	struct vdec_io_reg_callback_para tpara;
	struct vdec_device *v_dec = (struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0);
#endif
	
	l_cc_sema_id = osal_semaphore_create(1);
	if (OSAL_INVALID_ID == l_cc_sema_id)
	{
		CC_PRINTF("Create l_cc_sema_id failed!\n");
	}
		
	libpub_cmd_handler_register(LIBPUB_CMD_CC, cc_cmd_implement);
	
	ret_bool = libpub_overlay_cmd_register(LIBPUB_CMD_CC_PLAY);
	ASSERT(ret_bool);
	ret_bool = libpub_overlay_cmd_register(LIBPUB_CMD_CC_STOP);
	ASSERT(ret_bool);

#ifdef CHCHG_TICK_PRINTF
	reset_vdec_first_show();
    tpara.eCBType = VDEC_CB_FIRST_SHOWED;
    tpara.pCB = cb_vdec_first_show;
    vdec_io_control(v_dec, VDEC_IO_REG_CALLBACK, (UINT32)(&tpara));
#endif

}

INT32 chchg_play_channel(struct ft_frontend *ft, struct cc_param *param, BOOL blk_scrn)
{
    	struct libpub_cmd cmd;

		osal_task_dispatch_off();
		cc_cmd_cnt ++;
		osal_task_dispatch_on();

    	ENTER_CC_API();

    	MEMSET(&cmd, 0, sizeof(cmd));
    	cmd.cmd_type = LIBPUB_CMD_CC_PLAY;
     	if(hde_get_mode() == VIEW_MODE_MULTI)
     	{
        	cmd.sync = 1;
     	}
    	else
    	{
        	cmd.sync = 0;
    	}
    	if(blk_scrn)
    	{
        	cmd.para1 = 1;
    	}
      	MEMCPY(cmd.data1, ft, sizeof(struct ft_frontend));
    	MEMCPY(cmd.data2, param, sizeof(struct cc_param));
    	libpub_send_cmd(&cmd, OSAL_WAIT_FOREVER_TIME);
    	if(cmd.sync)
    	{
         	libpub_wait_cmd_finish(LIBPUB_FLG_CC_PLAY_BIT, OSAL_WAIT_FOREVER_TIME);
    	}
		
    	LEAVE_CC_API();
    	return SUCCESS;
}

INT32 chchg_stop_channel(struct cc_es_info *es, struct cc_device_list *dev_list, BOOL blk_scrn)
{
    	struct libpub_cmd cmd;
    
    	if(dev_list == NULL)
    	{
        	return !SUCCESS;
    	}

	ENTER_CC_API();
        
    	MEMSET(&cmd, 0, sizeof(cmd));
    	cmd.cmd_type = LIBPUB_CMD_CC_STOP;
    	cmd.para1 = blk_scrn;
    	cmd.sync = 1;
    	MEMCPY(cmd.data1, es, sizeof(struct cc_es_info));
    	MEMCPY(cmd.data2, dev_list, sizeof(struct cc_device_list));
    	libpub_send_cmd(&cmd, OSAL_WAIT_FOREVER_TIME);
    	libpub_wait_cmd_finish(LIBPUB_FLG_CC_STOP_BIT, OSAL_WAIT_FOREVER_TIME);

    	LEAVE_CC_API();
    	return SUCCESS;
}

INT32 chchg_pause_video(struct vdec_device *vdec)
{
    	struct libpub_cmd cmd;
    	struct cc_device_list dev_list;
    
    	if(vdec == NULL)
    	{
       	return !SUCCESS;
    	}
	ENTER_CC_API();
    
    	MEMSET(&cmd, 0, sizeof(cmd));
    	MEMSET(&dev_list, 0, sizeof(dev_list));
    	cmd.cmd_type = LIBPUB_CMD_CC_PAUSE_VIDEO;
    	cmd.sync = 1;
    	dev_list.vdec = vdec;
    	MEMCPY(cmd.data2, &dev_list, sizeof(struct cc_device_list));
    	libpub_send_cmd(&cmd, OSAL_WAIT_FOREVER_TIME);
    	libpub_wait_cmd_finish(LIBPUB_FLG_CC_PAUSE_BIT, OSAL_WAIT_FOREVER_TIME);

    	LEAVE_CC_API();
    	return SUCCESS;
}


INT32 chchg_resume_video(struct vdec_device *vdec)
{
    	struct libpub_cmd cmd;
    	struct cc_device_list dev_list;

    	if(vdec == NULL)
    	{
       	return !SUCCESS;
    	}
	ENTER_CC_API();
    
    	MEMSET(&cmd, 0, sizeof(cmd));
    	MEMSET(&dev_list, 0, sizeof(dev_list));
    	cmd.cmd_type = LIBPUB_CMD_CC_RESUME_VIDEO;
    	cmd.sync = 1;
    	dev_list.vdec = vdec;
    	MEMCPY(cmd.data2, &dev_list, sizeof(struct cc_device_list));
    	libpub_send_cmd(&cmd, OSAL_WAIT_FOREVER_TIME);
    	libpub_wait_cmd_finish(LIBPUB_FLG_CC_PAUSE_BIT, OSAL_WAIT_FOREVER_TIME);

    	LEAVE_CC_API();
    	return SUCCESS;
}

INT32 chchg_switch_audio_pid(struct cc_es_info *es, struct cc_device_list *dev_list)
{
    	struct libpub_cmd cmd;
    
    	if((es == NULL) || (dev_list == NULL))
       {
        	return !SUCCESS;
	}
    	ENTER_CC_API();
    
    	MEMSET(&cmd, 0, sizeof(cmd));
    	cmd.cmd_type = LIBPUB_CMD_CC_SWITCH_APID;
    	cmd.sync = 1;
    	MEMCPY(cmd.data1, es, sizeof(struct cc_es_info));
    	MEMCPY(cmd.data2, dev_list, sizeof(struct cc_device_list));
    	libpub_send_cmd(&cmd, OSAL_WAIT_FOREVER_TIME);
    	libpub_wait_cmd_finish(LIBPUB_FLG_CC_SWITCH_APID_BIT, OSAL_WAIT_FOREVER_TIME);

    	LEAVE_CC_API();
    	return SUCCESS;
}

INT32 chchg_switch_audio_chan(struct snd_device *snd, UINT8 audio_chan)
{
    	struct libpub_cmd cmd;
    	struct cc_device_list dev_list;
    	struct cc_es_info *es;
        
    	if(snd == NULL)
    	{
        	return !SUCCESS;
    	}
	ENTER_CC_API();
    
    	MEMSET(&cmd, 0, sizeof(cmd));
    	MEMSET(&dev_list, 0, sizeof(dev_list));
    	cmd.cmd_type = LIBPUB_CMD_CC_SWITCH_ACH;
    	cmd.sync = 1;
    	dev_list.snd_dev = snd;
    	es = (struct cc_es_info *)(cmd.data1);
    	es->a_ch = audio_chan;
    	MEMCPY(cmd.data2, &dev_list, sizeof(dev_list));
    	libpub_send_cmd(&cmd, OSAL_WAIT_FOREVER_TIME);
    	libpub_wait_cmd_finish(LIBPUB_FLG_CC_SWITCH_ACH_BIT, OSAL_WAIT_FOREVER_TIME);

    	LEAVE_CC_API();
    	return SUCCESS;
}

INT32 chchg_show_logo(struct cc_logo_info *plogo, struct cc_device_list *dev_list)
{
    	struct libpub_cmd cmd;

    	if((plogo == NULL) || (dev_list == NULL))
    	{
        	return !SUCCESS;
    	}
    	ENTER_CC_API();
    
    	MEMSET(&cmd, 0, sizeof(cmd));
    	cmd.cmd_type = LIBPUB_CMD_CC_SHOW_LOGO;
    	cmd.sync = 1;
    	MEMCPY(cmd.data1, plogo, sizeof(struct cc_logo_info));
    	MEMCPY(cmd.data2, dev_list, sizeof(struct cc_device_list));
    	libpub_send_cmd(&cmd, OSAL_WAIT_FOREVER_TIME);
    	libpub_wait_cmd_finish(LIBPUB_FLG_CC_LOGO_BIT, OSAL_WAIT_FOREVER_TIME);

    	LEAVE_CC_API();
    	return SUCCESS;
}


INT32 cc_get_cmd_cnt()
{
	return cc_cmd_cnt;
}


