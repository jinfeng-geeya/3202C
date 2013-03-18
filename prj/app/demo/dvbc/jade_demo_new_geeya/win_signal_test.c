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

#define DEB_PRINT	libc_printf

#include "win_signal_test_obj.h"

#define WIN_L     0
#define WIN_T     90
#define WIN_W     720
#define WIN_H     400

#define LIST_CON_L     (66+TV_OFFSET_L)
#define LIST_CON_T     (102+TV_OFFSET_T)
#define LIST_CON_W     604
#define LIST_CON_H     200

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
#define SCB_H (ITEM_CON_H*5+ITEM_CON_GAP*4)

#define TXT1_W     80
#define TXT2_W     140
#define TXT3_W     140
#define TXT4_W     (ITEM_CON_W-TXT1_W-TXT2_W-TXT3_W)

#define WIN_IDX	  WSTL_COMMON_BACK_2
#define LIST_CON_IDX	WSTL_WIN_1
#define LIST_IDX		WSTL_NOSHOW_IDX
#define BMP_IDX 	WSTL_TXT_3

#define LIST_BAR_SH_IDX  WSTL_SCROLLBAR_1
#define LIST_BAR_HL_IDX  WSTL_SCROLLBAR_1

#define LIST_BAR_MID_RECT_IDX  	    WSTL_NOSHOW_IDX
#define LIST_BAR_MID_THUMB_IDX  	WSTL_SCROLLBAR_2

#define CON_SH_IDX   WSTL_BUTTON_2
#define CON_HL_IDX   WSTL_BUTTON_1
#define CON_SL_IDX   WSTL_BUTTON_2
#define CON_GRY_IDX  WSTL_BUTTON_2

#define TXT_SH_IDX   WSTL_TXT_4
#define TXT_HL_IDX   WSTL_TXT_5
#define TXT_SL_IDX   WSTL_TXT_4
#define TXT_GRY_IDX  WSTL_TXT_4

static PRESULT SignalTestUnkownActProc(VACTION act);
static void SignalTestStrInit(UINT8 num);
static void SignalTestDrawTxt();
static void RecordPreNimPar();
static void RecoverPreNimPar();

#define VACT_DELETE 	(VACT_PASS + 1)

/*******************************************************************************
 *	Local vriable & function declare
 *******************************************************************************/
static VACTION signal_test_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

	switch (key)
	{
		case V_KEY_EXIT:
			RecoverPreNimPar();
			restore_scene();
			BackToFullScrPlay();
			break;
		case V_KEY_MENU:
			act = VACT_CLOSE;
			break;
		default:
			act = VACT_PASS;
	}
	return act;
}

struct help_item_resource signal_test_help[] =
{
    {1,IM_HELP_ICON_TB,RS_HELP_SELECT},
    {0,RS_MENU,RS_HELP_BACK},
    {0,RS_HELP_EXIT,RS_HELP_EXIT},
};

static void SignalTestListInit()
{
	UINT16 sel, cnt = 0, page;
	OBJLIST *ol;
	T_NODE t_node;
	struct nim_device *nim_dev = (struct nim_device*)dev_get_by_id(HLD_DEV_TYPE_NIM, 0);

	ol = &signal_test_ol;


	recreate_tp_view(VIEW_SINGLE_SAT, 1); //There is only one sat for dvbc and its id is 1.
	cnt = get_tp_num_sat(1);

	OSD_SetObjListCount(ol, cnt);

	if (cnt <= OSD_GetObjListCurPoint(ol))
	{
		
		OSD_SetObjListCurPoint (ol, 0);
		OSD_SetObjListNewPoint (ol, 0);
		OSD_SetObjListTop (ol, 0);
	}
	sel = OSD_GetObjListCurPoint(ol);
	get_tp_by_pos(sel, &t_node);
	nim_channel_change(nim_dev, t_node.frq, t_node.sym, t_node.FEC_inner);
}

