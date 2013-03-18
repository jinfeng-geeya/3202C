/****************************************************************************
*
*  ALi (Shanghai) Corporation, All Rights Reserved. 2003 Copyright (C)
*
*  File: db_3l.h
*
*  Description: The head file of program database
*              
*  History:
*      Date        	Author         	Version   		Comment
*      ====       	 ======        =======   	=======
*  1.  2006.03.31  Robbin Han     0.1.000   	Initial
* 
****************************************************************************/
#include <sys_config.h>
#include <types.h>
#include <hld/sto/sto.h>
#include <hld/sto/sto_dev.h>
#include <api/libc/printf.h>
#include <osal/osal.h>
#include <api/libc/string.h>

#include <api/libdb/db_return_value.h>
#include <api/libdb/db_config.h>
#include "db_basic_operation.h"

#define BO_PRINTF	PRINTF



static UINT32 bo_header_addr = 0xFFFFFFFF;
UINT32 g_db_base_addr = 0xFFFFFFFF;
UINT32 g_db_length = 0;

/*free addr in header index sector*/
static UINT32 bo_head_free_addr = 0xFFFFFFFF;
/*free addr in data sector*/
static UINT32 bo_data_free_addr = 0xFFFFFFFF;

/*current using data sector*/
static UINT8 bo_cur_data_sec_index = 0xFF;
/*header info sector */
static UINT8 bo_head_sec_index = 0xFF;

/*sector list to buffer each sector information*/
//static struct BO_Sec_Info bo_sec_list[BO_MAX_SECTOR_NUM];
static struct BO_Sec_Info* bo_sec_list = NULL;

/*free sector list, store the free sector index,bo_free_sec_list[0] is the number */
static UINT8 bo_free_sec_list[BO_MAX_SECTOR_NUM+1];

#define CUR_DATA_SEC_INDEX	(bo_cur_data_sec_index)
#define HEAD_SEC_INDEX	(bo_head_sec_index)


#define DB_SECTOR_NUM	(UINT8)(g_db_length/DB_SECTOR_SIZE)
#define DATA_FREE_ADDR	(bo_data_free_addr)
#define HEAD_FREE_ADDR	(bo_head_free_addr)



static struct sto_device *sto_flash_dev=NULL;

#if defined(DB_MULTI_SECTOR)

UINT32 BO_get_max_logicnum(UINT8 *sec_index)
{
	UINT32 max_logic_num = 0;
	UINT8 i = 0;
			
	for(;i < DB_SECTOR_NUM; i++)
	{
		if((bo_sec_list[i].type == BO_TYPE_DATA) && (bo_sec_list[i].status == BO_SS_VALID ||bo_sec_list[i].status == BO_SS_SELECT)
			&& (bo_sec_list[i].logic_number > max_logic_num))				
			max_logic_num = bo_sec_list[i].logic_number;
	}
	if(max_logic_num > 0)
	{
		for(i = 0;i < DB_SECTOR_NUM; i++)
		{
			if(max_logic_num == bo_sec_list[i].logic_number)
			break;
		}
		*sec_index = i ;
		return max_logic_num;

	}
	else
	{
		*sec_index = 0xFF;
		return 0xFFFFFFFF;
	}

}


INT32 BO_get_free_sector(UINT8 *sec_index)
{
	UINT8 sec_num = bo_free_sec_list[0];
	UINT8 i = 1;
	
	if(sec_num > BO_RECLAIM_THRESHOLD)
	{
		*sec_index = bo_free_sec_list[1];
		for(; i < sec_num; i++)
			bo_free_sec_list[i] = bo_free_sec_list[i+1];
		
		bo_free_sec_list[0] -= 1;
		return DB_SUCCES;
	}
	else 
		return DBERR_BO_THRESHOLD;

}

UINT8 BO_get_reserve_sec()
{
	UINT8 sec_idx, i;
	
	//if(bo_free_sec_list[0] == BO_RECLAIM_THRESHOLD)
	if(bo_free_sec_list[0] > 0)
	{
		sec_idx = bo_free_sec_list[1];
		for(i = 1; i < bo_free_sec_list[0]; i++)
			bo_free_sec_list[i] = bo_free_sec_list[i+1];
		
		bo_free_sec_list[0] -= 1;		
		return sec_idx;
	}
	else
		DB_ASSERT;

}

UINT8 BO_get_header_sec()
{
	return bo_head_sec_index;

}

