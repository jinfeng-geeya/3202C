#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <api/libpub/lib_hde.h>
#include <api/libpub/lib_pub.h>
#include <hld/dis/vpo.h>
#include <api/libtsi/si_monitor.h>
#include <hld/decv/vdec_driver.h>
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

#include "copper_common/system_data.h"
#ifdef MULTI_CAS
#if(CAS_TYPE==CAS_CONAX)
#include <api/libcas/mcas.h>
#endif
#endif

//include the header from xform 
#include "win_mainmenu_vega.h"

#include "single_pic_ad/ad_pic.h"
#include "geeya_ad/adv_geeya.h"
/*******************************************************************************
 *	Objects definition
 *******************************************************************************/
extern CONTAINER g_win_mainmenu;
//extern TEXT_FIELD txt_preview;

//extern CONTAINER mm_title;
extern BITMAP mm_dvbc_tv;
extern BITMAP mm_phone_num;

extern CONTAINER mm_main_container;

extern CONTAINER mm_item_con1;
extern CONTAINER mm_item_con2;
extern CONTAINER mm_item_con3;
extern CONTAINER mm_item_con4;
extern CONTAINER mm_item_con5;
extern CONTAINER mm_item_con6;
extern CONTAINER mm_item_con7;
extern CONTAINER mm_item_con8;

extern BITMAP mm_item_bmp1;
extern BITMAP mm_item_bmp2;
extern BITMAP mm_item_bmp3;
extern BITMAP mm_item_bmp4;
extern BITMAP mm_item_bmp5;
extern BITMAP mm_item_bmp6;
extern BITMAP mm_item_bmp7;
extern BITMAP mm_item_bmp8;

extern TEXT_FIELD mm_item_txt1;
extern TEXT_FIELD mm_item_txt2;
extern TEXT_FIELD mm_item_txt3;
extern TEXT_FIELD mm_item_txt4;
extern TEXT_FIELD mm_item_txt5;
extern TEXT_FIELD mm_item_txt6;
extern TEXT_FIELD mm_item_txt7;
extern TEXT_FIELD mm_item_txt8;

static UINT8 main_ad_pic_type = ADV_PIC_TYPE_UNKNOWN;

BOOL get_main_menu_adv_show_status(UINT8 *adv_type)
{

	*adv_type = ADV_PIC_TYPE_UNKNOWN;
	
	if(menu_stack_get_top() != (POBJECT_HEAD)&g_win_mainmenu)
		return FALSE;
	
	*adv_type = main_ad_pic_type;
	
	if(*adv_type  == ADV_PIC_TYPE_UNKNOWN)
		return FALSE;
	
	return TRUE;
}

/*static VACTION mm_item_con_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT mm_item_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION mm_con_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT mm_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);*/
static PRESULT mm_message_proc(UINT32 msg_type, UINT32 msg_code);

/*#define MAIN_CON_IDX    WSTL_MAINMENU_MAIN_CON
#define TITLE_IDX       WSTL_COMMON_BACK_1

#define MAINMENU_PREVIEW_IDX    WSTL_MAINMENU_PREVIEW
#define MM_CON_IDX   WSTL_NOSHOW_IDX

#define BMP_SH_IDX   WSTL_TXT_3
#define BMP_HL_IDX   WSTL_TXT_3
#define BMP_SL_IDX   WSTL_TXT_3
#define BMP_GRY_IDX  WSTL_TXT_3

#define TXT_SH_IDX   WSTL_TXT_3
#define TXT_HL_IDX   WSTL_TXT_9
#define TXT_SL_IDX   WSTL_TXT_9
#define TXT_GRY_IDX  WSTL_TXT_3

#define MAIN_CON_L     0
#define MAIN_CON_T     104
#define MAIN_CON_W     720
#define MAIN_CON_H     386

#define TITLE_L     0
#define TITLE_T     0
#define TITLE_W     720
#define TITLE_H     104

#define TITLE_PIC_TV_L     (48)
#define TITLE_PIC_TV_T     (48)
#define TITLE_PIC_TV_W     224
#define TITLE_PIC_TV_H     48

#define TITLE_PIC_NUM_L     (TV_OFFSET_L+444)
#define TITLE_PIC_NUM_T     (TV_OFFSET_T+62)
#define TITLE_PIC_NUM_W     240
#define TITLE_PIC_NUM_H     32


#define MM_CON_L		    (TV_OFFSET_L+70)
#define MM_CON_T		    (TV_OFFSET_T+118)
#define MM_CON_W		    (BMP_W+8*2)
#define MM_CON_H		    (BMP_H+TXT_H)
#define MM_CON_GAP_X		4
#define MM_CON_GAP_Y		394

#define BMP_L_OF	8
#define BMP_T_OF	0
#define BMP_W		80
#define BMP_H		64

#define TXT_L_OF  		0
#define TXT_T_OF 		BMP_H
#define TXT_W			(24*4)
#define TXT_H			24*/

