#ifndef _LIB_NVOD_H
#define _LIB_NVOD_H

#include <types.h>
#include <sys_config.h>

/*change this according to nvod tp count*/
#define NVOD_MAX_TP_CNT			4

#define NVOD_TP_REF_SERVICE_CNT	8	
#define NVOD_TP_SERVICE_CNT			8

#define NVOD_MAX_REF_SERVICE_CNT	(NVOD_MAX_TP_CNT*NVOD_TP_REF_SERVICE_CNT)
#define NVOD_MAX_SERVICE_CNT		(NVOD_MAX_TP_CNT*NVOD_TP_SERVICE_CNT)
/*change this according to total referece event count*/
#define NVOD_MAX_REF_EVNET_CNT	(256)
#define NVOD_MAX_EVENT_CNT		(NVOD_MAX_SERVICE_CNT*256)
#define NVOD_MAX_VIEW_CNT			512

//actual schedule 0x50~0x51
#define MAX_EIT_TABLE_CNT	2


#define NVOD_MODE_SINGLE_TP	0x0
#define NVOD_MODE_MULTI_TP	0x1

typedef void (*nvod_callback)(UINT8 type, UINT32 param);

typedef void (*nvod_eit_complete)();

enum{
    NVOD_UPDATE_REF_EVENT	    = 1,
    NVOD_ALL_REF_EVENT_OK	    = 2,
    NVOD_UPDATE_EVENT_SCH	    = 3,
    NVOD_ALL_EVENT_SCH_OK	    = 4,
    NVOD_TIMEOUT                        = 5,
    NVOD_NO_INFO                       = 6,
    NVOD_SCAN_OVER                  = 7,
    NVOD_DIRECT_UPDATE_EVENT    = 8,
}NVOD_MSG_TYPE;


enum{
    NVOD_STATUS_NULL = 0,
    NVOD_STATUS_INITED, 
    NVOD_STATUS_FIRST_OPEN,
    NVOD_STATUS_IN_SCAN,
    NVOD_STATUS_SCAN_OVER,

}NVOD_STATUS;


struct nvod_event
{
	UINT16 event_id;
	UINT16 service_id;
	UINT16 t_s_id;
	
	UINT16 ref_event_id;
	UINT16 ref_service_id;
	
	//year, moth, day mjd ; hour,min,second bcd
	UINT8 time[8];
};

struct nvod_ref_event
{
	UINT16 event_id;
	//reference service id
	UINT16 ref_service_id;
	UINT16 t_s_id;
	
	UINT8 name_len;
	UINT8 txt_len;
	UINT8 lang_code[3];
	UINT8 name[40];
	UINT8 txt[256];

};

struct nvod_service_id
{
	UINT16 sid;
	UINT16 t_s_id;
	UINT16 origin_netid;
	UINT16 ref_id;
};

struct nvod_sdt_info{
	UINT16 ref_cnt;
  	UINT16 ref_id[NVOD_TP_REF_SERVICE_CNT];
	UINT16 scnt;
	struct nvod_service_id sid[NVOD_TP_SERVICE_CNT];
};

struct subtable_hitmap
{
	UINT32 seg_full_value;
	//each bit for one segment 
	UINT32 seg_flg;
	UINT8 version;
	//32 segment, each segment max 8 sections
	UINT8 sec_flg[32];
};

struct eit_hitmap
{
	UINT8 last_tb_id;
	struct subtable_hitmap tables[MAX_EIT_TABLE_CNT];
};

struct nvod_tp{
	UINT16 transport_stream_id;
	UINT16 origin_netid;
	
	UINT16 ref_service_cnt;
	//each bit indicate one ref service eit 0x4e all sections parsed
	UINT32 ref_services_full_value;
	UINT32 ref_services_parsflg;
	UINT16 ref_service_id[NVOD_TP_REF_SERVICE_CNT];
	struct subtable_hitmap ref_eit_hitmap[NVOD_TP_REF_SERVICE_CNT];
	struct subtable_hitmap ref_eit_parsflg[NVOD_TP_REF_SERVICE_CNT];
	
	UINT16 service_cnt;
	/*each uint32 indicate one eit sub-table, each bit indicate one service's corresponding sub-table
	all sections parsed*/
	UINT32 eit_timeout_flag;
	UINT32 services_full_value[MAX_EIT_TABLE_CNT];
	UINT32 services_parsflg[MAX_EIT_TABLE_CNT];
	struct nvod_service_id service_id[NVOD_TP_SERVICE_CNT];
	struct eit_hitmap service_eit_hitmap[NVOD_TP_SERVICE_CNT];
	struct eit_hitmap service_eit_parsflg[NVOD_TP_SERVICE_CNT];
};


