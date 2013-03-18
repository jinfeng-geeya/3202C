#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <api/libtsi/sec_pmt.h>
#include <api/libpub/lib_pub.h>
#include <api/libpub/lib_hde.h>
#include <hld/dis/vpo.h>
#include <hld/nim/nim.h>
#include <hld/decv/vdec_driver.h>
#include <api/libdb/db_return_value.h>
#include <api/libdb/db_node_c.h>
#include <api/libdb/db_node_api.h>
#include <api/libdb/db_interface.h>

//#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"
#include "win_com.h"
#include "menus_root.h"
#include "win_com_popup.h"
#include "win_com_list.h"

#include "win_reserve_manage_obj.h"
//#include "win_book_detail.h"

#define DEB_PRINT	libc_printf

/*******************************************************************************
 *	Object Position
 *******************************************************************************/
#define MAIL_L     0
#define MAIL_T     90
#define MAIL_W     720
#define MAIL_H     400

#define LIST_CON_L     (66+TV_OFFSET_L)
#define LIST_CON_T     (102+TV_OFFSET_T)
#define LIST_CON_W     604
#define LIST_CON_H     350

#define LIST_L     LIST_CON_L
#define LIST_T     (LIST_CON_T+34)
#define LIST_W     LIST_CON_W
#define LIST_H     (LIST_CON_H-34)

#define ITEM_CON_L  (LIST_L+4)
#define ITEM_CON_T  LIST_T
#define ITEM_CON_W  580
#define ITEM_CON_H  30

#define ITEM_CON_GAP  0

#define SCB_L (ITEM_CON_L + ITEM_CON_W)
#define SCB_T ITEM_CON_T
#define SCB_W 12
#define SCB_H (ITEM_CON_H*10+ITEM_CON_GAP*9)

#define TXT1_W     60
#define TXT2_W     240
#define TXT3_W     100
#define TXT4_W     100
#define FLAG_W     50

#define COLORBTN_L      84
#define COLORBTN_T      460
#define COLORBTN_W      24
#define COLORBTN_GAP    150
#define COLORTXT_W      (24*2)

#define MAIL_IDX	  WSTL_COMMON_BACK_2
#define LIST_CON_IDX	WSTL_WIN_1
#define LIST_IDX		WSTL_NOSHOW_IDX
#define BMP_IDX 	WSTL_TXT_3

#define LIST_BAR_SH_IDX  WSTL_SCROLLBAR_1
#define LIST_BAR_HL_IDX  WSTL_SCROLLBAR_1

#define LIST_BAR_MID_RECT_IDX  	    WSTL_NOSHOW_IDX
#define LIST_BAR_MID_THUMB_IDX  	WSTL_SCROLLBAR_2

#define CON_SH_IDX   WSTL_BUTTON_2
#define CON_HL_IDX   WSTL_BUTTON_1 /**********/
#define CON_SL_IDX   WSTL_BUTTON_2
#define CON_GRY_IDX  WSTL_BUTTON_2

#define TXT_SH_IDX   WSTL_TXT_4
#define TXT_HL_IDX   WSTL_TXT_5
#define TXT_SL_IDX   WSTL_TXT_4
#define TXT_GRY_IDX  WSTL_TXT_4



static PRESULT reserve_con_proc(VACTION act);
static void reserve_con_str_init(UINT8 num);
//static void reserve_con_draw_txt();
static PRESULT reserve_list_proc(VACTION act);

/*******************************************************************************
 *	Local vriable & function declare
 *******************************************************************************/
#define MAX_RESERVE_NUM  10//MAX_TIMER_NUM
#define INDEX_SHIFT	16
#define INDEX_MARK	0x0000FFFF

#define VACT_DELETE 	(VACT_PASS + 1)

/*high 16 bit means timer valid or not;
 *low 16 bit means index of timers
 */
