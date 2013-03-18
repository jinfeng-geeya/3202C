#include <sys_config.h>
#include <types.h>
#include <retcode.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libc/list.h>
#include <hld/hld_dev.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>

//#define ADV_DMX_DBG
#ifdef ADV_DMX_DBG
#define ADV_DMX_PRINTF			libc_printf//soc_printf
#define MEM_CHK(v)				do{}while(0)
//#define MEM_CHK(v)	do{if((((UINT32)v)&0xff000000)!=0x80000000)while(1);}while(0)
#else
#define ADV_DMX_PRINTF(...)		do{}while(0)
#define MEM_CHK(v)				do{}while(0)
#endif
#define TOT_DMX_NUM					3
#define MAX_CH_GROUP				5
#define MAX_MASK_LEN				32	// Modified by Leo Ma 2011-08-01
#define MAX_MASK_NUM				32	// Modified by Leo Ma 2011-08-09
#define SEC_BUF_LEN					4096
typedef struct{
	struct list_head	node;
	UINT16 data_len;
	UINT16 flt_id;
	UINT8 * buf;
}BUF_NODE, *P_BUF_NODE;

typedef struct{
	struct get_section_param param;
	struct restrict_ext	mv;
	UINT8 value[MAX_MASK_NUM][MAX_MASK_LEN];
	UINT8 mask[MAX_MASK_NUM][MAX_MASK_LEN];
	UINT8 mask_en[MAX_MASK_NUM];
	UINT8 mask_hit[MAX_MASK_NUM];
	struct list_head sec_list;
	struct list_head * wk_buf;
	UINT32 hw_flt;
	UINT32 sec_flt_msk;
	INT32 (*app_sec_cb)(UINT32 ch_id, UINT32 flt_id, UINT8 * section, UINT32 length); 
	UINT8 dev_num;
	UINT8 ch_num;
	UINT16 allocated: 1;
	UINT16 enabled : 1;
	UINT16 reserved: 14;
}CHANNEL, * P_CHANNEL;

static struct dmx_device * dev_list[TOT_DMX_NUM] = {NULL, NULL, NULL};
static BUF_NODE 	adv_dmx_buf_list;
static P_CHANNEL	adv_dmx_ch_list = NULL;
#define BUF_NODE_HEAD		(&(adv_dmx_buf_list.node))
static UINT32 adv_dmx_init_ok = 0;

static OSAL_ID adv_dmx_wait = OSAL_INVALID_ID;
static OSAL_ID adv_dmx_task_id = OSAL_INVALID_ID;
static UINT32 total_channel_num = 0;


void node_return(struct list_head * new, struct list_head * head)
{
	osal_task_dispatch_off();	
	MEM_CHK(head);
	MEM_CHK(head->next);
	MEM_CHK(head->prev);
	MEM_CHK(new);
	MEM_CHK(((P_BUF_NODE)new)->buf);
	list_add_tail(new, head);
	MEM_CHK(new->next);
	MEM_CHK(new->prev);
	osal_task_dispatch_on();				
}
		
struct list_head * node_get(struct list_head * head)
{
	struct list_head * node;
	osal_task_dispatch_off();
	MEM_CHK(head);
	MEM_CHK(head->next);
	MEM_CHK(head->prev);
	if(list_empty(head))
	{
		osal_task_dispatch_on();
		return NULL;
	}
	node = head->next;
	MEM_CHK(node);
	MEM_CHK(node->next);
	MEM_CHK(node->prev);
	MEM_CHK(((P_BUF_NODE)node)->buf);
	list_del(node);
	osal_task_dispatch_on();
	return node;
}

struct list_head * node_try_get(struct list_head * head)
{
	struct list_head * node;
	osal_task_dispatch_off();
	MEM_CHK(head);
	MEM_CHK(head->prev);
	MEM_CHK(head->next);
	if(list_empty(head))
	{
		osal_task_dispatch_on();
		return NULL;
	}
	node = head->next;
	MEM_CHK(node);
	MEM_CHK(node->next);
	MEM_CHK(node->prev);
	MEM_CHK(((P_BUF_NODE)node)->buf);
	osal_task_dispatch_on();
	return node;
}

struct list_head * node_get_one(struct list_head * head, UINT32 flt_id)
{
	struct list_head * node;
	osal_task_dispatch_off();
	MEM_CHK(head);
	MEM_CHK(head->prev);
	MEM_CHK(head->next);
	list_for_each(node, head)
	{
		if(((P_BUF_NODE)node)->flt_id==flt_id)
			break;	
	}
	if(node!=head)
	{
		MEM_CHK(node);
		MEM_CHK(node->next);
		MEM_CHK(node->prev);
		MEM_CHK(((P_BUF_NODE)node)->buf);
		list_del(node);
		osal_task_dispatch_on();
		return node;
	}
	osal_task_dispatch_on();
	return NULL;
}

struct list_head * node_try_get_one(struct list_head * head, UINT32 flt_id)
{
	struct list_head * node;
	osal_task_dispatch_off();
	MEM_CHK(head);
	MEM_CHK(head->prev);
	MEM_CHK(head->next);
	list_for_each(node, head)
	{
		if(((P_BUF_NODE)node)->flt_id==flt_id)
			break;
	}
	osal_task_dispatch_on();
	if(node==head)
		return NULL;
	MEM_CHK(node);
	MEM_CHK(node->next);
	MEM_CHK(node->prev);
	MEM_CHK(((P_BUF_NODE)node)->buf);
	return node;
}

