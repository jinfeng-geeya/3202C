
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
#include <hld/decv/decv.h>
#include "imagedec_main.h"
#include "imagedec_acc.h"

extern enum IMAGE_ANGLE g_imagedec_angle[IMAGEDEC_MAX_INSTANCE];
extern int g_imagedec_m33_enable_hw_vld;
extern int g_imagedec_m33_extend_hw_vld;
extern int g_imagedec_m33_combine_hw_sw_flag;

static UINT32 g_imagedec_m33_reg_base_addr = JPEG_HW_ACUM_REG_BASE_29E;

#define JHWriteB(index, data)  *(volatile unsigned char *)(g_imagedec_m33_reg_base_addr+index) = (unsigned char)data//;\
                               // soc_printf("idx %x data %x\n",index,data)
#define JHWriteW(index, data) *(volatile unsigned short *)(g_imagedec_m33_reg_base_addr+index) = (unsigned short)data//:\
                                //soc_printf("idx %x data %x\n",index,data)
#define JHWriteD(index, data)  *(volatile unsigned int *)(g_imagedec_m33_reg_base_addr+index) = (unsigned int)data//;\
                               //soc_printf("idx %x data %x\n",index,data)
#define JHReadB(index)  (*(volatile unsigned char *)(g_imagedec_m33_reg_base_addr+index))
#define JHReadW(index) (*(volatile unsigned short *)(g_imagedec_m33_reg_base_addr+index))
#define JHReadD(index)  (*(volatile unsigned int *)(g_imagedec_m33_reg_base_addr+index))

IMAGEDEC_STATIC void jh_set_jpg_hw_work_mode(UINT8 hw_vld)
{
	volatile UINT32 reg_data = JHReadD(JPEG_HW_ACUM_CONTROL_29E);

	reg_data &= ~(1<<17);
	if(hw_vld)
		reg_data &= ~(1<<27);
	else
	reg_data |= 1<<27;
	
	JHWriteD(JPEG_HW_ACUM_CONTROL_29E,reg_data);
}



//0x14
IMAGEDEC_STATIC void jh_pic_chroma_format(int type)
{
	volatile UINT8 reg_type = 0;
	
	switch(type)
	{
		case JPGD_YH1V1:
			reg_type = 3<<3;
			break;
		case JPGD_YH4V1:
			reg_type = 1<<3;
			break;
		case JPGD_YH2V1:
			reg_type = 2<<3;
			break;
		case JPGD_YH1V2:
			reg_type = 1<<5;
			break;
		case JPGD_YH2V2:
			break;
		default:
			JPEG_ASSERT(0);
			return;
	}
	reg_type |= 3;
	JHWriteB(JPEG_HW_ACUM_START_29E + 3,reg_type);
}

IMAGEDEC_STATIC void jh_extra_op_write(UINT32 index,UINT32 content)
{
	index &= 0xFF;
	index |= (1<<31) | (3<<8);
	JHWriteD(JPEG_HW_ACUM_EXTRA_FUN_29E,index);	
	JHWriteD(JPEG_HW_ACUM_EXTRA_COT_29E,content);
}

IMAGEDEC_STATIC UINT32 jh_extra_op_read(UINT32 index)
{
	index &= 0xFF;
	index |= 3<<8;
	JHWriteD(JPEG_HW_ACUM_EXTRA_FUN_29E,index);
	return JHReadD(JPEG_HW_ACUM_EXTRA_COT_29E);
}

IMAGEDEC_STATIC void jh_mem_base(UINT32 addr)
{
	JHWriteD(JPEG_HW_ACUM_MEM_BASE_29E,(addr>>22)&0xFF);
}

