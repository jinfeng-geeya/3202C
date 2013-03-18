
#include <types.h>
#include <retcode.h>
#include <sys_config.h>
#include <hld/sto/sto.h>
#include <hld/sto/sto_dev.h>
#include <bus/flash/flash.h>
#include <api/libc/string.h>
#include <api/libc/printf.h>
#include <api/libstbinfo/stb_info.h>
#include <api/libstbinfo/stb_info_data.h>
#include <api/libchunk/chunk.h>

#ifdef NAND_FLASH_SUPPORT
#include <bus/nand/ali_nand_dev.h>
#endif

#ifdef NAND_DEBUG
#define STB_DATA_PRINTF libc_printf
#else
#define STB_DATA_PRINTF PRINTF
#endif

static STB_INFO_DATA _stb_data; // for backup compare to reduce read write
STB_INFO_DATA *g_stb_data = NULL;
int _stb_data_len = 0; // here is the total len of stb data, some not seen at this level

#ifdef NAND_FLASH_SUPPORT
static struct ali_nand_device *_nflash_dev = NULL;
#endif

void stb_info_data_clear_allchunk(void);

static int __stb_info_data_burn_part(UINT16 part_id, UINT8 *image_part_addr, UINT32 image_part_len)
{
	int ret = -1, i = 0;
	UINT32 f_chunk_addr;	//relative address
	UINT8 *m_pointer = 0;
	UINT32 id,offset,len;
	UINT32 sector_no;
	UINT32 temp_start,temp_end;
	unsigned char *f_pointer = NULL;
	STB_PART _part;

	STB_DATA_PRINTF("now burn part %d, addr = 0x%x, len = 0x%x\n", part_id, image_part_addr, image_part_len);
	if(part_id == 0) // Nor flash
	{
		f_pointer = (unsigned char *)0;
		temp_start = (UINT32)image_part_addr;
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
					flash_erase_sector((UINT32)f_pointer);
				case 1:
					osal_task_sleep(200);
					flash_copy((UINT32)f_pointer,(UINT8*)temp_start, len);
					break;
				case 0:
				default:
					break;	
	        }

			f_pointer += len;
			temp_start += len;
		}	
	}
#ifdef NAND_FLASH_SUPPORT	
	else
	{
		ret = NF_SavePartition(_nflash_dev, part_id, (void *)image_part_addr, image_part_len);
		if(ret != 0)
		{
			STB_DATA_PRINTF("save partition %d fail, ret = %d\n", part_id, ret);
		}
	}
#endif
	ret = 0;
	
RETURN:
	return ret;
}

