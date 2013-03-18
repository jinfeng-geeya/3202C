/*========================================================================================
 *
 * Copyright (C) 2011 ALi Shanghai Corp.
 *
 * Description:
 *
 * History
 *
 *   Date	    Author		Comment
 *   ========	===========	========================================
 *   ......
 *   2011/7/5    Steven     Re-arrange print message.
 *
 *=========================================================================================*/

#include <retcode.h>
#include <api/libc/string.h>
#include "epg_common.h"
#include "eit_parser.h"
#include "epg_db.h"


//#define _DEBUG

#define SHORT_EVENT_DESC  		0x4D
#define	EXTENTED_EVENT_DESC 	0x4E
#define COMPONENT_DESC			0x50
//#define CA_DESC					0x53
#define CONTENT_DESC			0x54
#define PARENTAL_RATING_DESC	0x55
//#define TELE_TEXT_DESC			0x57
#define PRIVATE_DATA_SPECIFIER_DESC	0x5F
#define PREFERRED_NAME_ID_DESC             0x85
//#define EIT_EVENT_ITEM_PARSER
//#define DTG_PVR

//Data_broadcast_descriptor
//Linkage descriptor
//Multilingual component descriptor
//PDC descriptor
//Time shifted event descriptor

static INLINE UINT8 hex2bcd(UINT8 hex)
{
	return ((hex >> 4) * 10 + (hex & 0x0f));
}

static INLINE UINT8 ch_index(UINT8 ch)
{
	if (ch >= 'a' && ch <= 'z')
		return ch - 'a';
	else if (ch >= 'A' && ch <= 'Z')
		return ch - 'A';
	else
	{
		EIT_PRINT("%s: char (%c) error, not within [a,z]/[A,Z]!\n",__FUNCTION__,ch);
		return 0;	//error
	}
}
static UINT16 lang3tolang2(UINT8* lang)	//3 byte to 2 byte
{
	return ((ch_index(lang[0])<<10) | (ch_index(lang[1])<<5) | ch_index(lang[2]));
}
static UINT8 lang[3];
UINT8 *lang2tolang3(UINT16 lang_code2)	//2 byte to 3 byte
{
	lang[0] = (lang_code2>>10) + 'a';
	lang[1] = ((lang_code2>>5) & 0x1F) + 'a';
	lang[2] = (lang_code2 & 0x1F) + 'a';
	return lang;
}
//eit short/extend event descriptor
struct EIT_EVENT_DESC
{
	//1 short
	UINT8 *lang;
	UINT8 *sht_text;
	UINT8 sht_len;

	//16 extend at most
	struct EIT_EXT_DESC
	{
		UINT8 *text;
		UINT8 len;
	} ext[16];
};

//parse short event descriptor
static INT32 eit_get_sht_desc(struct EIT_EVENT_DESC *pDsc, UINT8 dsc_cnt, UINT8* desc_buf, UINT16 buf_len)
{
	EIT_PRINT("		*short event descriptor\n");

#ifdef _DEBUG
	UINT8 event_name_len, event_text_len;
	UINT8 lang_code[3];
	UINT8 event_name[256];
	UINT8 text_desc[256];

	UINT8 *p = desc_buf;

	p += 2;
	lang_code[0] = *p++;
	lang_code[1] = *p++;
	lang_code[2] = *p++;
	EIT_PRINT("lang_code: %c%c%c\n",lang_code[0],lang_code[1],lang_code[2]);

	event_name_len = *p++;
	
	MEMCPY(event_name, p, event_name_len);
	event_name[event_name_len] = '\0';
	EIT_PRINT("event_name: %s\n",event_name);

	p += event_name_len;
	event_text_len = *p++;

	if (event_text_len > 0)
	{
		MEMCPY(text_desc, p, event_text_len);
		text_desc[event_text_len] = '\0';
		EIT_PRINT("text_desc: %s\n",text_desc);
	}
#endif

	int i, n = -1;
	UINT8 *lang = desc_buf+2;

	if (*(desc_buf+1) + 2 <= buf_len && *(desc_buf+1) >= 3)
	{
		for (i=0; i<dsc_cnt; i++)
		{
			if (pDsc[i].lang != NULL
				&& MEMCMP(lang, pDsc[i].lang, 3) == 0)
			{
				EIT_PRINT("short event descriptor (%c%c%c) already exists!!\n",desc_buf[2],desc_buf[3],desc_buf[4]);
                if (pDsc[i].sht_text != NULL)
				    return ERR_FAILUE;
				else
				{
                    //maybe sometime get ext_desc before sht_desc
                    n = i;
                    break;
				}
			}
			else if (pDsc[i].lang == NULL && n == -1)
				n = i;
		}
		if (n != -1)
		{
			pDsc[n].sht_len = *(desc_buf+1) - 3;
			pDsc[n].lang = lang;
			pDsc[n].sht_text = desc_buf + 5;
			EIT_PRINT("len = %d, lang = %c%c%c\n",pDsc[n].sht_len,lang[0],lang[1],lang[2]);
			return SUCCESS;
		}
		else
		{
			EIT_PRINT("descriptor table full!!\n");
		}
	}
	else
	{
		EIT_PRINT("desc length error: %d\n",*(desc_buf+1));
	}

	return ERR_FAILUE;
}

