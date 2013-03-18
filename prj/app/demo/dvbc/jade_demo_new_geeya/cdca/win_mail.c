#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>

#include <api/libcas/gy/cas_gy.h>
#include <api/libcas/gy/gyca_def.h>
#include "win_cas_com.h"

#include "win_mail.h"
/*******************************************************************************
 *	Objects definition

 *******************************************************************************/


static BOOL win_chlist_save_setting(UINT32 type);

static void mail_disp();

static void win_mail_scroll_string();

#define VACT_DELETE_SINGLE 	(VACT_PASS + 1)
#define VACT_DELETE_ALL 		(VACT_PASS + 2)

/*******************************************************************************
 *	Local vriable & function declare
 *******************************************************************************/

POBJECT_HEAD mail_titles[] =
{
    ( POBJECT_HEAD ) &mail_title0,
    ( POBJECT_HEAD ) &mail_title1,
    ( POBJECT_HEAD ) &mail_title2,
    ( POBJECT_HEAD ) &mail_title3,
    ( POBJECT_HEAD ) &mail_title4,
    ( POBJECT_HEAD ) &mail_title5,
};

static BOOL del_all_flag = FALSE;
static UINT8 mail_del_type = 0;
UINT32 del_mail_idx[GYCA_EMAIL_MAX_CNT/32+1] = {0,}; //定义数组
UINT8 cur_mail_id = 0;
UINT8 email_count =100;
UINT8  email_count_unread=0;
gycamail_head email_head[GYCA_EMAIL_MAX_CNT];


//#define LIST_CON_TOP_OFFSET 4
#if 0
static OSD_RECT rc_txt[] =
{
    {ITEM_CON_L,LIST_CON_T+LIST_CON_TOP_OFFSET,TXT1_W,24},
    {ITEM_CON_L+TXT1_W,LIST_CON_T+LIST_CON_TOP_OFFSET,TXT2_W,24},
    {ITEM_CON_L+TXT1_W+TXT2_W,LIST_CON_T+LIST_CON_TOP_OFFSET,TXT3_W,24},
    {ITEM_CON_L+TXT1_W+TXT2_W+TXT3_W,LIST_CON_T+LIST_CON_TOP_OFFSET,/*TXT4_W*/150,24},
    //{ITEM_CON_L+TXT1_W+TXT2_W+TXT3_W+TXT4_W,LIST_CON_T+LIST_CON_TOP_OFFSET,FLAG_W,24},
};

#endif
//#define TXT_NUM (sizeof(rc_txt)/sizeof(OSD_RECT))
/*
static UINT16 id_txt[] =
{
    RS_SEARIAL_NUM,//序号
    RS_MAIL_STATUS,//状态
    RS_TITLE,    //标题
    //RS_MAIL_SENDER,   //发送者
    //RS_HELP_DELETE,
};
*/

static BOOL mail_check_del_flag()
{
	UINT32 i;
	BOOL ret = FALSE;
	
	for (i=0; i<sizeof(del_mail_idx)/sizeof(UINT32); i++)
	{
		if (del_mail_idx[i])
			ret = TRUE;
	}

	return ret;
}

static void mail_set_del_flag(UINT32 index)  
{
	UINT32 wIndex, i;
	UINT8 bShiftBit;
	static BOOL del_flag = FALSE;
	if (index == 0xffff)
	{   
		if (FALSE == del_flag)
		{
			for (i=0; i<email_count; i++)
			{
				wIndex = i/32;
				bShiftBit = i%32;
				del_mail_idx[wIndex] |= (0x00000001 << bShiftBit);
			}
			del_flag = TRUE;
		}
		else
		{
			if (FALSE == mail_check_del_flag())
			{
				for (i=0; i<email_count; i++)
				{
					wIndex = i/32;
					bShiftBit = i%32;
					del_mail_idx[wIndex] |= (0x00000001 << bShiftBit);
				}
				del_flag = TRUE;
			}
			else
			{    
				for (i=0; i<email_count; i++)
				{
					wIndex = i/32;
					bShiftBit = i%32;
					del_mail_idx[wIndex] &= ~(0x00000001 << bShiftBit);
				}
				del_flag = FALSE;
			}
		}
	}
	else if (0 != email_count)
	{   
		wIndex = index / 32;
		bShiftBit = index % 32;
		del_mail_idx[wIndex] ^= (0x00000001 << bShiftBit);
	}
}

