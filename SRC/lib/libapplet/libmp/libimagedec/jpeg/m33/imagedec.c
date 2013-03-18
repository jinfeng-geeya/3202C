
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
#include "imagedec_osd.h"

Imagedec_hdl g_imagedec_hdl[IMAGEDEC_MAX_INSTANCE];
Imagedec_Init_Config g_imagedec_config[IMAGEDEC_MAX_INSTANCE];
static UINT32 g_imagedec_ins_num = 0;/*init device num*/

/*add the supported osd instance*/
imagedec_osd_ins g_imagedec_osd_ins;
imagedec_copy_frm_ins g_imagedec_copy_frm_ins;

/*imagedec control variables*/
volatile bool g_imagedec_pro_show[IMAGEDEC_MAX_INSTANCE];
volatile OSAL_ID g_imagedec_wait_flag[IMAGEDEC_MAX_INSTANCE]; 
volatile bool g_imagedec_close_vp_once[IMAGEDEC_MAX_INSTANCE];
/*imagedec status variables*/
volatile bool g_imagedec_dis_flag[IMAGEDEC_MAX_INSTANCE];
OSAL_ID g_imagedec_in_buf_task_id[IMAGEDEC_MAX_INSTANCE];
volatile bool g_imagedec_stop_flag[IMAGEDEC_MAX_INSTANCE];
volatile bool g_imagedec_need_new_pic[IMAGEDEC_MAX_INSTANCE];
volatile bool g_imagedec_dis_frm_idx[IMAGEDEC_MAX_INSTANCE];
volatile int g_imagedec_info_valid[IMAGEDEC_MAX_INSTANCE];
volatile int g_imagedec_need_logo_info[IMAGEDEC_MAX_INSTANCE];
volatile int g_imagedec_get_info[IMAGEDEC_MAX_INSTANCE];
volatile int g_imagedec_enable_hw_acce[IMAGEDEC_MAX_INSTANCE];
volatile int g_imagedec_vpo_device[IMAGEDEC_MAX_INSTANCE];
volatile enum IMAGE_ANGLE g_imagedec_angle[IMAGEDEC_MAX_INSTANCE];
volatile int g_imagedec_force_full_screen[IMAGEDEC_MAX_INSTANCE];

volatile int g_imagedec_m33_enable_hw_vld;
volatile int g_imagedec_m33_extend_hw_vld;
volatile int g_imagedec_m33_combine_hw_sw_flag;

extern int error_code;

/*Callback function registered into the VPO module for showing the picture*/
enum Output_Frame_Ret_Code imagedec_mp_request(void *dev, void *pinfo)
{
	pImagedec_hdl phdl = NULL;
	struct Display_Info *pDisplay_info = (struct Display_Info *)pinfo;
	int i = 0;
	int suc_num = 1;	/*only support one jpeg decoder device*/

	for(i=0;i<IMAGEDEC_MAX_INSTANCE;i++)
	{
		phdl = &g_imagedec_hdl[i];
		if(TRUE == g_imagedec_dis_flag[phdl->id])
		{
			g_imagedec_dis_flag[phdl->id] = FALSE;
			if(g_imagedec_need_logo_info[phdl->id])
			{
				g_imagedec_need_logo_info[phdl->id] = FALSE;
				phdl->imageout.logo_info.buf_idx = pDisplay_info->top_index;
				phdl->imageout.logo_info.frm_y_addr = pDisplay_info->top_y;
				phdl->imageout.logo_info.frm_c_addr = pDisplay_info->top_c;
				phdl->imageout.logo_info.width = pDisplay_info->pVideoInfo->width;
				phdl->imageout.logo_info.height = pDisplay_info->pVideoInfo->height;
				phdl->imageout.logo_info.stride = pDisplay_info->stride;
				jpeg_printf("idx <%d> w <%d>h<%d>stride<%d>\n",phdl->imageout.logo_info.buf_idx
					,phdl->imageout.logo_info.width,phdl->imageout.logo_info.height
					,phdl->imageout.logo_info.stride);
				break;
			}
			suc_num--;
			phdl->imageout.frm_idx_last_dis = g_imagedec_dis_frm_idx[phdl->id];
			phdl->imageout.dis_idx_total++;
			pDisplay_info->top_index = pDisplay_info->bottom_index = phdl->imageout.frm_idx_using;
			pDisplay_info->src_output_mode = 1;//VDEC27_FRM_ARRAY_MP;
			pDisplay_info->bTopFieldFirst = TRUE;
			pDisplay_info->src_adpcm_onoff = FALSE;
			pDisplay_info->src_adpcm_ratio = 0;
			pDisplay_info->pVideoInfo->eAspectRatio = DAR_4_3;	
			pDisplay_info->bshow_one_field_time = FALSE;
            		pDisplay_info->bbase_single_field = FALSE;
			pDisplay_info->bDitAlgValid = FALSE;
			pDisplay_info->src_top_fieldstructure= pDisplay_info->src_bot_fieldstructure = pDisplay_info->src_top_sml = pDisplay_info->src_bot_sml = FALSE;
			pDisplay_info->top_y = pDisplay_info->bottom_y = phdl->imageout.frm[g_imagedec_dis_frm_idx[phdl->id]].frm_y_addr;
			pDisplay_info->top_c = pDisplay_info->bottom_c = phdl->imageout.frm[g_imagedec_dis_frm_idx[phdl->id]].frm_c_addr;
			osal_cache_flush(pDisplay_info->top_y,phdl->imageout.frm[g_imagedec_dis_frm_idx[phdl->id]].frm_y_len);
			osal_cache_flush(pDisplay_info->top_c,phdl->imageout.frm[g_imagedec_dis_frm_idx[phdl->id]].frm_c_len);
			pDisplay_info->bProgressiveSource = TRUE;
			pDisplay_info->pVideoInfo->frame_rate = 25000;	
			pDisplay_info->bvp_act_flag |= VIDEO_OPEN_WIN_FLAG;
			imagedec_sub_set_pic_size(pDisplay_info,phdl);
		}
	}
	return suc_num;
}