static PRESULT signal_test_message_proc(UINT32 msg_type, UINT32 msg_code)
{
	PRESULT ret = PROC_LOOP;
	switch (msg_type)
	{
		case CTRL_MSG_SUBTYPE_STATUS_SIGNAL:
			win_signal_update();
            break;
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

static PRESULT signal_test_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	VACTION unact;

	switch (event)
	{
		case EVN_PRE_OPEN:
			record_scene();
			wincom_open_title_ext(RS_SIGNAL_CHECK, IM_TITLE_ICON_SYSTEM);
			wincom_open_help(signal_test_help, 3);
			SignalTestStrInit(40);
			SignalTestListInit();
			dm_set_onoff(0);
			break;
		case EVN_POST_OPEN:
			win_signal_open(pObj);
			SignalTestDrawTxt();
			RecordPreNimPar();
			win_signal_update();
			break;
		case EVN_PRE_CLOSE:
			*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
			RecoverPreNimPar();
			break;
		case EVN_POST_CLOSE:
			dm_set_onoff(1);
			restore_scene();
			win_signal_close();
			break;
		case EVN_UNKNOWN_ACTION:
			unact = (VACTION)(param1 >> 16);
			ret = SignalTestUnkownActProc(unact);
			break;
		case EVN_MSG_GOT:
			ret = signal_test_message_proc( param1, param2 );
			break;
	}

	return ret;
}


static PRESULT SignalTestListUnkownActProc(VACTION act)
{
	PRESULT ret = PROC_LOOP;
	OBJLIST *ol;
	UINT16 sel, ch_cnt;

	ol = &signal_test_ol;
	sel = OSD_GetObjListCurPoint(ol);
	ch_cnt = OSD_GetObjListCount(ol);

	switch (act)
	{
		case VACT_DELETE:
			break;
		default:
			break;
	}
	return ret;
}


static PRESULT SignalTestUnkownActProc(VACTION act)
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


static VACTION signal_test_list_keymap(POBJECT_HEAD pObj, UINT32 key)
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

static void SingalTestSetDisplay()
{
	SYSTEM_DATA *sys_data;
	INT32 i;
	CONTAINER *con;
	TEXT_FIELD *txt;
	UINT16 strID;
	P_NODE p_node;
	T_NODE t_node;
	UINT16 top, cnt, page, index, curitem, chan_idx;
	OBJLIST *ol;
	UINT32 valid_idx;
	UINT16 prog_name[30];
	UINT8 str[30];
    UINT16 qam[5] = {16,32,64,128,256};

	ol = &signal_test_ol;

	cnt = OSD_GetObjListCount(ol);
	page = OSD_GetObjListPage(ol);
	top = OSD_GetObjListTop(ol);
	curitem = OSD_GetObjListNewPoint(ol);

	sys_data = sys_data_get();
	for (i = 0; i < page; i++)
	{
		//con = (CONTAINER*)signal_test_items[i];
		con = (CONTAINER*)signal_test_ol_ListField[i];
		index = top + i;

		valid_idx = (index < cnt) ? 1 : 0;

		if (valid_idx)
		{
			get_tp_by_pos(index, &t_node);
		}
		else
		{
			STRCPY(str, "");
		}
		

		/* IDX */
		txt = (TEXT_FIELD*)OSD_GetContainerNextObj(con);
		if (valid_idx)
			sprintf(str, "%02d", index + 1);
		else
			STRCPY(str, "");
		OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)str);

		/*Frequency*/
		txt = (TEXT_FIELD*)OSD_GetObjpNext(txt);
		if (valid_idx)
		{
			sprintf(str, "%dMHz", t_node.frq/100);
		}
		else
		{
			STRCPY(str, "");
		}
		OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)str);

		/*Symbol*/
		txt = (TEXT_FIELD*)OSD_GetObjpNext(txt);
		if (valid_idx)
		{
			sprintf(str, "%dMBaud", t_node.sym);
		}
		else
		{
			STRCPY(str, "");
		}
		OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)str);

		/*Constellation*/
		txt = (TEXT_FIELD*)OSD_GetObjpNext(txt);
		if (valid_idx)
		{
			if (t_node.FEC_inner < QAM16 || t_node.FEC_inner > QAM256)
				t_node.FEC_inner = QAM16;

			sprintf(str, "%dQAM", qam[t_node.FEC_inner - QAM16]);
		}
		else
		{
			STRCPY(str, "");
		}
		OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)str);
	}

}

