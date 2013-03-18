/*
 * sie.c
 *
 * SI engine implementation revision 3.0
 * 
 * This engine has the following features:
 * - reduced CPU time usage.
 * - pipeline filter supported.
 * - simplified external data structure.
 *
 * History
 * 1. 20051125	2.00	Zhengdao Li	Re-design for section pipeline support.
 * 2. 20050117	2.01	Zhengdao Li	Merge to Dolphin Tree.
 * 3. 20050519	2.02	Zhengdao Li	remove lock from engine thread, 
 * 					allow cb controling filter.
 * 4. 20060707	2.03	Zhengdao Li	thread safe revised. timeout=0 bug-fix.
 * 5. 20080808  3.00	Yun You develop for supporting multi task & DMX.
 * 6. 20081215	3.01	Steven fix ring buffer Read issue.
 * 7. 20081224	3.02	Steven add 2 status according Ming Yi's suggestion.
 *                      This version of SIE still has 2 problems:
 *						(1)in dispatch_section, when callback section_parser return sie_freed,
 *                       it's not really freed immediately. This issue need user(such as SI_Search)
 *                       to avoid it.
 *                      (2)when sie_abort some ring buffer, some section of data in the ring buffer
 *                       maybe lost. This issue need user(such as EPG) to avoid it.
 */
 
#include <types.h>
#include <sys_config.h>

#include <api/libc/list.h>

#include <osal/osal.h>

#include <api/libc/alloc.h>
#include <api/libc/string.h>
#include <api/libc/fastCRC.h>

#include <api/libtsi/si_types.h>
#include <api/libtsi/sie.h>
#include <api/libsi/si_module.h>

#define DUPLICATE_PID_SUPPORT
#define SIE_DEBUG_LEVEL				0
#define DBG_PRINTF(...)				do{}while(0)
#if (SIE_DEBUG_LEVEL>1)
#include <api/libc/printf.h>
#define SIE_PRINTF				libc_printf
#define RINGBUF_PRINTF			libc_printf
#elif (SIE_DEBUG_LEVEL==1)
#include <api/libc/printf.h>
#define SIE_PRINTF				PRINTF
#define RINGBUF_PRINTF			libc_printf
#else
#define SIE_PRINTF(...)				do{}while(0)
#define RINGBUF_PRINTF(...)			do{}while(0)
#endif

enum {
	SIE_MODULE_TASK_QUANTUM			= 7,
	SIE_MODULE_TASK_STACKSIZE		= 0x1800,
	SIE_MODULE_TASK_PRIORITY		= OSAL_PRI_NORMAL,
};

enum {
	SIE_FLAG_TASK_ABORTING			= 0x80000000UL,
	SIE_FLAG_TASK_ABORTED			= 0x40000000UL,
	SIE_FLAG_ENGINE_CLOSING			= 0x20000000UL,
	SIE_FLAG_ENGINE_CLOSED			= 0x10000000UL,
	SIE_FLAG_ENGINE_ENABLE			= 0x08000000UL,
	SIE_FLAG_MUTEX					= 0x04000000UL,
	SIE_FLAG_RING_MUTEX				= 0x02000000UL,
};

enum {
	SIE_FLT_FREE	= 0x00,
	SIE_FLT_IDLE	= 0x01,
	SIE_FLT_ACTIVE	= 0x02,
	SIE_FLT_RUNNING = 0x03,
};

/*for filter continu get section mode, ring buf should be used, the bottom 4K byte of ring buf is
reserved as dmx working buf.
20081223. dmx working buf only need max_sec_length, no need 4k at all.*/
//#define SIE_RINGBUF_RESERVE_LEN	0x1000

#define MAX_DMX_NUM	3
/*
 * name		: sie_request
 * description	: request for SI engine.
 * field	:
 *	list		: the double-linked list management.
 *	dmx_filter	: the demux filter number in demux device.
 *	max_sec_length	: the maximum section length, normally 1024 or 4096.
 *  reserved_len: ring buffer reserved for dmx.
 *	timeout		: in mSecond, OSAL_WAIT_FOREVER_TIME for cyclic.
 *	read		: the read pointer for ring buffer.
 *	write		: the write pointer for ring buffer.
 *	target		: targeting linked list.
 *	gsp		: the param that demux need for a filter.
 *	filter		: the filter structure provided to module API.
 *	buf_len		: the total ring buffer length.
 *	buffer		: the ring buffer starting address.
 *  ring_buffer_mode:  0 - when ring buffer full, stop dmx getting section data. (for EPG)
 *					   1 - when ring buffer full, discard the oldest data. (for CA)
 *
 */
struct sie_request {
	//struct list_head list;
	UINT8 dmx_filter;
	UINT8 status;
	UINT16 max_sec_length;
	UINT16 reserved_len;
	UINT32 timeout;
	UINT32 read;
	UINT32 write;
	//struct list_head *target;
	struct get_section_param gsp;
	struct si_filter_t filter;
	struct si_filter_param fparam;
	UINT32 buf_len;
	UINT8 *buffer;
	UINT8 ring_buffer_mode;
};

/*
 * name		: sect_buffer
 * description	: ring buffer internal structure storing the meta information.
 * field	:
 *	hit_reason	: the hit reason demux provides, along with SIE reason.
 *	section_length	: the section length that follows.
 *	section		: the section content.
 * notice	: this structure is not packed, so some stuff byte might exist
 *		  between 2 sect_buffers.
 */
struct sect_buffer {
	UINT16 section_length;
	UINT16 hit_reason;
	UINT8 section[0];
};

/*
 * name		: sie_dmx_info
 * description	: the SI engine module parameters.
 * field	:
 *  mutex_task  : mutex of this sie task.
 *  sie_ring_mutex: ring buffer mutex only for ring_buffer_mode 1.
 *  mutex_cnt   : counter of mutex_task.
 *  sie_flag    : flag of this sie task.
 *	status		: the module status, inited, released, opened...
 *	filter		: the global filter status for demux device, only 32-way for maximum.
 *	request_nr	: the module's maximum concurrent request number, <= 32.
 *  addr        : request list addr.
 *	dmx		    : the demultiplex device handle.
 *  tmp_buffer  : when call external parser to parse a section of data,
 *              the parser is not mutex protected, so use a tmp_buffer to 
 *              avoid crash issue.
 *  task_id     : id of this sie task.
 */
struct sie_dmx_info {
	OSAL_ID mutex_task;
	OSAL_ID sie_ring_mutex;
	INT32 mutex_cnt;
	OSAL_ID sie_flag;
	UINT32 status;
	UINT32 filter;
	UINT32 request_nr;
	void *addr;
	struct dmx_device *dmx;
	UINT8 *tmp_buffer;
	OSAL_ID task_id;
};

/*
 * name		: dmx_poll
 * description	: the structure for dmx asynchronous filter polling.
 * field	:
 *	mask		: the mask bits the caller interested in.
 *	timeout		: the polling timeout value in mSecond.
 */
struct dmx_poll {
	UINT32 mask;
	UINT32 timeout;
};

