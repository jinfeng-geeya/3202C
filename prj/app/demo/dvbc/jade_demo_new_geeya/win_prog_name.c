#include <sys_config.h>
#include <types.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/decv/vdec_driver.h>
#include <api/libpub/lib_hde.h>
#include <api/libpub/lib_pub.h>
#include <api/libsi/lib_epg.h>
//#include <api/libosd/osd_lib.h>
#include "osdobjs_def.h"
#include "images.id"
#include "string.id"
#include "osd_config.h"
#include "win_com.h"
#include "menus_root.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_signalstatus.h"
#include "control.h"
#ifdef AD_SANZHOU
#include "sanzhou_ad/adprocess.h"
#endif
#ifdef MULTI_CAS
#if(CAS_TYPE == CAS_CONAX)
#include "conaxca/win_ca_mmi.h"
#endif
#endif
#include "win_prog_name_obj.h"
#if 0
/*******************************************************************************
 *	Objects definition
 *******************************************************************************/
extern CONTAINER g_win_progname;
extern BITMAP prog_bmp;
extern TEXT_FIELD prog_name;
extern TEXT_FIELD prog_time;
extern TEXT_FIELD prog_num;
extern TEXT_FIELD epg_time_now;
extern TEXT_FIELD epg_now;
extern TEXT_FIELD epg_time_next;
extern TEXT_FIELD epg_next;
extern TEXT_FIELD prog_process;

static VACTION win_progname_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT win_progname_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);
#endif
static void win_progname_scroll_string(void);
void win_progname_stop_scroll(void);
#if 0

#define PROG_WIN_SH_IDX		WSTL_PROG_NAME
#define PROG_BMP_SH_IDX		WSTL_TXT_2

#define PROG_NAME_TXT_IDX		WSTL_TXT_PROG_NAME
#define PROG_NAME_EPG_IDX		WSTL_TXT_PROG_NAME_EPG

#define PROG_TXT_WHITE_IDX	WSTL_TXT_4
#define PROG_TXT_BLACK_IDX		WSTL_TXT_3
#endif
#define PN_HOFFSET	0
#define PN_VOFFSET	0

#define W_L		(TV_OFFSET_L+PN_HOFFSET+72)
#define W_T		(TV_OFFSET_T+PN_VOFFSET+386)
#define W_W		566
#define W_H		144
#if 0
#define PROG_BMP_L			(W_L+16)
#define PROG_BMP_T			(W_T+8)
#define PROG_BMP_W			56
#define PROG_BMP_H			56

#define PROG_NAME_L		(W_L+84)
#define PROG_NAME_T		W_T
#define PROG_NAME_W		220
#define PROG_NAME_H		36

#define PROG_TIME_L			(W_L+308)
#define PROG_TIME_T			(W_T+12)
#define PROG_TIME_W		240
#define PROG_TIME_H			24

#define PROG_NUM_L			(W_L+16)
#define PROG_NUM_T			(W_T+64)
#define PROG_NUM_W			60
#define PROG_NUM_H			24

#define EPG_TIME_NOW_L		(W_L+92)
#define EPG_TIME_NOW_T		(W_T+52)
#define EPG_TIME_NOW_W	106
#define EPG_TIME_NOW_H	24

#define EPG_NOW_L			(EPG_TIME_NOW_L+EPG_TIME_NOW_W)
#define EPG_NOW_T			(W_T+52)
#define EPG_NOW_W			260
#define EPG_NOW_H			24

#define EPG_TIME_NEXT_L	(W_L+92)
#define EPG_TIME_NEXT_T	(W_T+82)
#define EPG_TIME_NEXT_W	106
#define EPG_TIME_NEXT_H	24

#define EPG_NEXT_L			(EPG_TIME_NEXT_L+EPG_TIME_NEXT_W)
#define EPG_NEXT_T			(W_T+82)
#define EPG_NEXT_W			260
#define EPG_NEXT_H			24
#endif

#define PROG_PROCESS_L		(W_L+286)
#define PROG_PROCESS_T		(W_T+123)
#define PROG_PROCESS_W		260
#define PROG_PROCESS_H		12

#ifdef AD_SANZHOU
#define BANNER_PIC_L		452
#define BANNER_PIC_T		386
#define BANNER_PIC_W		186
#define BANNER_PIC_H		124
#endif

#if 0
#define LDEF_BMP(root,varBmp,nxtObj,l,t,w,h,sh,icon)		\
	DEF_BITMAP(varBmp,root,nxtObj,C_ATTR_ACTIVE,0, \
	    0,0,0,0,0, l,t,w,h, sh,sh,sh,sh,   \
	    NULL,NULL,C_ALIGN_CENTER|C_ALIGN_VCENTER,0,0,icon)

#define LDEF_TXT(root,varTxt,nxtObj,l,t,w,h,shidx,str)		\
	DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
	    0,0,0,0,0, l,t,w,h, shidx,shidx,shidx,shidx,   \
	    NULL,NULL,C_ALIGN_LEFT|C_ALIGN_VCENTER, 0,0,0,str)

#define LDEF_PROG_NAME(root,varTxt,nxtObj,l,t,w,h,shidx,str)		\
	DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
	    0,0,0,0,0, l,t,w,h, shidx,shidx,shidx,shidx,   \
	    NULL,NULL, C_ALIGN_LEFT|C_ALIGN_BOTTOM,0,0,0,str)

#define LDEF_PROG_NUM(root,varTxt,nxtObj,l,t,w,h,shidx,str)		\
	DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,2, \
	    0,0,0,0,0, l,t,w,h, shidx,shidx,shidx,shidx,   \
	    NULL,NULL,C_ALIGN_LEFT|C_ALIGN_VCENTER,0,0,0,str)

#define LDEF_WIN(varWnd,nxtObj,l,t,w,h,sh,nxtInObj,focusID)		\
    DEF_CONTAINER(varWnd,NULL,NULL,C_ATTR_ACTIVE,0, \
	    0,0,0,0,0, l,t,w,h, sh,sh,sh,sh,   \
	    win_progname_keymap,win_progname_callback,  \
	    nxtInObj, focusID,1)


