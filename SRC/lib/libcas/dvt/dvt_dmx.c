/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2008 Copyright (C)
 *
 *  File: dvt_sc.c
 *
 *  Description: This file fills the Demux related API of DVT CA library.
 *
 *  History:
 *           Date            Author         Version       Comment
 *      ========    ======      ======     =========
 *  1.  2008.01.31      Grady Xu       0.1.000       Create this file
 *  2.  2008.02.19      Grady Xu       0.2.000       modify for the section request
 *
 ****************************************************************************/
#include <sys_config.h>
#include <types.h>
#include <retcode.h>
#include <hld/hld_dev.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/alloc.h>
#include <api/libc/string.h>
#include <api/libchunk/chunk.h>
#include <hld/dmx/dmx.h>

#include <api/libcas/dvt/DVTCAS_STBDataPublic.h>
#include <api/libcas/dvt/DVTSTB_CASInf.h>
#include <api/libcas/dvt/cas_dvt.h>
#include "dvt_dmx.h"

static OSAL_ID cas_sema = OSAL_INVALID_ID;
#define CAS_DMX_MUTEX_INIT()	if (cas_sema == OSAL_INVALID_ID) {\
			if ((cas_sema = osal_semaphore_create(1))==OSAL_INVALID_ID){\
			CAS_DMXERR_PRINTF("create semaphore failure with %d\r", cas_sema); \
			}}
#define CAS_DMX_MUTEX_DEL()	if (cas_sema != OSAL_INVALID_ID) {\
			osal_semaphore_delete(cas_sema);\
			cas_sema = OSAL_INVALID_ID;}
#define CAS_DMX_MUTEX_LOCK();//	if(cas_sema!=OSAL_INVALID_ID) {while(osal_semaphore_capture(cas_sema, 1000) != SUCCESS) \
		//CAS_DMXERR_PRINTF("%s: capture timeout\n", __FUNCTION__);}
#define CAS_DMX_MUTEX_UNLOCK();//	if(cas_sema!=OSAL_INVALID_ID) osal_semaphore_release(cas_sema);


struct cas_channel *channel_array[CAS_MAX_CHAN_NUM];
static struct dmx_device *cas_dmx_dev = NULL;
static UINT8 dvt_dmx_stop_flag  = TRUE;
//static void (*cas_demux_callback)(UINT8 req_id, bool bOK, UINT16 pid, UINT8* sec_data, UINT16 length);

/**************************************************************************
 							****Internal Functions****
 **************************************************************************/
INT8 EmmAloSend;
INT8 EmmInfSend;
INT8 EmmValSend;
INT8 EmmOESend;
UINT8 bOESend = 0;
UINT8 bInfSend = 0;
UINT8 bValSend = 0;

UINT8 EMM2_BACK[188];
UINT8 EMM3_BACK[188];
UINT8 EMM4_BACK[188];
UINT8 EMM5_BACK[188];


static UINT32 GYCAS_InfSend_time;
static UINT32 GYCAS_OESend_time;
static UINT32 GYCAS_ValSend_time;

#define INTERVAL 600
const UINT32 crc32_table[] =
    {
        0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005,
        0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61, 0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd,
        0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9, 0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
        0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3, 0x709f7b7a, 0x745e66cd,
        0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039, 0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5,
        0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
        0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49, 0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95,
        0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1, 0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d,
        0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
        0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca,
        0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde, 0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02,
        0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1, 0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
        0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692,
        0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6, 0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a,
        0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e, 0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
        0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34, 0xdc3abded, 0xd8fba05a,
        0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637, 0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb,
        0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f, 0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
        0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5, 0x3f9b762c, 0x3b5a6b9b,
        0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff, 0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623,
        0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
        0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f, 0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3,
        0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7, 0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b,
        0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
        0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c,
        0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8, 0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24,
        0x119b4be9, 0x155a565e, 0x18197087, 0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
        0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654,
        0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0, 0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c,
        0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18, 0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
        0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c,
        0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668, 0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
    };

UINT32 calCRC32(UINT8 *sectionData)
{
	UINT32 i;
	UINT32 section_length;
	UINT32 crc = 0xFFFFFFFF;

	UINT32 crcTemp = 0;

	section_length = ((((sectionData[1] & 0x0F) << 8) + sectionData[2]) + 3);
	for ( i = 0; i < (section_length - 4); i++)
	{
		crc = crc32_table[((crc>>24) ^ sectionData[i]) & 0xFF] ^ (crc << 8);
	}

	crcTemp = (~(crc ^ 0xFFFFFFFF));
	sectionData[section_length - 4] = ((crcTemp&0xFF000000)>>24);
	sectionData[section_length - 3] = ((crcTemp&0x00FF0000)>>16);
	sectionData[section_length - 2] = ((crcTemp&0x0000FF00)>>8);
	sectionData[section_length - 1] = crcTemp&0x000000FF;

	return (crc = (~(crc ^ 0xFFFFFFFF)));
}