struct prog_id_map{
	UINT16 service_id;
	UINT16 t_s_id;
	UINT32 db_prog_id;
};

struct nvod_info{
	//128K byte ring buffer, last 4K for dmx get section
	/*change this buf size according to eit section count*/
	UINT8 eit_buf[4096*17];

	//UINT8 status;
	nvod_callback call_back;
	nvod_eit_complete eit_complete;

	//currently used tp node and tp nvod info
	T_NODE cur_tp;
	struct nvod_tp nvod_tp;
	
	UINT16 tp_cnt;
	T_NODE tp[NVOD_MAX_TP_CNT];
	struct nvod_tp tp_eitbitmap[NVOD_MAX_TP_CNT];
	
	UINT16 service_cnt;
	//P_NODE service[NVOD_MAX_SERVICE_CNT];
	struct prog_id_map service[NVOD_MAX_SERVICE_CNT];

	UINT16 ref_event_cnt;
	struct nvod_ref_event ref_event[NVOD_MAX_REF_EVNET_CNT];

	UINT16 event_cnt;
	struct nvod_event event[NVOD_MAX_EVENT_CNT];

	UINT16 view_node_cnt;
	UINT32 view_node_time[NVOD_MAX_VIEW_CNT];
	struct nvod_event *view[NVOD_MAX_VIEW_CNT];
	

	/****patch for Xuyi special case********/
	#if 0
	//current service's event's reference event list
	UINT16 cur_service_event_cnt;
	UINT32 cur_service_event_time[NVOD_MAX_REF_EVNET_CNT];
	struct nvod_event *cur_service_event_list[NVOD_MAX_REF_EVNET_CNT];

	//now playing event list
	struct nvod_event *playing_event_list[NVOD_MAX_SERVICE_CNT];
	#endif
	/*********************************/
};


struct tp_param{
	UINT32 freq;
	UINT16 symbol;
	UINT16 constellation;
};
	

/****API for external use********************************************/
#ifdef NVOD_FEATURE

//API for search
INT32 nvod_scan_start(T_NODE *tp,struct nvod_sdt_info * sdt_info);
INT32 nvod_save_program(PROG_INFO *info, T_NODE *cur_tp);

//API for play channel
    //direction 0 save tp eitbitmap info; 1 load tp eitbitmap info
INT32 nvod_saveorload_tp_eit(UINT8 direction,T_NODE *tp);


//API for application
//INT32 nvod_start_eit(BOOL search_mode);
UINT16 nvod_get_ref_event_cnt();
INT32 nvod_get_ref_event(UINT16 pos, struct nvod_ref_event *ref);
INT32 nvod_create_refevent_timelist(UINT16 ref_event_id, UINT16 ref_service_id, UINT16 t_s_id, UINT16 *cnt);
INT32 nvod_get_refevent_timelist(UINT16 start_pos, UINT16 cnt, struct nvod_event *buf, UINT16 *actual_cnt);
INT32 nvod_get_channel_progid(UINT16 service_id, UINT16 t_s_id,UINT32 *prog_id);
INT32 nvod_get_cur_event(UINT16 service_id, UINT16 ref_service_id,UINT16 t_s_id,struct nvod_event *event);
INT32 nvod_rearrange_event();

/*
INT32 nvod_play_channel(UINT16 service_id);
INT32 nvod_stop_channel(UINT8 black_screen);
INT32 nvod_reset_cur_channel();
INT32 nvod_get_channel(UINT16 service_id, P_NODE *prog);
INT32 nvod_modify_channel(UINT16 service_id, P_NODE *prog);
*/
INT32 nvod_get_tp_entry(T_NODE *tp);

INT32 nvod_open(UINT8 mode, struct tp_param *t_param, nvod_callback callback,nvod_eit_complete eit_complete);
INT32 nvod_close();

INT32 nvod_init(UINT32 frq, UINT32 symbol, UINT8 qam);
INT32 nvod_release();

#else
#define nvod_scan_start(...)                do{}while(0)
#define nvod_save_program(...)          do{}while(0)
#define nvod_saveorload_tp_eit(...)     do{}while(0)

#endif


#endif
