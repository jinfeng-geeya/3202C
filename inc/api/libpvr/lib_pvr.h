/****************************************************************************
*
*  ALi (Shanghai) Corporation, All Rights Reserved. 2005 Copyright (C)
*
*  File: TsNV.h
*
*  Description: NV module to manipunate TS stream file
*              
*  History:
*      Date        	Author         	Version   		Comment
*      ====        	======      	=======   		=======
*  1. 2005-3-2 		Jay 	     	0.1.000   		Initial
*  2. 2005-3-17		Zhengdao	0.1.001			Porting to lib_pub27.
****************************************************************************/

#ifndef	__LIB_PVR_H__
#define	__LIB_PVR_H__

#include <types.h>
#include <sys_config.h>
#include <basic_types.h>


#ifdef PVR3_SUPPORT

#include "lib_pvr3.h"

#else

//the type for player or recorder handle
typedef UINT32	PVR_HANDLE;

#define DVR_DMX_FOR_TMSHIFT			0
#define DVR_DMX_FOR_RECORDING		1
#define DVR_DMX_FOR_PLAYBACK		2

//the time-shifting index, reverse the 1 for time shifting
#define TMS_INDEX			1
#define MAX_BOOKMARK_NUM	20
#define MAX_PVR_AUDIO_PID	10
#define MAX_PVR_ECM_PID		16
#define MAX_PVR_EMM_PID		16
#define MAX_PVR_SUBT_PID		11
#define MAX_PVR_TTX_PID		11
#define MAX_PVR_TTX_SUBT_PID	11
#define MAX_AGELIMIT_NUM    512
#define MAX_RATINGCTL_NUM		256

#define MAX_PVR_DISK_NUM 		2 // pvr support disk num
#define PVR_REC_AND_TMS_DISK 	0
#define PVR_REC_ONLY_DISK 		1
#define PVR_TMS_ONLY_DISK 		2

#define PVR_MAX_PID_NUM	32

//##: notice message from DVR module to APP/UI
#define PVR_END_DATAEND		1		//the player playback to the file EOF
#define PVR_END_DISKFULL		2 		//the HDD is full!
#define PVR_END_TMS			3		//in the time shifting mode, the player catch up the recorder
#define PVR_END_REVS			4		//in the backword mode, the player gets to the beginning of a file
#define PVR_HDD_READY		5
#define PVR_HDD_TEST_SPEED	6
#define PVR_HDD_PVR_CHECK	7
#define PVR_HDD_FAIL			8
#define PVR_END_WRITEFAIL	9 
#define PVR_END_READFAIL		10 
#define PVR_HDD_FORMATTED	11 
#define PVR_TMS_OVERLAP       12
#define PVR_END_REC        0xFF //rec time is up, need to stop rec

#define TXTI_LEN 34
#define RECORDING_NORMAL 	0
#define RECORDING_TIMESHIFT	1
//#ifdef DTG_PVR
#define EVENT_NAME_MAX_LEN 70
#define EVENT_DETAIL_MAX_LEN 1024
enum PVR_DETAIL_TYPE
{
    SHORT_DETAIL = 0,
    EXTEND_DETAIL,
};
//#endif
enum PVR_MODE
{
	PVR_REC_SAME_FOLDER = 0,
	PVR_REC_SUB_FOLDER,
	PVR_REC_RANDOM_FOLDER,
};

enum PVR_PROJECT_MODE
{
	PVR_DVBS2	= (1<<0),
	PVR_DVBT	= (1<<1),
	PVR_DVBT_2M = (1<<2),
	PVR_ATSC	= (1<<3),
};

enum PVR_DISKMODE
{
	PVR_DISK_INVALID = 0,
	PVR_DISK_REC_AND_TMS,
	PVR_DISK_REC_WITH_TMS,
	PVR_DISK_ONLY_REC,
	PVR_DISK_ONLY_TMS,
};

