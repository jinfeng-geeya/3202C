#include <sys_config.h>
#include "dev_handle.h"
#include "boot_system_data.h"
#include "stb_data.h"

#ifndef __MM_UPG_TOP_ADDR
#ifdef __MM_BUF_PVR_TOP_ADDR
#define __MM_UPG_TOP_ADDR __MM_BUF_PVR_TOP_ADDR
#else 
#define __MM_UPG_TOP_ADDR __MM_HEAP_TOP_ADDR
#endif
#endif

#define STB_DATA_DEBUG PRINTF
STB_DATA system_stb_data;

#define SYS_SW_MODEL 0x1
#define SYS_SW_VERSION 0x1

#define BACKUP_LOADER_ID 0x00FF0100
#define BACKUP_LOADER_ADDR	(STB_NOR_FLASH_SIZE-BACKUP_LOADER_LEN)
#define BACKUP_LOADER_LEN	0x20000

/* VDAC */
#if (SYS_MAIN_BOARD == BOARD_S3202C_DEMO_01V01)
UINT8 _board_vdac_config[VDAC_NUM_MAX] = 
{VDAC_CVBS, VDAC_YUV_U, VDAC_YUV_Y, VDAC_YUV_V, VDAC_NULL, VDAC_NULL};
#elif(SYS_MAIN_BOARD == BOARD_S3202C_NEWLAND_02V01||SYS_MAIN_BOARD == BOARD_S3202C_DEMO_02V01)
UINT8 _board_vdac_config[VDAC_NUM_MAX] = 
{VDAC_YUV_U, VDAC_CVBS, VDAC_YUV_Y, VDAC_YUV_V, VDAC_NULL, VDAC_NULL};
#elif(SYS_MAIN_BOARD == BOARD_S3202_DEMO_01V03)
UINT8 _board_vdac_config[VDAC_NUM_MAX] = 
{VDAC_YUV_V, VDAC_YUV_U, VDAC_YUV_Y, VDAC_CVBS, VDAC_NULL, VDAC_NULL};
#endif

static UINT8 led_display_data[5];
static UINT8 led_data_len=0;
static UINT8 led_display_flag = PAN_DISP_LED_BUF;

void set_pan_display_type(PanDispType_t type)
{
	led_display_flag = type;
}

void stb_pan_display(char *data, UINT32 len)
{
#ifdef PANEL_DISPLAY
	if(led_display_flag!=PAN_DISP_BUF_ONLY)
	{
		pan_display(g_pan_dev, data, len);
	}

	if(led_display_flag!=PAN_DISP_LED_ONLY)
	{
		MEMCPY(led_display_data, data, len);
		led_display_data[len] = '\0';
		led_data_len = len;
	}
#endif	
}

void stb_pan_display_revert(void)
{
#ifdef PANEL_DISPLAY
	pan_display(g_pan_dev, led_display_data, led_data_len);
#endif
}

