#include "adv_geeya.h"
#include <hld/nim/nim.h>
#include <api/libstbinfo/stb_info_data.h>

#include "../copper_common/system_data.h"
//#include "GYAD_Interface.h"

//#include "adv_info.h"

#define GYAD_DATA_PID   0xfd

//#define  BOOT_ADV_LEN  0x2d0000
//#define	AD_PNG_ID				0x0AF50100 //0x02FD0200


static UINT32	LockSem = 0;
GYADS8			Status = AD_BUSY;
sRegId_Fid		Reg_fid;
INT8			cont = 0;
static UINT32 	pri_channel=0;
static UINT8 	g_sec_data[4096];
UINT8 			sec_cont = 0;

static struct dmx_device *gyad_dmx_dev = NULL;
static struct gyad_channel *gyad_channel_array[GYAD_MAX_CHAN_NUM];
static struct sto_device *flash_dev = NULL;


/**************************************************************
* Function: GYSTBAD_Malloc(GY_U32 dwMemSize)
* Descripe: Called by GYAD
* Para:
	dwMemSize: 需要分配的内存的大小
* Return: None
* Note: 
* Author: Archer
***************************************************************/
GYADVOID *GYSTBAD_Malloc(GYADU32 dwMemSize)
{
	MALLOC(dwMemSize);
}



/**************************************************************
* Function: GYSTBAD_Free(GY_VOID* buffer)
* Descripe: Called by GYAD
* Para:
	buffer: 内存地址
* Return: None
* Note: 
* Author: Archer
***************************************************************/
GYADVOID GYSTBAD_Free(GYADVOID *buffer)
{
	FREE(buffer);
}



/**************************************************************
* Function: GYSTBAD_Memset(GY_VOID* pDestBuf, GY_U8 c, GY_U32 dwSize)
* Descripe: Called by GYAD
* Para:
	pDestBuf: 填充地址
	cData: 填充的数据
	dwSize: 填充的长度
* Return: None
* Note: 
* Author: Archer
***************************************************************/
GYADVOID GYSTBAD_Memset(GYADVOID *pDestBuf, GYADU32 c, GYADU32 dwSize)
{
	MEMSET(pDestBuf, c, dwSize);
}



/**************************************************************
* Function: GYSTBAD_Memcopy(GY_VOID* pDestBuf, GY_VOID* pSrcBuf, GY_U32 dwSize)
* Descripe: Called by GYAD
* Para:
	pDestBuf: 
	pSrcBuf: 
	dwSize: 
* Return: None
* Note: 
* Author: Archer
***************************************************************/
GYADVOID GYSTBAD_Memcpy(GYADVOID *pDestBuf, GYADVOID *pSrcBuf, GYADU32 dwSize)
{
	MEMCPY(pDestBuf, pSrcBuf, dwSize);
}

void gyad_demxu_set_mv(struct restrict *mask_value, UINT8 index, UINT8* value, UINT8* mask, UINT8 mask_len)
{
	UINT8 i;
	libc_printf("gyad_demxu_set_mv():mask_len[%d]\n",mask_len);	
	if((mask_value->tb_flt_msk & (1<<index)) != 0)
	{
		;//TFCAS_DMX_PRINTF("tfcas_demxu_set_mv: filter will be resetted\n");
	}
	mask_value->mask_len = MAX_SEC_MASK_LEN;
	if(mask_len > MAX_SEC_MASK_LEN)
		mask_len = MAX_SEC_MASK_LEN;
	osal_task_dispatch_off();
	for(i=0; i<mask_len; i++)
	{
		mask_value->value[index][i] = value[i];
		mask_value->multi_mask[index][i] = mask[i];
	}
	mask_value->tb_flt_msk |= 1<<index;
	osal_task_dispatch_on();
	//TFCAS_DMXERR_PRINTF("tfcas_demxu_set_mv():index[%d]\n",index);
	//TFCAS_DMX_DUMP(mask_value->value[index],mask_value->mask_len);
	//TFCAS_DMX_DUMP(mask_value->multi_mask[index],mask_value->mask_len);	
	return;
}

static void gyad_rcv_sec_cb(struct get_section_param * param)
{
	UINT8 i;
	/*call tfcas callback function*/
	static UINT8 data[4];
	struct gyad_channel *tmp_channle = (struct gyad_channel *)param;

	//TFCAS_DMX_PRINTF("\n>>[%02x]\n",param->buff[0]);	
	if(param->sec_hit_num >= MAX_MV_NUM)
		return;
	
	if(((1<<param->sec_hit_num) & param->mask_value->tb_flt_msk)==0)
		return;

#if 0
	if((data[3]==param->buff[param->sec_tbl_len-1])
		&&(data[2]==param->buff[param->sec_tbl_len-2])
		&&(data[1]==param->buff[param->sec_tbl_len-3])
		&&(data[0]==param->buff[param->sec_tbl_len-4]))
		return;

	data[3] = param->buff[param->sec_tbl_len-1];
	data[2] = param->buff[param->sec_tbl_len-2];
	data[1] = param->buff[param->sec_tbl_len-3];
	data[0] = param->buff[param->sec_tbl_len-4];	
#endif	

#if 0
	if((param->buff[0]==0x82))//||(param->buff[0]==0x81))
	{
		int i;
		TFCAS_DMX_PRINTF("\n-------------------------------EMM-start -len[%d]---------------------------------\n",param->sec_tbl_len);
		for(i=0;i<param->sec_tbl_len;i++)
		//for(i=0;i<8;i++)
			TFCAS_DMX_PRINTF("%02x ",param->buff[i]);
		TFCAS_DMX_PRINTF("\n-------------------------------EMM--end---------------------------------\n ");		
	}
#endif	
	if(param->buff[0] == 0x82)
	libc_printf("\n\n###############data got! req_id:%x, filter:%x, pid: 0x%04x\n\n", tmp_channle->req_id[param->sec_hit_num], param->buff[0],param->pid);
	/*提交数据给ca 库*/
 
	//tmp_channle->get_sec_param.continue_get_sec = 0;/*临时这么改*/
	if(GYADSTB_PrivateDataGot(tmp_channle->req_id[param->sec_hit_num], TRUE, param->pid, param->buff, param->sec_tbl_len) != GYAD_ERROR_SUCCESS)
	{
		return ;
	}  
	#if 0
	if (TFCAS_DMX_ECM_REQUESTID == tmp_channle->req_id[0])//if ECM, release channel
	{
		for(i=0; i<MAX_SEC_MASK_LEN; i++)
		{
			tmp_channle->mask_value.value[param->sec_hit_num][i] = 0;
			tmp_channle->mask_value.multi_mask[param->sec_hit_num][i] = 0;
		}
		tmp_channle->mask_value.tb_flt_msk &= ~(1<<param->sec_hit_num);
	}
	#endif
	return;
}

