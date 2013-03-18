/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2008 Copyright (C)
 *
 *  File: dvt_sc.c
 *
 *  Description: This file implement the main thread for DVT CA library.
 *
 *  History:
 *           Date            Author         Version       Comment
 *      ========    ======      ======     =========
 *  1.  2008.02.01      Grady Xu       0.1.000       Create this file
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
#include <hld/dmx/dmx.h>
#include <api/libdb/db_config.h>
#include <api/libdb/db_node_c.h>
#include <api/libchunk/chunk.h>


#include <api/libcas/dvt/DVTCAS_STBDataPublic.h>
#include <api/libcas/dvt/DVTSTB_CASInf.h>
#include <api/libcas/dvt/cas_dvt.h>
#include <api/libcas/dvt/DVTCAS_STBInf.h>
#include "dvt_dmx.h"
#include "dvt_catask.h"
#include "dvt_nvm.h"

#include <api/libcas/gy/gyca_interface.h>
#include "../gy/cas_gy.h"
#include <api/libcas/gy/gyca_def.h>



/**************************************************************************
 							****Internal Functions****
 **************************************************************************/
static UINT8 cas_cat_buffer[CAS_CAT_SEC_LEN];
mcas_t cas_thread_info;
static UINT8 cas_cat_channel_id = CAS_CHANNEL_INVALID;
static struct dmx_device *cas_dmx_dev = NULL;
static OSAL_ID cas_filter_sema = OSAL_INVALID_ID;
struct ecm_pid cas_ecm_pid = {0,-1,0,0};
extern struct cas_channel *channel_array[CAS_MAX_CHAN_NUM];

gycas_info_priv_t gyca_priv;
static UINT16 cas_emmpid = INVALID_PID;

gycas_info_priv_t* gycas_get_priv(void)
{
	
	return  &gyca_priv;
}

void cas_free_filter(UINT8 *filter_id)
{
	if(cas_filter_sema == OSAL_INVALID_ID)
	{
		if ((cas_filter_sema = osal_semaphore_create(1))==OSAL_INVALID_ID)
		{
			CAS_CATSKERR_PRINTF("create semaphore failure with %d\r", cas_filter_sema); \
		}
	}

	while(osal_semaphore_capture(cas_filter_sema, 1000) != SUCCESS)
			CAS_CATSKERR_PRINTF("%s:cas_sema capture timeout\n", __FUNCTION__);
	if(*filter_id != CAS_CHANNEL_INVALID)
	{
		CAS_CATSK_PRINTF("cas_free_filter: close filter %d\n", *filter_id);
		dmx_io_control(cas_dmx_dev, IO_ASYNC_CLOSE, *filter_id);
		*filter_id = CAS_CHANNEL_INVALID;		
	}
	osal_semaphore_release(cas_filter_sema);
}


void cas_send_msg(UINT8 type, UINT32 param)
{
	struct cas_msg msg;
	msg.type = type;
	msg.param = param;
	osal_msgqueue_send(cas_thread_info.msgque_id, &msg, sizeof(struct cas_msg), OSAL_WAIT_FOREVER_TIME);
}

struct restrict cas_pmt_restrict;
struct get_section_param cas_pmt_param;
struct restrict cas_cat_restrict;
struct get_section_param cas_cat_param;


