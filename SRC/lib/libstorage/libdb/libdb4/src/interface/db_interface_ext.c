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
#include <api/libsi/si_config.h>
#include <api/libtsi/db_3l.h>
#include <api/libdb/db_node_api.h>

#define DB_EXT_PRINTF	PRINTF


extern OSAL_ID db_access_sema;
extern DB_VIEW db_view;
extern DB_TABLE db_table[DB_TABLE_NUM];


#ifndef LCN_VAR_RANGE
#define LCN_VAR_RANGE 2000
#endif

#if defined(DB_USE_UNICODE_STRING)
INT32 DB_STRCMP(UINT16 *s, UINT16 *t)
{
	while (*s && *s == *t) {
		s++, t++;    
	}
	return (*s - *t);
}

UINT16 *DB_STRCPY(UINT16 *dest, UINT16 *src)
{
	UINT16 *ptr = dest;
	while ((*dest++ = *src++))
		;
	return ptr;
}

INT32 DB_STRLEN(UINT16 *s)
{
	INT32 i;
	for (i=0; *s; ++s) 
		++i;  
	return i;
}
#endif

#if !defined(DB_USE_UNICODE_STRING)
INT32 node_name_cmp(char *dest, char *src)
{
	unsigned int i,j;
	char d_tmp,s_tmp;
	if(dest[0] != '?')/* ABC */
	{
		for (i = 0; dest[i] != '\0' && src[i] != '\0'; i++)
		{
			d_tmp = dest[i];
			s_tmp = src[i];
			if(s_tmp > 96 && s_tmp < 123)
				s_tmp -= 32;
			if (d_tmp != s_tmp)
			{
				return -2;
			}
		}

		if (dest[i] == '\0' && src[i] == '\0')
		{
			return 0;
		} else if (dest[i] == '\0')
		{
			return -1;
		} else
		{
			return 1;
		}
	}
	else/* ?ABC */
	{
		if(STRLEN(dest) == 1)
			return -2;
		dest += 1;
		while(*src != '\0')
		{
			if(STRLEN(src) < STRLEN(dest))
				return -2;
			i = 0,j = 0;
			while(dest[i] != '\0')
			{
				d_tmp = dest[i];
				s_tmp = src[i];
				if(s_tmp > 96 && s_tmp < 123)
					s_tmp -= 32;
				if (d_tmp != s_tmp)
				{
					j = 1;
					break;
				}
				else
					i++;
			}
			if(j == 0)
				return 0;
			src++;
		}
		return -2;
	}
}
#else
INT32 node_name_cmp(UINT16 *dest, UINT16 *src)
{
	unsigned int i,j;
	UINT16 d_tmp,s_tmp;
	if(dest[0] != ((UINT16)'?')<<8)/* ABC */
	{
		for (i = 0; dest[i] != (UINT16)'\0' && src[i] != (UINT16)'\0'; i++)
		{
			d_tmp = dest[i];
			s_tmp = src[i];
			if(((s_tmp&0xFF)==0x00)&&(s_tmp>(96<<8))&&(s_tmp<(123<<8)))
				s_tmp -= (32<<8);
			if(((d_tmp&0xFF)==0x00)&&(d_tmp>(96<<8))&&(d_tmp<(123<<8)))
				d_tmp -= (32<<8);

			if (d_tmp != s_tmp)
			{
				return -2;
			}
		}

		if (dest[i] == (UINT16)'\0' && src[i] == (UINT16)'\0')
		{
			return 0;
		} else if (dest[i] == (UINT16)'\0')
		{
			return -1;
		} else
		{
			return 1;
		}
	}
	else/* ?ABC */
	{
		if (dest[1] == (UINT16)'\0')
			return -2;
		else
			dest += 1;
		while(*src != (UINT16)'\0')
		{
			i = 0,j = 0;
			while(dest[i] != (UINT16)'\0')
			{
				if(*src == (UINT16)'\0')
					return 1;
				
				d_tmp = dest[i];
				s_tmp = src[i];
				if(((s_tmp&0xFF)==0x00)&&(s_tmp>(96<<8))&&(s_tmp<(123<<8)))
					s_tmp -= (32<<8);
				if(((d_tmp&0xFF)==0x00)&&(d_tmp>(96<<8))&&(d_tmp<(123<<8)))
					d_tmp -= (32<<8);				
				if (d_tmp != s_tmp)
				{
					j = 1;
					break;
				}
				else
					i++;
			}
			if(j == 0)
				return 0;
			src++;
		}
		return -2;
	}
}
#endif

//static UINT16 find_node_by_name(DB_VIEW *view, UINT8 n_type, name_compare_t name_cmp,UINT8 *name, UINT8 *node, UINT32 node_len)
static UINT16 find_node_by_name(DB_VIEW *view, UINT8 n_type, UINT8 *name, UINT8 *node, UINT32 node_len)
{
	UINT16 i;
	INT32 ret;
	S_NODE *s_node;
	P_NODE *p_node;
	DB_ETYPE *node_name= NULL;
	UINT16 f_cnt =0;
	UINT32 node_id = 0, node_addr = 0;
			
	for(i = 0;i < view->node_num; i++)
	{
		view->node_flag[i] &= ~V_NODE_FIND_FLAG;
		DB_get_node_by_pos(view,i, &node_id, &node_addr);
		ret = DB_read_node(node_id,node_addr,node, node_len);
		if(ret != DB_SUCCES)
			return 0xFFFF;
		/*
		if(n_type == TYPE_SAT_NODE)
		{
			s_node = (S_NODE *)node;
			node_name = (DB_ETYPE *)s_node->sat_name;
		}
		else if(n_type == TYPE_PROG_NODE)*/
		{
			p_node = (P_NODE *)node;
			node_name = (DB_ETYPE *)p_node->service_name;
		}
			
		ret = node_name_cmp((DB_ETYPE *)name,node_name);
		if(ret == -1 || ret == 0)
		{
			f_cnt++;
			view->node_flag[i] |= V_NODE_FIND_FLAG;
		}
	}
	
	return f_cnt;
}


static INT32 get_find_node_by_pos( DB_VIEW *view, UINT16 f_pos, UINT8*node, UINT32 node_len)
{
	UINT16 i;
	UINT16 f_cnt = 0;
	INT32 ret = DB_SUCCES;
	UINT32 node_id = 0, node_addr = 0;

	for(i = 0;i < view->node_num; i++)
	{
		if((view->node_flag[i]&V_NODE_FIND_FLAG) == V_NODE_FIND_FLAG)
		{
			if(f_cnt == f_pos)
			{
				DB_get_node_by_pos(view,i, &node_id, &node_addr);
				ret = DB_read_node(node_id,node_addr,node, node_len);
				return ret;
			}
			f_cnt++;
		}
	}
	DB_EXT_PRINTF("get_find_node():node at f_pos %d not find!\n",f_pos);
	return DBERR_API_NOFOUND;
}


///UINT16 find_prog_by_name(name_compare_t name_cmp,UINT8 *name, P_NODE *node)
UINT16 find_prog_by_name(UINT8 *name, P_NODE *node)
{
	UINT16 num = 0;
	DB_VIEW *view = (DB_VIEW *)&db_view;

	DB_ENTER_MUTEX();
	if(TYPE_PROG_NODE != DB_get_cur_view_type(view))
	{
		DB_RELEASE_MUTEX();
		DB_EXT_PRINTF(" find_prog_by_name(): cur_view_type not prog type %d !\n");
		return 0xFFFF;
	}
	num = find_node_by_name(view, TYPE_PROG_NODE, name, (UINT8 *)node, sizeof(P_NODE));
	if(num > 0 && num != 0xFFFF)
		get_find_node_by_pos(view, 0, (UINT8 *)node, sizeof(P_NODE));

	DB_RELEASE_MUTEX();
	return num;
}

static UINT16 get_node_by_event(DB_VIEW *view, UINT8 n_type, UINT32 tp_id, 
							UINT16 service_id, UINT8 *node, UINT32 node_len)
{
	UINT16 i;
	INT32 ret;
	P_NODE *p_node;
	UINT16 f_cnt =0;
	UINT32 node_id = 0, node_addr = 0;
			
	for(i = 0;i < view->node_num; i++)
	{
		view->node_flag[i] &= ~V_NODE_FIND_FLAG;
		DB_get_node_by_pos(view,i, &node_id, &node_addr);
		ret = DB_read_node(node_id,node_addr,node, node_len);
		if(ret != DB_SUCCES)
			return 0xFFFF;
		{
			p_node = (P_NODE *)node;
		}
			
		if(p_node->tp_id == tp_id && p_node->prog_number == service_id)
		{
			f_cnt++;
			view->node_flag[i] |= V_NODE_FIND_FLAG;
		}
	}
	
	return f_cnt;
}


UINT16 get_prog_by_event(UINT32 tp_id, UINT16 service_id, P_NODE *node)
{
	UINT16 num = 0;
	DB_VIEW *view = (DB_VIEW *)&db_view;

	DB_ENTER_MUTEX();
	if(TYPE_PROG_NODE != DB_get_cur_view_type(view))
	{
		DB_RELEASE_MUTEX();
		DB_EXT_PRINTF(" find_prog_by_name(): cur_view_type not prog type %d !\n");
		return 0xFFFF;
	}
	num = get_node_by_event(view, TYPE_PROG_NODE, tp_id, service_id, (UINT8 *)node, sizeof(P_NODE));
	if(1 == num)
	{
		get_find_node_by_pos(view, 0, (UINT8 *)node, sizeof(P_NODE));
	}
	else
	{
		DB_EXT_PRINTF(" get_prog_by_event(): p_node more than one %d !\n");
	}

	DB_RELEASE_MUTEX();
	return num;
}

INT32 get_find_prog_by_pos(UINT16 f_pos, P_NODE *node)
{
	INT32 ret = DB_SUCCES;
	DB_VIEW *view = (DB_VIEW *)&db_view;

	DB_ENTER_MUTEX();
	if(TYPE_PROG_NODE != DB_get_cur_view_type(view))
	{
		DB_RELEASE_MUTEX();
		DB_EXT_PRINTF(" get_find_prog_by_pos(): cur_view_type not prog type %d !\n");
		return DBERR_API_NOFOUND;
	}
	ret = get_find_node_by_pos(view, f_pos, (UINT8 *)node, sizeof(P_NODE));
	DB_RELEASE_MUTEX();
	return ret;
}


void clear_node_find_flag()
{
	UINT16 i;
	DB_VIEW *view = (DB_VIEW *)&db_view;
	
	DB_ENTER_MUTEX();
	
	for(i = 0;i < view->node_num; i++)
		view->node_flag[i] &=  ~V_NODE_FIND_FLAG;

	DB_RELEASE_MUTEX();
}

/****************************************
Sort node

*****************************************/
static UINT8 *db_sort_t_buf = NULL;
static UINT8 *db_sort_p_buf = NULL;
extern UINT8 * db_update_tmp_buf;

INT32 sort_assign_buf(UINT8 sort_flag)
{
	//UINT32 tp_buf_len = MAX_TP_NUM *sizeof(struct sort_t_info);
	UINT32 pg_buf_len = MAX_PROG_NUM*(sizeof(NODE_IDX_PARAM)+sizeof(KEY_VALUE)+1);
		
	
	if((sort_flag == TP_DEFAULT_SORT) || (sort_flag == TP_DEFAULT_SORT_EXT))
	{
		db_sort_t_buf = db_update_tmp_buf;
	}
	else 
	{
		if(db_sort_p_buf == NULL)
		{
			db_sort_p_buf = (UINT8 *)((UINT32)MALLOC(pg_buf_len+3)&0xFFFFFFFC);
			if(db_sort_p_buf == NULL)
				return DBERR_BUF;
			db_sort_p_buf = (UINT8 *)(((UINT32)db_sort_p_buf)&0xFFFFFFFC);
		}
	}

	if((sort_flag == PROG_DEFAULT_SORT) || (sort_flag == PROG_DEFAULT_SORT_EXT)
		|| (sort_flag == PROG_LCN_SORT )
		||(sort_flag == PROG_DEFAULT_SORT_THEN_FTA)
		||(sort_flag == PROG_TYPE_SID_SORT)||(sort_flag == PROG_TYPE_SID_SORT_EXT)
		)
			db_sort_t_buf = db_update_tmp_buf;
		
		
	return DB_SUCCES;
	

}


