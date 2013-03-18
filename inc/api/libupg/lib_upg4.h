#ifndef _LIB_UPG4_H_
#define _LIB_UPG4_H_

#include <types.h>
#include <sys_config.h>
#include <api/libtsi/si_types.h>
#include <api/libloader/ld_boot_info.h>

#define UPG_ALLCODE_NORMAL_MODE		0 // force to upg all code, 		except  bootloader, private data(hdcp, cadata, userdb¡­),	 	include stbinfo
#define UPG_CHUNK_NORMAL_MODE		1 // upg each chunk by ver/flag, 	except  bootloader, private data(hdcp, cadata, userdb¡­), 	except  stbinfo
#define UPG_ALLCODE_UNLIMITED_MODE	2 // force to upg all code, 		include bootloader, private data(hdcp, cadata, userdb¡­),		include stbinfo
#define UPG_CHUNK_UNLIMITED_MODE	3 // upg each chunk by ver/flag, 	include bootloader, private data(hdcp, cadata, userdb¡­), 	except  stbinfo
#define UPG_PRIMITIVE_CHUNK_MODE	4 // upg chunk by primitive abs
#define UPG_PRIMITIVE_DATA_MODE	5 // upg data by addr&len

// N-- not upgrade, Y-- upgrade
//all-mode (mode0 & mode2) need at least part0 (nor flash) be upgraded!

//mode			bootloader(as private data)		stbinfo		private data			normal-chunk
//0(all-nrml)		N							Y			N					Y
//1(chk-nrml)		N							N			N					Y(by Ver/upg_flag)
//2(all-ultd)		Y							Y			Y					Y
//3(chk-ultd)		Y(by Ver/upg_flag)			N			Y(by Ver/upg_flag)	Y(by Ver/upg_flag)


#define UPG_BACKUP_MIRROR_FLAG		0xF1
#define UPG_BURN_PART_FLAG			0xF2


typedef enum
{
    UPG_PROCESS_NUM,
    UPG_PART_INFO,
    UPG_STATUS_INFO,
    UPG_FLOW_NOTIFY,
}UPG_MSG_TYPE;

typedef enum
{
    FORCE_UPG,
    CHUNK_UPG,
}UPG_TYPE;

typedef enum
{
	STB_UPG_SUCCESS = 0,
	STB_UPG_NO_BURN,
	STB_UPG_FAIL,
	STB_UPG_IGNORE = 0xFF,
}UPG_RESULT;

#define	STB_UPG_STATE_TRIGGER 0xFFFF
#define	STB_UPG_STATE_LOCKNIM 0xFFFA
#define	STB_UPG_STATE_DOWNLOADINFO 0xF5FA
#define	STB_UPG_STATE_DOWNLOADDATA 0xF5AA
#define	STB_UPG_STATE_CHECKDATA 0x55AA
#define	STB_UPG_STATE_PREBURN 0x5588
//#define	STB_UPG_STATE_UPGLOADERUPGRADED 0x1188
#define	STB_UPG_STATE_NORMAL 0
#define STB_UPG_STATE_UPGLOADER_UPGRADED 0x1
#define STB_UPG_STATE_UPGLOADER_RESET 0x2

typedef INT32 (*t_progress_disp)(UINT32 param1,UINT32 param2);
typedef INT32 (*t_version_cb)(UINT32 param1,UINT32 param2);
typedef INT32 (*t_keydata_cb)(UINT32 param1,UINT32 param2, UINT32 param3,UINT32 param4);
typedef INT32 (*t_trigger_cb)(UINT32 param1,UINT32 param2);

union BOOT_INFO_EXT
{
	struct
	{
		UINT8 type;	// 0:s_info; 1:c_info; 2:t_info
	}common;
	struct
	{
		// 0B
		UINT8	type;
		// 1B
		UINT8 	standby_flag	:1;/*it will saved in temp_info head position,then bootloader will judge the flag if need sleep*/
		UINT8	reserved3b		:3;
		UINT8	ota_flag		:2;
		UINT8	ota_count		:2;
		// 2B
		UINT8	boot_tv_mode 		: 4;
		UINT8	boot_tv_ratio 		: 4;
		// 3B
		UINT8	boot_display_mode 	: 4;
		UINT8	boot_scart_out 		: 4;
		// 4B
		UINT32	ota_frequency;//xxx00
		// 8B-11B
		UINT32	ota_symbol;
		// 12B-13B
		UINT16	ota_pid;
		// 14B
		UINT8	ota_qam;
		// 15B - 20B
		UINT8	vdac_out[VDAC_NUM_MAX];
		// 21B
		UINT8 main_qam;
		// 24B - 27B
		UINT32 main_frequency;
		// 28B - 31B
		UINT32 main_symbol;
		// 32B
		UINT8	osd_lang;
		// 33B
		UINT8	rf_mode;
		// 34B
		UINT8	rf_channel;
		// 35B
		UINT8	boot_counter;
		// 36B-39B
		UINT32 upg_state_flag;
		// 40B
		UINT8 both_upgloader_bad;
		// 41B
		UINT8 upg_control_flag; // for OUI/HW/SW enable
		// 42B
		UINT8 upg_upgraded; // last upgraded the upgloader1
		// 43B
		UINT8 upg_retry_count;
		// 44B-45B
		UINT16 sw_version_chufa;	//sw version in triger stream
		// 46B-55B
		UINT8	reserved8[10];
		// 56B-59B
		UINT32 crc_len;
		// 60B-63B
		UINT32 crc;
	}c_info;
};

typedef struct
{
	UINT8 *compressed_addr; // for primary zip data
	UINT32 compressed_len;
	UINT8 *uncompressed_addr; // for unzip data
	UINT32 uncompressed_len;
	UINT8 *data_mirror_addr; // for old flash data, maybe need merge to final image
	UINT32 data_mirror_len;
	UINT8 *data_image_addr; // for burn flash
	UINT32 data_image_len;
	UINT8 *swap_addr; // for unzip internel use
	UINT32 swap_len;
	t_progress_disp upg_disp_cb;
	t_version_cb upg_version_cb;
	t_keydata_cb upg_keydata_cb;
	t_trigger_cb upg_trigger_cb;
}UPG_CONFIG;


void upg_mem_config(UPG_CONFIG mem_cfg);

#endif /* _LIB_UPG4_H_ */