static BOOL mail_get_del_flag(UINT32 index)
{
	UINT32 wIndex;
	UINT8 bShiftBit;

	wIndex = index / 32;
	bShiftBit = (UINT8)(index % 32);

	if ((del_mail_idx[wIndex]) &(0x00000001 << bShiftBit))
		return TRUE;
	else
		return FALSE;
}

static void mail_clear_del_flag(void)
{
	MEMSET(del_mail_idx, 0, sizeof(del_mail_idx));
};

static void mail_init_str(UINT8 num)
{
	UINT8 i = 0;

	for (i = 0; i < num; i++)
	{
		MEMSET(display_strs[i], 0, sizeof(display_strs[i]));
	}
}

 


static void MailDrawNum0()  //对新邮件和邮件总数赋值      ---yuanlin
{
    UINT8 *src;
	UINT8 tmp[6]={0,}; 
	UINT8 num=GYCA_EMAIL_MAX_CNT;//                             ------yuanlin
	UINT16 strLen;
	OSD_RECT rc;
	TEXT_FIELD *txt;
	UINT8 total, newmail;
	
	txt = &mail_new;
	OSD_SetTextFieldStrPoint(txt, len_display_str);

	src = OSD_GetUnicodeString(RS_CTI_NEW_MAIL);
	ComUniStrCopyChar((UINT8*)len_display_str, src);
	strLen = ComUniStrLen(len_display_str);
	sprintf(tmp, " %d",email_count_unread );
	ComAscStr2Uni(tmp, &len_display_str[strLen]);
	OSD_SetTextFieldContent(txt, STRING_UNICODE, (UINT32)len_display_str);
	OSD_DrawObject((POBJECT_HEAD)txt, C_UPDATE_ALL);
	OSD_SetTextFieldStrPoint(txt, NULL);

	txt = &mail_total;
	OSD_SetTextFieldStrPoint(txt, len_display_str);
	
	MEMSET(len_display_str, 0, sizeof(len_display_str));
	MEMSET(tmp, 0, sizeof(tmp));
	src = OSD_GetUnicodeString(RS_RESIDUAL_SPACE);
	ComUniStrCopyChar((UINT8*)len_display_str, src);
	strLen = ComUniStrLen(len_display_str);
	sprintf(tmp, " %d",num-email_count);
	ComAscStr2Uni(tmp, &len_display_str[strLen]);
	OSD_SetTextFieldContent(txt, STRING_UNICODE, (UINT32)len_display_str);
	OSD_DrawObject((POBJECT_HEAD)txt, C_UPDATE_ALL);
	OSD_SetTextFieldStrPoint(txt, NULL);
}


#define mailinfo_debug
INT32 mail_got_info(void)
{
	UINT8 ret,i,j;
	UINT8 total, newmail;
	UINT32 MJD_Date;
	UINT8 UTC_Date[5]={0};
	gycamail data;
	
	MEMSET(email_head, 0, sizeof(gycamail_head)*GYCA_EMAIL_MAX_CNT);
	email_count = GYCA_EMAIL_MAX_CNT;
	
	ret = gyca_getmail_condition_from_ram(&total, &newmail);
	if (GYCA_OK != ret)
	{
	//	cas_popup_msg(NULL, NULL, RS_CAERR_NOINFO);
	//	email_count = 0;
	//	return 0;
		email_count = 0;
		total = 0;
		newmail = 0;
		return ;
	}
	
	email_count_unread = newmail;
	
	j = 0;
	for(i=0; i<total; i++)
	{	
		MEMSET(&data, 0, sizeof(gycamail));
		if(!gyca_readmail_one_from_ram(i, &data))
		{
			MEMCPY(email_head[j].title, data.info.m_szEmail_Title, STRLEN(data.info.m_szEmail_Title));
			MEMCPY(email_head[j].create_Time, data.info.m_byEmail_Create_Time, 5);
			
			MJD_Date = data.info.m_byEmail_Create_Time[0]*256 + data.info.m_byEmail_Create_Time[1];
			/*UTC_Date[0] = ( MJD_Date-15078 )/365 + 1900;//MJD算法计算发行年份since 1900
			UTC_Date[1] =  ( MJD_Date-14956-UTC_Date[0]*365.25 )/30;//月份
			UTC_Date[2] = MJD_Date-14956-UTC_Date[0]*365-UTC_Date[1]*30;//日期
			MEMCPY(email_head[j].create_Time, UTC_Date, 5);
			*/
			email_head[j].status = data.status;
			j++;
		}
	}
	
	email_count = j;	
	return 1;
}

