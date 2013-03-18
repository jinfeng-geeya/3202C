#include <sys_config.h>

#ifdef SHOW_PICS
#if(SHOW_PICS == SHOW_SINGLE_PIC)

#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <hld/ge/ge.h>
#include <hld/dis/vpo.h>
//#include <api/libimagedec/imagedec.h>
//#include <api/libmp/png.h>

#include "ad_pic_gfx.h"
#include "ad_pic_png.h"


#define AD_PNG_DEBUG
#ifdef AD_PNG_DEBUG
#define AD_PNG_PRINTF	libc_printf
#else
#define AD_PNG_PRINTF(...)	do{}while(0)
#endif

//#define PNG_OUT_CLUT8_PIC


static struct png_instance *png = NULL;

static UINT8 *png_file_addr;
static UINT32 png_file_size;
static INT32 png_input_pos = 0;


static int png_read_data(UINT32 fh,BYTE *buf, UINT32 size)
{
	UINT32 read_pos;
	UINT32 read_len;

	read_pos = png_input_pos+size;
	if(read_pos>png_file_size)
	{
		read_len = png_file_size-png_input_pos;
		MEMCPY((void *)buf, (void *)(png_file_addr+png_input_pos), read_len);
		png_input_pos += read_len;
		return read_len;
	}
	else
	{
		MEMCPY((void *)buf, (void *)(png_file_addr+png_input_pos), size);
		png_input_pos+=size;
		return size;
	}
}

static int png_seek_data(UINT32 fh,long offset, UINT32 origin)
{
	if(origin == 0)
	{
		png_input_pos = (UINT32)offset;
		return TRUE;
	}
	return FALSE;
}

static int png_tell_pos(UINT32 fh)
{
	return png_input_pos;
}


static PNG_RET png_view_routine_ge(struct png_image *img, struct png_view_par *par)
{
	ge_rect_t dst_rect, src_rect;
	ge_surface_desc_t *src_surf, *dst_surf;
	struct ge_device *ge_dev;
	INT32 ret;

	ge_dev = (struct ge_device *)dev_get_by_id(HLD_DEV_TYPE_GE, 0);
	if (ge_dev == NULL)
	{
		AD_PNG_PRINTF("%s() ge device find failed!\n", __FUNCTION__);
		return -1;
	}

	src_surf = (ge_surface_desc_t *)ad_pic_get_ge_surface(AD_SRC_SURFACE);
	dst_surf = (ge_surface_desc_t *)ad_pic_get_ge_surface(AD_DST_SURFACE);

	if (src_surf == NULL || dst_surf == NULL)
		return -1;
	
	src_rect.left = 0;
	src_rect.top = 0;
	src_rect.width = img->size.w;
	src_rect.height = img->size.h;
	
	dst_rect.left = par->pos.x;
	dst_rect.top = par->pos.y;
	dst_rect.width = src_rect.width;
	dst_rect.height = src_rect.height;

	src_surf->region[3].addr = (UINT8 *)img->graphic_buf;
	src_surf->region[3].pitch = img->pitch;
	src_surf->region[3].rect.width = src_rect.width;
	src_surf->region[3].rect.height = src_rect.height;
	src_surf->region[3].pixel_fmt = GE_PF_AYCBCR8888;
	src_surf->region[3].pixel_bits = 32;

	//dst_surf->alpha_blend.glob_alpha_en = 0;
	//dst_surf->alpha_blend.glob_alpha = 0x7f;
	//dst_surf->alpha_blend.src_mode = GE_AB_SRC_NOT_PREMULTI;

	ge_lock(ge_dev);

	//ge_set_cur_region(ge_dev, dst_surf, 0);
	//ge_set_cur_region(ge_dev, src_surf, 0);

	ge_set_cur_region(ge_dev, dst_surf, 0);
	ge_set_cur_region(ge_dev, src_surf, 3);

	

	ret = ge_blt(ge_dev, dst_surf, src_surf, &dst_rect, &src_rect, GE_BLT_SRC2_VALID | GE_BLT_YUV2RGB);

	//ret = ge_blt_ex(ge_dev, dst_surf, dst_surf, src_surf, &dst_rect, &dst_rect, &src_rect, 
	//	GE_BLT_SRC2_VALID | GE_BLT_SRC1_VALID | GE_BLT_ALPHA_BLEND);
	ge_unlock(ge_dev);
	
	return ret;
}

#define PNG_FILE_SIZE		762

#define PNG_BIT_BUF			(__MM_IMAGEDEC_BUF_ADDR&0x8FFFFFFF)  //0xA1100000
#define PNG_BIT_SIZE		0x8000

#define PNG_DEC_BUF		    ((PNG_BIT_BUF + PNG_BIT_SIZE)&0x8FFFFFFF)  //0xA1200000
//#define PNG_DEC_SIZE		0x84600  //w*h*4 + 0x30000 //0x200000
#define PNG_DEC_SIZE		((__MM_IMAGEDEC_MEM_LEN - PNG_BIT_SIZE)&0x8FFFFFFF)

//void ad_png_init(void)
void ad_pic_png_init(void)
{
	struct png_cfg cfg;
	
	if(png != NULL)
		return;
	
	MEMSET((void *)&cfg,0,sizeof(struct png_cfg));
	cfg.bit_buf_start = PNG_BIT_BUF;
	cfg.bit_buf_size = PNG_BIT_SIZE;
	cfg.dec_buf_start = PNG_DEC_BUF;
	cfg.dec_buf_size =  PNG_DEC_SIZE;
	png = png_open(&cfg);    
}

//static void ad_png_dec(struct OSDRect *rect)
static void ad_pic_png_dec(struct OSDRect *rect)
{
	struct png_file file;
	MEMSET((void *)&file, 0, sizeof(struct png_file));
	file.handle = 0x56;
	file.fread = png_read_data;
	file.fseek = png_seek_data;
	file.ftell = png_tell_pos;

	struct png_image img;
	MEMSET((void *)&img, 0, sizeof(struct png_image));
#ifdef PNG_OUT_CLUT8_PIC
	img.cf_defined = PNG_CF_DEFINED_AUTOMATICALLY;
#else
	img.cf_defined = PNG_CF_DEFINED_MANUALLY; 
#endif
	img.plt_type = PNG_PLT_AYCBCR8888;
    
	png_dec(png, &file, &img, PNG_SYNC);  //the synchronous mode

	struct png_view_par view_par;
	MEMSET((void *)&view_par, 0, sizeof(struct png_view_par));
	view_par.handle0 = 0;
	view_par.pos.x = rect->uLeft;
	view_par.pos.y = rect->uTop;//0;
	png_view(png, &img, &view_par, png_view_routine_ge);
}

//INT32 ad_png_draw(UINT8* file, UINT32 file_len, struct OSDRect *rect)
INT32 ad_pic_png_draw(UINT8* file, UINT32 file_len, struct OSDRect *rect)
{
	png_input_pos = 0;
	png_file_addr = (UINT8*)file;
	png_file_size = file_len;

	ad_pic_png_init();
	ad_pic_png_dec(rect);	
	return SUCCESS;
}

void ad_pic_png_close(void)
{

	if (png == NULL)
		return;
/*	png_stop(png);
*/	png_close(png);
    png = NULL;

//	png->busy = 0;  //for the synchronous mode for png_dec(png, &file, &img, PNG_SYNC);
}

#endif
#endif