INT32 gyad_filter_callback(UINT8 req_id, UINT8* value, UINT8* mask, UINT8 mask_len, UINT16 pid, UINT8 wait_seconds)
{
	UINT8 i;
	UINT8 j;
	UINT8 k;
	UINT8 l;
	UINT8 m;
	struct gyad_channel *tmp_channel;
	UINT32 tmp_time_out;

	libc_printf("gyad_filter_callback: mask_len[%d]\n",mask_len);


	GYADAPI_SemaphoreLock();
	if(wait_seconds == 0)
		tmp_time_out = OSAL_WAIT_FOREVER_TIME;
	else
		tmp_time_out = wait_seconds*1000;
	//TFCAS_DMX_PRINTF("tfcas_demux_request: open new channel. pid:0x%04x , req_id: %02x\n", pid, req_id);
	if (1)
	{
			j = GYAD_MAX_CHAN_NUM;
			for(i=GYAD_MAX_CHAN_NUM-1; i!=0xFF; i--)
			{
				tmp_channel = gyad_channel_array[i];
				if(tmp_channel->get_sec_param.pid == INVALID_PID)	/*the first free*/
					j = i;
				else if(tmp_channel->get_sec_param.pid == pid)
					break;
			}
			if(i != 0xFF)
			{
				/*channel for the pid has been open*/
				//TFCAS_DMX_PRINTF("\nchannel for the pid has been open\n");
				//TFCAS_DMX_PRINTF("tfcas_demux_request: opened. pid: 0x%04x\n", tmp_channel->get_sec_param.pid);		
				//tmp_channel->get_sec_param.continue_get_sec = 1;/*临时这么改*/
				m = MAX_MV_NUM;
				for(l=MAX_MV_NUM-1; l!=0xFF; l--)
				{
					if((tmp_channel->mask_value.tb_flt_msk & (1<<l)) == 0)
					{
						/*the first free filter*/
						m = l;
					}
					if(tmp_channel->req_id[l] == req_id)
						break;
				}
				if(l != 0xFF)
				{
					/*req_id has exist, reset it*/
					//TFCAS_DMX_PRINTF("tfcas_demux_request: reset mv. index:%02x , req_id: %02x\n", l, tmp_channel->req_id[l]);
					tmp_channel->status[l].enable = TRUE;
					tmp_channel->status[l].tick = osal_get_tick();
					tmp_channel->status[l].timeout = tmp_time_out;
					gyad_demxu_set_mv(&(tmp_channel->mask_value), l, value, mask, mask_len);
				}
				else if(m == MAX_MV_NUM)
				{
					libc_printf("tfcas_demux_request: no filter!\n");
					GYADAPI_SemaphoreUnlock();
					return GYAD_ERROR_UNKNOWN;
				}
				else
				{
					/*set new mask value to filter*/
					tmp_channel->req_id[m] = req_id;
					//TFCAS_DMX_PRINTF("tfcas_demux_request: set mv. index:%02x , req_id: %02x\n", m, tmp_channel->req_id[m]);				
					tmp_channel->status[m].enable = TRUE;
					tmp_channel->status[m].tick = osal_get_tick();
					tmp_channel->status[m].timeout = tmp_time_out;
					gyad_demxu_set_mv(&(tmp_channel->mask_value), m, value, mask, mask_len);
				}
				GYADAPI_SemaphoreUnlock();
				return	GYAD_ERROR_SUCCESS;
			}
			else if(j == GYAD_MAX_CHAN_NUM)
			{
				libc_printf("tfcas_demux_request: no channel!\n");
				GYADAPI_SemaphoreUnlock();
				return GYAD_ERROR_UNKNOWN;
			}
			else
			{
				//TFCAS_DMX_PRINTF("tfcas_demux_request: open new channel. pid:0x%04x , req_id: %02x\n", pid, req_id);
				
				/*open new channel for the pid*/
				tmp_channel = gyad_channel_array[j];
				tmp_channel->req_id[0] = req_id;
				
				gyad_demxu_set_mv(&(tmp_channel->mask_value), 0, value, mask, mask_len);
	
				tmp_channel->get_sec_param.pid = pid;
				tmp_channel->get_sec_param.buff = (UINT8 *)&(tmp_channel->sec_buf);
				tmp_channel->get_sec_param.buff_len = GYAD_MAX_SEC_SIZE;
				
				tmp_channel->get_sec_param.wai_flg_dly = tmp_time_out;
				tmp_channel->get_sec_param.mask_value = &(tmp_channel->mask_value);
				tmp_channel->get_sec_param.continue_get_sec = 1;/*每次只收一个section怎么做,luqz@20101121*/
				tmp_channel->get_sec_param.get_sec_cb = gyad_rcv_sec_cb;
	
				if(dmx_async_req_section(gyad_dmx_dev, &(tmp_channel->get_sec_param), &(tmp_channel->flt_id)) != RET_SUCCESS)
				{
					libc_printf("tfcas_demux_request: dmx_async_req_section() failed\n");
					GYADAPI_SemaphoreUnlock();
					return GYAD_ERROR_UNKNOWN;
				}
				tmp_channel->status[0].enable = TRUE;
				tmp_channel->status[0].tick = osal_get_tick();
				tmp_channel->status[0].timeout = tmp_time_out;
				libc_printf("tfcas_demux_request: filter %d\n", tmp_channel->flt_id);
				GYADAPI_SemaphoreUnlock();
				return GYAD_ERROR_SUCCESS;
			}
		}


}