INT32 cas_parse_pmt(UINT8 *section, INT32 length)
{
	UINT16 index, prog_info_len,section_len,es_info_len;
	UINT16 left_len;
	UINT16 prg_number;
	UINT16 ca_system_id = 0;
	UINT16 ecm_pid = 0;
	UINT8 *discriptor_pointer = NULL;
	UINT8 *discriptor_pointer2 = NULL;
	UINT16 loop_length;
	UINT16 ca_discriptor_num;
	UINT8 stream_type;
	SDVTCAServiceInfo dvtcas_info;
	//cas_ecm_pid.uniform_ecm_pid = INVALID_PID;

	section_len = ((section[1] & 0x0F) << 8)|section[2];
	prog_info_len = ((section[10] & 0x0F) << 8)|section[11];
	left_len = section_len -9 -prog_info_len -4;
	
	discriptor_pointer = &section[12];
	discriptor_pointer2 = discriptor_pointer + prog_info_len;

	prg_number = (section[3]<<8) | section[4];

	cas_ecm_pid.service_id = prg_number;
	//dvtcas_info.m_wServiceID = prg_number;
	
	//first loop, prog_info_length
	while (prog_info_len > 0)
	{
		/*the CA descriptor tag*/
		if (*discriptor_pointer == 0x09)
		{
			ca_system_id = (discriptor_pointer[2] <<8) | discriptor_pointer[3];	
			ecm_pid = ((discriptor_pointer[4]&0x1F) << 8) | discriptor_pointer[5];
			//if(DVTCASTB_CASIDVerify(ca_system_id))
			if(GYCA_IsMatchCAID(ca_system_id))
			{
				cas_ecm_pid.uniform_ecm_pid = ecm_pid;
				if((stream_type == 0x01)||(stream_type == 0x02)||(stream_type == 27))	//Video
					{
						cas_ecm_pid.video_ecm_pid = ecm_pid;
					}
					else if((stream_type == 0x03)||(stream_type == 0x04))	//Audio
					{
						cas_ecm_pid.audio_ecm_pid = ecm_pid;
					}
					if(cas_ecm_pid.video_ecm_pid == cas_ecm_pid.audio_ecm_pid)
					{
						cas_ecm_pid.uniform_ecm_pid = ecm_pid;
						cas_ecm_pid.video_ecm_pid = INVALID_PID;
						cas_ecm_pid.audio_ecm_pid = INVALID_PID;
					}
				//dvtcas_info.m_wEcmPid = ecm_pid;
				//cas_ecm_pid.uniform_ecm_pid = ecm_pid;
				//DVTCASTB_SetCurEcmInfo(&dvtcas_info);
				//CAS_CATSK_PRINTF("%s: ecm pid=0x%0x\n",__FUNCTION__, dvtcas_info.m_wEcmPid);

					CAS_CATSK_PRINTF("####CHCHG %s():line%d cas_parse_pmt, ecm_pid = %x, v_ecmpid = %x, a_ecmpid = %x\n", __FUNCTION__,__LINE__,
									  cas_ecm_pid.uniform_ecm_pid,cas_ecm_pid.video_ecm_pid,cas_ecm_pid.audio_ecm_pid);

				return;
			}			
		}
		prog_info_len -= 2+discriptor_pointer[1];
		discriptor_pointer += 2+discriptor_pointer[1];
	}
	discriptor_pointer = discriptor_pointer2;

	//second loop, N1, es_info_length
	while(left_len > 0)
	{
		stream_type = discriptor_pointer[0];
		es_info_len = ((discriptor_pointer[3]&0x03)<<8)  | discriptor_pointer[4];
		loop_length = es_info_len;
		discriptor_pointer += 5;
		
		while(loop_length > 0)
		{
			/*the CA descriptor tag*/
			if (*discriptor_pointer == 0x09)
			{
				ca_system_id = (discriptor_pointer[2] <<8) | discriptor_pointer[3];	
				ecm_pid = ((discriptor_pointer[4]&0x1F) << 8) | discriptor_pointer[5];
				//if(DVTCASTB_CASIDVerify(ca_system_id))
				if(GYCA_IsMatchCAID(ca_system_id))
				{
					//dvtcas_info.m_wEcmPid = ecm_pid;
					cas_ecm_pid.uniform_ecm_pid = ecm_pid;
					if((stream_type == 0x01)||(stream_type == 0x02)||(stream_type == 27))	//Video
					{
						cas_ecm_pid.video_ecm_pid = ecm_pid;
					}
					else if((stream_type == 0x03)||(stream_type == 0x04))	//Audio
					{
						cas_ecm_pid.audio_ecm_pid = ecm_pid;
					}
					if(cas_ecm_pid.video_ecm_pid == cas_ecm_pid.audio_ecm_pid)
					{
						cas_ecm_pid.uniform_ecm_pid = ecm_pid;
						cas_ecm_pid.video_ecm_pid = INVALID_PID;
						cas_ecm_pid.audio_ecm_pid = INVALID_PID;
					}
					//DVTCASTB_SetCurEcmInfo(&dvtcas_info);
					//CAS_CATSK_PRINTF("%s: ecm pid=0x%0x\n",__FUNCTION__, dvtcas_info.m_wEcmPid);
					CAS_CATSK_PRINTF("####CHCHG %s():line%d cas_parse_pmt, ecm_pid = %x, v_ecmpid = %x, a_ecmpid = %x\n", __FUNCTION__,__LINE__,
									  cas_ecm_pid.uniform_ecm_pid,cas_ecm_pid.video_ecm_pid,cas_ecm_pid.audio_ecm_pid);
					return;
				}			
			}
			loop_length -= 2+discriptor_pointer[1];
			discriptor_pointer += 2+discriptor_pointer[1];
		}

		left_len -= 5+es_info_len;

	}

	//if no valid ECM pid is found, set it to 0
	//dvtcas_info.m_wEcmPid = 0;
	cas_ecm_pid.uniform_ecm_pid = INVALID_PID;
	cas_ecm_pid.video_ecm_pid = INVALID_PID;
	cas_ecm_pid.audio_ecm_pid = INVALID_PID;
	CAS_CATSK_PRINTF("####CHCHG %s():line%d cas_parse_pmt, ecm_pid = %x, v_ecmpid = %x, a_ecmpid = %x\n", __FUNCTION__,__LINE__,
					  cas_ecm_pid.uniform_ecm_pid,cas_ecm_pid.video_ecm_pid,cas_ecm_pid.audio_ecm_pid);
	//DVTCASTB_SetCurEcmInfo(&dvtcas_info);
	//CAS_CATSK_PRINTF("%s: ecm pid=0x%0x\n",__FUNCTION__, dvtcas_info.m_wEcmPid);	
	return;
}