INT32 mail_update_info()
{
	UINT8 ret,i,j;
	UINT8 total, newmail;
	gycamail data;
	OBJLIST* ol = &mail_ol;
	
	MEMSET(email_head, 0, sizeof(gycamail_head)*GYCA_EMAIL_MAX_CNT);
	email_count = GYCA_EMAIL_MAX_CNT;

	ret = gyca_getmail_condition_from_ram(&total, &newmail);

	if (GYCA_OK != ret)
	{
		//cas_popup_msg(NULL, NULL, RS_CAERR_NOINFO);
		//email_count = 0;
		//return 0;
		total = 0;
		newmail = 0;
		email_count  = 0;
	}
	
	email_count_unread = newmail;
	
	for(i=0,j=0; i<total; i++)
	{	
		MEMSET(&data, 0, sizeof(gycamail));
		if(!gyca_readmail_one_from_ram(i, &data))
		{
			MEMCPY(email_head[j].title, data.info.m_szEmail_Title, STRLEN(data.info.m_szEmail_Title));
			MEMCPY(email_head[j].create_Time, data.info.m_byEmail_Create_Time, 5);
			email_head[j].status = data.status;
			j++;
		}
	}
	email_count = j;
	OSD_SetObjListCount(ol, email_count);
	if (email_count <= OSD_GetObjListCurPoint(ol))
	{
		OSD_SetObjListCurPoint(ol, 0);
		OSD_SetObjListNewPoint(ol, 0);
		OSD_SetObjListTop(ol, 0);
	}
	return 1;
}

void mail_delete_one(UINT32 index)
{
	BOOL flag = TRUE;
	UINT32 i, ret = GYCA_OK;
	UINT8 back_saved;
	win_popup_choice_t choice;

	win_compopup_init(WIN_POPUP_TYPE_OKNO);
	win_compopup_set_msg(NULL, NULL, RS_MAIL_POP_DEL_CURRENT);
	choice = win_compopup_open_ext(&back_saved);
	if (choice == WIN_POP_CHOICE_YES)
	{
		/**
		** 防止未插卡时不能删除邮件
		**/
		#if 0
		if(gy_get_sc_status() == FALSE)
		{
			cas_popup_msg(NULL, NULL, RS_CAERR_INSERT_CARD);
			return;
		}
		#endif
		ret = gyca_delmail_one_from_ram(index);
		if (ret == GYCA_OK)
		{
			mail_update_info();
		}
		else
		{
			cas_popup_msg(NULL, NULL, RS_MAIL_DEL_ERR);
		}
	}
}

