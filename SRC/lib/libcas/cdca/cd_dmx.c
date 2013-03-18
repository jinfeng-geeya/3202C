/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2006-2007 Copyright (C)
 *
 *  File: tf_dmx.c
 *
 *  Description: This file fills the Demux related API of Tongfang CA library.
 *
 *  History:
 *      Date            Author            Version   Comment
 *      ====        ======      =======  =======
 *  1. 2006.3.19  Gushun Chen     0.1.000    Initial
 *  2. 2007.10.22  Grady Xu		0.2.000	  re-org the Tongfang CA lib.
 ****************************************************************************/

#include <sys_config.h>
#include <types.h>
#include <retcode.h>
#include <hld/hld_dev.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/alloc.h>
#include <api/libc/string.h>
#include <hld/dmx/dmx.h>
#include <api/libtsi/si_types.h>

//#include <api/libcas/cdca/pub_st.h>
#include <api/libcas/cdca/CDCAS30.h>

#include "cd_dmx.h"
#include "cd_catask.h"


static OSAL_ID cas_sema = OSAL_INVALID_ID;
#define CAS_DMX_MUTEX_INIT()	if (cas_sema == OSAL_INVALID_ID) {\
			if ((cas_sema = osal_semaphore_create(1))==OSAL_INVALID_ID){\
			CAS_DMXERR_PRINTF("create semaphore failure with %d\r", cas_sema); \
			}}
#define CAS_DMX_MUTEX_DEL()	if (cas_sema != OSAL_INVALID_ID) {\
			osal_semaphore_delete(cas_sema);\
			cas_sema = OSAL_INVALID_ID;}
#define CAS_DMX_MUTEX_LOCK()	if(cas_sema!=OSAL_INVALID_ID) {while(osal_semaphore_capture(cas_sema, 1000) != SUCCESS) \
		CAS_DMXERR_PRINTF("cas_sema capture timeout\n");}
#define CAS_DMX_MUTEX_UNLOCK()	if(cas_sema!=OSAL_INVALID_ID) osal_semaphore_release(cas_sema);


static struct cas_channel *channel_array[CAS_MAX_CHAN_NUM];
static struct dmx_device *cas_dmx_dev = NULL;
static void (*cas_demux_callback)(UINT8 req_id, BOOL bOK, UINT16 pid, UINT8* sec_data, UINT16 length);
static UINT32 cas_waiting_nit = 0;// higher 2B==section length, the lower 2B == data enable(0x55**)& NIT request(0x**55)
static UINT8 cas_nit_section[1024+1];

/*****************************************************************************
* Interal functions
******************************************************************************/
INT32 cas_demux_init(void (*dmx_cb)(UINT8 req_id, BOOL bOK, UINT16 pid, UINT8* sec_data, UINT16 length))
{
	UINT8 i;
	struct cas_channel *tmp_channel;
	CAS_DMX_MUTEX_INIT();
#if (SYS_CHIP_MODULE == ALI_S3601 || SYS_CHIP_MODULE == ALI_S3602)
	cas_dmx_dev =  (struct dmx_device *)dev_get_by_name("DMX_S3601_0");
#else
	cas_dmx_dev =  (struct dmx_device*)dev_get_by_name("DMX_M3327_0");
#endif
	dmx_io_control(cas_dmx_dev, DMX_BYPASS_CSA, 0);	
	for(i=0; i<CAS_MAX_CHAN_NUM; i++)
	{
		tmp_channel = (struct cas_channel*) MALLOC(sizeof(struct cas_channel));
		if(tmp_channel == NULL)
		{
			CAS_DMXERR_PRINTF("cas_demux_request: malloc error!\n");
			return CDCA_FALSE;
		}
		MEMSET(tmp_channel, 0, sizeof(struct cas_channel));
		tmp_channel->get_sec_param.pid = INVALID_PID;
		channel_array[i] = tmp_channel;
	}
	cas_demux_callback = dmx_cb;
	return CDCA_TRUE;
}

static void conti_rcv_sec_cb(struct get_section_param * param)
{
	UINT8 i;
	/*call cas callback function*/
	struct cas_channel *tmp_channle = (struct cas_channel *)param;
	if(param->sec_hit_num >= MAX_MV_NUM)
		return;
	if(((1<<param->sec_hit_num) & param->mask_value->tb_flt_msk)==0)
		return;
	CAS_DMX_PRINTF("data got! req_id:%d, pid: 0x%04x\n", tmp_channle->req_id[param->sec_hit_num], param->pid);
	CDCASTB_PrivateDataGot(tmp_channle->req_id[param->sec_hit_num], CDCA_TRUE, param->pid, param->buff, param->sec_tbl_len);

	if (CAS_DMX_ECM_REQUESTID == tmp_channle->req_id[0])//if ECM, release channel
	{
		for(i=0; i<MAX_SEC_MASK_LEN; i++)
		{
			tmp_channle->mask_value.value[param->sec_hit_num][i] = 0;
			tmp_channle->mask_value.multi_mask[param->sec_hit_num][i] = 0;
		}
		tmp_channle->mask_value.tb_flt_msk &= ~(1<<param->sec_hit_num);
	}
	return;
}