int stb_info_data_init(STB_INFO_DATA *stb_info)
{
	int ret = 0, _ret = 0;
	UINT8 i = 0, j = 0;

	MEMSET(stb_info, 0, sizeof(STB_INFO_DATA));
	//stb_info->crc_len = sizeof(STB_INFO_DATA) - 8;	
	stb_info->sw_model= SYS_SW_MODEL;	
	stb_info->sw_version = SYS_SW_VERSION;
	
	STRCPY(stb_info->download_date, "2010-12-15 17:31");
	STRCPY(stb_info->firmware_date, "2010-12-15");

	stb_info->nor_enable = 1;
	stb_info->nor_flash_size = STB_NOR_FLASH_SIZE;
	stb_info->stb_part_num = 1;

	for(i=0; i<stb_info->stb_part_num; i++)
	{
		stb_info->part[i].id = i;
	}
	STRCPY(stb_info->part[0].name, STB_PART_CORE);

	stb_info->bacup_part_id = 0;
	stb_info->state = STB_INFO_DATA_STATE_NORMAL;

	stb_info->upg_info.c_info.boot_tv_mode = TV_MODE_PAL;
	stb_info->upg_info.c_info.boot_tv_ratio = TV_ASPECT_RATIO_43;
	stb_info->upg_info.c_info.boot_display_mode = DISPLAY_MODE_PANSCAN;
	stb_info->upg_info.c_info.boot_scart_out = SCART_CVBS;
	for(i=0;i<VDAC_NUM_MAX;i++)
		stb_info->upg_info.c_info.vdac_out[i] = _board_vdac_config[i];

	stb_info->upg_info.c_info.ota_frequency = OTA_FREQUENCY;
	stb_info->upg_info.c_info.ota_symbol = OTA_SYM_RATE;
	stb_info->upg_info.c_info.ota_qam = 6;
	stb_info->upg_info.c_info.ota_pid = 0x1ca0;

	stb_info->upg_info.c_info.osd_lang = CHINESE_ENV;

	stb_info->upg_info.c_info.ota_qam = 6;
	stb_info->upg_info.c_info.upg_trigger_type = STB_INFO_UPG_TRIGGER_NONE;
	stb_info->upg_info.c_info.upg_forbidden = 0;
	stb_info->upg_info.c_info.upg_control_flag = 0; // for OUI/HW/SW enable
	stb_info->upg_info.c_info.upg_needbackup = 0; // last upgraded the upgloader1
	stb_info->upg_info.c_info.upg_retry_count = 0;
	stb_info->upg_info.c_info.sw_version_ctrl = STB_INFO_SW_VER_LARGE;

	stb_info->param1 = BACKUP_LOADER_ID;
	stb_info->param2 = BACKUP_LOADER_ADDR;	
	stb_info->param3 = BACKUP_LOADER_LEN;

	return ret;
}

int stb_info_data_init_chunk(STB_INFO_DATA *stb_info)
{
	int ret = 0, _ret = 0;
	UINT8 i = 0, j = 0;
	char str[100];
	UINT8 stb_info_buf[32];
	UINT32 chunk_num = 0;
	CHUNK_HEADER header;
	STB_CHUNK_INFO *new_chunk = NULL;
	
	UINT8 chunk_number;

	CHUNK_LIST pchunk_list[STB_PART_MAX_CHUNK_NUM];

	MEMSET(str, 0, 100);
	MEMSET(stb_info_buf, 0, 32);

	stb_pan_display("LStb", 4);	

	chunk_number = sto_get_chunk_list(pchunk_list);
	
	stb_info->stb_chunk_num = chunk_number;
		
	for(i=0; i<stb_info->stb_part_num; i++)
	{
	//STEP1: load mirror data
		for(j=0; j<chunk_number; j++)
		{		
			new_chunk = &stb_info->chunk[j];
			new_chunk->id = pchunk_list[j].id;

			new_chunk->addr = sto_chunk_goto(&new_chunk->id, 0xFFFFFFFF, 1);			

			_ret = sto_get_chunk_header(new_chunk->id, &header);
			if(_ret != 1)
			{
				STB_DATA_DEBUG("%s:%d: part %d chunk0x%08x get header fail!\n", __FUNCTION__, __LINE__, stb_info->part[i].id, new_chunk->id);
				break;
			}
						
			if(header.offset == 0)
				new_chunk->len = header.len + 0x10;
			else
				new_chunk->len = header.offset;
				
			STRCPY(new_chunk->ver, header.version);
			new_chunk->upg_flag = 0;

			stb_info->part[i].chunk_list[stb_info->part[i].chunk_num] = j;
			stb_info->part[i].chunk_num++;

			stb_info->part[i].size += new_chunk->len;	

			sprintf(str, "%s", header.name);

			if (stb_info->state != STB_INFO_DATA_STATE_RESET) 
			{ 
				if(0 == strcmp(str, "stbinfo"))// system init, sw version is from stb info
				{
					sto_get_data(g_sto_dev, stb_info_buf, new_chunk->addr+CHUNK_HEADER_SIZE, 32);
					stb_info->sw_model = (stb_info_buf[13]<<8)+stb_info_buf[12];
					stb_info->sw_version = (stb_info_buf[17]<<8)+stb_info_buf[16];
				}

				if(0 == strcmp(str, "maincode"))
				{
					MEMCPY(stb_info->download_date, header.time, 16);
				}
			}
			
		}
	}	

	if(chunk_number==0 || stb_info->part[0].chunk_num<chunk_number)
	{
		stb_info->state = STB_INFO_DATA_STATE_LIMIT; 
	}

	return ret;
}

