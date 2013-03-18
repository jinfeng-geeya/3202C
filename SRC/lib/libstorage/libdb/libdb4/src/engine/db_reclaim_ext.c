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

#include <api/libdb/db_config.h>
#include <api/libdb/db_return_value.h>
#include "db_basic_operation.h"
#include "db_data_object.h"

#define RECLAIM_PRINTF	PRINTF

extern DB_VIEW db_view;
extern DB_TABLE db_table[3];
extern DB_CMD_BUF db_cmd_buf;

#if defined(DB_MULTI_SECTOR)
INT32 DB_reclaim_operation()
{
	struct BO_Sec_Info sec_info;
	UINT8 header_sec, reclaim_sec;
	INT32 ret = DB_SUCCES;
	UINT32 addr;
	UINT32 tmp;
	UINT16 node_num = 0;
	UINT16 i;
	DB_TABLE *table = (DB_TABLE *)&db_table[0];
	

	//get header sector, change its status
	/*
	header_sec = BO_get_header_sec();
	if(header_sec == 0xFF)
	{
		RECLAIM_PRINTF("DB_reclaim_operation(): header sector index invalid!\n");
		return DBERR_RECLAIM;
	}
	*/
	RECLAIM_PRINTF("DB_reclaim_operation()!\n");
	
	header_sec = BO_get_header_sec();
	BO_read_sec_info(header_sec, &sec_info);
	sec_info.status = BO_SS_SELECT;
	BO_set_sec_list(header_sec, sec_info);
	ret = BO_write_sec_info(header_sec, &sec_info);
	if(ret != DB_SUCCES)
	{
		RECLAIM_PRINTF("write header sector %d info to SELECT failed!\n",header_sec);
		return DBERR_RECLAIM;
	}

#ifdef DB_RAM_BACKUP
	if(DB_SUCCES != BO_erase_sector(header_sec))
		return DBERR_RECLAIM;
	BO_get_free_sector(&reclaim_sec);
#else
	//find reclaim sector, change its type,status
	if(DBERR_BO_THRESHOLD == BO_get_free_sector(&reclaim_sec))
		reclaim_sec = BO_get_reserve_sec();
#endif
	
	BO_read_sec_info(reclaim_sec, &sec_info);
	sec_info.type = BO_TYPE_HEAD;
	sec_info.status = BO_SS_COPYING;
	BO_set_sec_list(reclaim_sec, sec_info);
	ret = BO_write_sec_info(reclaim_sec, &sec_info);
	if(ret != DB_SUCCES)
	{
		RECLAIM_PRINTF("write reclaim sector %d info to COPYING failed!\n",reclaim_sec);
		return DBERR_RECLAIM;
	}

	//write table header into flash
	RECLAIM_PRINTF("begin to write all table header to flash!\n");
	tmp = reclaim_sec;
	addr = (tmp<<NODE_ADDR_OFFSET_BIT) + BO_SECTOR_INFO_SIZE;

	BO_set_header_addr(addr);
	tmp = BO_HEADER_START;
	BO_flash_write((UINT8 *)&tmp, 4, addr);
	addr += 4;
	for(i = 0; i < DB_TABLE_NUM; i++)
	{
		node_num += table[i].node_num;
	}
	BO_flash_write((UINT8 *)&node_num, 2, addr);
	addr += 2;
	
	for(i = 0; i < DB_TABLE_NUM; i++)
	{
		BO_flash_write((UINT8 *)table[i].table_buf,  table[i].node_num*sizeof(NODE_IDX_PARAM), addr);
		addr += table[i].node_num*sizeof(NODE_IDX_PARAM);
	}
	tmp = BO_HEADER_END;
	BO_flash_write((UINT8 *)&tmp, 4, addr);

	//change reclaim sector status VALID
	BO_read_sec_info(reclaim_sec, &sec_info);
	sec_info.status = BO_SS_VALID;
	BO_set_sec_list(reclaim_sec, sec_info);
	BO_write_sec_info(reclaim_sec, &sec_info);
	BO_set_header_sec(reclaim_sec);
	BO_set_head_freeaddr(addr + 4);

#ifndef DB_RAM_BACKUP
	//change old head sector status ERASING
	BO_read_sec_info(header_sec, &sec_info);
	sec_info.status = BO_SS_ERASING;
	BO_set_sec_list(header_sec, sec_info);
	BO_write_sec_info(header_sec, &sec_info);

	//erase old header sector
	if(DB_SUCCES != BO_erase_sector(header_sec))
		return DBERR_RECLAIM;
#endif
	return DB_SUCCES;	
}