//parse extended event descriptor
static INT32 eit_get_ext_desc(struct EIT_EVENT_DESC *pDsc, UINT8 dsc_cnt, UINT8* desc_buf, UINT16 buf_len)
{
	UINT8 item_len;
	UINT8 desc_num;
	UINT8 *lang = desc_buf + 3;
	int i, n = -1;

	EIT_PRINT("		*extended event descriptor\n");

	if (*(desc_buf+1) + 2 > buf_len || *(desc_buf+1) < 3)
	{
		EIT_PRINT("desc length error: %d\n",*(desc_buf+1));
		return ERR_FAILUE;
	}

	desc_num = desc_buf[2] >> 4;
	lang = desc_buf + 3;
 	n = -1;

    //soc_printf("lang: %c%c%c\n",lang[0],lang[1],lang[2]);
	for (i=0; i<dsc_cnt; i++)
	{
		if (pDsc[i].lang != NULL
			&& MEMCMP(lang, pDsc[i].lang, 3) == 0)
		{
			n = i;
			break;
		}
		else if (pDsc[i].lang == NULL && n == -1)
			n = i;
	}
	if (n != -1)
	{
		if (pDsc[n].lang == NULL)
			pDsc[n].lang = lang;
		item_len = *(desc_buf+6);
		pDsc[n].ext[desc_num].len = *(desc_buf+item_len+7);
		pDsc[n].ext[desc_num].text = desc_buf+item_len+8;
        EIT_PRINT("lang: %c%c%c, desc_num: %d, len: %d\n",lang[0],lang[1],lang[2],desc_num,pDsc[n].ext[desc_num].len);
		return SUCCESS;
	}
	else
	{
		EIT_PRINT("descriptor table full!!\n");
	}

	return ERR_FAILUE;
}

//parse content descriptor
static INLINE INT32 eit_get_content_desc(UINT8 *content_nibble1, UINT8* desc_buf, UINT16 buf_len)
{
	EIT_PRINT("		*content descriptor\n");
	*content_nibble1 = (*(desc_buf+2)) >> 4;
	EIT_PRINT("content nibble 1: %d\n",*content_nibble1);

#ifdef _DEBUG
	UINT8 *p = desc_buf + 2;
	UINT8 content_nibble2, user_nibble1, user_nibble2;

	do
	{
		*content_nibble1 = (*p) >> 4;
		content_nibble2 = 0x0F & (*p);
		user_nibble1 = (*(p+1)) >> 4;
		user_nibble2 = 0x0F & (*(p+1));
		EIT_PRINT("content_nibble1: %d\n",*content_nibble1);
		EIT_PRINT("content_nibble2: %d\n",content_nibble2);
		EIT_PRINT("user_nibble1: %d\n",user_nibble1);
		EIT_PRINT("user_nibble2: %d\n",user_nibble2);

		p += 2;

	} while (p < desc_buf+buf_len);
#endif

	return SUCCESS;
}