INT32 BO_set_header_sec(UINT8 sec_idx)
{
	if(sec_idx != 0xFF)
	{
		bo_head_sec_index = sec_idx;
		return DB_SUCCES;
	}
	else
		return DBERR_PARAM;
}

UINT8 BO_get_cur_data_sec( )
{
	return bo_cur_data_sec_index;
	
}

INT32 BO_set_cur_data_sec(UINT8 sec_idx)
{
	if(sec_idx != 0xFF)
	{
		bo_cur_data_sec_index = sec_idx;
		return DB_SUCCES;
	}
	else
		return DBERR_PARAM;
}


static INT32 BO_add_to_free_list(UINT8 sec_index)
{
	UINT8 sec_num = bo_free_sec_list[0];
    int i;
	if(sec_index >= DB_SECTOR_NUM)
	{
		BO_PRINTF("BO_add_to_free_list(): invalid param!\n");
		return DBERR_PARAM;
	}
    for( i = 1; i <= sec_num; i++)
    {
        if(bo_free_sec_list[i] == sec_index)
            return DB_SUCCES;
    }
    
	bo_free_sec_list[sec_num+1] = sec_index;
	bo_free_sec_list[0] += 1;
	return DB_SUCCES;

}



INT32 BO_read_sec_info(UINT8 sec_idx, struct BO_Sec_Info *sec_info)
{
	UINT32 sector_index = sec_idx;
	
	if(sec_idx == 0xFF)
		return DBERR_PARAM;
	return BO_read_data((sector_index<<NODE_ADDR_OFFSET_BIT), sizeof(struct BO_Sec_Info), (UINT8 *)sec_info);

}

INT32 BO_write_sec_info(UINT8 sec_idx, struct BO_Sec_Info *sec_info)
{
	UINT32 sector_index = sec_idx;
	
	if(sec_idx == 0xFF)
		return DBERR_PARAM;
	return BO_flash_write((UINT8 *)sec_info, sizeof(struct BO_Sec_Info), (sector_index<<NODE_ADDR_OFFSET_BIT));

}

void BO_set_sec_list(UINT8 sec_idx, struct BO_Sec_Info sec_info)
{
	MEMCPY(&bo_sec_list[sec_idx], &sec_info, sizeof(struct BO_Sec_Info));
}


static INT32 BO_sector_diagnose( UINT8 sec_idx, struct BO_Sec_Info *sec_info)
{
	UINT32 i, addr, tmp;
	INT32 ret = DB_SUCCES;
	UINT32 sector_idx = sec_idx;
	UINT8 sector_err = 0;
	
	if(((sec_info->status == BO_SS_VALID) || (sec_info->status == BO_SS_SPARE)) && (sec_info->valid_flag == BO_VALID_FLAG))
	{
		return DB_SUCCES;
	}
	//be selected as reclaim source sector, then maybe power loss
	else if((sec_info->status == BO_SS_SELECT)&& (sec_info->valid_flag == BO_VALID_FLAG))
	{
		BO_PRINTF("BO_sector_diagnose():power loss after change sector %d status to select, now change its status back to valid!\n ",sector_idx);
		return DB_SUCCES;
	}
	//as reclaim dest sector, during data copying, maybe power loss
	else if((sec_info->status == BO_SS_COPYING) && (sec_info->valid_flag == BO_VALID_FLAG))
	{
		BO_PRINTF("BO_sector_diagnose(): power loss during copy data to sector %d, now erase it!\n",sector_idx);
		ret = BO_erase_sector(sector_idx);
	}
	else if(sec_info->type == BO_TYPE_UNKNOWN &&sec_info->status == BO_SS_UNKNOWN
					&& sec_info->logic_number == 0xFFFFFFFF 
					&& sec_info->erase_count == 0xFFFFFFFF
					&& sec_info->valid_flag == 0xFF)
	{	//check last 16 bytes of the sector 
		addr = ((sector_idx+1)<<NODE_ADDR_OFFSET_BIT)-16;
		for(i = 0; i < 16; i += 4)
		{	
			BO_read_data(addr+i, 4, (UINT8 *)&tmp);
			if(tmp != 0xFFFFFFFF)
			{	
				sector_err = 1;
				break;
			}
		}
		//power loss during erasing
		if(sector_err == 1)
		{
			BO_PRINTF("BO_sector_diagnose():power loss during erase sector %d, now erase it!\n ",sector_idx);
			ret = BO_erase_sector(sector_idx);
		}
		//first init after buring flash
		else
		{
			sec_info->status = BO_SS_SPARE;
			sec_info->erase_count = 0;
			sec_info->valid_flag = BO_VALID_FLAG;
			ret = BO_flash_write((UINT8 *)sec_info, sizeof(struct BO_Sec_Info), sector_idx<<NODE_ADDR_OFFSET_BIT);
		}
	}
	//other errors
	else
	{
		BO_PRINTF("BO_sector_diagnose(): unknown error in sector %d, now erase it!\n",sector_idx);
		sec_info->erase_count = 0;
		ret = BO_erase_sector(sector_idx);
	}
	return ret;
}