int stb_info_data_check(STB_INFO_DATA *stb_data)
{
	int ret = -1;
	UINT32 i = 0, j = 0;
	STB_CHUNK_INFO chunk;
	STB_PART *part = NULL;
	STB_CHUNK_INFO *chunk_list = stb_data->chunk;

	if((stb_data->nor_enable != 1)
#ifdef NAND_FLASH_SUPPORT	
		|| (stb_data->nand_enable != 1)
#endif		
		)
	{
		STB_DATA_PRINTF("%s:%d: error no nor or nand flash enabled!\n", __FUNCTION__, __LINE__);
		ret = -1;
		goto RETURN;
	}
	if(stb_data->stb_part_num > STB_MAX_PART_NUM)
	{
		STB_DATA_PRINTF("%s:%d: error part num = %d, overflow max = %d!\n", __FUNCTION__, __LINE__, stb_data->stb_part_num, STB_MAX_PART_NUM);
		ret = -1;
		goto RETURN;
	}
	if(stb_data->stb_chunk_num > STB_MAX_CHUNK_NUM)
	{
		STB_DATA_PRINTF("%s:%d: error chunk num = %d, overflow max = %d!\n", __FUNCTION__, __LINE__, stb_data->stb_chunk_num, STB_MAX_CHUNK_NUM);
		ret = -1;
		goto RETURN;
	}

	for(i=0; i<stb_data->stb_part_num; i++)
	{
		part = &stb_data->part[i];
		if(part->id >= STB_MAX_PART_NUM)
		{
			STB_DATA_PRINTF("%s:%d: error part id = %d, overflow max = %d!\n", __FUNCTION__, __LINE__, part->id, STB_MAX_PART_NUM);
			ret = -1;
			goto RETURN;
		}
		if(part->chunk_num > STB_PART_MAX_CHUNK_NUM)
		{
			STB_DATA_PRINTF("%s:%d: error part chunk num = %d, overflow max = %d!\n", __FUNCTION__, __LINE__, part->chunk_num, STB_PART_MAX_CHUNK_NUM);
			ret = -1;
			goto RETURN;
		}
		if((part->id == 0) && (part->size > stb_data->nor_flash_size))
		{
			STB_DATA_PRINTF("%s:%d: error nor flash part size = 0x%x, overflow total flash = 0x%x!\n", __FUNCTION__, __LINE__, part->size, stb_data->nor_flash_size);
			ret = -1;
			goto RETURN;
		}
#ifdef NAND_FLASH_SUPPORT			
		if((part->id > 0) && (part->size > stb_data->nand_flash_size))
		{
			STB_DATA_PRINTF("%s:%d: error nand flash part %d size = 0x%x, overflow total flash = 0x%x!\n", __FUNCTION__, __LINE__, part->id, part->size, stb_data->nand_flash_size);
			ret = -1;
			goto RETURN;
		}
#endif

		for(j=0; j<part->chunk_num; j++)
		{
			ret = stb_info_data_get_chunk(chunk_list, part->chunk_list[j], &chunk);
			if(ret != -1)
			{
				if((chunk.id != STB_CHUNK_BOOTLOADER_ID) && (((chunk.id >> 16) & 0xFF) + ((chunk.id >> 24) & 0xFF) != 0xFF))
				{
					STB_DATA_PRINTF("%s:%d: error part %d chunk %d id = 0x%08x!\n", __FUNCTION__, __LINE__, i, j, chunk.id);
					ret = -1;
					goto RETURN;
				}

				if(chunk.addr >= part->size)
				{
					STB_DATA_PRINTF("!!!!!!!!!!!FATAL ERROR, chunk out of part range!!!!!!!!!!!!!!!!!\n");
					STB_DATA_PRINTF("part id: %d\n", part->id);
					STB_DATA_PRINTF("\tchunk num: %d\n", part->chunk_num);
					STB_DATA_PRINTF("\tname: %s\n", (char *)part->name);
					STB_DATA_PRINTF("\tsize: 0x%x\n", part->size);
					STB_DATA_PRINTF("\t\tchunk%d id: 0x%08x, ver: %s, addr: 0x%x, \tlen: 0x%x, \tupg_flag: %d\n\n\n", i+1, \
						chunk.id, chunk.ver, chunk.addr, chunk.len, chunk.upg_flag);
					ret = -1;
					goto RETURN;
				}
			}
		}
	}

	ret = 0;
	
RETURN:
	if(ret != 0)
	{
		stb_info_data_clear_allchunk();
		STB_DATA_PRINTF("!!!!!!!!!!!stb_info_data_check error, now set to RESET mode!!!!!!!!!!!!!!!!!\n");
		STB_DATA_PRINTF("!!!!!!!!!!!Now need to do Lstb to load right value!!!!!!!!!!!!!!!!!!!\n");
	}
	
	return ret;
}

void stb_info_data_print_part(STB_PART *part, STB_CHUNK_INFO *chunk_list)
{
	int ret = -1;
	UINT32 i = 0, j = 0;
	STB_CHUNK_INFO chunk;
	STB_PART _part;
	
	STB_DATA_PRINTF("part id: %d\n", part->id);
	STB_DATA_PRINTF("\tchunk num: %d\n", part->chunk_num);
	STB_DATA_PRINTF("\tname: %s\n", (char *)part->name);
	STB_DATA_PRINTF("\tsize: 0x%x\n", part->size);
	for(i=0; i<part->chunk_num; i++)
	{
		ret = stb_info_data_get_chunk(chunk_list, part->chunk_list[i], &chunk);
		if(ret == -1)
		{
			STB_DATA_PRINTF("%s:%d: have no chunk [idx=%d]\n", __FUNCTION__, __LINE__, part->chunk_list[i]);
		}
		else
		{
			STB_DATA_PRINTF("\t\tchunk%d id: 0x%08x, ver: %s, addr: 0x%x, \tlen: 0x%x, \tupg_flag: %d\n", i+1, \
				chunk.id, chunk.ver, chunk.addr, chunk.len, chunk.upg_flag);
		}
	}
}