//parse parent rating descriptor
#ifdef PARENTAL_SUPPORT
static INLINE INT32 eit_get_pr_desc(UINT8 *rating, UINT8* desc_buf, UINT16 buf_len)
{
	EIT_PRINT("		*parent rating descriptor\n");
	*rating = (*(desc_buf+5)) & 0x0F;	//ignore high 4 bits.
	EIT_PRINT("parent rating: %d\n",*rating);

#ifdef _DEBUG
	UINT8 *p = desc_buf + 2;
	UINT8 country_code[3];

	do
	{
		country_code[0] = *p;
		country_code[1] = *(p+1);
		country_code[2] = *(p+2);
		EIT_PRINT("country_code: %c%c%c\n",country_code[0],country_code[1],country_code[2]);
		EIT_PRINT("rating: %d\n",*rating);

		p += 4;

	} while (p < desc_buf+buf_len);
#endif

	return SUCCESS;
}
#endif

//parse Component descriptor
#ifdef COMPONENT_SUPPORT
static INLINE INT32 eit_get_component_desc(UINT8 *stream_content, UINT8 *component_type, UINT8* desc_buf, UINT16 buf_len)
{
	*stream_content = desc_buf[2] & 0xF;
	*component_type = *(desc_buf + 3);
	EIT_PRINT("		*component descriptor\n");
	EIT_PRINT("stream_content: %d\n",*stream_content);
	EIT_PRINT("component_type: %d\n",*component_type);

#ifdef _DEBUG
	UINT8 *p = desc_buf;
	UINT8 component_tag;
	UINT8 lang_code[3];
	UINT8 text_char[256];
	UINT8 N;

	component_tag = *(p + 4);
	EIT_PRINT("component_tag: %d\n",component_tag);
	lang_code[0] = *(p + 5);
	lang_code[1] = *(p + 6);
	lang_code[2] = *(p + 7);
	EIT_PRINT("lang_code: %c%c%c\n",lang_code[0],lang_code[1],lang_code[2]);
	N = *(p+1) - 6;
	MEMCPY(text_char, p + 8, N);
	text_char[N] = '\0';
	EIT_PRINT("text_char[%d]: %s\n",N,text_char);
#endif

	return SUCCESS;
}
#endif

static UINT8 buff[4096];	//event text buffer
//copy short/extend event descriptors' lang/name/text to the event node
static void process_event_desc(struct DB_NODE *node,
								struct EIT_EVENT_DESC *pDes,
								UINT8 desc_cnt)
{
	int i, j, n = 0/*language number of node*/;
	UINT8 *text_char, *ext_text_char = NULL;
	UINT16 text_length, ext_text_len;
	text_char = buff;
	UINT16 sht_text_length;

	EIT_PRINT("%s: lang cnt = %d\n",__FUNCTION__,desc_cnt);
	for (i=0; i<desc_cnt; i++)
	{
		EIT_PRINT("check lang(%d)\n",i);

		if (pDes[i].lang != NULL && n < MAX_EPG_LANGUAGE_COUNT)
		{
//			MEMCPY(node->lang[n].lang_code, pDes[i].lang, 3);
			node->lang[n].lang_code2 = lang3tolang2(pDes[i].lang);
			text_length = 0;
			sht_text_length = 0;
			ext_text_len = 0;
            ext_text_char = NULL;
			//short event descriptor
			if (pDes[i].sht_len != 0 && pDes[i].sht_text != NULL)
			{
				MEMCPY(text_char, pDes[i].sht_text, pDes[i].sht_len);
				text_char += pDes[i].sht_len;
				text_length = pDes[i].sht_len;
				sht_text_length = text_length;
			}
			//extend event descriptor
			for (j=0; j<16; j++)
			{
				if (pDes[i].ext[j].len == 0 || pDes[i].ext[j].text == NULL)
					continue;

				if (text_length == 0)	//no short event descriptor
				{
					text_char[0] = text_char[1] = 0;
					text_char += 2;
					text_length = 2;
					sht_text_length = text_length;
				}
				//if (j == 0)	//first extend event descriptor
				if (ext_text_char == NULL)
				{
					ext_text_char = text_char;
					text_char += 2;
					text_length += 2;
				}
				if (0 != pDes[i].ext[j].len)
				{
					MEMCPY(text_char, pDes[i].ext[j].text, pDes[i].ext[j].len);
					text_char += pDes[i].ext[j].len;
					ext_text_len += pDes[i].ext[j].len;
					text_length += pDes[i].ext[j].len;
				}
			}
			//node: text_char & text_length
			if (text_length > 0)
			{
				if (ext_text_len == 0)	//no extend event descriptor
				{
					ext_text_char = text_char;
					text_char += 2;
					text_length += 2;
				}
				//extend event descriptor text length
				ext_text_char[0] = ext_text_len >> 8;
				ext_text_char[1] = ext_text_len & 0xFF;
				//all event name + short text + extend text
				node->lang[n].text_char = text_char - text_length;
			}
			else
				node->lang[n].text_char = NULL;
			node->lang[n].text_length = text_length;

			EIT_PRINT("lang(%d) (%c%c%c), short len = %d, extend len = %d\n",n,pDes[i].lang[0],pDes[i].lang[1],pDes[i].lang[2],
						sht_text_length,ext_text_len);

			n++;
		}
		else
		{
			ERR_PRINT("lang addr (0x%X) error or count(%d) flow(max count %d)!\n",pDes[i].lang,n,MAX_EPG_LANGUAGE_COUNT);
		}
	}
}

