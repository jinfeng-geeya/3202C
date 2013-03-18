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
#include <hld/hld_dev.h>
#include <hld/dis/vpo.h>
#include "imagedec_main.h"
#include "imagedec_osd.h"

extern int error_code;
extern Imagedec_Init_Config g_imagedec_config[IMAGEDEC_MAX_INSTANCE];
extern volatile bool g_imagedec_pro_show[IMAGEDEC_MAX_INSTANCE];
extern volatile OSAL_ID g_imagedec_wait_flag[IMAGEDEC_MAX_INSTANCE]; 
extern volatile int g_imagedec_need_logo_info[IMAGEDEC_MAX_INSTANCE];
extern volatile bool g_imagedec_dis_flag[IMAGEDEC_MAX_INSTANCE];
extern volatile int g_imagedec_enable_hw_acce[IMAGEDEC_MAX_INSTANCE];
extern enum IMAGE_ANGLE g_imagedec_angle[IMAGEDEC_MAX_INSTANCE];

extern imagedec_osd_ins g_imagedec_osd_ins;
extern imagedec_copy_frm_ins g_imagedec_copy_frm_ins;

extern int g_imagedec_m33_enable_hw_vld;
extern int g_imagedec_m33_extend_hw_vld;

IMAGEDEC_STATIC void imagedec_zoom_setting(pImagedec_hdl phdl)
{
	struct vpo_device *dev = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS,0);
	UINT32 tv_aspect = 0;
	
	if(IMAGEDEC_REAL_SIZE == phdl->main.setting.mode)
	{
		/*position and size should be 16 aligned. src size is equal to the dis*/
		if((phdl->main.api_cmd.src.uWidth != phdl->main.api_cmd.dis.uWidth) ||	\
			(phdl->main.api_cmd.src.uHeight != phdl->main.api_cmd.dis.uHeight) || \
			(0 != (phdl->main.api_cmd.src.uStartX&0x7)) || (0 != (phdl->main.api_cmd.src.uStartY&0x7)) ||  \
			(0 != (phdl->main.api_cmd.src.uWidth&0x7))  || (0 != (phdl->main.api_cmd.src.uHeight&0x7)))
			{JPEG_ASSERT(0);}
	}
	MEMCPY(&(phdl->main.setting.src),&(phdl->main.api_cmd.src),sizeof(struct Rect));
	MEMCPY(&(phdl->main.setting.dis),&(phdl->main.api_cmd.dis),sizeof(struct Rect));
	phdl->main.setting.dis_setting_update = 1;
	vpo_ioctl(dev,VPO_IO_GET_TV_ASPECT,(UINT32)&tv_aspect);
	/*operation when 16:9 mode*/
	if((1 == tv_aspect) && (IMAGEDEC_THUMBNAIL == phdl->main.setting.mode))
	{
		phdl->main.setting.dis.uStartX = (phdl->main.setting.dis.uStartX - 90) * 704 / 540 + 16;// + (phdl->main.setting.dis.uWidth>>2);
		phdl->main.setting.dis.uWidth = phdl->main.setting.dis.uWidth * 704 /540;
		if(phdl->main.setting.dis.uStartX + phdl->main.setting.dis.uWidth > TV_MAX_WIDTH)
			phdl->main.setting.dis.uWidth = TV_MAX_WIDTH - 2 - phdl->main.setting.dis.uStartX;
	}
}

IMAGEDEC_STATIC bool imagedec_mode_setting(pImagedec_hdl phdl)
{
	if((!phdl->main.api_cmd.mode) || (phdl->main.api_cmd.mode > 4))
		return FALSE;
	phdl->main.setting.mode = phdl->main.api_cmd.mode;
	phdl->app.item[M_APP1&0xF].processor = NULL;
	phdl->app.item[M_APP1&0xF].par = NULL;
	phdl->app.item[M_APP1&0xF].active = FALSE;
	switch(phdl->main.setting.mode)
	{
		case IMAGEDEC_FULL_SRN:
			break;
		case IMAGEDEC_REAL_SIZE:
			break;
		case IMAGEDEC_THUMBNAIL:
			phdl->app.item[M_APP1&0xF].processor = app_exif_decoder;
			phdl->app.item[M_APP1&0xF].active = TRUE;	
			break;
		case IMAGEDEC_AUTO:
			break;
		default:
			return FALSE;
	}
	imagedec_zoom_setting(phdl);
	
	if(IMAGEDEC_FULL_SRN != phdl->main.setting.mode)
	{
		// close the osd mode
		phdl->main.osd_mode_update = 1;
		g_imagedec_osd_ins.api_cmd.on = 0;
	}
	
	return TRUE;
}

