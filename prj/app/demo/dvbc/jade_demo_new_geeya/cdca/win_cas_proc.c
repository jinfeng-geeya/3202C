#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>
#include <hld/decv/vdec_driver.h>
#include <hld/dis/vpo.h>
#include <hld/pan/pan.h>
#include <hld/pan/pan_dev.h>
#include <hld/dmx/dmx.h>
#include <api/libpub/lib_pub.h>
#include <api/libpub/lib_hde.h>
#include <api/libtsi/si_monitor.h>
//#include <api/libosd/osd_lib.h>
#include <api/libchunk/chunk.h>
#include <api/libcas/gy/cas_gy.h>
//#include <api/libcas/dvt/DVTCAS_STBDataPublic.h>

#include "win_cas_com.h"
#include "../control.h"

//#define CAS_PROC_DEBUG
#ifdef CAS_PROC_DEBUG
#define CAS_PROC_PRINTF	 libc_printf
#else
#define CAS_PROC_PRINTF(...) do{}while(0)
#endif
#define MAIN_MENU_HANDLE		((POBJECT_HEAD)&g_win_mainmenu)
#define CHANNEL_BAR_HANDLE	((POBJECT_HEAD)&g_win_progname)
#define VOLUME_BAR_HANDLE		((POBJECT_HEAD)&g_win_volume)
#define EPG_HANDLE				((POBJECT_HEAD)&g_win_epg)
#define AUDIO_HANDLE			((POBJECT_HEAD)&g_win_audio)

UINT8 CAS_POPUP_STATUS = 0;
//UINT8 MAIL_ICON_STATUS = DVTCAS_EMAIL_NONE;
UINT16 MESSAGE_STR = 0;
UINT16 urgent_broadcast_tip = 0;
static UINT8 CAS_SIGNAL_STATUS = FALSE;
static ID cas_lock_timer = OSAL_INVALID_ID;
static UINT8 cas_urgency = FALSE;
static UINT16 cas_prev_channel = INVALID_ID;
static BOOL cas_feeding_card = FALSE;
static OSAL_ID cas_feed_timer = OSAL_INVALID_ID;
static BOOL show_notice = FALSE;

/*********************************************************************
 *Signal status
 ********************************************************************/
void ap_set_signal_status(UINT8 status)
{
	CAS_SIGNAL_STATUS = status;
}

UINT8 ap_get_signal_status(void)
{
	return CAS_SIGNAL_STATUS;
}
UINT16 ap_get_cas_dispstr(UINT32 param)
{
	return MESSAGE_STR;
}

UINT8 ap_get_cas_urgency_status(void)
{
	return cas_urgency;
}

BOOL ap_get_cas_notice_status(void)
{
	return show_notice;
}

static void ap_set_cas_notice_status(BOOL flg)
{
	show_notice = flg;
}

/*****************************************
 *smc out proc
 *****************************************/
 //the position below is copied from  win_sys_items.c
#define PREVIEW_L  (TV_OFFSET_L+64)
#define PREVIEW_T  (TV_OFFSET_T+96)
#define PREVIEW_W  290
#define PREVIEW_H  234

#define PREVIEW_INNER_L  (PREVIEW_L+24)
#define PREVIEW_INNER_T  (PREVIEW_T+25)
#define PREVIEW_INNER_W  (PREVIEW_W-24-27)
#define PREVIEW_INNER_H  (PREVIEW_H-25-29-30)

void ap_tfcas_smcout_proc()
{
#if 0
	OSD_RECT rc_con;
	OSD_RECT rc_preview;
	if(-1 != window_pos_in_stack((POBJECT_HEAD)&g_win_condition_access) \
		&& (menu_stack_get_top()!=(POBJECT_HEAD)&g_win_feedcard) \
		&&(menu_stack_pop_to((POBJECT_HEAD)&g_win_sys_items)))
	{
		OSD_TrackObject((POBJECT_HEAD)&g_win_sys_items,C_UPDATE_ALL);

		//update the preview windows
		rc_con.uLeft = PREVIEW_L;
		rc_con.uTop = PREVIEW_T;
		rc_con.uWidth = PREVIEW_W;
		rc_con.uHeight = PREVIEW_H;

		rc_preview.uLeft = PREVIEW_INNER_L;
		rc_preview.uTop = PREVIEW_INNER_T;
		rc_preview.uWidth = PREVIEW_INNER_W;
		rc_preview.uHeight = PREVIEW_INNER_H;

		wincom_open_preview_ext(rc_con, rc_preview, WSTL_SUBMENU_PREVIEW);
	}
#endif
	return;
}
/*********************************************************************
 *Open/close DVTCA LIB print info
 ********************************************************************/
void ap_cas_caslib_print(UINT32 vkey)
{
	static UINT32 prev_time = 0;
	static UINT8  key_num = 0;

	if(V_KEY_BLUE == vkey)
	{
		prev_time = osal_get_tick();
		return;
	}
	if (prev_time)
	{
		if (osal_get_tick() - prev_time > 2000)
		{
			prev_time = 0;
			return;
		}
		if (V_KEY_GREEN == vkey)
		{
//			DVTCASTB_AddDebugMsgSign(TRUE);
			prev_time = 0;
		}
		else if (V_KEY_RED == vkey)
		{
//			DVTCASTB_AddDebugMsgSign(FALSE);
			prev_time = 0;
		}
	}

	return;
}


/*********************************************************************
 *check if OTA force upgrade is needed
 ********************************************************************/
void ap_cas_OTA_check(SYSTEM_DATA *data )
{
	SYSTEM_DATA *sys_data = data;

	if (sys_data->force_ota == 1)
	{
		sys_data->force_ota = 0;
		sys_data_save(0);
	}
}
/*********************************************************************
 *OTA upgrade function
 *type: manual or force upgrade
 ********************************************************************/
