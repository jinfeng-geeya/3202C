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
#include <api/libdb/db_node_c.h>

#include <api/libdb/db_node_api.h>

#define DB_PLUGINS_PRINTF PRINTF
#define DYNPID_PRINTF	PRINTF

extern INT32 get_tp_by_pos(UINT16 pos,  T_NODE *node);
extern INT32 get_node_by_id(UINT8 n_type,DB_VIEW *v_attr, UINT32 node_id, void *node, UINT32 node_len);
extern INT32 modify_node(UINT8 n_type,DB_VIEW *v_attr,  UINT32 node_id, void *node, UINT32 node_len);
extern INT32 del_node_by_pos(UINT8 n_type, DB_VIEW *v_attr, UINT16 pos);
extern INT32 del_node_by_id(UINT8 n_type, DB_VIEW *v_attr,UINT32 node_id);
extern UINT16 get_node_num(UINT8 n_type, DB_VIEW *view,db_filter_t filter,UINT16 filter_mode,UINT32 param);
extern INT32 db_unsave_data_changed();

//node packer and unpacker functions, should be in ap, temperally place here
INT32 node_packer(UINT8 node_type, UINT8*src_node, UINT32 src_len,UINT8*packed_node,UINT32 *packed_len)
{
	UINT8 node_len= 0;
	UINT32 offset = 0;
	UINT32 node_id = 0;
	
	if(node_type == TYPE_SAT_NODE)
	{
		S_NODE *s_node = (S_NODE *)src_node;
		node_id = s_node->sat_id;
		node_id = (node_id<<(NODE_ID_PG_BIT+NODE_ID_TP_BIT));
		MEMCPY(packed_node, (UINT8 *)&node_id, NODE_ID_SIZE);
		packed_node[NODE_ID_SIZE] = 1 + S_NODE_FIX_LEN + (1 + s_node->name_len);
		MEMCPY(packed_node+NODE_ID_SIZE+1, &(s_node->sat_orbit), S_NODE_FIX_LEN);
		offset = NODE_ID_SIZE+1+S_NODE_FIX_LEN;
		packed_node[offset] = (UINT8)s_node->name_len;
		MEMCPY((UINT8 *)&packed_node[offset]+1,s_node->sat_name,s_node->name_len);
		*packed_len = NODE_ID_SIZE+packed_node[NODE_ID_SIZE];
	}
	else if(node_type == TYPE_TP_NODE)
	{
		 T_NODE *t_node  = ( T_NODE*)src_node;
		 node_id = t_node->tp_id;
		 node_id = (node_id<<NODE_ID_PG_BIT);
		MEMCPY(packed_node,(UINT8 *)&node_id,NODE_ID_SIZE);
		
#if (defined( _MHEG5_ENABLE_) || defined( _MHEG5_V20_ENABLE_) || defined( NETWORK_ID_ENABLE))
		packed_node[NODE_ID_SIZE] =  1+24;
		MEMCPY(packed_node+NODE_ID_SIZE+1, (UINT8 *)&(t_node->tp_id)+sizeof(DB_TP_ID), 24);
#else
		packed_node[NODE_ID_SIZE] =  1+T_NODE_FIX_LEN;
		MEMCPY(packed_node+NODE_ID_SIZE+1, (UINT8 *)&(t_node->tp_id)+sizeof(DB_TP_ID), T_NODE_FIX_LEN);
#endif

		*packed_len = NODE_ID_SIZE+packed_node[NODE_ID_SIZE];
	}
	else if(node_type == TYPE_PROG_NODE)
	{
		 P_NODE*p_node = ( P_NODE*)src_node;
		MEMCPY(packed_node,(UINT8 *)&p_node->prog_id,NODE_ID_SIZE);
#ifdef DB_CAS_SUPPORT
		packed_node[NODE_ID_SIZE] =  1+P_NODE_FIX_LEN+ (1+p_node->cas_count*2)+\
			(1+p_node->audio_count * 4) +(1+p_node->name_len) +(1+p_node->provider_name_len);
#else
		packed_node[NODE_ID_SIZE] =  1+P_NODE_FIX_LEN+ (1+p_node->audio_count * 4) \
			+(1+p_node->name_len) +(1+p_node->provider_name_len);
#endif
#if (defined(DTG_AUDIO_SELECT) || defined(AUDIO_DESCRIPTION_SUPPORT))
		packed_node[NODE_ID_SIZE] +=  p_node->audio_count;	//size of audiotype
#endif
		offset = NODE_ID_SIZE+1;
		MEMCPY(packed_node+offset, (UINT8 *)&(p_node->prog_id)+sizeof(UINT32), P_NODE_FIX_LEN);
		offset += P_NODE_FIX_LEN;

#ifdef MULTI_BOUQUET_ID_SUPPORT
		packed_node[offset] = (UINT8)p_node->bouquet_count;
		offset += 1;
		MEMCPY(packed_node+offset, p_node->bouquet_id, p_node->bouquet_count*2);
		offset += p_node->bouquet_count *2;
#endif


#ifdef DB_CAS_SUPPORT
		packed_node[offset] = (UINT8)p_node->cas_count;
		offset += 1;
		//cas system id
		MEMCPY(packed_node+offset, p_node->cas_sysid, p_node->cas_count*2);
		offset += p_node->cas_count*2;
#endif
		
		//audio count
		packed_node[offset] = (UINT8)p_node->audio_count;
		offset += 1;
		MEMCPY(packed_node+offset, p_node->audio_pid, p_node->audio_count *2);
		offset += p_node->audio_count *2;
		MEMCPY(packed_node+offset, p_node->audio_lang, p_node->audio_count *2);
		offset += p_node->audio_count * 2;
#if (defined(DTG_AUDIO_SELECT) || defined(AUDIO_DESCRIPTION_SUPPORT))
		MEMCPY(packed_node+offset, p_node->audio_type, p_node->audio_count);
		offset += p_node->audio_count;
#endif
		
		//service name
		packed_node[offset] = (UINT8)p_node->name_len;
		offset += 1;
		MEMCPY(packed_node+offset,p_node->service_name, p_node->name_len);
		offset += p_node->name_len;

		//provider name
#if (SERVICE_PROVIDER_NAME_OPTION>0)
		packed_node[offset] = (UINT8)p_node->provider_name_len;
		offset += 1;
		MEMCPY(packed_node+offset, p_node->service_provider_name, p_node->provider_name_len);
#else
		packed_node[offset] = 0;
#endif
		*packed_len = NODE_ID_SIZE + packed_node[NODE_ID_SIZE];
	}

#if (defined( _MHEG5_ENABLE_) || defined( _MHEG5_V20_ENABLE_) || defined( NETWORK_ID_ENABLE))	
    if(node_type != TYPE_TP_NODE)
#endif		
    {
	    if(*packed_len > src_len)
		    return DBERR_PACK;
    }
	
	return SUCCESS;
}