void stb_info_data_print(STB_INFO_DATA *stb_data)
{
	UINT32 i = 0, j = 0;
	
	STB_DATA_PRINTF("version: %d\n", stb_data->sw_version);
	STB_DATA_PRINTF("state: %d\n", stb_data->state);
	STB_DATA_PRINTF("upgrade date: %s\n", stb_data->download_date);
	STB_DATA_PRINTF("firmware date: %s\n", stb_data->firmware_date);
	STB_DATA_PRINTF("found part num: %d\n", stb_data->stb_part_num);
	STB_DATA_PRINTF("found chunk num: %d\n", stb_data->stb_chunk_num);
	STB_DATA_PRINTF("backup part: %d\n", (stb_data->bacup_part_id>0) ? ((INT32)stb_data->bacup_part_id-1) : -1);
	for(i=0; i<stb_data->stb_part_num; i++)
	{
		stb_info_data_print_part(&stb_data->part[i], stb_data->chunk);
	}
}

int stb_info_data_get_info_by_name(char *part_name, STB_PART *part)
{
	int ret = -1;
	UINT8 i = 0;

	if(part == NULL)
	{
		STB_DATA_PRINTF("%s:%d, ERROR nf_part = NULL!\n", __FUNCTION__, __LINE__);
		goto RETURN; 
	}
	if(stb_info_data_check(g_stb_data) != 0)
	{
		STB_DATA_PRINTF("%s:%d, ERROR stb_data check fail!\n", __FUNCTION__, __LINE__);
		goto RETURN; 
	}

	for(i=0; i<g_stb_data->stb_part_num; i++)
	{
		if(!STRCMP(part_name, g_stb_data->part[i].name))
		{
			MEMCPY(part, &g_stb_data->part[i], sizeof(STB_PART));
			ret = 0;
			break;
		}
	}

RETURN:
	return ret;
}

int stb_info_data_get_info_by_id(UINT32 id, STB_PART *part)
{
	int ret = -1;
	UINT8 i = 0;

	if(part == NULL)
	{
		STB_DATA_PRINTF("%s:%d, ERROR nf_part = NULL!\n", __FUNCTION__, __LINE__);
		goto RETURN; 
	}
	if(stb_info_data_check(g_stb_data) != 0)
	{
		STB_DATA_PRINTF("%s:%d, ERROR stb_data check fail!\n", __FUNCTION__, __LINE__);
		goto RETURN; 
	}

	for(i=0; i<g_stb_data->stb_part_num; i++)
	{
		if(id == g_stb_data->part[i].id)
		{
			MEMCPY(part, &g_stb_data->part[i], sizeof(STB_PART));
			ret = 0;
			break;
		}
	}

RETURN:
	return ret;
}

int stb_info_data_update_part_size(UINT32 id, UINT32 size)
{
	int ret = -1;
	UINT8 i = 0;

	if(stb_info_data_check(g_stb_data) != 0)
	{
		STB_DATA_PRINTF("%s:%d, ERROR stb_data check fail!\n", __FUNCTION__, __LINE__);
		goto RETURN; 
	}
	
	for(i=0; i<g_stb_data->stb_part_num; i++)
	{
		if(id == g_stb_data->part[i].id)
		{
			g_stb_data->part[i].size = size;
			ret = 0;
			break;
		}
	}

RETURN:
	return ret;
}

int stb_info_data_get_sw_version(void)
{
	return g_stb_data->sw_version;
}

int stb_info_data_get_nor_size(void)
{
	return (g_stb_data->nor_enable ? g_stb_data->nor_flash_size : 0);
}