void ap_cas_OTA_upgrade(UINT32 type)
{
	UINT8 back_saved;
	SYSTEM_DATA *sys_data;
	win_popup_choice_t sel;

	if (CTRL_MSG_SUBTYPE_STATUS_OTA_MANUAL== type)
	{
		win_compopup_init ( WIN_POPUP_TYPE_OKNO);
		win_compopup_set_msg(NULL, NULL, RS_OTA_MANUAL);
		win_compopup_set_default_choice(WIN_POP_CHOICE_NO);
		sel = win_compopup_open_ext(&back_saved);
		if (sel == WIN_POP_CHOICE_NO)
		{
			return;
		}
        else
        {
        	sys_data = sys_data_get();
        	sys_data->force_ota = 1;
        	sys_data_save(0);
        }        
	}
	osal_interrupt_disable();

	win_compopup_init ( WIN_POPUP_TYPE_SMSG );
	win_compopup_set_frame ( 160, 240, 400, 80);
	win_compopup_set_msg_ext ( NULL, NULL, RS_OTA_FORCE );
	win_compopup_open_ext ( &back_saved );
	osal_task_sleep ( 3000 );
	win_compopup_smsg_restoreback();                

	sys_watchdog_reboot();
}


/*******************************************************
 *display black screen if needed.
 *******************************************************/
UINT32 ap_cas_disp_blackscreen()
{
	RET_CODE ret_code;
	BOOL stop_video = TRUE, stop_vpo = TRUE;
	struct vdec_device * vdec_handle = (struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0);
	struct vpo_device * vpo_handle = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0);
	
	if(TRUE == stop_vpo)
	{
		vpo_win_onoff(vpo_handle,FALSE);
	}

	if(TRUE == stop_video)
	{
		ret_code = vdec_stop(vdec_handle,TRUE,FALSE);

		if (ret_code == RET_BUSY)
		{
			ret_code = vdec_stop(vdec_handle,TRUE,FALSE);
		}
	}
	
	struct YCbCrColor tColor;
	tColor.uY = 0x10;
	tColor.uCb = 0x80;
	tColor.uCr = 0x80;
	vdec_io_control(vdec_handle, VDEC_IO_FILL_FRM, (UINT32)(&tColor));

	ret_code = vdec_start(vdec_handle);

}


/*******************************************************
 *POPUP message display function.
 *Below three functions are used to display the popup messages from 
 *the tf lib
 *Also the "No Signal" will be shown in the first priority 
 *******************************************************/
 extern void ap_cas_free_previewing_open(UINT16 strID);
extern ap_cas_free_previewing_close();
void ap_cas_message_close();
void ap_cas_message_open(UINT16 strID)
{
	UINT8 save;
	static UINT16 message_id=0;

	if(CAS_POPUP_STATUS==1)
	{
		if(message_id != strID)
		{
			//win_compopup_close();

			ap_cas_message_close();
		}
	}
	if(RS_SHOWMSG_FREEPREVIEWING == strID)
	{

		ap_cas_free_previewing_open(strID);
		CAS_POPUP_STATUS = 1;
		message_id = strID;
	}
	else

	{

		win_compopup_init ( WIN_POPUP_TYPE_SMSG );

		win_compopup_set_frame ( 180, 180, 360, 120);
		win_compopup_set_msg_ext(NULL,NULL, strID);

		//win_compopup_set_msg_ext(NULL,strID, 0);

		win_compopup_open_ext ( &save );

		CAS_POPUP_STATUS = 1;
		message_id = strID;
	}
}

void ap_cas_message_open_unicode(UINT16 *unicode)
{
	UINT8 save;
	static UINT16 message_id=0;

	if(CAS_POPUP_STATUS==1)
	{
			//win_compopup_close();
			ap_cas_message_close();
	}

	win_compopup_init ( WIN_POPUP_TYPE_SMSG );
	win_compopup_set_frame ( 180, 180, 360, 120);
	win_compopup_set_msg_ext(NULL,unicode, 0);
	win_compopup_open_ext ( &save );
	ap_cas_disp_blackscreen();
	CAS_POPUP_STATUS = 1;
}
void ap_cas_message_close()
{
	if (CAS_POPUP_STATUS)
	{
		win_compopup_close();
		CAS_POPUP_STATUS = 0;
	}
	ap_cas_free_previewing_close();
}

void ap_cas_smcin_proc()
{
#if 0
	if(-1 != window_pos_in_stack((POBJECT_HEAD)&g_win_condition_access)\
     && (menu_stack_get_top()!=(POBJECT_HEAD)&g_win_feedcard))
	{
		//wait smc reset ok
		osal_task_sleep(2500);
		win_condition_access_refresh();
	}
#endif
	return;
}

static ID cas_blk_scrn_id = OSAL_INVALID_ID;
PRESULT ap_cas_message_show();
static void cas_disp_blackscreen_task(UINT32 param0, UINT32 param1)
{
	struct  vdec_device * vdec_dev=(struct vdec_device*)dev_get_by_id(HLD_DEV_TYPE_DECV, 0);
	UINT32 vdec_disp_idx = 0;
	struct VDec_StatusInfo CurStatus;
	UINT8 av_try = 3;
	UINT32 i = 0;
	
	while(1)
	{
		osal_task_sleep(100);
		CAS_PROC_PRINTF("%s: task process\n",__FUNCTION__);
		vdec_io_control(vdec_dev, VDEC_IO_GET_STATUS, (UINT32)&CurStatus);
		if(CurStatus.uFirstPicShowed==TRUE)
		{
			for(i=0;i<av_try;i++)
			{
				vdec_disp_idx = CurStatus.display_idx;
				osal_task_sleep(200);
				vdec_io_control(vdec_dev, VDEC_IO_GET_STATUS, (UINT32)&CurStatus);
				if(vdec_disp_idx != CurStatus.display_idx )
				{
					break;
				}
			}
			if( i == av_try )
			{
				CAS_PROC_PRINTF("show black screen\n");
				ap_cas_disp_blackscreen();
				//ap_cas_message_show();
				//break;
			}
		}
		else
		{
			CAS_PROC_PRINTF("no show first pic\n");
			//break;
		}
	}
	/*osal_task_delete(cas_blk_scrn_id);
	cas_blk_scrn_id = OSAL_INVALID_ID;*/
}

