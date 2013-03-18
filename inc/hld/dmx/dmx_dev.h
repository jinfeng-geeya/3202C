/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2002 Copyright (C)
 *
 *  File: dmx_dev.h
 *
 *  Description: This file define the struct of demux device.
 *               Come of the linux video device.
 *  History:
 *      Date        Author         Version   Comment
 *      ====        ======         =======   =======
 *  1.  2002.12.16  David Wang     0.1.000   Initial
 *  2.  2003.02.11  David Wang	   0.1.001   Modification for new sw tree
 *  3.  2004.08.10  Goliath Peng      0.1.002   Merge the new hld code with old one.
 ****************************************************************************/
#ifndef _DMX_DEV_H_
#define _DMX_DEV_H_
#include <sys_config.h>
#include <mediatypes.h>


#ifdef DVR_PVR_SUPPORT
#define DMX_SUPPORT_PLAYBACK
#endif

enum STREAM_TYPE
{
	UNKNOW_STR = 0,
	PRG_STR_MAP,
	PRIV_STR_1,
	PAD_STR,
	PRIV_STR_2,
	AUDIO_STR,
	VIDEO_STR,
	ECM_STR,
	EMM_STR,
	DSM_CC_STR,
	ISO_13522_STR,
	H2221_A_STR,
	H2221_B_STR,
	H2221_C_STR,
	H2221_D_STR,
	H2221_E_STR,
	ANCILLARY_STR,
	REV_DATA_STR,
	PRG_STR_DIR
};

enum DES_STR{
	DES_VIDEO = 0,
	DES_AUDIO ,
	DES_PCR,
	DES_TTX,
	DES_SUP
};

enum PES_RETRIEVE_FMT{
	PES_HEADER_DISCARDED = 7,
	PES_HEADER_INCLUDED
};

typedef	RET_CODE (* Request_write)(void *, UINT32, void **, UINT32 *, struct control_block *);
typedef	void (* Update_write)(void *, UINT32 );

struct pes_retrieve_param{
	enum STREAM_TYPE str_type;
	enum PES_RETRIEVE_FMT retrieve_fmt;
	void * device;
	Request_write request_write;
	Update_write update_write;
	UINT8 filter_idx;
};

typedef enum
{
    DMX_ES_DATA=0,
    DMX_SEC_DATA=1,
    DMX_RAW_DATA=2,
    DMX_REC_DATA=3,
    DMX_PES_DATA=4
}tDMX_DATA_TYPE;

struct pvr_rec_io_param
{
	UINT8 *io_buff_in;
	UINT32 buff_in_len;
	UINT8 *io_buff_out;
	UINT32 buff_out_len;
	UINT32 hnd;
	UINT8 h264_flag;
	UINT8 is_scrambled;
	UINT8 record_whole_tp; //
	UINT8 rec_type;   //0:TS, 1:PS.
	INT32 (*request)(UINT32, UINT8 **, INT32, INT32 *);
	BOOL (*update)(UINT32, UINT32, UINT16);  
#ifdef SEE_ENABLE
	void *dec_dev;
	void *enc_dev;
#endif	
};

struct pvr_play_io_param
{
	UINT8 *io_buff_in;
	UINT32 buff_in_len;
	UINT8 *io_buff_out;
	UINT32 buff_out_len;	
	UINT32 hnd;
	UINT8 h264_flag;
	UINT8 is_scrambled;
	UINT8 rec_type;   //0:TS, 1:PS.
    INT32 (*p_request)(UINT32, UINT8 **, INT32, INT32 *);
    INT32 (*p_request_key)(UINT32, UINT8 *, UINT32 *, UINT8 *);
};

struct pvr_ps_av_start_info
{
	UINT32 handle;
	UINT8 *buffer;
	UINT8 scr1[5]; 
	UINT8 scr2[5]; 
	UINT8 pts2[5];
	UINT8 v_seq_info[150];
	UINT8 a_frm_info[4];	
};

struct register_service_new{
	void * device;
	Request_write request_write;//RET_CODE (* request_write)(void *, UINT32, void **, UINT32 *, struct control_block *);
	Update_write  update_write;//void (* update_write)(void *, UINT32 );
	enum STREAM_TYPE str_type;
	UINT32 service_pid;
	UINT32 service_id;
	tDMX_DATA_TYPE dmx_data_type;
	//UINT8 service_start_flag;

	void *param; /*pes_param or sec_param or dmx_rcd*/
	
};

struct register_service{
	void * device;
	Request_write request_write;//RET_CODE (* request_write)(void *, UINT32, void **, UINT32 *, struct control_block *);
	Update_write  update_write;//void (* update_write)(void *, UINT32 );
	enum STREAM_TYPE str_type;
	UINT16 service_pid;
	UINT16 reserved;
};

struct dmx_device
{
	struct dmx_device  *next;  /*next device */
	/*struct module *owner;*/
	INT32 type;
	INT8 name[HLD_MAX_NAME_SIZE];
	INT32 flags;

	INT32 hardware;
	INT32 busy;
	INT32 minor;

	void *priv;		/* Used to be 'private' but that upsets C++ */
	UINT32 base_addr;
	void      (*attach)(void);
	void      (*detach)(struct dmx_device **);
	RET_CODE	(*open)(struct dmx_device *);
	RET_CODE   	(*close)(struct dmx_device *);
	RET_CODE   	(*start)(struct dmx_device *);
	RET_CODE   	(*stop)(struct dmx_device *);
	RET_CODE   	(*pause)(struct dmx_device *);
	RET_CODE   	(*get_section)(struct dmx_device *, void *);
	RET_CODE   	(*async_get_section)(struct dmx_device*,void*, UINT8 * );
	RET_CODE   	(*ioctl)(struct dmx_device *, UINT32 , UINT32);	
	RET_CODE   	(*register_service)(struct dmx_device *, UINT8 , struct register_service *);
	RET_CODE   	(*unregister_service)(struct dmx_device *, UINT8);
	RET_CODE 	(*cfg_cw)(struct dmx_device *, enum DES_STR, UINT8, UINT32 *);

	UINT16 TotalFilter;
    UINT16 FreeFilter;
    RET_CODE   	(*register_service_new)(struct dmx_device *, struct register_service_new *);
	RET_CODE   	(*unregister_service_new)(struct dmx_device *, struct register_service_new *);
};


#endif /* _DMX_DEV_H_ */

