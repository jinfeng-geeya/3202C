#include <sys_config.h>
#include <types.h>
#include <mediatypes.h>
#include <math.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/flash/flash.h>
#include <api/libchunk/chunk.h>
#include <api/libstbinfo/stb_info.h>
#include <api/libstbinfo/stb_info_data.h>

#ifndef BACKUP_PRINTF
#define BACKUP_PRINTF PRINTF
#endif

/*burn m_pointer data to f_pointer*/
/*ask f_pointer and burn_size align to sector*/
int backup_burn_flash(UINT32 f_desc_addr, UINT32 m_src_addr, UINT32 burn_size)
{
	int ret;
	UINT32 size;
	UINT32 temp_start,temp_end;
	UINT32 sector_no;
	UINT32 f_pointer, m_pointer;

	f_pointer = f_desc_addr;
	m_pointer = m_src_addr;

	/*align f_pointer end address*/
	sector_no = flash_sector_align(f_pointer+burn_size);
	
	temp_start = flash_sector_start(sector_no);
	if (temp_start < f_pointer+burn_size)
	{
		ret = -1;
		BACKUP_PRINTF("Error: f_pointer end address not aligned.\n");
		return ret;
	}

	/*align f_pointer start address*/
	sector_no = flash_sector_align(f_pointer);
	
	temp_start = flash_sector_start(sector_no);
	if (temp_start < f_pointer)
	{
		ret = -1;
		BACKUP_PRINTF("Error:f_pointer start address not aligned.\n");
		return ret;
	}

	temp_end = m_pointer + burn_size;
	while(m_pointer < temp_end)
	{
		size = flash_sector_size(sector_no++);
		osal_task_sleep(200);
		switch (flash_verify((UINT32)f_pointer,(UINT8*)m_pointer, size))
		{
		case 2:	/*need erase flash first*/
			osal_task_sleep(200);
			flash_erase_sector((UINT32)f_pointer);
		case 1:
			osal_task_sleep(200);
			flash_copy((UINT32)f_pointer,(UINT8*)m_pointer, size);
			break;
		case 0:
			break;	
        	}

		m_pointer += size;
		f_pointer += size;
	}

	ret = 1;
	return ret;
}

int load_chunk_by_id(UINT32 chunk_id, UINT32 *load_len, UINT32 *m_pointer)
{
	int ret = -1, _ret = -1;
	STB_INFO_DATA *stb_data;
	STB_CHUNK_INFO chunk;
	UINT32 addr, len;
	struct sto_device *flash_dev;
	UINT8 *m_addr;

	/* Init FLASH device */
	flash_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);

	stb_data = stb_info_data_get();
	
	/*check upgloader*/
	if((stb_data != NULL) && !(stb_data->state & STB_INFO_DATA_STATE_RESET))
	{
		_ret = stb_info_data_get_chunk_by_id(NULL, chunk_id, &chunk);
		if(_ret<0)
		{
			ret = -1;
			BACKUP_PRINTF("%s:%d: can not get backup chunk [0x%x]\n", __FUNCTION__, __LINE__, chunk_id);
			return ret;
		}
		addr = chunk.addr;
		len = chunk.len;
	}
	else
	{
		addr = sto_chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
		len = sto_fetch_long(addr+CHUNK_OFFSET);
	}

	if(len > *load_len)
	{
		ret = -1;
		BACKUP_PRINTF("%s:%d: backup size not enough, chunk size [0x%x], back size[0x%x].\n", __FUNCTION__, __LINE__, len, *load_len);
		return ret;
	}
	*load_len = len;
	
	m_addr = MALLOC(len);
	if(m_addr==NULL)
	{
		ret = -1;
		BACKUP_PRINTF("%s:%d: malloc fail. [0x%x]\n", __FUNCTION__, __LINE__, len);
		return ret;
	}
	
	sto_get_data(flash_dev, m_addr, addr, len);

	chunk_init((UINT32)m_addr, len);
	_ret = chunk_check_single_crc(m_addr);
	if(_ret != 0)
	{	
		BACKUP_PRINTF("%s:%d: addr[0x%x] crc check fail:0x%x\n", __FUNCTION__, __LINE__, chunk_id, _ret);
		ret = -1;
		FREE(m_addr);
	}
	else
	{
		ret = 0;
		*m_pointer = (UINT32)m_addr;
	}

	return ret;
}

