#include <sys_config.h>

#ifdef SHOW_PICS
#if(SHOW_PICS == SHOW_SINGLE_PIC)

#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/ge/ge.h>
#include <hld/osd/osddrv_dev.h>
#include <hld/dis/vpo.h>
#include <api/libchunk/chunk.h>

#include "ad_pic.h"
#include "ad_pic_png.h"
#include "ad_pic_gif.h"
#include "ad_pic_img.h"
#include "ad_pic_gfx.h"


#include "../geeya_ad/adv_geeya.h"

static UINT8 channel_adv_buf[30*1024];

#define AD_DEBUG

#ifdef AD_DEBUG
#define AD_PRINTF		libc_printf
#else
#define AD_PRINTF(...)
#endif


// The chunk ID of the AD pictures in flash
static UINT32 ad_pic_id[4]={AD_PNG_ID, AD_GIF_ID, AD_JPG_ID,AD_BMP_ID};

static AD_FILE ad_pic_info[4] = 
{
	{
        0,
		{60,160,360,240} //{60,160,240,240}  // 
	},
	{
	    0,
		{120,160,360,240}
	},
	{
	    0,
		{180,160,360,240}
	},
	{
	    0,
		{240,160,360,240}
	},
};

void ad_pic_init(UINT8 type)
{
    struct osd_device  *g_osd_layer2_dev;
    g_osd_layer2_dev =(struct osd_device*)dev_get_by_id(HLD_DEV_TYPE_OSD, 1);
    if (g_osd_layer2_dev == NULL)
	{
		AD_PRINTF("%s() OSD_layer2 is not be opened!\n", __FUNCTION__);
	}
    else
    {
		OSDDrv_ShowOnOff((HANDLE)g_osd_layer2_dev,0);
        OSDDrv_Close((HANDLE)g_osd_layer2_dev);
        AD_PRINTF("%s() OSD_layer2 is closed!\n", __FUNCTION__);
		osal_task_sleep(100);
    }    
	ad_pic_gfx_init(type);
}


void ad_pic_exit(void)
{
    UINT8 i,len;
    len = sizeof(ad_pic_info)/sizeof(AD_FILE);
    for(i=0;i<len;i++)
    {
       if(0 != ad_pic_info[i].flag)
       {
           switch(i)
           {
               case 0:
					 ad_pic_png_close();
					 ad_pic_info[i].flag=0;
					 break;
               case 1:
                     ad_pic_gif_close();
                     ad_pic_info[i].flag=0;
                     break;
               case 2:
                     ad_pic_info[i].flag=0;
                     break;
               case 3:
                     ad_pic_info[i].flag=0;
                     break;
           }
       }
    }
	ad_pic_gfx_exit(0);
}

void close_ad_pic(UINT8 type)
{
	if((type != 1) && (type !=2))
		return;
    ad_pic_gif_close();
	ad_pic_png_close();
	ad_pic_gfx_clear(type);
	ad_pic_gfx_exit(type);
	MEMSET(channel_adv_buf,0,sizeof(channel_adv_buf));
	ad_pic_img_clear();
}

static void ad_np_scale(void)
{
	enum TVSystem out_sys;
	BOOL pal = TRUE;
	static BOOL last_system = FALSE;
	struct vpo_device *vpo_dev;
	struct ge_device* ge_dev;
	ge_scale_par_29e_t par;
	
	vpo_dev = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0);
	vpo_ioctl(vpo_dev, VPO_IO_GET_OUT_MODE, (UINT32)(&out_sys));

	switch (out_sys)
	{
		case NTSC:
		case NTSC_443:
			pal = FALSE;
			break;
		default:
			break;
	}

	if (pal != last_system)
	{
		last_system = pal;
		if (pal)
			par.dst_h = 576;
		else
			par.dst_h = 480;
		
		par.src_h = 576;
		par.suf_id = 0;

		ge_dev = (struct ge_device *)dev_get_by_id(HLD_DEV_TYPE_GE, 0);
		ge_io_ctrl(ge_dev, GE_IO_SCALE_OSD_29E, (UINT32)&par);
	}
}