void cas_disp_blackscreen_task_del()
{
	if(OSAL_INVALID_ID != cas_blk_scrn_id)
	{
		osal_task_delete(cas_blk_scrn_id);
		cas_blk_scrn_id = OSAL_INVALID_ID;
	}
}

static void cas_disp_blackscreen_task_init(UINT32 param0, UINT32 param1)
{
	OSAL_T_CTSK t_ctsk;

	if(OSAL_INVALID_ID != cas_blk_scrn_id)
	{
		CAS_PROC_PRINTF("%s: task still exist do not alloc new one\n",__FUNCTION__);
		return ;
	}
	t_ctsk.stksz = 0x1000;
	t_ctsk.quantum = 10;
	t_ctsk.itskpri = OSAL_PRI_NORMAL;
	t_ctsk.name[0] = 'B';
	t_ctsk.name[1] = 'L';
	t_ctsk.name[2] = 'K';
	t_ctsk.task = (FP)cas_disp_blackscreen_task;
	t_ctsk.para1 = 1;
	t_ctsk.para2 = 0;
	cas_blk_scrn_id = osal_task_create(&t_ctsk);
	if(OSAL_INVALID_ID ==  cas_blk_scrn_id)
	{
		CAS_PROC_PRINTF("%s:create task failed\n", __FUNCTION__);
		return ;
	}
}

#define DVTCA_CANCEL_PROMTMSG	0XFE   
void ap_cas_message_setid(UINT8 msgtype)
{
	UINT16 strid = 0;
	UINT8 blackscr = 1;
#if 0	
	switch(msgtype&0xff)
	{
		case DVTCA_CANCEL_PROMTMSG:
			strid = 0;
			blackscr = 0;
			break;
		case DVTCA_RATING_TOO_LOW:
			strid = RS_SHOWMSG_RATINGTOOLOW;
			break;
		case DVTCA_NOT_IN_WATCH_TIME:
			strid = RS_SHOWMSG_NOTINWATCHTIME;
			break;
		case DVTCA_NOT_PAIRED:
			strid = RS_SHOWMSG_NOTPAIRED;
			break;
		case DVTCA_IS_PAIRED_OTHER:
			strid = RS_SHOWMSG_PAIREDOTHER;
			break;
		case DVTCA_PLEASE_INSERT_CARD:
			strid = RS_SHOWMSG_INSERTCARD;
			break;
		case DVTCA_NO_ENTITLE:
			strid = RS_SHOWMSG_NOENTITLE;
			break;
		case DVTCA_PRODUCT_RESTRICT:
			strid = RS_SHOWMSG_PRODUCTRESTRICT;
			break;
		case DVTCA_AREA_RESTRICT:
			strid = RS_SHOWMSG_AREARESTRICT;
			break;
		case DVTCA_DECRYPT_FAILURE:
			//strid = RS_SHOWMSG_DECRYPTFAIL;
			break;
		case DVTCA_MOTHER_RESTRICT:
			strid = RS_SHOWMSG_MOTHERRESTRICT;
			break;
		case DVTCA_NO_MONEY:
			strid = RS_SHOWMSG_NOMONEY;
			break;
		case DVTCA_IPPV_NO_CONFIRM:
		case DVTCA_IPPT_NO_CONFIRM:
			strid = RS_SHOWMSG_IPPBUY;
			break;
		case DVTCA_IPPV_NO_BOOK:
		case DVTCA_IPPT_NO_BOOK:
			strid = RS_SHOWMSG_IPPNOTE;
			break;
		case DVTCA_IPPV_NO_MONEY:
			strid = RS_SHOWMSG_IPPVNOMONEY;
			break;
		case DVTCA_IPPT_NO_MONEY:
			strid = RS_SHOWMSG_IPPTNOMONEY;
			break;
		case DVTCA_DATA_INVALID:
			//strid = RS_SHOWMSG_DATAINVALID;
			break;
		case DVTCA_SLOT_NOT_FOUND:
			strid = RS_SHOWMSG_SLOTINVALID;
			break;
		case DVTCA_SC_NOT_SERVER:
			strid = RS_SHOWMSG_SCNOTSERVE;
			break;
		case DVTCA_TVS_NOT_FOUND:
			strid = RS_SHOWMSG_TVSINVALID;
			break;
		case DVTCA_KEY_NOT_FOUND:
			strid = RS_SHOWMSG_KEYINVALID;
			break;
		case DVTCA_IPPNEED_CALLBACK:
			strid = RS_SHOWMSG_IPPNEEDCALLBACK;
			break;
		case DVTCA_FREE_PREVIEWING:
			strid = RS_SHOWMSG_FREEPREVIEWING;
			break;			
		default:
			libc_printf("%s: invlid msg got!!!", __FUNCTION__);
			break;
	}
	
	MESSAGE_STR = strid;
#if 1
	if (blackscr)
		//&&((NULL == menu_stack_get_top())||(menu_stack_get_top()==CHANNEL_BAR_HANDLE)))
		//ap_cas_disp_blackscreen();
	{
		//add by Jarod for Black Screen
		cas_disp_blackscreen_task_init(0,0);
	}
#endif	
#endif
}
PRESULT ap_cas_message_show()
{
	UINT8 save, lock = 0;
	static UINT8 flag = 0;
	static UINT8 signal_flag = 0;
	static UINT16 notice_tmp[50];
	UINT16 *notice = NULL;
	static BOOL re_open = FALSE;
	//struct nim_device * nim_dev = dev_get_by_type(NULL, HLD_DEV_TYPE_NIM);
	if (!((NULL == menu_stack_get_top())
		||(menu_stack_get_top()==CHANNEL_BAR_HANDLE)
		||(menu_stack_get_top()==VOLUME_BAR_HANDLE)))
	{
		if((ap_get_cas_notice_status() == TRUE) && (CAS_POPUP_STATUS == 1))
		{
			re_open = TRUE;
		}
		return;
	}

	if( -1 != window_pos_in_stack((POBJECT_HEAD)&g_win_ippbuy))
	{
		if (1 == signal_flag)
		{

			win_compopup_close();
			signal_flag = 0;
		}
		return;
	}
	//handle the "No signal" display or not specially
	if (TRUE == CAS_SIGNAL_STATUS)
	{
		ap_cas_message_open(RS_MSG_NO_SIGNAL);
		if (0 == signal_flag)
			ap_cas_disp_blackscreen();
		signal_flag = 1;
		return;
	}
	else if (1 == signal_flag)
	{

		win_compopup_close();
		signal_flag = 0;
	}
	notice = gycas_get_noticemsg_info();
	//if(!MEMCMP(notice, notice_tmp,sizeof(UINT16)*50))
	//	return ;
	
	if((CAS_POPUP_STATUS == 1) && (ap_get_cas_notice_status() == TRUE) && \
		(!MEMCMP(notice, notice_tmp,sizeof(UINT16)*50)) && (re_open == FALSE))
	{
		return ;
	}
	else
		re_open = FALSE;

	MEMCPY(notice_tmp, notice,sizeof(UINT16)*50);
	//if(notice_tmp[0] !='\0')
	if(ap_get_cas_notice_status())
	{
		if(gyca_curchl_is_scramble() == TRUE)
			ap_cas_message_open_unicode(notice_tmp);
	}
	else 
	{
		ap_cas_message_close();
	}
	
#if 0	
	if (MESSAGE_STR>0)
	{
		ap_cas_message_open(MESSAGE_STR);
	}
	else if (0 == MESSAGE_STR)
	{
		ap_cas_message_close();
	}
#endif
	
}


