/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2002 Copyright (C)
 *
 *  File: dmx.h
 *
 *  Description: head file for demux device management.
 *
 *  History:
 *      Date        Author         Version   Comment
 *      ====        ======         =======   =======
 *  1.  2002.12.19  David Wang      0.1.000   Initial
 *  2.  2003.02.11  David Wang      0.1.001   Modification for new sw tree
 *  3.  2004.04.05  Goliath Peng    0.1.002   Add some new fields to structure definition
 *  4.  2004.07.17  Goliath Peng    0.1.003   Do some optimization.  
 *  5.  2004.08.10  Goliath Peng    0.1.004   Merge the new hld code with old one.
 *  6.  2008.08.04  Joy Chu         0.1.005   New commands added to support play two programms in one dmx.
 ****************************************************************************/
#ifndef  _DMX_H_
#define  _DMX_H_

#include <hld/dmx/dmx_dev.h>
#include <bus/dma/dma.h>
#include <sys_config.h>
#include <os/tds2/alitypes.h>
//#define DEBUG_DMX  
#ifdef DEBUG_DMX
#define DMX_PRINTF   PRINTF
#else
#define DMX_PRINTF(...) do{}while(0)
#endif

#define TP_SECH_UNI_BUF //In tp search mode, use united buffer mode to avoid si/psi channel overlap.
#define DMX_SI_SHARE_VBV_SIZE   (__MM_DMX_SI_TOTAL_LEN - __MM_SI_VBV_OFFSET)//Dmx will share these buffer with VBV

//#ifdef SEC_FLT_ENHANCE
#define MULTI_MASK
//#endif

#ifdef TP_SECH_UNI_BUF
#define DMX_SI_PSI_SIZE     DMX_SI_SHARE_VBV_SIZE  
#else
#define DMX_SI_PSI_SIZE             DMX_SI_SHARE_VBV_SIZE
#endif
/*define the demux device running status*/
#define DMX_STATE_DETACH    0
#define DMX_STATE_ATTACH    1
#define DMX_STATE_IDLE          2
#define DMX_STATE_PLAY         4
#define DMX_STATE_PAUSE      8

/*define the sub state of demux device */
#define DMX_SUB_STATE_BUSY          0x01
#define DMX_SUB_STATE_NO_DATA   0x02
#define DMX_SUB_STATE_NO_BUF        0x04
/*define the scramble flag*/
#define VDE_TS_SCRBL        0x01
#define VDE_PES_SCRBL       0x02
#define AUD_TS_SCRBL        0x04
#define AUD_PES_SCRBL       0x08
#define VDE_STR_INVALID    0x10
#define AUD_STR_INVALID    0x20

/*define demux command*/
#define DMX_CMD_STOP_GET_SEC    0x00000001
#define DMX_CMD_WAKEUP_SIAE  0x00000010 
#define DMX_CMD_PLAYBACK_START 0x00000002
#define DMX_CMD_PLAYBACK_STOP 0x00000004
#define DMX_CMD_PLAYBACK_PAUSE 0x00000008

/* define the device IO control code for demux */
#define  DMX_IO  0x0000000F