void adv_dmx_actual_mutex(UINT32 lock)
{
	static OSAL_ID adv_dmx_mutex = OSAL_INVALID_ID;
	if(OSAL_INVALID_ID == adv_dmx_mutex)
	{
		adv_dmx_mutex = osal_mutex_create();
		ASSERT(OSAL_INVALID_ID != adv_dmx_mutex);
	}
	if(lock)
	{
		ADV_DMX_PRINTF("%s lock ", __FUNCTION__);
		osal_mutex_lock(adv_dmx_mutex, OSAL_WAIT_FOREVER_TIME);
		ADV_DMX_PRINTF("OK \n");
	}
	else
	{
		ADV_DMX_PRINTF("%s unlock \n", __FUNCTION__);
		osal_mutex_unlock(adv_dmx_mutex);
	}
}

/* Encapsulation of adv_dmx mutex for porting layer callback threading
 * protection as well as from dead lock.
 * Added by Leo Ma. Date: 2011-11-07
*/
void adv_dmx_mutex(UINT32 lock)
{
	if(adv_dmx_task_id != osal_task_get_current_id())
		adv_dmx_actual_mutex(lock);
}

void adv_dmx_init_channel(P_CHANNEL ch_node)
{
	//MEMSET(ch_node, 0, sizeof(CHANNEL));
	MEMSET(&ch_node->param, 0, sizeof(ch_node->param));
	MEMSET(&ch_node->mv, 0, sizeof(ch_node->mv));
	MEMSET(ch_node->mask, 0, sizeof(ch_node->mask));
	MEMSET(ch_node->value, 0, sizeof(ch_node->value));
	MEMSET(ch_node->mask_en, 0, sizeof(ch_node->mask_en));
	MEMSET(ch_node->mask_hit, 0, sizeof(ch_node->mask_hit));
	ch_node->allocated = 0;
	ch_node->enabled = 0;
	ch_node->hw_flt = 0;
	ch_node->ch_num = 0;
	ch_node->dev_num = 0;
	ch_node->sec_flt_msk = 0;
	ch_node->wk_buf = NULL;
	ch_node->reserved = 0;
	ch_node->param.pid = INVALID_PID;
	ch_node->param.retrieve_sec_fmt = RETRIEVE_SEC;
	ch_node->mv.tb_flt_msk = 0xff00;
	ch_node->mv.multi_value_ext = &ch_node->value[0][0];
	ch_node->mv.multi_mask_ext = &ch_node->mask[0][0];
	ch_node->mv.multi_mask_en_ext = ch_node->mask_en;
	ch_node->mv.multi_mask_hit_ext = ch_node->mask_hit;
	ch_node->mv.multi_mask_num_ext = MAX_MASK_NUM;
	ch_node->mv.multi_mask_len_ext = MAX_MASK_LEN;
	INIT_LIST_HEAD(&(ch_node->sec_list));
}

UINT8 * adv_dmx_get_sec(UINT32 ch_id, UINT32 * flt_id, UINT32 * sec_len, UINT32 specific_flt)
{
	UINT8 * buf = NULL;
	*sec_len = 0;
	if((adv_dmx_init_ok)&&(ch_id<total_channel_num)&&(adv_dmx_ch_list[ch_id].allocated))
	{
		P_BUF_NODE wk_buf = NULL;
		ADV_DMX_PRINTF("%s: ch %d, specific_flt %d ", __FUNCTION__, ch_id, specific_flt);
		if(!specific_flt)
		{
			wk_buf = (P_BUF_NODE)node_try_get(&(adv_dmx_ch_list[ch_id].sec_list));
			
		}
		else
		{
			if(((*flt_id)<MAX_MASK_NUM)&&
				(adv_dmx_ch_list[ch_id].sec_flt_msk&(1<<(*flt_id)))&&
				(adv_dmx_ch_list[ch_id].mask_en[(*flt_id)]))
			{
				wk_buf = (P_BUF_NODE)node_try_get_one(&(adv_dmx_ch_list[ch_id].sec_list), *flt_id);
			}
		}
		ADV_DMX_PRINTF("wk_buf %08x\n", wk_buf);
		if(wk_buf)
		{
			buf = wk_buf->buf;
			*sec_len = wk_buf->data_len;
			if(!specific_flt)
				*flt_id = wk_buf->flt_id;
		}
	}
	return buf;
}

