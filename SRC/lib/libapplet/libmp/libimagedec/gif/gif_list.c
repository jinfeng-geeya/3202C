/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2008 Copyright (C)
 *  (C)
 *  File: gif_list.c
 *  (I)
 *  Description: list managment
 *  (S)
 *  History:(M)
 *      	Date        			Author         	Comment
 *      	====        			======		=======
 * 0.		2008.1.16			Sam		Create
 ****************************************************************************/
#include <sys_config.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <os/tds2/itron.h>
#include <api/libc/printf.h>
#include <osal/osal_timer.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <api/libmp/gif.h>
#include "gif_main.h"
#include "gif_list.h"
#include "gif_debug.h"

GIF_INLINE void list_create(struct gif_list *header,struct gif_list_unit *unit)
{
	struct gif_list_unit *list = unit;
	UINT8 i = 0;

	unit->id = 0;
	header->head = header->tail = unit++;
	for(i = 0;i < header->size - 1;i++)
	{
		unit->id = i + 1;
		list->next = unit;
		unit->before = list;
		list++;
		unit++;
	}
	unit = header->head;
	list->next = unit;
	unit->before = list;
}

GIF_INLINE void list_free(struct gif_list *header)
{
	if(0 == header->type)
	{
		struct gif_list_unit *punit = NULL;
		struct gif_list_unit *tmp = NULL;

		tmp = punit = header->head;
		do
		{
			tmp = tmp->next;
			if(tmp == header->head)
				break;
			if((UINT32)tmp < (UINT32)punit)
				punit = tmp;
		}while(1);

		if(NULL != punit)
			FREE(punit);
	}
		
	header->tail = header->head = NULL;
}

GIF_INLINE GIF_RET list_check_free(struct gif_list *list)
{
	return (list->head == list->tail && (1 == list->head->busy)) ? GIF_FALSE:GIF_TRUE;
}

void gif_list_enter_mutex(void)
{
	osal_task_dispatch_off();
}

void gif_list_exit_mutex(void)
{
	osal_task_dispatch_on();
}

GIF_RET gif_list_add_unit(struct gif_list *list,struct gif_list_unit *unit)
{
	struct gif_list_unit *ptail = list->tail;
	
	if(list_check_free(list))
	{
		MEMCPY((void *)&(ptail->text),(void *)&(unit->text),sizeof(struct gif_context));
		ptail->magic_num = unit->magic_num;
		ptail->busy = 1;
		ptail->slice = 0;
		list->tail = ptail->next;
		GIF_PRINTF("add unit in list %d magic %x\n",list->type,unit->magic_num);
		return GIF_TRUE;
	}	
	return GIF_FALSE;
}

// delete unit in the alive list
// punit1 --> to be deleted unit. punit2 --> the successfully deleted unit pointer
GIF_RET gif_list_delete_unit(struct gif_list *list,struct gif_list_unit *punit1,struct gif_list_unit **punit2)
{
	struct gif_list_unit *punit = list->head;
	struct gif_list_unit *ptail = list->tail;

	if(punit == ptail)
	{
		if(0 == punit->busy)
		{
			GIF_PRINTF("NULL in the list in delete magic %x\n",punit1->magic_num);
			return GIF_FALSE;
		}
		else
			punit = punit->next;
	}
	while(punit->magic_num != punit1->magic_num && (1 == punit->busy))
	{
		if(punit == list->tail)
		{
			GIF_PRINTF("Can't find the unit need to be deleted magic %x\n",punit1->magic_num);
			return GIF_FALSE;
		}
		punit = punit->next;
	}
	
	if(punit == list->head)
		list->head = punit->next;

	punit->busy = 0;	
	if(list->head != list->tail)
	{
		punit->next->before = punit->before;
		punit->before->next = punit->next;

		// choose the deleted unit as the list tail
		punit->next = ptail;
		punit->before = list->tail->before;
		ptail->before->next = punit;
		ptail->before = punit;
		list->tail = punit;
	}
	GIF_PRINTF("Delete the unit magic %x in list %d\n",punit->magic_num,list->type);
	*punit2 = punit;
	return GIF_TRUE;
}

struct gif_list_unit *gif_list_fetch_unit(struct gif_list *list)
{
	struct gif_list_unit *phead = list->head;

	if(1 == phead->busy)
	{
		phead->busy = 0;
		list->head = phead->next;
		GIF_PRINTF("Fetch unit in list %d\n",list->type);
		return phead;
	}
	return NULL;
}

// swith the unit in the alive list
// if no active unit, return NULL. when next unit is also active, move the head to the next unit
struct gif_list_unit *gif_list_next_unit(struct gif_list *list)
{
	struct gif_list_unit *phead = list->head;
	struct gif_list_unit *ptail = list->tail;

	if(1 == phead->busy)
	{
		if(phead->next != ptail)
		{
			list->head = phead->next;
			phead->before->next = phead->next;
			phead->next->before = phead->before;
			phead->before = ptail->before;
			phead->next = ptail;
			ptail->before->next = phead;
			ptail->before = phead;
		}
		return phead;
	}
	GIF_PRINTF("Next unit is NULL\n");
	return NULL;
}

GIF_RET gif_list_check_unit(struct gif_list *list,struct gif_list_unit *unit)
{
	struct gif_list_unit *punit = list->head;

	// check whether alive list is full
	if(!list_check_free(list))
		return GIF_FALSE;

	while(1 == punit->busy)
	{
		if(punit->magic_num == unit->magic_num)
		{
			GIF_PRINTF("file %d is already in the alive list\n",unit->magic_num);
			return GIF_FALSE;
		}
		punit = punit->next;
	}
	return GIF_TRUE;
}

GIF_RET gif_list_init(struct gif_main *main)
{
	struct gif_list_unit *punit = NULL;
	int i = 0;
	
	if(NULL != main)
	{
		punit = (struct gif_list_unit *)MALLOC(GIF_LIST_SIZE * GIF_LIST_NUM * sizeof(struct gif_list_unit));
		MEMSET((void *)punit,0,GIF_LIST_SIZE * GIF_LIST_NUM * sizeof(struct gif_list_unit));
		main->alive.size = main->wait.size = main->dead.size = GIF_LIST_SIZE;
		main->alive.type = 0;
		main->wait.type = 1;
		main->dead.type = 2;
		list_create(&main->alive,punit);
		punit += GIF_LIST_SIZE;
		list_create(&main->wait,punit);
		punit += GIF_LIST_SIZE;
		list_create(&main->dead,punit);
		return GIF_TRUE;
	}
	return GIF_FALSE;
}

void gif_list_free(struct gif_main *main)
{
	if(NULL != main)
	{
		list_free(&main->alive);
		list_free(&main->wait);
		list_free(&main->dead);
	}
}