#define IO_CREATE_AUDIO_STREAM              (DMX_IO + 1)
#define IO_CREATE_VIDEO_STREAM              (DMX_IO + 2)
#define IO_CREATE_VIDEO_PCR_STREAM          (DMX_IO + 3)
#define IO_CREATE_PCR_STREAM                (DMX_IO + 4)
#define IO_STREAM_ENABLE                    (DMX_IO + 5)
#define IO_STREAM_DISABLE                   (DMX_IO + 6)
#define IO_DMX_RESET                        (DMX_IO + 7)
#define IO_DELETE_AV_STREAM                 (DMX_IO + 8)
#define IO_CREATE_AV_STREAM                 (DMX_IO + 9)
#define IO_CHANGE_AUDIO_STREAM              (DMX_IO + 10)
#define IO_STOP_GET_SECTION             (DMX_IO + 11)
#define IO_STOP_GET_PRVDATA             (DMX_IO + 12)
#define IO_GET_STOP_STATUS                  (DMX_IO + 13)
#define AUDIO_STREAM_ENABLE                 (DMX_IO + 14)
#define AUDIO_STREAM_DISABLE                (DMX_IO + 15)
#define VIDEO_STREAM_ENABLE                 (DMX_IO + 16)
#define VIDEO_STREAM_DISABLE                (DMX_IO + 17)
#define CLEAR_STOP_GET_SECTION          (DMX_IO + 18)
#define IO_ASYNC_POLL                       (DMX_IO + 19)
#define IO_ASYNC_ABORT                      (DMX_IO + 20)
#define IO_ASYNC_CLOSE                      (DMX_IO + 21)
#define IS_DMA_RUNNING                      (DMX_IO + 22)
#define IS_AV_SCRAMBLED                 (DMX_IO + 23)
#define CB_CHANGE_LEN                       (DMX_IO + 24)
#define CB_RESUME_LEN                       (DMX_IO + 25)
#define IS_PROGRAM_LEGAL                    (DMX_IO + 26)
#define DMX_SPEED_UP                        (DMX_IO + 27)
#define DMX_NORMAL_PLAY                 (DMX_IO + 28)
#define DMX_BYPASS_CSA                      (DMX_IO + 29)
#define DMX_EN_DCW                          (DMX_IO + 30)
#define CREATE_RECORD_STR                   (DMX_IO + 31)
#define DELETE_RECORD_STR                   (DMX_IO + 32)
#define DMX_PARSE_PS                        (DMX_IO + 33)
#define DMX_PARSE_TS                        (DMX_IO + 34)
#define IO_DMX_CSA_SPEC                 (DMX_IO + 35) //for s3601
#define DMX_SWITCH_TO_UNI_BUF           (DMX_IO + 36)
#define DMX_SWITCH_TO_DVI_BUF               (DMX_IO + 37)
#define DMX_REQ_CH_BUFF                 (DMX_IO + 38)
#define DMX_WAKEUP_SIAE                 (DMX_IO + 39)
#define IO_DMX_NEW_CW_PARITY                (DMX_IO + 40) //for s3601, create a new CW parity
#define IO_DMX_SET_CW_PARITY                (DMX_IO + 41) //for s3601, select a new CW parity for a channel
#define IO_DMX_DEL_CW_PARITY                (DMX_IO + 42) //for s3601, delete a CW parity 
#define DMX_CHANGE_THLD                 (DMX_IO + 43)
#define RECORD_WHILE_PLAY                   (DMX_IO + 44)
#define DO_TIME_SHIFT                       (DMX_IO + 45)
#define DMX_ADJUST_AV_PTS                   (DMX_IO + 46)
#define DMX_BINDING_PES_RETRIEVE            (DMX_IO + 47)
#define DMX_PID_SURVEY_FOR_CA               (DMX_IO + 48) /*for DVB-C, to check which channel is playing specific pid*/
#define DMX_CHK_BUF_MODE                    (DMX_IO + 49) /*Check is dvide/unify buffer mode*/
#define DMX_ENABLE_IC_SORTING               (DMX_IO + 50) /*Enable IC sorting, set IC sorting param*/
#define DMX_PID_VALIDITY_CHECK          (DMX_IO + 51) /*for DVB-C, check the PID validity.*/
#define DMX_IS_TS_ENTER_CSA             (DMX_IO + 52) /*For DVB-C, check whether TS enter CSA core*/
#define DMX_CHANNEL_EN_DES              (DMX_IO + 53) /*For DVB-C, en/dis descramble for every channel*/
#define DMX_REGISTER_SPECIAL_CB         (DMX_IO + 54)
#define DMX_REGISTER_SPECIAL_MASK           (DMX_IO + 55)
#define DMX_DVR_CHANGE_PID              (DMX_IO + 56)
#define DMX_RECORD_PS                       (DMX_IO + 57)
#define DMX_RECORD_TS                       (DMX_IO + 58)
#define REGISTER_PVR_CALLBACK               (DMX_IO + 59)
#define ADD_RECORD_STR						(DMX_IO + 60) 
#define SUB_RECORD_STR						(DMX_IO + 61)
#define DMX_CONTINUOUS_TIMESHIFT			(DMX_IO + 62) /*For Continuous timeshift after channel change.*/
#define IO_DMX_PAUSE_PARSE                  (DMX_IO + 63)
#define IO_DMX_GET_PACKET_NUM				(DMX_IO + 64)
#define IO_DMX_GLEAR_PACKET_NUM				(DMX_IO + 65)

