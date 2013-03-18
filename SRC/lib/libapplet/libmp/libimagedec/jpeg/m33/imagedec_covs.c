
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
#include "imagedec_main.h"
#include "imagedec_covs.h"
#include "imagedec_acc.h"

extern enum IMAGE_ANGLE g_imagedec_angle[IMAGEDEC_MAX_INSTANCE];
extern int g_imagedec_m33_extend_hw_vld;

extern int g_imagedec_force_full_screen[IMAGEDEC_MAX_INSTANCE];

struct jhconvs_main g_jhconvs_info;
/*scale parameters*/
static volatile UINT32 g_jhconvs_scale_hor_int = 0;
static volatile UINT32 g_jhconvs_scale_hor_fra = 0;
static volatile UINT32 g_jhconvs_scale_hor_int_c = 0;
static volatile UINT32 g_jhconvs_scale_hor_fra_c = 0;
static volatile UINT32 g_jhconvs_scale_ver_int = 0;
static volatile UINT32 g_jhconvs_scale_ver_fra = 0;
static volatile UINT32 g_jhconvs_scale_ver_int_c = 0;
static volatile UINT32 g_jhconvs_scale_ver_fra_c = 0;
static volatile UINT32 g_jhconvs_scale_factor_inverse = 0;
static volatile UINT32 g_jhconvs_scale_factor_inverse_c = 0;
static volatile UINT32 *g_jhconvs_scale_bilinear_buf = NULL; 
static volatile UINT32 *g_jhconvs_scale_partsum_buf = NULL;
static volatile UINT32 *g_jhconvs_scale_partsum_buf_c = NULL;
/*control variable*/
static volatile UINT8 g_jhconvs_buf_init_flag = 0;
static volatile UINT32 g_jhconvs_buf_size = 0;

static volatile int g_jhconvs_hor_scale_flag = 0;

static volatile int m_jhconvs_hor_no_scale = 0;
static volatile int m_jhconvs_ver_no_scale = 0;

/*optimized cpy and set. address should 2-bytes aligned*/
void jh_mem_cpy(void *dst,void *src,UINT32 len)
{
	UINT16 *s = (UINT16 *)src;
	UINT16 *d = (UINT16 *)dst;
	UINT32 l = (len>>1);

	while(l--)
	{
		*d++ = *s++;
	}
	if(len&1)
	{
		*(UINT8 *)d = *(UINT8 *)s;
	}	
}
void jh_mem_set(void *dst,UINT8 value,UINT32 len)
{
	UINT16 *d = (UINT16 *)dst;
	UINT16 v = (value<<8)|value;
	UINT32 l = (len>>1);

	while(l--)
	{
		*d++ = v;
	}
	if(len&1)
	{
		*(UINT8 *)d = value;
	}
}

