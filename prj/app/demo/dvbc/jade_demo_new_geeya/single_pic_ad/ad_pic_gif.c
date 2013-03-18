#include <sys_config.h>

#ifdef SHOW_PICS
#if(SHOW_PICS == SHOW_SINGLE_PIC)

#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/ge/ge.h>
#include <hld/osd/osddrv_dev.h>
#include <api/libmp/gif.h>

#include "ad_pic_gfx.h"


#define AD_GIF_DEBUG

#ifdef AD_GIF_DEBUG
#define AD_GIF_PRINTF	libc_printf
#else
#define AD_GIF_PRINTF(...)	do{}while(0)
#endif

struct gif_file_list
{
	gif_file file;
	UINT8 *addr;
	INT32 pos;
	UINT32 size;
};

static struct gif_instance *gif = NULL;

#define MAX_GIF_FILE	256
static UINT32 gif_file_cnt = 0;
static struct gif_file_list gif_list[MAX_GIF_FILE];

static void gif_list_init(void)
{
	int i;

	for (i = 0; i < MAX_GIF_FILE; i++)
	{
		gif_list[i].file = 0xFFFFFFFF;
	}
}

static INT32 gif_get_file_in_list(gif_file fh)
{
	int i;

	for (i = 0; i < MAX_GIF_FILE; i++)
	{
		if (gif_list[i].file == fh)
			return i;
	}

	if (i == MAX_GIF_FILE)
		return -1;
}

static void gif_file_init(gif_file fh, UINT8 *addr, UINT32 size)
{
	int i = gif_get_file_in_list(fh);
	
	gif_list[i].addr = addr;
	gif_list[i].size = size;
	gif_list[i].pos = 0;
	return;
}

static int gif_read_data(gif_file fh, UINT8 *buf, UINT32 size)
{
	UINT32 read_pos;
	UINT32 read_len;
	int i = gif_get_file_in_list(fh);

	read_pos = gif_list[i].pos+size;
	if(read_pos>gif_list[i].size)
	{
		read_len = gif_list[i].size-gif_list[i].pos;
		MEMCPY((void *)buf, (void *)(gif_list[i].addr+gif_list[i].pos), read_len);
		gif_list[i].pos+=read_len;
		return read_len;
	}
	else
	{
		MEMCPY((void *)buf, (void *)(gif_list[i].addr+gif_list[i].pos), size);
		gif_list[i].pos+=size;
		return size;
	}
}

static BOOL gif_seek_data(gif_file fh, INT32 offset, UINT32 origin)
{
	int i = gif_get_file_in_list(fh);
	
	if(origin == 0)		//only support seek from begin;other not considered...
	{
		gif_list[i].pos = offset;
		return TRUE;
	}
	return FALSE;
}

static int gif_tell_pos(gif_file fh)
{
	int i = gif_get_file_in_list(fh);
	
	return gif_list[i].pos;
}

static INT32 ad_pic_gif_file_add(gif_file file)
{
	int i;
	
	if (gif_file_cnt >= MAX_GIF_FILE)
		return !SUCCESS;

	for (i = 0; i < MAX_GIF_FILE; i++)
	{
		if (gif_list[i].file == 0xFFFFFFFF)
		{
			gif_list[i].file = file;
			gif_file_cnt++;
			break;
		}
	}

	if (i == MAX_GIF_FILE)
		return !SUCCESS;
	
	return SUCCESS;
}

static INT32 ad_pic_gif_file_remove(gif_file file)
{
	int i;

	for (i = 0; i < MAX_GIF_FILE; i++)
	{
		if (gif_list[i].file == file)
		{
			gif_list[i].file = 0xFFFFFFFF;
			gif_file_cnt--;
			break;
		}
	}

	if (i == MAX_GIF_FILE)
		return !SUCCESS;
	return SUCCESS;
}

void ad_pic_gif_init(void)
{
	struct gif_cfg init;

	if(gif != NULL)
		return ;
    
	MEMSET((void *)&init,0,sizeof(struct gif_cfg));

	init.bit_buf_start = (UINT8 *)((__MM_IMAGEDEC_BUF_ADDR+0x10000) & ~(3<<29));//(UINT8 *)(__MM_GIF_BIT_ADDR & ~(7<<28));
	                                                                        //the size of processing pic is less than 0x10000
	init.bit_buf_size =  0X8000;  
	init.dec_buf_start = (UINT8 *)((__MM_IMAGEDEC_BUF_ADDR+0x18000) & ~(3<<29)); //(UINT8 *)(__MM_GIF_DEC_ADDR & ~(7<<28));
	init.dec_buf_size =  0x60000;  //0x5E600 = 360(d)*240(d)*4+0xa000

	init.fread_callback = gif_read_data;
	init.fseek_callback = gif_seek_data;
	init.ftell_callback = gif_tell_pos;	
	init.max_width = 360; 
	init.max_height = 240; 
	
	gif_init(&init);
	gif_list_init();
	gif = gif_open();
}


INT32 ad_pic_gif_draw(gif_file fh, UINT8* file, UINT32 file_len, struct OSDRect  *rect, BOOL animation)
{
	struct gif_dec_par par;
	struct ge_device* ge_dev;

	ge_dev = (struct ge_device *)dev_get_by_id(HLD_DEV_TYPE_GE, 0);
	if (ge_dev == NULL)
	{
		AD_GIF_PRINTF("%s() ge device find failed!\n", __FUNCTION__);
		return !SUCCESS;
	}
	if (SUCCESS != ad_pic_gif_file_add(fh))
		return !SUCCESS;
	
	gif_file_init(fh, file, file_len);

	if (animation)
		gif_io_ctrl(gif, GIF_IO_CMD_DEC_FIRST_PIC, GIF_FALSE);
	else
		gif_io_ctrl(gif, GIF_IO_CMD_DEC_FIRST_PIC, GIF_TRUE);

	MEMSET((void *)&par, 0, sizeof(struct gif_dec_par));
	par.in_par.file = fh;
	par.in_par.fread_callback = gif_read_data;
	par.in_par.fseek_callback = gif_seek_data;
	par.in_par.ftell_callback = gif_tell_pos;

	par.out_par.dev = (gif_ge_dev)ge_dev;
	par.out_par.handle = (gif_surface_handle)ad_pic_get_ge_surface(AD_DST_SURFACE);
	if(par.out_par.handle == NULL)
		return !SUCCESS;
	//par.out_par.region = 0;
	par.out_par.region = 0;//ad_pic_get_ge_region_id();
	par.out_par.pos.x = rect->uLeft;
	//par.out_par.pos.y = 0; //rect->uTop; //for the region
	par.out_par.pos.y = rect->uTop; //0;//for the region
	par.out_par.area.x = par.out_par.area.y = 0;
	par.out_par.area.w = rect->uWidth;
	par.out_par.area.h = rect->uHeight;
	
	gif_dec(gif, &par);

	return SUCCESS;
}

void ad_pic_gif_stop(gif_file file)
{
	if (gif == NULL)
		return;
	
	gif_stop(gif, file);
	osal_task_sleep(100);
	ad_pic_gif_file_remove(file);
}

void ad_pic_gif_close(void)
{
	UINT32 i;
	
	if (gif == NULL)
		return;
	
	for (i = 0; i < MAX_GIF_FILE; i++)
	{
		if (gif_list[i].file != 0xFFFFFFFF)
		{
			gif_stop(gif, gif_list[i].file);
			ad_pic_gif_file_remove(gif_list[i].file);
		}
	}
    
	gif_close(gif);
	gif = NULL;
}

#endif
#endif