#define IO_CREATE_AV_SET_PARAM              (DMX_IO + 66)

#define IO_GET_PS_AV_START_INFO             (DMX_IO + 67)
#define IO_SET_TSG_AV_MODE                  (DMX_IO + 68)
#define IO_CLEAR_TSG_AV_MODE                (DMX_IO + 69)

#define IO_SET_DEC_HANDLE                   (DMX_IO + 70)
#define IO_SET_DEC_STATUS                   (DMX_IO + 71)
#define IO_SET_DEC_CONFIG                   (DMX_IO + 72)

#define DMX_IO_DROP_ERR_PACKETS             (DMX_IO + 73)
#define DMX_IO_DO_DDP_CERTIFICATION         (DMX_IO + 74)
#define IO_GET_DEC_HANDLE                   (DMX_IO + 75)


#define DMX_DIVIDED_BUFF_MODE               0
#define DMX_UNITED_BUFF_MODE                1                   


#define DMX_ADV_IO                          (DMX_IO+0x200)
#define IO_SET_BYPASS_MODE              (DMX_ADV_IO + 1)
#define IO_BYPASS_GETDATA                   (DMX_ADV_IO + 2)
#define IO_CLS_BYPASS_MODE                  (DMX_ADV_IO + 3)
#define GET_PROG_BITRATE                    (DMX_ADV_IO + 4)
#define CHECK_DMX_REMAIN_BUF                (DMX_ADV_IO + 5) /*For DVR prj, check dmx remain channel buffer.(For TSG playback)*/
#define SET_TSG_PLAYBACK                    (DMX_ADV_IO + 6) /*For DVR prj, set TSG playback mode.*/
#define TSG_PLAYBACK_SYNC                   (DMX_ADV_IO + 7) /*For DVR prj, set TSG playback mode.*/
/*low level driver control command for DDK mode development.*/
#define DMX_REQUEST_SEC_CHANNEL             (DMX_ADV_IO + 8)
#define DMX_RELEASE_SEC_CHANNEL             (DMX_ADV_IO + 9)
#define DMX_SET_CHANNEL_PID             (DMX_ADV_IO + 10)
#define DMX_CONTROL_CHANNEL             (DMX_ADV_IO + 11)
#define DMX_INQUIRE_AVAILABLE_SEC_CH        (DMX_ADV_IO + 12)

#define DMX_PIP_SWAP                        (DMX_ADV_IO + 13)
#define DMX_ENABLE_TS_INPUT                 (DMX_ADV_IO + 14)
#define DMX_IS_TSG_PLAYBACK                 (DMX_ADV_IO + 15) /*For DVR prj, check if is TSG playback currently.*/
#define DMX_PLAY_IN_MAIN_PIC                (DMX_ADV_IO + 16)
#define DMX_IS_IN_TIMESHIFT             (DMX_ADV_IO + 17) /*For PIP prj, check if DMX is in timeshift mode*/
#define DMX_REGISTER_DMA_CALLBACK       (DMX_ADV_IO + 18) /*Register SGDMA callback function.*/
#define DMX_FORCE_CHANGE_SLOT        (DMX_ADV_IO + 19) /*For dynamic PID channel change to same PID but different prog with backgroud RCD prog */