#define MM_CON_L		    (TV_OFFSET_L+70)
#define BMP_W		80
#define MM_CON_W		    (BMP_W+8*2)

#define MM_PREVIEW_L		    (MM_CON_L+MM_CON_W+4)+50+36//170
#define MM_PREVIEW_T		    (TV_OFFSET_T+132)+93
#define MM_PREVIEW_W		    386-32+5
#define MM_PREVIEW_H		    316-63
#if 0
#define PREVIEW_L 200
#define PREVIEW_T 168
#define PREVIEW_W 324
#define PREVIEW_H 244
#endif
#define PREVIEW_L MM_PREVIEW_L
#define PREVIEW_T MM_PREVIEW_T
#define PREVIEW_W MM_PREVIEW_W
#define PREVIEW_H MM_PREVIEW_H

#define POPUP_MSG_LEFT	345
#define POPUP_MSG_TOP 310
#define POPUP_MSG_WIDTH	150
#define POPUP_MSG_HIGH	50
/*#define LDEF_CON(root, varCon,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,conobj,focusID)		\
    DEF_CONTAINER(varCon,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, MM_CON_IDX,MM_CON_IDX,MM_CON_IDX,MM_CON_IDX,   \
    mm_item_con_keymap,mm_item_con_callback,  \
    conobj, ID,1)

#define LDEF_BMP(root,varbmp,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,bmpID)		\
    DEF_BITMAP(varbmp,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, BMP_SH_IDX,BMP_HL_IDX,BMP_SL_IDX,BMP_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,bmpID)

#define LDEF_TXT(root,varTxt,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,resID,str)		\
    DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER| C_ALIGN_VCENTER, 0,0,resID,str)

#define LDEF_MM_ITEM(root,varCon,nxtObj,varbmp,varTxt,ID,IDl,IDr,IDu,IDd,l,t,w,h,bmpID,resID)	\
	LDEF_CON(&root,varCon,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,&varbmp,1)	\
	LDEF_BMP(&varCon,varbmp,&varTxt,1,1,1,1,1,l + BMP_L_OF,t + BMP_T_OF,BMP_W,BMP_H,bmpID)	\
	LDEF_TXT(&varCon,varTxt,NULL,0,0,0,0,0,l + TXT_L_OF, t + TXT_T_OF,TXT_W,TXT_H,resID,NULL)

#define LDEF_PREVIEW_WIN(root,varTxt,nxtObj,l,t,w,h)		\
    DEF_TEXTFIELD(varTxt,&root,nxtObj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, MAINMENU_PREVIEW_IDX,MAINMENU_PREVIEW_IDX,MAINMENU_PREVIEW_IDX,MAINMENU_PREVIEW_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,0,NULL)


LDEF_MM_ITEM ( g_win_mainmenu, mm_item_con1, &mm_item_con2, mm_item_bmp1, mm_item_txt1, 1,5,5,4,2,	\
               MM_CON_L, MM_CON_T + ( MM_CON_H + MM_CON_GAP_X ) *0, MM_CON_W, MM_CON_H,	0,	0 )

LDEF_MM_ITEM ( g_win_mainmenu, mm_item_con2, &mm_item_con3, mm_item_bmp2, mm_item_txt2, 2,6,6,1,3,	\
               MM_CON_L, MM_CON_T + ( MM_CON_H + MM_CON_GAP_X ) *1, MM_CON_W, MM_CON_H,	0,	0 )

LDEF_MM_ITEM ( g_win_mainmenu, mm_item_con3, &mm_item_con4, mm_item_bmp3, mm_item_txt3, 3,7,7,2,4,	\
               MM_CON_L, MM_CON_T + ( MM_CON_H + MM_CON_GAP_X ) *2, MM_CON_W, MM_CON_H,	0,	0 )

LDEF_MM_ITEM ( g_win_mainmenu, mm_item_con4, &mm_item_con5, mm_item_bmp4, mm_item_txt4, 4,8,8,3,1,	\
               MM_CON_L, MM_CON_T + ( MM_CON_H + MM_CON_GAP_X ) *3, MM_CON_W, MM_CON_H,	0,	0 )

LDEF_MM_ITEM ( g_win_mainmenu, mm_item_con5, &mm_item_con6, mm_item_bmp5, mm_item_txt5, 5,1,1,8,6,	\
               (MM_CON_L+MM_CON_W+MM_CON_GAP_Y), MM_CON_T + ( MM_CON_H + MM_CON_GAP_X ) *0, MM_CON_W, MM_CON_H,	0,	0 )

LDEF_MM_ITEM ( g_win_mainmenu, mm_item_con6, &mm_item_con7, mm_item_bmp6, mm_item_txt6, 6,2,2,5,7,	\
               (MM_CON_L+MM_CON_W+MM_CON_GAP_Y), MM_CON_T + ( MM_CON_H + MM_CON_GAP_X ) *1, MM_CON_W, MM_CON_H,	0,	0 )

LDEF_MM_ITEM ( g_win_mainmenu, mm_item_con7, &mm_item_con8, mm_item_bmp7, mm_item_txt7, 7,3,3,6,8,	\
               (MM_CON_L+MM_CON_W+MM_CON_GAP_Y), MM_CON_T + ( MM_CON_H + MM_CON_GAP_X ) *2, MM_CON_W, MM_CON_H,	0,	0 )

LDEF_MM_ITEM ( g_win_mainmenu, mm_item_con8,NULL, mm_item_bmp8, mm_item_txt8, 8,4,4,7,5,	\
               (MM_CON_L+MM_CON_W+MM_CON_GAP_Y), MM_CON_T + ( MM_CON_H + MM_CON_GAP_X ) *3, MM_CON_W, MM_CON_H,	0,	0 )

DEF_CONTAINER ( g_win_mainmenu, NULL, NULL, C_ATTR_ACTIVE, 0, \
                1, 0, 0, 0, 0, MAIN_CON_L, MAIN_CON_T, MAIN_CON_W, MAIN_CON_H, MAIN_CON_IDX, MAIN_CON_IDX, MAIN_CON_IDX, MAIN_CON_IDX,   \
                mm_con_keymap, mm_con_callback,  \
                ( POBJECT_HEAD ) &mm_item_con1, 1, 0 )

DEF_CONTAINER ( mm_title, NULL, NULL, C_ATTR_ACTIVE, 0, \
                1, 0, 0, 0, 0, TITLE_L, TITLE_T, TITLE_W, TITLE_H, TITLE_IDX, TITLE_IDX, TITLE_IDX, TITLE_IDX,   \
                NULL, NULL,  \
                ( POBJECT_HEAD ) &mm_dvbc_tv, 0, 0 )

DEF_BITMAP(mm_dvbc_tv,&mm_title,&mm_phone_num,C_ATTR_ACTIVE,0, \
    1,0,0,0,0, TITLE_PIC_TV_L,TITLE_PIC_TV_T,TITLE_PIC_TV_W,TITLE_PIC_TV_H, BMP_SH_IDX,BMP_HL_IDX,BMP_SL_IDX,BMP_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,IM_TITLE_TEXT_01)

DEF_BITMAP(mm_phone_num,&mm_title,NULL,C_ATTR_ACTIVE,0, \
    1,0,0,0,0, TITLE_PIC_NUM_L,TITLE_PIC_NUM_T,TITLE_PIC_NUM_W,TITLE_PIC_NUM_H, BMP_SH_IDX,BMP_HL_IDX,BMP_SL_IDX,BMP_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,IM_TITLE_TEXT_02)*/

