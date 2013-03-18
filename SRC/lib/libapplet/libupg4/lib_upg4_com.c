#include <basic_types.h>
#include <mediatypes.h>

#include <sys_config.h>

#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libc/alloc.h>
#include <api/libchunk/chunk.h>
#include <bus/flash/flash.h>
//#include <api/libota/lib_ota_c.h>

#include "lib_upg4_com.h"

void upg4_update_state(UINT32 type, UINT32 code);

STB_INFO_DATA g_upg_part_info;
UPG_CONFIG g_upg_cfg;
STB_CHUNK_INFO g_upg_chunk_image[STB_MAX_CHUNK_NUM];

UINT8 g_upg_burn_order[STB_MAX_PART_NUM]; // config by APP, the fixed order for part when upgrade, default is reverse-order: such as 6-5-4-3-2-1-0
UINT8 g_upg_burn_array[STB_MAX_PART_NUM]; // ordered by UPG4 according to g_upg_burn_order and upgrade data! 0xFF is default
	
static int g_upg_err_domain = -1;
static int g_upg_err_type = 0;
static int g_upg_state = UPG4_STATE_SUCCESS; 

#ifdef NAND_FLASH_SUPPORT
struct ali_nand_device *g_upg_nflash_dev = NULL;
#endif

struct sto_device *g_upg_flash_dev = NULL;
struct pan_device *g_upg_pan_dev = NULL;

UINT32 g_upg_sw_size;
UINT32 g_upg_size;

UINT32 g_bootloader_id = 0x23010010;
UINT32 g_stbinfo_id = 0;

BOOL _really_upgrade_bootloader = FALSE;

static UINT32 g_restore_chunk_id[32];
static UINT32 g_restore_chunk_num=0;

void upg_reg_bootloader_id(UINT32 id)
{
	g_bootloader_id = id;

	chunk_reg_bootloader_id(id);
}

void upg_reg_stbinfo_id(UINT32 id)
{
	g_stbinfo_id = id;
}

void upg_reg_restore_id(UINT32 id)
{
	g_restore_chunk_id[g_restore_chunk_num++] = id;
}

void _upg_set_err_type(int type)
{
	g_upg_err_type = type;
}

void _upg_set_domain(int part_id)
{
	g_upg_err_domain = part_id;
}

void _upg_set_upg_state(int state)
{
	g_upg_state = state;
}

void upg4_get_err_string(char *string)
{
	if(g_upg_state == UPG4_STATE_SUCCESS)
	{
		STRCPY(string, "U100");
	}
	else if(g_upg_state == UPG4_STATE_FAIL)
	{
		sprintf(string, "Er%01X%01X", g_upg_err_domain, g_upg_err_type);
	}
	else
	{
	}
}

BOOL upg_un7z()
{
	UINT8* block_addr;
	UINT32 check_size,check_crc,offset,code_size;
    
	UPG4_DEBUG("Unzip code...\n");
	if(un7zip(g_upg_cfg.compressed_addr, g_upg_cfg.uncompressed_addr, g_upg_cfg.swap_addr) != 0)
	{
		UPG4_DEBUG("ERROR: un7zip failed!\n");
		return FALSE;		
	}
	g_upg_size = *((UINT32*)g_upg_cfg.swap_addr);
	if((UINT32)g_upg_size > g_upg_cfg.uncompressed_len)
	{
		UPG4_DEBUG("ERROR: unzip_buf exceed!\n");
		return FALSE;			
	}

    return TRUE;
}

BOOL mem_cmp(UINT8* src,UINT8* dst,UINT32 size)
{
	UINT32 i;
	for(i=0;i<size;i++)
		if(*src++ != *dst++) return FALSE;
	return TRUE;
}

int upg_get_norflash_data(UINT8 *part_addr, UINT32 part_len, UINT32 disp_min,UINT32 disp_max)
{
	UINT32 i;
	UINT32 buff_size,len;
	UINT32 offset;
	UINT8* buffer;

	upg4_update_state(UPG_PROCESS_NUM, disp_min);
	
	buff_size = 0x10000;
	buffer = part_addr;
	offset = 0;

	while(offset < part_len)
	{
		len = (offset +buff_size) <= part_len? (buff_size) : (part_len-offset);
		sto_get_data(g_upg_flash_dev, buffer, offset, len);
		offset += len;
		buffer += len;
		osal_task_sleep(50);

		upg4_update_state(UPG_PROCESS_NUM, ((UINT32)buffer-(UINT32)part_addr)*(disp_max-disp_min)/part_len+disp_min);
	}

	return 0;
}

int upg_burn_norflash(STB_PART *part, UINT8 *image_part_addr, UINT32 image_part_len, UINT32 disp_min,UINT32 disp_max)
{
	int _ret=-1, ret = -1, i = 0;
	UINT32 f_chunk_addr;	//relative address
	UINT8 *m_pointer = 0;
	UINT32 id,offset,len;
	UINT32 sector_no;
	UINT32 temp_start = 0,temp_end = 0;
	STB_PART _part;
	STB_CHUNK_INFO image_chunk;
	UINT32 f_pointer = 0, data_off;
	STB_INFO_DATA *stb_info = stb_info_data_get();

	upg4_update_state(UPG_PROCESS_NUM, disp_min);

	for(i=0; i<part->chunk_num; i++)
	{
		_ret = stb_info_data_get_chunk(g_upg_chunk_image, part->chunk_list[i], &image_chunk);
		if(_ret == -1)
		{
			ret = -1;
			UPG4_DEBUG("%s:%d: can not get image chunk [%d]\n", __FUNCTION__, __LINE__, part->chunk_list[i]);
			goto RETURN;
		}

		if(image_chunk.upg_flag == 0)
			continue;

		f_pointer = image_chunk.addr;
		if(image_chunk.upg_flag == 3) // only update chunk header!
		{
			len = CHUNK_HEADER_SIZE;
		}
		else
		{
			len = image_chunk.len;
		}
		temp_end = f_pointer + len; 
		
		/* align burn start addr */
		sector_no = flash_sector_align((UINT32)f_pointer);
		if (sector_no >= flash_sectors_val)
		{
			ret = -1;
			UPG4_DEBUG("Error: Address greater than flash size. (Flash ID: %02X)\n", flash_id_val);
			goto RETURN;
		}
		temp_start = flash_sector_start(sector_no);
		
		if(temp_start < f_pointer)
		{
			f_pointer = temp_start;
		}		

		/* align burn end addr */
		sector_no = flash_sector_align((UINT32)temp_end);
		if (sector_no >= flash_sectors_val)
		{
			ret = -1;
			UPG4_DEBUG("Error: Address greater than flash size. (Flash ID: %02X)\n", flash_id_val);
			goto RETURN;
		}
		temp_start = flash_sector_start(sector_no);
		
		if(temp_start < temp_end)
		{
			temp_end = temp_start + flash_sector_size(sector_no);
		}		

#ifndef NAND_FLASH_SUPPORT
		/* set upgloader update flag */
		if(stb_info->param1 == image_chunk.id)
		{
			stb_info->upg_info.c_info.upg_needbackup = 1;
			stb_info_data_save();
		}
#endif
		
		UPG4_DEBUG("f_pointer = 0x%08x, temp_start = 0x%08x, temp_end = 0x%08x\n", f_pointer, temp_start, temp_end);
		
		temp_start = (UINT32)(image_part_addr + f_pointer);
		temp_end = (UINT32)image_part_addr + temp_end;
		
		sector_no = flash_sector_align((UINT32)f_pointer);
		
		while (temp_start < temp_end)
		{
			UPG4_DEBUG("f_pointer = 0x%08x, temp_start = 0x%08x, temp_end = 0x%08x\n", f_pointer, temp_start, temp_end);
			len = flash_sector_size(sector_no++);
			osal_task_sleep(200);
			switch (flash_verify((UINT32)f_pointer,(UINT8*)temp_start, len))
			{
				case 2:	/*need erase flash first*/
					osal_task_sleep(200);
					UPG4_DEBUG("sector need erase\n");
					flash_erase_sector((UINT32)f_pointer);
				case 1:
					osal_task_sleep(200);
					flash_copy((UINT32)f_pointer,(UINT8*)temp_start, len);
					break;
				case 0:
					osal_task_sleep(200);
					UPG4_DEBUG("no need to write this sector\n");
				default:
					break;	
	        	}

			f_pointer += len;
			temp_start += len;

			upg4_update_state(UPG_PROCESS_NUM, (temp_start-(UINT32)image_part_addr)*(disp_max-disp_min)/image_part_len+disp_min);
		}	
	}

	ret = 0;
	
RETURN:
	return ret;
}

