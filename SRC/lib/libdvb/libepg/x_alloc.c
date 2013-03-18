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
 *   2011/7/6    Steven     free_list按地址大小排序，利于defrag.
 *
 *=========================================================================================*/

#ifndef WIN32
#include <api/libc/string.h>
#include "epg_common.h"
#define INLINE			inline
#endif
#include "x_alloc.h"

#define X_ALLOC_DEBUG_LEVEL	0

#define MEM_ALIGN_SIZE(size) (((size) + 3) & 0xFFFFFFFC)

//content node for a section of EIT data
struct content_t
{
	struct content_t *next;
	UINT32 len;		//content buf len(shall >= section data len)
	UINT8  addr[0];	//section buf addr
};

#define GARBAGE_LEN_THRESHOLD		20

/*
 * alloc a content, it includes both content_struct and section_data area.
 */
static struct content_t *alloc_content(struct content_t **head, UINT32 len)
{
	struct content_t *pre = NULL;
	struct content_t *new_node = NULL;
	struct content_t *pre_new_node = NULL;
	struct content_t *node = *head;
	UINT32 len_limit1 = len + GARBAGE_LEN_THRESHOLD;

	UINT32 size = MEM_ALIGN_SIZE(len + sizeof(struct content_t));
	UINT32 len_limit2 = size + 2 * GARBAGE_LEN_THRESHOLD;

	X_PRINT("[epg alloc] alloc from the free list method 1!\n");

	while (node != NULL)
	{
		//check if wihin [len, len + threshold]
		if (node->len >= len)
		{
			if (node->len <= len_limit1)
			{
				//del the node from the list
				if (pre == NULL)	//head one
					*head = node->next;
				else
					pre->next = node->next;
				
				node->next = NULL;
				return node;
			}
			else if (new_node == NULL || node->len < new_node->len)	//get the shortest one
			{
				new_node = node;
				pre_new_node = pre;
			}
		}

		pre = node;
		node = node->next;
	}

	//found one > len + threshold
	if (new_node != NULL)
	{
		// >= len + 2*threshold, divide it
		if (new_node->len >= len_limit2)
		{
			node = new_node;

			node->len -= size;
			new_node = (struct content_t*)(node->addr + node->len);
			new_node->len = size - sizeof(struct content_t);
			new_node->next = NULL;
			return new_node;
		}
		else	//(len + threshold, len + 2*threshold)
		{
			//del the node from the list
			if (pre_new_node == NULL)	//head one
				*head = new_node->next;
			else
				pre_new_node->next = new_node->next;
			
			new_node->next = NULL;
			return new_node;
		}
	}

	X_PRINT("[epg alloc] alloc a content node failed!\n");
	return NULL;
}

/*
 * add the content node to the list
 */
static void add_content_list(struct content_t **head, struct content_t *node)
{
#if 0
	//add to head
	node->next = (*head);
	*head = node;
#else
	struct content_t *prev, *next;

	X_PRINT("%s: node 0x%X, head 0x%X\n",__FUNCTION__,node,*head);

	if (*head == NULL)	//20110708, fix *head=NULL, system crash issue.
	{
		*head = node;
		return;
	}

	//按地址大小顺序insert list
	if ((UINT32)node < (UINT32)(*head))
	{
		node->next = (*head);
		*head = node;
		return;
	}

	prev = *head;
	next = (*head)->next;
	X_PRINT("%s: next 0x%X\n",__FUNCTION__,next);

	while (next != NULL)
	{
		if ((UINT32)node < (UINT32)next)
		{
			node->next = next;
			prev->next = node;
			return;
		}

		prev = next;
		next = prev->next;
		X_PRINT("%s: next 0x%X\n",__FUNCTION__,next);
	}

	//last one
	//node->next = next;
	prev->next = node;
#endif
}

/*
 * defrag a content node with the free list
 */
static INT32 defrag_content(struct content_t **head, struct content_t *node)
{
	struct content_t *list = *head;
	struct content_t *pre = NULL;
	struct content_t *next;
	UINT32 addr;
	
	if (list == NULL || node == NULL)
		return ERR_FAILED;

	addr = (UINT32)(node->addr + node->len);
	while (list != NULL)
	{
		if ((UINT32)(list->addr + list->len) == (UINT32)node)
		{
			// list <-- node
			list->len += (sizeof(struct content_t) + node->len);
			X_PRINT("[epg alloc] list: 0x%X <-- node: 0x%X\n",list, node);

			//check list & list->next(!=NULL), 20110705
			next = list->next;
			if ((UINT32)(list->addr + list->len) == (UINT32)(next))
			{
				// list <-- (list->next)
				X_PRINT("[epg alloc] list: 0x%X <-- list->next 0x%X\n",list, next);
				list->len += (sizeof(struct content_t) + next->len);
				list->next = next->next;
			}

			return SUCCESS;
		}
		else if (addr == (UINT32)list)
		{
			// node <-- list
			node->len += (sizeof(struct content_t) + list->len);
			node->next = list->next;
			if (pre == NULL)
				*head = node;
			else
				pre->next = node;

			X_PRINT("[epg alloc] node: 0x%X <-- list: 0x%X\n",node, list);
			return SUCCESS;
		}

		pre = list;
		list = list->next;
	}

	X_PRINT("[epg alloc] defrag failed!\nnode: 0x%X\n", node);
	return ERR_FAILED;
}

