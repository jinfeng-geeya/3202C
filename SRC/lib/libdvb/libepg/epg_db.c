/*========================================================================================
 *
 * Copyright (C) 2011 ALi Shanghai Corp.
 *
 * Description:
 *
 * History
 *
 *   Date	    Author		Comment
 *   ========	===========	========================================
 *   ......
 *   2011/7/5    Steven     Re-arrange print message.
 *
 *=========================================================================================*/

#ifndef WIN32
#include <retcode.h>
#include <osal/osal.h>
#include <api/libc/string.h>
#include "epg_common.h"
#define INLINE			inline
#endif
#include <api/libsi/lib_epg.h>
#include "epg_db.h"
#include "x_alloc.h"

#define DB_MALLOC		x_alloc
#define DB_FREE			x_free

#define INVALID_POS		(-1)

typedef INT32 (*node_op_t)(struct DB_NODE *, void *);

//db control struct
static struct DB_CONTROL
{
	OSAL_ID flag;

	struct DB_NODE *node;
	int max_count;
	int index;		//last one index
	int count;		//NODE_ACTIVE count

} db_block;

//struct view
static struct EPG_DB_VIEW
{
	UINT32 *node_addr;	//node addr
	UINT32 num;
	UINT32 max_num;

	struct DB_CHECK *check;

} epg_view;

static void epgdb_enter_mutex()
{
	UINT32 flag;
	osal_flag_wait(&flag, db_block.flag, EPGDB_FLAG_MUTEX, 
				OSAL_TWF_ANDW|OSAL_TWF_CLR, OSAL_WAIT_FOREVER_TIME);
}
static void epgdb_leave_mutex()
{
	osal_flag_set(db_block.flag, EPGDB_FLAG_MUTEX);
}

/*
 * init the epg db
 * buffer: IN, db node table.
 */
INT32 init_epg_db(OSAL_ID flag, UINT8 *buffer, UINT32 buf_len)
{
	if (buffer == NULL || buf_len == 0)
	{
		DB_PRINT("%s: buffer is NULL!\n",__FUNCTION__);
		return ERR_FAILED;
	}

	MEMSET(buffer, 0, buf_len);
	db_block.node = (struct DB_NODE*)buffer;
	db_block.max_count = buf_len / sizeof(struct DB_NODE);
	db_block.index = 0;
	db_block.count = 0;
	DB_PRINT("[epg db] buffer: 0x%X, len: %d, sizeof DB_NODE: %d, max count: %d\n",
				buffer, buf_len, sizeof(struct DB_NODE), db_block.max_count);

	if (flag == OSAL_INVALID_ID)
	{
		DB_PRINT("%s: flag ivalid!\n",__FUNCTION__);
		return ERR_FAILED;
	}
	db_block.flag = flag;

	osal_flag_set(db_block.flag, EPGDB_FLAG_MUTEX);

	return SUCCESS;
}

//reset the epg db
INT32 reset_epg_db()
{
	DB_PRINT("[epg db] reset\n");

	epgdb_enter_mutex();

	if (db_block.node != NULL)
	{
		MEMSET(db_block.node, 0, db_block.max_count * sizeof(struct DB_NODE));
		db_block.index = 0;
		db_block.count = 0;
	}

	MEMSET(&epg_view, 0, sizeof(epg_view));	//reset view

	epgdb_leave_mutex();

	return SUCCESS;
}

//release the epg db
INT32 release_epg_db()
{
	DB_PRINT("[epg db] release\n");

	epgdb_enter_mutex();
	
	if (db_block.node != NULL)
	{
		db_block.node = NULL;
		db_block.max_count = 0;
		db_block.index = 0;
		db_block.count = 0;
	}

	epgdb_leave_mutex();

	return SUCCESS;
}

INT32 get_epg_db_status()
{
#ifdef EPG_TEST_CASE_DB_FULL
	if (db_block.count >= MAX_TP_EVENT_COUNT)
	{
		DB_PRINT("[epg db] %s: test case - db full, not available\n",__FUNCTION__);
		return STATUS_UNAVAILABLE;
	}
#endif
	if (db_block.count < db_block.max_count)
	{
		MSG_PRINT("[epg db] available\n");
		return STATUS_AVAILABLE;
	}
	else
	{
		DB_PRINT("[epg db] full, not available\n");
		return STATUS_UNAVAILABLE;
	}
}

