#ifndef __LIB_STB_INFO_DATA_H__
#define __LIB_STB_INFO_DATA_H__

#include <types.h>
#include <retcode.h>
#include <sys_config.h>
#include <api/libloader/ld_boot_info.h>

#ifndef STB_MAX_PART_NUM
#define STB_MAX_PART_NUM 11
#endif

#ifndef STB_PART_MAX_CHUNK_NUM
#define STB_PART_MAX_CHUNK_NUM 32
#endif

#ifndef STB_MAX_CHUNK_NUM
#define STB_MAX_CHUNK_NUM 64
#endif

#define STB_CHUNK_ID_MASK 0xFFFF0000
#define STB_CHUNK_PART_MASK 0xFF

// here define all chunk used in STB, please make sure be unify
#ifndef STB_CHUNK_BOOTLOADER_ID
#define STB_CHUNK_BOOTLOADER_ID 	0x23010010
#endif
#define STB_CHUNK_STBINFO_ID 		0x20DF0100
#define STB_CHUNK_HDCPKEY_ID 		0x09F60100
#define STB_CHUNK_LOGO_ID			0x02FD0100
#define STB_CHUNK_CAS_ID			0x08F70100
#define STB_CHUNK_IPANEL_DATA_ID	0x0BF40200
#define STB_CHUNK_IPANEL_DATA2_ID	0x0BF40300
#define STB_CHUNK_DEFAULT_DB_ID		0x03FC0100
#define STB_CHUNK_USER_DB_ID		0x04FB0100

#define STB_CHUNK_MAINCODE_ID		0x01FE0100
#define STB_CHUNK_SEECODE_ID		0x0AF50100

#define STB_CHUNK_OTA_ID 			0x00FF0100

#ifndef GEEYA_AD
#define STB_CHUNK_ADV_LOGO_ID		0x0AF50500//adv_logo
#define STB_CHUNK_ADV_DATA_ID		0x0AF50600//adv
#define STB_CHUNK_ADV_LOGO_LEN		64*1024 //adv_logo
#define STB_CHUNK_ADV_DATA_LEN		512*1024//adv_logo
#endif
#define STB_CHUNK_FONT_ID			0x10EF0100
#define STB_CHUNK_BMP_ID			0x11EE0100

#define STB_CHUNK_IPANEL_SKIN_ID	0x0BF40100

#define STB_CHUNK_STUFF_ID			0xEE110100


#define STB_PART_CORE "STBCore"
#define STB_PART_MAINCODE "MainCode"
#define STB_PART_OTALDER "OTALoader"
#define STB_PART_OTALDER2 "OTALoader2"
#define STB_PART_RSC "RSC"
#define STB_PART_RESV "Resv"
#define STB_PART_MW "MW"
#define STB_PART_BACKUP "Backup"

#define	STB_INFO_DATA_STATE_NORMAL 	0
#define	STB_INFO_DATA_STATE_PREBURN (1<<0)  //pre-burn during upgrade, need revert stb_info
#define	STB_INFO_DATA_STATE_LIMIT	(1<<1)  //need ALL-unlimited-mode upgrade to reset to right value
#define	STB_INFO_DATA_STATE_RESET	(1<<2)  //need maincode/upgloader to complete stb_info_data


#define STB_INFO_UPG_TRIGGER_NONE		(0<<0)	
#define STB_INFO_UPG_TRIGGER_BOOTFAIL	(1<<0)	// trigger by boot fail(crc fail) 
#define STB_INFO_UPG_TRIGGER_INTERRUPT	(1<<1)	// last upg be interrupted, need goon
#define STB_INFO_UPG_TRIGGER_BOOTKEY	(1<<2)	// trigger by panel key
#define STB_INFO_UPG_TRIGGER_APPOTA		(1<<3)	// app trigger ota

#define STB_INFO_SW_VER_LARGE		(0<<0)
#define STB_INFO_SW_VER_SAME		(1<<0)
#define STB_INFO_SW_VER_DIFFER		(1<<1)

typedef struct
{
	UINT32 id; // chunk id
	char ver[17]; //data version
	UINT8 upg_flag; //0-ignore, 1-by version, 2-upgrade anyway
	UINT8 resv[2];
	UINT32 addr; //data address offset within mother part
	UINT32 len; //data len, maybe have stuff payload
}STB_CHUNK_INFO; // 32Byte

