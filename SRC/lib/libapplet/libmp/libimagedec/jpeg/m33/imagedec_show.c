
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

#define MAX_SHOW_ROUTINE_NUM			(6)
#define SHOW_REG_WIDTH				(720)
#define SHOW_REG_HEIGHT				(576)

#define SHOW_BLOCK_WIDTH				(45)
#define SHOW_BLOCK_HEIGHT				(36)

extern  bool g_imagedec_stop_flag[IMAGEDEC_MAX_INSTANCE];
extern  OSAL_ID g_imagedec_wait_flag[IMAGEDEC_MAX_INSTANCE]; 
extern bool g_imagedec_dis_frm_idx[IMAGEDEC_MAX_INSTANCE];
extern bool g_imagedec_dis_flag[IMAGEDEC_MAX_INSTANCE];

/*
	ReturnValue:
		0 -- 	time out
		1 --		stop flag enable. exit the show operation
		2 -- 	fail
*/
static UINT8 show_timer(pImagedec_hdl phdl,UINT32 time)
{
	ER s_flag;
	UINT32 ptn_flag;
	
	if(TRUE == g_imagedec_stop_flag[phdl->id])
	{
		g_imagedec_stop_flag[phdl->id] = FALSE;
		s_flag = osal_flag_set(g_imagedec_wait_flag[phdl->id],IMAGEDEC_STOP_PTN);
		if(E_OK != s_flag)
		{
			jpeg_printf("set flag error<%s>\n",__FUNCTION__);
			return 2;
		}
		return 1;
	}
	osal_flag_wait(&ptn_flag,g_imagedec_wait_flag[phdl->id],IMAGEDEC_STOP_PTN,TWF_ANDW,time);	
	return 0;
}

static void SHOW_NULL(pImagedec_hdl phdl)
{
	// NULL
}

/*
	Description:	Copy a 720X576 frame to a display frame with the shutters mode. Between the operations add a 
				time out. The start position of every window is 16 pixels aligned.
				1, horizontally.
					576 = 64 X 9
				2, vertically. 
					720 = 64 X 11 + 16
*/

