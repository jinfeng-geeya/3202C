#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <api/libpub/lib_pub.h>
#include <api/libpub/lib_hde.h>
#include <hld/dis/vpo.h>
//#include <api/libosd/osd_lib.h>
#include <api/libsi/lib_epg.h>

#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"
#include "win_com.h"
#include "menus_root.h"
#include "win_com_popup.h"
#include "win_com_list.h"

#include "copper_common/com_epg.h"
#include "win_epg.h"
#include "control.h"

/*******************************************************************************
 *	Objects definition
 *******************************************************************************/
CONTAINER g_win_epg_detail;

BITMAP epg_detail_chan_bmp;
TEXT_FIELD epg_detail_chan;
TEXT_FIELD epg_detail_event_name;
TEXT_FIELD epg_detail_time;
TEXT_FIELD epg_detail_lang;
TEXT_FIELD epg_detail_content;

MULTI_TEXT epg_detail_info;
SCROLL_BAR epg_detail_scb;

BITMAP epg_detail_help_bmp;
TEXT_FIELD epg_detail_help_txt;

static VACTION epg_detail_info_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT epg_detail_info_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION epg_detail_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT epg_detail_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);


#define WIN_SH_IDX	  WSTL_POP_WIN_1

#define EVENT_SH_IDX WSTL_TXT_3
#define DETAIL_SH_IDX WSTL_LIST_BG///WSTL_TXT_WIN_POP



#define LANG_SH_IDX WSTL_TXT_3

#define BAR_SH_IDX  WSTL_SCROLLBAR_1
#define BAR_HL_IDX  WSTL_SCROLLBAR_1

#define BAR_MID_RECT_IDX  	WSTL_NOSHOW_IDX
#define BAR_MID_THUMB_IDX  	WSTL_SCROLLBAR_2

#define W_L     	150
#define W_T     	100
#define W_W		350
#define W_H     	320

#define EVENT_TIMEBMP_L 170
#define EVENT_TIMEBMP_T (W_T+12)
#define EVENT_TIMEBMP_W 24
#define EVENT_TIMEBMP_H 24

#define CHAN_L (EVENT_TIMEBMP_L+EVENT_TIMEBMP_W)
#define CHAN_T (EVENT_TIMEBMP_T)
#define CHAN_W 270
#define CHAN_H 24

#define EVENT_NAME_L (CHAN_L)
#define EVENT_NAME_T (EVENT_TIMEBMP_T+30)
#define EVENT_NAME_W (CHAN_W)
#define EVENT_NAME_H 24

#define EVENT_TIME_L (EVENT_NAME_L)
#define EVENT_TIME_T (EVENT_NAME_T+24)
#define EVENT_TIME_W 124
#define EVENT_TIME_H 24

#define EVENT_LANG_L 330
#define EVENT_LANG_T (EVENT_TIME_T)
#define EVENT_LANG_W 134
#define EVENT_LANG_H 24

#define EVENT_CONTENT_L (EVENT_TIME_L)
#define EVENT_CONTENT_T (EVENT_TIME_T+24)
#define EVENT_CONTENT_W 140
#define EVENT_CONTENT_H 24

#define DTL_INFO_L 170
#define DTL_INFO_T (EVENT_CONTENT_T+30)
#define DTL_INFO_W 294
#define DTL_INFO_H 156

#define SCB_L 470
#define SCB_T (DTL_INFO_T)
#define SCB_W 14
#define SCB_H (DTL_INFO_H)

#define HELP_BMP_L 370
#define HELP_BMP_T 384
#define HELP_BMP_W 24
#define HELP_BMP_H 24

#define HELP_TXT_L 402
#define HELP_TXT_T (HELP_BMP_T)
#define HELP_TXT_W 80
#define HELP_TXT_H 24