void cas_demxu_set_mv(struct restrict *mask_value, UINT8 index, UINT8* value, UINT8* mask, UINT8 mask_len)
{
	UINT8 i;
	if((mask_value->tb_flt_msk & (1<<index)) != 0)
	{
		CAS_DMX_PRINTF("cas_demxu_set_mv: filter will be resetted\n");
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
	return;
}


INT32 cas_demux_request(UINT8 req_id, UINT8* value, UINT8* mask, UINT8 mask_len, UINT16 pid, UINT8 wait_seconds)
{
	UINT8 i;
	UINT8 j;
	UINT8 k;
	UINT8 l;
	UINT8 m;
	struct cas_channel *tmp_channel;
	UINT32 tmp_time_out;

	if(pid == SI_NIT_PID)
	{
		cas_waiting_nit &= 0xFFFFFF00;//0x55 is a flag
		cas_waiting_nit |= 0x55;//0x55 is a flag
		CAS_DMX_PRINTF("cas_demux_request: nit!\n");		
		return CDCA_TRUE;
	}
	
	if((value==NULL)||(mask==NULL)||(mask_len==0))
	{
		CAS_DMXERR_PRINTF("cas_demux_request: error param!\n");
		return CDCA_FALSE;
	}
	else
	{
		CAS_DMX_MUTEX_LOCK();
		j = CAS_MAX_CHAN_NUM;
		for(i=CAS_MAX_CHAN_NUM-1; i!=0xFF; i--)
		{
			tmp_channel = channel_array[i];
			if(tmp_channel->get_sec_param.pid == INVALID_PID)	/*the first free*/
				j = i;
			else if(tmp_channel->get_sec_param.pid == pid)
				break;
		}
		if(i != 0xFF)
		{
			/*channel for the pid has been open*/
		//	CAS_DMX_PRINTF("cas_demux_request: opened. pid: 0x%04x\n", tmp_channel->get_sec_param.pid);		
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
				//CAS_DMX_PRINTF("cas_demux_request: reset mv. index:%02x , req_id: %02x\n", l, tmp_channel->req_id[l]);
				cas_demxu_set_mv(&(tmp_channel->mask_value), l, value, mask, mask_len);
			}
			else if(m == MAX_MV_NUM)
			{
				CAS_DMX_MUTEX_UNLOCK();
				CAS_DMXERR_PRINTF("cas_demux_request: no filter!\n");
				return CDCA_FALSE;
			}
			else
			{
				/*set new mask value to filter*/
				tmp_channel->req_id[m] = req_id;
				//CAS_DMX_PRINTF("cas_demux_request: set mv. index:%02x , req_id: %02x\n", m, tmp_channel->req_id[m]);				
				cas_demxu_set_mv(&(tmp_channel->mask_value), m, value, mask, mask_len);
			}
			CAS_DMX_MUTEX_UNLOCK();
			return CDCA_TRUE;
		}
		else if(j == CAS_MAX_CHAN_NUM)
		{
			CAS_DMX_MUTEX_UNLOCK();
			CAS_DMXERR_PRINTF("cas_demux_request: no channel!\n");
			return CDCA_FALSE;
		}
		else
		{
			//CAS_DMX_PRINTF("cas_demux_request: open new channel. pid:0x%04x , req_id: %02x\n", pid, req_id);
			
			/*open new channel for the pid*/
			tmp_channel = channel_array[j];
			tmp_channel->req_id[0] = req_id;
			
			cas_demxu_set_mv(&(tmp_channel->mask_value), 0, value, mask, mask_len);

			tmp_channel->get_sec_param.pid = pid;
			tmp_channel->get_sec_param.buff = (UINT8 *)&(tmp_channel->sec_buf);
			tmp_channel->get_sec_param.buff_len = CAS_MAX_SEC_SIZE;
			if(wait_seconds == 0)
				tmp_time_out = OSAL_WAIT_FOREVER_TIME;
			else
				tmp_time_out = wait_seconds*1000;
			tmp_channel->get_sec_param.wai_flg_dly = tmp_time_out;
			tmp_channel->get_sec_param.mask_value = &(tmp_channel->mask_value);
			tmp_channel->get_sec_param.continue_get_sec = 1;
			tmp_channel->get_sec_param.get_sec_cb = conti_rcv_sec_cb;

			if(dmx_async_req_section(cas_dmx_dev, &(tmp_channel->get_sec_param), &(tmp_channel->flt_id)) != RET_SUCCESS)
			{
				CAS_DMX_MUTEX_UNLOCK();
				CAS_DMXERR_PRINTF("cas_demux_request: no hardware channel\n");
				return CDCA_FALSE;
			}
			CAS_DMX_MUTEX_UNLOCK();
			OTHER_PRINTF("cas_demux_request: filter %d\n", tmp_channel->flt_id);
			return CDCA_TRUE;
		}
	}
}