typedef struct
{
	UINT8 id; //means locate, 0-Nor, 1-Nand partition 1, ...
	UINT8 chunk_num; //how many chunk within mother part
	UINT8 resv[2];
	union
	{
		char name[16]; //Part Unify identify name
		struct
		{
			char *filename;
			UINT8 upg_mode; // 4type
			UINT8 upg_flag;
			UINT8 resv[10];
		}upgrade;
	};
	UINT32 size; //part total size
	UINT8 chunk_list[STB_PART_MAX_CHUNK_NUM];
}STB_PART; //56B

union UPG_INFO
{
	struct
	{
		UINT8 type;	// 0:s_info; 1:c_info; 2:t_info
	}common;
	struct
	{
		UINT8 type;
		UINT8 upg_control_flag; // for OUI/HW/SW enable
		UINT8 upg_trigger_type;
		UINT8 upg_needbackup:2; // last upgraded the upgloader1
		UINT8 upg_forbidden:2;	
		UINT8 upg_retry_count:4;
		UINT32 sw_version_trigger;	//sw version in triger stream
		
		UINT8 boot_tv_mode:4;
		UINT8 boot_tv_ratio:4;
		UINT8 boot_display_mode:4;
		UINT8 boot_scart_out:4;
		UINT8 vdac_out[VDAC_NUM_MAX];	
		
		UINT32 ota_frequency;//xxx00
		UINT32 ota_symbol;	
		UINT16 ota_pid;
		UINT8 ota_qam;
		UINT8 osd_lang;
		
		UINT8 sw_version_ctrl;
		UINT8 reserved8[35];
	}c_info;
}; // 64B

typedef struct
{	
	UINT32 crc_len;	
	UINT32 crc;	
	union UPG_INFO upg_info; //64B
	UINT32 sw_model;	
	UINT32 sw_version;
	UINT8 download_date[20];
	UINT8 firmware_date[20];
	UINT32 state; //refer to STB_INFO_DATA_STATE
	UINT8 bacup_part_id; //may used for restore
	UINT8 upg_mode:4;
	UINT8 nor_enable:2;
	UINT8 nand_enable:2;
	UINT8 stb_part_num; // part 0 is Nor, part 1 is Nand partition 1, ...
	UINT8 stb_chunk_num; // part 0 is Nor, part 1 is Nand partition 1, ...
	UINT32 nor_flash_size;
	UINT32 nand_flash_size; //136B
	STB_PART part[STB_MAX_PART_NUM]; //STB_MAX_PART_NUM*56
	STB_CHUNK_INFO chunk[STB_MAX_CHUNK_NUM]; //32*STB_MAX_CHUNK_NUM
	UINT32 param1;	// backup loader id, for nor-flash only case
	UINT32 param2;	// backup loader addr, for nor-flash only case
	UINT32 param3;	// backup loader len, for nor-flash only case

	UINT8 resv[52];
}STB_INFO_DATA;

void stb_info_data_print_part(STB_PART *part, STB_CHUNK_INFO *chunk_list);
void stb_info_data_print(STB_INFO_DATA *stb_data);
int stb_info_data_get_info_by_name(char *part_name, STB_PART *part);
int stb_info_data_get_info_by_id(UINT32 id, STB_PART *part);
int stb_info_data_get_sw_version(void);
int stb_info_data_get_total_size(void);
void stb_info_data_set_backup_partid(UINT32 id);
void stb_info_data_set_state(UINT8 state);
int stb_info_data_add_chunk(STB_CHUNK_INFO *base, STB_CHUNK_INFO *chunk);
int stb_info_data_delete_chunk(STB_CHUNK_INFO *base, UINT32 chunk_id);
int stb_info_data_update_chunk(STB_CHUNK_INFO *base, STB_CHUNK_INFO *chunk);
int stb_info_data_get_chunk(STB_CHUNK_INFO *base, UINT32 idx, STB_CHUNK_INFO *chunk);
BOOL stb_info_data_restore_check(void);
int stb_info_data_restore(UINT8 *image_addr, UINT32 image_len);
void stb_info_data_save(void);
STB_INFO_DATA* stb_info_data_get(void);
int stb_info_data_load(UINT32 chid);
void stb_info_data_update_part(STB_PART *part, STB_CHUNK_INFO *base);
void stb_info_data_update_info(UINT32 sw_version, char *download_date, char *firmware_date);

#endif /* __LIB_STB_INFO_DATA_H__ */

