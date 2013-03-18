
#include <types.h>
#include <retcode.h>
#include <sys_config.h>
#include <hld/sto/sto.h>
#include <hld/sto/sto_dev.h>
#include <api/libc/string.h>
#include <api/libc/printf.h>
#include <api/libstbinfo/stb_info.h>

#ifdef NAND_DEBUG
#define STBINFO_PRINTF libc_printf
#else
#define STBINFO_PRINTF PRINTF
#endif

#define STBINFO_SECTOR_LEN		(64*1024)

#define STBINFO_SECTOR1_ADDR	0
#define STBINFO_SECTOR2_ADDR	(64*1024)

static INT32 stbinfo_f_addr = -1;
static UINT32 g_stbinfo_base_addr = 0;
static UINT32 g_stbinfo_len = 0;
static UINT32 g_stbinfo_sector_len = 0;
static UINT8 *g_stbinfo_buf = NULL;

static INT32 stbinfo_backward_locate(INT32 start_addr, INT32 margin, UINT32 ident);
int load_stbinfo_buf(UINT8 *buf, UINT32 len);

static INT32 _read_stbinfo_data(UINT32 addr,UINT8 *buff,UINT16 buff_len)
{
	INT32 ret;
	struct sto_device *sto_flash_dev;
	UINT32 f_addr;

	f_addr = addr;
	if(f_addr>=STBINFO_SECTOR2_ADDR)
		f_addr -= STBINFO_SECTOR_LEN;
	
	if(f_addr + buff_len > STBINFO_SECTOR_LEN)
	{
		STBINFO_PRINTF("_read_stbinfo_data(): invalid addr!\n");	
		return LOAD_STB_INFO_FAILED;
	}
	
	/*read data from stbinfo memory buf*/
//	if(g_stbinfo_buf!=NULL)
//	{
//		MEMCPY(buff, &g_stbinfo_buf[addr], buff_len);
//		return SUCCESS;
//	}
	sto_flash_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
	if(sto_get_data(sto_flash_dev, buff, addr+g_stbinfo_base_addr, (INT32)buff_len) == (INT32)buff_len)
		return SUCCESS;
	else
	{
		STBINFO_PRINTF("_read_stbinfo_data(): sto_read error at addr %x\n",addr);
		return LOAD_STB_INFO_FAILED;
	}
}

static INT32 _get_stbinfo_sector_free_addr(UINT32 base_addr)
{
 	UINT8 *sector_buf;
	INT32 i;
	UINT8 data;
	
	sector_buf = &g_stbinfo_buf[base_addr];
	
	for(i = STBINFO_SECTOR_LEN -sizeof(data); i>=0; i-=sizeof(data)) 
	{
		if(sector_buf[i]!=0xFF)
			break;
	}

	return  (i+sizeof(data));
}