LDEF_BMP(&g_win_progname, prog_bmp, &prog_name, \
	PROG_BMP_L, PROG_BMP_T, PROG_BMP_W, PROG_BMP_H, PROG_BMP_SH_IDX, 0)

LDEF_PROG_NAME(&g_win_progname, prog_name, &prog_time, \
	PROG_NAME_L, PROG_NAME_T, PROG_NAME_W, PROG_NAME_H, PROG_NAME_TXT_IDX ,display_strs[20])
LDEF_TXT(&g_win_progname, prog_time, &prog_num, \
	PROG_TIME_L, PROG_TIME_T, PROG_TIME_W, PROG_TIME_H, PROG_TXT_BLACK_IDX, display_strs[21])
LDEF_PROG_NUM(&g_win_progname, prog_num, &epg_time_now, \
	PROG_NUM_L, PROG_NUM_T, PROG_NUM_W, PROG_NUM_H, PROG_TXT_BLACK_IDX, display_strs[22])
LDEF_TXT(&g_win_progname, epg_time_now, &epg_now, \
	EPG_TIME_NOW_L, EPG_TIME_NOW_T, EPG_TIME_NOW_W, EPG_TIME_NOW_H, PROG_TXT_WHITE_IDX, display_strs[23])
LDEF_TXT(&g_win_progname, epg_now, &epg_time_next, \
	EPG_NOW_L, EPG_NOW_T, EPG_NOW_W, EPG_NOW_H, PROG_NAME_EPG_IDX, display_strs[24])
LDEF_TXT(&g_win_progname, epg_time_next, &epg_next, \
	EPG_TIME_NEXT_L, EPG_TIME_NEXT_T, EPG_TIME_NEXT_W, EPG_TIME_NEXT_H, PROG_TXT_WHITE_IDX, display_strs[25])
LDEF_TXT(&g_win_progname, epg_next, &prog_process, \
	EPG_NEXT_L, EPG_NEXT_T, EPG_NEXT_W, EPG_NEXT_H, PROG_NAME_EPG_IDX, display_strs[26])
LDEF_TXT(&g_win_progname, prog_process, NULL, \
PROG_PROCESS_L, PROG_PROCESS_T, PROG_PROCESS_W, PROG_PROCESS_H, WSTL_TXT_13 ,NULL)

LDEF_WIN(g_win_progname, NULL, W_L, W_T, W_W, W_H, PROG_WIN_SH_IDX, &prog_bmp, 1)
#endif
/*******************************************************************************
 *	Local vriable & function declare
 *******************************************************************************/
static UINT32 PN_exit_key[] =
{
	V_KEY_ENTER,V_KEY_MENU,V_KEY_LEFT,V_KEY_RIGHT,V_KEY_EXIT,V_KEY_V_UP,
	V_KEY_V_DOWN,V_KEY_0,V_KEY_1,V_KEY_2,V_KEY_3,V_KEY_4,V_KEY_5,V_KEY_6,
	V_KEY_7,V_KEY_8,V_KEY_9,V_KEY_EPG,V_KEY_AUDIO,V_KEY_FAV,	
};

#define MAIN_MENU_HAND ((POBJECT_HEAD)&g_win_mainmenu)
#define PN_exit_key_count  (sizeof(PN_exit_key)/sizeof(UINT32))

static ID progname_timer = OSAL_INVALID_ID;
UINT32 PROGNAME_TIMER_TIME;
#define PROGNAME_TIMER_NAME 	"progname"

static PTEXT_FIELD scroll_txts[] =
{
	&prog_name,
	&epg_now,
	&epg_next
};

#define SCROLL_TXTS_CNT 	(sizeof(scroll_txts)/sizeof(PTEXT_FIELD))

UINT8 show_and_playchannel;

UINT16 recall_play_channel(UINT8 index);
static BOOL check_exit_key_validate(UINT32 key);

static UINT16 present_name[33];
static UINT16 following_name[33];
static BOOL event_update = FALSE; //used for update epg pf info
extern UINT16 weekday_id[];

#ifdef AD_SANZHOU
static OSD_RECT banner_pic_rt = {BANNER_PIC_L,BANNER_PIC_T,BANNER_PIC_W,BANNER_PIC_H};
#endif

