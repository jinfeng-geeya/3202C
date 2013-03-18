#include <sys_config.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <os/tds2/itron.h>
#include <api/libc/printf.h>
#include <osal/osal_timer.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include "imagedec_main.h"
#include "imagedec_bmp_decoder.h"
#include "imagedec_osd.h"

/*row count of input and output*/
static volatile int g_bmp_src_row_cnt = 0;
static volatile int g_bmp_dst_row_cnt = 0;
static volatile int g_bmp_dst_row_pos = 0;
static volatile int g_bmp_dst_col_cnt = 0;
static volatile int g_bmp_dst_col_pos = 0;
static volatile int g_bmp_dst_stride = 0;
static volatile int g_bmp_line_inverse_seq = 1;
/*scale down variables*/
static volatile int g_bmp_scale_hor_int = 0;
static volatile int g_bmp_scale_hor_fra = 0;
static volatile int g_bmp_scale_ver_int = 0;
static volatile int g_bmp_scale_ver_fra	= 0;
static volatile int g_bmp_scale_factor_inverse_2D = 0;
static volatile int *g_bmp_scale_bilinear_buf_y = NULL;
static volatile int *g_bmp_scale_partsum_buf_y = NULL;
static volatile int *g_bmp_scale_bilinear_buf_cb = NULL;
static volatile int *g_bmp_scale_partsum_buf_cb = NULL;
static volatile int *g_bmp_scale_bilinear_buf_cr = NULL;
static volatile int *g_bmp_scale_partsum_buf_cr = NULL;
static volatile int *g_bmp_scale_output_buf = NULL;
extern int image_data_end;
extern volatile int g_imagedec_info_valid[IMAGEDEC_MAX_INSTANCE];
extern volatile int g_imagedec_get_info[IMAGEDEC_MAX_INSTANCE];
extern enum IMAGE_ANGLE g_imagedec_angle[IMAGEDEC_MAX_INSTANCE];
extern imagedec_osd_ins g_imagedec_osd_ins;

#define STORE_MB_DATA
IMAGEDEC_STATIC void bmp_line_2_mb(UINT32 *dst_y,UINT32 *dst_c,UINT32 stride,UINT32 row,UINT32 col,UINT32 *src,int width)
{
	UINT8 *dst_y_start = (UINT8 *)dst_y;
	UINT8 *dst_c_start = (UINT8 *)dst_c;
	int col_cnt = col&0xF;
	UINT32 data = 0;
	UINT32 cnt = 0;
	int i,c_flag = 0;

	jpeg_printf("row<%d>col<%d>width<%d>\n",row,col,width);
	if(!(row&0x01))
		c_flag = 1;
	dst_y_start += (row&0xFFF0)*stride + ((row&0x0F)<<4) + ((col&0xFFF0)<<4) + (col&0x0F);
	dst_c_start += ((row>>5)<<4)*stride + (((row>>4)&0x1)<<7) + (((row>>1)&0x7)<<4)+ ((col&0xFFF0)<<4) + (col&0xE);
	if(ANG_180 == g_imagedec_angle[0])
	{
		for(i=0;i<width;i++)
		{
			data = *src++;
			*dst_y_start-- = (data>>16)&0xFF;
			if((!(col_cnt&0x01))&&(c_flag))
			{
				*dst_c_start++ = (data>>8)&0xFF;
				*dst_c_start = data&0xFF;
				dst_c_start -= 3;
			}
			if(0 == (col_cnt&0xF))
			{
				col_cnt = 16;
				dst_y_start -= 240;
				dst_c_start -= 240;
			}
			col_cnt--;
		}
	}
	else
	{
		for(i=0;i<width;i++)
		{
			data = *src++;
			*dst_y_start++ = (data>>16)&0xFF;
			if((!(col_cnt&0x01))&&(c_flag))
			{
				*dst_c_start++ = (data>>8)&0xFF;
				*dst_c_start++ = data&0xFF;
			}
			col_cnt++;
			if(16 == (col_cnt&0x1F))
			{
				col_cnt = 0;
				dst_y_start += 240;
				dst_c_start += 240;
			}
		}
	}
}
IMAGEDEC_STATIC void bmp_line_2_mb_90(UINT32 *dst_y,UINT32 *dst_c,UINT32 stride,UINT32 row,UINT32 col,UINT32 *src,int height)
{
	UINT8 *dst_y_start = (UINT8 *)dst_y;
	UINT8 *dst_c_start = (UINT8 *)dst_c;
	int col_cnt = col&0xF;
	int row_cnt = row;
	UINT32 data = 0;
	UINT32 cnt = 0;
	UINT32 s = (stride<<4) - 256;
	int i,c_flag = 0;

	jpeg_printf("row<%d>col<%d>width<%d>\n",row,col,height);
	if(!(row&0x01))
		c_flag = 1;
	dst_y_start += (row&0xFFF0)*stride + ((row&0x0F)<<4) + ((col&0xFFF0)<<4) + (col&0x0F);
	dst_c_start += ((row>>5)<<4)*stride + (((row>>4)&0x1)<<7) + (((row>>1)&0x7)<<4)+ ((col&0xFFF0)<<4) + (col&0xE);
	if(ANG_90_A== g_imagedec_angle[0])
	{
		for(i=0;i<height;i++)
		{
			data = *src++;
			*dst_y_start = (data>>16)&0xFF;
			dst_y_start += 16;
			if((!(col_cnt&0x01))&&(c_flag))
			{
				*dst_c_start++ = (data>>8)&0xFF;
				*dst_c_start = data&0xFF;
				dst_c_start += 15;
				if(0xF == ((row_cnt>>1)&0xF))
				{
					dst_c_start += s;
				}
			}
			if(0xF == (row_cnt&0xF))
			{
				dst_y_start += s;
			}
			row_cnt++;
			if(!(row_cnt&0x01))
				c_flag = 1;
			else
				c_flag = 0;
		}
	}
	else
	{
		for(i=0;i<height;i++)
		{
			data = *src++;
			*dst_y_start = (data>>16)&0xFF;
			dst_y_start -= 16;
			if((!(col_cnt&0x01))&&(c_flag))
			{
				*dst_c_start++ = (data>>8)&0xFF;
				*dst_c_start = data&0xFF;
				dst_c_start -= 17;
				if(0 == ((row_cnt>>1)&0xF))
				{
					dst_c_start -= s;
				}
			}
			if(0 == (row_cnt&0xF))
			{
				dst_y_start -= s;
			}
			row_cnt--;
			if(!(row_cnt&0x01))
				c_flag = 1;
			else
				c_flag = 0;
		}
	}
}
#define DUPLICATION
static volatile int p_x,p_y,e_x,e_y,l_x,l_y;
IMAGEDEC_STATIC void bmp_init_dup_para(pImagedec_hdl phdl)
{
	int width = phdl->decoder.image_x_size;
	int height = phdl->decoder.image_y_size;
	
	p_x = phdl->main.setting.src.uStartX;
	p_y = phdl->main.setting.src.uStartY;
	e_x = phdl->main.setting.src.uWidth + p_x;
	e_y = phdl->main.setting.src.uHeight + p_y;
	width -= p_x;
	if(width <= 0)
		{JPEG_ASSERT(0);}
	else if(width >= phdl->main.setting.src.uWidth)
		l_x = phdl->main.setting.src.uWidth;
	else if(width < phdl->main.setting.src.uWidth)
	{
		l_x = width;
		e_x = p_x + l_x;
	}
	else if(width > phdl->main.setting.src.uWidth)
		l_x = phdl->main.setting.src.uWidth;
	height -= p_y;
	if(height <= 0)
		{JPEG_ASSERT(0);}
	else if(height >= phdl->main.setting.src.uHeight)
		l_y = phdl->main.setting.src.uHeight;
	else if(height < phdl->main.setting.src.uHeight)
	{
		l_y = height;
		e_y = p_y + l_y;
	}
	g_bmp_dst_row_cnt = l_y - 1;
	g_bmp_dst_col_pos = 0;
	if(!g_imagedec_osd_ins.on)
	{
		/*operation when size of src is smaller than 720X576*/
		if(width<TV_HOR_COL_NUM)
		{
			g_bmp_dst_col_pos = (TV_HOR_COL_NUM - width)>>1;
			if(g_bmp_dst_col_pos&0x01)
				g_bmp_dst_col_pos--;
			p_x = 0;e_x = p_x + width;
		}
		if(height<TV_VER_LINE_NUM)
		{	
			g_bmp_dst_row_cnt = (TV_VER_LINE_NUM + height)>>1;
			if(g_bmp_dst_row_cnt&0x1)
				g_bmp_dst_row_cnt--;
			p_y = 0;e_y = p_y + height;
		}
		g_bmp_dst_stride = TV_HOR_COL_NUM;		
	}
	else
		g_bmp_dst_stride = g_imagedec_osd_ins.pic.stride;
	jpeg_printf("p_x <%d>p_y <%d>e_x<%d>e_y <%d>dst_x<%d>dst_y<%d>\n",p_x,p_y
		,e_x,e_y,g_bmp_dst_col_pos,g_bmp_dst_row_cnt);
	phdl->decoder.bmp_real_size_done = 0;	
}