/*****************************************
 *Display finger pirnt function, and check to write
 *flash together
 *****************************************/
extern T_CAFingerInfo* gycas_get_fingermsg_info(void);
PRESULT ap_cas_fingerprint_proc(UINT8 newinfo, UINT8 clear)
{
	static UINT8 disp_flag = 0, pos_flag = 0;
	static UINT32 card_id = 0, finger_start_time = 0, duration = 0, xpos = 450;
	POBJECT_HEAD top_handle = menu_stack_get_top();
	T_CAFingerInfo *fingerinfo=NULL;
	static BOOL nocloseflg = FALSE;

	if (1 == newinfo)
	{
		fingerinfo = gycas_get_fingermsg_info();
		if(fingerinfo == NULL)
			return ;
		duration = fingerinfo->m_byShowTime*1000;
		finger_start_time = osal_get_tick();
		pos_flag = 1;//need to display in a new x position
		if(duration == 0)
			nocloseflg = TRUE;
	}
	
	if (!(top_handle == NULL || 
		top_handle == CHANNEL_BAR_HANDLE || 
		top_handle == VOLUME_BAR_HANDLE ||
		top_handle == AUDIO_HANDLE))
	{
		return;
	}

	{
		if((nocloseflg == TRUE)&&(clear != 1))
			goto show;
		if ((osal_get_tick() -finger_start_time > duration) || (1 == clear))
		{
			if (1 == disp_flag)
			{
				show_finger_print(0, 0,xpos);
			}
			
			disp_flag = 0;
			card_id = 0;
			finger_start_time = 0;
			duration = 0;
			pos_flag = 0;
			nocloseflg = FALSE;
			xpos = ali_rand(600);
		}
		else
show:		{
			// in case the new request come in before the previous has not finished
			if ((1 == pos_flag) && (1 == disp_flag))
			{
				show_finger_print(0, 0,xpos);
				xpos = ali_rand(600);
			}
			
			show_finger_print(1,card_id, xpos);
			pos_flag = 0;
			disp_flag = 1;
		}
	}
}


/*****************************************
 *Display mail icon and detitle icon
 *****************************************/
PRESULT ap_cas_mail_proc()
{
	static UINT8 flag = 1;
	static UINT8 flag2 = 0;
	mail_status status = 0xff;
	POBJECT_HEAD top_handle = menu_stack_get_top();
	
	if (!(top_handle == NULL || 
		top_handle == CHANNEL_BAR_HANDLE || 
		top_handle == VOLUME_BAR_HANDLE ||
		top_handle == AUDIO_HANDLE))
	{
		return;
	}

	status = gyca_getmail_status();
	
	
	//else if (DVTCAS_EMAIL_NEW == MAIL_ICON_STATUS)
	if(status == MAIL_NEW)
	{
		ShowBMailOSDOnOff(1);
		flag2 = 1;
	}
	
	//else if (DVTCAS_EMAIL_NEW_NO_ROOM == MAIL_ICON_STATUS)
	else if(status == MAIL_SPACE_FULL)
	{
		ShowBMailOSDOnOff(flag);
		flag = !flag;
		flag2 = 1;
	}
	
	//if (DVTCAS_EMAIL_NONE == MAIL_ICON_STATUS)
	else//(status == MAIL_OLD_FULL)
	{
		if (flag2)
		{
			ShowBMailOSDOnOff(0);
			flag2 = 0;
		}
	}
}

