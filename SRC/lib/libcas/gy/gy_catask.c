/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2008 Copyright (C)
 *
 *  File: dvt_sc.c
 *
 *  Description: This file implement the main thread for GY CA library.
 *
 *  History:
 *           Date            Author         Version       Comment
 *      ========    ======      ======     =========
 *  1.  2008.02.01      GEEYA       0.1.000       Create this file
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


#include "gyca_dmx.h"
#include "gy_catask.h"
#include "gyca_nvm.h"
#include <api/libcas/gy/cas_gy.h>
#include <api/libcas/gy/gyca_interface.h>
#include <api/libcas/gy/gyca_def.h>
#include "../../../../prj/app/demo/dvbc/jade_demo_new_geeya/copper_common/system_data.h"
//#include "../../../../prj/app/demo/dvbc/jade_demo_new_geeya/gyca/win_cas_com.h"
//#include "../../../../prj/app/demo/dvbc/jade_demo_new_geeya/control.h"
#include <api/libc/time.h>

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
extern UINT8 g_enter_search_from_null;

gycas_info_priv_t gyca_priv;
static UINT16 cas_emmpid = INVALID_PID;

UINT16 gycas_notice[50];
//T_CAOSDInfo  gycas_osdinfo={2,60,56,"osd test !!! osd test !!! osd test !!! osd test !!! ----"};
T_CAOSDInfo  gycas_osdinfo={2,30,63,"osd test !!! osd test !!! osd test !!! osd test !!! ---- 啊 123"};
T_CAFingerInfo  gycas_fingerinfo={10,16,"1234567890123456"};
cas_ippv_info   gycas_ippvinfo;
static T_gy_chgchl_f g_stChgchl_info;

extern BOOL get_search_signal_menu_status(void);

T_gy_chgchl_f * gy_get_forcechgchl_info(void)
{
	return &g_stChgchl_info;
}

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

BOOL g_curchl_scramble  = 0; //yangdiaozhi //avoid free channle show CA msg;
/*********************************************************
*解决 " 费用到期"提示，在免费节目弹出
*********************************************************/
BOOL gyca_curchl_is_scramble(void)
{
	return g_curchl_scramble;
}

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

	section_len = ((section[1] & 0x0F) << 8)|section[2];
	prog_info_len = ((section[10] & 0x0F) << 8)|section[11];
	left_len = section_len -9 -prog_info_len -4;
	
	discriptor_pointer = &section[12];
	discriptor_pointer2 = discriptor_pointer + prog_info_len;

	prg_number = (section[3]<<8) | section[4];

	cas_ecm_pid.service_id = prg_number;
	
	//first loop, prog_info_length
	while (prog_info_len > 0)
	{
		/*the CA descriptor tag*/
		if (*discriptor_pointer == 0x09)
		{
			ca_system_id = (discriptor_pointer[2] <<8) | discriptor_pointer[3];	
			ecm_pid = ((discriptor_pointer[4]&0x1F) << 8) | discriptor_pointer[5];
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
	cas_ecm_pid.uniform_ecm_pid = INVALID_PID;
	cas_ecm_pid.video_ecm_pid = INVALID_PID;
	cas_ecm_pid.audio_ecm_pid = INVALID_PID;
	CAS_CATSK_PRINTF("####CHCHG %s():line%d cas_parse_pmt, ecm_pid = %x, v_ecmpid = %x, a_ecmpid = %x\n", __FUNCTION__,__LINE__,
					  cas_ecm_pid.uniform_ecm_pid,cas_ecm_pid.video_ecm_pid,cas_ecm_pid.audio_ecm_pid);
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
			if(GYCA_IsMatchCAID(ca_system_id))
			{	
				emm_pid = ((section[index+4]&0x1F)<<8) | section[index+5];
				if(emm_pid !=0)
				{
					cas_emmpid = emm_pid;
					libc_printf("####CHCHG %s():line%d cas_parse_cat, emm_pid = %x\n", __FUNCTION__,__LINE__,emm_pid);
					return TRUE;
				}
			}
		}
		cat_length -= 2 + section[index+1];
		index += 2 + section[index+1];
	}
	cas_emmpid = INVALID_PID;
	return TRUE;
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

