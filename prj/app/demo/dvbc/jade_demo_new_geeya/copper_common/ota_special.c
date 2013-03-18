#include <types.h>
#include <sys_config.h>
#include <retcode.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libc/alloc.h>
#include <api/libsi/si_tdt.h>
#include <api/libpub/lib_pub.h>
#include <api/libtsi/db_3l.h>
#include <api/libtsi/si_descriptor.h>
#include <api/libsi/si_desc_cab_delivery.h>
#include <api/libtsi/sie.h>
//#include <api/libota/lib_ota_c.h>
#include <api/libstbinfo/stb_info_data.h>
#include <hld/sto/sto.h>
#include "com_api.h"
#include "ota_special.h"
#include "api/libosd/osd_lib.h"
#include "system_data.h"
#include "../menus_root.h"

#define OTA_TRIGGER_TS_SUPPORT
#if (CAS_TYPE == CAS_DVT)
#include <api/libcas/dvt/DVTCAS_STBDataPublic.h>
#endif

#define OTA_DEBUG_LEVEL		0
#if (OTA_DEBUG_LEVEL>0)
#define OTA_PRINTF			libc_printf

#define OTA_DUMP(data,len) { const int l=(len); int i;\
for(i=0 ; i<l ; i++) OTA_PRINTF(" 0x%02x",*((data)+i)); \
OTA_PRINTF("\n"); }

#else
#define OTA_PRINTF(...)		do{}while(0)
#define OTA_DUMP(data,len) ;
#endif

extern struct nim_device *nim_dev;
static UINT32 tfcas_nit_crc = 0;
UINT32 test_nit[] = {
    0x005AF040,0x0000C101,0x0B4023F0,0x53414313, 
    0x5754454E,0xA14B524F,0x44000014,0x0098040B, 
    0x03F1FF00,0x0F756800,0x000202E5,0x002AF000, 
    0xF0010001,0x040B4424,0xFF000098,0x680003F0, 
    0x0F410075,0x00010100,0x03000102,0x01040001, 
    0x0F010500,0x56544F04,0xBD782600,0xFFFFFFA4 
};

#define OTA_PID		0x1CA0

static UINT8 ota_table_buff[PSI_SHORT_SECTION_LENGTH];
static OTA_TP_INFO otaTPInfo;
static UINT8 ota_sw_version_crtl;
static UINT16 ota_download_pid = 0, new_sw_version = 0;
static BOOL ota_info_init = FALSE;
static BOOL ota_force_flag = FALSE;
static BOOL manual_upgrade_flag = TRUE; // set this flag to false would disable manual msg to send
static ota_special_cb ota_callback = NULL;
static ota_special_cb ota_nit_callback = NULL;

extern UINT8 nit_cur_version;

extern UINT32 bcd2integer(UINT8 *ptr, UINT8 digits, INT32 offset);
static void ota_nit_parse(UINT8 *buff, INT16 buff_len);
static void nit_version_check(UINT8 *buff);

/*return value:  -1 fail else success*/
INT8 Char2Hex(UINT8 ch)
{
	INT8 ret =  - 1;
	if ((ch <= 0x39) && (ch >= 0x30))
	// '0'~'9'
		ret = ch &0xf;
	else if ((ch <= 102) && (ch >= 97))
	//'a'~'f'
		ret = ch - 97+10;
	else if ((ch <= 70) && (ch >= 65))
	//'A'~'F'
		ret = ch - 65+10;

	return ret;
}

UINT32 Str2UINT32(UINT8 *str, UINT8 len)
{
	UINT32 ret = 0;
	UINT8 i;
	INT temp;

	if (str == NULL)
		return 0;

	for (i = 0; i < len; i++)
	{
		temp = Char2Hex(*str++);
		if (temp ==  - 1)
			return 0;

		ret = (ret << 4) | temp;
	}

	return ret;
}

enum {
 	BCD_WIDTH				= 4,
	BCD_POWER				= 10,
	BCD_MASK				= 0x0F,
};