//PRESULT ap_cas_ipp_popup_proc(UINT16 ecmpid)
//PRESULT ap_cas_ipp_popup_proc(void)
PRESULT ap_cas_ipp_popup_proc(UINT8 open_close)
{
	if(open_close == 0)
	{	
		if( -1 == window_pos_in_stack((POBJECT_HEAD)&g_win_ippbuy))
		{
			return  1;
		}

		OSD_ObjClose(&g_win_ippbuy, C_CLOSE_CLRBACK_FLG);
		if ((POBJECT_HEAD)&g_win_ippbuy == menu_stack_get_top())
			menu_stack_pop();
		return 0;
	}	

	if((POBJECT_HEAD)&g_win_mainmenu== menu_stack_get(0)
		||(POBJECT_HEAD)&g_win_epg== menu_stack_get_top())
	{		
		return 1;
	}
	
	if (OSD_ObjOpen((POBJECT_HEAD)&g_win_ippbuy, 0xFFFFFFFF) != PROC_LEAVE)
	{
		if ((POBJECT_HEAD)&g_win_ippbuy != menu_stack_get_top())
			menu_stack_push((POBJECT_HEAD)&g_win_ippbuy);
	}
	return 0;
}

void cas_dis_urgentbc(UINT8 flag)
{
	UINT8 save;
	static UINT8 disp_type = 0;
	static UINT16 strid = 0;

	if (flag)
	{
		if ((POBJECT_HEAD)&g_win_mainmenu== menu_stack_get(0)
			||(POBJECT_HEAD)&g_win_epg== menu_stack_get_top())
		{
			urgent_broadcast_tip = RS_URGENT_BROADCAST;
			disp_type = 2;//display in menu
		}
		else
		{
			urgent_disp(TRUE, RS_URGENT_BROADCAST);
			disp_type = 1;//display outof menu
		}
	}
	else
	{
		if (1 == disp_type)
		{
			//win_compopup_smsg_restoreback();
			urgent_disp(FALSE, 0);
			disp_type = 0;
		}
		if (2 == disp_type)
		{
			if (urgent_broadcast_tip)
				urgent_broadcast_tip = 0;
			disp_type = 0;
		}
		
	}

}
void cas_cancel_lockservice(UINT8 switchch)
{
	if (FALSE == cas_urgency)
		return;

	api_stop_timer(&cas_lock_timer);

	//OSD lib does not support OSD DRAWING in timer
	ap_send_msg ( CTRL_MSG_SUBTYPE_STATUS_MCAS,(CAS_MSG_URGENT_BROADCAST<<24)|0x0, FALSE );
	
	if (TRUE == switchch)
	{
		//cas_dis_urgentbc(TRUE);
		cas_dis_urgentbc(FALSE);
		if (INVALID_ID != cas_prev_channel)
			api_play_channel(cas_prev_channel, TRUE, FALSE, FALSE);
		cas_urgency = FALSE;
		key_set_enable_flag(TRUE);
	}
}

void cas_lock_timer_handle(UINT param)
{
	api_stop_timer(&cas_lock_timer);

	//OSD lib does not support OSD DRAWING in timer
	ap_send_msg ( CTRL_MSG_SUBTYPE_STATUS_MCAS,(CAS_MSG_UNLOCK_SERVICE<<24)|0x0, FALSE );
}

PRESULT ap_cas_unlockservice_proc()
{
	cas_cancel_lockservice(TRUE);
}
PRESULT ap_cas_lockservice_proc()
{
	UINT16 i, prog_cnt = 0;
	gy_urgent_info info;
	P_NODE prog;
	T_NODE tp;

	//DVTSTBCA_GetUrgentInfo(&info);

	sys_data_change_group(ALL_GROUP_TYPE);
	prog_cnt = get_prog_num(VIEW_ALL|PROG_ALL_MODE, 0);
	for(i = 0; i < prog_cnt; i++)
	{
		if(SUCCESS != get_prog_at(i, &prog))
			return 0;
		if(info.service_id==prog.prog_number)
		{
			if(SUCCESS != get_tp_by_id(prog.tp_id, &tp))
				return 0;
			if((info.ts_id == tp.t_s_id)&&(info.network_id == tp.network_id))
			{
				break;
			}
		}
	}

	//no program is found, do nothing
	if (i>=prog_cnt)
	{
		return 0;
	}

	if (FALSE == cas_urgency)
		cas_prev_channel = sys_data_get_cur_group_cur_mode_channel();
	else
	{
		cas_cancel_lockservice(FALSE);
	}
	
	if (0)
	{
		BackToFullScrPlay();
	}
	api_play_channel(i, TRUE, FALSE, FALSE);
	cas_lock_timer = api_start_timer("cas_lock_timer", info.duration*1000, cas_lock_timer_handle);
	cas_dis_urgentbc(TRUE);
	cas_urgency = TRUE;
	ap_send_msg ( CTRL_MSG_SUBTYPE_STATUS_MCAS,(CAS_MSG_URGENT_BROADCAST<<24)|0x1, FALSE);
	key_set_enable_flag(FALSE);

	return 1;
}