void cas_demxu_free_channel(UINT8 req_id)
{
	UINT8 i;
	UINT8 j;
	struct cas_channel *tmp_channel;
	CAS_DMX_MUTEX_LOCK();
	for(i=0; i<CAS_MAX_CHAN_NUM; i++)
	{
		tmp_channel = channel_array[i];
		for(j=0; j<MAX_MV_NUM; j++)
		{
			if(tmp_channel->req_id[j] == req_id)
			{
				CAS_DMX_PRINTF("cas_dmx_free: req_id:0x%02x, flt_id:0x%02x\n", req_id, tmp_channel->flt_id);
				dmx_io_control(cas_dmx_dev, IO_ASYNC_CLOSE, tmp_channel->flt_id);
				MEMSET(tmp_channel, 0, sizeof(struct cas_channel));
				tmp_channel->get_sec_param.pid = INVALID_PID;
				break;
			}
		}
	}
	CAS_DMX_MUTEX_UNLOCK();
}

void cas_demux_free(UINT8 free_type)	// 1, free ecm; 2, free emm
{
	UINT8 i;
	UINT8 req_id;
	struct cas_channel *tmp_channel;
	
	if(CAS_DMX_ECM_REQUESTID == free_type)	// free req_id == 1
	{
		cas_demxu_free_channel(1);
	}
	else if(CAS_DMX_EMM_REQUESTID == free_type)	// free req_id >= 2
	{
		for(i=2; i<7; i++)	
			cas_demxu_free_channel(i);
	}
	return;
}

INT32 cas_got_nit_data(UINT32 section, UINT32 section_length)
{
	UINT32 len;

	if (0x55 != (cas_waiting_nit & 0x00ff))//no request for NIT, don't update data
		return;
	
	if (section_length >1024)
	{
		CAS_DMXERR_PRINTF("!!%s: section is too large!\n", __FUNCTION__);
		len = 1024;
	}
	else
		len = section_length;
	
	cas_waiting_nit &= 0x000000FF;
	MEMSET(cas_nit_section, 0, sizeof(cas_nit_section));
	MEMCPY(cas_nit_section, (UINT8 *)section, len);
	cas_waiting_nit |= 0x00005500;
	cas_waiting_nit |= (len<<16);
	
}
void cas_nit_callback()
{
	if (0x55 != (cas_waiting_nit & 0x00ff))//no request for NIT
		return;
	if (0x5500 !=(cas_waiting_nit & 0x0000ff00))//No NIT data got
		return;
	CAS_DMX_PRINTF("CAS got NIT\n");
	CDCASTB_PrivateDataGot(1, CDCA_TRUE, SI_NIT_PID, cas_nit_section, (cas_waiting_nit>>16));
	cas_waiting_nit &= 0x00000000;
}

/*****************************************************************************
* Tongfang CAS interface implementation
******************************************************************************/

extern CDCA_BOOL CDSTBCA_GetPrivateData(CDCA_U8 byReqID, const CDCA_U8* pbyFilter, const CDCA_U8* pbyMask, CDCA_U8 byLen, CDCA_U16 wPid, CDCA_U8 byWaitSeconds)
{
	UINT8 i;
	UINT8 value[MAX_SEC_MASK_LEN];
	UINT8 mask[MAX_SEC_MASK_LEN];

	CAS_DMX_PRINTF("STBCA_GetPrivateData:PID: %04x, req_id:%02x\n", wPid, byReqID);	
#if 0
	//CAS_DMX_PRINTF("STBCA_GetPrivateData:req_id:%02x, timeout:%02x\n", byReqID, byWaitSeconds);	
	CAS_DMX_PRINTF("pid:%04x, MVlen:%02x\n", wPid, byLen);
	CAS_DMX_PRINTF("Value: ");
	CAS_DMX_DUMP(pbyFilter, 8);
	CAS_DMX_PRINTF("Mask: ");
	CAS_DMX_DUMP(pbyMask, 8);
#endif
	MEMSET(value, 0, sizeof(value));
	MEMSET(mask, 0, sizeof(mask));
	value[0] = pbyFilter[0];
	mask[0] = pbyMask[0];
	for(i=1; i<byLen-2; i++)
	{
		value[i+2] = pbyFilter[i];
		mask[i+2] = pbyMask[i];
	}

	return cas_demux_request(byReqID, value, mask, byLen, wPid, byWaitSeconds);

}