INT32  adv_dmx_rel_sec(UINT32 ch_id, UINT32 flt_id, UINT32 specific_flt)
{
	INT32 rlt = !RET_SUCCESS;

	if((adv_dmx_init_ok)&&(ch_id<total_channel_num)&&(adv_dmx_ch_list[ch_id].allocated))
	{
		struct list_head * wk_buf = NULL;
		ADV_DMX_PRINTF("%s: ch %d, flt %d, specific_flt %d ", __FUNCTION__, ch_id, flt_id, specific_flt);					
		if(!specific_flt)
		{
//			ADV_DMX_PRINTF("%s: node_get\n", __FUNCTION__);
			wk_buf = node_get(&(adv_dmx_ch_list[ch_id].sec_list));
		}
		else
		{
			if((flt_id<MAX_MASK_NUM)&&
				(adv_dmx_ch_list[ch_id].sec_flt_msk&(1<<flt_id))&&
				(adv_dmx_ch_list[ch_id].mask_en[flt_id]))
			{
				wk_buf = node_get_one(&(adv_dmx_ch_list[ch_id].sec_list), flt_id);
			}
		}
		ADV_DMX_PRINTF("wk_buf %08x\n", wk_buf);
		if(wk_buf)
		{
			node_return(wk_buf, BUF_NODE_HEAD);
			rlt = RET_SUCCESS;
		}
	}
	return rlt;
}

INT32 adv_dmx_retrieve_sec(UINT32 ch_id, UINT32 * flt_id, UINT8 * buf, UINT32 * sec_len, UINT32 max_len, UINT32 specific_flt)
{
	UINT8 * sec_buf = NULL;
	INT32 rlt = !RET_SUCCESS;
	ADV_DMX_PRINTF("%s\n", __FUNCTION__);		
	* sec_len = 0;
	sec_buf = adv_dmx_get_sec(ch_id, flt_id, sec_len, specific_flt);
	if(sec_buf&&((*sec_len)>0)&&((*sec_len)<=max_len))
	{
		MEMCPY(buf, sec_buf, *sec_len);
		if(RET_SUCCESS==adv_dmx_rel_sec(ch_id, *flt_id, specific_flt))
			rlt = RET_SUCCESS;
	}
	return rlt;
}

void adv_dmx_sec_cb(struct get_section_param * param)
{
	P_CHANNEL ch = (P_CHANNEL)param;
	UINT32 ch_id = ch->ch_num;
	UINT32 flt_id = ch->param.sec_hit_num;
	ADV_DMX_PRINTF("%s: pid %d, tbid %02x: ", __FUNCTION__, param->pid, param->buff[0]);
	if((adv_dmx_init_ok)&&(ch_id<total_channel_num)&&
		(flt_id<MAX_MASK_NUM)&&ch->allocated&&ch->enabled&&
		(ch->sec_flt_msk&(1<<flt_id))&&(ch->mask_en[flt_id]))
	{
//		ADV_DMX_PRINTF("%s: node_get: ", __FUNCTION__);
		P_BUF_NODE wk_buf = (P_BUF_NODE)node_get(BUF_NODE_HEAD);
		if(wk_buf)
		{
			ADV_DMX_PRINTF("OK! node %08x, buf %08x\n", wk_buf, wk_buf->buf);
			MEMCPY(wk_buf->buf, param->buff, param->sec_tbl_len);
			wk_buf->data_len = param->sec_tbl_len;
			wk_buf->flt_id = flt_id;
			node_return((struct list_head *)wk_buf, (struct list_head *)(&ch->sec_list));
			osal_mutex_unlock(adv_dmx_wait);
			return ;
		}
		ADV_DMX_PRINTF("Faile!\n");
	}
	ADV_DMX_PRINTF(">>\n");
}

void adv_dmx_task(UINT32 param1, UINT32 param2)
{
	OSAL_T_CTSK task_param ;
	while(param1==1)
	{
		UINT32 i;
		osal_mutex_lock(adv_dmx_wait, 20);

		for(i=0; i<total_channel_num; i++)
		{
			if(adv_dmx_ch_list[i].allocated&&adv_dmx_ch_list[i].enabled)
			{
				while(1)
				{
					P_BUF_NODE wk_buf = (P_BUF_NODE)node_try_get(&(adv_dmx_ch_list[i].sec_list));
				
					if(!wk_buf)
						break;
					ADV_DMX_PRINTF("%s: call back \n", __FUNCTION__);
					//adv_dmx_actual_mutex(1);
					if(RET_SUCCESS==adv_dmx_ch_list[i].app_sec_cb(i, wk_buf->flt_id, wk_buf->buf, wk_buf->data_len))
					{
						struct list_head * node = node_get(&(adv_dmx_ch_list[i].sec_list));
						if(node == (struct list_head *)wk_buf)						
							node_return(node, BUF_NODE_HEAD);
						ADV_DMX_PRINTF("Submit Sec @: ch %d flt %d\n", i, wk_buf->flt_id);
					}
					else
					{
						//adv_dmx_actual_mutex(0);		
						ADV_DMX_PRINTF("Notify Sec @: ch %d flt %d\n", i, wk_buf->flt_id);	
						break;
					}
					//adv_dmx_actual_mutex(0);		
				}
			}
		}
	}
	if((param1==0)&&(adv_dmx_task_id == OSAL_INVALID_ID))
	{
		//creat adv_dmx_task
		task_param.task   =  (FP)adv_dmx_task;
		task_param.stksz  =  0x1000; 
		task_param.quantum = 10;
		task_param.itskpri = OSAL_PRI_NORMAL;
		task_param.name[0] = 'A';
		task_param.name[1] = 'D';
		task_param.name[2] = 'M';
		task_param.para1 = 1;
		task_param.para2 = 0;
		//create task 
		adv_dmx_task_id=osal_task_create(&task_param);
		ASSERT(adv_dmx_task_id != OSAL_INVALID_ID);
		ADV_DMX_PRINTF("%s: created!\n", __FUNCTION__);
	}
}