IMAGEDEC_STATIC bool imagedec_show_mode_setting(pImagedec_hdl phdl)
{
	phdl->main.setting.show_mode = phdl->main.api_cmd.show_mode;
	MEMCPY((void *)phdl->main.setting.show_mode_par,(void *)phdl->main.api_cmd.show_mode_par,IMAGEDEC_SHOW_MODE_PAR_LEN);

	return 1;
}

IMAGEDEC_STATIC void imagedec_osd_mode_setting(pImagedec_hdl phdl,pimagedec_osd_ins posd)
{
	//if(IMAGEDEC_FULL_SRN == phdl->main.setting.mode)
	{
		posd->on = posd->api_cmd.on;

		if(posd->on)
		{
			g_imagedec_pro_show[phdl->id] = FALSE;
			posd->out.dis_rect.uStartX = posd->api_cmd.dis_rect.uStartX;
			posd->out.dis_rect.uStartY = posd->api_cmd.dis_rect.uStartY;
			posd->out.dis_rect.uWidth = posd->api_cmd.dis_rect.uWidth;
			posd->out.dis_rect.uHeight = posd->api_cmd.dis_rect.uHeight;
			if(posd->api_cmd.bitmap)
			{
				posd->out.bitmap = (posd->api_cmd.bitmap & 0xFFFFFFF) | 0x80000000;
				posd->out.buf_size = posd->api_cmd.buf_size;
			}
			else
			{
				jpeg_printf("OSD bitmap buff is NULL\n");
				error_code = JPGD_OSD_BITMAP_ERROR;
			}
		}	
	}
}