INT32 cas_demux_init()
{
	UINT8 i;
	struct cas_channel *tmp_channel;
	
	CAS_DMX_MUTEX_INIT();
#if (SYS_CHIP_MODULE == ALI_S3601 || SYS_CHIP_MODULE == ALI_S3602)
	cas_dmx_dev = (struct dmx_device *)dev_get_by_name("DMX_S3601_0");
#else
	cas_dmx_dev = (struct dmx_device*)dev_get_by_name("DMX_M3327_0");
#endif
	//dmx_io_control(cas_dmx_dev, DMX_BYPASS_CSA, 0);	

	for(i=0; i<CAS_MAX_CHAN_NUM; i++)
	{
		tmp_channel = (struct cas_channel*) MALLOC(sizeof(struct cas_channel));
		if(tmp_channel == NULL)
		{
			CAS_DMXERR_PRINTF("Err: %s-malloc error!\n", __FUNCTION__);
			return false;
		}
		MEMSET(tmp_channel, 0, sizeof(struct cas_channel));
		tmp_channel->get_sec_param.pid = INVALID_PID;
		channel_array[i] = tmp_channel;
	}
	
	return true;
}

void cas_demux_free_channel(UINT8 req_id)
{
	UINT8 i;
	UINT8 j;
	struct cas_channel *tmp_channel;
	//CAS_DMX_MUTEX_LOCK();
	for(i=0; i<CAS_MAX_CHAN_NUM; i++)
	{
		tmp_channel = channel_array[i];
		for(j=0; j<MAX_MV_NUM; j++)
		{
			if(tmp_channel->req_id[j] == req_id)
			{
				CAS_DMX_PRINTF("cas_dmx_free: req_id:0x%02x, flt_id:0x%02x\n", req_id, tmp_channel->flt_id);
				dmx_io_control(cas_dmx_dev, IO_ASYNC_CLOSE, tmp_channel->flt_id);
				CAS_DMX_PRINTF("cas_dmx_free: req_id:0x%02x, flt_id:0x%02x 2\n", req_id, tmp_channel->flt_id);
				MEMSET(tmp_channel, 0, sizeof(struct cas_channel));
				tmp_channel->get_sec_param.pid = INVALID_PID;
				break;
			}
		}
	}
	//CAS_DMX_MUTEX_UNLOCK();
}

void cas_demux_free(UINT8 free_type)	// 1, free ecm; 2, free emm
{
	UINT8 i;
	UINT8 req_id;
	struct cas_channel *tmp_channel;
	
	if(DVT_ECM_REQUEST_ID == free_type)	// free req_id == 1
	{
		cas_demux_free_channel(1);
	}
	else if(DVT_EMM_REQUEST_ID == free_type)	// free req_id >= 2
	{
		for(i=2; i<4; i++)	
			cas_demux_free_channel(i);
	}
	return;
}

void cas_dmx_timeout_check()
{
	UINT8 i,j;
	UINT32 cur_tick;

	for (i=0; i<CAS_MAX_CHAN_NUM; i++)
	{
		for (j=0; j<CAS_MAX_CHAN_NUM; j++)
		{
			cur_tick = osal_get_tick();
			if ((TRUE == channel_array[i]->status[j].enable)
				&& (OSAL_WAIT_FOREVER_TIME != channel_array[i]->status[j].timeout)
				&&(cur_tick-channel_array[i]->status[j].tick > channel_array[i]->status[j].timeout))
			{
				CAS_DMX_PRINTF("$$$DMX timeout! req:%x-pid:%x\n",channel_array[i]->req_id[j],channel_array[i]->get_sec_param.pid);
				osal_task_dispatch_off();
				channel_array[i]->status[j].enable = FALSE;
				osal_task_dispatch_on();
				GYCA_TableReceived(channel_array[i]->req_id[j], false, false,channel_array[i]->get_sec_param.pid, NULL, 0);
				//DVTCASTB_StreamGuardDataGot(channel_array[i]->req_id[j], false,channel_array[i]->get_sec_param.pid,0, NULL);
				//cas_demxu_free_channel(channel_array[i]->req_id);
			}
			
			//ECM PID, only 1 mask&value
			if (DVT_ECM_CHANNEL_ID == i)
				break;
		}
			
	}
	
}

