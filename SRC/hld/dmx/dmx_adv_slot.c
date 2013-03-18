/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2010 Copyright (C)
 *
 *  File: dmx_adv_slot.c
 *
 *  Description: This file implement slot wrapped interface of dmx_adv.c.
 *
 *  History:
 *
 *        Date         Author         Version       Comment
 *      ========     =========      ==========     =========
 *  1.  2010.06.01   Trueve Hu        0.1.000       Create this file
 *
 ****************************************************************************/
 
#include <sys_config.h>
#include <types.h>
#include <retcode.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/dmx/dmx_dev.h>


#define DMX_ADV_SLOT_DEBUG		1

#if (DMX_ADV_SLOT_DEBUG > 0)
#define DMX_ADV_SLOT_ERROR		libc_printf
#define DMX_ADV_SLOTERR_DUMP(data,len) { const int l=(len); int i;\
                         for(i=0;i<l;i++) DMX_ADV_SLOT_ERROR(" %02x",*((data)+i)); \
                         DMX_ADV_SLOT_ERROR("\n"); }
#else
#define DMX_ADV_SLOT_ERROR(...)
#define DMX_ADV_SLOTERR_DUMP(...)
#endif

#if (DMX_ADV_SLOT_DEBUG > 1)
#define DMX_ADV_SLOT_PRINTF		libc_printf
#define DMX_ADV_SLOT_DUMP(data,len) { const int l=(len); int i;\
                         for(i=0;i<l;i++) DMX_ADV_SLOT_PRINTF(" %02x",*((data)+i)); \
                         DMX_ADV_SLOT_PRINTF("\n"); }
#else
#define DMX_ADV_SLOT_PRINTF(...)
#define DMX_ADV_SLOT_DUMP(...)
#endif


#define MAX_MV_LEN			16
#define MAX_SLOT_NUM		64
#define MAX_SLOT_CB_NUM		8

typedef struct dmx_adv_slot_t
{
	UINT32 dmx_id;
	UINT32 ch_id;
	UINT32 flt_id;
	INT32 slot_id;
	UINT16 pid;
	UINT16 mv_len;
	UINT8 mask[MAX_MV_LEN];
	UINT8 value[MAX_MV_LEN];
	slot_sec_cb callback[MAX_SLOT_CB_NUM];
}dmx_adv_slot;

static dmx_adv_slot dmx_adv_slots[MAX_SLOT_NUM];

static BOOL dmx_adv_slot_inited = FALSE;
static UINT32 crc_enable = 0;
static OSAL_ID dmx_adv_slot_mutex = INVALID_ID;

#define DMX_ADV_SLOT_LOCK()			osal_mutex_lock(dmx_adv_slot_mutex, OSAL_WAIT_FOREVER_TIME)
#define DMX_ADV_SLOT_UNLOCK()		osal_mutex_unlock(dmx_adv_slot_mutex)


static INT32 adv_dmx_slot_callback(UINT32 ch_id, UINT32 flt_id, UINT8 *section, UINT32 length)
{
	INT32 i, j;

	DMX_ADV_SLOT_PRINTF("%s(%d, %d, %p, %d)\n", __FUNCTION__, ch_id, flt_id, section, length);

	//DMX_ADV_SLOT_LOCK();

	for (i = 0; i < MAX_SLOT_NUM; i++)
	{
		if (dmx_adv_slots[i].ch_id == ch_id && dmx_adv_slots[i].flt_id == flt_id)
		{
			for (j = 0; j < MAX_SLOT_CB_NUM; j++)
			{
				if (dmx_adv_slots[i].callback[j])
					dmx_adv_slots[i].callback[j](dmx_adv_slots[i].slot_id, section, length);
			}
			break;
		}
	}

	return SUCCESS;

	//DMX_ADV_SLOT_UNLOCK();
}

INT32 adv_dmx_slot_init(UINT32 crc_en)
{
	INT32 i;

	if (dmx_adv_slot_inited)
		return SUCCESS;
	
	DMX_ADV_SLOT_PRINTF("%s()\n", __FUNCTION__);

	crc_enable = crc_en;

	for (i = 0; i < MAX_SLOT_NUM; i++)
	{
		dmx_adv_slots[i].dmx_id = (UINT32)-1;
		dmx_adv_slots[i].ch_id = (UINT32)-1;
		dmx_adv_slots[i].pid = INVALID_PID;
		dmx_adv_slots[i].flt_id = (UINT32)-1;
		dmx_adv_slots[i].slot_id = -1;
		dmx_adv_slots[i].mv_len = 0;
		MEMSET(dmx_adv_slots[i].mask, 0, MAX_MV_LEN);
		MEMSET(dmx_adv_slots[i].value, 0, MAX_MV_LEN);
		MEMSET(dmx_adv_slots[i].callback, 0, sizeof(slot_sec_cb) * MAX_SLOT_CB_NUM);
	}

	dmx_adv_slot_mutex = osal_mutex_create();
	if (OSAL_INVALID_ID == dmx_adv_slot_mutex)
	{
		DMX_ADV_SLOT_ERROR("%s() create mutex for adv dmx slot failed!\n", __FUNCTION__);
		return -1;
	}

	dmx_adv_slot_inited = TRUE;
}

