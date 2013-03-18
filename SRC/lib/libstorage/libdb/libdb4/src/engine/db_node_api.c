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
#include "db_data_object.h"
#include "db_reclaim.h"
#include "db_basic_operation.h"



#define NODE_API_PRINTF	PRINTF


extern DB_TABLE db_table[DB_TABLE_NUM];


INT32 DB_clear_cmd_buf()
{
	return DO_clear_cmd_buf();
}


INT32 DB_regist_packer_unpacker(node_pack_t packer, node_unpack_t unpacker)
{
	return DO_regist_packer_unpacker(packer, unpacker);
}

INT32 DB_set_update_buf(UINT8 * buf)
{
	return DO_set_update_buf(buf);
}

INT32 DB_create_view(UINT8 type, DB_VIEW *v_attr,db_filter_t filter, db_filter_pip pip_filter, UINT16 filter_mode, UINT32 filter_param)
{
	return DO_create_view(type,v_attr,filter, pip_filter,  filter_mode,filter_param);
}

INT32 DB_update_operation()
{
	return DO_update_operation();
}



INT32 DB_get_node_by_pos_from_table(DB_TABLE *table, UINT16 pos,UINT32 *node_id, UINT32 *node_addr)
{
	//UINT32 node_addr = 0, node_id = 0;
	
	if(pos >= table->node_num)
	{
		NODE_API_PRINTF("DB_get_node_by_pos_from_table():pos %d exceed node num %d!\n",pos, table->node_num);
		return DBERR_PARAM;
	}

	*node_id = 0;
	*node_addr = 0;
	MEMCPY((UINT8 *)node_addr, table->table_buf[pos].node_addr, NODE_ADDR_SIZE);
	MEMCPY((UINT8 *)node_id,  table->table_buf[pos].node_id, NODE_ID_SIZE);
/*	
	if(SUCCESS != DO_read_node(node_id, node_addr,node_len, node))
	{
		NODE_API_PRINTF("DB_get_node_by_pos_from_table(): read node failed!\n");
		return DBERR_PACK;
	}
*/
	return DB_SUCCES;

}

INT32 DB_get_node_by_id_from_table(DB_TABLE *table, UINT32 node_id,UINT32 *node_addr)
{
	//UINT32 node_addr = 0;
	UINT16 i;

	*node_addr = 0;
	for(i = 0; i < table->node_num; i++)
	{
		if(0 == MEMCMP((UINT8 *)&node_id, table->table_buf[i].node_id, NODE_ID_SIZE))
		{
			MEMCPY((UINT8 *)node_addr, table->table_buf[i].node_addr,NODE_ADDR_SIZE);
			break;
		}
	}
	if(i == table->node_num)
	{
		NODE_API_PRINTF("DB_get_node_by_id_from_table(): node id %d not find!\n",node_id);
		return DBERR_API_NOFOUND;
	}
/*
	if(SUCCESS != DO_read_node(node_id, node_addr,node_len, node))
	{
		NODE_API_PRINTF("DB_get_node_by_id_from_table():read node failed!\n");
		return DBERR_PACK;
	}
*/
	return DB_SUCCES;

}


INT32 DB_get_node_by_pos(DB_VIEW *v_attr, UINT16 pos, UINT32 *node_id, UINT32 *node_addr )
{
	//UINT32 node_addr = 0;
	//UINT32 node_id = 0;
	
	if(pos >= v_attr->node_num)
	{
		NODE_API_PRINTF("DB_get_node_by_pos(): pos %d exceed view node num %d!\n",pos, v_attr->node_num);
		return DBERR_PARAM;
	}

	*node_id = 0;
	*node_addr = 0;
	MEMCPY((UINT8 *)node_addr, v_attr->node_buf[pos].node_addr, NODE_ADDR_SIZE);
	MEMCPY((UINT8 *)node_id, v_attr->node_buf[pos].node_id, NODE_ID_SIZE);
/*	
	if(SUCCESS != DO_read_node(node_id, node_addr,node_len, node))
	{
		NODE_API_PRINTF("DB_get_node_by_pos(): node_id = %d failed!\n",node_id);
		return DBERR_PACK;
	}
*/
	return DB_SUCCES;
}