/*prog info sort*/
struct strip_area {
	INT16 f;
	INT16 t;
	UINT16 offset;
};

static struct strip_area *sa = NULL;
static INT16 sa_cnt;

static __inline__ void init_strip(struct strip_area *ref, INT16 max_cnt)
{
	sa = ref;
	sa_cnt = 0;
}

static __inline__ void install_strip(INT16 f, INT16 t, UINT16 offset)
{
	if (t-f>=1) 
	{
		sa[sa_cnt].f = f;
		sa[sa_cnt].t = t;
		sa[sa_cnt++].offset = offset;
	}
}

static __inline__ void load_strip(INT16 *f, INT16 *t, UINT16 *offset)
{
	if (sa_cnt > 0) 
	{
		*f = sa[sa_cnt-1].f;
		*t = sa[sa_cnt-1].t;
		*offset = sa[sa_cnt-1].offset;
		sa_cnt--;
	} 
	else 
	{
		*f = *t = *offset =0;
	}
}

static __inline__ UINT32 bswap32(UINT32 x)
{
	return ((x&0xff)<<24)|((x&0xff00)<<8)|((x&0xff0000)>>8)|(x>>24);
}

#if (SYS_CPU_ENDIAN == ENDIAN_LITTLE)
#define cpu_to_be32(x)		bswap32(x)
#elif (SYS_CPU_ENDIAN == ENDIAN_BIG)
#define cpu_to_be32(x)		(x)
#endif

static __inline__ void name_capital_change(KEY_VALUE *key)
{
	if(((*key&0xFF0000)>0x600000)&&((*key&0xFF0000)<0x7B0000))
		*key -= ((32)<<16);
	if(((*key&0xFF)>0x60)&&((*key&0xFF)<0x7B))
		*key -= 32;
}


static void load_prog_array(INT16 f, INT16 t, KEY_VALUE *array, UINT16 offset, UINT8 sort_flag)
{
	INT16 i;
	UINT32 id = 0, addr = 0;
	DB_VIEW *view = (DB_VIEW *)&db_view;
	KEY_VALUE sample;
	KEY_VALUE be_sample;
	
	P_NODE p_node;
	UINT8 tmp;
	UINT16 p_node_offset = NODE_ID_SIZE+1+P_NODE_FIX_LEN;
	UINT16 p_name_len = 0;
	UINT16 read_bytes=0;
	
	for(i=f; i<=t; i++) 
	{
		p_node_offset = NODE_ID_SIZE+1+P_NODE_FIX_LEN;
		sample = 0;
		DB_get_node_by_pos(view, i, &id, &addr);
				
		if ((sort_flag == PROG_NAME_SORT)||(sort_flag == PROG_NAME_SORT_EXT)) 
		{
#if 0
#ifdef DB_CAS_SUPPORT
			//get cas count, 1 byte (tmp= p_node->cas_count)
			BO_read_data(addr+p_node_offset, 1, &tmp);
			p_node_offset += sizeof(UINT8)+tmp*2;	// offset += 1 +  p_node->cas_count*2;
#endif
			//get audio count, 1 byte (tmp= p_node->audio_count)
			BO_read_data(addr+p_node_offset, 1, &tmp);
			p_node_offset += sizeof(UINT8)+tmp*4;	// (p_node->audio_pid)*2 + (p_node->audio_lang)*2
#if (defined(DTG_AUDIO_SELECT) || defined(AUDIO_DESCRIPTION_SUPPORT))
			//audio type
			p_node_offset += tmp;		// p_node->audio_type
#endif
			//get name length, 1 byte
			BO_read_data(addr+p_node_offset, 1, &p_name_len);
			p_node_offset += 1;
			if(offset < p_name_len)
			{
				read_bytes = ((p_name_len-offset) >= (UINT16)sizeof(KEY_VALUE))?(UINT16)sizeof(KEY_VALUE):(p_name_len-offset);
				BO_read_data(addr+p_node_offset+offset, read_bytes, &sample);
				be_sample = cpu_to_be32(sample);
				name_capital_change(&be_sample);
				array[i] = be_sample;
				//libc_printf("prog %d name %d and %d char is '%c' and '%c'\n", i, offset/2 +1, offset/2+2,(UINT8)((array[i])>>16),(UINT8)array[i]);
			}
			/*
			else if((offset > p_name_len-sizeof(KEY_VALUE))&&(offset < p_name_len))
			{
				BO_read_data(addr+p_node_offset+offset, p_name_len-offset, &sample);
				be_sample = cpu_to_be32(sample);
				name_capital_change(&be_sample);
				array[i] = be_sample;
				libc_printf("prog %d name %d and %d char is %s and %s\n", i, offset/2 -1, offset/2,(UINT8)((array[i])>>16),(UINT8)array[i]);
			}
			*/
			else
			{
				array[i] = 0;
				//libc_printf("prog %d name len=%d, less that offset=%d\n", i,p_name_len, offset);
			}
#else
			DB_read_node( id , addr ,(UINT8 *)(&p_node) , sizeof(P_NODE) );
			if(offset < p_node.name_len )
			{
				read_bytes = ((p_node.name_len-offset) >= (UINT16)sizeof(KEY_VALUE))?(UINT16)sizeof(KEY_VALUE):(p_node.name_len-offset);
				MEMCPY( (void*)(&sample) , &p_node.service_name[offset] , read_bytes );
				be_sample = cpu_to_be32(sample);
				name_capital_change(&be_sample);
				array[i] = be_sample;
			}
			else
			{
				array[i] = 0;
			}
#endif
		} 
		else if(sort_flag == PROG_LOGICAL_NUM_SORT)
		{
			UINT8 tmp_buf[P_NODE_FIX_LEN];

		#if (SYS_PROJECT_FE == PROJECT_FE_DVBC)
			BO_read_data(addr+NODE_ID_SIZE+1, P_NODE_FIX_LEN, (UINT8 *)&tmp_buf);
			MEMCPY((UINT8 *)&(p_node.prog_id)+sizeof(UINT32), tmp_buf, P_NODE_FIX_LEN);
			// support for DVB-C logical number sorting.
			array[i] = p_node.logical_channel_num;
		#endif
		}
	}
	
}


static void shellpass(DB_VIEW *view,KEY_VALUE *array, INT16 step, INT16 from, INT16 to)
{
	INT16 i, j;
	KEY_VALUE key = 0;

	NODE_IDX_PARAM view_node;
	UINT8 flag;
	//DB_VIEW *view = (DB_VIEW *)&db_view;
	NODE_IDX_PARAM *view_buf = view->node_buf;
	UINT8 *view_flag = NULL;
	
	if(view->node_flag)
		view_flag = view->node_flag;

	for(i = from+step; i <= to; i++)
	{
		if(array[i] < array[i-step])
		{
			MEMCPY(&view_node, &view_buf[i], sizeof(NODE_IDX_PARAM));
			flag = view_flag[i];
			key = array[i]; j = i-step;
		
			do{
				array[j+step] = array[j];
				MEMCPY(&view_buf[j+step], &view_buf[j], sizeof(NODE_IDX_PARAM));
				view_flag[j+step] = view_flag[j];
				j -= step;

				}while(j>=from && array[j] > key);
			array[j+step] = key;
			MEMCPY(&view_buf[j+step], &view_node, sizeof(NODE_IDX_PARAM));
			view_flag[j+step] = flag;
		}
	}
}

static void shellsort(DB_VIEW *view,KEY_VALUE *array, INT16 from, INT16 to)
{
	INT16 step = to-from+1;

	if(from == to)
		return;
	do{
		if(step / 3 == 1)
			step = 1;
		else
			step = step / 3 + 1;
		shellpass(view,array, step, from, to);

		}while(step > 1);

}

static void shellpass2(struct sort_t_info *ti,KEY_VALUE *array, INT16 step, INT16 from, INT16 to)
{
	INT16 i, j;
	KEY_VALUE key = 0;
	struct sort_t_info tmp_ti;
	
	for(i = from+step; i <= to; i++)
	{
		if(array[i] < array[i-step])
		{
			MEMCPY(&tmp_ti, &ti[i], sizeof(struct sort_t_info));
			key = array[i]; j = i-step;
		
			do{
				array[j+step] = array[j];
				MEMCPY(&ti[j+step], &ti[j], sizeof(struct sort_t_info));
				j -= step;

				}while(j>=from && array[j] > key);
			array[j+step] = key;
			MEMCPY(&ti[j+step], &tmp_ti, sizeof(struct sort_t_info));
		}
	}
}

static void shellsort2(struct sort_t_info *ti,KEY_VALUE *array, INT16 from, INT16 to)
{
	INT16 step = to-from+1;

	if(from == to)
		return;
	do{
		if(step / 3 == 1)
			step = 1;
		else
			step = step / 3 + 1;
		shellpass2(ti ,array, step, from, to);

		}while(step > 1);

}


static INT32 prog_info_sort(UINT8 sort_flag)
{
	INT16 from, to;
	INT16 i,j, k;
	INT16 n;
	NODE_IDX_PARAM tmp_view_node;
	UINT8 tmp_flag;
	INT32 ret = DB_SUCCES;
	DB_VIEW *view = (DB_VIEW *)&db_view;
	
	UINT16 offset = 0;
	KEY_VALUE*array = (KEY_VALUE*)db_sort_p_buf;
	struct strip_area *sa_ptr = (struct strip_area *)(db_sort_p_buf+sizeof(KEY_VALUE)*MAX_PROG_NUM);
	
	DB_EXT_PRINTF("prog_info_sort(): begin sort,tick = %d\n", osal_get_tick());
	
	if (view->node_num <= 1)
		return ret;

	init_strip(sa_ptr, view->node_num);
	install_strip(0, view->node_num - 1, offset);
	do {
		//libc_printf("\n\nstrip count =%d\n",sa_cnt);
		load_strip(&from, &to, &offset);
		//libc_printf("\nstrip[%d],from=%d, to=%d, key offset=%d\n",sa_cnt,from, to,offset);
	
		/* read prog info to key array*/
		load_prog_array(from, to, array, offset, sort_flag);
		/* compare key and sort*/
		shellsort(view,array,from,to);

		//for(i=sa_ptr[sa_cnt].f;i<=sa_ptr[sa_cnt].t;i++)
		//	libc_printf("after sort, %d st prog name char %d and %d is '%c' and '%c'\n", i,offset/2+1,offset/2+2,(UINT8)((array[i])>>16),(UINT8)array[i]);

		offset += sizeof(KEY_VALUE);
		if ((sort_flag==PROG_NAME_SORT_EXT)||(sort_flag == PROG_NAME_SORT)) 
		{
			for(i=j=from; i<=to; i++) 
			{
				//divide strip inside (from, to)
				if (array[i] != array[j])
				{
					if(offset < (MAX_SERVICE_NAME_LENGTH+3))
						install_strip(j, i-1, offset);
					j = i;
				}
			}
			//after (MAX_SERVICE_NAME_LENGTH+3)/2 number char compared, jump this strip
			if(offset < (MAX_SERVICE_NAME_LENGTH+3))
				install_strip(j, to, offset);
		}
	}while (sa_cnt>0);

	if(sort_flag == PROG_NAME_SORT_EXT)
	{
		n = view->node_num -1;
		for(i = 0; i < n-i; i++)
		{
			MEMCPY(&tmp_view_node, &view->node_buf[i], sizeof(NODE_IDX_PARAM));
			tmp_flag = view->node_flag[i];
			MEMCPY(&view->node_buf[i], &view->node_buf[n-i], sizeof(NODE_IDX_PARAM));
			view->node_flag[i] = view->node_flag[n-i];
			MEMCPY(&view->node_buf[n-i], &tmp_view_node, sizeof(NODE_IDX_PARAM));
			view->node_flag[n-i] = tmp_flag;
		}
	}
	
	DB_EXT_PRINTF("prog_info_sort(): finished,tick = %d\n",osal_get_tick());
	db_table[TYPE_PROG_NODE].node_moved = 1;
	return DB_SUCCES;
}


