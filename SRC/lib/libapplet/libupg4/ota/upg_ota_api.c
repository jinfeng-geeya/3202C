#include <basic_types.h>
#include <mediatypes.h>

#include <sys_config.h>
#ifndef DISABLE_OTA

#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libc/alloc.h>

#include <api/libdbc/lib_dbc.h>

#include <api/libtsi/sec_pat.h>
#include <api/libtsi/sec_pmt.h>
#include <api/libtsi/p_search.h>
#include <api/libnim/lib_nim.h>

#include <hld/hld_dev.h>
#include <hld/sto/sto.h>
#include <hld/dmx/dmx.h>
#include <hld/decv/vdec_driver.h>
#include <hld/nim/nim_dev.h>
#include <api/libchunk/chunk.h>
#include <bus/flash/flash.h>

#include <api/libtsi/si_descriptor.h>
#include <api/libupg/ota_upgrade.h>
#include "../lib_upg4_com.h"

UINT32	g_oui;
UINT32	g_hw_model;
UINT32	g_hw_version;
UINT32	g_sw_model;
UINT32	g_sw_version;
UINT32  g_sw_version_chufa; //sw version of triger stream
UINT32	g_upg_sw_version;
//ota control flag, see OTA_PARAM_ANY_MASK,OTA_SW_VER_ANY_MASK
//default value: 0
UINT8 g_ota_ctrl_flag = OTA_CTRL_ALL;

extern UINT32 g_upg_sw_size;

struct DCGroupInfo g_dc_group;
#define DC_MODULE_MAX 	8
struct DCModuleInfo g_dc_module[DC_MODULE_MAX];
UINT8 	g_dc_module_num;
UINT16	g_dc_blocksize;

#define g_ota_section_buf_len 5000 
static UINT8 g_ota_section_buf[g_ota_section_buf_len];

static struct dl_info g_dl_info;

static INT32 received_section_num;
static INT32 total_block_num;

INT32 parse_dsmcc_sec(UINT8 *buffer, INT32 buffer_length, void *param)
{
	BOOL ret;
	
	UINT8 byte_hi,byte_lo;
	UINT8 table_id,section_num,last_section_num;
	UINT16 section_len,table_id_extension;
	UINT16 new_sw_version, sw_version;

	struct section_param *sec_param = (struct section_param *)param;
	

	table_id = *buffer++;	// 0
	byte_hi = *buffer++;	// 1
	byte_lo = *buffer++;	// 2
	section_len = ((byte_hi&0x0f)<<8) + byte_lo;
	byte_hi = *buffer++;	// 3
	byte_lo = *buffer++;	// 4
	table_id_extension = (byte_hi<<8) + byte_lo;	
	byte_lo = *buffer++;	// 5
	section_num = *buffer++;	// 6
	last_section_num = *buffer++;	// 7

	switch(sec_param->section_type)
	{
		case 0x00: /*------------------STEP_DSI---------------------------*/	

			if(table_id_extension!=0x0000 && table_id_extension!=0x0001)
				return OTA_FAILURE;

			sw_version = ((g_ota_ctrl_flag&OTA_SW_VER_EQ_CHUFA)!=0)?g_sw_version_chufa:g_sw_version;
			ret = dsi_message(buffer,
							section_len-9,
							(g_ota_ctrl_flag<<24)|g_oui,	//0xffffff						
							g_hw_model,//8							
							g_hw_version,
							g_sw_model,//8							
							sw_version,
							&new_sw_version,
							&g_dc_group);
			if(ret == TRUE)
			{
#ifdef STAR_OTA_GROUP_INFO_BYTE
				g_dl_info.OUI = g_dc_group.OUI;
				g_dl_info.global_hw_ver = g_dc_group.global_hw_ver;
				g_dl_info.global_sw_ver = g_dc_group.global_sw_ver;
				g_dl_info.stbid_start = g_dc_group.stbid_start;
				g_dl_info.stbid_end = g_dc_group.stbid_end;
#endif
				g_dl_info.hw_version = 0;
				g_dl_info.sw_version = new_sw_version;
				g_dl_info.sw_type = 0;
				g_dl_info.sw_size = g_dc_group.group_size;

				return OTA_SUCCESS;
			}
			else
				return OTA_FAILURE;	
		case 0x01: /*------------------STEP_DII---------------------------*/	

			if(table_id_extension!=(g_dc_group.group_id&0xffff))
				return OTA_FAILURE;
			ret = dii_message(buffer,
							section_len-9,
							g_dc_group.group_id,
							(UINT8*)g_upg_cfg.compressed_addr,
							g_dc_module,
							&g_dc_module_num,
							&g_dc_blocksize);
			if(ret == TRUE)
				return OTA_SUCCESS;
			else
				return OTA_FAILURE;			
			
		case 0x02: /*------------------STEP_DDB---------------------------*/	

			if(table_id_extension!=g_dc_module[sec_param->param8].module_id)
				return OTA_FAILURE;
			ret = ddb_data(buffer,
						section_len-9,
						&g_dc_module[sec_param->param8],
						g_dc_blocksize,
						section_num);
			if(ret == TRUE)
				return OTA_SUCCESS;
			else
				return OTA_FAILURE;	
		default:
			return OTA_FAILURE;
	}

}

