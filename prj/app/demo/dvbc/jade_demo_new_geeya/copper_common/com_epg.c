
/*****************************************************************************
 *    Copyright (C)2003 Ali Corporation. All Rights Reserved.
 *
 *    File:    epg_demo.c
 *
 *    Description:    This file describes epg application design.
 *    History:
 *           Date            Athor        Version          Reason
 *	    ============	=============	=========	=================
 *	1.	Sep.23.2003      Goliath Peng       Ver 0.1    Create file.
 *     2.     Jan.19.2004      Goliath Peng       Ver 0,2     Modified the get section mechanism
 *	3. 	Aug.12.2004 	   Firs Chen		Ver 1.0	 Modified the eit Data Structure and some api.
 *	4. 	Apr.17.2006 	   Sunny Yin		Ver 1.1	 move from rock
 *****************************************************************************/
#include <sys_config.h>

#include "com_epg.h"
//#include <api/libtsi/db_3l.h>
//#include <api/libtsi/sec_eit.h>
#include <api/libc/string.h>
//#include<api/libtsi/si_epg.h>
#include <api/libsi/lib_epg.h>
#include "system_data.h"
#include <osal/osal.h>
//#include <api/libosd/osd_common.h>
#include <api/libchar/lib_char.h>


static char *content_nibble[16] =
{
	" ",
	"Movie",
	"News",
	"Show/Game",
	"Sports",
	"Childen's",
	"Music",
	"Arts/Culture",
	"Social/Eco",
	"Edu/Science",
	"Leisure",
	"Other",
	"Other",
	"Other",
	"Other",
	"Other"
};

static UINT8 epg_lang_select = 0; //for multi-language
UINT16 *epg_get_event_name(eit_event_info_t *ep, UINT16 *name, INT32 max_len)
{
	UINT8 len;

	if (ep==NULL)	return NULL;

	if (ep->lang[epg_lang_select].text_char == NULL)	return NULL;

	len = ep->lang[epg_lang_select].text_char[0];
	if (len > 0)
	{
		dvb_to_unicode(ep->lang[epg_lang_select].text_char+1, len, name, max_len,
				get_iso639_refcode(lang2tolang3(ep->lang[epg_lang_select].lang_code2)));
		return name;
	}
	else
		return NULL;
}

static INT32 ConvertDateTimeToString(date_time *dt, UINT8 *str)
{
	UINT8 hour, min;
	INT32 hour_off, min_off, sec_off;
	date_time tmp;

	SYSTEM_DATA *sys_data;

	get_STC_offset(&hour_off, &min_off, &sec_off);

	convert_time_by_offset(&tmp, dt, hour_off, min_off);

	hour = tmp.hour;
	min = tmp.min;

	sprintf(str, "%02d:%02d", hour, min);
	return 5;
}

INT32 epg_get_event_start_time(eit_event_info_t *ep, UINT8 *str)
{
	date_time dt;

	if (ep == NULL)
		return 0;

	get_event_start_time(ep, &dt);

	return ConvertDateTimeToString(&dt, str);
}

INT32 epg_get_event_end_time(eit_event_info_t *ep, UINT8 *str)
{
	date_time dt;

	if (ep == NULL)
		return 0;

	get_event_end_time(ep, &dt);

	return ConvertDateTimeToString(&dt, str);
}

UINT8 *epg_get_event_language(eit_event_info_t *ep)
{
	if (NULL == ep)
		return NULL;

	return lang2tolang3(ep->lang[epg_lang_select].lang_code2);
}

INT32 epg_get_event_content_type(eit_event_info_t *ep, UINT8 *str, INT32 *len)
{
	if (ep==NULL) return !SUCCESS;

	STRCPY(str, content_nibble[ep->nibble]);
	*len = STRLEN(content_nibble[ep->nibble]);
	str[ *len] = '\0';

	return SUCCESS;
}

UINT32 epg_get_event_all_short_detail(eit_event_info_t *ep, UINT16 *short_buf, UINT32 maxLen)
{
	UINT8 *text;
	UINT16 len;

	if (ep==NULL)	return 0;

	if (ep->lang[epg_lang_select].text_char == NULL)	return 0;

	len = ep->lang[epg_lang_select].text_char[0]; //event name
	text = ep->lang[epg_lang_select].text_char + len + 1; //short text char
	len = text[0];
	if (len > 0)
	{
		return dvb_to_unicode(text+1, len, short_buf, maxLen,
							get_iso639_refcode(lang2tolang3(ep->lang[epg_lang_select].lang_code2)));
	}
	else
		return 0;
}

UINT32 epg_get_event_all_extented_detail(eit_event_info_t *ep, UINT16 *extented_buf, UINT32 maxLen)
{
	UINT8 *text;
	UINT16 len;

	if (ep==NULL)	return 0;

	if (ep->lang[epg_lang_select].text_char == NULL)	return 0;

	len = ep->lang[epg_lang_select].text_char[0]; //event name
	text = ep->lang[epg_lang_select].text_char + len + 1; //short text char
	len = text[0];
	text += (len + 1); //extended text char
	len = (text[0] << 8) | text[1];
	if (len > 0)
	{
		return dvb_to_unicode(text+2, len, extented_buf, maxLen,
							get_iso639_refcode(lang2tolang3(ep->lang[epg_lang_select].lang_code2)));
	}
	else
		return 0;
}

eit_event_info_t *epg_get_cur_service_event(INT32 prog_idx, UINT32 event_type, date_time *start_dt, date_time *end_dt, INT32 *event_num, BOOL update)
{
	P_NODE service_info;

	if (event_num != NULL)
		*event_num = 0;

	if (SUCCESS == get_prog_at(prog_idx, &service_info))
	{
		return epg_get_service_event(service_info.tp_id, service_info.prog_number,
									event_type, start_dt, end_dt, event_num, update);
	}
	else
		return NULL;
}
