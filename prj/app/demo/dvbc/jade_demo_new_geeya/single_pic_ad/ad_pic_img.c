#include <sys_config.h>

#ifdef SHOW_PICS
#if(SHOW_PICS == SHOW_SINGLE_PIC)

#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <hld/ge/ge.h>
#include <hld/osd/osddrv_dev.h>
#include <api/libimagedec/imagedec.h>
#include <hld/dis/vpo.h>

#include "ad_pic_gfx.h"
#include "ad_pic_img.h"


#define AD_IMG_DEBUG
#ifdef AD_IMG_DEBUG
#define AD_IMG_PRINTF	libc_printf
#else
#define AD_IMG_PRINTF(...)	do{}while(0)
#endif


static UINT8 *img_file_addr; //start address of the buffer to restore the image file	
static UINT32 img_file_size;  // image file size	
static INT32 img_input_pos = 0;

static int img_read_data(UINT32 fh,BYTE *buf, UINT32 size)
{
	UINT32 read_pos;
	UINT32 read_len;

	if((img_file_addr == NULL)||(img_file_size == 0))
	{
		MEMSET((void *)buf, 0x0, size);
		return 0;
	}
	
	read_pos = img_input_pos+size;
	if(read_pos>img_file_size)
	{
		read_len = img_file_size-img_input_pos;
		MEMCPY((void *)buf, (void *)(img_file_addr+img_input_pos), read_len);
		img_input_pos += read_len;
		return read_len;
	}
	else
	{
		MEMCPY((void *)buf, (void *)(img_file_addr+img_input_pos), size);
		img_input_pos+=size;
		return size;
	}
}

static int img_seek_data(UINT32 fh,long offset, UINT32 origin)
{
	if(origin == 0)
	{
		img_input_pos = (UINT32)offset;
		return TRUE;
	}
	return FALSE;
}

static void ad_pic_img_blt(void *handle, pImagedec_Out_Image pimage, struct OSDRect *rect)
{
	ad_pic_gfx_draw(rect, (UINT8 *)pimage->start_buf);
}

