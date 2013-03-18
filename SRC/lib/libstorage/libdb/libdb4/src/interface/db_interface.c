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
*  2.  2009.12.21  Yun You        0.1.000   	Add Multifeed feature
*  3.  2010.01.07  Zhenbo Liu     0.1.000   	Sort according by user operation
* 
****************************************************************************/

#include <sys_config.h>
#include <types.h>
#include <hld/sto/sto.h>
#include <hld/sto/sto_dev.h>
#include <api/libc/printf.h>
#include <osal/osal.h>
#include <api/libc/string.h>
#include <api/libdb/tmp_info.h>
#include <api/libtsi/db_3l.h>
#include <api/libsi/si_service_type.h>

#include <api/libdb/db_node_api.h>
#include "../engine/db_basic_operation.h"

#ifdef RAPS_SUPPORT
#include <api/libchunk/chunk.h>
#define SECTOR_SIZE		(64*1024)
#endif
#define DB_PRINTF	PRINTF
#ifdef User_order_sort
UINT32 cur_user_node_cnt = 0;
#endif
OSAL_ID db_access_sema = OSAL_INVALID_ID;
UINT8 * db_update_tmp_buf = NULL;

UINT16 TV_NUM = 1;
UINT16 RADIO_NUM = 1;

extern DB_VIEW db_view;
extern DB_TABLE db_table[DB_TABLE_NUM];

UINT8 db_search_mode = 0;

DB_VIEW db_search_view;
static UINT8 *db_search_view_buf = NULL;
UINT8 *db_search_tp_buf = NULL;
static UINT8 *db_search_pg_buf = NULL;

struct dynamic_prog_back dynamic_program;
#if 0
static void default_s2db(OLD_S_NODE *sdefault,  S_NODE *sdb)
{
	void *p_from, *p_to;
	p_from = (void *)sdefault;
	p_to = (void *)sdb;
	INT32 len;

	sdb->sat_orbit = (UINT16)sdefault->sat_orbit;
	sdb->tuner2_antena.lnb_power=sdb->lnb_power = sdefault->lnb_power;
	sdb->tuner2_antena.lnb_type =sdb->lnb_type = sdefault->lnb_type;
	sdb->tuner2_antena.pol =sdb->pol = sdefault->pol;
	sdb->tuner2_antena.k22 =sdb->k22 = sdefault->k22;
	sdb->tuner2_antena.v12 =sdb->v12 = sdefault->v12;
	sdb->tuner2_antena.toneburst =sdb->toneburst = sdefault->toneburst;
	sdb->tuner2_antena.lnb_low =sdb->lnb_low = sdefault->lnb_low;
	sdb->tuner2_antena.lnb_high =sdb->lnb_high = sdefault->lnb_high;
	sdb->tuner2_antena.DiSEqC_type =sdb->DiSEqC_type = sdefault->DiSEqC_type;
	sdb->tuner2_antena.DiSEqC_port =sdb->DiSEqC_port = sdefault->DiSEqC_port;
	sdb->tuner2_antena.DiSEqC11_type =sdb->DiSEqC11_type = sdefault->DiSEqC11_type;
	sdb->tuner2_antena.DiSEqC11_port =sdb->DiSEqC11_port = sdefault->DiSEqC11_port;
	sdb->tuner2_antena.positioner_type =sdb->positioner_type = sdefault->positioner_type;
	sdb->tuner2_antena.position =sdb->position = sdefault->position;
		
	len = DB_STRLEN((DB_ETYPE *)sdefault->sat_name);
	sdb->name_len = (UINT8)(len*sizeof(DB_ETYPE));
	//DB_STRCPY((DB_ETYPE *)sdb->sat_name,(DB_ETYPE *)sdefault->sat_name);
	MEMCPY(sdb->sat_name, sdefault->sat_name, sdb->name_len);

}

static void default_t2db(OLD_T_NODE *tdefault,  T_NODE *tdb)
{
	void *p_from, *p_to;
	p_from = (void *)tdefault;
	p_to = (void *)tdb;
	INT32 len;

	tdb->frq = (UINT16)tdefault->frq;
	tdb->sym = (UINT16)tdefault->sym;
	tdb->pol = tdefault->pol;
	tdb->FEC_inner = tdefault->FEC_inner;
	tdb->universal_22k_option = tdefault->universal_22k_option;
	tdb->Big5_indicator = tdefault->Big5_indicator	;
	tdb->nit_pid = tdefault->nit_pid	;
	tdb->t_s_id = tdefault->t_s_id;
	tdb->network_id = tdefault->network_id;
	
}

static INT32 tmp_read_data_from_flash(UINT32 addr, UINT32 len, UINT8 *data)
{
	struct sto_device *sto_dev = NULL;
	INT32 ret = DB_SUCCES;
	sto_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
	
	if(sto_get_data(sto_dev, data, addr, (INT32)len) == (INT32)len)
		return DB_SUCCES;
	else
		return DBERR_BO_READ;
}




// ************************************************************
// * Summary:
// *     Description:
// *     clear all data in user db, load the default node data.
// * Returns:
// *     DB_SUCCES if success, Else error value;
// * Parameters:
// *     set_mode :            load default node's mode;
// *     default_value_addr :  address of default node data;   
INT32 set_default_value(UINT8 set_mode,UINT32 default_value_addr)
{
	INT16 s_cnt,t_cnt,p_cnt;
	UINT32 cur_addr;
	 P_NODE p_node;
	 T_NODE t_node;
	 S_NODE s_node;
	OLD_S_NODE old_set;
	OLD_T_NODE old_tp;
	P_DEFAULT_NODE p_default;
	
	UINT32 s_id=1, t_id=1, p_id = 1;
	UINT32 tmp_id;

	UINT16	nIdentifyCode;		//new format of default database starts with 0xeffe
	UINT16  nVersion;			//database version
	BOOL	bCapabilityWithOrder=FALSE;	//allow to specify channel order in default database
	
	 P_NODE *po_PNode=NULL;
	BOOL   *pnNodeExist=NULL;
	UINT16	nCount;
	UINT16	nOrder;
	INT32 ret = DB_SUCCES;
	DB_TABLE *table = (DB_TABLE *)&db_table[0];

	DB_ENTER_MUTEX();
	/* clear current database */	
	if(DB_clear_db() != DB_SUCCES)
	{
		DB_PRINTF("set_default_value(): clear db failed!\n");
		DB_RELEASE_MUTEX();
		return !DB_SUCCES;
	}

	cur_addr = default_value_addr;
	DB_PRINTF("start time: %d\n", osal_get_tick());
	cur_addr &= 0x8FFFFFFF;
	//osal_cache_invalidate(cur_addr + (SYS_FLASH_BASE_ADDR & 0x8fffffff), TABLE_BLOCK_SPACE_SIZE);

	//Header Checker
	tmp_read_data_from_flash(cur_addr, 2, (UINT8 *)&nIdentifyCode);
	if (nIdentifyCode!=(UINT16)0xeffe)
		nVersion = 1;	
	else
	{
		cur_addr +=2;
		tmp_read_data_from_flash(cur_addr, 2, (UINT8 *)&nVersion);
		if (nVersion>0x1)
		{
			bCapabilityWithOrder = TRUE;
		}
		cur_addr +=2;
	}
	//if need order, malloc memory
	if (bCapabilityWithOrder==TRUE)
	{
		tmp_read_data_from_flash(cur_addr, 2, (UINT8 *)&nCount);
		cur_addr +=2;
		po_PNode = ( P_NODE *)MALLOC(nCount * sizeof( P_NODE));
		if (po_PNode==NULL) return !DB_SUCCES;
	
		pnNodeExist = (BOOL *)MALLOC(nCount * sizeof(BOOL));
		if (pnNodeExist==NULL) 
		{
			FREE(po_PNode);
			return !DB_SUCCES;
		}
		MEMSET(pnNodeExist, FALSE, nCount*sizeof(BOOL));
	};


	tmp_read_data_from_flash(cur_addr, 2, (UINT8 *)&s_cnt);
	cur_addr += 2;
	//add sat into table
	for(;s_cnt>0;s_cnt--)
	{
		t_id = 1;
		//tmp_read_data_from_flash(cur_addr, sizeof( S_NODE), (UINT8 *)&s_node);
		//cur_addr += sizeof( S_NODE);
		tmp_read_data_from_flash(cur_addr, sizeof(OLD_S_NODE), (UINT8 *)&old_set);
		cur_addr += sizeof(OLD_S_NODE);
		MEMSET(&s_node, 0, sizeof( S_NODE));
		s_node.sat_id= (UINT16)(s_id);
		default_s2db(&old_set, &s_node);
		DB_add_node_to_table(&table[TYPE_SAT_NODE], (s_id<<(NODE_ID_TP_BIT+NODE_ID_PG_BIT)), (UINT8 *)&s_node, sizeof( S_NODE));
		s_id++;

		//add tp of this sat into table
		tmp_read_data_from_flash(cur_addr, 2, (UINT8 *)&t_cnt);
		cur_addr += 2;
		for(;t_cnt>0;t_cnt--)
		{
			p_id = 1;
			//tmp_read_data_from_flash(cur_addr, sizeof( T_NODE), (UINT8 *)&t_node);
			//cur_addr += sizeof( T_NODE);
			tmp_read_data_from_flash(cur_addr, sizeof(OLD_T_NODE), (UINT8 *)&old_tp);
			cur_addr += sizeof(OLD_T_NODE);
			MEMSET(&t_node, 0, sizeof( T_NODE));
			t_node.preset_flag = 1;
			tmp_id = ((UINT32)s_node.sat_id<<(NODE_ID_PG_BIT+NODE_ID_TP_BIT)) | (t_id<<NODE_ID_PG_BIT);
			t_node.sat_id = s_node.sat_id;
			t_node.tp_id = (DB_TP_ID)(tmp_id>>NODE_ID_PG_BIT);
			default_t2db(&old_tp, &t_node);
			DB_add_node_to_table(&table[TYPE_TP_NODE], tmp_id, (UINT8 *)&t_node, sizeof( T_NODE));
			t_id++;
			
			//add program of this tp into table
			tmp_read_data_from_flash(cur_addr, 2, (UINT8 *)&p_cnt);
			cur_addr += 2;
			for(;p_cnt>0;p_cnt--)
			{
				if(set_mode == DEFAULT_SAT_TP_ONLY)
				{
					cur_addr += (sizeof(P_DEFAULT_NODE));
					continue;
				}
			
				MEMSET((UINT8 *)&p_node,0,sizeof( P_NODE));
				if (bCapabilityWithOrder==TRUE)
				{
					tmp_read_data_from_flash(cur_addr, 2, (UINT8 *)&nOrder);
					cur_addr += 2;
				}
				if(nVersion <= 0x2)
				{
					tmp_read_data_from_flash(cur_addr, sizeof(P_DEFAULT_NODE), (UINT8 *)&p_default);
					cur_addr += (sizeof(P_DEFAULT_NODE));
					//default2db(&p_default, &p_node);
				}
#ifdef FEATURE_DEFAULT_NODE_WITH_ORDER
				else if(nVersion == 0x03)
		             {
		                   
		              }
#endif				

				if(set_mode == DEFAULT_DOMESTIC_PROG && p_node.level == 1)
					continue;

				p_node.sat_id = s_node.sat_id;
				p_node.tp_id = t_node.tp_id;
				tmp_id = t_node.tp_id;
				tmp_id = (tmp_id<<NODE_ID_PG_BIT) | p_id;
				p_node.prog_id = tmp_id;
				p_id++;
				p_node.audio_volume = AUDIO_DEFAULT_VOLUME;
				if(p_node.prog_number == 0) 
				{
					p_node.prog_number = p_cnt;
					/* 
					 * inform that this program don't have 
					 * valid program number yet
					 */
#ifdef MULTI_AUDIO_PID					 
					p_node.pmt_pid = 0x1FFF; 
#endif					
				}
#if (SERVICE_PROVIDER_NAME_OPTION>0)
				MEMCPY(p_node.service_provider_name, DB_DEFAULT_PROV_NAME, DB_DEFAULT_PROV_LEN);
#endif
				if(p_node.video_pid == 0 || p_node.video_pid == 0x1FFF) 
					p_node.av_flag = 0;
				else 
					p_node.av_flag = 1;
				
				if (bCapabilityWithOrder==FALSE)
				{
					DB_add_node_to_table(&table[TYPE_PROG_NODE], p_node.prog_id, (UINT8 *)&p_node, sizeof( P_NODE));
					
				}
				else if (bCapabilityWithOrder==TRUE)
				{
					MEMCPY(&po_PNode[nOrder], &p_node, sizeof( P_NODE));
					pnNodeExist[nOrder]=TRUE;
				}

			}
		}
		
	}

	DB_PRINTF("end time: %d\n", osal_get_tick());

	if (bCapabilityWithOrder == TRUE)
	{
		for (nOrder=0; bCapabilityWithOrder==TRUE && nOrder<nCount; nOrder++)
		{
			if (pnNodeExist[nOrder]==FALSE)
				continue;
			DB_add_node_to_table(&table[TYPE_PROG_NODE], po_PNode[nOrder].prog_id, (UINT8 *)&po_PNode[nOrder], sizeof( P_NODE));
			
		}
	}
	//write table into flash as header
	ret = DB_reclaim_write_header();
	if(ret != DB_SUCCES)
	{
		DB_PRINTF("set_default_value(): write header to flash failed!\n");
		DB_RELEASE_MUTEX();
		return ret;
	}
	
	if (bCapabilityWithOrder == TRUE)
	{
		if (po_PNode) FREE(po_PNode);
		if (pnNodeExist) FREE(pnNodeExist);
	}
	
	DB_RELEASE_MUTEX();

	DB_PRINTF("set_default_value(): End!\n");
	return DB_SUCCES;
}
#endif


