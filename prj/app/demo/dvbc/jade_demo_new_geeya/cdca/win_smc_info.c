#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <api/libchunk/chunk.h>

#include <api/libcas/gy/gyca_def.h>
#include <api/libcas/gy/gyca_interface.h>
#include "win_cas_com.h"
#include "win_smc_info.h"

/*rectangle parameters & style define for g_win_smc_info*/
#define     WIN_BG_IDX   	WSTL_EMIAL_OL_CON
#define     WIN_BG_L     	(TV_OFFSET_L+286) //0
#define     WIN_BG_T     	(TV_OFFSET_T+99)//
#define     WIN_BG_W     	394//720
#define     WIN_BG_H     	348//400

/*rectangle parameters & style define for smc_info_menu*/
#define     CON_MENU_IDX   	WSTL_WIN_SIGNAL_01
#define	    CON_MENU_L      (TV_OFFSET_L+291) //66
#define	    CON_MENU_T      (TV_OFFSET_T+137) //102
#define	    CON_MENU_W		384//604
#define	    CON_MENU_H      304//380

/*rectangle parameters & style define for TEXTS*/
#define		TXT_ITEM_IDX		WSTL_TXT_5

#define 	TXT_NEME_L				296//(CON_MENU_L+10) //76
#define 	TXT_NEME_W			372//260//300		
#define 	TXT_VALUE_L			475//(CON_MENU_L+TXT_NEME_W)	//246
#define 	TXT_VALUE_W			195//340//300	
#define	    TXT_VERTICAL_GAP		6

#define 	TXT_T				(CON_MENU_T+10)
#define 	TXT_H				24//28

static UINT16 smc_items_name_string_id[] =
{
	RS_TF3_PAIRINGSTATUS,
    	RS_SMC_CARD1,
    	RS_SMC_CARD2,
    	RS_SMC_CARD3,
    	RS_SMC_CARD4,
    	RS_SMC_CARD5,
	RS_CASINFO_TFCASVERSION,
	RS_MATURITY_RATING,
	RS_CA_WORKTIME_SETTING,
};

static UINT16 *smc_items_value_str[] =
{
	NULL,
 	display_strs[42],  //smc no.,      2
 	display_strs[43],  //account balance,      3
 	display_strs[44],  //Iissue date,    4
 	display_strs[45],  //expire date	5
 	display_strs[46],  //user type	6
 	display_strs[47],  //pairing status, 7
 	display_strs[48],  //ca card type, 8
 	display_strs[49],  //watch level,   9
    	display_strs[59],
};
TEXT_FIELD *smcinfo_list[]=
{
	NULL,
	&smcinfo_txt9,	
	&smcinfo_txt10,	
	&smcinfo_txt11,
	&smcinfo_txt12,
	&smcinfo_txt13,
	&smcinfo_txt14,
	&smcinfo_txt15,
	&smcinfo_txt16,
};
static OSD_RECT rc_txt[] =
{
    {TXT_NEME_L,TXT_T+0*(TXT_H+TXT_VERTICAL_GAP)+1*TXT_H,TXT_NEME_W,TXT_H},
			{TXT_VALUE_L,TXT_T + 0 *(TXT_H*2 + TXT_VERTICAL_GAP)+1*TXT_H,TXT_VALUE_W,TXT_H},
    {TXT_NEME_L,TXT_T+1*(TXT_H+TXT_VERTICAL_GAP)+1*TXT_H,TXT_NEME_W,TXT_H},
    			{TXT_VALUE_L,TXT_T + 1*(TXT_H+TXT_VERTICAL_GAP)+1*TXT_H,TXT_VALUE_W,TXT_H},
     {TXT_NEME_L,TXT_T+2*(TXT_H+TXT_VERTICAL_GAP)+1*TXT_H,TXT_NEME_W,TXT_H},
    			{TXT_VALUE_L,TXT_T + 2*(TXT_H+TXT_VERTICAL_GAP)+1*TXT_H,TXT_VALUE_W,TXT_H},
    {TXT_NEME_L,TXT_T+3*(TXT_H+TXT_VERTICAL_GAP)+1*TXT_H,TXT_NEME_W,TXT_H},
			{TXT_VALUE_L,TXT_T + 3 *(TXT_H + TXT_VERTICAL_GAP)+1*TXT_H,TXT_VALUE_W,TXT_H},
    {TXT_NEME_L,TXT_T+4*(TXT_H+TXT_VERTICAL_GAP)+1*TXT_H,TXT_NEME_W,TXT_H},
			{TXT_VALUE_L,TXT_T + 4 *(TXT_H + TXT_VERTICAL_GAP)+1*TXT_H,TXT_VALUE_W,TXT_H},
    {TXT_NEME_L,TXT_T+5*(TXT_H+TXT_VERTICAL_GAP)+1*TXT_H,TXT_NEME_W,TXT_H},
			{TXT_VALUE_L,TXT_T + 5 *(TXT_H + TXT_VERTICAL_GAP)+1*TXT_H,TXT_VALUE_W,TXT_H},
    {TXT_NEME_L,TXT_T+6*(TXT_H+TXT_VERTICAL_GAP)+1*TXT_H,TXT_NEME_W,TXT_H},
			{TXT_VALUE_L,TXT_T + 6 *(TXT_H + TXT_VERTICAL_GAP)+1*TXT_H,TXT_VALUE_W,TXT_H},
    {TXT_NEME_L,TXT_T+7*(TXT_H+TXT_VERTICAL_GAP)+1*TXT_H,TXT_NEME_W,TXT_H},
			{TXT_VALUE_L,TXT_T + 7 *(TXT_H + TXT_VERTICAL_GAP)+1*TXT_H,TXT_VALUE_W,TXT_H},
    {TXT_NEME_L,TXT_T+8*(TXT_H+TXT_VERTICAL_GAP)+1*TXT_H,TXT_NEME_W,TXT_H},
			{TXT_VALUE_L,TXT_T + 8 *(TXT_H + TXT_VERTICAL_GAP)+1*TXT_H,TXT_VALUE_W,TXT_H},
			
};