#define SHUTTERS_TIME_OUT		100
static void SHOW_SHUTTERS(pImagedec_hdl phdl)
{
	pImagedec_show_shutters pshutters = (pImagedec_show_shutters)phdl->main.setting.show_mode_par;
	
	UINT8 *dst_y = phdl->imageout.frm[phdl->imageout.frm_idx_using].frm_y_addr;
	UINT8 *dst_c = phdl->imageout.frm[phdl->imageout.frm_idx_using].frm_c_addr;
	UINT8 *src_y = phdl->imageout.frm_y_addr;
	UINT8 *src_c = phdl->imageout.frm_c_addr;

	UINT32 *start_addr = NULL;// 0~3: dst_y_addr 4~7: dst_c_addr 8~11: src_y_addr 12~15: src_c_addr
	UINT32 loop = 0;
	
	UINT32 wins = 0;
	UINT32 cnt = 0,cnt2 = 0;
	UINT32 stride_y = 0,stride_c = 0;
	UINT32 size_y = 0,size_c = 0;
	UINT32 i = 0;
	UINT32 j = 0;

	if(0 == pshutters->direction){// horizontal
		wins = cnt = 9;
		start_addr = (UINT32 *)imagedec_malloc(cnt<<4);
		if(NULL == start_addr)		return;
		// init variables
		loop = (SHOW_REG_HEIGHT/cnt)>>2;// 64/4 = 16. evervy time copy 4 lines
		stride_y = (SHOW_REG_HEIGHT/cnt) * SHOW_REG_WIDTH;
		stride_c = stride_y>>1;
		cnt <<= 2;
		for(i = 0;i < cnt;i += 4){
			*(start_addr + i) = (UINT32)dst_y;
			*(start_addr + i + 1) = (UINT32)dst_c;
			*(start_addr + i + 2) = (UINT32)src_y;
			*(start_addr + i + 3) = (UINT32)src_c;
			dst_y += stride_y;
			dst_c += stride_c;
			src_y += stride_y;
			src_c += stride_c;
		}
		// loop operation
		while(loop--){
			cnt = wins<<2;
			for(i = 0;i < cnt;i += 4){
				dst_y = (UINT8 *)*(start_addr + i);
				dst_c = (UINT8 *)*(start_addr + i + 1);
				src_y = (UINT8 *)*(start_addr + i + 2);
				src_c = (UINT8 *)*(start_addr + i + 3);			
				cnt2 = SHOW_REG_WIDTH/16;
				size_y = 64;// 16 X 4
				size_c = 32;// 16 X 2
				while(cnt2--){
					jh_mem_cpy(dst_y,src_y,size_y); 
					jh_mem_cpy(dst_c,src_c,size_c);
					dst_y += 256;// 16 X 16
					dst_c += 256;// 16X16
					src_y += 256;
					src_c += 256;
				}
				//generate the next loop start address
				dst_y = (UINT8 *)*(start_addr + i);
				dst_c = (UINT8 *)*(start_addr + i + 1);
				src_y = (UINT8 *)*(start_addr + i + 2);
				src_c = (UINT8 *)*(start_addr + i + 3);	
				dst_y += size_y;
				dst_c += size_c;
				src_y += size_y;
				src_c += size_c;
				if(0 == (loop%4)){
					dst_y += 11264;// 720X16 - 256
					src_y += 11264;// 720X16 - 256
					if(0 == (loop%8)){
						dst_c += 11264;
						src_c += 11264;
					}
				}
				*(start_addr + i) = (UINT32)dst_y;
				*(start_addr + i + 1) = (UINT32)dst_c;
				*(start_addr + i + 2) = (UINT32)src_y;
				*(start_addr + i + 3) = (UINT32)src_c;	
			}
			if(show_timer(phdl,pshutters->time) > 0)
				return;
		}
	}
	else if(1 == pshutters->direction){
		wins = cnt = 12;// 64X11 + 16
		start_addr = (UINT32 *)imagedec_malloc(cnt<<4);
		if(NULL == start_addr)		return;
		// init variables
		loop = 16;// 64/4 = 16. evervy time copy 4 arrays data
		stride_y = 1024;// 256 X 4
		stride_c = 1024;
		cnt <<= 2;
		for(i = 0;i < cnt;i += 4){
			*(start_addr + i) = (UINT32)dst_y;
			*(start_addr + i + 1) = (UINT32)dst_c;
			*(start_addr + i + 2) = (UINT32)src_y;
			*(start_addr + i + 3) = (UINT32)src_c;
			dst_y += stride_y;
			dst_c += stride_c;
			src_y += stride_y;
			src_c += stride_c;
		}
		// loop operation
		while(loop--){
			cnt = wins<<2;
			for(i = 0;i < cnt;i += 4){
				dst_y = (UINT8 *)*(start_addr + i);
				dst_c = (UINT8 *)*(start_addr + i + 1);
				src_y = (UINT8 *)*(start_addr + i + 2);
				src_c = (UINT8 *)*(start_addr + i + 3);		
				cnt2 = 576;
				if((i == (cnt - 4)) && (loop < 12))
					break;
				size_y = 4;
				size_c = 4;
				while(cnt2--){
					jh_mem_cpy(dst_y,src_y,size_y); 
					dst_y += 16;
					src_y += 16;
					if(0 == (cnt2%2)){
						jh_mem_cpy(dst_c,src_c,size_c);
						dst_c += 16;
						src_c += 16;
						if(0 == (cnt2%16)){
							dst_y += 11264;
							src_y += 11264;
							if(0 == (cnt2%32)){
								dst_c += 11264;
								src_c += 11264;
							}
						}
					}
				}
				//generate the next loop start address
				dst_y = (UINT8 *)*(start_addr + i);
				dst_c = (UINT8 *)*(start_addr + i + 1);
				src_y = (UINT8 *)*(start_addr + i + 2);
				src_c = (UINT8 *)*(start_addr + i + 3);	
				dst_y += size_y;
				dst_c += size_c;
				src_y += size_y;
				src_c += size_c;
				if(0 == (loop%4)){
					dst_y += 240;
					src_y += 240;
					dst_c += 240;
					src_c += 240;
				}
				*(start_addr + i) = (UINT32)dst_y;
				*(start_addr + i + 1) = (UINT32)dst_c;
				*(start_addr + i + 2) = (UINT32)src_y;
				*(start_addr + i + 3) = (UINT32)src_c;	
			}
			if(show_timer(phdl,pshutters->time) > 0)
				return;
		}	
	}
	phdl->imageout.frm[phdl->imageout.frm_idx_using].busy = 1;
}

