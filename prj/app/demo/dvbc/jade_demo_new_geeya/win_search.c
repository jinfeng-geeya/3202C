#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <api/libpub/lib_pub.h>
#include <api/libpub/lib_as.h>
#include <hld/nim/nim.h>

#include <hld/decv/decv.h>

//#include <api/libosd/osd_lib.h>
#include <api/libtsi/psi_db.h>
#include <api/libsi/si_service_type.h>

#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"
#include "win_com.h"
#include "menus_root.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_signalstatus.h"

//include the header from xform 
#include "win_search_vega.h"

#define DBG_PRINTF PRINTF//soc_printf
/*******************************************************************************
 *	Objects definition
 *******************************************************************************/
extern CONTAINER g_win_search;
extern CONTAINER search_con;
extern CONTAINER srch_con_condition;
extern CONTAINER ConBarrier1;
extern CONTAINER ConBarrier2;
extern CONTAINER ConBarrier3;


extern TEXT_FIELD srch_tv_back;
extern TEXT_FIELD srch_radio_back;
//extern TEXT_FIELD srch_txt_freq;
//extern TEXT_FIELD srch_txt_symbol;
//extern TEXT_FIELD srch_txt_qam;

extern TEXT_FIELD srch_txt_freq_val;
extern TEXT_FIELD srch_txt_symbol_val;
extern TEXT_FIELD srch_txt_qam_val;


extern TEXT_FIELD srch_info;
//extern PROGRESS_BAR srch_progress_bar;
extern TEXT_FIELD srch_progress_txt;

extern void sdt_monitor_on(UINT32 index);

struct help_item_resource win_search_help[] =
{
    {0,RS_HELP_EXIT,RS_HELP_EXIT},
};

typedef enum
{
    PROG_IDX,
    TP_FREQ,
    TP_IDX
}APPEND_PARAM_TYPE;

/*static PRESULT srch_backtxt_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);
static VACTION srch_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT srch_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);


#define W_BG_IDX      WSTL_SEARCH_BG_CON

#define WIN_CON_IDX	  WSTL_WIN_2
#define COND_TXT_IDX  WSTL_TXT_4


#define TVBACK_SH_IDX		WSTL_TXT_7
#define RADIOBACK_SH_IDX	WSTL_TXT_7
#define TPBACK_SH_IDX		WSTL_TXT_4
#define COND_SH_IDX		    WSTL_TXT_4*/


#define TV_TXT_SH_IDX		WSTL_TXT_BULE_BG
#define RADIO_TXT_SH_IDX	WSTL_TXT_BULE_BG
/*#define PROGRESS_TXT_SH_IDX	WSTL_TXT_8

#define PROGRESSBAR_SH_IDX			WSTL_BAR_HEAD_BG
#define PROGRESSBAR_MID_SH_IDX		WSTL_NOSHOW_IDX*/
#define PROGRESSBAR_PROGRESS_SH_IDX	WSTL_BAR_TICK_FORCE



/*#define W_BG_L     	0
#define W_BG_T     	90
#define W_BG_W	    720
#define W_BG_H     	400*/

#define W_CON_L     	(TV_OFFSET_L+66)
#define W_CON_T     	(TV_OFFSET_T+102)
//#define W_CON_W			604
//#define W_CON_H     	286

/*#define COND_TXT_L     	W_CON_L
#define COND_TXT_T     	(W_CON_T+W_CON_H+18)
#define COND_TXT_W		W_CON_W
#define COND_TXT_H     	24

#define COND_FREQ_TXT_L     	(COND_TXT_L+2)
#define COND_FREQ_TXT_T     	COND_TXT_T
#define COND_FREQ_TXT_W		    70
#define COND_FREQ_TXT_H     	COND_TXT_H

#define COND_FREQ_TXT_VAL_L     	(COND_FREQ_TXT_L+COND_FREQ_TXT_W)
#define COND_FREQ_TXT_VAL_T     	COND_TXT_T
#define COND_FREQ_TXT_VAL_W		    110
#define COND_FREQ_TXT_VAL_H     	COND_TXT_H

#define COND_FREQ_TXT_UINT_L     	(COND_FREQ_TXT_VAL_L+COND_FREQ_TXT_VAL_W)
#define COND_FREQ_TXT_UINT_T     	COND_TXT_T
#define COND_FREQ_TXT_UINT_W		5
#define COND_FREQ_TXT_UINT_H     	COND_TXT_H

#define COND_SYMBOL_TXT_L  (COND_FREQ_TXT_UINT_L+COND_FREQ_TXT_UINT_W+2)
#define COND_SYMBOL_TXT_T  COND_TXT_T
#define COND_SYMBOL_TXT_W  90
#define COND_SYMBOL_TXT_H  COND_TXT_H

#define COND_SYMBOL_VAL_L  (COND_SYMBOL_TXT_L+COND_SYMBOL_TXT_W)
#define COND_SYMBOL_VAL_T  COND_TXT_T
#define COND_SYMBOL_VAL_W  110
#define COND_SYMBOL_VAL_H  COND_TXT_H

#define COND_SYMBOL_UINT_L   (COND_SYMBOL_VAL_L+COND_SYMBOL_VAL_W)
#define COND_SYMBOL_UINT_T  COND_TXT_T
#define COND_SYMBOL_UINT_W	2
#define COND_SYMBOL_UINT_H  COND_TXT_H

#define COND_QAM_TXT_L     (COND_SYMBOL_UINT_L+COND_SYMBOL_UINT_W)
#define COND_QAM_TXT_T     COND_TXT_T
#define COND_QAM_TXT_W	   110
#define COND_QAM_TXT_H     COND_TXT_H

#define COND_QAM_VAL_L     (COND_QAM_TXT_L+COND_QAM_TXT_W)
#define COND_QAM_VAL_T     COND_TXT_T
#define COND_QAM_VAL_W	   90
#define COND_QAM_VAL_H     COND_TXT_H

#define COND_QAM_UINT_L    (COND_QAM_VAL_L+COND_QAM_VAL_W)
#define COND_QAM_UINT_T    COND_TXT_T
#define COND_QAM_UINT_W	   2
#define COND_QAM_UINT_H    COND_TXT_H*/


#define TVB_L		(W_CON_L+50)
#define TVB_T		(W_CON_T+4)
#define TVB_W       200
#define TVB_H		24

#define RADIOB_L	(TVB_L + 332)
/*#define RADIOB_T	TVB_T
#define RADIOB_W    TVB_W
#define RADIOB_H	TVB_H

#define BAR_L	(W_CON_L)
#define BAR_T	(W_CON_T + W_CON_H + 66)
#define BAR_W	(W_CON_W - BAR_TXT_W - 10)
#define BAR_H	16

#define BAR_TXT_L (BAR_L + BAR_W + 6)
#define BAR_TXT_W	60
#define BAR_TXT_T ( BAR_T - 2)
#define BAR_TXT_H	24*/