IMAGEDEC_STATIC void jh_reg_control(UINT32 stride,UINT32 width_mcu,UINT32 h_pre,UINT32 v_pre)
{
	volatile UINT32 tmp = 0;

	tmp = (h_pre&0x3);
	tmp |= (v_pre&0x3)<<2;
	tmp |= ((width_mcu-1)&0x3ff)<<4;
	tmp |= stride<<16;
	jh_extra_op_write(JPEG_HW_EXT_CONTROL,tmp);
}
IMAGEDEC_STATIC void jh_hw_quantiser_table( QUANT_TYPE *table,UINT32 y_c)
{
	QUANT_TYPE data = 0;
	UINT8 index = 0;

	for(index=0;index<64;index++)
	{
		data = (*(table++))&0x7F;
		data |= index<<8;
		data |= y_c<<14;
		JHWriteD(JPEG_HW_ACUM_QUT_INDEX_29E,data);
	}
}
IMAGEDEC_STATIC UINT8 jh_set_quantiser_table(pImagedec_hdl phdl)
{
	int i = 0;
	int quant_table_id = 0xFF;
	int quant_table_num = 1;
	QUANT_TYPE *q = NULL;
	UINT32 y_c = 0;
	
	for (i = 0;i < phdl->decoder.comps_in_scan; i++)
    	{
		if(phdl->decoder.comp_quant[i] != quant_table_id)
		{
			quant_table_id = phdl->decoder.comp_quant[i];
			q = phdl->decoder.quant[quant_table_id];
			if(0 == quant_table_id)
			{
				y_c = 1;
			}
			else if(1 == quant_table_id)
			{
				y_c = 0;
			}
			else
			{
				jpeg_printf("too many quantiser tables");
				continue;
			}
			jh_hw_quantiser_table(q,y_c);
		}
	}
	return 1;
}

IMAGEDEC_STATIC void jh_set_vld_buf(UINT32 start, UINT32 end)
{
	UINT32 vld = start;
	
	if((start&0x0000000F!=0)||((end+1)&0x0000000F!=0))
		ASSERT(0);
	start &= 0x00FFFFFF;
	start |= ((vld>>24) & 0x7)<<27;  
	JHWriteD(JPEG_HW_VBV_START_ADDR, start);
	JHWriteD(JPEG_HW_VBV_END_ADDR, (end&0x00FFFFFF));
}

IMAGEDEC_STATIC void jh_build_vld_mgr(pImagedec_hdl phdl)
{
	pImagedec_stream_mgr pmgr = &phdl->in_stream;
	pImagedec_hw_vld_mgr pvld_mgr = &pmgr->vld_mgr;
	int byte_skip = 0, bit_left = 0, i = 0, j = 0;

	pvld_mgr->buf_start_addr = (UINT32)pmgr->in_buf;
	pvld_mgr->buf_size = pmgr->in_buf_len;
	pvld_mgr->buf_end_addr = pvld_mgr->buf_start_addr + pvld_mgr->buf_size - 1;
	jh_set_vld_buf(pvld_mgr->buf_start_addr,pvld_mgr->buf_end_addr);

	if(pmgr->in_buf != pmgr->Pin_buf_ofs)
	{
		byte_skip = 2 + (phdl->decoder.bits_left>>3);
		bit_left = phdl->decoder.bits_left;
		j = 1;
		
		if(0xFF == ((phdl->decoder.bit.bit_buf>>16) & 0xFF))
		{
			if(0 == *(volatile UINT8 *)(pmgr->Pin_buf_ofs - j))
			{
				j++;
				byte_skip++;
			}			
		}
		j++;
		
		if(0xFF == ((phdl->decoder.bit.bit_buf>>24) & 0xFF))
		{
			if(0 == *(volatile UINT8 *)(pmgr->Pin_buf_ofs - j))
			{
				j++;
				byte_skip++;
			}			
		}
		j++;
		
		if(16 == bit_left)	
		{
			if(0xFF == ((phdl->decoder.bit.bit_buf) & 0xFF))
			{
				if(0 == *(volatile UINT8 *)(pmgr->Pin_buf_ofs - j))
				{
					j++;
					byte_skip++;
				}			
			}
			j++;

			if(0xFF == ((phdl->decoder.bit.bit_buf>>8) & 0xFF))
			{
				if(0 == *(volatile UINT8 *)(pmgr->Pin_buf_ofs - j))
				{
					j++;
					byte_skip++;
				}			
			}	
			
		}
		else if(8 == bit_left)
		{

			if(0xFF == ((phdl->decoder.bit.bit_buf>>8) & 0xFF))
			{
				if(0 == *(volatile UINT8 *)(pmgr->Pin_buf_ofs - j))
				{
					j++;
					byte_skip++;
				}			
			}
		}
			
		pmgr->in_buf_left += byte_skip;
		if(pmgr->in_buf_left)
		{
			MEMCPY((void *)pvld_mgr->buf_start_addr, (void *)(pmgr->Pin_buf_ofs - byte_skip),pmgr->in_buf_left);      
			osal_cache_flush((void *)pvld_mgr->buf_start_addr, pmgr->in_buf_left);
		}
	}
	
	pvld_mgr->uread = pvld_mgr->buf_start_addr;
	pvld_mgr->valid_size = pmgr->in_buf_left;
	pvld_mgr->uwrite = pvld_mgr->uread + pvld_mgr->valid_size;
	pvld_mgr->used_bits = 0;
	
	pvld_mgr->cfg_length = pvld_mgr->valid_size;
	pvld_mgr->cfg_offset = 0;
	pvld_mgr->cfg_control_bits = 0;

	jpeg_printf("HW vld buf start %x end %x\n",pvld_mgr->buf_start_addr,pvld_mgr->buf_end_addr);
}