INT32 node_unpacker(UINT8 node_type,UINT8 *packed_node,UINT32 packed_len,UINT8 *unpack_node, UINT32 unpack_len)
{
	INT32 ret = SUCCESS;
	UINT32 node_id = 0;
	UINT32 offset = 0;
#if (defined( _MHEG5_ENABLE_) || defined( _MHEG5_V20_ENABLE_) || defined( NETWORK_ID_ENABLE))		
	 if(node_type != TYPE_TP_NODE)
#endif	 	
	 {
	    if(packed_len > unpack_len)
		    return DBERR_PARAM;
	 }
     
	MEMSET(unpack_node, 0, unpack_len);
	MEMCPY((UINT8 *)&node_id, packed_node,NODE_ID_SIZE);

	if(node_type == TYPE_SAT_NODE)
	{
		 S_NODE *s_node = ( S_NODE *)unpack_node;
		s_node->sat_id = (UINT16)(node_id>>(NODE_ID_PG_BIT+NODE_ID_TP_BIT));
		offset = NODE_ID_SIZE+1;
		MEMCPY(&(s_node->sat_orbit), packed_node+offset, S_NODE_FIX_LEN);
		offset += S_NODE_FIX_LEN;
		s_node->name_len = packed_node[offset];
		offset += 1;
		MEMCPY(s_node->sat_name, packed_node+offset, s_node->name_len);
		
	}
	else if(node_type == TYPE_TP_NODE)
	{
		 T_NODE* t_node = ( T_NODE*)unpack_node;
		t_node->sat_id = (UINT16)(node_id>>(NODE_ID_PG_BIT+NODE_ID_TP_BIT));
		t_node->tp_id = (DB_TP_ID)(node_id>>NODE_ID_PG_BIT);
#if (defined( _MHEG5_ENABLE_) || defined( _MHEG5_V20_ENABLE_) || defined( NETWORK_ID_ENABLE))	
		MEMCPY((UINT8 *)&(t_node->tp_id)+sizeof(DB_TP_ID), packed_node+NODE_ID_SIZE+1, 24);
#else
		MEMCPY((UINT8 *)&(t_node->tp_id)+sizeof(DB_TP_ID), packed_node+NODE_ID_SIZE+1, T_NODE_FIX_LEN);
#endif
	}
	else if(node_type == TYPE_PROG_NODE)
	{
		 P_NODE* p_node = ( P_NODE*)unpack_node;
		p_node->prog_id = node_id;
		p_node->sat_id = (UINT16)(node_id>>(NODE_ID_PG_BIT+NODE_ID_TP_BIT));
		p_node->tp_id = (DB_TP_ID)(node_id>>NODE_ID_PG_BIT);
		offset = NODE_ID_SIZE+1;
		MEMCPY((UINT8 *)&(p_node->prog_id)+sizeof(UINT32), packed_node+offset,P_NODE_FIX_LEN);
		offset += P_NODE_FIX_LEN;

#ifdef MULTI_BOUQUET_ID_SUPPORT
		p_node->bouquet_count = packed_node[offset];
		offset += 1;
		MEMCPY(p_node->bouquet_id, packed_node+offset, p_node->bouquet_count*2);
		offset += p_node->bouquet_count*2;	
#endif

#ifdef DB_CAS_SUPPORT
		p_node->cas_count = packed_node[offset];
		offset += 1;
		//cas system id
		MEMCPY(p_node->cas_sysid, packed_node+offset, p_node->cas_count*2);
		offset += p_node->cas_count*2;
#endif
		
		//audio count
		p_node->audio_count = packed_node[offset];
		offset += 1;
		MEMCPY(p_node->audio_pid, packed_node+offset, p_node->audio_count *2);
		offset += p_node->audio_count *2;
		MEMCPY(p_node->audio_lang, packed_node+offset, p_node->audio_count *2);
		offset += p_node->audio_count *2;
#if (defined(DTG_AUDIO_SELECT) || defined(AUDIO_DESCRIPTION_SUPPORT))
		MEMCPY(p_node->audio_type, packed_node+offset, p_node->audio_count);
		offset += p_node->audio_count;
#endif
		//service name
		p_node->name_len = packed_node[offset];
		offset += 1;
		MEMCPY(p_node->service_name,packed_node+offset, p_node->name_len);
		offset += p_node->name_len;
#if (SERVICE_PROVIDER_NAME_OPTION>0)
		//provider name
		p_node->provider_name_len = packed_node[offset];
		offset += 1;
		MEMCPY(p_node->service_provider_name, packed_node+offset, p_node->provider_name_len);
#else
		p_node->provider_name_len = 0;
#endif
	}
	return SUCCESS;

}