JHCONVS_INLINE void jhcovs_m3329e_y_full(struct jhconvs_main *main)
{
	UINT32 x_s = main->src_info.rect.x;
	UINT32 y_s = main->src_info.rect.y;
	UINT32 w_s = main->src_info.rect.w;
	UINT32 h_s = main->src_info.rect.h;
	UINT32 pit_s = main->src_info.stride;
	UINT32 x_d = main->dst_info.rect.x;
	UINT32 y_d = main->dst_info.rect.y;
	UINT32 w_d = main->dst_info.rect.w;
	UINT32 h_d = main->dst_info.rect.h;
	UINT32 pit_d = main->dst_info.stride;	
	UINT32 ver_part_sum_cnt = 0,ver_int_cnt = 0,ver_fra_counter =  g_jhconvs_scale_ver_fra,fra = 0,fra2 = 0,out = 0;
	UINT8 *addr_s = 0,*addr_d = 0,*ver_part_sum_buf = 0,*ver_bilinear_buf = 0;

	while(h_s)
	{
		w_s = main->src_info.rect.w;
		x_s = main->src_info.rect.x;
		ver_part_sum_buf = (UINT8 *)(((UINT32)g_jhconvs_scale_partsum_buf)|(0x1<<31));
		addr_s = (UINT8 *)(main->src_info.y_addr+ (y_s & 0xFFF0) * pit_s + ((y_s & 0x0F) <<4) + \
			((x_s & 0xFFF0) <<4) + (x_s & 0x0F));
		while(w_s)
		{
			*ver_part_sum_buf++ = *addr_s++;
			if(0xF == (x_s&0xF))
				addr_s += 240;
			w_s--;x_s++;
		}
		w_d = main->dst_info.rect.w;
		x_d = main->dst_info.rect.x;
		ver_part_sum_buf = (UINT8 *)(((UINT32)g_jhconvs_scale_partsum_buf)|(0x1<<31));
		ver_bilinear_buf = (UINT8 *)(((UINT32)g_jhconvs_scale_bilinear_buf)|(0x1<<31));
//		ver_part_sum_cnt++;
//		if(ver_part_sum_cnt >= g_jhconvs_scale_ver_int)
		{
			ver_int_cnt++;
			if(ver_int_cnt > (ver_fra_counter>>8))
			{
				addr_d = (UINT8 *)(main->dst_info.y_addr + (y_d & 0xFFF0) * pit_d + ((y_d & 0x0F) <<4) + \
					((x_d & 0xFFF0) <<4) + (x_d & 0x0F));	
				fra = ver_fra_counter&0xFF;
				fra2 = 0xFF - fra;
				while(w_d)
				{
	/*				if(0 != fra)
					{
						out = fra * ((UINT32)(*ver_part_sum_buf++)) + fra2 * ((UINT32)(*ver_bilinear_buf++));
						if(ANG_180 == g_imagedec_angle[0])
							*addr_d-- = (UINT8)((out>>8)&0xFF);							
						else
							*addr_d++ = (UINT8)((out>>8)&0xFF);
					}
					else
					{
						if(ANG_180 == g_imagedec_angle[0])
							*addr_d-- = *ver_bilinear_buf++;
						else
							*addr_d++ = *ver_bilinear_buf++;
					}*/
					out = fra * ((UINT32)(*ver_part_sum_buf++)) + fra2 * ((UINT32)(*ver_bilinear_buf++));
					if(ANG_180 == g_imagedec_angle[0])
					{
						*addr_d-- = (UINT8)((out>>8)&0xFF);		
						if(0 == (x_d&0xF))
							addr_d -= 240;
						x_d--;
					}
					else
					{
						*addr_d++ = (UINT8)((out>>8)&0xFF);
						if(0xF == (x_d&0xF))
							addr_d += 240;
						x_d++;
					}
					w_d--;
				}
				ver_fra_counter += g_jhconvs_scale_ver_fra;
				if(ANG_180 == g_imagedec_angle[0])
					y_d--;
				else
					y_d++;
			}
			jh_mem_cpy((void *)g_jhconvs_scale_bilinear_buf,(void *)g_jhconvs_scale_partsum_buf,g_jhconvs_buf_size);
			jh_mem_set((void *)g_jhconvs_scale_partsum_buf,0,g_jhconvs_buf_size);
		//	ver_part_sum_cnt = 0;
		}		
		h_s--;y_s++;
	}
}
JHCONVS_INLINE void jhcovs_m3329e_y_full_90(struct jhconvs_main *main)
{
	UINT32 x_s = main->src_info.rect.x;
	UINT32 y_s = main->src_info.rect.y;
	UINT32 w_s = main->src_info.rect.w;
	UINT32 h_s = main->src_info.rect.h;
	UINT32 pit_s = main->src_info.stride;
	UINT32 x_d = main->dst_info.rect.x;
	UINT32 y_d = main->dst_info.rect.y;
	UINT32 w_d = main->dst_info.rect.w;
	UINT32 h_d = main->dst_info.rect.h;
	UINT32 pit_d = main->dst_info.stride;	
	UINT32 ver_part_sum_cnt = 0,ver_int_cnt = 0,ver_fra_counter =  g_jhconvs_scale_ver_fra,fra = 0,fra2 = 0,out = 0;
	UINT8 *addr_s = 0,*addr_d = 0,*ver_part_sum_buf = 0,*ver_bilinear_buf = 0;
	UINT32 ads_cnt = (pit_d<<4) - 256;
	
	while(h_s)
	{
		w_s = main->src_info.rect.w;
		x_s = main->src_info.rect.x;
		ver_part_sum_buf = (UINT8 *)(((UINT32)g_jhconvs_scale_partsum_buf)|(0x1<<31));
		addr_s = (UINT8 *)(main->src_info.y_addr+ (y_s & 0xFFF0) * pit_s + ((y_s & 0x0F) <<4) + \
			((x_s & 0xFFF0) <<4) + (x_s & 0x0F));
		while(w_s)
		{
			*ver_part_sum_buf++ = *addr_s++;
			if(0xF == (x_s&0xF))
				addr_s += 240;
			w_s--;x_s++;
		}
		h_d = main->dst_info.rect.h;
		y_d = main->dst_info.rect.y;
		ver_part_sum_buf = (UINT8 *)(((UINT32)g_jhconvs_scale_partsum_buf)|(0x1<<31));
		ver_bilinear_buf = (UINT8 *)(((UINT32)g_jhconvs_scale_bilinear_buf)|(0x1<<31));
//		ver_part_sum_cnt++;
//		if(ver_part_sum_cnt >= g_jhconvs_scale_ver_int)
		{
			ver_int_cnt++;
			if(ver_int_cnt > (ver_fra_counter>>8))
			{
				addr_d = (UINT8 *)(main->dst_info.y_addr + (y_d & 0xFFF0) * pit_d + ((y_d & 0x0F) <<4) + \
					((x_d & 0xFFF0) <<4) + (x_d & 0x0F));	
				fra = ver_fra_counter&0xFF;
				fra2 = 0xFF - fra;
				while(h_d)
				{
					out = fra * ((UINT32)(*ver_part_sum_buf++)) + fra2 * ((UINT32)(*ver_bilinear_buf++));
					*addr_d = (UINT8)((out>>8)&0xFF);
					if(ANG_90_A == g_imagedec_angle[0])
					{
						addr_d += 16;
						if(0xF == (y_d&0xF))
							addr_d += ads_cnt;
						y_d++;
					}
					else
					{
						addr_d -= 16;
						if(0 == (y_d&0xF))
							addr_d -= ads_cnt;					
						y_d--;
					}
					h_d--;
				}
				ver_fra_counter += g_jhconvs_scale_ver_fra;
				if(ANG_90_A == g_imagedec_angle[0])
					x_d--;
				else
					x_d++;
			}
			jh_mem_cpy((void *)g_jhconvs_scale_bilinear_buf,(void *)g_jhconvs_scale_partsum_buf,g_jhconvs_buf_size);
			jh_mem_set((void *)g_jhconvs_scale_partsum_buf,0,g_jhconvs_buf_size);
		//	ver_part_sum_cnt = 0;
		}		
		h_s--;y_s++;
	}
}
JHCONVS_INLINE void jhcovs_m3329e_c_420_full(struct jhconvs_main *main)
{
	UINT32 x_s = main->src_info.rect.x>>1;
	UINT32 y_s = main->src_info.rect.y>>1;
	UINT32 w_s = main->src_info.rect.w&(~0x01);
	UINT32 h_s = main->src_info.rect.h>>1;
	UINT32 pit_s = main->src_info.stride;
	UINT32 x_d = main->dst_info.rect.x>>1;
	UINT32 y_d = main->dst_info.rect.y>>1;
	UINT32 w_d = main->dst_info.rect.w&(~0x01);
	UINT32 h_d = main->dst_info.rect.h>>1;
	UINT32 pit_d = main->dst_info.stride;	
	UINT32 ver_part_sum_cnt = 0,ver_int_cnt = 0,ver_fra_counter = g_jhconvs_scale_ver_fra_c,fra = 0,fra2 = 0,out = 0;
	UINT8 *addr_s = 0,*addr_d = 0,*ver_part_sum_buf = 0,*ver_bilinear_buf = 0;

	while(h_s)
	{
		w_s = main->src_info.rect.w&(~0x01);
		x_s = main->src_info.rect.x>>1;
		ver_part_sum_buf = (UINT8 *)(((UINT32)g_jhconvs_scale_partsum_buf)|(1<<31));
		addr_s = (UINT8 *)(main->src_info.c_addr +  (y_s & 0xFFF0) * pit_s + \
			((y_s & 0x0F) <<4) + ((x_s & 0xFFF8) <<5) + ((x_s & 7) <<1));
		x_s <<= 1;
		while(w_s)
		{
			*ver_part_sum_buf++ = *addr_s++;
			if(0xF == (x_s&0xF))
				addr_s += 240;
			w_s--;x_s++;
		}
		w_d = main->dst_info.rect.w&(~0x01);
		x_d = main->dst_info.rect.x>>1;
		ver_part_sum_buf = (UINT8 *)(((UINT32)g_jhconvs_scale_partsum_buf)|(1<<31));
		ver_bilinear_buf = (UINT8 *)(((UINT32)g_jhconvs_scale_bilinear_buf)|(1<<31));
//		ver_part_sum_cnt++;
//		if(ver_part_sum_cnt >= g_jhconvs_scale_ver_int)
		{
			ver_int_cnt++;
			if(ver_int_cnt > (ver_fra_counter>>8))
			{
				addr_d = (UINT8 *)(main->dst_info.c_addr +  (y_d & 0xFFF0) * pit_d + \
					((y_d & 0x0F) <<4) + ((x_d & 0xFFF8) <<5) + ((x_d & 7) <<1));		
				fra = ver_fra_counter&0xFF;
				fra2 = 0xFF - fra;
				x_d <<= 1;
				while(w_d)
				{
					out = fra * ((UINT32)(*ver_part_sum_buf++)) + fra2 * ((UINT32)(*ver_bilinear_buf++));
					if(ANG_180 == g_imagedec_angle[0])
					{
						*addr_d-- = (UINT8)((out>>8)&0xFF);		
						if(0 == (x_d&0xF))
							addr_d -= 240;
						x_d--;
					}
					else
					{
						*addr_d++ = (UINT8)((out>>8)&0xFF);
						if(0xF == (x_d&0xF))
							addr_d += 240;
						x_d++;
					}
					w_d--;
				}
				ver_fra_counter += g_jhconvs_scale_ver_fra_c;
				if(ANG_180 == g_imagedec_angle[0])
					y_d--;
				else
					y_d++;
			}
			jh_mem_cpy((void *)g_jhconvs_scale_bilinear_buf,(void *)g_jhconvs_scale_partsum_buf,g_jhconvs_buf_size);
			jh_mem_set((void *)g_jhconvs_scale_partsum_buf,0,g_jhconvs_buf_size);
		//	ver_part_sum_cnt = 0;
		}		
		h_s--;y_s++;
	}
}
JHCONVS_INLINE void jhcovs_m3329e_c_420_full_90(struct jhconvs_main *main)
{
	UINT32 x_s = main->src_info.rect.x>>1;
	UINT32 y_s = main->src_info.rect.y>>1;
	UINT32 w_s = main->src_info.rect.w&(~0x01);
	UINT32 h_s = main->src_info.rect.h>>1;
	UINT32 pit_s = main->src_info.stride;
	UINT32 x_d = main->dst_info.rect.x>>1;
	UINT32 y_d = main->dst_info.rect.y>>1;
	UINT32 w_d = main->dst_info.rect.w&(~0x01);
	UINT32 h_d = main->dst_info.rect.h>>1;
	UINT32 pit_d = main->dst_info.stride;	
	UINT32 ver_part_sum_cnt = 0,ver_int_cnt = 0,ver_fra_counter = g_jhconvs_scale_ver_fra_c,fra = 0,fra2 = 0,out = 0;
	UINT8 *addr_s = 0,*addr_d = 0,*ver_part_sum_buf = 0,*ver_bilinear_buf = 0;
	UINT32 ads_cnt = (pit_d<<4) - 256;

	while(h_s)
	{
		w_s = main->src_info.rect.w&(~0x01);
		x_s = main->src_info.rect.x>>1;
		ver_part_sum_buf = (UINT8 *)(((UINT32)g_jhconvs_scale_partsum_buf)|(1<<31));
		addr_s = (UINT8 *)(main->src_info.c_addr +  (y_s & 0xFFF0) * pit_s + \
			((y_s & 0x0F) <<4) + ((x_s & 0xFFF8) <<5) + ((x_s & 7) <<1));
		x_s <<= 1;
		while(w_s)
		{
			*ver_part_sum_buf++ = *addr_s++;
			if(0xF == (x_s&0xF))
				addr_s += 240;
			w_s--;x_s++;
		}
		h_d = main->dst_info.rect.h&(~0x01);
		y_d = main->dst_info.rect.y>>1;
		ver_part_sum_buf = (UINT8 *)(((UINT32)g_jhconvs_scale_partsum_buf)|(1<<31));
		ver_bilinear_buf = (UINT8 *)(((UINT32)g_jhconvs_scale_bilinear_buf)|(1<<31));
//		ver_part_sum_cnt++;
//		if(ver_part_sum_cnt >= g_jhconvs_scale_ver_int)
		{
			ver_int_cnt++;
			if(ver_int_cnt > (ver_fra_counter>>8))
			{
				addr_d = (UINT8 *)(main->dst_info.c_addr +  (y_d & 0xFFF0) * pit_d + \
					((y_d & 0x0F) <<4) + ((x_d & 0xFFF8) <<5) + ((x_d & 7) <<1));		
				fra = ver_fra_counter&0xFF;
				fra2 = 0xFF - fra;
				//y_d <<= 1;
				while(h_d)
				{
					out = fra * ((UINT32)(*ver_part_sum_buf++)) + fra2 * ((UINT32)(*ver_bilinear_buf++));
					if(ANG_90_A == g_imagedec_angle[0])
					{
						*addr_d++ = (UINT8)((out>>8)&0xFF);		
						out = fra * ((UINT32)(*ver_part_sum_buf++)) + fra2 * ((UINT32)(*ver_bilinear_buf++));						
						*addr_d = (UINT8)((out>>8)&0xFF);	
						addr_d += 15;
						if(0xF == (y_d&0xF))
							addr_d += ads_cnt;
						y_d++;
					}
					else
					{
						*addr_d++ = (UINT8)((out>>8)&0xFF);		
						out = fra * ((UINT32)(*ver_part_sum_buf++)) + fra2 * ((UINT32)(*ver_bilinear_buf++));						
						*addr_d = (UINT8)((out>>8)&0xFF);	
						addr_d -= 17;
						if(0 == (y_d&0xF))
							addr_d -= ads_cnt;
						y_d--;
					}
					h_d -= 2;
				}
				ver_fra_counter += g_jhconvs_scale_ver_fra_c;
				if(ANG_90_A == g_imagedec_angle[0])
					x_d--;
				else
					x_d++;
			}
			jh_mem_cpy((void *)g_jhconvs_scale_bilinear_buf,(void *)g_jhconvs_scale_partsum_buf,g_jhconvs_buf_size);
			jh_mem_set((void *)g_jhconvs_scale_partsum_buf,0,g_jhconvs_buf_size);
		//	ver_part_sum_cnt = 0;
		}		
		h_s--;y_s++;
	}
}
JHCONVS_INLINE void jhcovs_m3329e_c_422_full(struct jhconvs_main *main)
{
	UINT32 x_s = main->src_info.rect.x&(~0x01);
	UINT32 y_s = main->src_info.rect.y&(~0x01);
	UINT32 w_s = main->src_info.rect.w&(~0x01);
	UINT32 h_s = main->src_info.rect.h&(~0x01);
	UINT32 pit_s = main->src_info.stride;
	UINT32 x_d = main->dst_info.rect.x>>1;
	UINT32 y_d = main->dst_info.rect.y>>1;
	UINT32 w_d = main->dst_info.rect.w&(~0x01);
	UINT32 h_d = main->dst_info.rect.h>>1;
	UINT32 pit_d = main->dst_info.stride;	
	UINT32 ver_part_sum_cnt = 0,ver_int_cnt = 0,ver_fra_counter =  g_jhconvs_scale_ver_fra_c,fra = 0,fra2 = 0,out = 0;
	UINT8 *addr_s = 0,*addr_d = 0,*ver_part_sum_buf = 0,*ver_bilinear_buf = 0;

	while(h_s)
	{
		w_s = main->src_info.rect.w&(~0x01);
		x_s = main->src_info.rect.x&(~0x01);
		ver_part_sum_buf = (UINT8 *)(((UINT32)g_jhconvs_scale_partsum_buf)|(0x1<<31));
		addr_s = (UINT8 *)(main->src_info.c_addr+ (y_s & 0xFFF0) * pit_s + ((y_s & 0x0F) <<4) + \
			((x_s & 0xFFF0) <<4) + (x_s & 0x0F));
		while(w_s)
		{
			*ver_part_sum_buf++ = *addr_s++;
			if(0xF == (x_s&0xF))
				addr_s += 240;
			w_s--;x_s++;
		}
		w_d = main->dst_info.rect.w&(~0x01);
		x_d = main->dst_info.rect.x>>1;
		ver_part_sum_buf = (UINT8 *)(((UINT32)g_jhconvs_scale_partsum_buf)|(0x1<<31));
		ver_bilinear_buf = (UINT8 *)(((UINT32)g_jhconvs_scale_bilinear_buf)|(0x1<<31));
//		ver_part_sum_cnt++;
//		if(ver_part_sum_cnt >= g_jhconvs_scale_ver_int)
		{
			ver_int_cnt++;
			if(ver_int_cnt > (ver_fra_counter>>8))
			{
				addr_d = (UINT8 *)(main->dst_info.c_addr +  (y_d & 0xFFF0) * pit_d + \
					((y_d & 0x0F) <<4) + ((x_d & 0xFFF8) <<5) + ((x_d & 7) <<1));	
				fra = ver_fra_counter&0xFF;
				fra2 = 0xFF - fra;
				x_d <<= 1;
				while(w_d)
				{
					out = fra * ((UINT32)(*ver_part_sum_buf++)) + fra2 * ((UINT32)(*ver_bilinear_buf++));
					if(ANG_180 == g_imagedec_angle[0])
					{
						*addr_d-- = (UINT8)((out>>8)&0xFF);		
						if(0 == (x_d&0xF))
							addr_d -= 240;
						x_d--;
					}
					else
					{
						*addr_d++ = (UINT8)((out>>8)&0xFF);
						if(0xF == (x_d&0xF))
							addr_d += 240;
						x_d++;
					}
					w_d--;
				}
				//JHCON_PRINTF("c line <%d>\n",y_d);
				ver_fra_counter += g_jhconvs_scale_ver_fra_c;
				if(ANG_180 == g_imagedec_angle[0])
					y_d--;
				else
					y_d++;
			}
			jh_mem_cpy((void *)g_jhconvs_scale_bilinear_buf,(void *)g_jhconvs_scale_partsum_buf,g_jhconvs_buf_size);
			jh_mem_set((void *)g_jhconvs_scale_partsum_buf,0,g_jhconvs_buf_size);
		//	ver_part_sum_cnt = 0;
		}		
		h_s--;y_s++;
	}	
}
JHCONVS_INLINE void jhcovs_m3329e_c_422_full_90(struct jhconvs_main *main)
{
	UINT32 x_s = main->src_info.rect.x&(~0x01);
	UINT32 y_s = main->src_info.rect.y&(~0x01);
	UINT32 w_s = main->src_info.rect.w&(~0x01);
	UINT32 h_s = main->src_info.rect.h&(~0x01);
	UINT32 pit_s = main->src_info.stride;
	UINT32 x_d = main->dst_info.rect.x>>1;
	UINT32 y_d = main->dst_info.rect.y>>1;
	UINT32 w_d = main->dst_info.rect.w&(~0x01);
	UINT32 h_d = main->dst_info.rect.h>>1;
	UINT32 pit_d = main->dst_info.stride;	
	UINT32 ver_part_sum_cnt = 0,ver_int_cnt = 0,ver_fra_counter =  g_jhconvs_scale_ver_fra_c,fra = 0,fra2 = 0,out = 0;
	UINT8 *addr_s = 0,*addr_d = 0,*ver_part_sum_buf = 0,*ver_bilinear_buf = 0;
	UINT32 ads_cnt = (pit_d<<4) - 256;

	while(h_s)
	{
		w_s = main->src_info.rect.w&(~0x01);
		x_s = main->src_info.rect.x&(~0x01);
		ver_part_sum_buf = (UINT8 *)(((UINT32)g_jhconvs_scale_partsum_buf)|(0x1<<31));
		addr_s = (UINT8 *)(main->src_info.c_addr+ (y_s & 0xFFF0) * pit_s + ((y_s & 0x0F) <<4) + \
			((x_s & 0xFFF0) <<4) + (x_s & 0x0F));
		while(w_s)
		{
			*ver_part_sum_buf++ = *addr_s++;
			if(0xF == (x_s&0xF))
				addr_s += 240;
			w_s--;x_s++;
		}
		h_d = main->dst_info.rect.h&(~0x01);
		y_d = main->dst_info.rect.y>>1;
		ver_part_sum_buf = (UINT8 *)(((UINT32)g_jhconvs_scale_partsum_buf)|(0x1<<31));
		ver_bilinear_buf = (UINT8 *)(((UINT32)g_jhconvs_scale_bilinear_buf)|(0x1<<31));
//		ver_part_sum_cnt++;
//		if(ver_part_sum_cnt >= g_jhconvs_scale_ver_int)
		{
			ver_int_cnt++;
			if(ver_int_cnt > (ver_fra_counter>>8))
			{
				addr_d = (UINT8 *)(main->dst_info.c_addr +  (y_d & 0xFFF0) * pit_d + \
					((y_d & 0x0F) <<4) + ((x_d & 0xFFF8) <<5) + ((x_d & 7) <<1));	
				fra = ver_fra_counter&0xFF;
				fra2 = 0xFF - fra;
				//y_d <<= 1;
				while(h_d)
				{
					out = fra * ((UINT32)(*ver_part_sum_buf++)) + fra2 * ((UINT32)(*ver_bilinear_buf++));
					if(ANG_90_A == g_imagedec_angle[0])
					{
						*addr_d++ = (UINT8)((out>>8)&0xFF);		
						out = fra * ((UINT32)(*ver_part_sum_buf++)) + fra2 * ((UINT32)(*ver_bilinear_buf++));						
						*addr_d = (UINT8)((out>>8)&0xFF);	
						addr_d += 15;
						if(0xF == (y_d&0xF))
							addr_d += ads_cnt;
						y_d++;
					}
					else
					{
						*addr_d++ = (UINT8)((out>>8)&0xFF);		
						out = fra * ((UINT32)(*ver_part_sum_buf++)) + fra2 * ((UINT32)(*ver_bilinear_buf++));						
						*addr_d = (UINT8)((out>>8)&0xFF);	
						addr_d -= 17;
						if(0 == (y_d&0xF))
							addr_d -= ads_cnt;
						y_d--;
					}
					h_d -= 2;
				}
				//JHCON_PRINTF("c line <%d>\n",y_d);
				ver_fra_counter += g_jhconvs_scale_ver_fra_c;
				if(ANG_90_A == g_imagedec_angle[0])
					x_d--;
				else
					x_d++;
			}
			jh_mem_cpy((void *)g_jhconvs_scale_bilinear_buf,(void *)g_jhconvs_scale_partsum_buf,g_jhconvs_buf_size);
			jh_mem_set((void *)g_jhconvs_scale_partsum_buf,0,g_jhconvs_buf_size);
		//	ver_part_sum_cnt = 0;
		}		
		h_s--;y_s++;
	}	
}
JHCONVS_INLINE void jhcovs_m3329e_c_411_full(struct jhconvs_main *main)
{
	UINT32 x_s = main->src_info.rect.x&(~0x01);
	UINT32 y_s = main->src_info.rect.y&(~0x01);
	UINT32 w_s = main->src_info.rect.w&(~0x01);
	UINT32 h_s = main->src_info.rect.h&(~0x01);
	UINT32 pit_s = (((main->src_info.stride>>4)+1)>>1)<<4;
	UINT32 x_d = main->dst_info.rect.x>>1;
	UINT32 y_d = main->dst_info.rect.y>>1;
	UINT32 w_d = main->dst_info.rect.w&(~0x01);
	UINT32 h_d = main->dst_info.rect.h>>1;
	UINT32 pit_d = main->dst_info.stride;	
	UINT32 ver_part_sum_cnt = 0,ver_int_cnt = 0,ver_fra_counter =  g_jhconvs_scale_ver_fra_c,fra = 0,fra2 = 0,out = 0;
	UINT8 *addr_s = 0,*addr_d = 0,*ver_part_sum_buf = 0,*ver_bilinear_buf = 0;
	UINT8 cb = 0,cr = 0;

	while(h_s)
	{
		w_s = main->src_info.rect.w>>1;
		x_s = main->src_info.rect.x>>2;
		ver_part_sum_buf = (UINT8 *)(((UINT32)g_jhconvs_scale_partsum_buf)|(0x1<<31));
		addr_s = (UINT8 *)(main->src_info.c_addr+ (y_s & 0xFFF0) * pit_s + ((y_s & 0x0F) <<4) + \
			((x_s & 0xFFF8) <<5) + ((x_s & 0x07)<<1));
		x_s <<= 1;
		while(w_s)
		{
			if(x_s&0x01)
			{
				cr = *addr_s++;
				*ver_part_sum_buf++ = cb;
				*ver_part_sum_buf++ = cr;				
				*ver_part_sum_buf++ = cb;
				*ver_part_sum_buf++ = cr;
			}
			else
			{
				cb = *addr_s++;
			}	
			if(0xF == (x_s&0xF))
				addr_s += 240;
			w_s--;x_s++;
		}
		w_d = main->dst_info.rect.w&(~0x01);
		x_d = main->dst_info.rect.x>>1;
		ver_part_sum_buf = (UINT8 *)(((UINT32)g_jhconvs_scale_partsum_buf)|(0x1<<31));
		ver_bilinear_buf = (UINT8 *)(((UINT32)g_jhconvs_scale_bilinear_buf)|(0x1<<31));
//		ver_part_sum_cnt++;
//		if(ver_part_sum_cnt >= g_jhconvs_scale_ver_int)
		{
			ver_int_cnt++;
			if(ver_int_cnt > (ver_fra_counter>>8))
			{
				addr_d = (UINT8 *)(main->dst_info.c_addr +  (y_d & 0xFFF0) * pit_d + \
					((y_d & 0x0F) <<4) + ((x_d & 0xFFF8) <<5) + ((x_d & 7) <<1));	
				fra = ver_fra_counter&0xFF;
				fra2 = 0xFF - fra;
				x_d <<= 1;
				while(w_d)
				{
					out = fra * ((UINT32)(*ver_part_sum_buf++)) + fra2 * ((UINT32)(*ver_bilinear_buf++));
					if(ANG_180 == g_imagedec_angle[0])
					{
						*addr_d-- = (UINT8)((out>>8)&0xFF);		
						if(0 == (x_d&0xF))
							addr_d -= 240;
						x_d--;
					}
					else
					{
						*addr_d++ = (UINT8)((out>>8)&0xFF);
						if(0xF == (x_d&0xF))
							addr_d += 240;
						x_d++;
					}
					w_d--;
				}
			//	JHCON_PRINTF("c line <%d>\n",y_d);
				ver_fra_counter += g_jhconvs_scale_ver_fra_c;
				if(ANG_180 == g_imagedec_angle[0])
					y_d--;
				else
					y_d++;
			}
			jh_mem_cpy((void *)g_jhconvs_scale_bilinear_buf,(void *)g_jhconvs_scale_partsum_buf,g_jhconvs_buf_size);
			jh_mem_set((void *)g_jhconvs_scale_partsum_buf,0,g_jhconvs_buf_size);
		//	ver_part_sum_cnt = 0;
		}		
		h_s--;y_s++;
	}	
}
JHCONVS_INLINE void jhcovs_m3329e_c_411_full_90(struct jhconvs_main *main)
{
	UINT32 x_s = main->src_info.rect.x&(~0x01);
	UINT32 y_s = main->src_info.rect.y&(~0x01);
	UINT32 w_s = main->src_info.rect.w&(~0x01);
	UINT32 h_s = main->src_info.rect.h&(~0x01);
	UINT32 pit_s = (((main->src_info.stride>>4)+1)>>1)<<4;
	UINT32 x_d = main->dst_info.rect.x>>1;
	UINT32 y_d = main->dst_info.rect.y>>1;
	UINT32 w_d = main->dst_info.rect.w&(~0x01);
	UINT32 h_d = main->dst_info.rect.h>>1;
	UINT32 pit_d = main->dst_info.stride;	
	UINT32 ver_part_sum_cnt = 0,ver_int_cnt = 0,ver_fra_counter =  g_jhconvs_scale_ver_fra_c,fra = 0,fra2 = 0,out = 0;
	UINT8 *addr_s = 0,*addr_d = 0,*ver_part_sum_buf = 0,*ver_bilinear_buf = 0;
	UINT8 cb = 0,cr = 0;
	UINT32 ads_cnt = (pit_d<<4) - 256;

	while(h_s)
	{
		w_s = main->src_info.rect.w>>1;
		x_s = main->src_info.rect.x>>2;
		ver_part_sum_buf = (UINT8 *)(((UINT32)g_jhconvs_scale_partsum_buf)|(0x1<<31));
		addr_s = (UINT8 *)(main->src_info.c_addr+ (y_s & 0xFFF0) * pit_s + ((y_s & 0x0F) <<4) + \
			((x_s & 0xFFF8) <<5) + ((x_s & 0x07)<<1));
		x_s <<= 1;
		while(w_s)
		{
			if(x_s&0x01)
			{
				cr = *addr_s++;
				*ver_part_sum_buf++ = cb;
				*ver_part_sum_buf++ = cr;				
				*ver_part_sum_buf++ = cb;
				*ver_part_sum_buf++ = cr;
			}
			else
			{
				cb = *addr_s++;
			}	
			if(0xF == (x_s&0xF))
				addr_s += 240;
			w_s--;x_s++;
		}
		h_d = main->dst_info.rect.h&(~0x01);
		y_d = main->dst_info.rect.y>>1;
		ver_part_sum_buf = (UINT8 *)(((UINT32)g_jhconvs_scale_partsum_buf)|(0x1<<31));
		ver_bilinear_buf = (UINT8 *)(((UINT32)g_jhconvs_scale_bilinear_buf)|(0x1<<31));
//		ver_part_sum_cnt++;
//		if(ver_part_sum_cnt >= g_jhconvs_scale_ver_int)
		{
			ver_int_cnt++;
			if(ver_int_cnt > (ver_fra_counter>>8))
			{
				addr_d = (UINT8 *)(main->dst_info.c_addr +  (y_d & 0xFFF0) * pit_d + \
					((y_d & 0x0F) <<4) + ((x_d & 0xFFF8) <<5) + ((x_d & 7) <<1));	
				fra = ver_fra_counter&0xFF;
				fra2 = 0xFF - fra;
				//x_d <<= 1;
				while(h_d)
				{
					out = fra * ((UINT32)(*ver_part_sum_buf++)) + fra2 * ((UINT32)(*ver_bilinear_buf++));
					if(ANG_90_A == g_imagedec_angle[0])
					{
						*addr_d++ = (UINT8)((out>>8)&0xFF);		
						out = fra * ((UINT32)(*ver_part_sum_buf++)) + fra2 * ((UINT32)(*ver_bilinear_buf++));						
						*addr_d = (UINT8)((out>>8)&0xFF);	
						addr_d += 15;
						if(0xF == (y_d&0xF))
							addr_d += ads_cnt;
						y_d++;
					}
					else
					{
						*addr_d++ = (UINT8)((out>>8)&0xFF);		
						out = fra * ((UINT32)(*ver_part_sum_buf++)) + fra2 * ((UINT32)(*ver_bilinear_buf++));						
						*addr_d = (UINT8)((out>>8)&0xFF);	
						addr_d -= 17;
						if(0 == (y_d&0xF))
							addr_d -= ads_cnt;
						y_d--;
					}
					h_d -= 2;
				}
			//	JHCON_PRINTF("c line <%d>\n",y_d);
				ver_fra_counter += g_jhconvs_scale_ver_fra_c;
				if(ANG_90_A == g_imagedec_angle[0])
					x_d--;
				else
					x_d++;
			}
			jh_mem_cpy((void *)g_jhconvs_scale_bilinear_buf,(void *)g_jhconvs_scale_partsum_buf,g_jhconvs_buf_size);
			jh_mem_set((void *)g_jhconvs_scale_partsum_buf,0,g_jhconvs_buf_size);
		//	ver_part_sum_cnt = 0;
		}		
		h_s--;y_s++;
	}	
}
/*this scale is designed for thumbnail mode*/
JHCONVS_INLINE void jhcovs_m3329e_y_thum(struct jhconvs_main *main)
{
	UINT32 x_s = main->src_info.rect.x;
	UINT32 y_s = main->src_info.rect.y;
	UINT32 w_s = main->src_info.rect.w;
	UINT32 h_s = main->src_info.rect.h;
	UINT32 pit_s = main->src_info.stride;
	UINT32 x_d = main->dst_info.rect.x;
	UINT32 y_d = main->dst_info.rect.y;
	UINT32 w_d = main->dst_info.rect.w;
	UINT32 h_d = main->dst_info.rect.h;
	UINT32 pit_d = main->dst_info.stride;	
	UINT32 ver_accumulator = 0;
	UINT32 hor_part_sum = 0,hor_bilinear = 0;
	UINT32 hor_part_sum_cnt = 0,hor_int_cnt = 0;
	UINT32 hor_fra_counter = g_jhconvs_scale_hor_fra;
	UINT32 ver_part_sum_cnt = 0,ver_int_cnt = 0,ver_fra_counter =  g_jhconvs_scale_ver_fra,fra = 0;
	UINT32 fra2 = 0,out = 0;
	UINT32 *ver_part_sum_buf = 0,*ver_bilinear_buf = 0;
	UINT8 *addr_s = 0,*addr_d = 0;

	if(m_jhconvs_ver_no_scale)
	{
		ver_int_cnt = 1;
		ver_fra_counter = 0;
	}
	
	while(h_s)
	{
		w_s = main->src_info.rect.w;
		x_s = main->src_info.rect.x;
		ver_part_sum_buf = (UINT32 *)(g_jhconvs_scale_partsum_buf);
		addr_s = (UINT8 *)(main->src_info.y_addr+ (y_s & 0xFFF0) * pit_s + ((y_s & 0x0F) <<4) + \
			((x_s & 0xFFF0) <<4) + (x_s & 0x0F));
		ver_accumulator = 0;
		if(m_jhconvs_hor_no_scale)
		{
			hor_int_cnt = 1;
			hor_fra_counter = 0;
		}
		else
		{
			hor_int_cnt = 0;
			hor_fra_counter = g_jhconvs_scale_hor_fra;
		}
		
		hor_part_sum_cnt = hor_part_sum = 0;
		while(w_s)
		{
			hor_part_sum_cnt++;
			if(m_jhconvs_hor_no_scale)
				hor_part_sum = (UINT32)(*addr_s++);
			else
				hor_part_sum += (UINT32)(*addr_s++);
			if(hor_part_sum_cnt >= g_jhconvs_scale_hor_int)
			{
				hor_int_cnt++;
				if(hor_int_cnt > (hor_fra_counter>>8))
				{
					fra = hor_fra_counter&0xFF;
					fra2 = 0x100 - fra;
					if(m_jhconvs_hor_no_scale)
						*ver_part_sum_buf++ = hor_part_sum;
					else
						*ver_part_sum_buf++ = hor_part_sum*fra + hor_bilinear*fra2;
					hor_fra_counter += g_jhconvs_scale_hor_fra;
					ver_accumulator++;
				}
				hor_bilinear = hor_part_sum;
				hor_part_sum = 0;
				hor_part_sum_cnt = 0;
			}
			if(0xF == (x_s&0xF))
				addr_s += 240;
			w_s--;x_s++;
		}
		//JHCON_PRINTF("ver accu <%d>\n",ver_accumulator);
		w_d = main->dst_info.rect.w;
		x_d = main->dst_info.rect.x;
		ver_part_sum_buf = (UINT32 *)(g_jhconvs_scale_partsum_buf);
		ver_bilinear_buf = (UINT32 *)(g_jhconvs_scale_bilinear_buf);
		ver_part_sum_cnt++;
		if(ver_part_sum_cnt >= g_jhconvs_scale_ver_int)
		{
			ver_int_cnt++;
			if(ver_int_cnt > (ver_fra_counter>>8))
			{
				addr_d = (UINT8 *)(main->dst_info.y_addr + (y_d & 0xFFF0) * pit_d + ((y_d & 0x0F) <<4) + \
					((x_d & 0xFFF0) <<4) + (x_d & 0x0F));	
				fra = ver_fra_counter&0xFF;
				fra2 = 0xFF - fra;
				while(ver_accumulator)
				{
					if(m_jhconvs_ver_no_scale)
						out = (UINT32)*ver_part_sum_buf++;						
					else
					{
						out = fra * ((UINT32)(*ver_part_sum_buf++)) + fra2 * ((UINT32)(*ver_bilinear_buf++));
						out >>= 16;
						out *= g_jhconvs_scale_factor_inverse;
						out >>= 15;
					}

					if(ANG_180 == g_imagedec_angle[0])
					{
						*addr_d-- = (UINT8)(out&0xFF);		
						if(0 == (x_d&0xF))
							addr_d -= 240;
						x_d--;
					}
					else
					{
						*addr_d++ = (UINT8)(out&0xFF);
						if(0xF == (x_d&0xF))
							addr_d += 240;
						x_d++;
					}
					
					ver_accumulator--;
				}
				//JHCON_PRINTF("y line <%d>\n",y_d);
				ver_fra_counter += g_jhconvs_scale_ver_fra;
				
				if(ANG_180 == g_imagedec_angle[0])
					y_d--;
				else
					y_d++;
				
			}
			jh_mem_cpy((void *)g_jhconvs_scale_bilinear_buf,(void *)g_jhconvs_scale_partsum_buf,g_jhconvs_buf_size<<2);
			jh_mem_set((void *)g_jhconvs_scale_partsum_buf,0,g_jhconvs_buf_size<<2);
			ver_part_sum_cnt = 0;
		}		
		h_s--;y_s++;
	}
}