int upg_burn_part(STB_PART *part, UINT32 f_addr, UINT8 *image_part_addr, UINT32 image_part_len, UINT32 disp_min,UINT32 disp_max)
{
	int ret = -1, i = 0;
	UINT32 f_chunk_addr;	//relative address
	UINT8 *m_pointer = 0;
	UINT32 id,offset,len;
	UINT32 sector_no;
	UINT32 temp_start = 0,temp_end = 0;
	STB_PART _part;
	STB_CHUNK_INFO _chunk;
	UINT32 f_pointer = 0, data_off;
	UINT8 burn_bootloader = 0, percent = 0;
	STB_INFO_DATA *stb_info = stb_info_data_get();

	upg4_update_state(UPG_PROCESS_NUM, disp_min);

	stb_info_data_get_info_by_id(part->id, &_part);
	UPG4_DEBUG("now burn part %d, addr = 0x%x, len = 0x%x, part_size = 0x%x\n", part->id, image_part_addr, image_part_len, _part.size);
	if(part->id == 0) // Nor flash
	{
		f_pointer = f_addr;
		
		/* align to sector */
		temp_end = f_addr;
		sector_no = flash_sector_align((UINT32)temp_end);
		temp_start = flash_sector_start(sector_no);
		
		if(temp_start < temp_end)
		{
			len = temp_start + flash_sector_size(sector_no) - temp_end;
			f_pointer += len;
		}

		if(f_pointer == 0)
		{
			UPG4_DEBUG("Now start to burn bootloader!\n");
			pan_display(g_upg_pan_dev, "Ubld", 4);
			burn_bootloader = 1;
		}

		data_off = f_pointer;

#ifndef NAND_FLASH_SUPPORT
		/* set upgloader update flag */
		stb_info->upg_info.c_info.upg_needbackup = 1;
		stb_info_data_save();
#endif		

		UPG4_DEBUG("f_pointer = 0x%08x, temp_start = 0x%08x, temp_end = 0x%08x\n", f_pointer, temp_start, temp_end);
		
		temp_start = (UINT32)(image_part_addr + data_off);
		temp_end = (UINT32)image_part_addr + image_part_len;
		
		sector_no = flash_sector_align((UINT32)f_pointer);
		
		while (temp_start < temp_end)
		{
			len = flash_sector_size(sector_no++);
			osal_task_sleep(200);
			switch (flash_verify((UINT32)f_pointer,(UINT8*)temp_start, len))
			{
				case 2:	/*need erase flash first*/
					osal_task_sleep(200);
					UPG4_DEBUG("sector need erase\n");
					flash_erase_sector((UINT32)f_pointer);
				case 1:
					osal_task_sleep(200);
					flash_copy((UINT32)f_pointer,(UINT8*)temp_start, len);
					break;
				case 0:
					osal_task_sleep(200);
					UPG4_DEBUG("no need to write this sector\n");
				default:
					break;	
	        }

			f_pointer += len;
			temp_start += len;

			percent = (temp_start-(UINT32)image_part_addr)*(disp_max-disp_min)/image_part_len+disp_min;
			if((burn_bootloader == 0) || (percent >= 67))
				upg4_update_state(UPG_PROCESS_NUM, percent);
		}	
	}
#ifdef NAND_FLASH_SUPPORT
	else
	{	
		
		// for OTA upgrade, need remember upgloader now be burned
		if(!STRCMP(_part.name, STB_PART_OTALDER))
		{
			if(stb_info->state == STB_INFO_DATA_STATE_PREBURN)
			{
				stb_info->upg_info.c_info.upg_needbackup = 1;
				stb_info_data_save();
				upg4_update_state(UPG_FLOW_NOTIFY, STB_UPG_STATE_UPGLOADER_UPGRADED);
			}
		}

		ret = NF_SavePartition(g_upg_nflash_dev, part->id, (void *)image_part_addr, image_part_len);
		if(ret != 0)
		{
			UPG4_DEBUG("save partition %d fail, ret = %d\n", part->id, ret);
			_upg_set_upg_state(UPG4_STATE_FAIL);
			_upg_set_err_type(UPG4_ERR_PART_WRITE_FAIL);
			ret = -1;
			goto RETURN;
		}
		upg4_update_state(UPG_PROCESS_NUM, disp_max);
	}
#endif
	ret = 0;
	
RETURN:
	return ret;
}

/* read chunk data to sdram from mirror, e.g. userdb, hdcp*/
// ret: -1 no restore, 1 success
static int __upg_restore_chunk(UINT32 chunk_id, UINT32 mirror_addr, UINT32 mirror_len,UINT32 image_addr,UINT32 image_len)
{
	int i = 0;
	UINT32 stb_chunk_id = 0, chunk_offset = 0;
	unsigned char *p_mirror, *p_image;
	UINT32 mirror_offset, image_offset;
	STB_INFO_DATA *stb_data = NULL;

	chunk_init((UINT32)mirror_addr,mirror_len);
	p_mirror = chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
	UPG4_DEBUG("chunk 0x%08x in mirror addr 0x%08x\n",chunk_id, p_mirror);
	if(p_mirror == 0)
	{
		stb_data = stb_info_data_get();
		if(NULL == stb_data)
		{
			UPG4_DEBUG("ERROR: %s, %d, cannot get stbdata\n",__FUNCTION__, __LINE__);
			return -1;
		}

		for(i = 0; i < stb_data->stb_chunk_num; i++)
		{
			stb_chunk_id = stb_data->chunk[i].id;
			if(stb_chunk_id != chunk_id)
			{
				chunk_offset += stb_data->chunk[i].len;
			}
			else
				break;
		}
		if(i == stb_data->stb_chunk_num)
		{
			UPG4_DEBUG("ERROR: %s, %d, cannot find chunk [0x%08x] in stbdata\n",__FUNCTION__, __LINE__, chunk_id);
			return -1;
		}

		if((0 == chunk_offset) ||(chunk_offset > mirror_len) || (chunk_offset != stb_data->chunk[i].addr) )
		{
			UPG4_DEBUG("ERROR: %s, %d, stbdata crashed, chunk [0x%08x] addr not match, chunk offset 0x%08x, addr 0x%08x\n",__FUNCTION__, __LINE__, chunk_id, chunk_offset, stb_data->chunk[i].addr);
			return -1;
		}

		p_mirror = (unsigned char*)(mirror_addr + chunk_offset);

		UPG4_DEBUG("%s, %d, mirror pointer [0x%08x], chunk offset [0x%08x]\n",__FUNCTION__, __LINE__, p_mirror, chunk_offset);
	}
	
	chunk_init((UINT32)image_addr,image_len);
	p_image = chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
	if(p_image == 0)
	{
		UPG4_DEBUG("ERROR: Not find chunk 0x%08x in image\n",chunk_id);
		return -1;
	}
	
	mirror_offset = fetch_long(p_mirror+CHUNK_OFFSET);
	if(mirror_offset == 0)
		mirror_offset = fetch_long(p_mirror+CHUNK_LENGTH)+0x10;

	image_offset = fetch_long(p_image+CHUNK_OFFSET);
	if(image_offset == 0)
		image_offset = fetch_long(p_image+CHUNK_LENGTH)+0x10;

	if(mirror_offset != image_offset)
	{
		UPG4_DEBUG("ERROR: mirror_offset != image_offset\n");
		return -1;
	}

	MEMCPY(p_image, p_mirror, image_offset);

	return 1;
}