#define BRUSH_TIME_OUT 	3
static void SHOW_BRUSH(pImagedec_hdl phdl)
{
	pImagedec_show_brush pbrush = (pImagedec_show_brush)phdl->main.setting.show_mode_par;
	
	UINT8 *dst_y = phdl->imageout.frm[phdl->imageout.frm_idx_using].frm_y_addr;
	UINT8 *dst_c = phdl->imageout.frm[phdl->imageout.frm_idx_using].frm_c_addr;
	UINT8 *src_y = phdl->imageout.frm_y_addr;
	UINT8 *src_c = phdl->imageout.frm_c_addr;

	UINT32 loop1 = 0,loop2 = 0;
	UINT8 *dst_y2 = NULL,*dst_c2 = NULL,*src_y2 = NULL,*src_c2 = NULL;
	
	if(0 == pbrush->direction){
		loop1 = 720;
		while(loop1--){
			dst_y2 = dst_y;dst_c2 = dst_c;src_y2 = src_y;src_c2 = src_c;
			loop2 = 576;
			while(loop2--){
				*dst_y2 = *src_y2;
				dst_y2 += 16;src_y2 += 16;
				if(0 == (loop2%16)){
					dst_y2 += 11264;// 720X16 - 256
					src_y2 += 11264;
				}
				if((0 != (loop1%2)) && (0 == (loop2%2))){
					*(UINT16 *)dst_c2 = *(UINT16 *)src_c2;	
					dst_c2 += 16;src_c2 += 16;
					if(0 == (loop2%32)){
						dst_c2 += 11264;
						src_c2 += 11264;
					}
				}
			}
			dst_y++;src_y++;
			dst_c++;src_c++;
			if(0 == (loop1%16)){
				dst_y += 240;
				src_y += 240;
				dst_c += 240;
				src_c += 240;
			}
			if(show_timer(phdl,pbrush->time) > 0)
				return;		
		}
	}
	else if(1 == pbrush->direction)
	{
		loop1 = 576;
		while(loop1--){
			dst_y2 = dst_y;dst_c2 = dst_c;src_y2 = src_y;src_c2 = src_c;
			loop2 = 720;
			while(loop2--){
				*dst_y2++ = *src_y2++;
				if(0 == (loop2%16)){
					dst_y2 += 240;// 256 - 16
					src_y2 += 240;
				}
				if((0 != (loop1%2)) && (0 == (loop2%2))){
					*(UINT16 *)dst_c2 = *(UINT16 *)src_c2;	
					dst_c2 += 2;src_c2 += 2;
					if(0 == (loop2%16)){
						dst_c2 += 240;
						src_c2 += 240;
					}
				}
			}
			dst_y += 16;src_y += 16;
			dst_c += 8;src_c += 8;
			if(0 == (loop1%16)){
				dst_y += 11264;
				src_y += 11264;
				if(0 == (loop1%32)){
					dst_c += 11264;
					src_c += 11264;
				}
			}
			if(show_timer(phdl,pbrush->time) > 0)
				return;		
		}
	}
	phdl->imageout.frm[phdl->imageout.frm_idx_using].busy = 1;
}