INT32 adv_dmx_init(UINT32 max_channel_num, UINT8 * buf, UINT32 buf_len)
{
	UINT32 tmp1, tmp2;
	ADV_DMX_PRINTF("%s: channel %d, buf %08x, len %d\n", __FUNCTION__, 
		max_channel_num, buf, buf_len);
	if(!max_channel_num)
		return !RET_SUCCESS;
	ADV_DMX_PRINTF("Need buffer size > %d Bytes!\n", max_channel_num*((SEC_BUF_LEN+sizeof(BUF_NODE))*2+sizeof(CHANNEL)));

	//Check max required channel number
	total_channel_num = max_channel_num;

	//DW alignment
	tmp1 = (UINT32)buf;
	tmp2 = tmp1&0x3;
	tmp1 &= (~0x3);
	if(buf_len>tmp2)
		buf_len -= tmp2;
	else
		return !RET_SUCCESS;
	buf = (UINT8 *)tmp1;

	//alloc channel node buffer
	if(buf_len<=(total_channel_num*sizeof(CHANNEL)))
	{
		ADV_DMX_PRINTF("No enough buffer for channel node!\n");
		return !RET_SUCCESS;
	}
	adv_dmx_ch_list = (P_CHANNEL)buf;
	for(tmp1=0; tmp1<total_channel_num; tmp1++)
		adv_dmx_init_channel(&adv_dmx_ch_list[tmp1]);
	buf += (total_channel_num*sizeof(CHANNEL));
	buf_len -= (total_channel_num*sizeof(CHANNEL));

	//alloc working buffer
	tmp1 = buf_len/(SEC_BUF_LEN+sizeof(BUF_NODE));
	if(tmp1<2)
	{
		ADV_DMX_PRINTF("No enough buffer for section!\n");
		return !RET_SUCCESS;
	}
	INIT_LIST_HEAD(BUF_NODE_HEAD);
	ADV_DMX_PRINTF("Totally %d section buffer available!\n", tmp1);
	while(tmp1--)
	{
		P_BUF_NODE tmp_nod = (P_BUF_NODE)buf;
		tmp_nod->buf = buf+sizeof(BUF_NODE);
		node_return((struct list_head *)tmp_nod, (struct list_head *)BUF_NODE_HEAD);
		buf += (SEC_BUF_LEN+sizeof(BUF_NODE));
	}

	//alloc dmx device node
	for(tmp1 = 0; tmp1<TOT_DMX_NUM; tmp1++)
	{
		dev_list[tmp1] = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, tmp1);
	}
	if(NULL==dev_list[0])
	{
		ADV_DMX_PRINTF("No dmx device 0 node!\n");
		return !RET_SUCCESS;
	}
	//create tas/mutex/etc
	if(OSAL_INVALID_ID == adv_dmx_wait)
	{
		adv_dmx_wait = osal_mutex_create();
		ASSERT(OSAL_INVALID_ID != adv_dmx_wait);
		osal_mutex_lock(adv_dmx_wait, OSAL_WAIT_FOREVER_TIME);
	}
	
	adv_dmx_task(0, 0);
	adv_dmx_init_ok = 1;
	return RET_SUCCESS;
}

INT32 adv_dmx_quit(void)
{
	UINT32 i, j;
	INT32 rlt = RET_SUCCESS;
	if(!adv_dmx_init_ok)
		return !RET_SUCCESS;
	for(i=0; i<total_channel_num; i++)
	{
		if(adv_dmx_ch_list[i].allocated)
		{
			ADV_DMX_PRINTF("%s: ch %d not free!\n", __FUNCTION__, i);
			rlt = !RET_SUCCESS;
			if(adv_dmx_ch_list[i].enabled)
			{
				ADV_DMX_PRINTF("%s: ch %d not disabled!\n", __FUNCTION__, i);
				rlt = !RET_SUCCESS;
				adv_dmx_dis_channel(i);
			}
			for(j=0; j<MAX_MASK_NUM;j++)
			{
				if(adv_dmx_ch_list[i].sec_flt_msk&(1<<j))
				{
					ADV_DMX_PRINTF("%s: ch %d , flt %d not free!\n", __FUNCTION__, i, j);
					rlt = !RET_SUCCESS;
					adv_dmx_free_filter(i, j);
				}
			}
			adv_dmx_free_channel(i);
		}
	}
	adv_dmx_init_ok = 0;
	osal_task_delete(adv_dmx_task_id);
	adv_dmx_task_id = OSAL_INVALID_ID;
	osal_mutex_delete(adv_dmx_wait);
	adv_dmx_wait = OSAL_INVALID_ID;
	for(i=0;i<TOT_DMX_NUM;i++)
		dev_list[i] = NULL;
	adv_dmx_ch_list = NULL;
	total_channel_num = 0;

	ADV_DMX_PRINTF("%s: %s!\n", __FUNCTION__, rlt==RET_SUCCESS?"OK":"FAIL");
	return rlt;
}

