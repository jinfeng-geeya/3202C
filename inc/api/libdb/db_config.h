#ifndef __DB_CONFIG_H__
#define __DB_CONFIG_H__


#include <api/libsi/si_config.h>

//#define  DB_TEST
//for testing some fatal error
#define DB_ASSERT	ASSERT(0)

/*configurations that may be changed depond on project*/
/***********************************************************************/
#define DB_MULTI_SECTOR
#define DB_LARGE_MEM_MODE
#define DB_USE_UNICODE_STRING
#define SERVICE_PROVIDER_NAME_OPTION 1

/*in search process, node number of tp and prog to be buffered*/
#define SEARCH_BUF_TP_NUM	300

/*max node number limit*/
//#define	MAX_SET_NUM	64
//#define	MAX_TP_NUM	2000
//#define	MAX_PG_NUM	4000
#define	MAX_CMD_NUM	128


typedef UINT32 DB_TP_ID;

typedef struct{
	UINT32 pg_id;
	UINT32 tp_id;
	UINT16 sat_id;
	UINT16 reserved1   	:1;
	UINT16 tun1_val      	:1;
	UINT16 tun2_val      	:1;
	UINT16 pip_av_mod    :1;
	UINT16 pol    			:2;
	UINT16 reserved     	:10;

	UINT32 *pip_calback;
}pip_db_info;

typedef BOOL (*db_filter_pip)(pip_db_info *ap_pip);

/***********************************************************************/

/*db function pointer define*/
typedef BOOL (*db_filter_t)(UINT32 id, UINT32 addr, UINT16 filter_mode,UINT32 param);

//typedef INT32 (*db_compare_t)(UINT32 id1, UINT32 id2, void *param);

typedef INT32 (*node_pack_t)(UINT8 node_type, UINT8*src_node, UINT32 src_len,UINT8*packed_node,UINT32 *packed_len);

typedef INT32 (*node_unpack_t)( UINT8 node_type,UINT8 *packed_node,UINT32 packed_len,UINT8 *unpack_node, UINT32 unpack_len);


/**************************************/
/*db config*/
#define DB_SECTOR_SIZE (64 * 1024)
#define DB_NODE_MAX_SIZE	256
#define DB_TABLE_NUM	3

/*db node id size and assignment*/
#if defined(DB_LARGE_MEM_MODE)
#define NODE_ID_SIZE		4
#define NODE_ID_SET_BIT	8
#define NODE_ID_TP_BIT	10
#define NODE_ID_PG_BIT	10

#define NODE_ID_SET_MASK	0x0FF00000
#define NODE_ID_TP_MASK		0x0FFFFC00
#define NODE_ID_PG_MASK	0x0FFFFFFF

#define SET_BIT_MASK		0x0FF00000
#define TP_BIT_MASK		0x000FFC00
#define PG_BIT_MASK		0x000003FF
#else
#define NODE_ID_SIZE		3
#define NODE_ID_SET_BIT	8
#define NODE_ID_TP_BIT	8
#define NODE_ID_PG_BIT	8

#define NODE_ID_SET_MASK	0x00FF0000
#define NODE_ID_TP_MASK		0x00FFFF00
#define NODE_ID_PG_MASK	0x00FFFFFF

#define SET_BIT_MASK		0x00FF0000
#define TP_BIT_MASK		0x0000FF00
#define PG_BIT_MASK		0x000000FF
#endif


/*db node address size and assignment*/
#define NODE_ADDR_SIZE		3
#define NODE_ADDR_OFFSET_BIT	16

/*db reclaim mode*/
#define DB_RECLAIM_OPERATION	0x01
#define DB_RECLAIM_DATA			0x02
/*****************************************/

#define DB_ENTER_MUTEX()  			osal_semaphore_capture(db_access_sema,OSAL_WAIT_FOREVER_TIME)
#define DB_RELEASE_MUTEX()			osal_semaphore_release(db_access_sema)

/* create view group flag */
#define VIEW_ALL			0x0100
#define VIEW_SINGLE_SAT	0x0200
#define VIEW_SINGLE_TP	0x0300
#define VIEW_FAV_GROUP	0x0400
#define VIEW_SINGLE_CH	0x0500
#define VIEW_ALL_CH		0x0600
#define VIEW_BOUQUIT_ID	0x0700
#define VIEW_CAS			0x0900
#define VIEW_PROVIDER  	0x0a00


#ifdef _MHEG5_ENABLE_
#define CREATE_VIEW_ALL_CH	VIEW_ALL_CH
#define CREATE_VIEW_ALL_SAT	VIEW_ALL
#endif

/* program av mode flag */
#define PROG_RADIO_MODE	    0x00
#define PROG_TV_MODE		    0x01
#define PROG_TVRADIO_MODE	    0x02
#define PROG_DATA_MODE		0x03
#define PROG_NVOD_MODE		0x04
#define PROG_ALL_MODE               0x05
#define PROG_PRIVATE_MODE       0x06
#define PROG_TVRADIODATA_MODE       0x07
#define PROG_TVDATA_MODE		0x08