/*
 * name		: si_filter_op_t
 * description	: a typedef of function proto-type for filter control operation.
 * parameter	: 1
 *	filter		: the filter to be operated.
 * return value	: INT32
 *	sie_started	: this filter will be put to active list.
 *	sie_stopped	: this filter will be put to idle list.
 *	sie_freed	: this filter will be put to free list. (reserved now)
 */
typedef INT32(*si_filter_op_t)(struct si_filter_t *, struct si_filter_param *);

static struct sie_dmx_info sie_info[MAX_DMX_NUM];

OSAL_ID sie_module_flag = OSAL_INVALID_ID;

/*only ring buffer mode 1 need use this mutex*/
static __inline__ sie_enter_ring_mutex(struct sie_dmx_info *info)
{
	UINT32 flag;

	osal_flag_wait(&flag, info->sie_flag, SIE_FLAG_RING_MUTEX, 
		OSAL_TWF_ANDW|OSAL_TWF_CLR, OSAL_WAIT_FOREVER_TIME);
}
static __inline__ sie_leave_ring_mutex(struct sie_dmx_info *info)
{
	osal_flag_set(info->sie_flag, SIE_FLAG_RING_MUTEX);
}

/*
 * name		: sie_enter_mutex
 * description	: a mutex interface for sie atomic operation.
 * parameter	: 0
 * return value	: -
 */
static __inline__ void sie_enter_mutex(struct sie_dmx_info *info)
{
	UINT32 flag;
	if (info->mutex_task != osal_task_get_current_id())
	{
		osal_flag_wait(&flag, info->sie_flag, SIE_FLAG_MUTEX, 
			OSAL_TWF_ANDW|OSAL_TWF_CLR, OSAL_WAIT_FOREVER_TIME);
		info->mutex_task = osal_task_get_current_id();
	} 
	osal_task_dispatch_off();
	++info->mutex_cnt;
	osal_task_dispatch_on();
}

/*
 * name		: sie_leave_mutex
 * description	: a mutex interface for sie atomic operation.
 * parameter	: 0
 * return value	: -
 */
static __inline__ void sie_leave_mutex(struct sie_dmx_info *info)
{
	osal_task_dispatch_off();
	if (--info->mutex_cnt==0)
	{
		info->mutex_task = OSAL_INVALID_ID;
		osal_flag_set(info->sie_flag, SIE_FLAG_MUTEX);
	}
	osal_task_dispatch_on();
}

/*
 * name		: dispatch_section
 * description	: dispatch received section data.
 * parameter	: 2
 *	request		: the request that receives the section data.
 * 	mask		: the current active filter mask.
 * return value	: sie_status_t
 *	sie_started	: the filter will be enabled.
 *	
 *	sie_freed	: the filter will be freed.
 */
static sie_status_t dispatch_section(struct sie_dmx_info *info, struct sie_request *request, UINT32 mask, UINT8 reason)
{
	INT32 ret = 0;
	struct sect_buffer *buffer=NULL;
	struct sie_request t_request;
	UINT16 section_len = 0;
	UINT16 hit_reason = 0;

	MEMCPY(&t_request, request, sizeof(struct sie_request));

	DBG_PRINTF("\n##%s():\n", __FUNCTION__);
	if(reason == SIE_REASON_FILTER_TIMEOUT)
	{
		request->status = SIE_FLT_RUNNING;
		sie_leave_mutex(info);
		ret = t_request.fparam.section_parser(t_request.gsp.pid,&t_request.filter,
				SIE_REASON_FILTER_TIMEOUT,NULL,0);
		SIE_PRINTF("##request filter[%d],pid[0x%x] time out,tick=%d!\n",request->dmx_filter,request->gsp.pid,osal_get_tick());
		sie_enter_mutex(info);
		if(request->status == SIE_FLT_RUNNING)
			request->status = SIE_FLT_ACTIVE;
		else
			return sie_freed;
	}
	else if((reason == SIE_REASON_FILTER_HIT)&&(request->read != request->write))
	{
		if (request->buf_len <= (request->reserved_len+sizeof(struct sect_buffer)+request->max_sec_length))
		{
			if (request->gsp.sec_tbl_len > SI_LONG_SECTION_LENGTH)
			{
#if (SIE_DEBUG_LEVEL > 0)
				SIE_PRINTF("%s: sec len %d > max length %d\n",__FUNCTION__,request->gsp.sec_tbl_len,SI_LONG_SECTION_LENGTH);
				ASSERT(0);
#endif
			}
			else
			{
				MEMCPY(info->tmp_buffer, request->gsp.buff, request->gsp.sec_tbl_len);
				request->status = SIE_FLT_RUNNING;
				sie_leave_mutex(info);
				ret = t_request.fparam.section_parser(t_request.gsp.pid,&t_request.filter,
					t_request.gsp.sec_hit_num, info->tmp_buffer,t_request.gsp.sec_tbl_len);
				sie_enter_mutex(info);			
				if(request->status == SIE_FLT_RUNNING)
					request->status = SIE_FLT_ACTIVE;
				else
					return sie_freed;
			}
			request->write = 0;
		} 
#ifndef SIE_MINIMUM_MODEL
		else
		{
			UINT16 section_cnt;

			if(request->ring_buffer_mode == 0)
				section_cnt = 2;
			else
				section_cnt = 1;
			
			if(request->ring_buffer_mode == 1)
				sie_enter_ring_mutex(info);

			/* here process data in ring buffer */
			while ((request->read!=request->write)&&(section_cnt)) 
			{
				buffer = (struct sect_buffer *)(request->buffer+request->read);
				DBG_PRINTF("    ****r: 0x%X, len: %d\n",request->read,buffer->section_length);
				if (buffer->section_length > SI_LONG_SECTION_LENGTH)
				{
#if (SIE_DEBUG_LEVEL > 0)
					RINGBUF_PRINTF("sie: sec len %d > max sec len %d!\n",buffer->section_length,SI_LONG_SECTION_LENGTH);
					ASSERT(0);
#endif
				}
				else
				{
					MEMCPY(info->tmp_buffer, buffer->section, buffer->section_length);
					section_len = buffer->section_length;
					hit_reason = buffer->hit_reason;

					request->status = SIE_FLT_RUNNING;
					sie_leave_mutex(info);

					//do NOT protect callback to avoid dead lock!
					ret = t_request.fparam.section_parser(t_request.gsp.pid,&t_request.filter,
								hit_reason, info->tmp_buffer,section_len);
				
					sie_enter_mutex(info);

					if(request->status == SIE_FLT_RUNNING)
						request->status = SIE_FLT_ACTIVE;
					else
					{
						if(t_request.ring_buffer_mode == 1)
							sie_leave_ring_mutex(info);
						return sie_freed;
					}
				}

				if (ret != sie_started)
				{
					RINGBUF_PRINTF("    ****ret: %d, request status: %d\n",ret,request->status);
					break;
				}

				request->read += ((3+sizeof(struct sect_buffer)+buffer->section_length)&0xFFFFFFFC);
				DBG_PRINTF("	****ring buffer read move to 0x%x!\n", request->read);
				--section_cnt;

				if ((request->read+sizeof(struct sect_buffer)+request->max_sec_length+request->reserved_len)>request->buf_len)
				{
					request->read = 0;
					RINGBUF_PRINTF("	****ring buffer read rewind to 0!\n");

					//rewind write!!!
					if (((buffer->hit_reason&SIE_REASON_RINGBUF_FULL) != 0)
						&& ((request->write+sizeof(struct sect_buffer)+request->max_sec_length+request->reserved_len)>request->buf_len))
					{
						RINGBUF_PRINTF("	&&&&ring buffer write rewind to 0!\n");
						request->write = 0;
					}
				}

				//sie_leave_mutex(info);

			}//end of while

			if(request->ring_buffer_mode == 1)
				sie_leave_ring_mutex(info);

			//read pointer != write, ringbuf still has sections not parsed, wake up sie to run
			if(request->read!=request->write)
			{
				dmx_io_control(info->dmx, DMX_WAKEUP_SIAE, 1);
			}
		} 
#endif		
	}
	
	//sie_enter_mutex(info);
	if ((sie_started==ret)&&((NULL==buffer)||(buffer->hit_reason&SIE_REASON_RINGBUF_FULL)))
	{
		if (request->status==SIE_FLT_ACTIVE)
		{
			dmx_io_control(info->dmx, IO_ASYNC_CLOSE, request->dmx_filter);
			info->filter &= (~(1<<request->dmx_filter));
			request->gsp.continue_get_sec = 1;
			if (request->fparam.timeout != OSAL_WAIT_FOREVER_TIME)
				request->timeout = osal_get_tick()+request->fparam.timeout;
			else
				request->timeout = OSAL_WAIT_FOREVER_TIME;

			if (dmx_async_req_section(info->dmx, &request->gsp, &request->dmx_filter)!=RET_SUCCESS)
			{
				SIE_PRINTF("	##request filter for pid[0x%x] fail!\n", request->gsp.pid);
				request->status = SIE_FLT_FREE;
				//sie_leave_mutex(info);
				return sie_freed;
			}
			
			//debug ring buffer only
			//if ((buffer != NULL) && (buffer->hit_reason&SIE_REASON_RINGBUF_FULL))
			{
				DBG_PRINTF("	##request filter[%d],pid[0x%x],timeout[%d] continue to get section!\n",request->dmx_filter,
					request->gsp.pid, request->timeout);
			}
			info->filter |= (1<<request->dmx_filter);
		}
		
	}
	else if ((ret==sie_freed)&&(request->status==SIE_FLT_ACTIVE))
	{
		dmx_io_control(info->dmx, IO_ASYNC_CLOSE, request->dmx_filter);
		SIE_PRINTF("##filter[%d],pid[0x%x] freed!\n",request->dmx_filter,request->gsp.pid);
		info->filter &= (~(1<<request->dmx_filter));
		request->status = SIE_FLT_FREE;
	}
	//sie_leave_mutex(info);
	return ret;
}

