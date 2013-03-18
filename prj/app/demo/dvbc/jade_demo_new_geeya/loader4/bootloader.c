/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 Copyright (C)
*
*    File:    bootloader.c
*
*    Description:    This file contains all globe micros and functions declare
*                    of Flash boot loader.
*    History:
*           Date            Athor        Version          Reason
*       ============    =============   =========   =================
*   1.  Oct.28.2003     Justin Wu       Ver 0.1    Create file.
 *  2.  2006.5.16       Justin Wu       Ver 0.2    STO chunk support & clean up.
*****************************************************************************/
#include <sys_config.h>
#include <sys_parameters.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libchunk/chunk.h>
#include <bus/sci/sci.h>
#include <bus/flash/flash.h>
#include <bus/dog/dog.h>
#include <hal/hal_gpio.h>
#include <hal/hal_mem.h>
#include <hld/pan/pan_dev.h>
#include <hld/pan/pan.h>
#include <api/libstbinfo/stb_info.h>
#include <api/libstbinfo/stb_info_data.h>
#include <api/libloader/ld_boot_info.h>
#include "../copper_common/stb_data.h"

#ifndef BL_PRINTF
#define BL_PRINTF PRINTF
#endif

#ifdef HW_SET_BOOTLOGO
#undef HW_SET_BOOTLOGO
#endif
#ifdef HW_SET_GPIO
#undef HW_SET_GPIO
#endif
#ifdef HW_SET_BOOTTYPE
#undef HW_SET_BOOTTYPE
#endif
#ifdef HW_SET_PANTYPE
#undef HW_SET_PANTYPE
#endif

#define HW_SET_BOOTLOGO		0x9C
#define HW_SET_GPIO			0x104
#define HW_SET_BOOTTYPE		0x12D
#define BOOT_TYPE_MASK			0x80
#define HW_SET_PANTYPE			0x9F
#define PAN_TYPE_MASK			0xE0
#define STB_INFO_INDEX			0x01

#ifdef CHUNKID_MAINCODE
#undef CHUNKID_MAINCODE
#endif
#ifdef CHUNKID_SEECODE
#undef CHUNKID_SEECODE
#endif

#define CHUNKID_MAINCODE 0x01FE0100
#define CHUNKID_OTA 0x00FF0100
#define CHUNKID_STBINFO 0x20DF0100

#define US_TICKS        (SYS_CPU_CLOCK / 2000000)
#define WAIT_300MS      (300000 * US_TICKS)
#define WAIT_500MS		(500000 * US_TICKS)
#define WAIT_5MS        (5000   * US_TICKS)
#define WAIT_1MS        (1000   * US_TICKS)

/*****************************
*SDRAM mapping in bootloader
******************************
	________________________       2+8                      8+32                     4+16(need check)
	|					¡ý	| ¡ú0X80800000 (8M)          0x82000000(32M)          0x81000000(16M)  
	|FRAME BUFF			    |
	|-----------------------| ¡ú0X8070000 (7M)           0x81D00000(29M)          0X80D00000(13M)
	|                       |
	|					 	|//#define SYS_FLASH_SIZE 0x800000						
	|CHUNK BUFF			¡ü	|//a chunks image which maps from flash for program check, expand, and ota						
	|-----------------------| ¡ú0X80480000 (4.5M)	     0X81500000(21M)          0X80080000(8M)
	|						|
	|BOOTLOADER CODE	¡ü	|//bootloader code start addr
	|-----------------------| ¡ú0X80400000 (4M)		     0X800A0000(10M)          0X80050000(5M)
	|						|
	|MAIN CODE			¡ü	|//MAIN_ENTRY: maincode start addr	
	|-----------------------| ¡ú0X80000200			     0X80000200               0X80000200
	|_______________________|
*/
#if(SYS_SDRAM_SIZE==8)
#define BOOT_BASE_ADDR 0x80400000
#elif(SYS_SDRAM_SIZE==16)
#define BOOT_BASE_ADDR 0x80500000
#elif(SYS_SDRAM_SIZE==32)
#define BOOT_BASE_ADDR 0x80a00000
#endif

#define MAIN_ENTRY 0x80000200
#if(SYS_SDRAM_SIZE==8)
#define FLASH_IMAGE_BASE_ADDR 0X80480000
#elif(SYS_SDRAM_SIZE==16)
#define FLASH_IMAGE_BASE_ADDR 0X80800000
#elif(SYS_SDRAM_SIZE==32)
#define FLASH_IMAGE_BASE_ADDR 0X81500000
#endif