inline void jh_reset_dc_value(void)
{
	UINT8 reg_type = JHReadB(JPEG_HW_ACUM_START_29E + 3);

	reg_type &= ~(1<<6);
	reg_type |= 1<<6;
	JHWriteB(JPEG_HW_ACUM_START_29E + 3,reg_type);
}
inline void jh_reg_mcu_coord_y(UINT32 y_mcu)
{
	volatile UINT32 tmp = jh_extra_op_read(JPEG_HW_EXT_MCU_CORD);

	tmp &= ~(0xFFFF<<16);
	jh_extra_op_write(JPEG_HW_EXT_MCU_CORD,(y_mcu<<16)|tmp);
}

inline void jh_reg_mcu_coord_x(UINT32 x_mcu)
{
	volatile UINT32 tmp = jh_extra_op_read(JPEG_HW_EXT_MCU_CORD);

	tmp &= ~0xFFFF;
	jh_extra_op_write(JPEG_HW_EXT_MCU_CORD,tmp|x_mcu);
}

inline UINT8 jh_hw_jpeg_busy(void)
{
	return ((JHReadD(JPEG_HW_ACUM_STATUS_29E)>>9)&0x01);
}

inline UINT8 jh_hw_vld_busy(void)
{
	return ((JHReadD(JPEG_HW_ACUM_STATUS_29E)>>8)&0x01);
}

inline UINT8 jh_hw_request_data(void)
{
	return ((JHReadD(JPEG_HW_ACUM_STATUS_29E)>>2)&0x01);
}


inline UINT8 jh_hw_iqis_buf_full(void)
{
	return ((~(JHReadD(JPEG_HW_ACUM_STATUS_29E)>>15))&0x01);
}

inline void jh_hw_clear_status(void)
{
	JHWriteB(JPEG_HW_ACUM_STATUS_29E,JHReadB(JPEG_HW_ACUM_STATUS_29E) | 0x07);
}

inline UINT32 jh_hw_vld_get_offset(void)
{
	return JHReadD(JPEG_HW_VBV_OFFSET);
}

void jh_hw_reset(void)
{
	osal_interrupt_disable();
	vdec_io_control((struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0), VDEC_IO_RESET_VE_HW,0);
	osal_interrupt_enable();
}