// ret: 0 upgrade, -1 error, 1 no need upgrade, goto next part
static int __upg_reorg_part(STB_PART *upg_part, STB_PART *image, UINT8 *upgrade_data, UINT32 upgrade_len, UINT8 *mirror_data, UINT32 mirror_len, UINT8 *image_data, UINT32 *image_len, UINT32 disp_min, UINT32 disp_max)
{
	int ret = -1, _ret = -1;
	STB_PART image_part, mirror_part;
	UINT32 chunk_num = 0;
	BOOL from_ugrade = FALSE, need_upgrade=FALSE;
	UINT32  i = 0, j = 0, len = 0;
	STB_CHUNK_INFO upg_chunk, mirror_chunk, image_chunk;

#ifdef NAND_FLASH_SUPPORT	
	struct ali_nand_info _nflash_info;

	NF_GetNandInfo(g_upg_nflash_dev, &_nflash_info);
#endif

	if(upg_part == NULL)
	{
		ret = -1;
		UPG4_DEBUG("%s:%d: part is NULL, error!\n", __FUNCTION__, __LINE__);
		goto RETURN;
	}

	if(upg_part->chunk_num == 0)
	{
		ret = -1;
		UPG4_DEBUG("%s:%d: part chunk num=0, error!\n", __FUNCTION__, __LINE__);
		goto RETURN;
	}

	*image_len = 0;
	stb_info_data_get_info_by_id(upg_part->id, &mirror_part);
	UPG4_DEBUG("now reorg part %d, upgade_chunk_num = %d, mirror_chunk_num = %d\n", upg_part->id, upg_part->chunk_num, mirror_part.chunk_num);

	chunk_num = upg_part->chunk_num;	
	MEMCPY(&image_part, &mirror_part, sizeof(STB_PART));
	
	for(i=0; i<mirror_part.chunk_num; i++)
	{	
		_ret = stb_info_data_get_chunk(NULL, mirror_part.chunk_list[i], &mirror_chunk);
		if(_ret == -1)
		{
			ret = -1;
			UPG4_DEBUG("%s:%d: can not get mirror chunk [%d]\n", __FUNCTION__, __LINE__, mirror_part.chunk_list[i]);
			goto RETURN;
		}

		from_ugrade = FALSE;
		
		// the chunk within upgrade chunk list?
		for(j=0; j<upg_part->chunk_num; j++)
		{
			_ret = stb_info_data_get_chunk(g_upg_part_info.chunk, upg_part->chunk_list[j], &upg_chunk);
			if(_ret == -1)
			{
				ret = -1;
				UPG4_DEBUG("%s:%d: can not get image chunk [%d]\n", __FUNCTION__, __LINE__, upg_part->chunk_list[j]);
				goto RETURN;
			}
			
			if(mirror_chunk.id == upg_chunk.id)
			{
				if(upg_chunk.upg_flag == 0)
					from_ugrade = FALSE;
				else
					from_ugrade = TRUE;

				break;
			}
		}

		if(from_ugrade)
		{/*copy upg chunk data to image buf*/
			if(upg_part->id == 0) // for Nor flash disable chunk enlarge
			{
				if(mirror_chunk.len != upg_chunk.len)
				{
					ret = -1;
					UPG4_DEBUG("%s:%d: by chunk can't enlarge chunk [id 0x%x, want 0x%x -> 0x%x]\n", __FUNCTION__, __LINE__, upg_chunk.id, mirror_chunk.len, upg_chunk.len);
					goto RETURN;
				}
			}
			
			MEMCPY(&image_chunk, &upg_chunk, sizeof(STB_CHUNK_INFO));
			len = upg_chunk.len;
			if(upg_chunk.upg_flag == 3) // update chunk header!
			{
				MEMCPY((UINT8*)image_data, (UINT8*)(upgrade_data + upg_chunk.addr), CHUNK_HEADER_SIZE);
			}
			else
			{
				MEMCPY((UINT8*)image_data, (UINT8*)(upgrade_data + upg_chunk.addr), len);
			}
			need_upgrade = TRUE;
		}
		else
		{/*copy mirror chunk data to image buf*/
			MEMCPY(&image_chunk, &mirror_chunk, sizeof(STB_CHUNK_INFO));
			len = mirror_chunk.len;
			MEMCPY((UINT8*)image_data, (UINT8*)(mirror_data + mirror_chunk.addr), len);
		}
		
		UPG4_DEBUG("\tpart %d, chunk 0x%08x from %s, ver = 0x%s, len = 0x%x\n", 
			image_part.id, image_chunk.id, \
			(from_ugrade ? "UPGRADE" : "FLASH"), image_chunk.ver, len);

		image_chunk.addr = *image_len;
		//image_chunk.upg_flag = 0;

		_ret = stb_info_data_update_chunk(g_upg_chunk_image, &image_chunk);
		if(_ret == -1)
		{
			UPG4_DEBUG("%s:%d: update chunk 0x%x fail!\n", __FUNCTION__, __LINE__, image_chunk.id);
		}

		/*point to next chunk*/
		image_data += image_chunk.len;
		*image_len += image_chunk.len;
		
		upg4_update_state(UPG_PROCESS_NUM, (*image_len)*(disp_max-disp_min)/mirror_part.size+disp_min);			
	}
		
	image_part.size = *image_len;

	// flash size check!
	if(upg_part->id == 0)
	{
		if(image_part.size > (UINT32)stb_info_data_get_nor_size())
		{
			UPG4_DEBUG("fail: Nor flash reorg image size [0x%x] overflow [nor flash size = 0x%x]!\n", image_part.size, stb_info_data_get_nor_size());
			_upg_set_upg_state(UPG4_STATE_FAIL);
			_upg_set_err_type(UPG4_ERR_PART_SIZE_OVERFLOW);
			ret = -1;
			goto RETURN;
		}
	}
#ifdef NAND_FLASH_SUPPORT	
	else
	{
		UINT32 nand_part_size = _nflash_info.partInfo[upg_part->id].len - _nflash_info.blocksize;
		image_part.size = ((image_part.size + _nflash_info.pagesize) / _nflash_info.pagesize) * (_nflash_info.pagesize); // pagesize align for nand load!
		if(nand_part_size < image_part.size)
		{
			UPG4_DEBUG("fail: Nand flash reorg image size [0x%x] overflow [nand part size = 0x%x]!\n", image_part.size, nand_part_size);
			_upg_set_upg_state(UPG4_STATE_FAIL);
			_upg_set_err_type(UPG4_ERR_PART_SIZE_OVERFLOW);
			ret = -1;
			goto RETURN;
		}
	}
#endif

	if(image != NULL)
	{
		MEMCPY(image, &image_part, sizeof(STB_PART));
	}

	upg_part->size = image_part.size;
	ret = (need_upgrade ? 0 : 1);
	
RETURN:		
	return ret;
}

static int __upg_reorg_part_primitive(STB_PART *upg_part, STB_PART *image, UINT8 *upgrade_data, UINT32 upgrade_len, UINT8 *mirror_data, UINT32 mirror_len, UINT8 *image_data, UINT32 *image_len, UINT32 disp_min, UINT32 disp_max)
{
	int ret = -1, _ret = -1;
	STB_PART image_part, mirror_part;
	UINT32 chunk_num = 0;
	BOOL from_ugrade = FALSE, need_upgrade=FALSE;
	UINT32  i = 0, j = 0, len = 0;
	STB_CHUNK_INFO upg_chunk, mirror_chunk, image_chunk;
	CHUNK_HEADER chunk_head;

#ifdef NAND_FLASH_SUPPORT
	struct ali_nand_info _nflash_info;

	NF_GetNandInfo(g_upg_nflash_dev, &_nflash_info);
#endif

	if(upg_part == NULL)
	{
		ret = -1;
		UPG4_DEBUG("%s:%d: part is NULL, error!\n", __FUNCTION__, __LINE__);
		goto RETURN;
	}

	if(upg_part->chunk_num == 0)
	{
		ret = -1;
		UPG4_DEBUG("%s:%d: part chunk num=0, error!\n", __FUNCTION__, __LINE__);
		goto RETURN;
	}

	*image_len = 0;
	stb_info_data_get_info_by_id(upg_part->id, &mirror_part);
	UPG4_DEBUG("now reorg part %d, upgade_chunk_num = %d, mirror_chunk_num = %d\n", upg_part->id, upg_part->chunk_num, mirror_part.chunk_num);

	chunk_num = upg_part->chunk_num;	
	MEMCPY(&image_part, &mirror_part, sizeof(STB_PART));
	
	for(i=0; i<mirror_part.chunk_num; i++)
	{	
		_ret = stb_info_data_get_chunk(NULL, mirror_part.chunk_list[i], &mirror_chunk);
		if(_ret == -1)
		{
			ret = -1;
			UPG4_DEBUG("%s:%d: can not get mirror chunk [%d]\n", __FUNCTION__, __LINE__, mirror_part.chunk_list[i]);
			goto RETURN;
		}

		from_ugrade = FALSE;
		
		// the chunk within upgrade chunk list?
		for(j=0; j<upg_part->chunk_num; j++)
		{
			_ret = stb_info_data_get_chunk(g_upg_part_info.chunk, upg_part->chunk_list[j], &upg_chunk);
			if(_ret == -1)
			{
				ret = -1;
				UPG4_DEBUG("%s:%d: can not get image chunk [%d]\n", __FUNCTION__, __LINE__, upg_part->chunk_list[j]);
				goto RETURN;
			}
			
			if(mirror_chunk.id == upg_chunk.id)
			{
				if(upg_chunk.upg_flag == 0)
					from_ugrade = FALSE;
				else
				{
					if(mirror_chunk.len > upg_chunk.len+CHUNK_HEADER_SIZE)
						from_ugrade = TRUE;
					else
					{
						ret = -1;
						UPG4_DEBUG("%s:%d: primitive chunk[0x%08x] data too large > [0x%08x]\n", __FUNCTION__, __LINE__, mirror_chunk.id, mirror_chunk.len);
						goto RETURN;
					}
				}

				break;
			}
		}

		if(from_ugrade)
		{/*copy upg chunk data to image buf*/
			MEMCPY(&image_chunk, &mirror_chunk, sizeof(STB_CHUNK_INFO));
			image_chunk.upg_flag = upg_chunk.upg_flag;

			get_chunk_header_by_addr(mirror_data + mirror_chunk.addr, &chunk_head);
			
			MEMCPY((UINT8*)image_data, (UINT8*)(mirror_data + mirror_chunk.addr), CHUNK_HEADER_SIZE);
			MEMCPY((UINT8*)image_data+CHUNK_HEADER_SIZE, (UINT8*)(upgrade_data + upg_chunk.addr), upg_chunk.len);

			if(chunk_head.crc != NO_CRC)
			{
				chunk_head.len = upg_chunk.len + 0x70;
				chunk_head.crc = MG_Table_Driven_CRC(0xFFFFFFFF, image_data + 16, chunk_head.len);	
				store_long((UINT8*)image_data+CHUNK_LENGTH, chunk_head.len);
				store_long((UINT8*)image_data+CHUNK_CRC, chunk_head.crc);
				len = chunk_head.len;
			}
			
			need_upgrade = TRUE;
		}
		else
		{/*copy mirror chunk data to image buf*/
			MEMCPY(&image_chunk, &mirror_chunk, sizeof(STB_CHUNK_INFO));
			len = mirror_chunk.len;
			MEMCPY((UINT8*)image_data, (UINT8*)(mirror_data + mirror_chunk.addr), len);
		}
		
		UPG4_DEBUG("\tpart %d, chunk 0x%08x from %s, ver = 0x%s, len = 0x%x\n", 
			image_part.id, image_chunk.id, \
			(from_ugrade ? "UPGRADE" : "FLASH"), image_chunk.ver, len);

		image_chunk.addr = *image_len;
		//image_chunk.upg_flag = 0;

		_ret = stb_info_data_update_chunk(g_upg_chunk_image, &image_chunk);
		if(_ret == -1)
		{
			UPG4_DEBUG("%s:%d: add chunk 0x%x fail!\n", __FUNCTION__, __LINE__, image_chunk.id);
		}

		/*point to next chunk*/
		image_data += image_chunk.len;
		*image_len += image_chunk.len;
		
		upg4_update_state(UPG_PROCESS_NUM, (*image_len)*(disp_max-disp_min)/mirror_part.size+disp_min);			
	}
		
	image_part.size = *image_len;

	// flash size check!
	if(upg_part->id == 0)
	{
		if(image_part.size > (UINT32)stb_info_data_get_nor_size())
		{
			UPG4_DEBUG("fail: Nor flash reorg image size [0x%x] overflow [nor flash size = 0x%x]!\n", image_part.size, stb_info_data_get_nor_size());
			_upg_set_upg_state(UPG4_STATE_FAIL);
			_upg_set_err_type(UPG4_ERR_PART_UPGBUFFER_OVERFLOW);
			ret = -1;
			goto RETURN;
		}
	}
#ifdef NAND_FLASH_SUPPORT	
	else
	{
		UINT32 nand_part_size = _nflash_info.partInfo[upg_part->id].len - _nflash_info.blocksize;
		image_part.size = ((image_part.size + _nflash_info.pagesize) / _nflash_info.pagesize) * (_nflash_info.pagesize); // pagesize align for nand load!
		if(nand_part_size < image_part.size)
		{
			UPG4_DEBUG("fail: Nand flash reorg image size [0x%x] overflow [nand part size = 0x%x]!\n", image_part.size, nand_part_size);
			_upg_set_upg_state(UPG4_STATE_FAIL);
			_upg_set_err_type(UPG4_ERR_PART_SIZE_OVERFLOW);
			ret = -1;
			goto RETURN;
		}
	}
#endif

	if(image != NULL)
	{
		MEMCPY(image, &image_part, sizeof(STB_PART));
	}

	upg_part->size = image_part.size;
	ret = (need_upgrade ? 0 : 1);
	
RETURN:		
	return ret;
}

