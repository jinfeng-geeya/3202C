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

#include <api/libtsi/si_descriptor.h>

#include <api/libota/lib_ota_c.h>

#ifdef USB_UPG_DVBC
#include <api/libfs2/stat.h>
#include <api/libfs2/dirent.h>
#include <api/libfs2/statvfs.h>
#include <api/libfs2/unistd.h>
#include <api/libfs2/stdio.h>
#endif

#define OTA_PRINTF  PRINTF

static OSAL_ID g_ota_tsk_id;
static OSAL_ID g_ota_mbf_id = OSAL_INVALID_ID;
OSAL_ID  g_ota_flg_id = OSAL_INVALID_ID;

static UINT8* g_ota_ram1_addr;// = (UINT8*)__MM_TTX_PB_START_ADDR;
static UINT32 g_ota_ram1_len;

static UINT32 g_ota_size;

static UINT8* g_ota_ram2_addr;// = (UINT8*)__MM_DVW_START_ADDR;
static UINT32 g_ota_ram2_len;
static INT32 g_ota_size_2;

static UINT8* g_ota_swap_addr;// = (UINT8*)__MM_SUB_PB_START_ADDR;

struct DCGroupInfo g_dc_group;
#define DC_MODULE_MAX 	4
struct DCModuleInfo g_dc_module[DC_MODULE_MAX];
UINT8 	g_dc_module_num;
UINT16	g_dc_blocksize;


#define g_ota_section_buf_len 5000 
static UINT8 g_ota_section_buf[g_ota_section_buf_len];

static struct service_info g_service_info;
static struct dl_info g_dl_info;
static UINT32 g_lnb_frequency;

static INT32 received_section_num;
static INT32 total_block_num;
static t_progress_disp progress_disp;
static t_parse_cb parse_cb;

static UINT8 ota_task_suspend_flag = 0;


INT32 ota_send_cmd(struct ota_cmd* cmd,UINT32 tmout);

void ota_reg_disp_callback(t_progress_disp ota_progress_disp)
{
    if(ota_progress_disp == NULL)
        return;
    
    progress_disp = ota_progress_disp;
}

void ota_reg_parse_callback(t_parse_cb ota_parse_cb)
{
    if(ota_parse_cb == NULL)
        return;
    
    parse_cb = ota_parse_cb;
}