JHCONVS_INLINE void jhcovs_m3329e_y_thum_90(struct jhconvs_main *main)
{
	UINT32 x_s = main->src_info.rect.x;
	UINT32 y_s = main->src_info.rect.y;
	UINT32 w_s = main->src_info.rect.w;
	UINT32 h_s = main->src_info.rect.h;
	UINT32 pit_s = main->src_info.stride;
	UINT32 x_d = main->dst_info.rect.x;
	UINT32 y_d = main->dst_info.rect.y;
	UINT32 w_d = main->dst_info.rect.w;
	UINT32 h_d = main->dst_info.rect.h;
	UINT32 pit_d = main->dst_info.stride;	
	UINT32 ver_accumulator = 0;
	UINT32 hor_part_sum = 0,hor_bilinear = 0;
	UINT32 hor_part_sum_cnt = 0,hor_int_cnt = 0;
	UINT32 hor_fra_counter = g_jhconvs_scale_hor_fra;
	UINT32 ver_part_sum_cnt = 0,ver_int_cnt = 0,ver_fra_counter =  g_jhconvs_scale_ver_fra,fra = 0;
	UINT32 fra2 = 0,out = 0;
	UINT32 *ver_part_sum_buf = 0,*ver_bilinear_buf = 0;
	UINT8 *addr_s = 0,*addr_d = 0;
	UINT32 ads_cnt = (pit_d<<4) - 256;	

	while(h_s)
	{
		w_s = main->src_info.rect.w;
		x_s = main->src_info.rect.x;
		ver_part_sum_buf = (UINT32 *)(g_jhconvs_scale_partsum_buf);
		addr_s = (UINT8 *)(main->src_info.y_addr+ (y_s & 0xFFF0) * pit_s + ((y_s & 0x0F) <<4) + \
			((x_s & 0xFFF0) <<4) + (x_s & 0x0F));
		ver_accumulator = 0;
		hor_int_cnt = 0;
		hor_fra_counter = g_jhconvs_scale_hor_fra;
		hor_part_sum_cnt = hor_part_sum = 0;
		while(w_s)
		{
			hor_part_sum_cnt++;
			hor_part_sum += (UINT32)(*addr_s++);
			if(hor_part_sum_cnt >= g_jhconvs_scale_hor_int)
			{
				hor_int_cnt++;
				if(hor_int_cnt > (hor_fra_counter>>8))
				{
					fra = hor_fra_counter&0xFF;
					fra2 = 0x100 - fra;
					*ver_part_sum_buf++ = hor_part_sum*fra + hor_bilinear*fra2;
					hor_fra_counter += g_jhconvs_scale_hor_fra;
					ver_accumulator++;
				}
				hor_bilinear = hor_part_sum;
				hor_part_sum = 0;
				hor_part_sum_cnt = 0;
			}
			if(0xF == (x_s&0xF))
				addr_s += 240;
			w_s--;x_s++;
		}
		//JHCON_PRINTF("ver accu <%d>\n",ver_accumulator);
		h_d = main->dst_info.rect.h;
		y_d = main->dst_info.rect.y;
		ver_part_sum_buf = (UINT32 *)(g_jhconvs_scale_partsum_buf);
		ver_bilinear_buf = (UINT32 *)(g_jhconvs_scale_bilinear_buf);
		ver_part_sum_cnt++;
		if(ver_part_sum_cnt >= g_jhconvs_scale_ver_int)
		{
			ver_int_cnt++;
			if(ver_int_cnt > (ver_fra_counter>>8))
			{
				addr_d = (UINT8 *)(main->dst_info.y_addr + (y_d & 0xFFF0) * pit_d + ((y_d & 0x0F) <<4) + \
					((x_d & 0xFFF0) <<4) + (x_d & 0x0F));	
				fra = ver_fra_counter&0xFF;
				fra2 = 0xFF - fra;
				while(ver_accumulator)
				{
					out = fra * ((UINT32)(*ver_part_sum_buf++)) + fra2 * ((UINT32)(*ver_bilinear_buf++));
					out >>= 16;
					out *= g_jhconvs_scale_factor_inverse;
					out >>= 15;

					*addr_d = (UINT8)(out&0xFF);
					if(ANG_90_A == g_imagedec_angle[0])
					{
						addr_d += 16;
						if(0xF == (y_d&0xF))
							addr_d += ads_cnt;
						y_d++;
					}
					else
					{
						addr_d -= 16;
						if(0 == (y_d&0xF))
							addr_d -= ads_cnt;					
						y_d--;
					}
					
					ver_accumulator--;
				}
				//JHCON_PRINTF("y line <%d>\n",y_d);
				ver_fra_counter += g_jhconvs_scale_ver_fra;
				
				if(ANG_90_A == g_imagedec_angle[0])
					x_d--;
				else
					x_d++;
			}
			jh_mem_cpy((void *)g_jhconvs_scale_bilinear_buf,(void *)g_jhconvs_scale_partsum_buf,g_jhconvs_buf_size<<2);
			jh_mem_set((void *)g_jhconvs_scale_partsum_buf,0,g_jhconvs_buf_size<<2);
			ver_part_sum_cnt = 0;
		}		
		h_s--;y_s++;
	}
}

JHCONVS_INLINE void jhcovs_m3329e_c_420_thum(struct jhconvs_main *main)
{
	UINT32 x_s = (main->src_info.rect.x>>1)&(~0x01);
	UINT32 y_s = main->src_info.rect.y>>1;
	UINT32 w_s = main->src_info.rect.w&(~0x01);
	UINT32 h_s = main->src_info.rect.h>>1;
	UINT32 pit_s = main->src_info.stride;
	UINT32 x_d = main->dst_info.rect.x>>1;
	UINT32 y_d = main->dst_info.rect.y>>1;
	UINT32 w_d = main->dst_info.rect.w&(~0x01);
	UINT32 h_d = main->dst_info.rect.h>>1;
	UINT32 pit_d = main->dst_info.stride;	
	UINT32 ver_accumulator = 0;
	UINT32 hor_part_sum = 0,hor_bilinear = 0,hor_out = 0;
	UINT32 hor_part_sum_c = 0,hor_bilinear_c = 0;
	UINT32 hor_part_sum_cnt = 0,hor_int_cnt = 0;
	UINT32 hor_fra_counter = g_jhconvs_scale_hor_fra_c;
	UINT32 ver_part_sum_cnt = 0,ver_int_cnt = 0,ver_fra_counter = g_jhconvs_scale_ver_fra_c,fra = 0,fra2 = 0,out = 0;
	UINT8 *addr_s = 0,*addr_d = 0;
	UINT32 *ver_part_sum_buf = 0,*ver_bilinear_buf = 0;

	if(m_jhconvs_ver_no_scale)
	{
		ver_int_cnt = 1;
		ver_fra_counter = 0;
	}
	
	while(h_s)
	{
		w_s = main->src_info.rect.w&(~0x01);
		x_s = main->src_info.rect.x>>1;
		ver_part_sum_buf = (UINT32 *)(g_jhconvs_scale_partsum_buf);
		ver_bilinear_buf = (UINT32 *)(g_jhconvs_scale_bilinear_buf);		
		addr_s = (UINT8 *)(main->src_info.c_addr +  (y_s & 0xFFF0) * pit_s + \
			((y_s & 0x0F) <<4) + ((x_s & 0xFFF8) <<5) + ((x_s & 7) <<1));
		x_s <<= 1;
#if 1		
		ver_accumulator = 0;
		if(m_jhconvs_hor_no_scale)
		{
			hor_int_cnt = 1;
			hor_fra_counter = 0;
		}
		else
		{
			hor_int_cnt = 0;
			hor_fra_counter = g_jhconvs_scale_hor_fra_c;
		}
		hor_part_sum_cnt = hor_part_sum = hor_part_sum_c = 0;
		hor_bilinear = hor_bilinear_c = 0;
#endif		
		while(w_s)
		{
			hor_part_sum_cnt++;
			if(m_jhconvs_hor_no_scale)
			{
				hor_bilinear = hor_part_sum = (UINT32)(*addr_s++);
				hor_bilinear_c = hor_part_sum_c = (UINT32)(*addr_s++);
			}				
			else
			{
				hor_part_sum += (UINT32)(*addr_s++);
				hor_part_sum_c += (UINT32)(*addr_s++);
			}
			if(hor_part_sum_cnt >= g_jhconvs_scale_hor_int_c)
			{
				hor_int_cnt++;
				if(hor_int_cnt > (hor_fra_counter>>8))
				{
					fra = hor_fra_counter&0xFF;
					fra2 = 0x100 - fra;
					
					if(m_jhconvs_hor_no_scale)
					{
						*ver_part_sum_buf++ = hor_part_sum;
						*ver_part_sum_buf++ = hor_part_sum_c;
					}
					else
					{
						*ver_part_sum_buf++ = hor_part_sum*fra + hor_bilinear*fra2;
						*ver_part_sum_buf++ = hor_part_sum_c*fra + hor_bilinear_c*fra2;	
					}

					if(m_jhconvs_ver_no_scale)
					{
						*ver_bilinear_buf++ = *(ver_part_sum_buf - 2); 
						*ver_bilinear_buf++ = *(ver_part_sum_buf - 1);
					}
					
					hor_fra_counter += g_jhconvs_scale_hor_fra_c;
					ver_accumulator += 2;
				}
				hor_bilinear = hor_part_sum;
				hor_bilinear_c = hor_part_sum_c;
				hor_part_sum = 0;
				hor_part_sum_c = 0;
				hor_part_sum_cnt = 0;
			}
			if(0xE == (x_s&0xF))
				addr_s += 240;
			w_s -= 2;
			x_s += 2;
		}
		w_d = ver_accumulator;// main->dst_info.rect.w&(~0x01);
		x_d = main->dst_info.rect.x>>1;
		ver_part_sum_buf = (UINT32 *)(g_jhconvs_scale_partsum_buf);
		ver_bilinear_buf = (UINT32 *)(g_jhconvs_scale_bilinear_buf);
		ver_part_sum_cnt++;
		if(ver_part_sum_cnt >= g_jhconvs_scale_ver_int)
		{
			ver_int_cnt++;
			if(ver_int_cnt > (ver_fra_counter>>8))
			{
				addr_d = (UINT8 *)(main->dst_info.c_addr +  (y_d & 0xFFF0) * pit_d + \
					((y_d & 0x0F) <<4) + ((x_d & 0xFFF8) <<5) + ((x_d & 7) <<1));		
				fra = ver_fra_counter&0xFF;
				fra2 = 0xFF - fra;
				x_d <<= 1;
				while(w_d)
				{
					if(m_jhconvs_ver_no_scale)
						out = (UINT32)*ver_part_sum_buf++;		
					else
					{
						out = fra * ((UINT32)(*ver_part_sum_buf++)) + fra2 * ((UINT32)(*ver_bilinear_buf++));
						out >>= 16;
						out *= g_jhconvs_scale_factor_inverse_c;
						out >>= 15;
					}
					
					if(ANG_180 == g_imagedec_angle[0])
					{
						*addr_d-- = (UINT8)(out&0xFF);		
						if(0 == (x_d&0xF))
							addr_d -= 240;
						x_d--;
					}
					else
					{
						*addr_d++ = (UINT8)(out&0xFF);
						if(0xF == (x_d&0xF))
							addr_d += 240;
						x_d++;
					}
					w_d--;
				}
				ver_fra_counter += g_jhconvs_scale_ver_fra_c;
				if(ANG_180 == g_imagedec_angle[0])
					y_d--;
				else
					y_d++;
			}
			jh_mem_cpy((void *)g_jhconvs_scale_bilinear_buf,(void *)g_jhconvs_scale_partsum_buf,g_jhconvs_buf_size<<2);
			jh_mem_set((void *)g_jhconvs_scale_partsum_buf,0,g_jhconvs_buf_size<<2);
			ver_part_sum_cnt = 0;
		}		
		h_s--;y_s++;
	}
}