static int check_precision(pImagedec_hdl phdl)
{
	UINT8 h_precision = 0,v_precision = 0;
	UINT32 w = 0, h = 0;

	w = (UINT32)phdl->decoder.image_x_size;h = (UINT32)phdl->decoder.image_y_size;
	
	if(IMAGEDEC_THUMBNAIL == phdl->main.setting.mode)
	{
		while(w > (UINT32)(JPEG_HW_MIN_OUT_WIDTH_THU<<h_precision))
			h_precision++;
		while(h > (UINT32)(JPEG_HW_MIN_OUT_HEIGHT_THU<<v_precision))
			v_precision++;	
		if(h_precision > 3)
		{
			h_precision = 3;
			if((w>>h_precision)>JPEG_HW_MAX_SCALED_WIDTH)
				return -1;
		}
		if(v_precision > 3)
		{
			v_precision = 3;
			if((h>>v_precision)>JPEG_HW_MAX_SCALED_HEIGHT)
				return -1;
		}
	}
	else
	{
		UINT32 scale_w = JPEG_HW_MAX_SCALED_WIDTH;
		UINT32 scale_h = JPEG_HW_MAX_SCALED_HEIGHT;

		if(w > (576<<3))
		{
			return -1;
		}
#ifdef JPEG_CUT_EDGE
		scale_w = JPEG_HW_MAX_SCALED_WIDTH2;
#endif
		if((ANG_90_A == g_imagedec_angle[phdl->id]) || (ANG_90_C == g_imagedec_angle[phdl->id]))
		{
			scale_w = 576;
		}
		while(w > (UINT32)(scale_w<<h_precision))
			h_precision++;
		while(h > (UINT32)(scale_h<<v_precision))
			v_precision++;
		if((h_precision > 3) || (v_precision > 3))
			return -1;
	}

	phdl->imageout.hw_acce_rect.uStartX = phdl->imageout.hw_acce_rect.uStartY = 0;
	phdl->imageout.hw_acce_rect.uWidth = (w>>h_precision)&(~0x03);
	phdl->imageout.hw_acce_rect.uHeight = (h>>v_precision)&(~0x01);
	phdl->imageout.hw_h_pre = h_precision;
	phdl->imageout.hw_v_pre = v_precision;
#ifdef JPEG_CUT_EDGE
	phdl->imageout.hw_acc_stride = JPEG_HW_MAX_SCALED_WIDTH2;
#else
	phdl->imageout.hw_acc_stride = JPEG_HW_DEC_BUF_STRIDE;
#endif
	return 1;
}

static int check_precision_ext(pImagedec_hdl phdl)
{
	UINT8 h_precision = 0,v_precision = 0;
	UINT32 w = 0, h = 0;

	w = (UINT32)phdl->decoder.image_x_size;h = (UINT32)phdl->decoder.image_y_size;
		
	if(IMAGEDEC_THUMBNAIL == phdl->main.setting.mode)
	{
		while(w > (UINT32)(JPEG_HW_MIN_OUT_WIDTH_THU<<h_precision))
			h_precision++;
		while(h > (UINT32)(JPEG_HW_MIN_OUT_HEIGHT_THU<<v_precision))
			v_precision++;	
	}
	else
	{
		UINT32 scale_w = JPEG_HW_MAX_SCALED_WIDTH;
		UINT32 scale_h = JPEG_HW_MAX_SCALED_HEIGHT;

#ifdef JPEG_CUT_EDGE
		scale_w = JPEG_HW_MAX_SCALED_WIDTH2;
#endif
		if((ANG_90_A == g_imagedec_angle[phdl->id]) || (ANG_90_C == g_imagedec_angle[phdl->id]))
		{
			scale_w = 576;
		}
		while(w > (UINT32)(scale_w<<h_precision))
			h_precision++;
		while(h > (UINT32)(scale_h<<v_precision))
			v_precision++;
	}

	phdl->imageout.hw_max_divide_v = phdl->imageout.hw_max_divide_h = 0;
	
	if(h_precision > 3)
	{
		h_precision = 3;
		if(phdl->decoder.mcus_per_row > JPEG_HW_MAX_MCU_VALUE)		
		{
			phdl->imageout.hw_max_divide_h = phdl->decoder.mcus_per_row / JPEG_HW_MAX_MCU_VALUE;
			phdl->imageout.last_mcu_row = 0;
			phdl->imageout.divide_mcu_row = 0;	
		}
	}

	if(v_precision > 3)
	{
		v_precision = 3;
		if(phdl->decoder.mcus_per_col > JPEG_HW_MAX_MCU_VALUE)		
		{
			phdl->imageout.hw_max_divide_v = phdl->decoder.mcus_per_col / JPEG_HW_MAX_MCU_VALUE;
			phdl->imageout.last_mcu_col = 0;
			phdl->imageout.divide_mcu_col = 0;	
		}
	}

	switch(phdl->decoder.scan_type)
	{
		case JPGD_YH1V1:	//	4:4:4
			phdl->imageout.pixel_per_mcu_h = 3;
			phdl->imageout.pixel_per_mcu_v = 3;
			break;
		case JPGD_YH2V1:	// 4:2:2
			phdl->imageout.pixel_per_mcu_h = 4;
			phdl->imageout.pixel_per_mcu_v = 3;
			break;
		case JPGD_YH2V2:	// 4:2:0
			phdl->imageout.pixel_per_mcu_h = 4;
			phdl->imageout.pixel_per_mcu_v = 4;
			break;
		case JPGD_YH1V2:	// 4:2:0
			phdl->imageout.pixel_per_mcu_h = 3;
			phdl->imageout.pixel_per_mcu_v = 4;
			break;					
		case JPGD_YH4V1:	// 4:1:1
			phdl->imageout.pixel_per_mcu_h = 5;
			phdl->imageout.pixel_per_mcu_v = 3;
			break;
		default:
			return -1;
	}


	phdl->imageout.hw_acce_rect.uStartX = phdl->imageout.hw_acce_rect.uStartY = 0;
	phdl->imageout.hw_acce_rect.uWidth = (w>>h_precision)&(~0x03);
	phdl->imageout.hw_acce_rect.uHeight = (h>>v_precision)&(~0x01);
	phdl->imageout.hw_h_pre = h_precision;
	phdl->imageout.hw_v_pre = v_precision;
	phdl->imageout.hw_acc_stride = JPEG_HW_MAX_SCALED_WIDTH_EXT;
	return 1;
}