#define SLIDE_TIME_OUT			2
static void SHOW_SLIDE(pImagedec_hdl phdl)
{
	pImagedec_show_slide pslide = (pImagedec_show_slide)phdl->main.setting.show_mode_par;
	
	UINT8 *dst_y = phdl->imageout.frm[phdl->imageout.frm_idx_using].frm_y_addr;
	UINT8 *dst_c = phdl->imageout.frm[phdl->imageout.frm_idx_using].frm_c_addr;
	UINT8 *src_y = phdl->imageout.frm_y_addr;
	UINT8 *src_c = phdl->imageout.frm_c_addr;
	
	UINT8 *dst_y2 = NULL,*dst_c2 = NULL,*src_y2 = NULL,*src_c2 = NULL;
	UINT8 *dst_y3 = NULL,*dst_c3 = NULL,*src_y3 = NULL,*src_c3 = NULL;
	
	UINT32 loop = 0,loop1 = 0,loop2 = 0,loop3 = 0,left  = 0;
	UINT32 dst_row = 0,src_row = 0;
	
	if(0 == pslide->direction){
		loop = phdl->imageout.image_out_w>>4;// 720/16
		left = phdl->imageout.image_out_w & 0xF;
		loop3 = 16;
		dst_y3 = dst_y;
		dst_c3 = dst_c;
		src_y3 = src_y + ((loop - 1)<<8);// 11264 when full size;
		src_c3 = src_c + ((loop - 1)<<8);
		while(loop--){
			loop1 = loop3;
			dst_y = dst_y3;dst_c = dst_c3;src_y = src_y3;src_c = src_c3;
			while(loop1){
				dst_y2 = dst_y;
				dst_c2 = dst_c;
				src_y2 = src_y;
				src_c2 = src_c;
				loop2 = 36;
				while(loop2--){
					jh_mem_cpy(dst_y2,src_y2,256);
					jh_mem_cpy(dst_c2,src_c2,128);
					dst_y2 += 11520;
					src_y2 += 11520;
					if(0 == (loop2%2)){
						dst_c2 += 11392;
						src_c2 += 11392;
					}
					else{
						dst_c2 += 128;
						src_c2 += 128;
					}
				}
				dst_y += 256;
				src_y += 256;
				dst_c += 256;
				src_c += 256;
				loop1 -= 16;
			}
			if(0 == loop){
				if(0 != left){
					left &= ~3;
					loop3 <<= 4;
					dst_y = dst_y3 + loop3;
					dst_c = dst_c3 + loop3;
					src_y = src_y3 + loop3;
					src_c = src_c3 + loop3;
					loop2 = 576;
					while(loop2--){
						jh_mem_cpy(dst_y,src_y,left);
						if(0 == (loop2%2)){	
							jh_mem_cpy(dst_c,src_c,left);
							dst_c += 16;
							src_c += 16;
							if(0 == (loop2%16)){
								dst_y += 11264;// 720X16 - 256;
								src_y += 11264;
								if(0 == (loop2%32)){
									dst_c += 11264;
									src_c += 11264;
								}
							}
						}
						dst_y += 16;
						src_y += 16;
					}
				}
			}		
			loop3 += 16;
			src_y3 -= 256;
			src_c3 -= 256;
			if(show_timer(phdl,pslide->time) > 0)
				return;		
		}	
	}
	else if(1 == pslide->direction){
		loop = phdl->imageout.image_out_h>>4;// 576/16
		left = phdl->imageout.image_out_h & 0xF;
		loop3 = 16;
		dst_y3 = dst_y;
		dst_c3 = dst_c;
		src_y3 = src_y + (11520 * (loop - 1));// 720X576 - 720X16
		src_c3 = src_c + (11520 * ((loop - 1)>>1)) + ((((loop - 1)&1))?128:0);// 720X272 + 128
		while(loop--){
			loop1 = loop3;
			dst_y = dst_y3;
			dst_c = dst_c3;
			src_y = src_y3;
			src_c = src_c3;
			dst_row = 0;
			src_row = loop;
			while(loop1){
				jh_mem_cpy(dst_y,src_y,11520);
				loop2 = 45;
				dst_c2 = dst_c;
				src_c2 = src_c;
				while(loop2--){
					jh_mem_cpy(dst_c2,src_c2,128);
					dst_c2 += 256;
					src_c2 += 256;
				}
				if(0 == (src_row%2))
					src_c += 128;
				else
					src_c += 11392;
				if(0 == (dst_row%2))
					dst_c += 128;
				else
					dst_c += 11392;
				dst_y += 11520;
				src_y += 11520;
				loop1 -= 16;
				dst_row++;
				src_row++;
			}		
			if(0 == loop){
				if(0 != left){
					left &= ~3;
					dst_y = dst_y3 + (720 * loop3);
					dst_c = dst_c3 + (720 * (loop3>>1)) + ((0 == (loop3%32))?0:128);
					src_y = src_y3 + (720 * loop3);
					src_c = src_c3 + (720 * (loop3>>1)) + ((0 == (loop3%32))?0:128);
					while(left--){
						dst_y2 = dst_y;
						dst_c2 = dst_c;
						src_y2 = src_y;
						src_c2 = src_c;
						loop2 = 45;					
						while(loop2--){
							jh_mem_cpy(dst_y2,src_y2,16);
							dst_y2 += 256;
							src_y2 += 256;
							if(0 != (left%2)){
								jh_mem_cpy(dst_c2,src_c2,16);
								dst_c2 += 256;
								src_c2 += 256;
							}
						}
						dst_y += 16;
						src_y += 16;
						dst_c += 8;
						src_c += 8;
					}
				}
			}
			loop3 += 16;
			src_y3 -= 11520;
			if(0 != (loop%2))
				src_c3 -= 128;
			else
				src_c3 -= 11392;// 720X16 - 128
			if(show_timer(phdl,pslide->time) > 0)
				return;		
		}	
	}
	phdl->imageout.frm[phdl->imageout.frm_idx_using].busy = 1;
}