extern void	CDSTBCA_ScrSetCW(CDCA_U16 wEcmPID, const CDCA_U8* pbyOddKey, const CDCA_U8* pbyEvenKey, CDCA_U8 byKeyLen, CDCA_BOOL bTapingEnabled)
{
	UINT8 i;
	UINT16 ret;
	UINT8 rating;
	UINT8 pin[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	struct dmx_device *dmx_dev;	

#ifdef CHCHG_TICK_PRINTF
	libc_printf("####CHCHG %s():line%d, ecmPID0x%x, tick=%d\n", __FUNCTION__,__LINE__, wEcmPID,osal_get_tick());
#endif

	CAS_DMX_PRINTF("STBCA_ScrSetCW:ecmPID:%04x @%d\n", wEcmPID,osal_get_tick());
	dmx_dev = (struct dmx_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DMX);
	if(dmx_dev == NULL)
	{
		CAS_DMXERR_PRINTF("STBCA_ScrSetCW: Dev_get_by_type error!\n");
		return;
	}

	if(RET_SUCCESS!=dmx_io_control(dmx_dev, DMX_IS_TS_ENTER_CSA, 0))
		dmx_io_control(dmx_dev, DMX_BYPASS_CSA, 0);
	
//	CAS_DMX_PRINTF("Odd : ");
//	CAS_DMX_DUMP(pbyOddKey, byKeyLen);
//	CAS_DMX_PRINTF("Even: ");
//	CAS_DMX_DUMP(pbyEvenKey, byKeyLen);
	
	for(i=0; i<2;i++)
	{
	if(cas_ecm_pid.uniform_ecm_pid == wEcmPID)
	{
		dmx_cfg_cw(dmx_dev, DES_VIDEO, 2, (UINT32 *)pbyEvenKey);
		dmx_cfg_cw(dmx_dev, DES_VIDEO, 3, (UINT32 *)pbyOddKey);
		dmx_cfg_cw(dmx_dev, DES_AUDIO, 2, (UINT32 *)pbyEvenKey);
		dmx_cfg_cw(dmx_dev, DES_AUDIO, 3, (UINT32 *)pbyOddKey);
		dmx_cfg_cw(dmx_dev, DES_TTX, 2, (UINT32 *)pbyEvenKey);
		dmx_cfg_cw(dmx_dev, DES_TTX, 3, (UINT32 *)pbyOddKey);
		dmx_cfg_cw(dmx_dev, DES_SUP, 2, (UINT32 *)pbyEvenKey);
		dmx_cfg_cw(dmx_dev, DES_SUP, 3, (UINT32 *)pbyOddKey);
	}
	else if(cas_ecm_pid.video_ecm_pid == wEcmPID)
	{
		dmx_cfg_cw(dmx_dev, DES_VIDEO, 2, (UINT32 *)pbyEvenKey);
		dmx_cfg_cw(dmx_dev, DES_VIDEO, 3, (UINT32 *)pbyOddKey);
	} 
	else if(cas_ecm_pid.audio_ecm_pid == wEcmPID)
	{
		dmx_cfg_cw(dmx_dev, DES_AUDIO, 2, (UINT32 *)pbyEvenKey);
		dmx_cfg_cw(dmx_dev, DES_AUDIO, 3, (UINT32 *)pbyOddKey);
	}
	else if(cas_ecm_pid.ttx_ecm_pid == wEcmPID)
	{
		dmx_cfg_cw(dmx_dev, DES_TTX, 2, (UINT32 *)pbyEvenKey);
		dmx_cfg_cw(dmx_dev, DES_TTX, 3, (UINT32 *)pbyOddKey);
	}
	else if(cas_ecm_pid.subt_ecm_pid == wEcmPID)
	{
		dmx_cfg_cw(dmx_dev, DES_SUP, 2, (UINT32 *)pbyEvenKey);
		dmx_cfg_cw(dmx_dev, DES_SUP, 3, (UINT32 *)pbyOddKey);
	}
	
	}
	return;
}