/* sat select and tuner setting flag */
#define SET_SELECTED			0x0800

#define SET_TUNER1_VALID		0x01
#define SET_TUNER2_VALID		0x02
#define SET_EITHER_TUNER_VALID	0x03

/*view node flag*/
#define V_NODE_FIND_FLAG	0x01
#define V_NODE_DEL_FLAG		0x02


/*load default prog flag*/
#define DEFAULT_DOMESTIC_PROG		0x01
#define DEFAULT_ALL_PROG			0x02
#define DEFAULT_SAT_TP_ONLY			0x03

#define PRE_SET_TP_FLAG			0x01

#define PROG_ADD_REPLACE_OLD			0x00
#define PROG_ADD_DIRECTLY				0x01
#define PROG_ADD_PRESET_REPLACE_NEW	0x02
#define PROG_ADD_REPLACE_NONPRESET	0x03

/* for cal program num */
#define SAT_PROG_NUM			0x01
#define FAV_PROG_NUM			0x02
#define TP_PROG_NUM				0x04
#define NVOD_PROG_NUM			0x08
#define DATA_PROG_NUM			0x10


#ifndef DB_USE_UNICODE_STRING
#define DB_DEFAULT_NAME				"No Name"
#define DB_DEFAULT_PROV_NAME	"Unknown PP"
#define DB_DEFAULT_NAME_LEN		8
#define DB_DEFAULT_PROV_LEN		11

#define DB_STRCMP				STRCMP
#define DB_STRLEN				STRLEN
typedef UINT8 DB_ETYPE;
#else
typedef UINT16 DB_ETYPE;
#define DB_DEFAULT_NAME				"\0N\0o\0 \0N\0a\0m\0e\0"
#define DB_DEFAULT_PROV_NAME	"\0U\0n\0k\0n\0o\0w\0n\0 \0P\0P\0"
#define DB_DEFAULT_NAME_LEN			16
#define DB_DEFAULT_PROV_LEN			22

INT32 DB_STRCMP(UINT16 *s, UINT16 *t);
INT32 DB_STRLEN(UINT16 *s);
#endif /* DB_USE_UNICODE_STRING */

typedef INT32 (*name_compare_t)(DB_ETYPE *name1, DB_ETYPE *name2);

/*db node type*/
enum
{
	TYPE_PROG_NODE = 0,
	TYPE_TP_NODE =	1,
	TYPE_SAT_NODE = 2,
	TYPE_SEARCH_TP_NODE = 3,
	TYPE_UNKNOWN_NODE = 0xFF,
};


/* flag for sort program */
enum {
	PROG_LOCK_SORT			= 0x01,
	PROG_LOCK_SORT_EXT		= 0x02,
	
	PROG_FTA_SORT				= 0x03,
	PROG_FTA_SORT_EXT			= 0x04,
	
	PROG_NAME_SORT			= 0x05,
	PROG_NAME_SORT_EXT		= 0x06,
	
	PROG_DEFAULT_SORT			= 0x07,
	PROG_DEFAULT_SORT_EXT		= 0x08,
	
	PROG_LOGICAL_NUM_SORT	= 0x09,
	PROG_DEFAULT_SORT_THEN_FTA	= 0x0A,

	PROG_CAS_SORT				= 0x0B,
	PROG_PROVIDER_SORT		= 0x0C,
	PROG_LCN_SORT				= 0x0D,
	PROG_TYPE_SID_TSID_SORT	= 0x0E,
	PROG_TYPE_SID_SORT		= 0x0F,
	PROG_TYPE_SID_SORT_EXT	= 0x10,

	PROG_USERORDER_SORT	= 0x11,


};

enum {
	TP_DEFAULT_SORT			= 0x11,
	TP_DEFAULT_SORT_EXT		= 0x12,
};



/*db filter mode*/
union filter_mode{
	struct 
	{
		UINT8 group_mode;
		UINT8 av_mode;
	} decode;
	UINT16 encode;
};


/*db node index param*/
typedef struct 
{
	UINT8 node_id[NODE_ID_SIZE];
	UINT8 node_addr[NODE_ADDR_SIZE];
	
}NODE_IDX_PARAM;

/*db table*/
typedef struct t_attr_t
{
	UINT8 type;
	UINT8 node_moved;
	UINT16 max_cnt;
	UINT16 node_num;
	UINT8 *tmp_head_buf;
	NODE_IDX_PARAM *table_buf;
	//DB_CMD_BUF *cmd_buf; 

}DB_TABLE;

/*db view*/
typedef struct v_attr_t
{
	UINT8 cur_type;
	UINT8 pre_type;
	UINT16 max_cnt;
	UINT16 node_num;
	NODE_IDX_PARAM *node_buf;
	UINT8 * node_flag;
	UINT32 view_param;
	UINT32 pre_param;
	UINT16 cur_filter_mode;
	UINT16 pre_filter_mode;
	db_filter_t cur_filter;
	db_filter_t pre_filter;
	//DB_TABLE *table;

}DB_VIEW;





#endif