INT32 set_default_value(UINT8 set_mode,UINT32 default_value_addr)
{
	UINT32 default_start;
	UINT32 cur_addr=0;
	UINT32 header_flag;
	UINT16 node_num, i;
	UINT8 tmp_buf[DB_NODE_MAX_SIZE];
	UINT8 tmp_buf_for_order[DB_NODE_MAX_SIZE];
	UINT8 node_idx[NODE_ID_SIZE+NODE_ADDR_SIZE];
	struct sto_device *sto_dev = NULL;
	INT32 ret = DB_SUCCES;
	UINT32 node_id =0, node_addr = 0;
	UINT8 node_len = 0;
	DB_TABLE *table = (DB_TABLE *)&db_table[0];
	UINT8 node_type;
	INT32 sector_node_len[BO_MAX_SECTOR_NUM];
	UINT16 table_node_idx = 0;
    	P_NODE pg;
	S_NODE s_node;
		
#ifdef User_order_sort
	P_NODE *tmp_pg;
#endif	

	TV_NUM = 1;
	RADIO_NUM = 1;
	
	DB_ENTER_MUTEX();
	MEMSET(&dynamic_program, 0 , sizeof(struct dynamic_prog_back));
	sto_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
	/* clear current database */	
	if(DB_clear_db() != DB_SUCCES)
	{
		DB_PRINTF("set_default_value(): clear db failed!\n");
		DB_RELEASE_MUTEX();
		return DBERR_DEFAULT;
	}
//  Begin: added by oscar 20110430   	
	if(default_value_addr==0||default_value_addr>=(UINT32)(sto_dev->totol_size))
	{
		DB_RELEASE_MUTEX();	
		return DBERR_DEFAULT;
	} 
// End: added by oscar 20110430 	
	default_start = default_value_addr;
	default_start &= 0x8FFFFFFF;
	//osal_cache_invalidate(default_start + (SYS_FLASH_BASE_ADDR & 0x8fffffff), TABLE_BLOCK_SPACE_SIZE);

/*
	//sub-block head is 16 byte, default db sub-block type must is 0x01
	sto_get_data(sto_dev, tmp_buf, default_start, 7);	
	if(tmp_buf[0] != 0x01)
	{
		DB_PRINTF("set_default_value(): sub-block type=%d error!\n", tmp_buf[0]);
		DB_RELEASE_MUTEX();
		return DBERR_DEFAULT;
	}	
	default_start += 16;
*/
	//check header start flag, node num and end flag
	cur_addr = default_start;
	sto_get_data(sto_dev, tmp_buf, cur_addr, 6);
	MEMCPY(&header_flag,tmp_buf, 4);
	MEMCPY(&node_num,tmp_buf+4, 2);
	if((header_flag != BO_HEADER_START) || (node_num==0))
	{
		DB_PRINTF("default header start or node num error!\n");
		DB_RELEASE_MUTEX();
		return DBERR_DEFAULT;
	}
	sto_get_data(sto_dev, (UINT8 *)&header_flag, cur_addr+6+(NODE_ID_SIZE+NODE_ADDR_SIZE)*node_num, 4);
	if(header_flag != BO_HEADER_END)
	{
		DB_PRINTF("default header end error!\n");
		DB_RELEASE_MUTEX();
		return DBERR_DEFAULT;
	}
	//read node one by one
	MEMSET(sector_node_len, 0, sizeof(sector_node_len));
	DO_get_table_node_length(BO_get_sec_num(), &sector_node_len[0]);
	cur_addr += 6;
	for(i=node_num; i>0; i--)
	{
		//read node id, addr in header
		sto_get_data(sto_dev, node_idx, cur_addr, (NODE_ID_SIZE+NODE_ADDR_SIZE));
		MEMCPY(&node_id, node_idx, NODE_ID_SIZE);
		MEMCPY(&node_addr, node_idx+NODE_ID_SIZE, NODE_ADDR_SIZE);
		//read node len
		sto_get_data(sto_dev, &node_len, default_start+node_addr+NODE_ID_SIZE, 1);
		//read node data
		sto_get_data(sto_dev, tmp_buf, default_start+node_addr, node_len+NODE_ID_SIZE);
		//add node into user db and table
		node_type = DO_get_node_type(node_id);
		if(node_type == TYPE_UNKNOWN_NODE)
		{
			DB_PRINTF("set_default_value(): node id unknown!\n");
			DB_RELEASE_MUTEX();
			return DBERR_DEFAULT;
		}
#ifdef SYS_VOLUME_FOR_PER_PROGRAM		
        if(node_type == TYPE_PROG_NODE)
        {
            extern node_unpack_t db_node_unpacker;
            extern node_pack_t db_node_packer;
            if(db_node_unpacker)
            {
                if(SUCCESS != db_node_unpacker(node_type, tmp_buf, (node_len), &pg, sizeof(pg)))	
                {
                    DB_PRINTF("set_default_value(): node id unknown!\n");
			        DB_RELEASE_MUTEX();
			        return DBERR_DEFAULT;
                }
            }
			
            pg.audio_volume = AUDIO_DEFAULT_VOLUME;
	    if(db_node_packer)
            {
                if(SUCCESS != db_node_packer(node_type,  &pg, sizeof(pg), tmp_buf, &node_len))
                {
                    DB_PRINTF("set_default_value(): node id unknown!\n");
			        DB_RELEASE_MUTEX();
			        return DBERR_DEFAULT;
                }
            }
        }
#endif		


#ifdef User_order_sort

	 if(node_type == TYPE_PROG_NODE)
        {
            extern node_unpack_t db_node_unpacker;
            extern node_pack_t db_node_packer;
            if(db_node_unpacker)
            {
                if(SUCCESS != db_node_unpacker(node_type, tmp_buf, (node_len), &pg, sizeof(pg)))	
                {
                    DB_PRINTF("set_default_value(): node id unknown!\n");
			        DB_RELEASE_MUTEX();
			        return DBERR_DEFAULT;
                }
            }

 	if((tmp_buf[5]&0x10) >>4)
 		pg.user_order =TV_NUM++;
 	else
 		pg.user_order = RADIO_NUM++;

	    if(db_node_packer)
            {
                if(SUCCESS != db_node_packer(node_type,  &pg, sizeof(pg), tmp_buf, &node_len))
                {
                    DB_PRINTF("set_default_value(): node id unknown!\n");
			        DB_RELEASE_MUTEX();
			        return DBERR_DEFAULT;
                }
            }
        }

#endif

#ifdef RAPS_SUPPORT
        if(TYPE_SAT_NODE == node_type)
        {
            extern node_unpack_t db_node_unpacker;
            extern node_pack_t db_node_packer;
            if(db_node_unpacker)
            {
                if(SUCCESS != db_node_unpacker(node_type, tmp_buf, (node_len), &s_node, sizeof(s_node)))	
                {
                    DB_PRINTF("set_default_value(): node id unknown!\n");
			        DB_RELEASE_MUTEX();
			        return DBERR_DEFAULT;
                }
            }
            s_node.reference_chan1 = 0;
            s_node.reference_chan2 = 0;
            if(db_node_packer)
            {
                if(SUCCESS != db_node_packer(node_type,  &s_node, sizeof(s_node), tmp_buf, &node_len))
                {
                    DB_PRINTF("set_default_value(): node id unknown!\n");
			        DB_RELEASE_MUTEX();
			        return DBERR_DEFAULT;
                }
            }            
        }
#endif

		ret = DB_add_node_to_table(&table[node_type],node_id, tmp_buf, node_len+NODE_ID_SIZE);
		if(ret != DB_SUCCES)
		{
			DB_RELEASE_MUTEX();
			return DBERR_DEFAULT;
		}
		table_node_idx = table[node_type].node_num - 1;
		sector_node_len[table[node_type].table_buf[table_node_idx].node_addr[NODE_ADDR_SIZE-1]] += node_len+NODE_ID_SIZE;
		cur_addr += (NODE_ID_SIZE+NODE_ADDR_SIZE);
		
	}
	
	//write table into flash as header
	ret = DB_reclaim_write_header();
	if(ret != DB_SUCCES)
	{
		DB_PRINTF("DB_load_default_node(): write header to flash failed!\n");
		DB_RELEASE_MUTEX();
		return ret;
	}

	ret = DO_set_table_node_length(BO_get_sec_num(), &sector_node_len[0]);
	
	DB_RELEASE_MUTEX();
	return ret;
}


//DB_regist_packer_unpacker(node_packer, node_unpacker);
static INT32 set_multisector_update_buf()
{
	if(db_update_tmp_buf == NULL)
	{
		db_update_tmp_buf = (UINT8 *)MALLOC(DB_SECTOR_SIZE + 3);
		if(db_update_tmp_buf == NULL)
		{
			DB_PRINTF("set_nultisector_update_buf() failed,No memory!\n");
			return DBERR_BUF;
		}
		db_update_tmp_buf = (UINT8 *)(((UINT32)db_update_tmp_buf)&0xFFFFFFFC);
		MEMSET(db_update_tmp_buf,0,DB_SECTOR_SIZE);	
	}
	DB_set_update_buf(db_update_tmp_buf);
	return DB_SUCCES;
}

extern INT32 node_packer(UINT8 node_type, UINT8*src_node, UINT32 src_len,UINT8*packed_node,UINT32 *packed_len);
extern INT32 node_unpacker(UINT8 node_type,UINT8 *packed_node,UINT32 packed_len,UINT8 *unpack_node, UINT32 unpack_len);

