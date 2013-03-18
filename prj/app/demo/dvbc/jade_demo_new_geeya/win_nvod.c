#include <sys_config.h>
#ifdef NVOD_FEATURE
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <api/libpub/lib_pub.h>
#include <api/libpub/lib_hde.h>
#include <hld/dis/vpo.h>
#include <api/libsi/si_eit.h>

//#include <api/libosd/osd_lib.h>
#include <api/libsi/lib_nvod.h>
#include <hld/decv/decv.h>

#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"
#include "win_com.h"
#include "menus_root.h"

#include "win_com_popup.h"
#include "win_com_list.h"

#include "win_signalstatus.h"
#include "win_timer_set.h"

#include "win_nvod_obj.h"

TEXT_CONTENT nvod_sch_mtxt_content;

extern UINT32  nr_count;
static void nvod_draw_cur_prg_bmp();
static void nvod_display_str_init(UINT8 max);
static PRESULT win_nvod_unkown_act_proc(VACTION act);
void win_nvod_get_preview_rect(UINT16 *x, UINT16 *y, UINT16 *w, UINT16 *h);

static void win_nvod_ref_event_set_display(void);
static void win_nvod_sch_set_display(void);
static void nvod_ref_list_init();
static void nvod_sch_list_init();
void win_nvod_msg_translate(UINT8 type, UINT32 param);
static void DrawRefEventDetail();
#if 0
static VACTION nvod_detail_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT nvod_detail_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);
#endif
void GetRefEventStartEndTime(struct nvod_event *event, date_time *start_dt, date_time *end_dt, date_time *duration_dt);
void NvodUpdateTimeList(void);
static UINT8 NvodProgBook();
void CovertByOffset(date_time *dt);
static UINT8 NVODEventBookedOrNot(struct nvod_event event);
UINT32 get_played_event();//Add by Ryan Lin to cease warning

#define PREVIEW_WIN_SH_IDX	WSTL_EPG_PREVIEW
#define PREVIEW_BMP_SH_IDX	WSTL_TXT_4

#define WIN_IDX	  WSTL_COMMON_BACK_2

#define TITLE_SH_IDX	WSTL_WINCHANNELIST_01_TITLE
#define TITLE_TXT_SH_IDX	WSTL_TEXT_24

#define EVENT_CON_SH_IDX WSTL_WIN_EPG_01
#define EVENT_SH_IDX WSTL_TEXT_09

#define HELPBAR_BMP_TXT_IDX  WSTL_TEXT_15

#define LST_IDX		WSTL_NOSHOW_IDX

#define SCH_CON_IDX   WSTL_WIN_1
#define SCH_DETAIL_SH_IDX   WSTL_TXT_NVOD_DETAIL//WSTL_WIN_2

#define LIST_BAR_SH_IDX  WSTL_SCROLLBAR_1
#define LIST_BAR_HL_IDX  WSTL_SCROLLBAR_1

#define LIST_BAR_MID_RECT_IDX  	WSTL_NOSHOW_IDX
#define LIST_BAR_MID_THUMB_IDX  	WSTL_SCROLLBAR_2

#define OL_SH_IDX WSTL_TXT_13

#define DATETIME_SH_IDX WSTL_TEXT_22

#define ONTIME_SH_IDX WSTL_TXT_13

#define CHANCON_SH_IDX WSTL_TEXT_09
#define CHANCON_HL_IDX WSTL_TEXT_19

#define TXTI_SH_IDX   WSTL_TXT_4//WSTL_TXT_3
#define TXTI_HL_IDX   WSTL_TXT_4//WSTL_TXT_3
#define TXTI_SL_IDX   WSTL_TXT_4//WSTL_TXT_3
#define TXTI_GRY_IDX  WSTL_TXT_4//WSTL_TXT_3

#define TXTN_SH_IDX   WSTL_BUTTON_01_FG
#define TXTN_HL_IDX   WSTL_BUTTON_8
#define TXTN_SL_IDX   WSTL_BUTTON_01_FG
#define TXTN_GRY_IDX  WSTL_BUTTON_01_FG

#define LINE_SH_IDX WSTL_WIN_EPG_LINE_01

#define HELP_SH_IDX WSTL_TXT_13

#define CON_SH_IDX   WSTL_BUTTON_2
#define CON_HL_IDX   WSTL_BUTTON_1
#define CON_SL_IDX   WSTL_BUTTON_1
#define CON_GRY_IDX  WSTL_BUTTON_2

#define LSTCON_IDX  WSTL_WIN_1

#define W_L     	0
#define W_T     	90
#define W_W		    720
#define W_H     	400

#define LSTCON_L    (TV_OFFSET_L+66)
#define LSTCON_T    (TV_OFFSET_T+98)
#define LSTCON_W    364
#define LSTCON_H    187

#define LST_PRG_L (LSTCON_L+4)
#define LST_PRG_T (LSTCON_T+30)
#define LST_PRG_W (LSTCON_W-4*2)
#define LST_PRG_H (30*5)

#define SCH_TITLE_L LSTCON_L
#define SCH_TITLE_W LSTCON_W
#define SCH_TITLE_T (TV_OFFSET_T+292)
#define SCH_TITLE_H 192

#define LST_SCH_L LST_PRG_L
#define LST_SCH_T (SCH_TITLE_T+34)
#define LST_SCH_W LST_PRG_W
#define LST_SCH_H (30*5)

#define ITEM_L	    LST_PRG_L
#define ITEM_PRG_T	LST_PRG_T
#define ITEM_SCH_T	LST_SCH_T
#define ITEM_W	    (LST_PRG_W-12)
#define ITEM_H	    30
#define ITEM_GAP	0

#define SCB_L (ITEM_L + ITEM_W)
#define SCB_PRG_T (LST_PRG_T)
#define SCB_SCH_T (LST_SCH_T)
#define SCB_W 12
#define SCB_H (ITEM_H*5)

#define CUR_PRG_BMP_W    18
#define CUR_PRG_BMP_H    18
#define CUR_PRG_BMP_L    ITEM_L

#define SCH_DTL_INFO_L  (TV_OFFSET_L+436)
#define SCH_DTL_INFO_W  234
#define SCH_DTL_INFO_T  SCH_TITLE_T
#define SCH_DTL_INFO_H  192

#define ITEM_PRG_IDX_L     ITEM_L
#define ITEM_PRG_IDX_W     100//60
#define ITEM_PRG_NAME_L    (ITEM_PRG_IDX_L+ITEM_PRG_IDX_W)
#define ITEM_PRG_NAME_W    (ITEM_W-ITEM_PRG_IDX_W-SCB_W)

#define ITEM_SCH_DATE_L     4
#define ITEM_SCH_DATE_W     70
#define ITEM_SCH_TIME_L     (ITEM_SCH_DATE_L+ITEM_SCH_DATE_W)
#define ITEM_SCH_TIME_W     140
#define ITEM_SCH_STATUS_L   (ITEM_SCH_TIME_L+ITEM_SCH_TIME_W)
#define ITEM_SCH_STATUS_W   (ITEM_W-ITEM_SCH_DATE_W-ITEM_SCH_TIME_W-SCB_W)

#define PREVIEW_L	(434+TV_OFFSET_L)
#define PREVIEW_T	(94+TV_OFFSET_T)
#define PREVIEW_W	238
#define PREVIEW_H	194

#define INTER_PREVIEW_L	(PREVIEW_L+18)
#define INTER_PREVIEW_T	(PREVIEW_T+20)
#define INTER_PREVIEW_W	(PREVIEW_W-38)
#define INTER_PREVIEW_H	(PREVIEW_H-40)
#define PREPAUSE_L	(PREVIEW_L + PREVIEW_W - PREBMP_W*2 - 20)
#define PREMUTE_L	(PREVIEW_L + PREVIEW_W - PREBMP_W - 10)
#define PREBMP_T (PREVIEW_T + 10)
#define PREBMP_W 30
#define PREBMP_H 30

#define HELP_INFO_L	LST_PRG_L
#define HELP_INFO_T	(LSTCON_T+4)
#define HELP_INFO_W	LST_PRG_W
#define HELP_INFO_H	24
#define HELP_INFO_IDX	WSTL_TXT_14

TEXT_CONTENT nvod_sch_mtxt_content =
{
    STRING_UNICODE, 0
};

POBJECT_HEAD nvod_sch_status[] =
{
        ( POBJECT_HEAD ) &nvod_sch_status0,
        ( POBJECT_HEAD ) &nvod_sch_status1,
        ( POBJECT_HEAD ) &nvod_sch_status2,
        ( POBJECT_HEAD ) &nvod_sch_status3,
        ( POBJECT_HEAD ) &nvod_sch_status4,
};

static ID nvodTimerID=INVALID_ID;


#define MAX_PLAYED_PROG 5

#define WIN_NVOD_WARN       (POBJECT_HEAD)&g_win_nvod_warn
#define WIN_VOLUME (POBJECT_HEAD)&g_win_volume

static UINT8 Pre_Ch_Mode;
static UINT16 channelIDX;
static UINT8 groupIDX;
static BOOL show_flag = FALSE;
static date_time last_time;
static date_time this_time;

static UINT32 time_remain=0;
static UINT16 nt_event_id=0;
static UINT16 nt_event_sid=0;
BOOL send_enable = FALSE;
BOOL rise_from_standby = FALSE;

/*this arrary hold all the played service under the same reference list*/
static struct nvod_event nvod_service_list[MAX_PLAYED_PROG];
static UINT32 active_service_event_nr;
static UINT32 current_idx;
//TIMER_SET_CONTENT nvodTimerContent[MAX_NVOD_TIMER_NUM];


#define  TIMER_PASED			1
#define  TIMER_FULL    			2
#define  TIMER_CONFILT			3
#define  TIMER_INVALID_CHANNEL	4



static UINT8 find_available_nvod_timer()
{
	UINT8 i;
	SYSTEM_DATA * system_data = sys_data_get();
	resort_timer(TIMER_TYPE_NVOD,&system_data->timer_set.TimerContent[MAX_EPG_TIMER_NUM]);
//	MEMCPY(&system_data->timer_set.TimerContent[MAX_EPG_TIMER_NUM],&nvodTimerContent[0],sizeof(nvodTimerContent));
	for (i = 0; i < MAX_NVOD_TIMER_NUM; i++)
		if (system_data->timer_set.TimerContent[i+MAX_EPG_TIMER_NUM].timer_mode == TIMER_MODE_OFF)
			return i + 1;
	return 0;
}