JHCONVS_INLINE void jhcovs_m3329e_c_420_thum_90(struct jhconvs_main *main)
{
	UINT32 x_s = (main->src_info.rect.x>>1)&(~0x01);
	UINT32 y_s = main->src_info.rect.y>>1;
	UINT32 w_s = main->src_info.rect.w&(~0x01);
	UINT32 h_s = main->src_info.rect.h>>1;
	UINT32 pit_s = main->src_info.stride;
	UINT32 x_d = main->dst_info.rect.x>>1;
	UINT32 y_d = main->dst_info.rect.y>>1;
	UINT32 w_d = main->dst_info.rect.w&(~0x01);
	UINT32 h_d = main->dst_info.rect.h>>1;
	UINT32 pit_d = main->dst_info.stride;	
	UINT32 ver_accumulator = 0;
	UINT32 hor_part_sum = 0,hor_bilinear = 0,hor_out = 0;
	UINT32 hor_part_sum_c = 0,hor_bilinear_c = 0;
	UINT32 hor_part_sum_cnt = 0,hor_int_cnt = 0;
	UINT32 hor_fra_counter = g_jhconvs_scale_hor_fra_c;
	UINT32 ver_part_sum_cnt = 0,ver_int_cnt = 0,ver_fra_counter = g_jhconvs_scale_ver_fra_c,fra = 0,fra2 = 0,out = 0;
	UINT8 *addr_s = 0,*addr_d = 0;
	UINT32 *ver_part_sum_buf = 0,*ver_bilinear_buf = 0;
	UINT32 ads_cnt = (pit_d<<4) - 256;	

	while(h_s)
	{
		w_s = main->src_info.rect.w&(~0x01);
		x_s = main->src_info.rect.x>>1;
		ver_part_sum_buf = (UINT32 *)(g_jhconvs_scale_partsum_buf);
		addr_s = (UINT8 *)(main->src_info.c_addr +  (y_s & 0xFFF0) * pit_s + \
			((y_s & 0x0F) <<4) + ((x_s & 0xFFF8) <<5) + ((x_s & 7) <<1));
		x_s <<= 1;
#if 0		
		ver_accumulator = 0;
		hor_int_cnt = 0;
		hor_fra_counter = g_jhconvs_scale_hor_fra_c;
		hor_part_sum_cnt = hor_part_sum = hor_part_sum_c = 0;
#endif		
		while(w_s)
		{
			hor_part_sum_cnt++;
			hor_part_sum += (UINT32)(*addr_s++);
			hor_part_sum_c += (UINT32)(*addr_s++);
			if(hor_part_sum_cnt >= g_jhconvs_scale_hor_int_c)
			{
				hor_int_cnt++;
				if(hor_int_cnt > (hor_fra_counter>>8))
				{
					fra = hor_fra_counter&0xFF;
					fra2 = 0x100 - fra;
					*ver_part_sum_buf++ = hor_part_sum*fra + hor_bilinear*fra2;
					*ver_part_sum_buf++ = hor_part_sum_c*fra + hor_bilinear_c*fra2;					
					hor_fra_counter += g_jhconvs_scale_hor_fra_c;
					ver_accumulator += 2;
				}
				hor_bilinear = hor_part_sum;
				hor_bilinear_c = hor_part_sum_c;
				hor_part_sum = 0;
				hor_part_sum_c = 0;
				hor_part_sum_cnt = 0;
			}
			if(0xE == (x_s&0xF))
				addr_s += 240;
			w_s -= 2;
			x_s += 2;
		}
		h_d = main->dst_info.rect.h&(~0x01);
		y_d = main->dst_info.rect.y>>1;
		ver_part_sum_buf = (UINT32 *)(g_jhconvs_scale_partsum_buf);
		ver_bilinear_buf = (UINT32 *)(g_jhconvs_scale_bilinear_buf);
		ver_part_sum_cnt++;
		if(ver_part_sum_cnt >= g_jhconvs_scale_ver_int)
		{
			ver_int_cnt++;
			if(ver_int_cnt > (ver_fra_counter>>8))
			{
				addr_d = (UINT8 *)(main->dst_info.c_addr +  (y_d & 0xFFF0) * pit_d + \
					((y_d & 0x0F) <<4) + ((x_d & 0xFFF8) <<5) + ((x_d & 7) <<1));		
				fra = ver_fra_counter&0xFF;
				fra2 = 0xFF - fra;
				
				while(h_d)
				{
					out = fra * ((UINT32)(*ver_part_sum_buf++)) + fra2 * ((UINT32)(*ver_bilinear_buf++));
					out >>= 16;
					out *= g_jhconvs_scale_factor_inverse_c;
					out >>= 15;
					
					if(ANG_90_A == g_imagedec_angle[0])
					{
						*addr_d++ = (UINT8)(out&0xFF);		
						out = fra * ((UINT32)(*ver_part_sum_buf++)) + fra2 * ((UINT32)(*ver_bilinear_buf++));	
						out >>= 16;
						out *= g_jhconvs_scale_factor_inverse_c;
						out >>= 15;						
						*addr_d = (UINT8)(out&0xFF);	
						addr_d += 15;
						if(0xF == (y_d&0xF))
							addr_d += ads_cnt;
						y_d++;
					}
					else
					{
						*addr_d++ = (UINT8)(out&0xFF);		
						out = fra * ((UINT32)(*ver_part_sum_buf++)) + fra2 * ((UINT32)(*ver_bilinear_buf++));	
						out >>= 16;
						out *= g_jhconvs_scale_factor_inverse_c;
						out >>= 15;						
						*addr_d = (UINT8)(out&0xFF);	
						addr_d -= 17;
						if(0 == (y_d&0xF))
							addr_d -= ads_cnt;
						y_d--;
					}
					h_d -= 2;
				}
				ver_fra_counter += g_jhconvs_scale_ver_fra_c;
				
				if(ANG_90_A == g_imagedec_angle[0])
					x_d--;
				else
					x_d++;
			}
			jh_mem_cpy((void *)g_jhconvs_scale_bilinear_buf,(void *)g_jhconvs_scale_partsum_buf,g_jhconvs_buf_size<<2);
			jh_mem_set((void *)g_jhconvs_scale_partsum_buf,0,g_jhconvs_buf_size<<2);
			ver_part_sum_cnt = 0;
		}		
		h_s--;y_s++;
	}
}

JHCONVS_INLINE void jhcovs_m3329e_c_422_thum(struct jhconvs_main *main)
{
	UINT32 x_s = main->src_info.rect.x&(~0x01);
	UINT32 y_s = main->src_info.rect.y&(~0x01);
	UINT32 w_s = main->src_info.rect.w&(~0x01);
	UINT32 h_s = main->src_info.rect.h&(~0x01);
	UINT32 pit_s = main->src_info.stride;
	UINT32 x_d = main->dst_info.rect.x>>1;
	UINT32 y_d = main->dst_info.rect.y>>1;
	UINT32 w_d = main->dst_info.rect.w&(~0x01);
	UINT32 h_d = main->dst_info.rect.h>>1;
	UINT32 pit_d = main->dst_info.stride;	
	UINT32 ver_accumulator = 0;
	UINT32 hor_part_sum = 0,hor_bilinear = 0,hor_out = 0;
	UINT32 hor_part_sum_c = 0,hor_bilinear_c = 0;
	UINT32 hor_part_sum_cnt = 0,hor_int_cnt = 0;
	UINT32 hor_fra_counter = g_jhconvs_scale_hor_fra_c;
	UINT32 ver_part_sum_cnt = 0,ver_int_cnt = 0,ver_fra_counter =  g_jhconvs_scale_ver_fra_c,fra = 0,fra2 = 0,out = 0;
	UINT8 *addr_s = 0,*addr_d = 0;
	UINT32 *ver_part_sum_buf = 0,*ver_bilinear_buf = 0;

	if(m_jhconvs_ver_no_scale)
	{
		ver_int_cnt = 1;
		ver_fra_counter = 0;
	}
	
	while(h_s)
	{
		w_s = main->src_info.rect.w&(~0x01);
		x_s = main->src_info.rect.x&(~0x01);
		ver_part_sum_buf = (UINT32 *)(g_jhconvs_scale_partsum_buf);
		ver_bilinear_buf = (UINT32 *)(g_jhconvs_scale_bilinear_buf);			
		addr_s = (UINT8 *)(main->src_info.c_addr+ (y_s & 0xFFF0) * pit_s + ((y_s & 0x0F) <<4) + \
			((x_s & 0xFFF0) <<4) + (x_s & 0x0F));
#if 1		
		ver_accumulator = 0;
		if(m_jhconvs_hor_no_scale)
		{
			hor_int_cnt = 1;
			hor_fra_counter = 0;
		}
		else
		{
			hor_int_cnt = 0;
			hor_fra_counter = g_jhconvs_scale_hor_fra_c;
		}
		hor_part_sum_cnt = hor_part_sum = hor_part_sum_c = 0;
		hor_bilinear = hor_bilinear_c = 0;		
#endif		
		while(w_s)
		{
			hor_part_sum_cnt++;
			if(m_jhconvs_hor_no_scale)
			{
				hor_bilinear = hor_part_sum = (UINT32)(*addr_s++);
				hor_bilinear_c = hor_part_sum_c = (UINT32)(*addr_s++);			
			}
			else
			{
				hor_part_sum += (UINT32)(*addr_s++);
				hor_part_sum_c += (UINT32)(*addr_s++);
			}
			if(hor_part_sum_cnt >= g_jhconvs_scale_hor_int_c)
			{
				hor_int_cnt++;
				if(hor_int_cnt > (hor_fra_counter>>8))
				{
					fra = hor_fra_counter&0xFF;
					fra2 = 0x100 - fra;
					
					if(m_jhconvs_hor_no_scale)
					{
						*ver_part_sum_buf++ = hor_part_sum;
						*ver_part_sum_buf++ = hor_part_sum_c;
					}
					else
					{					
						*ver_part_sum_buf++ = hor_part_sum*fra + hor_bilinear*fra2;
						*ver_part_sum_buf++ = hor_part_sum_c*fra + hor_bilinear_c*fra2;		
					}

					if(m_jhconvs_ver_no_scale)
					{
						*ver_bilinear_buf++ = *(ver_part_sum_buf - 2); 
						*ver_bilinear_buf++ = *(ver_part_sum_buf - 1);
					}
					
					hor_fra_counter += g_jhconvs_scale_hor_fra_c;
					ver_accumulator += 2;
				}
				hor_bilinear = hor_part_sum;
				hor_bilinear_c = hor_part_sum_c;
				hor_part_sum = 0;
				hor_part_sum_c = 0;
				hor_part_sum_cnt = 0;
			}
			if(0xE == (x_s&0xF))
				addr_s += 240;
			w_s -= 2;
			x_s += 2;
		}
		w_d = ver_accumulator;//main->dst_info.rect.w&(~0x01);
		x_d = main->dst_info.rect.x>>1;
		ver_part_sum_buf = (UINT32 *)(g_jhconvs_scale_partsum_buf);
		ver_bilinear_buf = (UINT32 *)(g_jhconvs_scale_bilinear_buf);
		ver_part_sum_cnt++;
		if(ver_part_sum_cnt >= g_jhconvs_scale_ver_int)
		{
			ver_int_cnt++;
			if(ver_int_cnt > (ver_fra_counter>>8))
			{
				addr_d = (UINT8 *)(main->dst_info.c_addr +  (y_d & 0xFFF0) * pit_d + \
					((y_d & 0x0F) <<4) + ((x_d & 0xFFF8) <<5) + ((x_d & 7) <<1));	
				fra = ver_fra_counter&0xFF;
				fra2 = 0xFF - fra;
				x_d <<= 1;
				while(w_d)
				{
					if(m_jhconvs_ver_no_scale)
						out = (UINT32)*ver_part_sum_buf++;		
					else
					{					
						out = fra * ((UINT32)(*ver_part_sum_buf++)) + fra2 * ((UINT32)(*ver_bilinear_buf++));
						out >>= 16;
						out *= g_jhconvs_scale_factor_inverse_c;
						out >>= 15;
					}
					
					if(ANG_180 == g_imagedec_angle[0])
					{
						*addr_d-- = (UINT8)(out&0xFF);		
						if(0 == (x_d&0xF))
							addr_d -= 240;
						x_d--;
					}
					else
					{
						*addr_d++ = (UINT8)(out&0xFF);
						if(0xF == (x_d&0xF))
							addr_d += 240;
						x_d++;
					}		
					
					w_d--;
				}
			//	JHCON_PRINTF("c line <%d>\n",y_d);
				ver_fra_counter += g_jhconvs_scale_ver_fra_c;
			
				if(ANG_180 == g_imagedec_angle[0])
					y_d--;
				else
					y_d++;
				
			}
			jh_mem_cpy((void *)g_jhconvs_scale_bilinear_buf,(void *)g_jhconvs_scale_partsum_buf,g_jhconvs_buf_size<<2);
			jh_mem_set((void *)g_jhconvs_scale_partsum_buf,0,g_jhconvs_buf_size<<2);
			ver_part_sum_cnt = 0;
		}		
		h_s--;y_s++;
	}
}

JHCONVS_INLINE void jhcovs_m3329e_c_422_thum_90(struct jhconvs_main *main)
{
	UINT32 x_s = main->src_info.rect.x&(~0x01);
	UINT32 y_s = main->src_info.rect.y&(~0x01);
	UINT32 w_s = main->src_info.rect.w&(~0x01);
	UINT32 h_s = main->src_info.rect.h&(~0x01);
	UINT32 pit_s = main->src_info.stride;
	UINT32 x_d = main->dst_info.rect.x>>1;
	UINT32 y_d = main->dst_info.rect.y>>1;
	UINT32 w_d = main->dst_info.rect.w&(~0x01);
	UINT32 h_d = main->dst_info.rect.h>>1;
	UINT32 pit_d = main->dst_info.stride;	
	UINT32 ver_accumulator = 0;
	UINT32 hor_part_sum = 0,hor_bilinear = 0,hor_out = 0;
	UINT32 hor_part_sum_c = 0,hor_bilinear_c = 0;
	UINT32 hor_part_sum_cnt = 0,hor_int_cnt = 0;
	UINT32 hor_fra_counter = g_jhconvs_scale_hor_fra_c;
	UINT32 ver_part_sum_cnt = 0,ver_int_cnt = 0,ver_fra_counter =  g_jhconvs_scale_ver_fra_c,fra = 0,fra2 = 0,out = 0;
	UINT8 *addr_s = 0,*addr_d = 0;
	UINT32 *ver_part_sum_buf = 0,*ver_bilinear_buf = 0;
	UINT32 ads_cnt = (pit_d<<4) - 256;	

	while(h_s)
	{
		w_s = main->src_info.rect.w&(~0x01);
		x_s = main->src_info.rect.x&(~0x01);
		ver_part_sum_buf = (UINT32 *)(g_jhconvs_scale_partsum_buf);
		addr_s = (UINT8 *)(main->src_info.c_addr+ (y_s & 0xFFF0) * pit_s + ((y_s & 0x0F) <<4) + \
			((x_s & 0xFFF0) <<4) + (x_s & 0x0F));
#if 0		
		ver_accumulator = 0;
		hor_int_cnt = 0;
		hor_fra_counter = g_jhconvs_scale_hor_fra_c;
		hor_part_sum_cnt = hor_part_sum = hor_part_sum_c = 0;
#endif		
		while(w_s)
		{
			hor_part_sum_cnt++;
			hor_part_sum += (UINT32)(*addr_s++);
			hor_part_sum_c += (UINT32)(*addr_s++);
			if(hor_part_sum_cnt >= g_jhconvs_scale_hor_int_c)
			{
				hor_int_cnt++;
				if(hor_int_cnt > (hor_fra_counter>>8))
				{
					fra = hor_fra_counter&0xFF;
					fra2 = 0x100 - fra;
					*ver_part_sum_buf++ = hor_part_sum*fra + hor_bilinear*fra2;
					*ver_part_sum_buf++ = hor_part_sum_c*fra + hor_bilinear_c*fra2;					
					hor_fra_counter += g_jhconvs_scale_hor_fra_c;
					ver_accumulator += 2;
				}
				hor_bilinear = hor_part_sum;
				hor_bilinear_c = hor_part_sum_c;
				hor_part_sum = 0;
				hor_part_sum_c = 0;
				hor_part_sum_cnt = 0;
			}
			if(0xE == (x_s&0xF))
				addr_s += 240;
			w_s -= 2;
			x_s += 2;
		}
		h_d = main->dst_info.rect.h&(~0x01);
		y_d = main->dst_info.rect.y>>1;
		ver_part_sum_buf = (UINT32 *)(g_jhconvs_scale_partsum_buf);
		ver_bilinear_buf = (UINT32 *)(g_jhconvs_scale_bilinear_buf);
		ver_part_sum_cnt++;
		if(ver_part_sum_cnt >= g_jhconvs_scale_ver_int)
		{
			ver_int_cnt++;
			if(ver_int_cnt > (ver_fra_counter>>8))
			{
				addr_d = (UINT8 *)(main->dst_info.c_addr +  (y_d & 0xFFF0) * pit_d + \
					((y_d & 0x0F) <<4) + ((x_d & 0xFFF8) <<5) + ((x_d & 7) <<1));	
				fra = ver_fra_counter&0xFF;
				fra2 = 0xFF - fra;
				while(h_d)
				{
					out = fra * ((UINT32)(*ver_part_sum_buf++)) + fra2 * ((UINT32)(*ver_bilinear_buf++));
					out >>= 16;
					out *= g_jhconvs_scale_factor_inverse_c;
					out >>= 15;
					
					if(ANG_90_A == g_imagedec_angle[0])
					{
						*addr_d++ = (UINT8)(out&0xFF);		
						out = fra * ((UINT32)(*ver_part_sum_buf++)) + fra2 * ((UINT32)(*ver_bilinear_buf++));
						out >>= 16;
						out *= g_jhconvs_scale_factor_inverse_c;
						out >>= 15;					
						*addr_d = (UINT8)(out&0xFF);	
						addr_d += 15;
						if(0xF == (y_d&0xF))
							addr_d += ads_cnt;
						y_d++;
					}
					else
					{
						*addr_d++ = (UINT8)(out&0xFF);		
						out = fra * ((UINT32)(*ver_part_sum_buf++)) + fra2 * ((UINT32)(*ver_bilinear_buf++));	
						out >>= 16;
						out *= g_jhconvs_scale_factor_inverse_c;
						out >>= 15;						
						*addr_d = (UINT8)(out&0xFF);	
						addr_d -= 17;
						if(0 == (y_d&0xF))
							addr_d -= ads_cnt;
						y_d--;
					}
					h_d -= 2;
				}
			//	JHCON_PRINTF("c line <%d>\n",y_d);
				ver_fra_counter += g_jhconvs_scale_ver_fra_c;
			
				if(ANG_90_A == g_imagedec_angle[0])
					x_d--;
				else
					x_d++;
				
			}
			jh_mem_cpy((void *)g_jhconvs_scale_bilinear_buf,(void *)g_jhconvs_scale_partsum_buf,g_jhconvs_buf_size<<2);
			jh_mem_set((void *)g_jhconvs_scale_partsum_buf,0,g_jhconvs_buf_size<<2);
			ver_part_sum_cnt = 0;
		}		
		h_s--;y_s++;
	}
}