static BOOL gy_sc_status = FALSE;//true:sc inserted,false:sc out.

BOOL gy_get_sc_status()
{
    return gy_sc_status;
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

INT32 gyca_area_control(UINT16 network_id)
{
	return GYCA_RegisterNetworkInfo(network_id,0,0);
}
static void cas_monitor_thread(UINT32 param1, UINT32 param2)
{
	UINT8 i;
	struct cas_msg msg;
	UINT32 msg_len;
	struct smc_device *smc_dev = NULL;
	struct dmx_device *dmx_dev = NULL;
//	SGYCAServiceInfo cas_info;
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
	gyca_mail_init();
	gyca_check_new_mail();
	//gyca_mail_test();
	while (1)
	{
		if (osal_msgqueue_receive(&msg, &msg_len, cas_thread_info.msgque_id, 100) != RET_SUCCESS)
		{
			cas_dmx_timeout_check();
			cas_flash_status_check();
			gyca_check_writemail_to_flash();
			continue;
		}
		
		switch (msg.type)
		{
			case CAS_MSG_SEC_PMT:
				CAS_CATSK_PRINTF("PMT GOT:servid=0x%x, ecmpid= 0x%x!\n", cas_ecm_pid.service_id, cas_ecm_pid.uniform_ecm_pid);
				service_info.m_wOriNetworkID=0x1fff;
				service_info.m_wTSID=0x1fff;
				service_info.m_byServiceCount = 1;
				service_info.m_wServiceID[0] = cas_ecm_pid.service_id;
				
				priv = gycas_get_priv();
				priv->service_id = cas_ecm_pid.service_id;
				priv->ecm_pid = cas_ecm_pid.uniform_ecm_pid;
				osal_mutex_lock(priv->mutex,OSAL_WAIT_FOREVER_TIME);
				GYCA_SetEcmPID (CA_INFOR_CLEAR, (T_CAServiceInfo *)NULL);
				
				if(cas_ecm_pid.uniform_ecm_pid != INVALID_PID)
				{
					service_info.m_wEcmPid = (UINT16)cas_ecm_pid.uniform_ecm_pid;
					CAS_CATSK_PRINTF("uniform_ecm_pid[%04x]\n",cas_ecm_pid.uniform_ecm_pid);
					//libc_printf("\n\nGYCA GYCA_SetEcmPID before tick: %d\n\n",osal_get_tick());
					GYCA_SetEcmPID (CA_INFOR_ADD, &service_info);
					g_curchl_scramble = TRUE;
					//libc_printf("\n\nGYCA GYCA_SetEcmPID after tick: %d\n\n",osal_get_tick());
				}
				else
					g_curchl_scramble = FALSE;
				osal_mutex_unlock(priv->mutex);
				break;
			case CAS_MSG_SEC_CAT:
				cas_free_filter(&cas_cat_channel_id);
				//GYCA_SetEmmPID(CA_INFOR_CLEAR,0);
				if(cas_emmpid == INVALID_PID)
					break;
				priv = gycas_get_priv();
				osal_mutex_lock(priv->mutex,OSAL_WAIT_FOREVER_TIME);
				GYCA_SetEmmPID(CA_INFOR_ADD,cas_emmpid);
				osal_mutex_unlock(priv->mutex);
				break;				
			case CAS_MSG_SMC_INSERT:
				CAS_CATSK_PRINTF("%s:smc insert!\n", __FUNCTION__);

				//M3701E use IO_SET_DEC_STATUS instead of bypass_CSA
				if (sys_ic_get_chip_id() >= ALI_S3602F)
					dmx_io_control(dmx_dev, IO_SET_DEC_STATUS, (UINT32)1);
				
				GYCA_SC_DRV_ResetCard(0);
				priv = gycas_get_priv();
				osal_mutex_lock(priv->mutex, OSAL_WAIT_FOREVER_TIME);
				if(GYCA_SC_ValidateATR(priv->atr,priv->atr_size) == GYFALSE)
				{
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
				cas_pmt_crc = 0;//get PMT now to speed up
				gy_sc_status = TRUE;
				
				break;
			case CAS_MSG_SMC_REMOVE:
				CAS_CATSK_PRINTF("%s:smc removed!\n", __FUNCTION__);
				cas_thread_info.disp_callback(CAS_MSG_SMC_OUT, 0);
				//dmx_io_control(dmx_dev, DMX_BYPASS_CSA, 1);
				//M3701E use IO_SET_DEC_STATUS instead of bypass_CSA
				if (sys_ic_get_chip_id() >= ALI_S3602F)
					dmx_io_control(dmx_dev, IO_SET_DEC_STATUS, (UINT32)0);
				else
					dmx_io_control(dmx_dev, DMX_BYPASS_CSA, 1);

					priv = gycas_get_priv();

				if(priv->smc_notify)
					priv->smc_notify(0,GYCA_CARD_OUT,0,0);
				else
					CAS_CATSKERR_PRINTF("gysmc_callback-->priv->smc_notify==NULL\n");
				gy_sc_status = FALSE;
				break;
			default:
				CAS_CATSKERR_PRINTF("%s:UNKNOWN MSG GOT!\n", __FUNCTION__);
				break;
		}
		cas_dmx_timeout_check();
		cas_flash_status_check();
		gyca_check_writemail_to_flash();
	}
}

void cas_monitor_init(cas_disp_func_t mcas_disp_func)
{
	OSAL_T_CTSK t_ctsk;	
	OSAL_T_CMBF t_cmbf;
	int i = 0;
	MEMSET(&cas_thread_info, 0, sizeof(mcas_t));
	MEMSET(&g_stChgchl_info, 0, sizeof(T_gy_chgchl_f));
	cas_thread_info.disp_callback = mcas_disp_func;
	cas_thread_info.msgque_id = OSAL_INVALID_ID;
	cas_thread_info.mutex_id = OSAL_INVALID_ID;
	
	MEMSET(&gyca_priv, 0, sizeof(gycas_info_priv_t));

	INIT_LIST_HEAD(&gyca_priv.buf_list);

	gyca_priv.dmx_dev = (struct dmx_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DMX);

	if (gyca_priv.dmx_dev == NULL) {
		CAS_CATSKERR_PRINTF("[Cas_gy.c] gycas_init: priv->dmx_dev fail\n");
		return ;
	}

	gyca_priv.smc_dev = (struct smc_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_SMC);
	if(gyca_priv.smc_dev == NULL)
	{		
		CAS_CATSKERR_PRINTF("Err: %s-No smc!\n",__FUNCTION__);
		ASSERT(0);
	}

	gyca_priv.mutex =  osal_mutex_create();
	if (OSAL_INVALID_ID == gyca_priv.mutex)
	{
		CAS_CATSKERR_PRINTF("Err: %s-create mutex failed!\n",__FUNCTION__);
		ASSERT(0);
	}
	
	UINT8 str[100];
	UINT8 len;
	MEMSET(gyca_priv.stb_id, '0', sizeof(gyca_priv.stb_id));
	gyca_priv.stb_id_len = 0;
	MEMSET(str, 0, sizeof(str));
#if 1
	api_sn_get(str);

	//LAHead_GetSerialNumber(str);
	len = STRLEN(str);
	if(len>20)
	{
		gyca_priv.stb_id_len = 20;
		MEMCPY(gyca_priv.stb_id, &str[len-20], 20);
	}
	else
	{
		gyca_priv.stb_id_len = len;
		MEMCPY(gyca_priv.stb_id, &str[0], len);
	}
#endif
	#if 0
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
	#endif
	//addr = sto_chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
	//addr += CHUNK_HEADER_SIZE;
	//gyca_priv.sto_offset = addr;
	//gyca_priv.sto_len = 64*1024;
	
	
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
#ifdef GYCA_AREA
	//gyca_area_control(2); 
#endif
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
	gycas_info_priv_t *priv = gycas_get_priv();
	libc_printf("####CHCHG %s():line%d api_mcas_stop_service\n", __FUNCTION__,__LINE__);
	osal_mutex_lock(priv->mutex,OSAL_WAIT_FOREVER_TIME);
	GYCA_SetEcmPID(CA_INFOR_CLEAR ,(T_CAServiceInfo*)NULL);
	osal_mutex_unlock(priv->mutex);
	cas_demux_free(GY_ECM_REQUEST_ID);
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
	gycas_info_priv_t *priv = gycas_get_priv();
	osal_mutex_lock(priv->mutex,OSAL_WAIT_FOREVER_TIME);
	GYCA_SetEmmPID(CA_INFOR_CLEAR,0);
	osal_mutex_unlock(priv->mutex);
    cas_demux_free(GY_EMM_REQUEST_ID);
	cas_cat_stop();
	start_transponder = FALSE;
    return SUCCESS;
}
INT32 api_mcas_switch_audiopid(UINT16 a_pid)
{
    return SUCCESS;
}


UINT16* gycas_get_noticemsg_info(void)
{
	return (UINT16*)gycas_notice;
}

T_CAOSDInfo* gycas_get_osdmsg_info(void)
{
	return (T_CAOSDInfo*)&gycas_osdinfo;
}

T_CAFingerInfo* gycas_get_fingermsg_info(void)
{
	return (T_CAFingerInfo*)&gycas_fingerinfo;
}



cas_ippv_info* gycas_get_ippvmsg_info(void)
{
	return (cas_ippv_info*)&gycas_ippvinfo;
}


void gycas_save_noticmsg_info(UINT8 *gbstr ,UINT16 *uincode_str,UINT8 flag)
{	
     if(flag ==1)
 	{
 		 MEMSET(&gycas_notice,0,sizeof(gycas_notice));
     	convert_gb2312_to_unicode(gbstr,100,&gycas_notice,sizeof(gycas_notice));
 	}
    else
	{
		 MEMCPY(uincode_str, &gycas_notice,sizeof(gycas_notice));
	}
}
void gycas_save_osdmsg_info(T_CAOSDInfo *osdinfo,UINT32 len)
{   
     MEMSET(&gycas_osdinfo,0,sizeof(T_CAOSDInfo));
     MEMCPY(&gycas_osdinfo,osdinfo,sizeof(T_CAOSDInfo));
}

void gycas_save_fingermsg_info(T_CAFingerInfo *fingerinfo,UINT32 len)
{   
     MEMSET(&gycas_fingerinfo,0,sizeof(T_CAFingerInfo));
     MEMCPY(&gycas_fingerinfo,fingerinfo,sizeof(T_CAFingerInfo));
}

UINT8 gycas_save_ippvmsg_info(UINT32 param1,UINT32 len)
{   
	T_CA_IPPVInfo *gy_ippv_info = (T_CA_IPPVInfo*)param1;
	
	cas_ippv_info cas_ippv;
	struct tm send_time;
	gycas_info_priv_t *priv = gycas_get_priv();

	libc_printf("GYCA_IPPV_INFO service id=%x  play sid=%x\n",gy_ippv_info->m_wServiceId,priv->service_id);
	if(gy_ippv_info->m_wServiceId!= priv->service_id)
		return FALSE;
	
	if(gy_ippv_info->price.m_byEventPriceType == 0)
		cas_ippv.ippvtype = CAS_IPPV_PAY;
	else
		cas_ippv.ippvtype = CAS_IPPT_PAY;
	
	cas_ippv.product_id = gy_ippv_info->casID;
	cas_ippv.service_id = gy_ippv_info->m_wServiceId;
	cas_ippv.event_id = gy_ippv_info->m_wEventId;
	cas_ippv.price = gy_ippv_info->price.m_wEventPrice;
	cas_ippv.platform_id = gy_ippv_info->m_wPlatformId;

	cas_ippv.ecm_pid = priv->ecm_pid;

	cas_ippv.startdate.time.hour = gy_ippv_info->validDate.time.m_byHour;
	cas_ippv.startdate.time.minute= gy_ippv_info->validDate.time.m_byMinute;
	cas_ippv.startdate.time.second = gy_ippv_info->validDate.time.m_bySecond;
	mjd_to_tm(gy_ippv_info->validDate.date.m_byDate, &send_time);
	cas_ippv.startdate.date.year = send_time.tm_year+1900;
	cas_ippv.startdate.date.month = send_time.tm_mon+1;
	cas_ippv.startdate.date.day = send_time.tm_mday;

	cas_ippv.expiredate.time.hour = gy_ippv_info->expiredDate.time.m_byHour;
	cas_ippv.expiredate.time.minute= gy_ippv_info->expiredDate.time.m_byMinute;
	cas_ippv.expiredate.time.second = gy_ippv_info->expiredDate.time.m_bySecond;
	mjd_to_tm(gy_ippv_info->expiredDate.date.m_byDate, &send_time);
	cas_ippv.expiredate.date.year = send_time.tm_year+1900;
	cas_ippv.expiredate.date.month = send_time.tm_mon+1;
	cas_ippv.expiredate.date.day = send_time.tm_mday;

	 MEMSET(&gycas_ippvinfo,0,sizeof(cas_ippv_info));
	 MEMCPY(&gycas_ippvinfo,&cas_ippv,sizeof(cas_ippv_info));

	libc_printf("ecm pid =%x \n",cas_ippv.ecm_pid);
	libc_printf("产品号:%d\n类型:%d\n业务ID:%d ecm pid:%x\n价格:%d\n开始时间:%04d-%02d-%02d %02d:%02d:%02d\n结束时间:%04d-%02d-%02d %02d:%02d:%02d\n\n",
		cas_ippv.product_id,cas_ippv.ippvtype,cas_ippv.service_id,cas_ippv.ecm_pid,cas_ippv.price,
		cas_ippv.startdate.date.year,cas_ippv.startdate.date.month,cas_ippv.startdate.date.day,
		cas_ippv.startdate.time.hour,cas_ippv.startdate.time.minute,cas_ippv.startdate.time.second,
		cas_ippv.expiredate.date.year,cas_ippv.expiredate.date.month,cas_ippv.expiredate.date.day,
		cas_ippv.expiredate.time.hour,cas_ippv.expiredate.time.minute,cas_ippv.expiredate.time.second);				
	return TRUE;   
}
GYS32 gycas_show_msg_cb(GYU32 event,GYU32 param1,GYU32 param2,GYU32 param3)
{
    UINT32 param;
	UINT8 msg_str[40];
	UINT8 email_staus;
	libc_printf("\n\nca库消息:event[%d]  param1[%d] param2[%d] param3[%d]\n\n",event,param1,param2,param3);
	switch (event)
	{
			case GYCA_NOTICE_SHOW:		/* 显示节目没有授权*/
				 libc_printf("\n****%s*****len[%d]**********\n",param1,param2);
				 gycas_save_noticmsg_info(param1,NULL,1);
				 if (NULL != cas_thread_info.disp_callback)
					cas_thread_info.disp_callback(CAS_MSG_SHOWPROMPTMSG, 0);
				 	
				  break;
			case GYCA_NOTICE_ERASE:		/* 擦除提示信息*/
				//gycas_notice[0] = '\0';
				if (NULL != cas_thread_info.disp_callback)
					cas_thread_info.disp_callback(CAS_MSG_HIDEPROMPTMSG, 0);
				break;
			case GYCA_OSD_INFO:           /*显示osd信息*/
				{
				T_CAOSDInfo *info = (T_CAOSDInfo *)param1;
				libc_printf("\n****%s*****len[%d]**********\n",info->m_szOSD_Text,info->m_byOSD_Text_length);
				gycas_save_osdmsg_info(info,param2);
				}
				if (NULL != cas_thread_info.disp_callback)
			     	cas_thread_info.disp_callback(CAS_MSG_SHOW_OSD_MESSAGE, 0); 
				break;
			case GYCA_OSD_ERASE:		/* 擦除OSD信息*/
				if(param1 == 0)	
				{
					if (NULL != cas_thread_info.disp_callback)
			     	cas_thread_info.disp_callback(CAS_MSG_HID_TOP_OSD_MESSAGE, 0);  
				}
				else if(param1 == 1) 
				{
					if (NULL != cas_thread_info.disp_callback)
			     	cas_thread_info.disp_callback(CAS_MSG_HID_BTM_OSD_MESSAGE, 0);  
				}
				else if(param1 == 2) 
				{
					if (NULL != cas_thread_info.disp_callback)
			     	cas_thread_info.disp_callback(CAS_MSG_HID_MID_OSD_MESSAGE, 0);  
				}  
			      break;
			case GYCA_RESTART_CHAN:		/* 请求应用程序重新播放当前节目*/
				if (NULL != cas_thread_info.disp_callback)
			     	cas_thread_info.disp_callback(CAS_RESTART_CHAN, 0);  
                 break;
			case GYCA_MAIL_INFO:			/* 有EMM,PEMM Mail信息通知*/
				{
			  	T_CAMailInfo *gy_mail_info = (T_CAMailInfo*)param1;
				gyca_addmail_one_to_ram(gy_mail_info);
				gyca_check_new_mail();
				if (NULL != cas_thread_info.disp_callback)
			     	cas_thread_info.disp_callback(CAS_MSG_EMAIL_NOTIFY, 0);  
				}
				 break;
			case GYCA_EXECUTE_OP_INFO:	/* 要求机顶盒执行具体的机顶盒操作*/
				 break;
			case GYCA_SW_UPDATE_INFO:	/* 有EMM,PEMM 软件更新信息通知*/
				 break;
			case GYCA_TRANSPARENT_INFO:	/* CA 透传给机顶盒的私有信息*/
				{
					//if( MEMCMP((UINT8 *)&g_stChgchl_info.m_para,(UINT8 *)param1,sizeof(T_CARemoteControlInfo)) == 0)
					//{
					//	break;
					//}
					MEMCPY((UINT8 *)&g_stChgchl_info.m_para,(UINT8 *)param1,sizeof(T_CARemoteControlInfo));
					g_stChgchl_info.m_len = param2;

					if (NULL != cas_thread_info.disp_callback)
					{
			     			cas_thread_info.disp_callback(CAS_MSG_FORCE_TRANSPARENT, 0);  
					}

				 }
				break;
				#if 0
			case GYCA_TRANSPARENT_INFO:	/* CA 透传给机顶盒的私有信息*/
				{
					T_CARemoteControlInfo *t_info = (T_CARemoteControlInfo *)param1;
					UINT32 t_info_len = param2;
					UINT8  OpCode;

					if(t_info_len == 0)
					{
						libc_printf("GYCA_TRANSPARENT_INFO fail!, parameter is empty\n");
						break;
					}
		
					OpCode = t_info->m_byOpCode;
					switch(OpCode)
					{
						case 0x00:    /* 要求搜索指定频点*/
						{
							POBJECT_HEAD submenu;
							UINT32 freq = t_info->m_dwFrequency/10;
							UINT32	symbol = 6875,mode = 6;/*6:QAM64*/
							UINT8 i,lock;
							//struct nim_device *nim_dev;
							libc_printf("focre search prog ,freq is %d\n",freq);

							/*LOCK*/
							union ft_xpond xpond;
							    struct nim_device *nim = dev_get_by_id(HLD_DEV_TYPE_NIM, 0);
							    MEMSET(&xpond, 0, sizeof(xpond));
							    xpond.c_info.type = FRONTEND_TYPE_C;
							    xpond.c_info.frq = freq;
							    xpond.c_info.sym = symbol;
							    xpond.c_info.modulation = mode;
							frontend_set_nim(nim, NULL, &xpond, 1);
							
							for (i=0; i<100; i++)
							{
								nim_get_lock(nim, &lock);
								if (lock)
								{
									libc_printf("$$NIMLOCK use %d ms!", i*10);
									break;
								}
								osal_task_sleep(10);
								
							}
							/*SEARCH*/
							submenu = (POBJECT_HEAD) &g_win_search;
							gy_force_search_setflag(TRUE);
							win_set_search_param(0, freq , symbol, mode, 0);
							SetSearchParamMode(1);						
							if (OSD_ObjOpen(submenu, 0xFFFFFFFF) != PROC_LEAVE)
							{
								libc_printf("force search ok\n");
							}
							else
							{
								libc_printf("force search fail\n");
							}

						 }
						break;
						case 0x01:    /* 换台指令*/
						   {
								UINT32 freq;
								UINT16 ts_id;
								UINT16 s_id;
								T_NODE t_node;
								P_NODE p_node;
								UINT32 j;
								UINT32 ch_cnt = 0;
							
								libc_printf("nGYCA_TRANSPARENT_INFO: channel change\n");
								if(t_info_len < sizeof(T_CARemoteControlInfo))
								{
									libc_printf("nGYCA_TRANSPARENT_INFO: channel change, parameter err!\n");
									break;
								}
								freq = t_info->m_dwFrequency;
								ts_id = t_info->m_wTsID;
								s_id = t_info->m_wServiceID;
								libc_printf("\n======freq:0x%x(%d),ts_id:0x%x(%d),s_id:0x%x(%d)\n",freq,freq,ts_id,ts_id,s_id,s_id);

								if(get_search_signal_menu_status())
								{
									libc_printf("GYCA_TRANSPARENT_INFO, search is running, so don't change channel\n");
									break;
								}
								MEMSET(&t_node, 0, sizeof(T_NODE));
								MEMSET(&p_node, 0, sizeof(P_NODE));

								sys_data_change_group(ALL_GROUP_TYPE);
					            		ch_cnt = get_prog_num(VIEW_ALL|PROG_ALL_MODE, 0);
								if (ch_cnt == 0)
								{
									libc_printf("GYCA_TRANSPARENT_INFO, channel change fail!, STB not any program\n");
									break;
								}
								
								for(j=0; j<ch_cnt; j++)
								{
									get_prog_at(j, &p_node);
									if(p_node.prog_number == s_id)
									{
										get_tp_by_id(p_node.tp_id, &t_node);
										if((t_node.frq*10 == freq))// && (t_node.t_s_id == ts_id))
										{
											api_play_channel(j, TRUE, FALSE, FALSE);
											libc_printf("GYCA_TRANSPARENT_INFO, channel change sucessful!\n");

											break;
										}
									}
								}
								if(j>=ch_cnt)
								{
									libc_printf("GYCA_TRANSPARENT_INFO, channel change fail!, STB not find program of match\n");
									break;
								}
							}
							break;
						default:
							break;
						}	
				 }
				 break;
				#endif
			case GYCA_FINGER_SHOW:		/* 要求显示指纹信息*/
				gycas_save_fingermsg_info(param1,param2);
				if (NULL != cas_thread_info.disp_callback)
			     	cas_thread_info.disp_callback(CAS_MSG_FINGER_PRINT, 0); 
				 break;
			case GYCA_FINGER_ERASE:		/* 要求擦除指纹信息*/
				if (NULL != cas_thread_info.disp_callback)
					cas_thread_info.disp_callback(CAS_MSG_FINGER_HID, 0);
				 break;
			case GYCA_IPPV_INFO:			/* IPPV 信息*/
				if(FALSE == gycas_save_ippvmsg_info(param1,param2))
					break;
				if (NULL != cas_thread_info.disp_callback)
		     		cas_thread_info.disp_callback(CAS_MSG_START_IPPVDLG, 0); 
				 break;		
		       default:
			    break;
       	}
    
	return SUCCESS;
}