INT32 DB_get_node_by_id(DB_VIEW *v_attr, UINT32 id, UINT32 *node_addr)
{
	UINT16 i;
		
	*node_addr = 0;
	for(i = 0; i < v_attr->node_num; i++)
	{
		if(0 == MEMCMP((UINT8 *)&id, v_attr->node_buf[i].node_id, NODE_ID_SIZE))
		{
			MEMCPY((UINT8 *)node_addr, v_attr->node_buf[i].node_addr, NODE_ADDR_SIZE);
			break;
		}
	}

	if(*node_addr == 0)
	{
		NODE_API_PRINTF("DB_get_node_by_id(): node id %d not find!\n",id);
		return DBERR_API_NOFOUND;
	}
/*
	if(SUCCESS != DO_read_node(id, node_addr,node_len, node))
	{
		NODE_API_PRINTF("DB_get_node_by_id(): node_id = %d failed!\n",id);
		return DBERR_PACK;
	}
*/
	return DB_SUCCES;

}

void DB_get_node_id_addr_by_pos(UINT8 view_dismatch,DB_TABLE *table, DB_VIEW *view, UINT16 pos,UINT32 *id, UINT32 *addr)
{
	if(view_dismatch == 1)
		DB_get_node_by_pos_from_table(table, pos, id, addr);
	else
		DB_get_node_by_pos(view, pos, id, addr);

}


INT32 DB_read_node(UINT32 node_id, UINT32 addr, UINT8 *node,UINT32 len)
{
	return DO_read_node(node_id, addr, len, node);
}

INT32 DB_add_node_to_table(DB_TABLE *table, UINT32 node_id, UINT8 *node, UINT32 node_len)
{
	INT32 ret = DB_SUCCES;
	UINT32 ret_addr = 0;
	//UINT32 bytes_write = 0;
	
	//write node into flash
	//ret = DO_write_node(node_id, node, node_len, &ret_addr,&bytes_write);
	
	ret = BO_write_data(node,node_len, &ret_addr);
	if(ret != DB_SUCCES)
	{
		NODE_API_PRINTF("DB_add_node_to_table(): write node(id,%d) to flash failed!\n",node_id);
		return ret;
	}
	ret = DO_t_add_node(table, ret_addr,node_id);

	return ret;
}