IMAGEDEC_STATIC void bmp_direct_dup(void *para1,void *para2)
{
	pImagedec_hdl phdl = (pImagedec_hdl) para1;
	UINT32 *src = (UINT32 *)para2;
	UINT32 *y_d = (UINT32 *)phdl->imageout.frm_y_addr;
	UINT32 *c_d = (UINT32 *) phdl->imageout.frm_c_addr;

	if(g_imagedec_osd_ins.on)
	{
		y_d = (UINT32 *)g_imagedec_osd_ins.pic.y_buf;
		c_d = (UINT32 *)g_imagedec_osd_ins.pic.c_buf;
	}
	
	if(g_bmp_src_row_cnt < p_y)
	{
		phdl->decoder.bmp_real_size_done = 1;
		jpeg_printf("real size done <%d>\n",g_bmp_src_row_cnt);
		return;
	}
	if(g_bmp_src_row_cnt <= e_y)
	{
		src += p_x;
		bmp_line_2_mb(y_d,c_d,g_bmp_dst_stride,g_bmp_dst_row_cnt,g_bmp_dst_col_pos,src,l_x);
		if(g_bmp_line_inverse_seq)
			g_bmp_dst_row_cnt--;
		else
			g_bmp_dst_row_cnt++;
	}
}
#define BILINEAR_SCALING
static int scale_precision,scale_precisionX2;		
static int scale_bits_field;
static int scale_fra_total_value;
static int ver_part_sum_cnt,ver_int_cnt,ver_fra_counter;
/*	Description	:	scale down the original size to 720X576.if the size is smaller than 720X576, just store it without 
*					scale up.
*					integer	= 	src/dst - 1;	
*					tmp_size = src/integer;
*					fraction 	= 	tmp_size/dst;
*/
IMAGEDEC_STATIC void bmp_init_scale_para(pImagedec_hdl phdl)
{
	int width = phdl->decoder.image_x_size;
	int height = phdl->decoder.image_y_size;
	int widthX3 = width*3;
	int heightX4 = height<<2;
	int dst_w = phdl->main.setting.dis.uWidth;
	int dst_h = phdl->main.setting.dis.uHeight;
	int dst_x = phdl->main.setting.dis.uStartX;
	int dst_y = phdl->main.setting.dis.uStartY;
	int bk_w = dst_w;
	int bk_h = dst_h;
	int blank_x = 0;
	int blank_y = 0;
	int dec_width,dec_height,int_scaled_width = 0,int_scaled_height = 0;

	if(IMAGEDEC_FULL_SRN == phdl->main.setting.mode)
	{
		switch(g_imagedec_angle[phdl->id])
		{
			case ANG_90_A:
			case ANG_90_C:
				width = phdl->decoder.image_y_size;
				height = phdl->decoder.image_x_size;
				widthX3 = width*3;
				heightX4 = height<<2;
				break;
			default:
				break;
		}
	}
	/*buffer for the decoder*/
	g_bmp_scale_bilinear_buf_y = (int *)imagedec_malloc(2880);
	g_bmp_scale_partsum_buf_y = (int *)imagedec_malloc(2880);
	g_bmp_scale_bilinear_buf_cb = (int *)imagedec_malloc(2880);
	g_bmp_scale_partsum_buf_cb = (int *)imagedec_malloc(2880);
	g_bmp_scale_bilinear_buf_cr = (int *)imagedec_malloc(2880);
	g_bmp_scale_partsum_buf_cr = (int *)imagedec_malloc(2880);
	g_bmp_scale_output_buf = (int *)imagedec_malloc(2880);	
	jh_mem_set((void *)g_bmp_scale_bilinear_buf_y,0,2880);
	jh_mem_set((void *)g_bmp_scale_partsum_buf_y,0,2880);
	jh_mem_set((void *)g_bmp_scale_bilinear_buf_cb,0,2880);
	jh_mem_set((void *)g_bmp_scale_partsum_buf_cb,0,2880);
	jh_mem_set((void *)g_bmp_scale_bilinear_buf_cr,0,2880);
	jh_mem_set((void *)g_bmp_scale_partsum_buf_cr,0,2880);
	jh_mem_set((void *)g_bmp_scale_output_buf,0,2880);	
	/*align w:h = 4:3*/
	if(widthX3 > heightX4)
	{
		dec_width = width;
		dec_height = widthX3>>2;
		blank_x = 0;
		blank_y = dec_height - height;
	}
	else if(widthX3 < heightX4)
	{
		dec_width = heightX4/3;
		dec_height = height;
		blank_x = dec_width - width;
		blank_y = 0;
	}
	else
	{
		dec_width = width;
		dec_height = height;
	}
	/*FULL SRN dst width and pos redefine*/
	if(IMAGEDEC_FULL_SRN == phdl->main.setting.mode)
	{
		dst_x = dst_y = 0;
		if((dec_width < bk_w) && (dec_height < bk_h))
		{
			dst_w = dec_width;
			dst_h = dec_height;
			if(!g_imagedec_osd_ins.on)
			{
				dst_x = (bk_w - dst_w)>>1;
				dst_y = (bk_h - dst_h)>>1;
			}			
		}
		
		if(g_imagedec_osd_ins.on)
		{
			dst_w = dec_width = width;
			dst_h = dec_height = height;
			blank_x = blank_y = 0;
			g_bmp_dst_stride = g_imagedec_osd_ins.pic.stride;
		}
		else
			g_bmp_dst_stride = TV_HOR_COL_NUM;
		
		jpeg_printf("Full rect w <%d>h<%d>x <%d>y<%d>\n",dst_w,dst_h,dst_x,dst_y);
	}
	else if (IMAGEDEC_THUMBNAIL == phdl->main.setting.mode)//Thumbnail mode
	{
		bk_w = phdl->imageout.logo_info.width;
		bk_h = phdl->imageout.logo_info.height;
		g_bmp_dst_stride = phdl->imageout.logo_info.stride<<4;
		dst_w = (dst_w * bk_w) / TV_HOR_COL_NUM;
		dst_h = (dst_h * bk_h) /TV_VER_LINE_NUM;
		dst_x = (dst_x * bk_w)/TV_HOR_COL_NUM;
		dst_y = (dst_y * bk_h)/TV_VER_LINE_NUM;
		jpeg_printf("thumb src dec_w<%d>dec_h<%d>dst_x<%d>dst_y<%d>dst_w<%d>dst_h<%d>\n",dec_width,dec_height
			,dst_x,dst_y,dst_w,dst_h);
		if((dec_width < dst_w) && (dec_height < dst_h))
		{
			dst_x += (dst_w - dec_width)>>1;		
			dst_y += (dst_h - dec_height)>>1;		
			dst_w = dec_width;
			dst_h = dec_height;
		}
		jpeg_printf("Thumb rect w <%d>h<%d>x <%d>y<%d>\n",dst_w,dst_h,dst_x,dst_y);
	}
	/*blank col and row cnt and update it into dst position*/
	if(blank_x)
	{
		blank_x = (blank_x*dst_w)/dec_width;
		jpeg_printf("blank x <%d>\n",blank_x);
		dst_x += blank_x>>1;
	}
	if(blank_y)
	{
		blank_y = (blank_y*dst_h)/dec_height;
		jpeg_printf("blank y <%d>\n",blank_y);
		dst_y += blank_y>>1;
	}
	/*init the scale parameters*/	
	g_bmp_scale_hor_int = 1;
	g_bmp_scale_hor_fra = 256;		
	g_bmp_scale_ver_int = 1;
	g_bmp_scale_ver_fra = 256;	
	if((dec_width < dst_w) && (dec_height < dst_h))
	{
		jpeg_printf("not support scale up <%s>\n",__FUNCTION__);
		terminate(JPGD_NOT_SUPPORT_SCALE_UP);
	}
	if(dec_width != dst_w)
	{
		g_bmp_scale_hor_int = 1;
		do
		{
			g_bmp_scale_hor_int++;
			int_scaled_width = dec_width/g_bmp_scale_hor_int;
		}while(int_scaled_width > dst_w);
		g_bmp_scale_hor_int--;
		int_scaled_width = dec_width/g_bmp_scale_hor_int;
		g_bmp_scale_hor_fra = (int_scaled_width<<8)/dst_w;
		if(g_bmp_scale_hor_fra * dst_w != (int_scaled_width<<8))
			g_bmp_scale_hor_fra++;
	}
	if(dec_height != dst_h)
	{
#if 1	
		g_bmp_scale_ver_int = 1;
		do
		{
			g_bmp_scale_ver_int++;
			int_scaled_height = dec_height/g_bmp_scale_ver_int;
		}while(int_scaled_height > dst_h);
		g_bmp_scale_ver_int--;		
#else
		g_bmp_scale_ver_int = g_bmp_scale_hor_int;
#endif
		int_scaled_height = dec_height/g_bmp_scale_ver_int;
		g_bmp_scale_ver_fra = (int_scaled_height<<8)/dst_h;
		if(g_bmp_scale_ver_fra * dst_h != (int_scaled_height<<8))
			g_bmp_scale_ver_fra++;
	}
	scale_precision = 8;scale_precisionX2 = 16;
	scale_bits_field = 0xFF;
	scale_fra_total_value = 0x100;
	if((g_bmp_scale_ver_int > 10) || (g_bmp_scale_hor_int > 10))
	{
		g_bmp_scale_hor_fra = (int_scaled_width<<4)/dst_w;
		g_bmp_scale_ver_fra = (int_scaled_height<<4)/dst_h;
		scale_precision = 4;scale_precisionX2 = 8;
		scale_bits_field = 0xF;
		scale_fra_total_value = 0x10;
		if(g_bmp_scale_ver_fra * dst_h != (int_scaled_height<<8))
			g_bmp_scale_ver_fra++;
		if(g_bmp_scale_hor_fra * dst_w != (int_scaled_width<<8))
			g_bmp_scale_hor_fra++;		
	}
	/*init dst start position*/
	g_bmp_dst_col_pos = dst_x;
	if(g_bmp_dst_col_pos&0x01)
		g_bmp_dst_col_pos--;
	if(g_bmp_line_inverse_seq)
		g_bmp_dst_row_pos = dst_y + dst_h - blank_y;
	else
		g_bmp_dst_row_pos = dst_y;
	if(g_bmp_dst_row_pos&0x01)
		g_bmp_dst_row_pos--;
	g_bmp_scale_factor_inverse_2D = 32768/(g_bmp_scale_hor_int*g_bmp_scale_ver_int);
	/*init the ver cnt*/
	//switch hor and ver scale parameters in the rotation with 90 angle
	if(IMAGEDEC_FULL_SRN == phdl->main.setting.mode)
	{
		int tmp = 0;
		
		switch(g_imagedec_angle[phdl->id])
		{
			case ANG_90_A:
			case ANG_90_C:
				tmp = g_bmp_scale_ver_int;
				g_bmp_scale_ver_int = g_bmp_scale_hor_int;
				g_bmp_scale_hor_int = tmp;
				tmp = g_bmp_scale_ver_fra;
				g_bmp_scale_ver_fra = g_bmp_scale_hor_fra;
				g_bmp_scale_hor_fra = tmp;
				break;
			default:
				break;
		}
	}
	ver_part_sum_cnt = ver_int_cnt = 0;
	ver_fra_counter = g_bmp_scale_ver_fra;
	if(IMAGEDEC_FULL_SRN == phdl->main.setting.mode)
	{
		switch(g_imagedec_angle[phdl->id])
		{
			case ANG_90_A:
				g_bmp_dst_row_cnt = dst_y&(~1);
				g_bmp_dst_col_cnt = g_bmp_dst_col_pos;	
				break;
			case ANG_90_C:
				g_bmp_dst_row_cnt = g_bmp_dst_row_pos - 1;
				g_bmp_dst_col_cnt = g_bmp_dst_col_pos + dst_w - blank_x - 1;	
				break;
			case ANG_180:
				g_bmp_dst_row_cnt = dst_y&(~1);
				g_bmp_dst_col_cnt = g_bmp_dst_col_pos + dst_w - blank_x - 1;
				break;
			default:
				g_bmp_dst_row_cnt = g_bmp_dst_row_pos - 1;
				g_bmp_dst_col_cnt = g_bmp_dst_col_pos;				
				break;
		}
	}
	else
	{
		g_bmp_dst_row_cnt = g_bmp_dst_row_pos - 1;
		g_bmp_dst_col_cnt = g_bmp_dst_col_pos;		
	}
	jpeg_printf("scale para ver_int <%d>hor_int<%d>ver_f <%d>hor_f<%d>pos_x<%d>pos_y<%d>prec<%d>\n",g_bmp_scale_ver_int
		,g_bmp_scale_hor_int,g_bmp_scale_ver_fra,g_bmp_scale_hor_fra,g_bmp_dst_col_pos,g_bmp_dst_row_pos
		,scale_precision);
}
/*the same algorithm with the jpeg scale down*/
IMAGEDEC_STATIC void bmp_bilinear_scalling(void *para1,void *para2)
{
	pImagedec_hdl phdl = (pImagedec_hdl)para1;
	UINT32 *src = (UINT32 *)para2;
	UINT32 *y_d = (UINT32 *)phdl->imageout.frm_y_addr;
	UINT32 *c_d = (UINT32 *) phdl->imageout.frm_c_addr;
	int len = phdl->decoder.image_x_size,fraction = 0;
	int hor_part_sum_cnt = 0,hor_int_sum_cnt = 0;
	int hor_fra_counter = g_bmp_scale_hor_fra;
	UINT32 hor_part_sum = 0,hor_bilinear = 0;
	UINT32 hor_part_sum_y = 0,hor_part_sum_cb = 0,hor_part_sum_cr = 0;
	UINT32 hor_bilinear_y = 0,hor_bilinear_cb = 0,hor_bilinear_cr = 0;
	UINT32 hor_output = 0,ver_output = 0;
	int *ver_part_sum_buf_y = (int *)g_bmp_scale_partsum_buf_y;
	int *ver_bilinear_buf_y = (int *)g_bmp_scale_bilinear_buf_y;
	int *ver_part_sum_buf_cb = (int *)g_bmp_scale_partsum_buf_cb;
	int *ver_bilinear_buf_cb = (int *)g_bmp_scale_bilinear_buf_cb;
	int *ver_part_sum_buf_cr = (int *)g_bmp_scale_partsum_buf_cr;
	int *ver_bilinear_buf_cr = (int *)g_bmp_scale_bilinear_buf_cr;	
	int *pOutput_buffer = (int *)g_bmp_scale_output_buf;	
	int ver_accumulator = 0;/*the pixels of output after scaling down*/
	int i;
	int test_flag = 1;

	if(g_imagedec_osd_ins.on)
	{
		y_d = (UINT32 *)g_imagedec_osd_ins.pic.y_buf;
		c_d = (UINT32 *)g_imagedec_osd_ins.pic.c_buf;
	}
	
	/*horizontal scale down*/
	for(i=0;i<len;i++)
	{
		hor_part_sum = *src++;
		hor_part_sum_cr += hor_part_sum&0xFF;
		hor_part_sum_cb += (hor_part_sum>>8)&0xFF;
		hor_part_sum_y += (hor_part_sum>>16)&0xFF;
		hor_part_sum_cnt++;
		if(hor_part_sum_cnt >= g_bmp_scale_hor_int)
		{
			hor_int_sum_cnt++;
			if(hor_int_sum_cnt > (hor_fra_counter>>scale_precision))
			{
				fraction = hor_fra_counter&scale_bits_field;
				hor_output = hor_bilinear_cr*(scale_fra_total_value - fraction) + hor_part_sum_cr*fraction;
				*ver_part_sum_buf_cr += (int)hor_output;
				ver_part_sum_buf_cr++;			
				hor_output = hor_bilinear_cb*(scale_fra_total_value - fraction) + hor_part_sum_cb*fraction;
				*ver_part_sum_buf_cb += (int)hor_output;
				ver_part_sum_buf_cb++;
				hor_output = hor_bilinear_y*(scale_fra_total_value - fraction) + hor_part_sum_y*fraction;
				*ver_part_sum_buf_y += (int)hor_output;
				ver_part_sum_buf_y++;				
				ver_accumulator++;
				hor_fra_counter += g_bmp_scale_hor_fra;
			}
			hor_bilinear_cr = hor_part_sum_cr;	
			hor_bilinear_cb = hor_part_sum_cb;
			hor_bilinear_y = hor_part_sum_y;			
			hor_part_sum_cr = 0;
			hor_part_sum_cb = 0;
			hor_part_sum_y = 0;			
			hor_part_sum_cnt = 0;
		}
	}
	ver_part_sum_buf_y = (int *)g_bmp_scale_partsum_buf_y;
	ver_bilinear_buf_y = (int *)g_bmp_scale_bilinear_buf_y;
	ver_part_sum_buf_cb = (int *)g_bmp_scale_partsum_buf_cb;
	ver_bilinear_buf_cb = (int *)g_bmp_scale_bilinear_buf_cb;
	ver_part_sum_buf_cr = (int *)g_bmp_scale_partsum_buf_cr;
	ver_bilinear_buf_cr = (int *)g_bmp_scale_bilinear_buf_cr;
	/*vertical scale down*/
	ver_part_sum_cnt++;
	if(ver_part_sum_cnt >= g_bmp_scale_ver_int)
	{
		ver_int_cnt++;
		if(ver_int_cnt > (ver_fra_counter>>scale_precision))
		{
			fraction = ver_fra_counter&scale_bits_field;
			for(i = 0;i < ver_accumulator;i++)
			{
				ver_output = 0;
				hor_output = (*ver_bilinear_buf_cr++)*(scale_fra_total_value - fraction) + (*ver_part_sum_buf_cr++)*fraction;	
				hor_output >>= scale_precisionX2;
				hor_output *= g_bmp_scale_factor_inverse_2D;
				hor_output >>= 15;
				ver_output |= hor_output&0xFF;				
				hor_output = (*ver_bilinear_buf_cb++)*(scale_fra_total_value - fraction) + (*ver_part_sum_buf_cb++)*fraction;	
				hor_output >>= scale_precisionX2;
				hor_output *= g_bmp_scale_factor_inverse_2D;
				hor_output >>= 15;
				ver_output |= (hor_output&0xFF)<<8;
				hor_output = (*ver_bilinear_buf_y++)*(scale_fra_total_value - fraction) + (*ver_part_sum_buf_y++)*fraction;	
				hor_output >>= scale_precisionX2;
				hor_output *= g_bmp_scale_factor_inverse_2D;
				hor_output >>= 15;
				ver_output |= (hor_output&0xFF)<<16;
				*pOutput_buffer++ = (int)ver_output;
			}
			ver_fra_counter += g_bmp_scale_ver_fra;
			if(IMAGEDEC_FULL_SRN == phdl->main.setting.mode)
			{
				switch(g_imagedec_angle[phdl->id])
				{
					case ANG_90_A:
						bmp_line_2_mb_90(y_d,c_d,g_bmp_dst_stride,g_bmp_dst_row_cnt,g_bmp_dst_col_cnt,(UINT32 *)g_bmp_scale_output_buf,ver_accumulator);
						g_bmp_dst_col_cnt++;
						break;
					case ANG_90_C:
						bmp_line_2_mb_90(y_d,c_d,g_bmp_dst_stride,g_bmp_dst_row_cnt,g_bmp_dst_col_cnt,(UINT32 *)g_bmp_scale_output_buf,ver_accumulator);
						g_bmp_dst_col_cnt--;
						break;
					case ANG_180:
						bmp_line_2_mb(y_d,c_d,g_bmp_dst_stride,g_bmp_dst_row_cnt,g_bmp_dst_col_cnt,(UINT32 *)g_bmp_scale_output_buf,ver_accumulator);
						g_bmp_dst_row_cnt++;
						break;
					default:
						bmp_line_2_mb(y_d,c_d,g_bmp_dst_stride,g_bmp_dst_row_cnt,g_bmp_dst_col_cnt,(UINT32 *)g_bmp_scale_output_buf,ver_accumulator);
						g_bmp_dst_row_cnt--;
						break;
				}
			}
			else
			{
				bmp_line_2_mb(y_d,c_d,g_bmp_dst_stride,g_bmp_dst_row_cnt,g_bmp_dst_col_cnt,(UINT32 *)g_bmp_scale_output_buf,ver_accumulator);
				if(g_bmp_line_inverse_seq)
					g_bmp_dst_row_cnt--;
				else
					g_bmp_dst_row_cnt++;
			}
		}
		jh_mem_cpy((void *)g_bmp_scale_bilinear_buf_y,(void *)g_bmp_scale_partsum_buf_y,2880);
		jh_mem_set((void *)g_bmp_scale_partsum_buf_y,0,2880);
		jh_mem_cpy((void *)g_bmp_scale_bilinear_buf_cb,(void *)g_bmp_scale_partsum_buf_cb,2880);
		jh_mem_set((void *)g_bmp_scale_partsum_buf_cb,0,2880);
		jh_mem_cpy((void *)g_bmp_scale_bilinear_buf_cr,(void *)g_bmp_scale_partsum_buf_cr,2880);
		jh_mem_set((void *)g_bmp_scale_partsum_buf_cr,0,2880);
		ver_part_sum_cnt = 0;
	}
}
#define RGB_DECODE
//volatile static pImagedec_bmp_pallete g_bmp_pallete = NULL;
static volatile INT32 *g_bmp_pallete = NULL;
IMAGEDEC_INLINE void RGB_2_YUV(UINT32 *data)
{
	UINT8 *dst = (UINT8 *)data;
	int blue = (*data)&0xff;
	int green = ((*data)>>8)&0xff;
	int red = ((*data)>>16)&0xff;
	int y,cb,cr;

	y = (red*Y_COF_R + green*Y_COF_G + blue*Y_COF_B);

#if 0
	cb = ((blue<<16) - y)/C_COF_Cb;/*need to be optimized*/
	cr = ((red<<16) - y)/C_COF_Cr;
	y = (y>>16) + 16;
	cb += 128;
	cr += 128;		
#else
	cb = - red*CB_COF_R - green*CB_COF_G + blue*CB_COF_B;
	cr = red*CR_COF_R - green*CR_COF_G - blue*CR_COF_B;
	y = (y>>16) + 16;
	cb = (cb>>16) + 128;
	cr = (cr>>16) + 128;	
#endif
	if(y<16)
		y = 16;
	else if(y > 235)
		y = 235;
	if(cb<16)
		cb = 16;
	else if(cb>240)
		cb = 240;
	if(cr<16)
		cr = 16;
	else if(cr>240)
		cr = 240;
	/*store the ycbcr value*/
	*dst++ = cr;
	*dst++ = cb;
	*dst++ = y;
	*dst = 0;
}