BOOL cas_feed_card_popup(UINT16 strid, BOOL plug_insert)
{
	UINT16 msg[100] = {0};
	UINT16 no_op[20] = {0};
	UINT16 nr[2] = {0x0d00, 0};
	UINT8 save;
	UINT32 hkey, vkey;
	BOOL card_out = FALSE;

	ComUniStrCopyChar(no_op, OSD_GetUnicodeString(RS_SHOWMSG_NO_OPERATION));
	ComUniStrCopyChar(msg, OSD_GetUnicodeString(strid));
	ComUniStrCat(msg, nr);
	ComUniStrCat(msg, no_op);

	win_compopup_init(WIN_POPUP_TYPE_SMSG);
	win_compopup_set_msg(NULL, msg, 0);
	win_compopup_open_ext(&save);

	while(1)
	{
		if (plug_insert == FALSE && gy_get_sc_status() == TRUE)
		{
			win_compopup_smsg_restoreback();
			return TRUE;
		}
		else if (plug_insert == TRUE)
		{
			if (card_out == FALSE)
			{
				if (gy_get_sc_status() == FALSE)
					card_out = TRUE;
			}
			else
			{
				if (gy_get_sc_status() == TRUE)
				{
					win_compopup_smsg_restoreback();
					return TRUE;
				}
			}
		}
		
		hkey = ap_get_key_msg();
		if (hkey == INVALID_HK || hkey == INVALID_MSG)
		{
			continue;
		}
		ap_hk_to_vk(0, hkey, &vkey);
		if (vkey == V_KEY_NULL)
		{
			continue;
		}
		if (vkey == V_KEY_MENU || vkey == V_KEY_EXIT)
		{
			win_compopup_smsg_restoreback();
			return FALSE;
		}

	}
}

BOOL ap_cas_feed_card(void)
{
	UINT32 ret, id=0;
	UINT8 feedinfo[250+2]={0}, len=250;
	UINT16 msg[100] = {0};
	UINT16 no_op[20] = {0};
	UINT16 nr[2] = {0x0d00, 0};
	win_popup_choice_t choice;
	UINT8 save;

	if (!((NULL == menu_stack_get_top())
		||(menu_stack_get_top()==CHANNEL_BAR_HANDLE)
		||(menu_stack_get_top()==VOLUME_BAR_HANDLE)))
	{
		return FALSE;
	}


	if (TRUE != cas_feed_card_popup(RS_SHOWMSG_INSERTMOTHER, TRUE))
		return TRUE;

	while (1)
	{
//		ret = DVTCASTB_GetMotherInfo(&id);
		if (GYCA_OK != ret || (GYCA_OK == ret && id != 0))
		{
			if (TRUE != cas_feed_card_popup(RS_SHOWMSG_MOTHER_FAIL, TRUE))
				return FALSE;
		}
		else
			break;
	}

	while (1)
	{
		len = 250;
		MEMSET(feedinfo, 0, sizeof(feedinfo));
		MEMSET(msg, 0, sizeof(msg));
		ComUniStrCopyChar(msg, OSD_GetUnicodeString(RS_SHOWMSG_READ_MOTHER));
		ComUniStrCat(msg, nr);
		ComUniStrCat(msg, no_op);
		win_compopup_init(WIN_POPUP_TYPE_SMSG);
		win_compopup_set_msg(NULL, msg, 0);
		win_compopup_open_ext(&save);
		osal_task_sleep(3000);
//		ret = DVTCASTB_GetCorrespondInfo(&len, feedinfo);
		libc_printf("return value of DVTCASTB_GetCorrespondInfo is %d\n", ret);
		win_compopup_smsg_restoreback();
		if (GYCA_OK != ret)
		{
			if (TRUE != cas_feed_card_popup(RS_SHOWMSG_MOTHER_FAIL, TRUE))
				return FALSE;
		}
		else
			break;
	}

	if (TRUE != cas_feed_card_popup(RS_SHOWMSG_MOTHER_OK, TRUE))
		return FALSE;

	while (1)
	{
		if (ap_cas_wait_sc_ok(2000) != TRUE)
			ret = GYCA_CARD_ERR;
		else
		{
			osal_task_sleep(1000);
//			ret = DVTCASTB_SetCorrespondInfo(len, feedinfo);
			libc_printf("return value of DVTCASTB_SetCorrespondInfo is %d\n", ret);
		}

		if (GYCA_OK == ret)
		{
			win_compopup_init(WIN_POPUP_TYPE_OKNO);
			win_compopup_set_btnstr(1, RS_HELP_EXIT);
			win_compopup_set_msg(NULL, NULL, RS_SHOWMSG_FEEDOK);
			choice = win_compopup_open_ext(&save);
			if (choice != WIN_POP_CHOICE_YES)
				break;
		}
		else
		{
			win_compopup_init(WIN_POPUP_TYPE_OKNO);
			win_compopup_set_btnstr(1, RS_HELP_EXIT);
			win_compopup_set_msg(NULL, NULL, RS_SHOWMSG_FEEDFAIL);
			choice = win_compopup_open_ext(&save);
			if (choice != WIN_POP_CHOICE_YES)
				break;
		}
	}

	return FALSE;
}

void ap_cas_periodic_check()
{

	ap_cas_mail_proc();
	ap_cas_fingerprint_proc(0, 0);
	ap_cas_message_show();
}

void cas_feed_timer_handle(UINT param)
{
	api_stop_timer(&cas_feed_timer);
	ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MCAS, (CAS_MSG_SHOWPROMPTMSG << 24) | 8, FALSE);
}

static UINT8 cur_channel_is_scrambled(UINT8 *scramble_type)
{
	UINT8 ret = 0,temp = 0;
	struct dmx_device *dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);
    
	osal_task_sleep(1000);
	temp = dmx_io_control(dmx_dev, IS_AV_SOURCE_SCRAMBLED, ( UINT32 )scramble_type);
	//CAS_PROC_PRINTF("$$$get channel scramble type=0x%x, ret=%d $$$\n",scramble_type,ret);
	if(temp ==RET_SUCCESS)
	{
		ret = 1;
	}
	else
	{
		ret = 0;
	}

    return ret;
}

/**************************************************************
 *Main message process entry.
 *Most msg from CATASK will be processed here
 **************************************************************/