static INT32 DB_assign_node_id(DB_VIEW *v_attr, UINT8 n_type, UINT32 parent_id, UINT32 *node_id)
{
	UINT16 i,j;
	UINT32 id_flag[32];
	UINT32 tmp;
	UINT8 id_flag_num;
	UINT32 find_id = 0xFFFFFFFF;
	//fro each type node, find the corresponding id bits max value
	UINT32 max_id = 0;
	UINT32 tmp2 = 0;
	UINT32 id_max_bit_value = 0;
			
	MEMSET(id_flag, 0, sizeof(id_flag));
	for(i = 0; i < v_attr->node_num; i++)
	{
		MEMCPY(&tmp2, v_attr->node_buf[i].node_id, NODE_ID_SIZE);
		if(n_type == TYPE_SAT_NODE)
		{
			if(((tmp2&NODE_ID_SET_MASK) != 0) &&(v_attr->node_buf[i].node_id[NODE_ID_SIZE-1] != 0xFF))
			{
				tmp2 = tmp2>>(NODE_ID_TP_BIT+NODE_ID_PG_BIT);
				id_flag[(tmp2>>5)] |= (1<<(tmp2%32));
				if(tmp2 > max_id)
					max_id = tmp2;
			}
		}
		else if(n_type == TYPE_TP_NODE)
		{
			if((tmp2&NODE_ID_SET_MASK)== parent_id)
			{
				tmp2 = (tmp2&TP_BIT_MASK)>>NODE_ID_PG_BIT;
				id_flag[(tmp2>>5)] |= (1<<(tmp2%32));
				if(tmp2 > max_id)
					max_id = tmp2;
			}
		}
		else if(n_type == TYPE_PROG_NODE)
		{
			if((tmp2&NODE_ID_TP_MASK)== parent_id)
			{
				tmp2 = tmp2&PG_BIT_MASK;
				id_flag[(tmp2>>5)] |= (1<<(tmp2%32));
				if(tmp2 > max_id)
					max_id = tmp2;
			}
		}

	}
	NODE_API_PRINTF("DB_assign_node_id(): n_type=%d node's max id bits=%x!\n",n_type,max_id);

	if(n_type == TYPE_SAT_NODE)
		id_max_bit_value = SET_BIT_MASK>>(NODE_ID_TP_BIT+NODE_ID_PG_BIT);
	else if(n_type == TYPE_TP_NODE)
		id_max_bit_value = TP_BIT_MASK>>NODE_ID_PG_BIT;
	else if(n_type == TYPE_PROG_NODE)
		id_max_bit_value = PG_BIT_MASK;
	
	if(max_id < (id_max_bit_value - 2))
		find_id = max_id + 1;
	//if max id already 0xFE, find a empty id from beginning id 1
	else
	{
		if(n_type == TYPE_SAT_NODE)
			id_flag_num = 8;
		else 
			id_flag_num = 32;
		
		//first id flag 32 bit, id 0 reserved
		if(id_flag[0] != 0xFFFFFFFE)
		{
			for(i = 1; i < 32; i++)
			{
				tmp = (id_flag[0]&(1<<i));
				if(tmp == 0 )
				{
					find_id = i;
					break;
				}
			}
			
		}
		else
		{
			//for(i = 1; i < id_flag_num -1; i++)
			for(i = 1; i < id_flag_num ; i++)
			{
				for(j = 0; j < 32; j++)
				{
					tmp = (id_flag[i]&(1<<j));
					if(tmp == 0 )
					{
						find_id = i*32+j;
						break;
					}
				}
				if(find_id != 0xFFFFFFFF)
					break;
			}
		}
		
	}
	if(find_id > (id_max_bit_value - 1))
		return DBERR_ID_ASSIGN;
	
	if(find_id != 0xFFFFFFFF)
	{
		if(n_type == TYPE_SAT_NODE)
			*node_id = (find_id<<(NODE_ID_PG_BIT+NODE_ID_TP_BIT));
		else if(n_type == TYPE_TP_NODE)
			*node_id = parent_id | (find_id<<NODE_ID_PG_BIT);
		else if(n_type == TYPE_PROG_NODE)
			*node_id = parent_id | find_id;

		NODE_API_PRINTF("DB_assign_node_id():assign a id for n_type=%d node,id=%x!\n",n_type,*node_id);
		return DB_SUCCES;
	}
	
	NODE_API_PRINTF("DB_assign_node_id(): can not assign a id for n_type=%d node,its parent_id=%x!\n",n_type,parent_id);
	return DBERR_ID_ASSIGN;

}