#define DTV_ID	            1
#define PROG_SEARCH_ID	    6
#define EMAIL_ID            4
#define GAME_ID	            7
#define RADIO_ID	        2
#define NVOD_ID		        8
#define EPG_ID		        3
#define SYS_CONFIG_ID		5

extern BOOL mode_changed_flag;
extern void ChanListSetModeChangedFlag(BOOL flag);
UINT8 new_channel_change_flag = 0;

void show_black();
void fill_black();
/*cause in mainmenu  can not change mode so we didn't need mode, group..*/
static P_NODE current_pnode;

uint red1=0;

/*******************************************************************************
 *	Local functions & variables define
 *******************************************************************************/
POBJECT_HEAD mm_menu_items[] =
{
    ( POBJECT_HEAD ) &mm_item_con1,
    ( POBJECT_HEAD ) &mm_item_con2,
    ( POBJECT_HEAD ) &mm_item_con3,
    ( POBJECT_HEAD ) &mm_item_con4,
    ( POBJECT_HEAD ) &mm_item_con5,
};

BITMAP *mm_item_bmps[] =
{
    &mm_item_bmp1,
    &mm_item_bmp2,
    &mm_item_bmp3,
    &mm_item_bmp4,
    &mm_item_bmp5,
};


UINT16 mm_item_bmp_ids[][3] =
{
        {IM_ICON1_B,	IM_ICON1_Y,			IM_ICON1_Y},
        {IM_ICON2_B,	IM_ICON2_Y,			IM_ICON2_Y},
        {IM_ICON3_B,	IM_ICON3_Y,			IM_ICON3_Y},
        {IM_ICON4_B,	IM_ICON4_Y,			IM_ICON4_Y},
        {IM_ICON5_B,	IM_ICON5_Y,			IM_ICON5_Y},
};
#if 0
UINT16 mm_item_str_ids[] =
{
	RS_DIGITAL_TV,
#ifdef DATA_BROADCAST
	RS_DATA_BROADCAST,
#else
	RS_PROG_SEARCH,
#endif
        RS_EMAIL,
        RS_TV_GAME,
        RS_RADIO,
        RS_NVOD,
        RS_EPG,
        RS_SYSTEM_SETUP
};
#endif