//parse event buffer
static INT32 eit_get_event_info(UINT32 tp_id, UINT16 service_id, UINT8 table_id,
								UINT8* event_buf, UINT32 event_len, UINT8 event_type, UINT8 sec_num)
{
	UINT16 event_loop_start = 0;
	UINT16 desc_loop_start = 0;
	UINT16 desc_loop_len;

	UINT8 desc_len;
	UINT8 desc_tag;
	UINT8 *p = event_buf;

	struct DB_NODE event_node;
	UINT8 nibble;
	UINT8 rating;
	UINT8 stream_content;
	INT32 ret = ERR_FAILED;
	
	struct EIT_EVENT_DESC Desc[MAX_EPG_LANGUAGE_COUNT];

	if(event_len < 12)
	{
		EIT_PRINT("[eit] error: tp 0x%X, service 0x%X, table 0x%X, sec %d, event len %d < 12\n",tp_id,service_id,table_id,sec_num,event_len);
		return !SUCCESS;
	}

	//process event loop
	do
	{
		//init event node
		MEMSET(&event_node, 0, sizeof(event_node));
#ifdef EPG_MULTI_TP
		event_node.tp_id = tp_id;
#endif
		event_node.service_id = service_id;
		event_node.tab_num = table_id & 0x0F;	//just table number
		event_node.event_type = event_type;

		//get event info
		event_node.event_id = *p<<8 | *(p+1);
		p += 2;
		EIT_PRINT(">>>>> [eit] event: 0x%X begin\n",event_node.event_id);

		event_node.mjd_num = *p<<8 | *(p+1);
		p += 2;
		
		event_node.time.hour = hex2bcd(*p++);
		event_node.time.minute = hex2bcd(*p++);
		event_node.time.second = hex2bcd(*p++);

		event_node.duration.hour = hex2bcd(*p++);
		event_node.duration.minute = hex2bcd(*p++);
		event_node.duration.second = hex2bcd(*p++);
		
		desc_loop_len = ((*p&0x0f)<<8) | *(p+1);
		p += 2;
		EIT_PRINT("[eit] desc loop length: %d\n",desc_loop_len);

		event_loop_start += 12;
		if ((UINT32)(event_loop_start+desc_loop_len) > event_len)
		{
			ERR_PRINT("[eit] desc loop length overflow!\n");
			ERR_PRINT("[eit] loop start %d, desc loop length %d, event length %d\n",event_loop_start,desc_loop_len,event_len);
			return !SUCCESS;
		}

		//process event's desc loop
		desc_loop_start = 0;
		MEMSET(Desc, 0, sizeof(Desc));
		while (desc_loop_start < desc_loop_len)
		{
			desc_tag = *p;
			desc_len = *(p+1);
			//EIT_PRINT("[eit] descriptor tag: 0x%X\n",desc_tag);
			//EIT_PRINT("[eit] descriptor length: %d\n",desc_len);

			if (desc_loop_start+desc_len+2 <= desc_loop_len)
			{
				switch (desc_tag)
				{
					case SHORT_EVENT_DESC:
						eit_get_sht_desc(Desc, MAX_EPG_LANGUAGE_COUNT, p, desc_len+2);
						break;
					case CONTENT_DESC:
						eit_get_content_desc(&nibble, p, desc_len+2);
						event_node.nibble = nibble;
						break;
#ifdef PARENTAL_SUPPORT
					case PARENTAL_RATING_DESC:
						eit_get_pr_desc(&rating, p, desc_len+2);
//						event_node.parental[0].rating = rating;
						event_node.rating = rating;
						break;
#endif
					case EXTENTED_EVENT_DESC:
						eit_get_ext_desc(Desc, MAX_EPG_LANGUAGE_COUNT, p, desc_len+2);
						break;
#ifdef COMPONENT_SUPPORT
					case COMPONENT_DESC:
						eit_get_component_desc(&stream_content, &event_node.component_type, p, desc_len+2);
						event_node.stream_content = stream_content;
						break;
#endif
					default:
						EIT_PRINT("[eit] unknown descriptor tag: 0x%X\n",desc_tag);
						break;
				}
			}

			p += (desc_len+2);
			desc_loop_start += (desc_len+2);
		}

		//process short/extended event descriptor's language code ,event name & text char
		process_event_desc(&event_node, Desc, MAX_EPG_LANGUAGE_COUNT);

		//it's best to check epg db & del other tp events here!!!
		/*if (get_epg_db_status() == STATUS_UNAVAILABLE)
		{
			epg_del_service_events(epg_info.tp_id, NOT_EQUAL);
		}*/

		ret = add_event(&event_node);

		if (ret == ERR_FAILED || ret == ERR_DB_FULL)
		{
			return ret;
		}
		else if (ret == ERR_NO_MEM)
		{
			//TO DO...
		}

		EIT_PRINT("<<<<< [eit] event: 0x%X end\n",event_node.event_id);
		event_loop_start += desc_loop_len;
		
	} while(event_loop_start < event_len);

	return ret;
}