static int check_precision_ext2(pImagedec_hdl phdl)
{
	UINT8 h_precision = 0,v_precision = 0;
	UINT32 w = 0, h = 0;

	w = (UINT32)phdl->decoder.image_x_size;h = (UINT32)phdl->decoder.image_y_size;
	
	if(IMAGEDEC_THUMBNAIL == phdl->main.setting.mode)
		return -1;
	
	UINT32 scale_size = JPEG_HW_MAX_SCALED_SIZE_EXT2;

	while(w > (UINT32)(scale_size<<h_precision))
		h_precision++;
	while(h > (UINT32)(scale_size<<v_precision))
		v_precision++;

	if(h_precision > 3)
	{
		h_precision = 3;	
	}

	if(v_precision > 3)
		v_precision = 3;

	phdl->imageout.hw_max_divide_h = 0;
	if(phdl->decoder.mcus_per_row > JPEG_HW_MAX_MCU_VALUE)		
	{
		jpeg_printf("extend the hw limitation %d\n", phdl->decoder.mcus_per_row);
		phdl->imageout.hw_max_divide_h = phdl->decoder.mcus_per_row / JPEG_HW_MAX_MCU_VALUE;
		phdl->imageout.last_mcu_row = 0;
		phdl->imageout.divide_mcu_row = 0;	
	}	
		
	phdl->imageout.combine_hw_mcu_col_cnt = 0;
	phdl->imageout.combine_hw_mcu_row_cnt = 0;
	
	switch(phdl->decoder.scan_type)
	{
		case JPGD_YH1V1:	//	4:4:4
			phdl->imageout.pixel_per_mcu_h = 3;	
			phdl->decoder.out_type = JPGD_YH2V1;
			phdl->decoder.out_blocks_per_mcu = 4;
			phdl->imageout.combine_hw_mcu_col_max = JPEG_HW_MacroBlock_NUM * 2 * (1<<v_precision);
			phdl->imageout.combine_hw_mcu_row_max = phdl->decoder.mcus_per_row>>( 1 + h_precision);
			break;
		case JPGD_YH2V1:	// 4:2:2
			phdl->imageout.pixel_per_mcu_h = 4;		
			phdl->decoder.out_type = JPGD_YH2V1;	
			phdl->decoder.out_blocks_per_mcu = 4;	
			phdl->imageout.combine_hw_mcu_col_max = JPEG_HW_MacroBlock_NUM * 2 * (1<<v_precision);
			phdl->imageout.combine_hw_mcu_row_max = phdl->decoder.mcus_per_row>>h_precision;
			break;
		case JPGD_YH2V2:	// 4:2:0
			phdl->imageout.pixel_per_mcu_h = 4;	
			phdl->decoder.out_type = JPGD_YH2V2;		
			phdl->decoder.out_blocks_per_mcu = 6;			
			phdl->imageout.combine_hw_mcu_col_max = JPEG_HW_MacroBlock_NUM * (1<<v_precision);
			phdl->imageout.combine_hw_mcu_row_max = phdl->decoder.mcus_per_row>>h_precision;
			break;
		case JPGD_YH1V2:	// 4:2:0
			phdl->imageout.pixel_per_mcu_h = 3;	
			phdl->decoder.out_type = JPGD_YH2V2;		
			phdl->decoder.out_blocks_per_mcu = 6;			
			phdl->imageout.combine_hw_mcu_col_max = JPEG_HW_MacroBlock_NUM * (1<<v_precision);
			phdl->imageout.combine_hw_mcu_row_max = phdl->decoder.mcus_per_row>>( 1 + h_precision);			
			break;					
		case JPGD_YH4V1:	// 4:1:1
			phdl->imageout.pixel_per_mcu_h = 5;	
			phdl->decoder.out_type = JPGD_YH4V1;	
			phdl->decoder.out_blocks_per_mcu = 6;			
			phdl->imageout.combine_hw_mcu_col_max = JPEG_HW_MacroBlock_NUM * 2 * (1<<v_precision);
			phdl->imageout.combine_hw_mcu_row_max = phdl->decoder.mcus_per_row>>h_precision;			
			break;
		default:
			return -1;
	}
	
	phdl->imageout.hw_acce_rect.uStartX = phdl->imageout.hw_acce_rect.uStartY = 0;
	phdl->imageout.hw_acce_rect.uWidth = (w>>h_precision)&(~0x03);
	phdl->imageout.hw_acce_rect.uHeight = (h>>v_precision)&(~0x01);
	phdl->imageout.hw_h_pre = h_precision;
	phdl->imageout.hw_v_pre = v_precision;
	phdl->imageout.hw_acc_stride = JPEG_HW_MAX_SCALED_WIDTH_EXT;

	jpeg_printf("hw width %d height %d p_h %d p_v %d\n", phdl->imageout.hw_acce_rect.uWidth
		, phdl->imageout.hw_acce_rect.uHeight, phdl->imageout.hw_h_pre
		, phdl->imageout.hw_v_pre);
	return 1;
}