struct PVR_DTM
{
	UINT32  mjd;
	UINT16  year;
	UINT8   month;
	UINT8   day;
	UINT8   weekday;
	UINT8   hour;
	UINT8   min;
	UINT8   sec;
};

enum sort_type
{
	PVR_SORT_RAND = 0, //read order
	PVR_SORT_NAME, //alphabet
	PVR_SORT_TYPE, //a/v
	PVR_SORT_DATE, //record start time
	PVR_SORT_LEN, //record time len
	PVR_SORT_SIZE, //record size
};

enum vob_status {
	VOB_START	= 0,
	VOB_NORMAL	= 1,
	VOB_END		= 2,
};

enum p_open_config_t
{
	P_OPEN_DEFAULT 			= 0,
	P_OPEN_VPO_NO_CTRL 		= (1<<0),
	P_OPEN_PREVIEW 			= (1<<1),
	P_OPEN_FROM_HEAD 		= (1<<2),
	P_OPEN_FROM_TAIL		= (1<<3),
	P_OPEN_FROM_LAST_POS	= (1<<4),
	P_OPEN_FROM_PTM		= (1<<5),
};

// for pvr_p_close stop_mode type define
enum p_stop_mode_t
{
	P_STOP_AND_REOPEN 	= 0,	// the devices will be stopped and reopened to playback
	P_STOPPED_ONLY 		= 1,	// the devices will be stopped
};

enum dvr_HDD_info_status
{
	HDDS_SUCCESS 		= 0,			// hdd can read,write.
	HDDS_NOT_EXIST 		= (1<<5),	// device not exist .
	HDDS_SPACE_FULL		= (1<<6),	// hdd space is full, only enable read
	HDDS_CREATE_DIR_ERR	= (1<<7),	// create root directory error, disable pvr read write
	HDDS_WRITE_ERR		= (1<<8),	// hdd write error, maybe device is locked.
	HDDS_READ_ERR		= (1<<9),  	// hdd read error, reserved
};

typedef enum    
{
	NV_STOP			= 0, 		//stop
	NV_PLAY  		= 1, 		//normal playback
	NV_PAUSE 		= 2, 		//pause
	NV_FF	 		= 3,		//fast forward	
	NV_STEP  		= 4,		//step
	NV_FB	 		= 5,		//fast backward
	NV_SLOW		= 6,		//slow 
	NV_REVSLOW		= 7,		//reverse slow
	
	NV_RECORDING 	= 11, 
} PVR_STATE;

typedef INT8 (*PVR2UI_Callback)(PVR_HANDLE handle, UINT8 msg);

struct pvr_config
{
	enum PVR_MODE	dvr_mode;
	PVR2UI_Callback	event_callback;
	char 		dvr_path_prefix[256]; //like "AliDvr"
	char 		info_file_name[256]; //like "info.dvr"
	char 		ts_file_format[256]; //like "dvr"
	char 		ps_file_format[256]; //like "dvr"
	char 		test_file1[256]; //like "test_write1.dvr"
	char 		test_file2[256]; //like "test_write2.dvr"
	UINT32 		min_record_len; //in second, the record will be deleted if shorter that this limit
	UINT32		tms_max_time_len;
	UINT32 		tms_file_min_size;	// in MBytes,  recomment to 10M
	UINT16		ps_packet_size;
	UINT8		recorder_num;
	UINT8		player_num;
	UINT8		tms_delete_background;
	UINT8		continuous_tms_enable;
	UINT8		tms2record_support;
	UINT8		copy_support;
	UINT8		record_filepack_support;
	UINT8		ac3_decode_support;
	UINT8		pvr_name_in_unicode;
	UINT8		playback_trick_mode; // 0: by video, 1: by time (keep time precise)
	UINT8		debug_enable;
};

