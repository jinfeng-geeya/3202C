#ifndef __LIBMP_PE_H__
#define __LIBMP_PE_H__

#include <api/libimagedec/imagedec.h>
#include <api/libfs2/types.h>
//Current supporte media stream type in decoders
#include <api/libfs2/stdio.h>
#include <api/libfs2/unistd.h>
#include <osal/osal.h>

#define UNKNOWN_STREAM 					0
#define MPEG_12_FILE_STREAM             1
#define AVI_STREAM 						2
#define MKV_STREAM 						3

//define the callback function for APP
#define MP_FS_MOUNT				    (1 << 0)
#define MP_FS_UNMOUNT				(1 << 1)
#define MP_MUSIC_PLAYBACK_END		(1 << 2)
#define MP_IMAGE_PLAYBACK_END		(1 << 3)
#define MP_IMAGE_DECODE_PROCESS 	(1 << 4)
#define MP_FS_IDE_MOUNT_NAME		(1 << 5)
#define MP_FS_USB_MOUNT_NAME		(1 << 6)
#define MP_VIDEO_PLAYBACK_END       (1 << 7)
#define MP_IMAGE_DECODER_ERROR		(1 << 8)
#define MP_VIDEO_BUILD_IDX_TBL      (1 << 9)
#define MP_VIDEO_PARSE_END          (1 << 10)
#define MP_VIDEO_DECODER_ERROR		(1 << 11)
#define MP_DRM_NOT_AUTHORIZED       (1 << 12)
#define MP_DRM_RENTAL_EXPIRED       (1 << 13)
#define MP_DRM_VIEW_RENTAL          (1 << 14)
#define MP_DRM_VERSION_ERR          (1 << 15)
#define MP_DRM_ICT_ENABLE           (1 << 16)

// param in mp_callback_func(MP_VIDEO_BUILD_IDX_TBL, param)
#define MP_VIDEO_BUILD_IDX_TBL_BEGIN    0
#define MP_VIDEO_BUILD_IDX_TBL_END      1

/* param in mp_callback_func(MP_VIDEO_DECODER_ERROR, param),
   and in mp_callback_func(MP_VIDEO_PARSE_END, param);  */
#define MP_DERR_OK                      0
#define MP_DERR_UNSUPPORTED_VIDEO_CODEC (1<<0)
#define MP_DERR_UNSUPPORTED_AUDIO_CODEC (1<<1)
#define MP_DERR_UNSUPPORTED_RESOLUTION  (1<<2)
#define MP_DERR_UNSUPPORTED_FRAME_RATE  (1<<3)
#define MP_DERR_TOO_BIG_INTERLEAVE      (1<<4)
#define MP_DERR_MP4_DRM_AUTH            (1<<5)
#define MP_DERR_UNKNOWN                 0xffffffff

#define PE_CACHE_NUM    2

#define PE_CACHE_CLOSED     0
#define PE_CACHE_OPENED     1

#define PE_FILE_READING     0
#define PE_FILE_FAILED      1
#define PE_FILE_EOF         2

#define PE_PREVIOUS_BUFFER_SIZE 10240 //10K

#define PE_NET_FILE_TYPE_URL 0
#define PE_ALI_SPECIAL_STREAM 1

#define PE_NET_FILE_TYPE_MAX 2

typedef void(*mp_callback_func)(unsigned long type, unsigned long param);
#ifdef PE_CALLBACK_TO_APP_ENABLE		
typedef void(*pccache_callback_func)(UINT8 type, UINT8 param);
#endif
//define function point for pe_cache use
typedef  FILE * (* pe_cache_open_fp)(const char *, unsigned char* , unsigned int, unsigned int );//filename, buf, cache length, offset
typedef size_t (* pe_cache_read_fp)(void *, size_t, size_t, FILE *);
typedef int(* pe_cache_seek_fp)(FILE *, off_t, int);
typedef int (* pe_cache_feof_fp)(FILE*);
typedef off_t (* pe_cache_ftell_fp)(FILE *);
typedef int (* pe_cache_fclose_fp)(FILE *);

typedef struct
{
    UINT16 internal_cache;   //:1;
    FILE *fp;
    UINT32 block_size;
    off_t file_size;

#ifdef ENABLE_PE_FLAG
    UINT32 req_size;
    ID flag;
#endif
//add function point  for pe_cache callback
pe_cache_read_fp file_read;
pe_cache_seek_fp file_seek;
pe_cache_feof_fp file_eof;
pe_cache_ftell_fp file_tell;
pe_cache_fclose_fp file_close;
#ifdef LOG_PE_CACHE
    UINT32 read_delay;
    UINT32 read_count;
    UINT32 read_tick;
    UINT32 read_size;
#endif
}pe_cache, *p_pe_cache;

typedef struct
{
	UINT8	status;
	UINT8	sub_status;
	ID		mutex;
	UINT8 *	cache_buff;
	UINT32	cache_size;
	UINT32	data_len;
	UINT32	rd_pos;
	UINT32	wr_pos;
	off_t		file_offset;	
}pe_cache_ex;

#ifdef DUAL_ENABLE
#define PE_CACHE_CMD_NULL  0
#define PE_CACHE_CMD_OPEN  1
#define PE_CACHE_CMD_CLOSE 2
#define PE_CACHE_CMD_SEEK  3

#define PE_CACHE_CMD_STATUS_NEW 1
#define PE_CACHE_CMD_STATUS_IMPLEMENTED 2

typedef struct
{
    UINT32 status     :2; 
    UINT32 type       :2;
    UINT32 reserved   :28;
    UINT32 param[4];
}pe_cache_cmd;
#endif