void jh_frame_addr(UINT32 y_addr,UINT32 c_addr)
{
	jh_extra_op_write(JPEG_HW_EXT_MEM_BASE_Y,(y_addr & 0xFFFFFFF));
	jh_extra_op_write(JPEG_HW_EXT_MEM_BASE_C,(c_addr & 0xFFFFFFF));
}

//jpeg picture max size is JPEG_HW_MAX_SCALED_WIDTHX8  -- JPEG_HW_MAX_SCALED_HEIGHTX8
//						5760X7200
int jh_hw_init(pImagedec_hdl phdl)
{
	UINT32 w = 0, h = 0;

	w = (UINT32)phdl->decoder.image_x_size;h = (UINT32)phdl->decoder.image_y_size;
	if((w < 8) || (h < 8))
	{
		return -1;
	}
	
	if(g_imagedec_m33_extend_hw_vld)
	{
		if(check_precision_ext(phdl) < 0)
			return -1;
	}
	else
	{
		
		if(check_precision(phdl) < 0)
		{
			if(g_imagedec_m33_enable_hw_vld)
			{
				if(check_precision_ext2(phdl) < 0)
					return -1;

				jpeg_printf("\n combine sw and hw together! \n ");
				g_imagedec_m33_combine_hw_sw_flag = 1;				
			}
			else
				return -1;
		}
	}
	
if((sys_ic_is_M3202() && (sys_ic_get_rev_id() > IC_REV_2))
	|| ((ALI_M3329E == sys_ic_get_chip_id()) && (sys_ic_get_rev_id() > IC_REV_4)))
        g_imagedec_m33_reg_base_addr = JPEG_HW_ACUM_REG_BASE_29E5;
    else
        g_imagedec_m33_reg_base_addr = JPEG_HW_ACUM_REG_BASE_29E;

	jh_hw_reset();	
    jh_hw_clear_status();
	jh_pic_chroma_format(phdl->decoder.scan_type);
	jh_set_jpg_hw_work_mode(g_imagedec_m33_enable_hw_vld);
	//set the quantiser table
	jh_set_quantiser_table(phdl);
#if 1
	jh_mem_base((UINT32)(phdl->imageout.hw_acc_frm_y));
	jh_frame_addr((UINT32)(phdl->imageout.hw_acc_frm_y),(UINT32)(phdl->imageout.hw_acc_frm_c));
#else
	jh_mem_base((UINT32)(phdl->imageout.frm_y_addr));
	jh_frame_addr((UINT32)(phdl->imageout.frm_y_addr),(UINT32)(phdl->imageout.frm_c_addr));
#endif
	jh_reg_control(phdl->imageout.hw_acc_stride,phdl->decoder.mcus_per_row,phdl->imageout.hw_h_pre,phdl->imageout.hw_v_pre);
	jh_reg_mcu_coord_y(0);
	jh_reg_mcu_coord_x(0);
	jh_reset_dc_value();
	phdl->main.setting.hw_acc_flag = 1;
	
	if(g_imagedec_m33_enable_hw_vld)
		jh_build_vld_mgr(phdl);

	return 0;
}