UINT32 cas_pmt_crc = 0;
INT32 cas_pmt_callback(UINT8 *section, INT32 length, UINT16 param)
{
	UINT16 pmt_pid;
	INT32 sec_len;
	UINT32 crc = 0;

	sec_len = ((section[1]&0x0f)<<8)|section[2];
	crc = section[sec_len-1]|section[sec_len]<<8|section[sec_len+1]<<16|section[sec_len+2]<<24;

	if(cas_pmt_crc == crc)	// same PMT
		return SUCCESS;
	cas_pmt_crc = crc;
	cas_parse_pmt(section, length);
	cas_send_msg(CAS_MSG_SEC_PMT, 0);
	return RET_SUCCESS;	
}

BOOL cas_parse_cat(UINT8 *section, INT32 length)
{
	UINT16 index;
	UINT16 cat_length;
	UINT16 ca_system_id;
	UINT16 emm_pid = 0;
	cat_length = ((UINT16)(section[1]&0x0F)<<8) | section[2];
	index = 8;
	cat_length -= 9;	/*only length of ca_discriptor*/
	while(cat_length > 0)
	{
		if (section[index] == 0x09)
		{
			ca_system_id = (section[index+2] << 8) | section[index+3];
			//if(DVTCASTB_CASIDVerify(ca_system_id))
			if(GYCA_IsMatchCAID(ca_system_id))
			{	
				emm_pid = ((section[index+4]&0x1F)<<8) | section[index+5];
				if(emm_pid !=0)
				{
					cas_emmpid = emm_pid;
					//GYCA_SetEmmPID(CA_INFOR_CLEAR,0);
					//GYCA_SetEmmPID(CA_INFOR_ADD,emm_pid);
					libc_printf("####CHCHG %s():line%d cas_parse_cat, emm_pid = %x\n", __FUNCTION__,__LINE__,emm_pid);
					return TRUE;
				}
				//DVTCASTB_SetEmmPid(emm_pid);
			}
		}
		cat_length -= 2 + section[index+1];
		index += 2 + section[index+1];
	}
	cas_emmpid = INVALID_PID;
	return FALSE;
}

void cas_cat_sec_cb(struct get_section_param *cas_sec_param)
{
	cas_sec_param->continue_get_sec = 0;
	if (TRUE == cas_parse_cat(cas_sec_param->buff, cas_sec_param->buff_len))
		cas_send_msg(CAS_MSG_SEC_CAT, 0);
	return;
}

INT32 cas_cat_start(/*UINT16 pmt_pid, UINT16 service_id*/)
{
	RET_CODE ret;
	/*request channel for CAT*/
	MEMSET(cas_cat_buffer, 0, sizeof(cas_cat_buffer));
	MEMSET(&cas_cat_restrict, 0, sizeof(struct restrict));

	cas_cat_restrict.mask_len = 1; //6;
	cas_cat_restrict.multi_mask[0][0] = 0xFF;		/* Table ID */
	cas_cat_restrict.tb_flt_msk = 1;
	cas_cat_restrict.value[0][0] = 0x01;	/* Table ID */
	MEMSET(&cas_cat_param, 0, sizeof(struct get_section_param));
	cas_cat_param.pid = 0x01;	/*cat pid*/
	cas_cat_param.buff = cas_cat_buffer;
	cas_cat_param.buff_len = sizeof(cas_cat_buffer);
	cas_cat_param.crc_flag = 1;
	cas_cat_param.wai_flg_dly = 20000;
	cas_cat_param.mask_value = &cas_cat_restrict;
	cas_cat_param.get_sec_cb = cas_cat_sec_cb;
	cas_cat_param.continue_get_sec = 1;
	
	if(dmx_async_req_section(cas_dmx_dev, &cas_cat_param, &cas_cat_channel_id) != RET_SUCCESS)
	{
		CAS_CATSKERR_PRINTF("lib_cas_start alloc cat_filter error\n");
		return !RET_SUCCESS;
	}
	CAS_CATSK_PRINTF("%s: get CAT section on channel %d\n",__FUNCTION__, cas_cat_channel_id);

	return RET_SUCCESS;
}

INT32 cas_cat_stop()
{
	CAS_CATSK_PRINTF("%s\n", __FUNCTION__);
	//cas_free_filter(&cas_cat_channel_id);
	if(cas_cat_channel_id != CAS_CHANNEL_INVALID)
		cas_free_filter(&cas_cat_channel_id);
	cas_cat_channel_id = CAS_CHANNEL_INVALID;

	return RET_SUCCESS;
}

static BOOL dvt_sc_status = FALSE;//true:sc inserted,false:sc out.

BOOL dvt_get_sc_status()
{
    return dvt_sc_status;
}

static void cas_smc_notify(UINT8 inserted)
{
	if(inserted == 0)
	{
		cas_send_msg(CAS_MSG_SMC_REMOVE, 0);
	}
	else if(inserted == 1)
	{
		cas_send_msg(CAS_MSG_SMC_INSERT, 0);
	}
	return;
}