/* Added by Joy. Date: 30.07.2008 */
#define IO_CREATE_AV_STREAM_EXT             (DMX_ADV_IO + 20)
#define IO_STREAM_ENABLE_EXT                (DMX_ADV_IO + 21)
#define IO_STREAM_DISABLE_EXT               (DMX_ADV_IO + 22)
#define IS_BOTH_SLOT_PLAY                   (DMX_ADV_IO + 23)
#define IS_AV_SCRAMBLED_EXT                 (DMX_ADV_IO + 24)
#define IS_AV_SOURCE_SCRAMBLED			    (DMX_ADV_IO + 25)
#define CHECK_VDEC_OCCUPATION				(DMX_ADV_IO + 26)
#define CREATE_RECORD_STR_EXT				(DMX_ADV_IO + 27)

#define DMX_CMD_REC_ADD_PID                 (DMX_ADV_IO + 28)
#define DMX_CMD_REC_DEL_PID                 (DMX_ADV_IO + 29)

#define DMX_SEAMLESS_SWITCH_OPEN_NEXT_V_STREAM  (DMX_ADV_IO + 30)


/* Added by Penson. Date: 02.22.2010 */
#define IO_GET_DISCONTINUE_COUNT            (DMX_ADV_IO + 31)
#define IO_ClEAR_DISCONTINUE_COUNT          (DMX_ADV_IO + 32)

#define CHECK_ADEC_OCCUPATION               (DMX_ADV_IO + 33)

/*DMX channel control command (For DDK mode development)*/
#define DMX_CHANNEL_DISABLE 0
#define DMX_CHANNEL_ENABLE  1
#define DMX_CHANNEL_RESET   2

/* Enable/Disable high bit rate detection. */
#define DMX_IO_HIGH_BIT_RATE_DETECT_EN      (DMX_ADV_IO + 34)

/*
 * Adjust incorrect PTS for Kinvon M3381T
 * Data:2010.06.08 by Joy Chu
 */
#define DMX_IO_ADJUST_PTS                   (DMX_ADV_IO + 35)

#define IO_CLEAR_DMX_BUFFER 				(DMX_ADV_IO + 36)

/* Get TS packet number by pid for Coship M3701E porting.
 * Date: 2011-08-11	by Leo Ma
*/
#define DMX_IO_GET_TS_PACKET_NUM_BY_PID		(DMX_ADV_IO + 37)

/* Add by Joy for implementing an independent A/V sync module.
 * Date: 2011.10.10.
 */
typedef INT32 (*ALI_AVSYNC_CTRL_CALLBACK)(UINT32 Pts, UINT32 FrmBufTotalLen, UINT32 FrmBufCurLen);

enum ALI_AVSYNC_MODE
{
    ALI_AVSYNC_LEGACY,
    ALI_AVSYNC_PCR_MASTER,
    ALI_AVSYNC_AUDIO_MASTER,
    ALI_AVSYNC_VIDEO_MASTER,
    ALI_AVSYNC_VIDEO_REERUN,
};

struct AliAvsyncModeParam
{
    enum ALI_AVSYNC_MODE AvSyncMode;

    ALI_AVSYNC_CTRL_CALLBACK AvSyncCtrlCallback; /* Need to be called before a new frame is shown. */
};


#define DMX_SET_AVSYNC_MODE (DMX_ADV_IO + 38)

#define ALI_SET_AVSYNC_MODE (DMX_ADV_IO + 39)



//for S3601 descramling
enum CSA_SPEC
{    //for IO_DMX_CSA_SPEC
    DMX_CSA_2_0  = 0, //the default value
    DMX_CSA_1_1 
};

struct cw_parity_t
{   //for IO_DMX_NEW_CW_PARITY, 
    UINT32 *cw;   //IN, pointer to a CW parity, DWORD 0: ECW, DWORD 1: OCW  
    UINT8 handle;  //OUT, handle returned for this ocw/ecw parity
};

struct cw_sel_t
{   //for IO_DMX_SET_CW_PARITY
    UINT32 cw_parity_handle;   //IN
    UINT8 flt_idx;   //IN
};

struct dec_parse_param
{  
    void *dec_dev;   
    UINT32 type;
};


/*define the max length of device list */
#define DMX_MAX_DEV_QUEUE_LEN      8
#define DMX_IS_OPEN        0x00000001
 
