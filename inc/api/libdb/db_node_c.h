#ifndef _DB_NODE_S_H_
#define _DB_NODE_S_H_

#define INVALID_POS_NUM	0xFFFF

#define MAX_SERVICE_NAME_LENGTH		17
#define MAX_SATELLITE_NAME_LENGTH		17

#define MAX_AUDIO_CNT	8
#define MAX_BOUQUET_CNT 8
#define MAX_CAS_CNT	4

#define AUDIO_DEFAULT_VOLUME		66//13//
#define AUDIO_CH_L				0x00
#define AUDIO_CH_R				0x01	
#define AUDIO_CH_STEREO			0x02
#define AUDIO_CH_MONO			0x03

#define S_NODE_FIX_LEN	24
#if (defined( _MHEG5_ENABLE_) || defined( _MHEG5_V20_ENABLE_) || defined( NETWORK_ID_ENABLE))
#define T_NODE_FIX_LEN	28
#else
#define T_NODE_FIX_LEN	16
#endif

#ifdef User_order_sort
#if(defined( _MHEG5_ENABLE_) ||defined(_LCN_ENABLE_) ||defined(_MHEG5_V20_ENABLE_))
#ifdef MULTI_BOUQUET_ID_SUPPORT
#define P_NODE_FIX_LEN	(28+4+8-2)
#else
#define P_NODE_FIX_LEN	(28+4+8)
#endif
#else
#ifdef MULTI_BOUQUET_ID_SUPPORT
#define P_NODE_FIX_LEN	(28+4-2)
#else
#define P_NODE_FIX_LEN	(28+4)
#endif
#endif
#else
#if(defined( _MHEG5_ENABLE_) ||defined(_LCN_ENABLE_) ||defined(_MHEG5_V20_ENABLE_))
#ifdef MULTI_BOUQUET_ID_SUPPORT
#define P_NODE_FIX_LEN	(28+8 -2)
#else
#define P_NODE_FIX_LEN	(28+8)
#endif
#else
#ifdef MULTI_BOUQUET_ID_SUPPORT
#define P_NODE_FIX_LEN	28-2
#else
#define P_NODE_FIX_LEN	28
#endif
#endif
#endif

#define FRQ_EDG	2
#define SYM_EDG 19


enum {
	LNB_CTRL_STD				= 0x00,	/* standard non dual-LNB	*/
	LNB_CTRL_POL				= 0x10,	/* dual LNB controlled by polar	*/
	LNB_CTRL_22K				= 0x20,	/* dual LNB controlled by 22k	*/
	LNB_CTRL_UNICABLE			= 0x30,	/* dual LNB controlled from unicable */
	LNB_CTRL_RESERVED			= 0x70	/* reserved control method	*/
};


typedef struct antena_t
{
	UINT8 lnb_power		: 1;
	UINT8 lnb_type		: 7;	

	UINT8 pol			: 2;
	UINT8 k22			: 1;
	UINT8 v12			: 1;
	UINT8 toneburst		: 2;	
	UINT8 reserve_1		: 2;
	
	UINT16 lnb_low;
	UINT16 lnb_high;

	UINT8 DiSEqC_type	: 4;
	UINT8 DiSEqC_port 	: 4;
	
	UINT8 DiSEqC11_type	: 4;
	UINT8 DiSEqC11_port 	: 4;
		
	UINT16 positioner_type	: 3;	
	UINT16 position		: 8;
	UINT16 reserve_2		: 5;

}/*__attribute__((packed))*/ANTENA;


typedef struct sat
{
	//for db index
	UINT16 sat_id;
	UINT16 reserve_id;
	
	UINT16 sat_orbit;
	
	UINT16 selected_flag	: 1;
	UINT16 tuner1_valid	: 1;
	UINT16 tuner2_valid	: 1;
	UINT16 reserve_1		: 13;
	
	//tuner1 antenna setting*****
	UINT8 lnb_power		: 1;
	UINT8 lnb_type		: 7;	

	UINT8 pol			: 2;
	UINT8 k22			: 1;
	UINT8 v12			: 1;
	UINT8 toneburst		: 2;	
	UINT8 reserve_2		: 2;
	
	UINT16 lnb_low;
	UINT16 lnb_high;

	UINT8 DiSEqC_type	: 4;
	UINT8 DiSEqC_port 	: 4;
	
	UINT8 DiSEqC11_type	: 4;
	UINT8 DiSEqC11_port 	: 4;
		
	UINT16 positioner_type	: 3;	
	UINT16 position		: 8;
	UINT16 reserve_3		: 5;
	//*************************
	
	ANTENA tuner2_antena;
	
	UINT16 name_len;
	UINT8 sat_name[2*(MAX_SERVICE_NAME_LENGTH + 1)];

}/*__attribute__((packed))*/ S_NODE;

typedef struct tp
{	
	UINT16 sat_id;
	DB_TP_ID tp_id;

	UINT32 frq;				
	UINT32 sym;				

	UINT16 pol					: 1;
	UINT16 FEC_inner				: 4;
	UINT16 nvod_flg                 	: 1;
	UINT16 sdt_version			: 5;
	UINT16 ft_type				: 2;//front end type,check lib_frontend.h        
	UINT16 reserved				: 3;

	UINT16 preset_flag			: 2;
	UINT16 usage_status			: 1;
	UINT16 nit_pid				:13;
		
	UINT32 t_s_id				:16;
	UINT32 network_id			:16;

#if (defined( _MHEG5_ENABLE_) || defined( _MHEG5_V20_ENABLE_) || defined( NETWORK_ID_ENABLE))
    UINT16 net_id     			:16;	/* network_id */
	UINT16 reserved2			:16;
#endif

}/*__attribute__((packed))*/ T_NODE;