IMAGEDEC_STATIC void skip_num_bytes(UINT32 num)
{
	while (num)
  	{
    		get_bits_1_lit_end(8);num--;
  	}
}
IMAGEDEC_STATIC void skip_padding_data(UINT32 width,int bits)
{
	UINT32 pad_cnt_bytes = ((width*bits)>>3)&0x03;	

	if((width*bits)&0x07)
		pad_cnt_bytes++;
	pad_cnt_bytes = 4 - pad_cnt_bytes;
	if((4 != pad_cnt_bytes)&&(0 != pad_cnt_bytes))
		skip_num_bytes(pad_cnt_bytes);
}
IMAGEDEC_STATIC void bmp_decode_bit1(void *para1,void *para2)
{
	pImagedec_hdl phdl = (pImagedec_hdl) para1;
	UINT32 *dst = (UINT32 *)para2;
	UINT8 index = 0,idx = 0;
	UINT32 width = phdl->decoder.image_x_size;
	UINT32 color = 0,shift_cnt = 0;
	UINT32 i;

	for(i = 0;i<width;i++)
	{
		if(IsTerminated()) return;
		if(!(i&0x07))
		{
			index = get_bits_1_lit_end(8);
			idx = (index>>7)&0x1;
			shift_cnt = 6;
		}
		else
		{
			idx = (index>>shift_cnt)&0x01;
			shift_cnt--;
		}
		color = g_bmp_pallete[idx];
		RGB_2_YUV(&color);
		*dst++ = color;
	}	
	skip_padding_data(width,1);
}
IMAGEDEC_STATIC void bmp_decode_bit4(void *para1,void *para2)
{
	pImagedec_hdl phdl = (pImagedec_hdl) para1;
	UINT32 *dst = (UINT32 *)para2;
	UINT8 index = 0,idx = 0;
	UINT32 width = phdl->decoder.image_x_size;
	UINT32 color = 0;
	UINT32 i;

	for(i = 0;i<width;i++)
	{
		if(IsTerminated()) return;
		if(!(i&0x01))
		{
			index = get_bits_1_lit_end(8);
			idx = (index>>4)&0xF;
		}
		else
		{
			idx = index&0xF;
		}
		color = g_bmp_pallete[idx];
		RGB_2_YUV(&color);
		*dst++ = color;
	}	
	skip_padding_data(width,4);
}
IMAGEDEC_STATIC void bmp_decode_bit8(void *para1,void *para2)
{
	pImagedec_hdl phdl = (pImagedec_hdl) para1;
	UINT32 *dst = (UINT32 *)para2;
	UINT8 index = 0;
	UINT32 width = phdl->decoder.image_x_size;
	UINT32 color = 0;
	UINT32 i;

	for(i = 0;i<width;i++)
	{
		if(IsTerminated()) return;
		index = get_bits_1_lit_end(8);
		color = g_bmp_pallete[index];
		RGB_2_YUV(&color);
		*dst++ = color;
	}
	skip_padding_data(width,8);
}
static int shift_r = 0,shift_g = 0,shift_b = 0;
static UINT32 r_code_16 = 0,g_code_16 = 0,b_code_16 = 0;
IMAGEDEC_STATIC void bmp_bitfields_init(UINT32 r_code,UINT32 g_code,UINT32 b_code)
{
	if((!r_code)||(!g_code)||(!b_code))
		jpeg_printf("bit fields code error\n");
	
	r_code_16 = r_code;
	g_code_16 = g_code;
	b_code_16 = b_code;
	
	shift_r = shift_g = shift_b = 0;
	while(1)
	{
		if(b_code&0x01)
			break;
		else
			shift_b++;
		b_code >>= 1;
	}	
	while(1)
	{
		if(g_code&0x01)
			break;
		else
			shift_g++;
		g_code >>= 1;
	}
	while(1)
	{
		if(r_code&0x01)
			break;
		else
			shift_r++;
		r_code >>= 1;
	}
	jpeg_printf("shift_b <%d>g<%d>r<%d>\n",shift_b,shift_g,shift_r);
}
IMAGEDEC_STATIC void bmp_decode_b16_bitfields(void *para1,void *para2)
{
	pImagedec_hdl phdl = (pImagedec_hdl) para1;
	UINT32 *dst = (UINT32 *)para2;
	UINT32 width = phdl->decoder.image_x_size;
	UINT32 color = 0;
	UINT32 r,g,b;
	UINT32 i;

	for(i=0;i<width;i++)
	{
		if(IsTerminated()) return;
		color = get_bits_1_lit_end(16);
		b = color&b_code_16;
		g = color&g_code_16;
		r = color&r_code_16;
		if(shift_b) b >>= shift_b;
		if(shift_g) g >>= shift_g;
		if(shift_r)  r >>= shift_r;
		color = (r<<16)|(g<<8)|b;
		RGB_2_YUV(&color);
		*dst++ = color;
	}
	skip_padding_data(width,16);	
}
IMAGEDEC_STATIC void bmp_decode_bit16(void *para1,void *para2)
{
	pImagedec_hdl phdl = (pImagedec_hdl) para1;
	UINT32 *dst = (UINT32 *)para2;
	UINT32 width = phdl->decoder.image_x_size;
	UINT32 color = 0;
	UINT32 r,g,b;
	UINT32 i;

	for(i=0;i<width;i++)
	{
		if(IsTerminated()) return;
		color = get_bits_1_lit_end(16);
		b = color&0x1F;
		b = b<<3 | b>>2;
		g = (color>>5)&0x1F;
		g = g<<3 | g>>2;
		r = (color>>10)&0x1F;
		r = r<<3 | r>>2;
		color = (r<<16)|(g<<8)|b;
		RGB_2_YUV(&color);
		*dst++ = color;
	}
	skip_padding_data(width,16);
}
IMAGEDEC_STATIC void bmp_decode_bit24(void *para1,void *para2)
{
	pImagedec_hdl phdl = (pImagedec_hdl) para1;
	UINT32 *dst = (UINT32 *)para2;
	UINT32 width = phdl->decoder.image_x_size;
	UINT32 color = 0;
	UINT32 i;

	for(i = 0;i<width;i++)
	{
		if(IsTerminated()) return;
		color = get_bits_1_lit_end(8);
		color |= get_bits_1_lit_end(8)<<8;
		color |= get_bits_1_lit_end(8)<<16;
		RGB_2_YUV(&color);
		*dst++ = color;
	}
	skip_padding_data(width,24);
}
IMAGEDEC_STATIC void bmp_decode_bit32(void *para1,void *para2)
{
	pImagedec_hdl phdl = (pImagedec_hdl) para1;
	UINT32 *dst = (UINT32 *)para2;
	UINT32 width = phdl->decoder.image_x_size;
	UINT32 color = 0;
	UINT32 i;

	for(i = 0;i<width;i++)
	{
		if(IsTerminated()) return;
		color = get_bits_1_lit_end(8);
		color |= get_bits_1_lit_end(8)<<8;
		color |= get_bits_1_lit_end(8)<<16;
		get_bits_1_lit_end(8);
		RGB_2_YUV(&color);
		*dst++ = color;
	}
	skip_padding_data(width,32);
}
#define RLE_PARSING
/*RLE global variables*/
static volatile UINT32 lastest_pixel = 0x108080;
static volatile int delta_hor_cnt = 0;
static volatile int delta_ver_cnt = 0;
IMAGEDEC_INLINE RLE_MODE bmp_rle_parsing(int m1,int m2)
{
	RLE_MODE mode = ENC_NOR;
	
	if(0 == m1)
	{
		if(0 == m2)
			mode = ENC_1ST;
		else if(1 == m2)
			mode = ENC_2ND;
		else if(2 == m2)
			mode = ENC_3RD;
		else if(m2 <= 0xFF)
			mode = ABS_MODE;
	}
	return mode;	
}
IMAGEDEC_STATIC void bmp_decode_bit4_rle4(void *para1,void *para2)
{
	pImagedec_hdl phdl = (pImagedec_hdl) para1;
	UINT32 *dst = (UINT32 *)para2;
	int offset = 0,marker1,marker2;
	UINT32 pixel1 = 0,pixel2 = 0,pixel_switch = 0;
	int width = phdl->decoder.image_x_size;
	int index = 0, pixel_cnt = 0;
	int end_flag = 0,word_agn_flag = 0;
	RLE_MODE mode = ENC_NOR;
	int i;
	UINT32 screen = 0;
	
	/*3rd rule of encoded mode*/
	if(delta_ver_cnt)
	{
		if((1 == delta_ver_cnt) && (!delta_hor_cnt))
		{
			for(i=0;i<delta_hor_cnt;i++,offset++)
			{
				*dst++ = lastest_pixel;	
			}
		}
		else
		{
			for(i=0;i<width;i++)
			{
				*dst++ =  lastest_pixel;	
			}
			end_flag = 1;
		}
		delta_ver_cnt--;
	}
	while(!end_flag)
	{
		if(IsTerminated()) return;
		marker1 = get_bits_1_lit_end(8);
		marker2 = get_bits_1_lit_end(8);
		mode = bmp_rle_parsing(marker1,marker2);
		switch(mode)
		{
			case ENC_NOR:
				pixel_switch = 0;
				pixel_cnt = marker1;
				pixel1 = g_bmp_pallete[(marker2>>4)&0xF];
				pixel2 = g_bmp_pallete[marker2&0xF];
				RGB_2_YUV(&pixel1);RGB_2_YUV(&pixel2);
				while(pixel_cnt--)
				{
					if(pixel_switch)
					{
						pixel_switch = 0;*dst++ = pixel2;
					}
					else
					{
						pixel_switch = 1;*dst++ = pixel1;						
					}
					offset++;
				}
				break;
			case ENC_2ND:	/*end of the image*/
				jpeg_printf("end bmp rle image\n");
				g_bmp_src_row_cnt = 1;
			case ENC_1ST:
				for(i=offset;i<width;i++)
				{
					*dst++ = lastest_pixel;
				}
				end_flag = 1;
				break;
			case ENC_3RD:
				delta_hor_cnt = get_bits_1_lit_end(8);
				delta_ver_cnt = get_bits_1_lit_end(8);				
				if(delta_ver_cnt)
				{
					for(i=offset;i<width;i++)
					{
						*dst++ = lastest_pixel;
					}
					end_flag = 1;
				}
				else
				{
					for(i=0;i<delta_hor_cnt;i++)
					{
						*dst++ = lastest_pixel;
						offset++;
					}					
				}
				break;
			case ABS_MODE:
				if(marker2&0x01)
					word_agn_flag = 1;
				marker2 >>= 1;
				pixel_cnt = marker2;
				while(pixel_cnt--)
				{
					index = get_bits_1_lit_end(8);
					pixel1 = g_bmp_pallete[(index>>4)&0xF];
					pixel2 = g_bmp_pallete[index&0xF];
					RGB_2_YUV(&pixel1);RGB_2_YUV(&pixel2);
					*dst++ = pixel1;offset++;
					*dst++ = pixel2;offset++;
				}
				if(word_agn_flag)
				{
					index = get_bits_1_lit_end(8);
					pixel1 = g_bmp_pallete[(index>>4)&0xF];
					RGB_2_YUV(&pixel1);
					*dst++ = pixel1;offset++;
					word_agn_flag = 0;
					if(!(marker2&0x01))
					{
						if(get_bits_1_lit_end(8))
							{jpeg_printf("ABS padding error\n");}
					}
				}
				else
				{
					if(marker2&0x01)
					{
						if(get_bits_1_lit_end(8))
							{jpeg_printf("ABS padding error\n");}
					}
				}
				break;
			default:
		{JPEG_ASSERT(0);}
		}
	}	
}