#define INFO_TXT_H	24

#define INFO_TV_T	(TVB_T + 62)
#define INFO_TV_L	TVB_L
#define INFO_TV_W	TVB_W

#define INFO_RADIO_T	INFO_TV_T
#define INFO_RADIO_L	RADIOB_L
#define INFO_RADIO_W	TVB_W


/*#define TXT_CONDITON_L  W_CON_L
#define TXT_CONDITON_T  (W_CON_T+W_CON_H+18)
#define TXT_CONDITON_W  W_CON_W
#define TXT_CONDITON_H  24*/


/*******************************************************************************
barrier related macro & postion prarmeter define
 *******************************************************************************/
/*#define BARRIER_IDX  WSTL_COMMON_BACK_1

#define CON_BARRIER1_L  (W_CON_L+8)
#define CON_BARRIER1_T  (W_CON_T+38)
#define CON_BARRIER1_W  290
#define CON_BARRIER1_H  2

#define CON_BARRIER2_L  (W_CON_L+306)
#define CON_BARRIER2_T  CON_BARRIER1_T
#define CON_BARRIER2_W  CON_BARRIER1_W
#define CON_BARRIER2_H  CON_BARRIER1_H

#define CON_BARRIER3_L  (W_CON_L+300)
#define CON_BARRIER3_T  (W_CON_T+6)
#define CON_BARRIER3_W  2
#define CON_BARRIER3_H  274*/

#define INFOR_CHAN_CNT		7



/******************************************************************************/
/*#define LDEF_COND_BACK(root,varTxt,nxtObj,id,sh,l,t,w,h,resID,pstring)		\
    DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
    id,id,id,id,id, l,t,w,h, sh,sh,sh,sh,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,resID,pstring)

#define LDEF_BACK(root,varTxt,nxtObj,id,sh,l,t,w,h,resID)		\
    DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
    id,id,id,id,id, l,t,w,h, sh,sh,sh,sh,   \
    NULL,srch_backtxt_callback,  \
    C_ALIGN_LEFT | C_ALIGN_TOP, 30,0,resID,NULL)

#define LDEF_INFOR_TXT(root,varTxt)		\
    DEF_TEXTFIELD(varTxt,root,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0,  0,0,0,0, 0,0,0,0,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0,n[0])


#define LDEF_PROGRESS_BAR(root,varBar,nxtObj,l,t,w,h,style,rl,rt,rw,rh)	\
	DEF_PROGRESSBAR(varBar, root, nxtObj, C_ATTR_ACTIVE, 0, \
		0, 0, 0, 0, 0, l, t, w, h, PROGRESSBAR_SH_IDX, PROGRESSBAR_SH_IDX, PROGRESSBAR_SH_IDX, PROGRESSBAR_SH_IDX,\
		NULL, NULL, style, 0, 0, PROGRESSBAR_MID_SH_IDX, PROGRESSBAR_PROGRESS_SH_IDX, \
		rl,rt , rw, rh, 1, 100, 100, 1)

#define LDEF_PROGRESS_TXT(root,varTxt,nxtObj,l,t,w,h)		\
    DEF_TEXTFIELD(varTxt,root,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, PROGRESS_TXT_SH_IDX,PROGRESS_TXT_SH_IDX,PROGRESS_TXT_SH_IDX,PROGRESS_TXT_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0,display_strs[1])


#define LDEF_CON(varCon,nxtObj,l,t,w,h,focusID)		\
    DEF_CONTAINER(varCon,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_CON_IDX,WIN_CON_IDX,WIN_CON_IDX,WIN_CON_IDX,   \
    NULL,NULL,  \
    nxtObj, focusID,0)


DEF_CONTAINER ( g_win_search, NULL, NULL, C_ATTR_ACTIVE, 0, \
                1, 0, 0, 0, 0, W_BG_L, W_BG_T, W_BG_W, W_BG_H, W_BG_IDX, W_BG_IDX, W_BG_IDX, W_BG_IDX,   \
                srch_keymap, srch_callback,  \
                ( POBJECT_HEAD ) &search_con, 1, 0 )

LDEF_CON(search_con, &srch_tv_back, W_CON_L, W_CON_T, W_CON_W, W_CON_H, 1)


LDEF_BACK(&search_con, srch_tv_back, &srch_radio_back,	1 , TVBACK_SH_IDX, \
          TVB_L, TVB_T, TVB_W, TVB_H, RS_INFO_TV)

LDEF_BACK(&search_con, srch_radio_back, &ConBarrier1,	2, RADIOBACK_SH_IDX, \
          RADIOB_L, RADIOB_T, RADIOB_W, RADIOB_H, RS_INFO_RADIO)


DEF_CONTAINER ( ConBarrier1, NULL, NULL, C_ATTR_ACTIVE, 0, \
                2, 0, 0, 0, 0, CON_BARRIER1_L, CON_BARRIER1_T, CON_BARRIER1_W, CON_BARRIER1_H, BARRIER_IDX, BARRIER_IDX, BARRIER_IDX, BARRIER_IDX,   \
                NULL, NULL,  \
                ( POBJECT_HEAD ) &ConBarrier2, 1, 0 )

DEF_CONTAINER ( ConBarrier2, NULL, NULL, C_ATTR_ACTIVE, 0, \
                3, 0, 0, 0, 0, CON_BARRIER2_L, CON_BARRIER2_T, CON_BARRIER2_W, CON_BARRIER2_H, BARRIER_IDX, BARRIER_IDX, BARRIER_IDX, BARRIER_IDX,   \
                NULL, NULL,  \
                ( POBJECT_HEAD ) &ConBarrier3, 1, 0 )

DEF_CONTAINER ( ConBarrier3, NULL, NULL, C_ATTR_ACTIVE, 0, \
                4, 0, 0, 0, 0, CON_BARRIER3_L, CON_BARRIER3_T, CON_BARRIER3_W, CON_BARRIER3_H, BARRIER_IDX, BARRIER_IDX, BARRIER_IDX, BARRIER_IDX,   \
                NULL, NULL,  \
                (POBJECT_HEAD)&srch_con_condition, 1, 0 )


DEF_CONTAINER ( srch_con_condition, NULL, NULL, C_ATTR_ACTIVE, 0, \
                1, 0, 0, 0, 0, COND_TXT_L, COND_TXT_T, COND_TXT_W, COND_TXT_H, COND_TXT_IDX, COND_TXT_IDX, COND_TXT_IDX, COND_TXT_IDX,   \
                NULL, NULL,  \
                ( POBJECT_HEAD ) &srch_txt_freq, 1, 0 )

LDEF_COND_BACK(&srch_con_condition, srch_txt_freq, &srch_txt_freq_val, 2, COND_SH_IDX, \
          COND_FREQ_TXT_L, COND_FREQ_TXT_T, COND_FREQ_TXT_W, COND_FREQ_TXT_H, RS_SEARCH_COD_FREQ,NULL)

LDEF_COND_BACK(&srch_con_condition, srch_txt_freq_val, &srch_txt_symbol, 3, WSTL_TXT_12, \
          COND_FREQ_TXT_VAL_L, COND_FREQ_TXT_VAL_T, COND_FREQ_TXT_VAL_W, COND_FREQ_TXT_VAL_H, 0,display_strs[30])


LDEF_COND_BACK(&srch_con_condition, srch_txt_symbol, &srch_txt_symbol_val, 5, COND_SH_IDX, \
          COND_SYMBOL_TXT_L, COND_SYMBOL_TXT_T, COND_SYMBOL_TXT_W, COND_SYMBOL_TXT_H, RS_SEARCH_COD_SYMBOL,NULL)

LDEF_COND_BACK(&srch_con_condition, srch_txt_symbol_val, &srch_txt_qam, 6, WSTL_TXT_12, \
          COND_SYMBOL_VAL_L, COND_SYMBOL_VAL_T, COND_SYMBOL_VAL_W, COND_SYMBOL_VAL_H, 0,display_strs[31])


LDEF_COND_BACK(&srch_con_condition, srch_txt_qam, &srch_txt_qam_val, 8, COND_SH_IDX, \
          COND_QAM_TXT_L, COND_QAM_TXT_T, COND_QAM_TXT_W, COND_QAM_TXT_H, RS_SEARCH_COD_COMSEL,NULL)

LDEF_COND_BACK(&srch_con_condition, srch_txt_qam_val, NULL, 9, WSTL_TXT_12, \
          COND_QAM_VAL_L, COND_QAM_VAL_T, COND_QAM_VAL_W, COND_QAM_VAL_H, 0,display_strs[32])


LDEF_PROGRESS_BAR(&search_con, srch_progress_bar, &srch_progress_txt,	\
                  BAR_L, BAR_T, BAR_W, BAR_H, PROGRESSBAR_HORI_NORMAL | PBAR_STYLE_RECT_STYLE, 0, 3, BAR_W, BAR_H - 6)


LDEF_PROGRESS_TXT(&search_con, srch_progress_txt, &ConBarrier1, \
                  BAR_TXT_L, BAR_TXT_T, BAR_TXT_W, BAR_TXT_H)
                  
LDEF_INFOR_TXT(&search_con, srch_info)*/