extern OSAL_ID db_access_sema;

extern DB_VIEW db_view;
extern DB_TABLE db_table[DB_TABLE_NUM];

extern DB_VIEW db_search_view;
extern UINT8 db_search_mode;
extern UINT8 *db_search_tp_buf;

extern struct dynamic_prog_back dynamic_program;

BOOL db_same_node_checker(UINT8 n_type, void* old_node, void* new_node)
{
	if(n_type == TYPE_SAT_NODE)
	{
		 S_NODE* s1=( S_NODE*)old_node;
		 S_NODE* s2=( S_NODE*)new_node;
		if((s1->sat_orbit == s2->sat_orbit) &&
			(s1->name_len == s2->name_len) &&
			(0 == MEMCMP(s1->sat_name,s2->sat_name,s1->name_len)))
			{
				s2->sat_id = s1->sat_id;
				return TRUE;
			}
	}
	else if(n_type == TYPE_TP_NODE || n_type == TYPE_SEARCH_TP_NODE)
	{
		 T_NODE* t1=( T_NODE*)old_node;
		 T_NODE* t2=( T_NODE*)new_node;
		if((t1->sat_id == t2->sat_id) &&
			(((INT32)(t1->frq - t2->frq)<=FRQ_EDG) && ((INT32)(t2->frq - t1->frq)<=FRQ_EDG)) &&
			
			(((INT32)(t1->sym - t2->sym)<=SYM_EDG) && ((INT32)(t2->sym - t1->sym)<=SYM_EDG)) &&
			(t1->FEC_inner == t2->FEC_inner))
		{
			t2->tp_id = t1->tp_id;
			return TRUE;
		}
	}
	else if(n_type == TYPE_PROG_NODE)
	{
		 P_NODE* p1=( P_NODE*)old_node;
		 P_NODE* p2=( P_NODE*)new_node;
		if((p1->sat_id == p2->sat_id) && (p1->tp_id == p2->tp_id))
		{
			if(p2->prog_number!=0)
			{
				if(p1->prog_number==p2->prog_number)
				{
					p2->prog_id = p1->prog_id;
					return TRUE;
				}
			}
			/*for programs which not listed in PAT,PMT, user input the a,v,pcr pids and 
			set prog_number=0 for pid search
			*/
			else
			{
				if((p2->av_flag==p1->av_flag)&&(p2->video_pid==p1->video_pid)
					&&(p2->pcr_pid==p1->pcr_pid))
				{
					if((p2->av_flag==1)||((p2->av_flag==0)&&(p2->audio_pid[0]==p1->audio_pid[0])))
					{
						p2->prog_id = p1->prog_id;
						return TRUE;
					}
				}
			}
			
		}
	}
	return FALSE;
}