/**************************************************************
* Function: GYSTBAD_RequestPrivateData(GYADU8 cReqID, const GYADU8 *pcFilter, const GYADU8* pcMask, 
									GYADU8 cLen, GYADU16 wPid, GYADU8 byWaitSeconds);
* Descripe: Called by GYAD
* Para:
	cReqID: 收取私有表的请求号，与GYADSTB_PrivateDataCallback中的cReqID相同
	pcFilter: 当前过滤器的值，8字节
	pcFilter: 当前过滤器的掩码，8字节
	wLen: 取到的私有数据的长度
	wPid: 数据流的PID，，与GYADSTB_PrivateDataCallback中的wPid相同
	byWaitSeconds: 超时等待时间，单位为秒，为0则一直等待
* Return: Refer to GYADErrorType_e
* Note: 只要调用了本函数，就必须调用GYADSTB_PrivateDataGot，它们是一一对应的
* Author: Archer
***************************************************************/
GYADErrorType_e GYSTBAD_RequestPrivateData(GYADU8 cReqID, const GYADU8 *pcFilter, 
											const GYADU8 *pcMask, GYADU8 cLen, 
											GYADU16 wPid, GYADU8 byWaitSeconds)
{
	UINT8 i;
	UINT8 value[MAX_SEC_MASK_LEN];
	UINT8 mask[MAX_SEC_MASK_LEN];

	if(wPid==0)
	{
	 	//CHCAS_ReceivedTable(bRequestID,0, wPid,NULL, 0);	
		libc_printf("data is 0x00,return timeout!!!\n: ");
		return GYAD_ERROR_SUCCESS;
	}

	MEMSET(value, 0, sizeof(value));
	MEMSET(mask, 0, sizeof(mask));
	value[0] = pcFilter[0];
	mask[0] = pcMask[0];
	//libc_printf("value=%x\n", value[0]);
	//yqtong added 20110719
	//if(value[0] == 0x40)
	//	bSendOSDdata = 1;
	if((cLen>=2)&&(cLen<=16))
	{
		for(i=1; i<cLen; i++)
		{
			value[i+2] = pcFilter[i];
			mask[i+2] = pcMask[i];
		}
	}


	if(cLen>16)
		libc_printf("\n\n\n*****************************error****************************\n\n\n");

	/*if((value[0] == 0x80) || (value[0] == 0x81))
		byReqID = 1;
	else
		byReqID = 2;

	if(mask[0] == 0xF0)
	{
		mask[0] =  0x00;
		value[0] = 0x00;
	}*/
	return gyad_filter_callback(cReqID, value, mask, cLen+2, wPid, byWaitSeconds);	
}



GYADVOID GYSTBAD_Print(const GYADU8 *fmt, ...)
{
	//if(gy_switch_debug_get())
	//{
		libc_printf(fmt);
	//}
#if 0
	va_list arg;
	va_start(arg, fmt);
	vprintf(fmt, arg);
	va_end(arg);
#endif
	return;
}

GYADVOID gyad_check_data_timeout(void)
{
	GYADU8 i;
	GYADU8 j;
	GYADU32  cur_tick = 0;
	GYADU8 timeout = FALSE;
	struct gyad_channel *tmp_channel;

	//if(!GYSTBAD_End())
	//	return ;
	
	libc_printf("%s, tick:%d\n", __FUNCTION__, osal_get_tick()/1000);
	
	for(i=0; i<GYAD_MAX_CHAN_NUM; i++)
	{
		tmp_channel = gyad_channel_array[i];
		for(j=0; j<MAX_MV_NUM; j++)
		{
			if(tmp_channel->status[j].enable == TRUE)
			{
				cur_tick = osal_get_tick();
				if(((tmp_channel->get_sec_param.pid != 0)
					&&(tmp_channel->get_sec_param.pid != INVALID_PID))
					&& (OSAL_WAIT_FOREVER_TIME != tmp_channel->status[j].timeout)
					&&((cur_tick-tmp_channel->status[j].tick) > tmp_channel->status[j].timeout))
					{
						timeout = TRUE;
						libc_printf("%s, rec ad data timeout !! , reqID[%d](%d), timeout(%d), curtime(%d), starttime(%d), cur-start(%d)\n",
									__FUNCTION__, j, tmp_channel->req_id[j], tmp_channel->status[j].timeout, cur_tick, tmp_channel->status[j].tick,
									cur_tick-tmp_channel->status[j].tick);
					//GYADAPI_SemaphoreLock();
						dmx_io_control(gyad_dmx_dev, IO_ASYNC_CLOSE, tmp_channel->flt_id);
						MEMSET(tmp_channel, 0, sizeof(struct gyad_channel));
						tmp_channel->get_sec_param.pid = INVALID_PID;
						tmp_channel->status[j].enable = FALSE;
					//GYADAPI_SemaphoreUnlock();
						
					}
			}
		}
	}
	if(timeout == TRUE)
	{
		GYADAPI_Exit();
		timeout = FALSE;
		libc_printf("-----------------------------------------------\n");
	}
}

GYADVOID GYSTBAD_RequestFree(GYADU8 cReqID)
{
	UINT8 i;
	UINT8 j;
	struct gyad_channel *tmp_channel;

	for(i=0; i<GYAD_MAX_CHAN_NUM; i++)
	{
		tmp_channel = gyad_channel_array[i];
		for(j=0; j<MAX_MV_NUM; j++)
		{
			if(tmp_channel->req_id[j] == cReqID)
			{
				//TFCAS_DMX_PRINTF("tfcas_dmx_free: req_id:0x%02x, flt_id:0x%02x\n", req_id, tmp_channel->flt_id);

				//GYADAPI_SemaphoreLock();
				//dmx_io_control(gyad_dmx_dev, IO_ASYNC_CLOSE, tmp_channel->flt_id);
				//MEMSET(tmp_channel, 0, sizeof(struct gyad_channel));
				//tmp_channel->get_sec_param.pid = INVALID_PID;
				//tmp_channel->status[j].enable = FALSE;
				//GYADAPI_SemaphoreUnlock();

		#if 1		
				//GYADAPI_SemaphoreLock();
				dmx_io_control(gyad_dmx_dev, IO_ASYNC_CLOSE, tmp_channel->flt_id);
				MEMSET(tmp_channel, 0, sizeof(struct gyad_channel));
				tmp_channel->get_sec_param.pid = INVALID_PID;
				//GYADAPI_SemaphoreUnlock();
		#endif
				break;
			}
		}
	}
}