static void cas_monitor_thread(UINT32 param1, UINT32 param2)
{
	UINT8 i;
	struct cas_msg msg;
	UINT32 msg_len;
	struct smc_device *smc_dev = NULL;
	struct dmx_device *dmx_dev = NULL;
	SDVTCAServiceInfo cas_info;
	gycas_info_priv_t* priv ;
	T_CAServiceInfo service_info;



	dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);
	smc_dev = (struct smc_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_SMC);
	if(smc_dev == NULL)
	{		
		CAS_CATSKERR_PRINTF("Err: %s-No smc!\n",__FUNCTION__);
		ASSERT(0);
	}
	if(RET_SUCCESS != smc_open(smc_dev, cas_smc_notify))
	{
		CAS_CATSKERR_PRINTF("Err: %s-smc_open failed!\n",__FUNCTION__);
		ASSERT(0);
	}

	if(cas_filter_sema == OSAL_INVALID_ID)
	{
		if ((cas_filter_sema = osal_semaphore_create(1))==OSAL_INVALID_ID)
		{
			CAS_CATSKERR_PRINTF("Err: create semaphore failure with %d\r", cas_filter_sema); \
		}
	}
			
	cas_dmx_dev = (struct dmx_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DMX);
	if(cas_dmx_dev == NULL)
	{
		CAS_CATSKERR_PRINTF("Err: %s-get dmx device failed!\n",__FUNCTION__);
		ASSERT(0);
	}

	si_monitor_register_pmt_cb(cas_pmt_callback);
	//cas_smc_notify(CAS_MSG_SMC_INSERT);
	while (1)
	{
		if (osal_msgqueue_receive(&msg, &msg_len, cas_thread_info.msgque_id, 100) != RET_SUCCESS)
		{
			cas_dmx_timeout_check();
			cas_flash_status_check();
			continue;
		}
		
		switch (msg.type)
		{
			case CAS_MSG_SEC_PMT:
				CAS_CATSKERR_PRINTF("PMT GOT:servid=0x%x, ecmpid= 0x%x!\n", cas_ecm_pid.service_id, cas_ecm_pid.uniform_ecm_pid);
				//cas_info.m_wServiceID = cas_ecm_pid.service_id;
				//cas_info.m_wEcmPid = cas_ecm_pid.uniform_ecm_pid;
				////osal_mutex_lock(cas_thread_info.mutex_id, OSAL_WAIT_FOREVER_TIME);
				//DVTCASTB_SetCurEcmInfo(&cas_info);
				////osal_mutex_unlock(cas_thread_info.mutex_id);
				service_info.m_wOriNetworkID=0x1fff;
				service_info.m_wTSID=0x1fff;
				service_info.m_byServiceCount = 1;
				service_info.m_wServiceID[0] = cas_ecm_pid.service_id;
				//osal_task_sleep(1000);
				//gycas_info_priv_t* priv ;
				priv = gycas_get_priv();
				osal_mutex_lock(priv->mutex,OSAL_WAIT_FOREVER_TIME);
				GYCA_SetEcmPID (CA_INFOR_CLEAR, (T_CAServiceInfo *)NULL);
				
				if(cas_ecm_pid.uniform_ecm_pid != INVALID_PID)
				{
					service_info.m_wEcmPid = (UINT16)cas_ecm_pid.uniform_ecm_pid;
					CAS_CATSKERR_PRINTF("uniform_ecm_pid[%04x]\n",cas_ecm_pid.uniform_ecm_pid);
					GYCA_SetEcmPID (CA_INFOR_ADD, &service_info);				
				}
				else 
				{

					if(cas_ecm_pid.video_ecm_pid != INVALID_PID)
						service_info.m_wEcmPid = cas_ecm_pid.audio_ecm_pid;
					else if(cas_ecm_pid.audio_ecm_pid != INVALID_PID)
						service_info.m_wEcmPid = cas_ecm_pid.audio_ecm_pid;
					else
						service_info.m_wEcmPid = 0x1fff;
					CAS_CATSKERR_PRINTF("gycas_info.m_wEcmPid[%04x]\n",cas_ecm_pid.m_wEcmPid);					
					GYCA_SetEcmPID (CA_INFOR_ADD, &service_info);
				}
				osal_mutex_unlock(priv->mutex);
				//GYCA_SetEcmPID(CA_INFOR_ADD ,&service_info);
				break;
			case CAS_MSG_SEC_CAT:
				cas_free_filter(&cas_cat_channel_id);
				if(cas_emmpid == INVALID_PID)
					break;
				//gycas_info_priv_t* priv ;
				priv = gycas_get_priv();
				CAS_CATSKERR_PRINTF("gycas_info.m_wEmmPid[%04x] 1\n",cas_emmpid);
				osal_mutex_lock(priv->mutex,OSAL_WAIT_FOREVER_TIME);
				GYCA_SetEmmPID(CA_INFOR_CLEAR,0);
				CAS_CATSKERR_PRINTF("gycas_info.m_wEmmPid[%04x] 2\n",cas_emmpid);
				GYCA_SetEmmPID(CA_INFOR_ADD,cas_emmpid);
				osal_mutex_unlock(priv->mutex);
				break;				
			case CAS_MSG_SMC_INSERT:
				CAS_CATSKERR_PRINTF("%s:smc insert!\n", __FUNCTION__);
					libc_printf("==================================== CAS_MSG_SMC_INSERT 1 \n");

				//M3701E use IO_SET_DEC_STATUS instead of bypass_CSA
				if (sys_ic_get_chip_id() >= ALI_S3602F)
					dmx_io_control(dmx_dev, IO_SET_DEC_STATUS, (UINT32)1);
				//if (CAS_SUCCESS == DVTSTBCA_SCReset())
				{
					libc_printf("==================================== CAS_MSG_SMC_INSERT 2 \n");

					GYCA_SC_DRV_ResetCard(0);
					libc_printf("==================================== CAS_MSG_SMC_INSERT 3 \n");
			
			//gycas_info_priv_t* priv ;
			priv = gycas_get_priv();
			osal_mutex_lock(priv->mutex, OSAL_WAIT_FOREVER_TIME);
			if(GYCA_SC_ValidateATR(priv->atr,priv->atr_size) == GYFALSE)
			{
				libc_printf("==================================== CAS_MSG_SMC_INSERT 4 \n");
				CAS_CATSKERR_PRINTF("[Cas_gy.c] gysmc_callback GYCA_SC_ValidateATR==GYFALSE\n");
				priv->smc_notify(0,GYCA_CARD_NOTVALID,0,0);
				osal_mutex_unlock(priv->mutex);

				break;
			}

			if(priv->smc_notify)
				priv->smc_notify(0,GYCA_CARD_IN,priv->atr,priv->atr_size);
			else
				CAS_CATSKERR_PRINTF("gysmc_callback-->priv->smc_notify==NULL\n");
			
			osal_mutex_unlock(priv->mutex);
			
					cas_thread_info.disp_callback(CAS_MSG_SMC_IN, 0);
					
					libc_printf("==================================== CAS_MSG_SMC_INSERT 5 \n");

					#ifdef EMM_TASK
					osal_mutex_lock(priv->mutex, OSAL_WAIT_FOREVER_TIME);
					priv->emmbuf_read = 0;
					priv->emmbuf_write = 0;
					priv->emmbuf_end = GYCAS_CH_BUF_EMM_SIZE;
					osal_mutex_unlock(priv->mutex);
					#endif
					//DVTCASTB_SCInsert(0);
					cas_pmt_crc = 0;//get PMT now to speed up
					dvt_sc_status = TRUE;
				}
				
				break;
			case CAS_MSG_SMC_REMOVE:
				CAS_CATSKERR_PRINTF("%s:smc removed!\n", __FUNCTION__);
				cas_thread_info.disp_callback(CAS_MSG_SMC_OUT, 0);
				//dmx_io_control(dmx_dev, DMX_BYPASS_CSA, 1);
				//M3701E use IO_SET_DEC_STATUS instead of bypass_CSA
				if (sys_ic_get_chip_id() >= ALI_S3602F)
					dmx_io_control(dmx_dev, IO_SET_DEC_STATUS, (UINT32)0);
				else
					dmx_io_control(dmx_dev, DMX_BYPASS_CSA, 1);

					//gycas_info_priv_t* priv ;
					priv = gycas_get_priv();

			if(priv->smc_notify)
				priv->smc_notify(0,GYCA_CARD_OUT,0,0);
			else
				CAS_CATSKERR_PRINTF("gysmc_callback-->priv->smc_notify==NULL\n");
				
					#ifdef EMM_TASK
					osal_mutex_lock(priv->mutex, OSAL_WAIT_FOREVER_TIME);
					priv->emmbuf_read = 0;
					priv->emmbuf_write = 0;
					priv->emmbuf_end = GYCAS_CH_BUF_EMM_SIZE;
					osal_mutex_unlock(priv->mutex);
					#endif
				//DVTCASTB_SCRemove();
				dvt_sc_status = FALSE;
				break;
			default:
				CAS_CATSKERR_PRINTF("%s:UNKNOWN MSG GOT!\n", __FUNCTION__);
				break;
		}
		cas_dmx_timeout_check();
		cas_flash_status_check();
	}
}