#define BMP_SH_ICON	0
#define BMP_HL_ICON	1
#define BMP_SL_ICON	2

#define MENU_ITME_NUM	(sizeof(mm_menu_items)/sizeof(mm_menu_items[0]))
/*******************************************************************************
 *	key mapping and event callback definition
 *******************************************************************************/

//************************menu item: key mapping & event definition**************
static VACTION mm_item_con_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

	switch (key)
	{
		case V_KEY_ENTER:
			act = VACT_ENTER;
			break;
		case  V_KEY_RED:
			red1=1;
			act = VACT_PASS;
			break;
		case V_KEY_1:	 
			if(1==red1)
			{
			red1=2;
			}
			act = VACT_PASS;
			break;
		default:
			act = VACT_PASS;
	}

	return act;
}

POBJECT_HEAD mm_windows_list[8] =
{
    (POBJECT_HEAD)&g_win_chanlist,//tv program
	(POBJECT_HEAD)&g_win_chanlist,//radio program
	(POBJECT_HEAD)&g_win_epg,//EPG
#ifdef MULTI_CAS
#if(CAS_TYPE!=CAS_IRDETO)
    (POBJECT_HEAD)&g_win_mail,
#else
	NULL,
#endif
#else
    NULL,
#endif
 (POBJECT_HEAD)&g_win_sys_management,
 //////////////////////////////
#ifdef DATA_BROADCAST
    #if(DATA_BROADCAST==DATA_BROADCAST_IPANEL)
	    (POBJECT_HEAD)&g_win_ipanel,
	#elif(DATA_BROADCAST==DATA_BROADCAST_ENREACH)
        (POBJECT_HEAD)&g_win_enreach,
	#endif
#else
    (POBJECT_HEAD)&g_win_prog_search,
#endif
#ifdef MULTI_CAS
#if(CAS_TYPE!=CAS_IRDETO)
    (POBJECT_HEAD)&g_win_mail,
#else
	NULL,
#endif
#else
    NULL,
#endif
    (POBJECT_HEAD)&g_win_games,
    (POBJECT_HEAD)&g_win_chanlist,
#ifdef NVOD_FEATURE
    (POBJECT_HEAD)&g_win_nvod,
#else
	NULL,
#endif
    (POBJECT_HEAD)&g_win_epg,
    (POBJECT_HEAD)&g_win_sys_management,
};

void ChanListSetTitleID(UINT16 strID, UINT16 bmpID);

static PRESULT mm_item_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	POBJECT_HEAD item;
	VACTION unact;
	UINT8 id = OSD_GetObjID(pObj);
	UINT8 back_saved;
	BITMAP *bmp;
	UINT8 av_mode;
	UINT16 curChannel;
	UINT8 pre_chan_mode;
	UINT8 groupIdx=0;    