#define GET_RAN(max)			((read_tsc()&0xFFFF)%max)
#define RANDOM_TIME_OUT	1
#define RANDOM_RANGE		4

static void SHOW_RANDOM(pImagedec_hdl phdl)
{
	pImagedec_show_random prandom = (pImagedec_show_random)phdl->main.setting.show_mode_par;
	
	UINT8 *dst_y = phdl->imageout.frm[phdl->imageout.frm_idx_using].frm_y_addr;
	UINT8 *dst_c = phdl->imageout.frm[phdl->imageout.frm_idx_using].frm_c_addr;
	UINT8 *src_y = phdl->imageout.frm_y_addr;
	UINT8 *src_c = phdl->imageout.frm_c_addr;

	UINT8 *dst_y2 = NULL,*dst_c2 = NULL,*src_y2 = NULL,*src_c2 = NULL;
	UINT8 *dst_y3 = NULL,*dst_c3 = NULL,*src_y3 = NULL,*src_c3 = NULL;	
	UINT32 *block_addr = NULL;
	UINT32 *table = NULL,*table2 = NULL;
	UINT32 blk_num = 0;
	UINT32 i = 0,j = 0,tmp = 0;

	blk_num = (SHOW_REG_HEIGHT>>4) * (SHOW_REG_WIDTH>>4);	
	if(0 == prandom->type){
		table = block_addr = (UINT32 *)imagedec_malloc(blk_num<<4);// every item include 16 bytes
		if(NULL == table) 		return;
		// init the block items
		i = (SHOW_REG_HEIGHT>>4);
		dst_y2 = dst_y;
		dst_c2 = dst_c;
		src_y2 = src_y;
		src_c2 = src_c;
		while(i--){
			dst_y3 = dst_y2;
			dst_c3 = dst_c2;
			src_y3 = src_y2;
			src_c3 = src_c2;
			j = (SHOW_REG_WIDTH>>4);
			while(j--){
				*table++ = (UINT32)dst_y3;
				*table++ = (UINT32)dst_c3;
				*table++ = (UINT32)src_y3;
				*table++ = (UINT32)src_c3;
				dst_y3 += 256;
				dst_c3 += 256;
				src_y3 += 256;
				src_c3 += 256;
			}
			dst_y2 += 11520;
			src_y2 += 11520;
			if(0 == (i%2)){
				dst_c2 += 11392;
				src_c2 += 11392;
			}
			else{
				dst_c2 += 128;
				src_c2 += 128;
			}
		}
		//rerange the bolck sequence in random
		i = blk_num;
		table = table2 = block_addr;
		while(i--){
			table2 = block_addr + (GET_RAN(blk_num)<<2);	
			j = 4;
			while(j--){
				tmp = *table;
				*table++ = *table2;
				*table2++ = tmp;
			}
		}
		//output the graphic by the block array
		i = blk_num;
		table = block_addr;
		while(i--){
			dst_y2 = (UINT8 *)*table++;
			dst_c2 = (UINT8 *)*table++;
			src_y2 = (UINT8 *)*table++;
			src_c2 = (UINT8 *)*table++;			
			jh_mem_cpy(dst_y2,src_y2,256);
			jh_mem_cpy(dst_c2,src_c2,128);
			if(show_timer(phdl,prandom->time) > 0)
				return;	
		}
	}
	else if(1 == prandom->type) // random fade
	{
		UINT8 *status = NULL;
		UINT32 *step = NULL;
		UINT32 step_cnt = 0,ran = 0;

		UINT32 m = 0;

		status = (UINT8 *)imagedec_malloc(blk_num);
		step = (UINT32 *)imagedec_malloc(blk_num<<2);		
		table = block_addr = (UINT32 *)imagedec_malloc(blk_num<<4);// every item include 16 bytes
		if((NULL == table) || (NULL == status) || (NULL == step)) 		return;
		// init the block items
		i = (SHOW_REG_WIDTH>>4);
		dst_y2 = dst_y;
		dst_c2 = dst_c;
		src_y2 = src_y;
		src_c2 = src_c;
		while(i--){
			dst_y3 = dst_y2;
			dst_c3 = dst_c2;
			src_y3 = src_y2;
			src_c3 = src_c2;
			j = (SHOW_REG_HEIGHT>>4);
			while(j--){
				*table++ = (UINT32)dst_y3;
				*table++ = (UINT32)dst_c3;
				*table++ = (UINT32)src_y3;
				*table++ = (UINT32)src_c3;
				dst_y3 += 11520;
				src_y3 += 11520;
				if(0 == (j%2))
				{
					dst_c3 += 11392;
					src_c3 += 11392;
				}
				else
				{
					dst_c3 += 128;
					src_c3 += 128;
				}
			}
			dst_y2 += 256;
			src_y2 += 256;
			dst_c2 += 256;
			src_c2 += 256;
		}
		// rerange the blocks 
		i = 0;
		step_cnt = 0;
		m = 0;
		while(i < SHOW_BLOCK_WIDTH){
			j = 0;	
			while(j < SHOW_BLOCK_HEIGHT){
				if(0 == status[m]){
					tmp = m;
					if(i > (SHOW_BLOCK_WIDTH - RANDOM_RANGE)){
						step[step_cnt++] = tmp;
						status[tmp] = 1;
					}
					else{
						ran = GET_RAN(RANDOM_RANGE) + 1;
						while(ran--){
							step[step_cnt++] = tmp;
							status[tmp] = 1;	
							tmp += SHOW_BLOCK_HEIGHT;
						}
					}
				}
				m++;
				j++;
			}
			i++;
		}
		// do the operataion
		i = 0;
		while(i < blk_num){
			table = block_addr + (step[i]<<2);
			dst_y2 = (UINT8 *)*table++;
			dst_c2 = (UINT8 *)*table++;
			src_y2 = (UINT8 *)*table++;
			src_c2 = (UINT8 *)*table++;			
			jh_mem_cpy(dst_y2,src_y2,256);
			jh_mem_cpy(dst_c2,src_c2,128);
			if(show_timer(phdl,prandom->time) > 0)
				return;
			i++;
		}		
	}
	else if(2 == prandom->type){// box fade
		UINT32 *step = NULL;
		UINT32 step_cnt = 0,def_point = 0,m = 0;
		
		step = (UINT32 *)imagedec_malloc(blk_num<<2);		
		table = block_addr = (UINT32 *)imagedec_malloc(blk_num<<4);// every item include 16 bytes
		if((NULL == table) || (NULL == step)) 		return;
		// init the block items
		i = (SHOW_REG_HEIGHT>>4);
		dst_y2 = dst_y;
		dst_c2 = dst_c;
		src_y2 = src_y;
		src_c2 = src_c;
		while(i--){
			dst_y3 = dst_y2;
			dst_c3 = dst_c2;
			src_y3 = src_y2;
			src_c3 = src_c2;
			j = (SHOW_REG_WIDTH>>4);
			while(j--){
				*table++ = (UINT32)dst_y3;
				*table++ = (UINT32)dst_c3;
				*table++ = (UINT32)src_y3;
				*table++ = (UINT32)src_c3;
				dst_y3 += 256;
				dst_c3 += 256;
				src_y3 += 256;
				src_c3 += 256;
			}
			dst_y2 += 11520;
			src_y2 += 11520;
			if(0 == (i%2)){
				dst_c2 += 11392;
				src_c2 += 11392;
			}
			else{
				dst_c2 += 128;
				src_c2 += 128;
			}
		}
		// rerange the blocks
		step_cnt = 0;
		def_point = ((SHOW_BLOCK_HEIGHT>>1) - 1) * SHOW_BLOCK_WIDTH + (SHOW_BLOCK_WIDTH>>1);
		step[step_cnt++] = def_point;
		def_point -= SHOW_BLOCK_WIDTH - 1;
		for(i = 1;i < 18;i++){
			tmp = i<<1;
			// down
			for(m = 0;m < tmp;m++){
				def_point += SHOW_BLOCK_WIDTH;
				step[step_cnt++] = def_point;
			}
			// left
			for(m = 0;m < tmp;m++){
				def_point--;
				step[step_cnt++] = def_point;
			}			
			// up
			for(m = 0;m < tmp;m++){
				def_point -= SHOW_BLOCK_WIDTH;
				step[step_cnt++] = def_point;
			}
			// right
			for(m = 0;m < tmp;m++){
				def_point++;
				step[step_cnt++] = def_point;
			}			
			// up and right
			def_point -= SHOW_BLOCK_WIDTH - 1;		
		}

		// fill the right side
		def_point += SHOW_BLOCK_WIDTH;
		j = tmp = def_point;
		for(m = 0;m < SHOW_BLOCK_HEIGHT;m++){
			i = 5;
			while(i--){
				step[step_cnt++] = tmp++;
			}
			j += SHOW_BLOCK_WIDTH;
			tmp = j;
		}
		// fill the left side
		def_point = 4;
		j = tmp = def_point;
		for(m = 0;m < SHOW_BLOCK_HEIGHT;m++){
			i = 5;
			while(i--){
				step[step_cnt++] = tmp--;
			}
			j += SHOW_BLOCK_WIDTH;
			tmp = j;
		}		
		// fill the bottom side
		def_point = (SHOW_BLOCK_HEIGHT - 1) * SHOW_BLOCK_WIDTH + 5;
		m =  35;
		while(m--){
			step[step_cnt++] = def_point++;
		}
		
		// do the operataion
		i = 0;
		while(i < blk_num){
			table = block_addr + (step[i]<<2);
			dst_y2 = (UINT8 *)*table++;
			dst_c2 = (UINT8 *)*table++;
			src_y2 = (UINT8 *)*table++;
			src_c2 = (UINT8 *)*table++;			
			jh_mem_cpy(dst_y2,src_y2,256);
			jh_mem_cpy(dst_c2,src_c2,128);
			if(show_timer(phdl,prandom->time) > 0)
				return;
			i++;
		}	
	}
	phdl->imageout.frm[phdl->imageout.frm_idx_using].busy = 1;	
}