//void SetNVODTimerMode(UINT8 index, UINT8 mode)
//{
//	nvodTimerContent[index].timer_mode = mode;
//}
static BOOL nvodScanOver = FALSE;//nvod scanned or not

BOOL Get_NVOD_Scan_status()
{
	return nvodScanOver;
}

extern P_NODE nvod_pre_pnode ;
UINT16 nvodpre_channel_index=0;

#if 0
#define LIST_STYLE (LIST_VER | LIST_NO_SLECT     | LIST_ITEMS_NOCOMPLETE |  LIST_SCROLL | LIST_GRID | LIST_FOCUS_FIRST | LIST_PAGE_KEEP_CURITEM | LIST_FULL_PAGE)

LDEF_OL(g_win_nvod, nvod_ref_event_ol, &nvod_sch_title, 1, LST_PRG_L, ITEM_PRG_T, LST_PRG_W, (ITEM_H + ITEM_GAP) *5, LIST_STYLE,  \
    5, 6, nvod_prg_items, &nvod_prg_scb, NULL, NULL ,nvod_ref_event_list_callback)
LDEF_OL ( g_win_nvod, nvod_sch_ol, &nvod_ref_event_detail, 2,LST_SCH_L, ITEM_SCH_T, LST_SCH_W, ( ITEM_H + ITEM_GAP ) *5, LIST_STYLE,\
	5, 10, nvod_sch_items, &nvod_sch_scb, NULL, NULL, nvod_sch_list_callback)

LDEF_CONT(&g_win_nvod, nvod_ref_event_con, &nvod_ref_event_ol, LSTCON_L, LSTCON_T,  \
	LSTCON_W, LSTCON_H, NULL, LSTCON_IDX)

LDEF_WIN(g_win_nvod, &nvod_ref_event_con, W_L, W_T, W_W, W_H, 1)


DEF_TEXTFIELD(nvod_time_remain,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, 60,60,80,24, WSTL_TXT_17,WSTL_TXT_17,WSTL_TXT_17,WSTL_TXT_17,   \
    NULL,NULL,  \
    C_ALIGN_RIGHT| C_ALIGN_VCENTER, 0,0,0,display_strs[32])
#endif

/*******************************************************************************
 *	Local vriable & function declare
 *******************************************************************************/
static UINT8 screen_back_saved = 0XFF;

static BOOL larger_than_1m();
static BOOL need_update_time();
static BOOL WinNvodPlayChannel(P_NODE * para);
static void set_defalut_value();
void Draw_help_info(UINT32 type,UINT16 stringid);
void show_pop_up_info(UINT16 ID);
INT32 nvod_rise_from_timer();
void nvod_back_from_timer();
void nvod_draw_remain_time();
BOOL get_remain_time(INT32* total_time,INT32* h,INT32* m,INT32* sec);
INT32 get_event_for_timer(UINT16 event_d, UINT16 event_sid);
void nvod_stop_play();
UINT32 get_played_event();
BOOL nvod_wait_play();
void reset_update_flag();
void reset_all_flags();
/*******************************************************************************
 *	key mapping and event callback definition
 *******************************************************************************/

static VACTION nvod_sch_list_item_con_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

	return act;
}

static PRESULT nvod_sch_list_item_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
    UINT32 vKey = 0;


    switch ( event )
	{
        case EVN_UNKNOWNKEY_GOT:
            ap_hk_to_vk ( 0, param1, &vKey );
            if((nvodScanOver!=TRUE)&&(vKey!=V_KEY_EXIT)&&(vKey!=V_KEY_MENU))
                ret = PROC_LOOP;
            break;
        default:
            break;
    }
	return ret;
}

#define VACT_PLAY 			(VACT_PASS + 1)
#define VACT_BOOK			(VACT_PASS + 2)
#define VACT_CHAHGE_FOCUS 	(VACT_PASS + 3)
#define VACT_TIMER 			(VACT_PASS + 4)
#define	VACT_MUTE			(VACT_PASS + 5)
#define	VACT_PAUSE			(VACT_PASS + 6)

static VACTION nvod_list_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act;

	switch (key)
	{
		case V_KEY_UP:
			act = VACT_CURSOR_UP;
			break;
		case V_KEY_DOWN:
			act = VACT_CURSOR_DOWN;
			break;
		case V_KEY_P_UP:
			act = VACT_CURSOR_PGUP;
			break;
		case V_KEY_P_DOWN:
			act = VACT_CURSOR_PGDN;
			break;
		case V_KEY_ENTER:
			act = VACT_PLAY;
			break;
		case V_KEY_BLUE:
			act = VACT_BOOK;
			break;
		default:
			act = VACT_PASS;
	}

	return act;
}

static BOOL nvodFullScrPlayFlag = FALSE;  //true: play nvod prog full screen
static BOOL nvodModeFlag = FALSE;	//true:enter full screen play from win_nvod or in win_nvod status
static BOOL nvodFromTimerFlag = FALSE;//true: nvod wake up from timer
static struct nvod_event curEvent;

void SetNvodFromTimerFlag(BOOL b)
{
	nvodFromTimerFlag=b;
}

BOOL GetNvodFromTimerFlag()
{
	return nvodFromTimerFlag;
}

BOOL GetNvodPlayFlag()
{
    return nvodFullScrPlayFlag;
}

void SetNvodPlayFlag()
{
	nvodFullScrPlayFlag=TRUE;
}

void ReSetNvodPlayFlag()
{
	nvodFullScrPlayFlag=FALSE;
}

BOOL GetNvodModeFlag()
{
    return nvodModeFlag;
}

void SetNvodModle(BOOL b)
{
	nvodModeFlag = b;
}

void GetCurEvent(struct nvod_event *event)
{
	if (event == NULL)
		return ;
	MEMCPY(event, &curEvent, sizeof(struct nvod_event));
}

static PRESULT nvod_sch_list_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	VACTION unact;
	UINT16 uni_name[30];
	UINT8 *new_name;
	OBJLIST *ol;
	UINT16 curitem;
	struct nvod_event schEvent;
	UINT16 actualEventNum;
	UINT32 prog_id = 0;

	SYSTEM_DATA * sys_data= sys_data_get();

	ol = &nvod_sch_ol;
	curitem = OSD_GetObjListNewPoint(ol);

	switch (event)
	{
		case EVN_PRE_DRAW:
			win_nvod_sch_set_display();
			if(param1 == C_DRAW_TYPE_HIGHLIGHT)
			{				
				OSD_TrackObject(pObj, C_UPDATE_ALL);
				ret = PROC_LOOP;
			}
			break;
		case EVN_ITEM_POST_CHANGE:
			if(curitem < active_service_event_nr)
			{
				nvod_get_channel_progid(nvod_service_list[curitem].service_id, nvod_service_list[0].t_s_id, &prog_id);
				do_play_nvod_channel(prog_id,NULL);
		 	}
			break;
		case EVN_UNKNOWN_ACTION:
			unact = (VACTION)(param1 >> 16);
			if(!nvodScanOver)
				break;
			date_time localTime, start_time, end_time;
			char strTemp[5];
			INT32 day, h, m, sec;
			get_local_time(&localTime);
			nvod_get_refevent_timelist(curitem, 1, &schEvent, &actualEventNum);

			if (unact == VACT_PLAY || unact == VACT_BOOK)
			{
				GetRefEventStartEndTime(&schEvent, &start_time, &end_time, NULL);
				nvod_get_channel_progid(schEvent.service_id, schEvent.t_s_id, &prog_id);
				CovertByOffset(&start_time);
				CovertByOffset(&end_time);
				get_time_offset(&start_time, &localTime, &day, &h, &m, &sec);
                if((day*24*60+h*60+m)>0)
				{
					if(unact == VACT_PLAY )
					{
						ret = PROC_LEAVE;
	                    nvodFullScrPlayFlag = TRUE;
						MEMCPY(&curEvent, &schEvent, sizeof(struct nvod_event));
					}
				}
				else
				{
					if(unact == VACT_BOOK)
					{
						UINT32 value = find_suited_timer(prog_id, &start_time, &end_time);
						if (value)/*if booked,then cancel;*/
						{
							if(value>=MAX_EPG_TIMER_NUM+1)
							{
								//nvodTimerContent[value -MAX_EPG_TIMER_NUM - 1].timer_mode = TIMER_MODE_OFF;
								sys_data->timer_set.TimerContent[value - 1].timer_mode = TIMER_MODE_OFF;
							}
						}
						else /*if not booked,then book it.*/
						{
							if (NvodProgBook() == 0)
							{
								/*copy to the system_data*/
								//MEMCPY(&sys_data->timer_set.TimerContent[MAX_EPG_TIMER_NUM],nvodTimerContent,sizeof(nvodTimerContent));
								show_pop_up_info(RS_BOOK_SUCCESS);
							}
						}
						OSD_TrackObject((POBJECT_HEAD) &nvod_sch_ol, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
					}
				}
			}
			break;
	}

	return ret;
}

static UINT32 preProgID = 0xFFFFFFFF;

static BOOL WinNvodPlayChannel(P_NODE * para)
{
    date_time localTime,start_time;
	UINT32 idx = 0;
    UINT32 prog_id;

    get_local_time(&localTime);
	
	active_service_event_nr = get_played_event();
	
    if(active_service_event_nr <= 0)
    {
        preProgID = 0xFFFFFFFF;
	    return FALSE;
    }

	nvod_get_channel_progid(nvod_service_list[0].service_id, nvod_service_list[0].t_s_id, &prog_id);
	current_idx = 0;
	return do_play_nvod_channel(prog_id,para);

}



/* this routine will play program with the specified prog_id, but if play failed, 
 * we didn't back to pre channel ,just make sure stop play and show blank.
 */