INT32 win_progname_set_info(void)
{
	UINT16 group_name[MAX_SERVICE_NAME_LENGTH + 1+10];
	UINT8 group_type, av_mode;

	UINT16 cur_channel;
	P_NODE p_node;
	S_NODE s_node;
	char string[100];
	char prog_name[MAX_SERVICE_NAME_LENGTH + 1];
	INT32 ret, len;
	UINT32 i, j;
	UINT16 icon;
	date_time dt;
	eit_event_info_t *pe = NULL,  *fe = NULL;
	UINT8 *s1 = NULL,  *s2 = NULL;
	INT32 strlen;
	struct ACTIVE_SERVICE_INFO service;
	UINT8 *src;
    UINT8 *src8;
	date_time start_time, end_time, local_time;
	INT32 day, h, m, sec, timeLen, timePassed, progProcessLen;

	UINT32 fav_mask;

	struct t_ttx_lang *ttx_lang_list;
	UINT8 ttx_lang_num;
	struct t_subt_lang* sub_lang_list;
	UINT8 sub_lang_num;
	fav_mask = 0;
	for (i = 0; i < MAX_FAVGROUP_NUM; i++)
		fav_mask |= (0x01 << i);

	get_STC_offset(&h, &m, &sec);

	av_mode = sys_data_get_cur_chan_mode();
	cur_channel = sys_data_get_cur_group_cur_mode_channel();
	ret = get_prog_at(cur_channel, &p_node);
	if(ret != SUCCESS)
		return -1;
	get_sat_by_id(p_node.sat_id, &s_node);
	get_cur_group_name((char*)group_name, &group_type);
	get_local_time(&dt);
	service.tp_id = p_node.tp_id;
	service.service_id = p_node.prog_number;
	epg_set_active_service(&service, 1);

	/* Get current next epg info */
	pe = epg_get_cur_service_event((INT32)cur_channel, PRESENT_EVENT, NULL, NULL, NULL, event_update);
	fe = epg_get_cur_service_event((INT32)cur_channel, FOLLOWING_EVENT, NULL, NULL, NULL, FALSE);
	s1 = (UINT8*)epg_get_event_name(pe, present_name, 32);
	s2 = (UINT8*)epg_get_event_name(fe, following_name, 32);

	#ifdef TTX_ON
	TTXEng_GetInitLang(&ttx_lang_list, &ttx_lang_num);
	#endif
	#ifdef SUBTITLE_ON
	subt_get_language(&sub_lang_list ,&sub_lang_num);
	#endif
	#ifdef TTX_ON
	if(sub_lang_num == 0)
		TTXEng_GetSubtLang(&ttx_lang_list,&sub_lang_num);
	#endif
	/* Set bmp content */
	if (av_mode == TV_CHAN)
		icon = IM_PAY;
	else
		icon = IM_INFORMATION_RADIO;
	OSD_SetBitmapContent(&prog_bmp, icon);
	#ifdef TTX_ON
	if(ttx_lang_num>0)
		icon = IM_INFORMATION_ICON_TTX;
	else
		icon = INVALID_ID; 
	//soc_printf("ttx icon id:%x, ", icon);
	OSD_SetBitmapContent(&prog_name_ttx, icon);
	//OSD_SetRect(&bmp->head.frame, p_prog_bmp->left, p_prog_bmp->top, p_prog_bmp->width,p_prog_bmp->height);
	OSD_DrawObject( (OBJECT_HEAD*) (&prog_name_ttx), C_UPDATE_ALL);
	#endif
	

	#ifdef SUBTITLE_ON
	if(sub_lang_num>0)
		icon = IM_INFORMATION_ICON_SUBTITLE;
	else 
		icon = INVALID_ID;
	//soc_printf("subt icon id:%x\n", icon);
	OSD_SetBitmapContent(&prog_name_subt, icon);
	OSD_DrawObject( (OBJECT_HEAD*) (&prog_name_subt), C_UPDATE_ALL);
	#endif

	/* Init display strings */
	for (i=0; i<7; i++)
		MEMSET(display_strs[20+i], 0, 2*MAX_DISP_STR_LEN);

	/* Set prog_name content */
	MEMSET(string, 0, sizeof(string));
	if (p_node.ca_mode == 0)
		STRCPY(string, "");
	else
		STRCPY(string, "$");

	ComAscStr2Uni(string, display_strs[20]);
	strlen = ComUniStrLen(display_strs[20]);
	ComUniStrCopyChar((UINT8*) &display_strs[20][strlen], p_node.service_name);

	/* Set prog_time content */
	MEMSET(string, 0, sizeof(string));
	sprintf(string, "%4d.%02d.%02d ", dt.year, dt.month, dt.day);
	ComAscStr2Uni(string, display_strs[21]);
	strlen = ComUniStrLen(display_strs[21]);
	src = OSD_GetUnicodeString(weekday_id[dt.weekday%7]);
	ComUniStrCopyChar((UINT8*) &display_strs[21][strlen], src);

	/* Set prog_num content */
	MEMSET(string, 0, sizeof(string));
	sprintf(string, "%03d", cur_channel + 1);
	ComAscStr2Uni(string, display_strs[22]);

	/* Set epg_time_now, epg_now, prog_process content */
	if (s1)
	{
		MEMSET(string, 0, sizeof(string));
		get_STC_offset(&h, &m, &sec);
		get_event_start_time(pe, &start_time);
		get_event_end_time(pe, &end_time);

		convert_time_by_offset(&start_time, &start_time, h, m);
		convert_time_by_offset(&end_time, &end_time, h, m);
 
		sprintf(string, "%02d:%02d~%02d:%02d ", start_time.hour, start_time.min,  \
			end_time.hour, end_time.min);
		ComAscStr2Uni(string, (UINT16*)display_strs[23]);
		ComUniStrCopyChar((UINT8*) &display_strs[24][0], s1);

		//draw prog process
		get_local_time(&local_time);
		get_time_offset(&start_time, &end_time, &day, &h, &m, &sec);
		timeLen = day * 24 * 60+h * 60+m;
		get_time_offset(&start_time, &local_time, &day, &h, &m, &sec);
		timePassed = day * 24 * 60+h * 60+m;
		if ((timeLen <= 0) || (timePassed <= 0))
			progProcessLen = 0;
		else
			progProcessLen = timePassed * PROG_PROCESS_W / timeLen;
		if (progProcessLen > PROG_PROCESS_W)
			progProcessLen = PROG_PROCESS_W;
		///prog_process.head.frame.uWidth = progProcessLen;
	}
	else
	{
		MEMSET(string, 0, sizeof(string));
        src8 = OSD_GetUnicodeString(RS_EPG_NO_INFORMATION);
		ComUniStrCopyChar((UINT8*) &display_strs[23][0], src8);
		sprintf(string, " ");
		ComAscStr2Uni(string, (UINT16*)display_strs[24]);
	}
	
	/* Set epg_time_next, epg_next content */
	if (s2)
	{
		MEMSET(string, 0, sizeof(string));
		get_STC_offset(&h, &m, &sec);
		get_event_start_time(fe, &start_time);
		get_event_end_time(fe, &end_time);


		convert_time_by_offset(&start_time, &start_time, h, m);
		convert_time_by_offset(&end_time, &end_time, h, m);

		sprintf(string, "%02d:%02d~%02d:%02d ", start_time.hour, start_time.min,  \
			end_time.hour, end_time.min);

		ComAscStr2Uni(string, display_strs[25]);
		ComUniStrCopyChar((UINT8*) &display_strs[26][0], s2);
	}
	else
	{
		MEMSET(string, 0, sizeof(string));
		sprintf(string, " ");
		ComAscStr2Uni(string, (UINT16*)display_strs[25]);
		ComAscStr2Uni(string, (UINT16*)display_strs[26]);
	}
	return 0;
}