#ifdef MULTI_CAS
#if(CAS_TYPE==CAS_DVT)
	if(2==red1)
	{
	if(PROC_LEAVE != OSD_ObjOpen((POBJECT_HEAD)&g_win_conceal_container, MENU_OPEN_TYPE_MENU))
		{
			#ifdef AD_SANZHOU
			fill_black();
			curChannel = sys_data_get_cur_group_cur_mode_channel();
			api_play_channel(curChannel, TRUE, FALSE, FALSE);
			if (screen_back_state == SCREEN_BACK_RADIO)
			{
				api_show_radio_logo();
			}
			#endif
			menu_stack_push((POBJECT_HEAD)&g_win_conceal_container);
			red1=0;
		}			
	}
#endif
#endif

	bmp = mm_item_bmps[id - 1];

	switch (event)
	{
		case EVN_FOCUS_PRE_GET:
			/* Clear normal show & set bmp highlight icon */
			OSD_SetBitmapContent(bmp, mm_item_bmp_ids[id - 1][BMP_HL_ICON]);
			break;
		case EVN_FOCUS_POST_GET:
			break;
		case EVN_FOCUS_PRE_LOSE:
			/* Clear higlight show & set bmp normal icon */
			OSD_SetBitmapContent(bmp, mm_item_bmp_ids[id - 1][BMP_SH_ICON]);
			break;
		case EVN_UNKNOWN_ACTION:
			unact = (VACTION)(param1 >> 16);
			if (unact == VACT_ENTER)
			{
				
				if (mm_windows_list[id - 1] != NULL)
				{
/*#ifdef DATA_BROADCAST
					if (id == PROG_SEARCH_ID)
					{
						curChannel = sys_data_get_cur_group_cur_mode_channel();
						pre_chan_mode = sys_data_get_cur_chan_mode();
						groupIdx = sys_data_get_cur_group_index();
                 
						api_stop_play(TRUE);
						ali_ipanel_enter_mw(1, 31500);
						ap_osd_init();

						OSD_ObjOpen((POBJECT_HEAD) &g_win_mainmenu, C_UPDATE_ALL);
*/
						/*return to the previous db view, group, channel,*/
						/*if(groupIdx>=(1+MAX_SAT_NUM))		
							recreate_prog_view ( VIEW_FAV_GROUP | pre_chan_mode, groupIdx-1- MAX_SAT_NUM);
						else
							recreate_prog_view ( VIEW_ALL | pre_chan_mode, 0 );
						sys_data_change_group ( groupIdx );
						sys_data_set_cur_group_channel(curChannel);
                        
						api_play_channel(curChannel, TRUE, FALSE, FALSE);
						ret = PROC_LOOP;
						break;
					}
#endif*/
					av_mode = sys_data_get_cur_chan_mode();
					if ((id == DTV_ID))
					{
						if (TV_CHAN != av_mode&&new_channel_change_flag==0)
							ChanListSetModeChangedFlag(TRUE);
						else
							ChanListSetModeChangedFlag(FALSE);
						recreate_prog_view(VIEW_ALL | TV_CHAN, 0);
						sys_data_set_cur_chan_mode(TV_CHAN);
						if (get_prog_num(VIEW_ALL | TV_CHAN, 0) > 0)
						{
							/*if the mode will change, then we show black*/
							////fill_black();
							//show_black();
							//ChanListSetModeChangedFlag(FALSE);
							if (TV_CHAN != av_mode)
							{
								sys_data_set_cur_chan_mode(TV_CHAN);
								sys_data_change_group(0);
							}
						}
						else
						{
							win_compopup_init(WIN_POPUP_TYPE_SMSG);
							win_compopup_set_msg(NULL, NULL, RS_MSG_NO_PROGRAM_TV);
							win_compopup_set_frame(POPUP_MSG_LEFT, POPUP_MSG_TOP, POPUP_MSG_WIDTH, POPUP_MSG_HIGH);
							win_compopup_open_ext(&back_saved);
							osal_task_sleep(1000);
							win_compopup_smsg_restoreback();
							if (TV_CHAN != av_mode)
								recreate_prog_view(VIEW_ALL | av_mode, 0);
							key_pan_display("noCH", 4);
							ret = PROC_LOOP;
							return ret;
						}
						ChanListSetTitleID(RS_DIGITAL_TV, IM_TITLE_ICON_TV);
					}

					if ((id == RADIO_ID))
					{
						if (RADIO_CHAN != av_mode&&new_channel_change_flag==0)
							ChanListSetModeChangedFlag(TRUE);
						else
							ChanListSetModeChangedFlag(FALSE);
						recreate_prog_view(VIEW_ALL | RADIO_CHAN, 0);
						sys_data_set_cur_chan_mode(RADIO_CHAN);
						if(get_prog_num(VIEW_ALL | RADIO_CHAN, 0) > 0)
						{
							//show_black();
							//ChanListSetModeChangedFlag(TRUE);
							if (RADIO_CHAN != av_mode)
							{
								sys_data_set_cur_chan_mode(RADIO_CHAN);
								sys_data_change_group(0);
							}
						}
						else
						{
							win_compopup_init(WIN_POPUP_TYPE_SMSG);
							win_compopup_set_msg(NULL, NULL, RS_MSG_NO_RADIO_CHANNEL);
							win_compopup_set_frame(POPUP_MSG_LEFT, POPUP_MSG_TOP, POPUP_MSG_WIDTH, POPUP_MSG_HIGH);
							win_compopup_open_ext(&back_saved);
							osal_task_sleep(1000);
							win_compopup_smsg_restoreback();
							if (RADIO_CHAN != av_mode)
								recreate_prog_view(VIEW_ALL | av_mode, 0);
							key_pan_display("noCH", 4);
							ret = PROC_LOOP;
							return ret;								
						}
						ChanListSetTitleID(RS_RADIO, IM_TITLE_ICON_RADIO);
					}
					if ((id == EPG_ID))
					{
						if (TV_CHAN != av_mode)
							recreate_prog_view(VIEW_ALL | TV_CHAN, 0);
						
						if (get_prog_num(VIEW_ALL | TV_CHAN, 0) > 0)
						{
							/*if the mode will change, then we show black*/
							////fill_black();
							//show_black();
							
							if (TV_CHAN != av_mode)
							{
								//ChanListSetModeChangedFlag(TRUE);
								sys_data_set_cur_chan_mode(TV_CHAN);
								sys_data_change_group(0);
							}
						}
						else
						{
							win_compopup_init(WIN_POPUP_TYPE_SMSG);
							win_compopup_set_msg(NULL, NULL, RS_MSG_NO_CHANNELS);
							win_compopup_set_frame(POPUP_MSG_LEFT, POPUP_MSG_TOP, POPUP_MSG_WIDTH, POPUP_MSG_HIGH);
							win_compopup_open_ext(&back_saved);
							osal_task_sleep(1000);
							win_compopup_smsg_restoreback();
							if (RADIO_CHAN != av_mode)
								recreate_prog_view(VIEW_ALL | av_mode, 0);
							key_pan_display("noCH", 4);
							ret = PROC_LOOP;
							return ret;								
						}
					}

                   			 //ad_stop(AD_MAINMENU);
                   	wincom_close_preview();
                   	OSD_ClearObject((POBJECT_HEAD)& g_win_mainmenu, C_UPDATE_ALL);
					if (OSD_ObjOpen(mm_windows_list[id - 1], 0xFFFFFFFF) != PROC_LEAVE)
					{
						#ifdef AD_SANZHOU
						fill_black();
						curChannel = sys_data_get_cur_group_cur_mode_channel();
						api_play_channel(curChannel, TRUE, FALSE, FALSE);
						if (screen_back_state == SCREEN_BACK_RADIO)
						{
							api_show_radio_logo();
						}
						#endif
						menu_stack_push(mm_windows_list[id - 1]);
					}
                    else
                    {
                        //ad_show(AD_MAINMENU);
					}
				}
			}

			ret = PROC_LOOP;
			break;
		default:
			break;
	}
	return ret;
}