BOOL do_play_nvod_channel(UINT32 prog_id,P_NODE * pre_para)
{
	enum API_PLAY_TYPE play_ret;
	struct cc_param param;
	UINT16 channel = 0xFFFF;

    if(nvodScanOver != TRUE)
        return FALSE;
    
	if((prog_id != 0xFFFFFFFF)&&(prog_id != preProgID))
	{
		play_ret = api_play_channel(prog_id, TRUE, FALSE, TRUE);
		if(play_ret==API_PLAY_NORMAL)
		{
			channel = get_prog_pos(prog_id);
			if(channel != 0xFFFF)
			{
				sys_data_set_cur_group_channel(channel);
			}
			/*here we got the info of the nvod program for later use */
			memset(&nvod_pre_pnode, 0, sizeof(nvod_pre_pnode));
			get_prog_by_id(prog_id, &nvod_pre_pnode);
            preProgID = prog_id;
			//current_idx = 0;
			goto normal_out;
		}
	}
	/*fall through*/
falied_out:
	if(NULL!=pre_para)
	{
		/*make sure to stop the last played prog,and show black*/
	    MEMSET(&param, 0, sizeof(param));
	    api_set_channel_info(pre_para, &param);
	    chchg_stop_channel(&param.es, &param.dev_list, TRUE);
	}
	return FALSE;

normal_out:
	return TRUE;
}

static PRESULT nvod_ref_event_list_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	VACTION unact;
	OBJLIST *ol;
	UINT16 curitem=0, eventNum = 0;
	struct nvod_ref_event ref;
	struct nvod_event schEvent;
	UINT16 actualEventNum;
	date_time start_time,localTime;
	INT32 day,h,m,sec;
	UINT32 prg_id = 0;
	static P_NODE last_pnode;

	ol = &nvod_ref_event_ol;
	switch (event)
	{
		case EVN_PRE_DRAW:
			if(param1 == C_DRAW_TYPE_HIGHLIGHT)
			{
				win_nvod_ref_event_set_display();
				OSD_TrackObject(pObj, C_UPDATE_ALL);
			}
			ret = PROC_LOOP;
			break;
		case EVN_ITEM_PRE_CHANGE:
			curitem =OSD_GetObjListCurPoint(ol);
			get_local_time(&localTime);
			nvod_get_ref_event(curitem, &ref);
			nvod_get_refevent_timelist(0,1,&schEvent,&actualEventNum);
		    if(actualEventNum == 0)
		    {
		        break;
		    }
		    GetRefEventStartEndTime(&schEvent,&start_time,NULL,NULL);
		    CovertByOffset(&start_time);
		    get_time_offset(&start_time,&localTime,&day,&h,&m,&sec);
		    if((day*24*60+h*60+m)>0)
		    {	
				nvod_get_channel_progid(schEvent.service_id, schEvent.t_s_id, &prg_id);
				if(0xFFFFFFFF!=prg_id) 
				{
					get_prog_by_id(prg_id, &last_pnode);
				}
			}
			break;
		case EVN_ITEM_POST_CHANGE:
			curitem = OSD_GetObjListNewPoint(ol);
			nvod_get_ref_event(curitem, &ref);
			DrawRefEventDetail();
            //nvod_create_refevent_timelist(ref.event_id,ref.ref_service_id,ref.t_s_id,&eventNum);
            //OSD_SetObjListCount (&nvod_sch_ol,eventNum);
#if 0				
            if(eventNum == 0)
            {
            	//win_nvod_sch_set_blank();
                break;
            }
#endif				
            OSD_SetObjListCurPoint(&nvod_sch_ol,0);
			OSD_SetObjListNewPoint(&nvod_sch_ol, 0);
			OSD_SetObjListTop(&nvod_sch_ol, 0);
			OSD_DrawObject((POBJECT_HEAD) &nvod_sch_ol, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);

			if(!nvodScanOver) 
				break;

			WinNvodPlayChannel(&last_pnode);
			break;
		case EVN_UNKNOWN_ACTION:
			if(!nvodScanOver)
				break;
			unact = (VACTION)(param1 >> 16);
			/*if(unact == VACT_OPEN_RESER)
			{
	        	if (OSD_ObjOpen(WIN_RESERVER, 0xFFFFFFFF) != PROC_LEAVE)
	        		menu_stack_push(WIN_RESERVER);
			}
			*/
			break;


	}

	return ret;
}
/*************************************
function: NVODIsValidTimer
parameter:
return:
0	- invalid
1   	- valid
 **************************************/
/*
static INT32 NVODIsValidTimer(TIMER_SET_CONTENT *timer)
{
	INT32 ret, i;
	TIMER_SET_CONTENT *timer1,  *timer2;
	date_time dt, dt2;

	if (timer->timer_mode == TIMER_MODE_OFF)
		return 1;

	MEMSET(&dt, 0, sizeof(date_time));
	MEMSET(&dt2, 0, sizeof(date_time));

	dt.year = timer->wakeup_year;
	dt.month = timer->wakeup_month;
	dt.day = timer->wakeup_day;
	dt.hour = timer->wakeup_time / 3600;
	dt.min = timer->wakeup_time / 60 % 60;
	dt.sec = timer->wakeup_time % 60;

	for (i = 0; i < MAX_NVOD_TIMER_NUM; i++)
	{
		if ( nvodTimerContent[i].timer_mode == TIMER_MODE_OFF ) continue;

		timer2 = &nvodTimerContent[i];

		dt2.year = timer2->wakeup_year;
		dt2.month = timer2->wakeup_month;
		dt2.day = timer2->wakeup_day;
		dt2.hour = timer2->wakeup_time / 3600;
		dt2.min = timer2->wakeup_time / 60 % 60;
		dt2.sec = timer2->wakeup_time % 60;

		ret = api_compare_day_time_ext(&dt, &dt2);
		if (ret == 0)
			return 0;
		ret = api_compare_timer(timer, &nvodTimerContent[i]);

		if ( ret <= 0 ) return 0;

		// timer1: [a,b]; timer2: [c,d]; b<c
		if (ret == 1)
		{
			timer1 = timer;
			timer2 = &nvodTimerContent[i];
		}
		else
		{
			timer1 = &nvodTimerContent[i];
			timer2 = timer;
		}

		if (api_check_timer(timer1, timer2) == 0)
			return 0;
	}

	return 1;
}
*/
extern TIMER_SET_CONTENT timer_set;
/*0 book success  or fail*/
static UINT8 NvodProgBook()
{
	OBJLIST *ol;
	UINT16 curitem;
	struct nvod_event schEvent;
	UINT16 actualEventNum;
	date_time localTime, start_time, end_time, duration_time;
	char strTemp[5];
	INT32 day, h, m, sec;
	UINT8 ret = 0;
	TIMER_SET_CONTENT timer;
	UINT32 prog_id;
	INT8 timer_num=0;
	UINT8 back_saved;
	SYSTEM_DATA *sys_data = sys_data_get();
	struct nvod_ref_event ref;
	UINT32 retValue;
	UINT16 unistr[32];
	INT32 time_len=0;
	INT8 conflict_idx;
	TIMER_SET_CONTENT *conflict_timer;
	win_popup_choice_t pop_choice;

	ol = &nvod_sch_ol;
	curitem = OSD_GetObjListNewPoint(ol);
	get_local_time(&localTime);
	nvod_get_refevent_timelist(curitem, 1, &schEvent, &actualEventNum);
	GetRefEventStartEndTime(&schEvent, &start_time, &end_time, &duration_time);
	CovertByOffset(&start_time);
	CovertByOffset(&end_time);
	get_time_offset(&start_time, &localTime, &day, &h, &m, &sec);
    if((day*24*60+h*60+m)>0)
	{
		return TIMER_PASED; //Have started and can not be booked.
	}
	
	MEMSET(&timer,0,sizeof(TIMER_SET_CONTENT));
	timer.timer_mode = TIMER_MODE_ONCE;
	timer.timer_service = TIMER_SERVICE_CHANNEL;

	nvod_get_channel_progid(schEvent.service_id, schEvent.t_s_id, &prog_id);
	if (prog_id == 0xFFFFFFFF)
		return TIMER_INVALID_CHANNEL;

	timer.wakeup_channel = prog_id;
	//  libc_printf("set timer channel %x\n",prog_id);
	timer.wakeup_chan_mode = sys_data_get_cur_chan_mode();
	timer.wakeup_group_idx = NVOD_GROUP_TYPE;
	timer.wakeup_state = TIMER_STATE_READY;
	//timer.wakeup_message = TIMER_MSG_BIRTHDAY;

	timer.wakeup_year = start_time.year;
	timer.wakeup_month = start_time.month;
	timer.wakeup_day = start_time.day;
	timer.wakeup_time = start_time.hour * 3600+start_time.min * 60 + start_time.sec;
	//timer.wakeup_time = localTime.hour * 60 + localTime.min+1;

	//get_time_offset(&start_time, &end_time, &day, &h, &m, &sec);
	api_get_time_len(&start_time,&end_time,&time_len);
	if(time_len<0)
		time_len = -time_len;
	
	timer.wakeup_duration_time = time_len;
	timer.nvod_event_id = schEvent.event_id;
	timer.nvod_event_servceid = schEvent.service_id;
	//timer.wakeup_duration_time = 1;

	retValue = nvod_get_ref_event(OSD_GetObjListCurPoint(&nvod_ref_event_ol), &ref);
	if (retValue == SUCCESS)
	{
		//convert_gb2312_to_unicode(ref.name, (UINT32)ref.name_len, unistr, 31);
        dvb_to_unicode(ref.name, (UINT32)ref.name_len, unistr, 31, 2312);
		MEMCPY(timer.event_name,(UINT8 *)unistr,40);
	}

	//sys_data->timer_set.timer_num = timer_num;

	conflict_idx = IsValidTimer(&timer);
	if (conflict_idx)
	{	
		/*if conflict with epg timer we also save it,*/
//		if(conflict_idx<32)
//			goto SAVE_TIMEER;
		/*if conflict with nvod timer,we pop for user to chosse */
            UINT16 book_msg[100];
	        UINT16 str_len;
	        UINT8 *s,str[5];
            win_popup_choice_t choice;
            win_compopup_init(WIN_POPUP_TYPE_OKNO);
			s = OSD_GetUnicodeString(CS_MSG_EPG_REPLACE_TIMER1);
			ComUniStrCopyChar((UINT8 *)&book_msg[0], s);
			str_len = ComUniStrLen(book_msg);
			ComUniStrCopyChar((UINT8*)&book_msg[str_len], sys_data->timer_set.TimerContent[conflict_idx - 1].event_name);	
			str_len = ComUniStrLen(book_msg);

			s = OSD_GetUnicodeString(CS_MSG_EPG_REPLACE_TIMER2);
			ComUniStrCopyChar((UINT8 *)&book_msg[str_len], s);

			win_compopup_set_msg(NULL, (UINT8*)book_msg, 0);
			win_compopup_set_frame(160,172,400,200);
			choice=win_compopup_open_ext(&back_saved);
			if(choice==WIN_POP_CHOICE_YES)
			{
				MEMCPY(&sys_data->timer_set.TimerContent[conflict_idx - 1], &timer, sizeof(TIMER_SET_CONTENT));
				sys_data->timer_set.timer_num = timer_num-1;
			}
	        else
			    return TIMER_CONFILT;
	}
	
	/* timer_num has bounds 0-3*/
	timer_num = find_available_nvod_timer();
	if (timer_num == 0)
	{
		//not available timer
		win_compopup_init(WIN_POPUP_TYPE_SMSG);
		win_compopup_set_msg(NULL, NULL, RS_BOOK_FULL);
		win_compopup_open_ext(&back_saved);
		osal_task_sleep(2000);
		win_compopup_smsg_restoreback();

		return TIMER_FULL;
	}
	
/*fall through*/
SAVE_TIMEER:
	ret = insert_timer(TIMER_TYPE_NVOD,&timer, timer_num-1);
	return ret;
}