struct pvr_pid_info
{
	UINT8	total_num;// be sure less than 32 (27 for PIP project), redundant pids will be ignored by PVR and dmx!
	UINT8	audio_count;
	UINT8	ecm_pid_count;
	UINT8	emm_pid_count;
	UINT8	ttx_pid_count;
	UINT8	subt_pid_count;
	UINT8	ttx_subt_pid_count;
	UINT8	resv0[5];
	UINT16	video_pid;
	UINT16	audio_pid[MAX_PVR_AUDIO_PID];
	UINT16	audio_lang[MAX_PVR_AUDIO_PID];
	UINT16	pcr_pid;
	UINT16	pat_pid;
	UINT16	pmt_pid;
	UINT16	sdt_pid;
	UINT16	eit_pid;
	UINT16	cat_pid;
	UINT16	nit_pid;
	UINT16	ecm_pids[MAX_PVR_ECM_PID];
	UINT16	emm_pids[MAX_PVR_EMM_PID];
	UINT16	ttx_pids[MAX_PVR_TTX_PID];
	UINT16	subt_pids[MAX_PVR_SUBT_PID];
	UINT16	ttx_subt_pids[MAX_PVR_TTX_SUBT_PID];
	UINT16	resv1[11];
};//total 220 bytes! if updated, remember to update record info structure!!

struct record_prog_param
{
	UINT16 mode;
	BOOL is_scrambled;
	char folder_name[1024]; // whole route on RANDOM_FOLDER mode
	UINT8 av_flag;
	UINT8 lock_flag;
	UINT8 ca_mode;
	UINT8 h264_flag;
	UINT8 audio_channel;
	UINT8 nim_id;
	UINT8 dmx_id;
	UINT8 live_dmx_id;
	UINT32 channel_id;
	UINT16 cur_audio_pid_sel;
	struct pvr_pid_info pid_info;
	char service_name[36];
	UINT16 name_len;
	UINT16 prog_number;
	UINT8  rec_type;//0:TS, 1:PS
     //#ifdef DTG_PVR
       UINT8 event_name[EVENT_NAME_MAX_LEN];
     //#endif
};

struct playback_param
{
	UINT16 index;
	char path[1024];
	PVR_STATE state;
	UINT8 speed;
	UINT32 start_time; //start time or pos
	UINT8 start_mode; //open mode: enum p_open_config_t
	UINT8 dmx_id;
	UINT8 live_dmx_id;
};

struct ListInfo
{
	UINT8	pvr_version;	
	UINT8 	del_flag;// delete flag: <b>0</b> - Not deleted, <b>1</b> - Deleted	      	         		   
	UINT8	lock_flag;	// locked flag: <b>0</b> - Not locked, <b>1</b> - Locked
	UINT8	rec_mode;// the type of the Recording List: <b>0</b> - Normal, <b>1</b> - Time-shift                                                   
	UINT8	channel_type;// the channel type: <b>0</b> - TV, <b>1</b> - Radio
	UINT8	audio;// bit0~3: audio type(0 --MPEG 1, 1 --MPEG 2); bit4~7: audio channel
	UINT16	index;// the index of the Recording List 
	enum sort_type sort_type;
	struct  PVR_DTM tm;// The start recording time information for the Recording List
	UINT32  	duration;// The playback/recording duration for this Recording List.
	UINT32	channel;// The channel number for the Recording List
	UINT8   	txti[TXTI_LEN];// The text information (name of the Recording List)
	UINT16 	group;// The Group for the Recording List
	UINT16  	pid_pcr;
	UINT16	pid_v;
	UINT16	pid_a;
	UINT16 	pid_pmt;
	UINT16 	pid_cat;
	UINT16 	prog_number;
	UINT32	quantum_num;	
	UINT32	ts_bitrate;//bps
	UINT8	ca_mode; // $** or ** prog	
	UINT8	is_scrambled;	
	UINT8	is_recording; 
	UINT8	level;//record level for QoS
	UINT32	size; //**K disk size
	UINT16	subt_num;
	UINT16	ttx_num;
	UINT16	ttx_subt_num;
	struct t_subt_lang *subt_list;
	struct t_ttx_lang *ttx_list;
	struct t_ttx_lang *ttx_subt_list;
	UINT8	audio_count;						// total audio pid count
	UINT8	cur_audio_pid_sel;					// current audio select pid.
	UINT16 multi_audio_pid[MAX_PVR_AUDIO_PID];	// multi audio pid
	UINT16 multi_audio_lang[MAX_PVR_AUDIO_PID]; // multi audio language
	UINT8  rec_type;//0:TS, 1:PS
	struct pvr_pid_info pid_info;
	UINT8 record_pid_num; // add for dynamic pid
	UINT16 record_pids[PVR_MAX_PID_NUM]; // add for dynamic pid
	UINT32 reserved[64];
       //#ifdef DTG_PVR
       UINT8  event_txti[EVENT_NAME_MAX_LEN];// The text information (event name of the Record)
       //#endif
};