BOOL db_same_tpnode_checker(UINT8 n_type, void* old_node, void* new_node)
{
	if(n_type == TYPE_SAT_NODE)
	{
		 S_NODE* s1=( S_NODE*)old_node;
		 S_NODE* s2=( S_NODE*)new_node;
		if((s1->sat_orbit == s2->sat_orbit) &&
			(s1->name_len == s2->name_len) &&
			(0 == MEMCMP(s1->sat_name,s2->sat_name,s1->name_len)))
			{
				s2->sat_id = s1->sat_id;
				return TRUE;
			}
	}
	else if(n_type == TYPE_TP_NODE || n_type == TYPE_SEARCH_TP_NODE)
	{
		 T_NODE* t1=( T_NODE*)old_node;
		 T_NODE* t2=( T_NODE*)new_node;

		 if(t1->network_id==t2->network_id&&t1->t_s_id==t2->t_s_id)
		{
			t2->tp_id = t1->tp_id;
			return TRUE;
		}
	}
	else if(n_type == TYPE_PROG_NODE)
	{
		 P_NODE* p1=( P_NODE*)old_node;
		 P_NODE* p2=( P_NODE*)new_node;
		if((p1->sat_id == p2->sat_id) && (p1->tp_id == p2->tp_id))
		{
			if(p2->prog_number!=0)
			{
				if(p1->prog_number==p2->prog_number)
				{
					p2->prog_id = p1->prog_id;
					return TRUE;
				}
			}
			/*for programs which not listed in PAT,PMT, user input the a,v,pcr pids and 
			set prog_number=0 for pid search
			*/
			else
			{
				if((p2->av_flag==p1->av_flag)&&(p2->video_pid==p1->video_pid)
					&&(p2->pcr_pid==p1->pcr_pid))
				{
					if((p2->av_flag==1)||((p2->av_flag==0)&&(p2->audio_pid[0]==p1->audio_pid[0])))
					{
						p2->prog_id = p1->prog_id;
						return TRUE;
					}
				}
			}
			
		}
		#ifdef INDIA_LCN
		else if(p2->LCN!=0)
		{
			if(p1->LCN==p2->LCN)
			{
				p2->prog_id = p1->prog_id;
				return TRUE;
			}
		}
		#endif
	}
	return FALSE;
}