/*for sort by binary key value, such as av_mode, lock etc.*/
static INT32 prog_bina_key_sort(UINT8 sort_flag)
{
	INT16 i, j, k;
	P_NODE p_node;
	UINT8 key_value = 0xFF;
	UINT8 tmp[P_NODE_FIX_LEN];
	DB_VIEW *view = (DB_VIEW *)&db_view;
	UINT32 node_addr = 0;
	INT32 ret = DB_SUCCES;
	NODE_IDX_PARAM *buf = (NODE_IDX_PARAM *)(db_sort_p_buf);
	UINT8 *flag = (db_sort_p_buf + MAX_PROG_NUM*sizeof(NODE_IDX_PARAM));
	INT16 key_change_pos = 0;
	INT16 num = (INT16)view->node_num;

	if(num <= 1)
		return ret;
	for(i = 0, j = 0, k =num-1; i < num; i++)
	{
		MEMCPY(&node_addr, view->node_buf[i].node_addr, NODE_ADDR_SIZE);
		ret = BO_read_data(node_addr+NODE_ID_SIZE+1, P_NODE_FIX_LEN, tmp);
		if(ret != DB_SUCCES)
			return ret;
		MEMCPY((UINT8 *)&(p_node.prog_id)+sizeof(UINT32), tmp, P_NODE_FIX_LEN);
		
		if(sort_flag == PROG_FTA_SORT || sort_flag == PROG_FTA_SORT_EXT)
			key_value = p_node.ca_mode;
		else if(sort_flag == PROG_LOCK_SORT || sort_flag == PROG_LOCK_SORT_EXT)
			key_value = p_node.lock_flag;
		
			if(key_value == 1)
			{
				MEMCPY(&buf[j], &(view->node_buf[i]), sizeof(NODE_IDX_PARAM));
				//if(view->node_flag)
					flag[j] = view->node_flag[i];
				j++;
			}
			else if(key_value == 0)
			{
				MEMCPY(&buf[k], &(view->node_buf[i]), sizeof(NODE_IDX_PARAM));
				//if(view->node_flag)
					flag[k] = view->node_flag[i];
				k--;
			}
		
	}
	key_change_pos = j;
	
	if(sort_flag == PROG_FTA_SORT || sort_flag == PROG_LOCK_SORT)
	{
		for(i = 0; i < key_change_pos; i++)
		{
			MEMCPY(&(view->node_buf[i]), &buf[i],sizeof(NODE_IDX_PARAM));
			//if(view->node_flag)
				view->node_flag[i] = flag[i];
		}
		for(j = num-1; j >= key_change_pos; i++, j--)
		{
			MEMCPY(&(view->node_buf[i]), &buf[j],sizeof(NODE_IDX_PARAM));
			//if(view->node_flag)
				view->node_flag[i] = flag[j];
		}
	}
	else if(sort_flag == PROG_FTA_SORT_EXT || sort_flag == PROG_LOCK_SORT_EXT)
	{
		for(i = 0, j = num-1; j >= key_change_pos; i++, j--)
		{
			MEMCPY(&(view->node_buf[i]), &buf[j],sizeof(NODE_IDX_PARAM));
			//if(view->node_flag)
				view->node_flag[i] = flag[j];
	
		}
		for(j = 0; j < key_change_pos; i++, j++)
		{
			MEMCPY(&(view->node_buf[i]), &buf[j],sizeof(NODE_IDX_PARAM));
			//if(view->node_flag)
				view->node_flag[i] = flag[j];
		}
	}

	db_table[TYPE_PROG_NODE].node_moved = 1;
	return DB_SUCCES;
}

/*for default sort, tp on same sat by frq order*/
static INT16 lookup_ti_id(struct sort_t_info *ti, DB_TP_ID id, INT16 n)
{
	INT16 low = -1;
	INT16 high = n;
	INT16 mid;
	do {
		mid = (low+high)>>1;
		if (ti[mid].id<id) 
			low = mid;
		else if (ti[mid].id>id) 
			high = mid;
		else 
			return mid;
	}while(high-low>1);
	return -1;
}


static INT32 sort_default_sort(UINT8 node_type, INT8 sort_order)
{

	UINT16 i, j;
	INT16 t_idx, k;
	struct sort_t_info *ti = (struct sort_t_info *)db_sort_t_buf;
	struct sort_p_info *pi = (struct sort_p_info *)db_sort_p_buf;
	KEY_VALUE *key_array = (KEY_VALUE *)(db_sort_p_buf+MAX_PROG_NUM*sizeof(struct sort_p_info));
	NODE_IDX_PARAM *sort = (NODE_IDX_PARAM *)(db_sort_p_buf+MAX_PROG_NUM*sizeof(struct sort_p_info));
	UINT8 *flg = (UINT8 *)(db_sort_p_buf+MAX_PROG_NUM*(sizeof(struct sort_p_info)+sizeof(NODE_IDX_PARAM)));

	DB_VIEW *view = (DB_VIEW *)&db_view;
	DB_TABLE *table = (DB_TABLE *)&db_table[TYPE_TP_NODE];
	T_NODE t_node;
	UINT32 value;
	UINT16 next;
	NODE_IDX_PARAM node_idx;
	UINT8 flag;
	UINT16 n = 0;
	UINT32 id = 0, addr = 0;
	//INT32 ret =DB_SUCCES;

	DB_EXT_PRINTF("sort_default_sort():begin, tick = %d\n",osal_get_tick());

	/* retrieve current tp view, store them into p_info array. */
	if(node_type == TYPE_TP_NODE)
		n = view->node_num;
	else if(node_type == TYPE_PROG_NODE)
		n = table->node_num;
	for(i = 0; i < n; i++)
	{
		if(node_type == TYPE_TP_NODE)
			DB_get_node_by_pos(view, i, &id, &addr);
		else if(node_type == TYPE_PROG_NODE)
			DB_get_node_by_pos_from_table(table, i, &id, &addr);

		DB_read_node(id, addr, (UINT8 *)&t_node, sizeof(T_NODE));
		ti[i].id = t_node.tp_id;
		key_array[i] = ti[i].id;
		ti[i].value = (((UINT32)t_node.sat_id)<<16)|t_node.frq;
		ti[i].next_pos = INVALID_POS_NUM;
		//libc_printf("ti[%d], id=0x%x, value=0x%x,next_pos=%d\n",i,ti[i].id,ti[i].value,ti[i].next_pos);
	}

	if (node_type == TYPE_PROG_NODE) 
	{
		//sort tp by id in ascend order
		shellsort2(ti, key_array, 0, n-1);

		//libc_printf("\ntp after id sort:\n");
		//for(i = 0; i<n; i++)
		//	libc_printf("ti[%d], id=0x%x, value=0x%x,next_pos=%d\n",i,ti[i].id,ti[i].value,ti[i].next_pos);
		
		// retrieve prog view , read the tp_id field and add it into the p_info->next single linked list.
		//here need start from the last program to link the programs under one tp 
		for(i=0,k=(INT16)view->node_num-1; k>=0; k--,i++) 
		{
			MEMCPY(&id, view->node_buf[k].node_id, NODE_ID_SIZE);
			id = (id&NODE_ID_TP_MASK)>>NODE_ID_PG_BIT;
			t_idx = lookup_ti_id(ti, id, n);
			if(t_idx == -1)
				return DBERR_API_NOFOUND;
			pi[i].node_pos = k;
			pi[i].next_pos = ti[t_idx].next_pos;
			
			//libc_printf("pi[%d], prog_pos=%d, next_pos=%d,tp t_idx=%d\n",i,pi[i].node_pos,pi[i].next_pos,t_idx);
			ti[t_idx].next_pos = k;
		}

		/* sort the t_info array by value in ascend way. */
		for(i=0; i< n ; i++)
			key_array[i] = ti[i].value;
		shellsort2(ti, key_array, 0, n-1);
		//libc_printf("\n tp after value sort:\n");
		//for(i = 0; i<n; i++)
		//	libc_printf("ti[%d], value=0x%x, id=0x%x, next_pos=%d\n",i,ti[i].value, ti[i].id, ti[i].next_pos);
	
		UINT16 max_idx = view->node_num-1;
		/* generating the sorted program list */
		for(t_idx=i=0; i<n; i++)
		{
			for(j=ti[i].next_pos; j!=INVALID_POS_NUM; j = pi[max_idx-j].next_pos) 
			{
				if (pi[max_idx-j].node_pos != INVALID_POS_NUM) 
				{
					MEMCPY(&sort[t_idx], &view->node_buf[pi[max_idx-j].node_pos], sizeof(NODE_IDX_PARAM));
					//if(view->node_flag)
						flg[t_idx] = view->node_flag[pi[max_idx-j].node_pos];
					
					t_idx++;
				}
			}
		}

		MEMCPY(view->node_buf, sort, sizeof(NODE_IDX_PARAM)*t_idx);
		//if(view->node_flag)
		MEMCPY(view->node_flag, flg, sizeof(UINT8)*t_idx);
	} 
	else if (node_type == TYPE_TP_NODE) 
	{
		for(i=0; i< n ; i++)
			key_array[i] = ti[i].value;
		shellsort(view, key_array, 0, n-1);
	} 
	//if need sort descend order
	if(sort_order == 1)
	{
		n -= 1;
		for(i = 0; i < n-i; i++)
		{
			MEMCPY(&node_idx,&view->node_buf[i],sizeof(NODE_IDX_PARAM));
			flag = view->node_flag[i];
			MEMCPY(&view->node_buf[i],&view->node_buf[n-i],sizeof(NODE_IDX_PARAM));
			view->node_flag[i] = view->node_flag[n-i];
			MEMCPY(&view->node_buf[n-i],&node_idx,sizeof(NODE_IDX_PARAM));
			view->node_flag[n-i] = flag;
		}		
	}
	

	DB_EXT_PRINTF("sort_default_sort():finished, tick = %d\n", osal_get_tick());
	
	db_table[node_type].node_moved = 1;
	return DB_SUCCES;
}