int stb_info_data_get_total_size(void)
{
	return _stb_data_len;
}

void stb_info_data_set_backup_partid(UINT32 id)
{
	g_stb_data->bacup_part_id = id + 1;
}

void stb_info_data_set_state(UINT8 state)
{
	g_stb_data->state = state;
}

int stb_info_data_set_info_by_id(UINT32 id, STB_PART *part)
{
	int ret = -1;
	UINT8 i = 0;

	if(part == NULL)
	{
		STB_DATA_PRINTF("%s:%d, ERROR nf_part = NULL!\n", __FUNCTION__, __LINE__);
		goto RETURN; 
	}
	if(stb_info_data_check(g_stb_data) != 0)
	{
		STB_DATA_PRINTF("%s:%d, ERROR stb_data check fail!\n", __FUNCTION__, __LINE__);
		goto RETURN; 
	}

	for(i=0; i<g_stb_data->stb_part_num; i++)
	{
		if(id == g_stb_data->part[i].id)
		{
			MEMCPY(&g_stb_data->part[i], part, sizeof(STB_PART));
			ret = 0;
			break;
		}
	}

RETURN:
	return ret;
}

int stb_info_data_add_chunk(STB_CHUNK_INFO *base, STB_CHUNK_INFO *chunk)
{
	int ret = -1, i = 0;
	STB_CHUNK_INFO *chunk_base = (base == NULL) ? g_stb_data->chunk : base;

	for(i=0; i<STB_MAX_CHUNK_NUM; i++)
	{
		if(chunk_base[i].id == 0)
		{
			ret = i;
			MEMCPY(&chunk_base[i], chunk, sizeof(STB_CHUNK_INFO));
			if(base == NULL)
			{
				g_stb_data->stb_chunk_num++;
				if(g_stb_data->stb_chunk_num >= STB_MAX_CHUNK_NUM)
				{
					libc_printf("%s:%d: chunk num overflow, total is %d!\n", __FUNCTION__, __LINE__, g_stb_data->stb_chunk_num);
					ret = -1;
					goto RETURN;
				}
			}
			break;
		}
	}

RETURN:
	return ret;
}

int stb_info_data_delete_chunk(STB_CHUNK_INFO *base, UINT32 chunk_id)
{
	int ret = -1, i = 0;
	STB_CHUNK_INFO *chunk_base = (base == NULL) ? g_stb_data->chunk : base;

	for(i=0; i<STB_MAX_CHUNK_NUM; i++)
	{
		if(chunk_base[i].id == chunk_id)
		{
			ret = i;
			MEMSET(&chunk_base[i], 0, sizeof(STB_CHUNK_INFO));
			if(base == NULL)
			{
				if(g_stb_data->stb_chunk_num == 0)
				{
					libc_printf("%s:%d: chunk num underflow, total is 0!\n", __FUNCTION__, __LINE__);
					ret = -1;
					goto RETURN;
				}
				g_stb_data->stb_chunk_num--;
			}
			break;
		}
	}

RETURN:
	return ret;
}

int stb_info_data_update_chunk(STB_CHUNK_INFO *base, STB_CHUNK_INFO *chunk)
{
	int ret = -1, i = 0;
	STB_CHUNK_INFO *chunk_base = (base == NULL) ? g_stb_data->chunk : base;

	for(i=0; i<STB_MAX_CHUNK_NUM; i++)
	{
		if(chunk_base[i].id == chunk->id)
		{
			ret = i;
			MEMCPY(&chunk_base[i], chunk, sizeof(STB_CHUNK_INFO));
			break;
		}
	}

RETURN:
	return ret;
}

int stb_info_data_update_chunk_list(STB_CHUNK_INFO *base)
{
	int ret = -1, i = 0;

	MEMCPY(g_stb_data->chunk, base, STB_MAX_CHUNK_NUM* sizeof(STB_CHUNK_INFO));
	ret = 0;

RETURN:
	return ret;
}