#endif

UINT8 BO_get_sec_num()
{
	return DB_SECTOR_NUM;
}


static INT32 BO_get_free_addr_in_sector(UINT8 sec_index, UINT8 search_type,UINT8 mode, UINT32 *free_addr)
{
	UINT32 start_addr= 0;
	UINT32 end_addr= 0;
	UINT32 tmp;
	UINT32 sector_idx = sec_index;
	UINT32 tmp_free;
	UINT8 tmp2[4];
	UINT8 i, flag;
	UINT32 address = 0;

	if(mode == BO_SEARCH_FROM_START)
	{
		start_addr = (sector_idx <<NODE_ADDR_OFFSET_BIT) + BO_SECTOR_INFO_SIZE;
		end_addr = ((sector_idx + 1) <<NODE_ADDR_OFFSET_BIT) -4;
	}
	else if(mode == BO_SEARCH_FROM_END)
	{
		start_addr = ((sector_idx + 1) <<NODE_ADDR_OFFSET_BIT) -4;
		end_addr = (sector_idx <<NODE_ADDR_OFFSET_BIT) + BO_SECTOR_INFO_SIZE;
	}

	if(mode == BO_SEARCH_FROM_START)
	{
		while(start_addr < end_addr)
		{
			if(BO_read_data(start_addr,4, (UINT8 *)&tmp) != DB_SUCCES)
				return DBERR_BO_READ;
			
			if(tmp == 0xFFFFFFFF)
			{
				flag = 0;
				//check next 12 byte if 0xFF
				for(i=1;i<4; i++)
				{
					address = start_addr + i*4;
					if((address<end_addr)&&(DB_SUCCES==BO_read_data(address,4, (UINT8 *)&tmp)))
						if(tmp != 0xFFFFFFFF)
						{
							start_addr = address + 4;
							flag = 1;
							break;
						}
				}
				if(flag==0)
				{
					if(BO_read_data(end_addr,4, (UINT8 *)&tmp)==DB_SUCCES)
						if(tmp!=0xFFFFFFFF)
							return DBERR_BO_INIT;
						
					tmp_free = start_addr;
					//check if tmp_free is at sector info end
					if((search_type == BO_SEARCH_OPERATION) &&
						(tmp_free-4 >= ((sector_idx<<NODE_ADDR_OFFSET_BIT)+BO_SECTOR_INFO_SIZE)))
					{
						//check if the 3 bytes before is 0xFF
						BO_read_data(tmp_free-4,4, tmp2);
						for(i = 0; i < 4; i++)
						{
							if(tmp2[3-i] != 0xFF)
							{
								*free_addr = tmp_free - i;
								return DB_SUCCES;
							}
						}
					}
					*free_addr = tmp_free;
					return DB_SUCCES;
				}
			}
			else
				start_addr += 4;
		}
		*free_addr = end_addr;
		return DB_SUCCES;
	}
	else if(mode == BO_SEARCH_FROM_END)
	{
		while(start_addr > end_addr)
		{
			if(BO_read_data(start_addr,4, (UINT8 *)&tmp) != DB_SUCCES)
				return DBERR_BO_READ;
			
			if(tmp == 0xFFFFFFFF)
			{
				flag = 0;
				//check next 12 byte if 0xFF
				for(i=1;i<4; i++)
				{
					address = start_addr - i*4;
					if((address>end_addr)&&(DB_SUCCES==BO_read_data(address,4, (UINT8 *)&tmp)))
						if(tmp != 0xFFFFFFFF)
						{
							start_addr = address - 4;
							flag = 1;
							break;
						}
				}
				if(flag==0)
				{
					tmp_free = start_addr + 4;
					*free_addr = tmp_free;
					return DB_SUCCES;
				}
			}
			else
				start_addr -= 4;
		}
		*free_addr = end_addr;
		return DB_SUCCES;
	}
}