void cas_monitor_init(cas_disp_func_t mcas_disp_func)
{
	OSAL_T_CTSK t_ctsk;	
	OSAL_T_CMBF t_cmbf;
	int i = 0;
	libc_printf("==================================== 1 \n");
	MEMSET(&cas_thread_info, 0, sizeof(mcas_t));
	cas_thread_info.disp_callback = mcas_disp_func;
	cas_thread_info.msgque_id = OSAL_INVALID_ID;
	cas_thread_info.mutex_id = OSAL_INVALID_ID;

//add by gyca
	//gyca_priv = (gycas_info_priv_t)MALLOC(sizeof(gycas_info_priv_t));
	//if (gyca_priv == NULL) {
	//	GYCAS_ERR("[Cas_gy.c] gycas_init: priv malloc fail\n");
	//	return ERR_FAILURE;
	//}
	MEMSET(&gyca_priv, 0, sizeof(gycas_info_priv_t));

	INIT_LIST_HEAD(&gyca_priv.buf_list);
#if 0
	gyca_priv.emm_ch_info.chid = -1;
	gyca_priv.emm_ch_info.desc_id = -1;
	gyca_priv.emm_ch_info.pid= 0x1FFF;
	gyca_priv.emm_ch_info.ch_buf = MALLOC(GYCAS_CH_BUF_EMM_SIZE);
	if (gyca_priv.emm_ch_info.ch_buf == NULL) {
		ASSERT(0);
	}
	MEMSET(gyca_priv.emm_ch_info.ch_buf, 0, GYCAS_CH_BUF_EMM_SIZE);
	for (i=0; i<GYCAS_MAX_ECM_NUM; i++) {
		gyca_priv.ecm_ch_info[i].chid = -1;
		gyca_priv.ecm_ch_info[i].desc_id = -1;
		gyca_priv.ecm_ch_info[i].pid= 0x1FFF;
		gyca_priv.ecm_ch_info[i].ch_buf = MALLOC(GYCAS_CH_BUF_ECM_SIZE);
		if (gyca_priv.ecm_ch_info[i].ch_buf == NULL) {
			ASSERT(0);
		}
		MEMSET(gyca_priv.ecm_ch_info[i].ch_buf, 0, GYCAS_CH_BUF_ECM_SIZE);
	}
	for (i=0; i<GYCAS_FILTER_NUM; i++) 
		gyca_priv.req_param[i].filter_id = -1;
	
	gyca_priv.first_play_flag = 1;
#endif	
	gyca_priv.dmx_dev = (struct dmx_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DMX);

	if (gyca_priv.dmx_dev == NULL) {
		GYCAS_ERR("[Cas_gy.c] gycas_init: priv->dmx_dev fail\n");
		return ;
	}

	gyca_priv.smc_dev = (struct smc_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_SMC);
	if(gyca_priv.smc_dev == NULL)
	{		
		CAS_CATSKERR_PRINTF("Err: %s-No smc!\n",__FUNCTION__);
		ASSERT(0);
	}
#if 0

	#ifdef EMM_TASK
	extern void emmtask(void *param1, void *param2);
	OSAL_T_CTSK gy_emmtsk;	

	gy_emmtsk.task = (OSAL_T_TASK_FUNC_PTR)emmtask;
	gy_emmtsk.itskpri = OSAL_PRI_NORMAL;
	gy_emmtsk.stksz = 0x1000;
	gy_emmtsk.quantum = 10;
	gy_emmtsk.name[0] = 'E';
	gy_emmtsk.name[1] = 'M';
	gy_emmtsk.name[2] = 'M';
	
	gyca_priv.emmtask = osal_task_create(&gy_emmtsk);
	if (OSAL_INVALID_ID == gyca_priv.emmtask)
	{
		CAS_CATSKERR_PRINTF("Err: %s-create task failed!\n",__FUNCTION__);
		ASSERT(0);
	}

	gyca_priv.emmbuf = (UINT8 *)MALLOC(GYCAS_CH_BUF_EMM_SIZE);
	if(gyca_priv.emmbuf == NULL)
		return ;
	else
		MEMSET(gyca_priv.emmbuf,0x00,GYCAS_CH_BUF_EMM_SIZE);
	#endif		
#endif	
	gyca_priv.mutex =  osal_mutex_create();
	if (OSAL_INVALID_ID == gyca_priv.mutex)
	{
		CAS_CATSKERR_PRINTF("Err: %s-create mutex failed!\n",__FUNCTION__);
		ASSERT(0);
	}


	UINT32 chunk_id = CAS_CHUNK_ID;
	UINT32 addr = 0;
	CHUNK_HEADER chuck_hdr;
	struct sto_device *sto_flash_dev = NULL;

	/* get table info base addr by using chunk */
	if(sto_get_chunk_header(chunk_id, &chuck_hdr) == 0)
	{
		CAS_CATSKERR_PRINTF("Err: %s get chunk header fail!\n", __FUNCTION__);
		return CAS_FAIL;
	}
	
	addr = sto_chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
	addr += CHUNK_HEADER_SIZE;
	gyca_priv.sto_offset = addr;
	gyca_priv.sto_len = 64*1024;
	
	
	cas_demux_init();
	
	/* create cas massage buffer,mutex and monitor task */
	t_cmbf.bufsz = 0x800;
	t_cmbf.maxmsz = sizeof(struct cas_msg);
	cas_thread_info.msgque_id = osal_msgqueue_create(&t_cmbf);
	if (OSAL_INVALID_ID == cas_thread_info.msgque_id)
	{
		CAS_CATSKERR_PRINTF("Err: %s-create message buffer failed!\n",__FUNCTION__);
		ASSERT(0);
	}

	cas_thread_info.mutex_id = osal_mutex_create();
	if (OSAL_INVALID_ID == cas_thread_info.mutex_id)
	{
		CAS_CATSKERR_PRINTF("Err: %s-create mutex failed!\n",__FUNCTION__);
		ASSERT(0);
	}

	t_ctsk.task = (OSAL_T_TASK_FUNC_PTR)cas_monitor_thread;
	t_ctsk.itskpri = OSAL_PRI_NORMAL;
	t_ctsk.stksz = 0x1000;
	t_ctsk.quantum = 10;
	t_ctsk.name[0] = 'S';
	t_ctsk.name[1] = 'M';
	t_ctsk.name[2] = 'C';
	cas_thread_info.thread_id = osal_task_create(&t_ctsk);
	if (OSAL_INVALID_ID == cas_thread_info.thread_id)
	{
		CAS_CATSKERR_PRINTF("Err: %s-create task failed!\n",__FUNCTION__);
		ASSERT(0);
	}
	//DVTCASTB_Init();
	INT32 result;
	result = GYCA_Initialize(OSAL_PRI_NORMAL);
	if(result == GYSUCCESS)
	{
		PRINTF("GYCA_Initialize---SUCCESS\n");
	}
	else
	{
		PRINTF("GYCA_Initialize---FAIL\n");
	}
	extern INT32 gycas_show_msg_cb(UINT32 event_type, UINT32 param1, UINT32 param2, UINT32 param3);
	GYCA_RegisterAvaegisAPPEventCallback(gycas_show_msg_cb);
}