/*******************************************************************************
 *	Local vriable & function declare
 *******************************************************************************/
struct as_service_param win_search_param;


static char *QamTable[] =
{
   	"QAM-16",
    "QAM-32",
    "QAM-64",
    "QAM-128",
    "QAM-256",
};

//static UINT8 *mid27_buffer = NULL;
static UINT16 cur_srch_sat;
static UINT16 pre_sat_id;
static UINT16 cur_sat_id;
static UINT8 tp_chsrched;
static UINT16 srch_tp_cnt, srch_ch_cnt[2],data_broadcast_cnt,nvod_cnt;
static OSAL_ID srch_cb_sem = OSAL_INVALID_ID;
static P_NODE win_search_cur_prog[2];

static UINT8 updateParam;
static INT32 last_searched_channel=-1;
#ifndef NEW_OSD_SHOW_FOR_SEARCH   // xc.liu  add for  TV and RADIO show in common list
static	UINT16 Total = 0;
#endif
#if (CAS_GY == CAS_TYPE)
static UINT8 b_force_search = 0;
static UINT16 g_wPreChl = 0;
static void gy_force_search_start(void);
static void gy_force_search_end(void);
void gy_force_search_setflag(UINT8 flag);
#endif
static void win_srch_draw_info(OSD_RECT *pRect, char *str, UINT16 *unistr, UINT8 shidx);
static void win_search_capture_semaphore(void);
static void win_search_release_semaphore(void);
static void search_callback(UINT8 aue_type, UINT32 aue_value);
static void win_search_program_append(APPEND_PARAM_TYPE type, UINT32 param2);
static PRESULT win_search_update_progress(UINT8 progress_prec);
static void win_search_init(void);
static PRESULT win_search_message_proc(UINT32 msg_type, UINT32 msg_code);
void win_set_search_param(BOOL useGloablData, UINT32 freqSpecify, UINT32 symbolSpecify, UINT8 qamSpecify, UINT32 nitType);
void DrawSearchParamInfo(UINT32 freq,UINT32 sym,UINT8 qam,BOOL update);
void SetSearchParamMode(UINT8 type);
static void win_search_clear_trash();
void InitSearchParamInfo();

void set_last_searched_prog(INT32 channel);
INT32 get_last_searched_prog(void);
//static void search_restore_nim();
//static void search_record_nim();

/*******************************************************************************
 *	key mapping and event callback definition
 *******************************************************************************/

static PRESULT srch_backtxt_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	UINT8 bID = OSD_GetObjID(pObj);
	OSD_RECT rect;

	switch (event)
	{
		case EVN_POST_DRAW:
			break;

	}

	return ret;
}

static VACTION srch_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act;
	switch (key)
	{
		case V_KEY_EXIT:
		case V_KEY_MENU:
			OSD_ClearObject((POBJECT_HEAD) &g_win_search, C_UPDATE_ALL);
			win_signal_close();
			act = VACT_CLOSE;
			break;
		default:
			act = VACT_PASS;
	}
	return act;
}

void group_by_bat()
{
    SYSTEM_DATA * p_sys_data = sys_data_get();   		
	if(!bat_priv_info_init(p_sys_data->main_frequency,p_sys_data->main_symbol,p_sys_data->main_qam,TRUE))
	{
		get_service_bouquet_info((UINT16 *)&p_sys_data->local_group_cnt,(UINT16 *)p_sys_data->local_group_id,
			(UINT16*)p_sys_data->local_group_name);
		update_services_info();
	}
    update_data();
}
static void searched_prog_sort(UINT8 needBatGroup,UINT8 sort_flag)
{
   	if(needBatGroup&&0==updateParam)
	{		
		group_by_bat();
	}
    switch(sort_flag)
    {
        case 1:  //sort by bat LCN           
        	/********add code here**********/
            break;
         case 2://sort by NIT LCN           
        	/********add code here**********/
            break;
         case 3://sort by Service id            
            sort_prog_node(PROG_TYPE_SID_SORT);
            break;
         case 4://sort by name           
            sort_prog_node(PROG_NAME_SORT);
            break;
         default:
            break;
    }
    
	update_data();
}