// *************************************************
// * Summary:
// *     Description:
// *     database initialization.
// * Returns:
// *     DB_SUCCES if success, Else error value;
// * Parameters:
// *     db_base_addr :   base address of db;
// *     db_length :      length of db;
// *     tmp_info_base :  base address of temp info;
// *     tmp_info_len :   length of temp info;      
INT32 init_db(UINT32 db_base_addr, UINT32 db_length, UINT32 tmp_info_base, UINT32 tmp_info_len)
{
	INT32 ret = DB_SUCCES;
	struct sto_device *sto_flash_dev;

	DB_regist_packer_unpacker(node_packer, node_unpacker);
	
 	sto_flash_dev = (struct sto_device*)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);//(struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
	if (sto_open(sto_flash_dev) != SUCCESS)
	{
		DB_PRINTF("in init_db(): sto_open failed!\n");
		return DBERR_INIT;
	}

	init_tmp_info(tmp_info_base, tmp_info_len);

	if(db_access_sema==OSAL_INVALID_ID)
	{
		db_access_sema = osal_semaphore_create(1);
		if (db_access_sema==OSAL_INVALID_ID)
		{
			DB_PRINTF("init_db():create semaphore failed!\n"); 
			return DBERR_INIT;
		}
	}

	ret = set_multisector_update_buf();
	if(DB_SUCCES!= ret)
		return ret;

	MEMSET(&dynamic_program, 0, sizeof(dynamic_program));
	ret = DB_init(db_base_addr, db_length);
	if(DB_SUCCES!= ret)
		return ret;	

	DB_PRINTF("init_db() finished!\n");
	return DB_SUCCES;
}

INT32 clear_db()
{
	INT32 ret = DB_SUCCES;
#ifdef User_order_sort
	TV_NUM = 0;
	RADIO_NUM = 0;
#endif
	DB_ENTER_MUTEX();
	ret = DB_clear_db();
	DB_RELEASE_MUTEX();
	return ret;
}

INT32 get_node_by_pos(UINT8 n_type, DB_VIEW *v_attr, UINT16 pos, void* node, UINT32 node_len)
{
	INT32 ret = DB_SUCCES;
	UINT32 node_id = 0, node_addr = 0;
	//UINT32 node_len = 0;

	DB_ENTER_MUTEX();
	if(n_type != DB_get_cur_view_type(v_attr))
	{	
		if(n_type == TYPE_PROG_NODE)
		{
			DB_RELEASE_MUTEX();
			DB_PRINTF("get pg at pos %d failed, cur_view_type not pg!\n",pos);
			return DBERR_API_NOFOUND;
		}

		ret = DB_get_node_by_pos_from_table(&db_table[n_type], pos, &node_id, &node_addr);
	}
	else
		ret = DB_get_node_by_pos(v_attr,pos, &node_id, &node_addr);
	
	if(ret == DB_SUCCES)
	{
		if((n_type==TYPE_PROG_NODE) && (dynamic_program.prog.prog_id==node_id)
			&& (dynamic_program.status==DYNAMIC_PG_STEAM_UPDATED))
				MEMCPY(node, &(dynamic_program.prog), node_len);
#ifdef MULTIFEED_SUPPORT
		else if((n_type==TYPE_PROG_NODE)&&(SUCCESS == multifeed_modify_node(node_id, node, TRUE)))
			;
#endif
		else
			ret = DB_read_node(node_id,node_addr,(UINT8 *)node, node_len);
	}
	DB_RELEASE_MUTEX();
	return ret;

}

INT32 get_node_by_id(UINT8 n_type, DB_VIEW *v_attr,UINT32 node_id, void *node, UINT32 node_len)
{
	INT32 ret = DB_SUCCES;
	UINT32 node_addr = 0;
	

	DB_ENTER_MUTEX();
	if(n_type != DB_get_cur_view_type(v_attr))
		ret = DB_get_node_by_id_from_table(&db_table[n_type], node_id, &node_addr);
	else
	{
		ret = DB_get_node_by_id(v_attr,node_id, &node_addr);
		//for prog, in one group view, want to get prog not in this group view
		if(ret == DBERR_API_NOFOUND)
			ret = DB_get_node_by_id_from_table(&db_table[n_type], node_id, &node_addr);
	}

	if(ret == DB_SUCCES)
	{
#ifndef STAR_PROG_SEARCH_NIT_SDT_SUPPORT
		if((n_type==TYPE_PROG_NODE) && (dynamic_program.prog.prog_id==node_id)
			&& (dynamic_program.status==DYNAMIC_PG_STEAM_UPDATED))
				MEMCPY(node, &(dynamic_program.prog), node_len);
#ifdef MULTIFEED_SUPPORT
		else if((n_type==TYPE_PROG_NODE)&&(SUCCESS == multifeed_modify_node(node_id, node, TRUE)))
			;
#endif
		else
#endif
			ret = DB_read_node(node_id,node_addr,(UINT8 *)node, node_len);
	}
	DB_RELEASE_MUTEX();
	return ret;

}


INT32 get_prog_at(UINT16 pos,  P_NODE* node)
{
	return get_node_by_pos(TYPE_PROG_NODE, (DB_VIEW *)&db_view,pos, (void *)node, sizeof( P_NODE));

}

INT32 get_prog_by_id(UINT32 pg_id,  P_NODE *node)
{
	return get_node_by_id(TYPE_PROG_NODE,(DB_VIEW *)&db_view, pg_id, (void *)node, sizeof( P_NODE));
}

INT32 get_tp_by_pos(UINT16 pos,  T_NODE *node)
{
	if(db_search_mode == 1)
	{
		DB_ENTER_MUTEX();
		MEMCPY((UINT8 *)node, db_search_tp_buf+pos*sizeof(T_NODE),sizeof(T_NODE));
		DB_RELEASE_MUTEX();
		return DB_SUCCES;
	}
	else
		return get_node_by_pos(TYPE_TP_NODE, (DB_VIEW *)&db_view,pos, (void *)node, sizeof( T_NODE));
}

INT32 get_tp_by_id(DB_TP_ID tp_id,  T_NODE *node)
{
	DB_VIEW *v_attr;
	UINT8 type;
	UINT32 tmp_id = tp_id;
	tmp_id = tmp_id<<NODE_ID_PG_BIT;
	UINT16 i;
	
	if(db_search_mode == 1)
	{
		DB_ENTER_MUTEX();
		v_attr = (DB_VIEW *)&db_search_view;
		type = TYPE_SEARCH_TP_NODE;
		for(i = 0; i < v_attr->node_num; i++)
		{
			if(0 == MEMCMP((UINT8 *)&tmp_id, v_attr->node_buf[i].node_id, NODE_ID_SIZE))
				break;
		}
		if(i == v_attr->node_num)
		{
			DB_RELEASE_MUTEX();
			return DBERR_API_NOFOUND;
		}
		MEMCPY((UINT8 *)node, db_search_tp_buf+i*sizeof(T_NODE),sizeof(T_NODE));
		DB_RELEASE_MUTEX();
		return DB_SUCCES;
	}
	else
	{
		v_attr = (DB_VIEW *)&db_view;
		type = TYPE_TP_NODE;
		return get_node_by_id(type, v_attr, tmp_id, (void *)node, sizeof( T_NODE));
	}
	
}

/*Used to store particular node id. Add by Ryan Lin*/
static void (* db_node_cmp_callback) (P_NODE *node) = NULL;
void db_node_cmp_callback_reg(void * fun_p)
{
	db_node_cmp_callback = fun_p;
}

void db_node_cmp_callback_unreg()
{
	db_node_cmp_callback = NULL;
}
/*End add*/

// **************************************
// Summary:
//     Description:
//     add a node into db
// Returns:
//     DB_SUCCES if success, 
// 	  else error value in db_return_value.h.
// Parameters:
//     n_type :     node type
//     parent_id :  node's parent's id
//     node :       node to be added
//     ret_id :     returned id for node 
INT32 add_node(UINT8 n_type, UINT32 parent_id,void *node)
{
	INT32 ret = DB_SUCCES;
	UINT8 view_type;
	UINT32 node_len=0;
	DB_VIEW *v_attr = (DB_VIEW *)&db_view;
	UINT32 ret_id = 0;
	UINT32 parent_node_id = 0;
	UINT8 i=0;

	if(n_type == TYPE_SAT_NODE)
	{
		S_NODE *s_node = (S_NODE *)node;
		node_len = sizeof( S_NODE);
		s_node->name_len = sizeof(DB_ETYPE) *(UINT16)DB_STRLEN((DB_ETYPE *)s_node->sat_name);
		parent_node_id = 0;
	}
	else if(n_type==TYPE_TP_NODE || n_type== TYPE_SEARCH_TP_NODE)
	{
		node_len = sizeof( T_NODE);
		parent_node_id = (parent_id<<(NODE_ID_TP_BIT+NODE_ID_PG_BIT));
	}
	else if(n_type==TYPE_PROG_NODE)
	{
		P_NODE *p_node = (P_NODE *)node;
		node_len = sizeof( P_NODE);
#ifdef User_order_sort	
		if( p_node->av_flag == 1)
			p_node->user_order = TV_NUM++;
		else
			p_node->user_order = RADIO_NUM++;
#endif		
		p_node->name_len = sizeof(DB_ETYPE) *(UINT16)DB_STRLEN((DB_ETYPE *)p_node->service_name);
	#if(defined(SUPPORT_MTILINGUAL_SERVICE_NAME))
		for(i=0;i<p_node->mtl_name_count;i++)
		{
			p_node->mtl_name_len[i] = sizeof(DB_ETYPE) *(UINT16)DB_STRLEN((DB_ETYPE *)p_node->mtl_service_name[i]);
		}
	#endif
		p_node->provider_name_len = sizeof(DB_ETYPE) *(UINT16)DB_STRLEN((DB_ETYPE *)p_node->service_provider_name);
		parent_node_id = (parent_id<<NODE_ID_PG_BIT);

	}

	DB_ENTER_MUTEX();
	view_type = DB_get_cur_view_type(v_attr);
	if(n_type != view_type)
	{
		if(n_type == TYPE_SEARCH_TP_NODE)
		{
			v_attr = (DB_VIEW *)&db_search_view;
			n_type = TYPE_TP_NODE;
		}
		else
		{
			DB_RELEASE_MUTEX();
			DB_PRINTF("add_node(): node type %d, not match cur_view_type!\n",n_type);
			return DBERR_API_NOFOUND;
		}
	}
	
	ret = DB_pre_add_node(v_attr, n_type,parent_node_id,node_len, &ret_id);
	if(ret != DB_SUCCES)
	{
		DB_RELEASE_MUTEX();
		if(ret == DBERR_ID_ASSIGN)
		{	
			DB_PRINTF("add_node(): node type %d, no valid id for it!\n",n_type);
			return DBERR_MAX_LIMIT;
		}
		return ret;
	}
	
	if(n_type == TYPE_SAT_NODE)
	{
		S_NODE *s_node = (S_NODE *)node;
		s_node->sat_id = (UINT16)(ret_id>>(NODE_ID_TP_BIT+NODE_ID_PG_BIT));
	}
	else if(n_type==TYPE_TP_NODE)
	{
		T_NODE *t_node = (T_NODE *)node;
		t_node->tp_id = (DB_TP_ID)(ret_id>>NODE_ID_PG_BIT);
		//in search, add new tp into memory buf
		if(db_search_mode == 1)
			MEMCPY(db_search_tp_buf+v_attr->node_num*node_len, node, node_len);
	}
	else if(n_type==TYPE_PROG_NODE)
	{
		P_NODE *p_node = (P_NODE *)node;
		p_node->prog_id = ret_id;
		//in search, add new pg into memory buf
		if(db_search_mode == 1)
			MEMCPY(db_search_pg_buf+v_attr->node_num*node_len, node, node_len);
		if (NULL != db_node_cmp_callback)
			db_node_cmp_callback(node);
	}

	ret = DB_add_node(v_attr, ret_id, node,node_len);
	DB_RELEASE_MUTEX();
	return ret;

}