INT32 adv_dmx_slot_alloc(UINT32 dmx_id, UINT16 pid, UINT8 *mask, UINT8 *value, UINT8 length, slot_sec_cb slot_cb)
{
	INT32 i, j;
	UINT32 alloced_idx = (UINT32)-1, valid_idx = (UINT32)-1;;
	UINT32 ch_id, flt_id;

	if (!dmx_adv_slot_inited)
		return -1;

	DMX_ADV_SLOT_PRINTF("%s(%d, %04x, %d, %p)\n", __FUNCTION__, dmx_id, pid, length, slot_cb);

	DMX_ADV_SLOT_LOCK();

	for (i = 0; i < MAX_SLOT_NUM; i++)
	{
		if (valid_idx == (UINT32)-1 && dmx_adv_slots[i].dmx_id == (UINT32)-1 && \
			dmx_adv_slots[i].ch_id == (UINT32)-1 && dmx_adv_slots[i].slot_id == -1)
			valid_idx = i;
		
		if (dmx_adv_slots[i].dmx_id == dmx_id && dmx_adv_slots[i].pid == pid)
		{
			alloced_idx = i;
			ch_id = dmx_adv_slots[i].ch_id;
			if (0 == MEMCMP(dmx_adv_slots[i].mask, mask, length) && \
				0 == MEMCMP(dmx_adv_slots[i].value, value, length)&& \
				length == dmx_adv_slots[i].mv_len)
			{
				INT32 cb_pos = -1;
				
				DMX_ADV_SLOT_PRINTF("%s() same mask and value.\n", __FUNCTION__);
				
				for (j = 0; j < MAX_SLOT_CB_NUM; j++)
				{
					if (cb_pos == -1 && (UINT32)dmx_adv_slots[i].callback[j] == 0)
						cb_pos = j;
					
					if ((UINT32)slot_cb == (UINT32)dmx_adv_slots[i].callback[j])
					{
						DMX_ADV_SLOT_PRINTF("%s() reged callback %p already.\n", __FUNCTION__, slot_cb);
						break;
					}
				}
				if (j == MAX_SLOT_CB_NUM)
					dmx_adv_slots[i].callback[cb_pos] = slot_cb;
				
				DMX_ADV_SLOT_UNLOCK();
				
				return dmx_adv_slots[i].slot_id;
			}
		}
	}

	if (alloced_idx == (UINT32)-1)
	{
		ch_id = adv_dmx_alloc_channel(dmx_id, crc_enable, adv_dmx_slot_callback);
		if (ch_id == (UINT32)-1)
		{
			DMX_ADV_SLOT_ERROR("%s() adv_dmx_alloc_channel failed!\n", __FUNCTION__);
			DMX_ADV_SLOT_UNLOCK();
			return -1;
		}

		if (SUCCESS != adv_dmx_set_pid(ch_id, pid))
		{
			DMX_ADV_SLOT_ERROR("%s() adv_dmx_set_pid(%04x) failed!\n", __FUNCTION__, pid);
			DMX_ADV_SLOT_UNLOCK();
			return -1;
		}
	}
	else
	{
		ch_id = dmx_adv_slots[alloced_idx].ch_id;
		adv_dmx_dis_channel(ch_id);
	}

	flt_id = adv_dmx_alloc_filter(ch_id);
	if (flt_id == (UINT32)-1)
	{
		DMX_ADV_SLOT_ERROR("%s() adv_dmx_alloc_filter(%d) failed!\n", __FUNCTION__, ch_id);
		if(alloced_idx == (UINT32)-1)
			adv_dmx_free_channel(ch_id);
		else
			adv_dmx_en_channel(ch_id);
		DMX_ADV_SLOT_UNLOCK();
		return -1;
	}

	if (SUCCESS != adv_dmx_set_filter(ch_id, flt_id, mask, value, (UINT32)length))
	{
		DMX_ADV_SLOT_ERROR("%s() adv_dmx_set_filter(%d, %d) failed!\n", __FUNCTION__, ch_id, flt_id);
		if(alloced_idx == (UINT32)-1)
			adv_dmx_free_channel(ch_id);
		else
			adv_dmx_en_channel(ch_id);
		DMX_ADV_SLOT_UNLOCK();
		return -1;
	}

	if (SUCCESS != adv_dmx_en_channel(ch_id))
	{
		DMX_ADV_SLOT_ERROR("%s() adv_dmx_en_channel(%d) failed!\n", __FUNCTION__, ch_id);
		DMX_ADV_SLOT_UNLOCK();
		return -1;
	}

	dmx_adv_slots[valid_idx].dmx_id = dmx_id;
	dmx_adv_slots[valid_idx].pid = pid;
	dmx_adv_slots[valid_idx].ch_id = ch_id;
	dmx_adv_slots[valid_idx].flt_id = flt_id;
	MEMCPY(dmx_adv_slots[valid_idx].mask, mask, length);
	MEMCPY(dmx_adv_slots[valid_idx].value, value, length);
	dmx_adv_slots[valid_idx].mv_len = length;
	dmx_adv_slots[valid_idx].slot_id = valid_idx;
	dmx_adv_slots[valid_idx].callback[0] = slot_cb;

	DMX_ADV_SLOT_UNLOCK();
	
	return valid_idx;
}