static INT32 _get_stbinfo_free_addr(UINT16 stbinfo_len)
{
	UINT32 sector1_addr,sector2_addr,sector_addr;
	INT32 h_addr,t_addr,f_addr;

	STBINFO_PRINTF("%s, %d, stbinfo_f_addr 0x%08x\n", __FUNCTION__, __LINE__, stbinfo_f_addr);
	
	if(stbinfo_f_addr == -1) 
	{ 
		INT32 i;
		UINT8 data;
		
		/*load one stbinfo to memory, then search free addr*/
		if(g_stbinfo_buf==NULL)
		{
			g_stbinfo_buf = (UINT8 *)MALLOC(g_stbinfo_len);
			if(g_stbinfo_buf==NULL)
			{
				STBINFO_PRINTF("%s(): malloc fail!\n",__FUNCTION__);
				return LOAD_STB_INFO_FAILED;
			}
			MEMSET(g_stbinfo_buf, 0xff, g_stbinfo_len);

			if(load_stbinfo_buf(g_stbinfo_buf, g_stbinfo_len)<0)
				return LOAD_STB_INFO_FAILED;
		}
		
		sector1_addr =_get_stbinfo_sector_free_addr(STBINFO_SECTOR1_ADDR);
		sector2_addr =_get_stbinfo_sector_free_addr(STBINFO_SECTOR2_ADDR);
		STBINFO_PRINTF("%s, %d, sector1 addr 0x%08x, sector2_addr 0x%08x\n", __FUNCTION__, __LINE__, sector1_addr, sector2_addr);
		if(sector2_addr == 0)			
		{
			stbinfo_f_addr = sector1_addr+STBINFO_SECTOR1_ADDR;
		}
		else if(sector1_addr == 0)			
		{
			stbinfo_f_addr = sector2_addr+STBINFO_SECTOR2_ADDR;
		}
		else
		{	
			if(sector1_addr > sector2_addr)
			{
				h_addr = sector2_addr + STBINFO_SECTOR2_ADDR;
				sector_addr = STBINFO_SECTOR2_ADDR;
				f_addr = sector2_addr + STBINFO_SECTOR2_ADDR;
			}
			else
			{
				h_addr = sector1_addr + STBINFO_SECTOR1_ADDR;
				sector_addr = STBINFO_SECTOR1_ADDR;
				f_addr = sector1_addr + STBINFO_SECTOR1_ADDR;
			}
			
			do {
				/*
				 * looking for ending ID from tail to the position
				 * where only 1 valid entry could write.
				 */
				t_addr = stbinfo_backward_locate(h_addr-4, sector_addr+stbinfo_len+4, STB_INFO_END_ID);
				STBINFO_PRINTF("%s, %d, t_addr 0x%08x, stbinfo_len %d\n", __FUNCTION__, __LINE__, t_addr, stbinfo_len);
				if (t_addr<0) 
				{
					break;
				}
				/*
				 * looking for starting ID from tail-4
				 */
				h_addr = stbinfo_backward_locate(t_addr-4, sector_addr, STB_INFO_START_ID);
				STBINFO_PRINTF("%s, %d, h_addr 0x%08x\n", __FUNCTION__, __LINE__, h_addr);
				if (h_addr<0) 
				{
					break;
				}
			}while (t_addr-h_addr != stbinfo_len+4); /* at the correct position.*/

			STBINFO_PRINTF("%s, %d, t_addr 0x%08x, h_addr 0x%08x, sector1 addr 0x%08x, sector2 addr 0x%08x\n", __FUNCTION__, __LINE__, t_addr, h_addr, sector1_addr, sector2_addr);
			if(t_addr<0 || h_addr<0)
			{
				if(sector1_addr > sector2_addr)
				{/* can not find valid start_ID and end_ID in sector2, erase sector2 and return sector1 addr */
					sector_addr = STBINFO_SECTOR2_ADDR;
					f_addr = sector1_addr + STBINFO_SECTOR1_ADDR;
				}
				else
				{/* can not find valid start_ID and end_ID in sector1, erase sector1 and return sector2 addr */
					sector_addr = STBINFO_SECTOR1_ADDR;
					f_addr = sector2_addr + STBINFO_SECTOR2_ADDR;
				}
			}
			else
			{
				if(sector1_addr > sector2_addr)
				{/* find valid start_ID and end_ID in sector2, erase sector1 and return sector2 addr */
					sector_addr = STBINFO_SECTOR1_ADDR;
					f_addr = sector2_addr + STBINFO_SECTOR2_ADDR;
				}
				else
				{/* find valid start_ID and end_ID in sector1, erase sector2 and return sector1 addr */
					sector_addr = STBINFO_SECTOR2_ADDR;
					f_addr = sector1_addr + STBINFO_SECTOR1_ADDR;
				}
			}

			erase_stbinfo_sector(sector_addr);
			MEMSET(g_stbinfo_buf+sector_addr, 0xff, STBINFO_SECTOR_LEN);
			stbinfo_f_addr = f_addr;
		}

	}
	STBINFO_PRINTF("stbinfo_f_addr=%08x\n", stbinfo_f_addr);
	return stbinfo_f_addr;
}




/**
 * stbinfo_backward_locate
 *
 * locating ident value from start_addr backward till to margin (include).
 * @start_addr	: the starting locating address.
 * @margin	: the address to end search.
 * @ident	: the identifier
 * return	: -1: identifier not found, other: the identifier address.
 */
static INT32 stbinfo_backward_locate(INT32 start_addr, INT32 margin, UINT32 ident)
{
	UINT32 data;
	//INT32 f_addr;

	//f_addr = start_addr;
	//if(f_addr>=STBINFO_SECTOR2_ADDR)
		//f_addr -= STBINFO_SECTOR_LEN;

	STBINFO_PRINTF("%s, %d, start_addr 0x%08x, margin 0x%08x, ident 0x%08x\n", __FUNCTION__, __LINE__, start_addr, margin, ident);
	
	if (_read_stbinfo_data(start_addr, (UINT8 *)&data, sizeof(data))!= SUCCESS)
		return -1;
	while (data != ident) 
	{
		start_addr -= sizeof(UINT32); 
		//f_addr -= sizeof(UINT32);
		if (start_addr<margin) 
		{
			break;
		} 
		else if(_read_stbinfo_data(start_addr, (UINT8 *)&data, sizeof(data))!= SUCCESS) 
		{
			return -1;
		}
 	}
	return (start_addr<margin)? -1: start_addr;
}