UINT32 bcd2str(UINT8 *ptr, UINT8 digits, UINT8 *str)
{
	UINT32 ret=0;
	INT32 i;
	UINT8 bcd;
	
	for(i=0; i<digits; i++) {
		bcd = ptr[i>>1];
		bcd = (i&0x01)? bcd&BCD_MASK: bcd>>BCD_WIDTH;

		//ComInt2Str(&str[i], bcd, 1);
		if (bcd >= 0x0a)
			str[i] = 'A' + bcd - 0x0a;
		else
			str[i] = bcd + '0';	//no matter bcd > 0x09
	}
	return ret;
}

//return -1:sn1<sn2; 0:sn1=sn2; 1:sn1<sn2
INT32 sn_cmp(UINT8 *sn1, UINT8 *sn2, UINT32 len)
{
	UINT32 ret=0;
	UINT32 i;
	UINT8 v1, v2;
	UINT8 offt;
	
	for(i=0; i<len; i++) {
		v1 = Char2Hex(*sn1++);
		v2 = Char2Hex(*sn2++);

		if(v1 < v2)
			return -1;
		else if(v1 > v2)
			return 1;
	}
	
	return 0;
}

static BOOL chunk_check_version(UINT32 chunk_id, UINT16 version)
{
	CHUNK_HEADER blk_header;
	UINT16 chunk_version;
	
	sto_get_chunk_header(chunk_id, &blk_header);
	chunk_version = Str2UINT32(&blk_header.version[0], 4);

	if (version > chunk_version)
		return TRUE;
	else
		return FALSE;
}