INT32 BO_flash_write(UINT8 *data, UINT32 len, UINT32 addr)
{
	//struct sto_device *sto_dev = NULL;
	INT32 ret = DB_SUCCES;
	INT32 bytes_write = 0;
	
	if(addr+len > g_db_length)
	{
		BO_PRINTF("BO_flash_write(): invalid addr!\n");	
		return DBERR_PARAM;
	}
	
	//sto_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
		
	bytes_write = sto_put_data(sto_flash_dev, addr+g_db_base_addr, data, (INT32)len);
	if(bytes_write == (INT32)len)
		return DB_SUCCES;
	else
	{
		BO_PRINTF("BO_flash_write(): sto_write error at addr %x, %d bytes write!\n",addr,bytes_write);
		return DBERR_BO_WRITE;
	}

}


INT32 BO_set_head_freeaddr(UINT32 addr)
{
	if(addr != 0xFFFFFFFF)
	{
		bo_head_free_addr = addr;
		return DB_SUCCES;
	}
	else
		return DBERR_PARAM;
}


INT32 BO_set_header_addr(UINT32 addr)
{
	if(addr != 0xFFFFFFFF)
	{
		bo_header_addr = addr;
		return DB_SUCCES;
	}
	else
		return DBERR_PARAM;
}

INT32 BO_set_data_freeaddr(UINT32 addr)
{
	if(addr != 0xFFFFFFFF)
	{
		bo_data_free_addr = addr;
		return DB_SUCCES;
	}
	else
		return DBERR_PARAM;
}



INT32 BO_get_header(UINT32 *head_addr, UINT16 *node_num,UINT32 *free_addr)
{
	UINT32 start_addr= 0;
	UINT32 end_addr = 0;
	UINT32 tmp;
	UINT16 num = 0;
	UINT8 i, flag1;
	UINT32 head_start_flag = BO_HEADER_START;
	UINT32 head_end_flag = BO_HEADER_END;
	UINT32 sec_idx = HEAD_SEC_INDEX;

	if(bo_header_addr != 0xFFFFFFFF)
	{
		*head_addr = bo_header_addr;
		BO_read_data(bo_header_addr+4, 2, (UINT8 *)node_num);
		*free_addr = HEAD_FREE_ADDR;
		return DB_SUCCES;
	}


	start_addr = (sec_idx <<NODE_ADDR_OFFSET_BIT) + BO_SECTOR_INFO_SIZE;
	end_addr = HEAD_FREE_ADDR;

	if(end_addr == start_addr)
	{
		UINT8 empty_head[10];
		MEMCPY(empty_head, (UINT8 *)&head_start_flag,4);
		MEMSET(&empty_head[4], 0, 2);
		MEMCPY(&empty_head[6], (UINT8 *)&head_end_flag,4);
		if(BO_flash_write(empty_head, 10, start_addr)!= DB_SUCCES)
			return DBERR_BO_BAD_HEADER;
		else
		{
			HEAD_FREE_ADDR = start_addr + 10;
			*head_addr = start_addr;
			*node_num = 0;
			*free_addr = HEAD_FREE_ADDR;
			return DB_SUCCES;
		}

	}

	while(start_addr < end_addr)
	{
		flag1 = 1;
		
		if(BO_read_data(start_addr,4, (UINT8 *)&tmp) != DB_SUCCES)
				return DBERR_BO_READ;
		for(i = 0;i < 4; i++)			
		{
			if(*((UINT8 *)&tmp + i) != *((UINT8 *)&head_start_flag + i))
			{
				flag1 = 0;
				break;
			}
		}

		if(flag1 == 1)
		{
			if(BO_read_data(start_addr+4, 2, (UINT8 *)&num) != DB_SUCCES)
				return DBERR_BO_READ;

			if(BO_read_data(start_addr+4+2+num*(NODE_ID_SIZE+NODE_ADDR_SIZE), 4, (UINT8 *)&tmp) != DB_SUCCES)
				return DBERR_BO_READ;

			if(tmp == BO_HEADER_END)
			{
				*head_addr = bo_header_addr = start_addr;
				*node_num = num;
				*free_addr = end_addr;
				return DB_SUCCES;
			}
		
		}
		else 
			start_addr++;
	}

	return DBERR_BO_BAD_HEADER;

}