UINT32 adv_dmx_alloc_channel(UINT32 dmx_id, UINT32 crc_en, 
	INT32 (*reg_sec_cb)(UINT32 ch_id, UINT32 flt_id, UINT8 * section, UINT32 length))
{
	UINT32 i = 0xffffffff;
	if(!adv_dmx_init_ok)
		return i;
	if(!reg_sec_cb)
	{
		ADV_DMX_PRINTF("No channel call back function, it's MUST !\n");
		return i;
	}
	adv_dmx_mutex(1);
	for(i=0; i<total_channel_num; i++)
	{
		if(!adv_dmx_ch_list[i].allocated)
			break;
	}
	if(i==total_channel_num)
	{
		ADV_DMX_PRINTF("No channel available!\n");
		i = 0xffffffff;
	}
	else
	{
		if((dmx_id>=TOT_DMX_NUM)||(NULL==dev_list[dmx_id]))
		{
			ADV_DMX_PRINTF("DMX id %d or device %08x not available!\n", dmx_id, dev_list[dmx_id]);
			i = 0xffffffff;
		}
		else
		{
			//ADV_DMX_PRINTF("%s: node_get\n", __FUNCTION__);
			P_BUF_NODE wk_buf = (P_BUF_NODE)node_get(BUF_NODE_HEAD);
			if(!wk_buf)
			{
				ADV_DMX_PRINTF("No more working buffer for channel %d\n", i);
				i = 0xffffffff;
			}
			else
			{ 
				adv_dmx_ch_list[i].param.buff = wk_buf->buf;
				adv_dmx_ch_list[i].param.buff_len = SEC_BUF_LEN;
				adv_dmx_ch_list[i].param.crc_flag = crc_en;
				adv_dmx_ch_list[i].param.get_sec_cb = adv_dmx_sec_cb;
				adv_dmx_ch_list[i].param.continue_get_sec = 1;
				adv_dmx_ch_list[i].app_sec_cb = reg_sec_cb;
				adv_dmx_ch_list[i].param.mask_value = (struct restrict *)(&adv_dmx_ch_list[i].mv);
				adv_dmx_ch_list[i].allocated = 1;
				adv_dmx_ch_list[i].dev_num = dmx_id;
				adv_dmx_ch_list[i].ch_num = i;
				adv_dmx_ch_list[i].wk_buf = (struct list_head *)wk_buf;
				ADV_DMX_PRINTF("Allocated channel number %d OK\n", i);
			}
		}
	}
	adv_dmx_mutex(0);
	return i;
}

INT32 adv_dmx_free_channel(UINT32 ch_id)
{
	UINT32 rlt = !RET_SUCCESS;
	if((adv_dmx_init_ok)&&(ch_id<total_channel_num))
	{
		adv_dmx_mutex(1);
		if((adv_dmx_ch_list[ch_id].allocated)&&(!adv_dmx_ch_list[ch_id].enabled))
		{
			struct list_head * sec_node = NULL;
			//ADV_DMX_PRINTF("%s: node_get\n", __FUNCTION__);
			while(NULL!=(sec_node = node_get(&(adv_dmx_ch_list[ch_id].sec_list))))
				node_return(sec_node, BUF_NODE_HEAD);
				
			if(adv_dmx_ch_list[ch_id].wk_buf)
			{
				node_return(adv_dmx_ch_list[ch_id].wk_buf, BUF_NODE_HEAD);
				adv_dmx_init_channel(&adv_dmx_ch_list[ch_id]);
				ADV_DMX_PRINTF("Free channel number %d OK\n", ch_id);
				rlt = RET_SUCCESS;
			}
			else
				ASSERT(0);
		}
		adv_dmx_mutex(0);
	}
	return rlt;
}

INT32 adv_dmx_set_pid(UINT32 ch_id, UINT16 pid)
{
	UINT32 rlt = !RET_SUCCESS;
	if((adv_dmx_init_ok)&&(ch_id<total_channel_num)&&(INVALID_PID!=pid))
	{
		adv_dmx_mutex(1);
		if((adv_dmx_ch_list[ch_id].allocated)&&(!adv_dmx_ch_list[ch_id].enabled))
		{
			struct list_head * sec_node = NULL;
			while(NULL!=(sec_node = node_get(&(adv_dmx_ch_list[ch_id].sec_list))))
				node_return(sec_node, BUF_NODE_HEAD);
			adv_dmx_ch_list[ch_id].param.pid = pid;
			ADV_DMX_PRINTF("%s: ch %d, pid %d\n", __FUNCTION__, ch_id, pid);		
			rlt = RET_SUCCESS;
		}
		adv_dmx_mutex(0);
	}
	return rlt;
}

UINT16 adv_dmx_get_pid(UINT32 ch_id)
{
	UINT16 rlt = INVALID_PID;
	if((adv_dmx_init_ok)&&(ch_id<total_channel_num))
	{
		adv_dmx_mutex(1);
		if(adv_dmx_ch_list[ch_id].allocated)
		{
			struct list_head * sec_node = NULL;
			while(NULL!=(sec_node = node_get(&(adv_dmx_ch_list[ch_id].sec_list))))
				node_return(sec_node, BUF_NODE_HEAD);
			rlt = adv_dmx_ch_list[ch_id].param.pid;
			ADV_DMX_PRINTF("%s: ch %d, pid %d\n", __FUNCTION__, ch_id, rlt);
		}
		adv_dmx_mutex(0);
	}
	return rlt;
}