//static UINT32 del_flag[MAX_RESERVE_NUM] = {0};
 UINT32 del_flag[MAX_TIMER_NUM] = {0};

#define GET_TIMER_INDEX(p) (p & INDEX_MARK)
#define CHECK_TIMER_VALID(P) (1 == (P>>INDEX_SHIFT))

static void set_del_flag(UINT32 flag_index, UINT32 timer_index);
static void switch_del_flag(UINT32 index);
static BOOL check_del_flag();
static void save_setting();
static void switch_bimap(UINT32 index);
///extern CONTAINER g_win_book_detail;

static struct help_item_resource reserve_help[] =
{
    {1,IM_EPG_COLORBUTTON_RED,RS_HELP_DELETE},
    {1,IM_HELP_ICON_TB,RS_HELP_SELECT},
    {0,RS_MENU,RS_HELP_BACK},
    {0,RS_HELP_EXIT,RS_HELP_EXIT},
};

/*******************************************************************************
 *	Local vriable & function Definition
 *******************************************************************************/
static VACTION reserve_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

	switch (key)
	{
		case V_KEY_EXIT:
		
			save_setting();
			sys_data_save(1);
			MEMSET(del_flag, 0, sizeof(del_flag));
			win_clear_trash(display_strs[1], 40);
			BackToFullScrPlay();
			break;
		case V_KEY_MENU:
			if(window_pos_in_stack((POBJECT_HEAD)&g_win_sys_management) == -1)
			{
			save_setting();
			sys_data_save(1);
			MEMSET(del_flag, 0, sizeof(del_flag));
			win_clear_trash(display_strs[1], 40);
			BackToFullScrPlay();
			}
			act = VACT_CLOSE;
			break;
		case V_KEY_FIND:
			if(window_pos_in_stack((POBJECT_HEAD)&g_win_sys_management) == -1)
				{
					save_setting();
			sys_data_save(1);
			MEMSET(del_flag, 0, sizeof(del_flag));
			win_clear_trash(display_strs[1], 40);
			BackToFullScrPlay();


				}
			act = VACT_PASS;
			
		default:
			act = VACT_PASS;
	}
	return act;
}

static void ReserveManageListInit()
{
	UINT16 cur = 0, timer_cnt = 0, page;
	OBJLIST *ol;
	SYSTEM_DATA *sys_data;
	TIMER_SET_CONTENT *timer;
	UINT8 i = 0, insert_pos, timer_id;
	UINT32 index = 0;

	ol = &reserve_ol;

	sys_data = sys_data_get();

	page = OSD_GetObjListPage(ol);
	//OSD_SetObjListCurPoint(ol, cur);
	////cur = OSD_GetObjListCount(ol);

	for (i = 0; i < MAX_TIMER_NUM; i++)
	{
		timer = &sys_data->timer_set.TimerContent[i];
		if (timer->timer_mode != TIMER_MODE_OFF)
		{
                    insert_pos = index;
                    while(insert_pos != 0)
                    {
                        timer_id = del_flag[insert_pos-1]&0x0000FFFF;
                        if(timer->wakeup_year > sys_data->timer_set.TimerContent[timer_id].wakeup_year)
                        {
                            break;
                        }
						
                        else if(timer->wakeup_year == sys_data->timer_set.TimerContent[timer_id].wakeup_year)
                        {
                            if(timer->wakeup_month > sys_data->timer_set.TimerContent[timer_id].wakeup_month)
                            {
                                break;
                            }
                            else if(timer->wakeup_month == sys_data->timer_set.TimerContent[timer_id].wakeup_month)
                            {
                                if(timer->wakeup_day > sys_data->timer_set.TimerContent[timer_id].wakeup_day)
                                {
                                    break;
                                }
                                else if(timer->wakeup_day == sys_data->timer_set.TimerContent[timer_id].wakeup_day)
                                {
                                    if(timer->wakeup_time > sys_data->timer_set.TimerContent[timer_id].wakeup_time)
                                    {
                                        break;
                                    }
                                }
                            }
                        }
                        del_flag[insert_pos] = del_flag[insert_pos-1];
                        insert_pos--;
                    }
                    del_flag[insert_pos] = 0;
                    set_del_flag(insert_pos, i);
                    index++;
                    timer_cnt++;
		}
	}

	OSD_SetObjListCount(ol, timer_cnt);
	OSD_SetObjListCurPoint(ol, cur);
	///OSD_SetObjListNewPoint(ol, 3);
	OSD_SetObjListTop(ol, cur / page * page);
}