static void conti_rcv_sec_cb(struct get_section_param * param)
{
#if 1
	UINT8 i;
	UINT32 temp;
	/*call tfcas callback function*/
	static UINT8 data[4];
	struct cas_channel *tmp_channle = (struct cas_channel *)param;
      UINT8 tst;
	//GYCAS_DMX_PRINTF("\n>>[%02x]\n",param->buff[0]);	
	if(param->sec_hit_num >= MAX_MV_NUM)
		return;
	if(((1<<param->sec_hit_num) & param->mask_value->tb_flt_msk)==0)
		return;
    if(tmp_channle->req_id[param->sec_hit_num] == 4 && EmmAloSend != 1)
		return;
	if(tmp_channle->req_id[param->sec_hit_num] == 4 && EmmAloSend == 1)
		EmmAloSend = 0;

	if(tmp_channle->req_id[param->sec_hit_num] == 5 && EmmInfSend != 1)
		return;
	if(tmp_channle->req_id[param->sec_hit_num] == 5 && EmmInfSend == 1)
	{
		temp = osal_get_tick() -GYCAS_InfSend_time;
		    if (temp >INTERVAL)
			{
			    EmmInfSend = 0;
				bInfSend = 1;
				//libc_printf_gyca("\nSID=%d,SPid=%d\n",tmp_channle->req_id[param->sec_hit_num],param->pid);

		    }
			else
				return;
	    //osal_delay(SLEEP_TIME);
	}
	
    if(tmp_channle->req_id[param->sec_hit_num] == 2 && EmmValSend != 1)
		return;
	if(tmp_channle->req_id[param->sec_hit_num] == 2 && EmmValSend == 1)
	{
		temp = osal_get_tick() -GYCAS_ValSend_time;
		    if (temp >700)
			{
			    EmmValSend = 0;
				bValSend = 1;

		    }
			else
				return;
		//osal_delay(SLEEP_TIME);
	}

	
	if(tmp_channle->req_id[param->sec_hit_num] == 3 && EmmOESend != 1)
		return;
	if(tmp_channle->req_id[param->sec_hit_num] == 3 && EmmOESend == 1)
	{
		temp = osal_get_tick() -GYCAS_OESend_time;
	    if (temp >INTERVAL)
		{
		    EmmOESend = 0;
			bOESend = 1;

	    }
		else
			return;
		//osal_delay(SLEEP_TIME);
	}
	CAS_DMX_PRINTF("\nSID=%d,SPid=%d\n",tmp_channle->req_id[param->sec_hit_num],param->pid);


	if((param->buff[0]==0x80)||(param->buff[0]==0x81))
	{
		
		if(!calCRC32(param->buff))
		{
			CAS_DMX_PRINTF("\ncrc error...\n");
			return;
		}
		else
		{
			CAS_DMX_PRINTF("\ncrc ok...\n");
		}

	}
	
      CAS_DMX_PRINTF("\n\n###############data got! req_id:%d, pid: 0x%04x\n\n", tmp_channle->req_id[param->sec_hit_num], param->pid);

	/*提交数据给ca 库*/
	//tmp_channle->get_sec_param.continue_get_sec = 0;/*临时这么改*/
	//libc_printf_gyca("\nSID=%d,SPid=%d\n",tmp_channle->req_id[param->sec_hit_num],param->pid);
	GYCA_TableReceived(tmp_channle->req_id[param->sec_hit_num],1,param->pid,	param->buff,param->sec_tbl_len);	
	if (DVT_ECM_REQUEST_ID == tmp_channle->req_id[0])//if ECM, release channel
	{
		for(i=0; i<MAX_SEC_MASK_LEN; i++)
		{
			tmp_channle->mask_value.value[param->sec_hit_num][i] = 0;
			tmp_channle->mask_value.multi_mask[param->sec_hit_num][i] = 0;
		}
		tmp_channle->mask_value.tb_flt_msk &= ~(1<<param->sec_hit_num);
	}
	/*if(tmp_channle->req_id[param->sec_hit_num] == 4 && EmmSend == 0)
		gycas_demxu_free_channel(4);*/
	return;
#endif
#if 0
	UINT8 i;
	/*call cas callback function*/
	struct cas_channel *tmp_channle = (struct cas_channel *)param;
	if(param->sec_hit_num >= MAX_MV_NUM)
		return;
	if(((1<<param->sec_hit_num) & param->mask_value->tb_flt_msk)==0)
	{
		CAS_DMX_PRINTF("%s: not match!\n", __FUNCTION__);
		return;
	}
	if (TRUE != tmp_channle->status[param->sec_hit_num].enable)
	{
		CAS_DMX_PRINTF("abandan data! req_id:%d, pid: 0x%04x\n", tmp_channle->req_id, param->pid);
		return;
	}
	CAS_DMX_PRINTF("data got! req_id:%d, pid: 0x%04x\n", tmp_channle->req_id[param->sec_hit_num], param->pid);
	osal_task_dispatch_off();
	tmp_channle->status[param->sec_hit_num].enable = FALSE;
	osal_task_dispatch_on();
	GYCA_TableReceived(tmp_channle->req_id[param->sec_hit_num], true, param->pid, param->buff, param->sec_tbl_len);
	//DVTCASTB_StreamGuardDataGot(tmp_channle->req_id[param->sec_hit_num], true, param->pid, param->sec_tbl_len, param->buff);
	return;
#endif
}