int stb_info_data_get_chunk(STB_CHUNK_INFO *base, UINT32 idx, STB_CHUNK_INFO *chunk)
{
	int ret = -1;
	STB_CHUNK_INFO *chunk_base = (base == NULL) ? g_stb_data->chunk : base;

	if(chunk_base[idx].id != 0)
	{
		ret = idx;
		MEMCPY(chunk, &chunk_base[idx], sizeof(STB_CHUNK_INFO));
	}

RETURN:
	return ret;
}

int stb_info_data_get_chunk_by_id(STB_CHUNK_INFO *base, UINT32 id, STB_CHUNK_INFO *chunk)
{
	int ret = -1, i = 0;
	STB_CHUNK_INFO *chunk_base = (base == NULL) ? g_stb_data->chunk : base;

	for(i=0; i<STB_MAX_CHUNK_NUM; i++)
	{
		if(chunk_base[i].id == id)
		{
			ret = i;
			MEMCPY(chunk, &chunk_base[i], sizeof(STB_CHUNK_INFO));
			break;
		}
	}

RETURN:
	return ret;
}


//check whether need restore Part according to stb_info_data state
BOOL stb_info_data_restore_check(void)
{
	return (g_stb_data->state == STB_INFO_DATA_STATE_PREBURN);
}

#ifdef NAND_FLASH_SUPPORT	
int stb_info_data_restore_default(UINT8 *image_addr, UINT32 image_len, UINT16 dest_part_id, UINT16 src_part_id)
{
	int ret = -1;
	int stbinfo_new_len = -1;
	struct ali_nand_info _nflash_info;

	// revert stb_data to last NORMAL one
	STB_DATA_PRINTF("Now need restore part%d, from part%d!\n", dest_part_id, src_part_id);

	_nflash_dev = (struct ali_nand_device *)dev_get_by_name("ALI_NAND");
	if(_nflash_dev == NULL)
	{
		STB_DATA_PRINTF("%s:%d: dev_get_by_name ALI_NAND fail!\n", __FUNCTION__, __LINE__);
		ret = -1;
		goto RETURN;
	}

	NF_GetNandInfo(_nflash_dev, &_nflash_info);

	image_len = ((image_len + _nflash_info.pagesize) / _nflash_info.pagesize) * (_nflash_info.pagesize); // pagesize align for nand load!
	ret = NF_LoadPartition(_nflash_dev, src_part_id, (void *)image_addr, image_len);
	if(ret != 0)
	{
		STB_DATA_PRINTF("%s:%d: load partition %d fail, ret = %d\n", __FUNCTION__, __LINE__, src_part_id, ret);
		ret = -1;
		goto RETURN;
	}
	
	ret = __stb_info_data_burn_part(dest_part_id, image_addr, image_len);
	if(ret != 0)
	{
		STB_DATA_PRINTF("%s:%d: restore part burn fail, ret = %d\n", __FUNCTION__, __LINE__, ret);
		ret = -1;
		goto RETURN;
	}
	
	ret = SUCCESS;
	
RETURN:
	return ret;
}