static PRESULT reserve_mg_message_proc(UINT32 msg_type, UINT32 msg_code)
{
	PRESULT ret = PROC_LOOP;

	switch (msg_type)
	{
		case CTRL_MSG_SUBTYPE_CMD_TIMEDISPLAYUPDATE:
			if (is_time_inited())
			{
				////wincom_draw_title_time();
			}
            break;
		default:
			break;
	}

	return ret;
}

static PRESULT reserve_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	VACTION unact;

	switch (event)
	{
		case EVN_PRE_OPEN:
			///wincom_open_title_ext(RS_RESERVE_MANAGEMENT, IM_TITLE_ICON_SYSTEM);
			///wincom_open_help(reserve_help, 4);
			reserve_con_str_init(40);          		
			//ReserveManageListInit();
			if(window_pos_in_stack((POBJECT_HEAD)&g_win_sys_management) != -1)
			{
			
					OSD_ClearObject((POBJECT_HEAD) &g_win_sys_management, C_UPDATE_ALL);

				}
			break;
		case EVN_PRE_DRAW:
			MEMSET(del_flag, 0, sizeof(del_flag));
		        ReserveManageListInit();
		        break;
		case EVN_POST_OPEN:
			
			//reserve_con_draw_txt();
			break;
		case EVN_PRE_CLOSE:
			*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
			/*
			save_setting();
			sys_data_save(1);
			MEMSET(del_flag, 0, sizeof(del_flag));
			win_clear_trash(display_strs[1], 40);
			*/
			break;
		case EVN_POST_CLOSE:
			
			break;
		case EVN_UNKNOWN_ACTION:
			unact = (VACTION)(param1 >> 16);
			ret = reserve_con_proc(unact);
			break;
		case EVN_MSG_GOT:
			ret = reserve_mg_message_proc( param1, param2 );
			break;
	}

	return ret;
}


static PRESULT reserve_list_proc(VACTION act)
{
	PRESULT ret = PROC_LOOP;
	OBJLIST *ol;
	UINT16 sel, ch_cnt;
	UINT32 index;
	TIMER_SET_CONTENT *timer;
	SYSTEM_DATA *sys_data;
	P_NODE p_node;



	ol = &reserve_ol;
	sel = OSD_GetObjListCurPoint(ol);
	ch_cnt = OSD_GetObjListCount(ol);
	index = del_flag[sel] &0x00010000;


	sys_data = sys_data_get();
	timer = &sys_data->timer_set.TimerContent[index];

	get_prog_by_id(timer->wakeup_channel, &p_node);
	switch (act)
	{
		case VACT_DELETE:
			switch_del_flag(sel);
			/////
			/*
			save_setting();
			sys_data_save(1);
			MEMSET(del_flag, 0, sizeof(del_flag));
			//win_clear_trash(display_strs[1], 40);
			///BackToFullScrPlay();
			*/
			////
			OSD_TrackObject((POBJECT_HEAD)ol, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
			break;
		default:
			break;
	}
	return ret;
}


static PRESULT reserve_con_proc(VACTION act)
{
	PRESULT ret = PROC_LOOP;

	switch (act)
	{
		//case VACT_TV_RADIO_SW:
		//	break;
		default:
			break;
	}
	return ret;
}


static VACTION reserve_list_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

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
		case V_KEY_RED:
			act = VACT_DELETE;
			break;
		case V_KEY_EXIT:
			act = VACT_PASS;
			break;
		default:
			break;
	}
	return act;
}
////