INT32 modify_node(UINT8 n_type, DB_VIEW *v_attr,UINT32 node_id, void *node, UINT32 node_len)
{
	INT32 ret = DB_SUCCES;
	UINT8 view_type;
	DB_VIEW *view = v_attr;
	UINT8 i=0;

	if(n_type == TYPE_SAT_NODE)
	{
		S_NODE *s_node = (S_NODE *)node;
		s_node->name_len = sizeof(DB_ETYPE) *(UINT16)DB_STRLEN((DB_ETYPE *)s_node->sat_name);
	}
	else if(n_type==TYPE_PROG_NODE)
	{
		P_NODE *p_node = (P_NODE *)node;
		p_node->name_len = sizeof(DB_ETYPE) *(UINT16)DB_STRLEN((DB_ETYPE *)p_node->service_name);
	#if(defined(SUPPORT_MTILINGUAL_SERVICE_NAME))
		for(i=0;i<p_node->mtl_name_count;i++)
		{
			p_node->mtl_name_len[i] = sizeof(DB_ETYPE) *(UINT16)DB_STRLEN((DB_ETYPE *)p_node->mtl_service_name[i]);
		}
	#endif
		p_node->provider_name_len = sizeof(DB_ETYPE) *(UINT16)DB_STRLEN((DB_ETYPE *)p_node->service_provider_name);

	}
	DB_ENTER_MUTEX();
	view_type = DB_get_cur_view_type(view);
	if(n_type != view_type)
	{
		DB_RELEASE_MUTEX();
		DB_PRINTF(" modify_node(): cur_view_type not node type %d!\n",n_type);
		return DBERR_API_NOFOUND;
	}
#ifdef MULTIFEED_SUPPORT
	if((n_type==TYPE_PROG_NODE)&&(SUCCESS == multifeed_modify_node(node_id, node, FALSE)))
	{
		DB_RELEASE_MUTEX();
		return ret;
	}
#endif
	if((n_type==TYPE_PROG_NODE) &&(dynamic_program.prog.prog_id==node_id))
			MEMCPY(&(dynamic_program.prog), node ,node_len);
	
	if ((NULL != db_node_cmp_callback)&&(n_type==TYPE_PROG_NODE))
		db_node_cmp_callback(node);
	
	ret = DB_modify_node_by_id(view, node_id, node, node_len);
	DB_RELEASE_MUTEX();
	return ret;
}


INT32 modify_prog(UINT32 pg_id,  P_NODE *node)
{
	return modify_node(TYPE_PROG_NODE, (DB_VIEW *)&db_view,pg_id, (void *)node, sizeof( P_NODE));
}

INT32 modify_tp(DB_TP_ID tp_id,  T_NODE *node)
{
	UINT8 view_type;
	UINT32 tmp_id = tp_id;
	DB_VIEW *view;
	UINT16 i;

	tmp_id = tmp_id<<NODE_ID_PG_BIT;
	if(db_search_mode == 1)
	{	
		view_type = TYPE_SEARCH_TP_NODE;
		view = (DB_VIEW *)&db_search_view;
		for(i = 0; i < view->node_num; i++)
		{
			if(0 == MEMCMP((UINT8 *)&tmp_id, view->node_buf[i].node_id, NODE_ID_SIZE))
				MEMCPY(db_search_tp_buf+i*sizeof(T_NODE), (UINT8 *)node, sizeof(T_NODE));
		}
	}
	else
	{
		view_type = TYPE_TP_NODE;
		view = (DB_VIEW *)&db_view;
	}
	
	return modify_node(view_type,view, tmp_id, (void *)node, sizeof( T_NODE));
}


INT32 del_node_by_pos(UINT8 n_type,DB_VIEW *v_attr,UINT16 pos)
{
	INT32 ret = DB_SUCCES;

	DB_ENTER_MUTEX();
	if(n_type!= DB_get_cur_view_type(v_attr))
	{
		DB_RELEASE_MUTEX();
		DB_PRINTF(" del_node_by_pos(): cur_view_type not node type %d !\n",n_type);
		return DBERR_API_NOFOUND;
	}
	ret = DB_del_node_by_pos(v_attr, pos);
	DB_RELEASE_MUTEX();
	return ret;

}

INT32 del_node_by_id(UINT8 n_type, DB_VIEW *v_attr,UINT32 node_id)
{
	INT32 ret = DB_SUCCES;

	DB_ENTER_MUTEX();
	if(n_type!= DB_get_cur_view_type(v_attr))
	{
		DB_RELEASE_MUTEX();
		DB_PRINTF(" del_node_by_pos(): cur_view_type not node type %d !\n",n_type);
		return DBERR_API_NOFOUND;
	}
	ret = DB_del_node_by_id(v_attr, node_id);
	DB_RELEASE_MUTEX();
	return ret;

}


INT32 del_prog_at(UINT16 pos)
{
	return del_node_by_pos(TYPE_PROG_NODE,(DB_VIEW *)&db_view, pos);
}

INT32 del_prog_by_id(UINT32 pg_id)
{
	return del_node_by_id(TYPE_PROG_NODE,(DB_VIEW *)&db_view, pg_id);
}


INT32 del_child_prog(UINT8 parent_type, UINT32 parent_id)
{
	INT32 ret = DB_SUCCES;
	UINT32 tmp_id;
	DB_VIEW *view = (DB_VIEW *)&db_view;
	UINT16 i;
	UINT32 node_id = 0, node_addr = 0;
	P_NODE p_node;

	tmp_id = parent_id;
	if(parent_type == TYPE_SAT_NODE)
		tmp_id = tmp_id<<(NODE_ID_PG_BIT+NODE_ID_TP_BIT);
	
	else if(parent_type == TYPE_TP_NODE || parent_type == TYPE_SEARCH_TP_NODE)
	{
		parent_type = TYPE_TP_NODE;
		tmp_id = tmp_id<<(NODE_ID_PG_BIT);
	}
	
	DB_ENTER_MUTEX();
	if(TYPE_PROG_NODE != DB_get_cur_view_type(view))
	{
		DB_RELEASE_MUTEX();
		DB_PRINTF("del_child_prog(): cur_view_type not prog type!\n");
		return DBERR_API_NOFOUND;
	}
	ret = DB_del_node_by_parent_id(view, parent_type, tmp_id);

	//reload prog on this tp into memory
	if(db_search_mode == 1)
	{
		for(i = 0; i < view->node_num; i++)
		{
			DB_get_node_by_pos(view, i, &node_id, &node_addr);
			ret = DB_read_node(node_id, node_addr, (UINT8 *)&p_node, sizeof(P_NODE));
			if(ret != DB_SUCCES)
			{	
				DB_RELEASE_MUTEX();
				return ret;
			}
			MEMCPY(db_search_pg_buf+ i*sizeof(P_NODE), (UINT8 *)&p_node, sizeof(P_NODE));
		}
	}
	DB_RELEASE_MUTEX();
	return ret;
}


//modify the del_child_prog() function as the special function which do not delete the programs in 
//the favorite groups...
INT32 del_child_prog_except_fav_prog(UINT8 parent_type, UINT32 parent_id)
{
	INT32 ret = DB_SUCCES;
	UINT32 tmp_id;
	DB_VIEW *view = (DB_VIEW *)&db_view;
	UINT16 i;
	UINT32 node_id = 0, node_addr = 0;
	P_NODE p_node;

	tmp_id = parent_id;
	if(parent_type == TYPE_SAT_NODE)
		tmp_id = tmp_id<<(NODE_ID_PG_BIT+NODE_ID_TP_BIT);
	
	else if(parent_type == TYPE_TP_NODE || parent_type == TYPE_SEARCH_TP_NODE)
	{
		parent_type = TYPE_TP_NODE;
		tmp_id = tmp_id<<(NODE_ID_PG_BIT);
	}
	
	DB_ENTER_MUTEX();
	if(TYPE_PROG_NODE != DB_get_cur_view_type(view))
	{
		DB_RELEASE_MUTEX();
		DB_PRINTF("del_child_prog_except_fav_prog(): cur_view_type not prog type!\n");
		return DBERR_API_NOFOUND;
	}
	ret = DB_del_node_by_parent_id_except_fav_prog(view, parent_type, tmp_id);

	//reload prog on this tp into memory
	if(db_search_mode == 1)
	{
		for(i = 0; i < view->node_num; i++)
		{
			DB_get_node_by_pos(view, i, &node_id, &node_addr);
			ret = DB_read_node(node_id, node_addr, (UINT8 *)&p_node, sizeof(P_NODE));
			if(ret != DB_SUCCES)
			{	
				DB_RELEASE_MUTEX();
				return ret;
			}
			MEMCPY(db_search_pg_buf+ i*sizeof(P_NODE), (UINT8 *)&p_node, sizeof(P_NODE));
		}
	}
	DB_RELEASE_MUTEX();
	
	return ret;
}


INT32 del_tp_by_id(DB_TP_ID tp_id)
{
	UINT32 tmp_id = tp_id;
	tmp_id = tmp_id<<NODE_ID_PG_BIT;
	return del_node_by_id(TYPE_TP_NODE,(DB_VIEW *)&db_view, tmp_id);
}


UINT16 get_node_pos(UINT8 n_type, DB_VIEW *v_attr,UINT32 node_id)
{
	UINT16 ret;

	DB_ENTER_MUTEX();
	if(n_type!= DB_get_cur_view_type(v_attr))
	{
		DB_RELEASE_MUTEX();
		DB_PRINTF(" get_node_pos(): cur_view_type not node type %d!\n",n_type);
		return 0xFFFF;
	}
	ret = DB_get_node_pos(v_attr,node_id);
	DB_RELEASE_MUTEX();
	return ret;

}

UINT16 get_prog_pos(UINT32 pg_id)
{
	return get_node_pos(TYPE_PROG_NODE,(DB_VIEW *)&db_view, pg_id);
}


INT32 move_prog(UINT16 dest_pos,UINT16 src_pos)
{
	INT32 ret = DB_SUCCES;

	DB_ENTER_MUTEX();
	if(TYPE_PROG_NODE!= DB_get_cur_view_type((DB_VIEW *)&db_view))
	{
		DB_RELEASE_MUTEX();
		DB_PRINTF(" move_prog(): cur_view_type not pg!\n");
		return DBERR_API_NOFOUND;
	}
	ret = DB_move_node((DB_VIEW *)&db_view, src_pos, dest_pos);
	DB_RELEASE_MUTEX();
	return ret;

}


// ***********************************************************
// * Summary:
// *     Description:
// *     discard the operations made before, clear cmd buffer.
// * Returns:
// *     DB_SUCCES if success, Else error value;              
INT32 db_unsave_data_changed()
{
	return DB_clear_cmd_buf();
}


