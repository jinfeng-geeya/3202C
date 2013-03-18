
#ifndef _GIF_
#define _GIF_

#include <sys_config.h>
#include <basic_types.h>
#include <mediatypes.h>

//MACRO&ERUM
#define GIF_TRUE	((INT32)1)
#define GIF_FALSE	((INT32)0)

// io cmd
#define GIF_IO_CMD_ROTATE							0x00000001
#define GIF_IO_CMD_DEC_FIRST_PIC					0x00000002
#define GIF_IO_CMD_IGNORE_BACKGROUND_COLOR		0x00000003
#define GIF_IO_CMD_GET_STATUS_INFO				0x00000004

/* set one color as transpanrent color. the color format is RGB888 */
#define GIF_IO_CMD_SET_TRANS_COLOR				0x00000005 

#define GIF_IO_CMD_FORCE_SHOW_PIC				0x00000006

enum GIF_BIT_DEPTH
{
	BITS8_PIXEL,
	BITS16_PIXEL,
	BITS32_PIXEL,
};

//STRUCTURE
typedef UINT32 GIF_RET;

typedef UINT32 gif_file;
typedef UINT32 gif_ge_dev;
typedef UINT32 gif_surface_handle;
typedef UINT8 gif_region_id;

typedef struct 
{
	INT32 x;
	INT32 y;
}gif_pos;

typedef struct 
{
	INT32 w;
	INT32 h;
}gif_size;

typedef struct 
{
	INT32 x;
	INT32 y;
	INT32 w;
	INT32 h;
}gif_rect;
	
struct gif_cfg
{
	//address and size should be 4 bytes aligned
	UINT8 *bit_buf_start;
	UINT32 bit_buf_size;
	UINT8 *dec_buf_start;
	UINT32 dec_buf_size;
	
	//file operation callback function
	int (*fread_callback)(gif_file,UINT8 *, UINT32);
	BOOL (*fseek_callback)(gif_file,INT32, UINT32);
	int (*ftell_callback)(gif_file);

	//others
	UINT32 max_width;
	UINT32 max_height;

	UINT32 only_dec_first_pic:1;
	UINT32 reserved:31;
};

struct gif_dec_out_par
{
	//output parameters
	gif_ge_dev dev;
	gif_surface_handle handle;
	gif_region_id region;
	gif_pos pos;	// the position of the left-top pointer
	gif_rect area; // choose the displayed area when meet the gif pic which's resolution is bigger than this area
	
	//others
	UINT32 res1;
	UINT32 res2;
	UINT32 gif_delay_num;
	gif_size size;
};

struct gif_dec_in_par
{
	/*input parametes*/
	gif_file file;
	int (*fread_callback)(gif_file,UINT8 *, UINT32);
	BOOL (*fseek_callback)(gif_file,INT32, UINT32);
	int (*ftell_callback)(gif_file);
	int frame_num;
};

struct gif_dec_par
{
	struct gif_dec_out_par out_par;
	struct gif_dec_in_par in_par;
};

struct gif_instance
{
	UINT8 id;
	UINT32 ins;
	UINT32 par;
};

struct gif_status_info
{
	UINT32 first_pic:1;
	UINT32 res:31;
};

//ROUTINE
GIF_RET gif_init(struct gif_cfg *init);
struct gif_instance *gif_open(void);
GIF_RET gif_close(struct gif_instance *gif);
GIF_RET gif_dec(struct  gif_instance *gif,struct gif_dec_par *par);
GIF_RET gif_stop(struct gif_instance *gif,gif_file file);
GIF_RET gif_io_ctrl(struct gif_instance *gif,UINT32 io_cmd,UINT32 io_par);

#endif