INT32 BO_read_data(UINT32 addr, UINT32 len, UINT8 *data)
{
	//struct sto_device *sto_dev = NULL;
	
	if(addr + len > g_db_length)
	{
		BO_PRINTF("BO_read_data(): invalid addr!\n");	
		return DBERR_PARAM;
	}
	
	if(sto_get_data(sto_flash_dev, data, addr+g_db_base_addr, (INT32)len) == (INT32)len)
		return DB_SUCCES;
	else
	{
		BO_PRINTF("BO_read_data(): sto_read error at addr %x\n",addr);
		return DBERR_BO_READ;
	}

}


INT32 BO_write_data(UINT8 *data, UINT32 len, UINT32 *addr)
{
	UINT32 data_free_addr = DATA_FREE_ADDR;


	/*db length single sector*/
	if(DB_SECTOR_NUM == 1)
	{
		if(data_free_addr - len <= HEAD_FREE_ADDR)
		{
			BO_PRINTF("BO_write_data():single sector, no eough space to write data!\n");
			return DBERR_BO_NEED_RACLAIM;

		}
		
		if(BO_flash_write(data, len, data_free_addr - len) != DB_SUCCES)
		{
			BO_PRINTF("BO_write_data():single sector, write data %d bytes to addr %x failed!\n ",len,data_free_addr-len);
			return DBERR_BO_WRITE;
		}
			 
		DATA_FREE_ADDR = data_free_addr - len;
		*addr = DATA_FREE_ADDR;
	}

	/*db length multi sector*/
	else
	{
#if defined(DB_MULTI_SECTOR)
		/*data will over sector boundary*/
		if((data_free_addr >> NODE_ADDR_OFFSET_BIT) != ((data_free_addr + len) >> NODE_ADDR_OFFSET_BIT))
		{
			UINT32 tmp = 0;
			UINT32 fill_len = len - ((data_free_addr + len) % DB_SECTOR_SIZE);
			UINT8 i;
			UINT8 free_sec_index;

			/*fill 0 to the sector boundary*/
			for(i=0;(fill_len>=4)&&( i < (fill_len/4)); i++)
				BO_flash_write((UINT8 *)&tmp,4,data_free_addr+i*4);
			DATA_FREE_ADDR = data_free_addr + i*4;
			if(BO_get_free_sector(&free_sec_index) == DBERR_BO_THRESHOLD)
			{
				BO_PRINTF("BO_write_data():nulti sector, write %d bytes to addr %x to threshold, need reclaim!\n ",len,data_free_addr);
				return DBERR_BO_NEED_RACLAIM;
			}
			UINT32 sec_idx = free_sec_index;
			data_free_addr = (sec_idx<<NODE_ADDR_OFFSET_BIT) + BO_SECTOR_INFO_SIZE;

			/*change free sector info: type, status, logical num*/
			struct BO_Sec_Info *sec_info = &bo_sec_list[free_sec_index];
			UINT8 data_sec_index;
			UINT32 max_logic_num = BO_get_max_logicnum(&data_sec_index);
			
			sec_info->type = BO_TYPE_DATA;
			sec_info->status = BO_SS_VALID;
			if(max_logic_num == 0xFFFFFFFF)
			{
				BO_PRINTF("BO_write_data():nulti sector,get max logic number failed!\n");
				return DBERR_BO_WRITE;
			}
			sec_info->logic_number = max_logic_num + 1;
			if(BO_flash_write((UINT8 *)sec_info, sizeof(struct BO_Sec_Info), (UINT32)free_sec_index<<NODE_ADDR_OFFSET_BIT) != DB_SUCCES)
			{
				BO_PRINTF("BO_write_data():nulti sector,change sector info failed!\n ");
				return DBERR_BO_WRITE;
			}
			CUR_DATA_SEC_INDEX = free_sec_index;

		}
		
		if(BO_flash_write(data, len, data_free_addr) != DB_SUCCES)
		{
			BO_PRINTF("BO_write_data():nulti sector,write data %d bytes to addr %x failed!\n ",len,data_free_addr);
			return DBERR_BO_WRITE;
		}
		*addr = data_free_addr;
		DATA_FREE_ADDR = data_free_addr + len;
#endif
	}

	return DB_SUCCES;

}