BOOL imagedec_mp_release(void *dev,UINT8 utop_idx,UINT8 frm_array_idx)
{
	return TRUE;
}
enum Output_Frame_Ret_Code imagedec_mp_request_ext(void *dev,struct Display_Info *pinfo,struct Request_Info *pRequest_Info)
{

}
RET_CODE imagedec_mp_release_ext(void *pdev,struct Release_Info *pRelease_Info)
{

}
/* 
	Name:	imagedec_init()
	Parameters:
		pconfig		--> 		imagedec config paramters	
*/
imagedec_id imagedec_init(pImagedec_Init_Config pconfig)
{
	int i =0;
	pImagedec_hdl phdl = NULL;
	struct MPSource_CallBack imagedecMPCallBack;
	OSAL_T_CTSK in_buf_task;

	if(++g_imagedec_ins_num > IMAGEDEC_MAX_INSTANCE)
	{
		g_imagedec_ins_num--;
		jpeg_printf("max ins in the dec already <%d>\n",g_imagedec_ins_num);
		return FALSE;
	}
	phdl = &g_imagedec_hdl[g_imagedec_ins_num - 1];
	phdl->id = g_imagedec_ins_num - 1;
	phdl->status = INITED;
	phdl->file_format = JPEG;
	phdl->file_endian = BIG_ENDIAN;
	MEMSET(phdl,0,sizeof(Imagedec_hdl));
	MEMSET((void *)&g_imagedec_osd_ins,0,sizeof(imagedec_osd_ins));
	MEMSET((void *)&g_imagedec_copy_frm_ins,0,sizeof(imagedec_copy_frm_ins));	
	g_imagedec_pro_show[phdl->id] = FALSE;
	g_imagedec_dis_flag[phdl->id] = FALSE;
	g_imagedec_stop_flag[phdl->id] = FALSE;
	g_imagedec_need_new_pic[phdl->id] = FALSE;
	g_imagedec_info_valid[phdl->id] = FALSE;
	g_imagedec_need_new_pic[phdl->id] = FALSE;
	g_imagedec_get_info[phdl->id] = FALSE;
	g_imagedec_enable_hw_acce[phdl->id] = TRUE;
	g_imagedec_wait_flag[phdl->id] = osal_flag_create(0);
	g_imagedec_angle[phdl->id] = 0;
	g_imagedec_m33_combine_hw_sw_flag = 0;
		
	if((sys_ic_is_M3202() && (sys_ic_get_rev_id() > IC_REV_2))
		|| ((!sys_ic_is_M3202()) && (sys_ic_get_rev_id() > IC_REV_4)))
		g_imagedec_m33_enable_hw_vld = 1;
	else
		g_imagedec_m33_enable_hw_vld = 0;
	
	if(OSAL_INVALID_ID == g_imagedec_wait_flag[g_imagedec_ins_num - 1])
		{JPEG_ASSERT(0);}
	g_imagedec_vpo_device[phdl->id] = (int)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS);

	g_imagedec_force_full_screen[phdl->id] = 0;	
	imagedec_sub_buffer_init(phdl,pconfig);
	imagedec_sub_status_init(phdl,pconfig);
	/*create a task for filling the in buffer*/
#ifndef IMAGEDEC_SINGLE_INPUT_BUF	
	in_buf_task.itskpri = OSAL_PRI_NORMAL;
	in_buf_task.stksz = 0x1000;
	in_buf_task.quantum = 10;
	in_buf_task.para1 = (UINT32)phdl;
	in_buf_task.para2 = (UINT32)g_imagedec_wait_flag[phdl->id];
	in_buf_task.task = imagedec_fill_in_buffer_task;
	g_imagedec_in_buf_task_id[phdl->id] = osal_task_create(&in_buf_task);
	if(OSAL_INVALID_ID == g_imagedec_in_buf_task_id[phdl->id])
		{JPEG_ASSERT(0);}