int stb_data_factory_init(void)
{
	int ret = 0, _ret = -1;
	MEMSET(&system_stb_data, 0, sizeof(STB_DATA));

	_ret = stb_info_data_init(&system_stb_data.stb_info);
	if(_ret != 0)
	{
		STB_DATA_DEBUG("%s:%d, ERROR stb_info_data_init!\n", __FUNCTION__, __LINE__);
		ret = -1;
	}

	_ret = stb_info_data_init_chunk(&system_stb_data.stb_info);
	if(_ret != 0)
	{
		STB_DATA_DEBUG("%s:%d, ERROR stb_info_data_init_chunk!\n", __FUNCTION__, __LINE__);
		ret = -1;
	}

	// TODO: other stb_data info init

	system_stb_data.stb_info.crc_len = sizeof(STB_INFO_DATA) - 8;
	system_stb_data.stb_info.crc = MG_Table_Driven_CRC(0xFFFFFFFF, &system_stb_data.stb_info.upg_info, system_stb_data.stb_info.crc_len);

	save_stbinfo_data((UINT8 *)&system_stb_data, sizeof(STB_DATA));
	
	_ret = stb_info_data_load(STB_CHUNK_STBINFO_ID);
	if(_ret != 0)
	{
		STB_DATA_DEBUG("%s:%d, ERROR stb_info_data_load!\n", __FUNCTION__, __LINE__);
		ret = -1;
}

RETURN:
	return ret;
}

int stb_data_factory_complete(void)
{
	int ret = 0, _ret = -1;
	STB_INFO_DATA *_stb_data = stb_info_data_get();

	MEMCPY(&system_stb_data.stb_info, _stb_data, sizeof(STB_INFO_DATA));
	_ret = stb_info_data_init_chunk(&system_stb_data.stb_info);
	if(_ret != 0)
	{
		STB_DATA_DEBUG("%s:%d, ERROR stb_info_data_init_chunk!\n", __FUNCTION__, __LINE__);
		ret = -1;
	}

	system_stb_data.stb_info.crc_len = sizeof(STB_INFO_DATA) - 8;
	system_stb_data.stb_info.crc = MG_Table_Driven_CRC(0xFFFFFFFF, &system_stb_data.stb_info.upg_info, system_stb_data.stb_info.crc_len);

	save_stbinfo_data((UINT8 *)&system_stb_data, sizeof(STB_DATA));
	
	_ret = stb_info_data_load(STB_CHUNK_STBINFO_ID);
	if(_ret != 0)
	{
		STB_DATA_DEBUG("%s:%d, ERROR stb_info_data_load!\n", __FUNCTION__, __LINE__);
		ret = -1;
	}
	
RETURN:
	return ret;
}
int stb_data_load(void)
{
	int ret = -1;
	UINT32 chid;
	UINT32 offset;
	
	MEMSET(&system_stb_data, 0, sizeof(STB_DATA));

	// init struct
	chid = STB_CHUNK_STBINFO_ID;
	offset = sto_chunk_goto(&chid, 0xFFFFFFFF, 1);

	if (offset <= 0x10000000)
	{
	   	init_stb_info(offset+0x80, 2*64*1024);

		MEMSET(&system_stb_data, 0, sizeof(STB_DATA));
		ret = load_stbinfo((UINT8*)&system_stb_data, sizeof(STB_DATA));
		if(ret != SUCCESS)
		{
			STB_DATA_DEBUG("%s:%d, ERROR load_stbinfo!\n", __FUNCTION__, __LINE__);
			ret = -1;
			goto RETURN;
		}
		ret = stb_info_data_load(STB_CHUNK_STBINFO_ID);
		if(ret != 0)
		{
			STB_DATA_DEBUG("%s:%d, ERROR stb_info_data_load!\n", __FUNCTION__, __LINE__);
			ret = -1;
			goto RETURN;
		}
	}
	ret = 0;

RETURN:
	return ret;
}