/**************************************************************************
 							****Channel change functions****
 **************************************************************************/
 static BOOL start_transponder = FALSE;

#ifdef NEW_DEMO_FRAME
INT32 api_mcas_start_service(UINT16 channel_index, UINT32 sim_id)
#else
INT32 api_mcas_start_service(UINT16 channel_index)
#endif
{
	cas_pmt_crc = 0;
	/*无节目时，如只搜索单tp，播放节目时因为tp没变，不会调用
	api_mcas_start_transponder()，emm request就没有启动，此函数
	在api_mcas_start_transponder()之后调用，必要时启动emm*/
	if(start_transponder == FALSE)
	{
		cas_cat_start();
		start_transponder = TRUE;
	}
}
#ifdef NEW_DEMO_FRAME
INT32 api_mcas_stop_service(UINT32 sim_id)
#else
INT32 api_mcas_stop_service()
#endif
{
#if 0
    SDVTCAServiceInfo cas_info;

	cas_info.m_wServiceID = cas_ecm_pid.service_id;
	cas_info.m_wEcmPid = 0;
	DVTCASTB_SetCurEcmInfo(&cas_info);
#endif
	libc_printf("####CHCHG %s():line%d api_mcas_stop_service\n", __FUNCTION__,__LINE__);
	GYCA_SetEcmPID(CA_INFOR_CLEAR ,(T_CAServiceInfo*)NULL);
	cas_demux_free(DVT_ECM_REQUEST_ID);
    return SUCCESS;
}