// -1 err, 0 upg, 1 ignore, 2 ignore but need update part_info
static int __upg_check_part(STB_PART *upg_part, UINT8 *upgrade_data, UINT32 upgrade_len, UINT8 *mirror_data, UINT32 mirror_len, BOOL unlimit_flag)
{
	int ret = 1, _ret = -1;
	UINT32 i = 0, j = 0;
	unsigned char *upgrade_pointer = NULL, *mirror_pointer = NULL;
	STB_CHUNK_INFO image_chunk, mirror_chunk, upg_chunk;
	STB_PART mirror_part;
	INT32 cmd_cb = -1;
	BOOL _continue = FALSE;
	BOOL need_upgrade = FALSE;
	UINT32 part_addr = 0;
	UINT8 ver[17];

	if(upg_part == NULL)
	{
		ret = -1;
		UPG4_DEBUG("%s:%d: part is NULL, error!\n", __FUNCTION__, __LINE__);
		goto RETURN;
	}

	if(upg_part->chunk_num == 0)
	{
		ret = 1;
		UPG4_DEBUG("%s:%d: part chunk num=0, error!\n", __FUNCTION__, __LINE__);
		goto RETURN;
	}

	chunk_init((UINT32)mirror_data,mirror_len);
	stb_info_data_get_info_by_id(upg_part->id, &mirror_part);

	// now mode0 nor flash will use chunk mode to do upgrade, so need check the upg_flag be 2 to force upgrade!
	if(g_upg_part_info.upg_mode == UPG_ALLCODE_NORMAL_MODE)
	{
		for(i=0; i<upg_part->chunk_num; i++)
		{
			_ret = stb_info_data_get_chunk(g_upg_part_info.chunk, upg_part->chunk_list[i], &upg_chunk);
			if(_ret == -1) 
			{
				UPG4_DEBUG("%s:%d: get chunk 0x%08x from upg part %d fail, now continue to ignore it!\n", __FUNCTION__, __LINE__, g_restore_chunk_id[i], upg_part->id);
				continue;
			}

			UPG4_DEBUG("now clear mode0 private chunk 0x%x upg_flag!\n", upg_chunk.id);
			upg_chunk.upg_flag = 2;
			_ret = stb_info_data_update_chunk(g_upg_part_info.chunk, &upg_chunk);
			if(_ret == -1)
			{
				UPG4_DEBUG("%s:%d: update chunk [idx=%d] fail, now continue to ignore it!\n", __FUNCTION__, __LINE__, upg_part->chunk_list[i]);
				continue;
			}

		}
	}
	// need clear private chunk's upg_flag, to avoid its data be cleared when its header missing!
	if((unlimit_flag == FALSE) && (upg_part->id == 0))
	{						
		for(i=0; i<g_restore_chunk_num; i++)
		{
			_ret = stb_info_data_get_chunk_by_id(g_upg_part_info.chunk, g_restore_chunk_id[i], &upg_chunk);
			if(_ret == -1) 
			{
				UPG4_DEBUG("%s:%d: get chunk 0x%08x from upg part %d fail, now continue to ignore it!\n", __FUNCTION__, __LINE__, g_restore_chunk_id[i], upg_part->id);
				continue;
			}

			UPG4_DEBUG("now clear mode0 private chunk 0x%x upg_flag!\n", upg_chunk.id);
			upg_chunk.upg_flag = 0;
			_ret = stb_info_data_update_chunk(g_upg_part_info.chunk, &upg_chunk);
			if(_ret == -1)
			{
				UPG4_DEBUG("%s:%d: update chunk [idx=%d] fail, now continue to ignore it!\n", __FUNCTION__, __LINE__, upg_part->chunk_list[i]);
				continue;
			}

		}
	}

	// check whether need restore chunk linkage, mirror nor flash chunk maybe destroyed by power cut while upgrade!
	for(i=0; i<mirror_part.chunk_num; i++)		
	{
		_ret = stb_info_data_get_chunk(NULL, mirror_part.chunk_list[i], &mirror_chunk);
		if(_ret == -1) 
		{
			ret = -1;
			UPG4_DEBUG("%s:%d: get chunk idx %d of part %d fail\n", __FUNCTION__, __LINE__, mirror_part.chunk_list[i], upg_part->id);
			goto RETURN;
		}
		
		mirror_pointer = mirror_data + mirror_chunk.addr;

		if(chunk_check_single_crc(mirror_pointer) != 0)
		{
			_ret = stb_info_data_get_chunk_by_id(g_upg_part_info.chunk, mirror_chunk.id, &upg_chunk);
			if(_ret == -1) 
			{
				ret = -1;
				UPG4_DEBUG("%s:%d: get chunk 0x%08x from upg part %d fail!\n", __FUNCTION__, __LINE__, mirror_chunk.id, upg_part->id);
				goto RETURN;
			}

			upg_chunk.upg_flag += 3; // use +3 for such special case, need upgrade this chunk header!
			_ret = stb_info_data_update_chunk(g_upg_part_info.chunk, &upg_chunk);
			if(_ret == -1)
			{
				ret = -1;
				UPG4_DEBUG("%s:%d: update chunk [id=0x%08x] fail!\n", __FUNCTION__, __LINE__, upg_chunk.id);
				goto RETURN;
			}
		}
		
	}
		
	for(i=0; i<upg_part->chunk_num; i++)
	{
		_ret = stb_info_data_get_chunk(g_upg_part_info.chunk, upg_part->chunk_list[i], &upg_chunk);
		if(_ret == -1)
		{
			ret = -1;
			UPG4_DEBUG("%s:%d: have no chunk [idx=%d]\n", __FUNCTION__, __LINE__, upg_part->chunk_list[i]);
			goto RETURN;
		}

		if(i == 0)
			part_addr = upg_chunk.addr + (UINT32)upgrade_data;
		
		if((upg_chunk.upg_flag == 1) || (upg_chunk.upg_flag == 2))
		{
			if(unlimit_flag == FALSE)
			{/*skip bootloader, private data(hdcp, cadata, userdb¡­)*/							
				// check whether under restore array
				for(j=0; j<g_restore_chunk_num; j++)
				{
					if(g_restore_chunk_id[j] == upg_chunk.id)
					{
						upg_chunk.upg_flag = 0;
						_ret = stb_info_data_update_chunk(g_upg_part_info.chunk, &upg_chunk);
						if(_ret == -1)
						{
							ret = -1;
							UPG4_DEBUG("%s:%d: update chunk [idx=%d] fail!\n", __FUNCTION__, __LINE__, upg_part->chunk_list[i]);
							goto RETURN;
						}

						UPG4_DEBUG("\tnot upgrade part %d, chunk id = 0x%08x, it's in restore array\n", upg_part->id, upg_chunk.id);
						break;
					}
				}
				if(j < g_restore_chunk_num)
					continue;
			}

			if(upg_chunk.id == g_stbinfo_id) // stbinfo chunk must be upg_flag = 1 to force execute this code!! (ota/usb scripter will do this job)
			{
				upg_chunk.upg_flag = 0;
				_ret = stb_info_data_update_chunk(g_upg_part_info.chunk, &upg_chunk);
				if(_ret == -1)
				{
					ret = -1;
					UPG4_DEBUG("%s:%d: update chunk [idx=%d] fail!\n", __FUNCTION__, __LINE__, upg_part->chunk_list[i]);
					goto RETURN;
				}
				continue;
			}
			else if((g_upg_part_info.upg_mode != UPG_ALLCODE_NORMAL_MODE) && (upg_chunk.upg_flag == 1 && g_upg_cfg.upg_version_cb != NULL)) // now mode0(Nor flash)/1/3 all will reach this branch, but mode0(Nor flash) should ignore chunk version!
			{// check header info callback result
				upgrade_pointer = upgrade_data + upg_chunk.addr;

				_ret = stb_info_data_get_chunk_by_id(NULL, upg_chunk.id, &mirror_chunk);
				if(_ret == -1) //seems want to add new chunk
				{
					ret = -1;
					UPG4_DEBUG("%s:%d: chunk-mode seems want to add new chunk 0x%08x to part %d!\n", __FUNCTION__, __LINE__, upg_chunk.id, upg_part->id);
					goto RETURN;
				}

				mirror_pointer = mirror_data + mirror_chunk.addr;

				if(chunk_check_single_crc(mirror_pointer) == 0)
				{
					cmd_cb = g_upg_cfg.upg_version_cb((UINT32)upgrade_pointer,(UINT32)mirror_pointer);
					if(cmd_cb == 0)
					{/*not upgrade this chunk*/

						// check whether stb_info_data part_info chunk not be upgraded!
						MEMSET(ver, 0, 17);
						MEMCPY(ver, mirror_pointer+CHUNK_VERSION, 16);
						
						if(STRCMP(mirror_chunk.ver, ver)!=0)
						{
							ret = 0;
							continue;
						}
						
						upg_chunk.upg_flag = 0;
						_ret = stb_info_data_update_chunk(g_upg_part_info.chunk, &upg_chunk);
						if(_ret == -1)
						{
							ret = -1;
							UPG4_DEBUG("%s:%d: update chunk [idx=%d] fail!\n", __FUNCTION__, __LINE__, upg_part->chunk_list[i]);
							goto RETURN;
						}
						UPG4_DEBUG("\tnot upgrade part %d, chunk id = 0x%08x, it's disabled by chunk header check\n", upg_part->id, upg_chunk.id);

						continue;
					}
					else if(cmd_cb < 0)
					{/*error*/
						ret = -1;
						UPG4_DEBUG("%s:%d: callback ret = %d, error!\n", __FUNCTION__, __LINE__, cmd_cb);
						goto RETURN;
					}
				}
			}	
			
			ret = 0; // need upgrade
		}
		else if(upg_chunk.upg_flag >= 3) // upg_flag >= 3 MUST upgrade!
		{
			ret = 0;
		}
	}
	
	
RETURN:
	return ret;
}