void cas_demux_set_mv(struct restrict *mask_value, UINT8 index, UINT8* value, UINT8* mask, UINT8 mask_len)
{
	UINT8 i;
	if((mask_value->tb_flt_msk & (1<<index)) != 0)
	{
		CAS_DMX_PRINTF("%s -filter will be reset!\n", __FUNCTION__);
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

}

INT32 cas_demux_request(UINT8 req_id, UINT8* value, UINT8* mask, UINT8 mask_len, UINT16 pid, UINT8 wait_seconds)
{
	UINT8 i,j,k,l,m;
	UINT8 reqid_flag = FALSE, pid_flag = FALSE, free_channel = 0xFF, channel_id = 0xff;
	struct cas_channel *tmp_channel;
	UINT32 tmp_time_out = (wait_seconds )?((UINT32)wait_seconds*1000):OSAL_WAIT_FOREVER_TIME;
#if 1
	 if(req_id == 4)	
		EmmAloSend = 1;
	if(req_id == 5)	
	{
	    EmmInfSend = 1;
		if(bInfSend)
		{
		    GYCAS_InfSend_time = osal_get_tick();
			bInfSend = 0;
		}
	}
	if(req_id == 2)	
	{
	    EmmValSend = 1;
		if(bValSend)
		{
		    GYCAS_ValSend_time = osal_get_tick();
			bValSend = 0;
		}
	}
	if(req_id == 3)	
	{
		EmmOESend = 1;
		if(bOESend)
		{
		    GYCAS_OESend_time = osal_get_tick();
			bOESend = 0;
		}
	}
	CAS_DMX_PRINTF("gycas_demux_request:pid[%d]#### mask_len[%d]\n",mask_len,pid);
	
	if((value==NULL)||(mask==NULL)||(mask_len==0))
	{
		CAS_DMXERR_PRINTF("gycas_demux_request: error param!\n");
		return CAS_FAIL;
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
			//GYCAS_DMX_PRINTF("\nchannel for the pid has been open\n");
			//GYCAS_DMX_PRINTF("tfcas_demux_request: opened. pid: 0x%04x\n", tmp_channel->get_sec_param.pid);	

			tmp_channel->get_sec_param.continue_get_sec = 1;/*临时这么改*/
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
				CAS_DMX_PRINTF("gycas_demux_request: reset mv. index:%02x , req_id: %02x\n", l, tmp_channel->req_id[l]);
				cas_demux_set_mv(&(tmp_channel->mask_value), l, value, mask, mask_len);
			}
			else if(m == MAX_MV_NUM)
			{
				CAS_DMX_MUTEX_UNLOCK();
				CAS_DMXERR_PRINTF("gycas_demux_request: no filter!\n");
				return CAS_FAIL;
			}
			else
			{
				/*set new mask value to filter*/
				tmp_channel->req_id[m] = req_id;
				CAS_DMX_PRINTF("########tfcas_demux_request: set mv. index:%02x , req_id: %02x\n", m, tmp_channel->req_id[m]);				
				cas_demux_set_mv(&(tmp_channel->mask_value), m, value, mask, mask_len);
				if(req_id == 5)
					{
					GYCAS_InfSend_time = osal_get_tick();
					MEMSET(EMM5_BACK,0,188);
					}
				if(req_id == 3)
					{
					GYCAS_OESend_time = osal_get_tick();
					MEMSET(EMM3_BACK,0,188);
					}
				if(req_id == 2)
					{
					GYCAS_ValSend_time = osal_get_tick();
					MEMSET(EMM2_BACK,0,188);
					}
			}
			CAS_DMX_MUTEX_UNLOCK();
			return 	SUCCESS;
		}
		else if(j == CAS_MAX_CHAN_NUM)
		{
			CAS_DMX_MUTEX_UNLOCK();
			CAS_DMXERR_PRINTF("gycas_demux_request: no channel!\n");
			return CAS_FAIL;
		}
		else
		{
			CAS_DMX_PRINTF("gycas_demux_request: open new channel. pid:0x%04x , req_id: %02x\n", pid, req_id);
			
			/*open new channel for the pid*/
			tmp_channel = channel_array[j];
			tmp_channel->req_id[0] = req_id;
			
			cas_demux_set_mv(&(tmp_channel->mask_value), 0, value, mask, mask_len);
			tmp_channel->get_sec_param.pid = pid;
			tmp_channel->get_sec_param.buff = (UINT8 *)&(tmp_channel->sec_buf);
			tmp_channel->get_sec_param.buff_len = CAS_MAX_SEC_SIZE;
			if(wait_seconds == 0)
				tmp_time_out = OSAL_WAIT_FOREVER_TIME;
			else
				tmp_time_out = wait_seconds*1000;
			tmp_channel->get_sec_param.wai_flg_dly = tmp_time_out;
			tmp_channel->get_sec_param.mask_value = &(tmp_channel->mask_value);
			tmp_channel->get_sec_param.continue_get_sec = 1;/*每次只收一个section怎么做,luqz@20101121*/
			tmp_channel->get_sec_param.get_sec_cb = conti_rcv_sec_cb;

			if(dmx_async_req_section(cas_dmx_dev, &(tmp_channel->get_sec_param), &(tmp_channel->flt_id)) != RET_SUCCESS)
			{
				CAS_DMX_MUTEX_UNLOCK();
				CAS_DMXERR_PRINTF("gycas_demux_request: dmx_async_req_section() failed\n");
				return CAS_FAIL;
			}
			CAS_DMX_MUTEX_UNLOCK();
			CAS_DMX_PRINTF("gycas_demux_request: filter %d\n", tmp_channel->flt_id);
			return SUCCESS;
		}
	}