INT32 api_mcas_start_transponder()
{
	if(start_transponder == FALSE)
	{
		cas_cat_start();
		start_transponder = TRUE;
	}
    return SUCCESS;
}
INT32 api_mcas_stop_transponder()
{
    cas_demux_free(DVT_EMM_REQUEST_ID);
	cas_cat_stop();
	start_transponder = FALSE;
    return SUCCESS;
}
INT32 api_mcas_switch_audiopid(UINT16 a_pid)
{
    return SUCCESS;
}


/**************************************************************************
 							****DVT API Functions****
 **************************************************************************/
void   DVTSTBCA_ShowPromptMessage(BYTE byMesageNo)
{
	CAS_CATSK_PRINTF("%s: msgno = %d\n", __FUNCTION__, byMesageNo);
		
	if (NULL != cas_thread_info.disp_callback)
		cas_thread_info.disp_callback(CAS_MSG_SHOWPROMPTMSG, byMesageNo);
}
void   DVTSTBCA_HidePromptMessage(void)
{
	CAS_CATSK_PRINTF("%s.\n", __FUNCTION__);
	
	if (NULL != cas_thread_info.disp_callback)
		cas_thread_info.disp_callback(CAS_MSG_HIDEPROMPTMSG, 0);
}
void	DVTSTBCA_ShowFingerPrinting(DWORD dwCardID, WORD wDuration)
{
	CAS_CATSK_PRINTF("%s: cardID=0x%x, duration = %d\n", __FUNCTION__, dwCardID, wDuration);
	
	if (NULL != cas_thread_info.disp_callback)
	{
		cas_thread_info.finger_info[0] = dwCardID;
		cas_thread_info.finger_info[1] = wDuration*1000; //convert to ms
		cas_thread_info.disp_callback(CAS_MSG_FINGER_PRINT, 0);
	}
}