static VACTION reserve_item_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;
	UINT32 hKey;
	UINT16 cnt;
	switch (key)
	{
		case V_KEY_ENTER:
			//POBJECT_HEAD 
			///ol = &reserve_ol;
			///UINT16 cnt£»
			cnt = OSD_GetObjListCount(&reserve_ol);
			if(0==cnt)
				break;
			act = VACT_ENTER;
			break;
		default:
			act = VACT_PASS;
	}

	return act;

}

static PRESULT reserve_item_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	VACTION unact;

	switch (event)
	{
		
		case EVN_UNKNOWN_ACTION:
			//TODO: add code here
			unact = (VACTION)(param1 >> 16);
			if (unact == VACT_ENTER)
			{
					
				
					if (OSD_ObjOpen((POBJECT_HEAD)&g_win_book_detail, 0xFFFFFFFF) != PROC_LEAVE)
					{
	
						menu_stack_push((POBJECT_HEAD)&g_win_book_detail);
					}


			}
			ret = PROC_LOOP;
			break;
		default:
			break;
		
	}
	return ret;
}


///////
#if 0
static void ReserveManageSetDisplay()
{
	SYSTEM_DATA *sys_data;
	TIMER_SET_CONTENT *timer;
	INT32 i;
	CONTAINER *con;
	TEXT_FIELD *txt;
	BITMAP *bitmap;
	UINT16 strID;
	P_NODE p_node;
	UINT16 top, cnt, page, index, curitem, chan_idx;
	OBJLIST *ol;
	UINT32 valid_idx;
	UINT32 index_in_timer = 0;
	UINT16 prog_name[30];
	UINT8 str[30];
	UINT16 * pre_pstring = NULL;

	ol = &reserve_ol;

	cnt = OSD_GetObjListCount(ol);
	page = OSD_GetObjListPage(ol);
	top = OSD_GetObjListTop(ol);
	curitem = OSD_GetObjListNewPoint(ol);

    libc_printf("\n cnt= %d, ",cnt);
    libc_printf("\n page = %d ",page);
    libc_printf("\n top = %d ",top);
    libc_printf("\n curitem = %d \n",curitem);
  
	sys_data = sys_data_get();
	/*this can just show a page items   ...need modify leter if wan show more itmes*/
	for (i = 0; i < MAX_RESERVE_NUM; i++)
	{
		index_in_timer = GET_TIMER_INDEX(del_flag[i]);
		timer = &sys_data->timer_set.TimerContent[index_in_timer];
		index = top + i;

		valid_idx = (index < cnt) ? 1 : 0;
		con = (CONTAINER*)reserve_items[i];


		/* IDX */
		txt = (TEXT_FIELD*)OSD_GetContainerNextObj(con);
		if (valid_idx)
			sprintf(str, "%03d", index + 1);
		else
			STRCPY(str, "");
		OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)str);

		/*Date&Time*/
		txt = (TEXT_FIELD*)OSD_GetObjpNext(txt);
		if (valid_idx)
		{
			if (timer->timer_mode != TIMER_MODE_OFF)
    			sprintf ( str, "%02d/%02d/%04d %02d:%02d",
    			          timer->wakeup_month, timer->wakeup_day, timer->wakeup_year,
    			          timer->wakeup_time / 3600, timer->wakeup_time/60%60 );
		}
		else
		{
			STRCPY(str, "");
		}
		OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)str);





		get_prog_by_id(timer->wakeup_channel, &p_node);

		/*prog*/
		txt = (TEXT_FIELD*)OSD_GetObjpNext(txt);
		if (valid_idx)
		{
			ComUniStrCopy(prog_name, (UINT16*)p_node.service_name);
			OSD_SetTextFieldContent(txt, STRING_UNICODE, (UINT32)prog_name);
		}
		else
		{
			OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)"");
		}

		/*bref*/
		txt = (TEXT_FIELD*)OSD_GetObjpNext(txt);
		pre_pstring = txt->pString;
		txt->pString = NULL;
		if(valid_idx)
		{
			switch(timer->timer_mode)
			{
				case TIMER_MODE_ONCE:
					OSD_SetTextFieldContent(txt, STRING_ID, RS_SYSTEM_TIMERSET_ONCE);
					break;
				case TIMER_MODE_DAILY: 
					OSD_SetTextFieldContent(txt, STRING_ID, RS_SYSTEM_TIMERSET_DAILY);
					break;      
			    case TIMER_MODE_WEEKLY:
					OSD_SetTextFieldContent(txt, STRING_ID, RS_SYSTEM_TIMERSET_WEEKLY);
					break;        
			    case TIMER_MODE_MONTHLY:
					OSD_SetTextFieldContent(txt, STRING_ID, RS_SYSTEM_TIMERSET_MONTHLY);
					break;        
			    case TIMER_MODE_YEARLY:
					OSD_SetTextFieldContent(txt, STRING_ID, RS_SYSTEM_TIMERSET_YEARLY);
					break;
				default:
					break;		
			}
			
		}
		else
		{
			txt->wStringID = 0;
			txt->pString = pre_pstring;
			OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)"");
		}

		/* state */
		bitmap = (BITMAP*)OSD_GetObjpNext(txt);
		if (valid_idx)
			bitmap->wIconID = (del_flag[i] >> INDEX_SHIFT) ? IM_ICON_CLOCK_01 : 0;
		else
			bitmap->wIconID = 0;
	}
}
#endif
static void ReserveManageSetDisplay()
{
	SYSTEM_DATA *sys_data;
	TIMER_SET_CONTENT *timer;
	INT32 i;
	CONTAINER *con;
	TEXT_FIELD *txt;
	BITMAP *bitmap;
	UINT16 strID;
	P_NODE p_node;
	UINT16 top, cnt, page, index, curitem, chan_idx;
	OBJLIST *ol;
	UINT32 valid_idx;
	UINT32 index_in_timer = 0;
	UINT16 prog_name[30];
	UINT8 str[30];
	UINT16 * pre_pstring = NULL;
	UINT32 cur_channel;

	ol = &reserve_ol;

	cnt = OSD_GetObjListCount(ol);
	page = OSD_GetObjListPage(ol);
	top = OSD_GetObjListTop(ol);
	curitem = OSD_GetObjListNewPoint(ol);

	sys_data = sys_data_get();	
	for (i = 0; i < MAX_RESERVE_NUM; i++)
	{
		index = top + i;	
		index_in_timer = GET_TIMER_INDEX(del_flag[index]);
		timer = &sys_data->timer_set.TimerContent[index_in_timer];

		valid_idx = (index < cnt) ? 1 : 0;
		//con = (CONTAINER*)reserve_items[i];
		con = (CONTAINER*)reserve_ol_ListField[i];
		//IDX 
		txt = (TEXT_FIELD*)OSD_GetContainerNextObj(con);
		if (valid_idx)
			{
			cur_channel = get_prog_pos(timer->wakeup_channel);
			sprintf(str, "%03d",cur_channel+1);
			//sprintf(str, "%03d", index + 1);

			}
		else
			STRCPY(str, "");
		OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)str);