int _upg_upgrade_part_by_chunk(STB_PART *upg_part, UINT8 *upgrade_data, UINT32 upgrade_len, BOOL unlimit_flag)
{
	int ret = -1, _ret = -1, i = 0, _chunk_idx = -1, _chunk_err = -1;
	UINT8 *m_pointer = 0;
	UINT32 id,offset,len;
	STB_PART mirror_part, image_part, backup_part;
	UINT8 *mirror_part_addr = (UINT8 *)g_upg_cfg.data_mirror_addr; // what is old
	UINT32 mirror_part_len = 0;
	UINT8 *image_part_addr = (UINT8 *)g_upg_cfg.data_image_addr; // what is real to upgrade, merged from upper 2
	UINT32 image_part_len = 0;
	STB_CHUNK_INFO image_chunk;
	STB_CHUNK_INFO _chunk;
	CHUNK_HEADER _chunk_header;
	UINT8 __head[CHUNK_HEADER_SIZE];

#ifdef NAND_FLASH_SUPPORT
	struct ali_nand_info _nflash_info;

	NF_GetNandInfo(g_upg_nflash_dev, &_nflash_info);
#endif

	UPG4_DEBUG("now upgrade part %d\n", upg_part->id);
	
	upg4_update_state(UPG_PROCESS_NUM, 5);
	upg4_update_state(UPG_STATUS_INFO, UPG_BACKUP_MIRROR_FLAG);

	_ret = stb_info_data_get_info_by_name(STB_PART_BACKUP, &backup_part);
	if(_ret == 0)
	{
		if(upg_part->id == backup_part.id)
		{
			UPG4_DEBUG("%s:%d: seems try to upgrade backup part, now ignore it!\n", __FUNCTION__, __LINE__);
			ret = 1;
			goto RETURN;
		}
	}

//STEP1: load mirror data
	stb_info_data_get_info_by_id(upg_part->id, &mirror_part);
	mirror_part_len = mirror_part.size;
	if(mirror_part.id == 0) // Nor flash
	{
		upg_reg_restore_id(STB_CHUNK_BOOTLOADER_ID);
		//upg_reg_restore_id(STB_CHUNK_STBINFO_ID);

		upg_get_norflash_data(mirror_part_addr, mirror_part_len, 7,19);
	}
#ifdef NAND_FLASH_SUPPORT	
	else
	{
		mirror_part_len = ((mirror_part_len + _nflash_info.pagesize) / _nflash_info.pagesize) * (_nflash_info.pagesize); // pagesize align for nand load!
		_ret = NF_LoadPartition(g_upg_nflash_dev, mirror_part.id, (void *)mirror_part_addr, mirror_part_len);
		if(_ret != 0)
		{
			UPG4_DEBUG("%s:%d: load partition %d fail, ret = %d\n", __FUNCTION__, __LINE__, mirror_part.id, _ret);
			_upg_set_upg_state(UPG4_STATE_FAIL);
			_upg_set_err_type(UPG4_ERR_PART_LOAD_FAIL);
			ret = -1;
			goto RETURN;
		}
	}
#endif

	upg4_update_state(UPG_PROCESS_NUM, 20);
	
//STEP3: restore to clear some key chunk upg_flag -- based on chunk header info
	_ret = __upg_check_part(upg_part, upgrade_data, upgrade_len, mirror_part_addr, mirror_part_len, unlimit_flag);
	if(_ret == 1) // no need upgrade
	{
		UPG4_DEBUG("%s:%d: seems part %d no need upgrade, continue to next\n", __FUNCTION__, __LINE__, upg_part->id);
		ret = 1;
		goto RETURN;
	}
	else if(_ret < 0)
	{
		UPG4_DEBUG("%s:%d: part %d [id = %d] reorg_data fail\n", __FUNCTION__, __LINE__, i, upg_part->id);
		_upg_set_upg_state(UPG4_STATE_FAIL);
		_upg_set_err_type(UPG4_ERR_PART_CHECK_IMAGE_FAIL);
		ret = -1;
		goto RETURN;
	}

	upg4_update_state(UPG_PROCESS_NUM, 30);
	
//STEP4: reorg part data, need update the part info for stb-info
	_ret = __upg_reorg_part(upg_part, &image_part, upgrade_data, upgrade_len, mirror_part_addr, mirror_part_len, image_part_addr, &image_part_len, 30, 45);
	if(_ret == 1) // no need upgrade
	{
		UPG4_DEBUG("%s:%d: seems part [id = %d] no need upgrade, continue to next\n", __FUNCTION__, __LINE__, upg_part->id);
		ret = 1;
		goto RETURN;
	}
	else if(_ret < 0)
	{
		UPG4_DEBUG("%s:%d: part [id = %d] reorg_data fail\n", __FUNCTION__, __LINE__, upg_part->id);
		_upg_set_upg_state(UPG4_STATE_FAIL);
		_upg_set_err_type(UPG4_ERR_PART_REORG_IMAGE_FAIL);
		ret = -1;
		goto RETURN;
	}
	
	//log part info
	UPG4_DEBUG("here is the image part info need burn!\n");
	stb_info_data_print_part(&image_part, g_upg_chunk_image);

	upg4_update_state(UPG_PROCESS_NUM, 50);
	upg4_update_state(UPG_STATUS_INFO, UPG_BURN_PART_FLAG);
	
//STEP5: protect keydata(stbid/mac/oui/hwversion...)
	upg4_update_state(UPG_PROCESS_NUM, 55);

	// restore keydata, mac/stbid ...
	if((upg_part->id == 0) && (g_upg_cfg.upg_keydata_cb != NULL))
	{
		if(g_upg_cfg.upg_keydata_cb((UINT32)image_part_addr, image_part_len, (UINT32)mirror_part_addr, mirror_part_len)<0)
		{
			UPG4_DEBUG("%s:%d: part [id = %d] keydata save fail\n", __FUNCTION__, __LINE__, upg_part->id);
			_upg_set_upg_state(UPG4_STATE_FAIL);
			_upg_set_err_type(UPG4_ERR_PART_KEYDATA_FAIL);
			ret = -1;
			goto RETURN;
		}
	}

//STEP5: burn part	
	UPG4_DEBUG("Pre-BURN part!\n");
	if(image_part.id == 0)
		_ret = upg_burn_norflash(&image_part, image_part_addr, image_part_len, 60, 90);
#ifdef NAND_FLASH_SUPPORT
	else
		_ret = upg_burn_part(&image_part, 0, image_part_addr, image_part_len, 60, 90);
#endif

	upg4_update_state(UPG_PROCESS_NUM, 95);
	
	UPG4_DEBUG("Post-BURN part!\n");
	if(_ret < 0)
	{
		UPG4_DEBUG("%s:%d: part [id = %d] burn_data fail\n", __FUNCTION__, __LINE__, upg_part->id);
		ret = -1;
		goto RETURN;
	}

	stb_info_data_update_part(&image_part, g_upg_chunk_image);
	stb_info_data_save();	

	upg4_update_state(UPG_PROCESS_NUM, 99);
	
	UPG4_DEBUG("upgrade part finish!\n");

	ret = 0;

RETURN:
	return ret;
}