#define GET_DWORD(addr)            (*(volatile UINT32 *)(addr))
#define SET_DWORD(addr, d)         (*(volatile UINT32 *)(addr)) = (d)

/* Calcuate InternalBufferSize for 7-zip */
#define LZMA_BASE_SIZE  1846
#define LZMA_LIT_SIZE   768
#define BUFFER_SIZE     ((LZMA_BASE_SIZE + (LZMA_LIT_SIZE << (0 + 2))) * sizeof(UINT16))

#define WRITE_UINT8(uAddr, bVal)    do {*(volatile UINT8 *)(uAddr) = (bVal);} while(0)

struct pan_device *g_pan_dev;
struct sto_device *g_sto_dev;

UINT32  g_crc_bad;

BOOL stb_info_ok = FALSE;


extern int gzip_decompress(UINT8 *, UINT8 *, UINT8 *);
extern int un7zip(UINT8 *, UINT8 *, UINT8 *);
//extern int unlzo(UINT8 *, UINT8 *, UINT8 *);

UINT8 *expand(UINT32 offset, int unzip(UINT8 *, UINT8 *, UINT8 *))
{
	UINT8 *entry;
	UINT8 *buffer;
	UINT32 code_len;
	UINT8 *codestart;
	UINT32 zip_type;

	codestart = (UINT8 *)fetch_long((unsigned char *)offset + CHUNK_CODESTART);
	entry = (UINT8 *)fetch_long((unsigned char *)offset + CHUNK_ENTRY);
	
	if (codestart == 0 || codestart == (UINT8 *)0xFFFFFFFF)
		codestart = (UINT8 *)MAIN_ENTRY;
	
	if (entry == 0 || entry == (UINT8 *)0xFFFFFFFF)
		entry = codestart;
	
	code_len = fetch_long((unsigned char *)offset + CHUNK_LENGTH);
	zip_type = fetch_long((unsigned char *)offset + CHUNK_USER_VERSION);

	BL_PRINTF("11 offset=0x%x\n",offset - FLASH_IMAGE_BASE_ADDR);	
	BL_PRINTF("11 temp_start_addr=0x%x, size=0x%x, zip type=%d, \n",offset, code_len, zip_type);	

	if(zip_type == 0)
	{//uncompressed maincode

		MEMCPY(codestart, (void *)(offset + CHUNK_HEADER_SIZE), (code_len -CHUNK_HEADER_SIZE + 16));
		BL_PRINTF("11-1 read maincode to ram finish\n");		
	}
	else
	{
		/* Copy compressed code into DRAM */
		buffer = MALLOC(BUFFER_SIZE);
		if (buffer == NULL)
		{
			FIXED_PRINTF("Boot loader: No decompress buffer!\n");
			entry = 0;
		}
		else
		{
			if (unzip((void *)(offset + CHUNK_HEADER_SIZE), codestart, buffer) != 0)
			{
				FIXED_PRINTF("Boot loader: Decompress error!\n");
				entry = 0;
			}

			BL_PRINTF("Decompress len:0x%x\n", *(UINT32*)buffer);
			FREE(buffer);
		}
	}

    return entry;
}

/*support config OS resource
default setting: set_os_config(0,0,0,0,0);
*/
void set_os_config(UINT16 task_num, UINT16 sema_num, UINT16 flg_num, UINT16 mbf_num, UINT16 mutex_num)
{
	UINT16 * ptr = (UINT16*)0xa00001e0;

	ptr[0]= task_num;
	ptr[1]= sema_num;
	ptr[2]= flg_num;
	ptr[3]= mbf_num;
	ptr[4]= mutex_num;
	ptr[5]= task_num + sema_num + flg_num + mbf_num + mutex_num;
}

INT32 enter_standby_check()
{
	UINT32 boot_code;
	UINT8 boot_type;
	
	// Cold bootup
	boot_code = *(UINT32 *)(0xA0000110);
	boot_type = *(UINT8 *)(SYS_FLASH_BASE_ADDR + HW_SET_BOOTTYPE);
	if (boot_code == 0xdead3bee)
	{
		/* If cold bootup and enter standby enabled, enter standby */
		if(boot_type & BOOT_TYPE_MASK)
			return 1;		
	}
	return 0;
}

INT32 enter_rs232upg_check()
{
	UINT32 boot_tick;
	UINT8 ch=0;

#ifndef ENABLE_EROM
	boot_tick = read_tsc();
	do
	{
		if (sci_read_tm(boot_uart_get_id(), &ch, 5) == SUCCESS && ch == 'c')
			break;
		else
			ch = 0;
	}while (read_tsc() - boot_tick < WAIT_300MS);
	
	if (ch == 'c')
		return 1;
	else
#endif		
		return 0;
}

