#ifndef __LIB_OTA_H__
#define __LIB_OTA_H__
#include <types.h>
#include <sys_config.h>
#include <api/libtsi/si_types.h>

//#include <api/libtsi/si_table.h>

#ifdef OTA_CONTROL_FLOW_ENHANCE
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
#define OTA_STOP_SERVICE			0x0005

#define	PROC_SUCCESS	0x00000001
#define	PROC_FAILURE	0x00000002
#define	PROC_STOPPED	0x00000004
#endif

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
struct private_hdtv
{
	UINT8 download_type; //0x01: private ts ; 0x02:DC ts.
	UINT8 component_tag;	
	UINT8 hardware_version[4];
	UINT8 software_type[2];//0x0001: system sw; 0x0002: ;0x0003-0x00f: ; 0x0100-0xffff: .
	UINT8 software_version[4];
	UINT8 serial_num_start[16];
	UINT8 serial_num_end[16];
	UINT8 control_code; //0x00;0x01;0x02;0x03
	UINT8 private_data[4];
};

struct PartitionInfo
{
	UINT8* partition_buf_pointer;
	UINT32 partition_buf_size;
	UINT16 block_size;
	UINT8 block_num;
	UINT8 partition_download_finish;
};

struct private_table_info {
	UINT16 pid;				/* table pid*/
	UINT8 mask_len;
	UINT8* mask;
	UINT8* mask_value;
};

struct dl_info
{
	UINT16 hw_version;
	UINT16 sw_version;
	UINT8  sw_type;
	UINT32 sw_size;
	UINT32 data;
	UINT32 time;		
};
typedef void (*t_progress_disp)(UINT32);
typedef void (*t_burn_progress_disp)(UINT32);
#ifndef OTA_STATIC_SWAP_MEM
void ota_mem_config(UINT32 compressed,UINT32 compressed_len,UINT32 uncompressed,UINT32 uncompressed_len);
#else
void ota_mem_config(UINT32 compressed,UINT32 compressed_len,UINT32 uncompressed,UINT32 uncompressed_len,UINT32 swap_addr,UINT32 swap_len);
#endif
BOOL ota_check_service(UINT32 lnb_freq,UINT16* pid);
BOOL ota_get_download_info(UINT16 pid,struct dl_info *info);
BOOL ota_start_download(UINT16 pid,t_progress_disp progress_disp);
BOOL ota_start_burnflash(t_progress_disp progress_disp);
BOOL ota_reboot(void);

#ifdef AUTO_OTA
INT32 parse_dsmcc_sec(UINT8 *buffer, INT32 buffer_length, void *param);
#endif

#endif /* __LIB_OTA_H__ */
