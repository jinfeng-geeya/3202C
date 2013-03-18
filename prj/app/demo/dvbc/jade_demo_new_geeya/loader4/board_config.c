#include <sys_config.h>
#include <sys_parameters.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libchunk/chunk.h>
#include <bus/sci/sci.h>
#include <bus/flash/flash.h>
#include <hal/hal_gpio.h>
#include <hal/hal_mem.h>
#include <hld/pan/pan_dev.h>
#include <hld/pan/pan.h>

#include "../copper_common/stb_data.h"

extern struct pan_device *g_pan_dev;
extern struct sto_device *g_sto_dev;

void osal_delay_ms(UINT32 ms)
{
    UINT32 i;
    for (i = 0; i < ms; i++)
        osal_delay(1000);
}

void boot_pin_mux_config(void)
{
#if (SYS_MAIN_BOARD == BOARD_S3202C_DEMO_01V01)
	*((volatile UINT32*)0xb8000100) |= ((1 << 2)|(1 << 3)|(1 << 6)|(1 << 12)|(1 << 13)|(1 << 14)|(1 << 18) ); /* RF_AGC_PDM[b17&b18],GPIO2[b1],GPIO7[b16] */
	*((volatile UINT32*)0xb8000104) = 0; 

	//enable gpio
	*((volatile UINT32*)0xb8000040) = ((1 << 0) | (1 << 26) | (1 << 27) | (1 << 28));
	*((volatile UINT32*)0xb80000A0) = ((1 << 1) | (1 << 17)); 
	*((volatile UINT32*)0xb80000D0) = 0; 
	//enable GPIO[1] power	
	*((volatile UINT32*)0xb8000100) |= (1 << 12);	//pinmux spdif gpio[1]
	*((volatile UINT32*)0xb8000040) |= (1<<0);	// gpio enable
	*((volatile UINT32*)0xb8000058) |= (1<<0);	// output enable
	*((volatile UINT32*)0xb8000054) |= (1<<0);	// output 1
	//*((volatile UINT32*)0xb8000074) |= ((1<<4 )| (1<<25));//154M MEMERY
#elif(SYS_MAIN_BOARD == BOARD_S3202C_DEMO_02V01)	
	*((volatile UINT32*)0xb8000100) |= ((1 << 2)|(1 << 3)|(1 << 6)|(1 << 12)|(1 << 13)|(1 << 14)|(1 << 18) ); /* RF_AGC_PDM[b17&b18],GPIO2[b1],GPIO7[b16] */
#ifdef SSI_TS_OUT_IN_SET
	*((volatile UINT32*)0xb8000100) &=~((1 << 18)|(1 << 19)|(1 << 21));
	*((volatile UINT32*)0xb8000100) 	|=((1 << 20)|(1<<24));
#endif
	*((volatile UINT32*)0xb8000104) = 0; 

	//enable gpio
	*((volatile UINT32*)0xb8000040) = ((1 << 23) | (1 << 24) | (1 << 25)|(1 << 26) | (1 << 27) | (1 << 28) );
	*((volatile UINT32*)0xb80000A0) = ((1 << 0) | (1 << 1) |  (1 << 17)); 
	*((volatile UINT32*)0xb80000D0) = 0; 

	//enable GPIO[32] power	
	*((volatile UINT32*)0xb80000A0) |= (1<<0);	//enable gpio 32
	*((volatile UINT32*)0xb80000B8) |= (1<<0);	// set gpio 32 to be output
	*((volatile UINT32*)0xb80000B4) |= (1<<0);	// set gpio 32 output high level

#elif(SYS_MAIN_BOARD == BOARD_S3202C_NEWLAND_02V01)	
	*((volatile UINT32*)0xb8000100) |= ((1 << 2)|(1 << 3)|(1 << 6)|(1 << 12)|(1 << 13)|(1 << 14)|(1 << 18) ); /* RF_AGC_PDM[b17&b18],GPIO2[b1],GPIO7[b16] */
	*((volatile UINT32*)0xb8000104) = 0; 

	//enable gpio
	*((volatile UINT32*)0xb8000040) = ((1 << 23) | (1 << 24) | (1 << 25));
	*((volatile UINT32*)0xb80000A0) = ((1 << 0) | (1 << 1) | (1 << 12)|(1 << 13) | (1 << 14) | (1 << 17)); 
	*((volatile UINT32*)0xb80000D0) = 0; 

	//enable GPIO[32] power	
	*((volatile UINT32*)0xb80000A0) |= (1<<0);	//enable gpio 32
	*((volatile UINT32*)0xb80000B8) |= (1<<0);	// set gpio 32 to be output
	*((volatile UINT32*)0xb80000B4) |= (1<<0);	// set gpio 32 output high level
#elif(SYS_MAIN_BOARD == BOARD_S3202_DEMO_01V03)
    *((volatile UINT32*)0xb8000028) |= ((1 << 18) | (1 << 17) | (1 << 1) | (1 << 16)); /* RF_AGC_PDM[b17&b18],GPIO2[b1],GPIO7[b16] */
	*((volatile UINT32*)0xb800002c) |= (1 << 6); //SPIDF[b6]

	//enable hw7816 2
	*(volatile unsigned long*)(0xb800002c) |= (1 << 3);

    if(IC_REV_0!=sys_ic_get_rev_id())
    {
        /*uart1     tx/eeprom_addr[13], pin52, (2ch[1:0]  1:tx, 2:rx, 3:txrx )
                       rx/eeprom_addr[12], pin53, (28h[10:9] 0:tx, 1:rx, 2:txrx )*/
        *((volatile UINT8 *)0xb800002c) &= (~0x03);
        *((volatile UINT8 *)0xb800002c) |= 0x01;
        *((volatile UINT8 *)0xb8000029) &= (~0x06);
        *((volatile UINT8 *)0xb8000029) |= 0x02;
    }    
#endif

}