static BOOL create_prog_view_filter(UINT32 id, UINT32 addr, UINT16 create_mode,UINT32 param)
{
	UINT32 i;	
	 P_NODE node;
	 UINT8 tmp_buf[P_NODE_FIX_LEN];
	INT32 ret;
	ret = TRUE;
	union filter_mode view_mode;
	
	//view_mode.decode.group_mode = (UINT8)(create_mode>>8);
	view_mode.decode.av_mode = (UINT8)(create_mode);

	//DB_read_node(id, addr, (UINT8 *)&node, sizeof( P_NODE));
	//for speed up, only read the 12 byte of p_node after id and len
	if(((create_mode&0xFF00)==VIEW_CAS) || ((create_mode&0xFF00)==VIEW_PROVIDER))
	{
		return TRUE;
	}
	
	//if(view_mode.decode.av_mode != PROG_TVRADIO_MODE)
	{
		BO_read_data(addr+NODE_ID_SIZE+1, P_NODE_FIX_LEN, (UINT8 *)&tmp_buf);
		MEMCPY((UINT8 *)&(node.prog_id)+sizeof(UINT32), tmp_buf, P_NODE_FIX_LEN);
	}
	
	if ((create_mode&0xFF00)==VIEW_SINGLE_SAT) 
	{
		ret = ((id&NODE_ID_SET_MASK) == (param<<(NODE_ID_PG_BIT+NODE_ID_TP_BIT)));
	}
	else if ((create_mode&0xFF00)==VIEW_SINGLE_TP)
	{
		ret = ((id&NODE_ID_TP_MASK)==(param<<NODE_ID_PG_BIT));
	}
	else if ((create_mode&0xFF00)== VIEW_FAV_GROUP) 
	{
		ret = (((node.fav_group[0]>>param)&0x01)==1);
	}
#if (SYS_PROJECT_FE == PROJECT_FE_DVBC ||(SYS_PROJECT_FE==PROJECT_FE_DVBT && defined(STAR_PROG_SEARCH_BAT_SUPPORT)))
	else if((create_mode&0xFF00)== VIEW_BOUQUIT_ID) 
	{
#ifdef MULTI_BOUQUET_ID_SUPPORT
		UINT16 i = 0;
		ret = FALSE;
		for(i=0 ; i< node.bouquet_count ; i++)
		{
			if(ret = node.bouquet_id[i]==param)
			{	
				break;
			}
		}
#else
		ret = node.bouquet_id==param;	
#endif
	}
#endif
	if(view_mode.decode.av_mode == PROG_ALL_MODE)
		return ret;
	//node service type not valid
	if(node.service_type==0)
	{
		if((view_mode.decode.av_mode==PROG_TVRADIO_MODE)
			||(view_mode.decode.av_mode==PROG_TVRADIODATA_MODE))
			return ret;
		else
			return ret&&((node.av_flag == view_mode.decode.av_mode));
	}
	if(psi_service_type_exist(node.service_type)!= RET_SUCCESS||node.service_type==SERVICE_TYPE_DATA_SERVICE)
	{
		if((view_mode.decode.av_mode == PROG_TVRADIO_MODE)||(view_mode.decode.av_mode == PROG_TVRADIODATA_MODE))
		{
			return ret;
		}
		else if((view_mode.decode.av_mode == PROG_TV_MODE)||(view_mode.decode.av_mode == PROG_TVDATA_MODE))
		{
			return node.av_flag;
		}
		else if(view_mode.decode.av_mode == PROG_RADIO_MODE)
		{
			return !node.av_flag;
		}
		else
		{
			return FALSE;
		}
	}
	if(view_mode.decode.av_mode == PROG_NVOD_MODE)
		return ret&&(node.service_type==SERVICE_TYPE_NVOD_TS);
	else if(view_mode.decode.av_mode == PROG_TVRADIO_MODE)
		return ret&&((node.service_type==SERVICE_TYPE_DTV) ||\
		                   (node.service_type==SERVICE_TYPE_DRADIO)||\
		                   (node.service_type==SERVICE_TYPE_FMRADIO)||\
		                    (node.service_type==SERVICE_TYPE_HD_MPEG4)||\
                   (node.service_type==SERVICE_TYPE_DATABROAD)||\
    		        (node.service_type==SERVICE_TYPE_DATA_SERVICE)||\
			(node.service_type==SERVICE_TYPE_SD_MPEG4) ||\
			(node.service_type==SERVICE_TYPE_HD_MPEG2));
	else if(view_mode.decode.av_mode == PROG_TV_MODE)
		return ret&&((node.service_type==SERVICE_TYPE_DTV)||\
			(node.service_type==SERVICE_TYPE_HD_MPEG4)||\
			(node.service_type==SERVICE_TYPE_SD_MPEG4) ||\
			(node.service_type==SERVICE_TYPE_HD_MPEG2));
	else if(view_mode.decode.av_mode == PROG_RADIO_MODE)
		return ret&&((node.service_type==SERVICE_TYPE_DRADIO)||\
					(node.service_type==SERVICE_TYPE_FMRADIO));
	else if(view_mode.decode.av_mode == PROG_DATA_MODE)
		return ret&&(node.service_type==SERVICE_TYPE_DATABROAD);
	else if(view_mode.decode.av_mode == PROG_TVDATA_MODE)
		return ret&&((node.service_type==SERVICE_TYPE_DTV)
		                    ||(node.service_type==SERVICE_TYPE_DATABROAD));
	else if(view_mode.decode.av_mode == PROG_PRIVATE_MODE)
		return ret&&(node.service_type==SERVICE_TYPE_PRIVATE);
	else if(view_mode.decode.av_mode == PROG_TVRADIODATA_MODE)
		return ret&&((node.service_type==SERVICE_TYPE_DTV)\
		                    ||(node.service_type==SERVICE_TYPE_DRADIO)\
		                    ||(node.service_type==SERVICE_TYPE_FMRADIO)\
		                    ||(node.service_type==SERVICE_TYPE_DATABROAD));		                    
	else
		return ret&&((node.service_type!=SERVICE_TYPE_NVOD_TS)
		                    &&(node.service_type!=SERVICE_TYPE_DATABROAD)
		                    &&(node.service_type!=SERVICE_TYPE_PRIVATE)
		                    &&(node.av_flag == view_mode.decode.av_mode));

}


void *get_cur_view(void)
{
	return &db_view;
}

static UINT16 last_create_flag;
static UINT32 last_param;

INT32 get_cur_view_feature(UINT16 *create_flag, UINT32 *param)
{
	DB_ENTER_MUTEX();
	*create_flag = last_create_flag;
	*param = last_param;
	DB_RELEASE_MUTEX();
	return 0;
}

INT32 recreate_prog_view(UINT16 create_flag, UINT32 param)
{
	INT32 ret = DB_SUCCES;

	DB_ENTER_MUTEX();
	ret = DB_create_view(TYPE_PROG_NODE, (DB_VIEW *)&db_view,(db_filter_t)create_prog_view_filter, NULL, create_flag, param);

	db_unsave_data_changed();
	last_create_flag = create_flag;
	last_param = param;
	DB_RELEASE_MUTEX();
	return ret;

}

INT32 recreate_special_pip_view(UINT16 type, UINT32 *cal_back, UINT32 *param)
{
	INT32 ret = DB_SUCCES;
	UINT16 fil_mode = 0xffff;

	if(param == NULL)
		return DBERR_PARAM;
	DB_ENTER_MUTEX();
	ret = DB_create_view(type, (DB_VIEW *)&db_view,NULL, (db_filter_pip)cal_back, fil_mode, (UINT32)param);

	db_unsave_data_changed();
	DB_RELEASE_MUTEX();
	return ret;

}


static BOOL create_tp_view_filter(UINT32 id, UINT32 addr, UINT16 create_mode,UINT32 param)
{
	BOOL ret = TRUE;
		
	if ((create_mode&0xFF00)==VIEW_SINGLE_SAT) 
	{
		ret = ((id&NODE_ID_SET_MASK) == (param<<(NODE_ID_PG_BIT+NODE_ID_TP_BIT)));
	}
		
	return ret;
}

INT32 recreate_tp_view(UINT16 create_flag, UINT32 param)
{
	INT32 ret = DB_SUCCES;

	DB_ENTER_MUTEX();
	ret = DB_create_view(TYPE_TP_NODE, (DB_VIEW *)&db_view,(db_filter_t)create_tp_view_filter, NULL, create_flag, param);

	db_unsave_data_changed();
	DB_RELEASE_MUTEX();
	return ret;
}

//for search, create special tp view
INT32 db_search_create_tp_view(UINT16 parent_id)
{
	UINT16 node_num = 0;
	DB_TABLE *t = &db_table[TYPE_TP_NODE];
	DB_VIEW *v_attr = (DB_VIEW *)&db_search_view;
	UINT16 i;
	UINT32 node_id = 0;UINT32 node_addr = 0;
	 T_NODE t_node;
	INT32 ret = DB_SUCCES;
	UINT32 tmp_id = parent_id;

	DB_ENTER_MUTEX();
	tmp_id = (tmp_id<<(NODE_ID_PG_BIT+NODE_ID_TP_BIT));
	MEMSET((UINT8 *)v_attr, 0, sizeof(DB_VIEW));
	v_attr->pre_type = v_attr->cur_type = TYPE_SEARCH_TP_NODE;
	v_attr->max_cnt = MAX_TP_NUM;
	v_attr->node_buf = (NODE_IDX_PARAM *)db_search_view_buf;
	for(i = 0;i < t->node_num; i++)
	{
		MEMCPY((UINT8 *)&node_id, t->table_buf[i].node_id, NODE_ID_SIZE);
		MEMCPY((UINT8 *)&node_addr, t->table_buf[i].node_addr, NODE_ADDR_SIZE);

		if((parent_id == 0) || ((node_id&NODE_ID_SET_MASK)==tmp_id))
		{
			MEMCPY(v_attr->node_buf[node_num].node_id, (UINT8 *)&node_id,NODE_ID_SIZE);
			MEMCPY(v_attr->node_buf[node_num].node_addr,(UINT8 *)&node_addr,NODE_ADDR_SIZE);
			node_num++;
		}

	}
	v_attr->node_num = node_num;
	v_attr->cur_filter = NULL;
	if(parent_id == 0)
		v_attr->cur_filter_mode = VIEW_ALL;
	else
		v_attr->cur_filter_mode = VIEW_SINGLE_SAT;
	v_attr->view_param = parent_id;

	//load t_node of this view into memory
	for(i = 0; i < v_attr->node_num; i++)
	{
		DB_get_node_by_pos(v_attr,i, &node_id, &node_addr);
		ret = DB_read_node(node_id, node_addr, (UINT8 *)&t_node, sizeof( T_NODE));
		if(ret != DB_SUCCES)
		{	
			DB_RELEASE_MUTEX();
			return ret;
		}
		MEMCPY(db_search_tp_buf+ i*sizeof( T_NODE), (UINT8 *)&t_node, sizeof( T_NODE));
	}
	
	DB_RELEASE_MUTEX();
	return DB_SUCCES;
}

                            
// *****************************************************************
// * Summary:
// *     Description:
// *     create a view of pgs on the searching sat or tp, loading them into
// *     memory for looking up.
// * Returns:
// *     DB_SUCCES if success,Else error value;
// * Parameters:
// *     parnet_id :  the searching sat or tp's id;                            
INT32 db_search_create_pg_view(UINT8 parent_type, UINT32 parent_id, UINT8 prog_mode)
{
	INT32 ret = DB_SUCCES;
	UINT16 i;
	DB_VIEW *v_attr = (DB_VIEW *)&db_view;
	UINT32 node_id = 0, node_addr = 0;
	 P_NODE p_node;
	 UINT16 parent_group = VIEW_SINGLE_SAT;
	 UINT32 tmp_id = parent_id;
	
	if(parent_type == TYPE_SAT_NODE)
		parent_group = VIEW_SINGLE_SAT;
	else if(parent_type == TYPE_TP_NODE)
		parent_group = VIEW_SINGLE_TP;
	

	DB_ENTER_MUTEX();
	ret = DB_create_view(TYPE_PROG_NODE, v_attr,(db_filter_t)create_prog_view_filter, NULL, parent_group|prog_mode, tmp_id);
	if(ret != DB_SUCCES)
	{	
		DB_RELEASE_MUTEX();
		return ret;
	}
	
	//load prog on this tp into memory
	for(i = 0; i < v_attr->node_num; i++)
	{
		DB_get_node_by_pos(v_attr,i, &node_id, &node_addr);
		ret = DB_read_node(node_id, node_addr, (UINT8 *)&p_node, sizeof( P_NODE));
		if(ret != DB_SUCCES)
		{	
			DB_RELEASE_MUTEX();
			return ret;
		}
		MEMCPY(db_search_pg_buf+ i*sizeof( P_NODE), (UINT8 *)&p_node, sizeof( P_NODE));
	}

	DB_RELEASE_MUTEX();
	return ret;
}

