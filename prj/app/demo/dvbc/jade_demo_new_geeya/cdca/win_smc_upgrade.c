#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>

//#include <api/libosd/osd_lib.h>
#include <api/libchunk/chunk.h>


//include the header from xform 
#if (CAS_TYPE == CAS_CDCA)
#include "win_cas_com.h"
#include "win_smc_upgrade_info.h"


/*rectangle parameters & style define for g_win_stb_info*/
#define     WIN_BG_L     0
#define     WIN_BG_T     90
#define     WIN_BG_W     720
#define     WIN_BG_H     400

/*rectangle parameters & style define for stb_info_menu*/
#define	    CON_MENU_L      66
#define	    CON_MENU_T      111
#define	    CON_MENU_W		593
#define	    CON_MENU_H      355

/*rectangle parameters & style define for TEXTS*/
#define		TXT_ITEM_IDX		WSTL_TXT_3

#define 	TXT_NEME_L			(CON_MENU_L+10)
#define 	TXT_NEME_W			180
#define 	TXT_VALUE_L			(CON_MENU_L+50)//(CON_MENU_L+TXT_NEME_W)	
#define 	TXT_VALUE_W			300
#define 	TXT_VALUE_L2			(TXT_NEME_L+TXT_NEME_W)//(CON_MENU_L+TXT_NEME_W)	
#define 	TXT_VALUE_W2			(CON_MENU_L+CON_MENU_W-TXT_VALUE_L2)
#define	TXT_VERTICAL_GAP	4
#define	TXT_HORIZONTAL_GAP	(120)

#define 	TXT_T				(CON_MENU_T+10)
#define 	TXT_H				30

/*********rectangle parameters & style define for preview********/
#define PREVIEW_IDX  WSTL_SUBMENU_PREVIEW

#define PREVIEW_L (857)
#define PREVIEW_T (125)
#define PREVIEW_W (390)
#define PREVIEW_H (240)

#define PREVIEW_INNER_L  (872)
#define PREVIEW_INNER_T  (144)
#define PREVIEW_INNER_W  (360)
#define PREVIEW_INNER_H  (202)

static UINT16 smc_items_name_string_id[] =
{
	/*RS_TF3_PAIRINGSTATUS,
	RS_TF3_CASINFO_TFCASVERSION,
	RS_TF3_MATURITY_RATING,
	RS_CA_WORKTIME_SETTING,*/
	RS_SMCNO,
	RS_TF3_UPGRADE_DATA,
	RS_TF3_UPGRADE_STATUS,
	RS_TF3_SMC_UPGRADE_NO,
};

static UINT16 *smc_items_value_str[] =
{
	display_strs[42],  //pairing status
	display_strs[43],  //cas version
	display_strs[44],  //watch level
	display_strs[45],  //work time
	display_strs[46],  //smc no.
	display_strs[47],  //Update date
	display_strs[48],  //Update status
	display_strs[49],  //Update SMC No.
};