void mail_delete_all()
{
	BOOL flag = TRUE;
	UINT32 i, ret = GYCA_OK;
	UINT8 back_saved;
	win_popup_choice_t choice;

	win_compopup_init(WIN_POPUP_TYPE_OKNO);
	win_compopup_set_msg(NULL, NULL, RS_MAIL_POP_DEL_ALL);
	choice = win_compopup_open_ext(&back_saved);
	if (choice == WIN_POP_CHOICE_YES)
	{
		ret = gyca_delmail_all_from_ram();
		if (ret == GYCA_OK)
		{
			mail_update_info();
		}
	}
} 
void mail_exec_delete()
{
	BOOL flag = TRUE;
	UINT32 i, ret = GYCA_OK;
	UINT8 back_saved;
	win_popup_choice_t choice;
	UINT16 sel;
	OBJLIST *ol;
	
	ol = &mail_ol;
	sel = OSD_GetObjListCurPoint(ol);
	
	if (FALSE == mail_check_del_flag())
	{
		return;
	}

	if(mail_del_type == 0)
	{
		mail_delete_one(sel);
		gyca_check_new_mail();
	}
	else if(mail_del_type == 1)
	{
		mail_delete_all();
		gyca_check_new_mail();
	}
	else{;}
#if 0
		//first check if all mails need to be deleted
		for (i=0; i<email_count; i++)
		{
			if (FALSE == mail_get_del_flag(i))
			{
				flag = FALSE;
				break;
			}
		}
		
		if ((TRUE == flag) && (TRUE == del_all_flag))
		{
//			ret = DVTCASTB_DelEmail(-1);
			//libc_printf("$$MAIL DEL ALL\n");
		}
		else
		{
			for (i=0; i<email_count; i++)
			{
				if (TRUE == mail_get_del_flag(i))
				{
//					ret = DVTCASTB_DelEmail(email_head[i].m_dwVersion);
					if (ret != GYCA_OK)
						break;
				}
			}
			//libc_printf("$$MAIL DEL SINGLE\n");
		}	
//	if(ret == DVTCAERR_STB_NO_EMAIL)
//	{
//		cas_popup_msg(NULL, NULL, RS_MAIL_DEL_ERR);
//	}
#endif
	mail_clear_del_flag();
}

 

/*******************************************************************************
*	key mapping and event callback definition
*******************************************************************************/

static VACTION mail_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;
	switch (key)
	{    

		case V_KEY_MENU:
			act = VACT_CLOSE;
			break;
		case V_KEY_EXIT:
			mail_exec_delete();
			BackToFullScrPlay();
			//CDCASTB_RequestUpdateBuffer();
			act = VACT_CLOSE;
			break;
		default:
			act = VACT_PASS;
	}
	return act;
}

static PRESULT mail_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	BOOL changed;
	UINT8 TV_Out;
	UINT16 x, y, w, h;
	VACTION unact;
	UINT8 av_mode, id;
	UINT16 chan_idx;
    UINT8 rating;
    UINT16 rating_ret;
	OBJLIST* ol = &mail_ol;
	/*struct help_item_resource mail_help[] =
	{
		{1,IM_EPG_COLORBUTTON_RED,RS_HELP_DELETE},
		{1,IM_EPG_COLORBUTTON_GREEN,RS_DELETE_ALL},
		{1,IM_HELP_ICON_TB,RS_HELP_SELECT},
		{0,RS_HELP_ENTER,RS_HELP_ENTER_INTO},
	};
*/
	switch (event)
	{
		case EVN_PRE_OPEN:
			
		   OSD_ClearObject((POBJECT_HEAD)&g_win_mainmenu, C_UPDATE_ALL);
			
			rating_ret = GYCA_GetParentControl(&rating);
	    		/*if ((CDCA_RC_CARD_INVALID == rating_ret)|| (CDCA_RC_POINTER_INVALID ==rating_ret))
	    		{
	    			win_popup_msg(NULL, NULL, RS_CAERR_NOSMC);
	    			    return PROC_LEAVE;
	    		}
	    		else if (CDCA_RC_OK != rating_ret)
	    		{
	    			win_popup_msg(NULL, NULL, RS_CAERR_UNKNOWN);
	    			    return PROC_LEAVE;
	    		}
	    		*/
   			MEMSET(display_strs[0], 0, sizeof(display_strs[0])*MAX_DISP_STR_ITEMS);
			//wincom_open_title_ext(RS_EMAIL, IM_TITLE_ICON_EMAIL);
			//wincom_open_help(mail_help, 4);
			mail_got_info();
			//mail_init_str(20);			
			//mail_clear_del_flag();
			if(email_count<=100)
				OSD_SetObjListCount(ol, email_count);
			else
				OSD_SetObjListCount(ol, 100);
			if (email_count <= OSD_GetObjListCurPoint(ol))
			{
				OSD_SetObjListCurPoint(ol, 0);
				OSD_SetObjListNewPoint(ol, 0);
				OSD_SetObjListTop(ol, 0);
			}
			//CDCASTB_RequestMaskBuffer();
			break;
		case EVN_POST_OPEN:

			MailDrawNum0();
			wincom_scroll_textfield_resume();
			break;
		case EVN_PRE_CLOSE:
			mail_exec_delete();
			if(menu_stack_get(0)==((POBJECT_HEAD)&g_win_mainmenu))
				*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
			else
			{
				wincom_close_title();
				wincom_close_help();
			    *((UINT32*)param2) &= C_CLOSE_CLRBACK_FLG;
			}
			//CDCASTB_RequestUpdateBuffer();
			wincom_scroll_textfield_pause();
			break;
		case EVN_POST_CLOSE:
			break;
		case EVN_MSG_GOT:
			if( param1 == CTRL_MSG_SUBTYPE_STATUS_SCROLL_STRING)
				win_mail_scroll_string();
            if(param1 == CTRL_MSG_SUBTYPE_STATUS_SIGNAL)
                ret = PROC_LOOP;
		default:
			break;
		
	}

	return ret;
}


