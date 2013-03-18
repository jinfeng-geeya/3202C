
#ifndef __LIB_DBC_H__
#define __LIB_DBC_H__

#include <basic_types.h>
#include <mediatypes.h>

#if 0
struct dsi_filter_param 
{
	UINT16    new_sw_version;	
	UINT32 	data_code;
	UINT32 	time_code;
	struct DCGroupInfo* dc_group;	
};
struct dsi_readback_param 
{
	UINT32 	oui;
	UINT16    hw_version;
	UINT16    sw_version;	
};
struct dii_filter_param 
{
	UINT32 	group_id;
	struct DCModuleInfo* dc_module;
	UINT8 dc_module_num;
	
};
struct dii_readback_param 
{
	UINT16    dc_blocksize;
};
#endif

//32*32 blocks in a module = 4Mbytes in a module
#define BIT_MAP_NUM 32

struct DCModuleInfo
{
	UINT16 module_id;
	UINT8 module_linked;
	UINT8 module_first;
	UINT8 module_last;
	UINT8* module_buf_pointer;
	UINT32 module_buf_size;
	UINT16 block_num;
	UINT16 next_module_id;
	UINT16 next_block_num;
	UINT32 block_received[BIT_MAP_NUM];
	UINT8 module_download_finish;
};

struct DCGroupInfo
{
	UINT32 group_id;
	UINT32 group_size;
#ifdef STAR_OTA_GROUP_INFO_BYTE
	UINT32 OUI;				// Organization Unique Identifier
	UINT32 stbid_start;		// Start of ID of STB need OTA upgrade
	UINT32 stbid_end;		// End of ID of STB need OTA upgrade
	UINT32 global_sw_ver;	// Global software version
	UINT32 global_hw_ver;	// Global hardware version
#endif
/*	
	UINT32 hw_oui;
	UINT16 hw_model;
	UINT16 hw_version;
	UINT32 sw_oui;
	UINT16 sw_model;
	UINT16 sw_version;
*/	
};

BOOL dsi_message(UINT8* data,UINT16 len,UINT32 OUI,UINT16 hw_model,UINT16 hw_version,UINT16 sw_model,UINT16 sw_version,UINT16* new_sw_version,struct DCGroupInfo* group);
BOOL dii_message(UINT8* data,UINT16 len,UINT32 group_id,UINT8* data_addr,struct DCModuleInfo* module,UINT8* module_num,UINT16* blocksize);
BOOL ddb_data(UINT8* data,UINT16 len,struct DCModuleInfo* module,UINT16 blocksize,UINT8 blk_id);



#endif /*__LIB_DBC_H__*/