void imagedec_sub_buffer_init(pImagedec_hdl phdl,pImagedec_Init_Config pconfig)
{	
	phdl->imageout.frm_y_len = pconfig->frm_y_size;
	phdl->imageout.frm_y_addr = (UINT8 *)((pconfig->frm_y_addr&0x0FFFFFFF) | 0x80000000);
	phdl->imageout.frm_c_len = pconfig->frm_c_size;
	phdl->imageout.frm_c_addr = (UINT8 *)((pconfig->frm_c_addr&0x0FFFFFFF) | 0x80000000);
	phdl->imageout.frm[0].frm_y_len = pconfig->frm_y_size;
	phdl->imageout.frm[0].frm_y_addr = (UINT8 *)((pconfig->frm_y_addr&0x0FFFFFFF) | 0x80000000);
	phdl->imageout.frm[0].frm_c_len = pconfig->frm_c_size;
	phdl->imageout.frm[0].frm_c_addr = (UINT8 *)((pconfig->frm_c_addr&0x0FFFFFFF) | 0x80000000);
	phdl->imageout.frm[0].busy = 0;
	phdl->imageout.frm[1].frm_y_len = pconfig->frm2_y_size;
	phdl->imageout.frm[1].frm_y_addr = (UINT8 *)((pconfig->frm2_y_addr&0x0FFFFFFF) | 0x80000000);
	phdl->imageout.frm[1].frm_c_len = pconfig->frm2_c_size;
	phdl->imageout.frm[1].frm_c_addr = (UINT8 *)((pconfig->frm2_c_addr&0x0FFFFFFF) | 0x80000000);
	phdl->imageout.frm[1].busy = 0;
	phdl->imageout.frm_mb_type = pconfig->frm_mb_type;
	phdl->in_stream.in_buf = (UINT8 *)(((UINT32)(pconfig->decoder_buf)&0x0FFFFFFF) | (1<<31));
	phdl->in_stream.in_buf = (UINT8 *)(((UINT32)phdl->in_stream.in_buf + 15) & ~0xF); // input buf should be 16bytes aligned
	phdl->in_stream.in_buf_len = JPGD_INBUFSIZE;
	phdl->in_stream.buf_item[0].in_buf = phdl->in_stream.in_buf;
	phdl->in_stream.buf_item[0].in_buf_len = JPGD_INBUFSIZE;
	phdl->in_stream.buf_item[0].busy = 0;
#ifndef IMAGEDEC_SINGLE_INPUT_BUF
	phdl->in_stream.buf_item[1].in_buf = phdl->in_stream.in_buf + JPGD_INBUFSIZE + 256;
	phdl->in_stream.buf_item[1].in_buf_len = JPGD_INBUFSIZE;
	phdl->in_stream.buf_item[1].busy = 0;
#else
	phdl->in_stream.buf_item[1].in_buf = phdl->in_stream.in_buf;
	phdl->in_stream.buf_item[1].in_buf_len = 0;
	phdl->in_stream.buf_item[1].busy = 0;
#endif
	phdl->in_stream.buf_idx_using = 0;

	g_imagedec_m33_extend_hw_vld = 0;
	if(g_imagedec_enable_hw_acce[phdl->id])
	{
		phdl->mem.mem_pos = phdl->mem.mem_start = (unsigned long)phdl->in_stream.buf_item[1].in_buf + JPGD_INBUFSIZE + 256;
		if(g_imagedec_m33_enable_hw_vld)
		{
			if(pconfig->decoder_buf_len > ((JPGD_HWBUFYSIZE2<<1) + 0x200000))
			{
				g_imagedec_m33_extend_hw_vld = 1;
				phdl->imageout.hw_acc_frm_y = (UINT32)(pconfig->decoder_buf + pconfig->decoder_buf_len - JPGD_HWBUFYSIZE2)&(~0x1FF);
				phdl->imageout.hw_acc_frm_c = (phdl->imageout.hw_acc_frm_y - JPGD_HWBUFYSIZE2)&(~0x1FF);
			}
			else
			{
				phdl->imageout.hw_acc_frm_y = (UINT32)(pconfig->decoder_buf + pconfig->decoder_buf_len - JPGD_HWBUFYSIZE)&(~0x1FF);
				phdl->imageout.hw_acc_frm_c = (phdl->imageout.hw_acc_frm_y - JPGD_HWBUFYSIZE)&(~0x1FF);
			}

		}
		else
		{	phdl->imageout.hw_acc_frm_y = (UINT32)(pconfig->decoder_buf + pconfig->decoder_buf_len - JPGD_HWBUFYSIZE)&(~0x1FF);
			phdl->imageout.hw_acc_frm_c = (phdl->imageout.hw_acc_frm_y - JPGD_HWBUFYSIZE)&(~0x1FF);
		}
		phdl->mem.mem_up_limit = (unsigned long)phdl->imageout.hw_acc_frm_c; 
	}
	else
	{
		phdl->mem.mem_pos = phdl->mem.mem_start = (unsigned long)phdl->in_stream.buf_item[1].in_buf + JPGD_INBUFSIZE + 256;			
		phdl->mem.mem_up_limit = (unsigned long)(pconfig->decoder_buf + pconfig->decoder_buf_len); 
	}
	jhconvs_buffer_init();
	phdl->imageout.new_frm.frm_y_len = pconfig->frm3_y_size;
	phdl->imageout.new_frm.frm_y_addr = (UINT8 *)((pconfig->frm3_y_addr&0x0FFFFFFF) | 0x80000000);
	phdl->imageout.new_frm.frm_c_len = pconfig->frm3_c_size;
	phdl->imageout.new_frm.frm_c_addr = (UINT8 *)((pconfig->frm3_c_addr&0x0FFFFFFF) | 0x80000000);
	phdl->imageout.new_frm.busy = 0;
}
/*Description : 	response from api and set the imagedec mode
*  Parameters :	pImagedec_hdl phdel --- pointer of imagedec
*  Return 	:	FALSE or TRUE
*/
void imagedec_sub_response_to_api(pImagedec_hdl phdl)
{
	JPEG_ENTRY;
	if(phdl->main.mode_update)
	{
		phdl->main.mode_update = 0;
		if(!imagedec_mode_setting(phdl))
			{JPEG_ASSERT(0);}
		if(phdl->main.show_mode_update)
		{
			phdl->main.show_mode_update = 0;
			if(!imagedec_show_mode_setting(phdl))
				{JPEG_ASSERT(0);}
		}
	}
	if(phdl->main.zoom_update)
	{
		phdl->main.zoom_update = 0;
		imagedec_zoom_setting(phdl);
	}
		
	if(phdl->main.rot_par_update)
	{
		phdl->main.rot_par_update = 0;
		/*rotate operation is only active when decoder run at Full Screen Mode*/
		if(IMAGEDEC_FULL_SRN == phdl->main.setting.mode)
			g_imagedec_angle[phdl->id] = phdl->main.api_cmd.angle;
	}

	if(phdl->main.fill_logo_update)
	{
		phdl->main.fill_logo_update = 0;
		phdl->main.setting.fill_logo = 1;
		phdl->main.setting.fill_color = phdl->main.api_cmd.fill_color;
	}
	else
		phdl->main.setting.fill_logo = 0;

	if(phdl->main.osd_mode_update)
	{
		phdl->main.osd_mode_update = FALSE;
		imagedec_osd_mode_setting(phdl,&g_imagedec_osd_ins);
#if 1	
		if(g_imagedec_osd_ins.on) // disable the HW accelerator function
		{
			phdl->main.hw_act_update = 1;
			phdl->main.api_cmd.hw_acc_flag = 0;
		}
		else
		{
			phdl->main.hw_act_update = 1;
			phdl->main.api_cmd.hw_acc_flag = 1;			
		}
#endif
	}

	if(phdl->main.hw_act_update)
	{
		phdl->main.hw_act_update = 0;
		g_imagedec_enable_hw_acce[phdl->id] = phdl->main.api_cmd.hw_acc_flag;
		imagedec_sub_buffer_init(phdl, &g_imagedec_config[phdl->id]);
	}

	if(phdl->main.copy_frm_update)
	{
		pImagedec_Io_Cmd_Copy_Frm pfrm = &(g_imagedec_copy_frm_ins.input_par);
		
		phdl->main.copy_frm_update = 0;
		g_imagedec_copy_frm_ins.on = 1;
		g_imagedec_copy_frm_ins.logo_buf = pfrm->logo_buf;
		g_imagedec_copy_frm_ins.y_buf_size = pfrm->y_buf_size;
		g_imagedec_copy_frm_ins.c_buf_size = pfrm->c_buf_size;
		g_imagedec_copy_frm_ins.width = pfrm->width;
		g_imagedec_copy_frm_ins.height = pfrm->height;
	}

	if((sys_ic_is_M3202() && (sys_ic_get_rev_id() > IC_REV_2))
		|| ((!sys_ic_is_M3202()) && (sys_ic_get_rev_id() > IC_REV_4)))
		g_imagedec_m33_enable_hw_vld = 1;
	else
		g_imagedec_m33_enable_hw_vld = 0;
	
	JPEG_EXIT;
}
#ifndef IMAGEDEC_SINGLE_INPUT_BUF
void imagedec_fill_in_buffer_task(UINT32 upara1,UINT32 upara2)
{
	pImagedec_hdl phdl = (pImagedec_hdl)upara1;
	OSAL_ID wait_flg = (OSAL_ID)upara2;
	UINT32 ptn_flag;
	ER s_flag;

	while(1)
	{
		//soc_printf("wait read flag....\n");
		s_flag = osal_flag_wait(&ptn_flag,g_imagedec_wait_flag[phdl->id],IMAGEDEC_FILL_IN_BUF_PTN,TWF_ANDW,IMAGEDEC_TIME_OUT_READ);	
		if(E_OK != s_flag)
		{
			jpeg_printf("wait flag error<%d><%s>\n",s_flag,__FUNCTION__);
		}
		else
			{osal_flag_clear(g_imagedec_wait_flag[phdl->id],IMAGEDEC_FILL_IN_BUF_PTN);}
		jpeg_printf("wait read flag ok<%x>\n",ptn_flag);
		imagedec_fill_in_buffer(phdl,1 - phdl->in_stream.buf_idx_using);
	}
}
#endif
bool imagedec_sub_file_pos_init(pImagedec_hdl phdl)
{
	if (phdl->in_stream.fseek(phdl->fh,0, SEEK_SET) < 0) 
	{
		jpeg_printf("file seek error\n");
		return FALSE;
	}	
	return TRUE;
}
imagedec_file_format imagedec_sub_judge_file_format(pImagedec_hdl phdl)
{
	imagedec_file_format file_format = 0;
	UINT32 file_marker;
	UINT8 file_marker1 = 0,file_marker2 = 0;
	int ret = 0;

	if(!imagedec_sub_file_pos_init(phdl))
		return FALSE;
	error_code = 0;
	ret = phdl->in_stream.fread(phdl->fh,(UINT8 *)&file_marker, 4);
	if(4 != ret)
	{
		jpeg_printf("read file error\n");
		return FALSE;
	}
	file_marker1 = file_marker&0xFF;
	file_marker2 = (file_marker>>8)&0xFF;
	if((0x42 == file_marker1) && (0x4d == file_marker2))	
		file_format = Bitmap;
	else if((0xFF == file_marker1) && (0xD8 == file_marker2))
		file_format = JPEG;
	else
	{
		error_code = JPGD_WRONG_FILE_FLAG;
		return FALSE;
	}
	jpeg_printf("file format <%d>\n",file_format);
	return file_format;
}
bool imagedec_get_file_info(pImagedec_hdl phdl,file_h fh)
{
	imagedec_file_format format = JPEG;

	phdl->fh = fh;
	format = imagedec_sub_judge_file_format(phdl);
	if(!imagedec_sub_file_pos_init(phdl))
				return FALSE;
	switch(format)
	{
		case JPEG:
			jpeg_quick_parsing_sof(phdl);
			break;
		case Bitmap:
			imagedec_bmp_init(phdl);
			break;
		default:
			return FALSE;
	}
	if(IsTerminated())
		return FALSE;
	phdl->info.coding_progress = 0;
	phdl->file_format = format;
	return TRUE;
}