static INT32 _write_stbinfo_data(UINT32 addr,UINT8 *buff,UINT16 buff_len)
{
	INT32 ret;
	struct sto_device *sto_flash_dev;
	UINT32 f_addr;

	f_addr = addr;
	if(f_addr>=STBINFO_SECTOR2_ADDR)
		f_addr -= STBINFO_SECTOR_LEN;
	
	if(f_addr + buff_len > STBINFO_SECTOR_LEN)
		return SAVE_STB_INFO_OUTRANGE;

	/*write data to stbinfo buf at same time*/
	if(g_stbinfo_buf!=NULL)
	{
		MEMCPY(&g_stbinfo_buf[addr], buff, buff_len);
	}

	sto_flash_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
	if(sto_put_data(sto_flash_dev, addr+g_stbinfo_base_addr, buff, (INT32)buff_len) == (INT32)buff_len)
	{
		if(stbinfo_f_addr != -1)
			stbinfo_f_addr = addr + buff_len;
		return SUCCESS;
	}
	else
	{
		STBINFO_PRINTF("_write_stbinfo_data(): sto_read error at addr %x\n",addr);
		return SAVE_STB_INFO_FAILED;
	}
	
}



INT32 erase_stbinfo_sector(UINT32 sector_offset)
{
	UINT32 param;
	struct sto_device *sto_flash_dev;
	
 	sto_flash_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
	UINT32 flash_cmd;
	
	
	UINT32 tmp_param[2];
	tmp_param[0] = g_stbinfo_base_addr+sector_offset;
	tmp_param[1] = ( STBINFO_SECTOR_LEN>>10);
	param = (UINT32)tmp_param;
	flash_cmd = STO_DRIVER_SECTOR_ERASE_EXT;
	
	if(sto_io_control(sto_flash_dev, flash_cmd, param) != SUCCESS)
	{
		STBINFO_PRINTF("erase_stbinfo_sector(): Erase info table flash memory failed!\n");
		return ERASE_STB_INFO_FAILED;
	}
	stbinfo_f_addr = -1;
	return SUCCESS;
}