static INT32 prog_advance_sort(UINT8 sort_flag, UINT32 sort_param)
{
	INT32 ret = DB_SUCCES;
	INT16 i, j,k=0;
	P_NODE p_node;
	
	UINT8 tmp[DB_NODE_MAX_SIZE];
	DB_VIEW *view = (DB_VIEW *)&db_view;
	UINT32 node_addr = 0;
	NODE_IDX_PARAM *buf = (NODE_IDX_PARAM *)(db_sort_p_buf);
	UINT8 *flag = (db_sort_p_buf + MAX_PROG_NUM*sizeof(NODE_IDX_PARAM));
	UINT8 cas_cnt = 0;
	INT16 match_num = 0;

	if(view->node_num <= 1)
		return ret;
	
	if(sort_flag == PROG_CAS_SORT)
	{
		UINT16 sort_casys_id = ((UINT16)sort_param&0xFF00);
		UINT16 *prog_casys_id = (UINT16 *)tmp;
		
		for(i = 0; i < view->node_num; i++)
		{
			MEMCPY(&node_addr, view->node_buf[i].node_addr, NODE_ADDR_SIZE);
			//ca count
			BO_read_data(node_addr+NODE_ID_SIZE+1+P_NODE_FIX_LEN, 1, &cas_cnt);
			if(cas_cnt > 0)
			{
				//ca system id
				ret = BO_read_data(node_addr+NODE_ID_SIZE+1+P_NODE_FIX_LEN+1, cas_cnt*2, tmp);
				for(j = 0;j < cas_cnt; j++)
				{
					//0xFF00 means all cas program
					if((sort_casys_id==0xFF00) ||((prog_casys_id[j]&0xFF00)==sort_casys_id))
					{
						MEMCPY(&buf[match_num], &(view->node_buf[i]), sizeof(NODE_IDX_PARAM));
						flag[match_num] = view->node_flag[i];
						match_num++;
						break;
					}
				}
			}
		}
		//after get all matched cas prog
		if(match_num > 0)
		{
			MEMCPY(view->node_buf, buf, match_num*sizeof(NODE_IDX_PARAM));
			MEMCPY(view->node_flag, flag, match_num);
		}
		view->pre_filter = view->cur_filter;
		view->pre_filter_mode = view->cur_filter_mode;
		view->cur_filter_mode = VIEW_CAS | (view->cur_filter_mode&0xFF);
		view->pre_param = view->view_param;
		view->view_param = sort_casys_id;
		view->node_num = match_num;		
				
	}
	else if(sort_flag == PROG_PROVIDER_SORT)
	{
		if((DB_ETYPE *)sort_param==NULL)
			return DBERR_PARAM;
		DB_ETYPE *sort_name = (DB_ETYPE *)sort_param;
		UINT8 sort_name_len = (UINT8)DB_STRLEN(sort_name)*sizeof(DB_ETYPE);
		UINT8 packed_len;
		UINT8 offset = 0;
		
		for(i = 0; i < view->node_num; i++)
		{
			MEMSET(&p_node, 0, sizeof(P_NODE));
			MEMCPY(&node_addr, view->node_buf[i].node_addr, NODE_ADDR_SIZE);
			ret = BO_read_data(node_addr+NODE_ID_SIZE, 1, tmp);
			if(ret != DB_SUCCES)
				return ret;
			packed_len = tmp[0];
			ret = BO_read_data(node_addr+NODE_ID_SIZE+1+P_NODE_FIX_LEN, packed_len-1-P_NODE_FIX_LEN, tmp);
			if(ret != DB_SUCCES)
				return ret;
#ifdef DB_CAS_SUPPORT
			cas_cnt = tmp[0];
			//cas system id
			  offset = 1+cas_cnt*2;
#endif
			//audio count,pid,lang
			p_node.audio_count = tmp[offset];
			offset += (1+p_node.audio_count *4);
			//service name
			p_node.name_len = tmp[offset];
			offset += 1;
			//MEMCPY(p_node.service_name, tmp+offset, p_node.name_len);
			offset += (p_node.name_len);

			//mtl service name
			#if(defined(SUPPORT_MTILINGUAL_SERVICE_NAME))
			for(k=0;k<p_node.mtl_name_count;k++)
			{
				p_node.mtl_name_len[k] = tmp[offset];			
				offset += 1;
				//MEMCPY(p_node.service_name, tmp+offset, p_node.name_len);
				offset += (p_node.mtl_name_len[k]);
			}
			#endif		
#if (SERVICE_PROVIDER_NAME_OPTION>0)
			//provider name
			p_node.provider_name_len = tmp[offset];
			offset += 1;
			MEMCPY(p_node.service_provider_name, tmp+offset, p_node.provider_name_len);
#endif

			if((sort_name_len==p_node.provider_name_len)&&(DB_STRCMP((DB_ETYPE *)p_node.service_provider_name,sort_name)==0))		
			{
				MEMCPY(&buf[match_num], &(view->node_buf[i]), sizeof(NODE_IDX_PARAM));
				flag[match_num] = view->node_flag[i];
				match_num++;
			}
		}
		
		//after get all matched prog
		if(match_num > 0)
		{
			MEMCPY(view->node_buf, buf, match_num*sizeof(NODE_IDX_PARAM));
			MEMCPY(view->node_flag, flag, match_num);
		}
		view->pre_filter = view->cur_filter;
		view->pre_filter_mode = view->cur_filter_mode;
		view->cur_filter_mode = VIEW_PROVIDER;
		view->pre_param = view->view_param;
		view->view_param = sort_param;
		view->node_num = match_num;		
	}

	return ret;
}

#if(defined( _MHEG5_ENABLE_) || defined( _MHEG5_V20_ENABLE_) || defined( _LCN_ENABLE_))
INT16 check_LCN_index(UINT32 LCN)
{
	DB_VIEW *view = (DB_VIEW *)&db_view;
	DB_TABLE *table = (DB_TABLE *)&db_table[TYPE_PROG_NODE];
	P_NODE p_node;
	UINT16 n = 0;
	UINT16 i;
	UINT32 id = 0, addr = 0;
	INT16 ch_idx = -1;

	DB_ENTER_MUTEX();

	n = view->node_num;

	for(i= 0; i < n; i++)
	{
		DB_get_node_by_pos(view, i, &id, &addr);

		DB_read_node(id, addr, (UINT8 *)&p_node, sizeof(p_node));

#ifdef PORTING_ATSC
		if(p_node.LCN == LCN || ((LCN&0xFF)==0 && LCN == (p_node.LCN & 0xFF00)))
#else
		if(p_node.LCN == LCN)
#endif
		{
#ifdef _SERVICE_ATTRIBUTE_ENABLE_		
			if(p_node.num_sel_flag | p_node.visible_flag)
#endif
			ch_idx = i;
			break;
		}
	}

	DB_RELEASE_MUTEX();

	return ch_idx;

}

INT32 find_dtg_variant_region_LCN()
{
	DB_VIEW *view = (DB_VIEW *)&db_view;
	DB_TABLE *table = (DB_TABLE *)&db_table[TYPE_PROG_NODE];
	P_NODE p_node;
	UINT16 n = 0;
	UINT16 i, j, k;
	UINT32 id = 0, addr = 0;
	 UINT16 tmp_lcn = 900;
	 UINT16 max_tmp_lcn = 899;//for DTG LCN variant range 800~899 

	DB_ENTER_MUTEX();

	n = table->node_num;

	for(i=j= 0; i < n; i++)
	{
		DB_get_node_by_pos_from_table(table, i, &id, &addr);

		DB_read_node(id, addr, (UINT8 *)&p_node, sizeof(p_node));
		//if(p_node.LCN_true == FALSE)
		if (p_node.LCN>=800 && p_node.LCN<=899)
			tmp_lcn = p_node.LCN;
	}

	n = view->node_num;

	for(i=k= 0; i < n; i++)
	{
		DB_get_node_by_pos(view, i, &id, &addr);

		DB_read_node(id, addr, (UINT8 *)&p_node, sizeof(p_node));
		if (p_node.LCN>=800 && p_node.LCN<=899)
			tmp_lcn = p_node.LCN;

	}	

	DB_RELEASE_MUTEX();

	return tmp_lcn-1;

}
INT32 find_max_LCN_next()
{
	DB_VIEW *view = (DB_VIEW *)&db_view;
	DB_TABLE *table = (DB_TABLE *)&db_table[TYPE_PROG_NODE];
	P_NODE p_node;
	UINT16 n = 0;
	UINT16 i, j, k;
	UINT32 id = 0, addr = 0;
	UINT16 tmp_lcn = LCN_VAR_RANGE;
	UINT16 max_tmp_lcn = LCN_VAR_RANGE-1;//for DTG LCN variant range 800~899 

	DB_ENTER_MUTEX();

	n = table->node_num;

	for(i=j= 0; i < n; i++)
	{
		DB_get_node_by_pos_from_table(table, i, &id, &addr);

		DB_read_node(id, addr, (UINT8 *)&p_node, sizeof(p_node));
		if(p_node.LCN_true == FALSE)
		{
			if(p_node.LCN < tmp_lcn)
				tmp_lcn = p_node.LCN;
			//j++;
		}
	}

	n = view->node_num;

	for(i=k= 0; i < n; i++)
	{
		DB_get_node_by_pos(view, i, &id, &addr);

		DB_read_node(id, addr, (UINT8 *)&p_node, sizeof(p_node));
		if(p_node.LCN_true == FALSE)
		{
			if(p_node.LCN < tmp_lcn)
				tmp_lcn = p_node.LCN;
			//k++;
		}
	}

	//tmp_lcn = max_tmp_lcn-j-k;

	DB_RELEASE_MUTEX();

	return tmp_lcn;

}

INT32 find_max_LCN()
{
	DB_VIEW *view = (DB_VIEW *)&db_view;
	DB_TABLE *table = (DB_TABLE *)&db_table[TYPE_PROG_NODE];
	P_NODE p_node;
	UINT16 n = 0;
	UINT16 i;
	UINT32 id = 0, addr = 0;
	UINT16 tmp_lcn = 0;

	DB_ENTER_MUTEX();

	n = table->node_num;

	for(i = 0; i < n; i++)
	{
		DB_get_node_by_pos_from_table(table, i, &id, &addr);

		DB_read_node(id, addr, (UINT8 *)&p_node, sizeof(p_node));
		if(p_node.LCN > tmp_lcn)			
			tmp_lcn  = p_node.LCN;
	}

	n = view->node_num;

	for(i = 0; i < n; i++)
	{
		DB_get_node_by_pos(view, i, &id, &addr);

		DB_read_node(id, addr, (UINT8 *)&p_node, sizeof(p_node));
		if(p_node.LCN > tmp_lcn)			
			tmp_lcn  = p_node.LCN;
	}

	DB_RELEASE_MUTEX();

	return tmp_lcn;

}

INT32 find_max_default_index()
{
	DB_VIEW *view = (DB_VIEW *)&db_view;
	DB_TABLE *table = (DB_TABLE *)&db_table[TYPE_PROG_NODE];
	P_NODE p_node;
	UINT16 n = 0;
	UINT16 i;
	UINT32 id = 0, addr = 0;
	UINT16 tmp_default_index = 0;

	DB_ENTER_MUTEX();

	n = table->node_num;

	for(i = 0; i < n; i++)
	{
		DB_get_node_by_pos_from_table(table, i, &id, &addr);

		DB_read_node(id, addr, (UINT8 *)&p_node, sizeof(p_node));
		if(p_node.default_index > tmp_default_index)			
			tmp_default_index  = p_node.default_index;
	}

	n = view->node_num;

	for(i = 0; i < n; i++)
	{
		DB_get_node_by_pos(view, i, &id, &addr);

		DB_read_node(id, addr, (UINT8 *)&p_node, sizeof(p_node));
		if(p_node.default_index > tmp_default_index)			
			tmp_default_index  = p_node.default_index;
	}

	DB_RELEASE_MUTEX();

	return tmp_default_index;

}