void imagedec_sub_set_pic_size(struct Display_Info *pDisplay_info,pImagedec_hdl phdl)
{
		if(IMAGEDEC_THUMBNAIL != phdl->main.setting.mode)
		{
			if((1 == phdl->main.setting.hw_acc_flag) && (IMAGEDEC_FULL_SRN == phdl->main.setting.mode))
			{
				pDisplay_info->pVideoInfo->width = phdl->imageout.image_out_w;
				pDisplay_info->pVideoInfo->height = phdl->imageout.image_out_h;
				pDisplay_info->stride = (phdl->imageout.image_output_stride+0xF)>>4;
				jpeg_printf("DE w <%d> h <%d>\n",pDisplay_info->pVideoInfo->width,pDisplay_info->pVideoInfo->height);
				if((0 != MEMCMP((void *)&pDisplay_info->src_rect,(void *)&phdl->imageout.de_src_rect,sizeof(struct Rect)))
					|| (0 !=  MEMCMP((void *)&pDisplay_info->dst_rect,(void *)&phdl->imageout.de_dst_rect,sizeof(struct Rect))))
				{
					MEMCPY(&(pDisplay_info->src_rect),&(phdl->imageout.de_src_rect),sizeof(struct Rect));
					MEMCPY(&(pDisplay_info->dst_rect),&(phdl->imageout.de_dst_rect),sizeof(struct Rect));
					jpeg_printf("src : x %d y %d w %d h %d\n",pDisplay_info->src_rect.uStartX,pDisplay_info->src_rect.uStartY
						,pDisplay_info->src_rect.uWidth,pDisplay_info->src_rect.uHeight);
					jpeg_printf("dst x %d y %d w %d h %d\n",pDisplay_info->dst_rect.uStartX,pDisplay_info->dst_rect.uStartY
						,pDisplay_info->dst_rect.uWidth,pDisplay_info->dst_rect.uHeight);
					pDisplay_info->bvp_act_flag |= VIDEO_RESET_SRC_DST_FLAG;
				}
			}
			else
			{	
				pDisplay_info->pVideoInfo->width = 720;
				pDisplay_info->pVideoInfo->height = 576;
				pDisplay_info->stride = 45;
				phdl->imageout.de_src_rect.uStartX = phdl->imageout.de_src_rect.uStartY = 0;
				phdl->imageout.de_src_rect.uWidth = 720;
				phdl->imageout.de_src_rect.uHeight = 576*5;
				phdl->imageout.de_dst_rect.uStartX = phdl->imageout.de_dst_rect.uStartY = 0;
				phdl->imageout.de_dst_rect.uWidth = 720;
				phdl->imageout.de_dst_rect.uHeight = 576*5;		
				phdl->imageout.sw_scaled_rect.uStartX = phdl->imageout.sw_scaled_rect.uStartY = 0;
				phdl->imageout.sw_scaled_rect.uWidth = 720;
				phdl->imageout.sw_scaled_rect.uHeight = 576;
				MEMCPY((void *)&(phdl->imageout.de_dst_bk),&(phdl->imageout.de_dst_rect),sizeof(struct Rect));				
				if((0 != MEMCMP((void *)&pDisplay_info->src_rect,(void *)&phdl->imageout.de_src_rect,sizeof(struct Rect)))
					|| (0 !=  MEMCMP((void *)&pDisplay_info->dst_rect,(void *)&phdl->imageout.de_dst_rect,sizeof(struct Rect))))
				{
					MEMCPY(&(pDisplay_info->src_rect),&(phdl->imageout.de_src_rect),sizeof(struct Rect));
					MEMCPY(&(pDisplay_info->dst_rect),&(phdl->imageout.de_dst_rect),sizeof(struct Rect));
					pDisplay_info->bvp_act_flag |= VIDEO_RESET_SRC_DST_FLAG;
				}
			}
		}
		else
		{
			pDisplay_info->pVideoInfo->width = phdl->imageout.logo_info.width;
			pDisplay_info->pVideoInfo->height = phdl->imageout.logo_info.height;
			pDisplay_info->stride = phdl->imageout.logo_info.stride;
			MEMCPY(&(phdl->imageout.de_src_rect),&(pDisplay_info->src_rect),sizeof(struct Rect));
			MEMCPY(&(phdl->imageout.de_dst_rect),&(pDisplay_info->dst_rect),sizeof(struct Rect));			
		}
}