/**************************************************************
* Function: GYADSTB_SetTS(GYADVOID)
* Descripe: Called by GYAD
* Para:
	None
* Return: Refer to GYADErrorType_e
* Note: 锁定所在频点
* Author: Archer
***************************************************************/
GYADErrorType_e GYSTBAD_SetTS(GYADVOID)
{
	UINT32 freq = 0;
	UINT32 symbol = 0;
	UINT8 mode = 0;
	SYSTEM_DATA *sys_data = &system_config;
	
	struct nim_device *dev_nim;
	UINT32 sys_uint32_freq = sys_data->main_frequency;
	UINT8  lock;
	UINT8 i;
	
	if(sys_uint32_freq >86200&&sys_uint32_freq<11100)
		sys_uint32_freq = MAIN_FREQ_DEFAULT;
	struct nim_device *nim_dev = (struct nim_device*)dev_get_by_id(HLD_DEV_TYPE_NIM, 0);
	freq = sys_uint32_freq;
	symbol = 6875;
	mode 	= QAM64;

	nim_channel_change(nim_dev, freq, symbol, mode);
	osal_task_sleep(1000);
	for(i = 0; i<5; i++)
	{
		nim_get_lock(nim_dev, &lock);

		if(lock == 1)
		{
			libc_printf("TS lock!\n");
			return GYAD_ERROR_SUCCESS;
		}
		else
		{
			osal_task_sleep(10); 
			//libc_printf("TS no lock!\n");
			//return GYAD_ERROR_UNKNOWN;
		}
	}
	libc_printf("TS no lock,freq=%d!\n",sys_uint32_freq);
	return GYAD_ERROR_UNKNOWN;
	
}




/**************************************************************
* Function: GYSTBAD_SemaphoreLock(GYADVOID)
* Descripe: Called by GYAD
* Para:
	None
* Return: None
* Note: 
* Author: Archer
***************************************************************/
GYADVOID GYSTBAD_SemaphoreLock(GYADVOID)
{
	//if(0 == LockSem)
	//{
		//LockSem = osal_semaphore_create(1);
		//if(LockSem == 0)
		//{
		//	GYSTBAD_Print("Err: %s creat LockSem failed!\n", __FUNCTION__);
		//	return ;
		//}
	//}
	//osal_semaphore_capture(LockSem, GYAD_LOCK_TIMEOUT);
}




/**************************************************************
* Function: GYSTBAD_SemaphoreUnlock(GYADVOID)
* Descripe: Called by GYAD
* Para:
	None
* Return: None
* Note: 
* Author: Archer
***************************************************************/
GYADVOID GYSTBAD_SemaphoreUnlock(GYADVOID)
{
	//if(0 == LockSem)
	//{
	//	GYSTBAD_Print("Semaphore Lock Error!");
	//}
	//osal_semaphore_release(LockSem);
}


static UINT8 GYADWriteReady(GYADU32 addr, GYADU32 len)
{
	struct sto_device *flash_dev = NULL;
	UINT32 param, flash_cmd, tmp_param[2];
	UINT8 *buff = NULL;
	INT32  ret = -1;
	
	libc_printf("%s, ........................................ addr: %x, len: %d\n",__FUNCTION__, addr, len);
	flash_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
	if(NULL == flash_dev)
	{
		libc_printf("%s: not find flash dev !\n", __FUNCTION__);
		return GYAD_ERROR_UNKNOWN;
	}
	if(flash_dev->totol_size <= 0x400000)
	{
		param = addr << 10;
		param |= 64*1024 >> 10;
		flash_cmd = STO_DRIVER_SECTOR_ERASE;
	}
	else
	{
		tmp_param[0] = addr;
		tmp_param[1] = len >> 10;
		param = (UINT32)tmp_param;
		flash_cmd = STO_DRIVER_SECTOR_ERASE_EXT;
	}
	buff = (UINT8 *)MALLOC(64*1024);
	sto_io_control(flash_dev, STO_DRIVER_SECTOR_BUFFER, (UINT32)buff);
	ret = sto_io_control(flash_dev, flash_cmd, param);
	if(buff != NULL)
		free(buff);
	if(SUCCESS != ret )
	{
		libc_printf("%s: addr: %08x, len: %d, Erase failed!\n", __FUNCTION__, addr, len);
		return GYAD_ERROR_UNKNOWN;
	}
	return SUCCESS;
}