#endif
	MEMCPY((void *)&g_imagedec_config[phdl->id],(void *)pconfig,sizeof(Imagedec_Init_Config));
	return g_imagedec_ins_num;
}
/* 
	Name:	imagedec_release()
	Parameters:
		id 		--> 		imagedec device id
*/
BOOL imagedec_release(imagedec_id id)
{
	pImagedec_hdl phdl = NULL;
	if((id > IMAGEDEC_MAX_INSTANCE) || (id > g_imagedec_ins_num--))
	{
		jpeg_printf("imagede dec id error <%d>\n",id);
		return FALSE;
	}
	phdl = &g_imagedec_hdl[id-1];
	osal_flag_delete(g_imagedec_wait_flag[phdl->id]);
#ifndef IMAGEDEC_SINGLE_INPUT_BUF
	osal_task_delete(g_imagedec_in_buf_task_id[phdl->id]);
#endif
	phdl->status = RELEASED;
	return TRUE;
}
/* 
	Name:	imagedec_getinfo()
	Parameters:
		id 		--> 		imagedec device id
		fh	--> 		
		pInfo	--> 		
*/
BOOL imagedec_getinfo(imagedec_id id,file_h fh,pImage_info pInfo)
{
	pImagedec_hdl phdl = &g_imagedec_hdl[id-1];
	BOOL ret = TRUE;

	if(RELEASED == phdl->status)
	{	
		jpeg_printf("device is not inited\n");
		return FALSE;
	}
	if(STARTED != phdl->status)
	{
		phdl->status = STARTED;
		g_imagedec_get_info[phdl->id] = TRUE;
		if(!imagedec_get_file_info(phdl,fh))
		{
			ret --;
		}
		g_imagedec_get_info[phdl->id] = FALSE;
		jpeg_printf("direct pro\n");
		phdl->status =STOPED; 
	}
	else
	{
		if((STARTED == phdl->status)&&(!g_imagedec_info_valid[phdl->id]))
			{osal_task_sleep(100);}
		if(g_imagedec_info_valid[phdl->id])
		{
			jpeg_printf("main pro\n");
			g_imagedec_info_valid[phdl->id] = 0;
		}
		else
			ret--;
	}
	if(ret)
	{
		pInfo->file_type = phdl->file_format;
		pInfo->precision = phdl->info.precision;
		pInfo->image_width = phdl->info.width;
		pInfo->image_height= phdl->info.height;
		pInfo->coding_progress = phdl->info.coding_progress;
		jpeg_printf("get info ok w<%d>h<%d>",pInfo->image_width,pInfo->image_height);
	}
	else
		{jpeg_printf("get info fail\n");}
	return ret;
}
/* 
	Name:	imagedec_decode()
	Parameters:
		id 		--> 		imagedec device id
		fh	--> 				
*/
#ifdef JPEG_DUMP_ORI_FILE
extern UINT32 g_jpeg_test_size;
extern UINT8 *g_jpeg_test_addr;
#endif
BOOL imagedec_decode(imagedec_id id,file_h fh)
{
	pImagedec_hdl phdl = &g_imagedec_hdl[id - 1];
	unsigned long last_counter = 0, current_counter = 0;
	int comp = 0, Flag = 0;
	int index = 0;
	ER s_flag;
	int ret_flag = 1;
	imagedec_file_format file_format = 0;
	UINT32 progress = 0;

	if(STARTED == phdl->status)
	{
		jpeg_printf("imagedec start already <%d>\n",phdl->status);
		return FALSE;
	}else if(STARTED != phdl->status)
		phdl->status = STARTED;
	if(fh == 0)
	{
		jpeg_printf("file handle error <%d>\n",fh);
		return FALSE; 
	}
#ifdef JPEG_DUMP_ORI_FILE	
	g_jpeg_test_addr = (UINT8 *)JPEG_DUMP_START_ADDR;
	g_jpeg_test_size = 0;
#endif	
	phdl->fh =  fh;
	JPEG_ENTRY;
	g_imagedec_info_valid[phdl->id] = 0;
	g_imagedec_angle[phdl->id] = ANG_ORI;
	g_imagedec_copy_frm_ins.on = 0;
	phdl->info.coding_progress = 0;
	phdl->info.last_called_prog = 0;
	phdl->main.setting.hw_acc_flag = 0;
	//tranfer the api com into the main decoder
	imagedec_sub_response_to_api(phdl);
	//switch to another fram buffer
	file_format = imagedec_sub_judge_file_format(phdl);
	if(JPEG != file_format)
		phdl->main.setting.show_mode = M_NORMAL;
	index = imagedec_sub_get_frm_buf(phdl);
	if(g_imagedec_copy_frm_ins.on)
	{
		phdl->decoder.progressive_flag = 1;
		goto DEC_DONE;
	}
	if (!imagedec_sub_file_pos_init(phdl)) 
	{
			ret_flag--;
			goto DEC_DONE; 
	}
	jpeg_printf("file format <%d>\n",file_format);
	/*switch different format to different entry*/
	switch(file_format)
	{
		case JPEG:
			imagedec_jpeg_init();
			break;
		case Bitmap:
			imagedec_bmp_init(phdl);
			break;
		default:
			ret_flag--;
			goto DEC_DONE;	
	}
	phdl->file_format = file_format;
	if(Bitmap != file_format)
		goto JPEG_DECODER;
BMP_DECODER:
	if(TRUE == g_imagedec_pro_show[phdl->id])
	{
		g_imagedec_dis_frm_idx[phdl->id] = index;
		osal_interrupt_disable();
		g_imagedec_dis_flag[phdl->id] = TRUE;
		osal_interrupt_enable();
		while(g_imagedec_dis_flag[phdl->id])
			{osal_task_sleep(50);};
		jpeg_printf("pro show frm idx<%d>\n",index);
	}
	do
	{
		if (IsTerminated()) 
		{
			ret_flag--;
			goto DEC_DONE; 
		}
		/*check for the stop status*/
		if(TRUE == g_imagedec_stop_flag[phdl->id])
		{
			g_imagedec_stop_flag[phdl->id] = FALSE;
			s_flag = osal_flag_set(g_imagedec_wait_flag[phdl->id],IMAGEDEC_STOP_PTN);
			if(E_OK != s_flag)
			{
				jpeg_printf("set flag error<%s>\n",__FUNCTION__);
			}
			ret_flag--;
			goto DEC_DONE;
		}
		Flag = imagedec_bmp_decode_row(phdl);
		if(BMP_FAILED == Flag)
		{
			ret_flag--;
			goto DEC_DONE;
		}else if(Flag)
		{
			if(g_imagedec_osd_ins.on)
			{
				g_imagedec_osd_ins.update_osd((void *)&g_imagedec_osd_ins);
			}
						
			goto DEC_DONE;
		}
		
	}while(1);
	

JPEG_DECODER:
	if(JPEG != file_format)
	{
		ret_flag--;
		jpeg_printf("file format not support\n");
		goto DEC_DONE;
	}
	
	if (IsTerminated()) 
	{
		ret_flag--;
		goto DEC_DONE; 
	}
	if (IsProgressive(phdl)) 
	{
		if(phdl->main.setting.show_flag)
		{
			jh_mem_set(phdl->imageout.frm[index].frm_y_addr,0x10,phdl->imageout.frm[index].frm_y_len);
			jh_mem_set(phdl->imageout.frm[index].frm_c_addr,0x80,phdl->imageout.frm[index].frm_c_len);	
			jh_mem_set(phdl->imageout.frm[1 - index].frm_y_addr,0x10,phdl->imageout.frm[1 - index].frm_y_len);
			jh_mem_set(phdl->imageout.frm[1 - index].frm_c_addr,0x80,phdl->imageout.frm[1 - index].frm_c_len);				
		}
		if(phdl->main.setting.show_flag || IMAGEDEC_THUMBNAIL == phdl->main.setting.mode)
		{
			if(TRUE == g_imagedec_pro_show[phdl->id])
			{
				g_imagedec_dis_frm_idx[phdl->id] = index;
				osal_interrupt_disable();
				g_imagedec_dis_flag[phdl->id] = TRUE;
				osal_interrupt_enable();
				while(g_imagedec_dis_flag[phdl->id])
					{osal_task_sleep(50);};
				jpeg_printf("pro show frm idx<%d>\n",index);
			}
		}
	}
	do 
	{ 
		if (IsProgressive(phdl)) 
		{
			phdl->mem.mem_pos = phdl->mem.mem_start;
			if (!imagedec_sub_file_pos_init(phdl)) 
			{
				ret_flag--;
				goto DEC_DONE; 
			}
			set_comp(phdl,comp);
			imagedec_jpeg_init(); // init again if progressive JPG, because of the re-arranged mem.
			if (imagedec_jpeg_begin()) 
			{
				ret_flag--;
				goto DEC_DONE;
			}
		}
		else
		{
			if(imagedec_jpeg_begin())			// non-progressive JPG picture, so...
			{
				ret_flag--;
				goto DEC_DONE; 
			}
			if(!g_imagedec_force_full_screen[phdl->id])
			{
				if(TRUE == g_imagedec_pro_show[phdl->id])
				{
					g_imagedec_dis_frm_idx[phdl->id] = index;
					osal_interrupt_disable();
					g_imagedec_dis_flag[phdl->id] = TRUE;
					osal_interrupt_enable();
					while(g_imagedec_dis_flag[phdl->id])
						{osal_task_sleep(50);};
					jpeg_printf("pro show frm idx<%d>\n",index);
				}
			}
		}
	//////////////////////////////////////////////////////////////
		// picture width control
		// control it in the function of alloc(), so...
		if (IsTerminated()) 
		{
			ret_flag--;
			goto DEC_DONE;  
		}
		// we will decode one MCU(8 or 16 pixel lines) per time
		while (1)
		{
			/*check for the stop status*/
			if(TRUE == g_imagedec_stop_flag[phdl->id])
			{
				g_imagedec_stop_flag[phdl->id] = FALSE;
				s_flag = osal_flag_set(g_imagedec_wait_flag[phdl->id],IMAGEDEC_STOP_PTN);
				if(E_OK != s_flag)
				{
					jpeg_printf("set flag error<%s>\n",__FUNCTION__);
				}
				ret_flag--;
				goto DEC_DONE;
			}
			Flag = imagedec_jpeg_decode();
			if (JPGD_FAILED == Flag) 
			{
				jpeg_printf("JPEG decode return JPED_FAILED, mem_pos: 0x%x) code <%d>!\n",phdl->mem.mem_pos,error_code);
				ret_flag--;
				goto DEC_DONE; 
			}
			else if (Flag) 
			{
				jpeg_printf("JPEG done\n");
				break;
			}
			jpeg_printf("jpeg dec row ok\n");
			if (IsTerminated())
			{
				jpeg_printf("JPEG decode terminated!\n");
				ret_flag--;
				goto DEC_DONE;
			}
		}
	} while (++comp<max_ycbcr_comp(phdl) && IsProgressive(phdl) && !IsPro2_big()); // end do while()...
	if(phdl->main.setting.hw_acc_flag)
	{
		if(!g_imagedec_m33_combine_hw_sw_flag)
			jhcovs_m3329e_op(phdl);
	}

	if(g_imagedec_osd_ins.on)
	{
		g_imagedec_osd_ins.update_osd((void *)&g_imagedec_osd_ins);
	}
	else if(phdl->main.setting.show_flag)
		img_show_opn(phdl);

DEC_DONE:
	if ((IsProgressive(phdl)) || (g_imagedec_force_full_screen[phdl->id] == 1)) 
	{	
		if(IMAGEDEC_THUMBNAIL != phdl->main.setting.mode && !phdl->main.setting.show_flag)
		{
			if(TRUE == g_imagedec_pro_show[phdl->id])
			{
				g_imagedec_dis_frm_idx[phdl->id] = index;
				osal_interrupt_disable();
				g_imagedec_dis_flag[phdl->id] = TRUE;
				osal_interrupt_enable();
				while(g_imagedec_dis_flag[phdl->id])
					{osal_task_sleep(50);};
				jpeg_printf("pro show frm idx<%d>\n",index);
			}
		}
	}
	if(((ret_flag < 1) || IsTerminated()) && (IMAGEDEC_THUMBNAIL != phdl->main.setting.mode)){
		imagedec_sub_clear_dis_buf(phdl);
	}
	phdl->info.coding_progress = 1<<16;
	if (IsTerminated()) 
	{
		switch(error_code)
		{
			case JPGD_NOTENOUGHMEM:
				phdl->info.coding_progress |= IMG_ER_MEM<<24;
				break;
			default:
				phdl->info.coding_progress |= IMG_ER_FILE<<24;
				break;			
		}
		if(TRUE == g_imagedec_pro_show[phdl->id])
		{
			g_imagedec_dis_frm_idx[phdl->id] = index;
			osal_interrupt_disable();
			g_imagedec_dis_flag[phdl->id] = TRUE;
			osal_interrupt_enable();
			while(g_imagedec_dis_flag[phdl->id])
				{osal_task_sleep(50);};
			jpeg_printf("pro show frm idx<%d>\n",index);
		}		
	}
	if(NULL != phdl->main.status)
		phdl->main.status((void *)&(phdl->info.coding_progress));
	g_imagedec_info_valid[phdl->id] = 1;
	g_imagedec_dis_frm_idx[phdl->id] = index;
	if(TRUE == g_imagedec_need_new_pic[phdl->id])
	{
		g_imagedec_need_new_pic[phdl->id] = FALSE;
		s_flag = osal_flag_set(g_imagedec_wait_flag[phdl->id],IMAGEDEC_DECD_PTN);
		if(E_OK != s_flag)
		{
			jpeg_printf("set flag error<%s>\n",__FUNCTION__);
		}
	}
	jpeg_printf("mem used by dec <%d>k col_c <%d>ret_flag<%d>\n",(phdl->mem.mem_pos - phdl->mem.mem_start)>>10,phdl->decoder.mcus_of_col,ret_flag);
	phdl->imageout.pic_idx_total++;
	phdl->status = STOPED;
	if(TRUE == g_imagedec_stop_flag[phdl->id])
	{
		g_imagedec_stop_flag[phdl->id] = FALSE;
		s_flag = osal_flag_set(g_imagedec_wait_flag[phdl->id],IMAGEDEC_STOP_PTN);
		if(E_OK != s_flag)
		{
			jpeg_printf("set flag error<%s>\n",__FUNCTION__);
		}
	}

	if(g_imagedec_m33_enable_hw_vld&&g_imagedec_enable_hw_acce[0])
	{		
		osal_interrupt_disable();
		vdec_io_control((struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0), VDEC_IO_RESET_VE_HW,0);
		osal_interrupt_enable();
	}
	
	return ret_flag;
}
/* 
	Name:	imagedec_stop()
	Parameters:
		id 		--> 		imagedec device id
*/
BOOL imagedec_stop(imagedec_id id)
{
	pImagedec_hdl phdl = &g_imagedec_hdl[id - 1];
	ER s_flag;
	UINT32 ptn_flag;
	
	if(STARTED == phdl->status)
	{
		jpeg_printf("dec status started\n");
		g_imagedec_stop_flag[phdl->id] = TRUE;
		s_flag = osal_flag_wait(&ptn_flag,g_imagedec_wait_flag[phdl->id],IMAGEDEC_STOP_PTN,TWF_ANDW,IMAGEDEC_TIME_OUT_STOP);	
		if(E_OK == s_flag)
			osal_flag_clear(g_imagedec_wait_flag[phdl->id],IMAGEDEC_STOP_PTN);
	}
	jpeg_printf("stop suc\n");
	g_imagedec_stop_flag[phdl->id] = FALSE;
	phdl->status = STOPED;
	return TRUE;
}
/* 
	Name:	imagedec_set_mode()
	Parameters:
		id 		--> 		imagedec device id
		mode	--> 		
		pPar		--> 		
*/
BOOL imagedec_set_mode(imagedec_id id, enum IMAGE_DIS_MODE mode,pImagedec_Mode_Par pPar)
{	
	pImagedec_hdl phdl = &g_imagedec_hdl[id - 1];

	JPEG_ENTRY;
	if((STARTED == phdl->status) || (RELEASED == phdl->status))
	{
		jpeg_printf("status error <%s>\n",__FUNCTION__);
		return FALSE;
	}
	if(0 != mode)
		phdl->main.api_cmd.mode = mode;
	if(NULL != pPar)
	{
		g_imagedec_close_vp_once[phdl->id] = pPar->vp_close_control;
		g_imagedec_pro_show[phdl->id] = pPar->pro_show;
		MEMCPY(&(phdl->main.api_cmd.src),&(pPar->src_rect),sizeof(struct Rect));
		MEMCPY(&(phdl->main.api_cmd.dis),&(pPar->dis_rect),sizeof(struct Rect));

		//show mode
#if 1
		//if(pPar->show_mode != phdl->main.api_cmd.show_mode)
		{
			if(NULL != pPar->show_mode_par)
			{
				phdl->main.api_cmd.show_mode = pPar->show_mode;
				MEMCPY((void *)phdl->main.api_cmd.show_mode_par,(void *)pPar->show_mode_par,IMAGEDEC_SHOW_MODE_PAR_LEN);
				phdl->main.show_mode_update = 1;
			}
			else
			{
				phdl->main.api_cmd.show_mode = M_NORMAL;
				phdl->main.show_mode_update = 1;			
			}
		}
#else
		phdl->main.api_cmd.show_mode++;
		phdl->main.api_cmd.show_mode %= 6;
		phdl->main.api_cmd.show_mode_par[0] = 0;
		phdl->main.show_mode_update = 1;
#endif
	}
	else
	{
		g_imagedec_close_vp_once[phdl->id] = FALSE;
		switch(mode)
		{
			case IMAGEDEC_FULL_SRN:
			case IMAGEDEC_REAL_SIZE:
				/*default setting of src and dis rect when mode FULL_SRN and REAL_SIZE*/
				g_imagedec_pro_show[phdl->id] = TRUE;
				phdl->main.api_cmd.src.uStartX = 0;
				phdl->main.api_cmd.src.uStartY = 0;
				phdl->main.api_cmd.src.uWidth = 720;
				phdl->main.api_cmd.src.uHeight = 576;
				phdl->main.api_cmd.dis.uStartX = 0;
				phdl->main.api_cmd.dis.uStartY = 0;
				phdl->main.api_cmd.dis.uWidth = 720;
				phdl->main.api_cmd.dis.uHeight = 576;
				break;
			case IMAGEDEC_THUMBNAIL:
				/*default setting of src and dis rect when mode thumbnail*/
				g_imagedec_pro_show[phdl->id] = TRUE;
				phdl->main.api_cmd.src.uStartX = 0;
				phdl->main.api_cmd.src.uStartY = 0;
				phdl->main.api_cmd.src.uWidth = 720;
				phdl->main.api_cmd.src.uHeight = 576;
				phdl->main.api_cmd.dis.uStartX = 160;
				phdl->main.api_cmd.dis.uStartY = 160;
				phdl->main.api_cmd.dis.uWidth = 120;
				phdl->main.api_cmd.dis.uHeight = 90;
				break;
			case IMAGEDEC_AUTO:
			case RESERVED_MODE:
				{JPEG_ASSERT(0);}
		}
	}
	phdl->main.mode_update = 1;
	JPEG_EXIT;
	return TRUE;
}
/* 
	Name:	imagedec_dis_next_pic()
	Parameters:
		id 		--> 		imagedec device id
		mode	--> 		
		time_out	--> 		
*/
BOOL imagedec_dis_next_pic(imagedec_id id,UINT32 mode,UINT32 time_out)
{
	pImagedec_hdl phdl = &g_imagedec_hdl[id - 1];
	ER s_flag;
	UINT32 ptn_flag;

	JPEG_ENTRY;
	if(TRUE == g_imagedec_pro_show[phdl->id])
		return FALSE;
	if(STOPED == phdl->status)
	{
		jpeg_printf("dec status stop<%s>\n",__FUNCTION__);
		osal_interrupt_disable();
		g_imagedec_dis_flag[phdl->id] = TRUE;
	}
	else if(STARTED == phdl->status)
	{
		g_imagedec_need_new_pic[phdl->id] = TRUE;
		s_flag = osal_flag_wait(&ptn_flag,g_imagedec_wait_flag[phdl->id],IMAGEDEC_DECD_PTN,TWF_ANDW,time_out);	
		if(E_OK != s_flag)
		{
			jpeg_printf("wait flag error<%d><%s>\n",s_flag,__FUNCTION__);
			return FALSE;
		}
		else
		{
			osal_interrupt_disable();
			g_imagedec_dis_flag[phdl->id] = TRUE;
			osal_flag_clear(g_imagedec_wait_flag[phdl->id],IMAGEDEC_DECD_PTN);	

		}
	}
	else
		osal_interrupt_disable();
	/*close the vp main layer get new frm idx*/
	if(g_imagedec_dis_frm_idx[phdl->id] == phdl->imageout.frm_idx_last_dis) 
		g_imagedec_dis_flag[phdl->id] = FALSE;	
	osal_interrupt_enable();
	if(g_imagedec_dis_flag[phdl->id]&&g_imagedec_close_vp_once[phdl->id])
		vpo_win_onoff((struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS),FALSE);
	while(g_imagedec_dis_flag[phdl->id])
		{osal_task_sleep(40);}
	jpeg_printf("show next pic idx<%d>\n",g_imagedec_dis_frm_idx[phdl->id]);
	JPEG_EXIT;
	return TRUE;	
}