INT32 DB_pre_add_node(DB_VIEW *v_attr, UINT8 n_type, UINT32 parent_id, UINT32 node_len, UINT32 *node_id)
{
	INT32 ret = DB_SUCCES;
	UINT32 ret_id = 0;
	UINT32 ret_addr = 0;
	INT32 table_node_len, cmdbuf_node_len;
	UINT32 bytes_write = 0;
	UINT16 max_num = 0;
	UINT16 node_num;
	INT32 sector_node_len[BO_MAX_SECTOR_NUM];
	INT32 sector_cmdbuf_node_len[BO_MAX_SECTOR_NUM];

	if(n_type == TYPE_SAT_NODE)
		max_num = MAX_SAT_NUM;
	else if(n_type == TYPE_TP_NODE)
		max_num = MAX_TP_NUM;
	else if(n_type == TYPE_PROG_NODE)
		max_num = MAX_PROG_NUM;
	
	node_num = DO_check_type_node_cnt(n_type);
	if(node_num >= max_num)
	{
		//node_num = max_num;
		NODE_API_PRINTF("DB_pre_add_node(): node num already to max cnt!\n");
		return DBERR_MAX_LIMIT;
	}
	
	//if flash db space is full
	MEMSET(sector_node_len, 0, sizeof(sector_node_len));
	MEMSET(sector_cmdbuf_node_len, 0, sizeof(sector_cmdbuf_node_len));
	table_node_len = DO_get_table_node_length(BO_MAX_SECTOR_NUM, &sector_node_len[0]);
	cmdbuf_node_len = DO_get_cmdbuf_node_len(BO_MAX_SECTOR_NUM, &sector_cmdbuf_node_len[0]);
	if(table_node_len + cmdbuf_node_len + (INT32)node_len >= (INT32)DO_get_db_data_space())
	{
		NODE_API_PRINTF(" DB_add_node():flash space full,can not add node, table_node_len=%d,cmdbuf_node_len=%d!\n",
			table_node_len,cmdbuf_node_len);
		return DBERR_FLASH_FULL;
	}

	//assign node id
	ret = DB_assign_node_id(v_attr, n_type, parent_id, &ret_id);
	if(ret != DB_SUCCES)
		return ret;
	
	*node_id = ret_id;
	return DB_SUCCES;	
	
}

INT32 DB_add_node(DB_VIEW *v_attr, UINT32 node_id,UINT8 *node, UINT32 node_len)
{
	INT32 ret = DB_SUCCES;
	UINT32 ret_addr = 0, bytes_write = 0;

	//write node into flash
	ret = DO_write_node(node_id, node, node_len, &ret_addr, &bytes_write);
	if(ret == DBERR_BO_NEED_RACLAIM)
	{
		//reclaim data
		ret = DB_reclaim(DB_RECLAIM_DATA);
		if(ret != DB_SUCCES)
			return ret;
		ret = DO_write_node(node_id, node, node_len, &ret_addr,&bytes_write);
		if(ret == DBERR_BO_NEED_RACLAIM)
		{
			ret = DO_update_operation();
			if(ret != DB_SUCCES)
				return ret;
			ret = DB_reclaim(DB_RECLAIM_DATA);
			if(ret != DB_SUCCES)
				return ret;
			ret = DO_write_node(node_id, node, node_len, &ret_addr, &bytes_write);
			if(ret == DBERR_BO_NEED_RACLAIM)
				return DBERR_FLASH_FULL;
		}
		else if(ret != DB_SUCCES)
			return ret;
	}
	else if(ret != DB_SUCCES)
		return ret;
	ret = DO_v_add_node(v_attr,ret_addr,node_id);
	if(ret == DBERR_CMDBUF_FULL)
	{
		//update, if need reclaim, reclaim operation
		ret = DO_update_operation();
		if(ret != DB_SUCCES)
			return ret;
		ret = DO_v_add_node(v_attr,ret_addr,node_id);
	}
	return ret;

}