static INT32 sort_default_index(INT8 sort_order)
{

	UINT16 i, j;
	INT16 t_idx, k;
	struct sort_t_info *ti = (struct sort_t_info *)db_sort_t_buf;
	struct sort_p_info *pi = (struct sort_p_info *)db_sort_p_buf;
	KEY_VALUE *key_array = (KEY_VALUE *)(db_sort_p_buf+MAX_PROG_NUM*sizeof(struct sort_p_info));
	NODE_IDX_PARAM *sort = (NODE_IDX_PARAM *)(db_sort_p_buf+MAX_PROG_NUM*sizeof(struct sort_p_info));
	UINT8 *flg = (UINT8 *)(db_sort_p_buf+MAX_PROG_NUM*(sizeof(struct sort_p_info)+sizeof(NODE_IDX_PARAM)));

	DB_VIEW *view = (DB_VIEW *)&db_view;
	DB_TABLE *table = (DB_TABLE *)&db_table[TYPE_PROG_NODE];
	P_NODE p_node;
	UINT32 value;
	UINT16 next;
	NODE_IDX_PARAM node_idx;
	UINT8 flag;
	UINT16 n = 0;
	UINT32 id = 0, addr = 0;

	DB_EXT_PRINTF("sort_default_sort():begin, tick = %d\n",osal_get_tick());

	/* retrieve current tp view, store them into p_info array. */
	n = view->node_num;;//table->node_num;
	
	for(i = 0; i < n; i++)
	{
		DB_get_node_by_pos(view, i, &id, &addr);

		DB_read_node(id, addr, (UINT8 *)&p_node, sizeof(p_node));
		ti[i].id = p_node.tp_id;
		key_array[i] = ti[i].id;
		ti[i].value = p_node.default_index;
		ti[i].next_pos = INVALID_POS_NUM;
		//libc_printf("ti[%d], id=0x%x, value=0x%x,next_pos=%d\n",i,ti[i].id,ti[i].value,ti[i].next_pos);
	}

	//sort tp by id in ascend order
	//shellsort2(ti, key_array, 0, n-1);

	//libc_printf("\ntp after id sort:\n");
	//for(i = 0; i<n; i++)
	//	libc_printf("ti[%d], id=0x%x, value=0x%x,next_pos=%d\n",i,ti[i].id,ti[i].value,ti[i].next_pos);
	
	// retrieve prog view , read the tp_id field and add it into the p_info->next single linked list.
	//here need start from the last program to link the programs under one tp 
	for(i=0,k=(INT16)view->node_num-1; k>=0; k--,i++) 
	{
		MEMCPY(&id, view->node_buf[k].node_id, NODE_ID_SIZE);
		//id = (id&NODE_ID_TP_MASK)>>NODE_ID_PG_BIT;
		//t_idx = lookup_ti_id(ti, id, n);
		//if(t_idx == -1)
		//	return DBERR_API_NOFOUND;
		//pi[i].node_pos = k;
		//pi[i].next_pos = ti[t_idx].next_pos;
		
		//libc_printf("pi[%d], prog_pos=%d, next_pos=%d,tp t_idx=%d\n",i,pi[i].node_pos,pi[i].next_pos,t_idx);
		ti[k].next_pos = k;
	}

	/* sort the t_info array by value in ascend way. */
	for(i=0; i< n ; i++)
		key_array[i] = ti[i].value;
	shellsort2(ti, key_array, 0, n-1);
	//libc_printf("\n tp after value sort:\n");
	//for(i = 0; i<n; i++)
	//	libc_printf("ti[%d], value=0x%x, id=0x%x, next_pos=%d\n",i,ti[i].value, ti[i].id, ti[i].next_pos);

	UINT16 max_idx = view->node_num;
	/* generating the sorted program list */
	for(i=0; i<max_idx; i++)
			{
		MEMCPY(&sort[i], &view->node_buf[ti[i].next_pos], sizeof(NODE_IDX_PARAM));
		flg[i] = view->node_flag[ti[i].next_pos];
	}

	MEMCPY(view->node_buf, sort, sizeof(NODE_IDX_PARAM)*max_idx);
	//if(view->node_flag)
	MEMCPY(view->node_flag, flg, sizeof(UINT8)*max_idx);
	 
	//if need sort descend order
	if(sort_order == 1)
	{
		n -= 1;
		for(i = 0; i < n-i; i++)
		{
			MEMCPY(&node_idx,&view->node_buf[i],sizeof(NODE_IDX_PARAM));
			flag = view->node_flag[i];
			MEMCPY(&view->node_buf[i],&view->node_buf[n-i],sizeof(NODE_IDX_PARAM));
			view->node_flag[i] = view->node_flag[n-i];
			MEMCPY(&view->node_buf[n-i],&node_idx,sizeof(NODE_IDX_PARAM));
			view->node_flag[n-i] = flag;
		}		
	}
	
	DB_EXT_PRINTF("sort_default_sort():finished, tick = %d\n", osal_get_tick());
	
	db_table[TYPE_PROG_NODE].node_moved = 1;
	return DB_SUCCES;
}

static INT32 sort_LCN_sort(INT8 sort_order)
{

	UINT16 i, j;
	INT16 t_idx, k;
	struct sort_t_info *ti = (struct sort_t_info *)db_sort_t_buf;
	struct sort_p_info *pi = (struct sort_p_info *)db_sort_p_buf;
	KEY_VALUE *key_array = (KEY_VALUE *)(db_sort_p_buf+MAX_PROG_NUM*sizeof(struct sort_p_info));
	NODE_IDX_PARAM *sort = (NODE_IDX_PARAM *)(db_sort_p_buf+MAX_PROG_NUM*sizeof(struct sort_p_info));
	UINT8 *flg = (UINT8 *)(db_sort_p_buf+MAX_PROG_NUM*(sizeof(struct sort_p_info)+sizeof(NODE_IDX_PARAM)));

	DB_VIEW *view = (DB_VIEW *)&db_view;
	DB_TABLE *table = (DB_TABLE *)&db_table[TYPE_PROG_NODE];
	P_NODE p_node;
	UINT32 value;
	UINT16 next;
	NODE_IDX_PARAM node_idx;
	UINT8 flag;
	UINT16 n = 0;
	UINT32 id = 0, addr = 0;

	DB_EXT_PRINTF("sort_default_sort():begin, tick = %d\n",osal_get_tick());

	/* retrieve current tp view, store them into p_info array. */
	n = view->node_num;;//table->node_num;
	
	for(i = 0; i < n; i++)
	{
		//DB_get_node_by_pos_from_table(table, i, &id, &addr);
		DB_get_node_by_pos(view, i, &id, &addr);

		DB_read_node(id, addr, (UINT8 *)&p_node, sizeof(p_node));
		ti[i].id = p_node.tp_id;
		key_array[i] = ti[i].id;
		ti[i].value = p_node.LCN;
		ti[i].next_pos = INVALID_POS_NUM;
		//libc_printf("ti[%d], id=0x%x, value=0x%x,next_pos=%d\n",i,ti[i].id,ti[i].value,ti[i].next_pos);
	}

	//sort tp by id in ascend order
	//shellsort2(ti, key_array, 0, n-1);

	//libc_printf("\ntp after id sort:\n");
	//for(i = 0; i<n; i++)
	//	libc_printf("ti[%d], id=0x%x, value=0x%x,next_pos=%d\n",i,ti[i].id,ti[i].value,ti[i].next_pos);
	
	// retrieve prog view , read the tp_id field and add it into the p_info->next single linked list.
	//here need start from the last program to link the programs under one tp 
	for(i=0,k=(INT16)view->node_num-1; k>=0; k--,i++) 
	{
		MEMCPY(&id, view->node_buf[k].node_id, NODE_ID_SIZE);
		//id = (id&NODE_ID_TP_MASK)>>NODE_ID_PG_BIT;
		//t_idx = lookup_ti_id(ti, id, n);
		//if(t_idx == -1)
		//	return DBERR_API_NOFOUND;
		//pi[i].node_pos = k;
		//pi[i].next_pos = ti[t_idx].next_pos;
		
		//libc_printf("pi[%d], prog_pos=%d, next_pos=%d,tp t_idx=%d\n",i,pi[i].node_pos,pi[i].next_pos,t_idx);
		ti[k].next_pos = k;
	}

	/* sort the t_info array by value in ascend way. */
	for(i=0; i< n ; i++)
		key_array[i] = ti[i].value;
	shellsort2(ti, key_array, 0, n-1);
	//libc_printf("\n tp after value sort:\n");
	//for(i = 0; i<n; i++)
	//	libc_printf("ti[%d], value=0x%x, id=0x%x, next_pos=%d\n",i,ti[i].value, ti[i].id, ti[i].next_pos);

	UINT16 max_idx = view->node_num;
	/* generating the sorted program list */
	for(i=0; i<max_idx; i++)
			{
		MEMCPY(&sort[i], &view->node_buf[ti[i].next_pos], sizeof(NODE_IDX_PARAM));
		flg[i] = view->node_flag[ti[i].next_pos];
	}

	MEMCPY(view->node_buf, sort, sizeof(NODE_IDX_PARAM)*max_idx);
	//if(view->node_flag)
	MEMCPY(view->node_flag, flg, sizeof(UINT8)*max_idx);
	 
	//if need sort descend order
	if(sort_order == 1)
	{
		n -= 1;
		for(i = 0; i < n-i; i++)
		{
			MEMCPY(&node_idx,&view->node_buf[i],sizeof(NODE_IDX_PARAM));
			flag = view->node_flag[i];
			MEMCPY(&view->node_buf[i],&view->node_buf[n-i],sizeof(NODE_IDX_PARAM));
			view->node_flag[i] = view->node_flag[n-i];
			MEMCPY(&view->node_buf[n-i],&node_idx,sizeof(NODE_IDX_PARAM));
			view->node_flag[n-i] = flag;
		}		
	}
	
	DB_EXT_PRINTF("sort_default_sort():finished, tick = %d\n", osal_get_tick());
	
	db_table[TYPE_PROG_NODE].node_moved = 1;
	return DB_SUCCES;
}

#endif


static INT32 sort_lcn_sort(INT8 sort_order)
{

	UINT16 i, j;
	INT16 t_idx, k;
	struct sort_t_info *ti = (struct sort_t_info *)db_sort_t_buf;
	struct sort_p_info *pi = (struct sort_p_info *)db_sort_p_buf;
	KEY_VALUE *key_array = (KEY_VALUE *)(db_sort_p_buf+MAX_PROG_NUM*sizeof(struct sort_p_info));
	NODE_IDX_PARAM *sort = (NODE_IDX_PARAM *)(db_sort_p_buf+MAX_PROG_NUM*sizeof(struct sort_p_info));
	UINT8 *flg = (UINT8 *)(db_sort_p_buf+MAX_PROG_NUM*(sizeof(struct sort_p_info)+sizeof(NODE_IDX_PARAM)));

	DB_VIEW *view = (DB_VIEW *)&db_view;
	DB_TABLE *table = (DB_TABLE *)&db_table[TYPE_PROG_NODE];
	P_NODE p_node;
	UINT32 value;
	UINT16 next;
	NODE_IDX_PARAM node_idx;
	UINT8 flag;
	UINT16 n = 0;
	UINT32 id = 0, addr = 0;

	DB_EXT_PRINTF("sort_default_sort():begin, tick = %d\n",osal_get_tick());

	/* retrieve current tp view, store them into p_info array. */
	n = view->node_num;;//table->node_num;
	
	for(i = 0; i < n; i++)
	{
		DB_get_node_by_pos(view, i, &id, &addr);

		DB_read_node(id, addr, (UINT8 *)&p_node, sizeof(p_node));
		ti[i].id = p_node.tp_id;
		key_array[i] = ti[i].id;
		ti[i].value = p_node.logical_channel_num;
		ti[i].next_pos = INVALID_POS_NUM;
	}

	// retrieve prog view , read the tp_id field and add it into the p_info->next single linked list.
	//here need start from the last program to link the programs under one tp 
	for(i=0,k=(INT16)view->node_num-1; k>=0; k--,i++) 
	{
		MEMCPY(&id, view->node_buf[k].node_id, NODE_ID_SIZE);
		ti[k].next_pos = k;
	}

	/* sort the t_info array by value in ascend way. */
	for(i=0; i< n ; i++)
		key_array[i] = ti[i].value;
	shellsort2(ti, key_array, 0, n-1);

	UINT16 max_idx = view->node_num;
	/* generating the sorted program list */
	for(i=0; i<max_idx; i++)
			{
		MEMCPY(&sort[i], &view->node_buf[ti[i].next_pos], sizeof(NODE_IDX_PARAM));
		flg[i] = view->node_flag[ti[i].next_pos];
	}

	MEMCPY(view->node_buf, sort, sizeof(NODE_IDX_PARAM)*max_idx);
	//if(view->node_flag)
	MEMCPY(view->node_flag, flg, sizeof(UINT8)*max_idx);
	 
	//if need sort descend order
	if(sort_order == 1)
	{
		n -= 1;
		for(i = 0; i < n-i; i++)
		{
			MEMCPY(&node_idx,&view->node_buf[i],sizeof(NODE_IDX_PARAM));
			flag = view->node_flag[i];
			MEMCPY(&view->node_buf[i],&view->node_buf[n-i],sizeof(NODE_IDX_PARAM));
			view->node_flag[i] = view->node_flag[n-i];
			MEMCPY(&view->node_buf[n-i],&node_idx,sizeof(NODE_IDX_PARAM));
			view->node_flag[n-i] = flag;
		}		
	}
	
	DB_EXT_PRINTF("sort_default_sort():finished, tick = %d\n", osal_get_tick());
	
	db_table[TYPE_PROG_NODE].node_moved = 1;
	return DB_SUCCES;
}