IMAGEDEC_STATIC void bmp_decode_bit8_rle8(void *para1,void *para2)
{
	pImagedec_hdl phdl = (pImagedec_hdl) para1;
	UINT32 *dst = (UINT32 *)para2;
	int offset = 0,marker1,marker2;
	int width = phdl->decoder.image_x_size;
	int index = 0, pixel_cnt = 0;
	int end_flag = 0,word_agn_flag = 0;
	UINT32 pixel = 0;
	RLE_MODE mode = ENC_NOR;
	int i;
	
	/*3rd rule of encoded mode*/
	if(delta_ver_cnt)
	{
		if((1 == delta_ver_cnt) && (!delta_hor_cnt))
		{
			for(i=0;i<delta_hor_cnt;i++,offset++)
			{
				*dst++ = lastest_pixel;	
			}
		}
		else
		{
			for(i=0;i<width;i++)
			{
				*dst++ = lastest_pixel;	
			}
			end_flag = 1;
		}
		delta_ver_cnt--;
	}
	while(!end_flag)
	{
		if(IsTerminated()) return;
		marker1 = get_bits_1_lit_end(8);
		marker2 = get_bits_1_lit_end(8);
		mode = bmp_rle_parsing(marker1,marker2);
		switch(mode)
		{
			case ENC_NOR:
				pixel_cnt = marker1;
				index = marker2;
				pixel = g_bmp_pallete[index];
				RGB_2_YUV((UINT32 *)&pixel);
				while(pixel_cnt--)
				{*dst++ = pixel;offset++;}
				break;
			case ENC_2ND:	/*end of the image*/
				jpeg_printf("end bmp rle image\n");
				g_bmp_src_row_cnt = 1;
			case ENC_1ST:
				for(i=offset;i<width;i++)
				{*dst++ = lastest_pixel;}
				end_flag = 1;
				break;
			case ENC_3RD:
				delta_hor_cnt = get_bits_1_lit_end(8);
				delta_ver_cnt = get_bits_1_lit_end(8);				
				if(delta_ver_cnt)
				{
					for(i=offset;i<width;i++)
					{
						*dst++ = lastest_pixel;
					}
					end_flag = 1;
				}
				else
				{
					for(i=0;i<delta_hor_cnt;i++)
					{
						*dst++ = lastest_pixel;offset++;
					}					
				}
				break;
			case ABS_MODE:
				pixel_cnt = marker2;
				if(pixel_cnt&0x01)
					word_agn_flag = 1;
				while(pixel_cnt--)
				{
					index = get_bits_1_lit_end(8);
					pixel = g_bmp_pallete[index];
					RGB_2_YUV((UINT32 *)&pixel);
					*dst++ =pixel;offset++;
				}
				if(word_agn_flag)
				{
					if(get_bits_1_lit_end(8))
						{jpeg_printf("abs mode padding error\n");}
					word_agn_flag = 0;
				}
				break;
			default:
		{JPEG_ASSERT(0);}
		}
	}
}