/*
 * defrag the free list
 */
static void free_content(struct content_t **head, struct content_t *node)
{
	node->next = NULL;

	if (SUCCESS != defrag_content(head, node))
		add_content_list(head, node);
}

static struct ALLOC_OBJ
{
	OSAL_ID flag;

	UINT8* buffer;
	UINT32 buf_len;
	struct content_t *free_list;

	INT32  status;

} alloc_obj;

static void alloc_enter_mutex()
{
	UINT32 flag;
	osal_flag_wait(&flag, alloc_obj.flag, ALLOC_FLAG_MUTEX, 
				OSAL_TWF_ANDW|OSAL_TWF_CLR, OSAL_WAIT_FOREVER_TIME);
}
static void alloc_leave_mutex()
{
	osal_flag_set(alloc_obj.flag, ALLOC_FLAG_MUTEX);
}
#if X_ALLOC_DEBUG_LEVEL > 0
void print_freemem_size()
{
	UINT32 db_cnt = 0;
	UINT32 db_max = 0;
	struct content_t *list;
	list = alloc_obj.free_list;
	while (list != NULL) {
		db_cnt += list->len;
		if (list->len > db_max)
			db_max = list->len;
		list = list->next;
	}
	libc_printf("FreeMem: %d, Mxnode: %d\n", db_cnt, db_max);
}
#endif

void *x_alloc(UINT32 size)
{
	struct content_t *node;

	alloc_enter_mutex();
	
	node = alloc_content(&alloc_obj.free_list, size);

	alloc_leave_mutex();
#if (X_ALLOC_DEBUG_LEVEL > 0)
	print_freemem_size();
#endif

	if (node == NULL)
	{
		alloc_obj.status = STATUS_UNAVAILABLE;
		return NULL;
	}

	return (void*)((UINT32)node+sizeof(struct content_t));
}

INT32 x_free(void *addr)
{
	struct content_t *node;

	if (addr == NULL)
		return -1;

	alloc_enter_mutex();

	node = (struct content_t*)((UINT32)addr-sizeof(struct content_t));

	free_content(&alloc_obj.free_list, node);

	alloc_obj.status = STATUS_AVAILABLE;
	alloc_leave_mutex();

#if (X_ALLOC_DEBUG_LEVEL > 0)
	print_freemem_size();
#endif
	return 0;
}

INT32 x_init_mem(OSAL_ID flag, UINT8 *buffer, UINT32 buf_len)
{
	if (alloc_obj.free_list != NULL
		|| flag == OSAL_INVALID_ID
		|| buffer == NULL || buf_len < sizeof(struct content_t))
	{
		X_PRINT("%s: parameter error!\n",__FUNCTION__);
		return ERR_FAILED;
	}

	alloc_obj.flag = flag;
	osal_flag_set(alloc_obj.flag, ALLOC_FLAG_MUTEX);

	alloc_obj.buffer = buffer;
	alloc_obj.buf_len = buf_len;
	alloc_obj.free_list = (struct content_t*)buffer;
	alloc_obj.free_list->next = NULL;
	alloc_obj.free_list->len = buf_len - sizeof(struct content_t);
	alloc_obj.status = STATUS_AVAILABLE;
	X_PRINT("[epg alloc] malloc buffer: 0x%X, len: %d\n",buffer, buf_len);

	return SUCCESS;
}

INT32 x_reset_mem()
{
	alloc_enter_mutex();

	alloc_obj.free_list = (struct content_t*)alloc_obj.buffer;
	alloc_obj.free_list->next = NULL;
	alloc_obj.free_list->len = alloc_obj.buf_len - sizeof(struct content_t);
	alloc_obj.status = STATUS_AVAILABLE;

	alloc_leave_mutex();

	return SUCCESS;
}

INT32 x_release_mem()
{
	alloc_enter_mutex();

	alloc_obj.buffer = NULL;
	alloc_obj.buf_len = 0;
	alloc_obj.free_list = NULL;
	alloc_obj.status = STATUS_UNAVAILABLE;

	alloc_leave_mutex();

	return SUCCESS;
}

INT32 x_get_mem_status()
{
	return alloc_obj.status;
}