INT32 get_tp_at(UINT16 sat_id,UINT16 pos,  T_NODE *node)
{
	INT32 ret = DB_SUCCES;
	UINT32 node_id = 0, node_addr = 0;
	UINT16 i;
	DB_VIEW *v_attr;
	UINT16 j=0;
	UINT16 tmp_id;
	UINT16 num = 0;
	UINT8 view_type_differ = 0;
	DB_TABLE * table = (DB_TABLE *)&db_table[TYPE_TP_NODE];
	
	DB_ENTER_MUTEX();
	if(db_search_mode == 1)
		v_attr = (DB_VIEW *)&db_search_view;
	else
		v_attr = (DB_VIEW *)&db_view;
	
	if((db_search_mode == 0) && (TYPE_TP_NODE!= DB_get_cur_view_type(v_attr)))
	{
		num = table->node_num;
		view_type_differ = 1;
		/*
		for(i = 0; i < db_table[TYPE_TP_NODE].node_num; i++)
		{
			DB_get_node_by_pos_from_table(&db_table[TYPE_TP_NODE], i, &node_id, &node_addr);
			tmp_id = (UINT16)(node_id>>(NODE_ID_PG_BIT+NODE_ID_TP_BIT));
			if(sat_id == tmp_id)
			{
				if(j==pos)
					break;
				else
					j++;
			}
		}
		*/
	}
	else
	{
		num = v_attr->node_num;
		/*
		for(i = 0; i < v_attr->node_num; i++)
		{
			DB_get_node_by_pos(v_attr, i, &node_id, &node_addr);
			tmp_id = (UINT16)(node_id>>(NODE_ID_PG_BIT+NODE_ID_TP_BIT));
			if(sat_id == tmp_id)
			{
				if(j==pos)
					break;
				else
					j++;
			}
		}
		*/
	}
		for(i = 0; i < num; i++)
		{
			DB_get_node_id_addr_by_pos(view_type_differ, table, v_attr, i, &node_id, &node_addr);
			tmp_id = (UINT16)(node_id>>(NODE_ID_PG_BIT+NODE_ID_TP_BIT));
			if(sat_id == tmp_id)
			{
				if(j==pos)
					break;
				else
					j++;
			}
		}
	
	if(i == num)
	{
		DB_RELEASE_MUTEX();
		DB_PLUGINS_PRINTF("get tp at pos %d on sat_id %x, not found!\n",pos,sat_id);
		return DBERR_API_NOFOUND;
	}
	if(db_search_mode == 1)
		MEMCPY(node,db_search_tp_buf+i*sizeof(T_NODE),sizeof(T_NODE));
	else
		ret = DB_read_node(node_id, node_addr, (UINT8 *)node, sizeof( T_NODE));
	DB_RELEASE_MUTEX();
	return ret;
}

UINT16 get_tp_num_sat(UINT16 sat_id)
{
	UINT32 node_id = 0, node_addr = 0;
	DB_VIEW *view = (DB_VIEW *)&db_view;
	DB_TABLE *table = (DB_TABLE *)&db_table[TYPE_TP_NODE];
	UINT16 i;
	UINT16 j = 0;
	UINT16 tmp_id;
	UINT16 num = 0;
	UINT8 view_type_differ = 0;
	
	DB_ENTER_MUTEX();
	if(db_search_mode == 1)
	{
		DB_RELEASE_MUTEX();
		return db_search_view.node_num;
	}
	
	if(TYPE_TP_NODE!= DB_get_cur_view_type(view))
	{
		num = table->node_num;
		view_type_differ = 1;
		/*
		for(i = 0; i < db_table[TYPE_TP_NODE].node_num; i++)
		{
			DB_get_node_by_pos_from_table(&db_table[TYPE_TP_NODE], i, &node_id, &node_addr);
			tmp_id = (UINT16)(node_id>>(NODE_ID_PG_BIT+NODE_ID_TP_BIT));
			if(sat_id == tmp_id)
				j++;
		}
		*/
	}
	else
	{
		num = view->node_num;
		/*
		for(i = 0; i < view->node_num; i++)
		{
			DB_get_node_by_pos(view, i, &node_id, &node_addr);
			tmp_id = (UINT16)(node_id>>(NODE_ID_PG_BIT+NODE_ID_TP_BIT));
			if(sat_id == tmp_id)
				j++;
		}
		*/
	}
		for(i = 0; i < num; i++)
		{
			DB_get_node_id_addr_by_pos(view_type_differ, table, view, i, &node_id, &node_addr);
			tmp_id = (UINT16)(node_id>>(NODE_ID_PG_BIT+NODE_ID_TP_BIT));
			if(sat_id == tmp_id)
				j++;
		}
	
	DB_RELEASE_MUTEX();
	return j;
}