static PRESULT srch_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	UINT8 av_flag;
	UINT16 cur_chan;
	lpVSCR ap_vscr = NULL;
    OSD_RECT rc = {0,0,0,0};
	UINT8 pre_nim_locked = 0;
	SYSTEM_DATA * p_sys_data = sys_data_get();
	switch (event)
	{
		case EVN_PRE_OPEN:
			//pre_channel = sys_data_get_cur_group_cur_mode_channel();
			//mode = sys_data_get_cur_chan_mode();
		//	record_scene();//xc.liu
			#if (CAS_TYPE == CAS_GY)
				gy_force_search_start();
			#endif
			win_search_clear_trash();
			InitSearchParamInfo();
		//	wincom_open_title_ext(RS_SEARCH_RESULT, IM_TITLE_ICON_SYSTEM);
		//	wincom_open_help(win_search_help, 1);
			
			memset(display_strs[0], 0, 8);////win_search_clear_trash();////
			
			win_search_update_progress(0);
			key_pan_display("srch", 4);
				
			epg_off();
			stop_tdt();
                     sdt_monitor_off();
			ota_task_suspend();
			//if (mid27_buffer == NULL)
				//mid27_buffer = MALLOC(OSD_VSRC_MEM_MAX_SIZE);
			system_state = SYS_STATE_SEARCH_PROG;
			break;
		case EVN_POST_OPEN:
			win_signal_open(pObj);
			win_search_init();
			ap_vscr = OSD_GetTaskVscr(osal_task_get_current_id());
			OSD_SetRect2(&ap_vscr->vR, &rc);
			break;
		case EVN_PRE_CLOSE:
			as_service_stop();
			db_search_exit();
			/* Make OSD not flickering */

			if(getAutoFullbandScan()==TRUE)
			{
				wincom_close_title();
				wincom_close_help();
				*((UINT32*)param2) &= C_CLOSE_CLRBACK_FLG;
				setAutoFullbandScan(FALSE);
			}else
				*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;

			prog_callback_unregister();
			win_signal_close();
			break;
		case EVN_POST_CLOSE:
			if ((srch_ch_cnt[0] != 0x0) || (srch_ch_cnt[1] != 0x0) 
				|| (data_broadcast_cnt!=0) || (nvod_cnt!=0))
			{
				/*
				win_compopup_init(WIN_POPUP_TYPE_SMSG);
				win_compopup_set_msg(NULL, NULL, RS_MSG_WAIT_FOR_SAVING);
				win_compopup_open();
				*/
				searched_prog_sort(0,0);

				sys_data_check_channel_groups();

	
				/* we check searched tv program first, then radio */
				av_flag = TV_CHAN;
				if((srch_ch_cnt[0] != 0x0) || (srch_ch_cnt[1] != 0x0))
				{
					if(srch_ch_cnt[av_flag]==0)
						av_flag = (av_flag == TV_CHAN)? RADIO_CHAN : TV_CHAN;
				}
				sys_data_set_cur_chan_mode(av_flag);
				sys_data_change_group(0);

               
    //            win_compopup_smsg_restoreback();  //del by Linge 12.06.13
                
				if(srch_ch_cnt[av_flag]>0)
				{
					cur_chan = get_prog_pos(win_search_cur_prog[av_flag].prog_id);
					if(cur_chan == 0xFFFF)
						cur_chan = 0;
					set_last_searched_prog(cur_chan);

					//sys_data_set_cur_group_channel(cur_chan); 	
					//api_play_channel(cur_chan, TRUE, FALSE, FALSE);
				}
				else
					restore_scene();
			}
			else
			 /*search falied or nim can not lock, play pre_channel*/
			{
				restore_scene();
			}
			
			#if (CAS_TYPE == CAS_GY)
			gy_force_search_end();
			#endif
			start_tdt();
                     sdt_monitor_on(0);
			ota_task_resume();
            
			system_state = SYS_STATE_NORMAL;
            		if(get_last_searched_prog()!=-1)
			{			       
				api_play_channel(get_last_searched_prog(),TRUE,TRUE,FALSE);
				set_last_searched_prog(-1);				
			}
			break;
		case EVN_MSG_GOT:
			ret = win_search_message_proc(param1, param2);
			break;

	}

	return ret;
}

static void win_srch_draw_info(OSD_RECT *pRect, char *str, UINT16 *unistr, UINT8 shidx)
{
	TEXT_FIELD *txt;

	txt = &srch_info;
	OSD_SetRect2(&txt->head.frame, pRect);
	if (str != NULL)
		OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)str);
	else if (unistr != NULL)
		OSD_SetTextFieldContent(txt, STRING_UNICODE, (UINT32)unistr);
	OSD_SetColor(txt, shidx, shidx, shidx, shidx);
	OSD_DrawObject((POBJECT_HEAD)txt, C_UPDATE_ALL);
}



void win_search_set_param(struct as_service_param *param)
{
	ASSERT(param != NULL);

	MEMCPY(&win_search_param, param, sizeof(struct as_service_param));
	win_search_param.as_handler = search_callback;
}

static void win_search_capture_semaphore(void)
{
	if (srch_cb_sem == OSAL_INVALID_ID)
		srch_cb_sem = osal_semaphore_create(1);
	if (srch_cb_sem != OSAL_INVALID_ID)
		osal_semaphore_capture(srch_cb_sem, OSAL_WAIT_FOREVER_TIME);
}

static void win_search_release_semaphore(void)
{
	if (srch_cb_sem != OSAL_INVALID_ID)
		osal_semaphore_release(srch_cb_sem);
}

static UINT32 auto_srch_tp_num = 1;
static UINT32 auto_srch_cur_tp_cnt = 0;
static void search_callback(UINT8 aue_type, UINT32 aue_value)
{
	union as_message msg;
	static UINT32 as_progress = 0;
	UINT8 result = PROC_LOOP;
	UINT16 progress;

	DBG_PRINTF("Enter %s....\n", __FUNCTION__);
	//during scan, middle layer task will draw prog info, init this task's osd buffer
	//OSD_TaskBuffer_Init(osal_task_get_current_id(), NULL);
	win_search_capture_semaphore(); //create the queue? and ????
	if (aue_type == ASM_TYPE_PROGRESS)
	{
		if ((win_search_param.as_method==AS_METHOD_NIT_TP ||win_search_param.as_method==AS_METHOD_NIT_TP_EX)
                      && aue_value==0)
		{
			auto_srch_cur_tp_cnt = 0;
			auto_srch_tp_num = 0;
		}

		if (as_progress >= aue_value)
		{
			as_progress = aue_value;

			win_search_release_semaphore();
			DBG_PRINTF("Exit %s....\n", __FUNCTION__);
			return ;
		}
		/*
		 * without the following line will cause
		 * percentage inverse here.
		 */
		as_progress = aue_value;
	}

	msg.decode.asm_type = aue_type;
	msg.decode.asm_value = aue_value;

	switch (msg.decode.asm_type)
	{
		case ASM_TYPE_SATELLITE:
			cur_srch_sat++;
			win_search_release_semaphore();
			ap_clear_all_message();

			ap_send_msg(CTRL_MSG_SUBTYPE_CMD_SCAN, msg.msg_encode, TRUE);
			as_progress = 0;
			break;
		case ASM_TYPE_PROGRESS:
		case ASM_TYPE_ADD_PROGRAM:
		case ASM_TYPE_INC_TRANSPONDER:
		case ASM_TYPE_ADD_TRANSPONDER:
			ap_send_msg(CTRL_MSG_SUBTYPE_CMD_SCAN, msg.msg_encode, TRUE);
            osal_task_sleep(5);
			break;
		case ASM_TYPE_NIT_TP_CNT:
			if (win_search_param.as_method ==AS_METHOD_NIT_TP
                           || win_search_param.as_method ==AS_METHOD_NIT_TP_EX)
			{
				auto_srch_tp_num += msg.decode.asm_value;
			}
			break;
		default:
			;
	}
	//OSD_UpdateWinVscr(&win_search, FALSE);

	if (msg.decode.asm_type == ASM_TYPE_PROGRESS && result == PROC_LEAVE)
		ap_send_msg(CTRL_MSG_SUBTYPE_CMD_EXIT_TOP, 0, TRUE);


	if (msg.decode.asm_type != ASM_TYPE_SATELLITE)
	{
		OSD_ObjProc((POBJECT_HEAD) &search_con, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);

		win_search_release_semaphore();
	}

	lpVSCR pVscr = OSD_GetTaskVscr(osal_task_get_current_id());
	OSD_UpdateVscr(pVscr);
	DBG_PRINTF("Exit %s....\n", __FUNCTION__);
/*
	if (msg.decode.asm_value == 100 || msg.decode.asm_value == 150)
	{
		//when scan over, free middle layer task osd buffer
		OSD_TaskBuffer_Free(osal_task_get_current_id(), NULL);
	}
*/
}