static INT32 sort_serviceid_sort(INT8 sort_order)
{

	UINT16 i, j;
	INT16 t_idx, k;
	struct sort_t_info *ti = (struct sort_t_info *)db_sort_t_buf;
	struct sort_p_info *pi = (struct sort_p_info *)db_sort_p_buf;
	KEY_VALUE *key_array = (KEY_VALUE *)(db_sort_p_buf+MAX_PROG_NUM*sizeof(struct sort_p_info));
	NODE_IDX_PARAM *sort = (NODE_IDX_PARAM *)(db_sort_p_buf+MAX_PROG_NUM*sizeof(struct sort_p_info));
	UINT8 *flg = (UINT8 *)(db_sort_p_buf+MAX_PROG_NUM*(sizeof(struct sort_p_info)+sizeof(NODE_IDX_PARAM)));

	DB_VIEW *view = (DB_VIEW *)&db_view;
	DB_TABLE *table = (DB_TABLE *)&db_table[TYPE_PROG_NODE];
	P_NODE p_node;
	UINT32 value;
	UINT16 next;
	NODE_IDX_PARAM node_idx;
	UINT8 flag;
	UINT16 n = 0;
	UINT32 id = 0, addr = 0;

	DB_EXT_PRINTF("sort_default_sort():begin, tick = %d\n",osal_get_tick());

	/* retrieve current tp view, store them into p_info array. */
	n = view->node_num;;//table->node_num;
	
	for(i = 0; i < n; i++)
	{
		DB_get_node_by_pos(view, i, &id, &addr);

		DB_read_node(id, addr, (UINT8 *)&p_node, sizeof(p_node));
		ti[i].id = p_node.tp_id;
		key_array[i] = ti[i].id;
#ifdef DB_VIEW_TV_B4_RADIO		
		ti[i].value = ((1-p_node.av_flag&0x1)<<31) | (p_node.prog_number&0x7fffffff);
#else
		ti[i].value = p_node.prog_number;
#endif
		ti[i].next_pos = INVALID_POS_NUM;
	}

	// retrieve prog view , read the tp_id field and add it into the p_info->next single linked list.
	//here need start from the last program to link the programs under one tp 
	for(i=0,k=(INT16)view->node_num-1; k>=0; k--,i++) 
	{
		MEMCPY(&id, view->node_buf[k].node_id, NODE_ID_SIZE);
		ti[k].next_pos = k;
	}

	/* sort the t_info array by value in ascend way. */
	for(i=0; i< n ; i++)
		key_array[i] = ti[i].value;
	shellsort2(ti, key_array, 0, n-1);

	UINT16 max_idx = view->node_num;
	/* generating the sorted program list */
	for(i=0; i<max_idx; i++)
			{
		MEMCPY(&sort[i], &view->node_buf[ti[i].next_pos], sizeof(NODE_IDX_PARAM));
		flg[i] = view->node_flag[ti[i].next_pos];
	}

	MEMCPY(view->node_buf, sort, sizeof(NODE_IDX_PARAM)*max_idx);
	//if(view->node_flag)
	MEMCPY(view->node_flag, flg, sizeof(UINT8)*max_idx);
	 
	//if need sort descend order
	if(sort_order == 1)
	{
		n -= 1;
		for(i = 0; i < n-i; i++)
		{
			MEMCPY(&node_idx,&view->node_buf[i],sizeof(NODE_IDX_PARAM));
			flag = view->node_flag[i];
			MEMCPY(&view->node_buf[i],&view->node_buf[n-i],sizeof(NODE_IDX_PARAM));
			view->node_flag[i] = view->node_flag[n-i];
			MEMCPY(&view->node_buf[n-i],&node_idx,sizeof(NODE_IDX_PARAM));
			view->node_flag[n-i] = flag;
		}		
	}
	
	DB_EXT_PRINTF("sort_default_sort():finished, tick = %d\n", osal_get_tick());
	
	db_table[TYPE_PROG_NODE].node_moved = 1;
	return DB_SUCCES;
}

#if (defined(_ISDBT_ENABLE_))
INT16 check_ISDBT_index(UINT32 virtual_chan)
{
	DB_VIEW *view = (DB_VIEW *)&db_view;
	DB_TABLE *table = (DB_TABLE *)&db_table[TYPE_TP_NODE];
	P_NODE p_node;
	T_NODE t_node;
	UINT16 n = 0;
	UINT16 i;
	UINT32 id = 0, addr = 0;
	UINT32 tmp_id = 0;
	UINT32 prog_v_id = 0;
	INT16 ch_idx = -1;

	DB_ENTER_MUTEX();

	n = view->node_num;

	for (i= 0; i<n; i++)
	{
		DB_get_node_by_pos(view, i, &id, &addr);

		DB_read_node(id, addr, (UINT8 *)&p_node, sizeof(p_node));
		tmp_id = p_node.tp_id<<NODE_ID_PG_BIT;
		DB_get_node_by_id_from_table(table, tmp_id, &addr);
		DB_read_node(tmp_id, addr, &t_node, sizeof(t_node));

		prog_v_id = t_node.remote_control_key_id*100 + ((p_node.prog_number & 0x1f)>>3)*10 + ((p_node.prog_number & 0x07)+1);
		
		if (prog_v_id == virtual_chan || ((virtual_chan%100) == 0 && (virtual_chan/100) == t_node.remote_control_key_id))
		{
#ifdef _SERVICE_ATTRIBUTE_ENABLE_		
			if(p_node.num_sel_flag | p_node.visible_flag)
#endif
			ch_idx = i;
			break;
		}
	}

	DB_RELEASE_MUTEX();

	return ch_idx;

}

static INT32 sort_ISDBT_sort(INT8 sort_order)
{
	UINT16 i, j;
	INT16 t_idx, k;
	struct sort_t_info *ti = (struct sort_t_info *)db_sort_t_buf;
	struct sort_p_info *pi = (struct sort_p_info *)db_sort_p_buf;
	KEY_VALUE *key_array = (KEY_VALUE *)(db_sort_p_buf+MAX_PROG_NUM*sizeof(struct sort_p_info));
	NODE_IDX_PARAM *sort = (NODE_IDX_PARAM *)(db_sort_p_buf+MAX_PROG_NUM*sizeof(struct sort_p_info));
	UINT8 *flg = (UINT8 *)(db_sort_p_buf+MAX_PROG_NUM*(sizeof(struct sort_p_info)+sizeof(NODE_IDX_PARAM)));

	DB_VIEW *view = (DB_VIEW *)&db_view;
	DB_TABLE *table = (DB_TABLE *)&db_table[TYPE_TP_NODE];
	P_NODE p_node;
	T_NODE t_node;
	UINT32 value;
	UINT16 next;
	NODE_IDX_PARAM node_idx;
	UINT8 flag;
	UINT16 n = 0;
	UINT32 id = 0, addr = 0;
	UINT32 tmp_id = 0;

	DB_EXT_PRINTF("sort_default_sort():begin, tick = %d\n",osal_get_tick());

	/* retrieve current tp view, store them into p_info array. */
	n = view->node_num;;//table->node_num;
	
	for(i = 0; i < n; i++)
	{
		//DB_get_node_by_pos_from_table(table, i, &id, &addr);
		DB_get_node_by_pos(view, i, &id, &addr);

		DB_read_node(id, addr, (UINT8 *)&p_node, sizeof(p_node));
		tmp_id = p_node.tp_id<<NODE_ID_PG_BIT;
		DB_get_node_by_id_from_table(table, tmp_id, &addr);
		DB_read_node(tmp_id, addr, &t_node, sizeof(t_node));

		ti[i].id = p_node.tp_id;
		key_array[i] = ti[i].id;
		ti[i].value = (t_node.remote_control_key_id << 5) | (p_node.prog_number & 0x1f);
		ti[i].next_pos = i;
	}

	/* sort the t_info array by value in ascend way. */
	for(i=0; i< n ; i++)
		key_array[i] = ti[i].value;
	shellsort2(ti, key_array, 0, n-1);

	UINT16 max_idx = view->node_num;
	/* generating the sorted program list */
	for(i=0; i<max_idx; i++)
	{
		MEMCPY(&sort[i], &view->node_buf[ti[i].next_pos], sizeof(NODE_IDX_PARAM));
		flg[i] = view->node_flag[ti[i].next_pos];
	}

	MEMCPY(view->node_buf, sort, sizeof(NODE_IDX_PARAM)*max_idx);
	//if(view->node_flag)
	MEMCPY(view->node_flag, flg, sizeof(UINT8)*max_idx);
	 
	//if need sort descend order
	if(sort_order == 1)
	{
		n -= 1;
		for(i = 0; i < n-i; i++)
		{
			MEMCPY(&node_idx,&view->node_buf[i],sizeof(NODE_IDX_PARAM));
			flag = view->node_flag[i];
			MEMCPY(&view->node_buf[i],&view->node_buf[n-i],sizeof(NODE_IDX_PARAM));
			view->node_flag[i] = view->node_flag[n-i];
			MEMCPY(&view->node_buf[n-i],&node_idx,sizeof(NODE_IDX_PARAM));
			view->node_flag[n-i] = flag;
		}		
	}
	
	DB_EXT_PRINTF("sort_default_sort():finished, tick = %d\n", osal_get_tick());
	
	db_table[TYPE_PROG_NODE].node_moved = 1;
	return DB_SUCCES;
}
#endif

//sort program by service type, within eatch type, sort by ts_id and service_id
static UINT8 service_type[] =
{
	0x01,//SERVICE_TYPE_DTV,
	0x02,//SERVICE_TYPE_DRADIO,
	0x0c,//SERVICE_TYPE_DATABROAD
	0x05,//SERVICE_TYPE_NVOD_TS,
};

extern UINT8 db_search_mode;
extern DB_VIEW db_search_view;
static UINT32 prog_type_stsid_sort()
{
	//current support sort service type 1:tv; 2:radio; 0xc:data;4,5: nvod; 
	INT16 i, j;
	UINT16 node_cnt[4];
	//each service type group support max 256 program
	NODE_IDX_PARAM *param_array[4];
	UINT16 param_offset = 256*sizeof(NODE_IDX_PARAM);
		
	KEY_VALUE *key_array[4];
	UINT16 key_offset = 256*sizeof(KEY_VALUE);
	
	UINT8 *flag_array[4];
	UINT16 flag_offset = 256;
	
	//program view should be all program view
	DB_VIEW *p_view = (DB_VIEW *)&db_view;
	DB_VIEW *t_view = (DB_VIEW *)&db_search_view;	
	DB_VIEW sort_view;
	T_NODE *tnode = NULL;
	P_NODE *pnode = NULL;
	UINT16 ts_id = 0;
	UINT32 prog_buf = 0, tp_buf = 0;
	//if in search mode, pnode and tnode got from ram to make it faster
	if(db_search_mode!=1)
		return -1;
	MEMSET(node_cnt,0,sizeof(node_cnt));
	for(i = 0; i < 4; i++)
	{
		param_array[i] = (NODE_IDX_PARAM *)(db_sort_p_buf+i*param_offset);
		key_array[i] = (KEY_VALUE *)(db_sort_p_buf+4*param_offset+i*key_offset);
		flag_array[i] = (UINT8 *)(db_sort_p_buf+4*param_offset+4*key_offset+i*flag_offset);
	}
	db_get_search_nodebuf(&prog_buf, &tp_buf);
#if 0    
	libc_printf("%s(): prog view node cnt[%d], tick=%d\n",__FUNCTION__,p_view->node_num,osal_get_tick());
	for(i = 0; i < p_view->node_num; i++)
	{
		pnode =  (P_NODE *)(prog_buf+ i*sizeof( P_NODE));
		for(j = 0; j < t_view->node_num; j++)
		{
			tnode = (T_NODE *)(tp_buf+j*sizeof(T_NODE));
			if(pnode->tp_id==tnode->tp_id)
			{
				ts_id = tnode->t_s_id;
				libc_printf("service type[%d], id[%d], ts_id[%d]\n",pnode->service_type,pnode->prog_number,
					ts_id);
				break;
			}
		}
		if(j==t_view->node_num)
		{
			libc_printf("%s(): service[%d] tp not found\n",__FUNCTION__,pnode->prog_number);
			return -1;
		}
	}
#endif	

	//loop once, put programs into 4 service type groups
	for(i = 0; i < p_view->node_num; i++)
	{
		pnode =  (P_NODE *)(prog_buf+ i*sizeof( P_NODE));
		#if 0
		for(j = 0; j < t_view->node_num; j++)
		{
			tnode = (T_NODE *)(tp_buf+j*sizeof(T_NODE));
			if(pnode->tp_id==tnode->tp_id)
			{
				ts_id = tnode->t_s_id;
				break;
			}
		}
		if(j==t_view->node_num)
		{
			return -1;
		}
		#endif
		for(j = 0; j < 4; j++)
		{
			if(pnode->service_type==service_type[j])
			{
				NODE_IDX_PARAM *param = param_array[j];
				KEY_VALUE *key = key_array[j];
				UINT8 *flag = flag_array[j];
				UINT16 cnt = node_cnt[j];
				
				MEMCPY(&param[cnt], &p_view->node_buf[i], sizeof(NODE_IDX_PARAM));
				key[cnt] = pnode->prog_number;
				flag[cnt] = p_view->node_flag[i];
				node_cnt[j] += 1;
				break;
			}
		}
		
	}
	//within each type, sort program by tsid and service id
	for(i = 0; i < 4; i++)
	{
		MEMSET(&sort_view, 0, sizeof(DB_VIEW));
		sort_view.node_num = node_cnt[i];
		sort_view.node_buf = param_array[i];
		sort_view.node_flag = flag_array[i];
		shellsort(&sort_view, key_array[i], 0, node_cnt[i]-1);
	}

	//merge sorted programs into one view
	MEMCPY(p_view->node_buf, param_array[0], node_cnt[0]*sizeof(NODE_IDX_PARAM));
	MEMCPY(p_view->node_flag, flag_array[0], node_cnt[0]);
	j = node_cnt[0];
	for(i = 1; i < 4; i++)
	{
            MEMCPY(&p_view->node_buf[j],param_array[i],node_cnt[i]*sizeof(NODE_IDX_PARAM));
            MEMCPY(&p_view->node_flag[j],flag_array[i],node_cnt[i]);
		j += node_cnt[i];
	}
	db_table[TYPE_PROG_NODE].node_moved = 1;
#if 0
    P_NODE node;
    UINT8 tmp_buf[P_NODE_FIX_LEN];
    INT32 addr = 0;
    libc_printf("after sort, tick=%d\n",osal_get_tick());
    for(i = 0; i < p_view->node_num; i++)
    {
        addr=p_view->node_buf[i].node_addr[0]|p_view->node_buf[i].node_addr[1]<<8|p_view->node_buf[i].node_addr[2]<<16;
        BO_read_data(addr+NODE_ID_SIZE+1, P_NODE_FIX_LEN, (UINT8 *)&tmp_buf);
        MEMCPY((UINT8 *)&(node.prog_id)+sizeof(UINT32), tmp_buf, P_NODE_FIX_LEN);
        libc_printf("service type[0x%x],service id[%d] \n",node.service_type,node.prog_number);
    }
#endif	
  	return DB_SUCCES;
}