INT32 DB_reclaim_write_header()
{
	UINT32 tmp;
	UINT16 node_num = 0;
	UINT16 i;
	DB_TABLE *table = (DB_TABLE *)&db_table[0];
	UINT8 head_sec, tmp_sec;
	INT32 ret = DB_SUCCES;
	UINT32 addr = 0;
	UINT32 old_head_addr, old_free_addr;
	UINT16 old_node_num;
	
	for(i = 0; i < DB_TABLE_NUM; i++)
	{
		node_num += table[i].node_num;

	}
	BO_get_header(&old_head_addr,&old_node_num,&old_free_addr);
	head_sec = BO_get_header_sec();
	tmp_sec = (UINT8)((old_free_addr + node_num * (NODE_ID_SIZE+NODE_ADDR_SIZE)+ 10)>>NODE_ADDR_OFFSET_BIT);
	if(tmp_sec != head_sec)
	{
		ret = DB_reclaim_operation();
		if(ret != DB_SUCCES)
		{
			RECLAIM_PRINTF("DB_reclaim_write_header() failed!\n");
			return DBERR_RECLAIM;
		}
		return ret;
	
	}
	else
	{
		//BO_get_header(&old_head_addr,&old_node_num,&old_free_addr);
		addr = old_free_addr;
		BO_set_header_addr(addr);
		tmp = BO_HEADER_START;
		BO_flash_write((UINT8 *)&tmp, 4, addr);
		addr += 4;
		BO_flash_write((UINT8 *)&node_num, 2, addr);
		addr += 2;
		for(i = 0; i < DB_TABLE_NUM; i++)
		{
			BO_flash_write((UINT8 *)table[i].table_buf,  table[i].node_num*sizeof(NODE_IDX_PARAM), addr);
			addr += table[i].node_num*sizeof(NODE_IDX_PARAM);
		}
		tmp = BO_HEADER_END;
		BO_flash_write((UINT8 *)&tmp, 4, addr);
		BO_set_head_freeaddr(addr + 4);

		tmp = 0;
		BO_flash_write((UINT8 *)&tmp, 4, old_head_addr);
		BO_flash_write((UINT8 *)&tmp, 4, old_head_addr+old_node_num*(NODE_ID_SIZE+NODE_ADDR_SIZE)+6);

		return DB_SUCCES;
	}

}