void win_progname_redraw(BOOL if_reset_scroll)
{
	POBJECT_HEAD pObj = (POBJECT_HEAD) &g_win_progname;

	if(NULL == menu_stack_get_top() || (POBJECT_HEAD) &g_win_progname == menu_stack_get_top())
	{
		if (if_reset_scroll)
			win_progname_stop_scroll();
		OSD_DrawObject(pObj, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
		#ifdef AD_TYPE
		AD_UpdateAD(AD_TYPE_CHANNEL_BAR);
		#endif
	}
}

#define VACT_CH_UP			(VACT_PASS + 1)
#define VACT_CH_DOWN		(VACT_PASS + 2)
#define VACT_GRP_UP			(VACT_PASS + 3)
#define VACT_GRP_DOWN		(VACT_PASS + 4)
#define VACT_FCH_UP			(VACT_PASS + 5)
#define VACT_FCH_DOWN		(VACT_PASS + 6)
#define VACT_TV_RADIO_SW	(VACT_PASS + 7)
#define VACT_RECALL			(VACT_PASS + 8)
#define VACT_SCHEDULE		(VACT_PASS + 9)

void progname_timer_func(UINT unused)
{
	api_stop_timer(&progname_timer);
	ap_send_msg(CTRL_MSG_SUBTYPE_CMD_EXIT, 1, TRUE);
}

static VACTION win_progname_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

	switch (key)
	{
		case V_KEY_UP:
		case V_KEY_C_UP:
			act = VACT_CH_UP;
			break;
		case V_KEY_DOWN:
		case V_KEY_C_DOWN:
			act = VACT_CH_DOWN;
			break;
		case V_KEY_P_UP:
			act = VACT_GRP_UP;
			break;
		case V_KEY_P_DOWN:
			act = VACT_GRP_DOWN;
			break;
		case V_KEY_F_UP:
			act = VACT_FCH_UP;
			break;
		case V_KEY_F_DOWN:
			act = VACT_FCH_DOWN;
			break;
		case V_KEY_TVRADIO:
			act = VACT_TV_RADIO_SW;
			break;
		case V_KEY_RECALL:
			act = VACT_RECALL;
			break;
		case V_KEY_INFOR:
			act = VACT_SCHEDULE;
			break;
			/*
			case V_KEY_0:	case V_KEY_1:	case V_KEY_2:	case V_KEY_3:
			case V_KEY_4:	case V_KEY_5:	case V_KEY_6:	case V_KEY_7:
			case V_KEY_8:	case V_KEY_9:
			act = key - V_KEY_0 + VACT_NUM_0;
			break;
			 */
		case V_KEY_EXIT:
			if (sys_data_get_cur_chan_mode() == TV_CHAN)
				act = VACT_CLOSE;
			break;

		default:
			act = VACT_PASS;
	}

	return act;
}


static PRESULT win_progname_unkown_act_proc(VACTION act);
static PRESULT win_progname_message_proc(UINT32 msg_type, UINT32 msg_code);
static PRESULT win_progname_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	UINT32 i;
	VACTION unact;
	static UINT16 recall_channel = P_INVALID_ID;
	SYSTEM_DATA *sys_data;
	ID taskID;
	UINT8 *screenBuf;
	UINT32 vKey;
	enum API_PLAY_TYPE ret_enum;
	P_NODE pnode;
	UINT16 channel = 0;

	sys_data = sys_data_get();

	switch (event)
	{
		case EVN_PRE_OPEN:
#if(CAS_TYPE == CAS_CONAX)
                if(get_mmi_showed()!=5)
                {
#endif
//Clear the subtitle or teletext content of the last program
#if ((SUBTITLE_ON == 1 )||(TTX_ON ==1))
			api_osd_mode_change(OSD_WINDOW);
#endif
#if(CAS_TYPE == CAS_CONAX)
                    }
#endif
			taskID = osal_task_get_current_id();
			screenBuf = OSD_GetTaskVscrBuffer(taskID);
			MEMSET(screenBuf, OSD_TRANSPARENT_COLOR, OSD_VSRC_MEM_MAX_SIZE);

#ifdef NVOD_FEATURE		
			if(GetNvodPlayFlag())
				return PROC_LEAVE;
#endif
			PROGNAME_TIMER_TIME = sys_data->osd_set.time_out;
			if (PROGNAME_TIMER_TIME == 0 || PROGNAME_TIMER_TIME > 10)
				PROGNAME_TIMER_TIME = 5;
			PROGNAME_TIMER_TIME *= 1000;
			recall_channel = P_INVALID_ID;
			if ((param2 &MENU_OPEN_TYPE_MASK) == MENU_OPEN_TYPE_KEY)
			{
                vKey = param2 & MENU_OPEN_PARAM_MASK;
				if (vKey == V_KEY_RECALL)
				{
					recall_channel = recall_play_channel(0);
					if (recall_channel == P_INVALID_ID)
						return PROC_LEAVE;
				}
 #ifdef MULTI_CAS
#if(CAS_TYPE==CAS_CONAX)
				else if( (param2 & MENU_OPEN_PARAM_MASK ) == V_KEY_INFOR)
				{
					if(get_mmi_msg_cnt()>0)
					{
						ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MCAS, 0, FALSE);
						MMI_PRINTF("CTRL_MSG_SUBTYPE_STATUS_MCAS: info key; code:0");
						set_mmi_showed(10);
					}
				}
#endif
#endif
                else
                {
                    if(V_KEY_UP==vKey || V_KEY_DOWN==vKey || V_KEY_C_UP==vKey ||\
                        V_KEY_C_DOWN==vKey || V_KEY_P_UP==vKey || V_KEY_P_DOWN==vKey\
                         || V_KEY_TVRADIO==vKey)
                    {
                        unact = win_progname_keymap(pObj, vKey);
                        if(VACT_PASS != unact)
                        {
                            win_progname_unkown_act_proc(unact);
                        }
                    }
                }
			
			}