void ddb_section(struct get_section_param *sr_request)
{
	INT32 modul_idx;
	UINT8* section_buf = sr_request->buff;
	
	UINT16 table_id_extension = (section_buf[3]<<8) +section_buf[4];

	//UPG4_DEBUG("rcvd :module_id = %x, section_num = %d\n",table_id_extension,section_buf[6]);
	
	for(modul_idx=0;modul_idx<g_dc_module_num;modul_idx++)
	{
		if(g_dc_module[modul_idx].module_id == table_id_extension)
			break;
	}
	if(modul_idx==g_dc_module_num)
		return;
	if(((g_dc_module[modul_idx].block_num-1)&0xff) != (section_buf[7]))
	{
		UPG4_DEBUG("g_dc_module[i].block_num-1[%d] != last_section_num[%d]\n", ((g_dc_module[modul_idx].block_num-1)&0xff), (section_buf[7]));	
		return ;//ASSERT(0);
	}
	
	struct section_param sec_param;
	sec_param.section_type = 2;
	sec_param.param8 = modul_idx;
	if(parse_dsmcc_sec(sr_request->buff,sr_request->buff_len,(void*)(&sec_param)) != OTA_SUCCESS)
		return;
	for(modul_idx=0;modul_idx<g_dc_module_num;modul_idx++)
	{
		if(g_dc_module[modul_idx].module_download_finish != 1)
			break;
	}
	if(modul_idx==g_dc_module_num)
	{
		sr_request->continue_get_sec = 0;
	}
	received_section_num ++;
	//UPG4_DEBUG("%d%\n",received_section_num*90/total_block_num);
	
	upg4_update_state(UPG_PROCESS_NUM, received_section_num*90/total_block_num);	
	
	return;
}