JHCONVS_INLINE void jhcovs_m3329e_c_411_thum(struct jhconvs_main *main)
{
	UINT32 x_s = main->src_info.rect.x&(~0x01);
	UINT32 y_s = main->src_info.rect.y&(~0x01);
	UINT32 w_s = main->src_info.rect.w&(~0x01);
	UINT32 h_s = main->src_info.rect.h&(~0x01);
	UINT32 pit_s = (((main->src_info.stride>>4)+1)>>1)<<4;
	UINT32 x_d = main->dst_info.rect.x>>1;
	UINT32 y_d = main->dst_info.rect.y>>1;
	UINT32 w_d = main->dst_info.rect.w&(~0x01);
	UINT32 h_d = main->dst_info.rect.h>>1;
	UINT32 pit_d = main->dst_info.stride;	
	UINT32 ver_accumulator = 0;
	UINT32 hor_part_sum = 0,hor_bilinear = 0,hor_out = 0;
	UINT32 hor_part_sum_c = 0,hor_bilinear_c = 0;
	UINT32 hor_part_sum_cnt = 0,hor_int_cnt = 0;
	UINT32 hor_fra_counter = g_jhconvs_scale_hor_fra_c;
	UINT32 ver_part_sum_cnt = 0,ver_int_cnt = 0,ver_fra_counter =  g_jhconvs_scale_ver_fra_c,fra = 0,fra2 = 0,out = 0;
	UINT8 *addr_s = 0,*addr_d = 0;
	UINT32 *ver_part_sum_buf = NULL,*ver_bilinear_buf = NULL,*ver_part_sum_buf_c = NULL;
	UINT8 cb = 0,cr = 0;

	if(m_jhconvs_ver_no_scale)
	{
		ver_int_cnt = 1;
		ver_fra_counter = 0;
	}

	while(h_s)
	{
		w_s = main->src_info.rect.w>>1;
		x_s = main->src_info.rect.x>>2;
		ver_part_sum_buf_c = (UINT32 *)(g_jhconvs_scale_partsum_buf_c);
		ver_part_sum_buf = (UINT32 *)(g_jhconvs_scale_partsum_buf);
		ver_bilinear_buf = (UINT32 *)(g_jhconvs_scale_bilinear_buf);			
		addr_s = (UINT8 *)(main->src_info.c_addr+ (y_s & 0xFFF0) * pit_s + ((y_s & 0x0F) <<4) + \
			((x_s & 0xFFF8) <<5) + ((x_s & 0x07)<<1));
		x_s <<= 1;
#if 1		
		ver_accumulator = 0;
		if(m_jhconvs_hor_no_scale)
		{
			hor_int_cnt = 1;
			hor_fra_counter = 0;
		}
		else
		{
			hor_int_cnt = 0;
			hor_fra_counter = g_jhconvs_scale_hor_fra_c;
		}
		hor_part_sum_cnt = hor_part_sum = hor_part_sum_c = 0;	
		hor_bilinear = hor_bilinear_c = 0;		
#endif		
		while(w_s)
		{
			if(x_s&0x01)
			{
				cr = *addr_s++;
				*ver_part_sum_buf_c++ = cb;
				*ver_part_sum_buf_c++ = cr;				
				*ver_part_sum_buf_c++ = cb;
				*ver_part_sum_buf_c++ = cr;
			}
			else
			{
				cb = *addr_s++;
			}	
			if(0xF == (x_s&0xF))
				addr_s += 240;
			w_s--;x_s++;
		}
		ver_part_sum_buf_c = (UINT32 *)(g_jhconvs_scale_partsum_buf_c);		
		w_s = main->src_info.rect.w&(~0x01);
		x_s = main->src_info.rect.x&(~0x01);
		while(w_s)
		{
			hor_part_sum_cnt++;
			if(m_jhconvs_hor_no_scale)
			{
				hor_bilinear= hor_part_sum = *ver_part_sum_buf_c++;
				hor_bilinear_c = hor_part_sum_c = *ver_part_sum_buf_c++;
			}
			else
			{
				hor_part_sum += *ver_part_sum_buf_c++;
				hor_part_sum_c += *ver_part_sum_buf_c++;
			}
			if(hor_part_sum_cnt >= g_jhconvs_scale_hor_int_c)
			{
				hor_int_cnt++;
				if(hor_int_cnt > (hor_fra_counter>>8))
				{
					fra = hor_fra_counter&0xFF;
					fra2 = 0x100 - fra;
					if(m_jhconvs_hor_no_scale)
					{
						*ver_part_sum_buf++ = hor_part_sum;
						*ver_part_sum_buf++ = hor_part_sum_c;
					}
					else
					{
						*ver_part_sum_buf++ = hor_part_sum*fra + hor_bilinear*fra2;
						*ver_part_sum_buf++ = hor_part_sum_c*fra + hor_bilinear_c*fra2;
					}

					if(m_jhconvs_ver_no_scale)
					{
						*ver_bilinear_buf++ = *(ver_part_sum_buf - 2); 
						*ver_bilinear_buf++ = *(ver_part_sum_buf - 1);
					}
					
					hor_fra_counter += g_jhconvs_scale_hor_fra_c;
					ver_accumulator += 2;
				}
				hor_bilinear = hor_part_sum;
				hor_bilinear_c = hor_part_sum_c;
				hor_part_sum = 0;
				hor_part_sum_c = 0;
				hor_part_sum_cnt = 0;
			}
			if(0xE == (x_s&0xF))
				addr_s += 240;
			w_s -= 2;
			x_s += 2;
		}	
		w_d = ver_accumulator;// main->dst_info.rect.w&(~0x01);
		x_d = main->dst_info.rect.x>>1;
		ver_part_sum_buf = (UINT32 *)(g_jhconvs_scale_partsum_buf);
		ver_bilinear_buf = (UINT32 *)(g_jhconvs_scale_bilinear_buf);
		ver_part_sum_cnt++;
		if(ver_part_sum_cnt >= g_jhconvs_scale_ver_int)
		{
			ver_int_cnt++;
			if(ver_int_cnt > (ver_fra_counter>>8))
			{
				addr_d = (UINT8 *)(main->dst_info.c_addr +  (y_d & 0xFFF0) * pit_d + \
					((y_d & 0x0F) <<4) + ((x_d & 0xFFF8) <<5) + ((x_d & 7) <<1));	
				fra = ver_fra_counter&0xFF;
				fra2 = 0xFF - fra;
				x_d <<= 1;
				while(w_d)
				{
					if(m_jhconvs_ver_no_scale)
						out = (UINT32)*ver_part_sum_buf++;		
					else
					{
						out = fra * ((UINT32)(*ver_part_sum_buf++)) + fra2 * ((UINT32)(*ver_bilinear_buf++));
						out >>= 16;
						out *= g_jhconvs_scale_factor_inverse_c;
						out >>= 15;
					}
					if(ANG_180 == g_imagedec_angle[0])
					{
						*addr_d-- = (UINT8)(out&0xFF);		
						if(0 == (x_d&0xF))
							addr_d -= 240;
						x_d--;
					}
					else
					{
						*addr_d++ = (UINT8)(out&0xFF);
						if(0xF == (x_d&0xF))
							addr_d += 240;
						x_d++;
					}
					w_d--;
				}
			//	JHCON_PRINTF("c line <%d>\n",y_d);
				ver_fra_counter += g_jhconvs_scale_ver_fra_c;
				if(ANG_180 == g_imagedec_angle[0])
					y_d--;
				else
					y_d++;
			}
			jh_mem_cpy((void *)g_jhconvs_scale_bilinear_buf,(void *)g_jhconvs_scale_partsum_buf,g_jhconvs_buf_size<<2);
			jh_mem_set((void *)g_jhconvs_scale_partsum_buf,0,g_jhconvs_buf_size<<2);
			ver_part_sum_cnt = 0;
		}		
		h_s--;y_s++;
	}
}