static void win_search_program_append(APPEND_PARAM_TYPE type, UINT32 param2)
{
	UINT32 si, i, j, n, cnt, str_len;
	UINT16 *pstr;
	OSD_RECT rect;
	UINT8 shidx;
	char str[30];
	TEXT_FIELD *tfreq = NULL;
	UINT32 av_flag;

	tfreq = &srch_txt_freq_val;

	DBG_PRINTF("Enter %s....\n", __FUNCTION__);

	if (type == PROG_IDX)
	{
		P_NODE p_node;

		get_prog_at(param2, &p_node);
		if(p_node.service_type==SERVICE_TYPE_DATABROAD)
		{
            data_broadcast_cnt++;
		    return;
		}
		if(p_node.service_type==SERVICE_TYPE_NVOD_TS)
		{
			nvod_cnt++;
			return;
		}
		//if(p_node.video_pid != 0 && p_node.video_pid != 0x1FFF)
		if(p_node.service_type == SERVICE_TYPE_DTV)
		{
			av_flag = PROG_TV_MODE;
			Total ++;
		}
		else
		{
			if(p_node.audio_count == 0) /* we have other programs here, we just didn't show it up*/
				return;
			av_flag = PROG_RADIO_MODE;
			Total ++;
		}
		MEMCPY(&win_search_cur_prog[av_flag], &p_node, sizeof(p_node));
	///////////////////////////////////
		#ifndef NEW_OSD_SHOW_FOR_SEARCH   // xc.liu  add for  TV and RADIO show in common list
		cnt = Total;
		srch_ch_cnt[av_flag]++;
		cnt = srch_ch_cnt[av_flag];
		#endif

		if (cnt <= INFOR_CHAN_CNT)
		{
			si = 0;
			n = cnt;
		}
		else
		{
			si = cnt - INFOR_CHAN_CNT;
			n = INFOR_CHAN_CNT;
		}

		if (si != 0)
		{
			for (i = 0; i < n - 1; i++)
				#ifndef NEW_OSD_SHOW_FOR_SEARCH   // xc.liu  add for  TV and RADIO show in common list
				MEMCPY(display_strs[10 + i], display_strs[10 + i + 1], MAX_DISP_STR_LEN *2);
				#else
				MEMCPY(display_strs[10+p_node.av_flag * 10+i], display_strs[10+p_node.av_flag * 10+i + 1], MAX_DISP_STR_LEN *2);
				#endif
		}
		#ifndef NEW_OSD_SHOW_FOR_SEARCH
		pstr = display_strs[10 + n - 1];
		#else
		pstr = display_strs[10+p_node.av_flag * 10+n - 1];
		#endif
		if (p_node.ca_mode)
			ComAscStr2Uni("$", pstr);
		else
			ComAscStr2Uni("", pstr);
		str_len = ComUniStrLen(pstr);
		ComUniStrCopyChar((UINT8*) &pstr[str_len], p_node.service_name);

		if (p_node.service_type== PROG_TV_MODE)
		{
			rect.uLeft = (INFO_TV_L + 25)+340;
			rect.uTop = INFO_TV_T-28;
			rect.uWidth = INFO_TV_W;
			rect.uHeight = INFO_TXT_H;
			shidx = TV_TXT_SH_IDX;
		}
		else
		{
			rect.uLeft = (INFO_RADIO_L + 20);
			rect.uTop = INFO_RADIO_T;
			rect.uWidth = INFO_RADIO_W;
			rect.uHeight = INFO_TXT_H;
			shidx = RADIO_TXT_SH_IDX;
		}

		#ifdef NEW_OSD_SHOW_FOR_SEARCH
		for (i = 0; i < n; i++)
		{
			pstr = display_strs[10+p_node.av_flag * 10+i];
			win_srch_draw_info(&rect, NULL, pstr, shidx);
			rect.uTop += rect.uHeight;
		}


		rect.uTop = TVB_T;
		rect.uLeft += 160;
		rect.uWidth = 30;
		sprintf(str, "%d", cnt);
		win_srch_draw_info(&rect, str, NULL, WSTL_TXT_7);
		#else
		OSD_DrawObject((POBJECT_HEAD) &search_con, C_UPDATE_ALL);
		#endif
		
		tp_chsrched = 1;

	}
	else if(type==TP_FREQ)
	{
        //param2==0 means the first tp,and it had been draw when open the win_search window.
        if(0!=param2)
        {
            DrawSearchParamInfo(param2,0xffffffff,0xff,TRUE);
        }
	}
	else if(type==TP_IDX)
	{
		T_NODE t_node;
		get_tp_by_id(param2, &t_node);
        DrawSearchParamInfo(t_node.frq,t_node.sym,t_node.FEC_inner,TRUE);
	}

	DBG_PRINTF("Exit %s....\n", __FUNCTION__);
}