IMAGEDEC_STATIC void bmp_gen_color_pallete(pImagedec_hdl phdl)
{
	UINT32 *pallete = (UINT32 *)(phdl->decoder.bmp_pallete);
	UINT32 tmp = 0;
	int i = 0;

	while(phdl->decoder.file_offset_cnt < phdl->decoder.file_header.bfOffBits)
	{
		tmp = get_bits_1_lit_end(16);
		tmp |= get_bits_1_lit_end(16)<<16;
		*pallete++ = tmp;
		phdl->decoder.file_offset_cnt += 4;
		i++;
		if(i > 256)
		{
			jpeg_printf("pallete over flow <%d>\n",i);
			return;
		}
	}
}
#define BMP_HEADER
IMAGEDEC_STATIC bool bmp_file_header(pImagedec_hdl phdl)
{
	phdl->decoder.file_header.bfType = get_bits_1_lit_end(16);
	phdl->decoder.file_header.bfSize = get_bits_1_lit_end(16);
	phdl->decoder.file_header.bfSize |= get_bits_1_lit_end(16)<<16;
	phdl->decoder.file_header.bfReserved1 = get_bits_1_lit_end(16);
	phdl->decoder.file_header.bfReserved2 = get_bits_1_lit_end(16);	
	phdl->decoder.file_header.bfOffBits = get_bits_1_lit_end(16);
	phdl->decoder.file_header.bfOffBits |= get_bits_1_lit_end(16)<<16;	
	/*shift the file offset count*/
	phdl->decoder.file_offset_cnt += 14;
}