struct dvr_HDD_info
{	//size is in K
	UINT8 	valid;
	UINT8	disk_mode; 		// 0 usb disk, 1 ide disk
	char		mount_name[16]; 	// such as "/c"
	UINT8	type;			//1:FAT  2:NTFS
	UINT8	disk_usage;		// REC or TMS or total, default is total
	UINT8	init_list;			// inited pvr list
	UINT8	check_speed;
    UINT8   tms_level;      //0%-100%
	UINT32	total_size;		//the total size of the HDD.
	UINT32	free_size;		//free size
	UINT32	rec_size;			//record size
	UINT32	tms_size;			//for time shifting size
	UINT32	status;			//indicate current device status, refer to:enum dvr_HDD_info_status
	//used for internal
	char		disk_name[256];	//such as "/dev/hda1"
	UINT32	tms_free;
	UINT32	read_speed;
	UINT32	write_speed;
	UINT32	rw_speed;
};

struct pvr_control_info
{
	UINT8	disk_num;
	UINT8	tms_disk_id;//from 1, 0 is invalid
	UINT8	rec_disk_id;//from 1, 0 is invalid
	struct dvr_HDD_info disk[MAX_PVR_DISK_NUM];
};

struct pvr_register_info
{
	UINT8	disk_mode; // 0 usb disk, 1 ide disk
	UINT8 	disk_usage;
	char	mount_name[16]; // such as "/c"
	UINT32	vbh_addr;
	UINT32	vbh_len;
	UINT8	init_list; // need init pvr record list
	UINT8	check_speed; // need to check speed
};

struct pvr_clean_info
{
	UINT8	disk_mode; // 0 usb disk, 1 ide disk
	char	mount_name[16]; // such as "/c"
};

typedef struct
{
	UINT8 cache_level;			// vob cache number (read_cache+write_cache<=4)
	UINT8 ttx_en;				// record TTX
	UINT8 subt_en;				// record SUBT
	//UINT8 ttx_epg_share_mem;
	UINT8 max_rec_number;		// recorder number for recorder.c
}PVR_CFG, *PPVR_CFG;
// **************************************************************
// Description:
//     This is the callback function prepared by the application!
//     The pvr module notice the PVR events to APP/UI.
// Summary:
//     callback function for APP/UI.
// Parameters:
//     handle :  the handle of the player or the recorder
//     msg :     the notice message                              
typedef BOOL (*pfn_prog_id_to_record_param)(UINT32 prog_id, struct record_prog_param *prog_info);

typedef BOOL (*pvr_init_callback)(UINT32 param1, UINT32 param2);

#define C_PVR_ROOT_DIR_NAME_SIZE    32

// ********************************
// The callback functions structure
struct PVR2UIParam
{
	// *******************************************************
	// This function will be called while the playback is over
	PVR2UI_Callback	playback_over;
	pfn_prog_id_to_record_param prog_id_to_record;
	const char *pvr_root_dir_name;    // "AliDvr", max len is (C_PVR_ROOT_DIR_NAME_SIZE-1)
    UINT32 file_header_save_dur;		// File head save duration, warning: 
    									// to improve the write performance, don't set it too small(less than 30 seconds)
	pvr_init_callback pfn_pvr_init_cb;   									
};