static PRESULT signal_test_list_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	VACTION unact;
	UINT16 sel;
	OBJLIST *ol;
	T_NODE t_node;
	struct nim_device *nim_dev = (struct nim_device*)dev_get_by_id(HLD_DEV_TYPE_NIM, 0);


	ol = (OBJLIST*)pObj;

	switch (event)
	{
		case EVN_PRE_DRAW:
			ret = PROC_LOOP;
			if (param1 == C_DRAW_TYPE_HIGHLIGHT)
			{
				SingalTestSetDisplay();
				OSD_TrackObject(pObj, C_UPDATE_ALL);
			}
			break;
		case EVN_POST_DRAW:
			break;
		case EVN_ITEM_PRE_CHANGE:
			break;
		case EVN_ITEM_POST_CHANGE:
			sel = OSD_GetObjListCurPoint(ol);
			get_tp_by_pos(sel, &t_node);
			nim_channel_change(nim_dev, t_node.frq, t_node.sym, t_node.FEC_inner);
			break;
		case EVN_PRE_CHANGE:
			break;
		case EVN_POST_CHANGE:
			break;
		case EVN_UNKNOWN_ACTION:
			unact = (VACTION)(param1 >> 16);
			ret = SignalTestListUnkownActProc(unact);
			break;
		case EVN_UNKNOWNKEY_GOT:
			break;
		default:
			;
	}


	return ret;
}

static UINT32 pre_frq = 0, pre_sym = 0;
static UINT8 pre_qam = 0;

static void RecordPreNimPar()
{
	struct nim_device *nim_dev = (struct nim_device*)dev_get_by_id(HLD_DEV_TYPE_NIM, 0);

	nim_get_freq(nim_dev, &pre_frq);
	nim_get_sym(nim_dev, &pre_sym);
	nim_get_FEC(nim_dev, &pre_qam);
}

static void RecoverPreNimPar()
{
	struct nim_device *nim_dev = (struct nim_device*)dev_get_by_id(HLD_DEV_TYPE_NIM, 0);

	nim_channel_change(nim_dev, pre_frq, pre_sym, pre_qam);
}

static void SignalTestStrInit(UINT8 num)
{
	UINT8 i = 0;

	for (i = 0; i < num; i++)
	{
		MEMSET(display_strs[i], 0, sizeof(display_strs[i]));
	}
}

#define LIST_CON_TOP_OFFSET 4

static OSD_RECT rc_txt[] =
{
    {ITEM_CON_L,LIST_CON_T+LIST_CON_TOP_OFFSET,TXT1_W,24},
    {ITEM_CON_L+TXT1_W,LIST_CON_T+LIST_CON_TOP_OFFSET,TXT2_W,24},
    {ITEM_CON_L+TXT1_W+TXT2_W,LIST_CON_T+LIST_CON_TOP_OFFSET,TXT3_W,24},
    {ITEM_CON_L+TXT1_W+TXT2_W+TXT3_W,LIST_CON_T+LIST_CON_TOP_OFFSET,TXT4_W,24},
};

#define TXT_NUM (sizeof(rc_txt)/sizeof(OSD_RECT))

static UINT16 id_txt[] =
{
    RS_SEARIAL_NUM,
    RS_FREQUENCY,
    RS_INFO_SYMBOL,
    RS_CONSTELLATION,
};

static void SignalTestDrawTxt()
{
	UINT8 i = 0;
	TEXT_FIELD *txt;

	txt = &signal_test_txt;

	for (i = 0; i < TXT_NUM; i++)
	{
		OSD_SetRect2(&txt->head.frame, &rc_txt[i]);
		OSD_SetTextFieldContent(txt, STRING_ID, id_txt[i]);
		OSD_DrawObject((POBJECT_HEAD)txt, C_UPDATE_ALL);
	}
}