INT32 del_tp_on_sat(UINT16 sat_id)
{
	DB_VIEW *view = (DB_VIEW *)&db_view;
	UINT32 tmp_id;
	INT32 ret = DB_SUCCES;

	tmp_id = sat_id;
	tmp_id = tmp_id<<(NODE_ID_PG_BIT+NODE_ID_TP_BIT);
	DB_ENTER_MUTEX();
	if(TYPE_TP_NODE != DB_get_cur_view_type(view))
	{
		DB_RELEASE_MUTEX();
		DB_PLUGINS_PRINTF(" del_tp_on_sat(): cur_view_type not tp type!\n");
		return DBERR_API_NOFOUND;
	}
	ret = DB_del_node_by_parent_id(view, TYPE_SAT_NODE, tmp_id);
	DB_RELEASE_MUTEX();
	return ret;

}


INT32 get_sat_by_id(UINT16 sat_id,  S_NODE *node)
{
	UINT32 tmp_id = 0;

	tmp_id = sat_id;
	tmp_id = tmp_id<<(NODE_ID_PG_BIT+NODE_ID_TP_BIT);
	return get_node_by_id(TYPE_SAT_NODE,(DB_VIEW *)&db_view, tmp_id, (void *)node, sizeof( S_NODE));
}

INT32 modify_sat(UINT16 sat_id,  S_NODE *node)
{
	UINT32 tmp_id = 0;

	tmp_id = sat_id;
	tmp_id = tmp_id<<(NODE_ID_PG_BIT+NODE_ID_TP_BIT);
	return modify_node(TYPE_SAT_NODE,(DB_VIEW *)&db_view, tmp_id, (void *)node, sizeof( S_NODE));
}

INT32 del_sat_by_pos(UINT16 pos)
{
	return del_node_by_pos(TYPE_SAT_NODE,(DB_VIEW *)&db_view, pos);
}

INT32 del_sat_by_id(UINT16 sat_id)
{
	UINT32 tmp_id = sat_id;
	tmp_id = tmp_id<<(NODE_ID_PG_BIT+NODE_ID_TP_BIT);
	return del_node_by_id(TYPE_SAT_NODE,(DB_VIEW *)&db_view, tmp_id);
}

static BOOL sat_view_filter(UINT32 id, UINT32 addr, UINT16 create_mode,UINT32 param)
{
	 S_NODE node;
	BOOL ret = FALSE;
	union filter_mode mode;
			
	//mode.decode.group_mode = (UINT8)(create_mode>>8);
	mode.decode.av_mode = (UINT8)(create_mode);
	//to speed up, only read first 8 byte of packed s_node
	BO_read_data(addr+NODE_ID_SIZE+1, 4, (UINT8 *)&(node.sat_orbit));
	if ((create_mode&0xFF00) == SET_SELECTED) 
	{
		ret = (node.selected_flag==1);
		if(mode.decode.av_mode ==SET_TUNER1_VALID)
			ret = ret&&(node.tuner1_valid == 1);
		else if(mode.decode.av_mode ==SET_TUNER2_VALID)
			ret =  ret&&(node.tuner2_valid == 1);
		else if(mode.decode.av_mode ==SET_EITHER_TUNER_VALID)
			ret =  ret&&((node.tuner1_valid == 1)||(node.tuner2_valid == 1));
	}
	else
	{
		ret = TRUE;
		if(mode.decode.av_mode ==SET_TUNER1_VALID)
			ret =  ret&&(node.tuner1_valid == 1);
		else if(mode.decode.av_mode ==SET_TUNER2_VALID)
			ret =  ret&&(node.tuner2_valid == 1);
		else if(mode.decode.av_mode ==SET_EITHER_TUNER_VALID)
			ret =  ret&&((node.tuner1_valid == 1)||(node.tuner2_valid == 1));
	}
		
	return ret;
}