//check whether need restore Part according to stb_info_data state
int stb_info_data_restore(UINT8 *image_addr, UINT32 image_len)
{
	int ret = -1;
	STB_PART restore_part, backup_part;
	int stbinfo_new_len = -1;
	struct ali_nand_info _nflash_info;
	UINT32 backup_1st_chunk_id = 0, backup_part_id = 0;

	if(g_stb_data->state == STB_INFO_DATA_STATE_NORMAL)
	{
		goto RETURN;
	}

	_nflash_dev = (struct ali_nand_device *)dev_get_by_name("ALI_NAND");
	if(_nflash_dev == NULL)
	{
		STB_DATA_PRINTF("%s:%d: dev_get_by_name ALI_NAND fail!\n", __FUNCTION__, __LINE__);
		ret = -1;
		goto RETURN;
	}
	
	stbinfo_new_len = get_stbinfo_len();
	if(_stb_data_len != stbinfo_new_len)
	{
		STB_DATA_PRINTF("%s:%d: ERROR: stbinfo data len changed: old 0x%x, now 0x%x\n", __FUNCTION__, __LINE__, _stb_data_len, stbinfo_new_len);
		ret = -1;
		goto RETURN;
	}
	ret = stb_info_data_get_info_by_name(STB_PART_BACKUP, &backup_part);
	if(ret == -1)
	{
		STB_DATA_PRINTF("%s:%d: ERROR: no backup part\n", __FUNCTION__, __LINE__);
		ret = -1;
		goto RETURN;
	}

	NF_GetNandInfo(_nflash_dev, &_nflash_info);

	image_len = backup_part.size;
	image_len = ((image_len + _nflash_info.pagesize) / _nflash_info.pagesize) * (_nflash_info.pagesize); // pagesize align for nand load!
	ret = NF_LoadPartition(_nflash_dev, backup_part.id, (void *)image_addr, image_len);
	if(ret != 0)
	{
		STB_DATA_PRINTF("%s:%d: load partition %d fail, ret = %d\n", __FUNCTION__, __LINE__, backup_part.id, ret);
		ret = -1;
		goto RETURN;
	}
	chunk_init(image_addr, image_len);
	ret = chunk_check_crc();
	if(ret != 0)
	{
		STB_DATA_PRINTF("%s:%d: bakcup part check fail, ret = 0x%x!\n", __FUNCTION__, __LINE__, ret);
		ret = -1;
		goto RETURN;
	}

	backup_1st_chunk_id  = fetch_long((UINT32)image_addr + CHUNK_ID);
	backup_part_id = (backup_1st_chunk_id&0xF);
	if(backup_part_id != (g_stb_data->bacup_part_id - 1))
	{
		STB_DATA_PRINTF("%s:%d: WARNING -- bakcup part named %d but realy %d!\n", __FUNCTION__, __LINE__, (g_stb_data->bacup_part_id - 1), backup_part_id);
	}

	// revert stb_data to last NORMAL one
	STB_DATA_PRINTF("Now need restore part%d, from backup part!\n", backup_part_id);
	stb_info_data_get_info_by_id(backup_part_id, &restore_part);
	
	STB_DATA_PRINTF("part what need restore!\n");
	stb_info_data_print_part(&restore_part, g_stb_data->chunk);
	STB_DATA_PRINTF("part backuped!\n");
	stb_info_data_print_part(&backup_part, g_stb_data->chunk);

	if(backup_part.size > image_len)
	{
		STB_DATA_PRINTF("%s:%d: ERROR: backup data size 0x%x > image buffer size 0x%x\n", __FUNCTION__, __LINE__, backup_part.size > image_len);
		ret = -1;
		goto RETURN;
	}
	
	g_stb_data->state = STB_INFO_DATA_STATE_NORMAL;
	restore_part.chunk_num = backup_part.chunk_num;
	restore_part.size = backup_part.size;
	MEMCPY(restore_part.chunk_list, backup_part.chunk_list, STB_PART_MAX_CHUNK_NUM);
	
	ret = __stb_info_data_burn_part(restore_part.id, image_addr, backup_part.size);
	if(ret != 0)
	{
		STB_DATA_PRINTF("%s:%d: restore part burn fail, ret = %d\n", __FUNCTION__, __LINE__, ret);
		ret = -1;
		goto RETURN;
	}
	
	stb_info_data_update_part(&restore_part, NULL);
	ret = save_stbinfo_data((UINT8*)g_stb_data, stb_info_data_get_total_size());
	if(ret < 0)
	{
		STB_DATA_PRINTF("%s:%d, ERROR save_stbinfo_data!ret %d\n", __FUNCTION__, __LINE__, ret);
		ret = -1;
		goto RETURN;
	}
	
	ret = SUCCESS;
	
RETURN:
	return ret;
}
#endif

void stb_info_data_clear_allchunk(void)
{
	int i = 0;

	g_stb_data->state = STB_INFO_DATA_STATE_RESET;
	g_stb_data->stb_chunk_num = 0;
	for(i=0; i<STB_MAX_PART_NUM; i++)
	{
		g_stb_data->part[i].chunk_num = 0;
		g_stb_data->part[i].size = 0;
		MEMSET(g_stb_data->part[i].chunk_list, 0, STB_PART_MAX_CHUNK_NUM);
	}
	MEMSET(g_stb_data->chunk, 0, sizeof(STB_CHUNK_INFO)*STB_MAX_CHUNK_NUM);
}