/*
 * name		: inspect_active
 * description	: check each active request's filter condition.
 * parameter	: 
 *	
 * return value	: UINT32
 *	time interval to the next wake up in mSecond.
 */
static UINT32 inspect_active(struct sie_dmx_info *info)
{
	UINT32 cnt, ret;
	struct sie_request *request;
	UINT32 value, tick;
	UINT32 interval = OSAL_WAIT_FOREVER_TIME;
	sie_status_t status;
	struct dmx_poll poll;
	UINT8 dispatch_flag, i;
	UINT8 dispatch_reason = 0;

	poll.timeout = 0;
	cnt = 0;
	DBG_PRINTF("\ninspect_active():mask=0x%x,tick=%d!\n",info->filter,osal_get_tick());
	request = (struct sie_request *)info->addr;

	for(i = 0;i < info->request_nr; i++)
	{
		sie_enter_mutex(info);
		if((request[i].status==SIE_FLT_ACTIVE )&&(info->status == SIE_STATUS_OPENED))
		{	
			dispatch_flag = 1;
			poll.mask = 1<<request[i].dmx_filter;
			value = dmx_io_control(info->dmx, IO_ASYNC_POLL, (UINT32)&poll);
			cnt++;
			//timeout
			if(request[i].timeout <= osal_get_tick())
			{
				dispatch_reason = SIE_REASON_FILTER_TIMEOUT;
			}
			/*section hit.for ring buf mode, can not poll the section hit status,
			must rely on read and write pointer.			*/
			else if((poll.mask&value)||(request[i].read!=request[i].write))
			{
				dispatch_reason = SIE_REASON_FILTER_HIT;
			}
			//filter not hit, and not timeout
			else
			{
				if ((request[i].timeout!=OSAL_WAIT_FOREVER_TIME)&&(request[i].timeout<interval))
					interval = request[i].timeout;
							
				dispatch_flag = 0;
			}
				
			//filter hit or time out, dispatch
			if(dispatch_flag == 1)
			{
				//sie_leave_mutex(info);
				status = dispatch_section(info, &request[i], poll.mask, dispatch_reason);
				//sie_enter_mutex(info);
				if ((sie_started == status)&&(request[i].status==SIE_FLT_ACTIVE))
				{
					//find the minimum timeout(it's time point and not interval)
					if ((request[i].timeout!=OSAL_WAIT_FOREVER_TIME)&&(request[i].timeout<interval))
						interval = request[i].timeout;
				} 				
			}
						
		}

		sie_leave_mutex(info);
		
	}
	DBG_PRINTF("	poll fiter cnt=%d!\n", cnt);
		
	if(interval==OSAL_WAIT_FOREVER_TIME)
		return interval;
	tick = osal_get_tick();
	ret = (interval>tick)? interval-tick : 0;
	DBG_PRINTF("	tick=%d, of all active filters min timeout=%d,next poll interval=%d!\n", tick, interval,ret);
	return ret;
}

/*
 * name		: sie_main
 * description	: the main SI engine work flow function.
 * parameter	: -
 * return value	: -
 */
static void sie_main(struct sie_dmx_info *info)
{
	UINT32 filter, flag;
	struct dmx_poll poll;
	UINT8 i;
	struct sie_request *request = NULL;

	poll.mask = 0xFFFFFFE0;
	poll.timeout = OSAL_WAIT_FOREVER_TIME;

	while(1) {
		//task will suspend here until request hit, timeout, or DMX_WAKEUP_SIAE set.
		filter = dmx_io_control(info->dmx, IO_ASYNC_POLL, (UINT32)(&poll));
		/* clear up the trigger, since the task is waked up. */
		dmx_io_control(info->dmx, DMX_WAKEUP_SIAE, 0);
		
		if (osal_flag_wait(&flag, info->sie_flag, SIE_FLAG_ENGINE_CLOSING, 
			OSAL_TWF_ANDW, 0)==OSAL_E_OK)
		{
			sie_enter_mutex(info);
			request = (struct sie_request *)info->addr;
			for(i = 0;i < info->request_nr; i++)
			{
				if(request[i].status==SIE_FLT_ACTIVE)
					dmx_io_control(info->dmx, IO_ASYNC_CLOSE, request[i].dmx_filter);
			}
			info->filter = 0;
			sie_leave_mutex(info);
			break;
		}
		//remain time		
		poll.timeout = inspect_active(info);
		//get newest active filter bitmap, because new filter maybe enabled during inspect_active.
		poll.mask = info->filter;
		osal_task_sleep(1);
	}
	osal_flag_clear(info->sie_flag, SIE_FLAG_ENGINE_CLOSING);
	osal_flag_set(info->sie_flag, SIE_FLAG_ENGINE_CLOSED);
}