INT32 db_search_init(UINT8 *search_buf, UINT32 buf_len)
{
	DB_ENTER_MUTEX();
	db_search_mode = 1;	
	//assign buffer to search view, search tp buf and search pg buf
	MEMSET(search_buf, 0, buf_len);
	db_search_view_buf =  (UINT8 *)((UINT32)search_buf&0x8FFFFFFF);
	db_search_tp_buf =  (UINT8 *)(((UINT32)db_search_view_buf + MAX_TP_NUM*sizeof(NODE_IDX_PARAM)+3)&0x8FFFFFFC);
	db_search_pg_buf = (UINT8 *)(((UINT32)db_search_tp_buf + SEARCH_BUF_TP_NUM*sizeof( T_NODE)+3)&0x8FFFFFFC);
	
	DB_RELEASE_MUTEX();
}

INT32 db_search_exit()
{
	DB_ENTER_MUTEX();
	db_search_mode = 0;	
	MEMSET(&db_search_view, 0, sizeof(DB_VIEW));
	db_search_view.cur_type = TYPE_UNKNOWN_NODE;
	//db_search_view_buf = NULL;
	//db_search_tp_buf = NULL;
	//db_search_pg_buf = NULL;
	DB_RELEASE_MUTEX();
	return DB_SUCCES;
}


// ***************************************************
// * Summary:
// *     Description:
// *     look up if there is a same node in db.
// * Returns:
// *     DB_SUCCES if success, Else DBERR_API_NOFOUND;
// * Parameters:
// *     n_type :  node type;
// *     node :    node data;                         
INT32 lookup_node(UINT8 n_type, void *node, UINT32 parent_id)
{
	DB_VIEW *v_attr = (DB_VIEW *)&db_view;
	UINT16 i;
	UINT32 node_id =0, node_addr = 0;
	UINT32 tmp_id = parent_id;

	 P_NODE tmp_buf;
	 UINT32 node_len = 0;
	 UINT32 id_mask=0;

	DB_ENTER_MUTEX();
	if(n_type != DB_get_cur_view_type(v_attr))
	{
		DB_RELEASE_MUTEX();
		DB_PRINTF("look_up_node(): node type %d, not match cur_view_type!\n",n_type);
		return DBERR_API_NOFOUND;
	}
	/*
	if(n_type == TYPE_SAT_NODE)
	{
		 S_NODE tmp_s;
		
		for(i = 0; i < v_attr->node_num; i++)
		{
			DB_get_node_by_pos(v_attr, i, &node_id, &node_addr);
			DB_read_node(node_id, node_addr, (UINT8 *)&tmp_s, sizeof( S_NODE));
			if(TRUE == db_same_node_checker(TYPE_SAT_NODE, (void *)&tmp_s,node))
			{
				DB_RELEASE_MUTEX();
				return DB_SUCCES;
			}
		}
		
	}
	else if(n_type == TYPE_TP_NODE)
	{
		 T_NODE tmp_t;
		
		for(i = 0; i < v_attr->node_num; i++)
		{
			DB_get_node_by_pos(v_attr, i, &node_id, &node_addr);
			if((node_id&NODE_ID_SET_MASK) == (tmp_id<<(NODE_ID_PG_BIT+NODE_ID_TP_BIT)))
			{
				DB_read_node(node_id, node_addr, (UINT8 *)&tmp_t, sizeof( T_NODE));
				if(TRUE == db_same_node_checker(TYPE_TP_NODE, (void *)&tmp_t,node))
				{
					DB_RELEASE_MUTEX();
					return DB_SUCCES;
				}
			}
		}
		
	}
	else if(n_type == TYPE_PROG_NODE)
	{
		 P_NODE tmp_p;
		
		for(i = 0; i < v_attr->node_num; i++)
		{
			DB_get_node_by_pos(v_attr, i, &node_id, &node_addr);
			if((node_id&NODE_ID_TP_MASK) == (tmp_id<<NODE_ID_PG_BIT))
			{
				DB_read_node(node_id, node_addr, (UINT8 *)&tmp_p, sizeof( P_NODE));
				if(TRUE == db_same_node_checker(TYPE_PROG_NODE, (void *)&tmp_p,node))
				{
					DB_RELEASE_MUTEX();
					return DB_SUCCES;
				}
			}
		}
		
	}
	*/
	MEMSET(&tmp_buf, 0, sizeof(P_NODE));
	if(n_type == TYPE_SAT_NODE)
	{	
		node_len = sizeof( S_NODE);
		id_mask = 0x0;
		
	}
	else if(n_type == TYPE_TP_NODE)
	{
		node_len = sizeof( T_NODE);
		id_mask = NODE_ID_SET_MASK;
		tmp_id = parent_id<<(NODE_ID_PG_BIT+NODE_ID_TP_BIT);
	}
	else if(n_type == TYPE_PROG_NODE)
	{
		node_len = sizeof( P_NODE);
		id_mask = NODE_ID_TP_MASK;
		tmp_id = parent_id<<NODE_ID_PG_BIT;
	}
	for(i = 0; i < v_attr->node_num; i++)
	{
		DB_get_node_by_pos(v_attr, i, &node_id, &node_addr);
		if((node_id&id_mask) == tmp_id)
		{
			DB_read_node(node_id, node_addr, (UINT8 *)&tmp_buf, node_len);
			if(TRUE == db_same_node_checker(n_type, (void *)&tmp_buf, node))
			{
				DB_RELEASE_MUTEX();
				return DB_SUCCES;
			}
		}
	}
	
	DB_RELEASE_MUTEX();
	return DBERR_API_NOFOUND;
}


// *****************************************************
// * Summary:
// *     Description:
// *     look up if there is a same node in view buffer.
// * Returns:
// *     DB_SUCCES if success, else DBERR_API_NOFOUND.
// * Parameters:
// *     n_type :  node type;
// *     node :    node data found in search;           
INT32 db_search_lookup_node(UINT8 n_type, void *node)
{
	UINT16 i;
	UINT32 node_len=0;
	UINT8 *buf=NULL;
	DB_VIEW *v_attr=NULL;
	
	DB_ENTER_MUTEX();
	/*
	if(n_type == TYPE_SEARCH_TP_NODE)
	{
		 //T_NODE tmp_t;
		v_attr = (DB_VIEW *)&db_search_view;
		for(i = 0; i < v_attr->node_num; i++)
		{
			//MEMCPY((UINT8 *)&tmp_t, db_search_tp_buf+ i*sizeof( T_NODE),sizeof( T_NODE));
			//if(TRUE == db_same_node_checker(TYPE_TP_NODE, (void *)&tmp_t,node))
			if(TRUE == db_same_node_checker(TYPE_TP_NODE, (void *)(db_search_tp_buf+ i*sizeof(T_NODE)),node))	
			{
				DB_RELEASE_MUTEX();
				return DB_SUCCES;
			}
		}
		//if no same tp in search view, add the tp into db_search_tp_buf
		MEMCPY(db_search_tp_buf+ v_attr->node_num*sizeof( T_NODE), node, sizeof( T_NODE));
	}
	else if(n_type == TYPE_PROG_NODE)
	{
		 //P_NODE tmp_p;
		v_attr = (DB_VIEW *)&db_view;
		for(i = 0; i < v_attr->node_num; i++)
		{
			//MEMCPY((UINT8 *)&tmp_p, db_search_pg_buf+ i*sizeof( P_NODE),sizeof( P_NODE));
			//if(TRUE == db_same_node_checker(TYPE_PROG_NODE,(void *)&tmp_p,node))
			if(TRUE == db_same_node_checker(TYPE_PROG_NODE,(void *)(db_search_pg_buf+ i*sizeof(P_NODE)),node))
			{
				DB_RELEASE_MUTEX();
				return DB_SUCCES;
			}
		}
		//if no same pg in db view, add the pg into db_search_pg_buf
		MEMCPY(db_search_pg_buf+ v_attr->node_num*sizeof( P_NODE), node, sizeof( P_NODE));
	}
	*/
	if(n_type == TYPE_SEARCH_TP_NODE)
	{
		v_attr = (DB_VIEW *)&db_search_view;
		node_len = sizeof(T_NODE);
		buf = db_search_tp_buf;
	}
	else if(n_type == TYPE_PROG_NODE)
	{
		v_attr = (DB_VIEW *)&db_view;
		node_len = sizeof( P_NODE);
		buf = db_search_pg_buf;
	}
	for(i = 0; i < v_attr->node_num; i++)
	{
		if(TRUE == db_same_node_checker(n_type,(void *)(buf + i*node_len),node))
		{
			DB_RELEASE_MUTEX();
			return DB_SUCCES;
		}
	}
	//if no same node in db view, add it into db search buf
	//not correct, currently, the node id not assigned yet,still 0
	//MEMCPY(buf+v_attr->node_num*node_len, node, node_len);
	
	DB_RELEASE_MUTEX();
	return DBERR_API_NOFOUND;
}
// *****************************************************
// * Summary:
// *     Description:
// *     look up if there is a same tp node in view buffer.
// * Returns:
// *     DB_SUCCES if success, else DBERR_API_NOFOUND.
// * Parameters:
// *     n_type :  node type;
// *     node :    node data found in search;       
#if (SYS_PROJECT_FE == PROJECT_FE_DVBC)
INT32 db_search_lookup_tpnode(UINT8 n_type, void *node)
{
	UINT16 i;
	UINT32 node_len=0;
	UINT8 *buf=NULL;
	DB_VIEW *v_attr=NULL;
	
	DB_ENTER_MUTEX();
	if(n_type == TYPE_SEARCH_TP_NODE)
	{
		v_attr = (DB_VIEW *)&db_search_view;
		node_len = sizeof(T_NODE);
		buf = db_search_tp_buf;
	}

	for(i = 0; i < v_attr->node_num; i++)
	{
		if(TRUE == db_same_tpnode_checker(n_type,(void *)(buf + i*node_len),node))
		{
			DB_RELEASE_MUTEX();
			return DB_SUCCES;
		}
	}
	//if no same node in db view, add it into db search buf
	//not correct, currently, the node id not assigned yet,still 0
	//MEMCPY(buf+v_attr->node_num*node_len, node, node_len);
	
	DB_RELEASE_MUTEX();
	return DBERR_API_NOFOUND;
}
#endif

// **********************************************
// * Summary:
// *     Description:
// *     save the operations into flash.
// * Returns:
// *     DB_SUCCES if success, Else error value; 
INT32 update_data( )
{
	INT32 ret = DB_SUCCES;

	DB_ENTER_MUTEX();
	ret = DB_update_operation();
	DB_RELEASE_MUTEX();
	return ret;
}

