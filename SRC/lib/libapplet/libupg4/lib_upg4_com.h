#ifndef __LIB_UPG_COM_H__
#define __LIB_UPG_COM_H__
#include <types.h>
#include <sys_config.h>
#include <api/libtsi/si_types.h>
#include <api/libupg/lib_upg4.h>

#include <api/libfs2/types.h>
#include <api/libfs2/errno.h>
#include <api/libfs2/dirent.h>
#include <api/libfs2/stat.h>
#include <api/libfs2/statvfs.h>
#include <api/libfs2/fcntl.h>
#include <api/libfs2/unistd.h>
#include <api/libfs2/stdio.h>

#include <api/libchunk/chunk.h>
#include <api/libstbinfo/stb_info.h>
#include <api/libstbinfo/stb_info_data.h>
#include <api/libsi/si_tdt.h>

#ifdef NAND_FLASH_SUPPORT
#include <bus/nand/ali_nand_dev.h>
#endif

#ifndef UPG4_DEBUG
#define UPG4_DEBUG PRINTF
#endif

//g_upg_state
enum
{
	UPG4_STATE_SUCCESS = 0,
	UPG4_STATE_FAIL,
};

//g_upg_err_part ---- 0~9 A b  for PART, C for scripter flow err, d for control err, E for ...
enum
{
	UPG4_DOMAIN_PART = 0, // total 11 part at max
	UPG4_DOMAIN_SCRIPTER = 0xC,
};

// for UPG4_DOMAIN_PART
enum
{
	UPG4_ERR_PART_OK = 0,
	UPG4_ERR_PART_FILE_ERR,
	UPG4_ERR_PART_SIZE_OVERFLOW,
	UPG4_ERR_PART_UPGBUFFER_OVERFLOW,
	UPG4_ERR_PART_CHECK_IMAGE_FAIL,
	UPG4_ERR_PART_REORG_IMAGE_FAIL, // 5
	UPG4_ERR_PART_NOR_RESTORE_FAIL, 
	UPG4_ERR_PART_WRITE_FAIL,
	UPG4_ERR_PART_LOAD_FAIL,
	UPG4_ERR_PART_GET_PART_FAIL,
	UPG4_ERR_PART_GET_CHUNK_FAIL, // A
	UPG4_ERR_PART_KEYDATA_FAIL,
};

// for UPG4_DOMAIN_SCRIPTER 
enum
{
	UPG4_ERR_SCRIPTER_OK = 0,
	UPG4_ERR_SCRIPTER_NO_FILE,
	UPG4_ERR_SCRIPTER_FILE_ERR,
	UPG4_ERR_SCRIPTER_NO_PART,
	UPG4_ERR_SCRIPTER_PART_NUM_OVERFLOW,
	UPG4_ERR_SCRIPTER_PART_ID_OVERFLOW, // 5
	UPG4_ERR_SCRIPTER_PART_ID_REPEAT,
	UPG4_ERR_SCRIPTER_VERSION_FAIL,
	UPG4_ERR_SCRIPTER_NO_IMAGE,
	UPG4_ERR_SCRIPTER_GET_CHUNK_FAIL,
	UPG4_ERR_SCRIPTER_CHUNK_ADDR_WRONG, // A
	UPG4_ERR_SCRIPTER_UPDATE_CHUNK_FAIL,
	UPG4_ERR_SCRIPTER_CHUNKID_FAIL,
	UPG4_ERR_SCRIPTER_IMAGE_CRC_FAIL,
};


extern STB_INFO_DATA g_upg_part_info;
extern UPG_CONFIG g_upg_cfg;
extern STB_CHUNK_INFO g_upg_chunk_image[STB_MAX_CHUNK_NUM];
extern struct pan_device *g_upg_pan_dev;

#ifdef NAND_FLASH_SUPPORT
extern struct ali_nand_device *g_upg_nflash_dev;
#endif

extern UINT32 g_bootloader_id;
extern UINT32 g_stbinfo_id;
extern UINT8 g_upg_burn_order[STB_MAX_PART_NUM];
extern UINT8 g_upg_burn_array[STB_MAX_PART_NUM]; 


#endif /* __LIB_UPG_COM_H__ */
