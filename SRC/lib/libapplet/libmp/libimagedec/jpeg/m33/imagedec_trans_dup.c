#include <sys_config.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <os/tds2/itron.h>
#include <api/libc/printf.h>
#include <osal/osal_timer.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <hld/dis/vpo.h>
#include <hld/decv/vdec_driver.h>
#include <hld/hld_dev.h>
#include <api/libimagedec/imagedec.h>
#include "imagedec_main.h"

extern int *temp_IDCT_buf; //IDCT input, 
extern int *temp_IDCT_buf2; //IDCT output, for 4:1:1 we may have to do horizontal copying

/*global variables for frame duplication*/
static UINT32 p_x,p_y,c_x,c_y,e_x,e_y,stride_x,inc_x,inc_y,d_blk_cnt_y,d_blk_cnt_c,d_rows,d_cols;
static UINT8 n_row_flag;
static UINT32 y_dst = 0,cb_dst = 0,cr_dst = 0,y_dst_row = 0,c_dst_row = 0;
static UINT32 hor_inc_cnt = 0;

#define CLAMP_DUP(i) 	\
	i = (i&0xFF00)?((~i)>>15):i

/*
*	Parameters:
*				src	     :        block data(8X8). unit is 4bytes
*				dst	     :		frame buffer. uint is 1byte
*				scale_v :		0	--- without scale
*							1	---	scale down 2
*							2	---	scale up 2
*				scale_h : it is nearly the same as scale_v, but in hor direction
*/
static int dup_tmp_c[8][16];/*used when hor and ver scale down 2 together*/
static void block_c_dir_duplication(int *src,uchar *dst)
{
	int b_x = 8;
	int b_y = 8;
	uchar *dst_i = dst;
	int *src_i = src;
	int src_p = 0;
	int i,j;
	
	for(i=0;i<b_y;i++)
	{
		for(j=0;j<b_x;j++)
		{
			src_p = *src_i++;
			CLAMP_DUP(src_p);
			*dst_i = (uchar)src_p;
			dst_i += 2;
		}
	}
}
static void block_c_hor_scale_down2(int *src,uchar *dst)
{
	int b_x = 8;
	int b_y = 8;
	uchar *dst_i = dst;
	int *src_i = src;
	int i,j;
	int tmp1,tmp2,tmp3;

	tmp1 = 0;tmp2 = 0;tmp3 = *src_i;
	for(i=0;i<b_y;i++)
	{
		for(j=0;j<b_x;j += 2)
		{
			tmp1 = tmp3;
			tmp2 = *src_i++;
			tmp3 = *src_i++;
			tmp2 = ((tmp2<<1) + tmp1 + tmp3)>>2;
			CLAMP_DUP(tmp2);
			*dst_i = (uchar)tmp2;
			dst_i += 2;
		}
		dst_i += 8;
	}
}
static void block_c_ver_scale_down2(int *src,uchar *dst)
{	
	int b_x = 8;
	int b_y = 8;
	uchar *dst_i = dst;
	int *src_i = src;
	int i,j;
	int tmp1,tmp2,tmp3;

	tmp1 = 0;tmp2 = 0;tmp3 = *src_i;
	for(i=0;i<b_x;i++)
	{
		src_i = src + i;
		dst_i = dst + (i<<1);
		for(j=0;j<b_y;j += 2)
		{
			tmp1 = tmp3;
			tmp2 = *src_i;src_i += 8;
			tmp3 = *src_i;src_i += 8;
			tmp2 = ((tmp2<<1) + tmp1 + tmp3)>>2;
			CLAMP_DUP(tmp2);
			*dst_i = (uchar)tmp2;dst_i += 16;
		}
	}
}
static void block_c_hor_ver_scale_down2(int *src,uchar *dst)
{
	int b_x = 8;
	int b_y = 8;
	uchar *dst_i = dst;
	int *src_i = src;
	int i,j;
	int tmp1,tmp2,tmp3;

	tmp1 = 0;tmp2 = 0;tmp3 = *src_i;
	/*hor scale down to dup_tmp_c buffer*/
	for(i=0;i<b_y;i++)
	{
		for(j=0;j<b_x;j += 2)
		{
			tmp1 = tmp3;
			tmp2 = *src_i++;
			tmp3 = *src_i++;
			tmp2 = ((tmp2<<1) + tmp1 + tmp3)>>2;
			//CLAMP_DUP(tmp2);
			dup_tmp_c[i][j] = tmp2;
		}
	}
	tmp1 = 0;tmp2 = 0;tmp3 = dup_tmp_c[0][0];
	/*ver scale down to dst*/
	for(i=0;i<b_x;i += 2)
	{
		dst_i = dst + i;
		for(j=0;j<b_y;j += 2)
		{
			tmp1 = tmp3;
			tmp2 = dup_tmp_c[j][i];
			tmp3 = dup_tmp_c[j+1][i];
			tmp2 = ((tmp2<<1) + tmp1 + tmp3)>>2;
			CLAMP_DUP(tmp2);
			*dst_i = (uchar)tmp2;dst_i += 16;
		}
	}
}
static void block_c_hor_scale_up2_v_d2(int *src,uchar *dst)
{
	int b_x = 8;
	int b_y = 8;
	uchar *dst_i = dst;
	int *src_i = src;
	int i,j;
	int tmp1,tmp2,tmp3;

	for(i = 0;i < b_x;i++)
	{
		src_i = src + i;
		dst_i = dst + ((i&3)<<2);
		if(i&4)
			dst_i += 256;
		tmp1 = tmp2 = 0;tmp3 = *src_i;
		for(j = 0;j < b_y;j += 2)
		{
			tmp1 = tmp3;
			tmp2 = *src_i;
			src_i += 8;
			tmp3 = *src_i;
			src_i += 8;
			tmp2 = ((tmp2<<1) + tmp1 + tmp3)>>2;
			CLAMP_DUP(tmp2);
			*dst_i = tmp2;
			*(dst_i + 2) = tmp2;
			dst_i += 16;
		}
	}
}
static void block_y_duplication(int *src,uchar *dst)
{
	int b_x = 8;
	int b_y = 8;
	uchar *dst_i = dst;
	int *src_i = src;
	int src_p = 0;
	int i,j;

	for(i=0;i<b_y;i++)
	{
		for(j=0;j<b_x;j++)
		{
			src_p = *src_i++;
			CLAMP_DUP(src_p);
			*dst_i++ = (uchar)src_p;
		}
		dst_i += 8;
	}
}
IMAGEDEC_INLINE bool y_hor_block_in_mcu(int scan_type,int block_in_mcu)
{
	bool ret = FALSE;
	switch(scan_type)
	{
		case JPGD_YH4V1:
			if(block_in_mcu<=3)
				ret = TRUE;
			break;
		case JPGD_YH2V2:	
			if((0 == block_in_mcu) || (1 == block_in_mcu))
				ret = TRUE;
			break;
		case JPGD_YH2V1:
			if(block_in_mcu<=1)
				ret = TRUE;
			break;
		case JPGD_YH1V2:
			if(0 == block_in_mcu)
				ret = TRUE;
			break;
		case JPGD_GRAYSCALE:	
		case JPGD_YH1V1:
			if(block_in_mcu<=0)
				ret = TRUE;
			break;
		default:
			{JPEG_ASSERT(0);}
	}
	return ret;
}
static UINT32 valid_blk = 1;
void imagedec_idct_dup_init(pImagedec_hdl phdl)
{
	JPEG_ENTRY;
	p_x = phdl->main.setting.src.uStartX;
	p_y = phdl->main.setting.src.uStartY;
	e_x = phdl->main.setting.src.uWidth + p_x;
	e_y = phdl->main.setting.src.uHeight + p_y;
	
	jpeg_printf("p_x <%d>p_y <%d>e_x<%d>e_y <%d> type <%d>\n",p_x,p_y
		,e_x,e_y,phdl->decoder.scan_type);
	
	stride_x = 720;
	/*mcu shift count*/
	switch (phdl->decoder.scan_type)
	{
		case JPGD_YH2V2:
			inc_x = 8;
			inc_y = 16;
			phdl->imageout.y_dup = block_y_duplication;
			phdl->imageout.c_dup = block_c_dir_duplication;	
			break;
		case JPGD_YH1V2:
			inc_x = 8;
			inc_y = 16;
			phdl->imageout.y_dup = block_y_duplication;
			phdl->imageout.c_dup = block_c_hor_scale_down2;
			break;
		case JPGD_YH2V1:
			inc_x = 8;
			inc_y = 8;	
			phdl->imageout.y_dup = block_y_duplication;
			phdl->imageout.c_dup = block_c_ver_scale_down2;			
			break;			
		case JPGD_YH1V1:
			inc_x = 8;
			inc_y = 8;	
			phdl->imageout.y_dup = block_y_duplication;
			phdl->imageout.c_dup = block_c_hor_ver_scale_down2;			
			break;			
		case JPGD_YH4V1:
			inc_x = 8;
			inc_y = 8;	
			phdl->imageout.y_dup = block_y_duplication;
			phdl->imageout.c_dup = block_c_hor_scale_up2_v_d2;			
			break;			
		case JPGD_GRAYSCALE:
			inc_x = 8;
			inc_y = 8;	
			phdl->imageout.y_dup = block_y_duplication;
			phdl->imageout.c_dup = block_c_dir_duplication;			
			break;
		default:
			{JPEG_ASSERT(0);}
	}
	c_x = c_y = 0;
	d_blk_cnt_y = 0;
	d_blk_cnt_c = 0;
	d_rows = 0;
	d_cols = 0;
	n_row_flag = 1;
	hor_inc_cnt = 0;
	
	/*operation when size of src is smaller than 720X576*/
	if(phdl->decoder.image_x_size < TV_HOR_COL_NUM)
	{
		d_cols = (TV_HOR_COL_NUM - phdl->decoder.image_x_size)>>1;
		d_cols &= 0xFFF0;
		e_x = phdl->decoder.image_x_size&0xFFF8;
	}
	if(phdl->decoder.image_y_size < TV_VER_LINE_NUM)
	{
		d_rows = (TV_VER_LINE_NUM - phdl->decoder.image_y_size)>>1;
		d_rows &= 0xFFF0;
		e_y = phdl->decoder.image_y_size&0xFFF8;
	}
	/*init frame buffer address*/
	y_dst = y_dst_row = (UINT32)phdl->imageout.frm_y_addr;
	cb_dst = c_dst_row = (UINT32)phdl->imageout.frm_c_addr;
	y_dst += (d_rows&0xFFF0)*720 + ((d_rows&0xF)<<4) + ((d_cols&0xFFF0)<<4) + (d_cols&0xF);
	cb_dst += ((d_rows>>5)<<4)*720 + (((d_rows>>4)&0x1)<<7) + (((d_rows>>1)&0x7)<<4) + ((d_cols&0xFFF0)<<4) + (d_cols&0xE);
	cr_dst = cb_dst + 1;
	valid_blk = 1;
	JPEG_EXIT;
}
void imagedec_idct_dup_next_mcu(pImagedec_hdl phdl)
{
	if(hor_inc_cnt)
	{
		c_x += hor_inc_cnt;
		hor_inc_cnt = 0;
	}
	valid_blk = 1;
}
void imagedec_idct_dup_next_row(pImagedec_hdl phdl)
{
	JPEG_ENTRY;
	c_y += inc_y;
	c_x  = 0;
	d_blk_cnt_y = 0;
	d_blk_cnt_c = 0;
	n_row_flag = 1;
	valid_blk = 1;
	/*update dst row start addr*/
	y_dst = (UINT32)phdl->imageout.frm_y_addr;
	cb_dst = (UINT32)phdl->imageout.frm_c_addr;
	y_dst += (d_rows&0xFFF0)*720 + ((d_rows&0xF)<<4) + ((d_cols&0xFFF0)<<4) + (d_cols&0xF);
	cb_dst += ((d_rows>>5)<<4)*720 + (((d_rows>>4)&0x1)<<7) + (((d_rows>>1)&0x7)<<4) + ((d_cols&0xFFF0)<<4) + (d_cols&0xE);
	cr_dst = cb_dst + 1;
	//soc_printf("dst row <%d> y_addr <%x>c_b <%x>c_r <%x>\n",d_rows,y_dst,cb_dst,cr_dst);
	JPEG_EXIT;
}
/*copy the IDCT data to frame buffer for the JPEG lib real size mode*/
void IDCT_Duplication(pImagedec_hdl phdl,int mcu_in_row,int block_in_mcu,int ref_com)
{
	if(c_y>=e_y)
	{
		phdl->decoder.real_size_done = TRUE;
		return;
	}
	if((c_y<p_y) || (c_x<p_x) || (c_x>=e_x))
	{	
		if(((0 == ref_com)&&y_hor_block_in_mcu(phdl->decoder.scan_type,block_in_mcu)) || (1 == ref_com)|| (2 == ref_com))
		{
			if((1 == ref_com) || (2 == ref_com))
				hor_inc_cnt += inc_x;
			goto DUP_UP_SRC_CNT;
		}
		return;
	}
	if(n_row_flag)
	{
		n_row_flag = 0;
		d_rows += inc_y;
	}
	//soc_printf("src clo cnt <%d> blcok <%d>\n",c_x,block_in_mcu);
	idct(temp_IDCT_buf, temp_IDCT_buf2);
	/*saving data into frame buffer only with c simple scale down 2*/
	switch (phdl->decoder.scan_type)
	{
		case JPGD_YH2V2:
			switch(block_in_mcu)
			{	
				case 5:
					if(valid_blk)
						phdl->imageout.c_dup(temp_IDCT_buf2,(uchar *)cr_dst);
					cr_dst += 256;
					if(2 == ref_com)
					{
						hor_inc_cnt += inc_x;
						goto DUP_UP_SRC_CNT;
					}
					return;
				case 4:
					if(valid_blk)
						phdl->imageout.c_dup(temp_IDCT_buf2,(uchar *)cb_dst);
					cb_dst += 256;
					if(1 == ref_com)
					{
						hor_inc_cnt += inc_x;
						goto DUP_UP_SRC_CNT;
					}
					return;
				default://0~3
					if(!((0 == ref_com) && (((c_x + hor_inc_cnt >= e_x)&&(1 == block_in_mcu)) || ((c_x + hor_inc_cnt > e_x)&&(3 == block_in_mcu)))))
					{
 						if(! ((0 == ref_com) && (block_in_mcu > 1) && (c_y + 8 >=  e_y)))
 						{
							phdl->imageout.y_dup(temp_IDCT_buf2,(uchar *)y_dst);
							valid_blk = 1;
 						}
						else
							valid_blk = 0;
					}
					if(!d_blk_cnt_y)
						y_dst += 8;
					else
						y_dst += 120;
					d_blk_cnt_y = 1 - d_blk_cnt_y;
					if(block_in_mcu > 1)
						return;
			}					
			break;
		case JPGD_YH1V2:
			switch(block_in_mcu)
			{
				case 3:
					if(valid_blk)
						phdl->imageout.c_dup(temp_IDCT_buf2,(uchar *)cr_dst);
					if(0 == d_blk_cnt_c)
					{
						cr_dst += 8;
					}
					else if(1 == d_blk_cnt_c)
					{
						cr_dst += 248;
					}
					d_blk_cnt_c = 1 - d_blk_cnt_c;
					if(2 == ref_com)
						goto DUP_UP_SRC_CNT;
					return;
				case 2:
					if(valid_blk)
						phdl->imageout.c_dup(temp_IDCT_buf2,(uchar *)cb_dst);		
					if(0 == d_blk_cnt_c)
					{
						cb_dst += 8;
					}
					else if(1 == d_blk_cnt_c)
					{
						cb_dst += 248;
					}
					if(1 == ref_com)
					{
						d_blk_cnt_c = 1 - d_blk_cnt_c;
						goto DUP_UP_SRC_CNT;					
					}
					return;
				default:
					if(! ((0 == ref_com) && (block_in_mcu > 0) && (c_y + 8 >=  e_y)))
					{
						phdl->imageout.y_dup(temp_IDCT_buf2,(uchar *)y_dst);
						valid_blk = 1;
					}
					else
						valid_blk = 0;
					if(1 == d_blk_cnt_y)
						y_dst -= 120;
					else if(2 == d_blk_cnt_y)
						y_dst += 128;
					else if(3 == d_blk_cnt_y)
						y_dst += 120;
					else if(0 == d_blk_cnt_y)
						y_dst += 128;
					d_blk_cnt_y++;
					d_blk_cnt_y %= 4;
					if(block_in_mcu > 0)
						return;
			}
			break;
		case JPGD_YH2V1:
			switch(block_in_mcu)
			{
				case 3:
					phdl->imageout.c_dup(temp_IDCT_buf2,(uchar *)cr_dst);					
					cr_dst += 256;
					if(2 == ref_com)
					{
						hor_inc_cnt += inc_x;
						goto DUP_UP_SRC_CNT;
					}
					return;
				case 2:
					phdl->imageout.c_dup(temp_IDCT_buf2,(uchar *)cb_dst);	
					cb_dst += 256;
					if(1 == ref_com)
					{
						hor_inc_cnt += inc_x;
						goto DUP_UP_SRC_CNT;
					}				
					return;
				default:
					if(!((0 == ref_com) && (((c_x + hor_inc_cnt >= e_x)&&(1 == block_in_mcu)) || ((c_x + hor_inc_cnt > e_x)&&(3 == block_in_mcu)))))
						phdl->imageout.y_dup(temp_IDCT_buf2,(uchar *)y_dst);
					if(0 == d_blk_cnt_y)
						y_dst += 8;
					else if(1 == d_blk_cnt_y)
						y_dst += 248;
					d_blk_cnt_y = 1 - d_blk_cnt_y;
			}
			break;
		case JPGD_YH1V1:
			switch(block_in_mcu)
			{
				case 2:
					phdl->imageout.c_dup(temp_IDCT_buf2,(uchar *)cr_dst);					
					if(0 == d_blk_cnt_c)
					{
						cr_dst += 8;
					}
					else if(1 == d_blk_cnt_c)
					{
						cr_dst += 248;
					}
					d_blk_cnt_c = 1 - d_blk_cnt_c;
					if(2 == ref_com)
						goto DUP_UP_SRC_CNT;					
					return;
				case 1:
					phdl->imageout.c_dup(temp_IDCT_buf2,(uchar *)cb_dst);	
					if(0 == d_blk_cnt_c)
					{
						cb_dst += 8;
					}
					else if(1 == d_blk_cnt_c)
					{
						cb_dst += 248;
					}
					if(1 == ref_com)
					{
						d_blk_cnt_c = 1 - d_blk_cnt_c;
						goto DUP_UP_SRC_CNT;					
					}
					return;
				default:
					phdl->imageout.y_dup(temp_IDCT_buf2,(uchar *)y_dst);
					if(0 == d_blk_cnt_y)
						y_dst += 8;
					else if(1 == d_blk_cnt_y)
						y_dst += 248;
					d_blk_cnt_y = 1 - d_blk_cnt_y;
			}
			break;
		case JPGD_YH4V1:
			switch(block_in_mcu)
			{
				case 5:
					phdl->imageout.c_dup(temp_IDCT_buf2,(uchar *)cr_dst);	
					cr_dst += 512;
					if(2 == ref_com)
						goto DUP_UP_SRC_CNT;	
					return;
				case 4:
					phdl->imageout.c_dup(temp_IDCT_buf2,(uchar *)cb_dst);	
					cb_dst += 512;
					if(1 == ref_com)
						goto DUP_UP_SRC_CNT;					
					return;
				default:
					if(!((0 == ref_com)&&(c_x + hor_inc_cnt >= e_x)))
						phdl->imageout.y_dup(temp_IDCT_buf2,(uchar *)y_dst);
					if(0 == d_blk_cnt_y)
						y_dst += 8;
					else if(1 == d_blk_cnt_y)
						y_dst += 248;
					d_blk_cnt_y = 1 - d_blk_cnt_y;
			}
			break;
		case JPGD_GRAYSCALE:
		default:
			{break;;}
	}
DUP_UP_SRC_CNT:
	hor_inc_cnt += inc_x;
}
void imagedec_copy_frm_buf(UINT8 *src_y,UINT8 *src_c,UINT8 *dst_y,UINT8 *dst_c,UINT32 y_len,UINT32 c_len)
{
	dst_y = (UINT8 *)((((UINT32)dst_y)&0x0FFFFFFF) | 0x80000000);
	dst_c = (UINT8 *)((((UINT32)dst_c)&0x0FFFFFFF) | 0x80000000);
	src_y = (UINT8 *)((((UINT32)src_y)&0x0FFFFFFF) | 0x80000000);
	src_c = (UINT8 *)((((UINT32)src_c)&0x0FFFFFFF) | 0x80000000);
	MEMCPY(dst_y,src_y,y_len);
	MEMCPY(dst_c,src_c,c_len);
}