#endif	
#if 0
	if((value==NULL)||(mask==NULL)||(mask_len==0))
	{
		CAS_DMXERR_PRINTF("Err: %s- error param!\n",__FUNCTION__);
		return CAS_FAIL;
	}
	
	CAS_DMX_MUTEX_LOCK();
	if (DVT_ECM_REQUEST_ID == req_id) //ECM request channel, the pid would be different
	{
		tmp_channel = channel_array[DVT_ECM_CHANNEL_ID];
		if (pid != tmp_channel->get_sec_param.pid)
		{
			if (INVALID_PID != tmp_channel->get_sec_param.pid)
				cas_demux_free_channel(DVT_ECM_REQUEST_ID);
			free_channel = 0;
		}
		else
		{
			osal_task_dispatch_off();
			cas_demux_set_mv(&(tmp_channel->mask_value), 0, value, mask, mask_len);
			tmp_channel->status[0].enable = TRUE;
			tmp_channel->status[0].tick = osal_get_tick();
			tmp_channel->status[0].timeout = tmp_time_out;
			osal_task_dispatch_on();
			CAS_DMX_MUTEX_UNLOCK();
			return CAS_SUCCESS;
		}
	}
	else //EMM request channel,
	{
		j = CAS_MAX_CHAN_NUM;
		for(i=CAS_MAX_CHAN_NUM-1; i>0;  i--)
		{
			tmp_channel = channel_array[i];
			if (tmp_channel->get_sec_param.pid == INVALID_PID)
				j = i;
			 if(tmp_channel->get_sec_param.pid == pid)
				break;
		}
		if (0 != i) // channel for this pid has been opened
		{
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
				CAS_DMX_PRINTF("%s: reset mv. index:%02x , req_id: %02x\n", __FUNCTION__,l, tmp_channel->req_id[l]);
				osal_task_dispatch_off();
				cas_demux_set_mv(&(tmp_channel->mask_value), l, value, mask, mask_len);
				tmp_channel->status[l].enable = TRUE;
				tmp_channel->status[l].tick = osal_get_tick();
				tmp_channel->status[l].timeout = tmp_time_out;
				osal_task_dispatch_on();
				CAS_DMX_MUTEX_UNLOCK();
				return CAS_SUCCESS;
			}
			else if(m == MAX_MV_NUM)
			{
				CAS_DMXERR_PRINTF("Err: %s - no MV!\n", __FUNCTION__);
				CAS_DMX_MUTEX_UNLOCK();
				return CAS_FAIL;
			}
			else
			{
				/*set new mask value to filter*/
				tmp_channel->req_id[m] = req_id;
				CAS_DMX_PRINTF("%s: set mv. index:%02x , req_id: %02x\n", __FUNCTION__, m, tmp_channel->req_id[m]);				
				osal_task_dispatch_off();
				cas_demux_set_mv(&(tmp_channel->mask_value), m, value, mask, mask_len);
				tmp_channel->status[m].enable = TRUE;
				tmp_channel->status[m].tick = osal_get_tick();
				tmp_channel->status[m].timeout = tmp_time_out;
				osal_task_dispatch_on();
				CAS_DMX_MUTEX_UNLOCK();
				return CAS_SUCCESS;
			}
		}
		else if (j == CAS_MAX_CHAN_NUM)
		{
			CAS_DMXERR_PRINTF("Err: %s - no channel!\n", __FUNCTION__);
			CAS_DMX_MUTEX_UNLOCK();
			return CAS_FAIL;
		}

		free_channel = j;
	}
	
	{	
		//CAS_DMX_MUTEX_LOCK();
		/*open new channel for the pid*/
		tmp_channel = channel_array[free_channel];
		tmp_channel->req_id[0] = req_id;
		
		cas_demux_set_mv(&(tmp_channel->mask_value), 0, value, mask, mask_len);

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
			CAS_DMXERR_PRINTF("Err: %s-- no hardware channel\n", __FUNCTION__);
			return CAS_FAIL;
		}
		osal_task_dispatch_off();
		tmp_channel->status[0].enable = TRUE;
		tmp_channel->status[0].tick = osal_get_tick();
		tmp_channel->status[0].timeout = tmp_time_out;
		osal_task_dispatch_on();
		CAS_DMX_MUTEX_UNLOCK();
		CAS_DMX_PRINTF("%s: filter %d\n", __FUNCTION__, tmp_channel->flt_id);
		return CAS_SUCCESS;
	}
	#endif
}


