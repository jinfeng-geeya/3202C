#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>

//#include <api/libosd/osd_lib.h>
#include <api/libchunk/chunk.h>
#include <api/libstbinfo/stb_info_data.h>

#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"
#include "win_com.h"
#include "menus_root.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_signalstatus.h"


/*******************************************************************************
 *	Objects & callback functions declaration
 *******************************************************************************/
extern CONTAINER g_win_stb_info;
extern CONTAINER stb_info_menu;
extern TEXT_FIELD txt_item_name;
extern char libcore_banner[];
extern MULTI_TEXT libcore_info;
//extern SCROLL_BAR libcore_scb;

static VACTION win_stb_info_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT win_stb_info_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);
static VACTION libcore_info_keymap ( POBJECT_HEAD pObj, UINT32 key );
/*******************************************************************************
*	base macro & postion paramters define
*******************************************************************************/
#define LDEF_TEXT(root,varTxt,nxtObj,l,t,w,h,style,resID,pString) \
	DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0,\
    0,0,0,0,0,l,t,w,h,style,style,style,style,\
    NULL,NULL,\
    C_ALIGN_LEFT| C_ALIGN_VCENTER,0,0,resID,pString)
    
#define LDEF_WIN(varCon,root,nxtObj,l,t,w,h,nxtInObj,focusID) \
    DEF_CONTAINER(varCon,root,nxtObj,C_ATTR_ACTIVE,0, \
    1,0,0,0,0,l,t,w,h, WIN_BG_IDX,WIN_BG_IDX,WIN_BG_IDX,WIN_BG_IDX,\
    win_stb_info_keymap, win_stb_info_callback,  \
    nxtInObj, 1,0)

#define LDEF_CON(varCon,root,nxtObj,nxtInObj,l,t,w,h)		\
    DEF_CONTAINER(varCon,root,nxtObj,C_ATTR_ACTIVE,0, \
    1,0,0,0,0, l,t,w,h, CON_MENU_IDX,CON_MENU_IDX,CON_MENU_IDX,CON_MENU_IDX,   \
    NULL,NULL,  \
    nxtInObj, 1,0)

#define LDEF_MTXT(root,varMtxt,nextObj,ID,l,t,w,h,cnt,sb,content)	\
	DEF_MULTITEXT(varMtxt,root,nextObj,C_ATTR_ACTIVE,0, \
    	ID,ID,ID,ID,ID, l,t,w,h, WSTL_TXT_3,WSTL_TXT_3,WSTL_TXT_3,WSTL_TXT_3,   \
	libcore_info_keymap,NULL,  \
    	C_ALIGN_LEFT | C_ALIGN_TOP, cnt,  4,4,w-8,h-8,sb,content)

#define LDEF_BAR(root,varScb,page,l,t,w,h)	\
	DEF_SCROLLBAR(varScb, &root, NULL, C_ATTR_ACTIVE, 0, \
		0, 0, 0, 0, 0, l, t, w, h, LIST_BAR_SH_IDX, LIST_BAR_HL_IDX, LIST_BAR_SH_IDX, LIST_BAR_SH_IDX, \
		NULL, NULL, BAR_VERT_AUTO | SBAR_STYLE_RECT_STYLE, page, LIST_BAR_MID_THUMB_IDX, LIST_BAR_MID_RECT_IDX, \
		0, 10, w, h - 20, 100, 1)

TEXT_CONTENT libcore_content[] =
{
	{ STRING_UNICODE, 0},
};

/*rectangle parameters & style define for g_win_stb_info*/
#define     WIN_BG_IDX   	WSTL_SEARCH_BG_CON
#define     WIN_BG_L     	(TV_OFFSET_L+0)
#define     WIN_BG_T     	(TV_OFFSET_T+90)
#define     WIN_BG_W     	720
#define     WIN_BG_H     	400

/*rectangle parameters & style define for stb_info_menu*/
#define     CON_MENU_IDX   	WSTL_WIN_2
#define	    CON_MENU_L      (TV_OFFSET_L+66)
#define	    CON_MENU_T      (TV_OFFSET_T+102)
#define	    CON_MENU_W		604
#define	    CON_MENU_H      370