INT32 sort_prog_node(UINT8 sort_flag)
{
	DB_VIEW *view= (DB_VIEW *)&db_view;
	INT32 ret = DB_SUCCES;

	DB_ENTER_MUTEX();
	if(DB_get_cur_view_type(view) != TYPE_PROG_NODE)
	{
		DB_RELEASE_MUTEX();
		DB_EXT_PRINTF(" sort_prog_node(): cur_view_type not prog type!\n");
		return DBERR_API_NOFOUND;
	}

	ret = sort_assign_buf(sort_flag);
	if(ret != DB_SUCCES)
	{
		DB_RELEASE_MUTEX();
		return ret;
	}
		
	if(sort_flag == PROG_FTA_SORT || sort_flag == PROG_FTA_SORT_EXT 
		|| sort_flag == PROG_LOCK_SORT || sort_flag == PROG_LOCK_SORT_EXT)
		ret = prog_bina_key_sort(sort_flag);
	else if(sort_flag == PROG_DEFAULT_SORT)
	{
#if (defined(_ISDBT_ENABLE_))
		ret = sort_ISDBT_sort(0);
#elif (defined(_LCN_ENABLE_))	
		ret = sort_default_index(0);
#else
		ret = sort_default_sort(TYPE_PROG_NODE, -1);
#endif
	}
	else if(sort_flag == PROG_DEFAULT_SORT_EXT)
	{
#if(defined(_LCN_ENABLE_))
		ret = sort_default_index(1);
#else
		ret = sort_default_sort(TYPE_PROG_NODE, 1);
#endif
	}
	else if(sort_flag == PROG_LCN_SORT)
	{
		ret = sort_lcn_sort(0);
	}
	else if(sort_flag==PROG_TYPE_SID_TSID_SORT)
	{
		ret = prog_type_stsid_sort();
	}
	else if(sort_flag==PROG_TYPE_SID_SORT)
	{
		ret = sort_serviceid_sort(0);
	}
	else if(sort_flag==PROG_TYPE_SID_SORT_EXT)
	{
		ret = sort_serviceid_sort(1);
	}
	else 
	{
		ret = prog_info_sort(sort_flag);
	}


	DB_RELEASE_MUTEX();
	return ret;
}

//add this api for program cas sort
INT32 sort_prog_node_advance(UINT8 sort_flag, UINT32 sort_param)
{
	DB_VIEW *view= (DB_VIEW *)&db_view;
	INT32 ret = DB_SUCCES;
	
	DB_ENTER_MUTEX();
	if(DB_get_cur_view_type(view) != TYPE_PROG_NODE)
	{
		DB_RELEASE_MUTEX();
		DB_EXT_PRINTF(" sort_prog_node(): cur_view_type not prog type!\n");
		return DBERR_API_NOFOUND;
	}
	
	ret = sort_assign_buf(sort_flag);
	if(ret != DB_SUCCES)
	{
		DB_RELEASE_MUTEX();
		return ret;
	}

	ret = prog_advance_sort(sort_flag, sort_param);
	
	DB_RELEASE_MUTEX();
	return ret;

}


INT32 sort_tp_node(UINT8 sort_flag)
{
	DB_VIEW *view= (DB_VIEW *)&db_view;
	INT32 ret = DB_SUCCES;

	DB_ENTER_MUTEX();
	if(DB_get_cur_view_type(view) != TYPE_TP_NODE)
	{
		DB_RELEASE_MUTEX();
		DB_EXT_PRINTF(" sort_tp_node(): cur_view_type not tp type!\n");
		return DBERR_API_NOFOUND;
	}

	ret = sort_assign_buf(sort_flag);
	if(ret != DB_SUCCES)
	{
		DB_RELEASE_MUTEX();
		return ret;
	}
	
	if(sort_flag == TP_DEFAULT_SORT)
	{
		ret = sort_default_sort(TYPE_TP_NODE, -1);
	}
	else if(sort_flag == TP_DEFAULT_SORT_EXT)
	{
		ret = sort_default_sort(TYPE_TP_NODE, 1);
	}

	DB_RELEASE_MUTEX();	
	return ret;
}



UINT16 get_provider_num(void *name_buf, UINT32 buf_size)
{
	UINT16 num = 0;
	INT32 ret = DB_SUCCES;
	INT16 i, j,m=0; 
	INT16 k;
	P_NODE p_node;
	
	UINT8 tmp[DB_NODE_MAX_SIZE];
	DB_VIEW *view = (DB_VIEW *)&db_view;
	UINT32 node_addr = 0;
	UINT8 offset = 0, packed_len = 0;
	UINT8 cas_cnt = 0;
	
	if((DB_ETYPE *)name_buf==NULL)
	 return DBERR_PARAM;
	 
	UINT8 *provider_name = (UINT8 *)name_buf;   
	UINT16 provider_cnt_limit = buf_size/(2*(MAX_SERVICE_NAME_LENGTH + 1));
	DB_ENTER_MUTEX();
	
	for(i = 0; i < view->node_num; i++)
	{
		MEMSET(&p_node, 0, sizeof(P_NODE));
		MEMCPY(&node_addr, view->node_buf[i].node_addr, NODE_ADDR_SIZE);
		ret = BO_read_data(node_addr+NODE_ID_SIZE, 1, tmp);
		packed_len = tmp[0];
				
		ret = BO_read_data(node_addr+NODE_ID_SIZE+1+P_NODE_FIX_LEN, packed_len-1-P_NODE_FIX_LEN, tmp);
		
#ifdef DB_CAS_SUPPORT
		cas_cnt = tmp[0];
		//cas system id
		offset = 1+cas_cnt*2;
#endif
	   	//audio count,pid,lang
  		p_node.audio_count = tmp[offset];
  		offset += (1+p_node.audio_count *4);
  		//service name
  		p_node.name_len = tmp[offset];
		offset += 1;
		//MEMCPY(p_node.service_name, tmp+offset, p_node.name_len);
  		offset += (p_node.name_len);
	#if(defined(SUPPORT_MTILINGUAL_SERVICE_NAME))
		//mtl service name
		for(m=0;m<p_node.mtl_name_count;m++)
  		{
  			p_node.mtl_name_len[m] = tmp[offset];
			offset += 1;
		//MEMCPY(p_node.service_name, tmp+offset, p_node.name_len);
  			offset += (p_node.mtl_name_len[m]);
		}
	#endif
#if (SERVICE_PROVIDER_NAME_OPTION>0)
  		//provider name
  		p_node.provider_name_len = tmp[offset];
		if(p_node.provider_name_len==0)
			continue;
  		offset += 1;
  		MEMCPY(p_node.service_provider_name, tmp+offset, p_node.provider_name_len);
#endif
		/*
			libc_printf("\n%d prog name:",i+1);
			for(j=1;j<p_node.name_len;j+=2)
				libc_printf("%c",p_node.service_name[j]);
			libc_printf(", provider name:");
			for(j=1;j<p_node.provider_name_len;j+=2)
				libc_printf("%c",p_node.service_provider_name[j]);
			libc_printf("\n");
		*/
		for(j = 0; j < num; j++)
		{
			if(0==DB_STRCMP((DB_ETYPE *)p_node.service_provider_name,(DB_ETYPE *)(provider_name+2*(MAX_SERVICE_NAME_LENGTH + 1)*j)))
				break;
		}
      		if(j==num)
		{
			if(num >= provider_cnt_limit)
			{
				DB_EXT_PRINTF(" name buf size not enough, provider_cnt_limit=%d!\n",provider_cnt_limit);
				num = 0xFFFF;
				break;
			}
			MEMSET(provider_name+2*(MAX_SERVICE_NAME_LENGTH + 1)*num, 0, 2*(MAX_SERVICE_NAME_LENGTH + 1));
			MEMCPY(provider_name+2*(MAX_SERVICE_NAME_LENGTH + 1)*num,p_node.service_provider_name,p_node.provider_name_len);
			num++;
			/*
			libc_printf("add provider %d:",num);
			for(k=1;k<p_node.provider_name_len;k+=2)
				libc_printf("%c",p_node.service_provider_name[k]);
			libc_printf("\n");
			*/
		}
  
  	}
	
	//libc_printf("\nprovider count %d\n",num);	
	DB_RELEASE_MUTEX();
	return num;
}



UINT16 get_cas_num(UINT16*cas_id_buf, UINT32 buf_size)
{
	INT16 i, j, k;
	UINT8 tmp[DB_NODE_MAX_SIZE];
	DB_VIEW *view = (DB_VIEW *)&db_view;
	UINT32 node_addr = 0;
	UINT8 cas_cnt = 0;
	INT16 match_num = 0;
		
	DB_ENTER_MUTEX();
	UINT16 *prog_casys_id = (UINT16 *)tmp;
	UINT16 cas_num_limit = buf_size/sizeof(UINT16);
	
	for(i = 0; i < view->node_num; i++)
	{
		MEMCPY(&node_addr, view->node_buf[i].node_addr, NODE_ADDR_SIZE);
		//ca count
		BO_read_data(node_addr+NODE_ID_SIZE+1+P_NODE_FIX_LEN, 1, &cas_cnt);
		if(cas_cnt > 0)
		{
			//ca system id
			BO_read_data(node_addr+NODE_ID_SIZE+1+P_NODE_FIX_LEN+1, cas_cnt*2, tmp);
			//libc_printf("\nprogram %d cas cnt=%d:\n", i+1, cas_cnt);
			for(j = 0;j < cas_cnt; j++)
			{
				//libc_printf("    cas %d id=0x%x\n", j+1,prog_casys_id[j]);
				for(k = 0; k < match_num; k++)
				{
					if((prog_casys_id[j]&0xFF00) == cas_id_buf[k])
						break;
				}
				if(k==match_num)
				{
					if(match_num >= cas_num_limit)
					{
						//libc_printf("cas buf size not enough, cas_num_limit=%d!\n",cas_num_limit);
						DB_RELEASE_MUTEX();
						return 0xFFFF;
					}
					cas_id_buf[match_num] = prog_casys_id[j]&0xFF00;
					match_num++;
					//libc_printf("ADD cas system, id=0x%x!\n",prog_casys_id[j]&0xFF00);
				}
			}
		}
	}

	DB_RELEASE_MUTEX();
	return match_num;
}