int _upg_upgrade_part_by_all(STB_PART *upg_part, UINT8 *upgrade_data, UINT32 upgrade_len, BOOL unlimit_flag)
{
	int ret = -1;
	STB_PART mirror_part, backup_part;
	STB_CHUNK_INFO upg_chunk, mirror_chunk;
	UINT8 *mirror_part_addr = (UINT8 *)g_upg_cfg.data_mirror_addr; // what is old
	UINT32 mirror_part_len = 0;
	UINT8 *image_part_addr = (UINT8 *)g_upg_cfg.data_image_addr; // what is real to upgrade, merged from upper 2
	UINT32 image_part_len = 0;
	UINT32 part_addr=0, mirror_addr;
	UINT32 i;
	UINT32 f_start_offset;
	unsigned char *mirror_pointer = NULL;

#ifdef NAND_FLASH_SUPPORT
	struct ali_nand_info _nflash_info;

	NF_GetNandInfo(g_upg_nflash_dev, &_nflash_info);
#endif

	UPG4_DEBUG("now upgrade part %d\n", upg_part->id);

	upg4_update_state(UPG_PROCESS_NUM, 5);
	upg4_update_state(UPG_STATUS_INFO, UPG_BACKUP_MIRROR_FLAG);
	
//STEP1: load mirror data
	stb_info_data_get_info_by_id(upg_part->id, &mirror_part);
	mirror_part_len = mirror_part.size;
	if(mirror_part.id == 0) // Nor flash
	{
		upg_get_norflash_data(mirror_part_addr, mirror_part_len, 7, 19);	
	}
#ifdef NAND_FLASH_SUPPORT	
	else
	{
		mirror_part_len = ((mirror_part_len + _nflash_info.pagesize) / _nflash_info.pagesize) * (_nflash_info.pagesize); // pagesize align for nand load!
		ret = NF_LoadPartition(g_upg_nflash_dev, mirror_part.id, (void *)mirror_part_addr, mirror_part_len);
		if(ret != 0)
		{
			UPG4_DEBUG("load partition %d fail, ret = %d\n", mirror_part.id, ret);
			_upg_set_upg_state(UPG4_STATE_FAIL);
			_upg_set_err_type(UPG4_ERR_PART_LOAD_FAIL);
			ret = -1;
			goto RETURN;
		}
	}		
#endif

	upg4_update_state(UPG_PROCESS_NUM, 20);	

//STEP2: flash size check!
	if(upg_part->id == 0)
	{
		if(upg_part->size > (UINT32)stb_info_data_get_nor_size())
		{
			UPG4_DEBUG("fail: Nor flash image size [0x%x] overflow [nor flash size = 0x%x]!\n", upg_part->size, stb_info_data_get_nor_size());
			_upg_set_upg_state(UPG4_STATE_FAIL);
			_upg_set_err_type(UPG4_ERR_PART_SIZE_OVERFLOW);
			ret = -1;
			goto RETURN;
		}
	}
#ifdef NAND_FLASH_SUPPORT	
	else
	{
		UINT32 nand_part_size = _nflash_info.partInfo[upg_part->id].len - _nflash_info.blocksize;
		upg_part->size = ((upg_part->size + _nflash_info.pagesize) / _nflash_info.pagesize) * (_nflash_info.pagesize); // pagesize align for nand load!
		if(nand_part_size < upg_part->size)
		{
			UPG4_DEBUG("fail: Nand flash image size [0x%x] overflow [nand part size = 0x%x]!\n", upg_part->size, nand_part_size);
			_upg_set_upg_state(UPG4_STATE_FAIL);
			_upg_set_err_type(UPG4_ERR_PART_SIZE_OVERFLOW);
			ret = -1;
			goto RETURN;
		}
	}
#endif

	if(upg_part->size > g_upg_cfg.data_image_len)
	{
		UPG4_DEBUG("fail: upg buffer image size [0x%x] overflow [upg buffer size = 0x%x]!\n", upg_part->size, g_upg_cfg.data_image_len);
		_upg_set_upg_state(UPG4_STATE_FAIL);
		_upg_set_err_type(UPG4_ERR_PART_SIZE_OVERFLOW);
		ret = -1;
		goto RETURN;
	}

//STEP3: copy upg chunk data to image buf
	/* get upg chunk*/
	ret = stb_info_data_get_chunk(g_upg_part_info.chunk, upg_part->chunk_list[0], &upg_chunk);
	if(ret == -1)
	{
		UPG4_DEBUG("%s:%d: have no chunk [idx=%d]\n", __FUNCTION__, __LINE__, upg_part->chunk_list[0]);
		ret = -1;
		goto RETURN;
	}

	/*get upg part addr*/
	part_addr = upg_chunk.addr + (UINT32)upgrade_data;

	image_part_len = upg_part->size;
	// MEMCPY(image_part_addr, (UINT8*)part_addr, image_part_len);	

	upg4_update_state(UPG_PROCESS_NUM, 30);
	
//STEP4: restore part data
	if(unlimit_flag == FALSE)
	{/*restore bootloader, private data(hdcp, cadata, userdb¡­)*/
		for(i=0; i<g_restore_chunk_num; i++)
		{
            //put the mirro or image part back to download data 
			ret = __upg_restore_chunk(g_restore_chunk_id[i], (UINT32)image_part_addr, image_part_len,(UINT32)part_addr,image_part_len);
			if(ret > 0)
			{
				UPG4_DEBUG("part %d [id = 0x%08x] restore_data \n", upg_part->id,g_restore_chunk_id[i]);
			}
		}
	}
    //put the restored download data into image part and wait to burn flash.
    MEMCPY(image_part_addr, (UINT8*)part_addr, image_part_len);		
//STEP5: protect keydata(stbid/mac/oui/hwversion...)

	upg4_update_state(UPG_PROCESS_NUM, 50);
	upg4_update_state(UPG_STATUS_INFO, UPG_BURN_PART_FLAG);

	if(upg_part->id == 0) 
	{
		// Nor flash need restore stb_info into image since it had been just saved!
		chunk_init((UINT32)image_part_addr,image_part_len);
		part_addr = (UINT32)chunk_goto(&g_stbinfo_id, 0xFFFFFFFF, 1);
		if(part_addr == 0)
		{
			ret = -1;
			UPG4_DEBUG("%s:%d: can't find stbinfo in image!\n", __FUNCTION__, __LINE__);
			goto RETURN;
		}

		chunk_init((UINT32)mirror_part_addr,mirror_part_len);
		mirror_addr = (UINT32)chunk_goto(&g_stbinfo_id, 0xFFFFFFFF, 1);
		if(mirror_addr == 0)
		{
			ret = -1;
			UPG4_DEBUG("%s:%d: can't find stbinfo in mirror!\n", __FUNCTION__, __LINE__);
			goto RETURN;
		}

		mirror_pointer = (UINT8*)mirror_addr;

		/* save back stbinfo chunk head */
		MEMCPY((UINT8*)part_addr, (UINT8*)mirror_pointer, CHUNK_HEADER_SIZE);

		if(load_stbinfo_buf(part_addr+CHUNK_HEADER_SIZE, 0x20000)<0)
		{
			UPG4_DEBUG("%s:%d: look, impossible error happen, stb_info restore fail!\n", __FUNCTION__, __LINE__);
			_upg_set_upg_state(UPG4_STATE_FAIL);
			_upg_set_err_type(UPG4_ERR_PART_NOR_RESTORE_FAIL);
			ret = -1;
			goto RETURN;
		}

		// restore keydata, mac/stbid ...
		if(g_upg_cfg.upg_keydata_cb != NULL)
		{
			if(g_upg_cfg.upg_keydata_cb((UINT32)image_part_addr, image_part_len, (UINT32)mirror_part_addr, mirror_part_len)<0)
			{
				UPG4_DEBUG("%s:%d: part [id = %d] keydata save fail\n", __FUNCTION__, __LINE__, upg_part->id);
				_upg_set_upg_state(UPG4_STATE_FAIL);
				_upg_set_err_type(UPG4_ERR_PART_KEYDATA_FAIL);
				ret = -1;
				goto RETURN;
			}
		}
	}

//STEP5: burn part			
	f_start_offset = 0;
	if(upg_part->id == 0) 
	{
		if(unlimit_flag == FALSE)
		{// skip bootloader
			ret = stb_info_data_get_chunk(g_upg_part_info.chunk, upg_part->chunk_list[0], &upg_chunk);
			if(ret == -1)
			{
				UPG4_DEBUG("%s:%d: have no chunk [idx=%d]\n", __FUNCTION__, __LINE__, upg_part->chunk_list[0]);
				ret = -1;
				goto RETURN;
			}
	
			ret = stb_info_data_get_chunk_by_id(NULL, g_bootloader_id, &mirror_chunk);
			if(ret == -1)
			{
				ret = -1;
				UPG4_DEBUG("%s:%d: can't find bootloader in mirror!\n", __FUNCTION__, __LINE__);
				goto RETURN;
			}

			if(upg_chunk.len != mirror_chunk.len)
			{
				ret = -1;
				UPG4_DEBUG("%s:%d: bootloader not match!\n", __FUNCTION__, __LINE__);
				goto RETURN;
			}

			//skip stb_info
			ret = stb_info_data_get_chunk_by_id(NULL, g_stbinfo_id, &mirror_chunk);
			if(ret == -1)
			{
				ret = -1;
				UPG4_DEBUG("%s:%d: can't find bootloader in mirror!\n", __FUNCTION__, __LINE__);
				goto RETURN;
			}

			f_start_offset = mirror_chunk.addr+mirror_chunk.len;
		}
	}
	
	ret = upg_burn_part(upg_part, f_start_offset, image_part_addr, image_part_len, 60, 90);
	if(ret < 0)
	{
		UPG4_DEBUG("%s:%d: part [id = %d] burn_data fail\n", __FUNCTION__, __LINE__, upg_part->id);
		ret = -1;
		goto RETURN;
	}
		
	upg4_update_state(UPG_PROCESS_NUM, 99);
	
	UPG4_DEBUG("upgrade part finish!\n");

	ret = 0;
	
RETURN:
	return ret;
}