INT32 si_private_sec_parsing_start(UINT8 section_type,UINT16 pid, section_parser_t section_parser, void *param)
{
	INT32 i;
	struct dmx_device * dmx_dev;
	if((dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0)) == NULL)
	{
		UPG4_DEBUG("ERROR : get dmx dev failure !");
		return OTA_FAILURE;
	}
	struct get_section_param sr_request;
	struct restrict sr_restrict;

	MEMSET(&sr_restrict, 0, sizeof(struct restrict));
	
	sr_request.buff = (UINT8 *)g_ota_section_buf;											
	sr_request.buff_len = g_ota_section_buf_len;
	sr_request.crc_flag = 1;
	sr_request.pid = pid;
	sr_request.mask_value = &sr_restrict;
	sr_request.wai_flg_dly = 20000;//20s;
	
	if(section_type == 0)
	{
		/* DSI  */
#ifdef OTA_DSI_TIME_OUT
        sr_request.wai_flg_dly = OTA_DSI_TIME_OUT;
#endif        
		sr_restrict.mask_len = 7;
		sr_restrict.value_num = 1;
		MEMSET(sr_restrict.mask,0,sr_restrict.mask_len);
		MEMSET(sr_restrict.value[0],0,sr_restrict.mask_len);
		sr_restrict.mask[0] = 0xff;
		sr_restrict.mask[1] = 0x80;		
		sr_restrict.mask[3] = 0xff;
		sr_restrict.mask[4] = 0xfe;	
		sr_restrict.mask[6] = 0xff;
		sr_restrict.value[0][0] = 0x3b;
		sr_restrict.value[0][1] = 0x80;
		sr_restrict.value[0][3] = 0x00;
		sr_restrict.value[0][4] = 0x00;	 //0000 |0001
		sr_restrict.value[0][6] = 0x00;  //section_num = 0		
		
		sr_request.get_sec_cb = NULL;
		sr_request.continue_get_sec = 0;				
	}
	else if(section_type == 1)
	{
		/* DII  */
		sr_restrict.mask_len = 7;
		sr_restrict.value_num = 1;
		MEMSET(sr_restrict.mask,0,sr_restrict.mask_len);
		MEMSET(sr_restrict.value[0],0,sr_restrict.mask_len);	
		sr_restrict.mask[0] = 0xff;
		sr_restrict.mask[1] = 0x80;			
		sr_restrict.mask[3] = 0xff;
		sr_restrict.mask[4] = 0xff;	
		sr_restrict.mask[6] = 0xff;
		sr_restrict.value[0][0] = 0x3b;
		sr_restrict.value[0][1] = 0x80;
		sr_restrict.value[0][3] = ((g_dc_group.group_id)>>8)&0xff;
		sr_restrict.value[0][4] = (g_dc_group.group_id)&0xff;
		sr_restrict.value[0][6] = 0x00;  //section_num = 0	
		
		sr_request.get_sec_cb = NULL;
		sr_request.continue_get_sec = 0;					
	}
	else if(section_type == 2)
	{
		sr_request.wai_flg_dly = 200000;//200s;max support (200/8)=25M ts data at 1Mbps
		
		/* DDB  */
		if(g_dc_module_num>8)
		{
			sr_restrict.mask_len = 2;
			sr_restrict.value_num = 1;
			MEMSET(sr_restrict.mask,0,sr_restrict.mask_len);
			MEMSET(sr_restrict.value[0],0,sr_restrict.mask_len);	
			sr_restrict.mask[0] = 0xff;
			sr_restrict.mask[1] = 0x80;			
			sr_restrict.value[0][0] = 0x3c;
			sr_restrict.value[0][1] = 0x80;				
		}
		else
		{
			sr_restrict.mask_len = 5;
			sr_restrict.value_num = g_dc_module_num;
			MEMSET(sr_restrict.mask,0,sr_restrict.mask_len);
			sr_restrict.mask[0] = 0xff;
			sr_restrict.mask[1] = 0x80;		
			sr_restrict.mask[3] = 0xff;
			sr_restrict.mask[4] = 0xff;				
			for(i=0;i<g_dc_module_num;i++)
			{
				MEMSET(sr_restrict.value[i],0,sr_restrict.mask_len);					
				sr_restrict.value[i][0] = 0x3c;
				sr_restrict.value[i][1] = 0x80;	
				sr_restrict.value[i][3] = ((g_dc_module[i].module_id)>>8)&0xff;
				sr_restrict.value[i][4] = (g_dc_module[i].module_id)&0xff;					
			}
				
		}

		sr_request.get_sec_cb = ddb_section;
		sr_request.continue_get_sec = 1;
	}

	dmx_io_control(dmx_dev, CB_CHANGE_LEN, 0);
	if(dmx_req_section(dmx_dev, &sr_request)!= SUCCESS)
	{
		UPG4_DEBUG("ERROR : get section failure !");
		return OTA_FAILURE;//ASSERT(0);//return OTA_FAILURE;
	}
	dmx_io_control(dmx_dev, CB_RESUME_LEN, 0);
	if(section_parser!=NULL)
		return (section_parser(sr_request.buff,sr_request.buff_len,param));
	return OTA_SUCCESS;

}

BOOL ota_cmd_get_download_info(UINT16 pid,struct dl_info *info)
{
	UINT32 i,j;
	INT32 ret ;
	struct section_param sec_param;	

	UPG4_DEBUG("Get download_info...\n");
	_upg_set_domain(UPG4_DOMAIN_SCRIPTER);

/* step 1 : DSI  */

	sec_param.section_type = 0x00;
	
       ret = si_private_sec_parsing_start(0,
	   							pid,
								parse_dsmcc_sec,
								(void*)(&sec_param));	
	if(ret != OTA_SUCCESS)
	{
		UPG4_DEBUG("ERROR : No  ota service exist!\n");
		return FALSE;
	}	

/* step 2 : DII  */

	sec_param.section_type = 0x01;	

   ret = si_private_sec_parsing_start(1,
							pid,
							parse_dsmcc_sec,
							(void*)(&sec_param));
	if(ret != OTA_SUCCESS)
	{
		UPG4_DEBUG("ERROR : No  ota service exist!\n");
		return FALSE;
	}	

	g_upg_sw_size =  g_dl_info.sw_size;
	g_upg_sw_version = g_dl_info.sw_version;
	
	MEMCPY(info, &g_dl_info,sizeof(struct dl_info));

	UPG4_DEBUG("Get download inof Done!\n");
	return TRUE;
	
}