PRESULT ap_cas_message_proc ( UINT32 msg_type, UINT32 msg_code )
{
	UINT8 result = PROC_PASS;
	UINT8 scr_type = 0;
	UINT8 tfmsg_type = ( UINT8 ) ( msg_code>>24);
	UINT32 tfmsg_code = ( UINT32) ( msg_code&0xffffff);
	
	switch ( tfmsg_type)
	{
		case CAS_MSG_SHOWPROMPTMSG:
			#if 0
			if (cas_feeding_card == FALSE)
			{
				if ((tfmsg_code&0xff) == DVTCA_MOTHER_RESTRICT)
				{
					cas_feeding_card = TRUE;
					// Need repop after 5 seconds
					if (ap_cas_feed_card() == TRUE)
					{
						cas_feed_timer = api_start_timer("cas_feed_timer", 5000, cas_feed_timer_handle);
						if (cas_feed_timer == OSAL_INVALID_ID)
						{
							//libc_printf("start cas_feed_timer failed!\n");
						}
					}
					cas_feeding_card = FALSE;
				}
				else
				{
					ap_cas_message_setid(tfmsg_code&0xff);
					ap_cas_message_show();
				}
			}
			#endif
			
			//libc_printf(" ++++++++++++++++++++++++++++++++++++++++++++  ap_cas_message_proc()\n");
			//cas_osdmsg_send(CAS_MSG_SHOW_OSD_MESSAGE, 0);
			ap_set_cas_notice_status(TRUE);
			ap_cas_message_show();
			break;
		case CAS_MSG_HIDEPROMPTMSG:
			ap_set_cas_notice_status(FALSE);
			ap_cas_message_show();
			break;
		case CAS_MSG_FINGER_PRINT:
			//libc_printf(" ++++++++++++++++++++++++++++++++++++++++++++  ap_cas_message_proc()\n");
			//cas_osdmsg_send(CAS_MSG_SHOW_OSD_MESSAGE, 0);
			ap_cas_fingerprint_proc(1, 0);
			break;
		case CAS_MSG_FINGER_HID:
			ap_cas_fingerprint_proc(0,1);
			break;
		case CAS_MSG_LOCK_SERVICE:
			//ap_cas_lockservice_proc();
			break;
		case CAS_MSG_UNLOCK_SERVICE:
			//ap_cas_unlockservice_proc();
			break;
		case CAS_MSG_SHOW_OSD_MESSAGE:
			cas_osdmsg_send(CAS_MSG_SHOW_OSD_MESSAGE, 0);
			break;
		case CAS_MSG_HID_TOP_OSD_MESSAGE:
			cas_osdmsg_send(CAS_MSG_HID_TOP_OSD_MESSAGE, 0);
			break;
		case CAS_MSG_HID_BTM_OSD_MESSAGE:
			cas_osdmsg_send(CAS_MSG_HID_BTM_OSD_MESSAGE, 0);
			break;
		case CAS_MSG_HID_MID_OSD_MESSAGE:
			cas_osdmsg_send(CAS_MSG_HID_MID_OSD_MESSAGE, 0);
			break;
		case CAS_MSG_EMAIL_NOTIFY:
			ap_cas_mail_proc();
			break;
		case CAS_MSG_START_IPPVDLG:	
			ap_cas_ipp_popup_proc(1);
			//break;
			//ap_cas_ipp_popup_proc(tfmsg_code);
			break;
		case CAS_MSG_HID_IPPVDLG:
			ap_cas_ipp_popup_proc(0);
			break;
		case CAS_MSG_SMC_OUT:
		 #if 0
			if (cur_channel_is_scrambled(&scr_type)&&
			((scr_type&VDE_TS_SCRBL)||(scr_type&VDE_PES_SCRBL))) //pause video only when video is scrambled
			{
				ap_cas_disp_blackscreen();
				//api_video_set_pause(TRUE);
			}
			ap_tfcas_smcout_proc();
		#endif
			break;
		case CAS_MSG_SMC_IN:
		#if 0
			ap_cas_smcin_proc();//
			//api_video_set_pause(FALSE);
		#endif
			break;
		case CAS_MSG_URGENT_BROADCAST:
			//cas_dis_urgentbc(tfmsg_code&0xff);
			break;
		case CAS_MSG_FORCE_TRANSPARENT:
			{
				T_gy_chgchl_f *pstChgchl_info = gy_get_forcechgchl_info();
				T_gy_chgchl_f stChgchl_data;
				MEMCPY((UINT8 *)&stChgchl_data,(UINT8 *)pstChgchl_info,sizeof(T_gy_chgchl_f));
				T_CARemoteControlInfo *t_info = (T_CARemoteControlInfo *)&stChgchl_data.m_para;
				UINT32 t_info_len = stChgchl_data.m_len;
				UINT8  OpCode;

				if(t_info_len == 0)
				{
					CAS_PROC_PRINTF("GYCA_TRANSPARENT_INFO fail!, parameter is empty\n");
					break;
				}
		
				OpCode = t_info->m_byOpCode;
				CAS_PROC_PRINTF("info length=%d,Opcode=%d\n",t_info_len,OpCode);
				switch(OpCode)
				{
					case 0x00:    /* 要求搜索指定频点*/
					{
						POBJECT_HEAD submenu;
						UINT32 freq = t_info->m_dwFrequency/10;
						UINT32	symbol = 6875,mode = 6;/*6:QAM64*/
						UINT8 i,lock;
						//struct nim_device *nim_dev;
						libc_printf("focre search prog ,freq is %d\n",freq);

						/*LOCK*/
						union ft_xpond xpond;
						    struct nim_device *nim = dev_get_by_id(HLD_DEV_TYPE_NIM, 0);
						    MEMSET(&xpond, 0, sizeof(xpond));
						    xpond.c_info.type = FRONTEND_TYPE_C;
						    xpond.c_info.frq = freq;
						    xpond.c_info.sym = symbol;
						    xpond.c_info.modulation = mode;
						frontend_set_nim(nim, NULL, &xpond, 1);
						
						for (i=0; i<100; i++)
						{
							nim_get_lock(nim, &lock);
							if (lock)
							{
								libc_printf("$$NIMLOCK use %d ms!", i*10);
								break;
							}
							osal_task_sleep(10);
							
						}
						/*SEARCH*/
						submenu = (POBJECT_HEAD) &g_win_search;
						gy_force_search_setflag(TRUE);
						win_set_search_param(1, freq , symbol, mode, 0);
						//SetSearchParamMode(1);						
						if (OSD_ObjOpen(submenu, 0xFFFFFFFF) != PROC_LEAVE)
						{
							libc_printf("force search ok\n");
						}
						else
						{
							libc_printf("force search fail\n");
						}
					}
					break;
					
					case 0x01:    /* 换台指令*/
					   {
							UINT32 freq;
							UINT16 ts_id;
							UINT16 s_id;
							T_NODE t_node;
							P_NODE p_node;
							UINT32 j;
							UINT32 ch_cnt = 0;
						
							CAS_PROC_PRINTF("nGYCA_TRANSPARENT_INFO: channel change\n");
							if(t_info_len < sizeof(T_CARemoteControlInfo))
							{
								CAS_PROC_PRINTF("nGYCA_TRANSPARENT_INFO: channel change, parameter err!\n");
								break;
							}
							freq = t_info->m_dwFrequency;
							ts_id = t_info->m_wTsID;
							s_id = t_info->m_wServiceID;
							CAS_PROC_PRINTF("\n======freq:0x%x(%d),ts_id:0x%x(%d),s_id:0x%x(%d)\n",freq,freq,ts_id,ts_id,s_id,s_id);

							/*if(get_search_signal_menu_status())
							{
								libc_printf("GYCA_TRANSPARENT_INFO, search is running, so don't change channel\n");
								break;
							}*/
							MEMSET(&t_node, 0, sizeof(T_NODE));
							MEMSET(&p_node, 0, sizeof(P_NODE));

							sys_data_change_group(ALL_GROUP_TYPE);
				            		ch_cnt = get_prog_num(VIEW_ALL|PROG_ALL_MODE, 0);
							if (ch_cnt == 0)
							{
								CAS_PROC_PRINTF("GYCA_TRANSPARENT_INFO, channel change fail!, STB not any program\n");
								break;
							}
							
							for(j=0; j<ch_cnt; j++)
							{
								get_prog_at(j, &p_node);

								if(p_node.prog_number == s_id)
								{
									get_tp_by_id(p_node.tp_id, &t_node);

									/*(t_node.frq/10 == freq/100) 的处理是只比较频点整数，小数部分不参考*/
									if((t_node.frq/100 == freq/1000))// && (t_node.t_s_id == ts_id)) 
									{
										api_play_channel(j, TRUE, FALSE, FALSE);
										CAS_PROC_PRINTF("GYCA_TRANSPARENT_INFO, channel change sucessful!\n");

										break;
									}
								}
							} 
							if(j>=ch_cnt)
							{
								CAS_PROC_PRINTF("GYCA_TRANSPARENT_INFO, channel change fail!, STB not find program of match\n");
								break;
							}
						}
						break;
					default:
						break;
					}	
			}
			break;
		default:
			libc_printf("!!!!%s: unknown message %d got!!", __FUNCTION__, tfmsg_type);
			break;
		
	}

	return result;
}