#define LDEF_TXT(root,varTxt,nxtObj,l,t,w,h,sh,align,resID,str)		\
    DEF_TEXTFIELD(varTxt,&root,nxtObj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, sh,sh,sh,sh,   \
    NULL,NULL,  \
    align, 0,0,resID,str)

#define LDEF_BMP(root,varBmp,nxtObj,l,t,w,h,icon)	\
  DEF_BITMAP(varBmp,&root,nxtObj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, EVENT_SH_IDX,EVENT_SH_IDX,EVENT_SH_IDX,EVENT_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,icon)

#define LDEF_MTXT(root,varMtxt,nextObj,ID,l,t,w,h,sh,cnt,sb,content)	\
	DEF_MULTITEXT(varMtxt,&root,nextObj,C_ATTR_ACTIVE,0, \
    	ID,ID,ID,ID,ID, l,t,w,h, sh,sh,sh,sh,   \
	    epg_detail_info_keymap,epg_detail_info_callback,  \
    	C_ALIGN_LEFT | C_ALIGN_TOP, cnt,  4,4,w-8,h-8,sb,content)

#define LDEF_BAR(root,varScb,page,l,t,w,h)	\
	DEF_SCROLLBAR(varScb, &root, NULL, C_ATTR_ACTIVE, 0, \
		0, 0, 0, 0, 0, l, t, w, h, BAR_SH_IDX, BAR_HL_IDX, BAR_SH_IDX, BAR_SH_IDX, \
		NULL, NULL, BAR_VERT_AUTO | SBAR_STYLE_RECT_STYLE, page, BAR_MID_THUMB_IDX, BAR_MID_RECT_IDX, \
		0, 10, w, h - 20, 100, 1)

#define LDEF_WIN(varCon,nxtObj,l,t,w,h,focusID)		\
    DEF_CONTAINER(varCon,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WSTL_MY_EPG_DETAIL,WSTL_MY_EPG_DETAIL,WSTL_MY_EPG_DETAIL,WSTL_MY_EPG_DETAIL,   \
    epg_detail_keymap,epg_detail_callback,  \
    nxtObj, focusID,0)

TEXT_CONTENT detail_mtxt_content[];

LDEF_BMP ( g_win_epg_detail, epg_detail_chan_bmp, &epg_detail_chan, \
           EVENT_TIMEBMP_L, EVENT_TIMEBMP_T, EVENT_TIMEBMP_W, EVENT_TIMEBMP_H, IM_HELP_RED)

LDEF_TXT ( g_win_epg_detail, epg_detail_chan, &epg_detail_event_name, \
           CHAN_L, CHAN_T, CHAN_W, CHAN_H, EVENT_SH_IDX, C_ALIGN_LEFT | C_ALIGN_VCENTER, 0, display_strs[0] )

LDEF_TXT ( g_win_epg_detail, epg_detail_event_name, &epg_detail_time, \
           EVENT_NAME_L, EVENT_NAME_T, EVENT_NAME_W, EVENT_NAME_H, EVENT_SH_IDX, C_ALIGN_LEFT | C_ALIGN_VCENTER, 0, display_strs[1] )

LDEF_TXT ( g_win_epg_detail, epg_detail_time, &epg_detail_lang, \
           EVENT_TIME_L, EVENT_TIME_T, EVENT_TIME_W, EVENT_TIME_H, EVENT_SH_IDX, C_ALIGN_LEFT | C_ALIGN_VCENTER, 0, display_strs[2] )

LDEF_TXT ( g_win_epg_detail, epg_detail_lang, &epg_detail_content, \
           EVENT_LANG_L, EVENT_LANG_T, EVENT_LANG_W, EVENT_LANG_H, EVENT_SH_IDX, C_ALIGN_RIGHT | C_ALIGN_VCENTER, 0, display_strs[3] )

LDEF_TXT ( g_win_epg_detail, epg_detail_content, &epg_detail_info, \
           EVENT_CONTENT_L, EVENT_CONTENT_T, EVENT_CONTENT_W, EVENT_CONTENT_H, EVENT_SH_IDX, C_ALIGN_LEFT | C_ALIGN_VCENTER, 0, display_strs[4] )