/*******************************************************************************
 *	HELPER FUNCTIONS DECLARATION
 *******************************************************************************/
static void win_smc_info_load_data();
static void win_draw_text_content(OSD_RECT *pRect, char *str, UINT16 *unistr, UINT32 strtype, UINT8 shidx);
static void win_stb_clear_trash();
static UINT8 win_smc_check();

/*******************************************************************************
 *	GENERAL FUNCTIONS DEFINE
 *******************************************************************************/
static VACTION win_smc_info_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

	switch (key)
	{
		case V_KEY_MENU:
			act = VACT_CLOSE;
			break;
		case V_KEY_EXIT:
			act = VACT_CLOSE;
			break;
		default:
			break;
	}
	return act;
}

static PRESULT win_smc_info_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT psult = PROC_PASS;
    	UINT8 rating;
    	UINT16 rating_ret;
		
	MULTI_TEXT *SMC_version_no;
	SMC_version_no = 	&smcinfo_multxt0;
	UINT8 *aVersion=NULL;
	UINT16 uni_str[100]={0}; 
	
	struct help_item_resource win_smcinfo_help[] =
	{
		{0,RS_MENU,RS_HELP_BACK},
		{0,RS_HELP_EXIT,RS_HELP_EXIT},
	};

	switch (event)
	{
		case EVN_PRE_OPEN:
			 
	            /*rating_ret = CDCASTB_GetRating(&rating);
	    		if ((CDCA_RC_CARD_INVALID == rating_ret)|| (CDCA_RC_POINTER_INVALID ==rating_ret))
	    		{
	    			win_popup_msg(NULL, NULL, RS_CAERR_NOSMC);
	    			    return PROC_LEAVE;
	    		}
	    		else if (GYCA_OK != rating_ret)
	    		{
	    			win_popup_msg(NULL, NULL, RS_CAERR_UNKNOWN);
	    			    return PROC_LEAVE;
	    		}*/
			//wincom_open_title_ext(RS_SMC_INFO, IM_TITLE_ICON_SYSTEM);
			//wincom_open_help(win_smcinfo_help, 2);
			if(win_smc_check()==0)
			{
				cas_popup_msg(NULL, NULL, RS_CAERR_NOINFO);
				psult = PROC_LEAVE;	
			}
			win_stb_clear_trash();
			break;
		case EVN_PRE_DRAW:
			win_smc_info_load_data();
			break;
		case EVN_POST_OPEN:
			GYCA_GetVersionInfo(&aVersion);
			ComAscStr2Uni(aVersion,uni_str);
			smcinfo_multxt0_text_content[0].text.pString = uni_str;
			OSD_SetMultiTextContent(SMC_version_no, (PTEXT_CONTENT)&smcinfo_multxt0_text_content);
			OSD_DrawObject((POBJECT_HEAD)SMC_version_no, C_UPDATE_ALL);
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

#if 0
/*******************************************************************************
 *	HELPER FUNCTIONS DEFINE
 *******************************************************************************/

static void win_smc_info_load_data()
{
	UINT8 styleidx, num, tmp[50],str[50];
	UINT16 ret, id, strid;
	UINT32 ver;
	OSD_RECT rect;
	CHUNK_HEADER blk_header;

	ret = CDCASTB_IsPaired(&num, tmp);
	if (GYCA_OK == ret)
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
	sprintf(str, "0X%08X", ver);
    OSD_SetTextFieldContent(&smc_cas_ver_val,STRING_ANSI,str);
	ret = CDCASTB_GetRating(&num);
	if (GYCA_OK == ret)
	{
		sprintf(str, "%d", num);
	}
	else
	{
		sprintf(str, "%d", 0);
	}

	ret = CDCASTB_GetWorkTime(&tmp[0],&tmp[1],&tmp[2],&tmp[3],&tmp[4],&tmp[5]);
	if (GYCA_OK == ret)
	{
		sprintf(str, "%02d:%02d:%02d--%02d:%02d:%02d",tmp[0],tmp[1],tmp[2],tmp[3],tmp[4],tmp[5]);
	}
	else
	{
		sprintf(str, "00:00:00--00:00:00");
	}

	ret = CDCASTB_GetCardSN(tmp);
	if (GYCA_OK == ret)
	{
		sprintf(str, "%s",tmp);
	}
	else
	{
		sprintf(str, " ");
	}	
    OSD_SetTextFieldContent(&smc_smc_num_val,STRING_ANSI,str);
}
#endif

/*******************************************************************************
 *	HELPER FUNCTIONS DEFINE
 *******************************************************************************/

static void win_smc_info_load_data()
{
	UINT32 win_smc_strid,strid;
	UINT8 styleidx, num, tmp[50],save;
	UINT16  id;
	UINT32 MJD_Issue_Year,MJD_Expire_Year;
	INT32 ret;
	UINT8 asc_str[100]={0};
	UINT8 pbCardID[8];
	
	UINT8 pbIssueDate[50]={0};
	UINT8 pbExpireDate[50]={0};
	
	UINT8 pbIssueDateEx[50]={0};
	UINT8 pbExpireDateEx[50]={0};
	
	CHUNK_HEADER blk_header;
	SYSTEM_DATA* pSysData;
	INT32 wdBalance = 0;
	T_CA_IPPVBurseInfo *burseinfo = NULL;
	
	win_compopup_init(WIN_POPUP_TYPE_SMSG);
	win_compopup_set_msg_ext(NULL, NULL, RS_CAERR_COLLECTINFO);
	win_compopup_set_frame(290,260, 200, 50);
	win_compopup_open_ext(&save);
	
	/*版本信息,smc卡ID号*/
	MEMSET(pbCardID,0,sizeof(pbCardID));	
	ret  = GYCA_GetSMCID(pbCardID);
	if (GYCA_OK == ret)
	{
		sprintf(asc_str,"%02x%02x%02x%02x%02x%02x%02x%02x",pbCardID[0],pbCardID[1],pbCardID[2],pbCardID[3],pbCardID[4],pbCardID[5],pbCardID[6],pbCardID[7]);
	}
	else
	{
		sprintf(asc_str,"%c",NULL);
	}
	ComAscStr2Uni(asc_str, smc_items_value_str[1]);
	OSD_SetTextFieldContent(smcinfo_list[1], STRING_ID, (UINT32)smc_items_value_str[1]);
	
	/*账户金额*/
	burseinfo = (T_CA_IPPVBurseInfo *)MALLOC(sizeof(T_CA_IPPVBurseInfo));
	if(burseinfo != NULL)
	{
		ret = GYCA_IPPVGetBurseInfo(0,0,&burseinfo);
		if(GYCA_OK == ret)
		{
			wdBalance = burseinfo->dwBalanceToken;
		}
		else
			wdBalance = 0;
		FREE(burseinfo);
		burseinfo = NULL;
	}
	if(wdBalance >= 0)
	{
		sprintf(asc_str, "%d.%02d", wdBalance/100, wdBalance%100);
	}	
	else
	{
		wdBalance = -wdBalance;
		sprintf(asc_str, "-%d.%02d", wdBalance/100, wdBalance%100);
	}
	ComAscStr2Uni(asc_str, smc_items_value_str[2]);
	OSD_SetTextFieldContent(smcinfo_list[2], STRING_ID, (UINT32)smc_items_value_str[2]);
	
   	/*智能卡发行日期、过期日期*/
	
	ret = GYCA_GetSMCIssueInfo(pbIssueDate,pbExpireDate);
	MJD_Issue_Year= (( ((UINT16)pbIssueDate[0]*256 + (UINT16)pbIssueDate[1]) - 15078 )/365)+1900;//MJD算法计算发行年份since 1900
	MJD_Expire_Year= (( ((UINT16)pbExpireDate[0]*256 + (UINT16)pbExpireDate[1]) - 15078 )/365)+1900;//MJD算法计算过期年份since 1900

	ret = GYCA_GetSMCIssueInfoEx(pbIssueDateEx,pbExpireDateEx);
	
	if (GYCA_OK == ret)
	{ 
		//sprintf(asc_str, "%d%d-%d-%d",pbIssueDateEx[0],pbIssueDateEx[1],pbIssueDateEx[2],pbIssueDateEx[3]);
		sprintf(asc_str, "%02d%02d-%02d-%02d",(UINT8)(MJD_Issue_Year/100),(UINT8)(MJD_Issue_Year%100),pbIssueDateEx[2],pbIssueDateEx[3]);
		ComAscStr2Uni(asc_str, smc_items_value_str[3]);
		OSD_SetTextFieldContent(smcinfo_list[3], STRING_ID, (UINT32)smc_items_value_str[3]);
		
		//sprintf(asc_str, "%d%d-%d-%d",pbExpireDateEx[0],pbExpireDateEx[1],pbExpireDateEx[2],pbExpireDateEx[3]);
		sprintf(asc_str, "%02d%02d-%02d-%02d",(UINT8)(MJD_Expire_Year/100),(UINT8)(MJD_Expire_Year%100),pbExpireDateEx[2],pbExpireDateEx[3]);
		ComAscStr2Uni(asc_str, smc_items_value_str[4]);
		OSD_SetTextFieldContent(smcinfo_list[4], STRING_ID, (UINT32)smc_items_value_str[4]);
	}
	else
	{
		sprintf(asc_str, "0000-00-00");
		ComAscStr2Uni(asc_str, smc_items_value_str[3]);
		OSD_SetTextFieldContent(smcinfo_list[3], STRING_ID, (UINT32)smc_items_value_str[3]);
		
		sprintf(asc_str, "0000-00-00");
		ComAscStr2Uni(asc_str, smc_items_value_str[4]);
		OSD_SetTextFieldContent(smcinfo_list[4], STRING_ID, (UINT32)smc_items_value_str[4]);
	}
	
	/*用户类型(测试用户、普通用户、员工用户...)*/
	ret = GYCA_GetSMCUserType();
	if(ret >= 0)
	{
		switch(ret)
		{
			case 1:
				OSD_SetTextFieldContent(smcinfo_list[5], STRING_ID, RS_USER_BETA);
				break;
			case 2:
				break;
			case 3:
				;//
				break;
			default:
				break;
		}
	}
	else
	{
		;//获取用户类型信息失败
	}
	
	/*机卡配对信息*/
	ret = GYCA_GetPaired();
	if (GYCA_OK== ret)
	{
		strid = RS_CAERR_PAIRED_CURRENT;
	}
	else if (GYCA_CARD_NOPAIR == ret)
	{
		strid = RS_CAERR_NOT_PAIRED;
	}
	else if (GYCA_CARD_MISPAIR == ret)
	{
		strid = RS_CAERR_PAIRED_OTHERS;
	}
	else
	{
		strid = RS_CAERR_NOSMC;
        	MEMSET(tmp,0,50);
	}
	OSD_SetTextFieldContent(smcinfo_list[6], STRING_ID, strid);
	
	/*CA卡类型(字母卡)*/
	id = GYCA_IsSonCard();
	win_smc_strid = (id==1)?RS_SCINFO_CARDCHILD:RS_SCINFO_PARENTCARD;
	OSD_SetTextFieldContent(smcinfo_list[7], STRING_ID, win_smc_strid);
	
	/*观看级别*/
	ret = GYCA_GetParentControl(&num);
	if (GYCA_OK == ret)
	{
		sprintf(asc_str, "%d", num);
	}
	else
	{
		sprintf(asc_str, "%d", 0);
	}
	ComAscStr2Uni(asc_str, smc_items_value_str[8]);
	OSD_SetTextFieldContent(smcinfo_list[8], STRING_ID, (UINT32)smc_items_value_str[8]);
	
	//OSD_TrackObject((POBJECT_HEAD )&smc_info_menu, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
	/*
	styleidx = TXT_ITEM_IDX;
	id = 0;
	for (id = 0; id < 9; id++)
	{
		win_draw_text_content(&rc_txt[id*2], NULL, NULL, (UINT32)smc_items_name_string_id[id], styleidx);

		if (0 == id)
			win_draw_text_content(&rc_txt[id*2+1], NULL, NULL, strid, styleidx);
		else
			win_draw_text_content(&rc_txt[id*2+1], (char*)smc_items_value_str[id], NULL, 0, styleidx);
	}
	*/
}

/*
static void win_draw_text_content(OSD_RECT *pRect, char *str, UINT16 *unistr, UINT32 strtype, UINT8 shidx)
{
	TEXT_FIELD *txt;

	txt = &smc_item_str;
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
*/

static void win_stb_clear_trash()
{
	/*clear the buffer from  display_strs[40] to display_strs[46]*/
	MEMSET(display_strs[40], 0, MAX_DISP_STR_LEN *8 * 2);
}

static UINT8 win_smc_check()
{
	INT32 ret = 0;
	UINT8 pbCardID[8];
	ret  = GYCA_GetSMCID(pbCardID);
	if (GYCA_OK == ret)
		return 1;
	else
		return 0;
}