INT32 adv_dmx_slot_free(INT32 slot_id, slot_sec_cb slot_cb)
{
	INT32 i, j;
	INT32 ret = -1;

	if (!dmx_adv_slot_inited)
		return -1;
	
	DMX_ADV_SLOT_PRINTF("%s(%d)\n", __FUNCTION__, slot_id);

	DMX_ADV_SLOT_LOCK();

	for (i = 0; i < MAX_SLOT_NUM; i++)
	{
		if (slot_id == dmx_adv_slots[i].slot_id)
		{
			for (j = 0; j < MAX_SLOT_CB_NUM; j++)
			{
				if(dmx_adv_slots[i].callback[j] == slot_cb)
				{
					dmx_adv_slots[i].callback[j] = NULL;
					break;
				}
			}
			
			if (j == MAX_SLOT_CB_NUM)
			{
				DMX_ADV_SLOT_ERROR("%s(%d) can not find valid slot cb <%p>!\n", __FUNCTION__, slot_id, slot_cb);
				DMX_ADV_SLOT_UNLOCK();
				return -1;
			}

			for (j = 0; j < MAX_SLOT_CB_NUM; j++)
			{
				if(dmx_adv_slots[i].callback[j] != NULL)
				{
					DMX_ADV_SLOT_UNLOCK();
					return SUCCESS;
				}
			}
			
			adv_dmx_free_filter(dmx_adv_slots[i].ch_id, dmx_adv_slots[i].flt_id);
			if (0 == adv_dmx_get_filter_num(dmx_adv_slots[i].ch_id))
			{
				DMX_ADV_SLOT_PRINTF("%s() channel %d has no filter, ready to free it!\n", __FUNCTION__, dmx_adv_slots[i].ch_id);
				adv_dmx_dis_channel(dmx_adv_slots[i].ch_id);
				adv_dmx_free_channel(dmx_adv_slots[i].ch_id);
			}
			
			dmx_adv_slots[i].dmx_id = (UINT32)-1;
			dmx_adv_slots[i].ch_id = (UINT32)-1;
			dmx_adv_slots[i].pid = INVALID_PID;
			dmx_adv_slots[i].flt_id = (UINT32)-1;
			dmx_adv_slots[i].slot_id = -1;
			dmx_adv_slots[i].mv_len = 0;
			MEMSET(dmx_adv_slots[i].mask, 0, MAX_MV_LEN);
			MEMSET(dmx_adv_slots[i].value, 0, MAX_MV_LEN);
			ret = SUCCESS;
		}
	}
	
	DMX_ADV_SLOT_UNLOCK();

	return ret;
}

UINT16 adv_dmx_slot_get_pid(INT32 slot_id)
{
	INT32 i;
	UINT16 pid = INVALID_PID;

	for (i = 0; i < MAX_SLOT_NUM; i++)
	{
		if (dmx_adv_slots[i].slot_id == slot_id)
		{
			pid = dmx_adv_slots[i].pid;
			break;
		}
	}

	return pid;
}

INT32 adv_dmx_slot_exit(void)
{
	INT32 i;
	
	crc_enable = 0;

	DMX_ADV_SLOT_LOCK();

	for (i = 0; i < MAX_SLOT_NUM; i++)
	{
		dmx_adv_slots[i].dmx_id = (UINT32)-1;
		dmx_adv_slots[i].ch_id = (UINT32)-1;
		dmx_adv_slots[i].pid = INVALID_PID;
		dmx_adv_slots[i].flt_id = (UINT32)-1;
		dmx_adv_slots[i].slot_id = -1;
		dmx_adv_slots[i].mv_len = 0;
		MEMSET(dmx_adv_slots[i].mask, 0, MAX_MV_LEN);
		MEMSET(dmx_adv_slots[i].value, 0, MAX_MV_LEN);
		MEMSET(dmx_adv_slots[i].callback, 0, sizeof(slot_sec_cb) * MAX_SLOT_CB_NUM);
	}

	DMX_ADV_SLOT_UNLOCK();

	if (OSAL_INVALID_ID != dmx_adv_slot_mutex)
	{
		osal_mutex_delete(dmx_adv_slot_mutex);
	}

	dmx_adv_slot_inited = FALSE;
}