/*
		//Date&Time
		txt = (TEXT_FIELD*)OSD_GetObjpNext(txt);
		if (valid_idx)
		{
			if (timer->timer_mode != TIMER_MODE_OFF)
    			sprintf ( str, "%02d/%02d/%04d %02d:%02d",
    			          timer->wakeup_month, timer->wakeup_day, timer->wakeup_year,
    			          timer->wakeup_time / 3600, timer->wakeup_time/60%60 );
		}
		else
		{
			STRCPY(str, "");
		}
		OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)str);

*/
		get_prog_by_id(timer->wakeup_channel, &p_node);
		//prog
		txt = (TEXT_FIELD*)OSD_GetObjpNext(txt);
		if (valid_idx)
		{
			ComUniStrCopy(prog_name, /*(UINT16*)p_node.service_name*/(UINT16*)timer->event_name);
			OSD_SetTextFieldContent(txt, STRING_UNICODE, (UINT32)prog_name);
		}
		else
		{
			OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)"");
		}

/////
//Date&Time
		txt = (TEXT_FIELD*)OSD_GetObjpNext(txt);
		if (valid_idx)
		{
			if (timer->timer_mode != TIMER_MODE_OFF)
    			sprintf ( str, "%02d/%02d/%04d %02d:%02d",
    			          timer->wakeup_month, timer->wakeup_day, timer->wakeup_year,
    			          timer->wakeup_time / 3600, timer->wakeup_time/60%60 );
		}
		else
		{
			STRCPY(str, "");
		}
		OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)str);


		get_prog_by_id(timer->wakeup_channel, &p_node);