BOOL ota_cmd_start_download(UINT16 pid)
{
	UINT32 i;
	INT32 ret;

	if(g_upg_sw_size>g_upg_cfg.compressed_len)
	{
		UPG4_DEBUG("ERROR: download_buf exceed!\n");
		return FALSE;			
	}
	
	/* step 1: process_download */		

	UPG4_DEBUG("Start Download...\n");

	upg4_update_state(UPG_PROCESS_NUM, 0);	
	
	received_section_num = 0;
	total_block_num = 0;
	for(i=0;i<g_dc_module_num;i++)
		total_block_num+=g_dc_module[i].block_num;
	
       ret = si_private_sec_parsing_start(2,pid,NULL,NULL);
	if(ret != OTA_SUCCESS)
	{
		UPG4_DEBUG("ERROR : No  ota service exist!\n");
		return FALSE;
	}	
		
	/* step 2: check crc */	
	ret = upg_un7z();

	upg4_update_state(UPG_PROCESS_NUM, 100);
	
	UPG4_DEBUG("Download Finished !\n");
	return ret;
}

typedef struct ota_descriptor
{
	UINT8 desc[8];
	UINT32 desc_len;
	UINT16 ota_type;
	UINT16 chunk_num;
}OTA_DESCRIPTOR;

UINT32 ota_upg_part_list_init(UINT8* m_addr, STB_INFO_DATA* upg_stb_data,OTA_DESCRIPTOR*ota_desc_head);