static void ad_pic_show( UINT8 num, UINT8 *buf, UINT32 len, struct OSDRect rect)
{
    ad_np_scale();
    switch(ad_pic_id[num])
    {
		case AD_PNG_ID:
          	AD_PRINTF("\n The PNG AD is showed \n");
            ad_pic_info[num].flag = 1;
//            ad_pic_png_draw((png_file)num,buf,&rect);
			ad_pic_png_draw(buf,len,&rect);
          	break;
        case AD_GIF_ID:
          	AD_PRINTF("\n The GIF AD is showed \n");
            ad_pic_info[num].flag = 1;
            //If do not need show animation gif, we could set last parameter to FALSE, 
		    //and free buffer for gif file immediately 
		    ad_pic_gif_init();
            ad_pic_gif_draw((gif_file)num, buf, len, &rect, FALSE); //num is used as the filehand
          	break;
		case AD_JPG_ID:
          	AD_PRINTF("\n  The JPG AD is showed \n");
            ad_pic_info[num].flag = 1;
            ad_pic_img_draw(buf, len, &rect);
          	break;
        case AD_BMP_ID:
          	AD_PRINTF("\n The BMP AD is showed \n");
            ad_pic_info[num].flag = 1;
            ad_pic_img_draw(buf, len, &rect);
          	break;
        default:
            break;
    }
}


static struct sto_device *flash_dev = NULL;

void show_flash_pic(UINT8 num)
{
	UINT32 offset,len,id;
    INT32 result;
	UINT8 *buffer;

    num = num %(sizeof(ad_pic_id)/sizeof(UINT32));
    id = ad_pic_id[num];

	//---show AD pic from flash here
	CHUNK_HEADER chunk_hdr;
	if (sto_get_chunk_header(id, &chunk_hdr) == 0)
		return;
	offset = (sto_chunk_goto(&id, 0xFFFFFFFF,1) + CHUNK_HEADER_SIZE);
	len = chunk_hdr.len - CHUNK_HEADER_SIZE + CHUNK_NAME;
	buffer = (UINT8 *)__MM_IMAGEDEC_BUF_ADDR;
    
	flash_dev = (struct sto_device*)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
	if (flash_dev == NULL)
	{
		AD_PRINTF("\n Can't find FLASH device!\n");
        return;
	}
    
	result = sto_get_data(flash_dev, buffer, offset, len);
    if(ERR_DEV_ERROR == result)
    {
        AD_PRINTF("\n Can't get the pic from flash successfully \n");
        return;
    }

    ad_pic_show(num,buffer,len,ad_pic_info[num].rect);
	ad_pic_show_on();
	//---show AD pic from flash over
}

#endif

static void show_channel_pic_ad( UINT8 pic_type, UINT8 *buf, UINT32 len, struct OSDRect rect)
{
    ad_np_scale();
    switch(pic_type)
    {
   #if 0    //reserve after use
		case ADV_PIC_TYPE_PNG:
          	AD_PRINTF("\n The PNG AD is showed \n");
//            ad_pic_png_draw((png_file)num,buf,&rect);
			ad_pic_png_draw(buf,len,&rect);
          	break;
   #endif
        case ADV_PIC_TYPE_GIF:
          	AD_PRINTF("\n The GIF AD is showed \n");
            //If do not need show animation gif, we could set last parameter to FALSE, 
		    //and free buffer for gif file immediately 
		    ad_pic_gif_init();
			AD_PRINTF("\n The gif init ok \n");
            ad_pic_gif_draw((gif_file)1, buf, len, &rect, FALSE); //num is used as the filehand
          	break;
		case ADV_PIC_TYPE_JPG:
          	AD_PRINTF("\n  The JPG AD is showed \n");
            ad_pic_img_draw(buf, len, &rect);
          	break;
        case ADV_PIC_TYPE_BMP:
          	AD_PRINTF("\n The BMP AD is showed \n");
            ad_pic_img_draw(buf, len, &rect);
          	break;
        default:
            break;
    }
}