LDEF_BAR ( epg_detail_info, epg_detail_scb, 6, SCB_L, SCB_T, SCB_W, SCB_H )

LDEF_MTXT ( g_win_epg_detail, epg_detail_info, &epg_detail_help_bmp, 1, \
            DTL_INFO_L, DTL_INFO_T, DTL_INFO_W, DTL_INFO_H, DETAIL_SH_IDX, 1, &epg_detail_scb, detail_mtxt_content )

LDEF_BMP ( g_win_epg_detail, epg_detail_help_bmp, &epg_detail_help_txt, \
           HELP_BMP_L, HELP_BMP_T, HELP_BMP_W, HELP_BMP_H, IM_EPG_COLORBUTTON_RED )
LDEF_TXT ( g_win_epg_detail, epg_detail_help_txt, NULL, \
           HELP_TXT_L, HELP_BMP_T, HELP_TXT_W, HELP_BMP_H, EVENT_SH_IDX, C_ALIGN_LEFT | C_ALIGN_VCENTER, RS_COMMON_CANCEL, NULL )

LDEF_WIN ( g_win_epg_detail, &epg_detail_chan_bmp, W_L, W_T, W_W, W_H, 1 )

/*******************************************************************************
*	Local functions & variables declare
*******************************************************************************/
TEXT_CONTENT detail_mtxt_content[] =
    {
        {
            STRING_UNICODE, 0
        },
    };

#define DETAIL_BUFFER_LEN		1024*3

UINT16 *detail_str = NULL;
static UINT8 *bg_backup = NULL;

/*******************************************************************************
 *	Local functions definition
 *******************************************************************************/
static INT32 win_epg_detail_info_update(eit_event_info_t *ep, INT32 idx)
{
	INT32 strlen = 0, len;
	UINT8 *p_short = NULL;
	UINT8 *p_extend = NULL;
	UINT16 *p = detail_str;
	INT32 short_num, extented_num, i;

	len = epg_get_event_all_short_detail(ep, p + 1, DETAIL_BUFFER_LEN);
	if (len != 0)
	{
		p[0] = 0x2a00; //'*'
		len++;
		if (len < DETAIL_BUFFER_LEN - 1)
		{
			p[len++] = 0x0a00; //"\n"
			p[len] = 0;
		}
	}

	strlen = epg_get_event_all_extented_detail(ep, p + len + 1, DETAIL_BUFFER_LEN - len - 1);
	if (strlen != 0)
	{
		p[len] = 0x2a00; //'*'
		len += strlen + 1;
	}

	epg_detail_info.nLine = 0;

	if (p != NULL && len > 0)
	{
		OSD_SetAttr((POBJECT_HEAD) &epg_detail_scb, C_ATTR_ACTIVE);
	}
	else
	{
		OSD_SetAttr((POBJECT_HEAD) &epg_detail_scb, C_ATTR_HIDDEN);
	}

	detail_mtxt_content[0].text.pString = detail_str;

	return len;

}