//get node id
static INLINE UINT32 _get_node_id(struct DB_NODE *node)
{
//	return (UINT64)((node->tp_id << 32) | (node->service_id << 16) | node->event_id);
//	return (UINT32)( (node->service_id << 16) | node->event_id);
//	return ((node->mjd_num<<16) | (node->time.hour<<8) | (node->time.minute));
//	return ((node->mjd_num<<11) | (node->time.hour<<6) | (node->time.minute));
	return ((node->mjd_num<<17) | (node->time.hour<<12) | (node->time.minute<<6) | (node->time.second));
//	return *node->id;
}

//if dst is FREE or DIRTY, copy src to dst
static INT32 _node_cpy(struct DB_NODE *dst, struct DB_NODE *src)
{
	UINT8 i;
	INT32 ret;
	if (dst->status != NODE_ACTIVE)	//free or dirty
	{
		MEMCPY(dst, src, sizeof(struct DB_NODE));
		for(i=0;i<MAX_EPG_LANGUAGE_COUNT;i++)
		{
			dst->lang[i].text_length = 0;
			dst->lang[i].text_char = NULL;
		}
		dst->status = NODE_ACTIVE;
		db_block.count ++;
		MSG_PRINT("[epg db] %s: count %d\n",__FUNCTION__,db_block.count);

		ret = SUCCESS;
		for (i=0; i<MAX_EPG_LANGUAGE_COUNT; i++)
		{
			if (src->lang[i].text_length > 0)
			{
				dst->lang[i].text_char = (UINT8*)DB_MALLOC(src->lang[i].text_length);
				if (dst->lang[i].text_char == NULL)
				{
					ret = ERR_NO_MEM;
					ERR_PRINT("[epg db] %s: no mem!\n",__FUNCTION__);
					//break ???
					//break;
				}
				else
				{
					MEMCPY(dst->lang[i].text_char, src->lang[i].text_char, src->lang[i].text_length);
					dst->lang[i].text_length = src->lang[i].text_length;
				}
			}
		}

//still keep node
#if 0
		if(ret==ERR_NO_MEM)
		{
			for(i=0;i<MAX_EPG_LANGUAGE_COUNT;i++)
			{
				if(dst->lang[i].text_char!=NULL)
				{
					DB_FREE(dst->lang[i].text_char);
					dst->lang[i].text_char = NULL;
					dst->lang[i].text_length = 0;
				}
			}
			dst->status = NODE_DIRTY;
			db_block.count --;
		}
#endif

		//add 2 view???
//		_add2view(dst, &epg_view);

		return ret;
	}
	else
	{
		ERR_PRINT("[epg db] %s: dst is active, can not copy!\n",__FUNCTION__);
	}

	return ERR_FAILED;
}

//±éÀú & do something
INT32 _node_op(node_op_t op, void *priv)
{
	int i;

	for (i=0; i<db_block.index; i++)
	{
		if (op(&db_block.node[i], priv) == SUCCESS)
			return SUCCESS;
	}

	return ERR_FAILED;
}