/*
 * name		: sie_task
 * description	: the SI engine task function.
 * parameter	: 2
 *	param1		: dummy parameter.
 *	param2		: dummy parameter.
 * return value	: -
 */
static void sie_task(UINT32 param1, UINT32 param2)
{
	UINT32 flag;
	param2 = 0; /* to make gcc happy */

	while(1)
	{
		//wait main task to enable the task
		osal_flag_wait(&flag, sie_info[param1].sie_flag,SIE_FLAG_ENGINE_ENABLE, 
			OSAL_TWF_ORW, OSAL_WAIT_FOREVER_TIME);

		//clear this flag
		osal_flag_clear(sie_info[param1].sie_flag, SIE_FLAG_ENGINE_ENABLE);

		//enter main function
		sie_main(&sie_info[param1]);
	}
}


/*
 * name		: init_requests
 * description	: set sie requests status free, if buffer alloced,set each request's buffer ..
 * parameter	: 2
 *	addr		: the address to generate sie request.
 *	nr		: the request number.
 *
 * return value : -
 */
static void init_requests(void *addr, void *buffer, INT32 nr)
{
	INT32 i;
	struct sie_request *req = (struct sie_request *)addr;
	for(i=0; i<nr; i++)
	{
		req->status = SIE_FLT_FREE;
		if (buffer != NULL)
		{
			req->buffer = buffer+SI_SHORT_SECTION_LENGTH*i;
		}
		++req;
	}
}

/* 
 * name		: sie_create_task
 * description	: SI engine module initialization function
 * parameter	: -
 * return value	: INT32
 *	SI_SUCCESS	: the module is initialized successfully.
 *	other		: module couldn't be inited or has already initialized.
 */
static ID_THREAD sie_create_task(INT32 dmx_idx)
{
	OSAL_T_CTSK task;
	ID_THREAD task_id;
	if (sie_info[dmx_idx].sie_flag == OSAL_INVALID_ID)
	{
		sie_info[dmx_idx].sie_flag = osal_flag_create(SIE_FLAG_MUTEX|SIE_FLAG_RING_MUTEX);
		if (sie_info[dmx_idx].sie_flag == OSAL_INVALID_ID)
		{
			SIE_PRINTF("%s: flag create failure!\n", __FUNCTION__);
			return OSAL_INVALID_ID;
		}
	}

	sie_info[dmx_idx].mutex_task = OSAL_INVALID_ID;
	sie_info[dmx_idx].mutex_cnt = 0;
#if SIE_DEBUG_LEVEL>0
	task.name[0] = 'S';
	task.name[1] = 'I';
	task.name[2] = 'E';
#endif
	task.itskpri = SIE_MODULE_TASK_PRIORITY;
	task.task = sie_task;
	task.quantum = SIE_MODULE_TASK_QUANTUM;
	task.stksz = SIE_MODULE_TASK_STACKSIZE;
	task.para1 = dmx_idx;

	task_id = osal_task_create(&task);
	if (task_id==OSAL_INVALID_ID)
	{
		osal_flag_delete(sie_info[dmx_idx].sie_flag);
		sie_info[dmx_idx].sie_flag = OSAL_INVALID_ID;
		SIE_PRINTF("%s: task create failure!\n", __FUNCTION__);
		return OSAL_INVALID_ID;
	}

	sie_info[dmx_idx].task_id = task_id;
	sie_info[dmx_idx].status = SIE_STATUS_INITED;
		
	return task_id; 
}

/*
 * name		: sie_open_dmx
 * description	: the SI engine opening function
 * parameter	: 4
 *	dmx		: the demux device handle.
 *	max_filter_nr	: the maximum filter to be used.
 * 	buffer		: buffer for the SI engine requests space.
 *	buflen		: length of the buffer.
 * return value	: INT32
 *	SI_SUCCESS	: the engine is opened properly.
 *	other		: the status of the SI engine module is not correct.
 *			  or resource not permits.
 * comment	:
 *	when buffer is not provided, it will allocated memory by itself.
 */
INT32 sie_open_dmx(struct dmx_device *dmx, INT32 max_filter_nr, UINT8 *buffer, INT32 buflen)
{
	INT32 i;
	UINT32 flag;
	ID_THREAD task_id = OSAL_INVALID_ID;
	if ((dmx == NULL)||(max_filter_nr<=0)||(buflen<0))
	{
		SIE_PRINTF("%s: parameter invalid!\n", __FUNCTION__);
		return SI_EINVAL;
	}
	if ((buffer != NULL)&&(buflen<max_filter_nr*SI_SHORT_SECTION_LENGTH))
	{
		SIE_PRINTF("%s: buffer length not enough!\n", __FUNCTION__);
		return SI_EINVAL;
	}

	//get index of dmx
	i = (dmx->type)&HLD_DEV_ID_MASK;
	ASSERT(i < MAX_DMX_NUM);

	///////////protect from two tasks enter here////////////
	osal_flag_wait(&flag, sie_module_flag, SIE_FLAG_MUTEX, 
			OSAL_TWF_ANDW|OSAL_TWF_CLR, OSAL_WAIT_FOREVER_TIME);
	if(SIE_STATUS_RELEASED == sie_info[i].status)
	{
		//create SIE task
		task_id = sie_create_task(i);
		sie_info[i].tmp_buffer = MALLOC(SI_LONG_SECTION_LENGTH);
		if(NULL == sie_info[i].tmp_buffer)
		{
			///////////protect from two tasks enter here////////////
			osal_flag_set(sie_module_flag, SIE_FLAG_MUTEX);
			return SI_EINVAL;
		}
	}
	
	if(SIE_STATUS_INITED == sie_info[i].status)
	{
		sie_enter_mutex(&sie_info[i]);
		//initiate parameters
		sie_info[i].dmx = dmx;
		sie_info[i].request_nr = max_filter_nr;		
		sie_info[i].addr = MALLOC(max_filter_nr*sizeof(struct sie_request));
		if (sie_info[i].addr == NULL)
		{
			sie_leave_mutex(&sie_info[i]);
			SIE_PRINTF("%s: memory alloc failure!\n", __FUNCTION__);
			///////////protect from two tasks enter here////////////
			osal_flag_set(sie_module_flag, SIE_FLAG_MUTEX);
			return SI_EINVAL;
		}
		
		MEMSET(sie_info[i].addr, 0, max_filter_nr*sizeof(struct sie_request));
		init_requests(sie_info[i].addr, buffer, max_filter_nr);
		sie_info[i].status = SIE_STATUS_OPENED;
		sie_leave_mutex(&sie_info[i]);
		//enable SIE task
		osal_flag_set(sie_info[i].sie_flag, SIE_FLAG_ENGINE_ENABLE);
		///////////protect from two tasks enter here////////////
		osal_flag_set(sie_module_flag, SIE_FLAG_MUTEX);
		return SI_SUCCESS;
	} 
	else
	{
		SIE_PRINTF("%s: creating SIE task failed!\n", __FUNCTION__);
		///////////protect from two tasks enter here////////////
		osal_flag_set(sie_module_flag, SIE_FLAG_MUTEX);
		return SI_EINVAL;
	}
}