#if(SYS_CHIP_MODULE==ALI_S3601 || SYS_CHIP_MODULE == ALI_S3602)
	#if((SYS_CPU_MODULE == CPU_M6303) && (SYS_PROJECT_FE==PROJECT_FE_DVBT))//for 3105
		#define TOTAL_FILTER_NUMBER     32
	#else
		#define TOTAL_FILTER_NUMBER     48
	#endif
	#define MAX_FILTER_NUMBER            27
#else
	#define MAX_FILTER_NUMBER            27
	#define TOTAL_FILTER_NUMBER         32
#endif

#define MAX_SEC_MASK_LEN                  16
#define MAX_MV_NUM                              8

//#ifdef DMX_SUPPORT_PLAYBACK
#if(SYS_CHIP_MODULE==ALI_S3601 || SYS_CHIP_MODULE == ALI_S3602)
#define PS_SIZE                         (188*16)
#else
#define PS_SIZE                         (188*8)
#endif

#define REQ_DATA_LEN                    (0x17800>>1)
#define REQ_PKT_LEN                     (REQ_DATA_LEN/188)
#define REQ_PKT_LEN_PS                  (REQ_DATA_LEN/PS_SIZE)
//#endif

#define DMX_TS_BLOCK_SIZE               0xbc000

/*define parser running status*/
enum DEMUX_STATE{
    DEMUX_FIND_START = 0,
    DEMUX_HEADER,
    DEMUX_DATA,
    DEMUX_SKIP 
};


#define RETRIEVE_SEC    0
#define RETRIEVE_TS 0xa5
#define     INVALID_PID     0x1fff

struct dmx_mm_map{
    UINT32 dmx0_dma_buf;        //in byte unit, must 4 DW alignment
    UINT32 dmx1_dma_buf;        //in byte unit, must 4 DW alignment
    UINT16 ts_video_cb_len;         //in TS packet unit.
    UINT16 ts_audio_cb_len;     //in TS packet unit.
    UINT16 ts_pcr_cb_len;       //in TS packet unit.
    UINT16 ts_ttx_cb_len;       //in TS packet unit.
    UINT16 ts_common_cb_len;    //in TS packet unit.
    UINT16 ts_united_cb_len;        //in TS packet unit.
    UINT32 ts_ttx_tmp_buf;
    UINT16 ts_ttx_tmp_cb_len;
    UINT8 total_pid_filt_num;//video+audio+pcr+ttx+(sutbitle+N), N means N si/psi filters.
    UINT8 xfer_ves_by_dma;      //True: transfer Video ES by DMA; False: transfer Video ES by CPU copy.
    UINT32 dmx3_dma_buf;        //in byte unit, must 4 DW alignment
};
    
struct dmx_feature_config{
    struct dmx_mm_map mm_map;
    UINT8 adjust_av_pts;        //enable manually adjust A/V pts to solve some A/V sync issue   
    UINT8 audio_bitrate_detect; //enable audio ts stream high bitrate detect.
    UINT8 video_bitrate_detect;     //enable video ts stream high bitrate detect.
    UINT8 clock_polarity;       //0: standard clk polar; 1: clk polar reverse;
    UINT8 sync_mode;    //0: external sync, by psunc signal. 1: internal sync, by 0x47 sync byte.
    UINT8 sync_times;   //only used in internal sync mode: only support 1 time, 2, 3 8 times.
};

// RET_CODE dmx_m36f_pre_attach(const struct dmx_pre_attach_param *pre_attach_param);
struct dmx_pre_attach_param {
    UINT16 my_size;                 // must set it to sizeof(dmx_pre_attach_param) for compatibility
    UINT16 reserved;                // set to 0
    UINT32 dmx_ts_blk_buf1;         // must be 32 bytes aligned, in main memory
    UINT32 dmx_ts_blk_buf2;         // must be 32 bytes aligned, in private memory
    UINT32 dmx_ts_blk_buf1_size;    // DMX_TS_BLOCK_SIZE
    UINT32 dmx_ts_blk_buf2_size;    // DMX_TS_BLOCK_SIZE
};