/*********************Internal functions ending********************************/



/**************************************************************************
 							****DVT API Functions****
 **************************************************************************/


INT32 gyca_tablestart(BYTE byReqID, WORD wPID, const BYTE * szFilter, const BYTE * szMask, BYTE byLen, int nWaitSeconds)
{

	UINT8 i;
	UINT8 value[MAX_SEC_MASK_LEN];
	UINT8 mask[MAX_SEC_MASK_LEN];

	CAS_DMX_PRINTF("SetFilter: PID-%04x, req_id-%02x, len-%d,timeout-%ds\n", wPID, byReqID, byLen, nWaitSeconds);	
	//CAS_DMX_PRINTF("Value: ");
	//CAS_DMX_DUMP(szFilter, 8);
	//CAS_DMX_PRINTF("Mask: ");
	//CAS_DMX_DUMP(szMask, 8);

	if(wPID == 0)
		return CAS_SUCCESS;
	
#if 0

	if (FALSE == dvt_dmx_stop_flag)
	{
		while(FALSE == dvt_dmx_stop_flag)
		{
			osal_task_sleep(1000);
		}
	}
#endif

	MEMSET(value, 0, sizeof(value));
	MEMSET(mask, 0, sizeof(mask));
#if 1	
	value[0] = szFilter[0];
	mask[0] = szMask[0];
	if((byLen>=2)&&(byLen<=16))
	{
		for(i=1; i<byLen; i++)
		{
			value[i+2] = szFilter[i];
			mask[i+2] = szMask[i];
	//		 GYCAS_DMX_PRINTF("pbFilterMask[%d] =%x* \n",i,pbFilterMask[i]);
		}
	}


	if(byLen>16)
		CAS_DMX_PRINTF("\n\n\n*****************************error****************************\n\n\n");

	//if(value[0] == 0x81)
		//value[0] =  0x80;

	if(mask[0] == 0xF0)
	{
		mask[0] =  0x00;
		value[0] = 0x00;
	}
#endif	
#if 0	
	for(i=0; i<byLen; i++)
	{
		value[i] = szFilter[i] & szMask[i];
		mask[i] = szMask[i];
	}
#endif

	return cas_demux_request(byReqID, value, mask, byLen, wPID, nWaitSeconds);
}

INT32 GYCA_TableStart
(
	UINT8 bRequestID,
  	const UINT8* pbFilterMatch,
  	const UINT8* pbFilterMask,
  	UINT8 bLen,
  	UINT16 wPid,
  	UINT8 bWaitTime
)
{
	return(gyca_tablestart(bRequestID,wPid,pbFilterMatch,pbFilterMask,bLen,bWaitTime));
}

