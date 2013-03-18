/*
 * Copyright (C) ALi Shanghai Corp. 2006
 *
 * Description
 *	Temp information save and load utility.
 *
 * History
 * 1. 20050118	Zhengdao Li	Change dev_get_by_name to dev_get_by_id.
 */
#include <types.h>
#include <retcode.h>
#include <sys_config.h>
#include <hld/sto/sto.h>
#include <hld/sto/sto_dev.h>
#include <api/libc/string.h>
#include <api/libc/printf.h>
#include <api/libdb/tmp_info.h>


#define TMP_PRINTF PRINTF


static INT32 tmp_f_addr = -1;
static UINT32 g_tmp_data_base_addr = 0;
static UINT32 g_tmp_data_len = 0;
static UINT8 *g_tmp_buf = NULL;


static INT32 _read_tmp_data(UINT32 addr,UINT8 *buff,UINT16 buff_len)
{
	INT32 ret;
	struct sto_device *sto_flash_dev;

	if(addr + buff_len > g_tmp_data_len)
	{
		TMP_PRINTF("_read_tmp_data(): invalid addr!\n");	
		return LOAD_TMP_INFO_FAILED;
	}
	
	/*read data from tmp memory buf*/
	if(g_tmp_buf!=NULL)
	{
		MEMCPY(buff, &g_tmp_buf[addr], buff_len);
		return SUCCESS;
	}
	sto_flash_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
	if(sto_get_data(sto_flash_dev, buff, addr+g_tmp_data_base_addr, (INT32)buff_len) == (INT32)buff_len)
		return SUCCESS;
	else
	{
		TMP_PRINTF("_read_tmp_data(): sto_read error at addr %x\n",addr);
		return LOAD_TMP_INFO_FAILED;
	}
}


static INT32 _get_tmp_free_addr(UINT16 tmp_len)
{

	if(tmp_f_addr == -1) 
	{ 
		INT32 i;
		UINT8 data;
		
		/*load tmp data sector all to memory, then search free addr*/
		struct sto_device *sto_flash_dev = NULL;
		if(g_tmp_buf==NULL)
		{
			g_tmp_buf = (UINT8 *)MALLOC(g_tmp_data_len);
			if(g_tmp_buf==NULL)
			{
				TMP_PRINTF("%s(): malloc fail!\n",__FUNCTION__);
				return LOAD_TMP_INFO_FAILED;
			}
			sto_flash_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
			if(sto_get_data(sto_flash_dev, g_tmp_buf, g_tmp_data_base_addr, (INT32)g_tmp_data_len) != (INT32)g_tmp_data_len)
				return LOAD_TMP_INFO_FAILED;
		}
		
		for(i = g_tmp_data_len -sizeof(data); i>=0; i-=sizeof(data)) 
		{
			/*if (_read_tmp_data(i, (UINT8 *)&data, sizeof(data)) != SUCCESS) 
			{
				return LOAD_TMP_INFO_FAILED;
			}
			if (data != 0xFF)
				break;
			*/
			/*load tmp data sector all to memory, then search free addr*/
			if(g_tmp_buf[i]!=0xFF)
				break;
		}
		tmp_f_addr = i+sizeof(data);
	}
	TMP_PRINTF("tmp_f_addr=%08x\n", tmp_f_addr);
	return tmp_f_addr;
}




/**
 * backward_locate
 *
 * locating ident value from start_addr backward till to margin (include).
 * @start_addr	: the starting locating address.
 * @margin	: the address to end search.
 * @ident	: the identifier
 * return	: -1: identifier not found, other: the identifier address.
 */
static INT32 backward_locate(INT32 start_addr, INT32 margin, UINT32 ident)
{
	UINT32 data;
	if (_read_tmp_data(start_addr, (UINT8 *)&data, sizeof(data))!= SUCCESS)
		return -1;
	while (data != ident) 
	{
		start_addr -= sizeof(UINT32); 
		if (start_addr<margin) 
		{
			break;
		} 
		else if(_read_tmp_data(start_addr, (UINT8 *)&data, sizeof(data))!= SUCCESS) 
		{
			return -1;
		}
 	}
	return (start_addr<margin)? -1: start_addr;
}