struct restrict{
    UINT8  mask[MAX_SEC_MASK_LEN];
    UINT8  value[MAX_MV_NUM][MAX_SEC_MASK_LEN];
    UINT8  mask_len;                   /* Mask length in unit of byte */
    UINT8  value_num;                 /* Target value number. */    
    UINT8 multi_mask[MAX_MV_NUM][MAX_SEC_MASK_LEN];
    UINT16  tb_flt_msk; 
};

struct restrict_ext{
    UINT8   mask[MAX_SEC_MASK_LEN];
    UINT8   value[MAX_MV_NUM][MAX_SEC_MASK_LEN];
    UINT8   mask_len;                   /* Mask length in unit of byte */
    UINT8   value_num;                 /* Target value number. */    
    UINT8   multi_mask[MAX_MV_NUM][MAX_SEC_MASK_LEN];
    UINT16  tb_flt_msk;

    UINT8  *multi_value_ext;
    UINT8  *multi_mask_ext; 
    UINT8  *multi_mask_en_ext; 
    UINT8  *multi_mask_hit_ext;
    UINT16  multi_mask_num_ext;
    UINT16  multi_mask_len_ext;
};


/*This struct use to manage a section buffer*/
struct get_section_param{
    UINT8 * buff;              /*Start address of section buffer*/                                                
    UINT32  cur_pos;                  /*The end address of available data in section buffer.*/
    UINT16  buff_len;                   /*The size of allocated section buffer.*/
    UINT16  sec_tbl_len;        /*The length of the whole section = section_length + 3. */
    UINT8    get_sec_len;         /*:1 already get section length. 0: not yet.*/
    UINT8    crc_flag;                 /*:1 need CRC verification. :0 needn't :2or3 crc err.*/  
    UINT8    conti_conter;
    UINT8    overlap_flag;           /*currently useless*/
    UINT32  wai_flg_dly;            /*get section delay, defined by app, is wai_flg_dly==0, use default  */
    UINT32  crc_result;
    enum    DEMUX_STATE  dmx_state;                /* DEMUX_HEADER, DEMUX_DATA, DEMUX_SKIP */ 
    struct    restrict * mask_value;
    void      (*get_sec_cb)(struct get_section_param *); /*for continuously get section mode, */
    /*if continue_get_sec==1, dmx will call this call back function.*/
    UINT16  pid;                           /*indicated pid to get*/
    UINT16  sec_hit_num;        /*report which value matched*/
    UINT8   continue_get_sec; /*1: indicate dmx will continuously get section, */
    UINT8   retrieve_sec_fmt;/*RETRIEVE_SEC: retrieve section, RETRIEVE_TS: retrieve TS*/
    /*until continue_get_sec change to 0*/
#if (SYS_PROJECT_FE == PROJECT_FE_ATSC)
    UINT32 priv_param; // private use
#endif
};

struct dmx_ic_sorting_param {
    BOOL    bEnable;
    UINT32  pcr_sorting_begin;
    UINT32  pcr_threshold_start;
    UINT32  pcr_threshold_end;
    UINT8   *pTerminate;
    UINT8   *pShow_Version_OSD;
    UINT8   *pError_Indicator;
};

struct dmx_pvr_param
{
    UINT32 (*fp_pvr_get_handle_to_dmx)(UINT32);
    INT32   (*fp_pvr_p_request)(UINT32, UINT8 **, INT32, INT32 *);
    BOOL    (*fp_pvr_r_update)(UINT32, UINT32, UINT16);
    INT32   (*fp_pvr_r_request)(UINT32, UINT8 **, INT32);
};

struct dmx_dma_callback
{
    UINT8 (*fp_dma_open)(UINT8, UINT32, struct startcode_attr_t*);
    void (*fp_dma_close)(UINT8);
    UINT32 (*fp_dma_copy)(UINT8,void *, void *, UINT16, UINT8);
    void (*fp_dma_wait)(UINT32, UINT8);
    RET_CODE (*fp_dma_get_start_code)(UINT8, struct start_code_t *);
};