/* 
	Name:	imagedec_zoom()
	Parameters:
		id 		--> 		imagedec device id
		dis_rect	-->        it could be NULL	
		src_rect	--> 		scale parmaters, choose 1:2 scale up as the example:
						src->rect->uStartX = x; // x + width< 720 
						src->rect->uStartY = y; // y + height < 576
						src->rect->uWidth = 720/2;
						src->rect->uHeight = 576/2;
		
*/
BOOL imagedec_zoom(imagedec_id id,struct Rect dis_rect,struct Rect src_rect)
{
	pImagedec_hdl phdl = &g_imagedec_hdl[id - 1];
	struct Rect src,dst;
	BOOL ret = FALSE;
	UINT16 w = 0;
	UINT16 h = 0;
	UINT8 flag_src_w = 1 , flag_src_h = 1;

	if((STARTED == phdl->status) || (RELEASED == phdl->status))
	{
		jpeg_printf("status error <%s>\n",__FUNCTION__);
		return FALSE;
	}
	jpeg_printf("in x %d y %d w %d h %d\n",src_rect.uStartX,src_rect.uStartY,src_rect.uWidth,src_rect.uHeight);
	MEMCPY((void *)&src,(void *)&phdl->imageout.de_src_rect,sizeof(struct Rect));
	MEMCPY((void *)&dst,(void *)&phdl->imageout.de_dst_rect,sizeof(struct Rect));
	jpeg_printf("Src in x %d y %d w %d h %d\n",src.uStartX,src.uStartY,src.uWidth,src.uHeight);
	jpeg_printf("Dst in x %d y %d w %d h %d\n",dst.uStartX,dst.uStartY,dst.uWidth,dst.uHeight);
	
	if(g_imagedec_enable_hw_acce[phdl->id])
	{
		w = phdl->imageout.de_dst_bk.uWidth;
		h = phdl->imageout.de_dst_bk.uHeight / 5;
		flag_src_w = w < src_rect.uWidth? 0 : 1;
		flag_src_h = h < src_rect.uHeight? 0 : 1;
		
		if(1 == flag_src_w)
		{
			if((TV_MAX_WIDTH * w / src_rect.uWidth) > TV_MAX_WIDTH)
			{
				if(w < TV_MAX_WIDTH)
				{
					dst.uWidth = TV_MAX_WIDTH;
					dst.uStartX = 0;			
				}
			}
		}
		
		if(1 == flag_src_h)
		{
			if((TV_MAX_HEIGHT * h / src_rect.uHeight) > TV_MAX_HEIGHT)
			{
				if(h < TV_MAX_HEIGHT)
				{
					dst.uHeight= TV_MAX_HEIGHT * 5;
					dst.uStartY = 0;			
				}
			}
		}
		
	}
	else
	{
		flag_src_w = flag_src_h = 1;
	}
	
	if(1 == flag_src_w)
	{
		int img_w = phdl->imageout.sw_scaled_rect.uWidth;
		
		img_w = img_w * TV_MAX_WIDTH / src_rect.uWidth;

		if(img_w < TV_MAX_WIDTH)
		{
			src.uWidth = src_rect.uWidth;
			src.uStartX = (TV_MAX_WIDTH - src_rect.uWidth)>>1;
		}
		else
		{
			src.uWidth = src_rect.uWidth * TV_MAX_WIDTH / phdl->imageout.de_dst_bk.uWidth;	
			src.uStartX = phdl->imageout.sw_scaled_rect.uStartX;
			src_rect.uStartX = src_rect.uStartX * phdl->imageout.de_dst_bk.uWidth / TV_MAX_WIDTH;
			if(src_rect.uStartX > src.uStartX)
			{
				src.uStartX = TV_MAX_WIDTH - src.uWidth;
				if(src_rect.uStartX < src.uStartX)
					src.uStartX = src_rect.uStartX;
			}
		}
	}
	else
	{
		dst.uWidth = (w * TV_MAX_WIDTH) / src_rect.uWidth;
		dst.uStartX = (TV_MAX_WIDTH - dst.uWidth)>>1;
	}

	if(1 == flag_src_h)
	{
		int img_h = phdl->imageout.sw_scaled_rect.uHeight;

		img_h = img_h * TV_MAX_HEIGHT / src_rect.uHeight;
		if(img_h < TV_MAX_HEIGHT)
		{
			src.uHeight = src_rect.uHeight * 5;	
			src.uStartY = ((TV_MAX_HEIGHT - src_rect.uHeight)>>1) * 5;			
		}
		else
		{
			src.uHeight = src_rect.uHeight * TV_MAX_HEIGHT * 25 /phdl->imageout.de_dst_bk.uHeight;	
			src.uStartY = phdl->imageout.sw_scaled_rect.uStartY * 5;
			src_rect.uStartY = src_rect.uStartY * phdl->imageout.de_dst_bk.uHeight / TV_MAX_HEIGHT;
			if(src_rect.uStartY > src.uStartY)
			{
				src.uStartY = (TV_MAX_HEIGHT * 5) - src.uHeight;
				if(src_rect.uStartY < src.uStartY)
					src.uStartY = src_rect.uStartY;
			}
		}
	}
	else
	{
		dst.uHeight = (h * TV_MAX_HEIGHT * 5) / src_rect.uHeight;
		dst.uStartY = ((TV_MAX_HEIGHT * 5) - dst.uHeight)>>1;
	}

	if((TV_MAX_WIDTH == src_rect.uWidth) && (TV_MAX_HEIGHT == src_rect.uHeight) 
		&& g_imagedec_enable_hw_acce[phdl->id])
	{
		//MEMCPY((void *)&src,(void *)&phdl->imageout.de_src_bk,sizeof(struct Rect));
		src.uStartX = src.uStartY = 0;
		src.uWidth = TV_MAX_WIDTH;
		src.uHeight = TV_MAX_HEIGHT *5;
		MEMCPY((void *)&dst,(void *)&phdl->imageout.de_dst_bk,sizeof(struct Rect));		
	}
	jpeg_printf("Src out x %d y %d w %d h %d\n",src.uStartX,src.uStartY,src.uWidth,src.uHeight);
	jpeg_printf("Dst out x %d y %d w %d h %d\n\n\n",dst.uStartX,dst.uStartY,dst.uWidth,dst.uHeight);
	
	MEMCPY((void *)&phdl->imageout.de_src_rect,(void *)&src,sizeof(struct Rect));
	MEMCPY((void *)&phdl->imageout.de_dst_rect,(void *)&dst,sizeof(struct Rect));
	vpo_zoom((struct vpo_device *)g_imagedec_vpo_device[phdl->id],&src,&dst);
	return FALSE;
}