INT32 adv_dmx_en_channel(UINT32 ch_id)
{
	UINT32 rlt = !RET_SUCCESS;
	if((adv_dmx_init_ok)&&(ch_id<total_channel_num))
	{
		adv_dmx_mutex(1);
		if((adv_dmx_ch_list[ch_id].allocated)&&(!adv_dmx_ch_list[ch_id].enabled))
		{
			if(RET_SUCCESS==dmx_async_req_section(dev_list[adv_dmx_ch_list[ch_id].dev_num], \
				(void *)(&adv_dmx_ch_list[ch_id].param), (UINT8 *)(&(adv_dmx_ch_list[ch_id].hw_flt))))
			{
				adv_dmx_ch_list[ch_id].enabled = 1;
				rlt = RET_SUCCESS;
				ADV_DMX_PRINTF("%s OK @ %d\n", __FUNCTION__, ch_id);
			}
			else
			{
				ADV_DMX_PRINTF("%s Fail @ %d\n", __FUNCTION__, ch_id);
			}
		}
		adv_dmx_mutex(0);
	}
	return rlt;
}

INT32 adv_dmx_dis_channel(UINT32 ch_id)
{
	UINT32 rlt = !RET_SUCCESS;
	if((adv_dmx_init_ok)&&(ch_id<total_channel_num))
	{
		adv_dmx_mutex(1);
		if((adv_dmx_ch_list[ch_id].allocated)&&(adv_dmx_ch_list[ch_id].enabled))
		{
			if(RET_SUCCESS==dmx_io_control(dev_list[adv_dmx_ch_list[ch_id].dev_num],\
				IO_ASYNC_CLOSE, adv_dmx_ch_list[ch_id].hw_flt))
			{
				adv_dmx_ch_list[ch_id].enabled = 0;
			#if 0	
				struct list_head * sec_node = NULL;
				//ADV_DMX_PRINTF("%s: node_get\n", __FUNCTION__);
				while(NULL!=(sec_node = node_get(&(adv_dmx_ch_list[ch_id].sec_list))))
					node_return(sec_node, BUF_NODE_HEAD);
			#endif	
				rlt = RET_SUCCESS;
				ADV_DMX_PRINTF("%s OK @ %d\n", __FUNCTION__, ch_id);
			}
			else
			{
				ADV_DMX_PRINTF("%s Fail @ %d\n", __FUNCTION__, ch_id);
			}
		}
		adv_dmx_mutex(0);
	}
	return rlt;
}

INT32 adv_dmx_rst_channel(UINT32 ch_id)
{
	UINT32 rlt = !RET_SUCCESS;
	struct list_head * sec_node = NULL;
	if((adv_dmx_init_ok)&&(ch_id<total_channel_num))
	{
		adv_dmx_mutex(1);
		if(adv_dmx_ch_list[ch_id].allocated)
		{
			UINT8 need_restart = adv_dmx_ch_list[ch_id].enabled;
			if(need_restart)
			{
				if(RET_SUCCESS!=dmx_io_control(dev_list[adv_dmx_ch_list[ch_id].dev_num],\
					IO_ASYNC_CLOSE, adv_dmx_ch_list[ch_id].hw_flt))
				{
					ADV_DMX_PRINTF("%s Fail @ %d while stop channel\n", __FUNCTION__, ch_id);
					adv_dmx_mutex(0);
					return rlt;
				}
			}														
			while(NULL!=(sec_node = node_get(&(adv_dmx_ch_list[ch_id].sec_list))))
				node_return(sec_node, BUF_NODE_HEAD);
			if(need_restart)
			{
				if(RET_SUCCESS!=dmx_async_req_section(dev_list[adv_dmx_ch_list[ch_id].dev_num], \
				(void *)(&adv_dmx_ch_list[ch_id].param), (UINT8 *)(&(adv_dmx_ch_list[ch_id].hw_flt))))
				{
					ADV_DMX_PRINTF("%s Fail @ %d while re-start channel\n", __FUNCTION__, ch_id);
					adv_dmx_mutex(0);
					return rlt;
				}
			}
			ADV_DMX_PRINTF("%s OK @ %d\n", __FUNCTION__, ch_id);
			rlt = RET_SUCCESS;
		}
		adv_dmx_mutex(0);
	}
	return rlt;
}

UINT32 adv_dmx_alloc_filter(UINT32 ch_id)
{
	UINT32 i = 0xffffffff;
	if((adv_dmx_init_ok)&&(ch_id<total_channel_num))
	{
		adv_dmx_mutex(1);
		if(adv_dmx_ch_list[ch_id].allocated)
		{
			for(i=0; i<MAX_MASK_NUM; i++)
			{
				if(0==(adv_dmx_ch_list[ch_id].sec_flt_msk&(1<<i)))	
					break;
			}
			if(i<MAX_MASK_NUM)
			{
				adv_dmx_ch_list[ch_id].sec_flt_msk |= (1<<i);
				ADV_DMX_PRINTF("%s OK @ch %d with %d\n", __FUNCTION__, ch_id, i);
			}
			else
			{
				i = 0xffffffff;
				ADV_DMX_PRINTF("%s Fail @ch %d\n", __FUNCTION__, ch_id);
			}
		}
		adv_dmx_mutex(0);
	}
	return i;
}