BOOL ota_cmd_start_burnflash()
{
	struct sto_device *flash_dev;
	CHUNK_HEADER m_chunk_header;
	OTA_DESCRIPTOR ota_desc_head;
	UINT8* m_pointer = 0, *f_pointer=0;
	UINT8* m_uncompress_addr;
	UINT32 upg_len;
	UINT8* m_chunk_addr;
	UINT32 ota_desc_head_len;
	UINT32 i,j,n;
	INT32 cmd_cb;
	UINT32 sector_no;
	UINT32 temp_start,temp_end;
	UINT32 len;
	unsigned long chid, userdb_addr;
	unsigned char * addr;
	INT32 part_idx;
	UINT32 part_id;
	STB_CHUNK_INFO upg_chunk;
	STB_PART *upg_part, __part;
	STB_INFO_DATA *stb_data;
	int ret;
	date_time cur_time;

	upg4_update_state(UPG_FLOW_NOTIFY, STB_UPG_STATE_CHECKDATA);

	m_uncompress_addr = (UINT8*)g_upg_cfg.uncompressed_addr;

	stb_data = stb_info_data_get();

	upg_len = ota_upg_part_list_init(m_uncompress_addr, &g_upg_part_info, &ota_desc_head);
	if(upg_len == 0)
	{
		UPG4_DEBUG("%s:%d: part parse fail\n", __FUNCTION__, __LINE__);
		return FALSE;
	}
	
	m_uncompress_addr += ota_desc_head.desc_len;

#if 1
	MEMCPY(&g_upg_chunk_image, stb_data->chunk, STB_MAX_CHUNK_NUM * sizeof(STB_CHUNK_INFO));
#else
	if(ota_desc_head.ota_type == UPG_ALLCODE_NORMAL_MODE
		|| ota_desc_head.ota_type == UPG_ALLCODE_UNLIMITED_MODE)
	{/* by allcode */
		/* init image chunk list */	
		MEMSET(&g_upg_chunk_image, 0, STB_MAX_CHUNK_NUM * sizeof(STB_CHUNK_INFO));
	}
	else
	{/* by chunk */		
		/* init image chunk list */	
		MEMCPY(&g_upg_chunk_image, stb_data->chunk, STB_MAX_CHUNK_NUM * sizeof(STB_CHUNK_INFO));
	}
#endif

	UPG4_DEBUG("ota upg type: %d\n",ota_desc_head.ota_type);

	stb_data->upg_mode = ota_desc_head.ota_type; // save upg mode	
	g_upg_part_info.upg_mode = ota_desc_head.ota_type;
	stb_info_data_set_state(STB_INFO_DATA_STATE_PREBURN); // protect whole all-mode upgrade flow
	stb_info_data_save();

	upg4_update_state(UPG_FLOW_NOTIFY, STB_UPG_STATE_PREBURN);

	// order the burn array
	n = 0;
	for(i=0; i<STB_MAX_PART_NUM; i++)
	{
		if(g_upg_burn_order[i] < STB_MAX_PART_NUM)
		{
			for(j=0; j<g_upg_part_info.stb_part_num; j++)
			{
				upg_part = &g_upg_part_info.part[j];
				if(g_upg_burn_order[i] == upg_part->id)
				{
					g_upg_burn_array[n++] = j;
				}
			}
		}
	}
	if(n != g_upg_part_info.stb_part_num)
	{
		UPG4_DEBUG("%s:%d: order the burn array error! n=%d, g_upg_part_info.stb_part_num=%d\n", __FUNCTION__, __LINE__, n, g_upg_part_info.stb_part_num);
		SDBBP();
	}
	
	/* parse part*/
	for(i=0; i<g_upg_part_info.stb_part_num; i++)
	{
		upg_part = &g_upg_part_info.part[g_upg_burn_array[i]];

		upg4_update_state(UPG_PART_INFO, (i+1)|(g_upg_part_info.stb_part_num<<16));
		_upg_set_domain(upg_part->id);
		
		ret = -1;
		
		if(ota_desc_head.ota_type == UPG_ALLCODE_NORMAL_MODE
			|| ota_desc_head.ota_type == UPG_ALLCODE_UNLIMITED_MODE)
		{/* upg all code */
			if(ota_desc_head.ota_type==UPG_ALLCODE_NORMAL_MODE)
			{
#ifdef NAND_FLASH_SUPPORT	
				if(upg_part->id == 0) // Nor flash mode0 all-limited not support chunk change, avoid destroy private chunk data!
				{
					ret = _upg_upgrade_part_by_chunk(upg_part, m_uncompress_addr, upg_len, FALSE);
				}
				else
#endif					
				{
					ret = _upg_upgrade_part_by_all(upg_part, m_uncompress_addr, upg_len, FALSE);
				}
			}
			else
			{
				ret = _upg_upgrade_part_by_all(upg_part, m_uncompress_addr, upg_len, TRUE);
			}
		}
		else
		{/* upg by chunk */
			if(ota_desc_head.ota_type==UPG_CHUNK_NORMAL_MODE)
			{
				ret = _upg_upgrade_part_by_chunk(upg_part, m_uncompress_addr, upg_len, FALSE);
			}
			else//(ota_desc_head.ota_type==OTAUPG_CHUNK_UNLIMITED_MODE)
			{
				ret = _upg_upgrade_part_by_chunk(upg_part, m_uncompress_addr, upg_len, TRUE);			
			}
		}

		if(ret < 0) 
		{
			UPG4_DEBUG("%s:%d: part %d [id = %d] upgrade fail\n", __FUNCTION__, __LINE__, i, upg_part->id);
			return FALSE;
		}
	}

	stb_info_data_update_info(g_upg_part_info.sw_version, g_upg_part_info.download_date, g_upg_part_info.firmware_date);
	// save special stb_info for all-mode upgrade
	if((ota_desc_head.ota_type == UPG_ALLCODE_NORMAL_MODE) || (ota_desc_head.ota_type == UPG_ALLCODE_UNLIMITED_MODE))
	{
		stb_info_data_clear_allchunk();
	}
	else
	{
		stb_info_data_set_state(STB_INFO_DATA_STATE_NORMAL);
	}	
	stb_info_data_save();

	upg4_update_state(UPG_FLOW_NOTIFY, STB_UPG_STATE_NORMAL);

	upg4_update_state(UPG_PROCESS_NUM, 100);

	UPG4_DEBUG("Write flash done!\n");
	return TRUE;
}

BOOL ota_cmd_service_stopped()
{
	struct dmx_device * dmx_dev;
	if((dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0)) == NULL)
	{
		UPG4_DEBUG("ERROR : get dmx dev failure !");
		return OTA_FAILURE;
	}		
	dmx_io_control(dmx_dev, CLEAR_STOP_GET_SECTION, (UINT32)NULL);
}

void ota_config(struct ota_config_t *cfg)
{
	g_oui = cfg->oui;
	g_hw_model = cfg->hw_model;
	g_hw_version = cfg->hw_version;
	g_sw_model = cfg->sw_model;
	g_sw_version = cfg->sw_version;
	g_sw_version_chufa = cfg->sw_version_chufa;
	g_ota_ctrl_flag = cfg->ctrl_flag;
}