static PRESULT mail_unknow_proc(VACTION act)
{
	PRESULT ret = PROC_LOOP;
	UINT16 i,sel, cnt;
	OBJLIST *ol;
	POBJECT_HEAD temp;
    UINT8 back_saved;
	ol = &mail_ol;
	sel = OSD_GetObjListCurPoint(ol);
	cnt = OSD_GetObjListCount(ol);

	switch (act)
	{
		case VACT_DELETE_SINGLE:
			  	{
				mail_del_type = 0;
			   //if(email_count > 0)
                     //{
                   // win_compopup_init(WIN_POPUP_TYPE_OKNO);
	                //win_compopup_set_msg(NULL, NULL, RS_MAIL_POP_DEL_CURRENT);
                    //win_compopup_open_ext(&back_saved);
                    
        			mail_set_del_flag(sel);
					//mail_exec_delete();  //yuanlin change
        			OSD_TrackObject((POBJECT_HEAD)ol, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
                     //}
			  }
			break;
		case VACT_DELETE_ALL:
			mail_del_type = 1;
			{	
			    //win_compopup_init(WIN_POPUP_TYPE_OKNO);
	            //win_compopup_set_msg(NULL, NULL, RS_MAIL_POP_DEL_ALL);
				//win_compopup_open_ext(&back_saved);
				mail_set_del_flag(0xffff);//set all to delete
				//mail_exec_delete();  //yuanlin change
				OSD_TrackObject((POBJECT_HEAD)ol, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
			};        //----yuanlin
			break;
		case VACT_ENTER:
			 if (0 == email_count)
			 break;                                 //  ---yuanlin
			cur_mail_id = sel;
			temp = (POBJECT_HEAD)&g_win_mail_content; //进入到Win_mail_content界面
			if (OSD_ObjOpen(temp, 0xFFFFFFFF) != PROC_LEAVE)
			{
				menu_stack_push(temp);
				if(email_head[sel].status)
				 	email_count_unread--;
				//MailDrawNum0();         //yuanlin 21013/02/04
			}
			
			break;
		default:
			break;
	}
	return ret;
}


static VACTION mail_list_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;
	//chan_edit_grp_t* pcheditgrp;
	UINT32 i, n;

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
			act = VACT_DELETE_SINGLE;
			break;
		case V_KEY_YELLOW://V_KEY_GREEN:
			act = VACT_DELETE_ALL;
			break;
		case V_KEY_ENTER:
			act = VACT_ENTER;
			break;
		case V_KEY_EXIT:
			act = VACT_PASS;
			break;
		default:
			break;
	}
	return act;
}

static PRESULT mail_list_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	VACTION unact;
	UINT16 sel;
	OBJLIST *ol;

	ol = (OBJLIST*)pObj;

	switch (event)
	{
		case EVN_PRE_DRAW:
		   //画出信息列表 -- yuanlin
            
			ret = PROC_LOOP;
			if (param1 == C_DRAW_TYPE_HIGHLIGHT)
			{
				mail_disp();
				OSD_TrackObject(pObj, C_UPDATE_ALL);
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
			ret = mail_unknow_proc(unact);
			break;
		case EVN_UNKNOWNKEY_GOT:
			break;
		default:
			;
	}


	return ret;
}