void jh_m33_hw_set_hw_vld_table(pImagedec_hdl phdl)
{	
	int y_dc_idx = phdl->decoder.comp_dc_tab[0];
	int y_ac_idx = phdl->decoder.comp_ac_tab[0];
	int c_dc_idx = phdl->decoder.comp_dc_tab[1];
	int c_ac_idx = phdl->decoder.comp_ac_tab[1];	
	int i;
	UINT32 tmp;	
	
	//maxcode table
	for(i=0;i<16;i++)
	{
		tmp = (i << 16) | (phdl->decoder.hw_vld_max_code[y_dc_idx][i]);
		JHWriteD(JPEG_HW_ACUM_VLD_HUF_TAB_29E,tmp);
	}

	for(i=0;i<16;i++)
	{
		tmp = ((i + 16) << 16) | (phdl->decoder.hw_vld_max_code[c_dc_idx][i]);
		JHWriteD(JPEG_HW_ACUM_VLD_HUF_TAB_29E,tmp);
	}

	for(i=0;i<16;i++)
	{
		tmp = ((i + 32) << 16) | (phdl->decoder.hw_vld_max_code[y_ac_idx][i]);
		JHWriteD(JPEG_HW_ACUM_VLD_HUF_TAB_29E,tmp);
	}	
	
	for(i=0;i<16;i++)
	{
		tmp = ((i + 48) << 16) | (phdl->decoder.hw_vld_max_code[c_ac_idx][i]);
		JHWriteD(JPEG_HW_ACUM_VLD_HUF_TAB_29E,tmp);
	}	

	//value table
	for(i=0;i<16;i++)
	{
		tmp = ((i + 64) << 16) | (phdl->decoder.hw_vld_valptr[y_dc_idx][i]);
		JHWriteD(JPEG_HW_ACUM_VLD_HUF_TAB_29E,tmp);
	}

	for(i=0;i<16;i++)
	{
		tmp = ((i + 64 + 16) << 16) | (phdl->decoder.hw_vld_valptr[c_dc_idx][i]);
		JHWriteD(JPEG_HW_ACUM_VLD_HUF_TAB_29E,tmp);
	}
	
	for(i=0;i<16;i++)
	{
		tmp = ((i + 64 + 32) << 16) | (phdl->decoder.hw_vld_valptr[y_ac_idx][i]);
		JHWriteD(JPEG_HW_ACUM_VLD_HUF_TAB_29E,tmp);
	}
	
	for(i=0;i<16;i++)
	{
		tmp = ((i + 64 + 48) << 16) | (phdl->decoder.hw_vld_valptr[c_ac_idx][i]);
		JHWriteD(JPEG_HW_ACUM_VLD_HUF_TAB_29E,tmp);
	}	

	//symbol table
	for(i=0;i<8;i++)
	{
		tmp = ((i + 256) << 16) | ((phdl->decoder.hw_vld_symbol[y_dc_idx][i<<1]) 
          | ((phdl->decoder.hw_vld_symbol[y_dc_idx][(i<<1) + 1])<<8));
		JHWriteD(JPEG_HW_ACUM_VLD_HUF_TAB_29E,tmp);
	}

	for(i=0;i<8;i++)
	{
		tmp = ((i + 256 + 8) << 16) | ((phdl->decoder.hw_vld_symbol[c_dc_idx][i<<1]) 
          | ((phdl->decoder.hw_vld_symbol[c_dc_idx][(i<<1) + 1])<<8));
		JHWriteD(JPEG_HW_ACUM_VLD_HUF_TAB_29E,tmp);
	}
	
	for(i=0;i<81;i++)
	{
		tmp = ((i + 256 + 16) << 16) | ((phdl->decoder.hw_vld_symbol[y_ac_idx][i<<1]) 
          | ((phdl->decoder.hw_vld_symbol[y_ac_idx][(i<<1) + 1])<<8));
		JHWriteD(JPEG_HW_ACUM_VLD_HUF_TAB_29E,tmp);
	}
	
	for(i=0;i<81;i++)
	{
		tmp = ((i + 256 + 97) << 16) | ((phdl->decoder.hw_vld_symbol[c_ac_idx][i<<1]) 
          | ((phdl->decoder.hw_vld_symbol[c_ac_idx][(i<<1) + 1])<<8));
		JHWriteD(JPEG_HW_ACUM_VLD_HUF_TAB_29E,tmp);
	}
}