/*
static PRESULT nvod_sch_callback ( POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2 )
{
	PRESULT ret = PROC_PASS;

	switch ( event )
	{
		case EVN_PRE_OPEN:
			wincom_open_title ( pObj, RS_FAVORITE, 0 );
			break;
		case EVN_POST_OPEN:
			break;
		case EVN_PRE_CLOSE:
			// Make OSD not flickering 
			* ( ( UINT32* ) param2 ) &= ~C_CLOSE_CLRBACK_FLG;
			break;
		case EVN_POST_CLOSE:
			wincom_close_help();
			break;
	}

	return ret;
}
*/


void GetRefEventStartEndTime(struct nvod_event *event, date_time *start_dt, date_time *end_dt, date_time *duration_dt)
{
	date_time duration;

	if (event == NULL || start_dt == NULL)
		return ;

	start_dt->mjd = (event->time[0] << 8) | event->time[1];
	bcd_to_hms(&(event->time[2]), &start_dt->hour, &start_dt->min, &start_dt->sec);
	mjd_to_ymd(start_dt->mjd, &start_dt->year, &start_dt->month, &start_dt->day, &start_dt->weekday);

	bcd_to_hms(&(event->time[5]), &duration.hour, &duration.min, &duration.sec);

	if (end_dt != NULL)
	{
		convert_time_by_offset2(end_dt, start_dt, (INT32)duration.hour, (INT32)duration.min, (INT32)duration.sec);
	}

	if (duration_dt != NULL)
	{
		duration_dt->hour = duration.hour;
		duration_dt->min = duration.min;
		duration_dt->sec = duration.sec;
	}
}

static void win_nvod_sch_set_display(void)
{
	OBJLIST *ol;
	CONTAINER *item,  *chancon;
	TEXT_FIELD *txt;
	BITMAP *bm;
	UINT32 i;
	UINT32 valid_idx;
	UINT16 top, cnt, page, index, curitem;
	P_NODE p_node;
	UINT16 unistr[30];
	char str[40];
	UINT8 focusId;
	UINT16 eventNum = 0;
	date_time start_time, end_time; //event time
	INT32 h, m, sec;
	struct nvod_event eventList[5];
	UINT16 actualEventNum = 0;
	UINT8 *src;
	UINT32 strLen = 0;
	UINT16 nvod_display_str[40];
	INT32 retValue;
	struct nvod_ref_event ref;

	ol = &nvod_sch_ol;

	retValue = nvod_get_ref_event(OSD_GetObjListCurPoint(&nvod_ref_event_ol), &ref);
	if (retValue == SUCCESS)
	{
        nvod_create_refevent_timelist(ref.event_id,ref.ref_service_id,ref.t_s_id,&eventNum);
        OSD_SetObjListCount(ol,eventNum);
	}

	cnt = OSD_GetObjListCount(ol);
	page = OSD_GetObjListPage(ol);
	top = OSD_GetObjListTop(ol);
	curitem = OSD_GetObjListNewPoint(&nvod_sch_ol);

	eventNum = ((cnt - top) >= 5) ? 5 : (cnt - top);
	nvod_get_refevent_timelist(top, eventNum, eventList, &actualEventNum);
	for (i = 0; i < page; i++)
	{
		//item = (CONTAINER*)nvod_sch_items[i];
		item = (CONTAINER*)nvod_sch_ol_ListField[i];
		index = top + i;

		valid_idx = (i < actualEventNum) ? 1 : 0;
		if ( valid_idx )
		{
		}
		else
		{
			STRCPY(str, "");
			unistr[0] = 0;
		}

		get_STC_offset(&h, &m, &sec);

		if (i < actualEventNum)
			GetRefEventStartEndTime(&eventList[i], &start_time, &end_time, NULL);

		convert_time_by_offset2(&start_time, &start_time, h, m, sec);
		convert_time_by_offset2(&end_time, &end_time, h, m, sec);
		/* date */
		txt = (TEXT_FIELD*)OSD_GetContainerNextObj(item);
		if (valid_idx)
			sprintf(str, "%02d/%02d", start_time.month, end_time.day);
		OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)str);

		/* time */
		txt = (TEXT_FIELD*)OSD_GetObjpNext(txt);
		if (valid_idx)
		sprintf(str, "%02d:%02d~%02d:%02d", start_time.hour, start_time.min,  \
				end_time.hour, end_time.min);
		OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)str);

		/* status */
		bm = (BITMAP*)OSD_GetObjpNext(txt);
		if (OSD_GetObjpNext(bm) == NULL)
		{
			OSD_SetObjpNext(bm, nvod_sch_status[i]);
		}
		OSD_SetBitmapContent(bm, 0);
		txt = (TEXT_FIELD*)OSD_GetObjpNext(bm);
		if (valid_idx)
		{
			date_time localTime;
			char strTemp[5];
			INT32 day;
			get_local_time(&localTime);
			get_time_offset(&start_time, &localTime, &day, &h, &m, &sec);

			if((day*24*60+h*60+m)>0)
			{
				src = OSD_GetUnicodeString(RS_NVOD_PLAYED);
				ComUniStrCopyChar((UINT8*)nvod_display_str, src);
				strLen = ComUniStrLen(nvod_display_str);
				sprintf(strTemp, "%d", 60 *h + m);
				ComAscStr2Uni(strTemp, &nvod_display_str[strLen]);
				strLen = ComUniStrLen(nvod_display_str);
				src = OSD_GetUnicodeString(RS_NVOD_MIN);
				ComUniStrCopyChar((UINT8*) &nvod_display_str[strLen], src);

				OSD_SetTextFieldContent(txt, STRING_UNICODE, (UINT32)nvod_display_str);
			}
			else
			{
				if (NVODEventBookedOrNot(eventList[i]) != 0)
				{
					OSD_SetBitmapContent(bm, IM_ICON_CLOCK_01);
					OSD_SetObjpNext(bm, NULL);
					//OSD_SetTextFieldContent ( txt, STRING_ANSI,(UINT32)" ");
				}
				else
				{
					OSD_SetBitmapContent(bm, 0);
					OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)" ");
				}

			}
		}
		else
			OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)str);

	}
}

/*0 means not booked  and other value means the index of timer*/
static UINT8 NVODEventBookedOrNot(struct nvod_event event)
{
	date_time start_time, end_time; //event time
	UINT8 i;
	UINT32 prog_id;

	GetRefEventStartEndTime(&event, &start_time, &end_time, NULL);
	CovertByOffset(&start_time);
	CovertByOffset(&end_time);

	nvod_get_channel_progid(event.service_id, event.t_s_id, &prog_id);

	return find_suited_timer(prog_id, &start_time, &end_time);
}

/*void NVODTimerReset()
{
    UINT8 i;

    for(i = 0;i<MAX_NVOD_TIMER_NUM; i++)
    {
        if(nvodTimerContent[i].timer_mode!=TIMER_MODE_OFF)
        {
            nvodTimerContent[i].timer_mode = TIMER_MODE_OFF;
        }
    }
}*/

static VACTION nvod_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;
	UINT32 hKey;

	switch (key)
	{
		case V_KEY_EXIT:
		case V_KEY_MENU:
		case V_KEY_SUBTITLE:
			act = VACT_CLOSE;
			break;
		case V_KEY_GREEN:
			act = VACT_CHAHGE_FOCUS;
			break;
		case V_KEY_MUTE:
			act = VACT_MUTE;
			break;
		case V_KEY_PAUSE:
			act = VACT_PAUSE;
			break;
		case V_KEY_LEFT:
		case V_KEY_RIGHT:
			ap_vk_to_hk(0, key, &hKey);
			OSD_ObjProc((POBJECT_HEAD) &nvod_ref_event_detail, MSG_TYPE_KEY << 16, hKey, 0);
	        break;
		case V_KEY_POWER:
			power_switch(0);
			break;
		default:
			act = VACT_PASS;
	}

	return act;

}

void win_nvod_msg_translate(UINT8 type, UINT32 param)
{
	ControlMsgType_t msg_type = 0;

	if (type == NVOD_UPDATE_REF_EVENT)
		msg_type = CTRL_MSG_SUBTYPE_STATUS_NVOD_REFEVENT;
	else if (type == NVOD_UPDATE_EVENT_SCH)
		msg_type = CTRL_MSG_SUBTYPE_STATUS_NVOD_SCHEDULE;
	else if (type == NVOD_TIMEOUT)
		msg_type = CTRL_MSG_SUBTYPE_STATUS_NVOD_TIMEOUT;
	else if (type == NVOD_NO_INFO)
		msg_type = CTRL_MSG_SUBTYPE_STATUS_NVOD_NOINFO;
	else if (type == NVOD_SCAN_OVER)
		msg_type = CTRL_MSG_SUBTYPE_STATUS_NVOD_SCANOVER;
       else if(type==NVOD_DIRECT_UPDATE_EVENT)
	      msg_type = CTRL_MSG_SUBTYPE_STATUS_NVOD_DIRECT_UPDATE;
	ap_send_msg(msg_type, param, FALSE);
}

UINT16 NvodGetCurRefEventPos()
{
	return OSD_GetObjListCurPoint(&nvod_ref_event_ol);
}