int _upg_upgrade_part_by_primitive_chunk(STB_PART *upg_part, UINT8 *upgrade_data, UINT32 upgrade_len, BOOL unlimit_flag)
{
	int ret = -1, _ret = -1, i = 0;
	UINT8 *m_pointer = 0;
	UINT32 id,offset,len;
	STB_PART mirror_part, image_part, backup_part;
	UINT8 *mirror_part_addr = (UINT8 *)g_upg_cfg.data_mirror_addr; // what is old
	UINT32 mirror_part_len = 0;
	UINT8 *image_part_addr = (UINT8 *)g_upg_cfg.data_image_addr; // what is real to upgrade, merged from upper 2
	UINT32 image_part_len = 0;
	STB_CHUNK_INFO image_chunk;

#ifdef NAND_FLASH_SUPPORT	
	struct ali_nand_info _nflash_info;

	NF_GetNandInfo(g_upg_nflash_dev, &_nflash_info);
#endif

	UPG4_DEBUG("now upgrade part %d\n", upg_part->id);
	
	upg4_update_state(UPG_PROCESS_NUM, 5);
	upg4_update_state(UPG_STATUS_INFO, UPG_BACKUP_MIRROR_FLAG);

	_ret = stb_info_data_get_info_by_name(STB_PART_BACKUP, &backup_part);
	if(_ret == 0)
	{
		if(upg_part->id == backup_part.id)
		{
			UPG4_DEBUG("%s:%d: seems try to upgrade backup part, now ignore it!\n", __FUNCTION__, __LINE__);
			ret = 1;
			goto RETURN;
		}
	}

//STEP1: load mirror data
	stb_info_data_get_info_by_id(upg_part->id, &mirror_part);
	mirror_part_len = mirror_part.size;
	if(mirror_part.id == 0) // Nor flash
	{
		upg_reg_restore_id(STB_CHUNK_BOOTLOADER_ID);
		//upg_reg_restore_id(STB_CHUNK_STBINFO_ID);
		
		upg_get_norflash_data(mirror_part_addr, mirror_part_len, 7,19);		
	}
#ifdef NAND_FLASH_SUPPORT	
	else
	{
		mirror_part_len = ((mirror_part_len + _nflash_info.pagesize) / _nflash_info.pagesize) * (_nflash_info.pagesize); // pagesize align for nand load!
		_ret = NF_LoadPartition(g_upg_nflash_dev, mirror_part.id, (void *)mirror_part_addr, mirror_part_len);
		if(_ret != 0)
		{
			UPG4_DEBUG("%s:%d: load partition %d fail, ret = %d\n", __FUNCTION__, __LINE__, mirror_part.id, _ret);
			_upg_set_upg_state(UPG4_STATE_FAIL);
			_upg_set_err_type(UPG4_ERR_PART_LOAD_FAIL);
			ret = -1;
			goto RETURN;
		}
	}
#endif

	upg4_update_state(UPG_PROCESS_NUM, 20);


	upg4_update_state(UPG_PROCESS_NUM, 30);
	
//STEP4: reorg part data, need update the part info for stb-info
	_ret = __upg_reorg_part_primitive(upg_part, &image_part, upgrade_data, upgrade_len, mirror_part_addr, mirror_part_len, image_part_addr, &image_part_len, 30, 45);
	if(_ret == 1) // no need upgrade
	{
		UPG4_DEBUG("%s:%d: seems part [id = %d] no need upgrade, continue to next\n", __FUNCTION__, __LINE__, upg_part->id);
		ret = 0;
		goto RETURN;
	}
	else if(_ret < 0)
	{
		UPG4_DEBUG("%s:%d: part [id = %d] reorg_data fail\n", __FUNCTION__, __LINE__, upg_part->id);
		_upg_set_upg_state(UPG4_STATE_FAIL);
		_upg_set_err_type(UPG4_ERR_PART_REORG_IMAGE_FAIL);
		ret = -1;
		goto RETURN;
	}
	
	//log part info
	UPG4_DEBUG("here is the image part info need burn!\n");
	stb_info_data_print_part(&image_part, g_upg_chunk_image);

	upg4_update_state(UPG_PROCESS_NUM, 50);
	upg4_update_state(UPG_STATUS_INFO, UPG_BURN_PART_FLAG);

	upg4_update_state(UPG_PROCESS_NUM, 55);

	// restore keydata, mac/stbid ...
	if((upg_part->id == 0) && (g_upg_cfg.upg_keydata_cb != NULL))
	{
		if(g_upg_cfg.upg_keydata_cb((UINT32)image_part_addr, image_part_len, (UINT32)mirror_part_addr, mirror_part_len)<0)
		{
			UPG4_DEBUG("%s:%d: part [id = %d] keydata save fail\n", __FUNCTION__, __LINE__, upg_part->id);
			_upg_set_upg_state(UPG4_STATE_FAIL);
			_upg_set_err_type(UPG4_ERR_PART_KEYDATA_FAIL);
			ret = -1;
			goto RETURN;
		}
	}

	//STEP5: burn part
	UPG4_DEBUG("Pre-BURN part!\n");
	if(image_part.id == 0)
		_ret = upg_burn_norflash(&image_part, image_part_addr, image_part_len, 60, 90);
#ifdef NAND_FLASH_SUPPORT	
	else
		_ret = upg_burn_part(&image_part, 0, image_part_addr, image_part_len, 60, 90);
#endif

	upg4_update_state(UPG_PROCESS_NUM, 95);
	
	UPG4_DEBUG("Post-BURN part!\n");
	if(_ret < 0)
	{
		UPG4_DEBUG("%s:%d: part [id = %d] burn_data fail\n", __FUNCTION__, __LINE__, upg_part->id);
		ret = -1;
		goto RETURN;
	}

	stb_info_data_update_part(&image_part, g_upg_chunk_image);
	stb_info_data_save();

	upg4_update_state(UPG_PROCESS_NUM, 99);
	
	UPG4_DEBUG("upgrade part finish!\n");

	ret = 0;

RETURN:
	return ret;
}