/**************************************************************
* Function: GYSTBAD_SaveBootAd(sGYAdInfo_t *bootADInfo, GYADU8 bootADNum, GYADU8 bootADVer)
* Descripe: Called by GYAD
* Para:
	bootADInfo: 开机广告信息，需要保存
	bootADNum:   开机广告的个数
	bootADVer:   开机广告版本，需要保存
* Return: Refer to GYADErrorType_e
* Note:STB自行保存
* Author: Archer
_______________________________________________________________________________________
| u8ver | u8sum | u32第一段数据长度|....... |  第一段数据| ..........

***************************************************************/
GYADErrorType_e GYSTBAD_SaveBootAd(sGYAdInfo_t *bootADInfo, GYADU8 bootADNum, GYADU8 bootADVer)
{
	INT32	Error = -1;
	GYADU32 addr,len,u32DataLen;
	GYADU8	*BootAdvData 	= NULL;
	GYADU32	u32DataCur 	= 0;
	GYADU32	u32OneDataLen 	= 0;
	INT8	i = 0;	
    GYADU32 id = STB_CHUNK_ADV_LOGO_ID;

	CHUNK_HEADER chunk_hdr;
	if (sto_get_chunk_header(id, &chunk_hdr) == 0)
		return GYAD_ERROR_UNKNOWN;
	addr = (sto_chunk_goto(&id, 0xFFFFFFFF,1) + CHUNK_HEADER_SIZE);
	len = STB_CHUNK_ADV_LOGO_LEN - CHUNK_HEADER_SIZE;
	
	//save data len
	u32DataLen = 2*sizeof(GYADU8)+sizeof(GYADU32)*bootADNum;
	u32DataLen += bootADNum*sizeof(sGYAdInfo_t);
		
	for(i=0; i<bootADNum; i++)
	{
		u32DataLen += bootADInfo[i].dwAdLen;
	}
	GYSTBAD_Print("%s, BootDataLen = %d\n",__FUNCTION__,u32DataLen);

	if(u32DataLen > len)
	{
		GYSTBAD_Print("%s, BootDataLen > %d\n",__FUNCTION__, len);
		return GYAD_ERROR_UNKNOWN;
	}
	if(NULL == BootAdvData)
	{
		BootAdvData = (GYADU8*)GYADAPI_Malloc(u32DataLen);
		if(NULL == BootAdvData)
		{
			GYSTBAD_Print("%s, malloc fail!\n",__FUNCTION__);
			return GYAD_ERROR_UNKNOWN;
		}
	}

	BootAdvData[u32DataCur] = bootADVer;  //存放广告版本号 
	u32DataCur += sizeof(GYADU8);
	BootAdvData[u32DataCur] = bootADNum;  //存放开机广告总数
	u32DataCur += sizeof(GYADU8);
	
	for(i=0; i<bootADNum; i++)   //每段数据长度(u32)
	{
		u32OneDataLen = sizeof(sGYAdInfo_t)+bootADInfo[i].dwAdLen;
		GYSTBAD_Memcpy(&BootAdvData[u32DataCur], &u32OneDataLen, sizeof(GYADU32));
		u32DataCur += sizeof(GYADU32);
	}
	
	for(i=0; i<bootADNum; i++)//logo1信息,logo1数据,logo2信息,logo2数据
	{
		GYSTBAD_Memcpy(&BootAdvData[u32DataCur], &bootADInfo[i], sizeof(sGYAdInfo_t));
		u32DataCur = u32DataCur + sizeof(sGYAdInfo_t);
		GYSTBAD_Memcpy(&BootAdvData[u32DataCur], bootADInfo[i].cAdData, bootADInfo[i].dwAdLen);
		u32DataCur += bootADInfo[i].dwAdLen;
	}	
	
	Error = GYSTBAD_WriteNvram_Logo_Menu(bootADInfo[0].cAdData, bootADInfo[0].dwAdLen);
	if(GYAD_ERROR_SUCCESS != Error)
	{
		libc_printf("\n\n\n================= write to logo menu fail =====================\n\n\n");
		GYSTBAD_Free(BootAdvData);	
		return GYAD_ERROR_UNKNOWN;
	}
	
	//Save boot ad ver and info
	Error = GYSTBAD_WriteNvram(addr, BootAdvData, u32DataLen);
	if(GYAD_ERROR_SUCCESS != Error)
	{
		GYSTBAD_Print("%s, write logo data fail !",__FUNCTION__);
		GYSTBAD_Free(BootAdvData);	
		return GYAD_ERROR_UNKNOWN;
	}
	else
	{
		GYSTBAD_Print("%s, logo1 data of logo data before 14 bytes ,__in__",__FUNCTION__);
		for(i=bootADNum*sizeof(GYADU32)+2; i<sizeof(sGYAdInfo_t)+bootADNum*sizeof(GYADU32)+14; i++)
		{
			GYSTBAD_Print("  0x%02x",BootAdvData[i]);
		}
		GYSTBAD_Print("\n");
	}
	GYSTBAD_Free(BootAdvData);	
	return GYAD_ERROR_SUCCESS;
}


GYADErrorType_e GYSTBAD_GetBootAd(sGYAdInfo_t *pBootADInfo, GYADU8 u8BootADNum)
{
	INT32	Error = -1;
	GYADU32 addr,len,u32DataLen;
	GYADU32	u32InfoLen = 0;   //读取数据的长度
	GYADU32	u32CurInfoLen = 0;  //当前数据长度
	GYADU32 u32OffsetLen = 0;

	GYADU32 db_addr, db_len, default_db_addr, default_db_len;
	
	GYADU8	*pBootAdvData = NULL;
	GYADU8	u8BootAdvSum = 0;	
	INT8	i = 0;
    GYADU32  id = STB_CHUNK_ADV_LOGO_ID;
	CHUNK_HEADER chunk_hdr;

	if((pBootADInfo == NULL) || (pBootADInfo->cAdData == NULL) ||(u8BootADNum == 0))
	{
		return GYAD_ERROR_PARA;
	}
	if (sto_get_chunk_header(id, &chunk_hdr) == 0)
		return;
	addr = (sto_chunk_goto(&id, 0xFFFFFFFF,1) + CHUNK_HEADER_SIZE);
	len = STB_CHUNK_ADV_LOGO_LEN - CHUNK_HEADER_SIZE;

	Error = GYSTBAD_ReadNvram(addr+1, &u8BootAdvSum, 1);
	if(GYAD_ERROR_SUCCESS != Error)
	{
		return GYAD_ERROR_UNKNOWN;
	}
	
	if(u8BootAdvSum < u8BootADNum)
	{
		return GYAD_ERROR_PARA;
	}

	GYSTBAD_ReadNvram(addr+2*sizeof(GYADU8)+sizeof(GYADU32)*(u8BootADNum-1), &u32InfoLen, sizeof(GYADU32));//get last logo len
	u32OffsetLen = 2*sizeof(GYADU8)+sizeof(GYADU32)*u8BootAdvSum;
		
	for(i=0; i<u8BootADNum-1; i++)//get last logo data start addr
	{
		GYSTBAD_ReadNvram(addr+2*sizeof(GYADU8)+sizeof(GYADU32)*i, &u32CurInfoLen, sizeof(GYADU32));
		u32OffsetLen += u32CurInfoLen;
	}

	if(NULL == pBootAdvData)
	{
		pBootAdvData = (GYADU8*)GYADAPI_Malloc(u32InfoLen);
	}
	
	Error = GYSTBAD_ReadNvram(addr+u32OffsetLen, pBootAdvData, u32InfoLen);
	if(GYAD_ERROR_SUCCESS != Error)
	{
		libc_printf("%s, read logo data fail ! \ n",__FUNCTION__);
		return GYAD_ERROR_UNKNOWN;
	}
	GYSTBAD_Print("%s, __out__", __FUNCTION__);
	for(i=0; i<sizeof(sGYAdInfo_t)+12; i++)
	{
		GYSTBAD_Print("  0x%02x",pBootAdvData[i]);
	}
	GYSTBAD_Print("\n");
	GYSTBAD_Memcpy(pBootADInfo, pBootAdvData, sizeof(sGYAdInfo_t));
	u32DataLen = u32InfoLen-sizeof(sGYAdInfo_t);
	GYSTBAD_Memcpy(pBootADInfo->cAdData, &pBootAdvData[sizeof(sGYAdInfo_t)], u32DataLen);
	GYSTBAD_Free(pBootAdvData);
	return GYAD_ERROR_SUCCESS;
}