static PRESULT win_nvod_message_proc(UINT32 msg_type, UINT32 msg_code)
{
	PRESULT ret = PROC_LOOP;
	struct nvod_ref_event ref;
	static UINT8 eventIDX = 0;
	UINT16 eventNum = 0, curItem = 0, cnt = 0;
	INT32 retValue;
	BOOL bret = FALSE;
    UINT8 getTDTtimeout=0;
	//if(msg_type>=108)
	//    libc_printf("****nvod msg type proc : %d\n",msg_type);
//edit for delete ref event without timelist
	/*if(msg_type != CTRL_MSG_SUBTYPE_CMD_TIMEDISPLAYUPDATE)
	{
	    if((msg_type!=CTRL_MSG_SUBTYPE_STATUS_NVOD_SCANOVER)&&(msg_type!=CTRL_MSG_SUBTYPE_STATUS_NVOD_DIRECT_UPDATE))
	    {
	        if(!nvodScanOver)
	            return ret;
	    }
	}
	*/
	curItem = OSD_GetObjListCurPoint(&nvod_ref_event_ol);
	cnt = OSD_GetObjListCount(&nvod_ref_event_ol);
	switch (msg_type)
	{
		case CTRL_MSG_SUBTYPE_CMD_TIMEDISPLAYUPDATE:
			if(need_update_time())
			{
				wincom_open_title_ext(RS_NVOD, IM_TITLE_ICON_PROGRAM);
				if(nvodScanOver)
				{
					if(OSD_GetContainerFocus(&g_win_nvod) == 1)
						OSD_DrawObject((POBJECT_HEAD) &nvod_sch_ol, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
					else if(OSD_GetContainerFocus(&g_win_nvod) == 2)
						OSD_TrackObject((POBJECT_HEAD) &nvod_sch_ol, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
				}
                //restore_mutepause();
			}
			break;
		case CTRL_MSG_SUBTYPE_STATUS_NVOD_REFEVENT:
			break;
		case CTRL_MSG_SUBTYPE_STATUS_NVOD_SCHEDULE:
		case CTRL_MSG_SUBTYPE_STATUS_NVOD_TIMEOUT:
			if (cnt == 0)
			{
                //Draw_help_info(1,RS_NVOD_NO_INFO);
				break;
			}
			if (nvod_get_ref_event(curItem, &ref) != SUCCESS)
				break;
			//Draw_help_info(0,0);
			nvod_create_refevent_timelist(ref.event_id, ref.ref_service_id, ref.t_s_id, &eventNum);
			OSD_SetObjListCount(&nvod_sch_ol, eventNum);
			//libc_printf("-----sch event NUM %d\n",eventNum);
			DrawRefEventDetail();
			OSD_TrackObject((POBJECT_HEAD) &nvod_ref_event_ol, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
			OSD_DrawObject((POBJECT_HEAD) &nvod_sch_ol, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
			break;
		case CTRL_MSG_SUBTYPE_STATUS_NVOD_NOINFO:
            Draw_help_info(1,RS_NVOD_NO_INFO);
			nvodScanOver = TRUE;
			break;
		case CTRL_MSG_SUBTYPE_STATUS_NVOD_SCANOVER:
			nvod_sort_ref_event();
			stop_tdt();
			start_tdt();
			Draw_help_info(0,0);
            nvodScanOver = TRUE;
//edit for delete ref event without timelist
            nvod_rearrange_event();
            recreate_prog_view(VIEW_ALL|PROG_NVOD_MODE, 0);
            //OSD_HideObject((POBJECT_HEAD)&plsWaitInfo,C_UPDATE_ALL);
            //OSD_DrawObject((POBJECT_HEAD)&nvod_switch_window,C_UPDATE_ALL);
            if (nvod_get_ref_event_cnt() == 0)
			Draw_help_info(1,RS_NVOD_NO_INFO);
//edit for delete ref event without timelist
            OSD_TrackObject((POBJECT_HEAD)&nvod_ref_event_ol,C_UPDATE_ALL|C_DRAW_SIGN_EVN_FLG);            
            OSD_DrawObject((POBJECT_HEAD)&nvod_sch_ol,C_UPDATE_ALL|C_DRAW_SIGN_EVN_FLG);
            DrawRefEventDetail();
            
            bret = WinNvodPlayChannel(NULL);
			if(bret==TRUE)
			{
				show_flag = TRUE;
			}
			break;
        case CTRL_MSG_SUBTYPE_CMD_UPDATE_VIEW:
            if(OSD_GetObjListCount(&nvod_sch_ol)>0)
            {
                if(OSD_GetContainerFocus(&g_win_nvod) == 1)
                    OSD_DrawObject((POBJECT_HEAD)&nvod_sch_ol,C_UPDATE_ALL|C_DRAW_SIGN_EVN_FLG);
                else if(OSD_GetContainerFocus(&g_win_nvod) == 2)
                    OSD_TrackObject((POBJECT_HEAD)&nvod_sch_ol,C_UPDATE_ALL|C_DRAW_SIGN_EVN_FLG);
                
            }
            break;
        case CTRL_MSG_SUBTYPE_STATUS_NVOD_DIRECT_UPDATE:
            nvodScanOver = TRUE;
			Draw_help_info(0,0);
            nvod_rearrange_event();
            recreate_prog_view(VIEW_ALL|PROG_NVOD_MODE, 0);

            OSD_TrackObject((POBJECT_HEAD)&nvod_ref_event_ol,C_UPDATE_ALL|C_DRAW_SIGN_EVN_FLG);            
            OSD_DrawObject((POBJECT_HEAD)&nvod_sch_ol,C_UPDATE_ALL|C_DRAW_SIGN_EVN_FLG);
            DrawRefEventDetail();


			if(!GetPauseState())
			{
				/*if no pause, then play*/
                while(!is_time_inited()&&getTDTtimeout<=25)
                {
                    osal_task_sleep(200);
                    getTDTtimeout++;
                }
	            bret = WinNvodPlayChannel(NULL);
	            if(bret==TRUE)
				{
					show_flag = TRUE;
				}
			}
            //OSD_DrawObject((POBJECT_HEAD)&nvod_switch_window,C_UPDATE_ALL);                    
            break;
		case CTRL_MSG_SUBTYPE_STATUS_SIGNAL:
            if(Get_NVOD_Scan_status())
            {
			    wincom_preview_proc();
            }
        default:
			break;
	}

	return ret;
}

//static T_NODE nvodTNode;
extern void psi_nvod_eit_complete();
extern void ca_popup_close();
//extern UINT8 show_and_playchannel;

struct help_item_resource nvod_help[] =
{
    {1,IM_EPG_COLORBUTTON_GREEN,RS_SWITCH_WINDOW},
    {1,IM_EPG_COLORBUTTON_BLUE,RS_BOOK},
    {1,IM_HELP_ICON_LR,RS_EPG_DETAIL},
    {0,RS_HELP_EXIT,RS_HELP_EXIT},
};

extern void ap_epg_call_back(UINT32 tp_id, UINT16 service_id, UINT8 event_type);
extern UINT8 show_and_playchannel;
extern BOOL getBackfromNVOD();
extern void setBackfromNVOD(BOOL flag);
static PRESULT win_nvod_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	VACTION unact;
	struct YCbCrColor bgcolor;
	UINT16 x, y, w, h;
	UINT8 TV_Out;
	P_NODE pNode;
	T_NODE tNode;
	eit_event_info_t *sch_event;
	date_time tmp_date;
	INT32 event_num;
	UINT32 vkey;
	UINT8 str[2];
	struct tp_param tParam;
	struct vdec_device *vdec;
	struct vdec_io_get_frm_para vfrm_param;
	UINT32 addr, len;
	static UINT8 pre_chan_mode;
	static UINT16 channel=0;
	static UINT8 groupIdx=0;
	UINT8 back_saved;
	OSD_RECT rc_con;
	OSD_RECT rc_preview;
	INT16 menu_pos=0;
	INT32 ret_entry=0;
    UINT8 NOINFO_Wait_flag=0;
    UINT8 getTDTtimeout=0;
	switch (event)
	{
		case EVN_PRE_OPEN:
        	if((-1==nvod_get_tp_entry(&tNode)))
			{
				win_compopup_init(WIN_POPUP_TYPE_SMSG);
				win_compopup_set_title(NULL,NULL,RS_INFO_INFO);
				win_compopup_set_msg(NULL, NULL, RS_NVOD_NO_INFO);
				win_compopup_open_ext(&back_saved);		
				osal_task_sleep(1000);
				win_compopup_smsg_restoreback();	
				ret = PROC_LEAVE;
				break;
			}
		    key_pan_display("NUOD", 4); //There is no "v" in bitmap_list,so use "u" as "v".
		    if(FALSE == nvodModeFlag && (FALSE == GetNvodFromTimerFlag()))    // record the channel model,groupindex and group when it is not nvod model before
		    {
				record_scene();
			    pre_chan_mode = sys_data_get_cur_chan_mode();
			    groupIdx = sys_data_get_cur_group_index();
			    channel = sys_data_get_cur_group_cur_mode_channel();
		    }
			
			if(INVALID_ID!=nvodTimerID)
			{
				OS_DeleteTimer(nvodTimerID);
				nvodTimerID = INVALID_ID;
			}
			if(GetNvodFromTimerFlag())
			{
				/*reset the flag nvodFromTimerFlag*/
				SetNvodFromTimerFlag(FALSE);
			}


            if(!nvodModeFlag)
            {
                dm_set_onoff(0);
                api_stop_play(0);
                epg_off();
                //epg_release();
                nvod_ref_list_init();
                nvod_sch_list_init();
            }
            nvod_ref_event_detail.pTextTable = &nvod_sch_mtxt_content;
            nvod_sch_mtxt_content.text.pString = NULL;
			/*wincom_close_title();
			wincom_open_title_ext(RS_NVOD, IM_TITLE_ICON_PROGRAM);
			wincom_open_help(nvod_help, 4);
			/*preview* /
			bgcolor.uY = 185;
			bgcolor.uCb = 143;
			bgcolor.uCr = 116;

			vpo_ioctl(g_vpo_dev, VPO_IO_SET_BG_COLOR, (UINT32) &bgcolor);
			TV_Out = api_video_get_tvout();
			win_nvod_get_preview_rect(&x, &y, &w, &h);
			//hde_config_preview_window ( x, y, w, h, TV_Out == TV_MODE_PAL );
			hde_config_preview_window(x, y, w, h, 1); //always PAL(When change to N,hardware scale.)
			vpo_ioctl(g_vpo_dev, VPO_IO_DIRECT_ZOOM, 0);
			hde_set_mode(VIEW_MODE_PREVIEW);

			nvod_display_str_init(29);
			OSD_SetContainerFocus(&g_win_nvod, 1);


            if(!nvodScanOver)
            {
               NOINFO_Wait_flag=1;// Draw_help_info(1,RS_NVOD_WAIT);
            }

            if(!nvodFullScrPlayFlag)
            {
    	        struct VDec_StatusInfo CurStatus;
                vdec_io_control((struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0),VDEC_IO_GET_STATUS,(UINT32)&CurStatus);
        		//show blank screen in preview box
        		struct YCbCrColor tColor;
        		tColor.uY = 0x10;
        		tColor.uCb = 0x80;
        		tColor.uCr = 0x80;
        		vdec_io_control((struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0), VDEC_IO_FILL_FRM, (UINT32)(&tColor));
        		vpo_win_onoff((struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS),TRUE);			
            }*/

		    nvodFullScrPlayFlag = FALSE;
			reset_update_flag();

            if(screen_back_state == SCREEN_BACK_RADIO)
            {
                screen_back_state = SCREEN_BACK_VIDEO;/*fix bug:If enter nvod mode when a radio prog was playing,
                                                        then radio logo will not be showed when exit the nvod.*/
                sys_data_set_cur_chan_mode(TV_CHAN);//1 withour this line,the screen_back_state will be set back to SCREEN_BACK_RADIO
                                                    //when call api_play_channel(play nvod prog)
                                                    //2 channel mode will be recovered by function restore_scene
                                                    //3 there is no radio prog in nvod by now.
            }
			ret_entry = nvod_get_tp_entry(&tNode);

			if(!ret_entry)
				goto real_entry;

			//Draw_help_info(1,RS_NVOD_WAIT);
			if(nvod_get_module_status()==NVOD_STATUS_SCAN_OVER)
			{
				/*research nvod*/
				nvod_close();
				nvod_stop_play();
				reset_all_flags();
				nvod_release();
				nvod_init(system_config.main_frequency, system_config.main_symbol, system_config.main_qam);
			}

real_entry:
			tParam.constellation = tNode.FEC_inner;
			tParam.freq = tNode.frq;
			tParam.symbol = tNode.sym;
			if(SUCCESS != nvod_open(NVOD_MODE_MULTI_TP,&tParam,win_nvod_msg_translate,psi_nvod_eit_complete))
			{
				NOINFO_Wait_flag=2;//Draw_help_info(1,RS_NVOD_NO_INFO);
				break;
			}

            sys_data_set_cur_group_index(NVOD_GROUP_TYPE);
            
            if(!getBackfromNVOD())
            {
                win_compopup_init(WIN_POPUP_TYPE_SMSG);
                win_compopup_set_msg(NULL, NULL, RS_NVOD_WAIT);
                win_compopup_open_ext(&back_saved);
               while(!is_time_inited()&&getTDTtimeout<=25)
               {
                    osal_task_sleep(200);
                    getTDTtimeout++;
               }
               win_compopup_smsg_restoreback();
            }else
                setBackfromNVOD(FALSE);



            wincom_close_title();
			wincom_open_title_ext(RS_NVOD, IM_TITLE_ICON_PROGRAM);
			wincom_open_help(nvod_help, 4);
            
			/*preview*/
			bgcolor.uY = 185;
			bgcolor.uCb = 143;
			bgcolor.uCr = 116;

			vpo_ioctl(g_vpo_dev, VPO_IO_SET_BG_COLOR, (UINT32) &bgcolor);
			TV_Out = api_video_get_tvout();
			win_nvod_get_preview_rect(&x, &y, &w, &h);
			//hde_config_preview_window ( x, y, w, h, TV_Out == TV_MODE_PAL );
			hde_config_preview_window(x, y, w, h, 1); //always PAL(When change to N,hardware scale.)
			vpo_ioctl(g_vpo_dev, VPO_IO_DIRECT_ZOOM, 0);
			hde_set_mode(VIEW_MODE_PREVIEW);

			nvod_display_str_init(29);
			OSD_SetContainerFocus(&g_win_nvod, 1);


            if(!nvodScanOver)
            {
               NOINFO_Wait_flag=1;// Draw_help_info(1,RS_NVOD_WAIT);
            }

            if(!nvodFullScrPlayFlag)
            {
    	        struct VDec_StatusInfo CurStatus;
                vdec_io_control((struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0),VDEC_IO_GET_STATUS,(UINT32)&CurStatus);
        		//show blank screen in preview box
        		struct YCbCrColor tColor;
        		tColor.uY = 0x10;
        		tColor.uCb = 0x80;
        		tColor.uCr = 0x80;
        		vdec_io_control((struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0), VDEC_IO_FILL_FRM, (UINT32)(&tColor));
        		vpo_win_onoff((struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS),TRUE);			
            }
			break;
		case EVN_POST_OPEN:       
			rc_con.uLeft = PREVIEW_L;
			rc_con.uTop = PREVIEW_T;
			rc_con.uWidth = PREVIEW_W;
			rc_con.uHeight = PREVIEW_H;

			rc_preview.uLeft = INTER_PREVIEW_L;
			rc_preview.uTop = INTER_PREVIEW_T;
			rc_preview.uWidth = INTER_PREVIEW_W;
			rc_preview.uHeight = INTER_PREVIEW_H;

			wincom_open_preview(rc_con, rc_preview, WSTL_EPG_PREVIEW);//WSTL_SUBMENU_PREVIEW);   
			//win_nvod_draw_preview_window_ext();
			if(NOINFO_Wait_flag==2)
                Draw_help_info(1,RS_NVOD_NO_INFO);
            else if(NOINFO_Wait_flag=1)
                Draw_help_info(1,RS_NVOD_WAIT);
           /* if(!nvodScanOver)
            {
                Draw_help_info(1,RS_NVOD_WAIT);
            }

            if(!nvodFullScrPlayFlag)
            {
    	        struct VDec_StatusInfo CurStatus;
                vdec_io_control((struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0),VDEC_IO_GET_STATUS,(UINT32)&CurStatus);
        		//show blank screen in preview box
        		struct YCbCrColor tColor;
        		tColor.uY = 0x10;
        		tColor.uCb = 0x80;
        		tColor.uCr = 0x80;
        		vdec_io_control((struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0), VDEC_IO_FILL_FRM, (UINT32)(&tColor));
        		vpo_win_onoff((struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS),TRUE);			
            }

		    nvodFullScrPlayFlag = FALSE;
			reset_update_flag();

            if(screen_back_state == SCREEN_BACK_RADIO)
            {
                screen_back_state = SCREEN_BACK_VIDEO;/*fix bug:If enter nvod mode when a radio prog was playing,
                                                        then radio logo will not be showed when exit the nvod.*/
       /*         sys_data_set_cur_chan_mode(TV_CHAN);//1 withour this line,the screen_back_state will be set back to SCREEN_BACK_RADIO
                                                    //when call api_play_channel(play nvod prog)
                                                    //2 channel mode will be recovered by function restore_scene
                                                    //3 there is no radio prog in nvod by now.
            }
			ret_entry = nvod_get_tp_entry(&tNode);

			if(!ret_entry)
				goto real_entry;

			Draw_help_info(1,RS_NVOD_WAIT);
			if(nvod_get_module_status()==NVOD_STATUS_SCAN_OVER)
			{
				/*research nvod* /
				nvod_close();
				nvod_stop_play();
				reset_all_flags();
				nvod_release();
				nvod_init(system_config.main_frequency, system_config.main_symbol, system_config.main_qam);
			}

real_entry:
			tParam.constellation = tNode.FEC_inner;
			tParam.freq = tNode.frq;
			tParam.symbol = tNode.sym;
			if(SUCCESS != nvod_open(NVOD_MODE_MULTI_TP,&tParam,win_nvod_msg_translate,psi_nvod_eit_complete))
			{
				Draw_help_info(1,RS_NVOD_NO_INFO);
				break;
			}
			*/
			nvodModeFlag = TRUE;

			//restore_mutepause();
			/* if delete this code , from nvod fullscreen play, will be not play again */
			preProgID = 0xFFFFFFFF;
		    break;
		case EVN_UNKNOWNKEY_GOT:
			ret = win_nvod_message_proc(param1, param2);
			break;
		case EVN_UNKNOWN_ACTION:
			unact = (VACTION)(param1 >> 16);
			ret = win_nvod_unkown_act_proc(unact);
			break;
		case EVN_MSG_GOT:
			ret = win_nvod_message_proc(param1, param2);
			break;
		case EVN_PRE_CLOSE:
			menu_pos = window_pos_in_stack((POBJECT_HEAD) &g_win_mainmenu);
			if (menu_pos != -1)
			{
				if(!nvodFullScrPlayFlag)
				{
					*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
					break;
				}
			}
			wincom_close_help();
			wincom_close_title();
			wincom_close_preview();
			break;
		case EVN_POST_CLOSE:

			bgcolor.uY = 16;
			bgcolor.uCb = 128;
			bgcolor.uCr = 128;
			vpo_ioctl ( g_vpo_dev, VPO_IO_SET_BG_COLOR, ( UINT32 ) &bgcolor );
			hde_set_mode ( VIEW_MODE_FULL );

            if(nvodFullScrPlayFlag)
            {
				if(-1 != window_pos_in_stack((POBJECT_HEAD)&g_win_mainmenu))
				{
					menu_stack_pop_all();
				}

				if(!GetNvodFromTimerFlag())
					nvod_draw_remain_time();
				//else
				//	nvod_draw_remain_time_for_timer();

				NvodUpdateTimeList();

				key_pan_display("nuod", 4);
            }
            else
            {
				/*make sure to stop play the nvod program*/
				nvod_close();
				nvod_stop_play();
				reset_all_flags();
				UINT8 av_flag;

            	//epg_init(SIE_EIT_WHOLE_TP, NULL, 0x100000, ap_epg_call_back);
                dm_set_onoff(1);


            	/*return to the previous db view, group, channel,*/
 				sys_data_set_cur_chan_mode(pre_chan_mode);
            	sys_data_change_group ( groupIdx );
            	sys_data_set_cur_group_channel(channel);


				restore_scene();

				av_flag = sys_data_get_cur_chan_mode();
				if (sys_data_get_sate_group_num(av_flag))
					key_pan_display("----", 4);
				else
				{
					/*no normal channel return to mainmenu*/
					key_pan_display("noCH", 4);
					win_compopup_init(WIN_POPUP_TYPE_SMSG);
					win_compopup_set_msg(NULL, NULL, RS_MSG_NO_CHANNELS);
					win_compopup_open_ext(&back_saved);
					osal_task_sleep(1000);
					win_compopup_smsg_restoreback();

					ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32)((POBJECT_HEAD)&g_win_mainmenu), TRUE);
					break;
				}

            	
                if(menu_stack_get(0)!=(POBJECT_HEAD)&g_win_mainmenu)
                {
            		show_and_playchannel = 1;
            		ap_send_msg ( CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, ( POBJECT_HEAD ) &g_win_progname, TRUE );
                }
                else
                {
                        OSD_DrawObject((POBJECT_HEAD)&g_win_mainmenu,C_UPDATE_ALL);
                }

			}

			break;
	}

	return ret;
}


static void win_nvod_ref_event_set_display(void)
{
	OBJLIST *ol;
	CONTAINER *item,  *chancon;
	TEXT_FIELD *txt;
	UINT32 i;
	UINT32 valid_idx;
	UINT16 top, cnt, page, index, curitem;
	UINT16 unistr[30], refEventNum = 0;
	char str[10];
	UINT8 focusId;
	struct nvod_ref_event ref;

	ol = &nvod_ref_event_ol;

	refEventNum = nvod_get_ref_event_cnt();
	if (refEventNum == 0xFFFF)
		refEventNum = 0;
	OSD_SetObjListCount(ol, refEventNum);
	cnt = OSD_GetObjListCount(ol);
	page = OSD_GetObjListPage(ol);
	top = OSD_GetObjListTop(ol);
	curitem = OSD_GetObjListNewPoint(ol);

	for (i = 0; i < page; i++)
	{
		//item = (CONTAINER*)nvod_prg_items[i];
		item = (CONTAINER*)nvod_ref_event_ol_ListField[i];
		index = top + i;

		valid_idx = (index < cnt) ? 1 : 0;
		if (valid_idx)
		{
			nvod_get_ref_event(index, &ref);
		}
		else
		{
			STRCPY(str, "");
			unistr[0] = 0;
		}


		/* IDX */
		txt = (TEXT_FIELD*)OSD_GetContainerNextObj(item);
		if (valid_idx)
			sprintf(str, "%04d", index + 1);
		OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)str);

		/* Name */
		txt = (TEXT_FIELD*)OSD_GetObjpNext(txt);
		if (valid_idx)
		{
			//convert_gb2312_to_unicode(ref.name, (INT32)ref.name_len, unistr, 29);
            dvb_to_unicode(ref.name, (INT32)ref.name_len, unistr, 29, 2312);
			OSD_SetTextFieldContent(txt, STRING_UNICODE, (UINT32)unistr);
		}
		else
			OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)str);

	}
}