//************************window: key mapping & event definition**************
static VACTION mm_con_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act;
//    AD_DATA ad_data;

	switch (key)
	{
		case V_KEY_LEFT:
			act = VACT_CURSOR_LEFT;
			break;
		case V_KEY_RIGHT:
			act = VACT_CURSOR_RIGHT;
			break;
		case V_KEY_UP:
			act = VACT_CURSOR_UP;
			break;
		case V_KEY_DOWN:
			act = VACT_CURSOR_DOWN;
			break;
		case V_KEY_EXIT:
		case V_KEY_MENU:
			wincom_close_preview();
			OSD_ClearObject((POBJECT_HEAD) &g_win_mainmenu, C_UPDATE_ALL);
			act = VACT_CLOSE;
			break;
#ifdef ALI_DEMO_AD//test read ad data from flash&display
#ifdef ALI_AD_DEMO_ON
        case V_KEY_BLUE:
            ad_stop(AD_MAINMENU);
            osal_task_sleep(3000);
            ad_data.rawdata = NULL;
            if(0 == ad_sto_read(AD_MAINMENU,&ad_data))
            {
                ad_decoder_queue_insert(&ad_data,0);
                ad_display_queue_insert(&ad_data,1);
                osal_task_sleep(3000);                
                if(AD_GIF == ad_data.pic_type)
                {
                    ad_pic_gif_close_byid(ad_data.handle);
                }                
                FREE(ad_data.rawdata);
                ad_data.rawdata = NULL;
                ad_pic_gfx_clear(&ad_data.show_area);
            }
            ad_show(AD_MAINMENU);
            act = VACT_PASS;
            break;
#endif
#endif
		default:
			act = VACT_PASS;
	}

	return act;


}

