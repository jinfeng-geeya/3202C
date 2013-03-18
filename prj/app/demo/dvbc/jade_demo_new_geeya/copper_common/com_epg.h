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
#ifndef _EPG_DEMO_H_
#define _EPG_DEMO_H_

//#include <api/libtsi/sec_eit.h>
#include <api/libsi/lib_epg.h>

//eit_event_info_t* epg_get_cur_service_event(INT32 prog_idx, UINT32 event_type,date_time *dt,INT32 *event_num);
eit_event_info_t *epg_get_cur_service_event(INT32 prog_idx, UINT32 event_type, date_time *start_dt, date_time *end_dt, INT32 *event_num, BOOL update);
//UINT8* epg_get_event_name(eit_event_info_t *ep,INT32* len);
UINT16 *epg_get_event_name(eit_event_info_t *ep, UINT16 *name, INT32 max_len);

INT32 epg_get_event_start_time(eit_event_info_t *ep, UINT8 *str);
INT32 epg_get_event_end_time(eit_event_info_t *ep, UINT8 *str);
UINT8 *epg_get_event_language(eit_event_info_t *ep);
INT32 epg_get_event_content_type(eit_event_info_t *ep, UINT8 *str, INT32 *len);

UINT32 epg_get_event_all_short_detail(eit_event_info_t *ep, UINT16 *short_buf, UINT32 maxLen);
UINT32 epg_get_event_all_extented_detail(eit_event_info_t *ep, UINT16 *extented_buf, UINT32 maxLen);
/*
UINT8* epg_get_event_id(eit_event_info_t *ep,UINT8* str,INT32* len);
INT32 epg_get_event_detail_flag(eit_event_info_t *ep);
UINT8* epg_get_event_detail(eit_event_info_t *ep,INT32* len);
UINT8* epg_get_event_extented_detail(eit_event_info_t *ep,INT32 *len,INT32 idx);
UINT8* epg_get_event_short_detail(eit_event_info_t *ep,INT32 *len,INT32 idx);
INT32 epg_get_event_extented_desc_number(eit_event_info_t *ep);
INT32 epg_get_event_short_desc_number(eit_event_info_t *ep);
INT32 epg_get_event_start_time(eit_event_info_t *ep,UINT8* str);
INT32 epg_get_event_end_time(eit_event_info_t *ep,UINT8* str);
INT32 epg_get_event_time(int nType, eit_event_info_t *ep,UINT8* str);
INT32 epg_get_event_rating(eit_event_info_t* ep,UINT8* str,INT32* len);
INT32 epg_get_event_content_type(eit_event_info_t* ep,UINT8 *str,INT32* len);
INT32 epg_get_event_ca_id(eit_event_info_t *ep,UINT8 *str,INT32* len);	
INT32 epg_get_event_component(eit_event_info_t *ep,UINT8 *str,INT32 *len);
 */
#endif /*_EPG_DEMO_H_*/