#ifndef OTA_TRIGGER_TS_SUPPORT
extern struct nim_device *g_nim_dev;
static void ota_loop1_parse(UINT8 *head)
{
	UINT8 i, j, k;
	UINT8 desc_tag = head[0];
	UINT8 desc_len = head[1];
	ControlMsgType_t msgtype;
  
	if (desc_tag == 0x99)
	{
		UINT32 oui = 0;
		UINT8 oui_len = 0;
		HW_SW_INFO hsi;

		OTA_PRINTF("[OTA]desc tag = 0x%X, len = %d\n",desc_tag,desc_len);

		api_hwsw_Info_get(&hsi);

		for (i = 2; i < 2 + desc_len; i += 1 + oui_len)
		{
			oui_len = head[i];
			oui = (head[i + 1] << 16) + (head[i + 2] << 8) + head[i + 3];
			OTA_PRINTF("[OTA]OUI = 0x%X\n",oui);
			
			if (hsi.oui != oui)
			{
				OTA_PRINTF("[OTA]OUI != STB OUI 0x%X\n",hsi.oui);
				continue;
			}

			UINT8 selector_len = 0;
			UINT16 nhm = 0, nhv = 0, nsm = 0, nsv = 0, chunk_des_len = 0;
			UINT16 ota_type, ota_option;
			
			for (j = i + 4; (j < i + oui_len - 1) && (j < 2 + desc_len); j += chunk_des_len + 9)
			{
				nhm = (head[j] << 8) + head[j + 1];
				nhv = (head[j + 2] << 8) + head[j + 3];
				nsm = (head[j + 4] << 8) + head[j + 5];
				nsv = (head[j + 6] << 8) + head[j + 7];
				ota_type = (head[j + 8] << 8) + head[j + 9];	//allcode or chunk
				ota_option = (head[j + 10] << 8) + head[j + 11];	//by sw_version or force upgrade
				chunk_des_len = head[j + 12];
				OTA_PRINTF("[OTA]hw_model = 0x%X\n",nhm);
				OTA_PRINTF("[OTA]hw_version = 0x%X\n",nhv);
				OTA_PRINTF("[OTA]sw_model = 0x%X\n",nsm);
				OTA_PRINTF("[OTA]sw_version = 0x%X\n",nsv);
				OTA_PRINTF("[OTA]ota_type = 0x%X\n",ota_type);
				OTA_PRINTF("[OTA]ota_option = 0x%X\n",ota_option);

				if (nhm == hsi.hw_model && nhv == hsi.hw_version && nsm == hsi.sw_model)
				{
					if (nsv > hsi.sw_version )
					{
						if(ota_option == 2)
						{/*force to upgrade*/
							OTA_PRINTF("[OTA]force upgrade!\n");
							msgtype = CTRL_MSG_SUBTYPE_STATUS_OTA_FORCE;
						}
						else
						{/*only upgrade lower version */
#if 0						
							UINT32 chunk_id = 0;
							UINT16 chunk_ver = 0;
							for (k = j + 13; k < j + 13 + chunk_des_len; k += 6)
							{
								chunk_id = (head[k] << 24) | (head[k + 1] << 16) | (head[k + 2] << 8) |head[k + 3];
								chunk_ver = (head[k + 4] << 8) | head[k + 5];
								if (chunk_check_version(chunk_id, chunk_ver))
								{
									OTA_PRINTF("%s: Chunk ID %08x with new ver [%d] need upgrade!\n", __FUNCTION__, chunk_id, chunk_ver);
									break;
								}
							}
							if (k >= j + 13 + chunk_des_len)
							{
								OTA_PRINTF("%s: No chunk need upgrade!\n", __FUNCTION__);
								continue;
							}
#endif
							msgtype = CTRL_MSG_SUBTYPE_STATUS_OTA_MANUAL;
							OTA_PRINTF("[OTA]manual upgrade!\n");
						}

						OTA_PRINTF("%s: ready enter OTA!\n", __FUNCTION__);
					
						nim_get_freq(g_nim_dev, &otaTPInfo.frequency);
						nim_get_sym(g_nim_dev, &otaTPInfo.symbol_rate);
						nim_get_FEC(g_nim_dev, &otaTPInfo.modulation);

						ota_download_pid = 0x1ca0;
						
						if(ota_callback!=NULL)
							ota_callback(msgtype,nsv);
					}
					else
					{
						OTA_PRINTF("[OTA]STEAM sw_version 0x%X <= STB sw_version = 0x%X\n",nsv,hsi.sw_version);
					}
					// Ready reset to enter OTA					
				}
				else
				{
					OTA_PRINTF("[OTA]STREAM HW/SW != STB HW/SW:\n");
					OTA_PRINTF("[OTA]STB hw_model = 0x%X\n",hsi.hw_model);
					OTA_PRINTF("[OTA]STB hw_version = 0x%X\n",hsi.hw_version);
					OTA_PRINTF("[OTA]STB sw_model = 0x%X\n",hsi.sw_model);
				}
			}
		}
	}
#if (CAS_TYPE == CAS_DVT && defined DVTCA_VERSION_NEW)
	else if (desc_tag == 0x5F)
	{
		UINT32 pdsd = (head[2] << 24) + (head[3] << 16) + (head[4] << 8) + head[5];
		SYSTEM_DATA *psd = sys_data_get();
		if (psd->pdsd == pdsd)
			return;

		psd->pdsd = pdsd;
		sys_data_save(0);
		dvt_set_pdsd(pdsd);
	}
#endif	
	else
	{
		OTA_PRINTF("[OTA]desc_tag[0x%X] is not a valid OTA descriptor tag!\n",desc_tag);
	}
}