#ifdef MULTI_CAS
#if(CAS_TYPE==CAS_IRDETO)
			if(IRCA_BAN_GetBannerShow())
			{
				//ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MCAS,0x00200000 , FALSE);
				ap_send_msg_expand(CTRL_MSG_SUBTYPE_STATUS_MCAS,0x00000020 ,0, FALSE);
				//ap_cas_call_back(0x00000020);
			}
#endif
#endif
            //ad_show(AD_CHANNEL_BAR);
			OSD_SetBitmapContent(&Bitmappr2, IM_PIC);

			break;

		case EVN_POST_OPEN:                 
			channel = sys_data_get_cur_group_cur_mode_channel();
			
			if(get_prog_at(channel, &pnode) != SUCCESS)
			{
				return PROC_LEAVE;
			}
			
			/* if parent lock && no pwd, play it ,...just want to show pwd*/
			/*if(pnode.lock_flag && GetChannelParrentLock())
			{
				api_play_channel(channel,TRUE,TRUE,FALSE);
			}
			*/
			
			if (show_and_playchannel)
			{
				show_and_playchannel = 0;
				ret_enum = api_play_channel(channel,TRUE,TRUE,FALSE);
				#ifdef MIS_AD
				MIS_ShowEpgAdv(0);
				#endif
			}

			if (recall_channel != P_INVALID_ID)
			{
				ret_enum = api_play_channel(recall_channel, TRUE, TRUE, FALSE);
				OSD_TrackObject(pObj, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
			}
#ifdef AD_SANZHOU
			if((recall_channel == P_INVALID_ID)||(get_prog_at(recall_channel, &pnode) == SUCCESS))
			{
				szxc_ad_hide_txt();
				szxc_ad_show_txt(pnode.prog_number);
				szxc_ad_hide_pic(AD_BANNER);
				szxc_ad_show_banner(pnode.prog_number,banner_pic_rt);
			}
#endif
			progname_timer = api_start_timer(PROGNAME_TIMER_NAME, PROGNAME_TIMER_TIME, progname_timer_func);  

			ShowMuteOnOff();
			ShowPauseOnOff();

			#ifdef MIS_AD
			Mis_Set_SameChan_AdvShowOnce(TRUE);
			Mis_Set_EnterAutoShow(FALSE);
			#endif
			break;
		case EVN_PRE_CLOSE:
            		//ad_stop(AD_CHANNEL_BAR);
			#ifdef MIS_AD
			MIS_HideEpgAdv();
			#endif
			break;
		case EVN_POST_CLOSE:
			api_stop_timer(&progname_timer);
			win_progname_stop_scroll();
#if ((SUBTITLE_ON==1)||(TTX_ON==1))
			api_osd_mode_change(OSD_SUBTITLE);
#endif
#ifdef AD_SANZHOU
			szxc_ad_hide_pic(AD_BANNER);
#endif
			break;
		case EVN_PRE_DRAW:
			if(0 != win_progname_set_info())
				ret = PROC_LEAVE;
			break;
		case EVN_POST_DRAW:
			break;
		case EVN_UNKNOWNKEY_GOT:
			ap_hk_to_vk(0, param1, &vKey);
			if(check_exit_key_validate(vKey))
			{
				ap_send_msg(CTRL_MSG_SUBTYPE_KEY, param1, FALSE);
				ret = PROC_LEAVE;
			}
			break;
		case EVN_UNKNOWN_ACTION:
			unact = (VACTION)(param1 >> 16);
			ret = win_progname_unkown_act_proc(unact);
			break;
		case EVN_MSG_GOT:
			ret = win_progname_message_proc(param1, param2);
			break;

		default:
			break;
	}

	return ret;

}


void change_channel(INT32 shift);
void change_fav_channel(INT32 shift);
void change_group(INT32 shift);

void win_mcas_display(UINT16 ID);
static PRESULT win_progname_unkown_act_proc(VACTION act)
{
	PRESULT ret = PROC_LOOP;

	INT32 shift;
	UINT8 av_mode, back_saved;
	UINT16 channel;
	UINT16 strID;
#ifdef AD_SANZHOU
	P_NODE pnode;
#endif

	api_stop_timer(&progname_timer);
#ifdef MULTI_CAS
#if(CAS_TYPE==CAS_IRDETO)
			if(getStopChannelChange()&&(act!=VACT_RECALL&&act!=VACT_SCHEDULE))//check whether stop channel change
				return ret;
#endif
#endif
	shift =  - 1;
	switch (act)
	{
		case VACT_CH_UP:
			shift = 1;
		case VACT_CH_DOWN:
			change_channel(shift);
		#ifdef MIS_AD
			MIS_ShowEpgAdv(0);
		#endif
			break;
		case VACT_GRP_UP:
			shift = 1;
		case VACT_GRP_DOWN:
			change_group(shift);
			break;
		case VACT_FCH_UP:
			shift = 1;
		case VACT_FCH_DOWN:
			change_fav_channel(shift);
			break;
		case VACT_TV_RADIO_SW:
			av_mode = sys_data_get_cur_chan_mode();
			av_mode = (av_mode == TV_CHAN) ? RADIO_CHAN : TV_CHAN;
			//sys_data_set_cur_intgroup_index(0); /*force to return all group*/
			sys_data_get_group_channel(0, &channel, av_mode);
			if (channel == P_INVALID_ID)
			{	
				 /* If the opposite mode has no channel */
				win_compopup_init(WIN_POPUP_TYPE_SMSG);
				if(av_mode == TV_CHAN)
				{
					win_compopup_set_msg(NULL, NULL, RS_MSG_NO_PROGRAM_TV);
				}
				else
				{
					win_compopup_set_msg(NULL, NULL, RS_MSG_NO_RADIO_CHANNEL);
				}
				win_compopup_open_ext(&back_saved);
				osal_task_sleep(1000);
				win_compopup_smsg_restoreback();
#ifdef MULTI_CAS
#if(CAS_TYPE==CAS_CONAX)
				if(get_mmi_msg_cnt()>0)
				{
					ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MCAS, 0, FALSE);
					MMI_PRINTF("CTRL_MSG_SUBTYPE_STATUS_MCAS: tv/radio; code:%d\n",0);
					set_mmi_showed(10);
				}
#endif
#endif
			}
			else
			{
				sys_data_set_cur_chan_mode(av_mode);
				change_group(0);
			}
			break;
		case VACT_RECALL:
			channel = recall_play_channel(0);
			if (channel != P_INVALID_ID)
			{
#ifdef MULTI_CAS
#if(CAS_TYPE==CAS_CONAX)
				/*clean msg*/
				clean_mmi_msg(1, TRUE);
				clean_mmi_msg(3, TRUE);
				clean_mmi_msg(4, TRUE);
				clean_mmi_msg(6, TRUE);
				if(get_mmi_showed()==1||get_mmi_showed()==6)
					win_mmipopup_close();
				if(get_mmi_showed()!=5)
					set_mmi_showed(10);
#endif
#endif		
				api_play_channel(channel, TRUE, TRUE, FALSE);
#ifdef AD_SANZHOU
				if(get_prog_at(channel, &pnode) == SUCCESS)
				{
					szxc_ad_hide_txt();
					szxc_ad_show_txt(pnode.prog_number);
					szxc_ad_hide_pic(AD_BANNER);
					szxc_ad_show_banner(pnode.prog_number,banner_pic_rt);
				}
#endif
			}
#ifdef MULTI_CAS
#if(CAS_TYPE==CAS_CONAX)
			else if(get_mmi_msg_cnt()>0)
			{
				ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MCAS, 0, FALSE);
				MMI_PRINTF("CTRL_MSG_SUBTYPE_STATUS_MCAS: recall; code:%d\n",0);
				set_mmi_showed(10);
			}