INT32 adv_dmx_free_filter(UINT32 ch_id, UINT32 flt_id)
{
	UINT32 rlt = !RET_SUCCESS;
	if((adv_dmx_init_ok)&&(ch_id<total_channel_num)&&(flt_id<MAX_MASK_NUM))
	{
		adv_dmx_mutex(1);
		if(adv_dmx_ch_list[ch_id].allocated)
		{
			if(adv_dmx_ch_list[ch_id].sec_flt_msk&(1<<flt_id))	
			{
				struct list_head * node;
				adv_dmx_ch_list[ch_id].sec_flt_msk &= ~(1<<flt_id);
				adv_dmx_ch_list[ch_id].mask_en[flt_id] = 0;
				//scan specific node and delete it
				while(NULL!=(node = node_get_one(&(adv_dmx_ch_list[ch_id].sec_list), flt_id)))
					node_return(node, BUF_NODE_HEAD);
				rlt = RET_SUCCESS;
				ADV_DMX_PRINTF("%s OK @ch %d with %d\n", __FUNCTION__, ch_id, flt_id);
			}
			else
			{
				if(adv_dmx_ch_list[ch_id].mask_en[flt_id])
				{
					ADV_DMX_PRINTF("%s Fetal Error @ch %d with %d\n", __FUNCTION__, ch_id, flt_id);
					ASSERT(0);
				}
				ADV_DMX_PRINTF("%s Fail @ch %d with %d\n", __FUNCTION__, ch_id, flt_id);
			}
		}
		adv_dmx_mutex(0);
	}
	return rlt;
}

INT32 adv_dmx_set_filter(UINT32 ch_id, UINT32 flt_id, UINT8 * mask, UINT8 * value, UINT32 len)
{
	UINT32 rlt = !RET_SUCCESS;
	if((adv_dmx_init_ok)&&(ch_id<total_channel_num)&&(flt_id<MAX_MASK_NUM)&&len<=MAX_MASK_LEN)
	{
		adv_dmx_mutex(1);
		if(adv_dmx_ch_list[ch_id].allocated)
		{
			if(adv_dmx_ch_list[ch_id].sec_flt_msk&(1<<flt_id))	
			{
				struct list_head * node;
				
				adv_dmx_ch_list[ch_id].mask_en[flt_id] = 0;
				//scan specific node and delete it
				while(NULL!=(node = node_get_one(&(adv_dmx_ch_list[ch_id].sec_list), flt_id)))
					node_return(node, BUF_NODE_HEAD);
				MEMSET(&adv_dmx_ch_list[ch_id].mask[flt_id][0], 0, MAX_MASK_LEN);
				MEMSET(&adv_dmx_ch_list[ch_id].value[flt_id][0], 0, MAX_MASK_LEN);
				MEMCPY(&adv_dmx_ch_list[ch_id].mask[flt_id][0], mask, len);
				MEMCPY(&adv_dmx_ch_list[ch_id].value[flt_id][0], value, len);
				adv_dmx_ch_list[ch_id].mask_en[flt_id] = 1;
				rlt = RET_SUCCESS;
				ADV_DMX_PRINTF("%s OK @ %d with %d\n", __FUNCTION__, ch_id, flt_id);
				ADV_DMX_PRINTF("mask: %02x \n", mask[0]);
				ADV_DMX_PRINTF("value: %02x \n", value[0]);	
			}
			else
			{
				ADV_DMX_PRINTF("%s Fail @ %d with %d\n", __FUNCTION__, ch_id, flt_id);
			}
		}
		adv_dmx_mutex(0);
	}
	return rlt;
}

INT32 adv_dmx_get_filter_num(UINT32 ch_id)
{
	INT32 i;
	INT32 flt_num = 0;
	UINT32 rlt = -1;
	if((adv_dmx_init_ok)&&(ch_id<total_channel_num))
	{
		adv_dmx_mutex(1);
		if(adv_dmx_ch_list[ch_id].allocated)
		{
			for(i=0;i<MAX_MASK_NUM;i++)
			{
				if(adv_dmx_ch_list[ch_id].sec_flt_msk&(1<<i))
					flt_num++;
			}
		}
		adv_dmx_mutex(0);
		return flt_num;
	}
	return rlt;
}

INT32 adv_dmx_get_free_ch_num(void)
{
	UINT32 i;
	INT32 ch_num = 0;
	UINT32 rlt = -1;

	if (!adv_dmx_init_ok)
		return rlt;

	for(i = 0; i < total_channel_num; i++)
	{
		if(!adv_dmx_ch_list[i].allocated)
		{
			ch_num++;
		}
	}

	return ch_num;
}