int show_channel_ad(UINT8 adv_type,UINT16 sid ,UINT8 *pic_type)
{
	
	sGYAdInfo_t  pGetAdInfo ;
	
	struct OSDRect rect[2] ={{500,400,178,100},{268,/*0*/445,178,100}};
	UINT8 index = 0xff;
	*pic_type = ADV_PIC_TYPE_UNKNOWN;
	
	if((adv_type != SHOW_GYADV_CHANNEL_CHANGE)&&(adv_type != SHOW_GYADV_MAIN_MENU))
		return ERR_FAILURE;
	
	pGetAdInfo.cAdData = channel_adv_buf;
    if( gyad_get_channel_ad(sid, adv_type, &pGetAdInfo)!= SUCCESS )
	{	
		MEMSET(channel_adv_buf,0,sizeof(channel_adv_buf));
		//libc_printf("----------- don't find adv src -----------\n");
		return ERR_FAILURE;
	}

	if(adv_type == SHOW_GYADV_CHANNEL_CHANGE)
	{
		ad_pic_init(1);
		index = 0;
	}
	else if(adv_type == SHOW_GYADV_MAIN_MENU)
	{
		ad_pic_init(2);
		index = 1;
	}
	else
		return ERR_FAILURE;
	
	if((pGetAdInfo.cAdData[0] == 0x47)&&(pGetAdInfo.cAdData[1]==0x49)&&(pGetAdInfo.cAdData[2]==0x46)&&
		(pGetAdInfo.cAdData[3]==0x38))//ADV_PIC_TYPE_GIF	
	{	
		show_channel_pic_ad(ADV_PIC_TYPE_GIF,pGetAdInfo.cAdData,pGetAdInfo.dwAdLen,rect[index]);
		*pic_type = ADV_PIC_TYPE_GIF;
	}	
	else if((pGetAdInfo.cAdData[0] == 0xff)&&(pGetAdInfo.cAdData[1]==0xd8)&&(pGetAdInfo.cAdData[2]==0xff)&&
		(pGetAdInfo.cAdData[3]==0xe0||pGetAdInfo.cAdData[3] == 0xe1)) //yangdiaozhi@geeya for support exif app of jpeg
	{	
		*pic_type = ADV_PIC_TYPE_JPG;
		show_channel_pic_ad(ADV_PIC_TYPE_JPG,pGetAdInfo.cAdData,pGetAdInfo.dwAdLen,rect[index]);			
	}
	else if((pGetAdInfo.cAdData[0] == 0x42)&&(pGetAdInfo.cAdData[1]==0x4d)&&(pGetAdInfo.cAdData[2]==0x16)&&
		(pGetAdInfo.cAdData[3]==0xd4))
	{	
		*pic_type = ADV_PIC_TYPE_BMP;
		show_channel_pic_ad(ADV_PIC_TYPE_BMP,pGetAdInfo.cAdData,pGetAdInfo.dwAdLen,rect[index]);						
	}
	else if((pGetAdInfo.cAdData[0] == 0x89)&&(pGetAdInfo.cAdData[1]==0x50)&&(pGetAdInfo.cAdData[2]==0x4e)&&
		(pGetAdInfo.cAdData[3]==0x47))
	{	
		close_ad_pic(index+1);
		return ERR_FAILURE; //png
	}
	else
	{
		close_ad_pic(index+1);
		return ERR_FAILURE;
	}
	ad_pic_show_on();
	return SUCCESS;    
}

extern BOOL get_main_menu_adv_show_status(UINT8 *adv_type);
void refresh_main_menu_adv(void)
{
	sGYAdInfo_t  pGetAdInfo ;
	struct OSDRect rect = {268,445,178,100};
	UINT8 main_pic_type = ADV_PIC_TYPE_UNKNOWN;

	if(!get_main_menu_adv_show_status(&main_pic_type))
		return ;
	
	pGetAdInfo.cAdData = channel_adv_buf;
    if( gyad_get_channel_ad(0, SHOW_GYADV_MAIN_MENU, &pGetAdInfo)!= SUCCESS )
	{	
		MEMSET(channel_adv_buf,0,sizeof(channel_adv_buf));
		return ERR_FAILURE;
	}
	if(ADV_PIC_TYPE_GIF == main_pic_type)
	{	
		show_channel_pic_ad(ADV_PIC_TYPE_GIF,pGetAdInfo.cAdData,pGetAdInfo.dwAdLen,rect);
	}	
	else if(ADV_PIC_TYPE_JPG == main_pic_type)
	{	
		show_channel_pic_ad(ADV_PIC_TYPE_JPG,pGetAdInfo.cAdData,pGetAdInfo.dwAdLen,rect);			
	}
	else if(ADV_PIC_TYPE_BMP == main_pic_type)
	{	
		show_channel_pic_ad(ADV_PIC_TYPE_BMP,pGetAdInfo.cAdData,pGetAdInfo.dwAdLen,rect);						
	}
	else
		return;
	ad_pic_show_on();
}


#endif

