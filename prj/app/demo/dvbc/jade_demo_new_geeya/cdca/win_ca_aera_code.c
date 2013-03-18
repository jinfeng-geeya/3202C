#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
//#include <api/libosd/osd_lib.h>
#include <api/libchunk/chunk.h>

#include "win_cas_com.h"

#include "win_ca_aera_code.h"

/*******************************************************************************
*	Objects definition
*******************************************************************************/

/******************************************************************************
 *	HELPER FUNCTIONS DECLARATION
 *******************************************************************************/

extern UINT16 current_operator_ID;
#if 0
#define	    CON_MENU_L      	450
#define	    CON_MENU_T      	125
#define	    CON_MENU_W		797
#define	    CON_MENU_H      	480

#define		TXT_ITEM_IDX		WSTL_TXT_4

#define 	TXT_NAME_L				(CON_MENU_L+20)
#define 	TXT_NAME_W			170		
#define 	TXT_VALUE_L			(TXT_NAME_L+TXT_NAME_W)	
#define 	TXT_VALUE_W			(CON_MENU_L+CON_MENU_W-TXT_VALUE_L)	
#define	TXT_VERTICAL_GAP		10

#define 	TXT_T				(CON_MENU_T+40)
#define 	TXT_H				40
#endif
static void win_eigenvalue_info_load_data();
static void win_draw_text_content(UINT8 i_area, char *str, UINT16 *unistr, UINT32 strtype);
static void win_stb_clear_trash();

/*******************************************************************************
 *	GENERAL FUNCTIONS DEFINE
 *******************************************************************************/

static UINT16 *eigenvalue_items_value_str[] =
{
	display_strs[42],  //AreaCode
	display_strs[43],  //BouquetID
	display_strs[44],  //eigenvalue 1~3
	display_strs[45],  //eigenvalue 4~6
};


static VACTION win_eigenvalue_info_keymap(POBJECT_HEAD pObj, UINT32 key)
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

static PRESULT win_eigenvalue_info_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT psult = PROC_PASS;
	struct help_item_resource win_ac_help[] =
	{
    		{0,RS_MENU,RS_HELP_BACK},
    		{0,RS_HELP_EXIT,RS_FULL_PLAY},
	};

	switch (event)
	{
		case EVN_PRE_OPEN:
			wincom_open_title_ext(RS_SCINFO_AREACODE, IM_TITLE_ICON_SYSTEM);
			wincom_open_help(win_ac_help, 2);		
			//win_eigenvalue_info_load_data();
			break;
		case EVN_POST_OPEN:
			win_eigenvalue_info_load_data();
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

static void win_eigenvalue_info_load_data()
{
	UINT8  num, tmp[60]={0};
	UINT16 ret, id, strid;
	UINT32 ver;

	CHUNK_HEADER blk_header;
	CDCA_U32 ACList[CDCA_MAXNUM_ACLIST] = {0};
	MEMSET(display_strs[40], 0, MAX_DISP_STR_LEN *7 * 2);
	
	OSD_DrawObject((POBJECT_HEAD )&eigenvalue_info_menu, C_UPDATE_ALL);
	ver = CDCASTB_GetACList(current_operator_ID, ACList);
	 //AreaCode
	sprintf((UINT8 *)eigenvalue_items_value_str[0], "0x%08X", ACList[0]);

	 //BouquetID
	sprintf((UINT8 *)eigenvalue_items_value_str[1], "0x%08X", ACList[1]);

	//eigenvalue: ACList[4~9], (ACList[2~3] reserved)
	sprintf(tmp, "0x%08X", ACList[4]); 
	for(num=5; num<10; num++)
	{
		sprintf((char *)&tmp[10+(num-5)*9], " %08X", ACList[num]); 
	}

	//show string in two lines
	MEMCPY(eigenvalue_items_value_str[2], tmp, 28); 
	MEMCPY(eigenvalue_items_value_str[3], &tmp[28], 28); 
	

	for (id = 0; id < 4; id++)
	{
		win_draw_text_content((UINT8)id, (char*)eigenvalue_items_value_str[id], NULL, 0);
	}
	return ;

}


static void win_draw_text_content(UINT8  i_area, char *str, UINT16 *unistr, UINT32 strtype)
{
	TEXT_FIELD *txt=NULL;

	if(0==i_area)
	{
		txt = &(text_area_code);
	}
	else if(1==i_area)
	{
		txt = &(text_id);
	}
	else if(2==i_area)
	{
		txt = &(text_user_feature);
	}
	else if(3==i_area)
	{
		txt = &(text_user_feature2);
	}
	
	
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
	OSD_DrawObject((POBJECT_HEAD)txt, C_UPDATE_ALL);
}

static void win_stb_clear_trash()
{
	/*clear the buffer from  display_strs[40] to display_strs[46]*/
	MEMSET(display_strs[40], 0, MAX_DISP_STR_LEN *7 * 2);
}