int load_chunk_by_addr(UINT32 load_addr, UINT32 *load_len, UINT32 *m_pointer)
{
	int ret=-1, _ret=-1;
	struct sto_device *flash_dev;
	UINT32 len;
	UINT8 *m_addr;
	
	/* Init FLASH device */
	flash_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);

	len = sto_fetch_long(load_addr+CHUNK_OFFSET);

	if(len > *load_len)
	{
		ret = -1;
		BACKUP_PRINTF("%s:%d: backup size not enough, chunk size [0x%x], back size[0x%x].\n", __FUNCTION__, __LINE__, len, *load_len);
		return ret;
	}
	*load_len = len;
	
	m_addr = MALLOC(len);
	if(m_addr==NULL)
	{
		ret = -1;
		BACKUP_PRINTF("%s:%d: malloc fail. [0x%x]\n", __FUNCTION__, __LINE__, len);
		return ret;
	}
	
	sto_get_data(flash_dev, m_addr, load_addr, len);

	chunk_init((UINT32)m_addr, len);
	_ret = chunk_check_single_crc(m_addr);
	if(_ret != 0)
	{	
		BACKUP_PRINTF("%s:%d: addr[0x%x] crc check fail:0x%x\n", __FUNCTION__, __LINE__, load_addr, _ret);
		ret = -1;
		FREE(m_addr);
	}
	else
	{
		ret = 0;
		*m_pointer = (UINT32)m_addr;
	}

	return ret;
}

/* backup chunk data in flash, e.g. otaloader*/
int backup_upgloader(UINT32 backup_id,UINT32 backup_addr,UINT32 backup_max_len)
{
	int ret=-1, _ret=-1;
	struct sto_device *flash_dev;
	unsigned char  *m_pointer;
	UINT32 i;
	UINT32 len;

	BACKUP_PRINTF("backup_upgloader, id:0x%x, addr:0x%x, len:0x%x\n",backup_id, backup_addr, backup_max_len);
	
	/* Init FLASH device */
	flash_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);

	/* backup data len == 0, not backup this chunk*/
	if(backup_max_len == 0)
	{
		ret = 1;
		BACKUP_PRINTF("backup data len == 0, no need backup\n");
		return ret;
	}

	if(backup_addr + backup_max_len > (UINT32)flash_dev->totol_size)
	{
		ret = 1;
		BACKUP_PRINTF("flash size not enough to backup\n");
		return ret;
	}

	m_pointer = NULL;
	len = backup_max_len;
	_ret = load_chunk_by_id(backup_id, (UINT32*)&len, (UINT32*)&m_pointer);

	if(_ret == 0)
	{
		ret = 0;
		backup_burn_flash(backup_addr, (UINT32)m_pointer, len);

		if(m_pointer != NULL)
			FREE(m_pointer);
	}
	else
	{
		ret = -1;
	}

	return ret;
}

int restore_upgloader(UINT32 backup_id,UINT32 backup_addr, UINT32 backup_max_len)
{
	int ret=-1, _ret=-1;
	STB_CHUNK_INFO chunk;
	struct sto_device *flash_dev;
	unsigned char  *m_pointer;
	UINT32 i;
	STB_INFO_DATA *stb_data;
	UINT32 addr, len;

	BACKUP_PRINTF("restore_upgloader, id:0x%x, addr:0x%x, len:0x%x\n",backup_id, backup_addr, backup_max_len);

	stb_data = stb_info_data_get();
	
	/*check upgloader*/
	if(stb_data != NULL)
	{
		_ret = stb_info_data_get_chunk_by_id(NULL, backup_id, &chunk);
		if(_ret<0)
		{
			ret = -1;
			BACKUP_PRINTF("%s:%d: can not get backup chunk [0x%x]\n", __FUNCTION__, __LINE__, backup_id);
			return ret;
		}
		addr = chunk.addr;
	}
	else
	{
		addr = sto_chunk_goto(&backup_id, 0xFFFFFFFF, 1);
	}

	m_pointer = NULL;
	len = backup_max_len;
	_ret = load_chunk_by_addr(backup_addr, (UINT32*)&len, (UINT32*)&m_pointer);

	if(_ret == 0)
	{
		ret = 0;
		backup_burn_flash(addr, (UINT32)m_pointer, len);

		if(m_pointer != NULL)
			FREE(m_pointer);
	}
	else
	{
		ret = -1;
	}
	
	return ret;
}