void win_nvod_get_preview_rect(UINT16 *x, UINT16 *y, UINT16 *w, UINT16 *h)
{
	struct OSDRect osd_rect;
	UINT16 left, top, width, height;
	INT32 left_offset, top_offset, width_offset, height_offset;

#define BORDER_WIDHT    2

	OSDDrv_GetRegionPos((HANDLE)g_osd_dev, 0, &osd_rect);

	left_offset = BORDER_WIDHT;
	width_offset = BORDER_WIDHT * 2;

	left = PREVIEW_L;
	top = PREVIEW_T;
	width = PREVIEW_W;
	height = PREVIEW_H;

	top_offset = BORDER_WIDHT;
	height_offset = BORDER_WIDHT * 2;

	*x = osd_rect.uLeft + left + left_offset;
	*y = osd_rect.uTop + top + top_offset;
	*w = width - width_offset;
	*h = height - height_offset-5;
}



static PRESULT win_nvod_unkown_act_proc(VACTION act)
{
	PRESULT ret = PROC_LOOP;
	UINT8 av_mode;
	CONTAINER *item;
	UINT16 cur_point, top_idx;
	OBJLIST *prg_ol = &nvod_ref_event_ol;
	OBJLIST *sch_ol = &nvod_sch_ol;
	UINT32 prg_id=0;

	switch (act)
	{
		case VACT_MUTE:
		case VACT_PAUSE:
			if (act == VACT_MUTE)
				SetMuteOnOff(FALSE);
			else
			{
				av_mode = sys_data_get_cur_chan_mode();
				if (av_mode == TV_CHAN)
					SetPauseOnOff(FALSE);
			}
			break;
		case VACT_CHAHGE_FOCUS:
			if (OSD_GetContainerFocus(&g_win_nvod) == 1)
			{
				if (OSD_GetObjListCount(sch_ol) == 0)
					break;

				OSD_SetContainerFocus(&g_win_nvod, 2);
                //OSD_DrawObject((POBJECT_HEAD)&nvod_book,C_UPDATE_ALL);

				cur_point = OSD_GetObjListCurPoint(prg_ol);
				top_idx = OSD_GetObjListTop(prg_ol);
				//item = (CONTAINER*)nvod_prg_items[cur_point - top_idx];
				item = (CONTAINER*)nvod_ref_event_ol_ListField[cur_point - top_idx];
				OSD_DrawObject((POBJECT_HEAD)item, C_UPDATE_ALL);
				nvod_draw_cur_prg_bmp();

				cur_point = OSD_GetObjListCurPoint(sch_ol);
				top_idx = OSD_GetObjListTop(sch_ol);
				//item = (CONTAINER*)nvod_sch_items[cur_point - top_idx];_ol_ListField
				item = (CONTAINER*)nvod_sch_ol_ListField[cur_point - top_idx];
				OSD_TrackObject((POBJECT_HEAD)item, C_UPDATE_ALL);
				/*play the right channel */
				if(cur_point < active_service_event_nr)
				{
					nvod_get_channel_progid(nvod_service_list[cur_point].service_id, nvod_service_list[0].t_s_id, &prg_id);
					do_play_nvod_channel(prg_id,NULL);
			 	}
				//wincom_close_help();
				//wincom_open_help((POBJECT_HEAD)&g_win_nvod,nvod_help_2, (6<<16)|HELP_ITMES_SIGNAL_LINE,WSTL_HELPBACK_COMMON);
			}
			else if (OSD_GetContainerFocus(&g_win_nvod) == 2)
			{
				OSD_SetContainerFocus(&g_win_nvod, 1);
                //OSD_HideObject((POBJECT_HEAD)&nvod_book,C_UPDATE_ALL);
				cur_point = OSD_GetObjListCurPoint(prg_ol);
				top_idx = OSD_GetObjListTop(prg_ol);
				//item = (CONTAINER*)nvod_prg_items[cur_point - top_idx];
				item = (CONTAINER*)nvod_ref_event_ol_ListField[cur_point - top_idx];
				OSD_TrackObject((POBJECT_HEAD)item, C_UPDATE_ALL);

				cur_point = OSD_GetObjListCurPoint(sch_ol);
				top_idx = OSD_GetObjListTop(sch_ol);
				//item = (CONTAINER*)nvod_sch_items[cur_point - top_idx];
				item = (CONTAINER*)nvod_sch_ol_ListField[cur_point - top_idx];
				OSD_DrawObject((POBJECT_HEAD)item, C_UPDATE_ALL);
				//wincom_open_help((POBJECT_HEAD)&g_win_nvod,nvod_help_1, (5<<16)|HELP_ITMES_SIGNAL_LINE,WSTL_HELPBACK_COMMON);
			}
			break;
		default:
			break;
	}
	return ret;
}