INT32 BO_write_operation(UINT8 *data, UINT32 len, UINT32 *addr)
{
	UINT32 head_free_addr = HEAD_FREE_ADDR;

	/*db length single sector*/
	if(DB_SECTOR_NUM == 1)
	{
		if(head_free_addr + len >= DATA_FREE_ADDR)
		{
			BO_PRINTF("BO_write_operation():single sector, no space to write head op!\n");
			return DBERR_BO_NEED_RACLAIM;
		}
	}
	/*db length multi sector*/
	else
	{
#if defined(DB_MULTI_SECTOR)
		if((head_free_addr >> NODE_ADDR_OFFSET_BIT) != ((head_free_addr + len) >> NODE_ADDR_OFFSET_BIT))
		{
			BO_PRINTF("BO_write_operation():multi sector, write %d bytes to addr %x to sector end, need reclaim!\n ",len,head_free_addr);
			return DBERR_BO_NEED_RACLAIM;
		}
#endif
	}
		
	if(BO_flash_write(data, len, head_free_addr) != DB_SUCCES)
	{
		BO_PRINTF("BO_write_operation():write head op %d bytes to addr %x failed!\n\n ",len,head_free_addr);
		return DBERR_BO_WRITE;
	}
	*addr = head_free_addr; 
	HEAD_FREE_ADDR = head_free_addr + len;
	
	return DB_SUCCES;
}


INT32 BO_erase_sector(UINT8 sec_index)
{
	//struct sto_device * sto_dev = NULL;
	struct BO_Sec_Info *sec_info = &bo_sec_list[sec_index];
	UINT32 sector_idx = sec_index;
	UINT32 sector_size = DB_SECTOR_SIZE;

	UINT32 param;
	UINT32 flash_cmd;
	if(sto_flash_dev->totol_size <= 0x400000)
	{
		param = (sector_idx <<NODE_ADDR_OFFSET_BIT) + g_db_base_addr;

		param = (param << 10) + (sector_size >> 10);
		flash_cmd = STO_DRIVER_SECTOR_ERASE;
	}
	else
	{
		UINT32 tmp_param[2];
		tmp_param[0] = (sector_idx <<NODE_ADDR_OFFSET_BIT) + g_db_base_addr;
		tmp_param[1] =  (sector_size >> 10);
		param = (UINT32)tmp_param;
		flash_cmd = STO_DRIVER_SECTOR_ERASE_EXT;

	}
	//sto_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);

	/*change the sector status to BO_SS_ERASING*/
	//sec_info->status = BO_SS_ERASING;
	//if(BO_flash_write((UINT8 *)sec_info, sizeof(struct BO_Sec_Info), (UINT32)(sec_index<<NODE_ADDR_OFFSET_BIT)) != DB_SUCCES)
	//{
	//	BO_PRINTF("BO_erase_sector(): change sector %d status to erasing failed!\n",sec_index);
	//	return DBERR_BO_ERASE;
	//}

	/*begin to erase*/
	if(sto_io_control(sto_flash_dev, flash_cmd, param) != SUCCESS)
	{
		BO_PRINTF("BO_erase_sector(): Erase flash sector %d failed!\n",sec_index);
		return DBERR_BO_ERASE;
	}

	/*after erasing, change sector status and erase_count, set valid flag*/
	UINT32 erase_cnt = sec_info->erase_count;

	MEMSET((UINT8 *)sec_info,0xFF,sizeof(struct BO_Sec_Info));
	sec_info->status = BO_SS_SPARE;
	sec_info->erase_count = erase_cnt+1;
	sec_info->valid_flag = BO_VALID_FLAG;
	if(BO_flash_write((UINT8 *)sec_info, sizeof(struct BO_Sec_Info), (UINT32)(sec_index<<NODE_ADDR_OFFSET_BIT)) != DB_SUCCES)
	{
		BO_PRINTF("BO_erase_sector(): change sector %d status to spare failed!\n",sec_index);
		return DBERR_BO_ERASE;
	}

#if defined(DB_MULTI_SECTOR)
	/*add the erased sector into free list*/
	if(BO_add_to_free_list(sec_index) != DB_SUCCES)
	{
		BO_PRINTF("BO_erase_sector(): add sector %d into free list failed!\n",sec_index);
		return DBERR_BO_ERASE;
	}
#endif
	return DB_SUCCES;
}