INT32 ota_table_request()
{
	INT32 i, sec_len;
	UINT32 crc = 0;
	struct dmx_device *dmx_dev;

	OTA_PRINTF("[OTA]request OTA table, PID = 0x%X.\n",OTA_PID);

	if ((dmx_dev = (struct dmx_device*)dev_get_by_id(HLD_DEV_TYPE_DMX, 0)) == NULL)
	{
		OTA_PRINTF("ERROR : get dmx dev failure !");
		return 1;
	}
	struct get_section_param sr_request;
	struct restrict sr_restrict;

	MEMSET(&sr_restrict, 0, sizeof(struct restrict));
   
	sr_request.buff = (UINT8*)ota_table_buff;
	sr_request.buff_len = PSI_SHORT_SECTION_LENGTH;
	sr_request.crc_flag = 1;
	sr_request.pid = OTA_PID;
	sr_request.mask_value = &sr_restrict;
	sr_request.wai_flg_dly = 6000;

	sr_restrict.mask_len = 1;//6;
	sr_restrict.value_num = 1;
	MEMSET(sr_restrict.mask, 0, sr_restrict.mask_len);
	MEMSET(sr_restrict.value[0], 0, sr_restrict.mask_len);
	sr_restrict.mask[0] = 0xff;
	//sr_restrict.mask[1] = 0x80;
	//sr_restrict.mask[5] = 0x01;
	sr_restrict.value[0][0] = PSI_NIT_TABLE_ID;
	//sr_restrict.value[0][1] = 0x80;
	//sr_restrict.value[0][5] = 0x01;


	sr_request.get_sec_cb = NULL;
	sr_request.continue_get_sec = 0;

	if (dmx_req_section(dmx_dev, &sr_request) != SUCCESS)
	{
		OTA_PRINTF("ERROR : get section failure !\n");
		return 1; //ASSERT(0);//return OTA_FAILURE;
	}

	ota_nit_parse(sr_request.buff, sr_request.sec_tbl_len);    
	if (NULL != ota_nit_callback)
		ota_nit_callback((UINT32)(sr_request.buff), sr_request.sec_tbl_len);
	return 0;
}