/*
 * name		: sie_close_dmx
 * description	: Close SI engine.
 * parameter	: -
 * return value	: INT32
 *	SI_SUCCESS	: the SI engine is closed.
 *	SI_EINVAL	: the SI engine is not in open status.
 */
INT32 sie_close_dmx(struct dmx_device *dmx)
{
	UINT32 flag;
	INT32 i;

	if (dmx == NULL)
	{
		SIE_PRINTF("%s: dmx is NULL!\n", __FUNCTION__);
		return SI_EINVAL;
	}

	i = (dmx->type)&HLD_DEV_ID_MASK;
	ASSERT(i < MAX_DMX_NUM);

	sie_enter_mutex(&sie_info[i]);
	if (SIE_STATUS_OPENED == sie_info[i].status)
	{
		sie_info[i].status = SIE_STATUS_INITED;	//set status to closed mode
		sie_leave_mutex(&sie_info[i]);
		//inform SIE task to close all filters on the DMX and then release SIE task
		osal_flag_set(sie_info[i].sie_flag, SIE_FLAG_ENGINE_CLOSING);
		dmx_io_control(sie_info[i].dmx, DMX_WAKEUP_SIAE, 1);
		//wait until all filters are closed
		osal_flag_wait(&flag, sie_info[i].sie_flag, SIE_FLAG_ENGINE_CLOSED,OSAL_TWF_ANDW, OSAL_WAIT_FOREVER_TIME);
		//free resource
		if (sie_info[i].addr)
		{
			FREE(sie_info[i].addr);
			sie_info[i].addr = NULL;
		}
		
		osal_flag_clear(sie_info[i].sie_flag, SIE_FLAG_ENGINE_CLOSED);
		return SI_SUCCESS;
	} 
	else
	{
		sie_leave_mutex(&sie_info[i]);
		SIE_PRINTF("%s: module not in SIE_STAT_OPENED stat!\n", __FUNCTION__);
		return SI_EINVAL;
	}
}

/* 
 * name		: sie_module_init (multi demux version)
 * description	: SI engine module initialization function
 * parameter	: -
 * return value	: INT32
 *	SI_SUCCESS	: the module is initialized successfully.
 *	other		: module couldn't be inited or has already initialized.
 */
INT32 sie_module_init()
{
	INT32 i;

	if (sie_module_flag == OSAL_INVALID_ID)
	{
		sie_module_flag = osal_flag_create(SIE_FLAG_MUTEX);
		if (sie_module_flag == OSAL_INVALID_ID)
		{
			SIE_PRINTF("%s: flag create failure!\n", __FUNCTION__);
			return SI_SBREAK;
		}
	}
	
	for(i=0; i<MAX_DMX_NUM; i++)
	{
		sie_info[i].mutex_task = OSAL_INVALID_ID;
		sie_info[i].sie_ring_mutex = OSAL_INVALID_ID;
		sie_info[i].mutex_cnt = 0;
		sie_info[i].sie_flag = OSAL_INVALID_ID;
		sie_info[i].status = SIE_STATUS_RELEASED;
	}

	return SI_SUCCESS;
}

/*
 * name		: on_dmx_event
 * description	: the generic dmx filter hit response function.
 * parameter	: 1
 *	gsp		: the dmx parameter for this filter.
 * return value : -
 */
static void on_dmx_event(struct get_section_param *gsp)
{
	UINT32 hit_reason;
	struct sie_request *request;
#ifndef SIE_MINIMUM_MODEL
	BOOL ring_buffer_full = FALSE;
	struct sect_buffer *buffer = NULL;
#endif
	request = list_entry(gsp, struct sie_request, gsp);

	/*
	 * for non-async request status not started, it will be just ignored
	 */
	if (request->status!= SIE_FLT_ACTIVE
		&& request->status!= SIE_FLT_RUNNING)
	{
		/* this filter is no longer active. */
		gsp->continue_get_sec = 0;
		SIE_PRINTF("%s: entering a non-active filter!\n", __FUNCTION__);
		return;
	} 
	/*
	 * check CRC for those needed.
	 */
	hit_reason = gsp->sec_hit_num;
	if (request->fparam.attr[hit_reason]&SI_ATTR_HAVE_CRC32)
	{
		if (MG_FCS_Decoder(gsp->buff, gsp->sec_tbl_len) != 0)
		{
			gsp->continue_get_sec = 1;
			SIE_PRINTF("%s: failed to pass CRC32!\n",__FUNCTION__);
			return;
		}
	}
	/*
	 *1.call the filter's on_event if there is one,
	 *  it will return the recording or non-recording selection,
	 *  otherwise, regard it as a recording reply.
	 *
	 *2.for filter without pipeline buffer, a recording reply
	 *  will convert the filter into idle status and trigger the callback.
	 *  for filter with pipeline buffer, a record reply will be put into
	 *  the pipeline buffer and wakeup the SI egine to examine the data.
	 *  for pipeline buffer that are space-full, it is regarded as a 
	 *  conventional filter.
	 */
	DBG_PRINTF("\n****%s(): get section pid[0x%x]!tick=%d!\n", __FUNCTION__,gsp->pid,osal_get_tick());
	if (request->fparam.section_event==NULL)
	{
		gsp->continue_get_sec = 0;
	} 
	else
	{
		if (FALSE == request->fparam.section_event(gsp->pid,&request->filter,
			hit_reason, gsp->buff,gsp->sec_tbl_len))
			return;
	}

#ifndef SIE_MINIMUM_MODEL
	if (request->buf_len>(request->reserved_len+request->max_sec_length+sizeof(struct sect_buffer)))
	{
		/* figure out the current section buffer */
		buffer = (struct sect_buffer *)(request->buffer+request->write);
		buffer->hit_reason = (UINT8)hit_reason;
		buffer->section_length = gsp->sec_tbl_len;
		if (buffer->section_length > SI_LONG_SECTION_LENGTH)
		{
#if (SIE_DEBUG_LEVEL > 0)
			SIE_PRINTF("%s: sec len %d > max length %d\n",__FUNCTION__,buffer->section_length,SI_LONG_SECTION_LENGTH);
			ASSERT(0);
#endif
			return;
		}

		//copy section from dmx working buff
		//check buffer boundary
		MEMCPY((UINT8 *)(request->buffer+request->write+sizeof(struct sect_buffer)), gsp->buff, buffer->section_length);

		/* 4 byte aligned for the next sect_buffer */
		request->write += ((3+sizeof(struct sect_buffer)+buffer->section_length)&0xFFFFFFFC);
		DBG_PRINTF("	~~~~ring buffer current write=0x%x!\n", request->write);

		/* rewind control */
		if ((request->write+sizeof(struct sect_buffer)+request->max_sec_length+request->reserved_len)>request->buf_len)
		{
			if (request->read>0)
			{
				request->write = 0;
				RINGBUF_PRINTF("	~~~~ring buffer write rewind to 0!\n");
			}
			else //read =0
			{
				//ring buffer full
				//1. stop dmx, 2. let dispatch process read, 3. need rewind at dispatch_section()
				ring_buffer_full = TRUE;
				RINGBUF_PRINTF("    ~~~~read = 0 when write rewind!!!\n");
			}
		}
		
		if (ring_buffer_full 
			|| ((request->write<request->read)
				&&((request->write+sizeof(struct sect_buffer)+request->max_sec_length)>=request->read)))
		{
			/*
			 * no space for the next section arriving, 
			 * so this filter will be closed and wait 
			 * for read pointer to proceed! 
			 */
			if(request->ring_buffer_mode == 0)
			{
				RINGBUF_PRINTF("	~~~~ring buffer full,stop filter[%d] for pid[0x%x]!\n", request->dmx_filter,request->gsp.pid);
				buffer->hit_reason |= SIE_REASON_RINGBUF_FULL;
				gsp->continue_get_sec = 0;
				dmx_io_control(request->filter.dmx, DMX_WAKEUP_SIAE, 1);
			}
			else
			{
			 	RINGBUF_PRINTF("    ~~~~buf full\n");
				INT32 discard_length = 0;
				INT32 dmx_idx = request->filter.dmx->type&HLD_DEV_ID_MASK;

				sie_enter_ring_mutex(&sie_info[dmx_idx]);
			
				while (discard_length < request->max_sec_length)
				{
					buffer = (struct sect_buffer *)(request->buffer+request->read);
					RINGBUF_PRINTF("	~~~~ring buffer current read=0x%x!\n", request->read);

					//need protect this line of code
					request->read += ((3+sizeof(struct sect_buffer)+buffer->section_length)&0xFFFFFFFC);

					discard_length += ((3+sizeof(struct sect_buffer)+buffer->section_length)&0xFFFFFFFC);

					RINGBUF_PRINTF("	~~~~ring buffer read move to 0x%x!\n", request->read);
					if ((request->read+sizeof(struct sect_buffer)+request->max_sec_length+request->reserved_len)>request->buf_len)
					{
						request->read = 0;
						RINGBUF_PRINTF("	~~~~ring buffer read rewind to 0!\n");
					}
				}

				if ((request->write+sizeof(struct sect_buffer)+request->max_sec_length+request->reserved_len)>request->buf_len)
				{
					//request->read shall not be zero!
					request->write = 0;
					RINGBUF_PRINTF("	~~~~ring buffer write rewind to 0!\n");
				}
				sie_leave_ring_mutex(&sie_info[dmx_idx]);
				
				dmx_io_control(request->filter.dmx, DMX_WAKEUP_SIAE, 1);
			 }
		} 
		else
		{
//			buffer = (struct sect_buffer *)(request->buffer+request->write);
			
			dmx_io_control(request->filter.dmx, DMX_WAKEUP_SIAE, 1);
		}
	} 
	else 
#endif	
	{
		request->write = gsp->sec_tbl_len;
		gsp->continue_get_sec = 0;
	}

}