static INT32 _write_tmp_data(UINT32 addr,UINT8 *buff,UINT16 buff_len)
{
	INT32 ret;
	struct sto_device *sto_flash_dev;
	
	if(addr + buff_len > g_tmp_data_len)
		return SAVE_TMP_INFO_OUTRANGE;

	/*write data to tmp data buf at same time*/
	if(g_tmp_buf!=NULL)
	{
		MEMCPY(&g_tmp_buf[addr], buff, buff_len);
	}

	sto_flash_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
	if(sto_put_data(sto_flash_dev, addr+g_tmp_data_base_addr, buff, (INT32)buff_len) == (INT32)buff_len)
	{
		if(tmp_f_addr != -1)
				tmp_f_addr = addr + buff_len;
		return SUCCESS;
	}
	else
	{
		TMP_PRINTF("_read_tmp_data(): sto_read error at addr %x\n",addr);
		return SAVE_TMP_INFO_FAILED;
	}
	
}



INT32 erase_tmp_sector()
{
	UINT32 param;
	struct sto_device *sto_flash_dev;
	
 	sto_flash_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
	UINT32 flash_cmd;
	if(sto_flash_dev->totol_size <= 0x400000)
	{
		param = (g_tmp_data_base_addr<<10) + ( g_tmp_data_len>>10);
		flash_cmd = STO_DRIVER_SECTOR_ERASE;
	}
	else
	{
		UINT32 tmp_param[2];
		tmp_param[0] = g_tmp_data_base_addr;
		tmp_param[1] = ( g_tmp_data_len>>10);
		param = (UINT32)tmp_param;
		flash_cmd = STO_DRIVER_SECTOR_ERASE_EXT;
	}
	if(sto_io_control(sto_flash_dev, flash_cmd, param) != SUCCESS)
	{
		TMP_PRINTF("erase_tmp_sector(): Erase info table flash memory failed!\n");
		return ERASE_TMP_INFO_FAILED;
	}
	tmp_f_addr = -1;
	return SUCCESS;
}


INT32 save_tmp_data(UINT8 *buff,UINT16 buff_len)
{
	UINT32 f_addr;
	UINT32 start_id = TMP_DATA_START_ID,end_id = TMP_DATA_END_ID;

	f_addr = _get_tmp_free_addr(buff_len);
	if(f_addr + (UINT32)buff_len + 8 > g_tmp_data_len)/*include Start&End flag length */
	{
		/*free tmp data buf befor erase sector*/
		if(g_tmp_buf!=NULL)
		{
			FREE(g_tmp_buf);
			g_tmp_buf = NULL;
		}
		
		erase_tmp_sector();
		f_addr = _get_tmp_free_addr(buff_len);
	}

	/* if tmp data sector is blank,the free addr is 0 */
	_write_tmp_data(f_addr,(UINT8 *)&start_id,sizeof(UINT32));
	_write_tmp_data(f_addr+4,buff,buff_len);
	_write_tmp_data(f_addr+buff_len+4,(UINT8 *)&end_id,sizeof(UINT32));
	return SUCCESS;
}


INT32 load_tmp_data(UINT8 *buff,UINT16 buff_len)
{
	INT32 t_addr, h_addr;
	if (g_tmp_data_len < (UINT32)(h_addr = _get_tmp_free_addr(buff_len))) 
	{
		TMP_PRINTF("_load_tmp_data(): free address not enough or invalid.\n");
		return LOAD_TMP_INFO_FAILED;
	}
	TMP_PRINTF("tmp info free addr = %08x\n", h_addr);
	/*
	 * h_addr is not always the next position for ending ID.
	 * since reported from YST.
	 */
	do {
		/*
		 * looking for ending ID from tail to the position
		 * where only 1 valid entry could write.
		 */
		t_addr = backward_locate(h_addr-4, buff_len+4, TMP_DATA_END_ID);
		TMP_PRINTF("tail address=%08x\n",t_addr);
		if (t_addr<0) 
		{
			TMP_PRINTF("valid tail not found!\n");
			return LOAD_TMP_INFO_FAILED;
		}
		/*
		 * looking for starting ID from tail-4
		 */
		h_addr = backward_locate(t_addr-4, 0, TMP_DATA_START_ID);
		TMP_PRINTF("head address=%08x\n", h_addr);
		if (h_addr<0) 
		{
			TMP_PRINTF("valid head not found!\n");
			return LOAD_TMP_INFO_FAILED;
		}
	}while (t_addr-h_addr != buff_len+4); /* at the correct position.*/
	return _read_tmp_data(h_addr+4,buff,buff_len);

}


INT32 init_tmp_info(UINT32 base_addr, UINT32 len)
{
	if(g_tmp_data_base_addr != 0 && g_tmp_data_len != 0)
		return SUCCESS;

	g_tmp_data_base_addr = base_addr;
	g_tmp_data_len = len;

	return SUCCESS;
}