static void	nvod_ref_list_init()
{
	OBJLIST	*ol;

	ol = &nvod_ref_event_ol;

	OSD_SetObjListCount ( ol, 0 );
	OSD_SetObjListCurPoint ( ol, 0 );
	OSD_SetObjListNewPoint ( ol, 0 );
	OSD_SetObjListTop ( ol, 0 );
}

static void	nvod_sch_list_init()
{
	OBJLIST	*ol;

	ol = &nvod_sch_ol;

	OSD_SetObjListCount ( ol, 0 );
	OSD_SetObjListCurPoint ( ol, 0 );
	OSD_SetObjListNewPoint ( ol, 0 );
	OSD_SetObjListTop ( ol, 0 );
}

static void nvod_draw_cur_prg_bmp()
{
	OSD_RECT rect;
	OBJLIST *ol = &nvod_ref_event_ol;
	UINT16 cur_point, top_idx;

	cur_point = OSD_GetObjListCurPoint(ol);
	top_idx = OSD_GetObjListTop(ol);

	nvod_cur_prog_bmp.head.frame.uTop = LST_PRG_T + (ITEM_H + ITEM_GAP)*(cur_point - top_idx) + (ITEM_H - CUR_PRG_BMP_H) / 2;
	OSD_DrawObject((POBJECT_HEAD) &nvod_cur_prog_bmp, C_UPDATE_ALL);
}

static void nvod_display_str_init(UINT8 max)
{
	UINT8 i = 0;

	for (i = 0; i <= max; i++)
	{
		MEMSET(display_strs[i], 0, sizeof(display_strs[i]));
	}
}


UINT16 refEventBuf[256];

static void DrawRefEventDetail()
{
    UINT16 curItem = 0;
    struct nvod_ref_event ref;
	UINT32 vscr_idx;
	lpVSCR apVscr;
	
	vscr_idx = osal_task_get_current_id();
	apVscr = OSD_GetTaskVscr(vscr_idx);
    

    curItem = OSD_GetObjListCurPoint(&nvod_ref_event_ol);
    if(nvod_get_ref_event(curItem,&ref)==SUCCESS)
    {
        MEMSET((UINT8*)refEventBuf,0,sizeof(refEventBuf));
        //convert_gb2312_to_unicode(ref.txt,ref.txt_len,refEventBuf,255);
        dvb_to_unicode(ref.txt,ref.txt_len,refEventBuf,255,2312);
        nvod_sch_mtxt_content.text.pString = refEventBuf;
    }
    else
    {
        nvod_sch_mtxt_content.text.pString = NULL;
    }
	OSD_SetMultiTextLine(&nvod_ref_event_detail,0);
    OSD_DrawObject((POBJECT_HEAD)&nvod_ref_event_detail,C_UPDATE_ALL);
    OSD_UpdateVscr(apVscr);
}