//#################################################################
//###################### Function decleare here ############################

//##: DVR module initial and cleanup functions
RET_CODE 	pvr_attach(struct pvr_config *pvr_cfg);
INT32 		pvr_register(UINT32 param1,UINT32 param2);
INT32 		pvr_cleanup(void);
INT32 		pvr_cleanup_disk(UINT32 param);
RET_CODE 	pvr_set_disk_use(UINT8 use, const char *mount_name);
enum PVR_DISKMODE	pvr_get_cur_mode(char *rec_disk, char *tms_disk);

//##: DVR recording API
PVR_HANDLE	pvr_r_open(struct record_prog_param *param);
UINT32		pvr_r_trans(PVR_HANDLE handle);
BOOL		pvr_r_close(PVR_HANDLE *handle, BOOL sync);
RET_CODE	pvr_r_changemode(PVR_HANDLE handle);
void			pvr_r_set_scramble(void);
UINT32		pvr_r_get_time(PVR_HANDLE handle);
void 		pvr_r_set_ts_file_size(UINT8 G_num);
UINT32 		pvr_r_get_bitrate(UINT8 dmx_id, UINT16 v_pid, UINT16 a_pid);
UINT8 		pvr_r_get_nim_id(PVR_HANDLE handle);
UINT8 		pvr_r_get_dmx_id(PVR_HANDLE handle);
UINT32 		pvr_r_get_channel_id(PVR_HANDLE handle);
RET_CODE	pvr_r_pause(PVR_HANDLE handle);
RET_CODE	pvr_r_resume(PVR_HANDLE handle);
PVR_STATE 	pvr_r_get_state(PVR_HANDLE handle);

//##: DVR playback API
PVR_HANDLE 	pvr_p_open(UINT16 index, PVR_STATE state, UINT8 speed, UINT32 start_time, UINT8 config);
PVR_HANDLE 	pvr_p_open_ext(struct playback_param *param);
BOOL		pvr_p_close(PVR_HANDLE *handle, UINT8 stop_mode, UINT8 vpo_mode, BOOL sync);

//##: DVR playback state speed!
BOOL	pvr_p_play(PVR_HANDLE handle);
BOOL	pvr_p_pause(PVR_HANDLE handle);
BOOL	pvr_p_revslow(PVR_HANDLE handle, UINT8 speed);
BOOL	pvr_p_slow(PVR_HANDLE handle, UINT8 speed);
BOOL	pvr_p_step(PVR_HANDLE handle);
BOOL	pvr_p_fast_backward(PVR_HANDLE handle, UINT8 speed);
BOOL	pvr_p_fast_forward(PVR_HANDLE handle, UINT8 speed);
BOOL	pvr_p_timesearch(PVR_HANDLE handle, UINT32 ptm);
BOOL	pvr_p_jump(PVR_HANDLE handle, INT32 ptm);
BOOL 	pvr_p_seek(PVR_HANDLE handle, UINT32 pos);
BOOL	pvr_p_stop(PVR_HANDLE handle);
BOOL 	pvr_p_lock_switch(PVR_HANDLE handle, UINT32 lock_en);

//##: DVR playback information functions
UINT32 		pvr_p_get_pos(PVR_HANDLE handle);
UINT32		pvr_p_get_pos_time(PVR_HANDLE handle);
UINT32 		pvr_p_save_cur_pos(PVR_HANDLE handle); //for last play
UINT32		pvr_p_get_lastplay_pos(UINT16 index);
INT8 		pvr_p_get_direct(PVR_HANDLE handle);
UINT8		pvr_p_get_speed(PVR_HANDLE handle);
PVR_STATE	pvr_p_get_state(PVR_HANDLE handle);
UINT32		pvr_p_get_time(PVR_HANDLE handle);
UINT8 		pvr_p_get_dmx_id(PVR_HANDLE handle);