/* 
	Name:	imagedec_rotate()
	Parameters:
		id 		--> 		imagedec device id
		angle	--> 		rotate angle
		mode	--> 		rotate mode : 0 -- rotate the current picture, have no effect to the next pic£¬
								              it need to be implemented in the future
									1 -- set the device rotate angle for the next decoded pic
*/
BOOL imagedec_rotate(imagedec_id id,enum IMAGE_ANGLE angle,UINT32 mode)
{	
	pImagedec_hdl phdl = &g_imagedec_hdl[id - 1];

	if((STARTED == phdl->status) || (RELEASED == phdl->status))
	{
		jpeg_printf("status error <%s>\n",__FUNCTION__);
		return FALSE;
	}
	else
	{
		if(mode)
		{
			phdl->main.api_cmd.angle = angle;
			phdl->main.rot_par_update = 1;
			return TRUE;
		}
		else
		{
			// to be added
			return FALSE;
		}
	}
}

BOOL imagedec_ioctl(imagedec_id id,UINT32 cmd, UINT32 para)
{
	pImagedec_hdl phdl = &g_imagedec_hdl[id - 1];

	if(RELEASED == phdl->status)
	{
		jpeg_printf("status error <%s>\n",__FUNCTION__);
		return FALSE;
	}
	switch(cmd)
	{
		case IMAGEDEC_IO_CMD_OPEN_HW_ACCE:
			if(para)
				phdl->main.api_cmd.hw_acc_flag = TRUE;
			else
				phdl->main.api_cmd.hw_acc_flag = FALSE;
			phdl->main.hw_act_update = 1;
			break;
		case IMAGEDEC_IO_CMD_FILL_LOGO:
			phdl->main.fill_logo_update = 1;
			phdl->main.api_cmd.fill_color = para;
			break;
		case IMAGEDEC_IO_CMD_OSD_SHOW:
			{
				pImagedec_Osd_Io_Cmd pcmd = (pImagedec_Osd_Io_Cmd)para;
				
				phdl->main.osd_mode_update = 1;
				MEMSET((void *)&(g_imagedec_osd_ins.api_cmd),0,sizeof(imagedec_api_cmd));
				if(para)
				{
					g_imagedec_osd_ins.api_cmd.on = pcmd->on;
					g_imagedec_osd_ins.api_cmd.dis_rect.uStartX = pcmd->dis_rect.uStartX;
					g_imagedec_osd_ins.api_cmd.dis_rect.uStartY = pcmd->dis_rect.uStartY;
					g_imagedec_osd_ins.api_cmd.dis_rect.uWidth = pcmd->dis_rect.uWidth;
					g_imagedec_osd_ins.api_cmd.dis_rect.uHeight = pcmd->dis_rect.uHeight;
					g_imagedec_osd_ins.api_cmd.bitmap = pcmd->bitmap_start;
					g_imagedec_osd_ins.api_cmd.buf_size = pcmd->bitmap_size;
				}
				else // if para == 0, disable the osd show mode
				{
					g_imagedec_osd_ins.api_cmd.on = 0; 
				}
				break;
			}
		case IMAGEDEC_IO_CMD_COPY_FRM:
			{
				phdl->main.copy_frm_update = 1;

				if(para)
				{
					MEMCPY((void *)&g_imagedec_copy_frm_ins.input_par,(void *)para,sizeof(Imagedec_Io_Cmd_Copy_Frm));	
				}
				break;
			}
		case IMAGEDEC_IO_CMD_DIS_FRM_INFO:
			{
				pImagedec_Io_Cmd_Frm_Info pfrm = (pImagedec_Io_Cmd_Frm_Info)para;

				if(STARTED == phdl->status)
					return FALSE;
				
				if(NULL != pfrm)
				{
					pfrm->y_buf = (UINT32)(phdl->imageout.frm[g_imagedec_dis_frm_idx[phdl->id]].frm_y_addr);
					pfrm->c_buf = (UINT32)(phdl->imageout.frm[g_imagedec_dis_frm_idx[phdl->id]].frm_c_addr);
					pfrm->y_size = phdl->imageout.frm[g_imagedec_dis_frm_idx[phdl->id]].frm_y_len;
					pfrm->c_size = phdl->imageout.frm[g_imagedec_dis_frm_idx[phdl->id]].frm_c_len;
				}
				break;
			}
		case IMAGEDEC_IO_CMD_FORCE_FULL_SCREEN:
			{
				if(TRUE == para)
					g_imagedec_force_full_screen[phdl->id] = 1;
				else
					g_imagedec_force_full_screen[phdl->id] = 0;

				break;
			}
		default:
			break;
	}
	return TRUE;
}