INT32 DB_modify_node_by_id(DB_VIEW *v_attr, UINT32 id, UINT8 *node, UINT32 node_len)
{
	UINT16 i;
	UINT32 ret_addr = 0;
	INT32 ret = DB_SUCCES;
	UINT32 bytes_write = 0;

	for(i = 0; i< v_attr->node_num; i++)
	{
		if(0 == MEMCMP(v_attr->node_buf[i].node_id, (UINT8 *)&id, NODE_ID_SIZE))
			break;
	}
	if(i == v_attr->node_num)
	{
		NODE_API_PRINTF("v_attr->node_num: node id %d not find!\n",id);
		return DBERR_API_NOFOUND;
	}
	//perhaps need check flash space size
	
	//write node into flash
	ret = DO_write_node(id, node, node_len, &ret_addr, &bytes_write);
	if(ret == DBERR_BO_NEED_RACLAIM)
	{
		//reclaim data
		ret = DB_reclaim(DB_RECLAIM_DATA);
		if(ret != DB_SUCCES)
			return ret;
		ret = DO_write_node(id, node, node_len, &ret_addr, &bytes_write);
		if(ret == DBERR_BO_NEED_RACLAIM)
		{
			ret = DO_update_operation();
			if(ret != DB_SUCCES)
				return ret;
			ret = DB_reclaim(DB_RECLAIM_DATA);
			if(ret != DB_SUCCES)
				return ret;
			ret = DO_write_node(id, node, node_len, &ret_addr, &bytes_write);
			if(ret != DB_SUCCES)
				return ret;
		}
		else if(ret != DB_SUCCES)
			return ret;	
	}
	else if(ret != DB_SUCCES)
		return ret;
	ret = DO_v_modify_node(v_attr,i, ret_addr);
	if(ret == DBERR_CMDBUF_FULL)
	{
		//update, if need reclaim, reclaim operation
		ret = DO_update_operation();
		if(ret != DB_SUCCES)
			return ret;
		ret = DO_v_modify_node(v_attr,i, ret_addr);
	}
	return ret;
}




INT32 DB_del_node_by_pos(DB_VIEW *v_attr, UINT16 pos)
{
	INT32 ret = DB_SUCCES;
	
	ret = DO_v_del_node_by_pos(v_attr, pos);
	if(ret == DBERR_CMDBUF_FULL)
	{
		//update
		ret = DO_update_operation();
		if(ret != DB_SUCCES)
			return ret;
		ret = DO_v_del_node_by_pos(v_attr, pos);
	}
	return ret;
}

INT32 DB_del_node_by_id(DB_VIEW *v_attr, UINT32 id)
{
	INT32 ret = DB_SUCCES;
	UINT16 i;

	for(i = 0; i < v_attr->node_num; i++)
	{
		if(0 == MEMCMP(v_attr->node_buf[i].node_id, (UINT8 *)&id, NODE_ID_SIZE))
			break;

	}
	if(i == v_attr->node_num)
	{
		NODE_API_PRINTF("del_node_by_id(): not find node id = %d\n",id);
		return DBERR_API_NOFOUND;

	}
	ret = DB_del_node_by_pos(v_attr, i);
	
	return ret;

}


INT32 DB_del_node_by_parent_id(DB_VIEW *v_attr, UINT8 parent_type, UINT32 parent_id)
{
	UINT16 i;
	UINT32 id_mask = 0;
	UINT32 node_id = 0;
	INT32 ret = DB_SUCCES;

	if(parent_type == TYPE_SAT_NODE)
		id_mask = NODE_ID_SET_MASK;
	else if(parent_type == TYPE_TP_NODE)
		id_mask = NODE_ID_TP_MASK;
	
	for(i = 0; i < v_attr->node_num; )
	{
		MEMCPY((UINT8 *)&node_id, v_attr->node_buf[i].node_id, NODE_ID_SIZE);
		if((node_id&id_mask) == parent_id)
		{
			ret = DB_del_node_by_pos(v_attr, i);
			//each time del one node from view, the v_attr->node_num -1, the left nodes will be shifted ahead.
			if(ret != DB_SUCCES)
				return ret;			
		}
		else
			i++;
	}
	
	return ret;
}