/***********************************
PANEL
************************************/
struct pan_hw_info pan_hw_info_cp;

#define bitmap_list             NULL
#define bitmap_list_num     0

struct pan_configuration pan_config = {&pan_hw_info_cp, bitmap_list_num, bitmap_list};

void front_panel_init(UINT8* addr)
{
#ifdef PANEL_DISPLAY
	UINT8  *t1, *t2, i;

	t1 = (UINT8*)&pan_hw_info_cp;
	t2 = addr;//(UINT8*)(SYS_FLASH_BASE_ADDR+HW_SET_GPIO);//&pan_hw_info;

	for(i=0; i<sizeof(struct pan_hw_info); i++)
		*(t1+i) = *(t2+i);

#if (SYS_MAIN_BOARD == BOARD_S3202C_DEMO_01V01\
	||SYS_MAIN_BOARD == BOARD_S3202C_NEWLAND_02V01\
	||SYS_MAIN_BOARD == BOARD_S3202C_DEMO_02V01)
	pan_tp6x0_attach(&pan_config);
#else
	pan_hwscan_l_attach(&pan_config);
#endif
	g_pan_dev = (struct pan_device *)dev_get_by_id(HLD_DEV_TYPE_PAN, 0);  
	pan_open(g_pan_dev);
	set_pan_display_type(PAN_DISP_LED_BUF);
	stb_pan_display(" ON ", 4);		
	
#endif
}

/***********************************
FLASH
************************************/
void flash_init(void)
{
	flash_info_sl_init();
	sto_local_sflash_attach(NULL);	
	g_sto_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
	if (g_sto_dev == NULL)
	{
		PRINTF("Can't find FLASH device!\n");
	}

	if (ALI_M3329E == sys_ic_get_chip_id() || 1 == sys_ic_is_M3202())
	{
#if (SYS_MAIN_BOARD == BOARD_S3202C_DEMO_01V01\
	||SYS_MAIN_BOARD == BOARD_S3202C_NEWLAND_02V01\
	||SYS_MAIN_BOARD == BOARD_S3202C_DEMO_02V01)
		*((volatile UINT32*)0xb8008098) &= ~0xc0000000;
		if (g_sto_dev->totol_size <= 0x200000)
			*((volatile UINT32*)0xb8008098) |= 0;
		else if (g_sto_dev->totol_size > 0x200000 && g_sto_dev->totol_size <= 0x400000)
			*((volatile UINT32*)0xb8008098) |= 0x40000000;
		else if (g_sto_dev->totol_size > 0x400000 && g_sto_dev->totol_size <= 0x800000)
			*((volatile UINT32*)0xb8008098) |= 0x80000000;
		else
			*((volatile UINT32*)0xb8008098) |= 0xc0000000;
#else	
		*((volatile UINT32*)0xb8000098) &= ~0xc0000000;
		if (g_sto_dev->totol_size <= 0x200000)
			*((volatile UINT32*)0xb8000098) |= 0;
		else if (g_sto_dev->totol_size > 0x200000 && g_sto_dev->totol_size <= 0x400000)
			*((volatile UINT32*)0xb8000098) |= 0x40000000;
		else if (g_sto_dev->totol_size > 0x400000 && g_sto_dev->totol_size <= 0x800000)
			*((volatile UINT32*)0xb8000098) |= 0x80000000;
		else
			*((volatile UINT32*)0xb8000098) |= 0xc0000000;
#endif		
	}
	
	sto_open(g_sto_dev);
	sto_chunk_init(0, g_sto_dev->totol_size);
	flash_identify();

	chunk_reg_bootloader_id(STB_CHUNK_BOOTLOADER_ID);
}