/*rectangle parameters & style define for TEXTS*/
#define		TXT_ITEM_IDX		WSTL_TXT_3

#define 	TXT_NEME_L			(CON_MENU_L+10)
#define 	TXT_NEME_W			160		
#define 	TXT_VALUE_L			(CON_MENU_L+TXT_NEME_W)	
#define 	TXT_VALUE_W			320
#define		TXT_VERTICAL_GAP	8
#define		TXT_HORIZONTAL_GAP	250

#define 	TXT_T				(CON_MENU_T+10)
#define 	TXT_H				30

#define     MUTIEXT_HIGH        60

/*******************************************************************************
*	objects define
*******************************************************************************/
LDEF_WIN(g_win_stb_info,NULL,NULL,WIN_BG_L,WIN_BG_T,WIN_BG_W,WIN_BG_H,&stb_info_menu,1)

LDEF_CON(stb_info_menu,&g_win_stb_info,NULL,NULL,CON_MENU_L,CON_MENU_T,CON_MENU_W,CON_MENU_H)

LDEF_TEXT(&stb_info_menu,txt_item_name,NULL,TXT_NEME_L,TXT_T,TXT_NEME_W+100,TXT_H,TXT_ITEM_IDX,0,display_strs[40])

LDEF_MTXT(NULL,libcore_info, NULL, 6,\
	TXT_NEME_L, TXT_T+7*(TXT_H + TXT_VERTICAL_GAP),CON_MENU_W-5, MUTIEXT_HIGH, 1, NULL, libcore_content)

//LDEF_BAR(mail_content_str, libcore_scb, 5, SCB_L, SCB_T, SCB_W, SCB_H )



struct help_item_resource win_SI_help[] =
{
   	 	{0,RS_MENU,RS_HELP_BACK},
    	{0,RS_HELP_EXIT,RS_HELP_EXIT},

};

UINT16 items_name_string_id[] =
{
		RS_SERIAL_NUM,
		RS_FACTARY_ID,
		RS_INFO_HW,
		RS_INFO_SW,
		RS_LOADER_VER,
		RS_DOWNLOAD_DATA,
};

UINT16 *items_value_str[] =
{
	display_strs[41],  //serial num
	display_strs[42],  //factory id
	display_strs[43],  //hardware version
	display_strs[44],  //software version
	display_strs[45],  //loader software version
	display_strs[46],  //download data
};

#define ITERM_COUNT sizeof(items_value_str)/sizeof(UINT32)
/*******************************************************************************
 *	HELPER FUNCTIONS DECLARATION
 *******************************************************************************/
static PRESULT stb_info_message_proc(UINT32 msg_type, UINT32 msg_code);
static void win_stb_info_load_data();
static void win_draw_text_content(OSD_RECT *pRect, char *str, UINT16 *unistr, UINT32 strtype, UINT8 shidx);

/*******************************************************************************
 *	GENERAL FUNCTIONS DEFINE
 *******************************************************************************/
static VACTION win_stb_info_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

	switch (key)
	{
		case V_KEY_MENU:
			act = VACT_CLOSE;
			break;
		case V_KEY_EXIT:
			BackToFullScrPlay();
			break;
		default:
			break;
	}
	return act;
}

static PRESULT win_stb_info_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT psult = PROC_PASS;

	switch (event)
	{
		case EVN_PRE_OPEN:
			wincom_open_title_ext(RS_STB_INFO, IM_TITLE_ICON_SYSTEM);
			wincom_open_help(win_SI_help, 2);
			win_clear_trash(display_strs[40],ITERM_COUNT);
			break;
		case EVN_POST_OPEN:
			win_stb_info_load_data();
			break;
		case EVN_PRE_CLOSE:
			*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
			break;
		case EVN_POST_CLOSE:
			//win_clear_trash(display_strs[40],7);
			break;
		case EVN_MSG_GOT:
			psult = stb_info_message_proc( param1, param2 );
			break;
		default:
			break;
	}
	return psult;
}


/*******************************************************************************
 *	HELPER FUNCTIONS DEFINE
 *******************************************************************************/