#include <api/libdb/db_node_c.h>
extern struct dynamic_prog_back dynamic_program;
//modify the DB_del_node_by_parent_id() function as the special function which do not delete the programs in 
//the favorite groups and with lock flag...
INT32 DB_del_node_by_parent_id_except_fav_prog(DB_VIEW *v_attr, UINT8 parent_type, UINT32 parent_id)
{
	UINT16 i;
	UINT32 id_mask = 0;
	UINT32 node_id = 0;
	UINT32 node_addr = 0;
	INT32 ret = DB_SUCCES;
	P_NODE p_node;

	if(parent_type == TYPE_SAT_NODE)
		id_mask = NODE_ID_SET_MASK;
	else if(parent_type == TYPE_TP_NODE)
		id_mask = NODE_ID_TP_MASK;
	
	for(i = 0; i < v_attr->node_num; )
	{
		MEMCPY((UINT8 *)&node_id, v_attr->node_buf[i].node_id, NODE_ID_SIZE);
		MEMCPY((UINT8 *)&node_addr, v_attr->node_buf[i].node_addr, NODE_ADDR_SIZE);

		if(dynamic_program.prog.prog_id==node_id && dynamic_program.status==DYNAMIC_PG_STEAM_UPDATED)
			MEMCPY(&p_node, &(dynamic_program.prog), sizeof(P_NODE));
		else
			ret = DB_read_node(node_id, node_addr,(UINT8 *)&p_node, sizeof(P_NODE));

		if((node_id&id_mask) == parent_id && p_node.fav_group[0] == 0 && p_node.lock_flag == 0)
		{
			ret = DB_del_node_by_pos(v_attr, i);
			//each time del one node from view, the v_attr->node_num -1, the left nodes will be shifted ahead.
			if(ret != DB_SUCCES)
				return ret;			
		}
		else
			i++;
	}
	
	return ret;
}



INT32 DB_move_node(DB_VIEW * v_attr, UINT16 src_pos, UINT16 dest_pos)
{
	return DO_v_move_node(v_attr,src_pos,dest_pos);

}


#if 0
INT32 DB_read_data_from_flash(UINT32 addr, UINT32 len, UINT8 * data)
{
	return DO_read_data_from_flash(addr,len,data);

}
#endif

BOOL DB_check_node_changed(DB_VIEW *v_attr)
{
	return DO_check_node_changed(v_attr);
}


UINT16 DB_get_node_pos(DB_VIEW *v_attr, UINT32 id)
{
	UINT16 i;
	
	for(i = 0; i < v_attr->node_num; i++)
	{
		if(0 == MEMCMP(v_attr->node_buf[i].node_id, (UINT8 *)&id, NODE_ID_SIZE))
			return i;
	}
	NODE_API_PRINTF("DB_get_node_pos(): not find node id = %d\n",id);
	return 0xFFFF;
}


UINT16  DB_get_node_num(DB_VIEW *v_attr, db_filter_t filter, UINT16 filter_mode,UINT32 filter_param)
{
	UINT16 i;
	INT32 ret = DB_SUCCES;
	UINT16 j=0;
	UINT32 node_id = 0, node_addr = 0;
	
	for(i = 0; i < v_attr->node_num; i++)
	{
		ret = DB_get_node_by_pos(v_attr,i, &node_id, &node_addr);
		if(DB_SUCCES != ret)
				return 0xFFFF;
		
		if(filter == NULL || filter(node_id, node_addr, filter_mode,filter_param)==TRUE)
		{
			j++;
		}
		
	}
	return j;

}

/*
INT32 DB_get_node_num_ext(DB_VIEW *v_attr, UINT8 n_type, db_filter_t checker, UINT16 * cnt)
{


}

*/

INT32 DB_init(UINT32 db_base_addr, UINT32 db_length)
{
	return DO_init(db_base_addr, db_length);
	
}

UINT8 DB_get_cur_view_type(DB_VIEW *v_attr)
{
	return DO_get_cur_view_type(v_attr);
}

extern UINT32 g_db_base_addr;
extern UINT32 g_db_length;
INT32 DB_clear_db()
{
	INT32 ret = DO_clear_db();

	if(DB_SUCCES != ret)
		NODE_API_PRINTF("in DB_clear_db(): DO_clear_db() failed!\n");

	if((g_db_base_addr != 0xFFFFFFFF) && (g_db_length != 0))
		ret = DO_init(g_db_base_addr, g_db_length);
	if(DB_SUCCES != ret)
		NODE_API_PRINTF("in DB_clear_db(): DO_init() failed!\n");
	return ret;
}