void jh_m33_config_hw_vld(pImagedec_hw_vld_mgr pmgr)
{
	UINT8   bcontrolbit = pmgr->cfg_control_bits;
	UINT32 uoffset = pmgr->cfg_offset;
	UINT32 uLength = pmgr->cfg_length;
	UINT32 buf_start = pmgr->buf_start_addr;
	UINT32 buf_size = pmgr->buf_size;
	UINT32 used_bits = pmgr->used_bits;
	
/*	if((uoffset+uLength)>buf_size)
	{
		osal_cache_flush(buf_start+uoffset, buf_size-uoffset);
		osal_cache_flush(buf_start, uoffset+uLength-buf_size);
	}
	else
		osal_cache_flush(buf_start+uoffset, uLength);
*/		
	
	JHWriteD(JPEG_HW_VBV_OFFSET,(((uoffset<<3)+used_bits)&0xFFFFFF));
	JHWriteD(JPEG_HW_VBV_VLD_LENGTH,((uLength<<3)&0xFFFFFF));
	JHWriteD(JPEG_HW_VBV_START_ADDR,(JHReadD(JPEG_HW_VBV_START_ADDR)&0x38FFFFFF)|((bcontrolbit&0x07)<<24) );
}

inline void jh_input_rlc_data(UINT32 run, int level)
{
	if(level<0)
	{
		level = (~level) + 1;
		level &= 0x7FF;
		level |= 1<<11;
	}
	else
		level &= 0x7FF;
	if((level >= (int)2<<11) || (level <= - (int)(2<<11)))
		return;
	run &= 0xF;
	JHWriteD(JPEG_HW_ACUM_INPUT_29E,(run<<12)|level);	
}
inline void jh_mcu_start(void)
{
	volatile UINT8 reg_type = 0;

	reg_type |= 4;
	JHWriteB(JPEG_HW_ACUM_START_29E,reg_type);
}

inline void jh_hw_vld_mcu_start(void)
{
	volatile UINT8 reg_type = 0;

	reg_type |= 0xc;
	JHWriteB(JPEG_HW_ACUM_START_29E,reg_type);
}

inline void jh_unit_start(void)
{
	UINT8 reg_type = 0;

	//jpeg_printf("new unit\n");
	reg_type |= 0x06;
	JHWriteB(JPEG_HW_ACUM_START_29E,reg_type);
}