UINT32 ota_upg_part_list_init(UINT8* m_addr, STB_INFO_DATA* upg_stb_data,OTA_DESCRIPTOR*ota_desc_head)
{
	STB_PART *upg_part=NULL;
	STB_CHUNK_INFO *upg_chunk;
	CHUNK_HEADER m_chunk_header;
	UINT8* m_pointer = NULL;
	UINT8* ota_desc_upg;
	UINT32 ota_desc_head_len;
	INT32 part_idx,chunk_idx;
	UINT32 part_id;
	UINT32 i;
	UINT32 addr, offset;
	date_time cur_time;
	UINT8 date_str[20];
	UINT32 chunk_crc;

	if(m_addr==NULL ||upg_stb_data==NULL || ota_desc_head==NULL)
		return 0;

	MEMSET(upg_stb_data, 0, sizeof(STB_INFO_DATA));
	MEMSET(ota_desc_head, 0, sizeof(OTA_DESCRIPTOR));
	
	MEMCPY(&ota_desc_head->desc,&m_addr[0],8);
	ota_desc_head->desc_len = fetch_long(&m_addr[8]);
	ota_desc_head->ota_type = (m_addr[12] << 8) | m_addr[13];
	ota_desc_head->chunk_num = (m_addr[14] << 8) | m_addr[15];

	ota_desc_head_len = sizeof(OTA_DESCRIPTOR);
		
	m_pointer = m_addr+ota_desc_head->desc_len;

	ota_desc_upg = m_addr + ota_desc_head_len;

	part_idx = -1;
	part_id=0xff;

	addr = 0;//ota_desc_head->desc_len;
		
	/* parse chunk*/
	for(i=0; i<ota_desc_head->chunk_num; i++)
	{
		/*get chunk header info*/
		get_chunk_header_by_addr(m_pointer,&m_chunk_header);	

		upg_chunk = &upg_stb_data->chunk[i];
		if(m_chunk_header.offset==0)
			upg_chunk->len = m_chunk_header.len+0x10;
		else
			upg_chunk->len = m_chunk_header.offset;
		
		/* check chunk crc */
		chunk_init((UINT32)m_pointer, upg_chunk->len);
		chunk_crc = chunk_check_single_crc(m_pointer);
		if(chunk_crc != 0)
		{
			UPG4_DEBUG("%s:%d: [id=0x%x] crc error 0x%x\n", __FUNCTION__, __LINE__, m_chunk_header.id,chunk_crc);
			return 0;
		}
		
		if(part_id != (m_chunk_header.id & STB_CHUNK_PART_MASK))
		{
			if(m_chunk_header.id == g_bootloader_id)
				part_id = 0;
			else
				part_id = m_chunk_header.id & STB_CHUNK_PART_MASK;
	
			part_idx++;

			upg_part = &upg_stb_data->part[part_idx];
			upg_part->id = part_id;
			upg_part->size = 0;
		}

		/*get firmware_date from stb info*/
		if(m_chunk_header.id == g_stbinfo_id)
		{
			MEMCPY(g_upg_part_info.firmware_date, m_chunk_header.time, 16);
		}

		if(0 == strcmp(m_chunk_header.name, "maincode"))
		{
			MEMCPY(g_upg_part_info.download_date, m_chunk_header.time, 16);
		}
		
		upg_chunk->id = m_chunk_header.id;
		
		MEMCPY(upg_chunk->ver, m_chunk_header.version, 16);
		upg_chunk->addr = addr;
		upg_chunk->upg_flag = ota_desc_upg[i];

		upg_part->size += upg_chunk->len;

		upg_part->chunk_list[upg_part->chunk_num] = i;
		upg_part->chunk_num++;

		if((((m_chunk_header.id&STB_CHUNK_ID_MASK) == (STB_CHUNK_STUFF_ID&STB_CHUNK_ID_MASK)) && (upg_part->id > 0))
			||(((m_chunk_header.id&STB_CHUNK_ID_MASK) == (STB_CHUNK_USER_DB_ID&STB_CHUNK_ID_MASK)) && (upg_part->id == 0)))
		{
			if(m_chunk_header.offset!=0)
			{
				store_long(m_pointer+CHUNK_LENGTH, m_chunk_header.offset-0x10);
				store_long(m_pointer+CHUNK_OFFSET, 0);
			}
		}

		addr += upg_chunk->len;
		m_pointer += upg_chunk->len;
	}
	upg_stb_data->stb_part_num = part_idx+1;
	
	upg_stb_data->sw_model = g_sw_model;
	upg_stb_data->sw_version = g_upg_sw_version;

	return addr;
}

#endif