IMAGEDEC_STATIC bool bmp_info_header(pImagedec_hdl phdl)
{
	PBITMAPINFOHEADER pinfo = &(phdl->decoder.info_header);
	UINT32 count = 0;
	
	pinfo->biSize = get_bits_1_lit_end(16);
	pinfo->biSize |= get_bits_1_lit_end(16)<<16;
	pinfo->biWidth = get_bits_1_lit_end(16);
	pinfo->biWidth |= get_bits_1_lit_end(16)<<16;
	pinfo->biHeight = get_bits_1_lit_end(16);
	pinfo->biHeight |= get_bits_1_lit_end(16);
	pinfo->biPlanes = get_bits_1_lit_end(16);
	pinfo->biBitCount = get_bits_1_lit_end(16);
	pinfo->biCompression = get_bits_1_lit_end(16);
	pinfo->biCompression |= get_bits_1_lit_end(16)<<16;
	pinfo->biSizeImage = get_bits_1_lit_end(16);
	pinfo->biSizeImage |= get_bits_1_lit_end(16)<<16;
	pinfo->biXPelsPerMeter = get_bits_1_lit_end(16);
	pinfo->biXPelsPerMeter |= get_bits_1_lit_end(16)<<16;
	pinfo->biYPelsPerMeter = get_bits_1_lit_end(16);
	pinfo->biYPelsPerMeter |= get_bits_1_lit_end(16)<<16;
	pinfo->biClrUsed = get_bits_1_lit_end(16);
	pinfo->biClrUsed |= get_bits_1_lit_end(16)<<16;
	pinfo->biClrImportant = get_bits_1_lit_end(16);
	pinfo->biClrImportant |= get_bits_1_lit_end(16)<<16;
	if(0 != pinfo->biClrUsed)
		jpeg_printf("clr used <%d>\n",pinfo->biClrUsed);
	count = 40;
	phdl->decoder.file_offset_cnt += count;
	if(pinfo->biHeight < 0)
	{
		jpeg_printf("line not inverse\n");
		g_bmp_line_inverse_seq = 0;
	}
	else
		g_bmp_line_inverse_seq = 1;
}
#define BMP_API
IMAGEDEC_STATIC void bmp_put_row2frame(pImagedec_hdl phdl,UINT8 *src)
{
	if(NULL != phdl->decoder.bmp_transfer)
		phdl->decoder.bmp_transfer((void*)phdl,(void*)src);
}