extern UINT8 show_and_playchannel;
extern UINT8 play_chan_nim_busy;

struct help_item_resource mm_help[] =
{
    {0,RS_COMMON_OK,RS_HELP_ENTER_INTO},
};

static PRESULT mm_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	UINT8 focusID = OSD_GetFocusID(pObj);
	UINT32 i;
	POBJECT_HEAD item;
	BITMAP *bmp;
	TEXT_FIELD *txt;
	UINT8 back_saved, cur_mode;
	UINT8 av_flag;
	UINT8 group_idx;
	UINT8 no_channel = 0;
	OSD_RECT rc_con;
	OSD_RECT rc_preview;
	UINT16 cur_channel;
	VACTION uact;
    UINT32 galpah_value;
	item = OSD_GetFocusObject(pObj);
#ifdef USE_LIB_GE
    PGUI_VSCR pVscr = NULL;
    pVscr = OSD_GetTaskVscr(osal_task_get_current_id());
#endif    
	switch (event)
	{
		case EVN_PRE_OPEN:
			/* Set each item to noraml position and normal bmp icon */
			for (i = 0; i < MENU_ITME_NUM; i++)
			{
				bmp = mm_item_bmps[i];
				OSD_SetBitmapContent(bmp, mm_item_bmp_ids[i][BMP_SH_ICON]);
			}

			/* Set focus item highlight position and highlight bmp icon */
			item = OSD_GetFocusObject(pObj);
			bmp = mm_item_bmps[focusID - 1];
			OSD_SetBitmapContent(bmp, mm_item_bmp_ids[focusID - 1][BMP_HL_ICON]);

			av_flag = sys_data_get_cur_chan_mode();
			if (sys_data_get_sate_group_num(av_flag) > 0)
			{
				key_pan_display("----", 4);
				/*there are some prog playing, get it's pnode for later use*/
				cur_channel = sys_data_get_cur_group_cur_mode_channel();
				memset(&current_pnode, 0, sizeof(current_pnode));
				get_prog_at(cur_channel, &current_pnode);
			}
			else
			{
				key_pan_display("noCH", 4);
			}
#ifdef USE_LIB_GE
            OSD_SetVscrSuspend(pVscr);
			OSD_SetRect(&(pVscr->frm),0,0,720,576);
#endif            
//			OSD_DrawObject((POBJECT_HEAD) &mm_title, C_UPDATE_ALL);
		//	wincom_open_help(mm_help, 1);
			sys_data_set_cur_satidx((UINT16)(~0)); //restore sat_idx
			//set_update_flag(TRUE);
			rc_con.uLeft = MM_PREVIEW_L;
			rc_con.uTop = MM_PREVIEW_T;
			rc_con.uWidth = MM_PREVIEW_W;
			rc_con.uHeight = MM_PREVIEW_H;

			rc_preview.uLeft = PREVIEW_L;
			rc_preview.uTop = PREVIEW_T;
			rc_preview.uWidth = PREVIEW_W;
			rc_preview.uHeight = PREVIEW_H;
	
			wincom_open_preview(rc_con, rc_preview, WSTL_NOSHOW_EXT);
			//ad_show(AD_MAINMENU);
			if(SUCCESS != show_channel_ad(SHOW_GYADV_MAIN_MENU,0 ,&main_ad_pic_type))
				main_ad_pic_type = ADV_PIC_TYPE_UNKNOWN;
			break;
		case EVN_POST_OPEN:

			#ifdef AD_SANZHOU
			api_stop_play(TRUE);
			api_show_menu_logo();
			#endif
			////api_fill_black_screen();
 #ifdef USE_LIB_GE
			OSD_ClearVscrSuspend(pVscr);
 #endif
			break;
		case EVN_PRE_CLOSE:
			#ifdef AD_SANZHOU
			fill_black();
			#endif
			no_channel = 1;
			cur_mode = sys_data_get_cur_chan_mode();
			recreate_prog_view(VIEW_ALL | cur_mode, 0);
			if (get_prog_num(VIEW_ALL | cur_mode, 0) > 0)
			{
				/* we do nothing here, so enter to g_win_progname will not cause a pause*/
                		//ad_stop(AD_MAINMENU);
				break;
			}

			cur_mode = (cur_mode == PROG_TV_MODE) ? PROG_RADIO_MODE : PROG_TV_MODE;
			recreate_prog_view(VIEW_ALL | cur_mode, 0);
			if (get_prog_num(VIEW_ALL | cur_mode, 0) > 0)
			{
                		//ad_stop(AD_MAINMENU);
				sys_data_set_cur_chan_mode(cur_mode);
				/*we shoud play it since the mode have changed*/
				show_and_playchannel = 1; 
				break;
			}

			if (no_channel)
			{
				win_compopup_init(WIN_POPUP_TYPE_SMSG);
				win_compopup_set_msg(NULL, NULL, RS_MSG_NO_CHANNELS);
				win_compopup_open_ext(&back_saved);
				osal_task_sleep(1000);
				win_compopup_smsg_restoreback();
				key_pan_display("noCH", 4);
				OSD_ObjOpen((POBJECT_HEAD) &g_win_mainmenu, 0xFFFFFFFF);
				ret = PROC_LOOP;
			}
			
			break;
		case EVN_POST_CLOSE:
			#ifdef MIS_AD
			MIS_Set_EnterMainMenu_Full(FALSE);
			#endif
			#ifdef AD_SANZHOU
			cur_channel = sys_data_get_cur_group_cur_mode_channel();
			api_play_channel(cur_channel, TRUE, FALSE, FALSE);
			if (screen_back_state == SCREEN_BACK_RADIO)
			{
				api_show_radio_logo();
			}
			#endif
//			OSD_ClearObject((POBJECT_HEAD) &mm_title, C_UPDATE_ALL);
		//	wincom_close_help();
			wincom_close_preview();
			group_idx = sys_data_get_cur_group_index();
			sys_data_change_group(group_idx);
			#ifndef AD_SANZHOU
			cur_channel = sys_data_get_cur_group_cur_mode_channel();
			#endif
			key_pan_display_channel(cur_channel);

			////api_fill_black_screen();
			ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (POBJECT_HEAD) &g_win_progname, TRUE);