//add a node to db
//node: IN, external node, will be copied to new node of the db
static INT32 _add_node(struct DB_NODE *node)
{
	INT32 ret,i;
	static INT32 next = -1;	//next dirty index
	ret = ERR_FAILED;

#ifdef EPG_TEST_CASE_DB_FULL
	if (db_block.count >= MAX_TP_EVENT_COUNT)
	{
		DB_PRINT("[epg db] %s: test case - epg db fulll\n",__FUNCTION__);
		return ERR_DB_FULL;
	}
#endif

	//first add to tail
	if (db_block.index < db_block.max_count)
	{
		MSG_PRINT("[epg db] %s: index %d\n",__FUNCTION__,db_block.index);

		struct DB_NODE *dst = &db_block.node[db_block.index];
		MEMCPY(dst, node, sizeof(struct DB_NODE));
		dst->status = NODE_ACTIVE;
		for(i=0;i<MAX_EPG_LANGUAGE_COUNT;i++)
		{
			dst->lang[i].text_length = 0;
			dst->lang[i].text_char = NULL;
		}
		db_block.index ++;
		db_block.count ++;
		MSG_PRINT("[epg db] %s: count %d\n",__FUNCTION__,db_block.count);

		if ((db_block.count % 100) == 0)
		{
			DB_PRINT("[epg db] %s: count %d (@ tick %d)\n",__FUNCTION__,db_block.count,osal_get_tick());
		}

		ret = SUCCESS;
		for (i=0; i<MAX_EPG_LANGUAGE_COUNT; i++)
		{
			if (node->lang[i].text_length > 0)
			{
#ifdef EPG_TEST_CASE_NO_MEM
				if (db_block.index >= 100)
				{
					DB_PRINT("[epg db] %s: test case - epg no mem\n",__FUNCTION__);
					dst->lang[i].text_char = NULL;
				}
				else
#endif
				{
					dst->lang[i].text_char = (UINT8*)DB_MALLOC(node->lang[i].text_length);
				}

				if (dst->lang[i].text_char == NULL)
				{
					ret = ERR_NO_MEM;
					if ((db_block.count % 10) == 0)
						ERR_PRINT("[epg db] %s: no mem!\n",__FUNCTION__);
					//break ???
					//break;
				}
				else
				{
					MEMCPY(dst->lang[i].text_char, node->lang[i].text_char, node->lang[i].text_length);
					dst->lang[i].text_length = node->lang[i].text_length;
				}
			}
		}
//still keep this node
#if 0
		if(ret==ERR_NO_MEM)
		{	
			MTP_PRINT("free\n");
			for(i=0;i<MAX_EPG_LANGUAGE_COUNT;i++)
			{
				if(dst->lang[i].text_char!=NULL)
				{	
					DB_FREE(dst->lang[i].text_char);
					dst->lang[i].text_char = NULL;
					dst->lang[i].text_length = 0;
				}
			}
			dst->status = NODE_DIRTY;
			db_block.index --;
			db_block.count --;
			MTP_PRINT("done\n");
		}
		else
#endif

		//add 2 view???
//		_add2view(dst, &epg_view);
		return ret;
	}
	else	//else copy to some dirty one
	{
		if (db_block.count >= db_block.max_count)
		{
			ERR_PRINT("[epg db] %s: DB is full!!!\n",__FUNCTION__);
			ret = ERR_DB_FULL;
			return ret;
		}

		//ret =  _node_op((node_op_t)_node_cpy, node);
		for(i=0;i<db_block.index;i++)
		{
			//record last index, check from this index next time,
			//no need loop the table!
			next ++;
			if (next >= db_block.index)
				next -= db_block.index;

			if(db_block.node[next].status!=NODE_ACTIVE)
			{
				MSG_PRINT("[epg db] %s: add to index %d\n",__FUNCTION__,next);

				ret = _node_cpy(&db_block.node[next],node);

				if(ret==SUCCESS||ret==ERR_NO_MEM)
					break;
			}
		}

		if (i >= db_block.index)
		{
			ERR_PRINT("[epg db] %s: DB is full!!!\n",__FUNCTION__);
			ret = ERR_DB_FULL;
		}
		return ret;
	}
}

//del a node of the db
static INT32 __del_node(struct DB_NODE *node)
{
	UINT8 i;
	
	MSG_PRINT("[epg db] %s: del service 0x%X - event 0x%X)\n",__FUNCTION__,node->service_id,node->event_id);
	for (i=0; i<MAX_EPG_LANGUAGE_COUNT; i++)
	{
		if (node->lang[i].text_char != NULL)
		{
			DB_FREE(node->lang[i].text_char);
			node->lang[i].text_char = NULL;
		}
		node->lang[i].text_length = 0;
	}

	node->status = NODE_DIRTY;

	db_block.count --;

	return SUCCESS;
}

//if the node fit the condition, del it.
static INT32 _del_node(struct DB_NODE *node, struct DB_CHECK *check)
{
	UINT32 *count;

	if (node->status == NODE_ACTIVE)
	{
		if (check != NULL
			&& check->check_func != NULL
			&& check->check_func(node, check->priv))
		{
			//del from view???
//			_del_from_view(node, &epg_view);
			__del_node(node);

			count = ((UINT32*)check->priv)+3;
			(*count) ++;
		}
	}

	return !SUCCESS;
}