static void win_epg_detail_display(UINT16 channel, struct winepginfo *epginfo)
{
	P_NODE service_info;
	//	T_NODE tp_info;
	PTEXT_FIELD chan, name, lang, time, content;
	PMULTI_TEXT info;
	eit_event_info_t *ep = NULL;
	INT32 event_num;
	UINT16 unistr[30];
	INT8 str[30], str2[30];
	INT32 len;
	UINT16 event_name[33];
	UINT8 *s;
	UINT16 num = 0; //event index

	chan = &epg_detail_chan;
	name = &epg_detail_event_name;
	time = &epg_detail_time;
	lang = &epg_detail_lang;
	content = &epg_detail_content;
	info = &epg_detail_info;

	/*channel*/
	if (get_prog_at(channel, &service_info) != SUCCESS)
		goto NO_VALID_EVENT;

	if (service_info.ca_mode)
		sprintf(str, "%04d $", channel + 1);
	else
		sprintf(str, "%04d ", channel + 1);

	ComAscStr2Uni(str, unistr);
	len = ComUniStrLen(unistr);

	ComUniStrCopyChar((UINT8*) &unistr[len], service_info.service_name);
	OSD_SetTextFieldContent(chan, STRING_UNICODE, (UINT32)unistr);

	if (epginfo->event_idx == INVALID_ID)
	{
		goto NO_VALID_EVENT;
	}

	ep = epg_get_cur_service_event(channel, SCHEDULE_EVENT, &epginfo->start, &epginfo->end, &event_num, FALSE);
	/*	if(NULL == ep)
	{
	ep=epg_get_cur_service_event(channel, PRESENT_EVENT, &epginfo->start, &epginfo->end, &event_num, FALSE);	
	if (ep == NULL)
	ep=epg_get_cur_service_event(channel, FOLLOWING_EVENT, &epginfo->start, &epginfo->end, &event_num, FALSE);	
	}*/

	while (ep != NULL)
	{
		if (ep->event_id == epginfo->event_idx)
		{
			//		s = epg_get_event_name(ep,&len);
			s = (UINT8*)epg_get_event_name(ep, event_name, 32);
			if (s == NULL)
			{
				goto NO_VALID_EVENT;
			}

			OSD_SetTextFieldContent(name, STRING_UNICODE, (UINT32)s);

			epg_get_event_start_time(ep, str);
			len = STRLEN(str);

			STRCPY(&str[len], "~");
			len = STRLEN(str);

			epg_get_event_end_time(ep, str2);

			STRCPY(&str[len], str2);

			OSD_SetTextFieldContent(time, STRING_ANSI, (UINT32)str);

			s = (UINT8*)epg_get_event_language(ep);
			if (s == NULL)
			{
				goto NO_VALID_EVENT;
			}

			str[0] = s[0];
			str[1] = s[1];
			str[2] = s[2];
			str[3] = 0;

			OSD_SetTextFieldContent(lang, STRING_ANSI, (UINT32)str);

			epg_get_event_content_type(ep, str, &len);

			OSD_SetTextFieldContent(content, STRING_ANSI, (UINT32)str);

			len = win_epg_detail_info_update(ep, 0);

			break;
		}
		//		ep=ep->next;
		ep = epg_get_schedule_event(++num);
	}

	if (ep == NULL)
	{
NO_VALID_EVENT:
		STRCPY ( str, "" );
		OSD_SetTextFieldContent(name, STRING_ANSI, (UINT32)str);
		OSD_SetTextFieldContent(time, STRING_ANSI, (UINT32)str);
		OSD_SetTextFieldContent(lang, STRING_ANSI, (UINT32)str);
		OSD_SetTextFieldContent(content, STRING_ANSI, (UINT32)str);

		OSD_SetAttr((POBJECT_HEAD) &epg_detail_scb, C_ATTR_HIDDEN);
		detail_mtxt_content[0].text.pString = NULL;
	}
}

/*******************************************************************************
 *	key mapping and event callback definition
 *******************************************************************************/
static VACTION epg_detail_info_keymap(POBJECT_HEAD pObj, UINT32 key)
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
		default:
			act = VACT_PASS;
	}

	return act;

}

static PRESULT epg_detail_info_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;

	return ret;
}

static VACTION epg_detail_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

	switch (key)
	{
		case V_KEY_EXIT:
		case V_KEY_MENU:
		case V_KEY_RED:
			act = VACT_CLOSE;
			break;
		default:
			act = VACT_PASS;
	}

	return act;

}