void imagedec_bmp_init(pImagedec_hdl phdl)
{
	/*init decoder and blocks*/
	error_code = 0;
	image_data_end = 0;
#ifndef IMAGEDEC_SINGLE_INPUT_BUF	
	phdl->in_stream.buf_idx_using = 0;
	phdl->in_stream.buf_item[0].busy = 0;
	phdl->in_stream.buf_item[1].busy = 0;
	phdl->in_stream.buf_item[0].end = 0;
	phdl->in_stream.buf_item[1].end = 0;	
	phdl->in_stream.buf_item[0].error = 0;
	phdl->in_stream.buf_item[1].error = 0;	
	imagedec_fill_in_buffer(phdl,1 - phdl->in_stream.buf_idx_using);
#endif
	prep_in_buffer();
	MEMSET(&phdl->decoder,0,sizeof(phdl->decoder));
	MEMSET(&phdl->mem.blocks,0,sizeof(phdl->mem.blocks));
	phdl->file_endian = LITTLE_ENDIAN;
	phdl->mem.mem_pos = phdl->mem.mem_start;
	phdl->in_stream.Pin_buf_ofs = phdl->in_stream.in_buf;
	phdl->decoder.bits_left = 16;
	get_bits_1_lit_end(16);
	get_bits_1_lit_end(16);
	phdl->decoder.file_offset_cnt = 0;
	/*parsing bmp header(file header and bitmap info header)*/
	if(!bmp_file_header(phdl))
		{JPEG_ASSERT(0);}
	if(!bmp_info_header(phdl))
		{JPEG_ASSERT(0);}
	if(IsTerminated())
	{
		jpeg_printf("init bmp exit fail <%d>\n",error_code);
		return;
	}
	phdl->decoder.image_x_size = phdl->decoder.info_header.biWidth;
	phdl->decoder.image_y_size = phdl->decoder.info_header.biHeight;
	jpeg_printf("bmp w <%d>h <%d>\n",phdl->decoder.image_x_size,phdl->decoder.image_y_size);
	g_bmp_pallete = (UINT32 *)phdl->decoder.bmp_pallete;
	bmp_gen_color_pallete(phdl);	
	g_bmp_src_row_cnt = phdl->decoder.image_y_size;
	phdl->info.precision = phdl->decoder.info_header.biBitCount;
	phdl->info.width = phdl->decoder.image_x_size;
	phdl->info.height = phdl->decoder.image_y_size;
	if(g_imagedec_get_info[phdl->id])
	{
		g_imagedec_get_info[phdl->id] = FALSE;
		jpeg_printf("bmp get info\n");
		return;
	}
	phdl->decoder.bmp_dec_row_buf_len = (phdl->decoder.info_header.biWidth<<2) + 1;
	phdl->decoder.bmp_dec_row_buf = (UINT8 *)imagedec_malloc(phdl->decoder.bmp_dec_row_buf_len);
	switch(phdl->decoder.info_header.biBitCount)
	{
		case 1:
			phdl->decoder.bmp_decoder = bmp_decode_bit1;
			break;
		case 4:
			if(BI_RGB == phdl->decoder.info_header.biCompression)
				phdl->decoder.bmp_decoder = bmp_decode_bit4;
			else if(BI_RLE4 == phdl->decoder.info_header.biCompression)
			{
				lastest_pixel = g_bmp_pallete[0];
				RGB_2_YUV((UINT32 *)&lastest_pixel);				
				delta_hor_cnt = 0;
				delta_ver_cnt = 0;
				phdl->decoder.bmp_decoder = bmp_decode_bit4_rle4;
			}
			else
				{JPEG_ASSERT(0);}
			break;
		case 8:
			if(BI_RGB == phdl->decoder.info_header.biCompression)
				phdl->decoder.bmp_decoder = bmp_decode_bit8;
			else if(BI_RLE8 == phdl->decoder.info_header.biCompression)
			{
				lastest_pixel = g_bmp_pallete[0];
				RGB_2_YUV((UINT32 *)&lastest_pixel);
				delta_hor_cnt = 0;
				delta_ver_cnt = 0;
				phdl->decoder.bmp_decoder = bmp_decode_bit8_rle8;
			}
			else
				{JPEG_ASSERT(0);}
			break;
		case 16:
			if(BI_BITFIELDS == phdl->decoder.info_header.biCompression)
			{
				bmp_bitfields_init(g_bmp_pallete[2],g_bmp_pallete[1],g_bmp_pallete[0]);
				phdl->decoder.bmp_decoder = bmp_decode_b16_bitfields;
			}
			else		
				phdl->decoder.bmp_decoder = bmp_decode_bit16;
			break;	
		case 24:
			phdl->decoder.bmp_decoder = bmp_decode_bit24;
			break;			
		case 32:
			phdl->decoder.bmp_decoder = bmp_decode_bit32;
			break;		
		default:
			JPEG_ASSERT(0);
	}
	/*init the tranfer function*/
	switch(phdl->main.setting.mode)
	{
		case IMAGEDEC_REAL_SIZE:
			bmp_init_dup_para(phdl);
			phdl->decoder.bmp_transfer = bmp_direct_dup;
			break;
		default:
			if(g_imagedec_osd_ins.on)
			{
				pimagedec_osd_raw_pic pic = &g_imagedec_osd_ins.pic;

				/*allocate the raw pic buff built by Y and C buffer*/
				pic->width = phdl->decoder.image_x_size;
				pic->height = phdl->decoder.image_y_size;
				if((pic->width > MAX_WIDTH_BY_OSD) || (pic->height > MAX_HEIGHT_BY_OSD))
				{
					terminate(JPGD_OSD_UNSUPPORTED_PIC_SIZE);
				}
				else
				{
					pic->stride = (pic->width + 15) & 0xFFF0;
					pic->y_size = pic->stride * ((pic->height + 31) & 0xFFE0);
					pic->c_size = pic->y_size>>1;
					pic->y_buf = (UINT8 *)imagedec_malloc(pic->y_size + 256);
					pic->c_buf = (UINT8 *)imagedec_malloc(pic->c_size + 256);
					pic->y_buf = (UINT8 *)((UINT32)(pic->y_buf + 255) & 0xFFFFFF00);
					pic->c_buf = (UINT8 *)((UINT32)(pic->c_buf + 255) & 0xFFFFFF00);
					jpeg_printf("osd pic y %x c %x\n",pic->y_buf,pic->c_buf);
				}
				bmp_init_dup_para(phdl);
				phdl->decoder.bmp_transfer = bmp_direct_dup;				
			}
			else
			{
				bmp_init_scale_para(phdl);
				phdl->decoder.bmp_transfer = bmp_bilinear_scalling;
			}
			break;
	}	
}