/*
 * create a node from external node, and add to db
 * node: IN, external node created outside.
 */
INT32 add_event(struct DB_NODE *node)
{
	INT32 ret;
	
	epgdb_enter_mutex();
	
	ret = _add_node(node);

	epgdb_leave_mutex();

	return ret;
}

//check if del this node
//service_id: 0xFFFF, means del all service event of this tp
//			  0x0000, means del all other tp's event.
static BOOL check_del(struct DB_NODE *node, void *priv)
{
	UINT32 *value = (UINT32*)priv;
	UINT32 tp_id = value[0];
	UINT16 service_id = value[1] >> 16;
	UINT8 event_mask = value[1] & 0xFF;
	UINT8 tab_num = value[2];

#ifdef EPG_MULTI_TP
	if (service_id == 0xFFFF)	//means del all events of this tp
	{
		if (node->tp_id == tp_id)
			return TRUE;
		else
			return FALSE;
	}
	else if (service_id == 0)	//means del all events of not this tp
	{
		if (node->tp_id != tp_id)
			return TRUE;
		else
			return FALSE;
	}
#endif

	if (
		node->status==NODE_ACTIVE &&
#ifdef EPG_MULTI_TP
		node->tp_id == tp_id &&
#endif
		node->service_id == service_id
		&& node->tab_num == tab_num
		&& ((0x01<<node->event_type)&event_mask))
		return TRUE;
	else
		return FALSE;
}

/*
 * delete nodes of some events
 */
INT32 del_events(UINT32 tp_id, UINT16 service_id, UINT8 tab_num, UINT8 event_mask)
{
	struct DB_CHECK del_check;
	UINT32 value[4];

	DB_PRINT("[epg db] %s: del tp 0x%X, service 0x%X, table %d, event mask 0x%X\n",__FUNCTION__,tp_id,service_id,tab_num,event_mask);

	del_check.check_func = check_del;
	value[0] = tp_id;
	value[1] = (service_id<<16)|event_mask;
	value[2] = tab_num;
	value[3] = 0;	//del count
	del_check.priv = (void*)value;

	epgdb_enter_mutex();

	_node_op((node_op_t)_del_node, &del_check);

	epgdb_leave_mutex();

	DB_PRINT("[epg db] %s: del %d events, left %d events\n",__FUNCTION__,value[3],db_block.count);

	return value[3];
}


///////////////////////////////////////////////////////////////////////////

//insert 1 node into the view
static BOOL _insert2view(struct DB_NODE* node, struct EPG_DB_VIEW *view)
{
	struct DB_NODE* first_node;
	struct DB_NODE* last_node;
	struct DB_NODE* mid_node;
	UINT32 m, n, mid;
	UINT32 node_id = _get_node_id(node);

	if (view->num == 0)
	{
		view->node_addr[0] = (UINT32)node;
		return TRUE;
	}
		
	first_node = (struct DB_NODE*)view->node_addr[0];
	last_node = (struct DB_NODE*)view->node_addr[view->num-1];

	if (node_id == _get_node_id(first_node))	//already exists
	{
		//del node?
//		__del_node(node);
		if (
#ifdef EPG_MULTI_TP
            node->tp_id == first_node->tp_id && 
#endif
            node->service_id == first_node->service_id)
			return FALSE;
	}

	if (node_id == _get_node_id(last_node))	//already exists
	{
		if (
#ifdef EPG_MULTI_TP
            node->tp_id == last_node->tp_id && 
#endif
            node->service_id == last_node->service_id)
			return FALSE;
	}

	if (node_id <= _get_node_id(first_node))
	{
		//0 move to 1
		MEMMOVE(view->node_addr + 1, view->node_addr, 4*view->num);
		view->node_addr[0] = (UINT32)node;
	}
	else if (node_id >= _get_node_id(last_node))
	{
		view->node_addr[view->num] = (UINT32)node;
	}
	else
	{
		m = 0;
		n = view->num-1;
		do
		{
			if (m + 1 == n)
			{
				//n move to n+1
				MEMMOVE(view->node_addr + n + 1, view->node_addr + n, 4*(view->num - n));
				view->node_addr[n] = (UINT32)node;
				return TRUE;
			}

			mid = (m + n) / 2;
			mid_node = (struct DB_NODE*)view->node_addr[mid];

			if (node_id == _get_node_id(mid_node))	//already exists
			{
//				__del_node(node);
				if (
#ifdef EPG_MULTI_TP
                    node->tp_id == mid_node->tp_id && 
#endif
                    node->service_id == mid_node->service_id)
					return FALSE;
			}

			if (node_id < _get_node_id(mid_node))
				n = mid;
			else
				m = mid;

		} while (1);
	}

	return TRUE;
}