#endif
#endif				
			#ifdef MIS_AD		
			MIS_ShowEpgAdv(0);
			#endif
			break;
		case VACT_SCHEDULE:
			api_stop_timer(&progname_timer);
			//close the mini_epg
			ap_send_msg(CTRL_MSG_SUBTYPE_CMD_EXIT, 1, TRUE);
			//open the schedule
			//		ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32)(POBJECT_HEAD)&g_win_schedule, TRUE);
			break;
		default:
			;
	}

#ifdef MULTI_CAS
#if(CAS_TYPE==CAS_CDCA)
	show_finger_print(0, 0);
#elif(CAS_TYPE==CAS_DVT)
	ap_cas_fingerprint_proc(0, 1);
#endif
#endif
	win_progname_redraw(TRUE);

	progname_timer = api_start_timer(PROGNAME_TIMER_NAME, PROGNAME_TIMER_TIME, progname_timer_func);
#ifdef MIS_AD
	Mis_Set_SameChan_AdvShowOnce(TRUE);
	Mis_Set_EnterAutoShow(FALSE);
#endif
	return ret;

}

static void win_progname_scroll_string(void)
{
	UINT32 i;

	for (i=0; i<SCROLL_TXTS_CNT; i++)
	{
		wincom_scroll_textfield(scroll_txts[i]);
	}
}

void win_progname_stop_scroll(void)
{
	UINT32 i;

	for (i=0; i<SCROLL_TXTS_CNT; i++)
	{
		wincom_scroll_textfield_stop(scroll_txts[i]);
	}
}


static PRESULT win_progname_message_proc(UINT32 msg_type, UINT32 msg_code)
{
	PRESULT ret = PROC_LOOP;

	switch (msg_type)
	{
		case CTRL_MSG_SUBTYPE_CMD_EPG_PF_UPDATED:
			//	case CTRL_MSG_SUBTYPE_CMD_EPG_SCH_UPDATED:
		case CTRL_MSG_SUBTYPE_CMD_TIMEDISPLAYUPDATE:
		case CTRL_MSG_SUBTYPE_STATUS_SIGNAL:
			if (msg_type == CTRL_MSG_SUBTYPE_CMD_EPG_PF_UPDATED)
			{
				event_update = TRUE;
				win_progname_redraw(TRUE);
			}
			else
				event_update = FALSE;
			
			if (msg_type == CTRL_MSG_SUBTYPE_STATUS_SIGNAL)
				ret = PROC_PASS;
			break;
		case CTRL_MSG_SUBTYPE_CMD_EXIT:
			if (sys_data_get_cur_chan_mode() == TV_CHAN)
				ret = PROC_LEAVE;
            else if(sys_data_get_cur_chan_mode() == RADIO_CHAN)
            {
            //to avoid the temporary stop when fast change Radio program channel 
            	sys_data_save(1);  
            }

			break;
		case CTRL_MSG_SUBTYPE_STATUS_SCROLL_STRING:
			win_progname_scroll_string();
			break;
		default:
			ret = PROC_PASS;
	}

	return ret;
}



void change_channel(INT32 shift)
{
	UINT16 cur_channel, max_channel;
	P_NODE p_node;
	UINT32 n;
	SYSTEM_DATA *sys_data;

	sys_data = sys_data_get();

	//max_channel = get_node_num(TYPE_PROG_NODE, NULL);
	max_channel = get_prog_num(VIEW_ALL | sys_data->cur_chan_mode, 0);
	if (max_channel == 0)
	{
#ifdef MULTI_CAS
#if(CAS_TYPE==CAS_CONAX)
		if(get_mmi_msg_cnt()>0)
		{
			ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MCAS, 0, FALSE);
			MMI_PRINTF("CTRL_MSG_SUBTYPE_STATUS_MCAS: change ch only one; code:%d\n",0);
			set_mmi_showed(10);
		}
#endif
#endif    	
		return ;
	}
	cur_channel = sys_data_get_cur_group_cur_mode_channel();

	n = 0;
	do
	{
		cur_channel = (cur_channel + shift + max_channel) % max_channel;
		get_prog_at(cur_channel, &p_node);
		n++;
	}
	while ( ( p_node.skip_flag
	          || ( sys_data->chan_sw == CHAN_SWITCH_FREE && p_node.ca_mode )
	          || ( sys_data->chan_sw == CHAN_SWITCH_SCRAMBLED && !p_node.ca_mode ) )
	        && n != max_channel );

	if ( p_node.skip_flag || ( sys_data->chan_sw == CHAN_SWITCH_FREE && p_node.ca_mode )
	        || ( sys_data->chan_sw == CHAN_SWITCH_SCRAMBLED && !p_node.ca_mode ) )
	{
#ifdef MULTI_CAS
#if(CAS_TYPE==CAS_CONAX)
		if(get_mmi_msg_cnt()>0)
		{
			ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MCAS, 0, FALSE);
			MMI_PRINTF("CTRL_MSG_SUBTYPE_STATUS_MCAS: change ch invalid; code:%d\n",0);
			set_mmi_showed(10);
		}