static PRESULT win_search_update_progress(UINT8 progress_prec)
{
	UINT8 result = PROC_LOOP;
	win_popup_choice_t popup_result = WIN_POP_CHOICE_NULL;
	UINT8 back_save;
	//PROGRESS_BAR *bar;
	TEXT_FIELD *txt;
	P_NODE node;
	UINT8 str[32];

	DBG_PRINTF("Enter %s....\n", __FUNCTION__);

	BOOL is_flash_full = FALSE;

//	bar = &srch_progress_bar;
	txt = &srch_progress_txt;


	if (progress_prec == 150)
	{
		progress_prec = 100;
		is_flash_full = TRUE;
	}

//	OSD_SetProgressBarPos(bar, progress_prec);
//	if (progress_prec < 3)
//		bar->wTickFg = WSTL_NOSHOW_IDX;
//	else
//		bar->wTickFg = PROGRESSBAR_PROGRESS_SH_IDX;
	OSD_SetTextFieldContent(txt, STRING_NUM_PERCENT, progress_prec);
	if (/*win_search_param.as_method != AS_METHOD_NIT_TP &&*/
	        win_search_param.as_method != AS_METHOD_NIT &&
	        win_search_param.as_method != AS_METHOD_MULTI_NIT)
	{
//		OSD_DrawObject((POBJECT_HEAD)bar, C_UPDATE_ALL);
		OSD_DrawObject((POBJECT_HEAD)txt, C_UPDATE_ALL);
	} /*If search mode is AS_METHOD_NIT_TP/AS_METHOD_NIT/AS_METHOD_MULTI_NIT, do not show progress bar*/


	if (progress_prec == 100)
	// is completed ?
	{
		if ((srch_ch_cnt[0] != 0x0) // radio
		 || (srch_ch_cnt[1] != 0x0)//tv
		 ||(data_broadcast_cnt != 0)
		 ||(nvod_cnt!=0))//data broadcast prog num
		{
                UINT8 *src,strTemp[10];
                UINT32 strLen = 0;

                MEMSET(strTemp,0,sizeof(strTemp));
                MEMSET((UINT8*)len_display_str,0,sizeof(len_display_str));
        
		        src = OSD_GetUnicodeString(RS_SYSTEM_TV);
		        ComUniStrCopyChar((UINT8*)len_display_str, src);
		        strLen = ComUniStrLen(len_display_str);
		    	
		    	sprintf(strTemp,":%03d",srch_ch_cnt[1]);
		        ComAscStr2Uni(strTemp, &len_display_str[strLen]);
		    	strLen = ComUniStrLen(len_display_str);
		        
		        src = OSD_GetUnicodeString(RS_INSTALLATION_SCAN_CHANNEL_RADIO);
		        ComUniStrCopyChar((UINT8*)&len_display_str[strLen], src);
		        strLen = ComUniStrLen(len_display_str); 

		    	sprintf(strTemp,":%03d",srch_ch_cnt[0]);
		        ComAscStr2Uni(strTemp, &len_display_str[strLen]);
		    	strLen = ComUniStrLen(len_display_str);
		        
		    	src = OSD_GetUnicodeString(RS_DATA_BROADCAST_PRG_NUM);
		        ComUniStrCopyChar((UINT8*)&len_display_str[strLen], src);
		        strLen = ComUniStrLen(len_display_str); 

		    	sprintf(strTemp,":%03d",data_broadcast_cnt);
		        ComAscStr2Uni(strTemp, &len_display_str[strLen]);
		    	strLen = ComUniStrLen(len_display_str);
		        
		    	src = OSD_GetUnicodeString(RS_NVOD_PROG);
		        ComUniStrCopyChar((UINT8*)&len_display_str[strLen], src);
		        strLen = ComUniStrLen(len_display_str); 

		    	sprintf(strTemp,":%03d",nvod_cnt);
		        ComAscStr2Uni(strTemp, &len_display_str[strLen]);
		    	strLen = ComUniStrLen(len_display_str); 

		    	len_display_str[strLen++] = 0x0a00;

		    	sprintf(strTemp,"   ");
		    	ComAscStr2Uni(strTemp, &len_display_str[strLen]);
		    	strLen = ComUniStrLen(len_display_str); 
		    	
		    	src = OSD_GetUnicodeString(RS_MSG_SAVING_DATA);
		        ComUniStrCopyChar((UINT8*)&len_display_str[strLen], src);

		    	win_compopup_init(WIN_POPUP_TYPE_SMSG);
		    	win_compopup_set_title(NULL,NULL,RS_INFO_INFO);
		    	win_compopup_set_msg(NULL, (char *)len_display_str, 0);
		    	win_compopup_open_ext(&back_save);
				/*if(0==updateParam)
		    		osal_task_sleep(5000);//osal_task_sleep(18000);
				else*/
					osal_task_sleep(1000);
                win_compopup_smsg_restoreback();
		}
		else
		// not find
		{
			if (!is_flash_full)
			{
				win_compopup_init(WIN_POPUP_TYPE_OK);
				win_compopup_set_msg(NULL, NULL, RS_MSG_NO_PROGRAM_FOUND);
				win_compopup_open_ext(&back_save);
			}
		}

		if (is_flash_full == TRUE)
		{
			win_compopup_init(WIN_POPUP_TYPE_OK);
			win_compopup_set_msg(NULL, NULL, RS_MSG_SPACE_FULL);
			win_compopup_open_ext(&back_save);
		}
		OSD_ClearObject((POBJECT_HEAD)&g_win_search,C_UPDATE_ALL);
		result = PROC_LEAVE;
	}


	DBG_PRINTF("Exit %s....\n", __FUNCTION__);
	return result;
}



static void win_search_init(void)
{
	UINT32 sat_id = 0;
	S_NODE s_node;

	if (win_search_param.as_method != AS_METHOD_MULTI_FFT
	        && win_search_param.as_method != AS_METHOD_MULTI_NIT
	        && win_search_param.as_method != AS_METHOD_MULTI_TP)
		sat_id = win_search_param.sat_ids[0];
	else
	{
		cur_srch_sat = 0;
		//sat_id = win_search_param.as_sat_ids[0];    //Ryan
	}
	cur_srch_sat = 0;

	get_sat_by_id(sat_id, &s_node);
	//pre_sat_pos = INVALID_POS_NUM;//get_sat_pos(sat_id);
	//cur_sat_pos = get_sat_pos(sat_id);//pre_sat_pos;
	pre_sat_id = 0xFFFF;
	cur_sat_id = sat_id;

	srch_tp_cnt = srch_ch_cnt[0] = srch_ch_cnt[1] = 0;
    data_broadcast_cnt = 0;
	nvod_cnt = 0;

	tp_chsrched = 0;


	win_search_update_progress(0);

	as_service_start(&win_search_param);
}