/*
*	Return Value:
*					-1 	decode fail
*					0	decode a row done
*					1	decode all the row
*
*/
int imagedec_bmp_decode_row(pImagedec_hdl phdl)
{
	UINT8 *dst = phdl->decoder.bmp_dec_row_buf;

	phdl->info.coding_progress = ((phdl->decoder.image_x_size - g_bmp_src_row_cnt)<<16)/phdl->decoder.image_x_size;
	g_imagedec_info_valid[phdl->id] = 1;
	if((g_bmp_src_row_cnt <= 0)||(phdl->decoder.bmp_real_size_done))
		return BMP_DONE;
	/*dec the input file and store the row to the dst buffer*/
	phdl->decoder.bmp_decoder((void *)phdl,(void *)dst);
	if (IsTerminated()) return BMP_FAILED;
	/*put the data to display frame buffer with SW scale operation*/
	bmp_put_row2frame(phdl,dst);	
	phdl->info.width = phdl->decoder.image_x_size;
	phdl->info.height = phdl->decoder.image_y_size;
	if(phdl->info.coding_progress - phdl->info.last_called_prog >= 3000)
  	{
		phdl->info.last_called_prog = phdl->info.coding_progress;
		if(NULL != phdl->main.status)
			phdl->main.status((void *)&phdl->info.last_called_prog);
  	}
	g_bmp_src_row_cnt--;
	return BMP_ROW_OK;
}