#ifdef DVTCA_VERSION_NEW
void	DVTSTBCA_ShowOSDMsg(BYTE byPriority,const char * szOSD)
{
	UINT32 len = 0;
	
	CAS_CATSK_PRINTF("%s: msg = %s\n", __FUNCTION__, szOSD);
	
	if (NULL != cas_thread_info.disp_callback)
	{
		len = STRLEN(szOSD);
		if (len > DVT_OSDMSG_LEN)
		{
			CAS_CATSKERR_PRINTF("%s: msg overflow!\n", szOSD);
			len = DVT_OSDMSG_LEN;
		}
		MEMSET(cas_thread_info.dvt_osdbuf, 0, sizeof(cas_thread_info.dvt_osdbuf));
		convert_gb2312_to_unicode(szOSD, len, cas_thread_info.dvt_osdbuf, DVT_OSDMSG_LEN);
		cas_thread_info.disp_callback(CAS_MSG_SHOW_OSD_MESSAGE, 0);
	}
	else
		CAS_CATSKERR_PRINTF("Err:%s-callback is NULL");

}
#else
void   DVTSTBCA_ShowOSDMsg(const char * szOSD)
{
	UINT32 len = 0;
	
	CAS_CATSK_PRINTF("%s: msg = %s\n", __FUNCTION__, szOSD);
	
	if (NULL != cas_thread_info.disp_callback)
	{
		len = STRLEN(szOSD);
		if (len > DVT_OSDMSG_LEN)
		{
			CAS_CATSKERR_PRINTF("%s: msg overflow!\n", szOSD);
			len = DVT_OSDMSG_LEN;
		}
		MEMSET(cas_thread_info.dvt_osdbuf, 0, sizeof(cas_thread_info.dvt_osdbuf));
		convert_gb2312_to_unicode(szOSD, len, cas_thread_info.dvt_osdbuf, DVT_OSDMSG_LEN);
		cas_thread_info.disp_callback(CAS_MSG_SHOW_OSD_MESSAGE, 0);
	}
	else
		CAS_CATSKERR_PRINTF("Err:%s-callback is NULL");
}
#endif

void DVTSTBCA_EmailNotify(BYTE byShow)
{
	CAS_CATSK_PRINTF("%s: show = %d\n", __FUNCTION__, byShow);
	
	if (NULL != cas_thread_info.disp_callback)
		cas_thread_info.disp_callback(CAS_MSG_EMAIL_NOTIFY, byShow);
}

void DVTSTBCA_UrgencyBroadcast(WORD wOriNetID, WORD wTSID, WORD wServiceID, WORD wDuration)
{
	CAS_CATSK_PRINTF("%s:  NetID=0x%x, TSID=0x%x, ServiceID=0x%x, Duration=%d s\n", __FUNCTION__, wOriNetID, wTSID, wServiceID,wDuration);

	cas_thread_info.urgent.network_id = wOriNetID;
	cas_thread_info.urgent.ts_id = wTSID;
	cas_thread_info.urgent.service_id = wServiceID;
	cas_thread_info.urgent.duration = wDuration;
	
	if (NULL != cas_thread_info.disp_callback)
		cas_thread_info.disp_callback(CAS_MSG_LOCK_SERVICE, 0);
}
void DVTSTBCA_CancelUrgencyBroadcast( void )
{
	CAS_CATSK_PRINTF("%s!\n", __FUNCTION__);

	if (NULL != cas_thread_info.disp_callback)
		cas_thread_info.disp_callback(CAS_MSG_UNLOCK_SERVICE, 0);
}

void DVTSTBCA_InquireBookIpp(WORD wEcmPid, const SDVTCAIpp * pIpp )
{
	CAS_CATSK_PRINTF("%s: EcmPID=0x%x\n", __FUNCTION__, wEcmPid);

	MEMCPY(&cas_thread_info.ippinfo, pIpp, sizeof(SDVTCAIpp));

	if (NULL != cas_thread_info.disp_callback)
		cas_thread_info.disp_callback(CAS_MSG_START_IPPVDLG, wEcmPid);
}


HRESULT DVTSTBCA_SwitchChannel(WORD wWaitSeconds)
{
	return -1;
}

void DVTSTBCA_AreaLockOk(void)
{
	return;
}


DWORD DVTSTBCA_GetSerialNo( void )
{
	return 0x12345678;
}
DWORD DVTSTBCA_GetSoftVer(void)
{
	return 0x100;
}
HRESULT DVTSTBCA_GetCPUID(BYTE * pbyCPUID)
{
	return CAS_FAIL;
}


/**************************************************************************
					***Functions to interactive with AP****
 **************************************************************************/

//AP call this to get Finger print
UINT32 DVTSTBCA_GetFingerPrint(UINT32 *card_id, UINT32 *duration)
{
	*card_id = cas_thread_info.finger_info[0];
	*duration = cas_thread_info.finger_info[1];
	return TRUE;
}

//AP call this to get OSD msg, in unicode mode
extern UINT8 DVTSTBCA_GetOSDMessage(UINT8* Message)
{
	if (NULL == Message)
	{
		CAS_CATSKERR_PRINTF("!!!!GotOSDMessage: NULL pointer!!\n");
		return FALSE;
	}
	MEMCPY(Message, cas_thread_info.dvt_osdbuf, sizeof(cas_thread_info.dvt_osdbuf));
	return TRUE;
}

UINT16 DVTSTBCA_GetCurECMPID()
{
	return cas_ecm_pid.uniform_ecm_pid;
}

UINT32 DVTSTBCA_GetCurIPP(SDVTCAIpp *ipp)
{
	if (NULL == ipp)
		return FALSE;

	MEMCPY(ipp, &cas_thread_info.ippinfo, sizeof(SDVTCAIpp));
	return TRUE;
}

UINT32 DVTSTBCA_GetUrgentInfo(dvt_urgent_info *info)
{
	if (NULL == info)
		return FALSE;

	MEMCPY(info, &cas_thread_info.urgent, sizeof(dvt_urgent_info));

	return TRUE;
}