static OSD_RECT rc_txt[] =
{
   /* {TXT_NEME_L,TXT_T+0*(TXT_H+TXT_VERTICAL_GAP)+0*TXT_H,TXT_NEME_W,TXT_H},
			{TXT_VALUE_L,TXT_T + 0 *(TXT_H*2 + TXT_VERTICAL_GAP)+TXT_H,TXT_VALUE_W,TXT_H},
    {TXT_NEME_L,TXT_T+1*(TXT_H+TXT_VERTICAL_GAP)+1*TXT_H,TXT_NEME_W,TXT_H},
    			{TXT_VALUE_L2+50,TXT_T + 1*(TXT_H+TXT_VERTICAL_GAP)+1*TXT_H,TXT_VALUE_W2,TXT_H},
     {TXT_NEME_L,TXT_T+2*(TXT_H+TXT_VERTICAL_GAP)+1*TXT_H,TXT_NEME_W,TXT_H},
    			{TXT_VALUE_L2,TXT_T + 2*(TXT_H+TXT_VERTICAL_GAP)+1*TXT_H,TXT_VALUE_W2,TXT_H},
    {TXT_NEME_L,TXT_T+3*(TXT_H+TXT_VERTICAL_GAP)+1*TXT_H,TXT_NEME_W,TXT_H},
			{TXT_VALUE_L+100,TXT_T + 3 *(TXT_H + TXT_VERTICAL_GAP)+2*TXT_H,TXT_VALUE_W,TXT_H},
  */
    {TXT_NEME_L,TXT_T+0*(TXT_H+TXT_VERTICAL_GAP)+0*TXT_H,TXT_NEME_W,TXT_H},
			{TXT_VALUE_L2,TXT_T + 0 *(TXT_H + TXT_VERTICAL_GAP)+0*TXT_H,TXT_VALUE_W,TXT_H},
    {TXT_NEME_L,TXT_T+1*(TXT_H+TXT_VERTICAL_GAP)+0*TXT_H,TXT_NEME_W,TXT_H},
			{TXT_VALUE_L2,TXT_T + 1 *(TXT_H + TXT_VERTICAL_GAP)+0*TXT_H,TXT_VALUE_W,TXT_H},
    {TXT_NEME_L,TXT_T+2*(TXT_H+TXT_VERTICAL_GAP)+0*TXT_H,TXT_NEME_W,TXT_H},
			{TXT_VALUE_L2,TXT_T + 2 *(TXT_H + TXT_VERTICAL_GAP)+0*TXT_H,TXT_VALUE_W,TXT_H},
    {TXT_NEME_L,TXT_T+3*(TXT_H+TXT_VERTICAL_GAP)+0*TXT_H,TXT_NEME_W,TXT_H},
			{TXT_VALUE_L2,TXT_T + 3 *(TXT_H + TXT_VERTICAL_GAP)+0*TXT_H,TXT_VALUE_W,TXT_H},
			
};

/*******************************************************************************
 *	HELPER FUNCTIONS DECLARATION
 *******************************************************************************/
static void win_smc_upgrade_info_load_data();
static void win_draw_text_content(OSD_RECT *pRect, char *str, UINT16 *unistr, UINT32 strtype, UINT8 shidx);
static void win_stb_clear_trash();

/*******************************************************************************
*	Callback and Keymap Functions
*******************************************************************************/
static VACTION win_smc_up_info_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

	switch (key)
	{
		case V_KEY_EXIT:
		case V_KEY_MENU:
			act = VACT_CLOSE;
			break;
		default:
			break;
	}
	return act;

}
static PRESULT win_smc_up_info_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT psult = PROC_PASS;
	OSD_RECT rc_con;
	OSD_RECT rc_preview;
	static UINT8 preview_flag=0;
	struct help_item_resource win_smc_upgrade_help[] =
	{
		{0,RS_MENU,RS_HELP_BACK},
		{0,RS_HELP_EXIT,RS_HELP_EXIT},
	};


	switch (event)
	{
		case EVN_PRE_OPEN:
			wincom_open_title_ext(RS_SMC_UPGRADE_INFO, IM_TITLE_ICON_SYSTEM);
			wincom_open_help(win_smc_upgrade_help, 2);
	        win_stb_clear_trash();
			break;
		case EVN_POST_OPEN:
			win_smc_upgrade_info_load_data();
			break;
		case EVN_PRE_CLOSE:
			*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
			break;
		case EVN_POST_CLOSE:
			win_stb_clear_trash();
			break;
		default:
			break;
	}
	return psult;
}


/*******************************************************************************
 *	HELPER FUNCTIONS DEFINE
 *******************************************************************************/

