#ifndef _PNG_
#define _PNG_
#include <sys_config.h>
#include <basic_types.h>
#include <mediatypes.h>

//MACRO & ENUM
#define PNG_TRUE	((INT32)1)
#define PNG_FALSE	((INT32)0)

#define PNG_MAX_INSTANCE								2

// io command
#define PNG_IOCMD_POLL_STATUS							0x00000001

enum PNG_IMG_COLOR_FORMAT_DEFINED
{
	PNG_CF_DEFINED_AUTOMATICALLY, // output the original data
	PNG_CF_DEFINED_MANUALLY // output the defined data
};

enum PNG_COLOR_FORMAT
{
	PNG_ARGB8888,
	PNG_AYCBCR8888,
	PNG_ACLUT8,
};

enum PNG_PALLET_TYPE
{
	PNG_PLT_ARGB8888,
	PNG_PLT_AYCBCR8888,
};

enum PNG_STATUS
{
	PNG_IDLE,
	PNG_IN_JOB,
};

enum PNG_MODE 
{
	PNG_SYNC,
	PNG_POLL,
};

//STRUCTURE
typedef UINT32 png_file;
typedef UINT32 png_view_hdl;

typedef UINT32 PNG_RET;

typedef struct 
{
	INT32 x;
	INT32 y;
}png_pos;

typedef struct 
{
	INT32 w;
	INT32 h;
}png_size;

struct png_cfg
{
	//address and size should be 4 bytes aligned
	UINT32 bit_buf_start;
	UINT32 bit_buf_size;
	UINT32 dec_buf_start;
	UINT32 dec_buf_size;
	UINT32 used_size;
};

struct png_file
{
	/*input parametes*/
	png_file handle;
	int (*fread)(png_file,UINT8 *, UINT32);
	BOOL (*fseek)(png_file,INT32, UINT32);
	int (*ftell)(png_file);

	UINT32 offset;
	UINT32 finish:1;
	UINT32 res:31;
};

struct png_view_par
{
	//output parameters
	png_view_hdl handle0;
	png_view_hdl handle1;
	png_view_hdl handle2;
	png_view_hdl handle3;
	png_pos pos;	
};
	
struct png_image
{
	enum PNG_IMG_COLOR_FORMAT_DEFINED cf_defined;	
	enum PNG_COLOR_FORMAT color;
	void *graphic_buf;
	png_size size;
	INT32 pitch;

	void *pallete; // output the RGB 32 bits pallete if it exist
	enum PNG_PALLET_TYPE plt_type;
	UINT32 pal_size;
};

typedef int (*png_view_routine)(struct png_image *, struct png_view_par *);

struct png_instance
{
	UINT32 core;
	UINT32 par;
	UINT16 id;
	UINT16 busy:1;
	UINT16 res:15;

	png_view_routine view_routine;
};

//ROUTINE
struct png_instance *png_open(struct png_cfg *cfg);
void png_close(struct png_instance *png);
PNG_RET png_control(struct png_instance *png, UINT32 cmd, UINT32 par);
PNG_RET png_dec(struct png_instance *png,struct png_file *file,struct png_image *img,
	enum PNG_MODE mode);
void png_stop(struct png_instance *png);
PNG_RET png_view(struct png_instance *png,struct png_image *img,struct png_view_par *par
	,png_view_routine routine);
#endif