static void mail_disp()
{
	UINT32 i, valid_idx;
	UINT32 MJD_Date;
	UINT16 UTC_Date[3];
	UINT8 K;
	UINT8 asc_str[100];
	UINT16 top, cnt, page, index, strid,iconID;
	UINT16 mail_title[40+1], date[20];
	UINT8 str[6];//-----yuanlin
	BOOL del_flag;
	OBJLIST *ol;
	CONTAINER *item;
	TEXT_FIELD *txt;
	BITMAP *bmp;

	ol = &mail_ol;

	cnt = OSD_GetObjListCount(ol);
	page = OSD_GetObjListPage(ol);
	top = OSD_GetObjListTop(ol);

	for (i = 0; i < page; i++)
	{
		item = (CONTAINER*)mail_ol_ListField[i];
		index = top + i;
		del_flag = FALSE;

		valid_idx = (index < cnt) ? 1 : 0;
		if (valid_idx)
		{
			del_flag = mail_get_del_flag(index);
		}

		/* IDX */
		txt = (TEXT_FIELD*)OSD_GetContainerNextObj(item);
		if (valid_idx)
		{
			sprintf(str, "%02d", index + 1);
			OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)str);
		}
		else
			OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)"");
         
		/*mail title*/
		txt = (TEXT_FIELD*)OSD_GetObjpNext(txt);
		if (valid_idx)
		{
			convert_gb2312_to_unicode(email_head[index].title, 40+1, mail_title, 40+1);
			OSD_SetTextFieldContent(txt, STRING_UNICODE, (UINT32)mail_title);
		}
		else
			OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)"");
		wincom_scroll_textfield_stop(txt);



		
		/* time */
		txt = (TEXT_FIELD*)OSD_GetObjpNext(txt);
		if (valid_idx)
		{
			#if 1
			MEMSET(len_display_str, 0, sizeof(len_display_str));
			MJD_Date = email_head[index].create_Time[0]*256 + email_head[index].create_Time[1];
			UTC_Date[0] = ( MJD_Date-15078.2 )/365.25;//MJD算法计算发行年份
			UTC_Date[1] =  ( MJD_Date-14956.1-(UINT32)(UTC_Date[0]*365.25))/30.6001;//月份
			UTC_Date[2] = MJD_Date-14956-(UINT32)(UTC_Date[0]*365.25)-(UINT32)(UTC_Date[1]*30.6001);//日期
			if( (UTC_Date[1]==15) || (UTC_Date[1]==14) )
				K=1;
			else
				K=0;
			UTC_Date[0] += K;
			UTC_Date[1] = UTC_Date[1]-1-K*12;
			UTC_Date[0] += 1900;//Year since 1900
			
			sprintf(asc_str, "%02d%02d-%02d-%02d",(UINT8)(UTC_Date[0]/100),(UINT8)(UTC_Date[0]%100),UTC_Date[1],UTC_Date[2]);
			ComAscStr2Uni(asc_str, len_display_str);
			
			OSD_SetTextFieldContent(txt, STRING_UNICODE, (UINT32)len_display_str);
			#else
			convert_sysdate_to_uni(email_head[index].create_Time, date);
			date[16] = '\0';
			OSD_SetTextFieldContent(txt, STRING_UNICODE, (UINT32)date);
			#endif
		}
		else
			OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)"");
        
			/* Delete */
		bmp = (BITMAP*)OSD_GetObjpNext(txt);
		iconID = (del_flag) ?IM_TV_DEL: 0;
		OSD_SetBitmapContent(bmp, iconID);
         
	}
}

static void win_mail_scroll_string()
{
	POBJLIST ol = &mail_ol;
	UINT16 cur_point = 0;

	cur_point = OSD_GetObjListCurPoint(ol);
	cur_point -= OSD_GetObjListTop(ol);
	wincom_scroll_textfield((PTEXT_FIELD)mail_titles[cur_point]);
}