int _upg_upgrade_part_by_primitive_data(STB_PART *upg_part, UINT8 *upgrade_data, UINT32 upgrade_len, BOOL unlimit_flag)
{
	int ret = -1;
	STB_PART mirror_part, backup_part;
	STB_CHUNK_INFO upg_chunk, mirror_chunk;
	UINT8 *mirror_part_addr = (UINT8 *)g_upg_cfg.data_mirror_addr; // what is old
	UINT32 mirror_part_len = 0;
	UINT8 *image_part_addr = (UINT8 *)g_upg_cfg.data_image_addr; // what is real to upgrade, merged from upper 2
	UINT32 image_part_len = 0;
	UINT32 part_addr=0;
	UINT32 f_start_offset;
	unsigned char *mirror_pointer = NULL;
 
	if(upg_part->id != 0)
	{
		UPG4_DEBUG("%s:%d: not support part %d upgrade by primitive data\n", __FUNCTION__, __LINE__, upg_part->id);
		_upg_set_upg_state(UPG4_STATE_FAIL);
		_upg_set_err_type(UPG4_ERR_PART_LOAD_FAIL);
		ret = -1;
		goto RETURN;
	
	}
 
	UPG4_DEBUG("now upgrade part %d\n", upg_part->id);

	upg4_update_state(UPG_PROCESS_NUM, 5);
	upg4_update_state(UPG_STATUS_INFO, UPG_BACKUP_MIRROR_FLAG);
	
//STEP1: load mirror data
	stb_info_data_get_info_by_id(upg_part->id, &mirror_part);
	mirror_part_len = mirror_part.size;
	if(mirror_part.id == 0) // Nor flash
	{
		upg_get_norflash_data(mirror_part_addr, mirror_part_len, 7, 19);	
	}		
	
	upg4_update_state(UPG_PROCESS_NUM, 20);	

//STEP2: flash size check!
	if(upg_part->id == 0)
	{
		if(upg_part->size > (UINT32)stb_info_data_get_nor_size())
		{
			UPG4_DEBUG("fail: Nor flash image size [0x%x] overflow [nor flash size = 0x%x]!\n", upg_part->size, stb_info_data_get_nor_size());
			_upg_set_upg_state(UPG4_STATE_FAIL);
			_upg_set_err_type(UPG4_ERR_PART_SIZE_OVERFLOW);
			ret = -1;
			goto RETURN;
		}
	}

	if(upg_part->size > g_upg_cfg.data_image_len)
	{
		UPG4_DEBUG("fail: upg buffer image size [0x%x] overflow [upg buffer size = 0x%x]!\n", upg_part->size, g_upg_cfg.data_image_len);
		_upg_set_upg_state(UPG4_STATE_FAIL);
		_upg_set_err_type(UPG4_ERR_PART_SIZE_OVERFLOW);
		ret = -1;
		goto RETURN;
	}

//STEP3: copy upg chunk data to image buf
	/* get upg chunk*/
	ret = stb_info_data_get_chunk(g_upg_part_info.chunk, upg_part->chunk_list[0], &upg_chunk);
	if(ret == -1)
	{
		UPG4_DEBUG("%s:%d: have no chunk [idx=%d]\n", __FUNCTION__, __LINE__, upg_part->chunk_list[0]);
		ret = -1;
		goto RETURN;
	}

	/*get upg part addr*/
	part_addr = (UINT32)upgrade_data;

	image_part_len = mirror_part_len;

	MEMCPY(image_part_addr, (UINT8*)mirror_part_addr, mirror_part_len);
	MEMCPY(image_part_addr+upg_chunk.addr, (UINT8*)part_addr, upg_chunk.len);

	upg4_update_state(UPG_PROCESS_NUM, 30);
	
			
//STEP5: protect keydata(stbid/mac/oui/hwversion...)

	upg4_update_state(UPG_PROCESS_NUM, 50);
	upg4_update_state(UPG_STATUS_INFO, UPG_BURN_PART_FLAG);	

//STEP5: burn part			
	f_start_offset = 0;
	
	ret = upg_burn_part(upg_part, f_start_offset, image_part_addr, image_part_len, 60, 90);
	if(ret < 0)
	{
		UPG4_DEBUG("%s:%d: part [id = %d] burn_data fail\n", __FUNCTION__, __LINE__, upg_part->id);
		ret = -1;
		goto RETURN;
	}
		
	upg4_update_state(UPG_PROCESS_NUM, 99);
	
	UPG4_DEBUG("upgrade part finish!\n");

	ret = 0;
	
RETURN:
	return ret;
}

int upg_restore_chunk(UINT32 chunk_id, UINT32 mirror_addr, UINT32 mirror_len,UINT32 image_addr,UINT32 image_len)
{
	return __upg_restore_chunk(chunk_id, mirror_addr, mirror_len, image_addr, image_len);
}

void upg4_update_state(UINT32 type, UINT32 code)
{
	if(g_upg_cfg.upg_disp_cb != NULL)
	{
		if(type == UPG_PROCESS_NUM)
		{
			code |= g_upg_err_domain<<24;
		}
		g_upg_cfg.upg_disp_cb(type, code);	
	}
}

void upg4_config_burn_order(UINT8 *order, UINT8 num)
{
	MEMSET(g_upg_burn_order, 0xFF, STB_MAX_PART_NUM);
	MEMCPY(g_upg_burn_order, order, num);
}

void upg4_config(UPG_CONFIG *cfg)
{
	UINT32 i = 0;
	
	MEMSET(&g_upg_cfg, 0, sizeof(UPG_CONFIG));
	g_upg_cfg.compressed_addr = (UINT8*)((UINT32)cfg->compressed_addr & 0x8FFFFFFF);
	g_upg_cfg.compressed_len = cfg->compressed_len;
	g_upg_cfg.uncompressed_addr =  (UINT8*)((UINT32)cfg->uncompressed_addr & 0x8FFFFFFF);
	g_upg_cfg.uncompressed_len = cfg->uncompressed_len;
	g_upg_cfg.data_mirror_addr =  (UINT8*)((UINT32)cfg->data_mirror_addr & 0x8FFFFFFF);
	g_upg_cfg.data_mirror_len = cfg->data_mirror_len;
	g_upg_cfg.data_image_addr =  (UINT8*)((UINT32)cfg->data_image_addr & 0x8FFFFFFF);
	g_upg_cfg.data_image_len = cfg->data_image_len;
	g_upg_cfg.swap_addr =  (UINT8*)((UINT32)cfg->swap_addr & 0x8FFFFFFF);
	g_upg_cfg.swap_len = cfg->swap_len;
	g_upg_cfg.upg_disp_cb = cfg->upg_disp_cb;
	g_upg_cfg.upg_version_cb = cfg->upg_version_cb;
	g_upg_cfg.upg_keydata_cb = cfg->upg_keydata_cb;
	g_upg_cfg.upg_trigger_cb = cfg->upg_trigger_cb;
	
	/* Init FLASH device */
	if ((g_upg_flash_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0)) == NULL)
	{
		UPG4_DEBUG("ERROR: Not found Flash device!\n");
		return;
	}
	if (sto_open(g_upg_flash_dev) != SUCCESS)
	{
		UPG4_DEBUG("ERROR: sto_open failed!\n");
		return;
	}

#ifdef NAND_FLASH_SUPPORT
	g_upg_nflash_dev = (struct ali_nand_device *)dev_get_by_name("ALI_NAND");
	if(g_upg_nflash_dev == NULL)
	{
		UPG4_DEBUG("%s:%d: dev_get_by_name ALI_NAND fail!\n", __FUNCTION__, __LINE__);
		return;
	}
#endif

	g_upg_pan_dev = (struct pan_device *)dev_get_by_id(HLD_DEV_TYPE_PAN, 0);   

	for(i=0; i<STB_MAX_PART_NUM; i++)
	{
		g_upg_burn_order[i] = STB_MAX_PART_NUM - 1 - i;
		g_upg_burn_array[i] = 0xFF;
	}

}