INT32 get_sat_at(UINT16 pos, UINT16 select_mode, S_NODE *node)
{
	INT32 ret = DB_SUCCES;
	UINT32 node_id = 0, node_addr = 0;
	UINT16 i;
	DB_VIEW *v_attr = (DB_VIEW *)&db_view;
	DB_TABLE *table = (DB_TABLE *)&db_table[TYPE_SAT_NODE];
	UINT16 j=0;
	UINT16 num = 0;
	UINT8 view_type_differ = 0;

	DB_ENTER_MUTEX();
		
	if((TYPE_SAT_NODE!= DB_get_cur_view_type(v_attr)))
	{
		num = table->node_num;
		view_type_differ = 1;
		/*
		for(i = 0; i < db_table[TYPE_SAT_NODE].node_num; i++)
		{
			DB_get_node_by_pos_from_table(&db_table[TYPE_SAT_NODE], i, &node_id, &node_addr);
			if(sat_view_filter(node_id, node_addr, select_mode, 0)==TRUE)
			{
				if(j==pos)
					break;
				else
					j++;
			}
		}
		*/
	}
	else
	{
		num = v_attr->node_num;
		/*
		for(i = 0; i < v_attr->node_num; i++)
		{
			DB_get_node_by_pos(v_attr, i, &node_id, &node_addr);
			if(sat_view_filter(node_id, node_addr, select_mode, 0)==TRUE)
			{
				if(j==pos)
					break;
				else
					j++;
			}
		}
		*/
	}
		for(i = 0; i < num; i++)
		{
			DB_get_node_id_addr_by_pos(view_type_differ, table, v_attr, i, &node_id, &node_addr);
			if(sat_view_filter(node_id, node_addr, select_mode, 0)==TRUE)
			{
				if(j==pos)
					break;
				else
					j++;
			}
		}
	
	ret = DB_read_node(node_id, node_addr, (UINT8 *)node, sizeof( S_NODE));
	DB_RELEASE_MUTEX();
	return ret;
}

INT32 recreate_sat_view(UINT16 create_mode,UINT32 param)
{
	INT32 ret = DB_SUCCES;

	DB_ENTER_MUTEX();
	ret = DB_create_view(TYPE_SAT_NODE, (DB_VIEW *)&db_view,(db_filter_t)sat_view_filter,NULL,create_mode,param);

	db_unsave_data_changed();
	DB_RELEASE_MUTEX();
	return ret;
}

UINT16 get_sat_num(UINT16 select_mode)
{
	UINT16 num = 0;
	
	DB_ENTER_MUTEX();
	num = get_node_num(TYPE_SAT_NODE,(DB_VIEW *)&db_view, (db_filter_t)sat_view_filter, select_mode, 0);
	DB_RELEASE_MUTEX();
	return num;
}


/*
UINT16 get_sat_pos(UINT16 sat_id)
{
	return get_node_pos(TYPE_SAT_NODE, (DB_VIEW *)&db_view, sat_id);
}
*/


INT32 update_stream_info(P_NODE *prev, P_NODE *next, INT32 *node_pos)
{
	UINT32 aud_cnt;
	INT32 pos;
	INT32 ret = DB_SUCCES;
	P_NODE program;
		
	DB_PLUGINS_PRINTF("enter update_stream_info!\n");
	
	if ((pos=get_prog_pos(prev->prog_id))!= INVALID_POS_NUM)
	{
		ret = get_prog_at(pos, &program);
		DYNPID_PRINTF("\nupdate_stream_info(): 1.get prog from db tp_id=%d, prog_number=%d,vpid=%d\n",program.tp_id,program.prog_number,program.video_pid);
		program.ca_mode = next->ca_mode;
		#ifdef DB_CAS_SUPPORT
		program.cas_count = next->cas_count;
		MEMCPY(program.cas_sysid, next->cas_sysid, sizeof(UINT16)*next->cas_count);
		#endif
		program.audio_count = aud_cnt = next->audio_count;
		MEMCPY(program.audio_pid, next->audio_pid, aud_cnt*sizeof(UINT16));
		MEMCPY(program.audio_lang, next->audio_lang, aud_cnt*sizeof(UINT16));
		program.teletext_pid = next->teletext_pid;
		program.pcr_pid = next->pcr_pid;
		program.subtitle_pid = next->subtitle_pid;
		program.video_pid = next->video_pid;
		MEMCPY(next, &program, sizeof(P_NODE));
		*node_pos = pos;
		
		DB_ENTER_MUTEX();
		DYNPID_PRINTF("	2.dynamic_program tp_id=%d, prog_number=%d,vpid=%d\n",dynamic_program.prog.tp_id,dynamic_program.prog.prog_number,dynamic_program.prog.video_pid);
		dynamic_program.status = DYNAMIC_PG_STEAM_UPDATED;
		MEMCPY(&(dynamic_program.prog), &program, sizeof(P_NODE));
		DYNPID_PRINTF("	3.update dynamic_program, vpid=%d,\n",program.video_pid);
		DB_RELEASE_MUTEX();
	
	} 
	else 
	{
		DB_PLUGINS_PRINTF("get pos failure!\n");
		ret = DBERR_API_NOFOUND;
	}

	return ret;

}