INT32 DB_reclaim_data( )
{
	UINT16 i,j,k;
	UINT32 node_addr = 0;
	UINT8 reclaim_sec_idx = 0xFF;
	UINT8 target_sec_idx = 0xFF;
	/*
	UINT8 sec_idx = 0xFF;
	UINT16 node_num[BO_MAX_SECTOR_NUM][DB_TABLE_NUM];
	UINT16 node_num_in_sec = 0;
	UINT16 min_node_num_in_sec = 0;
	*/
	struct BO_Sec_Info sec_info;
	DB_TABLE *table = (DB_TABLE *)&db_table[0];
	DB_VIEW *view = (DB_VIEW *)&db_view;
	DB_CMD_BUF *cmd_buf = (DB_CMD_BUF *)&db_cmd_buf;

	INT32 ret = DB_SUCCES;
	UINT32 addr = 0;
	UINT8 node[DB_NODE_MAX_SIZE];
	UINT32 node_len;
	UINT8 node_type = TYPE_UNKNOWN_NODE;
	UINT8 find = 0xFF;
	UINT8 cur_data_sec;
	UINT8 db_sector_num = 0;
	UINT32 max_logic_num = 0;
	INT32 sector_node_len[BO_MAX_SECTOR_NUM];
	INT32 sector_cmdbuf_node_len[BO_MAX_SECTOR_NUM];
	INT32 min_node_len_sec = 0;
	INT32 sec_node_len = 0;
	INT32 tmp;
	
	//compute table node num in each data sector
	
/*
	MEMSET((UINT8 *)node_num, 0, sizeof(node_num));
	for(i = 0; i < DB_TABLE_NUM; i++)
	{
		for(j = 0; j < table[i].node_num; j++)
		{
			MEMCPY((UINT8 *)&node_addr, table[i].table_buf[j].node_addr, NODE_ADDR_SIZE);
			sec_idx = (UINT8)(node_addr >> NODE_ADDR_OFFSET_BIT);
			node_num[sec_idx][i] += 1;
		}
	}

	//find the data sector with min node num
	cur_data_sec = BO_get_cur_data_sec();
	for(j = 0; j < DB_TABLE_NUM; j++)
			min_node_num_in_sec += node_num[cur_data_sec][j];
	db_sector_num = BO_get_sec_num();
	for(i = 0; i < db_sector_num; i++)
	{
		BO_read_sec_info((UINT8)i, &sec_info);
		if((sec_info.type == BO_TYPE_DATA) && ((sec_info.status == BO_SS_VALID)||(sec_info.status == BO_SS_SELECT)))
		{
			for(j = 0; j < DB_TABLE_NUM; j++)
				node_num_in_sec += node_num[i][j];
			if(node_num_in_sec <= min_node_num_in_sec)
			{
				min_node_num_in_sec = node_num_in_sec;
				reclaim_sec_idx = (UINT8)i;
			}
			if(sec_info.logic_number > max_logic_num)
				max_logic_num = sec_info.logic_number;
		}
		node_num_in_sec = 0;
	}

for(i = 0; i < db_sector_num; i++)
	libc_printf("sector %d, prog num=%d, tp num=%d, sat num= %d, node_len=%d\n",
	i, node_num[i][0], node_num[i][1], node_num[i][2],sector_node_len[i]);
*/
	MEMSET(sector_node_len, 0, sizeof(sector_node_len));
	MEMSET(sector_cmdbuf_node_len, 0, sizeof(sector_cmdbuf_node_len));
	DO_get_table_node_length(BO_get_sec_num(), &sector_node_len[0]);
	DO_get_cmdbuf_node_len(BO_get_sec_num(), &sector_cmdbuf_node_len[0]);
	db_sector_num = BO_get_sec_num();
	cur_data_sec = BO_get_cur_data_sec();
	min_node_len_sec = sector_node_len[cur_data_sec]+sector_cmdbuf_node_len[cur_data_sec];
	for(i = 0; i < db_sector_num; i++)
	{
		BO_read_sec_info((UINT8)i, &sec_info);
		if((sec_info.type == BO_TYPE_DATA) && ((sec_info.status == BO_SS_VALID)||(sec_info.status == BO_SS_SELECT)))
		{
			sec_node_len = sector_node_len[i]+sector_cmdbuf_node_len[i];
			if(sec_node_len <= min_node_len_sec)
			{
				min_node_len_sec = sec_node_len;
				reclaim_sec_idx = (UINT8)i;
			}
			if(sec_info.logic_number > max_logic_num)
				max_logic_num = sec_info.logic_number;
		}
	}
	
	if(reclaim_sec_idx == 0xFF)
	{
		RECLAIM_PRINTF("DB_reclaim_data(): not find reclaim data sector!\n");
		return DBERR_RECLAIM;
	}

	//max_logic_num = BO_get_max_logicnum(&max_logicnum_sec);
	tmp = sector_node_len[reclaim_sec_idx];
	//change reclaim sector info, select a target sector
	BO_read_sec_info(reclaim_sec_idx, &sec_info);
	sec_info.status = BO_SS_SELECT;
	BO_set_sec_list(reclaim_sec_idx, sec_info);
	ret = BO_write_sec_info(reclaim_sec_idx, &sec_info);
	if(ret != DB_SUCCES)
	{
		RECLAIM_PRINTF("write reclaim sector %d info to SELECT failed!\n",reclaim_sec_idx);
		return DBERR_RECLAIM;
	}
	
#ifdef DB_RAM_BACKUP
	UINT32 ram_buf = (UINT32)DO_get_update_buf();
	UINT32 ram_node_addr = 0;

	if(((UINT8 *)ram_buf)==NULL)
		return DBERR_RECLAIM;
	addr = ((UINT32)(reclaim_sec_idx))<<NODE_ADDR_OFFSET_BIT;
	ret = BO_read_data(addr, DB_SECTOR_SIZE, (UINT8 *)ram_buf);
	if(ret != DB_SUCCES)
		return ret;
	if(DB_SUCCES != BO_erase_sector(reclaim_sec_idx))
			return DBERR_RECLAIM;
	BO_get_free_sector(&target_sec_idx);
	
#else
	if(DBERR_BO_THRESHOLD == BO_get_free_sector(&target_sec_idx))
		target_sec_idx = BO_get_reserve_sec();
#endif

	BO_read_sec_info(target_sec_idx, &sec_info);
	sec_info.type = BO_TYPE_DATA;
	sec_info.logic_number = max_logic_num+1;
	sec_info.status = BO_SS_COPYING;
	BO_set_sec_list(target_sec_idx, sec_info);
	ret = BO_write_sec_info(target_sec_idx, &sec_info);
	if(ret != DB_SUCCES)
	{
		RECLAIM_PRINTF("write target sector %d info to COPYING failed!\n",target_sec_idx);
		return DBERR_RECLAIM;
	}
	//write each node in reclaim sector into target sector, update its addr in table
	addr = (((UINT32)target_sec_idx)<<NODE_ADDR_OFFSET_BIT) + BO_SECTOR_INFO_SIZE;
	
	for(i = 0; i < DB_TABLE_NUM; i++)
	{
		for(j = 0; j < table[i].node_num; j++)
		{
			if(reclaim_sec_idx == table[i].table_buf[j].node_addr[NODE_ADDR_SIZE-1])
			{
				MEMCPY((UINT8 *)&node_addr, table[i].table_buf[j].node_addr, NODE_ADDR_SIZE);
			#ifdef DB_RAM_BACKUP
				ram_node_addr = ram_buf + (node_addr&0xFFFF);
				node_len = *((UINT8 *)(ram_node_addr+NODE_ID_SIZE))+NODE_ID_SIZE;
				BO_flash_write((UINT8 *)ram_node_addr, node_len, addr);
			#else
				BO_read_data(node_addr,NODE_ID_SIZE+1, node);
				node_len = node[NODE_ID_SIZE]+NODE_ID_SIZE;
				BO_read_data(node_addr,node_len, node);
				BO_flash_write((UINT8 *)node, node_len, addr);
			#endif
				
				MEMCPY(table[i].table_buf[j].node_addr, (UINT8 *)&addr, NODE_ADDR_SIZE);
			
				addr += node_len;
			}
		}
	}
	/*
	INT32 tmp_len = addr-(addr&0xFFFF0000)-BO_SECTOR_INFO_SIZE;
	if(tmp_len != sector_node_len[reclaim_sec_idx])
		libc_printf("**reclaim table node, reclaim sector[%d],target sector[%d],node len[%d] not equal the global record[%d]!\n",
		reclaim_sec_idx, target_sec_idx,tmp_len,sector_node_len[reclaim_sec_idx]);
	*/
	//write the node(whose addr in reclaim sector) in cmd buf into target sector, update its addr in cmd buf 
	for(i = 0; i < cmd_buf->cmd_cnt; i++)
	{
		if((cmd_buf->buf[i].op_type != OPERATION_DEL)&& (reclaim_sec_idx == cmd_buf->buf[i].node_addr[NODE_ADDR_SIZE-1]))
		{
			MEMCPY((UINT8 *)&node_addr, cmd_buf->buf[i].node_addr, NODE_ADDR_SIZE);
		#ifdef DB_RAM_BACKUP
			ram_node_addr = ram_buf + (node_addr&0xFFFF);
			node_len = *((UINT8 *)(ram_node_addr+NODE_ID_SIZE))+NODE_ID_SIZE;
			BO_flash_write((UINT8 *)ram_node_addr, node_len, addr);
		#else
			BO_read_data(node_addr,NODE_ID_SIZE+1, node);
			node_len = node[NODE_ID_SIZE]+NODE_ID_SIZE;
			BO_read_data(node_addr,node_len, node);
			BO_flash_write((UINT8 *)node, node_len, addr);
		#endif
			MEMCPY(cmd_buf->buf[i].node_addr, (UINT8 *)&addr, NODE_ADDR_SIZE);
		
			addr += node_len;
		}
	}
	/*
	INT32 tmp_len2 = addr-(addr&0xFFFF0000)-BO_SECTOR_INFO_SIZE-tmp_len;
	if(tmp_len2 != sector_cmdbuf_node_len[reclaim_sec_idx])
		libc_printf("**reclaim cmd buf, reclaim sector[%d],target sector[%d],node len[%d] not equal the global record[%d]!\n",
		reclaim_sec_idx, target_sec_idx,tmp_len2,sector_cmdbuf_node_len[reclaim_sec_idx]);
	*/
	sector_node_len[target_sec_idx] = tmp;
	sector_cmdbuf_node_len[target_sec_idx] = sector_cmdbuf_node_len[reclaim_sec_idx];
	if(target_sec_idx != reclaim_sec_idx)
	{
		sector_node_len[reclaim_sec_idx] = 0;
		sector_cmdbuf_node_len[reclaim_sec_idx] = 0;
	}
	ret = DO_set_table_node_length(BO_get_sec_num(), &sector_node_len[0]);

	//write a new header after data reclaim
	ret = DB_reclaim_write_header();
	if(ret != SUCCESS)
	{
		RECLAIM_PRINTF("after data copying from reclaim sector %d to target sector %d, write new header failed!\n",reclaim_sec_idx,target_sec_idx);
		return DBERR_RECLAIM;
	}	

	//change target sector status to valid
	BO_read_sec_info(target_sec_idx, &sec_info);
	sec_info.status = BO_SS_VALID;
	BO_set_sec_list(target_sec_idx, sec_info);
	BO_write_sec_info(target_sec_idx, &sec_info);
	
	BO_set_cur_data_sec(target_sec_idx);
	BO_set_data_freeaddr(addr);

#ifndef DB_RAM_BACKUP
	//change reclaim sector status ERASING
	BO_read_sec_info(reclaim_sec_idx, &sec_info);
	sec_info.status = BO_SS_ERASING;
	BO_set_sec_list(reclaim_sec_idx, sec_info);
	BO_write_sec_info(reclaim_sec_idx, &sec_info);

	//erase reclaim sector
	if(DB_SUCCES != BO_erase_sector(reclaim_sec_idx))
		return DBERR_RECLAIM;
#endif

	node_type = view->cur_type;
	//update node addr in view
	for(i = 0; i < view->node_num; i++)
	{
		find = 0xFF;
		//MEMCPY((UINT8 *)&node_id, view->node_buf[i].node_id, NODE_ID_SIZE);
		//node_type = DO_get_node_type(node_id);
		
		//update the addr of node in view that in reclaim sector
		if(view->node_buf[i].node_addr[NODE_ADDR_SIZE-1] == reclaim_sec_idx)
		{
			//check if the node exist in table
			for(j = 0; j < table[node_type].node_num; j++)
			{
				if(0 == MEMCMP(view->node_buf[i].node_id, table[node_type].table_buf[j].node_id, NODE_ID_SIZE))
				{
					find = 1;
					break;
				}
			}
			//check if the node exist in cmd buf
			for(k = 0; k < cmd_buf->cmd_cnt; k++)
			{
				if((cmd_buf->buf[k].op_type != OPERATION_DEL)
					&& (0 == MEMCMP(view->node_buf[i].node_id,cmd_buf->buf[k].node_id, NODE_ID_SIZE)))
				{
					find = 2;
					break;
				}
			}

			if(find == 1)
				MEMCPY(view->node_buf[i].node_addr, table[node_type].table_buf[j].node_addr, NODE_ADDR_SIZE);
			else if(find == 2)
				MEMCPY(view->node_buf[i].node_addr, cmd_buf->buf[k].node_addr, NODE_ADDR_SIZE);
			else
			{
				RECLAIM_PRINTF("node at pos %d in view not found in table or cmd buf!\n",i);
				return DBERR_RECLAIM;
			}
		}
	}
	
	return DB_SUCCES;

}

#else

INT32 DB_reclaim_write_header()
{
	INT32 ret = DB_SUCCES;

	
	return ret;
}


#endif

INT32 DB_reclaim(UINT8 mode)
{
	INT32 ret = DB_SUCCES;
	//multi sector reclaim
#if defined(DB_MULTI_SECTOR)
	if(mode == DB_RECLAIM_OPERATION)
		ret = DB_reclaim_operation();
	else if(mode == DB_RECLAIM_DATA)
		ret = DB_reclaim_data();
#else
	//single sector reclaim


#endif
	return ret;

}