//add 1 node into the view(sorted)
static INT32 _add2view(struct DB_NODE* node, struct EPG_DB_VIEW *view)
{
	if (view->num < view->max_num
		&& view->check != NULL
		&& view->check->check_func != NULL
		&& view->check->check_func(node, view->check->priv))
	{
		if (_insert2view(node, view))
		{
			MSG_PRINT("[epg db] %s: add 0x%X\n",__FUNCTION__,node->event_id);
			view->num ++;
		}
	}

	return !SUCCESS;
}

/*
 * create a view of some events.
 * view_buf: OUT, buffer to store node addr
 * buf_size: IN
 * check: IN, condition function & data struct
 * return value: node count in the view
 */
INT32 create_view(UINT32 *view_buf, UINT32 buf_size, struct DB_CHECK *check)
{
	epgdb_enter_mutex();

	epg_view.node_addr = view_buf;
	epg_view.max_num = buf_size / 4;
	epg_view.num = 0;
	epg_view.check = check;

	_node_op((node_op_t)_add2view, &epg_view);

	epgdb_leave_mutex();

	DB_PRINT("[epg db] %s: epg event view count %d\n",__FUNCTION__,epg_view.num);

	return epg_view.num;
}


//reset the epg view
INT32 reset_epg_view()
{
	DB_PRINT("[epg db] %s\n",__FUNCTION__);

	epgdb_enter_mutex();

	MEMSET(&epg_view, 0, sizeof(epg_view));	//reset view

	epgdb_leave_mutex();

	return SUCCESS;
}

//get one node of the current view by position
//buf: IN, buffer for event name/text char
INT32 get_event_by_pos(UINT32 pos, struct DB_NODE* dst, UINT8* buf, UINT16 size)
{
	struct DB_NODE* src;
	//UINT8 text_char[11] = {7, 'N', 'o', ' ', 'N', 'a', 'm', 'e', 0, 0, 0};	//default text char

	MSG_PRINT("[epg db] %s: pos %d\n",__FUNCTION__,pos);

	epgdb_enter_mutex();

	if (pos >= epg_view.num || dst == NULL || buf == NULL)
	{
		DB_PRINT("%s: parameter error!\n",__FUNCTION__);
		epgdb_leave_mutex();
		return ERR_PARA;
	}

	src = (struct DB_NODE*)epg_view.node_addr[pos];

	//copy src to dest
	MEMCPY(dst, src, sizeof(struct DB_NODE));
	UINT8 i;
	for (i=0; i<MAX_EPG_LANGUAGE_COUNT; i++)
	{
		if (size >= src->lang[i].text_length && src->lang[i].text_length > 0)
		{
			dst->lang[i].text_char = buf;
			MEMCPY(dst->lang[i].text_char, src->lang[i].text_char, dst->lang[i].text_length);

			buf += dst->lang[i].text_length;
			size -= dst->lang[i].text_length;
		}
		else
		{
			/*if (size >= 11
				&& (i == 0 || src->lang[i].text_length > 0))
			{
				dst->lang[i].text_length = 11;
				dst->lang[i].text_char = buf;
				MEMCPY(dst->lang[i].text_char, text_char, 11);
			}
			else*/
			{
				dst->lang[i].text_length = 0;
				dst->lang[i].text_char = NULL;

				if (src->lang[i].text_length > 0)
				{
					DB_PRINT("%s: text char buffer(%d) not enough(%d)!\n", __FUNCTION__, size, src->lang[i].text_length);
				}
			}
		}
	}

	epgdb_leave_mutex();

	return SUCCESS;
}