INT32 undo_prog_modify(UINT32 tp_id,UINT16 prog_number)
{
	DB_ENTER_MUTEX();

	if((dynamic_program.prog.tp_id==tp_id)&&(dynamic_program.prog.prog_number==prog_number))
	{
		dynamic_program.status = DYNAMIC_PG_INITED;
		DYNPID_PRINTF("\nundo_prog_modify(): dynamic_program tp_id=%d, prog_number=%d, vpid=%d,SET its status=INITED\n",tp_id,prog_number,dynamic_program.prog.video_pid);		
	}
	DB_RELEASE_MUTEX();
	return DB_SUCCES;
}

UINT32 db_channel_logicnum_to_id(UINT32 logic_number)
{
	UINT32 node_addr = 0;
	UINT32 node_id = 0;
	UINT16 cnt = 0;
	NODE_IDX_PARAM *pbuf = NULL;
	UINT16 i = 0;
	P_NODE p_node;

	DB_ENTER_MUTEX();
	if(TYPE_PROG_NODE != DB_get_cur_view_type(&db_view))
	{
		cnt = db_table[TYPE_PROG_NODE].node_num;
		pbuf = db_table[TYPE_PROG_NODE].table_buf;
	}
	else
	{
		cnt = db_view.node_num;
		pbuf = db_view.node_buf;
	}

	for(i = 0; i < cnt; i++, pbuf++)
	{
		MEMCPY((UINT8 *)&node_addr, pbuf->node_addr, NODE_ADDR_SIZE);
		if(DB_SUCCES != BO_read_data(node_addr+NODE_ID_SIZE+1, P_NODE_FIX_LEN, (UINT8 *)&(p_node.prog_id)+sizeof(UINT32)))
		{
			DB_PLUGINS_PRINTF("%s(): read prog from flash addr[0x%x] fail!\n",node_addr);
			DB_RELEASE_MUTEX();
			return 0xFFFFFFFF;
		}
		if(p_node.logical_channel_num==logic_number)
		{
			MEMCPY((UINT8 *)&node_id, pbuf->node_id, NODE_ID_SIZE);
			break;
		}
	}
	DB_RELEASE_MUTEX();
	if(i==cnt)
	{
		DB_PLUGINS_PRINTF("%s(): chanel logicnumber %d not found!\n",logic_number);
		node_id = 0xFFFFFFFF;
	}
	
	return node_id;

}
#ifdef _LCN_ENABLE_
UINT32 db_channel_lcn_to_id(UINT32 logic_number)
{
	UINT32 node_addr = 0;
	UINT32 node_id = 0;
	UINT16 cnt = 0;
	NODE_IDX_PARAM *pbuf = NULL;
	UINT16 i = 0;
	P_NODE p_node;

	DB_ENTER_MUTEX();
	if(TYPE_PROG_NODE != DB_get_cur_view_type(&db_view))
	{
		cnt = db_table[TYPE_PROG_NODE].node_num;
		pbuf = db_table[TYPE_PROG_NODE].table_buf;
	}
	else
	{
		cnt = db_view.node_num;
		pbuf = db_view.node_buf;
	}

	for(i = 0; i < cnt; i++, pbuf++)
	{
		MEMCPY((UINT8 *)&node_addr, pbuf->node_addr, NODE_ADDR_SIZE);
		if(DB_SUCCES != BO_read_data(node_addr+NODE_ID_SIZE+1, P_NODE_FIX_LEN, (UINT8 *)&(p_node.prog_id)+sizeof(UINT32)))
		{
			DB_PLUGINS_PRINTF("%s(): read prog from flash addr[0x%x] fail!\n",node_addr);
			DB_RELEASE_MUTEX();
			return 0xFFFFFFFF;
		}
		if(p_node.LCN==logic_number)
		{
			MEMCPY((UINT8 *)&node_id, pbuf->node_id, NODE_ID_SIZE);
			break;
		}
	}
	DB_RELEASE_MUTEX();
	if(i==cnt)
	{
		DB_PLUGINS_PRINTF("%s(): chanel logicnumber %d not found!\n",logic_number);
		node_id = 0xFFFFFFFF;
	}
	
	return node_id;

}
#endif