#else
static void ota_loop1_parse(UINT8 *head)
{
	UINT8 desc_tag = head[0];
	UINT8 desc_len = head[1];
	UINT8 *byte_pos = NULL;
	UINT8 download_type = 0;
	
	if(desc_tag == 0xA1)
	{
		UINT16 stb_manufacturer_id = 0;
		INT16 bytes_leave = 0;
		HW_SW_INFO hw_sw_version;
		UINT32 frequency = 0;
		UINT16 symbol = 0;
		UINT16 modulation = 0;
		UINT16 download_pid = 0;
		UINT32 new_hw_model=0, new_hw_version=0, new_sw_model=0;
		UINT8 serial_start[20], serial_end[20];
		UINT8 sn_str[30], sn_start_str[30], sn_end_str[30];
		UINT8 sw_version_crtl = 0;

		OTA_PRINTF("[OTA]desc tag = 0x%X, len = %d\n",desc_tag,desc_len);

		api_hwsw_Info_get(&hw_sw_version);

		stb_manufacturer_id = head[2] << 8 | head[3];
		OTA_PRINTF("stb_manufacturer_id: %d\n", stb_manufacturer_id);
		if (hw_sw_version.oui != stb_manufacturer_id)//not our OTA stream
			return;
		
		bytes_leave = desc_len - 2;
		byte_pos = head + 4;

		while (bytes_leave > 0)
		{
			UINT8 private_data_len = 0;

			//Cable_delivery_system_descriptor
			if (byte_pos[0] == CABLE_DELIVERY_SYSTEM_DESCRIPTOR)
			{
				struct cable_delivery_system_descriptor *cable;
				cable = (struct cable_delivery_system_descriptor*)byte_pos;

				frequency = bcd2integer(cable->frequency, 8, 0) / 100;
				symbol = (UINT16)(bcd2integer(cable->symbol_rate, 7, 0) / 10);
				modulation = (UINT16)(cable->modulation + 3);
				OTA_PRINTF("frequency: %d\n", frequency);
				OTA_PRINTF("symbol_rate: %d\n", symbol);
				OTA_PRINTF("modulation: %d\n", modulation);

			}

			// PID
			byte_pos += sizeof(struct cable_delivery_system_descriptor);
			download_pid = ((byte_pos[0] << 8) | (byte_pos[1])) >> 3;
			OTA_PRINTF("download_pid: %d\n", download_pid);

			// download type:  1: Manual download; 2: Force download
			download_type = byte_pos[1] &0x07;
			OTA_PRINTF("download_type: %d\n", download_type);

			// Private data descriptor
			private_data_len = byte_pos[2];
			OTA_PRINTF("private_data: \n");
			OTA_DUMP(byte_pos + 2, private_data_len + 1);

			byte_pos += 3;

			if (private_data_len >= 43)
			{
				new_hw_model = (byte_pos[0] << 24) | (byte_pos[1] << 16) | (byte_pos[2] << 8) | byte_pos[3];
				new_hw_version = (byte_pos[4] << 24) | (byte_pos[5] << 16) | (byte_pos[6] << 8) | byte_pos[7];
				new_sw_model = (byte_pos[8] << 24) | (byte_pos[9] << 16) | (byte_pos[10] << 8) | byte_pos[11];
				new_sw_version = (byte_pos[12] << 24) | (byte_pos[13] << 16) | (byte_pos[14] << 8) | byte_pos[15];
				MEMSET(serial_start, 0, sizeof(serial_start));
				MEMCPY(serial_start, &byte_pos[16], 13);
				MEMSET(serial_end, 0, sizeof(serial_end));
				MEMCPY(serial_end, &byte_pos[29], 13);
				sw_version_crtl = byte_pos[42];
			}

			if((new_hw_model != hw_sw_version.hw_model)||(new_hw_version != hw_sw_version.hw_version))
				return;

			otaTPInfo.frequency = frequency;
			otaTPInfo.symbol_rate = symbol;
			otaTPInfo.modulation = modulation;
			ota_download_pid = download_pid;
			ota_sw_version_crtl = sw_version_crtl;

			// serial number check
			MEMSET(sn_str, 0, sizeof(sn_str));
			api_sn_get(sn_str);

			MEMSET(sn_start_str, 0, sizeof(sn_start_str));
			bcd2str(serial_start, 26, sn_start_str);

			MEMSET(sn_end_str, 0, sizeof(sn_end_str));
			bcd2str(serial_end, 26, sn_end_str);

			if(sn_cmp(sn_start_str, sn_str, 26)>0 || sn_cmp(sn_str,sn_end_str, 26)>0)
			{
				OTA_PRINTF("sn_str:0x%x, sn_start_str:0x%x, sn_end_str:0x%x\n",sn_str,sn_start_str, sn_end_str);
				return ;
			}

			// sw version check
			if(sw_version_crtl==0)
			{// ask new_sw_version > hw_sw_version.sw_version
				if(new_sw_version <= hw_sw_version.sw_version)
				{
					OTA_PRINTF("sw_version_crtl=%d, new_sw_version:0x%x, hw_sw_version.sw_version:0x%x\n",sw_version_crtl,new_sw_version, hw_sw_version.sw_version);
					return ;
				}
			}
			else if(sw_version_crtl==1)
			{// ask new_sw_version = hw_sw_version.sw_version
				if(new_sw_version != hw_sw_version.sw_version)
				{
					OTA_PRINTF("sw_version_crtl=%d, new_sw_version:0x%x, hw_sw_version.sw_version:0x%x\n",sw_version_crtl,new_sw_version, hw_sw_version.sw_version);
					return ;
				}
			}
			else if(sw_version_crtl==2)
			{// ask new_sw_version != hw_sw_version.sw_version
				if(new_sw_version == hw_sw_version.sw_version)
				{
					OTA_PRINTF("sw_version_crtl=%d, new_sw_version:0x%x, hw_sw_version.sw_version:0x%x\n",sw_version_crtl,new_sw_version, hw_sw_version.sw_version);
					return ;
				}
			}
			else
			{
				OTA_PRINTF("sw_version_crtl=%d, unknow type\n",sw_version_crtl);
				return ;
			}
				
			
			if(download_type == 2)
			{//download type 2: force upgrade
				ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_OTA_FORCE,0, FALSE);
				OTA_PRINTF("OTA: Force upgrade!\n");
				return ;
			}
			else if(download_type == 1)
			{//downlaod type 1:manual upgrade
				ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_OTA_MANUAL,0, FALSE);
				OTA_PRINTF("OTA: manual upgrade!\n");
				return ;
			}
						
		}

	}
}