void boot_hw_init(void)
{
	/*pinmux*/
	boot_pin_mux_config();

	/*sci*/
#if 1//ndef ENABLE_EROM
      sci_16550uart_attach(1);    /* Use just 1 UART for RS232 */
      sci_mode_set(SCI_FOR_RS232, 115200, SCI_PARITY_EVEN);
      FIXED_PRINTF("BL: APP  init!\n");
#endif

	/*panel*/
	front_panel_init((UINT8*)(SYS_FLASH_BASE_ADDR+HW_SET_GPIO));

	/*flash*/
	flash_init();


}

/* front panel key code*/
#define PAN_HKEY_MENU	0xFFFF0002
#define PAN_HKEY_ENTER	0xFFFF0004

// ENTER --> MENU
BOOL boot_key_check(void)
{
	struct pan_key *pan_key = NULL;
	UINT32 boot_tick, n = 0;;
	
	/* if get front panel key , enter upg menu */	
	n = 0;
	boot_tick = read_tsc();
	do
	{
		pan_key = pan_get_key(g_pan_dev, 1);
		if(pan_key!=NULL)
		{
			if((pan_key->type == PAN_KEY_TYPE_PANEL) && (pan_key->code == PAN_HKEY_ENTER))
			{
				n++;
				break;	
			}
		}		
	}while (read_tsc() - boot_tick < 350*WAIT_1MS);	

	if(n == 0)
	{
		return FALSE;
	}

	set_pan_display_type(PAN_DISP_LED_ONLY);
	stb_pan_display("LD-1", 4);	
	n = 0;
	boot_tick = read_tsc();
	do
	{
		pan_key = pan_get_key(g_pan_dev, 1);
		if(pan_key!=NULL)
		{
			if((pan_key->type == PAN_KEY_TYPE_PANEL) && (pan_key->code == PAN_HKEY_MENU))
			{
				n++;
				break;	
			}
		}		
	}while (read_tsc() - boot_tick < 2000*WAIT_1MS);	

	set_pan_display_type(PAN_DISP_LED_BUF);

	if(n == 0)
	{
		stb_pan_display_revert();	
		return FALSE;
	}

	stb_pan_display("LOAD", 4);	

	return TRUE;
}
UINT8 restore_backup_otaloader=0;
int boot_enter_upgloader(STB_INFO_DATA *stb_data)
{
	int ret = -1, _ret = -1;
	CHUNK_HEADER upgloader_head, backup_head;
	unsigned char  *m_pointer;
	UINT32 chunk_id, backup_addr, backup_max_len;
	UINT8 upgloader_bad = 0, backuploader_bad = 0;

	if(stb_data == NULL)
		return ret;

	chunk_id = stb_data->param1;
	backup_addr = stb_data->param2;
	backup_max_len = stb_data->param3;

	/*check upgloader*/
	m_pointer = NULL;
	_ret = load_chunk_by_id(chunk_id, &backup_max_len, &m_pointer);
	if(_ret !=0 )
	{
		upgloader_bad = 1;
	}
	else
	{
		MEMCPY(&upgloader_head, m_pointer, CHUNK_HEADER_SIZE);
		if(m_pointer != NULL)
			FREE(m_pointer);
	}
	
	/*check backup loader*/
	m_pointer = NULL;
	backup_max_len = stb_data->param3;
	_ret = load_chunk_by_addr(backup_addr, &backup_max_len, &m_pointer);
	if(_ret !=0 )
	{
		backuploader_bad = 1;
	}
	else
	{
		MEMCPY(&backup_head, m_pointer, CHUNK_HEADER_SIZE);
		
		if(m_pointer != NULL)
			FREE(m_pointer);
	}

	/*upgloader & backup loader both crash, set upg_forbidden flag*/
	if(upgloader_bad==1 && backuploader_bad==1)
	{
		libc_printf("upgloader & backup loader both crash!\n");
		stb_data->upg_info.c_info.upg_forbidden = 1;
		return ret;
	}
	
	// do 2->1 restore back old upgloader
	// 1.last upgrade not finish but burned upgloader; 2. upgloader crash
	if(backuploader_bad == 0)
	{// backup loader ok
		if(((stb_data->state == STB_INFO_DATA_STATE_PREBURN)&&(stb_data->upg_info.c_info.upg_needbackup != 0))
			|| upgloader_bad==1)
		{// 1.last upgrade not finish but burned upgloader; 2. upgloader crash
			set_pan_display_type(PAN_DISP_LED_ONLY);
			
			BL_PRINTF("do 2->1 restore back old upgloader, upgloader bad:%d\n", upgloader_bad);
			stb_pan_display("b2-1", 4);
			if(restore_upgloader(stb_data->param1, stb_data->param2, stb_data->param3) < 0)
			{
				libc_printf("%s:%d ERROR: restore_upgloader 2 -> 1 fail!\n", __FUNCTION__, __LINE__);
				stb_pan_display("E2-1", 4);
				SDBBP();
			}
			
			//clear flag and save
			if(stb_data->upg_info.c_info.upg_needbackup != 0)
				stb_data->upg_info.c_info.upg_needbackup = 0;
			//stb_info_data_save();

			stb_pan_display_revert();
			set_pan_display_type(PAN_DISP_LED_BUF);
			
            		//if otaloader error and restore  need copy backup otaloader to memery
	    		if(0==sto_chunk_check_ext((UINT32)FLASH_IMAGE_BASE_ADDR, (UINT32)SYS_FLASH_SIZE)&&(stb_data->state != STB_INFO_DATA_STATE_PREBURN))
	    		{   //if stb_data->state == STB_INFO_DATA_STATE_PREBURN means STB is in OTA ,so when ota restored,STB need back to OTA MODE 
	               		restore_backup_otaloader=1;
	    		}
		}
	}

	// do 1->2,backup upgloader
	// 1.upgloader != backup loader; 2.backup loader crash
	if(upgloader_bad == 0)
	{// upgloader ok
		if(MEMCMP(&upgloader_head, &backup_head, sizeof(CHUNK_HEADER))!=0
			||backuploader_bad==1)
		{// 1.upgloader != backup loader; 2.backup loader crash
			set_pan_display_type(PAN_DISP_LED_ONLY);

			BL_PRINTF("do 1->2,backup upgloader, backloader bad:%d\n", backuploader_bad);
			stb_pan_display("b1-2", 4);
			if(backup_upgloader(stb_data->param1, stb_data->param2, stb_data->param3) < 0)
			{
				libc_printf("%s:%d ERROR: backup_upgloader 1 -> 2 fail!\n", __FUNCTION__, __LINE__);
				stb_pan_display("E1-2", 4);
				SDBBP();
			}

			stb_pan_display_revert();
			set_pan_display_type(PAN_DISP_LED_BUF);
		}
	}

	ret = 0;
	return ret;
}