void stb_info_data_save(void)
{
	INT32 ret = 0;
	static UINT8 _save_stb_info_num = 0;

	if(((g_stb_data != NULL) && (g_stb_data->crc != 0)) && (MEMCMP(&_stb_data, g_stb_data, sizeof(STB_INFO_DATA))))
	{
		_save_stb_info_num++;
		STB_DATA_PRINTF("-->save stbinfo num=%d, len=%d!\n", _save_stb_info_num, _stb_data_len);
		
		g_stb_data->crc_len = sizeof(STB_INFO_DATA) - 8;
		g_stb_data->crc = MG_Table_Driven_CRC(0xFFFFFFFF, &g_stb_data->upg_info, g_stb_data->crc_len);
		ret = save_stbinfo_data((UINT8 *)g_stb_data, _stb_data_len);
		if(ret < 0)
		{
			STB_DATA_PRINTF("Err1: %s, %d, cannot save stb info data. ret %d\n", __FUNCTION__, __LINE__, ret);
			return;
		}
		MEMCPY(&_stb_data, g_stb_data, sizeof(STB_INFO_DATA));
	}
	else
	{
		STB_DATA_PRINTF("Err: cannot or no need to save stb info data. g_stb_data 0x%08x, crc 0x%08x\n", g_stb_data, g_stb_data->crc);
		return;
	}
}

union UPG_INFO *stb_info_data_get_upginfo(void)
{
	union UPG_INFO *upg_info = NULL;
	
	if((g_stb_data != NULL) && (g_stb_data->crc != 0))
	{
		upg_info = &g_stb_data->upg_info;
	}

	return upg_info;
}

STB_INFO_DATA* stb_info_data_get(void)
{
	if((g_stb_data != NULL) && (g_stb_data->crc != 0))
	{
		return g_stb_data;
	}
	else
	{
		STB_DATA_PRINTF("%s:%d, ERROR stbinfo NULL!\n", __FUNCTION__, __LINE__);
		return NULL;
	}
}

int stb_info_data_load(UINT32 chid)
{
	int ret = -1;
	UINT32 offset;

	// init struct
	offset = sto_chunk_goto(&chid, 0xFFFFFFFF, 1);
   	init_stb_info(offset+0x80, 2*64*1024);
	MEMSET(&_stb_data, 0, sizeof(STB_INFO_DATA));

	if(g_stb_data == NULL)
	{
		_stb_data_len = get_stbinfo_len();
		if((_stb_data_len < 0) || ((UINT32)_stb_data_len < sizeof(STB_INFO_DATA)))
		{
			STB_DATA_PRINTF("%s:%d, ERROR stbinfo_len = %d!\n", __FUNCTION__, __LINE__, _stb_data_len);
			ret = -1;
			return ret;
		}
		STB_DATA_PRINTF("now need malloc for stb_info_data, size = %d, structure size = %d\n", _stb_data_len, sizeof(STB_INFO_DATA));

		g_stb_data = MALLOC(_stb_data_len);
		ASSERT(g_stb_data != NULL);
	}

	MEMSET(g_stb_data, 0, _stb_data_len);
	STB_DATA_PRINTF("now to load stb info data!\n");
	ret = load_stbinfo((UINT8*)g_stb_data, _stb_data_len);
	if(ret == SUCCESS)
	{
		if(g_stb_data->crc == (UINT32)MG_Table_Driven_CRC(0xFFFFFFFF, &g_stb_data->upg_info, g_stb_data->crc_len))
		{
			if(g_stb_data->state == STB_INFO_DATA_STATE_NORMAL)
			{
				ret = stb_info_data_check(g_stb_data);
			}
			if(ret != 0)
			{
				STB_DATA_PRINTF("stb_info_data_check fail, need reinit stb_info!\n");
				MEMSET(g_stb_data, 0, _stb_data_len);				
			}
			else
			{
				STB_DATA_PRINTF("here is the stb current stb data!\n");
				MEMCPY(&_stb_data, g_stb_data, sizeof(STB_INFO_DATA));
				stb_info_data_print(g_stb_data);
			}
		}
		else
		{
			ret = -1;
			MEMSET(g_stb_data, 0, _stb_data_len);				
			STB_DATA_PRINTF("stb_info_data crc fail, need reinit stb_info!\n");
		}
	}

	return ret;
}