/////
		
		/*
		//bref
		txt = (TEXT_FIELD*)OSD_GetObjpNext(txt);
		pre_pstring = txt->pString;
		txt->pString = NULL;
		if(valid_idx)
		{
			switch(timer->timer_mode)
			{
				case TIMER_MODE_ONCE:
					OSD_SetTextFieldContent(txt, STRING_ID, RS_SYSTEM_TIMERSET_ONCE);
					break;
				case TIMER_MODE_DAILY: 
					OSD_SetTextFieldContent(txt, STRING_ID, RS_SYSTEM_TIMERSET_DAILY);
					break;      
			    case TIMER_MODE_WEEKLY:
					OSD_SetTextFieldContent(txt, STRING_ID, RS_SYSTEM_TIMERSET_WEEKLY);
					break;        
			    case TIMER_MODE_MONTHLY:
					OSD_SetTextFieldContent(txt, STRING_ID, RS_SYSTEM_TIMERSET_MONTHLY);
					break;        
			    case TIMER_MODE_YEARLY:
					OSD_SetTextFieldContent(txt, STRING_ID, RS_SYSTEM_TIMERSET_YEARLY);
					break;
				default:
					break;		
			}
			
		}
		else
		{
			txt->wStringID = 0;
			txt->pString = pre_pstring;
			OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)"");
		}
*/
		// state
		bitmap = (BITMAP*)OSD_GetObjpNext(txt);
		if (valid_idx)
			bitmap->wIconID = (del_flag[index] >> INDEX_SHIFT) ? IM_ICON_CLOCK_01 : 0;
		else
			bitmap->wIconID = 0;
	}

}

static PRESULT reserve_list_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	VACTION unact;
	UINT16 sel;
	OBJLIST *ol;

	ol = (OBJLIST*)pObj;



	switch (event)
	{
		case EVN_PRE_DRAW:
			///ret = PROC_LOOP;
			if (param1 == C_DRAW_TYPE_HIGHLIGHT)
			{
				ReserveManageSetDisplay();
				////OSD_TrackObject(pObj, C_UPDATE_ALL);
			}
			break;
		case EVN_POST_DRAW:
			break;
		case EVN_ITEM_PRE_CHANGE:
			break;
		case EVN_ITEM_POST_CHANGE:
			break;
		case EVN_PRE_CHANGE:
			break;
		case EVN_POST_CHANGE:
			break;
		case EVN_UNKNOWN_ACTION:
			unact = (VACTION)(param1 >> 16);
			ret = reserve_list_proc(unact);
			break;
		case EVN_UNKNOWNKEY_GOT:
			break;
		default:
			;
	}


	return ret;
}

static void reserve_con_str_init(UINT8 num)
{
	UINT8 i = 0;

	for (i = 0; i < num; i++)
	{
		MEMSET(display_strs[i+1], 0, sizeof(display_strs[i+1]));
	}
}