void imagedec_sub_status_init(pImagedec_hdl phdl,pImagedec_Init_Config pconfig)
{
	phdl->imageout.frm_idx_using = 0;
	phdl->imageout.frm_idx_last_dis = 0;
	phdl->imageout.pic_idx_total = 0;
	phdl->imageout.dis_idx_total = 0;
	phdl->in_stream.fread = pconfig->fread_callback;
	phdl->in_stream.fseek = pconfig->fseek_callback;
	phdl->main.status = pconfig->imagedec_status;
	phdl->status = INITED;	
}
/*
	Parameters:
			struct Rect r		---		the rectangule of being filled
			UINT32 c			---		used color(bits sequence:Y Cb Cr)
*/
static void fill_rect_with_color(UINT8 *y_addr,UINT8 *c_addr,int stride,struct Rect *pr,UINT32 color)
{
	int row = (int)(pr->uStartY);
	int col = (int)(pr->uStartX);
	int w = (int)(pr->uWidth);
	int w_cnt = w;
	int h = (int)(pr->uHeight);
	int row_cnt = 0,col_cnt = 0;
	UINT8 *y_s = NULL,*c_s = NULL;

	JPEG_ENTRY;
	jpeg_printf("Fille color w <%d>h<%d>x<%d>y<%d>\n",w,h,col,row);
	while(h>0)
	{
		col_cnt = col&0xF;
		y_s = y_addr + ((row&0xFFF0)*stride) + ((row&0x0F)<<4) + ((col&0xFFF0)<<4) + (col&0x0F);
		c_s = c_addr + (((row>>5)<<4)*stride) + (((row>>4)&0x1)<<7) + (((row>>1)&0x7)<<4)+ ((col&0xFFF0)<<4) + (col&0xE);	
		w_cnt = w; 
		//soc_printf("fill test y_s <%x>row<%d>col <%d>w<%d>\n",y_s,row,col,w_cnt);
		while(w_cnt>0)
		{
			//soc_printf("y_s <%x>col_cnt<%d>\n",y_s,w_cnt,col_cnt);
			*y_s++ = (color>>16)&0xFF;
			if((!(col_cnt&0x01))&&(!(row&0x01)))
			{
				*c_s++ = (color>>8)&0xFF;
				*c_s++ = color&0xFF;
			}
			col_cnt++;
			if(16 == col_cnt)
			{
				//soc_printf("next col_cnt<%d>w_cnt<%d>\n",col_cnt,w_cnt);
				col_cnt = 0;
				y_s += 240;
				c_s += 240;
			}		
			w_cnt--;
		}
		row++;
		h--;
	}
	JPEG_EXIT;
}