HRESULT DVTSTBCA_SetStreamGuardFilter(BYTE byReqID, WORD wPID, const BYTE * szFilter, const BYTE * szMask, BYTE byLen, int nWaitSeconds)
{
	UINT8 i;
	UINT8 value[MAX_SEC_MASK_LEN];
	UINT8 mask[MAX_SEC_MASK_LEN];

	CAS_DMX_PRINTF("SetFilter: PID-%04x, req_id-%02x, len-%d,timeout-%ds\n", wPID, byReqID, byLen, nWaitSeconds);	
	//CAS_DMX_PRINTF("Value: ");
	//CAS_DMX_DUMP(szFilter, 8);
	//CAS_DMX_PRINTF("Mask: ");
	//CAS_DMX_DUMP(szMask, 8);

	if (FALSE == dvt_dmx_stop_flag)
	{
		while(FALSE == dvt_dmx_stop_flag)
		{
			osal_task_sleep(1000);
		}
	}
	MEMSET(value, 0, sizeof(value));
	MEMSET(mask, 0, sizeof(mask));
	
	for(i=0; i<byLen; i++)
	{
		value[i] = szFilter[i] & szMask[i];
		mask[i] = szMask[i];
	}

	return cas_demux_request(byReqID, value, mask, byLen, wPID, nWaitSeconds);
}

extern struct ecm_pid cas_ecm_pid;

HRESULT gyca_setcw(WORD wEcmPID, BYTE byKeyLen, const BYTE * szOddKey, const BYTE * szEvenKey, bool bTapingControl)
{
	UINT8 i;
	UINT16 ret;
	UINT8 rating;
	struct dmx_device *dmx_dev;	
	UINT8 even[8],odd[8];


	CAS_DMX_PRINTF("%s:EcmPID:%04x, KeyLen:%02x, Tape:%02x, szOddKey(adr):%08x, szEvenKey(adr):%08x\n", __FUNCTION__, wEcmPID, byKeyLen, bTapingControl,szOddKey,szEvenKey);
	dmx_dev = (struct dmx_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DMX);
	if(dmx_dev == NULL)
	{
		CAS_DMX_PRINTF("TFSTBCA_ScrSetCW: Dev_get_by_type error!\n");
		return CAS_FAIL;
	}

	if (sys_ic_get_chip_id() < ALI_S3602F)
	{
		if(RET_SUCCESS!=dmx_io_control(dmx_dev, DMX_IS_TS_ENTER_CSA, 0))
			dmx_io_control(dmx_dev, DMX_BYPASS_CSA, 0);
	}

	if((szOddKey==NULL)||(szEvenKey==NULL))
	{
		CAS_DMX_PRINTF("GYCAS_SetControlWord: NULL CW pointer\n");	
		//szOddKey = (GYU8*)MALLOC(8);
		//szEvenKey =(GYU8*)MALLOC(8);
		return;
	}


	if(RET_SUCCESS!=dmx_io_control(dmx_dev, DMX_IS_TS_ENTER_CSA, 0))
		dmx_io_control(dmx_dev, DMX_BYPASS_CSA, 0);
	
	/*CW_DMXERR_PRINTF("Odd : ");
	CW_DMXERR_DUMP(szOddKey, bKeyLen);
	CW_DMXERR_PRINTF("Even: ");
	CW_DMXERR_DUMP(szEvenKey, bKeyLen);*/

	CAS_DMX_PRINTF("Odd : ");
	CAS_DMX_DUMP(szOddKey, byKeyLen);
	CAS_DMX_PRINTF("Even: ");
	CAS_DMX_DUMP(szEvenKey, byKeyLen);

	MEMCPY(even,szEvenKey,8);
	MEMCPY(odd,szOddKey,8);		

	even[3] = even[0] +even[1] +even[2];
	even[7] = even[4] +even[5] +even[6];

	odd[3] = odd[0] +odd[1] +odd[2];
	odd[7] = odd[4] +odd[5] +odd[6];	
	
	dmx_cfg_cw(dmx_dev, DES_VIDEO, 2, (UINT32 *)even);
	dmx_cfg_cw(dmx_dev, DES_VIDEO, 3, (UINT32 *)odd);
	dmx_cfg_cw(dmx_dev, DES_AUDIO, 2, (UINT32 *)even);
	dmx_cfg_cw(dmx_dev, DES_AUDIO, 3, (UINT32 *)odd);
	dmx_cfg_cw(dmx_dev, DES_PCR, 2, (UINT32 *)even);
	dmx_cfg_cw(dmx_dev, DES_PCR, 3, (UINT32 *)odd);	
	
#if 0
	CAS_DMX_PRINTF("Odd : ");
	CAS_DMX_DUMP(szOddKey, byKeyLen);
	CAS_DMX_PRINTF("Even: ");
	CAS_DMX_DUMP(szEvenKey, byKeyLen);

	if(cas_ecm_pid.uniform_ecm_pid >=0)
	{
		for(i=0; i<2;i++)
		{
			if(cas_ecm_pid.uniform_ecm_pid == wEcmPID)
			{
				if(szEvenKey != NULL)
					dmx_cfg_cw(dmx_dev, DES_VIDEO, 2, (UINT32 *)szEvenKey);
				if(szOddKey != NULL)
					dmx_cfg_cw(dmx_dev, DES_VIDEO, 3, (UINT32 *)szOddKey);
				if(szEvenKey != NULL)
					dmx_cfg_cw(dmx_dev, DES_AUDIO, 2, (UINT32 *)szEvenKey);
				if(szOddKey != NULL)
					dmx_cfg_cw(dmx_dev, DES_AUDIO, 3, (UINT32 *)szOddKey);
			}
		}
	}
#endif	
	return CAS_SUCCESS;
}