//the parameter of rect is the showing region on GE
static INT32 ad_pic_img_dec(struct OSDRect *rect)
{
	imagedec_id id = 0;
	UINT32 file_handle = 0xFF;
	BOOL ret = 0;

	Imagedec_Init_Config img_init_info;
	MEMSET(&img_init_info,0,sizeof(Imagedec_Init_Config));
	img_init_info.frm_y_size = __MM_FB0_Y_LEN;
	img_init_info.frm_y_addr = __MM_FB0_Y_START_ADDR;
	img_init_info.frm_c_size = __MM_FB0_C_LEN;
	img_init_info.frm_c_addr = __MM_FB0_C_START_ADDR;
	img_init_info.frm2_y_size = __MM_FB1_Y_LEN;
	img_init_info.frm2_y_addr = __MM_FB1_Y_START_ADDR;
	img_init_info.frm2_c_size = __MM_FB1_C_LEN;
	img_init_info.frm2_c_addr = __MM_FB1_C_START_ADDR;
	img_init_info.frm3_y_size = __MM_FB2_Y_LEN;
	img_init_info.frm3_y_addr = __MM_FB2_Y_START_ADDR;
	img_init_info.frm3_c_size = __MM_FB2_C_LEN;
	img_init_info.frm3_c_addr = __MM_FB2_C_START_ADDR;	
	img_init_info.frm_mb_type = 0;
	img_init_info.decoder_buf = (UINT8 *)((__MM_IMAGEDEC_BUF_ADDR+0x10000) & ~(3<<29));//__MM_IMG_DEC_ADDR;
	img_init_info.decoder_buf_len = 0x54600; //0x7E800;//0x3FA40; // 0x3FA40=320*240*1.5+0x20000 ;0x15180=320*240//__MM_IMG_DEC_LEN;
	img_init_info.fread_callback = img_read_data; 
	img_init_info.fseek_callback = img_seek_data;
	id = imagedec_init(&img_init_info);
	if(0 == id)
	{
		AD_IMG_PRINTF("%s() imagedec_init failed(%d)!\n", __FUNCTION__, id);
		return !SUCCESS;
	}

	if (TRUE != imagedec_ioctl(id, IMAGEDEC_IO_CMD_OPEN_HW_ACCE, 0))
	{
		AD_IMG_PRINTF("%s() imagedec_ioctl(IMAGEDEC_IO_CMD_OPEN_HW_ACCE) failed!\n", __FUNCTION__);
		return !SUCCESS;
	}

	Imagedec_Osd_Config config;
	MEMSET((void *)&config, 0, sizeof(Imagedec_Osd_Config));
	config.blt = ad_pic_img_blt;
	config.handle = (void *)ad_pic_get_ge_surface(AD_DST_SURFACE);
	config.pic_frmt = 0;
	imagedec_osd_init(&config);

	Imagedec_Mode_Par dec_par;
	MEMSET((void *)&dec_par,0,sizeof(Imagedec_Mode_Par));
	dec_par.vp_close_control = 0;
	dec_par.pro_show = 1;
 	dec_par.dis_rect.uStartX = 0; //rect->uLeft;
	dec_par.dis_rect.uStartY = 0;
	dec_par.dis_rect.uWidth =  720;//rect->uWidth;
	dec_par.dis_rect.uHeight =  576;//rect->uHeight;   
	dec_par.src_rect.uStartX = 0;//rect->uLeft;
	dec_par.src_rect.uStartY = 0;//rect->uTop;
	dec_par.src_rect.uWidth = 720;//rect->uWidth;
	dec_par.src_rect.uHeight = 576;//rect->uHeight;	
	imagedec_set_mode(id, IMAGEDEC_FULL_SRN, &dec_par);
	
	Imagedec_Osd_Io_Cmd cmd;
	MEMSET((void *)&cmd,0,sizeof(Imagedec_Osd_Io_Cmd));
	cmd.on = 1;
    
	cmd.bitmap_start = (UINT8 *)((__MM_IMAGEDEC_BUF_ADDR+0x64600)& ~(3<<29)); //__MM_IMG_BITMAP_ADDR;
	cmd.bitmap_size = 0x2A300; //0x2A300= 320*240*2 //__MM_IMG_BITMAP_LEN;
//for testing, remember to delete the malloc size
//	cmd.bitmap_start = (UINT8 *)(MALLOC(0x2A300)); 
//	cmd.bitmap_size = 0x2A300; //0x2A300= 320*240*2 //__MM_IMG_BITMAP_LEN;
	
	cmd.dis_rect.uStartX = rect->uLeft;		
	cmd.dis_rect.uStartY = rect->uTop;//0;
	cmd.dis_rect.uWidth = rect->uWidth;
	cmd.dis_rect.uHeight = rect->uHeight;
	imagedec_ioctl(id, IMAGEDEC_IO_CMD_OSD_SHOW, (UINT32)&cmd);

	img_input_pos = 0;
	ret = imagedec_decode(id, file_handle);
    if(ret!=TRUE)
        AD_IMG_PRINTF("\n Decode Image Unsuccessfully");
	
	if(imagedec_stop(id)!=TRUE)
        AD_IMG_PRINTF("\n Stop Image decoding unsuccessfully!\n");
	if(imagedec_release(id)!=TRUE)
        AD_IMG_PRINTF("\n Release Image decoder unsuccessfully!\n");
}

INT32 ad_pic_img_draw(UINT8* file, UINT32 file_len, struct OSDRect *rect)
{

	img_file_addr = (UINT8*)file;
	img_file_size = file_len;

	ad_pic_img_dec(rect);
	
	return SUCCESS;
}

void ad_pic_img_clear(void)
{
	img_file_addr = (UINT8*)NULL;
	img_file_size = 0;
}

void ad_pic_img_close(void)
{
   AD_IMG_PRINTF("\n Close the IMG AD \n");
}

#endif
#endif


