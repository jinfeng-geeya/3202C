/*
 * Copyright (C) ALi Shanghai Corp. 2005
 *
 * File name	: p_search.h
 * Description	: program search via service informations declarations.
 *
 * History:
 *	...
 * 1. 20050519	0.5	Zhengdao Li	Add NIT parsing support.
 * 2. 20100531	0.6	Yun You	Add NIT search for DVB-T.
 */
#ifndef __P_SEARCH_H__
#define __P_SEARCH_H__

#include <types.h>
#include <sys_config.h>
#include <api/libsi/si_config.h>
#include <api/libtsi/sec_pmt.h>
#include <api/libtsi/sec_sdt.h>
#include <api/libtsi/db_3l.h>


#define P_SEARCH_BUFF_SIZE    		64
#define P_MAX_AUDIO_NUM		  	MAX_AUDIO_CNT
//138-a14, has 9 ecm
#define P_MAX_CA_NUM		  	16

enum {
	P_SEARCH_FTA		  	= 0x0001,
	P_SEARCH_SCRAMBLED		= 0x0002,
	P_SEARCH_RADIO			= 0x0004,
	P_SEARCH_TV			= 0x0008,
	P_SEARCH_DOMTESTIC_PATCH	= 0x0010,
	P_SEARCH_NIT			= 0x0020,
	P_SEARCH_BAT			= 0x0040,
	P_SEARCH_SATTP			= 0x0080,
	P_SEARCH_NVOD			= 0x0100,
	P_SEARCH_ALL			= 0x000F,
	P_SEARCH_DATA			= 0x0200,
	P_SEARCH_SDT_OTH		= 0x0400,
};

#define __MM_PS_SECTION_ADDR		((__MM_VBV_START_ADDR+DMX_SI_PSI_SIZE)&0x8FFFFFFF) /* prefer to use cachable memory */
#define SECTION_TABLE_LEN		4096

/* define a struct for hold data from program searching */
typedef struct prog_info
{
	UINT16	prog_number;

	UINT8	service_type;
	UINT8	audio_count;
	UINT16  audio_pid[P_MAX_AUDIO_NUM];
	UINT8	audio_lang[P_MAX_AUDIO_NUM][3];
	UINT16	video_pid;
    UINT32  cur_audio;//audio idx selection

	UINT16  teletext_pid;
	UINT16  subtitle_pid;

	UINT8	av_flag;
	UINT8   EIT_flag;

	CA_INFO CA_info[P_MAX_CA_NUM];
	UINT8 CA_count;

	UINT16 pcr_pid;
	UINT16 pmt_pid;
	UINT8 pmt_version;
	UINT8 pmt_status;
	UINT8 sdt_status;
	UINT16 sat_id;
	UINT32 	tp_id;
#ifdef DB_USE_UNICODE_STRING
	UINT8 service_name[2*(MAX_SERVICE_NAME_LENGTH + 1)];
#if (SERVICE_PROVIDER_NAME_OPTION>0)
	UINT8 service_provider_name[2*(MAX_SERVICE_NAME_LENGTH + 1)];
#endif
#else
	UINT8 service_name [ MAX_SERVICE_NAME_LENGTH + 1 ];
#if (SERVICE_PROVIDER_NAME_OPTION>0)
	UINT8 service_provider_name[MAX_SERVICE_NAME_LENGTH + 1];
#endif
#endif

#if (SYS_PROJECT_FE == PROJECT_FE_DVBC)
	UINT16 ref_sid;
	UINT16 bouquet_id;
	UINT16 logical_channel_num;
	UINT16 volume;
	UINT16 track;
#endif

#if(defined( _MHEG5_ENABLE_) ||defined(_LCN_ENABLE_) || defined(_MHEG5_V20_ENABLE_))
	UINT8 mheg5_exist;
	UINT16 lcn;
	UINT8 lcn_true;
#endif
#ifdef _SERVICE_ATTRIBUTE_ENABLE_
	UINT8 numeric_selection_flag;//get from NIT service_attribute_descriptor, private descriptor defined in UKDTT
	UINT8 visible_service_flag;
#endif

}PROG_INFO;

typedef struct stream_info
{
	UINT16 nit_pid;
	UINT16 transport_stream_id;
}STREAM_INFO;

typedef union {
	struct {
		UINT16 tsid;
		UINT16 onid;
	} common;
	
	struct {
		UINT16 tsid;
		UINT16 onid;
		UINT16 frequency;
		UINT16 symbol_rate;
		UINT16 position;
		UINT8 polarity;
		UINT8 FEC_inner;
	}s_info;
	
	struct {
		UINT16 tsid;
		UINT16 onid;
		UINT32 frequency;
		UINT16 symbol_rate;
		UINT16 modulation;
		UINT8 FEC_outer;
		UINT8 FEC_inner;
	}c_info;

}TP_INFO;

typedef UINT8 (*prog_search_progress_callback)(UINT8 progress, void *node);


typedef void (*check_nim_t)();

#ifdef __cplusplus
extern "C"{
#endif

INT32 get_section(UINT8 *buff,INT16 buff_len,UINT16 pid, UINT32 *crc);

INT32 parse_pat_section(UINT8 *buff,INT16 buff_len);

INT32 parse_pmt_section(UINT8 *buff,INT16 buff_len,PROG_INFO *p_info);

INT32 parse_sdt_section(UINT8 *buff,INT16 buff_len,PROG_INFO *p_info,UINT8 node_cnt);

void stop_prog_search();

void info2db(PROG_INFO *p_info, P_NODE *p_node);

INT32 async_prog_search(UINT16 sat_id, UINT32 tp_id, UINT8 search_type,
	prog_search_progress_callback progress_callbck, UINT8 add_flag, check_nim_t check_nim);

#ifdef BOU_AND_LOGIC_NUM_SUPPORT
INT32 get_si_section(UINT8 *buff, INT16 *buf_length, UINT16 pid, UINT8 tb_id, UINT16 spec_id);
INT32 check_service_and_logical(UINT16 ser_id, PROG_INFO *p_info);

#endif

#ifdef __cplusplus
}
#endif

#endif /* __P_SEARCH_H__ */