void GYCA_SetCW 
(  
	UINT16 wEcmPid,
 	const UINT8* szOddKey,
	const UINT8* szEvenKey,
	UINT8 bKeyLen,
	UINT8 bReservedFlag
)
{
	gyca_setcw(wEcmPid, bKeyLen, szOddKey, szEvenKey, (bool)bReservedFlag);
}

HRESULT DVTSTBCA_SetDescrCW(WORD wEcmPID, BYTE byKeyLen, const BYTE * szOddKey, const BYTE * szEvenKey, bool bTapingControl)
{
	UINT8 i;
	UINT16 ret;
	UINT8 rating;
	struct dmx_device *dmx_dev;	

	CAS_DMX_PRINTF("%s:EcmPID:%04x, KeyLen:%02x, Tape:%02x\n", __FUNCTION__, wEcmPID, byKeyLen, bTapingControl);
	dmx_dev = (struct dmx_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DMX);
	if(dmx_dev == NULL)
	{
		CAS_DMXERR_PRINTF("TFSTBCA_ScrSetCW: Dev_get_by_type error!\n");
		return CAS_FAIL;
	}

	if (sys_ic_get_chip_id() < ALI_S3602F)
	{
		if(RET_SUCCESS!=dmx_io_control(dmx_dev, DMX_IS_TS_ENTER_CSA, 0))
			dmx_io_control(dmx_dev, DMX_BYPASS_CSA, 0);
	}

	CAS_DMX_PRINTF("Odd : ");
	CAS_DMX_DUMP(szOddKey, byKeyLen);
	CAS_DMX_PRINTF("Even: ");
	CAS_DMX_DUMP(szEvenKey, byKeyLen);

	for(i=0; i<2;i++)
	{
		if(cas_ecm_pid.uniform_ecm_pid == wEcmPID)
		{
			if(szEvenKey != NULL)
				dmx_cfg_cw(dmx_dev, DES_VIDEO, 2, (UINT32 *)szEvenKey);
			if(szOddKey != NULL)
				dmx_cfg_cw(dmx_dev, DES_VIDEO, 3, (UINT32 *)szOddKey);
			if(szEvenKey != NULL)
				dmx_cfg_cw(dmx_dev, DES_AUDIO, 2, (UINT32 *)szEvenKey);
			if(szOddKey != NULL)
				dmx_cfg_cw(dmx_dev, DES_AUDIO, 3, (UINT32 *)szOddKey);
		}
	}
	
	return CAS_SUCCESS;
}



/**************************************************************************
 * API to AP, stop the dmx
 **************************************************************************/
 //flag: FALSE-stop and free channels occupied by cas lib
 //falg: TRUE - start ECM/EMM request
UINT8 dvt_cas_stop_dmx(UINT8 flag)
{
	dvt_dmx_stop_flag = flag;

	if (FALSE == flag)
	{
		osal_task_sleep(100);
		cas_demux_free(DVT_ECM_REQUEST_ID);
		cas_demux_free(DVT_EMM_REQUEST_ID);
	}
}

#ifdef DVTCA_VERSION_NEW
#define DVTCA_AREA_LOCK_SUPPORT
static UINT32 PDSD = 0;

void dvt_set_pdsd(UINT32 value)
{
	PDSD = value;
}

HRESULT	DVTSTBCA_GetNitValue(DWORD * pdwData)
{
#ifdef DVTCA_AREA_LOCK_SUPPORT
	*pdwData = PDSD;
	return CAS_SUCCESS;
#else
	*pdwData = 0;
	return -1;
#endif
}
#endif