static PRESULT win_search_message_proc(UINT32 msg_type, UINT32 msg_code)
{
	PRESULT ret = PROC_LOOP;
	OSD_RECT rect;
	union as_message msg;
	UINT16 progress;
	UINT32 posi;

	DBG_PRINTF("Enter %s....\n", __FUNCTION__);


	switch (msg_type)
	{
		case CTRL_MSG_SUBTYPE_CMD_SCAN:
			msg.msg_encode = msg_code;

			if (msg.decode.asm_type == ASM_TYPE_PROGRESS)
			{
				if (msg.decode.asm_value == 100 || msg.decode.asm_value == 150)
				{
					ret = win_search_update_progress(msg.decode.asm_value);
				}
				else
				{
					if (win_search_param.as_method != AS_METHOD_MULTI_FFT
				        && win_search_param.as_method != AS_METHOD_MULTI_NIT
				        && win_search_param.as_method != AS_METHOD_MULTI_TP)
						progress = msg.decode.asm_value;
					else
					{
						progress = (100 * (cur_srch_sat - 1) + msg.decode.asm_value) / win_search_param.as_sat_cnt;
						if (progress >= 100)
							progress = 99;
					}
					ret = win_search_update_progress(progress); //msg.decode.asm_value);
				}
			}
			else if (msg.decode.asm_type == ASM_TYPE_SATELLITE)
			{

			/*	S_NODE s_node;
				T_NODE t_node;
				struct cc_antenna_info antenna;
				struct cc_xpond_info xpond_info;
			*/	UINT8 back_saved;

				cur_sat_id = (UINT16)msg.decode.asm_value;
				//get_sat_at(cur_sat_pos, VIEW_ALL,&s_node);
		    /*	get_sat_by_id(cur_sat_id, &s_node);
				get_tp_at(cur_sat_id, 0, &t_node);

				//sat2antenna(&s_node, &antenna);
				sat2antenna_ext(&s_node, &antenna, s_node.tuner1_valid ? 0 : 1);
				set_antenna(&antenna);
				xpond_info.frq = t_node.frq;
				xpond_info.sym = t_node.sym;
				xpond_info.pol = t_node.pol;
				set_xpond(&xpond_info);
			*/
				//Ryan			wincom_dish_move_popup_open(pre_sat_id,cur_sat_id,antenna.lnb_id - 1,&back_saved);
				pre_sat_id = cur_sat_id;

			}
			else if(msg.decode.asm_type == ASM_TYPE_ADD_PROGRAM)
			{
				win_search_program_append(PROG_IDX, msg.decode.asm_value);
			}
			else if(msg.decode.asm_type == ASM_TYPE_INC_TRANSPONDER)
			{
				win_search_program_append(TP_FREQ,msg.decode.asm_value);
			}
			else if(msg.decode.asm_type == ASM_TYPE_ADD_TRANSPONDER)
			{
				win_search_program_append(TP_IDX,msg.decode.asm_value);
				if (win_search_param.as_method == AS_METHOD_NIT_TP
                                 ||win_search_param.as_method == AS_METHOD_NIT_TP_EX)
				{
					if (auto_srch_cur_tp_cnt == 0)
						win_search_update_progress(2);
					else
						win_search_update_progress(98 * auto_srch_cur_tp_cnt / auto_srch_tp_num);
					auto_srch_cur_tp_cnt++;
				}
			}
			break;
		case CTRL_MSG_SUBTYPE_CMD_UPDATE_VIEW:
			break;
		case CTRL_MSG_SUBTYPE_CMD_TIMEDISPLAYUPDATE:
			if (is_time_inited())
			{
			//	wincom_draw_title_time();
			}
            break;
		case CTRL_MSG_SUBTYPE_STATUS_SIGNAL:
			win_signal_update();
			break;
		default:
			break;
	}

	DBG_PRINTF("Exit %s....\n", __FUNCTION__);

	return ret;
}

void DrawSearchParamInfo(UINT32 freq,UINT32 sym,UINT8 qam,BOOL update)
{
   	TEXT_FIELD *tfreq = NULL;
	TEXT_FIELD *tsymbol = NULL;
	TEXT_FIELD *tqam = NULL;

    char str[20];

    MEMSET(str,0,20);

   	tfreq = &srch_txt_freq_val;
	tsymbol = &srch_txt_symbol_val;
	tqam = &srch_txt_qam_val;

    if(freq!=0xffffffff)
    {
        sprintf(str, "%d.%02dMHz", freq/ 100, freq % 100);
        OSD_SetTextFieldContent(tfreq, STRING_ANSI, (UINT32)str);
    }

    if(sym!=0xffffffff)
    {
    sprintf(str, "%dKBaud", sym);
    OSD_SetTextFieldContent(tsymbol, STRING_ANSI, (UINT32)str);
    }

    if(qam!=0xff)
    {
        sprintf(str, "%s", QamTable[qam - 4]);
        OSD_SetTextFieldContent(tqam, STRING_ANSI, (UINT32)str);
    }

    if(update)
    {
    	OSD_DrawObject((POBJECT_HEAD)&srch_txt_freq_val , C_UPDATE_ALL);
    	OSD_DrawObject((POBJECT_HEAD)&srch_txt_symbol_val , C_UPDATE_ALL);
    	OSD_DrawObject((POBJECT_HEAD)&srch_txt_qam_val , C_UPDATE_ALL);        
    }
}

void InitSearchParamInfo()
{
	UINT32 freq = 0;
	UINT32 symbol = 0;
	UINT8 mode = 0;

	SYSTEM_DATA *psd = sys_data_get();

	switch (updateParam)
	{
		case 0:
            freq = psd->main_frequency;
            symbol = psd->main_symbol;
            mode = psd->main_qam;
			break;
		case 1:
			freq = get_freq_from_manual();
			symbol = get_symbol_from_manual();
			mode = get_qam_from_manual();
			break;
		case 2:
            freq = win_search_param.as_from;
			symbol = get_symbol_from_full_ban();
			mode = get_qam_from_full_ban();
			break;
		default:
			break;
	}

    DrawSearchParamInfo(freq,symbol,mode,FALSE);
}

/* useGloablData:
 * 0: for auto,search parameters information will not update
 * 1: for manual ,search parameters information will be user specified
 * 2: for fullband ,search parameters information will updated while searching
 * nitType:
 * 0: not do nit search
 * 1: do nit search
 */