UINT16 get_node_num(UINT8 n_type, DB_VIEW *view,db_filter_t filter,UINT16 filter_mode,UINT32 param)
{
	UINT16 i;
	UINT16 j=0;
	UINT32 node_id = 0, node_addr = 0;

	if(n_type != DB_get_cur_view_type(view))
	{
		DB_TABLE *t = (DB_TABLE *)&db_table[n_type];
		for(i = 0; i < t->node_num; i++)
		{
			DB_get_node_by_pos_from_table(t,i, &node_id, &node_addr);
			if(filter==NULL || TRUE == filter(node_id, node_addr, filter_mode, param))
				j++;
		}
		return j;
	}
	else
		return DB_get_node_num(view, filter, filter_mode, param);
}

UINT16 get_prog_num(UINT16 filter_mode,UINT32 param)
{
	UINT16 num = 0;

	DB_ENTER_MUTEX();
	num = get_node_num(TYPE_PROG_NODE,(DB_VIEW *)&db_view, (db_filter_t)create_prog_view_filter, filter_mode, param);
	DB_RELEASE_MUTEX();
	return num;
}
	
static BOOL is_tv_node(P_NODE *p_node)
{
    BOOL ret = FALSE;
    if( 0 == p_node->service_type || p_node->service_type == SERVICE_TYPE_DATA_SERVICE )
    {
		if(p_node->av_flag == 1)
			ret = TRUE;
    }else if(SERVICE_TYPE_DTV == p_node->service_type || SERVICE_TYPE_HD_MPEG4 == p_node->service_type \
                || p_node->service_type == SERVICE_TYPE_SD_MPEG4 || SERVICE_TYPE_HD_MPEG2 == p_node->service_type ||\
                SERVICE_TYPE_DATA_SERVICE == p_node->service_type)
    {
        ret = TRUE;
    }
    return ret;
}
INT32 get_specific_prog_num(UINT8 prog_flag,UINT16 *ch_v_cnt,UINT16 *ch_a_cnt,UINT16 *fav_v_cnt,UINT16 *fav_a_cnt)
{
	INT32 ret = DB_SUCCES;
	UINT16 sat_num, tp_num, prog_num;
	UINT16 i, j;
	DB_TABLE *table = (DB_TABLE *)&db_table[0];
	UINT32 sat_id = 0, tp_id = 0,prog_addr = 0;
	P_NODE p_node;
	//UINT8 tmp_buf[P_NODE_FIX_LEN];
	UINT8 tmp_buf[NODE_ID_SIZE+1+P_NODE_FIX_LEN];
	UINT16 *node_num_buf;

	DB_ENTER_MUTEX();
	
	sat_num = table[TYPE_SAT_NODE].node_num;
	tp_num = table[TYPE_TP_NODE].node_num;
	prog_num = table[TYPE_PROG_NODE].node_num;

	for(i = 0; i < prog_num; i++)
	{
		MEMCPY((UINT8 *)&prog_addr, table[TYPE_PROG_NODE].table_buf[i].node_addr, NODE_ADDR_SIZE);
		BO_read_data(prog_addr, NODE_ID_SIZE+1+P_NODE_FIX_LEN, (UINT8 *)&tmp_buf);
		MEMCPY((UINT8 *)&(p_node.prog_id),tmp_buf, NODE_ID_SIZE);
		MEMCPY((UINT8 *)&(p_node.prog_id)+sizeof(UINT32), tmp_buf+NODE_ID_SIZE+1, P_NODE_FIX_LEN);

		//not normal tv or radio prog
#if 0
		if((p_node.service_type==SERVICE_TYPE_NVOD_TS)||(p_node.service_type==SERVICE_TYPE_DATABROAD)
			||(p_node.service_type==SERVICE_TYPE_PRIVATE))
#else
		if(create_prog_view_filter(p_node.prog_id,prog_addr,VIEW_ALL | PROG_TVRADIO_MODE,0)!=TRUE)
#endif
			continue;
		if(prog_flag&SAT_PROG_NUM)
		{
            
			if(is_tv_node(&p_node))
				node_num_buf = ch_v_cnt;
			else
				node_num_buf = ch_a_cnt;
			for(j = 0; j < sat_num; j++)
			{
				MEMCPY((UINT8 *)&sat_id, table[TYPE_SAT_NODE].table_buf[j].node_id, NODE_ID_SIZE);
				if((p_node.prog_id&NODE_ID_SET_MASK)==sat_id)
					break;
			}
			node_num_buf[j]++;
		}
		
		if(prog_flag&TP_PROG_NUM)
		{
			if(is_tv_node(&p_node))
				node_num_buf = ch_v_cnt;
			else
				node_num_buf = ch_a_cnt;
			for(j = 0; j < tp_num; j++)
			{
				MEMCPY((UINT8 *)&tp_id, table[TYPE_TP_NODE].table_buf[j].node_id, NODE_ID_SIZE);
				if((p_node.prog_id&NODE_ID_TP_MASK)==tp_id)
					break;
			}
			node_num_buf[j]++;
		}
		
		if(prog_flag&FAV_PROG_NUM)
		{
			if(is_tv_node(&p_node))
				node_num_buf = fav_v_cnt;
			else
				node_num_buf = fav_a_cnt;
			for(j=0; j<32; j++)
			{
				if(((p_node.fav_group[0]>>j)&0x01) == 1)
					node_num_buf[j]++;
			}
		}

	}

	DB_RELEASE_MUTEX();
	return ret;
}


BOOL check_node_modified(UINT8 n_type)
{
	BOOL ret;
	DB_VIEW *view = (DB_VIEW *)&db_view;

	DB_ENTER_MUTEX();
	
	if(n_type != DB_get_cur_view_type(view))
	{
		DB_RELEASE_MUTEX();
		DB_PRINTF(" check_node_modified(): cur_view_type not pg!\n");
		ASSERT(0);
	}
	ret = DB_check_node_changed(view);
	DB_RELEASE_MUTEX();
	return ret;	
}

INT32 set_node_del_flag(UINT8 n_type,DB_VIEW *view,UINT16 pos, UINT8 flag)
{
	INT32 ret = DB_SUCCES;
	
	DB_ENTER_MUTEX();
	if(n_type!= DB_get_cur_view_type(view))
	{
		DB_RELEASE_MUTEX();
		DB_PRINTF(" set_node_del_flag(): cur_view_type not node type %d !\n",n_type);
		return DBERR_API_NOFOUND;
	}
	if(view->node_flag != NULL)
	{
		if(flag == 1)
			view->node_flag[pos] |= V_NODE_DEL_FLAG;
		else 
			view->node_flag[pos] &= ~V_NODE_DEL_FLAG;

	}
	DB_RELEASE_MUTEX();
	return ret;	
}

BOOL get_node_del_flag(UINT8 n_type,DB_VIEW *view,UINT16 pos)
{
	BOOL ret = FALSE;
	
	DB_ENTER_MUTEX();
	if(n_type!= DB_get_cur_view_type(view))
	{
		DB_RELEASE_MUTEX();
		DB_PRINTF(" get_node_del_flag(): cur_view_type not node type %d !\n",n_type);
		return ret;
	}
	if(view->node_flag != NULL)
	{
		if((view->node_flag[pos]&V_NODE_DEL_FLAG)==0)
			ret = FALSE;
		else 
			ret = TRUE;
	}
	DB_RELEASE_MUTEX();
	return ret;	
}


INT32 set_prog_del_flag(UINT16 pos, UINT8 flag)
{
	return set_node_del_flag(TYPE_PROG_NODE, (DB_VIEW *)&db_view, pos, flag);
}

BOOL get_prog_del_flag(UINT16 pos)
{
	return get_node_del_flag(TYPE_PROG_NODE, (DB_VIEW *)&db_view, pos);
}



void get_audio_lang2b(UINT8 *src,UINT8 *des)
{
	UINT8 s0,s1,s2;

	if(src[0] == 0&&src[1]==0&&src[2]==0)
		return;
	s0 = src[0]-0x61;
	s1 = src[1]-0x61;
	s2 = src[2]-0x61;

	des[0] = (s0 << 3) |(s1 >> 2);
	des[1] = ((s1 << 6)&0xC0)|(s2 << 1);
}

void get_audio_lang3b(UINT8 *src,UINT8 *des)
{
	if(src[0] == 0&&src[1] == 0)
		return;
	des[0] = (src[0] >> 3) + 0x61;
	des[1] = (((src[0] << 2)&0x1C) |(src[1] >> 6)) + 0x61;
	des[2] = ((src[1] >> 1)&0x1F) + 0x61;

}

void db_get_search_nodebuf(UINT32 *pg_buf, UINT32 *tp_buf)
{
	*pg_buf = (UINT32)db_search_pg_buf;
	*tp_buf = (UINT32)db_search_tp_buf;
}

#ifdef RAPS_SUPPORT

static BOOL all_data_is_0xff(UINT8 *addr, UINT32 len)
{
    UINT32 i;
    for (i = 0; i < len; i++)
    {
        if (addr[i] != 0xff)
            return FALSE;
    }
    return TRUE;
}

INT32 add_node_raps_fav(UINT8 n_type, UINT32 parent_id,void *node, UINT32 *fav_prog_id)
{
	INT32 ret = DB_SUCCES;
	UINT8 view_type;
	UINT32 node_len=0;
	DB_VIEW *v_attr = (DB_VIEW *)&db_view;
	UINT32 ret_id = 0;
	UINT32 parent_node_id = 0;

	if(n_type == TYPE_SAT_NODE)
	{
		S_NODE *s_node = (S_NODE *)node;
		node_len = sizeof( S_NODE);
		s_node->name_len = sizeof(DB_ETYPE) *(UINT16)DB_STRLEN((DB_ETYPE *)s_node->sat_name);
		parent_node_id = 0;
	}
	else if(n_type==TYPE_TP_NODE || n_type== TYPE_SEARCH_TP_NODE)
	{
		node_len = sizeof( T_NODE);
		parent_node_id = (parent_id<<(NODE_ID_TP_BIT+NODE_ID_PG_BIT));
	}
	else if(n_type==TYPE_PROG_NODE)
	{
		P_NODE *p_node = (P_NODE *)node;
		node_len = sizeof( P_NODE);
		p_node->name_len = sizeof(DB_ETYPE) *(UINT16)DB_STRLEN((DB_ETYPE *)p_node->service_name);
		p_node->provider_name_len = sizeof(DB_ETYPE) *(UINT16)DB_STRLEN((DB_ETYPE *)p_node->service_provider_name);
		parent_node_id = (parent_id<<NODE_ID_PG_BIT);

	}

	DB_ENTER_MUTEX();
	view_type = DB_get_cur_view_type(v_attr);
	if(n_type != view_type)
	{
		if(n_type == TYPE_SEARCH_TP_NODE)
		{
			v_attr = (DB_VIEW *)&db_search_view;
			n_type = TYPE_TP_NODE;
		}
		else
		{
			DB_RELEASE_MUTEX();
			DB_PRINTF("add_node(): node type %d, not match cur_view_type!\n",n_type);
			return DBERR_API_NOFOUND;
		}
	}
	
	ret = DB_pre_add_node(v_attr, n_type,parent_node_id,node_len, &ret_id);
	if(ret != DB_SUCCES)
	{
		DB_RELEASE_MUTEX();
		if(ret == DBERR_ID_ASSIGN)
		{	
			DB_PRINTF("add_node(): node type %d, no valid id for it!\n",n_type);
			return DBERR_MAX_LIMIT;
		}
		return ret;
	}
	
	if(n_type == TYPE_SAT_NODE)
	{
		S_NODE *s_node = (S_NODE *)node;
		s_node->sat_id = (UINT16)(ret_id>>(NODE_ID_TP_BIT+NODE_ID_PG_BIT));
	}
	else if(n_type==TYPE_TP_NODE)
	{
		T_NODE *t_node = (T_NODE *)node;
		t_node->tp_id = (DB_TP_ID)(ret_id>>NODE_ID_PG_BIT);
		//in search, add new tp into memory buf
		if(db_search_mode == 1)
			MEMCPY(db_search_tp_buf+v_attr->node_num*node_len, node, node_len);
	}
	else if(n_type==TYPE_PROG_NODE)
	{
		P_NODE *p_node = (P_NODE *)node;
		p_node->prog_id = ret_id;
		*fav_prog_id = ret_id;
		//in search, add new pg into memory buf
		if(db_search_mode == 1)
			MEMCPY(db_search_pg_buf+v_attr->node_num*node_len, node, node_len);
		if (NULL != db_node_cmp_callback)
			db_node_cmp_callback(node);
	}

	ret = DB_add_node(v_attr, ret_id, node,node_len);
	DB_RELEASE_MUTEX();
	return ret;

}