/*
 * name		: __alloc_filter
 * description	: allocate a filter from free request list.
 * parameter	: 4
 *	pid		: the TS PID value, should be in range [0, 8191) in DVB.
 *	buffer		: the buffer to hold section(s).
 *	length		: total buffer length.
 *	max_sec_length	: maximum section length by syntax.
 * return value	: struct si_filter_t *
 *	!NULL		: the filter pointer.
 *	NULL		: can not assign new filter.
 * comment	: 
 * 	1. length >= max_sec_length.
 *	2. typical max_sec_length: 1024 or 4096.
 */
static struct si_filter_t *__alloc_filter(struct sie_dmx_info *info, UINT16 pid, UINT8 *buffer, UINT32 length, 
	UINT16 max_sec_length, UINT8 mode)
{
	struct sie_request *req = (struct sie_request *)info->addr;
	struct sie_request *request = NULL;
	UINT8 *addr = NULL;
	UINT8 i,flag;
	
	flag = 0;
	//check PID of request, no duplicate PID is allowed.
	for(i = 0;i < info->request_nr; i++)
	{
		if((req[i].status == SIE_FLT_FREE) && (flag==0))
		{
			request = &req[i];
			flag = 1;
		}
#ifndef DUPLICATE_PID_SUPPORT
		if(((req[i].status==SIE_FLT_ACTIVE)||(req[i].status == SIE_FLT_RUNNING)) && (req[i].gsp.pid==pid))
		{
			SIE_PRINTF("%s(): same pid 0x%x already active!\n", __FUNCTION__, pid);
			return NULL;
		}
#endif
	}

	if (NULL == request)
	{
		SIE_PRINTF("%s(): no free request left!\n", __FUNCTION__);
		return NULL;
	}

	if (buffer != NULL)
	{
		MEMSET(request, 0, sizeof(struct sie_request));
		request->buffer = buffer;
		request->reserved_len = max_sec_length;
#ifndef SIE_MINIMUM_MODEL
		//request buf long enough, use ring buf mode
		if(length > (request->reserved_len+max_sec_length+sizeof(struct sect_buffer)))	
		{
			addr = buffer+length-request->reserved_len;
		}
		else 
#endif
		{
			addr = buffer;
		}	
	}
	//in search mode, each request allocated 1024 byte buffer from vbv
	else 
	{
		if(request->buffer == NULL)
		{
			SIE_PRINTF("%s(): request buf NULL!\n", __FUNCTION__);
			return NULL;
		}
		addr = request->buffer;
		MEMSET(request, 0, sizeof(struct sie_request));
		request->buffer = addr;
		request->reserved_len = max_sec_length;
	}

	request->max_sec_length = max_sec_length;
	request->status = SIE_FLT_IDLE;
	request->buf_len = length;
	request->read = request->write = 0;
	request->filter.dmx = info->dmx;
	request->ring_buffer_mode = mode;
	struct get_section_param *gsp=&request->gsp;
	
	gsp->pid = pid;
	gsp->buff = addr;
	gsp->buff_len = request->max_sec_length;
	gsp->continue_get_sec = 1;
	gsp->mask_value = &request->fparam.mask_value;
	gsp->get_sec_cb = on_dmx_event;
	
	return &request->filter;
}

/*
 * name		: filter_sanity_check
 * description	: check the sanity of the filter
 * parameter	: 2
 *	filter		: the filter to be examined.
 *	check_status	: the linked list the filter should be in.
 * return value	: struct sie_request
 *	true		: the request associated with the filter.
 *	false		: the filter is not valid.
 */
static BOOL filter_sanity_check(struct si_filter_t *filter, UINT8 check_status)
{
	struct sie_request *request;
	
	request = list_entry(filter, struct sie_request, filter);
	if(request->status & check_status)
		return TRUE;
	else
		return FALSE;
}


static INT32 __config_filter(struct si_filter_t *filter, struct si_filter_param *fparam)
{
	struct sie_request *request;
		
	if(FALSE==filter_sanity_check(filter, SIE_FLT_IDLE|SIE_FLT_ACTIVE))
	{
		SIE_PRINTF("%s: filter not in active/idle status!\n",__FUNCTION__);
		return SI_EINVAL;
	}
	request = list_entry(filter, struct sie_request, filter);
	MEMCPY(&request->fparam, fparam, sizeof(struct si_filter_param));
	return SI_SUCCESS;
}