#endif
#endif        
		return ;
	}

#ifdef MULTI_CAS
#if(CAS_TYPE==CAS_CONAX)
	/*clean msg*/
	clean_mmi_msg(1, TRUE);
	clean_mmi_msg(3, TRUE);
	clean_mmi_msg(4, TRUE);
	clean_mmi_msg(6, TRUE);
	if(get_mmi_showed()==1||get_mmi_showed()==6)
		win_mmipopup_close();
	if(get_mmi_showed()!=5)
		set_mmi_showed(10);
#endif
#endif

	/*  If first time show the osd and play channel,
	to avoid display incorrect channel when meet skip channel.
	 */
	if (show_and_playchannel)
	{
		sys_data_set_cur_group_channel(cur_channel);
		win_progname_redraw(TRUE);
	}

	api_play_channel(cur_channel, TRUE, TRUE, FALSE);

    /////////ad update//////////////
#ifdef ALI_AD_DEMO_ON
    ad_update(AD_CHANNEL_BAR);
#endif
    ///////////////////////////////
#ifdef AD_SANZHOU
	szxc_ad_hide_txt();
	szxc_ad_show_txt(p_node.prog_number);
	szxc_ad_hide_pic(AD_BANNER);
	szxc_ad_show_banner(p_node.prog_number,banner_pic_rt);
#endif
}


void change_fav_channel(INT32 shift)
{
	UINT16 cur_channel, max_channel;
	P_NODE p_node;
	UINT32 i, n;
	SYSTEM_DATA *sys_data;

	UINT32 fav_mask;

	fav_mask = 0;
	for (i = 0; i < MAX_FAVGROUP_NUM; i++)
		fav_mask |= (0x01 << i);

	sys_data = sys_data_get();

	max_channel = get_prog_num(VIEW_ALL | sys_data->cur_chan_mode, 0);
	if (max_channel == 0)
	{
#ifdef MULTI_CAS
#if(CAS_TYPE==CAS_CONAX)
		if(get_mmi_msg_cnt()>0)
		{
			ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MCAS, 0, FALSE);
			MMI_PRINTF("CTRL_MSG_SUBTYPE_STATUS_MCAS: change fav ch only one; code:%d\n",0);
			set_mmi_showed(10);
		}
#endif
#endif    	
		return ;
	}
	cur_channel = sys_data_get_cur_group_cur_mode_channel();

	n = 0;
	do
	{
		cur_channel = (cur_channel + shift + max_channel) % max_channel;
		get_prog_at(cur_channel, &p_node);
		if (shift == 0)
			shift = 1;
		n++;
	}
	while ( ( p_node.skip_flag
	          || ( ( p_node.fav_group[0] & fav_mask ) == 0 )
	          || ( sys_data->chan_sw == CHAN_SWITCH_FREE && p_node.ca_mode )
	          || ( sys_data->chan_sw == CHAN_SWITCH_SCRAMBLED && !p_node.ca_mode ) )
	        && n != max_channel );

	if ( p_node.skip_flag
	        || ( ( p_node.fav_group[0] & fav_mask ) == 0 )
	        || ( sys_data->chan_sw == CHAN_SWITCH_FREE && p_node.ca_mode )
	        || ( sys_data->chan_sw == CHAN_SWITCH_SCRAMBLED && !p_node.ca_mode )
	   )
	{
#ifdef MULTI_CAS
#if(CAS_TYPE==CAS_CONAX)
		if(get_mmi_msg_cnt()>0)
		{
			ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MCAS, 0, FALSE);
			MMI_PRINTF("CTRL_MSG_SUBTYPE_STATUS_MCAS: change fav ch invalid; code:%d\n",0);
			set_mmi_showed(10);
		}
#endif
#endif    	
		return ;
	}

#ifdef MULTI_CAS
#if(CAS_TYPE==CAS_CONAX)
	/*clean msg*/
	/*clean msg*/
	clean_mmi_msg(1, TRUE);
	clean_mmi_msg(3, TRUE);
	clean_mmi_msg(4, TRUE);
	clean_mmi_msg(6, TRUE);
	if(get_mmi_showed()==1||get_mmi_showed()==6)
		win_mmipopup_close();
	if(get_mmi_showed()!=5)
		set_mmi_showed(10);
#endif
#endif

	/*  If first time show the osd and play channel,
	to avoid display incorrect channel when meet skip channel.
	 */
	if (show_and_playchannel)
	{
		sys_data_set_cur_group_channel(cur_channel);
		win_progname_redraw(TRUE);
	}

	api_play_channel(cur_channel, TRUE, TRUE, FALSE);
#ifdef AD_SANZHOU
	szxc_ad_hide_txt();
	szxc_ad_show_txt(p_node.prog_number);
	szxc_ad_hide_pic(AD_BANNER);
	szxc_ad_show_banner(p_node.prog_number,banner_pic_rt);
#endif
}


/* change group and recreate program view with current channel mode, then play it
 * this function does too many things, should be changed later..
 */