void win_set_search_param(BOOL useGloablData, UINT32 freqSpecify, UINT32 symbolSpecify, UINT8 qamSpecify, UINT32 nitType)
{
	UINT32 search_mode;
	UINT32 prog_type;
	S_NODE s_node;
	struct as_service_param param;
	struct vdec_device *vdec;
	struct vdec_io_get_frm_para_advance vfrm_param;
	UINT32 addr, len;
	T_NODE t_node;
	T_NODE t_node1;
	INT32 ret;

	UINT8 dot_pos;
	UINT32 freq, freq_e,symbol, constellation, nit_search=0;
    
	MEMSET(&param, 0, sizeof(struct as_service_param));
	MEMSET(&s_node, 0, sizeof(S_NODE));
	MEMSET(&t_node, 0, sizeof(T_NODE));
	MEMSET(&vfrm_param, 0, sizeof(struct vdec_io_get_frm_para_advance));

	api_stop_play(0);
	osal_task_sleep(100);

	/*get the parameters for the search*/
	if (useGloablData==0)
	{
		SYSTEM_DATA *sys_data = NULL;
		sys_data = sys_data_get();
		freq = sys_data->main_frequency;
		symbol = sys_data->main_symbol;
		constellation = sys_data->main_qam;
	}
	else if(useGloablData==1)
	{
		freq = freqSpecify;
		symbol = symbolSpecify;
		constellation = qamSpecify;
	}else
	{
		freq = 11100;
		freq_e = 86200;
		symbol = symbolSpecify;
		constellation = qamSpecify;
	//	get_search_param(&freq, &freq_e, &dot_pos,&symbol,&constellation);
	}
    s_node.sat_id = 1;
	//nit search
	if(useGloablData!=2)
	{
    	nit_search = nitType;    	
    	t_node.frq = freq;
    	t_node.sym = symbol;
    	t_node.FEC_inner = constellation;
    	t_node.sat_id = s_node.sat_id;
    	
    	t_node.ft_type = FRONTEND_TYPE_C;

    	recreate_tp_view(VIEW_SINGLE_SAT, s_node.sat_id);
    	ret = lookup_node(TYPE_TP_NODE, &t_node, t_node.sat_id);
    	if (ret != SUCCESS)
    	{
    		add_node(TYPE_TP_NODE, t_node.sat_id, &t_node);
    		update_data();
    	}

	}

	//tv/radio/all
	prog_type = P_SEARCH_TV | P_SEARCH_RADIO | P_SEARCH_DATA;
	search_mode = P_SEARCH_FTA | P_SEARCH_SCRAMBLED;

	param.as_prog_attr = prog_type | search_mode; 


    param.as_frontend_type = FRONTEND_TYPE_C;
    param.as_sat_cnt = 1;
	param.sat_ids[0] = s_node.sat_id;
    param.ft.c_param.sym = symbol;
    param.ft.c_param.constellation = constellation;

	param.as_from = t_node.tp_id;
    if(useGloablData!=2)
    {
        param.as_to = freq_e;
	    param.as_to = 0;
    	if (nit_search == 0)
    		param.as_method = AS_METHOD_TP;
    	else
    	{
    	    if (useGloablData==1)
    	        param.as_method = AS_METHOD_NIT_TP_EX;
           else
                param.as_method = AS_METHOD_NIT_TP;
    	}
    }else{
        param.as_from = freq;
	    param.as_to = freq_e;
	    param.as_method = AS_METHOD_FFT;
    }

	param.as_p_add_cfg = PROG_ADD_REPLACE_OLD;
	param.as_handler = NULL;

	// set param
	win_search_set_param(&param);

	vdec = (struct vdec_device*)dev_get_by_id(HLD_DEV_TYPE_DECV, 0);
	vfrm_param.ufrm_mode = VDEC_UN_DISPLAY;
	vfrm_param.request_frm_number = 1;
	vdec_io_control(vdec, VDEC_IO_GET_FRM_ADVANCE, (UINT32) &vfrm_param);
	if (vfrm_param.return_frm_number > 0)
	{
		addr = vfrm_param.tFrmInfo[0].uC_Addr;
		len = 720 * 576 * 3 / 2;
		db_search_init((UINT8*)addr, len);
	}
	else
		ASSERT(0);
    SetSearchParamMode(useGloablData);
}


/* updateParam:
 * 0: for auto,search parameters information will not update
 * 1: for manual ,search parameters information will be user specified
 * 2: for fullband ,search parameters information will updated while searching
 */
void SetSearchParamMode(UINT8 type)
{
	updateParam = type;
	return ;
}


static void win_search_clear_trash()
{
	UINT8 i;
	memset(display_strs[0], 0, MAX_DISP_STR_LEN *2);
	memset(display_strs[30], 0, MAX_DISP_STR_LEN *3);
	Total = 0;
	for(i = 0;i<INFOR_CHAN_CNT;i++)
	{
		memset(display_strs[10+i], 0, sizeof(display_strs[i]));
	}

}


void set_last_searched_prog(INT32 channel)
{
	last_searched_channel=channel;
}


INT32 get_last_searched_prog(void)
{
	return last_searched_channel;
}
#if (CAS_TYPE == CAS_GY)

static void gy_force_search_start(void)
{
	//PGUI_VSCR pVscr = NULL;
	if(!b_force_search)
	{
		return;
	}
//	struct help_item_resource win_ps_help[] =
//	{
//	    {IM_HELP_ICON_OK,IM_HELP_ICON_QR,RS_HELP_ENTER_INTO},
//	    {IM_HELP_ICON_MENU,IM_HELP_ICON_CD,RS_HELP_BACK},
//{IM_HELP_ICON_EXIT,IM_HELP_ICON_TC,RS_FULL_PLAY},          
//	};
	g_wPreChl = sys_data_get_cur_group_cur_mode_channel();
	//pVscr = OSD_GetTaskVscr(osal_task_get_current_id());
	//OSD_ClearScrn(pVscr);
	//OSD_ClearScreen();
	key_pan_display("srch", 4);
	ShowMuteOnOff();
	//struct title_item_resource title_rsc ={IM_TITLE_ICON_SEARCH_E,IM_TITLE_ICON_SEARCH,0};
	//wincom_open_title_ext2(&title_rsc);
	//wincom_open_title_ext(RS_SEARCH_RESULT, IM_TITLE_ICON_SYSTEM);
	wincom_open_help(win_search_help, 2);
	//wincom_open_help(win_ps_help, 2);
	//return cur_ch;
}

static void gy_force_search_end(void)
{
	UINT8 cur_mode;
	UINT8 group_idx;
	UINT16 cur_channel;
	if(!b_force_search)
	{
		return;
	}
	//PGUI_VSCR pVscr = NULL;
	UINT16 cur_ch = 0;
	//pVscr = OSD_GetTaskVscr(osal_task_get_current_id());
	//OSD_ClearScrn(pVscr);
	//OSD_ClearScreen();
	ShowMuteOnOff();
	ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (POBJECT_HEAD)&g_win_progname, TRUE);
	api_play_channel(g_wPreChl, TRUE, FALSE, FALSE);
	g_wPreChl = 0;
	gy_force_search_setflag(FALSE);
	//key_pan_display_channel(cur_channel);
}
void gy_force_search_setflag(UINT8 flag)
{
	b_force_search = flag;
	if(b_force_search)
	{
		menu_stack_push((POBJECT_HEAD) &g_win_search);
	}
	else
	{
		menu_stack_pop();
	}
}
#endif



/*
static void search_restore_nim()
{
UINT16 channel = 0;
UIChChgNimcc(freq,symbol,modulation);
channel = sys_data_get_cur_group_cur_mode_channel();
api_play_channel ( channel, TRUE, TRUE, FALSE );
}

static void search_record_nim()
{
struct nim_device *nim_dev = (struct nim_device *)dev_get_by_id ( HLD_DEV_TYPE_NIM, 0 );

nim_get_freq(nim_dev, &freq);
nim_get_sym(nim_dev, &symbol);
nim_get_FEC(nim_dev, &modulation);
}
 */