static INT32 __copy_filter(struct si_filter_t *filter, struct si_filter_param *fparam)
{
	struct sie_request *request;
	
	if(FALSE==filter_sanity_check(filter, SIE_FLT_IDLE|SIE_FLT_ACTIVE))
	{
		SIE_PRINTF("%s: filter not in active/idle status!\n",__FUNCTION__);
		return SI_EINVAL;
	}
	request = list_entry(filter, struct sie_request, filter);
	MEMCPY(fparam, &request->fparam, sizeof(struct si_filter_param));
	return SI_SUCCESS;
}

/*
 * name		: __enable_filter
 * description	: enable the given filter on demux.
 * parameter	: 1
 *	filter		: the filter to be enabled.
 * return value	: INT32
 *	SI_SUCCESS	: the filter is enabled.
 *	other		: the filter could not be enabled.
 */
static INT32 __enable_filter(struct si_filter_t *filter, struct si_filter_param *fparam)
{
	struct sie_request *request = NULL;
	struct dmx_device *dmx;
	UINT32 dmx_idx;
	
	if(FALSE==filter_sanity_check(filter, SIE_FLT_IDLE))
	{
		SIE_PRINTF("%s(): filter for pid 0x%x not in idle status!\n",__FUNCTION__,request->gsp.pid);
		return SI_EINVAL;
	}
	
	request = list_entry(filter, struct sie_request, filter);
	dmx = filter->dmx;
	dmx_idx = (dmx->type)&HLD_DEV_ID_MASK;
	
	if (request->fparam.timeout != OSAL_WAIT_FOREVER_TIME)
	{
		request->timeout = request->fparam.timeout+osal_get_tick();
	}
	 else
	{
		request->timeout = OSAL_WAIT_FOREVER_TIME;
	}
	request->gsp.wai_flg_dly = OSAL_WAIT_FOREVER_TIME;	
	request->status = SIE_FLT_ACTIVE;
	if(RET_SUCCESS!=dmx_async_req_section(dmx, &request->gsp, &request->dmx_filter))
	{
		SIE_PRINTF("%s(): req dmx filter for pid 0x%x failed!\n",__FUNCTION__,request->gsp.pid);
		request->status = SIE_FLT_FREE;
		MEMSET(&request->gsp, 0, sizeof(struct get_section_param));
		return SI_EINVAL;
	}

	sie_info[dmx_idx].filter |= 1<<request->dmx_filter;
	SIE_PRINTF("\n%s(): filter[%d] for pid[0x%x], timeout=%d, tick=%d!\n",__FUNCTION__, request->dmx_filter, 
		request->gsp.pid, request->timeout,osal_get_tick());
	dmx_io_control(dmx, DMX_WAKEUP_SIAE, 1);

	return SI_SUCCESS;
}


#ifndef SIE_MINIMUM_MODEL
static INT32 __get_pid(struct si_filter_t *filter, struct si_filter_param *param)
{
	struct sie_request *request;
	if (filter != NULL)
	{
		request = list_entry(filter, struct sie_request, filter);
		return request->gsp.pid;
	} 
	else
		return 0x1FFF;
}
#endif

/*
 * name		: filter_operation
 * description	: perform the function handle on the target filter.
 * parameter	: 2
 *	operation	: the function handle.
 *	filter		: the target filter.
 * return value	: INT32
 *	SI_SUCCESS	: the operations is successful.
 *	other		: the filter is not valid or could not perform such op.
 */
static INT32 filter_operation(si_filter_op_t operation, struct si_filter_t *filter,
	struct si_filter_param *fparam)
{
	INT32 ret;
	UINT32 flag;
	UINT32 dmx_idx;
	
	dmx_idx = (filter->dmx->type)&HLD_DEV_ID_MASK;
	if (SIE_STATUS_OPENED != sie_info[dmx_idx].status)
	{
		SIE_PRINTF("%s: module not in SIE_STATUS_OPENED stat!\n",__FUNCTION__);
		return SI_EINVAL;
	}
	sie_enter_mutex(&sie_info[dmx_idx]);
	if (osal_flag_wait(&flag, sie_info[dmx_idx].sie_flag,SIE_FLAG_ENGINE_CLOSED|SIE_FLAG_ENGINE_CLOSING,
		OSAL_TWF_ORW, 0) != OSAL_E_OK)
		ret = operation(filter, fparam);
	else
		ret = SI_UBREAK;
	
	sie_leave_mutex(&sie_info[dmx_idx]);
	return ret;
}


/*
 * name		: sie_alloc_filter_ext
 * description	: create a filter with given parameter
 * parameter	: 4
 *	pid		: the TS PID for this filter.
 *	buffer		: the buffer pointer.
 *	length		: total buffer memory size. for ring buffer, need length > 3*max_sec_length.
 *	max_sec_length	: syntax section maximum length, should <= length.
 *  mode		: ring buffer mode,
 *				0 - when ring buffer full, stop dmx getting section data. (for EPG)
 *				1 - when ring buffer full, discard the oldest data. (for CA)
 * return value		: struct si_filter_t
 *	!NULL		: the filter allocated.
 *	NULL		: the filter could not be allocated.
 */
struct si_filter_t *sie_alloc_filter_ext(struct dmx_device *dmx, UINT16 pid, UINT8 *buffer, UINT32 length, 
	UINT16 max_sec_length, UINT8 mode)
{
	struct si_filter_t *ret;
	UINT32 flag;
	INT32 i;

	ASSERT(dmx != NULL);
	i = (dmx->type)&HLD_DEV_ID_MASK;
	ASSERT(i < MAX_DMX_NUM);
	
	if (SIE_STATUS_OPENED != sie_info[i].status)
	{
		SIE_PRINTF("%s: module not in SIE_STATUS_OPENED stat!\n",__FUNCTION__);
		return NULL;
	}
	else if (length<max_sec_length)
	{
		SIE_PRINTF("%s: insufficent buffer length %x!\n", __FUNCTION__,length);
		return NULL;
	} 
	
	sie_enter_mutex(&sie_info[i]);
	if (osal_flag_wait(&flag, sie_info[i].sie_flag,SIE_FLAG_ENGINE_CLOSED|SIE_FLAG_ENGINE_CLOSING, 
		OSAL_TWF_ORW, 0)!=OSAL_E_OK)
		ret = __alloc_filter(&sie_info[i], pid, buffer, length, max_sec_length, mode);
	 else
		ret = NULL;
	
	sie_leave_mutex(&sie_info[i]);
	
	return ret;
}


/*
 * name		: sie_enable_filter
 * description	: convert an in-active filter into active status.
 * parameter	: 1
 *	filter		: the filter to be enabled.
 * return value	: INT32
 *	SI_SUCCESS	: the filter is enabled;
 *	other		: the filter is not enabled.
 */
INT32 sie_enable_filter(struct si_filter_t *filter)
{
	return filter_operation(__enable_filter, filter, NULL);
}