JHCONVS_INLINE void jhcovs_m3329e_c_411_thum_90(struct jhconvs_main *main)
{
	UINT32 x_s = main->src_info.rect.x&(~0x01);
	UINT32 y_s = main->src_info.rect.y&(~0x01);
	UINT32 w_s = main->src_info.rect.w&(~0x01);
	UINT32 h_s = main->src_info.rect.h&(~0x01);
	UINT32 pit_s = (((main->src_info.stride>>4)+1)>>1)<<4;
	UINT32 x_d = main->dst_info.rect.x>>1;
	UINT32 y_d = main->dst_info.rect.y>>1;
	UINT32 w_d = main->dst_info.rect.w&(~0x01);
	UINT32 h_d = main->dst_info.rect.h>>1;
	UINT32 pit_d = main->dst_info.stride;	
	UINT32 ver_accumulator = 0;
	UINT32 hor_part_sum = 0,hor_bilinear = 0,hor_out = 0;
	UINT32 hor_part_sum_c = 0,hor_bilinear_c = 0;
	UINT32 hor_part_sum_cnt = 0,hor_int_cnt = 0;
	UINT32 hor_fra_counter = g_jhconvs_scale_hor_fra_c;
	UINT32 ver_part_sum_cnt = 0,ver_int_cnt = 0,ver_fra_counter =  g_jhconvs_scale_ver_fra_c,fra = 0,fra2 = 0,out = 0;
	UINT8 *addr_s = 0,*addr_d = 0;
	UINT32 *ver_part_sum_buf = NULL,*ver_bilinear_buf = NULL,*ver_part_sum_buf_c = NULL;
	UINT8 cb = 0,cr = 0;
	UINT32 ads_cnt = (pit_d<<4) - 256;

	while(h_s)
	{
		w_s = main->src_info.rect.w>>1;
		x_s = main->src_info.rect.x>>2;
		ver_part_sum_buf_c = (UINT32 *)(g_jhconvs_scale_partsum_buf_c);
		ver_part_sum_buf = (UINT32 *)(g_jhconvs_scale_partsum_buf);
		addr_s = (UINT8 *)(main->src_info.c_addr+ (y_s & 0xFFF0) * pit_s + ((y_s & 0x0F) <<4) + \
			((x_s & 0xFFF8) <<5) + ((x_s & 0x07)<<1));
		x_s <<= 1;
#if 0		
		ver_accumulator = 0;
		hor_int_cnt = 0;
		hor_fra_counter = g_jhconvs_scale_hor_fra_c;
		hor_part_sum_cnt = hor_part_sum = hor_part_sum_c = 0;		
#endif		
		while(w_s)
		{
			if(x_s&0x01)
			{
				cr = *addr_s++;
				*ver_part_sum_buf_c++ = cb;
				*ver_part_sum_buf_c++ = cr;				
				*ver_part_sum_buf_c++ = cb;
				*ver_part_sum_buf_c++ = cr;
			}
			else
			{
				cb = *addr_s++;
			}	
			if(0xF == (x_s&0xF))
				addr_s += 240;
			w_s--;x_s++;
		}
		ver_part_sum_buf_c = (UINT32 *)(g_jhconvs_scale_partsum_buf_c);		
		w_s = main->src_info.rect.w&(~0x01);
		x_s = main->src_info.rect.x&(~0x01);
		while(w_s)
		{
			hor_part_sum_cnt++;
			hor_part_sum += *ver_part_sum_buf_c++;
			hor_part_sum_c += *ver_part_sum_buf_c++;
			if(hor_part_sum_cnt >= g_jhconvs_scale_hor_int_c)
			{
				hor_int_cnt++;
				if(hor_int_cnt > (hor_fra_counter>>8))
				{
					fra = hor_fra_counter&0xFF;
					fra2 = 0x100 - fra;
					*ver_part_sum_buf++ = hor_part_sum*fra + hor_bilinear*fra2;
					*ver_part_sum_buf++ = hor_part_sum_c*fra + hor_bilinear_c*fra2;					
					hor_fra_counter += g_jhconvs_scale_hor_fra_c;
					ver_accumulator += 2;
				}
				hor_bilinear = hor_part_sum;
				hor_bilinear_c = hor_part_sum_c;
				hor_part_sum = 0;
				hor_part_sum_c = 0;
				hor_part_sum_cnt = 0;
			}
			if(0xE == (x_s&0xF))
				addr_s += 240;
			w_s -= 2;
			x_s += 2;
		}	
		h_d = main->dst_info.rect.h&(~0x01);
		y_d = main->dst_info.rect.y>>1;
		ver_part_sum_buf = (UINT32 *)(g_jhconvs_scale_partsum_buf);
		ver_bilinear_buf = (UINT32 *)(g_jhconvs_scale_bilinear_buf);
		ver_part_sum_cnt++;
		if(ver_part_sum_cnt >= g_jhconvs_scale_ver_int)
		{
			ver_int_cnt++;
			if(ver_int_cnt > (ver_fra_counter>>8))
			{
				addr_d = (UINT8 *)(main->dst_info.c_addr +  (y_d & 0xFFF0) * pit_d + \
					((y_d & 0x0F) <<4) + ((x_d & 0xFFF8) <<5) + ((x_d & 7) <<1));	
				fra = ver_fra_counter&0xFF;
				fra2 = 0xFF - fra;
				while(h_d)
				{
					out = fra * ((UINT32)(*ver_part_sum_buf++)) + fra2 * ((UINT32)(*ver_bilinear_buf++));
					out >>= 16;
					out *= g_jhconvs_scale_factor_inverse_c;
					out >>= 15;
					if(ANG_90_A == g_imagedec_angle[0])
					{
						*addr_d++ = (UINT8)(out&0xFF);		
						out = fra * ((UINT32)(*ver_part_sum_buf++)) + fra2 * ((UINT32)(*ver_bilinear_buf++));	
						out >>= 16;
						out *= g_jhconvs_scale_factor_inverse_c;
						out >>= 15;						
						*addr_d = (UINT8)(out&0xFF);	
						addr_d += 15;
						if(0xF == (y_d&0xF))
							addr_d += ads_cnt;
						y_d++;
					}
					else
					{
						*addr_d++ = (UINT8)(out&0xFF);		
						out = fra * ((UINT32)(*ver_part_sum_buf++)) + fra2 * ((UINT32)(*ver_bilinear_buf++));	
						out >>= 16;
						out *= g_jhconvs_scale_factor_inverse_c;
						out >>= 15;						
						*addr_d = (UINT8)(out&0xFF);	
						addr_d -= 17;
						if(0 == (y_d&0xF))
							addr_d -= ads_cnt;
						y_d--;
					}
					h_d -= 2;
				}
			//	JHCON_PRINTF("c line <%d>\n",y_d);
				ver_fra_counter += g_jhconvs_scale_ver_fra_c;
				if(ANG_90_A == g_imagedec_angle[0])
					x_d--;
				else
					x_d++;
			}
			jh_mem_cpy((void *)g_jhconvs_scale_bilinear_buf,(void *)g_jhconvs_scale_partsum_buf,g_jhconvs_buf_size<<2);
			jh_mem_set((void *)g_jhconvs_scale_partsum_buf,0,g_jhconvs_buf_size<<2);
			ver_part_sum_cnt = 0;
		}		
		h_s--;y_s++;
	}
}
void jhcovs_m3329e_init_full(pImagedec_hdl phdl)
{
	UINT32 ori_w = 0,ori_h = 0,ori_w3 = 0,ori_h4 = 0;
	UINT32 x = 0, y = 0, w = 0, h = 0;
	UINT32 x_h1 = 0, y_h1 = 0, w_h1 = 0, h_h1 = 0;
	UINT32 x_h2 = 0, y_h2 = 0, w_h2 = 0, h_h2 = 0;
	UINT32 x_s = 0, y_s = 0, w_s = 0, h_s = 0;	
	UINT8 flag_gap_y = 0,flag_gap = 0;
	
	ori_w = phdl->imageout.hw_acce_rect.uWidth<<phdl->imageout.hw_h_pre;
	ori_h = phdl->imageout.hw_acce_rect.uHeight<<phdl->imageout.hw_v_pre;
	ori_w3 = ori_w * JHCONVS_MAX_OUT_HEIGHT;ori_h4 = ori_h * JHCONVS_MAX_OUT_WIDTH;
	 JHCON_PRINTF("Init Entry : ori_w <%d>ori_h<%d> h_pre <%d>v_pre<%d> hw_w <%d>hw_h<%d>\n",\
	 	ori_w,ori_h,phdl->imageout.hw_h_pre,phdl->imageout.hw_v_pre,phdl->imageout.hw_acce_rect.uWidth,phdl->imageout.hw_acce_rect.uHeight);
	/*align w:h = 4:3*/
	if(ori_w3 > ori_h4)
	{
		w = ori_w;
		h = ori_w3/JHCONVS_MAX_OUT_WIDTH;
		y = (h - ori_h)>>1;
	}
	else if(ori_w3 < ori_h4)
	{
		w = ori_h4/JHCONVS_MAX_OUT_HEIGHT;
		h = ori_h;
		x = (w - ori_w)>>1;
	}
	else
	{
		w = ori_w;
		h = ori_h;
	}
	JHCON_PRINTF("Align to 720X576: w <%d>h<%d>\n",w,h);
	/*scaled size and position of DE and SW*/
	x_h1 = y_h1 = x_h2 = y_h2 = 0;
	w_h1 = w_h2 = JHCONVS_MAX_OUT_WIDTH;
	h_h1 = h_h2 = JHCONVS_MAX_OUT_HEIGHT*5;
	x_s = y_s = 0;
	w_s = phdl->imageout.hw_acce_rect.uWidth;
	h_s = phdl->imageout.hw_acce_rect.uHeight;
	if(w > ori_w)
	{
		if(w > JHCONVS_MAX_OUT_WIDTH)
		{	
			w_h2 = ori_w*JHCONVS_MAX_OUT_WIDTH/w;	
			x_h2 = x* JHCONVS_MAX_OUT_WIDTH/w;
			if(h_s > JHCONVS_MAX_OUT_HEIGHT)
				h_s = JHCONVS_MAX_OUT_HEIGHT;
		}
		else
		{
		
			if(g_imagedec_force_full_screen[phdl->id] == 0)
			{
				flag_gap = 1;
				x_s = ( JHCONVS_MAX_OUT_WIDTH - ori_w)>>1;
				y_s = (JHCONVS_MAX_OUT_HEIGHT - ori_h)>>1;
			}
		}
	}
	else if(h > ori_h)
	{
		if(h > JHCONVS_MAX_OUT_HEIGHT)
		{
			y_h2 = y*JHCONVS_MAX_OUT_HEIGHT*5/h;
			h_h2 = ori_h*JHCONVS_MAX_OUT_HEIGHT*5/h;
			if(h_s > JHCONVS_MAX_OUT_HEIGHT)
				h_s = JHCONVS_MAX_OUT_HEIGHT;
		}
		else
		{
			if(g_imagedec_force_full_screen[phdl->id] == 0)
			{		
				flag_gap = 1;
				x_s = ( JHCONVS_MAX_OUT_WIDTH - ori_w)>>1;
				y_s = (JHCONVS_MAX_OUT_HEIGHT - ori_h)>>1;
			}
		}
	}
	else
	{
		if(h_s > JHCONVS_MAX_OUT_HEIGHT)
			h_s = JHCONVS_MAX_OUT_HEIGHT;

		if(h < JHCONVS_MAX_OUT_HEIGHT)
		{
			if(g_imagedec_force_full_screen[phdl->id] == 0)
			{		
				flag_gap = 1;
				x_s = ( JHCONVS_MAX_OUT_WIDTH - w_s)>>1;
				y_s = (JHCONVS_MAX_OUT_HEIGHT - h_s)>>1;			
			}
		}
	}

		
	/*conversion parameter init*/
	if(flag_gap)
	{
		phdl->imageout.image_out_w = JHCONVS_MAX_OUT_WIDTH;
		phdl->imageout.image_out_h = JHCONVS_MAX_OUT_HEIGHT;
	}
	else
	{
#ifdef JPEG_CUT_EDGE
		if(g_imagedec_m33_extend_hw_vld)
		{
			g_jhconvs_hor_scale_flag = 1;			
			if(w_s > JHCONVS_MAX_OUT_WIDTH)
			{
				if(w_s < JPEG_HW_MAX_SCALED_WIDTH2)
				{
					phdl->imageout.hw_acce_rect.uStartX = ((w_s - JHCONVS_MAX_OUT_WIDTH)>>1) & ~0x1F;
					phdl->imageout.hw_acce_rect.uWidth = JHCONVS_MAX_OUT_WIDTH;
				}
				w_s = JHCONVS_MAX_OUT_WIDTH;				
			}
		}
		else
		{
			if(w_s > JHCONVS_MAX_OUT_WIDTH)
			{
				phdl->imageout.hw_acce_rect.uStartX = ((w_s - JHCONVS_MAX_OUT_WIDTH)>>1) & ~0x1F;
				w_s = JHCONVS_MAX_OUT_WIDTH;
				phdl->imageout.hw_acce_rect.uWidth = w_s;
			}
		}
#endif
		phdl->imageout.image_out_w = w_s;
		phdl->imageout.image_out_h = h_s;
	}
	
	JHCON_PRINTF("SW x <%d>y<%d>w<%d>h<%d>\n",x_s,y_s,w_s,h_s);
	JHCON_PRINTF("DE_S x <%d>y<%d>w<%d>h<%d>\n",x_h1,y_h1,w_h1,h_h1);
	JHCON_PRINTF("DE_D x <%d>y<%d>w<%d>h<%d>\n",x_h2,y_h2,w_h2,h_h2);	
	
	phdl->imageout.de_src_rect.uStartX = x_h1;
	phdl->imageout.de_src_rect.uStartY = y_h1;
	phdl->imageout.de_src_rect.uWidth = w_h1;
	phdl->imageout.de_src_rect.uHeight = h_h1;
	phdl->imageout.de_dst_rect.uStartX = x_h2;
	phdl->imageout.de_dst_rect.uStartY = y_h2;
	phdl->imageout.de_dst_rect.uWidth = w_h2;
	phdl->imageout.de_dst_rect.uHeight = h_h2;	
	phdl->imageout.sw_scaled_rect.uStartX = x_s;
	phdl->imageout.sw_scaled_rect.uStartY = y_s;
	phdl->imageout.sw_scaled_rect.uWidth = w_s;
	phdl->imageout.sw_scaled_rect.uHeight = h_s;
	phdl->imageout.image_out_x = x_s;
	phdl->imageout.image_out_y = y_s;	
	phdl->imageout.image_output_stride = JHCONVS_MAX_OUT_WIDTH;
	g_jhconvs_info.src_info.y_addr = phdl->imageout.hw_acc_frm_y;
	g_jhconvs_info.src_info.c_addr = phdl->imageout.hw_acc_frm_c;
	g_jhconvs_info.src_info.stride = phdl->imageout.hw_acc_stride;
	g_jhconvs_info.src_info.rect.x = phdl->imageout.hw_acce_rect.uStartX;
	g_jhconvs_info.src_info.rect.y = phdl->imageout.hw_acce_rect.uStartY;
	g_jhconvs_info.src_info.rect.w = phdl->imageout.hw_acce_rect.uWidth;
	g_jhconvs_info.src_info.rect.h = phdl->imageout.hw_acce_rect.uHeight;	
	g_jhconvs_info.dst_info.y_addr = (UINT32)phdl->imageout.frm_y_addr;
	g_jhconvs_info.dst_info.c_addr = (UINT32)phdl->imageout.frm_c_addr;
	g_jhconvs_info.dst_info.stride = phdl->imageout.image_output_stride;
	g_jhconvs_info.dst_info.rect.x = phdl->imageout.sw_scaled_rect.uStartX;
	g_jhconvs_info.dst_info.rect.y = phdl->imageout.sw_scaled_rect.uStartY;
	g_jhconvs_info.dst_info.rect.w = phdl->imageout.sw_scaled_rect.uWidth;
	g_jhconvs_info.dst_info.rect.h = phdl->imageout.sw_scaled_rect.uHeight;	
	if(ANG_180 == g_imagedec_angle[0])
	{
		g_jhconvs_info.dst_info.rect.x += g_jhconvs_info.dst_info.rect.w - 1;
		g_jhconvs_info.dst_info.rect.y += g_jhconvs_info.dst_info.rect.h - 1;
	}	
	/*vertical scale init value of SW*/
	jh_mem_set((void *)g_jhconvs_scale_bilinear_buf,0,g_jhconvs_buf_size<<2);
	jh_mem_set((void *)g_jhconvs_scale_partsum_buf,0,g_jhconvs_buf_size<<2);	
	if(g_imagedec_m33_extend_hw_vld && g_jhconvs_hor_scale_flag)
	{
		g_jhconvs_scale_ver_int = 1;
		g_jhconvs_scale_ver_fra = 0x100;
		g_jhconvs_scale_hor_int = 1;
		g_jhconvs_scale_hor_fra = 0x100;
		if(g_jhconvs_info.dst_info.rect.h > g_jhconvs_info.src_info.rect.h)
		{
			JHCON_ASSERT();
		}
		else if(g_jhconvs_info.dst_info.rect.h != g_jhconvs_info.src_info.rect.h)
		{
			UINT32 int_scaled_size = 0;
			do
			{
				g_jhconvs_scale_ver_int++;
				int_scaled_size = g_jhconvs_info.src_info.rect.h/g_jhconvs_scale_ver_int;
			}while(int_scaled_size > g_jhconvs_info.dst_info.rect.h);
			g_jhconvs_scale_ver_int--;
			int_scaled_size = g_jhconvs_info.src_info.rect.h/g_jhconvs_scale_ver_int;
			g_jhconvs_scale_ver_fra = (int_scaled_size<<8)/g_jhconvs_info.dst_info.rect.h;
			if(g_jhconvs_scale_ver_fra * g_jhconvs_info.dst_info.rect.h != int_scaled_size<<8)
				g_jhconvs_scale_ver_fra++;
		}
		if(g_jhconvs_info.dst_info.rect.w > g_jhconvs_info.src_info.rect.w)
		{
			JHCON_ASSERT();
		}
		else if(g_jhconvs_info.dst_info.rect.w != g_jhconvs_info.src_info.rect.w)
		{
			UINT32 int_scaled_size = 0;
			do
			{
				g_jhconvs_scale_hor_int++;
				int_scaled_size = g_jhconvs_info.src_info.rect.w/g_jhconvs_scale_hor_int;
			}while(int_scaled_size > g_jhconvs_info.dst_info.rect.w);
			g_jhconvs_scale_hor_int--;
			int_scaled_size = g_jhconvs_info.src_info.rect.w/g_jhconvs_scale_hor_int;
			g_jhconvs_scale_hor_fra = (int_scaled_size<<8)/g_jhconvs_info.dst_info.rect.w;
			if(g_jhconvs_scale_hor_fra * g_jhconvs_info.dst_info.rect.w != int_scaled_size<<8)
				g_jhconvs_scale_hor_fra++;
		}		
	}
	else
	{
		g_jhconvs_scale_ver_int = 1;
		g_jhconvs_scale_ver_fra = 0x100;
		if(g_jhconvs_info.dst_info.rect.h > g_jhconvs_info.src_info.rect.h)
		{
			JHCON_ASSERT("not support scale up\n");
		}
		else if(g_jhconvs_info.dst_info.rect.h != g_jhconvs_info.src_info.rect.h)
		{
			g_jhconvs_scale_ver_fra = (g_jhconvs_info.src_info.rect.h<<8)/g_jhconvs_info.dst_info.rect.h;
		}
		if(g_jhconvs_info.src_info.rect.h<<8 != g_jhconvs_info.dst_info.rect.h * g_jhconvs_scale_ver_fra)
			g_jhconvs_scale_ver_fra++;
	}
	
	/*conversion operation function binding*/
	if(g_imagedec_m33_extend_hw_vld && g_jhconvs_hor_scale_flag)
		g_jhconvs_info.y_convert_op = jhcovs_m3329e_y_thum;		
	else
		g_jhconvs_info.y_convert_op = jhcovs_m3329e_y_full;
	switch(phdl->decoder.scan_type)
	{
		case JPGD_YH1V1:
		case JPGD_YH2V1:
			g_jhconvs_scale_ver_int_c = g_jhconvs_scale_ver_int;
			g_jhconvs_scale_ver_fra_c = g_jhconvs_scale_ver_fra<<1;
			g_jhconvs_info.src_info.format = JHCONVS_422;
			if(g_imagedec_m33_extend_hw_vld && g_jhconvs_hor_scale_flag)	
			{
				g_jhconvs_scale_hor_int_c = g_jhconvs_scale_hor_int;
				g_jhconvs_scale_hor_fra_c = g_jhconvs_scale_hor_fra;				
				g_jhconvs_info.c_convert_op = jhcovs_m3329e_c_422_thum;				
			}
			else
				g_jhconvs_info.c_convert_op = jhcovs_m3329e_c_422_full;
			break;
		case JPGD_YH4V1:
			g_jhconvs_info.src_info.format = JHCONVS_411;
			g_jhconvs_scale_ver_int_c = g_jhconvs_scale_ver_int;
			g_jhconvs_scale_ver_fra_c = g_jhconvs_scale_ver_fra<<1;
			if(g_imagedec_m33_extend_hw_vld && g_jhconvs_hor_scale_flag)	
			{
				g_jhconvs_scale_hor_int_c = g_jhconvs_scale_hor_int;
				g_jhconvs_scale_hor_fra_c = g_jhconvs_scale_hor_fra;
				g_jhconvs_info.c_convert_op = jhcovs_m3329e_c_411_thum;					
			}	
			else				
				g_jhconvs_info.c_convert_op = jhcovs_m3329e_c_411_full;			
			break;
		case JPGD_YH1V2:
		case JPGD_YH2V2:
			g_jhconvs_scale_ver_int_c = g_jhconvs_scale_ver_int;
			g_jhconvs_scale_ver_fra_c = g_jhconvs_scale_ver_fra;
			g_jhconvs_info.src_info.format = JHCONVS_420;
			if(g_imagedec_m33_extend_hw_vld && g_jhconvs_hor_scale_flag)	
			{
				g_jhconvs_scale_hor_int_c = g_jhconvs_scale_hor_int;
				g_jhconvs_scale_hor_fra_c = g_jhconvs_scale_hor_fra;
				g_jhconvs_info.c_convert_op = jhcovs_m3329e_c_420_thum;				
			}
			else
				g_jhconvs_info.c_convert_op = jhcovs_m3329e_c_420_full;
			break;
		default:
			JHCON_ASSERT(0);
			break;	
	}
	g_jhconvs_info.dst_info.format = JHCONVS_420;
	if(g_imagedec_m33_extend_hw_vld && g_jhconvs_hor_scale_flag)	
	{
		g_jhconvs_scale_factor_inverse = 32768/g_jhconvs_scale_hor_int;
		g_jhconvs_scale_factor_inverse_c = 32768/g_jhconvs_scale_hor_int_c;	
	}
	JHCON_PRINTF("Ver: y int <%d>fra <%d>\n",g_jhconvs_scale_ver_int,g_jhconvs_scale_ver_fra);
	JHCON_PRINTF("Ver: c int <%d>fra <%d>\n",g_jhconvs_scale_ver_int_c,g_jhconvs_scale_ver_fra_c);
	JHCON_PRINTF("Hor: y int <%d>fra <%d>\n",g_jhconvs_scale_hor_int,g_jhconvs_scale_hor_fra);
	JHCON_PRINTF("Hor: c int <%d>fra <%d>\n",g_jhconvs_scale_hor_int_c,g_jhconvs_scale_hor_fra_c);	
}
void jhcovs_m3329e_init_full_90(pImagedec_hdl phdl)
{
	UINT32 ori_w = 0,ori_h = 0,ori_w3 = 0,ori_h4 = 0;
	UINT32 x = 0, y = 0, w = 0, h = 0;
	UINT32 x_h1 = 0, y_h1 = 0, w_h1 = 0, h_h1 = 0;
	UINT32 x_h2 = 0, y_h2 = 0, w_h2 = 0, h_h2 = 0;
	UINT32 x_s = 0, y_s = 0, w_s = 0, h_s = 0;	
	UINT8 flag_gap_y = 0,flag_gap = 0;

#if 0
	ori_w = phdl->imageout.hw_acce_rect.uWidth<<phdl->imageout.hw_h_pre;
	ori_h = phdl->imageout.hw_acce_rect.uHeight<<phdl->imageout.hw_v_pre;
#else
	ori_h = phdl->imageout.hw_acce_rect.uWidth<<phdl->imageout.hw_h_pre;
	ori_w = phdl->imageout.hw_acce_rect.uHeight<<phdl->imageout.hw_v_pre;
#endif
	ori_w3 = ori_w * JHCONVS_MAX_OUT_HEIGHT;ori_h4 = ori_h * JHCONVS_MAX_OUT_WIDTH;
	 JHCON_PRINTF("Init Entry : ori_w <%d>ori_h<%d> h_pre <%d>v_pre<%d> hw_w <%d>hw_h<%d>\n",\
	 	ori_w,ori_h,phdl->imageout.hw_h_pre,phdl->imageout.hw_v_pre,phdl->imageout.hw_acce_rect.uWidth,phdl->imageout.hw_acce_rect.uHeight);
	/*align w:h = 4:3*/
	if(ori_w3 > ori_h4)
	{
		w = ori_w;
		h = ori_w3/JHCONVS_MAX_OUT_WIDTH;
		y = (h - ori_h)>>1;
	}
	else if(ori_w3 < ori_h4)
	{
		w = ori_h4/JHCONVS_MAX_OUT_HEIGHT;
		h = ori_h;
		x = (w - ori_w)>>1;
	}
	else
	{
		w = ori_w;
		h = ori_h;
	}
	JHCON_PRINTF("Align to 720X576: w <%d>h<%d>\n",w,h);
	/*scaled size and position of DE and SW*/
	x_h1 = y_h1 = x_h2 = y_h2 = 0;
	w_h1 = w_h2 = JHCONVS_MAX_OUT_WIDTH;
	h_h1 = h_h2 = JHCONVS_MAX_OUT_HEIGHT*5;
	x_s = y_s = 0;
#if 0
	w_s = phdl->imageout.hw_acce_rect.uWidth;
	h_s = phdl->imageout.hw_acce_rect.uHeight;
#else
	h_s = phdl->imageout.hw_acce_rect.uWidth;
	w_s = phdl->imageout.hw_acce_rect.uHeight;
#endif
	if(w > ori_w)
	{
		if(w > JHCONVS_MAX_OUT_WIDTH)
		{	
			w_h2 = ori_w*JHCONVS_MAX_OUT_WIDTH/w;	
			x_h2 = x* JHCONVS_MAX_OUT_WIDTH/w;
			if(w_s > JHCONVS_MAX_OUT_WIDTH)
				w_s = JHCONVS_MAX_OUT_WIDTH;
		}
		else
		{
			flag_gap = 1;
			x_s = ( JHCONVS_MAX_OUT_WIDTH - ori_w)>>1;
			y_s = (JHCONVS_MAX_OUT_HEIGHT - ori_h)>>1;
		}
	}
	else if(h > ori_h)
	{
		if(h > JHCONVS_MAX_OUT_HEIGHT)
		{
			y_h2 = y*JHCONVS_MAX_OUT_HEIGHT*5/h;
			h_h2 = ori_h*JHCONVS_MAX_OUT_HEIGHT*5/h;
			if(w_s > JHCONVS_MAX_OUT_WIDTH)
				w_s = JHCONVS_MAX_OUT_WIDTH;
		}
		else
		{
			flag_gap = 1;
			x_s = ( JHCONVS_MAX_OUT_WIDTH - ori_w)>>1;
			y_s = (JHCONVS_MAX_OUT_HEIGHT - ori_h)>>1;
		}
	}
	else
	{
		if(w_s > JHCONVS_MAX_OUT_WIDTH)
			w_s = JHCONVS_MAX_OUT_WIDTH;

		if(w < JHCONVS_MAX_OUT_WIDTH)
		{
			flag_gap = 1;
			x_s = ( JHCONVS_MAX_OUT_WIDTH - w_s)>>1;
			y_s = (JHCONVS_MAX_OUT_HEIGHT - h_s)>>1;		
		}
	}

	if(flag_gap)
	{
		phdl->imageout.image_out_w = JHCONVS_MAX_OUT_WIDTH;
		phdl->imageout.image_out_h = JHCONVS_MAX_OUT_HEIGHT;
	}
	else
	{
		if(g_imagedec_m33_extend_hw_vld)
		{
			if(h_s > JHCONVS_MAX_OUT_HEIGHT)
			{
				if(h_s < (JHCONVS_MAX_OUT_HEIGHT + 50))
				{
					phdl->imageout.hw_acce_rect.uStartX = ((h_s - JHCONVS_MAX_OUT_HEIGHT)>>1) & ~0x1F;
					phdl->imageout.hw_acce_rect.uWidth = JHCONVS_MAX_OUT_HEIGHT;
				}
				h_s = JHCONVS_MAX_OUT_HEIGHT;				
			}			
			g_jhconvs_hor_scale_flag = 1;
		}
		
		phdl->imageout.image_out_w = w_s;
		phdl->imageout.image_out_h = h_s;
	}
	
	JHCON_PRINTF("SW x <%d>y<%d>w<%d>h<%d>\n",x_s,y_s,w_s,h_s);
	JHCON_PRINTF("DE_S x <%d>y<%d>w<%d>h<%d>\n",x_h1,y_h1,w_h1,h_h1);
	JHCON_PRINTF("DE_D x <%d>y<%d>w<%d>h<%d>\n",x_h2,y_h2,w_h2,h_h2);		
	/*conversion parameter init*/
	phdl->imageout.de_src_rect.uStartX = x_h1;
	phdl->imageout.de_src_rect.uStartY = y_h1;
	phdl->imageout.de_src_rect.uWidth = w_h1;
	phdl->imageout.de_src_rect.uHeight = h_h1;
	phdl->imageout.de_dst_rect.uStartX = x_h2;
	phdl->imageout.de_dst_rect.uStartY = y_h2;
	phdl->imageout.de_dst_rect.uWidth = w_h2;
	phdl->imageout.de_dst_rect.uHeight = h_h2;	
	phdl->imageout.sw_scaled_rect.uStartX = x_s;
	phdl->imageout.sw_scaled_rect.uStartY = y_s;
	phdl->imageout.sw_scaled_rect.uWidth = w_s;
	phdl->imageout.sw_scaled_rect.uHeight = h_s;
	phdl->imageout.image_out_x = x_s;
	phdl->imageout.image_out_y = y_s;
	phdl->imageout.image_output_stride = JHCONVS_MAX_OUT_WIDTH;
	g_jhconvs_info.src_info.y_addr = phdl->imageout.hw_acc_frm_y;
	g_jhconvs_info.src_info.c_addr = phdl->imageout.hw_acc_frm_c;
	g_jhconvs_info.src_info.stride = phdl->imageout.hw_acc_stride;
	g_jhconvs_info.src_info.rect.x = phdl->imageout.hw_acce_rect.uStartX;
	g_jhconvs_info.src_info.rect.y = phdl->imageout.hw_acce_rect.uStartY;
	g_jhconvs_info.src_info.rect.w = phdl->imageout.hw_acce_rect.uWidth;
	g_jhconvs_info.src_info.rect.h = phdl->imageout.hw_acce_rect.uHeight;	
	g_jhconvs_info.dst_info.y_addr = (UINT32)phdl->imageout.frm_y_addr;
	g_jhconvs_info.dst_info.c_addr = (UINT32)phdl->imageout.frm_c_addr;
	g_jhconvs_info.dst_info.stride = phdl->imageout.image_output_stride;
	g_jhconvs_info.dst_info.rect.x = phdl->imageout.sw_scaled_rect.uStartX;
	g_jhconvs_info.dst_info.rect.y = phdl->imageout.sw_scaled_rect.uStartY;
	g_jhconvs_info.dst_info.rect.w = phdl->imageout.sw_scaled_rect.uWidth;
	g_jhconvs_info.dst_info.rect.h = phdl->imageout.sw_scaled_rect.uHeight;	
	if(ANG_90_A == g_imagedec_angle[0])
	{
		g_jhconvs_info.dst_info.rect.x += g_jhconvs_info.dst_info.rect.w - 1;
	}
	else
	{
		g_jhconvs_info.dst_info.rect.y += g_jhconvs_info.dst_info.rect.h - 1;		
	}
	/*vertical scale init value of SW*/
	jh_mem_set((void *)g_jhconvs_scale_bilinear_buf,0,g_jhconvs_buf_size<<2);
	jh_mem_set((void *)g_jhconvs_scale_partsum_buf,0,g_jhconvs_buf_size<<2);	
	if(g_imagedec_m33_extend_hw_vld && g_jhconvs_hor_scale_flag)
	{
		g_jhconvs_scale_ver_int = 1;
		g_jhconvs_scale_ver_fra = 0x100;
		g_jhconvs_scale_hor_int = 1;
		g_jhconvs_scale_hor_fra = 0x100;
		if(g_jhconvs_info.dst_info.rect.w > g_jhconvs_info.src_info.rect.h)
		{
			JHCON_ASSERT();
		}
		else if(g_jhconvs_info.dst_info.rect.w != g_jhconvs_info.src_info.rect.h)
		{
			UINT32 int_scaled_size = 0;
			do
			{
				g_jhconvs_scale_ver_int++;
				int_scaled_size = g_jhconvs_info.src_info.rect.h/g_jhconvs_scale_ver_int;
			}while(int_scaled_size > g_jhconvs_info.dst_info.rect.w);
			g_jhconvs_scale_ver_int--;
			int_scaled_size = g_jhconvs_info.src_info.rect.h/g_jhconvs_scale_ver_int;
			g_jhconvs_scale_ver_fra = (int_scaled_size<<8)/g_jhconvs_info.dst_info.rect.w;
			if(g_jhconvs_scale_ver_fra * g_jhconvs_info.dst_info.rect.w != int_scaled_size<<8)
				g_jhconvs_scale_ver_fra++;
		}
		
		if(g_jhconvs_info.dst_info.rect.h > g_jhconvs_info.src_info.rect.w)
		{
			JHCON_ASSERT();
		}
		else if(g_jhconvs_info.dst_info.rect.h != g_jhconvs_info.src_info.rect.w)
		{
			UINT32 int_scaled_size = 0;
			do
			{
				g_jhconvs_scale_hor_int++;
				int_scaled_size = g_jhconvs_info.src_info.rect.w/g_jhconvs_scale_hor_int;
			}while(int_scaled_size > g_jhconvs_info.dst_info.rect.h);
			g_jhconvs_scale_hor_int--;
			int_scaled_size = g_jhconvs_info.src_info.rect.w/g_jhconvs_scale_hor_int;
			g_jhconvs_scale_hor_fra = (int_scaled_size<<8)/g_jhconvs_info.dst_info.rect.h;
			if(g_jhconvs_scale_hor_fra * g_jhconvs_info.dst_info.rect.h != int_scaled_size<<8)
				g_jhconvs_scale_hor_fra++;
		}		
	}
	else
	{
		g_jhconvs_scale_ver_int = 1;
		g_jhconvs_scale_ver_fra = 0x100;
		if(g_jhconvs_info.dst_info.rect.w > g_jhconvs_info.src_info.rect.h)
		{
			JHCON_ASSERT("not support scale up\n");
		}
		else if(g_jhconvs_info.dst_info.rect.w != g_jhconvs_info.src_info.rect.h)
		{
			g_jhconvs_scale_ver_fra = (g_jhconvs_info.src_info.rect.h<<8)/g_jhconvs_info.dst_info.rect.w;
		}
		if(g_jhconvs_info.src_info.rect.h<<8 != g_jhconvs_info.dst_info.rect.w * g_jhconvs_scale_ver_fra)
			g_jhconvs_scale_ver_fra++;
	}
	
	/*conversion operation function binding*/
	if(g_imagedec_m33_extend_hw_vld && g_jhconvs_hor_scale_flag)
		g_jhconvs_info.y_convert_op = jhcovs_m3329e_y_thum_90;	
	else
		g_jhconvs_info.y_convert_op = jhcovs_m3329e_y_full_90;
	switch(phdl->decoder.scan_type)
	{
		case JPGD_YH1V1:
		case JPGD_YH2V1:
			g_jhconvs_scale_ver_int_c = g_jhconvs_scale_ver_int;
			g_jhconvs_scale_ver_fra_c = g_jhconvs_scale_ver_fra<<1;
			g_jhconvs_info.src_info.format = JHCONVS_422;
			if(g_imagedec_m33_extend_hw_vld && g_jhconvs_hor_scale_flag)			
			{
				g_jhconvs_scale_hor_int_c = g_jhconvs_scale_hor_int;
				g_jhconvs_scale_hor_fra_c = g_jhconvs_scale_hor_fra;	
				g_jhconvs_info.c_convert_op = jhcovs_m3329e_c_422_thum_90;				
			}
			else
				g_jhconvs_info.c_convert_op = jhcovs_m3329e_c_422_full_90;
			break;
		case JPGD_YH4V1:
			g_jhconvs_info.src_info.format = JHCONVS_411;
			g_jhconvs_scale_ver_int_c = g_jhconvs_scale_ver_int;
			g_jhconvs_scale_ver_fra_c = g_jhconvs_scale_ver_fra<<1;
			if(g_imagedec_m33_extend_hw_vld && g_jhconvs_hor_scale_flag)			
			{
				g_jhconvs_scale_hor_int_c = g_jhconvs_scale_hor_int;
				g_jhconvs_scale_hor_fra_c = g_jhconvs_scale_hor_fra;
				g_jhconvs_info.c_convert_op = jhcovs_m3329e_c_411_thum_90;				
			}
			else	
				g_jhconvs_info.c_convert_op = jhcovs_m3329e_c_411_full_90;			
			break;
		case JPGD_YH1V2:
		case JPGD_YH2V2:
			g_jhconvs_scale_ver_int_c = g_jhconvs_scale_ver_int;
			g_jhconvs_scale_ver_fra_c = g_jhconvs_scale_ver_fra;
			g_jhconvs_info.src_info.format = JHCONVS_420;
			if(g_imagedec_m33_extend_hw_vld && g_jhconvs_hor_scale_flag)			
			{
				g_jhconvs_scale_hor_int_c = g_jhconvs_scale_hor_int;
				g_jhconvs_scale_hor_fra_c = g_jhconvs_scale_hor_fra;
				g_jhconvs_info.c_convert_op = jhcovs_m3329e_c_420_thum_90;				
			}
			else
				g_jhconvs_info.c_convert_op = jhcovs_m3329e_c_420_full_90;
			break;
		default:
			JHCON_ASSERT(0);
			break;	
	}
	g_jhconvs_info.dst_info.format = JHCONVS_420;
	if(g_imagedec_m33_extend_hw_vld && g_jhconvs_hor_scale_flag)	
	{
		g_jhconvs_scale_factor_inverse = 32768/g_jhconvs_scale_hor_int;
		g_jhconvs_scale_factor_inverse_c = 32768/g_jhconvs_scale_hor_int_c;	
	}
	JHCON_PRINTF("Ver: y int <%d>fra <%d>\n",g_jhconvs_scale_ver_int,g_jhconvs_scale_ver_fra);
	JHCON_PRINTF("Ver: c int <%d>fra <%d>\n",g_jhconvs_scale_ver_int_c,g_jhconvs_scale_ver_fra_c);
	JHCON_PRINTF("Hor: y int <%d>fra <%d>\n",g_jhconvs_scale_hor_int,g_jhconvs_scale_hor_fra);
	JHCON_PRINTF("Hor: c int <%d>fra <%d>\n",g_jhconvs_scale_hor_int_c,g_jhconvs_scale_hor_fra_c);	
}