RET_CODE dmx_open(struct dmx_device * dev);
RET_CODE dmx_close(struct dmx_device * dev);
RET_CODE dmx_start(struct dmx_device * dev);
RET_CODE dmx_stop(struct dmx_device * dev);
RET_CODE dmx_pause(struct dmx_device * dev);
RET_CODE dmx_req_section(struct dmx_device * dev, struct get_section_param * sec_param);
RET_CODE dmx_async_req_section(struct dmx_device *dev,struct get_section_param *sec_param, UINT8 * flt_idx);
RET_CODE dmx_io_control(struct dmx_device * dev, UINT32 cmd, UINT32 param);
RET_CODE dmx_register_service(struct dmx_device * dev, UINT8 filter_idx, struct register_service * reg_serv);
RET_CODE dmx_unregister_service(struct dmx_device * dev, UINT8 filter_idx);
RET_CODE dmx_cfg_cw(struct dmx_device * dev, enum DES_STR str_type, UINT8 cw_type, UINT32 * cw);
RET_CODE dmx_register_service_new(struct dmx_device * dev, struct register_service_new * reg_serv);
RET_CODE dmx_unregister_service_new(struct dmx_device * dev, struct register_service_new * reg_serv);

INT32 adv_dmx_retrieve_sec(UINT32 ch_id, UINT32 * flt_id, UINT8 * buf, UINT32 * sec_len, UINT32 max_len, UINT32 specific_flt);
UINT8 * adv_dmx_get_sec(UINT32 ch_id, UINT32 * flt_id, UINT32 * sec_len, UINT32 specific_flt);
INT32  adv_dmx_rel_sec(UINT32 ch_id, UINT32 flt_id, UINT32 specific_flt);
UINT32 adv_dmx_alloc_channel(UINT32 dmx_id, UINT32 crc_en, 
	INT32 (*reg_sec_cb)(UINT32 ch_id, UINT32 flt_id, UINT8 * section, UINT32 length));
INT32 adv_dmx_free_channel(UINT32 ch_id);
INT32 adv_dmx_set_pid(UINT32 ch_id, UINT16 pid);
UINT16 adv_dmx_get_pid(UINT32 ch_id);
INT32 adv_dmx_en_channel(UINT32 ch_id);
INT32 adv_dmx_dis_channel(UINT32 ch_id);
INT32 adv_dmx_rst_channel(UINT32 ch_id);
UINT32 adv_dmx_alloc_filter(UINT32 ch_id);
INT32 adv_dmx_free_filter(UINT32 ch_id, UINT32 flt_id);
INT32 adv_dmx_set_filter(UINT32 ch_id, UINT32 flt_id, UINT8 * mask, UINT8 * value, UINT32 len);
INT32 adv_dmx_get_filter_num(UINT32 ch_id);
INT32 adv_dmx_init(UINT32 max_channel_num, UINT8 * buf, UINT32 buf_len);
INT32 adv_dmx_quit(void);

typedef INT32 (* slot_sec_cb)(INT32 slot_id, UINT8 *section, UINT32 length);
INT32 adv_dmx_slot_init(UINT32 crc_en);
INT32 adv_dmx_slot_alloc(UINT32 dmx_id, UINT16 pid, UINT8 *mask, UINT8 *value, UINT8 length, slot_sec_cb slot_cb);
INT32 adv_dmx_slot_free(INT32 slot_id, slot_sec_cb slot_cb);
UINT16 adv_dmx_slot_get_pid(INT32 slot_id);
INT32 adv_dmx_slot_exit(void);


//#ifdef DMX_SUPPORT_PLAYBACK
#define DMX_BUF_RESUME() do{}while(0)
/*
#define DMX_BUF_RESUME()    \
        do {
            extern struct dmx_device * g_dmx2_dev;  \
            dmx_io_control(g_dmx2_dev, BUF_RESUME, 0);  \
        } while (0)
*/
//#endif

#endif  /* _DMX_H_*/