void imagedec_sub_clear_dis_buf(pImagedec_hdl phdl)
{
	UINT8 index = phdl->imageout.frm_idx_using;

	if(!g_imagedec_osd_ins.on)
	{
		jh_mem_set(phdl->imageout.frm[index].frm_y_addr,0x10,phdl->imageout.frm[index].frm_y_len);
		jh_mem_set(phdl->imageout.frm[index].frm_c_addr,0x80,phdl->imageout.frm[index].frm_c_len);	
	}
}

int imagedec_sub_get_frm_buf(pImagedec_hdl phdl)
{
	int index = phdl->imageout.frm_idx_using;
	int flag = 0;

	if(IsTerminated())
	{
		phdl->imageout.frm[0].busy = 0;
		phdl->imageout.frm[1].busy = 0;		
	}
	if(IMAGEDEC_THUMBNAIL == phdl->main.setting.mode)
	{
		g_imagedec_need_logo_info[phdl->id] = TRUE;
		osal_interrupt_disable();
		g_imagedec_dis_flag[phdl->id] = TRUE;
		osal_interrupt_enable();
		while(g_imagedec_need_logo_info[phdl->id])
			{osal_task_sleep(40);}
		if((0xFFFF == phdl->imageout.logo_info.width) || (0 == phdl->imageout.logo_info.width)
			||(0xFFFF == phdl->imageout.logo_info.height) || (0 == phdl->imageout.logo_info.height) 
			|| (0xFFFF == phdl->imageout.logo_info.stride) || (0 == phdl->imageout.logo_info.stride))
		{
			phdl->imageout.logo_info.width = TV_HOR_COL_NUM;
			phdl->imageout.logo_info.height = TV_VER_LINE_NUM;
			phdl->imageout.logo_info.stride = TV_HOR_COL_NUM>>4;
			index = phdl->imageout.frm_idx_using = 1 - phdl->imageout.frm_idx_using;	
			jpeg_printf("logo info invalid\n");
			flag = 1;
		}
		else
		{
			index = phdl->imageout.logo_info.buf_idx;				
			if(index >= 2)
			{
				index  = phdl->imageout.frm_idx_using = 1 - phdl->imageout.frm_idx_using;
				imagedec_copy_frm_buf(phdl->imageout.logo_info.frm_y_addr,phdl->imageout.logo_info.frm_c_addr
					,phdl->imageout.frm[index].frm_y_addr,phdl->imageout.frm[index].frm_c_addr
					,phdl->imageout.frm[index].frm_y_len,phdl->imageout.frm[index].frm_c_len);
				jpeg_printf("index 2\n");
			}
			else
				phdl->imageout.frm_idx_using = index;
			jpeg_printf("get the logo suc logo idx<%d>using<%d>\n",phdl->imageout.logo_info.buf_idx,index);
		}
	}
	else
	{
		if(!g_imagedec_osd_ins.on)
		{
			index = phdl->imageout.frm_idx_using = 1 - phdl->imageout.frm_idx_using;
			jpeg_printf("simple switch to idx <%d>\n",index);
		}
	}
	jpeg_printf("imagedec frm index <%d>idx <%d>\n",index,phdl->imageout.frm_idx_using);

	if(IMAGEDEC_FULL_SRN == phdl->main.setting.mode)
	{
		if(M_NORMAL == phdl->main.setting.show_mode)
		{
			phdl->main.setting.show_flag = 0;
			phdl->imageout.frm_y_len = phdl->imageout.frm[index].frm_y_len;
			phdl->imageout.frm_y_addr = phdl->imageout.frm[index].frm_y_addr;
			phdl->imageout.frm_c_len = phdl->imageout.frm[index].frm_c_len;
			phdl->imageout.frm_c_addr = phdl->imageout.frm[index].frm_c_addr;	
			phdl->imageout.frm[index].busy = 1;
		}
		else
		{
			phdl->main.setting.show_flag = 1;
			phdl->imageout.old_frm.frm_y_len = phdl->imageout.frm[1 - index].frm_y_len;
			phdl->imageout.old_frm.frm_y_addr = phdl->imageout.frm[1 - index].frm_y_addr;
			phdl->imageout.old_frm.frm_c_len = phdl->imageout.frm[1 - index].frm_c_len;
			phdl->imageout.old_frm.frm_c_addr = phdl->imageout.frm[1 - index].frm_c_addr;
			phdl->imageout.old_frm.busy = phdl->imageout.frm[1 - index].busy;		
			phdl->imageout.frm_y_len = phdl->imageout.new_frm.frm_y_len;
			phdl->imageout.frm_y_addr = phdl->imageout.new_frm.frm_y_addr;
			phdl->imageout.frm_c_len = phdl->imageout.new_frm.frm_c_len;
			phdl->imageout.frm_c_addr = phdl->imageout.new_frm.frm_c_addr;
			if(0 == phdl->imageout.old_frm.busy)
			{
				jh_mem_set(phdl->imageout.old_frm.frm_y_addr,0x10,phdl->imageout.old_frm.frm_y_len);
				jh_mem_set(phdl->imageout.old_frm.frm_c_addr,0x80,phdl->imageout.old_frm.frm_c_len);						
				jh_mem_set(phdl->imageout.frm[index].frm_y_addr,0x10,phdl->imageout.frm[index].frm_y_len);
				jh_mem_set(phdl->imageout.frm[index].frm_c_addr,0x80,phdl->imageout.frm[index].frm_c_len);		
			}
			else
			{
				imagedec_copy_frm_buf(phdl->imageout.old_frm.frm_y_addr,phdl->imageout.old_frm.frm_c_addr
					,phdl->imageout.frm[index].frm_y_addr,phdl->imageout.frm[index].frm_c_addr
					,phdl->imageout.frm[index].frm_y_len,phdl->imageout.frm[index].frm_c_len);
			}
			phdl->imageout.frm[1 - index].busy = 0;
			phdl->imageout.frm[index].busy = 0;
		}
	}
	else
	{
		phdl->main.setting.show_flag = 0;
		phdl->imageout.frm_y_len = phdl->imageout.frm[index].frm_y_len;
		phdl->imageout.frm_y_addr = phdl->imageout.frm[index].frm_y_addr;
		phdl->imageout.frm_c_len = phdl->imageout.frm[index].frm_c_len;
		phdl->imageout.frm_c_addr = phdl->imageout.frm[index].frm_c_addr;
		phdl->imageout.frm[index].busy = 0;
		phdl->imageout.frm[1 - index].busy = 0;
	}
	
	if(IMAGEDEC_THUMBNAIL != phdl->main.setting.mode)
	{
		if(!g_imagedec_osd_ins.on)
		{
			jh_mem_set(phdl->imageout.frm_y_addr,0x10,phdl->imageout.frm_y_len);
			jh_mem_set(phdl->imageout.frm_c_addr,0x80,phdl->imageout.frm_c_len);
		}

		if(g_imagedec_copy_frm_ins.on)
		{
			// copy the extern logo data into the frm buf
			jh_mem_cpy((void *)phdl->imageout.frm_y_addr,g_imagedec_copy_frm_ins.logo_buf
				,g_imagedec_copy_frm_ins.y_buf_size);
			jh_mem_cpy((void *)phdl->imageout.frm_c_addr,g_imagedec_copy_frm_ins.logo_buf 
				+ g_imagedec_copy_frm_ins.y_buf_size,g_imagedec_copy_frm_ins.c_buf_size);		
		}
	}
	else
	{
		if(1 == phdl->main.setting.fill_logo)
		{
			INT32 c_size = phdl->imageout.frm_c_len & ~0x1;
			UINT16 *c_addr = (UINT16 *)phdl->imageout.frm_c_addr;
			
			UINT16 c_value = phdl->main.setting.fill_color & 0xFFFF;
			
			jh_mem_set(phdl->imageout.frm_y_addr,((phdl->main.setting.fill_color>>16) & 0xFF),phdl->imageout.frm_y_len);
			while(c_size > 0)
			{
				*c_addr++ = c_value;
				c_size -= 2;
			}
			flag = 0;

			phdl->imageout.logo_info.width = TV_HOR_COL_NUM;
			phdl->imageout.logo_info.height = TV_VER_LINE_NUM;
			phdl->imageout.logo_info.stride = TV_HOR_COL_NUM>>4;			
		}		

		if(1 == flag)
		{
			jh_mem_set(phdl->imageout.frm_y_addr,0x10,phdl->imageout.frm_y_len);
			jh_mem_set(phdl->imageout.frm_c_addr,0x80,phdl->imageout.frm_c_len);
		}
		else
		{
			struct Rect src_tmp;
			
			MEMCPY(&src_tmp,&(phdl->main.setting.dis),sizeof(struct Rect));
			if(TV_HOR_COL_NUM != phdl->imageout.logo_info.width)
			{
                		src_tmp.uStartX = src_tmp.uStartX*phdl->imageout.logo_info.width/TV_HOR_COL_NUM;
				src_tmp.uWidth = src_tmp.uWidth*phdl->imageout.logo_info.width/TV_HOR_COL_NUM;
			}
			if(TV_VER_LINE_NUM != phdl->imageout.logo_info.height)
			{
				src_tmp.uStartY = src_tmp.uStartY*phdl->imageout.logo_info.height/TV_VER_LINE_NUM;
				src_tmp.uHeight = src_tmp.uHeight*phdl->imageout.logo_info.height/TV_VER_LINE_NUM;
			}
			fill_rect_with_color(phdl->imageout.frm_y_addr,phdl->imageout.frm_c_addr,phdl->imageout.logo_info.stride<<4,&src_tmp,0x108080);//0x4164d4 -- red for test
		}		
	}
	return index;
}