INT32 save_stbinfo_data(UINT8 *buff,UINT16 buff_len)
{
	INT32 ret = 0;
	UINT32 f_addr, addr;
	UINT32 start_id = STB_INFO_START_ID,end_id = STB_INFO_END_ID;

	addr = _get_stbinfo_free_addr(buff_len);
	f_addr = addr;
	STBINFO_PRINTF("%s, %d, addr 0x%08x, buffer len %d\n", __FUNCTION__, __LINE__, addr, buff_len);
	
	if(f_addr>=STBINFO_SECTOR2_ADDR)
		f_addr -= STBINFO_SECTOR_LEN;

	/* find next stbinfo addr*/
	if(f_addr + (UINT32)buff_len + 8 > STBINFO_SECTOR_LEN)/*include Start&End flag length */
	{
		if(addr>STBINFO_SECTOR2_ADDR)
		{
			erase_stbinfo_sector(STBINFO_SECTOR1_ADDR);
			MEMSET(g_stbinfo_buf+STBINFO_SECTOR1_ADDR, 0xff, STBINFO_SECTOR_LEN);
			f_addr =_get_stbinfo_sector_free_addr(STBINFO_SECTOR1_ADDR);
			STBINFO_PRINTF("%s, %d, f_addr 0x%08x\n", __FUNCTION__, __LINE__, f_addr);
			f_addr += STBINFO_SECTOR1_ADDR;
		}
		else
		{
			erase_stbinfo_sector(STBINFO_SECTOR2_ADDR);
			MEMSET(g_stbinfo_buf+STBINFO_SECTOR2_ADDR, 0xff, STBINFO_SECTOR_LEN);
			f_addr =_get_stbinfo_sector_free_addr(STBINFO_SECTOR2_ADDR);
			STBINFO_PRINTF("%s, %d, f_addr 0x%08x\n", __FUNCTION__, __LINE__, f_addr);
			f_addr += STBINFO_SECTOR2_ADDR;
		}
		stbinfo_f_addr = f_addr;
	}
	else
		f_addr = addr;

	STBINFO_PRINTF("%s, %d, f_addr 0x%08x\n", __FUNCTION__, __LINE__, f_addr);

	/* if stbinfo sector is blank,the free addr is 0 */
	ret = _write_stbinfo_data(f_addr,(UINT8 *)&start_id,sizeof(UINT32));
	if(ret < 0)
	{
		STBINFO_PRINTF("Err1, %s, %d, cannot write stbinfo data, ret %d\n", __FUNCTION__, __LINE__, ret);
		return ret;
	}
	
	ret = _write_stbinfo_data(f_addr+4,buff,buff_len);
	if(ret < 0)
	{
		STBINFO_PRINTF("Err2, %s, %d, cannot write stbinfo data, ret %d\n", __FUNCTION__, __LINE__, ret);
		return ret;
	}
	
	ret = _write_stbinfo_data(f_addr+buff_len+4,(UINT8 *)&end_id,sizeof(UINT32));
	if(ret < 0)
	{
		STBINFO_PRINTF("Err3, %s, %d, cannot write stbinfo data, ret %d\n", __FUNCTION__, __LINE__, ret);
		return ret;
	}

	STBINFO_PRINTF("%s, %d, f_addr 0x%08x, addr 0x%08x\n", __FUNCTION__, __LINE__, f_addr, addr);

	/* erase sector */
	if(addr != f_addr)
	{
		/*free stbinfo buf befor erase sector*/
		if(g_stbinfo_buf!=NULL)
		{
			FREE(g_stbinfo_buf);
			g_stbinfo_buf = NULL;
		}

		if(addr>=STBINFO_SECTOR2_ADDR)
			erase_stbinfo_sector(STBINFO_SECTOR2_ADDR);
		else
			erase_stbinfo_sector(STBINFO_SECTOR1_ADDR);		
	}

	STBINFO_PRINTF("%s, %d, f_addr 0x%08x, addr 0x%08x\n", __FUNCTION__, __LINE__, f_addr, addr);

	if(f_addr >= STBINFO_SECTOR2_ADDR)
		f_addr -= STBINFO_SECTOR_LEN;
	else
		f_addr += STBINFO_SECTOR_LEN;

	ret = _write_stbinfo_data(f_addr,(UINT8 *)&start_id,sizeof(UINT32));
	if(ret < 0)
	{
		STBINFO_PRINTF("Err1, %s, %d, cannot write stbinfo data, ret %d\n", __FUNCTION__, __LINE__, ret);
		return ret;
	}
	
	ret = _write_stbinfo_data(f_addr+4,buff,buff_len);
	if(ret < 0)
	{
		STBINFO_PRINTF("Err2, %s, %d, cannot write stbinfo data, ret %d\n", __FUNCTION__, __LINE__, ret);
		return ret;
	}
	
	ret = _write_stbinfo_data(f_addr+buff_len+4,(UINT8 *)&end_id,sizeof(UINT32));
	if(ret < 0)
	{
		STBINFO_PRINTF("Err3, %s, %d, cannot write stbinfo data, ret %d\n", __FUNCTION__, __LINE__, ret);
		return ret;
	}
	
	return ret;
}


INT32 load_stbinfo(UINT8 *buff,UINT16 buff_len)
{
	INT32 t_addr, h_addr,f_addr;
	UINT32 sector_addr;

	h_addr = _get_stbinfo_free_addr(buff_len);
	f_addr = h_addr;
	STBINFO_PRINTF("%s, %d, f_addr 0x%08x, buff_len %d\n", __FUNCTION__, __LINE__, f_addr, buff_len);
	if(f_addr>=STBINFO_SECTOR2_ADDR)
	{
		f_addr -= STBINFO_SECTOR_LEN;
		sector_addr = STBINFO_SECTOR2_ADDR;
	}
	else
	{
		sector_addr = STBINFO_SECTOR1_ADDR;
	}
	if (STBINFO_SECTOR_LEN < f_addr) 
	{
		STBINFO_PRINTF("_get_stbinfo_free_addr(): free address not enough or invalid.\n");
		return LOAD_STB_INFO_FAILED;
	}
	STBINFO_PRINTF("stbinfo free addr = %08x\n", h_addr);
	/*
	 * h_addr is not always the next position for ending ID.
	 * since reported from YST.
	 */
	do {
		/*
		 * looking for ending ID from tail to the position
		 * where only 1 valid entry could write.
		 */
		t_addr = stbinfo_backward_locate(h_addr-4, sector_addr+buff_len+4, STB_INFO_END_ID);
		STBINFO_PRINTF("tail address=%08x\n",t_addr);
		if (t_addr<0) 
		{
			STBINFO_PRINTF("valid tail not found!\n");
			return LOAD_STB_INFO_FAILED;
		}
		/*
		 * looking for starting ID from tail-4
		 */
		h_addr = stbinfo_backward_locate(t_addr-4, sector_addr, STB_INFO_START_ID);
		STBINFO_PRINTF("head address=%08x\n", h_addr);
		if (h_addr<0) 
		{
			STBINFO_PRINTF("valid head not found!\n");
			return LOAD_STB_INFO_FAILED;
		}
	}while (t_addr-h_addr != buff_len+4); /* at the correct position.*/
	return _read_stbinfo_data(h_addr+4,buff,buff_len);

}