/*****************************************
 *Callback function for tf lib.
 * Middle ware notify the AP by this function.
 *****************************************/
void ap_cas_callback (CAS_MSG_TYPE_T type, UINT32 param)
{
	UINT32 msg_code;
	//libc_printf("&&&&UI got MSG: type = %d, param = %d\n", type, param);
	msg_code = ( UINT32 ) ((param&0xffffff) | (type << 24));
    //libc_printf("ap_cas_callback\n");
	if(!ap_send_msg ( CTRL_MSG_SUBTYPE_STATUS_MCAS, msg_code, FALSE ))
	{
      //libc_printf("ap_cas_callback  send failed 1\n");
      osal_task_sleep(50);
      if(!ap_send_msg ( CTRL_MSG_SUBTYPE_STATUS_MCAS, msg_code, TRUE ))
	  {
         // libc_printf("ap_cas_callback  send failed 2\n");
          if(type == CAS_MSG_START_IPPVDLG)//ipp dlg msg send fail,and need to notify the ca lib.
          {
            //DVTCASTB_InquireBookIppOver(param);//移植时暂时不用 最后根据金亚CA在进行处理
          }
      }
	}
	return;
}

extern struct nim_device *g_nim_dev;
void cas_get_pdsd(void)
{
#if 0
	SYSTEM_DATA *psd = sys_data_get();
	UINT32 pdsd = psd->pdsd;
	UINT8 lock = 0;
	UINT32 wait_cnt = 0;

	nim_channel_change(g_nim_dev, psd->main_frequency, psd->main_symbol, psd->main_qam);
	osal_task_sleep(1000);
	nim_get_lock(g_nim_dev, &lock);
	if (lock != 1)
	{
		libc_printf("cas_get_pdsd lock %d failed!\n", psd->main_frequency);
		return;
	}
	//ota_table_request();
#if (CAS_TYPE == CAS_DVT && defined DVTCA_VERSION_NEW)
	nit_pdsd_request();
#endif
	while(psd->pdsd == pdsd)
	{
		osal_task_sleep(100);
		wait_cnt++;
		if (wait_cnt > 20)
		{
			libc_printf("cas_get_pdsd get or parse NIT failed!\n");
			return;
		}
	}
#endif
}

/*****************************************
 *CAS Module Init
 *****************************************/
void cas_init()
{
	cas_monitor_init(ap_cas_callback);
}