void jhcovs_m3329e_init_thum(pImagedec_hdl phdl)
{
	UINT32 bk_w = (UINT32)phdl->imageout.logo_info.width;
	UINT32 bk_h = (UINT32)phdl->imageout.logo_info.height;
	UINT32 ori_w = 0,ori_h = 0,ori_w3 = 0,ori_h4 = 0;
	UINT32 x = 0, y = 0, w = 0, h = 0;
	UINT32 x_h1 = 0, y_h1 = 0, w_h1 = 0, h_h1 = 0;
	UINT32 x_h2 = 0, y_h2 = 0, w_h2 = 0, h_h2 = 0;
	UINT32 x_s = 0, y_s = 0, w_s = 0, h_s = 0;	
	UINT8 flag_gap_y = 0,flag_gap = 0;
	
	ori_w = phdl->imageout.hw_acce_rect.uWidth<<phdl->imageout.hw_h_pre;
	ori_h = phdl->imageout.hw_acce_rect.uHeight<<phdl->imageout.hw_v_pre;
	ori_w3 = ori_w * 3;ori_h4 = ori_h * 4;
	 JHCON_PRINTF("Init Entry : ori_w <%d>ori_h<%d> h_pre <%d>v_pre<%d> hw_w <%d>hw_h<%d>\n",\
	 	ori_w,ori_h,phdl->imageout.hw_h_pre,phdl->imageout.hw_v_pre,phdl->imageout.hw_acce_rect.uWidth,phdl->imageout.hw_acce_rect.uHeight);
	/*align w:h = 4:3*/
	if(ori_w3 > ori_h4)
	{
		w = ori_w;
		h = ori_w3/4;
		y = (h - ori_h)>>1;
	}
	else if(ori_w3 < ori_h4)
	{
		w = ori_h4/3;
		h = ori_h;
		x = (w - ori_w)>>1;
	}
	else
	{
		w = ori_w;
		h = ori_h;
	}
	JHCON_PRINTF("Align to 720X576: w <%d>h<%d>\n",w,h);
	/*scaled size and position of DE and SW*/
	w_s = phdl->main.setting.dis.uWidth;
	h_s = phdl->main.setting.dis.uHeight;
	x_s = phdl->main.setting.dis.uStartX;
	y_s = phdl->main.setting.dis.uStartY;	
	if((w_s < JHCONVS_MIN_OUT_WIDTH_THU) || (h_s < JHCONVS_MIN_OUT_HEIGHT_THU))
	{
		JHCON_PRINTF("w_s <%d> h_s <%d> error in thum <%s>\n",__FUNCTION__);
		phdl->main.setting.hw_acc_flag = 0;
		return;
	}
	w_s = w_s*bk_w/JHCONVS_MAX_OUT_WIDTH;
	h_s = h_s*bk_h/JHCONVS_MAX_OUT_HEIGHT;
	x_s = x_s*bk_w/JHCONVS_MAX_OUT_WIDTH;
	y_s = y_s*bk_h/JHCONVS_MAX_OUT_HEIGHT;		
	if((w < w_s) || (h < h_s))
	{
		if(w < w_s)
			x_s += (w_s - w)>>1;

		if(h < h_s)
			y_s += (h_s - h)>>1;
		
		x_s += x;
		y_s += y;
		
		if(w < w_s)		
			w_s = phdl->imageout.hw_acce_rect.uWidth;

		if(h < h_s)
			h_s = phdl->imageout.hw_acce_rect.uHeight;
	}
	else
	{
		if(x)
		{
			x_s += x*w_s/w;
			w_s  = ori_w*w_s/w;
		}
		if(y)
		{
			y_s += y*h_s/h;
			h_s = ori_h*h_s/h;
		}
	}
	JHCON_PRINTF("DE_S x <%d>y<%d>w<%d>h<%d>\n",x_h1,y_h1,w_h1,h_h1);
	JHCON_PRINTF("DE_D x <%d>y<%d>w<%d>h<%d>\n",x_h2,y_h2,w_h2,h_h2);		
	/*conversion parameter init*/
	x_s &= ~0x01;
	y_s &= ~0x01;
	w_s &= ~0x01;
	h_s &= ~0x01;	
	phdl->imageout.sw_scaled_rect.uStartX = x_s;
	phdl->imageout.sw_scaled_rect.uStartY = y_s;
	phdl->imageout.sw_scaled_rect.uWidth = w_s;
	phdl->imageout.sw_scaled_rect.uHeight = h_s;
	phdl->imageout.image_out_x = x_s;
	phdl->imageout.image_out_y = y_s;
	phdl->imageout.image_out_w = w_s;
	phdl->imageout.image_out_h = h_s;
	phdl->imageout.image_output_stride = phdl->imageout.logo_info.stride<<4;
	g_jhconvs_info.src_info.y_addr = phdl->imageout.hw_acc_frm_y;
	g_jhconvs_info.src_info.c_addr = phdl->imageout.hw_acc_frm_c;
	g_jhconvs_info.src_info.stride = phdl->imageout.hw_acc_stride;
	g_jhconvs_info.src_info.rect.x = phdl->imageout.hw_acce_rect.uStartX;
	g_jhconvs_info.src_info.rect.y = phdl->imageout.hw_acce_rect.uStartY;
	g_jhconvs_info.src_info.rect.w = phdl->imageout.hw_acce_rect.uWidth;
	g_jhconvs_info.src_info.rect.h = phdl->imageout.hw_acce_rect.uHeight;	
	g_jhconvs_info.dst_info.y_addr = (UINT32)phdl->imageout.frm_y_addr;
	g_jhconvs_info.dst_info.c_addr = (UINT32)phdl->imageout.frm_c_addr;
	g_jhconvs_info.dst_info.stride = phdl->imageout.image_output_stride;
	g_jhconvs_info.dst_info.rect.x = phdl->imageout.sw_scaled_rect.uStartX;
	g_jhconvs_info.dst_info.rect.y = phdl->imageout.sw_scaled_rect.uStartY;
	g_jhconvs_info.dst_info.rect.w = phdl->imageout.sw_scaled_rect.uWidth;
	g_jhconvs_info.dst_info.rect.h = phdl->imageout.sw_scaled_rect.uHeight;	

	if((g_jhconvs_info.dst_info.rect.x - phdl->main.setting.dis.uStartX) 
		< ((phdl->main.setting.dis.uWidth - g_jhconvs_info.dst_info.rect.w)>>1))
		g_jhconvs_info.dst_info.rect.x = ((phdl->main.setting.dis.uWidth - g_jhconvs_info.dst_info.rect.w)>>1)
			+ phdl->main.setting.dis.uStartX;

	if((g_jhconvs_info.dst_info.rect.y - phdl->main.setting.dis.uStartY) 
		< ((phdl->main.setting.dis.uHeight- g_jhconvs_info.dst_info.rect.h)>>1))
		g_jhconvs_info.dst_info.rect.y = ((phdl->main.setting.dis.uHeight - g_jhconvs_info.dst_info.rect.h)>>1)
			+ phdl->main.setting.dis.uStartY;	
	
	JHCON_PRINTF("SW x <%d>y<%d>w<%d>h<%d>\n",x_s,y_s,w_s,h_s);
	/*vertical scale init value of SW*/
	jh_mem_set((void *)g_jhconvs_scale_bilinear_buf,0,g_jhconvs_buf_size<<2);
	jh_mem_set((void *)g_jhconvs_scale_partsum_buf,0,g_jhconvs_buf_size<<2);
	g_jhconvs_scale_ver_int = 1;
	g_jhconvs_scale_ver_fra = 0x100;
	g_jhconvs_scale_hor_int = 1;
	g_jhconvs_scale_hor_fra = 0x100;
	m_jhconvs_ver_no_scale = m_jhconvs_hor_no_scale = 0;
	if(g_jhconvs_info.dst_info.rect.h > g_jhconvs_info.src_info.rect.h)
	{
		JHCON_ASSERT();
	}
	else if(g_jhconvs_info.dst_info.rect.h != g_jhconvs_info.src_info.rect.h)
	{
		UINT32 int_scaled_size = 0;
		do
		{
			g_jhconvs_scale_ver_int++;
			int_scaled_size = g_jhconvs_info.src_info.rect.h/g_jhconvs_scale_ver_int;
		}while(int_scaled_size > g_jhconvs_info.dst_info.rect.h);
		g_jhconvs_scale_ver_int--;
		int_scaled_size = g_jhconvs_info.src_info.rect.h/g_jhconvs_scale_ver_int;
		g_jhconvs_scale_ver_fra = (int_scaled_size<<8)/g_jhconvs_info.dst_info.rect.h;
		if(g_jhconvs_scale_ver_fra * g_jhconvs_info.dst_info.rect.h != int_scaled_size<<8)
			g_jhconvs_scale_ver_fra++;
	}
	else
		m_jhconvs_ver_no_scale = 1;
	
	if(g_jhconvs_info.dst_info.rect.w > g_jhconvs_info.src_info.rect.w)
	{
		JHCON_ASSERT();
	}
	else if(g_jhconvs_info.dst_info.rect.w != g_jhconvs_info.src_info.rect.w)
	{
		UINT32 int_scaled_size = 0;
		do
		{
			g_jhconvs_scale_hor_int++;
			int_scaled_size = g_jhconvs_info.src_info.rect.w/g_jhconvs_scale_hor_int;
		}while(int_scaled_size > g_jhconvs_info.dst_info.rect.w);
		g_jhconvs_scale_hor_int--;
		int_scaled_size = g_jhconvs_info.src_info.rect.w/g_jhconvs_scale_hor_int;
		g_jhconvs_scale_hor_fra = (int_scaled_size<<8)/g_jhconvs_info.dst_info.rect.w;
		if(g_jhconvs_scale_hor_fra * g_jhconvs_info.dst_info.rect.w != int_scaled_size<<8)
			g_jhconvs_scale_hor_fra++;
	}		
	else
		m_jhconvs_hor_no_scale = 1;

	jpeg_printf("%s : no scale falg hor %d ver %d\n", __FUNCTION__, m_jhconvs_hor_no_scale
		, m_jhconvs_ver_no_scale);
	
	/*conversion operation function binding*/
	g_jhconvs_info.y_convert_op = jhcovs_m3329e_y_thum;
	switch(phdl->decoder.scan_type)
	{
		case JPGD_YH1V1:
		case JPGD_YH2V1:	
/*			if(g_jhconvs_scale_ver_int > 1)
			{
				UINT32 int_scaled_size = 0;
				
				g_jhconvs_scale_ver_int_c = 1;
				g_jhconvs_scale_ver_fra_c = 0x100;
				do
				{
					g_jhconvs_scale_ver_int_c++;
					int_scaled_size = (g_jhconvs_info.src_info.rect.h>>1)/g_jhconvs_scale_ver_int_c;
				}while(int_scaled_size > g_jhconvs_info.dst_info.rect.h>>1);
				g_jhconvs_scale_ver_int_c--;
				int_scaled_size = g_jhconvs_info.src_info.rect.h/g_jhconvs_scale_ver_int_c;
				g_jhconvs_scale_ver_fra_c = (int_scaled_size<<8)/(g_jhconvs_info.dst_info.rect.h>>1);
				if(g_jhconvs_scale_ver_fra_c * (g_jhconvs_info.dst_info.rect.h>>1) != int_scaled_size<<8)
					g_jhconvs_scale_ver_fra_c++;
			}
			else
			{*/
				g_jhconvs_scale_ver_int_c = g_jhconvs_scale_ver_int;
				g_jhconvs_scale_ver_fra_c = g_jhconvs_scale_ver_fra<<1;
//			}
			g_jhconvs_scale_hor_int_c = g_jhconvs_scale_hor_int;
			g_jhconvs_scale_hor_fra_c = g_jhconvs_scale_hor_fra;
			g_jhconvs_info.src_info.format = JHCONVS_422;
			g_jhconvs_info.c_convert_op = jhcovs_m3329e_c_422_thum;
			break;
		case JPGD_YH4V1:
			g_jhconvs_info.src_info.format = JHCONVS_411;
			g_jhconvs_scale_ver_int_c = g_jhconvs_scale_ver_int;
			g_jhconvs_scale_ver_fra_c = g_jhconvs_scale_ver_fra<<1;
			g_jhconvs_scale_hor_int_c = g_jhconvs_scale_hor_int;
			g_jhconvs_scale_hor_fra_c = g_jhconvs_scale_hor_fra;			
			g_jhconvs_info.c_convert_op = jhcovs_m3329e_c_411_thum;			
			break;
		case JPGD_YH1V2:
		case JPGD_YH2V2:
			g_jhconvs_scale_ver_int_c = g_jhconvs_scale_ver_int;
			g_jhconvs_scale_ver_fra_c = g_jhconvs_scale_ver_fra;
			g_jhconvs_scale_hor_int_c = g_jhconvs_scale_hor_int;
			g_jhconvs_scale_hor_fra_c = g_jhconvs_scale_hor_fra;			
			g_jhconvs_info.src_info.format = JHCONVS_420;
			g_jhconvs_info.c_convert_op = jhcovs_m3329e_c_420_thum;
			break;
		default:
			JHCON_ASSERT(0);
			break;	
	}
	g_jhconvs_info.dst_info.format = JHCONVS_420;
	g_jhconvs_scale_factor_inverse = 32768/g_jhconvs_scale_hor_int;
	g_jhconvs_scale_factor_inverse_c = 32768/g_jhconvs_scale_hor_int_c;
	JHCON_PRINTF("Ver: y int <%d>fra <%d>\n",g_jhconvs_scale_ver_int,g_jhconvs_scale_ver_fra);
	JHCON_PRINTF("Ver: c int <%d>fra <%d>\n",g_jhconvs_scale_ver_int_c,g_jhconvs_scale_ver_fra_c);
	JHCON_PRINTF("Hor: y int <%d>fra <%d>\n",g_jhconvs_scale_hor_int,g_jhconvs_scale_hor_fra);
	JHCON_PRINTF("Hor: c int <%d>fra <%d>\n",g_jhconvs_scale_hor_int_c,g_jhconvs_scale_hor_fra_c);
	JHCON_PRINTF("Inv : y fac <%d>\n",g_jhconvs_scale_factor_inverse);
	JHCON_PRINTF("Inv : c fac <%d>\n",g_jhconvs_scale_factor_inverse_c);	
}