/**************************************************************
* Function: GYSTBAD_GetBootVer(GYADU8 *ver)
* Descripe: Called by GYAD
* Para:
	version: 开机广告的版本
* Return: Refer to GYADErrorType_e
* Note: 
* Author: Archer
***************************************************************/
GYADErrorType_e GYSTBAD_GetBootVer(GYADU8 *version)
{
	INT32	ret = -1;
	GYADU32 addr,len;
    GYADU32 id = STB_CHUNK_ADV_LOGO_ID;

	CHUNK_HEADER chunk_hdr;
	if (sto_get_chunk_header(id, &chunk_hdr) == 0)
		return;
	addr = (sto_chunk_goto(&id, 0xFFFFFFFF,1) + CHUNK_HEADER_SIZE);		
	ret = GYSTBAD_ReadNvram(addr, version, 1);
	if(GYAD_ERROR_SUCCESS != ret)
	{
		*version = 0xff;
		 libc_printf("\n %s, read logo version fail !\n",__FUNCTION__);
		return GYAD_ERROR_UNKNOWN;
	}
	GYSTBAD_Print("________________________________________version = 0x%02x\n", *version);
	return GYAD_ERROR_SUCCESS;
}


/**************************************************************
* Function: GYSTBAD_GetNvram(GYADU32 *pdwAddr, GYADU32 *len)
* Descripe: Called by GYAD
* Para:
	pdwAddr: NVRAM的起始地址
	len:   NVRAM的大小
* Return: Refer to GYADErrorType_e
* Note: ?
* Author: Archer
***************************************************************/
GYADVOID GYSTBAD_GetNvram(GYADU32 *pdwAddr, GYADU32 *len)
{
	GYADU32 id = STB_CHUNK_ADV_DATA_ID;
	CHUNK_HEADER chunk_hdr;
	
	if (sto_get_chunk_header(id, &chunk_hdr) == 0)
		return;
	*pdwAddr = (sto_chunk_goto(&id, 0xFFFFFFFF,1) + CHUNK_HEADER_SIZE);
	*len = STB_CHUNK_ADV_DATA_LEN-CHUNK_HEADER_SIZE;	
}




/**************************************************************
* Function: GYSTBAD_ReadNvram(GYADVOID)
* Descripe: Called by GYAD
* Para:
	data: 需要读取的数据
	len:   数据长度
* Return: Refer to GYADErrorType_e
* Note: 
* Author: Archer
***************************************************************/
GYADErrorType_e GYSTBAD_ReadNvram(GYADU32 dwAddr, GYADU8 *data, GYADU32 len)
{
	struct sto_device *sto;
	INT32 ret = 0;

	sto = (struct sto_device*)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
	if (sto == NULL)
	{
		libc_printf("\n %s, Can't find FLASH device!\n",__FUNCTION__);
        return GYAD_ERROR_FAIL;
	}

	ret = sto_get_data(sto, data, dwAddr, len);
	if ( ret != len)
	{
		GYSTBAD_Print(" %s, _____read______dwAddr = 0x%02x, len = 0x%02x, sto_read_len = %d\n",__FUNCTION__, dwAddr, len, ret);
		return GYAD_ERROR_FAIL;
	}

	GYSTBAD_Print("_____read______dwAddr = 0x%02x, len = 0x%02x \n", dwAddr, len);
	return GYAD_ERROR_SUCCESS;
}


/**************************************************************
* Function: GYSTBAD_EraseNvram(GYADU32 dwAddr, GYADU32 len)
* Descripe: Called by GYAD
* Para:
       dwAddr: 起始地址
	len:   擦除的NVRAM的长度
* Return: Refer to GYADErrorType_e
* Note: 
* Author: Archer
***************************************************************/
GYADErrorType_e GYSTBAD_EraseNvram(GYADU32 dwAddr, GYADU32 len)
{
	INT8	*Erase_data = NULL;

	UINT32 chunk_id;
	CHUNK_HEADER chunk_hdr;
	
	chunk_id = STB_CHUNK_ADV_LOGO_ID;
	if(dwAddr == (sto_chunk_goto(&chunk_id, 0xFFFFFFFF,1) + CHUNK_HEADER_SIZE))
	{
		if(SUCCESS != GYADWriteReady(dwAddr, STB_CHUNK_ADV_LOGO_LEN-CHUNK_HEADER_SIZE))
		{
			libc_printf("\n %s, write adv logo fail !\n",__FUNCTION__);
			return GYAD_ERROR_FAIL;
		}
	}
	else
	{
		chunk_id = STB_CHUNK_ADV_DATA_ID;
		if(dwAddr == (sto_chunk_goto(&chunk_id, 0xFFFFFFFF,1) + CHUNK_HEADER_SIZE))
		{
			if(SUCCESS != GYADWriteReady(dwAddr, STB_CHUNK_ADV_DATA_LEN-CHUNK_HEADER_SIZE))
			{
				libc_printf("\n %s, write adv data fail !\n",__FUNCTION__);
				return GYAD_ERROR_FAIL;
			}
		}
		else
			return GYAD_ERROR_UNLOCK;
	}
	
	return GYAD_ERROR_SUCCESS;
	//GYSTBAD_Memset(Erase_data, 0xff, len);
	//return (GYSTBAD_WriteNvram(dwAddr, Erase_data, len));
}