static INT32 api_chunk_data_write(UINT8 *buffer_adr, UINT32 buffer_size,UINT32 chunk_id)
{
	UINT8* malloc_sector_buff;	//read flash data to sdram,to compare if need write
	UINT32 param;	
	INT32 f_block_addr,burn_size;
	struct sto_device *f_dev;
	BOOL need_erase;

	burn_size = (INT32)buffer_size;

	f_block_addr = (INT32)sto_chunk_goto(&chunk_id, 0xFFFFFFFF, 1) + CHUNK_HEADER_SIZE;
	
	if ((f_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0)) == NULL)
	{
		DB_PRINTF("ERROR: Not found Flash device!\n");
		return !SUCCESS;
	}
	if (sto_open(f_dev) != SUCCESS)
	{
		DB_PRINTF("ERROR: sto_open failed!\n");
		return !SUCCESS;
	}
	if ((malloc_sector_buff = (UINT8*)MALLOC(SECTOR_SIZE)) == NULL)
	{
		DB_PRINTF("ERROR: malloc failed!\n");
		return  !SUCCESS;
	}
	
	if((f_block_addr % SECTOR_SIZE) != 0
		|| (buffer_size % SECTOR_SIZE) !=0)
	{
		INT32 head_sector_adr,tail_sector_adr,usbupg_blk_endadr;
		INT32 memcpy_src_adr,memcpy_dst_adr;
		UINT32 memcpy_len;

		head_sector_adr = f_block_addr - f_block_addr % SECTOR_SIZE;//get last sector head adr
		tail_sector_adr = (f_block_addr + buffer_size + SECTOR_SIZE) - (f_block_addr + buffer_size)%SECTOR_SIZE;
		usbupg_blk_endadr = f_block_addr + buffer_size;
		
		while(head_sector_adr < tail_sector_adr)
		{//write a sector,using malloc_sector_buff
			if (sto_lseek(f_dev, head_sector_adr, STO_LSEEK_SET) != head_sector_adr)
			{
				DB_PRINTF("ERROR: sto_lseek failed!\n");
				FREE(malloc_sector_buff);
				return  !SUCCESS;
			}
			if (sto_read(f_dev, malloc_sector_buff, SECTOR_SIZE) != SECTOR_SIZE)
			{
				DB_PRINTF("ERROR: sto_read failed!\n");
				FREE(malloc_sector_buff);
				return  !SUCCESS;
			}

			need_erase = !all_data_is_0xff(malloc_sector_buff, SECTOR_SIZE);
			/*copy upgrade data here*/
			if(head_sector_adr <= f_block_addr)
			{
				memcpy_len = (head_sector_adr + SECTOR_SIZE - f_block_addr);
				if(memcpy_len > buffer_size)
					memcpy_len = buffer_size;

				memcpy_src_adr = (INT32)buffer_adr;
				memcpy_dst_adr = (INT32)(malloc_sector_buff + f_block_addr - head_sector_adr);
				memcpy((void *)memcpy_dst_adr,(void *)memcpy_src_adr,memcpy_len);

				buffer_adr = buffer_adr + memcpy_len;
				buffer_size = buffer_size - memcpy_len;
			}
			else
			{
				if(buffer_size >= SECTOR_SIZE)
					memcpy_len = SECTOR_SIZE;
				else
					memcpy_len = buffer_size;

				memcpy_src_adr = (INT32)buffer_adr;
				memcpy_dst_adr = (INT32)malloc_sector_buff;
				memcpy((void *)memcpy_dst_adr,(void *)memcpy_src_adr,memcpy_len);

				buffer_adr = buffer_adr + memcpy_len;
				buffer_size = buffer_size - memcpy_len;
			}

			if (need_erase)
			{
				/* Uper 22 bits of MSB is start offset based on SYS_FLASH_BASE_ADDR */
				param = head_sector_adr;
				param = param << 10;	
				/* Lower 10 bits of LSB is length in K bytes*/
				param += SECTOR_SIZE >> 10;
				DB_PRINTF("sto_io_control ereser sector param%x\n",param);
				if(sto_io_control(f_dev, STO_DRIVER_SECTOR_ERASE, param) != SUCCESS)
				{
					DB_PRINTF("ERROR: erase flash memory failed!\n");
					FREE(malloc_sector_buff);
					return  !SUCCESS;
				}
			}
			if (!all_data_is_0xff(malloc_sector_buff, SECTOR_SIZE))
			{
				if (sto_lseek(f_dev,head_sector_adr,STO_LSEEK_SET) != head_sector_adr)
				{
					DB_PRINTF("ERROR: sto_lseek failed!\n");
					FREE(malloc_sector_buff);
					return  !SUCCESS;
				}	
				if(sto_write(f_dev,(UINT8 *)malloc_sector_buff, SECTOR_SIZE)!=SECTOR_SIZE)
				{
					DB_PRINTF("ERROR: sto_write failed!\n");
					FREE(malloc_sector_buff);
					return  !SUCCESS;
				}
			}
			head_sector_adr += SECTOR_SIZE;

			if(buffer_size == 0)
				break;
		}
	}//for block upgrade
	else
	{
		if (sto_lseek(f_dev, f_block_addr, STO_LSEEK_SET) != f_block_addr)
		{
			DB_PRINTF("ERROR: sto_lseek failed!\n");
			FREE(malloc_sector_buff);
			return !SUCCESS;
		}
		if (sto_read(f_dev, malloc_sector_buff, SECTOR_SIZE) != SECTOR_SIZE)
		{
			DB_PRINTF("ERROR: sto_read failed!\n");
			FREE(malloc_sector_buff);
			return !SUCCESS;
		}
		
		while(buffer_size)
		{
			sto_get_data(f_dev, malloc_sector_buff, f_block_addr, SECTOR_SIZE);
			if(mem_cmp(malloc_sector_buff, buffer_adr, SECTOR_SIZE)==FALSE)
			{
				need_erase = !all_data_is_0xff(malloc_sector_buff, SECTOR_SIZE);
				if (need_erase)
				{
					/* Uper 22 bits of MSB is start offset based on SYS_FLASH_BASE_ADDR */
					param = f_block_addr;
					param = param << 10;	
					/* Lower 10 bits of LSB is length in K bytes*/
					param += SECTOR_SIZE >> 10;
					if(sto_io_control(f_dev, STO_DRIVER_SECTOR_ERASE, param) != SUCCESS)
					{
						DB_PRINTF("ERROR: erase flash memory failed!\n");
						FREE(malloc_sector_buff);
						return  !SUCCESS;
					}
				}
				if (!all_data_is_0xff(buffer_adr, SECTOR_SIZE))
				{
					if (sto_lseek(f_dev,(INT32)f_block_addr,STO_LSEEK_SET) != (INT32)f_block_addr)
					{
						DB_PRINTF("ERROR: sto_lseek failed!\n");
						FREE(malloc_sector_buff);
						return  !SUCCESS;
					}	
					if(sto_write(f_dev, buffer_adr, SECTOR_SIZE)!=SECTOR_SIZE)
					{
						DB_PRINTF("ERROR: sto_write failed!\n");
						FREE(malloc_sector_buff);
						return  !SUCCESS;
					}
				}
			}
			
			f_block_addr +=SECTOR_SIZE;
			buffer_adr +=SECTOR_SIZE;
			buffer_size -= SECTOR_SIZE;
	
		}
	}

	FREE(malloc_sector_buff);
	return SUCCESS;
}
static INT32 api_chunk_data_read(UINT8* read_buffer, UINT32 read_size,UINT32 chunk_id )
{
	INT32 ret;
	UINT32 data_backup_addr;
	//UINT32 chunk_id = DATA_BACKUP_CHUNK_ID;
	CHUNK_HEADER chuck_hdr;
	struct sto_device *sto_flash_dev;

	DB_PRINTF("\n[Save_reload.c_L%d]--api_chunk_data_read...\n", __LINE__);
	if((read_buffer == NULL) || (read_size <= 0))
	{
		DB_PRINTF("MEM_Read: invalid parameter\n");
		return !SUCCESS;
	}

	sto_flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
	if(sto_flash_dev == NULL)
	{
		DB_PRINTF("MEM_Write: dev_get_by_name failed!\n");
		return !SUCCESS;
	}
	if (sto_open(sto_flash_dev) != SUCCESS)
	{
		DB_PRINTF("MEM_Write: sto_open failed!\n");
		return !SUCCESS;
	}

	/* get table info base addr by using chunk */
	if(sto_get_chunk_header(chunk_id, &chuck_hdr) == 0)
	{
		DB_PRINTF("MEM_Read: get keytable base addr failed!\n");
		return !SUCCESS;
	}
	data_backup_addr = (UINT32) sto_chunk_goto(&chunk_id, 0xFFFFFFFF, 1) + CHUNK_HEADER_SIZE;

	ret = sto_lseek(sto_flash_dev, data_backup_addr, STO_LSEEK_SET);
	if ((UINT32) ret != data_backup_addr)
	{

		DB_PRINTF("MEM_Write: seek base failed!\n");
		return !SUCCESS;
	}

	ret = sto_read(sto_flash_dev, read_buffer, read_size);
	if((UINT32) ret != read_size)
	{
		return !SUCCESS;
	}

	return SUCCESS;
}

#ifdef RAPS_SUPPORT

#define RAPS_UPDATE_CHUNK_ID	0x08F70101

INT32 api_raps_update_write(UINT8 *write_buffer, UINT32 write_size)
{
	return api_chunk_data_write(write_buffer,write_size,RAPS_UPDATE_CHUNK_ID);
}
INT32 api_raps_update_read(UINT8* read_buffer, UINT32 read_size)
{
	return api_chunk_data_read(read_buffer,read_size,RAPS_UPDATE_CHUNK_ID);
}
#endif

#ifdef FAV_LIST_EDIT_NOT_CHANGE_CHANNEL_LIST
#define FAV_UPDATE_CHUNK_ID	0x08F70102
INT32 api_fav_update_write(UINT8 *write_buffer, UINT32 write_size)
{
	return api_chunk_data_write(write_buffer,write_size,FAV_UPDATE_CHUNK_ID);
}
INT32 api_fav_update_read(UINT8* read_buffer, UINT32 read_size)
{
	return api_chunk_data_read(read_buffer,read_size,FAV_UPDATE_CHUNK_ID);
}
#endif


#endif