#ifdef MULTI_CAS
#if(CAS_TYPE==CAS_IRDETO)
			if(IRCA_BAN_GetBannerShow())
			{
				//ap_cas_call_back(0x00000020);
				//ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MCAS,0x00200000 , FALSE);
				ap_send_msg_expand(CTRL_MSG_SUBTYPE_STATUS_MCAS,0x00000020 ,0, FALSE);
			}
#elif(CAS_TYPE==CAS_CONAX)
			if((get_mmi_showed()== 0x02) && (ap_get_cas_dispstr(0)==RS_CONAX_NO_CARD))
				ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MCAS, MCAS_DISP_CARD_NON_EXIST<<16, FALSE);
			set_mmi_showed(10);
#endif
#endif
			break;
		case EVN_MSG_GOT:
			ret = mm_message_proc(param1, param2);
			break;
		default:
			break;
	}

	return ret;
}

static PRESULT mm_message_proc(UINT32 msg_type, UINT32 msg_code)
{
	PRESULT ret = PROC_LOOP;

	switch (msg_type)
	{
		#ifndef AD_SANZHOU
		case CTRL_MSG_SUBTYPE_STATUS_SIGNAL:
			wincom_preview_proc();
			break;
		#endif
		default:
			break;
	}

	return ret;
}

void show_black()
{
	struct cc_param param;
    MEMSET(&param, 0, sizeof(param));
    api_set_channel_info(&current_pnode, &param);
    chchg_stop_channel(&param.es, &param.dev_list, TRUE);
}

void fill_black()
{
	struct YCbCrColor tColor;
	tColor.uY = 0x10;
	tColor.uCb = 0x80;
	tColor.uCr = 0x80;
	vdec_io_control((struct vdec_device*)dev_get_by_id(HLD_DEV_TYPE_DECV, 0), VDEC_IO_FILL_FRM, (UINT32)(&tColor));
	vpo_win_onoff((struct vpo_device*)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS), TRUE);
}