typedef struct program
{
	UINT16 sat_id;
	DB_TP_ID tp_id;

	UINT32 prog_id;

	//prog feature
	UINT32 level				: 2;
	UINT32 preset_flag 		: 2;
	UINT32 av_flag			: 1;
	UINT32 ca_mode			: 1;
	UINT32 video_pid			:13;
	UINT32 pcr_pid			:13;

	UINT32 prog_number		:16;
	UINT32 pmt_pid			:13;
	UINT32 tuner1_valid		: 1;
	UINT32 tuner2_valid		: 1;
	UINT32 h264_flag    		: 1;

	UINT32 fav_group[0];
	UINT8 fav_grp0	    		: 1;
	UINT8 fav_grp1	    		: 1;
	UINT8 fav_grp2	    		: 1;
	UINT8 fav_grp3	    		: 1;
	UINT8 fav_grp4	    		: 1;
	UINT8 fav_grp5	    		: 1;
	UINT8 fav_grp6	    		: 1;
	UINT8 fav_grp7	    		: 1;
	UINT8 fav_group_byte2;
	UINT8 fav_group_byte3;
	UINT8 fav_group_byte4;

	UINT16 pmt_version		: 5;
	UINT16 service_type		: 8;
	UINT16 audio_channel   	: 2;
	UINT16 audio_select		: 1;
	
	UINT16 user_modified_flag: 1;
	UINT16 lock_flag	    		: 1;
	UINT16 skip_flag			: 1;
	UINT16 audio_volume		: 8;
	UINT16 reserve_2			: 5;
	
#ifdef User_order_sort
	UINT32 user_order;
#endif

	UINT32 provider_lock		: 1;
	UINT32 subtitle_pid		:13;
	UINT32 teletext_pid		:13;	
	UINT32 cur_audio			: 5;

	UINT16 nvod_sid;
	UINT16 nvod_tpid;
	
#ifndef MULTI_BOUQUET_ID_SUPPORT
	UINT16 bouquet_id;
#endif

	UINT16 logical_channel_num;
#if(defined( _MHEG5_ENABLE_) ||defined(_LCN_ENABLE_) ||defined(_MHEG5_V20_ENABLE_))
	UINT16 mheg5_exist		: 1;
	UINT32 orig_LCN			:16;
	UINT16 reserve_3		:14;
	UINT16 LCN_true			: 1;
	UINT16 LCN				:16;
	UINT32 default_index 	:16; 
#endif

#ifdef MULTI_BOUQUET_ID_SUPPORT
	UINT16 bouquet_count;
	UINT16 bouquet_id[MAX_BOUQUET_CNT];
#endif

#ifdef DB_CAS_SUPPORT
	UINT16 cas_count;
	UINT16 cas_sysid[MAX_CAS_CNT];
#endif
	
	UINT16 audio_count;
	UINT16 audio_pid[MAX_AUDIO_CNT];
	UINT16 audio_lang[MAX_AUDIO_CNT];

#if (defined(DTG_AUDIO_SELECT) || defined(AUDIO_DESCRIPTION_SUPPORT))
	UINT8 audio_type[MAX_AUDIO_CNT];
#endif

	UINT16 name_len;
	UINT8 service_name[2*(MAX_SERVICE_NAME_LENGTH + 1)];

	UINT16 provider_name_len;
	UINT8 service_provider_name[2*(MAX_SERVICE_NAME_LENGTH + 1)];
	
}/*__attribute__((packed))*/ P_NODE;

/*sort struct and define*/

typedef UINT32 KEY_VALUE;

struct sort_t_info
{
	UINT32 value;
	DB_TP_ID id;
	UINT16 next_pos;
};

struct sort_p_info
{
	UINT16 node_pos;
	UINT16 next_pos;
};


enum
{
	DYNAMIC_PG_INITED	= 0x00,
	DYNAMIC_PG_STEAM_UPDATED = 0x01,
	DYNAMIC_PG_USER_MODIFIED = 0x02,
};

struct dynamic_prog_back
{
	UINT8 status;
	P_NODE prog;
};

#define MAX_BAND_COUNT					10
#define MAX_COUNTRY					20
#define MAX_BAND_PARAMETERS			7

typedef struct
{
	UINT32 start_freq;
	UINT32 end_freq;
	UINT32 freq_step;
	UINT8  start_ch_no;
	UINT8  end_ch_no;
	UINT8  show_ch_no;
	char   show_ch_prefix;
}Band_param;


//database dvbs interface api
BOOL db_same_node_checker(UINT8 n_type, void* old_node, void* new_node);

INT32 get_tp_at(UINT16 sat_id,UINT16 pos, T_NODE *node);
UINT16 get_tp_num_sat(UINT16 sat_id);
INT32 del_tp_on_sat(UINT16 sat_id);

INT32 get_sat_at(UINT16 pos, UINT16 select_mode,S_NODE *node);
INT32 get_sat_by_id(UINT16 sat_id, S_NODE *node);
INT32 modify_sat(UINT16 sat_id, S_NODE *node);
INT32 del_sat_by_pos(UINT16 pos);
INT32 del_sat_by_id(UINT16 sat_id);

INT32 recreate_sat_view(UINT16 create_mode,UINT32 param);
UINT16 get_sat_num(UINT16 select_mode);




#endif