//parse eit section
INT32 eit_sec_parser(UINT32 tp_id, UINT8* buf, UINT32 len, EIT_CALL_BACK call_back)
{
	UINT8 table_id;
	UINT8 sec_num;
	UINT16 sec_len;
	UINT16 service_id;
	UINT8 event_type;
	INT32 ret;

#ifdef _DEBUG
	if (buf == NULL || len < 18 || len > 4096)
		return !SUCCESS;
#endif

	table_id = buf[0];
#ifdef _DEBUG
	if (!IS_PF(table_id) && !IS_SCH(table_id))
		return !SUCCESS;
#endif

	sec_len = ((buf[1]&0x0F)<<8) | buf[2];
#if 1//def _DEBUG
	if ((UINT32)(sec_len + 3) > len || sec_len < 15 || sec_len > 4093)
	{
		ERR_PRINT("section len %d overflow! (buffer len %d)\n",sec_len, len);
		return !SUCCESS;
	}
#endif

	service_id = (buf[3]<<8) | buf[4];
	sec_num = buf[6];

	event_type = IS_PF(table_id)?sec_num:SCHEDULE_EVENT;

	ret = eit_get_event_info(tp_id, service_id, table_id, buf+14, sec_len-15/*no CRC*/, event_type, sec_num);
	EIT_PRINT("[eit] parse event ret = %d\n",ret);

	if (SUCCESS == ret)
	{
		if (call_back != NULL)
			call_back(tp_id, service_id, event_type);
	}

	return ret;
}