void change_group(INT32 shift)
{
	UINT16 cur_channel, max_channel;
	P_NODE p_node;
	UINT32 n;
	SYSTEM_DATA *sys_data;
	UINT8 cur_chan_mode;
	BOOL b_ret;

	sys_data = sys_data_get();

	sys_data_change_group(0);
	
	cur_chan_mode = sys_data_get_cur_chan_mode();
	cur_channel = sys_data_get_cur_group_cur_mode_channel();
	max_channel = get_prog_num(VIEW_ALL | cur_chan_mode, 0);
	if (cur_channel >= max_channel)
		cur_channel = 0;

	n = 0;
	get_prog_at(cur_channel, &p_node);
	while ( ( p_node.skip_flag
	          || ( sys_data->chan_sw == CHAN_SWITCH_FREE && p_node.ca_mode )
	          || ( sys_data->chan_sw == CHAN_SWITCH_SCRAMBLED && !p_node.ca_mode ) )
	        && n != max_channel )
	{
		cur_channel = (cur_channel + 1+max_channel) % max_channel;
		get_prog_at(cur_channel, &p_node);
		n++;
	};

//	if (show_and_playchannel)
//	{
//		sys_data_set_cur_group_channel(cur_channel);
		//win_progname_redraw();
//	}
#ifdef MULTI_CAS
#if(CAS_TYPE==CAS_CONAX)
	/*clean msg*/
	clean_mmi_msg(1, TRUE);
	clean_mmi_msg(3, TRUE);
	clean_mmi_msg(4, TRUE);
	clean_mmi_msg(6, TRUE);
	if(get_mmi_showed()==1||get_mmi_showed()==6)
		win_mmipopup_close();
	if(get_mmi_showed()!=5)
		set_mmi_showed(10);
#endif
#endif

	api_play_channel(cur_channel, TRUE, TRUE, FALSE);
#ifdef AD_SANZHOU
	szxc_ad_hide_txt();
	szxc_ad_show_txt(p_node.prog_number);
	szxc_ad_hide_pic(AD_BANNER);
	szxc_ad_show_banner(p_node.prog_number,banner_pic_rt);
#endif
}

extern struct channel_info recent_channels[2];
UINT16 recall_play_channel(UINT8 index)
{
	UINT8 group_type, group_pos = 0;
	UINT8 cur_mode, new_mode, cur_group_idx, cur_group_type, cur_group_pos;
	UINT8 int_group_idx;
	UINT16 cur_channel;
	S_NODE s_node;
	P_NODE *pnode;
	BOOL exit = FALSE;
	INT32 n;
	UINT8 mode_changed;

	cur_group_idx = sys_data_get_cur_group_index();
	sys_data_get_cur_mode_group_infor(cur_group_idx, &cur_group_type, &cur_group_pos, &cur_channel);

	index = 0;

	cur_mode = sys_data_get_cur_chan_mode();

	if (cur_mode != recent_channels[index].mode)
	{
		sys_data_set_cur_chan_mode(recent_channels[index].mode);
		mode_changed = 1;
	}
	else
		mode_changed = 0;

	int_group_idx = recent_channels[index].internal_group_idx;

	if (int_group_idx == 0)
	{
		group_type = ALL_GROUP_TYPE;
		group_pos = 0;
	}
	else if (int_group_idx == DATA_GROUP_TYPE)
	{
		group_type = DATA_GROUP_TYPE;
	//	group_pos = DATA_GROUP_TYPE;
	}
	else if(int_group_idx == NVOD_GROUP_TYPE)
	{
		group_type = NVOD_GROUP_TYPE;
	//	group_pos = NVOD_GROUP_TYPE;
	}
	else if(int_group_idx < MAX_GROUP_NUM)
	{
		group_type = LOCAL_GROUP_TYPE;
		group_pos = int_group_idx;
	}
	else
	{
		group_type = FAV_GROUP_TYPE;
	//	group_pos = NVOD_GROUP_TYPE+1;
	}

	cur_group_idx = sys_data_get_cur_group_index();

	new_mode = sys_data_get_cur_chan_mode();

	//printf("%d,%d,%d\n",cur_group_type,cur_group_pos,cur_channel);
	if (mode_changed || group_type != cur_group_type || (group_pos != cur_group_pos && group_type != ALL_GROUP_TYPE))
	{

RECREATE_PROG_VIEW:
		if ( group_type == ALL_GROUP_TYPE )
			recreate_prog_view(VIEW_ALL | new_mode, 0);
		else if (group_type == DATA_GROUP_TYPE)
		{
			if (get_sat_at(group_pos, VIEW_ALL, &s_node) != SUCCESS)
				return P_INVALID_ID;
			recreate_prog_view(VIEW_SINGLE_SAT | new_mode, s_node.sat_id);
		}
		else
			recreate_prog_view(VIEW_FAV_GROUP | new_mode, group_pos);

		if (exit)
		{
			//printf("No this group,exit to previous group,int_group_idx%d\n",int_group_idx);
			return P_INVALID_ID;
		}

		if (get_prog_num(VIEW_ALL | new_mode, 0) > 0)
			sys_data_set_cur_intgroup_index(int_group_idx);
		else
		{
			exit = TRUE;
			group_type = cur_group_type;
			group_pos = cur_group_pos;
			new_mode = cur_mode;
			sys_data_set_cur_chan_mode(cur_mode);
			goto RECREATE_PROG_VIEW;
		}

	}

	cur_mode = sys_data_get_cur_chan_mode();

	//cur_channel = get_prog_pos(&recent_channels[index].p_node);
	pnode = &recent_channels[index].p_node;
	cur_channel = get_prog_pos(pnode->prog_id);
	n = get_prog_num(VIEW_ALL | cur_mode, 0);
	if (cur_channel >= n)
	{
		if (n > 0)
			cur_channel = n - 1;
		/*
		else
		{
		exit = TRUE;
		group_type = cur_group_type;
		group_pos = cur_group_pos;
		sys_data_set_cur_chan_mode(cur_mode);
		goto RECREATE_PROG_VIEW;
		}
		 */
	}



	return cur_channel;

}



static BOOL check_exit_key_validate(UINT32 key)
{
	UINT32 index;
	for(index = 0;index < PN_exit_key_count;index++)
	{
		if(PN_exit_key[index] == key)
			return TRUE;
	}
	return FALSE;
}
