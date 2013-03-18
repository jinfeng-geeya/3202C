#include <sys_config.h>

#include <types.h>
#include <basic_types.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>
#include <hld/osd/osddrv_dev.h>
#include <api/libge/osd_lib.h>
#include "osdobjs_def.h"
#include "images.id"
#include "string.id"
#include "osd_config.h"

#include "control.h"
#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"

//include the header from xform 
#include "win_book_detaile.h"
extern  TEXT_FIELD   TextField1;
extern  TEXT_FIELD 		reserve_idx0;
	extern  TEXT_FIELD TextFieldbd3;
	extern  TEXT_FIELD TextFieldbd1;
	extern  TEXT_FIELD TextFieldbd2;
	extern  TEXT_FIELD TextFieldbd4;
	extern OBJLIST reserve_ol;
	#define INDEX_MARK	0x0000FFFF
	extern UINT32 del_flag[MAX_TIMER_NUM];
	#define GET_TIMER_INDEX(p) (p & INDEX_MARK)
	//extern  TEXT_FIELD 
static UINT16 title_time_buf[30];
UINT16 weekday_id1[] = 
{
    RS_SUN,
    RS_MON,
    RS_TUES,
    RS_WED,
    RS_THURS,
    RS_FRI,
    RS_SAT
};
/*
static void win_book_detail_display(UINT16 channel,struct winepginfo *epginfo)
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

	chan = &TextFieldbd1;
	name = &TextFieldbd2;
	time = &TextFieldbd3;
	lang = &TextFieldbd4;
	content = &TextFieldbd5;
	//info = &epg_detail_info;

	
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

	while (ep != NULL)
	{
		if (ep->event_id == epginfo->event_idx)
		{
			////		s = epg_get_event_name(ep,&len);
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
				///ep=ep->next;
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

		///OSD_SetAttr((POBJECT_HEAD) &epg_detail_scb, C_ATTR_HIDDEN);
		//detail_mtxt_content[0].text.pString = NULL;
	}
}
*/
void play_time()
{
	TEXT_FIELD *txt;
	date_time dt;
	char str[40];
	UINT8 *src;
	UINT16 strLen;
	UINT16 preLeft;

	txt = &TextFieldbd1;
	preLeft = txt->head.frame.uLeft;
	
	txt->pString = title_time_buf;

	get_local_time(&dt);
	sprintf(str, "%4d.%02d.%02d ", dt.year, dt.month, dt.day);
	ComAscStr2Uni(str, len_display_str);
	strLen = ComUniStrLen(len_display_str);
	src = OSD_GetUnicodeString(weekday_id1[dt.weekday%7]);
	ComUniStrCopyChar((UINT8*) &len_display_str[strLen], src);

	strLen = ComUniStrLen(len_display_str);
	MEMSET(str, 0, sizeof(str));
	sprintf(str, " %02d:%02d ", dt.hour, dt.min);
	ComAscStr2Uni(str, &len_display_str[strLen]);

	OSD_SetTextFieldContent(txt, STRING_UNICODE, (UINT32)len_display_str);
}
/*******************************************************************************
*	Callback and Keymap Functions
*******************************************************************************/
static VACTION book_detailkeymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;
	UINT32 hKey;
	
	

	switch (key)
	{
		case V_KEY_EXIT:
			//BackToFullScrPlay();
			act = VACT_CLOSE;
			break;
		default:
			act = VACT_PASS;
	}

	return act;

}
static PRESULT book_detailcallback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	eit_event_info_t *sch_event = NULL;
	UINT16 *s;
	UINT16 eventName[30];
	TEXT_FIELD *txt=&TextFieldbd6;
	TEXT_FIELD *txt1=&TextFieldbd2;
	date_time start_time, end_time,tempTime;
	INT32 h, m,sec,a;
	char str[40];
	UINT16 cur_channel;
	SYSTEM_DATA *sys_data;
	UINT32 index_in_timer = 0;
		TIMER_SET_CONTENT *timer;
		INT32 hour_off, min_off, sec_off;

		UINT16 prog_name[30];
		///UINT32 prog_name2[30]={0};
		OBJLIST *ol=&reserve_ol;
		UINT8 *s1 = NULL;
		eit_event_info_t *pe = NULL;
		/*
		sys_data = sys_data_get();
		a=(OSD_GetObjListCurPoint(ol));
		index_in_timer = GET_TIMER_INDEX(del_flag[a-1]);
		timer = &sys_data->timer_set.TimerContent[index_in_timer];
		*/
	switch ( event )
	{
		case EVN_PRE_OPEN:

			sys_data = sys_data_get();
		a=(OSD_GetObjListCurPoint(ol));
		index_in_timer = GET_TIMER_INDEX(del_flag[a]);
		timer = &sys_data->timer_set.TimerContent[index_in_timer];
			sch_event = epg_get_schedule_event(2);
			s = epg_get_event_name(sch_event, eventName, 32);
			if (s != NULL)
				OSD_SetTextFieldContent(txt, STRING_UNICODE, (UINT32)eventName);

/*
			get_STC_offset(&h, &m, &sec);
			get_event_start_time(sch_event, &start_time);
			get_event_end_time(sch_event, &end_time);

			convert_time_by_offset(&start_time, &start_time, h, m);
			convert_time_by_offset(&end_time, &end_time, h, m);

	        sprintf(str, "%02d:%02d:%02d~%02d:%02d:%02d", start_time.hour, start_time.min, start_time.sec,  \
					end_time.hour, end_time.min, end_time.sec);
			OSD_SetTextFieldContent(txt1, STRING_ANSI, (UINT32)str);
			*/

			sprintf ( str, "%02d:%02d~%02d:%02d",
    			          timer->wakeup_time / 3600, timer->wakeup_time/60%60 ,(timer->wakeup_duration_time+timer->wakeup_time )/ 3600,(timer->wakeup_duration_time+timer->wakeup_time )/60%60);

			OSD_SetTextFieldContent(txt1, STRING_ANSI, (UINT32)str);
			if (timer->timer_mode != TIMER_MODE_OFF)
    			sprintf ( str, "%04d-%02d-%02d",
    			       timer->wakeup_year  , timer->wakeup_month, timer->wakeup_day );
			OSD_SetTextFieldContent(&TextFieldbd1, STRING_ANSI, (UINT32)str);


			ComUniStrCopy(prog_name,(UINT16*)timer->event_name);
			OSD_SetTextFieldContent(&TextFieldbd4, STRING_UNICODE, (UINT32)prog_name);

			cur_channel = get_prog_pos(timer->wakeup_channel);
			sprintf(prog_name, "%03d:",cur_channel+1);
			//ComUniStrCopy(prog_name,(UINT16*)timer->wakeup_chan_mode);
			OSD_SetTextFieldContent(&TextFieldbd3, STRING_ANSI, (UINT32)prog_name);


			pe = epg_get_cur_service_event((INT32)cur_channel, PRESENT_EVENT, NULL, NULL, NULL, FALSE);
			//pe = epg_get_cur_service_event((INT32)cur_channel, SCHEDULE_EVENT, &(timer->wakeup_time), &(timer->wakeup_duration_time+timer->wakeup_time ), NULL, FALSE);
			s1 = (UINT8*)epg_get_event_name(pe, prog_name, 32);
			get_event_start_time(pe, &start_time);
			get_event_end_time(pe, &end_time);


			get_STC_offset(&h, &m, &sec);


			convert_time_by_offset(&start_time, &start_time, h, m);
			convert_time_by_offset(&end_time, &end_time, h, m);
			
			start_time.hour=timer->wakeup_time / 3600;
			start_time.min=timer->wakeup_time/60%60;
			start_time.sec=timer->wakeup_time%60;
			end_time.hour=(timer->wakeup_duration_time+timer->wakeup_time )/ 3600;
			end_time.min=(timer->wakeup_duration_time+timer->wakeup_time )/60%60;
			end_time.sec=(timer->wakeup_duration_time+timer->wakeup_time )%60;
			//pe = epg_get_cur_service_event((INT32)cur_channel, SCHEDULE_EVENT, &start_time, &end_time, NULL, TRUE);
			///s1 = (UINT8*)epg_get_event_name(pe, prog_name, 32);
			convert_time_by_offset(&start_time, &start_time,- h, -m);
			convert_time_by_offset(&end_time, &end_time, -h, -m);

			pe = epg_get_cur_service_event((INT32)cur_channel, SCHEDULE_EVENT, &start_time, &end_time, NULL, TRUE);
			s1 = (UINT8*)epg_get_event_name(pe, prog_name, 32);
			ComUniStrCopyChar((UINT8*) &display_strs[15][0], s1);
			//win_book_detail_display(2,&current_hl_item);
			break;
		case EVN_POST_OPEN:
			break;
		case EVN_PRE_DRAW:
			break;
		case EVN_PRE_CLOSE:
			*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
			
			break;
		case EVN_POST_CLOSE:
			
			break;
	}
	return ret;
}