/********The following are sample code try to get EIT table for demo*********/
#if 0
void adv_dmx_test_mutex(UINT32 lock)
{
	static OSAL_ID adv_dmx_test_mutex = OSAL_INVALID_ID;
	if(OSAL_INVALID_ID == adv_dmx_test_mutex)
	{
		adv_dmx_test_mutex = osal_mutex_create();
		ASSERT(OSAL_INVALID_ID != adv_dmx_test_mutex);
	}
	if(lock)
		osal_mutex_lock(adv_dmx_test_mutex, OSAL_WAIT_FOREVER_TIME);
	else
		osal_mutex_unlock(adv_dmx_test_mutex);
}

INT32 adv_dmx_test_cb(UINT32 ch_id, UINT32 flt_id, UINT8 * section, UINT32 length)
{
	UINT32 i;
	adv_dmx_test_mutex(1);
	libc_printf("ch %d, flt %d submit sec_len %d, sec_buf %08x: ", ch_id, flt_id, length, section);
/*
	if(length>8)
		length = 8;
	for(i=0; i<length; i++)
		libc_printf("0x%02x ", section[i]);
*/	
	libc_printf("  @time %d ms\n", osal_get_tick());
	adv_dmx_test_mutex(0);
	return !RET_SUCCESS;
}

INT32 adv_dmx_test_dump(UINT32 ch_id, UINT32 flt_id, UINT8 * section, UINT32 length)
{
	UINT32 i;
	adv_dmx_test_mutex(1);
#if 1	
	libc_printf("ch %d, flt %d dump sec %d: ", ch_id, flt_id, length);
	if(length>8)
		length = 8;
	for(i=0; i<length; i++)
		libc_printf("0x%02x ", section[i]);
	libc_printf("\n");
#endif	
	adv_dmx_test_mutex(0);
	return RET_SUCCESS;
}

void adv_dmx_test_task(UINT32 param1, UINT32 param2)
{
	OSAL_T_CTSK task_param ;

	if(param1==0)
	{
		//creat adv_dmx_task
		task_param.task   =  (FP)adv_dmx_test_task;
		task_param.stksz  =  0x1000; 
		task_param.quantum = 10;
		task_param.itskpri = OSAL_PRI_NORMAL;
		task_param.name[0] = 'A';
		task_param.name[1] = 'D';
		task_param.name[2] = 'M';
		task_param.para1 = 1;
		task_param.para2 = 0;
		
		//create task 
		ASSERT(OSAL_INVALID_ID!=osal_task_create(&task_param));
		
	}
	while(param1)
	{
		UINT32 test_cnt = 30;
		UINT32 ch_id = 0;
		adv_dmx_init(2, ENREACH_DMXBUF_START, ENREACH_DMXBUF_SIZE);
		ch_id = adv_dmx_alloc_channel(0, 0, adv_dmx_test_cb);
		if(0xffffffff==ch_id)
		{	
			while(1);
		}
		else
		{
			UINT32 flt_list[2];
			UINT16 pid = 18;
			UINT8 mask0[] ={0xff, 0x80, 0x00, 0x00, 0x00, 0x01}; 
			UINT8 value0[] = {0x4e, 0x80, 0x00, 0x00, 0x00, 0x01}; 
			UINT8 mask1[] = {0xfc, 0x80, 0x00, 0x00, 0x00, 0x01}; 
			UINT8 value1[] = {0x50, 0x80, 0x00, 0x00, 0x00, 0x01}; 

			libc_printf("alloc ch @%d OK!\n", ch_id);
			flt_list[0] = adv_dmx_alloc_filter(ch_id);
			if(0xffffffff==flt_list[0])
			{	
				while(1);
			}
			libc_printf("alloc 1st flt @%d OK!\n", flt_list[0]);
			flt_list[1] = adv_dmx_alloc_filter(ch_id);
			if(0xffffffff==flt_list[1])
			{	
				while(1);
			}
			libc_printf("alloc 2nd flt @%d OK!\n", flt_list[1]);
			adv_dmx_set_pid(ch_id, pid);
			adv_dmx_set_filter(ch_id, flt_list[0], mask0, value0, 6);
			adv_dmx_set_filter(ch_id, flt_list[1], mask1, value1, 6);
			adv_dmx_en_channel(ch_id);
			while(test_cnt--)
			{
				UINT8 * buf;
				UINT32 sec_len;
				osal_task_sleep(200);
				while(1)
				{
					buf = adv_dmx_get_sec(ch_id, &flt_list[0], &sec_len, 1);
					if(buf==NULL||sec_len==0)
						break;
					adv_dmx_test_dump(ch_id, flt_list[0], buf, sec_len);
					adv_dmx_rel_sec(ch_id, flt_list[0], 1);
				}
				while(1)
				{
					buf = adv_dmx_get_sec(ch_id, &flt_list[1], &sec_len, 1);
					if(buf==NULL||sec_len==0)
						break;
					adv_dmx_test_dump(ch_id, flt_list[1], buf, sec_len);
					adv_dmx_rel_sec(ch_id, flt_list[1], 1);
				}
			}
			adv_dmx_dis_channel(ch_id);
			adv_dmx_free_filter(ch_id, flt_list[0]);
			adv_dmx_free_filter(ch_id, flt_list[1]);
			adv_dmx_free_channel(ch_id);
		}
		adv_dmx_quit();
		
	}
}
#endif