UINT32 BO_get_db_data_space()
{
	UINT32 data_space;
	UINT8 sec_num;
	
	
	if(DB_SECTOR_NUM == 1)
	{
		data_space = DB_SECTOR_SIZE;
		//16 byte as margin, max node number header len(include 10 byte header start ,node_num, end)
		data_space -= BO_SECTOR_INFO_SIZE + 16 + (MAX_SAT_NUM+MAX_TP_NUM+MAX_PROG_NUM)*sizeof(NODE_IDX_PARAM)+10;
	}
	else
	{	// 1sector for header and operation
		sec_num = DB_SECTOR_NUM - BO_RECLAIM_THRESHOLD - 1;
		//128 byte as margin for each sector
		data_space = sec_num*(DB_SECTOR_SIZE - BO_SECTOR_INFO_SIZE-128);
	}
	return data_space;
}


INT32 BO_init(UINT32 db_base_addr, UINT32 db_length)
{
	INT32 ret = DB_SUCCES;
	UINT32 head_free_addr;
	UINT32 data_free_addr;
	UINT8 i = 0;
	
	UINT8 data_sec_cnt = 0;
	UINT8 free_sec_cnt = 0;
	UINT32 unknow_sec_cnt = 0;
	UINT8 free_sec_idx;

	if(sto_flash_dev == NULL)
	{
		sto_flash_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
		if(sto_flash_dev == NULL)
		{
			BO_PRINTF("BO_init(): dev_get_by_id failed!\n");
			return DBERR_BO_INIT;
		}
		if (sto_open(sto_flash_dev) != SUCCESS)
		{
			PRINTF("BO_init(): sto_open failed!\n");
			return DBERR_BO_INIT;
		}
	}

	g_db_base_addr =  db_base_addr;
	g_db_length = db_length;

	if(bo_sec_list == NULL)
	{
		bo_sec_list = (struct BO_Sec_Info*)MALLOC(DB_SECTOR_NUM *sizeof(struct BO_Sec_Info));
		if(bo_sec_list == NULL)
		{
			PRINTF("BO_init(): Malloc bo_sec_list memory failed!\n");
			return DBERR_BO_INIT;
		}
	}

	MEMSET(bo_sec_list, 0x00, sizeof(struct BO_Sec_Info)*DB_SECTOR_NUM);
	MEMSET(bo_free_sec_list, 0, BO_MAX_SECTOR_NUM+1);

	if(DB_SECTOR_NUM == 1)
	{
		HEAD_SEC_INDEX = CUR_DATA_SEC_INDEX = 0;
	}
	else
	{
#if defined(DB_MULTI_SECTOR)
		for(;i < DB_SECTOR_NUM; i++)
			BO_read_data(((UINT32)i)<<NODE_ADDR_OFFSET_BIT, sizeof(struct BO_Sec_Info), (UINT8 *)&bo_sec_list[i]);
			
		for(i = 0; i < DB_SECTOR_NUM; i++)
		{	
			//here can diagnose db sectors, find out power loss, esrase unfinished etc. 
			ret = BO_sector_diagnose(i, &bo_sec_list[i]);
			if(ret != DB_SUCCES)
				return ret;
			
			if(bo_sec_list[i].type == BO_TYPE_HEAD && bo_sec_list[i].valid_flag == BO_VALID_FLAG)
				HEAD_SEC_INDEX = i;
			else if(bo_sec_list[i].type == BO_TYPE_DATA && bo_sec_list[i].valid_flag == BO_VALID_FLAG)
				data_sec_cnt += 1;
			else if(bo_sec_list[i].type == BO_TYPE_UNKNOWN && bo_sec_list[i].status == BO_SS_SPARE
					&& bo_sec_list[i].valid_flag == BO_VALID_FLAG)
			{
				free_sec_cnt += 1;
				BO_add_to_free_list(i);
			}
			else if(bo_sec_list[i].type == BO_TYPE_UNKNOWN && bo_sec_list[i].status == BO_SS_UNKNOWN
					&& bo_sec_list[i].logic_number == 0xFFFFFFFF 
					&& bo_sec_list[i].erase_count == 0xFFFFFFFF
					&& bo_sec_list[i].valid_flag == 0xFF)
			{
				bo_sec_list[i].status = BO_SS_SPARE;
				bo_sec_list[i].erase_count = 0;
				bo_sec_list[i].valid_flag = BO_VALID_FLAG;
				if(BO_flash_write((UINT8 *)&bo_sec_list[i], sizeof(struct BO_Sec_Info), (UINT32)i<<NODE_ADDR_OFFSET_BIT) != DB_SUCCES)
				{
					BO_PRINTF("BO_init(): change sector %d status to spare failed!\n",i);
					return DBERR_BO_INIT;
				}
				free_sec_cnt += 1;
				BO_add_to_free_list(i);
			}
		}

		if(free_sec_cnt == DB_SECTOR_NUM)
		{
			BO_get_free_sector(&free_sec_idx);
			HEAD_SEC_INDEX = free_sec_idx;
			if(DBERR_BO_THRESHOLD==BO_get_free_sector(&free_sec_idx))
				return DBERR_BO_INIT;
			CUR_DATA_SEC_INDEX = free_sec_idx;
			
			bo_sec_list[HEAD_SEC_INDEX].type = BO_TYPE_HEAD;
			bo_sec_list[HEAD_SEC_INDEX].status = BO_SS_VALID;
			if(BO_flash_write((UINT8 *)&bo_sec_list[HEAD_SEC_INDEX], sizeof(struct BO_Sec_Info), (UINT32)(HEAD_SEC_INDEX<<NODE_ADDR_OFFSET_BIT)) != DB_SUCCES)
			{
				BO_PRINTF("BO_init(): set sector 0  to head failed!\n");
				return DBERR_BO_INIT;
			}
			bo_sec_list[CUR_DATA_SEC_INDEX].type = BO_TYPE_DATA;
			bo_sec_list[CUR_DATA_SEC_INDEX].status = BO_SS_VALID;
			bo_sec_list[CUR_DATA_SEC_INDEX].logic_number = 1;
			if(BO_flash_write((UINT8 *)&bo_sec_list[CUR_DATA_SEC_INDEX], sizeof(struct BO_Sec_Info), (UINT32)(CUR_DATA_SEC_INDEX<<NODE_ADDR_OFFSET_BIT)) != DB_SUCCES)
			{
				BO_PRINTF("BO_init(): set sector 1  to data failed!\n");
				return DBERR_BO_INIT;
			}
			
		}
		else if(BO_get_max_logicnum(&bo_cur_data_sec_index) == 0xFFFFFFFF)
		{
			BO_PRINTF("BO_init(): get current data sec index failed!\n");
			return DBERR_BO_INIT;
		}
#endif
	}

	/*addr init*/
	if(HEAD_FREE_ADDR == 0xFFFFFFFF)
	{
		if(BO_get_free_addr_in_sector(HEAD_SEC_INDEX, BO_SEARCH_OPERATION,BO_SEARCH_FROM_START, &head_free_addr) != DB_SUCCES)
		{
			BO_PRINTF("BO_init(): init head free addr failed!\n");
			return DBERR_BO_INIT;
		}
		HEAD_FREE_ADDR = head_free_addr;

	}
	if(DATA_FREE_ADDR == 0xFFFFFFFF)
	{
		if(DB_SECTOR_NUM == 1)
		{
			if(BO_get_free_addr_in_sector(CUR_DATA_SEC_INDEX, BO_SEARCH_DATA,BO_SEARCH_FROM_END, &data_free_addr) != DB_SUCCES)
			{
				BO_PRINTF("BO_init(): init data free addr failed!\n");
				return DBERR_BO_INIT;
			}
		}
		else
		{
			if(BO_get_free_addr_in_sector(CUR_DATA_SEC_INDEX, BO_SEARCH_DATA,BO_SEARCH_FROM_START, &data_free_addr) != DB_SUCCES)
			{
				BO_PRINTF("BO_init(): init data free addr failed!\n");
				return DBERR_BO_INIT;
			}
		}

		DATA_FREE_ADDR = data_free_addr;
		
	}
	
	return DB_SUCCES;

}

INT32 BO_reset( )
{
	bo_header_addr = 0xFFFFFFFF;
	bo_head_free_addr = 0xFFFFFFFF;
	bo_data_free_addr = 0xFFFFFFFF;

	if(bo_sec_list != NULL)
	{	
		FREE(bo_sec_list);
		bo_sec_list = NULL;
	}
	
#if defined(DB_MULTI_SECTOR)
	bo_cur_data_sec_index = 0xFF;
	bo_head_sec_index = 0xFF;
#endif

	return DB_SUCCES;

}