#ifdef User_order_sort
void _prog_info_order_sort( INT8 sort_order )
{
	UINT16 i, j;
	INT16 t_idx, k;
	struct sort_t_info *ti = (struct sort_t_info *)db_sort_t_buf;
	struct sort_p_info *pi = (struct sort_p_info *)db_sort_p_buf;
	KEY_VALUE *key_array = (KEY_VALUE *)(db_sort_p_buf+MAX_PROG_NUM*sizeof(struct sort_p_info));
	NODE_IDX_PARAM *sort = (NODE_IDX_PARAM *)(db_sort_p_buf+MAX_PROG_NUM*sizeof(struct sort_p_info));
	UINT8 *flg = (UINT8 *)(db_sort_p_buf+MAX_PROG_NUM*(sizeof(struct sort_p_info)+sizeof(NODE_IDX_PARAM)));

	DB_VIEW *view = (DB_VIEW *)&db_view;
	DB_TABLE *table = (DB_TABLE *)&db_table[TYPE_PROG_NODE];
	P_NODE p_node;
	UINT32 value;
	UINT16 next;
	NODE_IDX_PARAM node_idx;
	UINT8 flag;
	UINT16 n = 0;
	UINT32 id = 0, addr = 0;
	UINT32 loop = 0;
	DB_EXT_PRINTF("sort_default_sort():begin, tick = %d\n",osal_get_tick());

	/* retrieve current tp view, store them into p_info array. */
	n = view->node_num;;//table->node_num;
	for(i = 0; i < n; i++)
	{
		DB_get_node_by_pos(view, i, &id, &addr);

		DB_read_node(id, addr, (UINT8 *)&p_node, sizeof(p_node));
		ti[i].id = p_node.prog_id;
		ti[i].value = p_node.user_order;
		ti[i].next_pos = i;
	}

	/* sort the t_info array by value in ascend way. */
	for(i=0; i< n ; i++)
		key_array[i] = ti[i].value;
	shellsort2(ti, key_array, 0, n-1);

	UINT16 max_idx = view->node_num;
	/* generating the sorted program list */
	
	for(i=0; i<max_idx; i++)
	{
		MEMCPY(&sort[i], &view->node_buf[ti[i].next_pos], sizeof(NODE_IDX_PARAM));
		flg[i] = view->node_flag[ti[i].next_pos];
	}
	
	MEMCPY(view->node_buf, sort, sizeof(NODE_IDX_PARAM)*max_idx);
	//if(view->node_flag)
	MEMCPY(view->node_flag, flg, sizeof(UINT8)*max_idx);
	 
	//if need sort descend order
	if(sort_order == 1)
	{
		n -= 1;
		for(i = 0; i < n-i; i++)
		{
			MEMCPY(&node_idx,&view->node_buf[i],sizeof(NODE_IDX_PARAM));
			flag = view->node_flag[i];
			MEMCPY(&view->node_buf[i],&view->node_buf[n-i],sizeof(NODE_IDX_PARAM));
			view->node_flag[i] = view->node_flag[n-i];
			MEMCPY(&view->node_buf[n-i],&node_idx,sizeof(NODE_IDX_PARAM));
			view->node_flag[n-i] = flag;
		}		
	}
	
	DB_EXT_PRINTF("sort_default_sort():finished, tick = %d\n", osal_get_tick());
	db_table[TYPE_PROG_NODE].node_moved = 1;
//	update_data();
	return DB_SUCCES;

}

/*UINT32 prog_user_order_sort_ext( INT8 sort_order )
{
	_prog_info_order_sort(sort_order);
	return 0;
}*/


UINT32 prog_set_def_order(void )
{
	UINT16 i;
	DB_VIEW *view = (DB_VIEW *)&db_view;
	P_NODE p_node;
	NODE_IDX_PARAM node_idx;
	UINT8 flag;
	UINT16 n = 0;
	UINT32 id = 0, addr = 0;
	n = view->node_num;;//table->node_num;	
	for(i = 0; i < n; i++)
	{
		DB_get_node_by_pos(view, i, &id, &addr);
		DB_read_node(id, addr, (UINT8 *)&p_node, sizeof(p_node));
		p_node.user_order = i+1;
		DB_modify_node_by_id(view, id, &p_node, sizeof(p_node));
		DB_update_operation();
	}	
	return DB_SUCCES;	
}


void test(void)
{	
	UINT16 i;
	DB_VIEW *view = (DB_VIEW *)&db_view;
	P_NODE p_node;
	NODE_IDX_PARAM node_idx;
	UINT8 flag;
	UINT16 n = 0;
	UINT32 id = 0, addr = 0;
	n = view->node_num;;//table->node_num;	
	for(i = 0; i < n; i++)
	{
		DB_get_node_by_pos(view, i, &id, &addr);
		DB_read_node(id, addr, (UINT8 *)&p_node, sizeof(p_node));
		soc_printf(" p_node->usr_order = %d\n  ",p_node.user_order);
	}	
	return;	
}

UINT32 prog_user_order_sort_ext( INT8 sort_order )
{
	sort_assign_buf(0x0a);
	_prog_info_order_sort( 0 );
	return 0;
}


INT32 move_prog_ext(UINT16 dest_pos,UINT16 src_pos)
{
	INT32 ret = DB_SUCCES;

	DB_ENTER_MUTEX();
	if(TYPE_PROG_NODE!= DB_get_cur_view_type((DB_VIEW *)&db_view))
	{
		DB_RELEASE_MUTEX();
		DB_EXT_PRINTF(" move_prog(): cur_view_type not pg!\n");
		return DBERR_API_NOFOUND;
	}
	ret = DO_v_move_node_ext((DB_VIEW *)&db_view, src_pos, dest_pos);
	DB_RELEASE_MUTEX();
	return ret;

}

INT32 DO_v_move_node_ext(DB_VIEW  * v_attr,UINT16 src_pos, UINT16 dest_pos)
{
	INT32 result = DB_SUCCES;
	UINT16 i;
	UINT8 tmp_flag = 0;
	UINT8 tmp_id[NODE_ID_SIZE];
	UINT8 tmp_addr[NODE_ADDR_SIZE];
	UINT32 snode_id,node_id;
	UINT32 snode_addr,node_addr;
	P_NODE p_node,sp_node;
	UINT32 *ret_addr;

//	prog_set_def_order();
//	test();
	

	if(src_pos >= v_attr->node_num || dest_pos >= v_attr->node_num)
	{
		DB_EXT_PRINTF("DO_v_move_node(): src_pos %d  or dest pos %d exceed view node num %d!\n", src_pos, dest_pos,v_attr->node_num);	
		return DBERR_PARAM;
	}

	if(src_pos == dest_pos)
		return DB_SUCCES;

//	DB_get_node_by_pos(v_attr, src_pos, &snode_id, &snode_addr);
//	DB_read_node(snode_id, snode_addr, (UINT8 *)&sp_node, sizeof(sp_node));
	DB_get_node_by_pos(v_attr, src_pos, &snode_id, &snode_addr);
	DB_read_node(snode_id, snode_addr, (UINT8 *)&sp_node, sizeof(sp_node));
	sp_node.user_order += dest_pos - src_pos;	
	DB_modify_node_by_id(v_attr, snode_id, &sp_node, sizeof(sp_node));
	DB_update_operation();
	
	
	MEMCPY(&tmp_id, &v_attr->node_buf[src_pos].node_id[0],NODE_ID_SIZE);
	MEMCPY(&tmp_addr, &v_attr->node_buf[src_pos].node_addr[0], NODE_ADDR_SIZE);
	
	if(v_attr->node_flag != NULL)
		tmp_flag = v_attr->node_flag[src_pos];
	

	if(src_pos < dest_pos)
	{
		for(i = src_pos ;i < dest_pos; i++)	
		{

			DB_get_node_by_pos(v_attr, i+1, &node_id, &node_addr);
			DB_read_node(node_id, node_addr, (UINT8 *)&p_node, sizeof(p_node));
			p_node.user_order--;
			DB_modify_node_by_id(v_attr, node_id, &p_node, sizeof(p_node));
			DB_update_operation();
		//	modify_prog( p_node.prog_id,&p_node );

			MEMCPY(&v_attr->node_buf[i], &v_attr->node_buf[i+1], sizeof(NODE_IDX_PARAM));
			if(v_attr->node_flag != NULL)
				v_attr->node_flag[i] = v_attr->node_flag[i+1];		

		}

	}
	else
	{
		for(i = src_pos;i > dest_pos; i--)	
		{

			DB_get_node_by_pos(v_attr, i-1, &node_id, &node_addr);
			DB_read_node(node_id, node_addr, (UINT8 *)&p_node, sizeof(p_node));
			p_node.user_order++;
			DB_modify_node_by_id(v_attr, node_id, &p_node, sizeof(p_node));
			DB_update_operation();
			//modify_prog( p_node.prog_id,&p_node );
			MEMCPY(&v_attr->node_buf[i], &v_attr->node_buf[i-1], sizeof(NODE_IDX_PARAM));
			if(v_attr->node_flag != NULL)
				v_attr->node_flag[i] = v_attr->node_flag[i-1];
		}

	}

	//DB_get_node_by_pos(v_attr, src_pos, &snode_id, &snode_addr);
	//DB_read_node(snode_id, snode_addr, (UINT8 *)&sp_node, sizeof(sp_node));
	//sp_node.user_order += dest_pos - src_pos;
	
//DB_get_node_by_id(v_attr, tmp_id,  &tmp_addr);

	MEMCPY(&v_attr->node_buf[dest_pos].node_id[0], &tmp_id, NODE_ID_SIZE);
	MEMCPY(&v_attr->node_buf[dest_pos].node_addr[0], &tmp_addr, NODE_ADDR_SIZE);

	DB_get_node_by_pos(v_attr, dest_pos, &snode_id, &snode_addr);
	DB_read_node(snode_id, snode_addr, (UINT8 *)&sp_node, sizeof(sp_node));
	
	if(v_attr->node_buf != NULL)
		 v_attr->node_flag[dest_pos] = tmp_flag;
	
	db_table[v_attr->cur_type].node_moved = 1;
	return result;


}
INT32 get_node_by_pro_num(DB_VIEW *v_attr, UINT32 pro_num, P_NODE * node)
{
	INT32 ret = DB_SUCCES;
	UINT32 i;
	UINT32 node_id = 0;
	UINT32 node_addr = 0;

//	DB_ENTER_MUTEX();

	if( TYPE_PROG_NODE == DB_get_cur_view_type(v_attr) )
	{	
		for(i = 0; i < v_attr->node_num; i++)
		{

			MEMCPY((UINT8 *)&node_addr, v_attr->node_buf[i].node_addr, NODE_ADDR_SIZE);
			MEMCPY((UINT8 *)&node_id, v_attr->node_buf[i].node_id, NODE_ID_SIZE);
			ret = DB_read_node( node_id, node_addr,(UINT8 *)node, sizeof(P_NODE));
			if( node->user_order == pro_num)
				break;

		}
	}
	else
	{
		for(i = 0; i < db_table[TYPE_PROG_NODE].node_num; i++)
		{
			MEMCPY((UINT8 *)&node_addr, db_table[TYPE_PROG_NODE].table_buf[i].node_addr, NODE_ADDR_SIZE);
			MEMCPY((UINT8 *)&node_id, db_table[TYPE_PROG_NODE].table_buf[i].node_id, NODE_ID_SIZE);
			ret = DB_read_node( node_id, node_addr , (UINT8 *)node, sizeof(P_NODE));
			if( node->user_order == pro_num)
				break;
		}
	}
//	DB_RELEASE_MUTEX();
	return ret;
}

#endif