/**************************************************************
* Function: GYSTBAD_WriteNvram(GYADVOID)
* Descripe: Called by GYAD
* Para:
	data: 需要保存的数据
	len:   数据长度
* Return: Refer to GYADErrorType_e
* Note: 
* Author: Archer
***************************************************************/
GYADErrorType_e GYSTBAD_WriteNvram(GYADU32 dwAddr, GYADU8 *data, GYADU32 len)
{
	struct sto_device *sto;
	INT32	ret = -1;
	UINT32 chunk_id;
	CHUNK_HEADER chunk_hdr;
	UINT32 flash_flg = 0;
	UINT8 *buff = NULL;

#if 0
	INT32 addr;
	
	libc_printf("%s, ........................................ addr: %x, len: %d\n",__FUNCTION__, dwAddr, len);
//TEST
	chunk_id = 0x0AF50400;
	if(sto_get_chunk_header(chunk_id, &chunk_hdr)== 0)
	{
		libc_printf("\n %s, chunk_id:%x08, get chunk header failed !\n",__FUNCTION__, chunk_id);
		return GYAD_ERROR_FAIL;
	}
	addr = sto_chunk_goto(&chunk_id, 0xFFFFFFFF,1);
	libc_printf("%s, ........................................ chunk addr before adv_logo : %x\n",__FUNCTION__, addr);


	chunk_id = STB_CHUNK_ADV_LOGO_ID;
	if(sto_get_chunk_header(chunk_id, &chunk_hdr)== 0)
	{
		libc_printf("\n %s, chunk_id:%x08, get chunk header failed !\n",__FUNCTION__, chunk_id);
		return GYAD_ERROR_FAIL;
	}
	addr = sto_chunk_goto(&chunk_id, 0xFFFFFFFF,1);
	libc_printf("%s, ........................................ chunk addr of adv_logo : %x\n",__FUNCTION__, addr);

	chunk_id = STB_CHUNK_ADV_DATA_ID;
	if(sto_get_chunk_header(chunk_id, &chunk_hdr)== 0)
	{
		libc_printf("\n %s, chunk_id:%x08, get chunk header failed !\n",__FUNCTION__, chunk_id);
		return GYAD_ERROR_FAIL;
	}
	addr = sto_chunk_goto(&chunk_id, 0xFFFFFFFF,1);
	libc_printf("%s, ........................................ chunk addr of adv_data : %x\n",__FUNCTION__, addr);

	chunk_id = STB_CHUNK_CAS_ID;
	if(sto_get_chunk_header(chunk_id, &chunk_hdr)== 0)
	{
		libc_printf("\n %s, chunk_id:%x08, get chunk header failed !\n",__FUNCTION__, chunk_id);
		return GYAD_ERROR_FAIL;
	}
	addr = sto_chunk_goto(&chunk_id, 0xFFFFFFFF,1);
	libc_printf("%s, ........................................ chunk addr of CA : %x\n",__FUNCTION__, addr);


	//return GYAD_ERROR_FAIL;

//TEST END
#endif



	chunk_id = STB_CHUNK_ADV_LOGO_ID;
	if(sto_get_chunk_header(chunk_id, &chunk_hdr)== 0)
	{
		libc_printf("\n %s, chunk_id:%08x, get chunk header failed !\n",__FUNCTION__, chunk_id);
		return GYAD_ERROR_FAIL;
	}
	
	if(dwAddr == (sto_chunk_goto(&chunk_id, 0xFFFFFFFF,1) + CHUNK_HEADER_SIZE))
	{
		//if(SUCCESS != GYADWriteReady(dwAddr, STB_CHUNK_ADV_LOGO_LEN-CHUNK_HEADER_SIZE))
		//{
		//	libc_printf("\n %s, write adv logo fail !\n",__FUNCTION__);
		//	return GYAD_ERROR_FAIL;
		//}
	}
	else
	{
		chunk_id = STB_CHUNK_ADV_DATA_ID;
		if(sto_get_chunk_header(chunk_id, &chunk_hdr)== 0)
		{
			libc_printf("\n %s, chunk_id:%08x, get chunk header failed !\n",__FUNCTION__, chunk_id);
			return GYAD_ERROR_FAIL;
		}
		if(dwAddr == (sto_chunk_goto(&chunk_id, 0xFFFFFFFF,1) + CHUNK_HEADER_SIZE))
		{
			//if(SUCCESS != GYADWriteReady(dwAddr, STB_CHUNK_ADV_DATA_LEN-CHUNK_HEADER_SIZE))
			//{
			//	libc_printf("\n %s, write adv data fail !\n",__FUNCTION__);
			//	return GYAD_ERROR_FAIL;
			//}
		}
		else
			return GYAD_ERROR_UNLOCK;
	}
	

	sto = (struct sto_device*)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
	if (sto == NULL)
	{
		libc_printf("\n %s, Can't find FLASH device!\n",__FUNCTION__);
        return GYAD_ERROR_FAIL;
	}	
	libc_printf("%s, _____write______dwAddr = 0x%02x, len = 0x%02x\n", __FUNCTION__,dwAddr, len);
	
	if(buff == NULL)
		buff = (UINT8 *)MALLOC(64*1024);
	sto_io_control(sto, STO_DRIVER_SECTOR_BUFFER, (UINT32)buff);

	flash_flg = sto->flag;
	sto_io_control(sto, STO_DRIVER_SET_FLAG,STO_FLAG_AUTO_ERASE | STO_FLAG_SAVE_REST);
	ret = sto_put_data(sto, dwAddr, data, len);
	sto_io_control(sto, STO_DRIVER_SET_FLAG,flash_flg);
	if(buff != NULL)
		free(buff);
	if(ret != len)
	{
		libc_printf("%s, _____write______dwAddr = 0x%02x, len = 0x%02x, sto_write_len = %d\n",
					  __FUNCTION__,dwAddr, len, ret);
		return GYAD_ERROR_FAIL;
	}
	return GYAD_ERROR_SUCCESS;
}