static PRESULT epg_detail_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	UINT16 channel;

	switch (event)
	{
		case EVN_PRE_OPEN:
			break;
		case EVN_POST_OPEN:
			break;
		case EVN_PRE_CLOSE:
			*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
			break;
		case EVN_POST_CLOSE:
#ifdef MULTI_CAS
#if(CAS_TYPE==CAS_IRDETO)
			if(IRCA_BAN_GetBannerShow())
			{
				//ap_cas_call_back(0x00000020);
				//ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MCAS,0x00200000 , FALSE);
				ap_send_msg_expand(CTRL_MSG_SUBTYPE_STATUS_MCAS,0x00000020 ,0, FALSE);
			}
#endif
#endif
			if (detail_str)
			{
#if((SYS_SDRAM_SIZE == 8)&&(SHOW_SINGLE_PIC == 2))
				detail_str = NULL;
#else
				FREE(detail_str);
				detail_str = NULL;
#endif 
			}
			if (bg_backup != NULL)
			{
#if((SYS_SDRAM_SIZE == 8)&&(SHOW_SINGLE_PIC == 2))
				bg_backup =NULL;
#else
				FREE(bg_backup);
		        bg_backup =NULL;
#endif 
			}
			break;
	}

	return ret;
}

void win_epg_detail_open(UINT16 channel, struct winepginfo *epginfo)
{
	POBJECT_HEAD pObj = (POBJECT_HEAD) &g_win_epg_detail;
	PRESULT bResult = PROC_LOOP;
//	UINT8 *bg_backup = NULL;
	UINT32 hkey;
	VSCR vscr;

	if (detail_str == NULL)
    {    
#if((SYS_SDRAM_SIZE == 8)&&(SHOW_SINGLE_PIC == 2))
		detail_str = (UINT16 *)(__MM_IMAGEDEC_BUF_ADDR&0x8FFFFFFF);
#else
		detail_str = MALLOC(DETAIL_BUFFER_LEN *sizeof(UINT16));
		if (detail_str == NULL)
			return ;
#endif    
    }
	MEMSET(detail_str, 0, DETAIL_BUFFER_LEN *sizeof(UINT16));


#if((SYS_SDRAM_SIZE == 8)&&(SHOW_SINGLE_PIC == 2))
	bg_backup = (UINT8 *)((__MM_IMAGEDEC_BUF_ADDR + DETAIL_BUFFER_LEN *sizeof(UINT16))&0x8FFFFFFF);
#else
	bg_backup = MALLOC(g_win_epg_detail.head.frame.uWidth * g_win_epg_detail.head.frame.uHeight * 2);
	if (bg_backup == NULL)
	{
		if (detail_str != NULL)
			FREE(detail_str);
		return;
	}
#endif 
	MEMSET(bg_backup, OSD_TRANSPARENT_COLOR, g_win_epg_detail.head.frame.uWidth * g_win_epg_detail.head.frame.uHeight * 2);
#ifdef USE_LIB_GE
       wincom_backup_region(&g_win_epg_detail.head.frame, bg_backup);
#else
	wincom_backup_region(&g_win_epg_detail.head.frame, &vscr);
#endif
	win_epg_detail_display(channel, epginfo);

	OSD_ObjOpen(pObj, MENU_OPEN_TYPE_OTHER);

	while (bResult != PROC_LEAVE)
	{

		hkey = ap_get_key_msg();
		if (hkey == INVALID_HK || hkey == INVALID_MSG)
		{
			continue;
		}

		bResult = OSD_ObjProc(pObj, (MSG_TYPE_KEY << 16), hkey, 0);
	}

#ifdef USE_LIB_GE
	wincom_restore_region(&g_win_epg_detail.head.frame, bg_backup, TRUE);
#else
	wincom_restore_region(&vscr);	
#endif

	if (detail_str)
	{
#if((SYS_SDRAM_SIZE == 8)&&(SHOW_SINGLE_PIC == 2))
		detail_str = NULL;
#else
		FREE(detail_str);
		detail_str = NULL;
#endif 
	}
}