#ifdef SHOW_BOOT_LOGO
UINT16 g_blogo_format = 0;  /* Bootloader logo parameters */
UINT8 *vbv_buffer_add = NULL;

BOOL init_vbv_buffer()
{
#if(8 == SYS_SDRAM_SIZE)
	vbv_buffer_add = (UINT8*)__MM_VBV_START_ADDR;
#else
	vbv_buffer_add = MALLOC(__MM_VBV_LEN);
#endif

	if(vbv_buffer_add == NULL)
		return FALSE;
	else
		return TRUE;
}

void vdac_init(STB_INFO_DATA *stb_data)
{
	UINT8 soc_vdac_num = 0;
	UINT8 cur_vdac_type, cur_vdac = 0;
	UINT8 vdac_type_index[] = {CVBS, YC_Y, CAV_Y, RGB_R, SECAM_YC, SECAM_Y};
	int i = 0;

	soc_vdac_num = 4;

	for (i = (soc_vdac_num - 1); i>=0; i--)
	{
		cur_vdac_type = (stb_data->upg_info.c_info.vdac_out[i] >> 2) & 0x3F;
		//if(vdac_type_index[cur_vdac_type]  == CVBS)
		//		osal_task_sleep(500);
		cur_vdac = vdac_type_index[cur_vdac_type] + (stb_data->upg_info.c_info.vdac_out[i] & 0x03);
		tvenc_set_dac(i, cur_vdac);
	}
}

void vpo_init(STB_INFO_DATA *stb_data)
{
	UINT32 boot_tick;
	
        //---reset video core: tve and vpo
        *(volatile UINT8 *)0xB8000060 = 0x09;
	osal_delay(5000);
	*(volatile UINT8 *)0xB8000060 = 0x00;
	
	declogo_attach_min(__MM_BUF_TOP_ADDR,__MM_FB0_Y_LEN+__MM_FB0_C_LEN);

	//first vpo init
	vpoInitGlobalValue();
	vpoInitRegs();

	//second vdac init
	vdac_init(stb_data);
}

void boot_logo(STB_INFO_DATA *stb_data, UINT8* addr)
{
	UINT32 offset;
	UINT32 len, i;
	UINT8 *buffer;
	UINT32 id = LOGO_ID;
	UINT16 blogo_para;
	UINT8  data[2];

	if (init_vbv_buffer()==FALSE)
	{
		ASSERT(0);
	}
	
	sto_get_data(g_sto_dev, data, (UINT32)addr, 2);
	blogo_para = ((data[1] << 8) | data[0]);
	g_blogo_format = blogo_para & LOGO_FORMAT_MASK;
	if (blogo_para)
	{		
		//---show logo here
		CHUNK_HEADER chuck_hdr;

		vpo_init(stb_data);

		if (sto_get_chunk_header(id, &chuck_hdr) == 0)
			return;
		offset = (sto_chunk_goto(&id, 0xFFFFFFFF,1) + CHUNK_HEADER_SIZE);
		len = chuck_hdr.len - CHUNK_HEADER_SIZE + CHUNK_NAME;
		if(offset>SYS_FLASH_SIZE ||len > offset)
		{
			libc_printf("boot logo error, length or offset of chunk is wrong. offset[%08X] , length[%08X]\n", offset, len);
			return;
		}
		buffer = (void *)vbv_buffer_add;
		sto_get_data(g_sto_dev, buffer, offset, len);
		decLogo((UINT8 *)buffer, len);
		//---show logo over
	}
}
#endif