GYADErrorType_e GYSTBAD_WriteNvram_Logo_Menu(GYADU8 *data, GYADU32 len)
{
	struct sto_device *sto;
	INT32	ret = -1;
	UINT32 chunk_id;
	CHUNK_HEADER chunk_hdr;
	UINT32 flash_flg = 0;
	UINT8 *buff = NULL;
	UINT32 addr;
	UINT8 *buff_tmp = NULL;
	UINT32 len_tmp = 0;

	chunk_id = BOOT_LOGO_ID;
	if(sto_get_chunk_header(chunk_id, &chunk_hdr)== 0)
	{
		libc_printf("\n %s, chunk_id:%x08, get chunk header failed !\n",__FUNCTION__, chunk_id);
		return GYAD_ERROR_FAIL;
	}

	if(len > chunk_hdr.offset - CHUNK_HEADER_SIZE)
	{
		libc_printf("\n %s,logo menu len > logo menu flash max len !\n",__FUNCTION__);
        return GYAD_ERROR_FAIL;
	}

	addr = sto_chunk_goto(&chunk_id, 0xFFFFFFFF, 1);

	sto = (struct sto_device*)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
	if (sto == NULL)
	{
		libc_printf("\n %s, Can't find FLASH device!\n",__FUNCTION__);
        return GYAD_ERROR_FAIL;
	}
	
	buff_tmp = (UINT8 *)MALLOC(len + CHUNK_HEADER_SIZE);
	if(buff_tmp == NULL)
	  return GYAD_ERROR_FAIL;

	if(CHUNK_HEADER_SIZE != sto_get_data(sto, buff_tmp, addr, CHUNK_HEADER_SIZE))
	{
		FREE(buff_tmp);
		buff_tmp = NULL;
		return GYAD_ERROR_FAIL;
	}
	
	buff = (UINT8 *)MALLOC(64*1024);
	if(buff == NULL)
		return GYAD_ERROR_FAIL;
	
	len_tmp = len + CHUNK_HEADER_SIZE - 16;
	buff_tmp[4] = (UINT8)((len_tmp>>24)&0xff);
	buff_tmp[5] = (UINT8)((len_tmp>>16)&0xff);
	buff_tmp[6] = (UINT8)((len_tmp>>8)&0xff);
	buff_tmp[7] = (UINT8)(len_tmp&0xff);

	MEMCPY(&buff_tmp[CHUNK_HEADER_SIZE], data, len);
	libc_printf("%s, _____write______dwAddr = 0x%02x, len = 0x%02x\n", __FUNCTION__,addr, len);
	sto_io_control(sto, STO_DRIVER_SECTOR_BUFFER, (UINT32)buff);
	flash_flg = sto->flag;
	sto_io_control(sto, STO_DRIVER_SET_FLAG,STO_FLAG_AUTO_ERASE | STO_FLAG_SAVE_REST);
	ret = sto_put_data(sto, addr, buff_tmp, len+CHUNK_HEADER_SIZE);
	sto_io_control(sto, STO_DRIVER_SET_FLAG,flash_flg);
	free(buff);
	free(buff_tmp);
	if(ret != len+CHUNK_HEADER_SIZE)
	{
		libc_printf("%s, _____write______dwAddr = 0x%02x, len = 0x%02x, sto_write_len = %d\n",
					  __FUNCTION__,addr, len, ret);
		return GYAD_ERROR_FAIL;
	}
	return GYAD_ERROR_SUCCESS;
}

GYADVOID GYSTBAD_Start(GYADVOID)
{
	Status = AD_BUSY;
	UINT8 i = 0;
	struct gyad_channel *tmp_channel;	
	
	gyad_dmx_dev = (struct dmx_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DMX);
	
	for(i=0; i<GYAD_MAX_CHAN_NUM; i++)
	{
		tmp_channel = (struct gyad_channel*) MALLOC(sizeof(struct gyad_channel));
		if(tmp_channel == NULL)
		{
			libc_printf("gyad_demux_request: malloc error!\n");
			return 0;
		}
		MEMSET(tmp_channel, 0, sizeof(struct gyad_channel));
		tmp_channel->get_sec_param.pid = INVALID_PID;
		gyad_channel_array[i] = tmp_channel;
	}	
	GYADSTB_ADStart(GYAD_DATA_PID);
}



/**************************************************************
* Function: GYSTBAD_ADExit(GYADVOID)
* Descripe: Called by GYAD
* Para:
	None
* Return: None
* Note: GYAD通知STB广告接收完毕
* Author: Archer
***************************************************************/
GYADVOID GYSTBAD_ADExit(GYADVOID)
{
	GYSTBAD_Print("GYSTBAD_ADExit\n");
	Status = AD_FREE;
}

GYAD_STATUS GYSTBAD_End(GYADVOID)
{
	return Status;
}

int gyad_get_channel_ad(UINT16 sid,UINT8 adv_type, void *ad_data)
{
 sGYAdInfo_t *pGetAdInfo = (sGYAdInfo_t *)ad_data;
 sGYADRelativeInfo_t  sGetAdRelInfo;
 UINT8 u8AdType = ADV_PIC_TYPE_UNKNOWN;

 if((pGetAdInfo == NULL) ||(pGetAdInfo->cAdData == NULL) || 
 	(adv_type>=SHOW_GYADV_TYPE_MAX)||(adv_type == 0))
 	return ERR_FAILURE;
 
 sGetAdRelInfo.cAdIndx = adv_type;//SHOW_GYADV_CHANNEL_CHANGE;
 sGetAdRelInfo.wSvcId = sid; 
 sGetAdRelInfo.wCurTime= 0;
 sGetAdRelInfo.wTsId  = 2;
 if(GYADSTB_GetAdInfo(sGetAdRelInfo, pGetAdInfo) != SUCCESS)
	 return ERR_FAILURE;

  return SUCCESS;
}

int gyad_get_boot_logo(void *logo_data)
{
	UINT8 boot_ver = 0xff;
	sGYAdInfo_t * bootlogo_info = (sGYAdInfo_t *)logo_data;
	
	if((bootlogo_info == NULL) || (bootlogo_info->cAdData == NULL))
		return ERR_FAILURE;
	
	GYADAPI_GetBootVer(&boot_ver);
	if(boot_ver == 0xff)
		return ERR_FAILURE;
	
	if(GYAD_ERROR_SUCCESS !=  GYSTBAD_GetBootAd(bootlogo_info, 1))
		return ERR_FAILURE;

	return SUCCESS;
}