INT32 ota_table_request()
{
	INT32 i, sec_len;
	UINT32 crc = 0;
	struct dmx_device *dmx_dev;
	if ((dmx_dev = (struct dmx_device*)dev_get_by_id(HLD_DEV_TYPE_DMX, 0)) == NULL)
	{
		OTA_PRINTF("ERROR : get dmx dev failure !");
		return 1;
	}
	struct get_section_param sr_request;
	struct restrict sr_restrict;

	MEMSET(&sr_restrict, 0, sizeof(struct restrict));

	sr_request.buff = (UINT8*)ota_table_buff;
	sr_request.buff_len = PSI_SHORT_SECTION_LENGTH;
	sr_request.crc_flag = 1;
	sr_request.pid = PSI_NIT_PID; //nit table id,dvb defined.
	sr_request.mask_value = &sr_restrict;
	sr_request.wai_flg_dly = 2000;

	sr_restrict.mask_len = 6;
	sr_restrict.value_num = 1;
	MEMSET(sr_restrict.mask, 0, sr_restrict.mask_len);
	MEMSET(sr_restrict.value[0], 0, sr_restrict.mask_len);
	sr_restrict.mask[0] = 0xff;
	sr_restrict.mask[1] = 0x80;
	sr_restrict.mask[5] = 0x01;
	sr_restrict.value[0][0] = PSI_NIT_TABLE_ID;
	sr_restrict.value[0][1] = 0x80;
	sr_restrict.value[0][5] = 0x01;

	sr_request.get_sec_cb = NULL;
	sr_request.continue_get_sec = 0;

	INT32 retval = dmx_req_section(dmx_dev, &sr_request);
	if(retval != SUCCESS)
	{
		OTA_PRINTF("ERROR : get section failure !");
		return 1; //ASSERT(0);//return OTA_FAILURE;
	}
	/*if (dmx_req_section(dmx_dev, &sr_request) != SUCCESS)
	{
		OTA_PRINTF("ERROR : get section failure !");
		return 1; //ASSERT(0);//return OTA_FAILURE;
	}*/
#ifdef AD_TYPE
	AD_ParseNit(sr_request.buff, sr_request.sec_tbl_len);
#endif
	ota_nit_parse(sr_request.buff, sr_request.sec_tbl_len);    
	if (NULL != ota_nit_callback)
		ota_nit_callback((UINT32)(sr_request.buff), sr_request.sec_tbl_len);
	return 0;
}
#endif

#if (CAS_TYPE == CAS_DVT && defined DVTCA_VERSION_NEW)
static UINT8 nit_table_buff[PSI_SHORT_SECTION_LENGTH];
static void nit_pdsd_parse(struct get_section_param *param)
{
	ota_nit_parse(param->buff, param->sec_tbl_len);	
}

INT32 nit_pdsd_request()
{
	INT32 i, sec_len;
	UINT32 crc = 0;
	UINT8 flt_idx;
	struct dmx_device *dmx_dev;
	libc_printf("enter %s\n",__FUNCTION__);
	if ((dmx_dev = (struct dmx_device*)dev_get_by_id(HLD_DEV_TYPE_DMX, 0)) == NULL)
	{
		libc_printf("ERROR : get dmx dev failure !");
		return 1;
	}
	struct get_section_param sr_request;
	struct restrict sr_restrict;

	MEMSET(&sr_restrict, 0, sizeof(struct restrict));
   
	sr_request.buff = (UINT8*)nit_table_buff;
	sr_request.buff_len = PSI_SHORT_SECTION_LENGTH;
	sr_request.crc_flag = 1;
	sr_request.pid = 0x10;
	sr_request.mask_value = &sr_restrict;
	sr_request.wai_flg_dly = 10000;

	sr_restrict.mask_len = 1;//6;
	sr_restrict.value_num = 1;
	MEMSET(sr_restrict.mask, 0, sr_restrict.mask_len);
	MEMSET(sr_restrict.value[0], 0, sr_restrict.mask_len);
	sr_restrict.mask[0] = 0xff;
	//sr_restrict.mask[1] = 0x80;
	//sr_restrict.mask[5] = 0x01;
	sr_restrict.value[0][0] = PSI_NIT_TABLE_ID;
	//sr_restrict.value[0][1] = 0x80;
	//sr_restrict.value[0][5] = 0x01;

	sr_request.get_sec_cb = nit_pdsd_parse;
	sr_request.continue_get_sec = 1;

	if (dmx_async_req_section(dmx_dev, &sr_request, &flt_idx) != SUCCESS)
	{
		libc_printf("ERROR : get section failure !\n");
		return 1; //ASSERT(0);//return OTA_FAILURE;
	}
	return 0;
}
#endif