#define LIST_CON_TOP_OFFSET 4

static OSD_RECT rc_txt[] =
{
    {ITEM_CON_L,LIST_CON_T+LIST_CON_TOP_OFFSET,TXT1_W,24},
    {ITEM_CON_L+TXT1_W,LIST_CON_T+LIST_CON_TOP_OFFSET,TXT2_W,24},
    {ITEM_CON_L+TXT1_W+TXT2_W,LIST_CON_T+LIST_CON_TOP_OFFSET,TXT3_W,24},
    {ITEM_CON_L+TXT1_W+TXT2_W+TXT3_W,LIST_CON_T+LIST_CON_TOP_OFFSET,TXT4_W,24},
    {ITEM_CON_L+TXT1_W+TXT2_W+TXT3_W+TXT4_W,LIST_CON_T+LIST_CON_TOP_OFFSET,FLAG_W,24},
};

/*#define TXT_NUM (sizeof(rc_txt)/sizeof(OSD_RECT))

static UINT16 id_txt[] =
{
    RS_SEARIAL_NUM,
    RS_SYSTEM_TIME,
    RS_SYSTEM_TIMERSET_CHANNEL,
    RS_SYSTEM_TIMERSET_TIMER_MODE,
    RS_STATUS,
};

static void reserve_con_draw_txt()
{
	UINT8 i = 0;
	TEXT_FIELD *txt;

	txt = &reserve_txt;

	for (i = 0; i < TXT_NUM; i++)
	{
		OSD_SetRect2(&txt->head.frame, &rc_txt[i]);
		OSD_SetTextFieldContent(txt, STRING_ID, id_txt[i]);
		OSD_DrawObject((POBJECT_HEAD)txt, C_UPDATE_ALL);
	}
}*/



static void set_del_flag(UINT32 flag_index, UINT32 timer_index)
{
	del_flag[flag_index] |= (0x00000001 << INDEX_SHIFT);
	del_flag[flag_index] |= timer_index;
}


static void switch_del_flag(UINT32 index)
{
	UINT32 temp = del_flag[index] &INDEX_MARK;
	/*hold the index for later use*/
	del_flag[index] ^= (0x00000001 << INDEX_SHIFT);
	del_flag[index] |= temp;
}

static BOOL check_del_flag()
{
	UINT32 count;
	OBJLIST *ol;
	ol = &reserve_ol;
	count = OSD_GetObjListCount(ol);

	UINT32 i = 0;
	for (; i < count; i++)
	{
		if (!CHECK_TIMER_VALID(del_flag[i]))
			return TRUE;
	}
	return FALSE;
}




static void save_setting()
{
	UINT8 back_saved;
	UINT32 index_of_timer = 0;
	UINT32 index = 0;
	UINT32 count = 0;
	INT32 ret = 0;
	OBJLIST *ol;
	TIMER_SET_CONTENT *timer;
	win_popup_choice_t choice;
	SYSTEM_DATA *sys_data = NULL;

	ol = &reserve_ol;
	count = OSD_GetObjListCount(ol);

	sys_data = sys_data_get();

	if (check_del_flag())
	{
		win_compopup_init(WIN_POPUP_TYPE_OKNO);
		win_compopup_set_msg(NULL, NULL, RS_MSG_ARE_YOU_SURE_TO_SAVE);
		choice = win_compopup_open_ext(&back_saved);
		if (choice == WIN_POP_CHOICE_YES)
		{
			for (; index < count; index++)
			{
				if (!(del_flag[index] >> INDEX_SHIFT))
				{
					index_of_timer = GET_TIMER_INDEX(del_flag[index]);
					timer = &sys_data->timer_set.TimerContent[index_of_timer];
					timer->timer_mode = TIMER_MODE_OFF;
				}
			}

		}
	}
}