#define FADE_TIME_OUT		0
static void SHOW_FADE(pImagedec_hdl phdl)
{
	pImagedec_show_fade pfade = (pImagedec_show_fade)phdl->main.setting.show_mode_par;

	UINT8 *old_y = phdl->imageout.old_frm.frm_y_addr;
	UINT8 *old_c = phdl->imageout.old_frm.frm_c_addr;
	
	UINT8 *dst_y = phdl->imageout.frm[1 - phdl->imageout.frm_idx_using].frm_y_addr;
	UINT8 *dst_c = phdl->imageout.frm[1 - phdl->imageout.frm_idx_using].frm_c_addr;
	UINT8 *src_y = phdl->imageout.frm_y_addr;
	UINT8 *src_c = phdl->imageout.frm_c_addr;

	UINT8 *dst_y2 = NULL,*dst_c2 = NULL,*src_y2 = NULL,*src_c2 = NULL,*old_y2 = NULL,*old_c2 = NULL;
	UINT32 y_len = 0x65400;
	UINT32 c_len = 0x32A00;

	UINT16 *mul_table = NULL,*table = NULL;
	UINT32 tab_size = 0;
	UINT8 level = 0,mul = 0,shift = 0;
	UINT8 index = 1 - phdl->imageout.frm_idx_using;
	UINT32 i = 0,j = 0;
	
	if(0 == pfade->type){
		level = 16;
#if 1		
		level = (level - 2)>>1;
#else
		level -= 1;
#endif
		shift = 4;
		tab_size = 512 * level;// 256 X 2. every entry is 2bytes
		//copy old frm data to hw acc buf
		jh_mem_cpy((void *)phdl->imageout.hw_acc_frm_y,(void *)old_y,y_len);
		jh_mem_cpy((void *)phdl->imageout.hw_acc_frm_c,(void *)old_c,c_len);
		old_y = (UINT8 *)phdl->imageout.hw_acc_frm_y;
		old_c = (UINT8 *)phdl->imageout.hw_acc_frm_c;
		// init the multiplication table
		table = mul_table = (UINT16 *)imagedec_malloc(tab_size);
		if(NULL == mul_table) 		return;
		for(i = 1;i <= level;i++){
#if 1			
			mul = i<<1;
#else
			mul = i;
#endif
			for(j = 0;j < 256;j++){
				*table++ = (UINT16)(j * mul);	
			}
		}
		// several alpha operations with different aplah value
		// fade levels: 2/16, 4/16,6/16,8/16,...,14/16
		table = mul_table + ((level - 1)<<8);
		while(level--){		
			dst_y2 = dst_y;
			dst_c2 = dst_c;
			src_y2 = src_y;
			src_c2 = src_c;
			old_y2 = old_y;
			old_c2 = old_c;
			for(i = 0;i < y_len;i++){
				*dst_y2++ = (UINT8)((*(table + *old_y2++) + *(mul_table + *src_y2++))>>shift);
				if(0 == (i%2)){
					*dst_c2++ = (UINT8)((*(table + *old_c2++) + *(mul_table + *src_c2++))>>shift);
				}
			}
			mul_table += 256;
			table -= 256;
			phdl->imageout.frm_idx_using = index;
			g_imagedec_dis_frm_idx[phdl->id] = index;
			osal_interrupt_disable();
			g_imagedec_dis_flag[phdl->id] = TRUE;
			osal_interrupt_enable();
			while(g_imagedec_dis_flag[phdl->id])
				{osal_task_sleep(50);};
			index = 1 - index;
			dst_y = phdl->imageout.frm[index].frm_y_addr;
			dst_c = phdl->imageout.frm[index].frm_c_addr;			
			if(show_timer(phdl,pfade->time) > 0)
				break;
		}
		jh_mem_cpy(dst_y,src_y,y_len);
		jh_mem_cpy(dst_c,src_c,c_len);
		phdl->imageout.frm_idx_using = index;
		g_imagedec_dis_frm_idx[phdl->id] = index;
		osal_interrupt_disable();
		g_imagedec_dis_flag[phdl->id] = TRUE;
		osal_interrupt_enable();
		while(g_imagedec_dis_flag[phdl->id])
			{osal_task_sleep(50);};		
	}
	phdl->imageout.frm[phdl->imageout.frm_idx_using].busy = 1;		
	phdl->imageout.frm[1 - phdl->imageout.frm_idx_using].busy = 0;	
}

static void (*show_opn[MAX_SHOW_ROUTINE_NUM])(pImagedec_hdl) = {SHOW_NULL
	,SHOW_SHUTTERS,SHOW_BRUSH,SHOW_SLIDE,SHOW_RANDOM,SHOW_FADE};

void img_show_opn(pImagedec_hdl phdl)
{
	enum IMAGE_SHOW_MODE mode;

	JPEG_ENTRY;
	mode = phdl->main.setting.show_mode;
	jpeg_printf("Show operation %d\n",mode);
	if(((UINT32)mode) < MAX_SHOW_ROUTINE_NUM)
		show_opn[mode](phdl);
	else
		{jpeg_printf("Show mode fail %d\n",mode);};
	JPEG_EXIT;
}