static PRESULT stb_info_message_proc(UINT32 msg_type, UINT32 msg_code)
{
	PRESULT ret = PROC_LOOP;
	switch (msg_type)
	{
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



static VACTION libcore_info_keymap ( POBJECT_HEAD pObj, UINT32 key )
{
	VACTION act = VACT_PASS;

	switch ( key )
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
		default:
			act = VACT_PASS;
	}

	return act;
}


static void win_stb_info_load_data()
{
	UINT8 styleidx;
	UINT32 ret;
	char str[100]={0};
	UINT16 uni_str[100]; 
	OSD_RECT rect;
	CHUNK_HEADER blk_header;
	UINT8 i;
	UINT32 id, mask;
	BYTE info_block_number;
	STB_INFO_DATA *stb_data;
	HW_SW_INFO hwswInfo;
	
	api_hwsw_Info_get(&hwswInfo);

	//load serail number
#if 0
	UINT32 stb_id = 0;
	UINT16 platform_id = 0;

	CDSTBCA_GetSTBID(&platform_id, &stb_id);
	sprintf((UINT8 *)items_value_str[0], "%.4x%.8x",platform_id,stb_id);
#else
	api_sn_get(str); // serial number
	sprintf((UINT8 *)items_value_str[0], "%s", str);
#endif

	//load hw_ver & sw_ver
	api_hwsw_Info_get(&hwswInfo);
	sprintf((UINT8 *)items_value_str[1], "0x%.6x", hwswInfo.oui);	// factory id);
	sprintf((UINT8 *)items_value_str[2], "0x%.4x", hwswInfo.hw_version);	// hw version
	sprintf((UINT8 *)items_value_str[3], "0x%.4x", hwswInfo.sw_version);	// sw version

	//load loader_ver
	sto_get_data(g_sto_dev, str, 0+CHUNK_VERSION, 0x4); 
	str[4] = 0;
	sprintf((UINT8 *)items_value_str[4], "0x%s", str);	// loader version
	items_value_str[4][8] = 0;
			
	//load download date
	stb_data = stb_info_data_get();
	sprintf((UINT8 *)items_value_str[5], "%s", stb_data->download_date);  //download data
	
	styleidx = TXT_ITEM_IDX;
	rect.uWidth = TXT_NEME_W;
	rect.uHeight = TXT_H;
	id = 0;
	for (id = 0; id < ITERM_COUNT; id++)
	{
		rect.uLeft = TXT_NEME_L;
		rect.uTop = TXT_T + id *(TXT_H + TXT_VERTICAL_GAP);
		win_draw_text_content(&rect, NULL, NULL, (UINT32)items_name_string_id[id], styleidx);
		rect.uWidth = TXT_VALUE_W;
		rect.uLeft += TXT_HORIZONTAL_GAP;
		win_draw_text_content(&rect, (char*)items_value_str[id], NULL, 0, styleidx);
	}

	/* show libcore info */
	memset(str, 0, sizeof(str));
	memcpy(str, libcore_banner, strlen(libcore_banner)-2);
#ifdef SFU_AUTO_TEST
	strcat(str, "[SFU]");
#endif
	ComAscStr2Uni(str,uni_str);
	libcore_content->text.pString = uni_str;
	OSD_DrawObject((POBJECT_HEAD)&libcore_info,C_UPDATE_ALL);
    //OSD_UpdateVscr(apVscr);
	return ;

}


static void win_draw_text_content(OSD_RECT *pRect, char *str, UINT16 *unistr, UINT32 strtype, UINT8 shidx)
{
	TEXT_FIELD *txt;

	txt = &txt_item_name;
	OSD_SetRect2(&txt->head.frame, pRect);
	if (NULL == str && NULL == unistr)
	{
		txt->pString = NULL;
		OSD_SetTextFieldContent(txt, STRING_ID, (UINT32)strtype);
	}
	else if (str != NULL)
	{
		txt->pString = display_strs[40];
		OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)str);
	}
	else if (unistr != NULL)
	{
		txt->pString = display_strs[40];
		OSD_SetTextFieldContent(txt, STRING_UNICODE, (UINT32)unistr);
	}
	OSD_SetColor(txt, shidx, shidx, shidx, shidx);
	OSD_DrawObject((POBJECT_HEAD)txt, C_UPDATE_ALL);
}