void jhcovs_m3329e_init(pImagedec_hdl phdl)
{	
	struct Rect old_src;
	struct Rect old_dst;
	UINT32 width = 0;
	UINT32 height = 0;

	g_jhconvs_hor_scale_flag = 0;
	
	if(IMAGEDEC_FULL_SRN == phdl->main.setting.mode)
	{
		width = phdl->imageout.image_out_w;
		height = phdl->imageout.image_out_h;
		MEMCPY((void *)&old_src,&(phdl->imageout.de_src_rect),sizeof(struct Rect));
		MEMCPY((void *)&old_dst,&(phdl->imageout.de_dst_rect),sizeof(struct Rect));
	}
	MEMSET((void *)&g_jhconvs_info,0,sizeof(struct jhconvs_main));
	if(IMAGEDEC_THUMBNAIL == phdl->main.setting.mode)
		jhcovs_m3329e_init_thum(phdl);
	else if(IMAGEDEC_FULL_SRN == phdl->main.setting.mode)
	{
		if((ANG_90_A == g_imagedec_angle[phdl->id]) || (ANG_90_C == g_imagedec_angle[phdl->id]))
			jhcovs_m3329e_init_full_90(phdl);
		else
			jhcovs_m3329e_init_full(phdl);
	}
	else
		{JHCON_ASSERT(0);}

	jh_mem_set((void *)g_jhconvs_scale_bilinear_buf,0,g_jhconvs_buf_size);
	jh_mem_set((void *)g_jhconvs_scale_partsum_buf,0,g_jhconvs_buf_size);
	jh_mem_set((void *)g_jhconvs_scale_partsum_buf_c,0,g_jhconvs_buf_size);
		
	MEMCPY((void *)&(phdl->imageout.de_dst_bk),&(phdl->imageout.de_dst_rect),sizeof(struct Rect));
#if 0		
	if((IMAGEDEC_FULL_SRN == phdl->main.setting.mode) 
		&& (0 == phdl->imageout.frm[phdl->imageout.frm_idx_using].busy))
	{
		if((width != phdl->imageout.image_out_w) || (height != phdl->imageout.image_out_h)
			|| (0 != MEMCMP((void *)&old_src,(void *)&phdl->imageout.de_src_rect,sizeof(struct Rect)))
			|| (0 != MEMCMP((void *)&old_dst,(void *)&phdl->imageout.de_dst_rect,sizeof(struct Rect))))
		{
			jpeg_printf("clear buf when de with different scale parametes\n");
			jh_mem_set(phdl->imageout.old_frm.frm_y_addr,0x10,phdl->imageout.old_frm.frm_y_len);
			jh_mem_set(phdl->imageout.old_frm.frm_c_addr,0x80,phdl->imageout.old_frm.frm_c_len);						
			jh_mem_set(phdl->imageout.frm[phdl->imageout.frm_idx_using].frm_y_addr,0x10,phdl->imageout.frm[phdl->imageout.frm_idx_using].frm_y_len);
			jh_mem_set(phdl->imageout.frm[phdl->imageout.frm_idx_using].frm_c_addr,0x80,phdl->imageout.frm[phdl->imageout.frm_idx_using].frm_c_len);			
		}
	}
#endif	
}
void jhconvs_buffer_init()
{
	if(!g_jhconvs_buf_init_flag)
	{
		if(g_imagedec_m33_extend_hw_vld)
			g_jhconvs_buf_size = JPEG_HW_MAX_SCALED_WIDTH_EXT;
		else
			g_jhconvs_buf_size = JPEG_HW_MAX_SCALED_WIDTH2;
		
		g_jhconvs_buf_init_flag = 1;
		g_jhconvs_scale_bilinear_buf = (UINT32 *)MALLOC(g_jhconvs_buf_size<<2);
		g_jhconvs_scale_partsum_buf = (UINT32 *)MALLOC(g_jhconvs_buf_size<<2);
		g_jhconvs_scale_partsum_buf_c = (UINT32 *)MALLOC(g_jhconvs_buf_size<<2);
		if((NULL == g_jhconvs_scale_bilinear_buf) || (NULL == g_jhconvs_scale_partsum_buf) 
			|| (NULL == g_jhconvs_scale_partsum_buf_c))
		{
			JHCON_PRINTF("malloc error %s\n",__FUNCTION__);
			JHCON_ASSERT(0);
		}
		jh_mem_set((void *)g_jhconvs_scale_bilinear_buf,0,g_jhconvs_buf_size);
		jh_mem_set((void *)g_jhconvs_scale_partsum_buf,0,g_jhconvs_buf_size);
		jh_mem_set((void *)g_jhconvs_scale_partsum_buf_c,0,g_jhconvs_buf_size);
		JHCON_PRINTF("b <%x>p <%x>c<%x>\n",g_jhconvs_scale_bilinear_buf,g_jhconvs_scale_partsum_buf
			,g_jhconvs_scale_partsum_buf_c);
	}
}
void jhcovs_m3329e_op(pImagedec_hdl phdl)
{
	if((NULL == g_jhconvs_info.y_convert_op) || (NULL == g_jhconvs_info.c_convert_op))
	{
		return;
	}
	JHCON_PRINTF("Conversion operation start...\n");
	g_jhconvs_info.y_convert_op(&g_jhconvs_info);
	g_jhconvs_info.c_convert_op(&g_jhconvs_info);
	JHCON_PRINTF("Conversion operation ok\n");
}