void 		pvr_resume_play_channel(void);
void 		pvr_p_set_ttx_lang(PVR_HANDLE handle);
void 		pvr_p_set_subt_lang(PVR_HANDLE handle);
void 		pvr_set_logo_id(UINT32 logo_id);
void 		pvr_p_switch_audio_pid(PVR_HANDLE handle, UINT16 pid);
void			pvr_p_switch_audio_channel(PVR_HANDLE handle, UINT8 a_ch);

UINT8 		pvr_p_get_dmx_id(PVR_HANDLE handle);

//##: for UI callback function
void		pvr_UI_init(PVR2UI_Callback ui_callback);
void		pvr_init_ui_param(struct PVR2UIParam *pvr_param);

UINT32		pvr_get_tms_capability(void);//the return value is in secounds!
RET_CODE 	pvr_set_bookmark(UINT16 record_idx, UINT32 mark_time);
UINT8 		pvr_get_bookmark(UINT16 record_idx, UINT32 *mark_ptm_array);
RET_CODE	pvr_set_agelimit(UINT16 record_idx, UINT32 ptm, UINT32 age);
UINT8		pvr_get_agelimit(UINT16 record_idx, UINT32 ptm);
RET_CODE	pvr_set_ratingctl(UINT16 record_idx, UINT32 ptm, UINT32 ratingctl);
UINT32		pvr_get_ratingctl(UINT16 record_idx, UINT32 ptm);

//##: Recording list management API
//#ifdef DTG_PVR
RET_CODE    pvr_set_event_detail(UINT16 record_idx, UINT8 *pDetail, UINT32 len, UINT8 type);
UINT8* pvr_get_event_detail(UINT16 record_idx, UINT8 type);
UINT8* pvr_get_event_name(UINT16 record_idx);
//#endif
UINT16	pvr_get_rl_count(void);
void		pvr_get_rl_info(UINT16 rl_id, struct ListInfo *info);
UINT16	pvr_get_rl_info_by_pos(UINT16 pos, struct ListInfo *info);
UINT16 	pvr_get_index(PVR_HANDLE handle);
void		pvr_set_rl_info(UINT16 rl_id, struct ListInfo info);
BOOL 	pvr_set_tms_size(UINT32 tms_size);//unit is K bytes
void    pvr_set_tms_level(UINT8 percent);
BOOL 	pvr_check_rl_del(void);
void 	pvr_clear_rl_del(void);
void 	pvr_save_rl(UINT16 index);
void		pvr_update_rl(void);
BOOL 	pvr_check_tms2rec(void);

RET_CODE	pvr_list_sort(enum sort_type mode, UINT8 order);

//##: HDD information
BOOL 	pvr_format_HDD(UINT8 mode);
UINT8	pvr_get_HDD_info(struct dvr_HDD_info *hdd_info);
void		pvr_check_disk(void);
BOOL 	pvr_p_use_tsgen(void);

//##: the following function is for the DMX and the VOB cache
INT32		pvr_p_request(PVR_HANDLE handle, UINT8 **addr, INT32 length, INT32 *indicator);
BOOL		pvr_r_update(PVR_HANDLE handle, UINT32 size, UINT16 offset);
INT32		pvr_r_request(PVR_HANDLE handle, UINT8 **addr, INT32 length);
PVR_HANDLE	pvr_get_handle_to_dmx(UINT32 dmx_id);

void pvr_r_set_record_all(BOOL bRecordAll);
BOOL pvr_r_get_record_all(void);

char *pvr_get_root_dir_name(void);       // return "ALiDvr"

// interfaces for CC TSG task
void cc_tsg_task_init(void);
RET_CODE cc_tsg_task_start(UINT32 prog_id);
RET_CODE cc_tsg_task_stop(void);

BOOL cc_tsg_task_is_running(void);

int pvr_tms_vol_cleanup(void);
int pvr_tms_vol_init(void);

BOOL pvr_r_add_PID(PVR_HANDLE handle, UINT16 pid_nums, UINT16 *pids);

#endif
#endif //__LIB_PVR_H__