void AppInit(void)
{
	UINT8  *t1, *t2, i;
	UINT32 chid=0;
	UINT32 offset,offset2=0;
	UINT8 *main_entry,*see_entry,*see_ram_address;
	UINT8  boot_type;
	UINT32 boot_code;
	UINT32 index,addr;
	UINT32 data,tmp;
	UINT32 tick1,tick2;
	UINT8 *ch_data;
	UINT32 stb_info_crc_bad = 0;
	STB_INFO_DATA *stb_data = NULL;
	STB_INFO_DATA stb_data_temp;
	
//SDBBP();
	tick1 = read_tsc();

	MG_Setup_CRC_Table();	

	boot_hw_init();

    /*enable M3202C POK for flash power loss protection*/
	if((1==sys_ic_is_M3202())&&(sys_ic_get_rev_id()>=IC_REV_4))
  		pdd_init(0, 0);
    
	if(stb_info_data_load(STB_CHUNK_STBINFO_ID)!=0)
	{
		BL_PRINTF("no valid stb data, now need init it!\n");
		stb_info_data_init(&stb_data_temp);
		stb_data = &stb_data_temp;		
	}
	else
	{
		stb_data = stb_info_data_get();
	}

#ifdef SHOW_BOOT_LOGO
//	if(g_crc_bad == 0)
		boot_logo(stb_data, (UINT8*)(HW_SET_BOOTLOGO));
#endif

	g_crc_bad = sto_chunk_check_ext((UINT32)FLASH_IMAGE_BASE_ADDR, (UINT32)SYS_FLASH_SIZE);
	stb_info_crc_bad = (STB_INFO_INDEX == g_crc_bad>>16);
	BL_PRINTF("boot crc check=0x%x, stbinfo crc bad %d\n",g_crc_bad, stb_info_crc_bad);	
	/*backup upgloader for upgloader update*/
	if((stb_data->state != STB_INFO_DATA_STATE_PREBURN) && (stb_data->upg_info.c_info.upg_needbackup != 0))
	{
		set_pan_display_type(PAN_DISP_LED_ONLY);
		
		BL_PRINTF("upgloader updated, now need backup it!\n");
		stb_pan_display("b1-2", 4);	
		backup_upgloader(stb_data->param1,stb_data->param2,stb_data->param3);

		stb_data->upg_info.c_info.upg_needbackup = 0;
		stb_info_data_save();

		stb_pan_display_revert();	
		set_pan_display_type(PAN_DISP_LED_BUF);
	}

	BL_PRINTF("upg_forbidden: %d\n", stb_data->upg_info.c_info.upg_forbidden);
	
	if(stb_data->upg_info.c_info.upg_forbidden == 0)
	{
		BL_PRINTF("upg_trigger_type:%d\n",stb_data->upg_info.c_info.upg_trigger_type);
		
		/*clear trigger info*/
		if(stb_data->upg_info.c_info.upg_trigger_type != STB_INFO_UPG_TRIGGER_APPOTA)
		{
			stb_data->upg_info.c_info.upg_trigger_type = STB_INFO_UPG_TRIGGER_NONE;
		}
		
		if(stb_data->state & STB_INFO_DATA_STATE_PREBURN)
		{
			BL_PRINTF("upg_trigger_type: STB_INFO_UPG_TRIGGER_INTERRUPT\n");
			stb_data->upg_info.c_info.upg_trigger_type = STB_INFO_UPG_TRIGGER_INTERRUPT;
		}
		
		if((stb_data->state & STB_INFO_DATA_STATE_LIMIT) || g_crc_bad)
		{
			BL_PRINTF("upg_trigger_type: STB_INFO_UPG_TRIGGER_BOOTFAIL. stb_data state %d, crc:0x%x\n", stb_data->state, stb_info_crc_bad);
			stb_data->upg_info.c_info.upg_trigger_type = STB_INFO_UPG_TRIGGER_BOOTFAIL;
		}

		/* if get front panel key, enter upg menu */
		if(boot_key_check())
		{
			BL_PRINTF("upg_trigger_type: STB_INFO_UPG_TRIGGER_BOOTKEY\n");
			stb_data->upg_info.c_info.upg_trigger_type = STB_INFO_UPG_TRIGGER_BOOTKEY;
		}

		if(stb_data->upg_info.c_info.upg_trigger_type!= STB_INFO_UPG_TRIGGER_NONE)
		{
			if(boot_enter_upgloader(stb_data)==0)
			{
				BL_PRINTF("boot_enter_upgloader ok.\n");
				if(restore_backup_otaloader==1){
					chid=CHUNKID_MAINCODE;
					restore_backup_otaloader=0;
				}else
					chid = CHUNKID_OTA;
			}
			else
			{
				BL_PRINTF("boot_enter_upgloader fail.\n");
			}

			stb_info_data_save();
		}
		
	}

	if(chid == 0)
	{
		if (g_crc_bad != 0)
		{
			BL_PRINTF("bootloader: CRC bad(%d).\n", g_crc_bad);
			goto BOOT_END;
		}

            	chid = CHUNKID_MAINCODE;
	}
	chunk_init((UINT32)FLASH_IMAGE_BASE_ADDR, SYS_FLASH_SIZE);
	offset = (UINT32)chunk_goto(&chid, 0xFFFFFFFF, 1);

	main_entry = 0;
//	if(chid == CHUNKID_MAINCODE)
//		main_entry = expand(offset, unlzo);
//	else
		main_entry = expand(offset, un7zip);
	
        if (main_entry != 0)
	{
#if 0	
		if(enter_rs232upg_check()==1)
		{
			BL_PRINTF("bootloader: get rs232 command.\n");
			goto BOOT_END;
		}
#endif			
		/* If cold bootup and enter standby enabled, enter standby */
		if(enter_standby_check()==1)
			sys_ic_enter_standby(0, 0);

		/*you can config OS resource here,
		default setting: set_os_config(0,0,0,0,0);	*/
		set_os_config(128,64,64,64,64);/*Call it before we enter main code*/

		stb_pan_display("    ", 4);

		tick2 = read_tsc();

		BL_PRINTF("boot time: %ds\n",(tick2-tick1)/WAIT_1MS);

		exec(main_entry);
	}
	
 BOOT_END:

	stb_pan_display("    ", 4);

#ifndef ENABLE_EROM
	lib_ash_shell();
#endif
	while(1);
}