void stb_info_data_update_part(STB_PART *part, STB_CHUNK_INFO *base)
{
	// it will update part to stb_info_data, and the chunk info refer to base, so will delete all chunks of the part in stb_info_data and then update with chunks in base
	int ret = -1, _ret = -1;
	UINT8 i = 0;
	STB_PART *dest_part = NULL;
	STB_CHUNK_INFO _chunk;
	UINT32 _part_chunk_base_addr = 0;

	if(part == NULL)
	{
		STB_DATA_PRINTF("%s:%d, ERROR nf_part = NULL!\n", __FUNCTION__, __LINE__);
		ret = -1;
		goto RETURN; 
	}
	if(stb_info_data_check(g_stb_data) != 0)
	{
		STB_DATA_PRINTF("%s:%d, ERROR stb_data check fail!\n", __FUNCTION__, __LINE__);
		ret = -1;
		goto RETURN; 
	}

	STB_DATA_PRINTF("update part %d\n", part->id);
	
	for(i=0; i<g_stb_data->stb_part_num; i++)
	{
		dest_part = NULL;
		if(part->id == g_stb_data->part[i].id)
		{
			dest_part = &g_stb_data->part[i];
			break;
		}
	}

	if(dest_part == NULL)
	{
		STB_DATA_PRINTF("%s:%d, seems no such part!\n", __FUNCTION__, __LINE__);
		ret = -1;
		goto RETURN; 
	}

	MEMCPY(dest_part, part, sizeof(STB_PART));
	
	// add new chunk from base
	for(i=0; i<part->chunk_num; i++)
	{
		_ret = stb_info_data_get_chunk(base, part->chunk_list[i], &_chunk);
		if(_ret == -1)
		{
			STB_DATA_PRINTF("%s:%d, ERROR get chunk idx=%d from 0x%x!\n", __FUNCTION__, __LINE__, part->chunk_list[i], base);
			ret = -1;
			goto RETURN; 
		}
		if(i == 0)
		{
			_part_chunk_base_addr = _chunk.addr;
		}
		
		_chunk.id &= 0xFFFFFFF0;
		_chunk.id |= part->id;
		_chunk.addr -= _part_chunk_base_addr;
		_chunk.upg_flag = 0;
		if((_chunk.id & 0xF) != part->id)
		{
			STB_DATA_PRINTF("%s:%d, ERROR save part %d, chunk %d id=0x%x not match!\n", __FUNCTION__, __LINE__, part->id, i, _chunk.id);
			ret = -1;
			goto RETURN; 
		}

		_ret = stb_info_data_update_chunk(NULL, &_chunk);
		if(_ret == -1)
		{
			//STB_DATA_PRINTF("now add chunk 0x%08x\n", _chunk.id);
			_ret = stb_info_data_add_chunk(NULL, &_chunk);
			if(_ret == -1)
			{
				STB_DATA_PRINTF("%s:%d, ERROR add chunk id=0x%x!\n", __FUNCTION__, __LINE__, _chunk.id);
				ret = -1;
				goto RETURN; 
			}
		}
		dest_part->chunk_list[i] = _ret;
		
	}
	ret = 0;
	
RETURN:
	return;
}

void stb_info_data_update_info(UINT32 sw_version, char *download_date, char *firmware_date)
{
	g_stb_data->sw_version = sw_version;
	MEMCPY((char *)g_stb_data->download_date, download_date,16);
	MEMCPY((char *)g_stb_data->firmware_date, firmware_date,16);
}


