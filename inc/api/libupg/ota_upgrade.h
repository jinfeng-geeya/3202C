#ifndef __LIB_UPG_OTA_C_H__
#define __LIB_UPG_OTA_C_H__
#include <types.h>
#include <sys_config.h>
#include <api/libtsi/si_types.h>

struct ota_cmd
{
	UINT16 cmd_type;
	UINT16 para16;
	UINT32 para32;
};

#define OTA_CHECK_SERVICE			0x0001
#define OTA_GET_DOWNLOADINFO		0x0002
#define OTA_START_DOWNLOAD		0x0003
#define OTA_START_BURNFLASH		0x0004
#define OTA_START_SERVICE			0x0005
#define OTA_STOP_SERVICE			0x0006

#define	PROC_SUCCESS	0x00000001
#define	PROC_FAILURE	0x00000002
#define	PROC_STOPPED	0x00000004

#define	OTA_SUCCESS		SI_SUCCESS
#define	OTA_FAILURE		(!SI_SUCCESS)
#define	OTA_CONTINUE	SI_SUCCESS
#define	OTA_BREAK		(!SI_SUCCESS)

#define SECTOR_SIZE		(64*1024)

#define	NIT_PID		0x0010
#define	PAT_PID	0x0000
#define	BAT_PID	0x0011

//OUT/HW/SW model&version control code
enum ota_ctrl_code
{
	OTA_CTRL_ALL = 0,
	//OTA_PARAM_ANY_MASK: means no need compare OUI/HW/SW
	OTA_PARAM_ANY = 0x80,
	//OTA_SW_VER_EQ_CHUFA: means no need compare OUI/HW/SW, but sw version shall eq sw version of triger stream
	OTA_SW_VER_EQ_CHUFA	= 0x40
};

enum si_table_attr {
	DVB_SI_ATTR_SPECIFIC_ID		= SI_ATTR_SPECIFIC_ID,		/* specific_id is used */
};
struct table_info {
	UINT16 pid;				/* table pid*/
	UINT8 table_id;			/* table id */
	UINT8 crc_retry;			/* crc error retry */
	UINT8 attr;				/* when attr&DVB_SI_ATTR_SPECIFIC_ID,the specific_id will be included in the mask/value  */
	UINT32 timeout[2];			/* timeout[0] for section retrieve timeout, timeout[1] for total table retrieve timeout */
						/* timeout[1] can be OSAL_WAIT_FOREVER_TIME, but time[0] should not be this long */
	UINT32 specific_id;
};

typedef INT32 (*section_parser_t)(UINT8 *buffer, INT32 buffer_length, void *param);

struct nit_bat_ts_proc{
	UINT16 ts_id;
	INT32 (*proc)(UINT8 , void*);
};

#define	CHECK_STEP1  0x00000000
#define	CHECK_STEP2  0x00000001
#define	CHECK_STEP3  0x00000002

struct linkage_descriptor
{
	//UINT8 desc_tag;
	//UINT8 desc_len;
	UINT8 transport_stream_id[2];
	UINT8 original_network_id[2];
	UINT8 service_id[2];
	UINT8 link_type;
	UINT8 private_data[0];
};

struct service_info
{
	UINT16 transport_stream_id;
	UINT16 original_network_id;
	UINT16 service_id;	
	UINT16 pmt_pid;
	UINT8 component_tag;
	UINT16 ota_pid;
	UINT8* private_data;
	UINT8 private_len;
};
struct section_param 
{
	UINT8 section_type;
	UINT8 param8;
	UINT16 param16;
	UINT32 param32;
	void *private;	
};

struct dl_info
{
	UINT16 hw_version;
	UINT16 sw_version;
	UINT8  sw_type;
	UINT32 sw_size;
	UINT32 data;
	UINT32 time;		
#ifdef STAR_OTA_GROUP_INFO_BYTE
	UINT32 OUI;				// Organization Unique Identifier
	UINT32 stbid_start;		// Start of ID of STB need OTA upgrade
	UINT32 stbid_end;		// End of ID of STB need OTA upgrade
	UINT32 global_sw_ver;	// Global software version
	UINT32 global_hw_ver;	// Global hardware version
#endif
};

struct ota_config_t
{
	UINT32 oui;
	UINT32 hw_model;
	UINT32 hw_version;
	UINT32 sw_model;
	UINT32 sw_version;
	UINT32 sw_version_chufa;	//sw version of triger stream
	UINT8  ctrl_flag;	//see: enum ota_ctrl_mask
};

typedef INT32 (*t_parse_cb)();
typedef INT32 (*t_loop_parse_cb)(UINT32 param1);

BOOL ota_start_service(UINT16 pid,struct dl_info *info);
void ota_config_ext(struct ota_config_t *cfg);

#endif /* __LIB_UPG_OTA_C_H__ */