static VACTION nvod_detail_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

	switch (key)
	{
		case V_KEY_LEFT:
			act = VACT_CURSOR_PGUP;
			break;
		case V_KEY_RIGHT:
			act = VACT_CURSOR_PGDN;
			break;
		default:
			act = VACT_PASS;
	}

	return act;

}

static PRESULT nvod_detail_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;

	return ret;
}

/*
static void DrawPlsWaitInfo(UINT16 ID)
{
    TEXT_FIELD *txt = &plsWaitInfo;

    OSD_SetTextFieldContent ( txt, STRING_ID, ID);
    OSD_DrawObject((POBJECT_HEAD)&nvod_con_waitinfo,C_UPDATE_ALL);
}
*/

static void NVODTimerProc(void)
{
	INT32 total_time,ih,im,is;
	static UINT32 times =0;

	if(get_remain_time(&total_time,&ih,&im,&is))
	{
		if(GetNvodPlayFlag())
		{
			send_enable = TRUE;
			ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_NVOD_TIME_UPDATE, 0, TRUE);
		}
	}
	else
	{
		/*now the timer must be running, so it's time to stop it*/
		if(GetNvodPlayFlag()&&send_enable)
			ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32)((POBJECT_HEAD)&g_win_nvod), TRUE);	
	}
	
	
}

static void NvodSendUpdateMsg(UINT para)
{
	static UINT8 cnt = 0;

	cnt++;
	if (cnt % 2 == 0)
	{
		ap_send_msg(CTRL_MSG_SUBTYPE_CMD_UPDATE_VIEW, 0, TRUE);
		cnt = 0;
	}
	NVODTimerProc();
	//    libc_printf("nvod timer\n");
}

void NvodUpdateTimeList(void)
{
	OSAL_T_CTIM t_dalm;

	t_dalm.callback = NvodSendUpdateMsg;
	t_dalm.type = TIMER_CYCLIC;
	t_dalm.time = 1000; //1 seconds

	if(nvodTimerID==INVALID_ID)
	{
		nvodTimerID = osal_timer_create(&t_dalm);
		// start~!
		osal_timer_activate(nvodTimerID, TRUE);
	}
}

void CovertByOffset(date_time *dt)
{
	INT32 h, m, s;

	get_STC_offset(&h, &m, &s);
	convert_time_by_offset(dt, dt, h, m);
}

void SetNvodScanOverFlag(BOOL flag)
{
	nvodScanOver = flag;
}


static BOOL need_update_time()
{
	get_local_time(&this_time);
	if(larger_than_1m())
	{
		last_time = this_time;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
	
}

static BOOL larger_than_1m()
{
	if(this_time.year != last_time.year)  return TRUE;
	if(this_time.month != last_time.month)  return TRUE;
	if(this_time.day != last_time.day)  return TRUE;
	if(this_time.hour != last_time.hour)  return TRUE;
	if((this_time.min > last_time.min) && (this_time.min - last_time.min >= 1))
		return TRUE;
	if((this_time.min < last_time.min) && (last_time.min - this_time.min >= 1))
		return TRUE;

	return FALSE;
		
}


static void set_defalut_value()
{
		preProgID = 0xFFFFFFFF;
		show_flag = FALSE;

}
void Draw_help_info(UINT32 type,UINT16 stringid)
{
    TEXT_FIELD *txt = &nvod_help_info;
	char buf[5]={0};

	if(type)
	{
		txt->pString = NULL;
    	OSD_SetTextFieldContent ( txt, STRING_ID, stringid);
    }
	else
	{
		txt->pString = NULL;
		OSD_SetTextFieldContent ( txt, STRING_ID, 0);
	}
    OSD_DrawObject((POBJECT_HEAD)&nvod_help_info,C_UPDATE_ALL);
}

void show_pop_up_info(UINT16 ID)
{
	UINT8 back_saved;
	win_compopup_init(WIN_POPUP_TYPE_SMSG);
	win_compopup_set_msg(NULL, NULL, ID);
	win_compopup_open_ext(&back_saved);
	osal_task_sleep(500);
	win_compopup_smsg_restoreback();
	
}

/*clear the nvod_timer which user exit from
 *there must be only one timer is in TIMER_MODE_OFF state
 */
/*void nvod_clear_current_timer()
{
	UINT32 index;
	TIMER_SET_CONTENT *timer;

	for (index = 0; index < MAX_NVOD_TIMER_NUM; index++)
	{
		timer = &nvodTimerContent[index];
		if (timer->wakeup_state != TIMER_STATE_RUNING)
		{
			continue;
		}
		else
		{
			timer->timer_mode = TIMER_MODE_OFF;
			break; 
		} 
	}
}*/



/*popup a window for user to choose, if yes, record the current channel model,channel,groupindex,
 *then create nvod view,and change to nvod mode,
 */
INT32 nvod_rise_from_timer()
{
	/*nor matter what mode, force to nvod group and create nvod data view*/
	sys_data_change_group(NVOD_GROUP_TYPE);
	SetNvodPlayFlag();
	SetNvodFromTimerFlag(TRUE);
	nvodModeFlag = TRUE;
	system_state = SYS_STATE_NVOD;
	OSD_SetTextFieldContent((PTEXT_FIELD)& nvod_time_remain, STRING_ANSI, (UINT32)" "); 
	return get_event_for_timer(nt_event_id,nt_event_sid);
}


void nvod_back_from_timer()
{
    api_stop_play(0);
    ReSetNvodPlayFlag();
    //ReSetNvodModelFlag();
    //ReSetNvodViaTimer();
	
    /* restore the previous channel mode,channel,groupindex*/
	
    sys_data_change_group(groupIDX);
    sys_data_set_cur_group_channel(channelIDX);
}

static void nvod_chang_channel(UINT32 vkey)
{
	UINT32 idx =0;
	UINT32 prog_id;
	
	if(V_KEY_DOWN == vkey)
	{
		current_idx = (current_idx+1)%active_service_event_nr;
	}
	else
	{
		if(current_idx>=1)
		{
			current_idx = (current_idx-1)%active_service_event_nr;
		}
		else
		{
			current_idx = active_service_event_nr-1;
		}
	}
	
	nvod_get_channel_progid(nvod_service_list[current_idx].service_id, nvod_service_list[current_idx].t_s_id, &prog_id);
	do_play_nvod_channel(prog_id,NULL);
	
}

UINT32 get_played_event()
{
	UINT32 idx = 0 ;
	UINT16 tmp = 0;
	UINT32 count = 0;
	date_time start_time;
	date_time localTime;
	INT32 day=0,h=0,m=0,sec=0;


	get_local_time(&localTime);
	MEMSET(nvod_service_list,0,sizeof(nvod_service_list));
	
	while(-1!=nvod_get_refevent_timelist(idx,1,&nvod_service_list[idx],&tmp))
	{
		GetRefEventStartEndTime(&nvod_service_list[idx],&start_time,NULL,NULL);
	    CovertByOffset(&start_time);
	    get_time_offset(&start_time,&localTime,&day,&h,&m,&sec);
	    if((day*24*60+h*60+m)<0)
		{
			break;
		}
		else
		{
			count += tmp;
			idx++;
			if(idx>MAX_PLAYED_PROG)
				break;
		}	
	}
	return count;
	
}


void reset_all_flags()
{
	show_flag = FALSE;
	nvodScanOver = FALSE;
	nvodModeFlag = FALSE;
	nvodFullScrPlayFlag = FALSE;
	nvodFromTimerFlag = FALSE;
	current_idx = 0;
	preProgID = 0xFFFFFFFF;
	reset_update_flag();
	
}

BOOL get_remain_time(INT32* total_time,INT32* h,INT32* m,INT32* sec)
{
	INT32 time_span=0;
	date_time localTime,start_time,end_time,duration_time;
	
	get_local_time(&localTime);
	GetRefEventStartEndTime(&curEvent, &start_time, &end_time, &duration_time);
	CovertByOffset(&start_time);
	CovertByOffset(&end_time);
	api_get_time_len(&localTime,&end_time,&time_span);
	*total_time = time_span;
	if(time_span>0)
		return TRUE; 
	else
		return FALSE;
}

void nvod_draw_remain_time()
{
	TEXT_FIELD * p_txt=NULL;
	INT32 total_time,ih,im,is;
	UINT8 char_buf[16];

	p_txt = &nvod_time_remain;

	MEMSET(char_buf,0,sizeof(char_buf));
	if(get_remain_time(&total_time,&ih,&im,&is) && GetNvodPlayFlag())
	{
		sprintf(char_buf,"%02d:%02d:%02d",total_time/3600,total_time/60%60,total_time%60);
		OSD_SetTextFieldContent(p_txt, STRING_ANSI, (UINT32)char_buf);
		OSD_DrawObject((POBJECT_HEAD)p_txt, C_UPDATE_ALL);
	}
}





INT32 get_event_for_timer(UINT16 event_d, UINT16 event_sid)
{
	struct nvod_event nent;
	INT32 ret_val = 0;
	
	MEMSET(&nent,0,sizeof(struct nvod_event));
	ret_val=nvod_get_event_byid(event_d,event_sid,&nent);
	if(!ret_val)	
		MEMCPY(&curEvent,&nent,sizeof(struct nvod_event));
		
	return ret_val;
}

void nvod_stop_play()
{
	struct cc_param param;
	MEMSET(&param, 0, sizeof(param));
	api_set_channel_info(&nvod_pre_pnode, &param);
	chchg_stop_channel(&param.es, &param.dev_list, TRUE);
	preProgID = 0xFFFFFFFF;
}


BOOL nvod_wait_play()
{
	UINT32 time_tick=0;
	UINT32 idx = 0;
	UINT32 lldx = 0;
	BOOL ret_val= FALSE;

	do
	{
		/*this code is just to avoid infinite loop,*/
		osal_task_sleep(1000);
		time_tick++;
		if(time_tick>25)
			break;
	}while(!is_time_inited());

	libc_printf("time_tick =%d\n",time_tick);
    ret_val = WinNvodPlayChannel(NULL);
	return;
}



void set_nt_event_para(UINT16 id_val,UINT16 sid_val)
{
	nt_event_id = id_val;
	nt_event_sid = sid_val;
}

void reset_update_flag()
{
	send_enable= FALSE;
}

#endif