static void ota_nit_parse(UINT8 *buff, INT16 buff_len)
{
	UINT16 i = 0;
	UINT16 sec_len = 0;
	UINT8 desc_tag = 0;
	UINT8 *loop1_header = NULL;
	UINT16 loop1_count = 0;
	UINT8 *loop2_header = NULL;
	UINT16 loop2_count = 0;
	UINT8 *loop3_header = NULL;
	UINT8 *byte_pos = NULL;
	UINT16 loop3_count = 0;

	UINT16 network_des_len = 0;
	UINT16 trans_stream_loop_len = 0;
	UINT16 ts_id = 0;
	UINT16 trans_des_len = 0;
	UINT16 desc_len = 0;
	struct nit_info *temp_header = NULL;
	struct nit_info *temp_com = NULL;
	UINT8 ret_loop1 = 0;

	OTA_PRINTF("enter ota_nit_parse\n");

//=========================================================
// jinfeng added: parse for version
	
	nit_version_check(buff);
	
//=========================================================

#if SYS_CPU_ENDIAN == ENDIAN_BIG	
	((INT8*) &network_des_len)[0] = buff[8] &0x0F;
	((INT8*) &network_des_len)[1] = buff[9];
	((INT8*) &sec_len)[0] = buff[1] &0x03;
	((INT8*) &sec_len)[1] = buff[2];
	((INT8*) &trans_stream_loop_len)[0] = buff[network_des_len + 10] &0x0F;
	((INT8*) &trans_stream_loop_len)[1] = buff[network_des_len + 11];
#endif
#if SYS_CPU_ENDIAN == ENDIAN_LITTLE
	((INT8*) &network_des_len)[0] = buff[9];
	((INT8*) &network_des_len)[1] = buff[8] &0x0F;
	((INT8*) &sec_len)[0] = buff[2];
	((INT8*) &sec_len)[1] = buff[1] &0x03;
	((INT8*) &trans_stream_loop_len)[0] = buff[network_des_len + 11];
	((INT8*) &trans_stream_loop_len)[1] = buff[network_des_len + 10] &0x0F;
#endif

	/*get ota download info*/
	loop1_header = buff + 10;
	while (network_des_len - loop1_count)
	{
		desc_tag = loop1_header[0];
		desc_len = loop1_header[1];

		ota_loop1_parse(loop1_header);

		loop1_count += (desc_len + 2);
		loop1_header += (desc_len + 2);
	}
}

void ota_change_nim()
{
    struct nim_device *nim = dev_get_by_id(HLD_DEV_TYPE_NIM, 0);

    nim_channel_change(nim,otaTPInfo.frequency,
        otaTPInfo.symbol_rate,otaTPInfo.modulation);
    //nim_channel_change(nim,61800,6875,6);
}

UINT16 ota_get_download_pid()
{
	return ota_download_pid;
}

void ota_get_tp_info(UINT32 *freq,UINT32 *sym,UINT32 *qam)
{
    *freq = otaTPInfo.frequency;
    *sym = otaTPInfo.symbol_rate;
    *qam = otaTPInfo.modulation;
}

BOOL ota_get_force_flag()
{
    return ota_force_flag;
}

UINT8 ota_get_sw_version_ctrl()
{
	return ota_sw_version_crtl;
}

void ota_callback_register(ota_special_cb cb, UINT8 type)
{
	if (0 == type)
		ota_callback = cb;
	else
		ota_nit_callback = cb;
}

static void nit_version_check(UINT8 *buff)
{
	UINT8 nit_version=0;
	
	nit_version = system_config.nit_version;

	nit_cur_version = (buff[5] & 0x01) ? (buff[5]>>1 & 0x1F) : (nit_version);

	if(nit_cur_version != nit_version)
		//ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_NIT_UPDATE, (UINT32)nit_cur_version, FALSE);
		ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_NIT_UPDATE, 0, FALSE);
	else 
		system_config.nit_version++;
}

