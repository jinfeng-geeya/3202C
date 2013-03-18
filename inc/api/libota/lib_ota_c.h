#ifndef __LIB_OTA_H__
#define __LIB_OTA_H__
#include <types.h>
#include <sys_config.h>
#include <api/libtsi/si_types.h>

//#include <api/libtsi/si_table.h>

struct ota_cmd
{
	UINT16 cmd_type;
	UINT16 para16;
	UINT32 para32;
};

#define OTA_START_SERVICE			0x0001
#define OTA_STOP_SERVICE			0x0002

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
	UINT32 stbid_start;		// Start ID of STBs need OTA upgrade
	UINT32 stbid_end;		// End ID of STBs need OTA upgrade
	UINT32 global_sw_ver;	// Global software version
	UINT32 global_hw_ver;	// Global hardware version
#endif	
};

typedef enum
{
    PROCESS_NUM,
    VERSION_INFO,
}UPG_MSG_TYPE;

typedef enum
{
    BOOTLOADER_MAINCODE, //bootloader and maincode
    MAINCODE,            //main code
}UPG_TYPE;

typedef INT32 (*t_progress_disp)(UINT32 param1,UINT32 param2);
typedef INT32 (*t_parse_cb)();
void ota_mem_config(UINT32 compressed,UINT32 compressed_len,UINT32 uncompressed,UINT32 uncompressed_len,UINT32 swap_addr,UINT32 swap_len);
BOOL ota_start_service(UINT16 pid,struct dl_info *info);
void ota_reg_disp_callback(t_progress_disp upg_progress_disp);
void ota_reg_parse_callback(t_parse_cb ota_parse_cb);

#ifdef USB_UPG_DVBC
//usb upgrade
#define FILE_NAME_SIZE 1024
#define USBUPG_MAX_FILENUM_SUPPORT 20
#define USBUPG_ROOT_DIR_LENGTH 2
#define USB_UPGRADE_SUPPORT_LONG_FILENAME

#ifndef USB_UPGRADE_SUPPORT_LONG_FILENAME
#define USBUPG_FILE_NAME_LENGTH (USBUPG_ROOT_DIR_LENGTH + 8 + 1 + 3 + 1)
#else
#define USBUPG_FILE_NAME_LENGTH FILE_NAME_SIZE
#endif
typedef struct
{
	char upg_file_name[USBUPG_FILE_NAME_LENGTH];
    long long size;
}USBUPG_FILENODE,*PUSBUPG_FILENODE;
//usb upgrade end.
#endif

#endif /* __LIB_OTA_H__ */