INT32 get_stbinfo_len()
{
	UINT32 sector1_addr,sector2_addr;
	struct sto_device *sto_flash_dev = NULL;
	INT32 h_addr1,t_addr1,h_addr2,t_addr2;
	INT32 h_addr,t_addr;
	
	/*load one stbinfo to memory, then search free addr*/
	if(g_stbinfo_buf==NULL)
	{
		g_stbinfo_buf = (UINT8 *)MALLOC(g_stbinfo_len);
		if(g_stbinfo_buf==NULL)
		{
			STBINFO_PRINTF("%s(): malloc fail!\n",__FUNCTION__);
			return LOAD_STB_INFO_FAILED;
		}
		MEMSET(g_stbinfo_buf, 0xff, g_stbinfo_len);

		if(load_stbinfo_buf(g_stbinfo_buf, g_stbinfo_len)<0)
			return LOAD_STB_INFO_FAILED;
	}
		
	sector1_addr =_get_stbinfo_sector_free_addr(STBINFO_SECTOR1_ADDR);
	sector2_addr =_get_stbinfo_sector_free_addr(STBINFO_SECTOR2_ADDR);

	h_addr = t_addr = -1;
	
	if(sector1_addr != 0)
	{
		h_addr1 = sector1_addr + STBINFO_SECTOR1_ADDR;
		t_addr1 = stbinfo_backward_locate(h_addr1-4, STBINFO_SECTOR1_ADDR+4, STB_INFO_END_ID);
		if(t_addr1>0)
		{
			h_addr1 = stbinfo_backward_locate(t_addr1-4, STBINFO_SECTOR1_ADDR, STB_INFO_START_ID);
			if(h_addr1>=0)
			{
				h_addr = h_addr1;
				t_addr = t_addr1;
			}
		}
		
	}
	
	if(sector2_addr != 0)
	{
		h_addr2 = sector2_addr + STBINFO_SECTOR2_ADDR;
		t_addr2 = stbinfo_backward_locate(h_addr2-4, STBINFO_SECTOR2_ADDR+4, STB_INFO_END_ID);
		if(t_addr2>0 && (t_addr<0 || t_addr2<t_addr))
		{
			h_addr2 = stbinfo_backward_locate(t_addr2-4, STBINFO_SECTOR2_ADDR, STB_INFO_START_ID);
			if(h_addr2>=0)
			{
				h_addr = h_addr2;
				t_addr = t_addr2;
			}
		}
	}

	if(h_addr<0 || t_addr<0)
		return -1;
	else
		return (t_addr-h_addr-4);

}

INT32 get_stbinfo_f_addr()
{
	INT32 ret = -1;
	INT32 stbinfo_len;
	INT32 f_addr;

	if(stbinfo_f_addr < 0)
		return -1;
	
	stbinfo_len = get_stbinfo_len();
	if(stbinfo_len<0)
	{
		STBINFO_PRINTF("%s:%d, not find stbinfo\n", __FUNCTION__, __LINE__);
		return -1;
	}	

	f_addr = stbinfo_f_addr - stbinfo_len - 4;
	
	if(f_addr < 0)
		return -1;
	
	return (g_stbinfo_base_addr+f_addr);
}

/*load stbinfo to memory*/
int load_stbinfo_buf(UINT8 *buf, UINT32 len)
{
	struct sto_device *sto_flash_dev = NULL;
	
	if(buf==NULL)
		return LOAD_STB_INFO_FAILED;

	if(len != g_stbinfo_len)
		return LOAD_STB_INFO_FAILED;
	
	sto_flash_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
	if(sto_get_data(sto_flash_dev, buf, g_stbinfo_base_addr, (INT32)g_stbinfo_len) != (INT32)g_stbinfo_len)
		return LOAD_STB_INFO_FAILED;

	return 1;
}

INT32 init_stb_info(UINT32 base_addr, UINT32 len)
{
	if(g_stbinfo_base_addr != 0 && g_stbinfo_len != 0)
		return SUCCESS;

	g_stbinfo_base_addr = base_addr;
	g_stbinfo_len = len;

	return SUCCESS;
}