static INT32 _sie_abort(INT32 dmx_idx, UINT8 **buff, UINT16 pid, struct restrict *mask_value)
{
	UINT8 i;
	struct sie_request *request;

	sie_enter_mutex(&sie_info[dmx_idx]);
	request = (struct sie_request *)sie_info[dmx_idx].addr;
	for(i = 0;i < sie_info[dmx_idx].request_nr; i++)
	{
		if(request[i].status==SIE_FLT_ACTIVE || request[i].status==SIE_FLT_RUNNING)
		{
			if((pid == request[i].gsp.pid)&&((mask_value==NULL)||
				(0==MEMCMP(mask_value, &(request[i].fparam.mask_value), sizeof(struct restrict)))))
			{				
				dmx_io_control(sie_info[dmx_idx].dmx, IO_ASYNC_CLOSE, request[i].dmx_filter);
				sie_info[dmx_idx].filter &= (~(1<<request[i].dmx_filter));
				SIE_PRINTF("%s: flt[%d] for pid[0x%x] closed!\n",__FUNCTION__,request[i].dmx_filter,request[i].gsp.pid);
				SIE_PRINTF("%s: r %d, w %d\n",__FUNCTION__,request[i].read,request[i].write);

				request[i].status = SIE_FLT_FREE;

				MEMSET(&request[i].gsp, 0, sizeof(struct get_section_param));
				if(buff)
					*buff = request[i].buffer;
				break;
			}
		}
	}
	
	sie_leave_mutex(&sie_info[dmx_idx]);
	return SI_SUCCESS;
}

INT32 sie_abort_ext(struct dmx_device *dmx, UINT8 **buff, UINT16 pid, struct restrict *mask_value)
{
	INT32 dmx_idx;

	if (dmx == NULL)
	{
		SIE_PRINTF("%s: dmx is NULL!\n",__FUNCTION__);
		return SI_EINVAL;
	}

	dmx_idx = (dmx->type)&HLD_DEV_ID_MASK;
	ASSERT(dmx_idx < MAX_DMX_NUM);
	
	if (SIE_STATUS_OPENED != sie_info[dmx_idx].status)
	{
		SIE_PRINTF("%s: module not in SIE_STATUS_OPENED stat!\n",__FUNCTION__);
		return SI_EINVAL;
	}

	return _sie_abort(dmx_idx, buff, pid, mask_value);
}


#ifndef SIE_MINIMUM_MODEL
INT32 sie_query_stat(struct dmx_device *dmx)
{
	INT32 i;

	ASSERT(dmx != NULL);
	i = (dmx->type)&HLD_DEV_ID_MASK;
	
	return sie_info[i].status;
}

INT32 sie_get_pid(struct si_filter_t *filter)
{
	return filter_operation(__get_pid, filter, NULL);
}
#endif

INT32 sie_config_filter(struct si_filter_t *filter,struct si_filter_param *fparam)
{
	if ((!fparam)||(!fparam->section_parser))
		return SI_EINVAL;
	else
		return filter_operation(__config_filter, filter, fparam);
}

INT32 sie_copy_config(struct si_filter_t *filter,struct si_filter_param *fparam)
{
	if (!fparam)
		return SI_EINVAL;
	else
		return filter_operation(__copy_filter, filter, fparam);
}

/************************************** old api ************************************************
*                                                                                                                                                                *
*                     These functions are used for being compatible with old API                                                       *
*                                                                                                                                                                *
*********************************************************************************************/

INT32 sie_open(struct dmx_device *dmx, INT32 max_filter_nr, UINT8 *buffer, INT32 buflen)
{
	return sie_open_dmx(dmx, max_filter_nr, buffer, buflen);
}

/* close all opened dmx */
INT32 sie_close()
{
	UINT32 flag;
	INT32 i;
	
	for(i=0; i<MAX_DMX_NUM; i++)
	{	
		sie_enter_mutex(&sie_info[i]);
		if (SIE_STATUS_OPENED == sie_info[i].status)
		{
			sie_info[i].status = SIE_STATUS_INITED;	//set status to closed mode			
			//sie_leave_mutex(&sie_info[i]);
			//inform SIE task to close all filters on the DMX and then release SIE task
			osal_flag_set(sie_info[i].sie_flag, SIE_FLAG_ENGINE_CLOSING);
			dmx_io_control(sie_info[i].dmx, DMX_WAKEUP_SIAE, 1);
			osal_cache_flush(&sie_info[i],sizeof(sie_info[i]));
			sie_leave_mutex(&sie_info[i]);
			
			//wait until all filters are closed
			osal_flag_wait(&flag, sie_info[i].sie_flag, SIE_FLAG_ENGINE_CLOSED,OSAL_TWF_ANDW, OSAL_WAIT_FOREVER_TIME);
			//free resource
			if (sie_info[i].addr)
			{
				FREE(sie_info[i].addr);
				sie_info[i].addr = NULL;
			}
			
			osal_flag_clear(sie_info[i].sie_flag, SIE_FLAG_ENGINE_CLOSED);
		}
		else
		{
			sie_leave_mutex(&sie_info[i]);
		}
	}

	return SI_SUCCESS;
}

/* alloc filter on the first opened dmx,
   ring buffer: length > 3*max_sec_length*/
struct si_filter_t *sie_alloc_filter(UINT16 pid, UINT8 *buffer, UINT32 length, 
	UINT16 max_sec_length)
{
	struct si_filter_t *ret;
	UINT32 flag;
	INT32 i;

	for(i=0; i<MAX_DMX_NUM; i++)
	{
		if(SIE_STATUS_OPENED == sie_info[i].status)
			break;
	}

	if(i == MAX_DMX_NUM)
	{
		SIE_PRINTF("%s: dmx NULL\n",__FUNCTION__);
		return NULL;
	}
	else if (length<max_sec_length)
	{
		SIE_PRINTF("%s: insufficent buffer length %x!\n", __FUNCTION__,length);
		return NULL;
	} 
	
	sie_enter_mutex(&sie_info[i]);
	if (osal_flag_wait(&flag, sie_info[i].sie_flag,SIE_FLAG_ENGINE_CLOSED|SIE_FLAG_ENGINE_CLOSING, 
		OSAL_TWF_ORW, 0)!=OSAL_E_OK)
		ret = __alloc_filter(&sie_info[i], pid, buffer, length, max_sec_length, 0);
	 else
	 {
	 	SIE_PRINTF("%s: sie closed!\n",__FUNCTION__);
		ret = NULL;
	 }
	
	sie_leave_mutex(&sie_info[i]);
	
	return ret;
}

/* abort filter on the first opened dmx,
   if mask_value is NULL, abort all filters of pid.*/
INT32 sie_abort(UINT16 pid, struct restrict *mask_value)
{
	INT32 dmx_idx;

	for(dmx_idx=0; dmx_idx<MAX_DMX_NUM; dmx_idx++)
	{
		if(SIE_STATUS_OPENED == sie_info[dmx_idx].status)
			break;
	}

	if(dmx_idx == MAX_DMX_NUM)
		return SI_EINVAL;

	return _sie_abort(dmx_idx, NULL, pid, mask_value);
}

/* set sie if continue getting sections or not, only used for epg.
     flag: 0: stop getting sections; 1: resume getting sections */
void sie_set_continue(struct si_filter_t *filter, UINT8 flag)
{
	struct sie_request *request;
	
	request = list_entry(filter, struct sie_request, filter);
	request->gsp.continue_get_sec = flag;
}