struct pe_feature_cfg
{
	UINT32 chip;

	UINT32 using_adpcm;
	UINT32 has_dview;  // has divew same as vpo of M3327C 
	UINT32 has_pip;		// has pip same as vpo of M3329E 
	UINT32 has_sca;		// has dview scaler as vpo of M3101C 
};	

struct pe_image_cfg
{
	
	// dst frame info
	UINT32 frm_y_addr;
	UINT32 frm_y_size;
	UINT32 frm_c_addr;
	UINT32 frm_c_size;
	UINT32 frm2_y_addr;
	UINT32 frm2_y_size;
	UINT32 frm2_c_addr;
	UINT32 frm2_c_size;
	UINT32 frm_mb_type;
	UINT32 frm3_y_addr;
	UINT32 frm3_y_size;
	UINT32 frm3_c_addr;
	UINT32 frm3_c_size;
	UINT32 frm4_y_addr;
	UINT32 frm4_y_size;
	UINT32 frm4_c_addr;
	UINT32 frm4_c_size;

	// buf for dec internal usage
	UINT8 *decoder_buf;
	UINT32 decoder_buf_len;
	
	//callback function for application
	mp_callback_func mp_cb;		
};

struct img_callback
{
    mp_callback_func mp_cb;
};

struct pe_music_cfg
{
	//buff
	UINT32 pcm_out_buff;
	UINT32 pcm_out_buff_size;
	UINT32 processed_pcm_buff;
	UINT32 processed_pcm_buff_size;
	
	//callback function for application
	mp_callback_func mp_cb;	
};

struct pe_video_cfg
{
	mp_callback_func mp_cb;
	UINT32 decoder_buf;
	UINT32 decoder_buf_len;
	UINT32 mm_vbv_len;
	UINT32 reserved;    // struct pe_video_cfg_extra *. set to 0 if no pe_video_cfg_extra
};

struct pe_video_cfg_extra
{
    UINT32 frame_buf_base;
    UINT32 frame_buf_len;
    UINT16 max_width_support, max_height_support;   // if 0, determined by frame_buf_len
    UINT32 reserved[5];    // use in the future, should set to 0
};

typedef struct
{
    struct pe_image_cfg image;
    struct pe_music_cfg music;
    struct pe_video_cfg video;
}PeConfig, *p_pe_config;

typedef struct
{
	pe_cache_open_fp file_open;
	pe_cache_read_fp file_read;
	pe_cache_seek_fp file_seek;
	pe_cache_feof_fp file_eof;
	pe_cache_ftell_fp file_tell;
	pe_cache_fclose_fp file_close;
}pe_cache_cb_fp;

//int pe_init(struct pe_music_cfg *music_cfg, struct pe_image_cfg *image_cfg);
int pe_cleanup();


//============================================================================
// image structure and functions

struct image_info
{
	unsigned long	fsize;
	unsigned long	width;
	unsigned long	height;
	unsigned long	bbp;
};





//slideshow effect
struct image_slideshow_effect
{
	
	enum IMAGE_SHOW_MODE mode;
	union 
	{
		Imagedec_show_shutters 	shuttles_param;
		Imagedec_show_brush 	brush_param;
		Imagedec_show_slide 	slide_param;
		Imagedec_show_show_random
								random_param;
		Imagedec_show_fade 		fade_param;
	}mode_param;
	
};


int image_abort();
int image_decode(char *file, int mode, int left, int top, int width, int height, unsigned char rotate);
int image_decode_ex(char *file, int mode, int left, int top, int width, int height, unsigned char rotate, struct image_slideshow_effect *effect);

int image_get_info(char *filename, struct image_info *info );
void image_restore_vpo_rect();
int image_zoom(struct Rect *dstRect, struct Rect *srcRect);



//============================================================================
// music structure and functions

typedef enum 
{
	Mp3,
	Wma,
	Aac,
	Rm
}MusicType;

typedef struct 
{
	char title[30];			/*标题*/
	char artist[30];		/*作者*/
	char album[30];			/*专集*/
	char year[4];			/*出品年代*/
	char comment[30];		/*备注*/
	char genre;			/*类型*/
	char track;
	unsigned long time;
	unsigned long file_length;
}MusicInfo;


typedef struct _DECODER_INFO {
	unsigned long	bit_rate;				//bps
	unsigned long	sample_rate;			//KHz
	unsigned long	channel_mode;
}DecoderInfo;

int music_play(char *filename);
void music_seek(int time);
void music_stop(void);
void music_pause(void);
int music_get_time(void);
void music_set_eq(int on, float preamp, float *bands);
int music_get_song_info(char * filename, MusicInfo *music_info);
int music_get_decoder_info(char * filename, DecoderInfo *decoder_info);

#define PE_CACHE_AUTO_THRESHOLD     0xFFFFFFFF
int pe_cache_init();
int pe_cache_open(char *filename, UINT8 *cache_buff, UINT32 cache_size, UINT32 block_size);
UINT32 pe_cache_close(int cache_id);
UINT32 pe_cache_read(int cache_id, UINT8 *buff, UINT32 size, UINT32 threshold);
UINT32 pe_cache_get(int cache_id, UINT8 **ptr, UINT32 size, UINT32 threshold);
void pe_cache_invalidate(int cache_id, UINT8 *ptr, UINT32 size);
int pe_cache_seek(int cache_id, off_t offset, int where);
int pe_cache_check_eof(int cache_id);
UINT32 pe_cache_get_data_len(int cache_id);
off_t pe_cache_tell(int cache_id);
#endif// __LIBMP_PE_H__



