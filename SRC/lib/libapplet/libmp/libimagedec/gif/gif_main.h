
#ifndef _GIF_MAIN_
#define _GIF_MAIN_

#include <sys_config.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libmp/gif.h>
#include "gif_list.h"
#include "gif_info.h"
#include "gif_in.h"
#include "gif_out.h"
#include "gif_lzw.h"
#include "gif_debug.h"

//MACRO&ENUM
#define GIF_INLINE						static inline

#define GIF_MAX_INSTANCE						1
#define GIF_MAX_SUPPORT_FILES					10
#define GIF_MAX_WIDTH				       		800
#define GIF_MAX_HEIGHT							600

#define GIF_BIT_BUF_SIZE				(0x8000)
#define GIF_DEC_BUF_SIZE				(0x583400)
#define GIF_INS_BIT_BUF_SIZE			(GIF_BIT_BUF_SIZE * GIF_MAX_SUPPORT_FILES)
#define GIF_INS_DEC_BUF_SIZE			(GIF_DEC_BUF_SIZE * GIF_MAX_SUPPORT_FILES)

//main operation flag
#define GIF_FALG_IDLE_TIME				50
#define GIF_FLAG_STOP_TIME				500
#define GIF_FLAG_SYNC_MAX_NUM		16

#define GIF_FLAG_START_PTN				0x00000001
#define GIF_FLAG_RESVERD				0x0000FFFE

#define GIF_FLAG_SYNC_PTNS				0x00010000


#define SEEK_CUR    1
#define SEEK_END    2
#define SEEK_SET    0

// core control flag
#define GIF_INFO_HEADER				0x00000001
#define GIF_INFO_LSDES					0x00000002
#define GIF_INFO_IMGDES				0x00000004
#define GIF_INFO_GCE					0x00000008
#define GIF_INFO_GCE_PRIOR				0x00000010

#define GIF_END_FILE_STR				0x00010000
#define GIF_END_IMG_DECODE			0x00020000

#define GIF_INFO_MUST					0x00000007

#define GIF_DELAY_THRESHOLD			0
#define GIF_DELAY_MIN_VALUE			40
#define GIF_DELAY_EXTENTION			20
#define GIF_INVALID_MAX_TICKS			1000

#define GIF_DEFAULT_TRAN_COLOR		0xFF

enum GIF_CORE_STEP
{
	GIF_PARSE_INFO,
	GIF_LZW_IMAGE,
	GIF_SHOW_DELAY,
};

//STRUCTURE
struct gif_sync_ptns
{
	UINT32 magic_num;
	UINT8 busy;
};

struct gif_sync
{
	struct gif_sync_ptns ptns[GIF_FLAG_SYNC_MAX_NUM];
	UINT32 busy:1;
	UINT32 res:31;
};

struct gif_block
{
	/*address and size should be 4 bytes aligned*/
	UINT32 bit_buf_start;
	UINT32 bit_buf_size;
	UINT32 dec_buf_start;
	UINT32 dec_buf_size;

	UINT32 busy:1;
	UINT32 res:31;
};
struct gif_res
{
	UINT8 id;
	UINT8 active;
	struct gif_cfg par;
	struct gif_block block[GIF_MAX_SUPPORT_FILES];
};

struct gif_status
{
	enum GIF_CORE_STEP step;
	UINT32 flag;
	UINT8 first_frame_done;
	INT32 error;
};

struct gif_core_buf
{
	UINT32 start;
	INT32 size;
	UINT8 valid;
	UINT8 id;
	UINT32 start_backup;
	UINT32 hard_surface_buf;//hardware surface buffer for transparent display
};

struct gif_core
{
	struct gif_status status;
	struct gif_lzw *lzw;
	struct gif_info *info;
	struct gif_core_buf buf;
};

struct gif_context
{
	struct gif_core core;
	struct gif_in in;
	struct gif_out *out;
};

struct gif_list_unit
{
	struct gif_context text;
	struct gif_list_unit *before;
	struct gif_list_unit *next;
	UINT32 magic_num;		// normally, it is the file descriptor
	UINT32 slice; 
	UINT8 id;
	UINT32 busy:1;		
	UINT32 res:31;
};

struct gif_list
{
	struct gif_list_unit *head;
	struct gif_list_unit *tail;
	UINT8 size;
	UINT8 type;
};

struct gif_main
{
	/*pointer to the current context*/
	struct gif_context *cur;
	struct gif_list_unit *unit;
	
	struct gif_list alive;
	struct gif_list wait;
	struct gif_list dead;
	OSAL_ID task_id;
	OSAL_ID flag_id;
	UINT32 sync_id;
	UINT32 resource;

	struct gif_status_info info;
};

struct gif_dec_block_t
{
	UINT32 address;
	UINT32 size;
};

struct gif_bit_block_t
{
	UINT32 address;
	UINT32 size;
	int used;
};

//ROUTINE

//core
void gif_core_dec(struct gif_main *main);
void gif_core_dec_stepon(struct gif_core *core);
void gif_core_set_error(struct gif_context *text ,INT32 num);
UINT32 gif_core_error(struct gif_context *text);


//list operation
struct gif_list_unit *gif_list_fetch_unit(struct gif_list *list);
struct gif_list_unit *gif_list_next_unit(struct gif_list *list);
GIF_RET gif_list_add_unit(struct gif_list *list,struct gif_list_unit *unit);
GIF_RET gif_list_delete_unit(struct gif_list *list,struct gif_list_unit *punit1,struct gif_list_unit **punit2);
void gif_list_enter_mutex(void);
void gif_list_exit_mutex(void);
GIF_RET gif_list_init(struct gif_main *main);
void gif_list_free(struct gif_main *main);

//lzw
GIF_RET gif_lzw_make_ready(struct gif_context *text);
GIF_RET gif_lzw_image_done(struct gif_context *text);
void gif_lzw_decompress(struct gif_context *text);
GIF_RET gif_lzw_init(struct gif_context * text);

//info
GIF_RET gif_info_parse(struct gif_context *text);
GIF_RET gif_info_init(struct gif_context *text);

//in
INT16 gif_in_next_byte(struct gif_in *in);
INT16 gif_in_read_byte(struct gif_in *in);
INT16 gif_in_read_bits(struct gif_in *in,UINT8 num);
GIF_RET gif_in_init(struct gif_context *text,struct gif_dec_in_par *par,struct gif_res *res);

//out
void gif_out_init_ge(void);
void gif_out_init_osd(void);
	
//sync
GIF_RET gif_sync_alloc(struct gif_main *main);
void gif_sync_free(struct gif_main *main);
GIF_RET gif_sync_wait(struct gif_main *main,UINT32 magic_num,UINT32 time_out);
void gif_sync_set(struct gif_main *main,UINT32 magic_num);

#endif