INT32 parse_dsmcc_sec(UINT8 *buffer, INT32 buffer_length, void *param)
{
	BOOL ret;
	
	UINT8 byte_hi,byte_lo;
	UINT8 table_id,section_num,last_section_num;
	UINT16 section_len,table_id_extension;
	UINT16 new_sw_version;

	struct section_param *sec_param = (struct section_param *)param;
	

	table_id = *buffer++;
	byte_hi = *buffer++;
	byte_lo = *buffer++;
	section_len = ((byte_hi&0x0f)<<8) + byte_lo;
	byte_hi = *buffer++;
	byte_lo = *buffer++;
	table_id_extension = (byte_hi<<8) + byte_lo;	
	byte_lo = *buffer++;
	section_num = *buffer++;
	last_section_num = *buffer++;

	switch(sec_param->section_type)
	{
		case 0x00: /*------------------STEP_DSI---------------------------*/	

			if(table_id_extension!=0x0000 && table_id_extension!=0x0001)
				return OTA_FAILURE;

			ret = dsi_message(buffer,
							section_len-9,
							0,			
							0,
							0,
							0,
							0,
							&new_sw_version,
							&g_dc_group);
			if(ret == TRUE)
			{
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
							g_ota_ram1_addr,
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

	OTA_PRINTF("rcvd :module_id = %x, section_num = %d\n",table_id_extension,section_buf[6]);
	
	for(modul_idx=0;modul_idx<g_dc_module_num;modul_idx++)
	{
		if(g_dc_module[modul_idx].module_id == table_id_extension)
			break;
	}
	if(modul_idx==g_dc_module_num)
		return;
	if((UINT8)(g_dc_module[modul_idx].block_num-1) != section_buf[7])
	{
		OTA_PRINTF("g_dc_module[i].block_num-1 != last_section_num\n");	
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
	OTA_PRINTF("%d%\n",received_section_num*100/total_block_num);
    if(progress_disp!=NULL)
	    progress_disp(PROCESS_NUM,received_section_num*100/total_block_num);	
	return;
}
INT32 si_private_sec_parsing_start(UINT8 section_type,UINT16 pid, section_parser_t section_parser, void *param)
{
	INT32 i;
	struct dmx_device * dmx_dev;
	if((dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0)) == NULL)
	{
		OTA_PRINTF("ERROR : get dmx dev failure !");
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
	sr_request.wai_flg_dly = 300000;
	
	if(section_type == 0)
	{
		/* DSI  */
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
	if(section_type == 2)
	{
		/* DDB  */
		if(g_dc_module_num>4)
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
		OTA_PRINTF("ERROR : get section failure !");
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

	OTA_PRINTF("Get download_info...\n");

	OTA_PRINTF("g_ota_ram1_addr = 0x%8x\n",g_ota_ram1_addr);

/* step 1 : DSI  */

	sec_param.section_type = 0x00;
	
       ret = si_private_sec_parsing_start(0,
	   							pid,
								parse_dsmcc_sec,
								(void*)(&sec_param));	
	if(ret != OTA_SUCCESS)
	{
		OTA_PRINTF("ERROR : No  ota service exist!\n");
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
		OTA_PRINTF("ERROR : No  ota service exist!\n");
		return FALSE;
	}	

	g_ota_size =  g_dl_info.sw_size;
	
	MEMCPY(info, &g_dl_info,sizeof(struct dl_info));

	OTA_PRINTF("Get download inof Done!\n");
	return TRUE;
	
}

static BOOL ota_un7z_check_crc()
{
	UINT8* block_addr;
	UINT32 check_size,check_crc,offset,code_size;
    
	OTA_PRINTF("Unzip code...\n");
	if(un7zip(g_ota_ram1_addr,g_ota_ram2_addr,g_ota_swap_addr) != 0)
	{
		OTA_PRINTF("ERROR: un7zip failed!\n");
		return FALSE;		
	}
	g_ota_size_2 = *((UINT32*)g_ota_swap_addr);
	if((UINT32)g_ota_size_2>g_ota_ram2_len)
	{
		OTA_PRINTF("ERROR: unzip_buf exceed!\n");
		return FALSE;			
	}

	OTA_PRINTF("Check CRC...\n");

	MG_Setup_CRC_Table();
	offset = 0;
	block_addr = g_ota_ram2_addr;
	code_size = g_ota_size_2;
	OTA_PRINTF("block_addr = 0x%8x \n",block_addr);
	OTA_PRINTF("code_size = 0x%8x \n",code_size);	
	do{
		block_addr += offset;
		check_size = (block_addr[4]<<24)
					+(block_addr[5]<<16)
					+(block_addr[6]<<8)
					+block_addr[7];

		offset = 		(block_addr[8]<<24)
					+(block_addr[9]<<16)
					+(block_addr[10]<<8)
					+block_addr[11];

		if(offset > code_size)
		{
			OTA_PRINTF("ERROR : size NOT enough !\n");
			return FALSE;//ASSERT(0);//return FALSE;
		}			
		code_size -= offset;
		
		
		check_crc = (block_addr[12]<<24)
					+(block_addr[13]<<16)
					+(block_addr[14]<<8)
					+block_addr[15];
		OTA_PRINTF("crc = 0x%8x \n",check_crc);
		if(check_crc != 0x4e435243)
		{
			if(check_crc != (UINT32)MG_Table_Driven_CRC(0xFFFFFFFF, block_addr + 0x10, check_size))
			{
				OTA_PRINTF("ERROR : CRC error !\n");
				return FALSE;//ASSERT(0);//return FALSE;
			}	
		}
	}while(offset!=0&&code_size>0);	  

    return TRUE;
}

BOOL ota_cmd_start_download(UINT16 pid)
{
	UINT32 i;
	INT32 ret;

	if(g_ota_size>g_ota_ram1_len)
	{
		OTA_PRINTF("ERROR: download_buf exceed!\n");
		return FALSE;			
	}
	
/* step 1: process_download */		

OTA_PRINTF("Start Download...\n");

    if(progress_disp!=NULL)
    	progress_disp(PROCESS_NUM,0);	
	received_section_num = 0;
	total_block_num = 0;
	for(i=0;i<g_dc_module_num;i++)
		total_block_num+=g_dc_module[i].block_num;
	
       ret = si_private_sec_parsing_start(2,pid,NULL,NULL);
	if(ret != OTA_SUCCESS)
	{
		OTA_PRINTF("ERROR : No  ota service exist!\n");
		return FALSE;
	}	

    if(progress_disp!=NULL)
	    progress_disp(PROCESS_NUM,100);
		
/* step 2: check crc */		
	ret = ota_un7z_check_crc();

	OTA_PRINTF("Download Finished !\n");
	return ret;

	
}

BOOL mem_cmp(UINT8* src,UINT8* dst,UINT32 size)
{
	UINT32 i;
	for(i=0;i<size;i++)
		if(*src++ != *dst++) return FALSE;
	return TRUE;
}

BOOL ota_cmd_start_burnflash(UPG_TYPE upg_type)
{
	UINT32 f_bootloader_len = 0,m_bootloader_len = 0;
	UINT32 code_size, write_size;
	UINT8* m_block_addr;
	UINT8* m_f_block_addr;	//read flash data to sdram
	UINT32 f_block_addr;	//relative address
	UINT8 *buf_add = NULL;
	UINT32 chunk_id;
	UINT32 total_len = 0,chunk_len,maincode_chunk_len;
	CHUNK_HEADER chunk_header;
	UINT32 f_pointer = 0;
	UINT32 data_id;
	UINT32 data_len;
	UINT32 data_off;
	
	struct sto_device *f_dev;
	UINT32 param;

	m_f_block_addr = g_ota_swap_addr;

	/* Init FLASH device */
	if ((f_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0)) == NULL)
	{
		OTA_PRINTF("ERROR: Not found Flash device!\n");
		return FALSE;
	}
	if (sto_open(f_dev) != SUCCESS)
	{
		OTA_PRINTF("ERROR: sto_open failed!\n");
		return FALSE;
	}


	m_block_addr = g_ota_ram2_addr;
	f_block_addr = 0; //relative address
	
	chunk_init((unsigned long)g_ota_ram2_addr,f_dev->totol_size);
	chunk_id = MAINCODE_ID;
	get_chunk_header(chunk_id, &chunk_header);
	maincode_chunk_len = chunk_header.offset;
	
	m_bootloader_len = (UINT32)chunk_goto(&chunk_id, 0xFFFFFFFC, 1) ;
	buf_add = (UINT8*)(m_bootloader_len + maincode_chunk_len);//The chunk behind maincode.

	sto_get_chunk_header(chunk_id, &chunk_header);
	f_pointer = sto_chunk_goto(&chunk_id, 0xFFFFFFFC, 1);
	f_bootloader_len = f_pointer;
	f_pointer += chunk_header.offset;

	if(maincode_chunk_len!=chunk_header.offset)
	{//The chunk number changes,so resort the chunks.
		/*Resort chunks in the buffer and write back to flash.*/
		do
		{
			data_id  = sto_fetch_long((UINT32)f_pointer + CHUNK_ID);
			data_len = sto_fetch_long((UINT32)f_pointer + CHUNK_LENGTH);
			data_off = sto_fetch_long((UINT32)f_pointer + CHUNK_OFFSET);

			if (f_pointer != 0)
			{
				if ((data_id >> 16) & 0xFF + (data_id >> 24) & 0xFF != 0xFF)
					return FALSE;
			}

			if (data_off > (UINT32)f_dev->totol_size)
			{
				return FALSE;
			}
			if ((data_off != 0) && (data_len > data_off))
			{
				return FALSE;
			}

			if(data_off == 0)//The offset of the last chunk(userdb) is 0.
				chunk_len = data_len+16;
			else
				chunk_len = data_off;
			sto_get_data(f_dev, buf_add, f_pointer, chunk_len);

			buf_add += chunk_len;
			f_pointer += chunk_len;
			total_len += chunk_len;		
			
		}while (data_off);
		/*end*/
	}
 
	code_size = g_ota_size_2 = total_len + maincode_chunk_len;

   	switch(upg_type)
	{
		case MAINCODE:
			f_block_addr += f_bootloader_len; //skip bootloader of flash address
			m_block_addr = (UINT8*)m_bootloader_len; //skip bootloader of sdram address
			break;
		case BOOTLOADER_MAINCODE:
			code_size += m_bootloader_len;
			break;
		default:
			break;
	}
	
#ifdef OTA_POWEROFF_SAFE
	UINT32 f_backup_addr;
	UINT32 ota_restore_size,ota_m_size,start_flag;
	UINT8 *ota_restore_addr,*ota_m_addr;
	UINT8 b_end_mark[16] = {0x65,0x72,0x6F,0x74,0x73,0x65,0x72,0x5F,0x67,0x6F,0x72,0x70,0x5F,0x61,0x74,0x6F};/* erotser_gorp_ato*/

	f_backup_addr = SYS_FLASH_BASE_ADDR + f_dev->totol_size - g_ota_size - 20;
	//align to 64k
	f_backup_addr = f_backup_addr - f_backup_addr%(64*1024);
	
	param = f_backup_addr - SYS_FLASH_BASE_ADDR;
	if((g_ota_size+20) < (f_dev->totol_size - param))
	{
        if(progress_disp!=NULL)
		    progress_disp(PROCESS_NUM,OTA_START_BACKUP_FLAG);
		ota_restore_addr = (UINT8 *)f_backup_addr - SYS_FLASH_BASE_ADDR;
		
		UINT32 flash_cmd;
		if(f_dev->totol_size <= 0x400000)
		{
			/* erase flash space for backup */
			param = param << 10;	
			/* Lower 10 bits of LSB is length in K bytes*/
			param += (f_dev->totol_size-(INT32)ota_restore_addr) >> 10;
			flash_cmd = STO_DRIVER_SECTOR_ERASE;
		}
		else
		{
			UINT32 tmp_param[2];
			tmp_param[0] = param;
			tmp_param[1] = (f_dev->totol_size-(INT32)ota_restore_addr) >> 10;
			param = (UINT32)tmp_param;
			flash_cmd = STO_DRIVER_SECTOR_ERASE_EXT;
		}
        if(progress_disp!=NULL)
		    progress_disp(PROCESS_NUM,3);
		if(sto_io_control(f_dev, flash_cmd, param) != SUCCESS)
		{
			OTA_PRINTF("ERROR: ota clean-up restore data sectors  failed!\n");
			return FALSE;
		}
		/* write start backup flag */
        if(progress_disp!=NULL)
		    progress_disp(PROCESS_NUM,15);
		if (sto_lseek(f_dev,(INT32)ota_restore_addr+16,STO_LSEEK_SET) != (INT32)ota_restore_addr+16)
		{
			OTA_PRINTF("ERROR: ota restore sto_lseek failed!\n");
			return FALSE;
		}
		start_flag = BACKUP_START_FLAG;
		if(sto_write(f_dev,(UINT8 *) &start_flag, 4)!=4)
		{
			OTA_PRINTF("ERROR:ota restore write start flag  failed!\n");
			return FALSE;
		}
        if(progress_disp!=NULL)
		    progress_disp(PROCESS_NUM,20);
		/* write ota data to bfd20000*/
		ota_restore_size = g_ota_size;
		ota_restore_addr += 20;
		ota_m_addr = g_ota_ram1_addr;
		while(ota_restore_size)
		{
			write_size = (ota_restore_size < SECTOR_SIZE) ? ota_restore_size : SECTOR_SIZE;	
			if (sto_lseek(f_dev,(INT32)ota_restore_addr,STO_LSEEK_SET) != (INT32)ota_restore_addr)
			{
				OTA_PRINTF("ERROR: ota sto_lseek failed!\n");
				return FALSE;
			}	
			if(sto_write(f_dev, ota_m_addr, write_size)!=(INT32)write_size)
			{
				OTA_PRINTF("ERROR:ota write restore data failed!\n");
				return FALSE;
			}		
			ota_restore_addr +=SECTOR_SIZE;			
			ota_m_addr +=write_size;			
			ota_restore_size -= write_size;
            if(progress_disp!=NULL)
			    progress_disp(PROCESS_NUM,(g_ota_size-ota_restore_size)*80/g_ota_size +20);
		}
		/* write restore data done flag */
		ota_restore_addr = (UINT8*)f_backup_addr - SYS_FLASH_BASE_ADDR;
		if (sto_lseek(f_dev,(INT32)ota_restore_addr,STO_LSEEK_SET) != (INT32)ota_restore_addr)
		{
			OTA_PRINTF("ERROR: ota sto_lseek failed!\n");
			return FALSE;
		}	
		if(sto_write(f_dev, (UINT8 *)b_end_mark, 16)!=16)
		{
			OTA_PRINTF("ERROR:ota write restore done flag failed!\n");
			return FALSE;
		}
        if(progress_disp!=NULL)
        {
    		progress_disp(PROCESS_NUM,100);
        }
		
	}
#endif
        if(progress_disp!=NULL)
        {
    		progress_disp(PROCESS_NUM,OTA_START_END_FLAG);
        }

	while(code_size)
	{
	    write_size = (code_size < SECTOR_SIZE) ? code_size : SECTOR_SIZE;
		sto_get_data(f_dev, m_f_block_addr, f_block_addr, write_size);
		if(mem_cmp(m_f_block_addr, m_block_addr, write_size)==FALSE)
		{
			UINT32 flash_cmd;
			if(f_dev->totol_size <= 0x400000)
			{
				/* Uper 22 bits of MSB is start offset based on SYS_FLASH_BASE_ADDR */
				param = f_block_addr;
				param = param << 10;	
				/* Lower 10 bits of LSB is length in K bytes*/
				param += SECTOR_SIZE >> 10;
				flash_cmd = STO_DRIVER_SECTOR_ERASE;
			}
			else
			{
				UINT32 tmp_param[2];
				tmp_param[0] = f_block_addr;
				tmp_param[1] = SECTOR_SIZE >> 10;
				flash_cmd = STO_DRIVER_SECTOR_ERASE_EXT;
				param = (UINT32)tmp_param;
			}
			if(sto_io_control(f_dev, flash_cmd, param) != SUCCESS)
			{
				OTA_PRINTF("ERROR: erase flash memory failed!\n");
				return FALSE;
			}		

			if (sto_lseek(f_dev,(INT32)f_block_addr,STO_LSEEK_SET) != (INT32)f_block_addr)
			{
				OTA_PRINTF("ERROR: sto_lseek failed!\n");
				return FALSE;
			}	
			if(sto_write(f_dev, m_block_addr, write_size)!=(INT32)write_size)
			{
				OTA_PRINTF("ERROR: sto_write failed!\n");
				return FALSE;
			}		
		}
		f_block_addr +=SECTOR_SIZE;			
		m_block_addr +=write_size;			
		code_size -= write_size;
		
        if(progress_disp!=NULL)
		    progress_disp(PROCESS_NUM,(g_ota_size_2-code_size)*100/g_ota_size_2);
	}

	OTA_PRINTF("Write flash done!\n");
	return TRUE;
		
}

BOOL ota_cmd_service_stopped()
{
	struct dmx_device * dmx_dev;
	if((dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0)) == NULL)
	{
		OTA_PRINTF("ERROR : get dmx dev failure !");
		return OTA_FAILURE;
	}		
	dmx_io_control(dmx_dev, CLEAR_STOP_GET_SECTION, (UINT32)NULL);
}

void ota_mem_config(UINT32 compressed,UINT32 compressed_len,UINT32 uncompressed,UINT32 uncompressed_len,UINT32 swap_addr,UINT32 swap_len)
{
	g_ota_ram1_addr = (UINT8*)compressed;
	g_ota_ram1_len = compressed_len;
	g_ota_ram2_addr  = (UINT8*)uncompressed;	
	g_ota_ram2_len = uncompressed_len;
	g_ota_swap_addr= (UINT8*)swap_addr;
	ASSERT(swap_len> 128*1024);
}

BOOL ota_start_service(UINT16 pid,struct dl_info *info)
{
	struct ota_cmd this_cmd;
	UINT32 flgptn_after;

	this_cmd.cmd_type = OTA_START_SERVICE;
	this_cmd.para16 = pid;
	this_cmd.para32 = (UINT32)info;

	return (E_OK == ota_send_cmd(&this_cmd,OSAL_WAIT_FOREVER_TIME));
}

BOOL ota_stop_service(void)
{
	struct ota_cmd this_cmd;
	UINT flgptn = 0;
	
	struct dmx_device * dmx_dev;
	
	if((dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0)) == NULL)
	{
		OTA_PRINTF("ERROR : get dmx dev failure !");
		return FALSE;   //OTA_FAILURE;
	}	
	dmx_io_control(dmx_dev, IO_STOP_GET_SECTION, (UINT32)NULL);

	this_cmd.cmd_type = OTA_STOP_SERVICE;
	this_cmd.para16 = 0;
	this_cmd.para32 = 0;

	ota_send_cmd(&this_cmd,OSAL_WAIT_FOREVER_TIME);
    return TRUE;
}

INT32 ota_send_cmd(struct ota_cmd* cmd,UINT32 tmout)
{
    return osal_msgqueue_send(g_ota_mbf_id, cmd, sizeof(struct ota_cmd),tmout);
}

BOOL ota_cmd_implement(UINT16 ota_cmd, UINT16 para16, UINT32 para32)
{
	BOOL ret_bool = FALSE;
	switch(ota_cmd)
	{
	case OTA_START_SERVICE:
		ret_bool = ota_cmd_get_download_info(para16,(struct dl_info *)para32);
		osal_flag_set(g_ota_flg_id,(ret_bool == TRUE)?PROC_SUCCESS:PROC_FAILURE);
	        if(ret_bool!=TRUE)
	            break;

		ret_bool = ota_cmd_start_download(para16);
		osal_flag_set(g_ota_flg_id,(ret_bool == TRUE)?PROC_SUCCESS:PROC_FAILURE);
	        if(ret_bool!=TRUE)
	            break;
        
		ret_bool = ota_cmd_start_burnflash(MAINCODE);
       		osal_flag_set(g_ota_flg_id,(ret_bool == TRUE)?PROC_SUCCESS:PROC_FAILURE);
		break;
	case OTA_STOP_SERVICE:
		ret_bool = ota_cmd_service_stopped();
		osal_flag_set(g_ota_flg_id,PROC_STOPPED);
		break;		
	default :		
		break;

	}
	
	return ret_bool;
}

void ota_cmd_implement_wrapped(struct ota_cmd * ota_cmd)
{
	ota_cmd_implement(ota_cmd->cmd_type,ota_cmd->para16,ota_cmd->para32);
}


void ota_task_suspend()
{
   ota_task_suspend_flag = 1;
}

void ota_task_resume()
{
    ota_task_suspend_flag = 0;
}


void ota_task(void)
{
	OSAL_ER ret_val;
	UINT32 msg_size;	
	struct ota_cmd msg;	
	while(1)
	{
		ret_val = osal_msgqueue_receive((VP)&msg, &msg_size, g_ota_mbf_id,2000);//OSAL_WAIT_FOREVER_TIME);

            /*suspend ota task for ipanel mw*/
            if(ota_task_suspend_flag==1)
            {
                osal_task_sleep(100);
                continue;
            }

            if ((ret_val != E_OK) || (msg_size != sizeof(struct ota_cmd)))
	   {
	            OTA_PRINTF("OTA: parse begin\n");
	            if(parse_cb)
	                parse_cb();
	            OTA_PRINTF("OTA: parse end\n\n");
				continue;
	   }
		ota_cmd_implement_wrapped(&msg);
	}
}

BOOL ota_init(void)
{
	T_CMBF t_cmbf;
	T_CTSK t_ctsk;

	t_cmbf.bufsz =  sizeof(struct ota_cmd) * 20;
	t_cmbf.maxmsz = sizeof(struct ota_cmd);
	g_ota_mbf_id = osal_msgqueue_create(&t_cmbf);

	if (g_ota_mbf_id == OSAL_INVALID_ID)
	{
		OTA_PRINTF("OTA: cre_mbf ota_msgbuf_id failed in %s\n",__FUNCTION__);
		return FALSE;
	}

    g_ota_flg_id=osal_flag_create(0x00000000);
	if(OSAL_INVALID_ID==g_ota_flg_id)
	{
	    OTA_PRINTF("OTA: cre_flg g_ota_flg_id failed in %s\n",__FUNCTION__);
	    return FALSE;  
	}	

	t_ctsk.task = (FP)ota_task;
	t_ctsk.stksz = 0x1000;
	t_ctsk.quantum = 10;
	t_ctsk.itskpri = OSAL_PRI_NORMAL;
	t_ctsk.name[0] = 'O';
	t_ctsk.name[1] = 'T';
	t_ctsk.name[2] = 'A';

	g_ota_tsk_id = osal_task_create(&t_ctsk);	
	if (g_ota_tsk_id == OSAL_INVALID_ID)
	{
	    OTA_PRINTF("OTA: cre_tsk g_ota_tsk_id failed in %s\n",__FUNCTION__);
	    return FALSE;
	}
	
	
    return TRUE;
}

BOOL ota_close()
{
    if(E_OK != OS_DelMessageBuffer(g_ota_mbf_id))
    {
        OTA_PRINTF("OTA: task message buffer free failed in %s\n",__FUNCTION__);
        return FALSE;
    }

    if(E_OK != OS_DelFlag(g_ota_flg_id))
    {
        OTA_PRINTF("OTA: flag delete failed in %s\n",__FUNCTION__);
        return FALSE;
    }
    
    if(E_OK != osal_task_delete(g_ota_tsk_id))
    {
        OTA_PRINTF("OTA: task delete failed in %s\n",__FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

#ifdef USB_UPG_DVBC
//usb upgrade.
USBUPG_FILENODE *usbupg_filelist=NULL;
UINT16 usbupg_node_num = 0;

BOOL usbupg_filelist_init()
{
	usbupg_filelist = (USBUPG_FILENODE *)MALLOC(sizeof(USBUPG_FILENODE) * USBUPG_MAX_FILENUM_SUPPORT);
	if(usbupg_filelist == NULL)
	{
		return RET_FAILURE;
	}
	else
	{
		usbupg_node_num = 0;//initlize node buffer
		MEMSET(usbupg_filelist,0x0, sizeof(USBUPG_FILENODE) * USBUPG_MAX_FILENUM_SUPPORT);
	}

	return RET_SUCCESS;
}

BOOL usbupg_filelist_free()
{
	if(usbupg_filelist != NULL)
	{
		usbupg_node_num = 0;//initlize node buffer
		free(usbupg_filelist);
		usbupg_filelist = NULL;
	}
    
	return RET_SUCCESS;
}

BOOL usbupg_create_filelist(UINT16* usbupg_nodenum)
{
	UINT16 str_len;
	int dir_fd;
	struct dirent *dent;
	char usbupg_fsbuff[sizeof(struct dirent) + FILE_NAME_SIZE];
	char upgfile_extends[2 + 1];

	sprintf(upgfile_extends,"%s","7Z");    
	dir_fd = fs_opendir("/c");
	if(dir_fd < 0)/*USB disk root dir open failure*/
	{
		*usbupg_nodenum = 0;
		return RET_FAILURE;
	}

	dent = ( struct dirent *) usbupg_fsbuff;
	usbupg_node_num = 0;//initlize node buffer
	MEMSET(usbupg_filelist,0x0, sizeof(USBUPG_FILENODE) * USBUPG_MAX_FILENUM_SUPPORT);
	while(1)
	{
		if (fs_readdir(dir_fd,dent) <= 0)
			break;/*Tail of the dir files*/
		
		if(S_ISDIR(dent->d_type))
		{
			continue;
		}
		else
		{
			str_len = STRLEN(dent->d_name);
			if(!strncasecmp((dent->d_name + str_len - 2),upgfile_extends,2))
			{
                struct stat buf;
                char _name[128];
				STRCPY((usbupg_filelist[usbupg_node_num].upg_file_name),dent->d_name);
                STRCPY(_name, "/c/");
                strcat(_name, dent->d_name);
                fs_stat(_name, &buf);
                usbupg_filelist[usbupg_node_num].size = buf.st_size;
				usbupg_node_num++;

				if(usbupg_node_num > USBUPG_MAX_FILENUM_SUPPORT)
					break;//check usbupg files if overflow
			}
		}
	}

	fs_closedir(dir_fd);
	*usbupg_nodenum = usbupg_node_num;
	if(usbupg_node_num == 0)
		return RET_FAILURE;
	else
		return RET_SUCCESS;
}

BOOL usbupg_get_filenode(PUSBUPG_FILENODE usbupg_node,UINT16 usbupg_nodeidx)
{
	if(usbupg_node_num == 0)
	{
		return RET_FAILURE;
	}
	else
	{
		MEMCPY(usbupg_node,(usbupg_filelist+usbupg_nodeidx),sizeof(USBUPG_FILENODE));
		return RET_SUCCESS;
	}
}

static BOOL usbupg_readfile(UINT16 file_idx,void *file_buffer,UINT32 buffer_size)
{
	char dir_str[USBUPG_FILE_NAME_LENGTH];
	FILE *fp_handle;
	
	if((file_buffer == NULL) || (buffer_size < usbupg_filelist[file_idx].size))
	{
		return RET_FAILURE;
	}
	
	sprintf(dir_str,"/c/%s",usbupg_filelist[file_idx].upg_file_name);	
	fp_handle = fopen(dir_str,"rb");
	if(fp_handle == NULL)
		return RET_FAILURE;
	fseek(fp_handle,0,SEEK_SET);
	fread(file_buffer,sizeof(char),usbupg_filelist[file_idx].size,fp_handle);
	fclose(fp_handle);

	return RET_SUCCESS;
}

BOOL usbupg_start(UINT16 file_idx,UPG_TYPE type)
{
    BOOL ret = RET_SUCCESS;
    
    if(usbupg_readfile(file_idx,g_ota_ram1_addr,g_ota_ram1_len)!=RET_SUCCESS)
    {
        OTA_PRINTF("usbupg read file failed in %s\n",__FUNCTION__);
        return RET_FAILURE;
    }

    //un7zip and check crc
    if(ota_un7z_check_crc()==TRUE)
    {
        
    }
    else
    {
        return RET_FAILURE;
    }

    if((progress_disp!=NULL)&&(progress_disp(VERSION_INFO,(UINT32)g_ota_ram2_addr)!=0))
    {
        return RET_FAILURE;
    }
    g_ota_size = usbupg_filelist[file_idx].size;
    //burn flash
    if(ota_cmd_start_burnflash(type) == TRUE)
        ret = RET_SUCCESS;
    else
        ret = RET_FAILURE;

    return ret;
}
//usb upgrad end.
#endif //USB_UPG_DVBC

#endif