static void win_smc_upgrade_info_load_data()
{
	UINT8 styleidx, num, tmp[50];
	UINT16 ret, id, strid;
	UINT32 ver;
	CHUNK_HEADER blk_header;
	SYSTEM_DATA* pSysData;
    OSD_TrackObject((POBJECT_HEAD )&smc_upgrade_item_str, C_UPDATE_ALL);
	/*ret = CDCASTB_IsPaired(&num, tmp);
	if (CDCA_RC_OK== ret)
	{
		strid = RS_CAERR_PAIRED_CURRENT;
	}
	else if (CDCA_RC_CARD_NOPAIR == ret)
	{
		strid = RS_CAERR_NOT_PAIRED;
	}
	else if (CDCA_RC_CARD_PAIROTHER == ret)
	{
		strid = RS_CAERR_PAIRED_OTHERS;
	}
	else
	{
		strid = RS_CAERR_NOSMC;
	}

	ver = CDCASTB_GetVer();
	sprintf((UINT8*)smc_items_value_str[1], "0x%08X", ver);/*cjj 100427 modify. CDCA 3.0(0x%08X)* /

	ret = CDCASTB_GetRating(&num);
	if (CDCA_RC_OK == ret)
	{
		sprintf((UINT8 *)smc_items_value_str[2], "%d", num);
	}
	else
	{
		sprintf((UINT8 *)smc_items_value_str[2], "%d", 0);
	}

	ret = CDCASTB_GetWorkTime(&tmp[0],&tmp[1],&tmp[2],&tmp[3],&tmp[4],&tmp[5]);
	if (CDCA_RC_OK == ret)
	{
		sprintf((UINT8 *)smc_items_value_str[3], "%02d:%02d:%02d--%02d:%02d:%02d",tmp[0],tmp[1],tmp[2],tmp[3],tmp[4],tmp[5]);
	}
	else
	{
		sprintf((UINT8 *)smc_items_value_str[3], "00:00:00--00:00:00");
	}

	ret = CDCASTB_GetCardSN(tmp);
	if (CDCA_RC_OK == ret)
	{
		sprintf((UINT8 *)smc_items_value_str[4], "%s",tmp);
	}
	else
	{
		sprintf((UINT8 *)smc_items_value_str[4], " ");
	}

	pSysData = sys_data_get();
	sprintf((UINT8 *)smc_items_value_str[5], "%04d-%02d-%02d  %02d:%02d", pSysData->smc_update_dt.year,
								pSysData->smc_update_dt.month, pSysData->smc_update_dt.day, pSysData->smc_update_dt.hour, pSysData->smc_update_dt.min);
	if (pSysData->bUpd_status)
	{
		sprintf((UINT8 *)smc_items_value_str[6], "Success");
	}
	else
	{
		sprintf((UINT8 *)smc_items_value_str[6], "Failed");
	}

	MEMCPY((UINT8 *)smc_items_value_str[7], pSysData->upd_smc_no, 17);



	styleidx = TXT_ITEM_IDX;
	id = 0;
	for (id = 0; id < 8; id++)
	{
		win_draw_text_content(&rc_txt[id*2], NULL, NULL, (UINT32)smc_items_name_string_id[id], styleidx);

		if (0 == id)
			win_draw_text_content(&rc_txt[id*2+1], NULL, NULL, strid, styleidx);
		else
			win_draw_text_content(&rc_txt[id*2+1], (char*)smc_items_value_str[id], NULL, 0, styleidx);
	}*/
	
	ret = CDCASTB_GetCardSN(tmp);
	if (CDCA_RC_OK == ret)
	{
		sprintf((UINT8 *)smc_items_value_str[0], "%s",tmp);
	}
	else
	{
		sprintf((UINT8 *)smc_items_value_str[0], " ");
	}

	pSysData = sys_data_get();
	sprintf((UINT8 *)smc_items_value_str[1], "%04d-%02d-%02d  %02d:%02d", pSysData->smc_update_dt.year,
								pSysData->smc_update_dt.month, pSysData->smc_update_dt.day, pSysData->smc_update_dt.hour, pSysData->smc_update_dt.min);
	if (pSysData->bUpd_status)
	{
		sprintf((UINT8 *)smc_items_value_str[2], "Success");
	}
	else
	{
		sprintf((UINT8 *)smc_items_value_str[2], "Failed");
	}

	MEMCPY((UINT8 *)smc_items_value_str[3], pSysData->upd_smc_no, 17);



	styleidx = TXT_ITEM_IDX;
	id = 0;
	for (id = 0; id < 4; id++)
	{
		win_draw_text_content(&rc_txt[id*2], NULL, NULL, (UINT32)smc_items_name_string_id[id], styleidx);

		/*if (0 == id)
			win_draw_text_content(&rc_txt[id*2+1], NULL, NULL, strid, styleidx);
		else*/
			win_draw_text_content(&rc_txt[id*2+1], (char*)smc_items_value_str[id], NULL, 0, styleidx);
	}
	return ;

}


static void win_draw_text_content(OSD_RECT *pRect, char *str, UINT16 *unistr, UINT32 strtype, UINT8 shidx)
{
	TEXT_FIELD *txt;

	txt = &smc_upgrade_item_str;
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

static void win_stb_clear_trash()
{
	/*clear the buffer from  display_strs[40] to display_strs[46]*/
	MEMSET(display_strs[40], 0, MAX_DISP_STR_LEN *7 * 2);
}
#endif

