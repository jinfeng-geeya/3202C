/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2006 Copyright (C)
 *
 *  File: cd_catask.c
 *
 *  Description: This file contains all functions for CDCA 3.
 *
 *  History:
 *      Date            Author            Version   Comment
 *      ====        ======      =======  =======
 *  1. 2006.3.19  Gushun Chen      0.1.000    Initial
 *  2. 2007.7.25  Grady Xu		 0.2.000	  re-org the Tongfang CA lib.
 *  3. 2007.11.5  Grady Xu		 0.3.000	  add lock service
 ****************************************************************************/

#include <sys_config.h>
#include <types.h>
#include <retcode.h>
#include <hld/hld_dev.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/alloc.h>
#include <api/libc/string.h>
#include <hld/sto/sto.h>
#include <hld/sto/sto_dev.h>
#include <api/libchunk/chunk.h>
#include <hld/dmx/dmx.h>
#include <api/libdb/db_config.h>
#include <api/libdb/db_node_c.h>
//#include <api/libcas/cdca/pub_st.h>
#include <api/libcas/cdca/pub_option.h>
#include <api/libcas/cdca/CDCAS30.h>
//#include <api/libcas/cdca/cas_cd.h>

#include "cd_catask.h"
#include "cd_dmx.h"

#define CAS_CHANNEL_INVALID	0xFF
#define CAS_PMT_SEC_LEN	4096
#define CAS_CAT_SEC_LEN	1024

static UINT8 cas_cat_buffer[CAS_CAT_SEC_LEN];
mcas_t cas_struct;
static UINT8 cas_cat_channel_id = CAS_CHANNEL_INVALID;
static struct dmx_device *cas_dmx_dev = NULL;

static OSAL_ID cas_filter_sema = OSAL_INVALID_ID;
void cas_free_filter(UINT8 *filter_id)
{
	if(cas_filter_sema == OSAL_INVALID_ID)
	{
		if ((cas_filter_sema = osal_semaphore_create(1))==OSAL_INVALID_ID)
		{
			CAS_ERROR_PRINTF("create semaphore failure with %d\r", cas_filter_sema); \
		}
	}

	while(osal_semaphore_capture(cas_filter_sema, 1000) != SUCCESS)
			CAS_ERROR_PRINTF("cas_sema capture timeout\n");
	if(*filter_id != CAS_CHANNEL_INVALID)
	{
		OTHER_PRINTF("cas_free_filter: close filter %d\n", *filter_id);
		dmx_io_control(cas_dmx_dev, IO_ASYNC_CLOSE, *filter_id);
		*filter_id = CAS_CHANNEL_INVALID;		
	}
	osal_semaphore_release(cas_filter_sema);
}

/**
Monitor MSG
**/
#define CAS_MSG_SEC_PMT	0
#define CAS_MSG_SEC_CAT	1
#define CAS_MSG_SMC_REMOVE	2
#define CAS_MSG_SMC_INSERT	3

struct cas_msg
{
	UINT8 type;
	UINT32 param;
};
void cas_send_msg(UINT8 type, UINT32 param)
{
	struct cas_msg msg;
	msg.type = type;
	msg.param = param;
	osal_msgqueue_send(cas_struct.msgque_id, &msg, sizeof(struct cas_msg), OSAL_WAIT_FOREVER_TIME);
}

struct restrict cas_pmt_restrict;
struct get_section_param cas_pmt_param;
struct restrict cas_cat_restrict;
struct get_section_param cas_cat_param;

struct ecm_pid cas_ecm_pid;
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
	
	cas_ecm_pid.uniform_ecm_pid = INVALID_PID;
	cas_ecm_pid.video_ecm_pid = INVALID_PID;
	cas_ecm_pid.audio_ecm_pid = INVALID_PID;
	cas_ecm_pid.ttx_ecm_pid = INVALID_PID;
	cas_ecm_pid.subt_ecm_pid = INVALID_PID;

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
			if(CDCASTB_IsCDCa(ca_system_id))
			{
				cas_ecm_pid.uniform_ecm_pid = ecm_pid;
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
				if(CDCASTB_IsCDCa(ca_system_id))
				{
					if((stream_type == 0x01)||(stream_type == 0x02))	//Video
					{
						cas_ecm_pid.video_ecm_pid = ecm_pid;
						ca_discriptor_num ++;	
					}
					else if((stream_type == 0x03)||(stream_type == 0x04))	//Audio
					{
						cas_ecm_pid.audio_ecm_pid = ecm_pid;
						ca_discriptor_num ++;
					}
					if(cas_ecm_pid.video_ecm_pid == cas_ecm_pid.audio_ecm_pid)
					{
						cas_ecm_pid.uniform_ecm_pid = ecm_pid;
						cas_ecm_pid.video_ecm_pid = INVALID_PID;
						cas_ecm_pid.audio_ecm_pid = INVALID_PID;
						ca_discriptor_num = 1;
					}
				}			
			}
			loop_length -= 2+discriptor_pointer[1];
			discriptor_pointer += 2+discriptor_pointer[1];
		}

		left_len -= 5+es_info_len;

	}

	if(cas_ecm_pid.uniform_ecm_pid != INVALID_PID || cas_ecm_pid.video_ecm_pid !=INVALID_PID
		|| cas_ecm_pid.audio_ecm_pid != INVALID_PID || cas_ecm_pid.ttx_ecm_pid != INVALID_PID 
		|| cas_ecm_pid.subt_ecm_pid != INVALID_PID)
		return RET_SUCCESS;
	else
		return !RET_SUCCESS;
}

void parse_cat(UINT8 *section, INT32 length)
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
			if(CDCASTB_IsCDCa(ca_system_id))
			{
				emm_pid = ((section[index+4]&0x1F)<<8) | section[index+5];
#ifdef CHCHG_TICK_PRINTF
				libc_printf("\n####CHCHG %s():line%d, SetEmmPid=0x%x,tick=%d\n", __FUNCTION__,__LINE__, emm_pid,osal_get_tick());
#endif				
				CDCASTB_SetEmmPid(emm_pid);
			}
		}
		cat_length -= 2 + section[index+1];
		index += 2 + section[index+1];
	}
	return;
}


void cas_cat_sec_cb(struct get_section_param *cas_sec_param)
{
	cas_sec_param->continue_get_sec = 0;
#ifdef CHCHG_TICK_PRINTF
	libc_printf("\n####CHCHG %s():line%d, tick=%d\n", __FUNCTION__,__LINE__, osal_get_tick());
#endif	
	parse_cat(cas_sec_param->buff, cas_sec_param->buff_len);
	cas_send_msg(CAS_MSG_SEC_CAT, 0);
	return;
}

INT32 lib_cas_start(/*UINT16 pmt_pid, UINT16 service_id*/)
{
	RET_CODE ret;

/*request channel for CAT*/
	MEMSET(cas_cat_buffer, 0, sizeof(cas_cat_buffer));
	MEMSET(&cas_cat_restrict, 0, sizeof(struct restrict));

	cas_cat_restrict.mask_len = 6; 
	cas_cat_restrict.multi_mask[0][0] = 0xFF;		/* Table ID */
//	cas_cat_restrict.mask[0][1] = 0x40;		/* Section syntax indicator */
						/* '0' */
//	cas_cat_restrict.mask[0][5] = 0x01;		/* Current next indicator */

	cas_cat_restrict.tb_flt_msk = 1;
	cas_cat_restrict.value[0][0] = 0x01;	/* Table ID */
//	cas_cat_restrict.value[0][1] = 0x80;	/* Section syntax indicator */
						/* '0' */
//	cas_cat_restrict.value[0][5] = 0x01;	/* Current next indicator */

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
		CAS_ERROR_PRINTF("lib_cas_start alloc cat_filter error\n");
		return !RET_SUCCESS;
	}
	OTHER_PRINTF("lib_cas_start cat_filter: %d\n", cas_cat_channel_id);

	return RET_SUCCESS;
}

INT32 lib_cas_stop()
{
	OTHER_PRINTF("cas_stop\n");

	cas_free_filter(&cas_cat_channel_id);
	//CDCASTB_SetEcmPid(TFCAS_LIST_FIRST, NULL);
	//CDCASTB_SetEcmPid(TFCAS_LIST_OK, NULL);
	//CDCASTB_SetEmmPid(0);
	return RET_SUCCESS;
}


static void cas_smc_notify(UINT8 inserted)
{
	if(inserted == 0)
		cas_send_msg(CAS_MSG_SMC_REMOVE, 0);
	else if(inserted == 1)
		cas_send_msg(CAS_MSG_SMC_INSERT, 0);
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
	//lib_cas_stop();
#ifdef CHCHG_TICK_PRINTF
	libc_printf("\n####CHCHG %s():line%d, tick=%d\n", __FUNCTION__,__LINE__, osal_get_tick());
#endif	
	if(cas_parse_pmt(section, length) == RET_SUCCESS)
	{
		cas_send_msg(CAS_MSG_SEC_PMT, 0);
		//lib_cas_start();
		return RET_SUCCESS;	
	}
	else 
	{
		CDCASTB_SetEcmPid(CDCA_LIST_FIRST, NULL);
		CDCASTB_SetEcmPid(CDCA_LIST_OK, NULL);
		return !RET_SUCCESS;
	}
}


extern UINT8 purse_symbol[4];
static void cas_monitor_thread(UINT32 param1, UINT32 param2)
{
	UINT8 i;
	struct cas_msg msg;
	UINT32 msg_len;
	struct smc_device *smc_dev = NULL;
	struct dmx_device *dmx_dev = NULL;
	SCDCASServiceInfo cas_info;

	dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);
	smc_dev = (struct smc_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_SMC);
	if(smc_dev == NULL)
	{		
		CAS_ERROR_PRINTF("Get Smart card reader failed\n");
		ASSERT(0);
	}
	if(RET_SUCCESS != smc_open(smc_dev, cas_smc_notify))
	{
		CAS_ERROR_PRINTF("smc_open failed!!\n");
		ASSERT(0);
	}

	if(cas_filter_sema == OSAL_INVALID_ID)
	{
		if ((cas_filter_sema = osal_semaphore_create(1))==OSAL_INVALID_ID)
		{
			CAS_ERROR_PRINTF("create semaphore failure with %d\r", cas_filter_sema); \
		}
	}
			
	cas_dmx_dev = (struct dmx_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DMX);
	if(cas_dmx_dev == NULL)
	{
		CAS_ERROR_PRINTF("lib_cas_start: get dmx error!\n");
		ASSERT(0);
	}

	si_monitor_register_pmt_cb(cas_pmt_callback);

	while (1)
	{
		if (osal_msgqueue_receive(&msg, &msg_len, cas_struct.msgque_id, 100) != RET_SUCCESS)
		{
			//check if nit data is requred
			cas_nit_callback();
			//check if writing flash is requred
			if (CDCA_TRUE == CAS_WriteEnable())
				CAS_WriteFlash();
			continue;
		}
		
		switch (msg.type)
		{
			case CAS_MSG_SEC_PMT:
				//cas_demux_free(CAS_DMX_ECM_REQUESTID);	// free ecm filter		
				MEMSET(&cas_info, 0, sizeof(SCDCASServiceInfo));				
				cas_info.m_byServiceNum = 1;
				cas_info.m_wServiceID[0] = cas_ecm_pid.service_id;
				osal_mutex_lock(cas_struct.mutex_id, OSAL_WAIT_FOREVER_TIME);
				CDCASTB_SetEcmPid(CDCA_LIST_FIRST, NULL);
#ifdef CHCHG_TICK_PRINTF
				libc_printf("\n####CHCHG %s():line%d, SetEcmPid,tick=%d\n", __FUNCTION__,__LINE__, osal_get_tick());
#endif				
				if(cas_ecm_pid.uniform_ecm_pid != INVALID_PID)
				{
					cas_info.m_wEcmPid = cas_ecm_pid.uniform_ecm_pid;
					CDCASTB_SetEcmPid(CDCA_LIST_ADD, &cas_info);
				}
				else 
				{
					if(cas_ecm_pid.video_ecm_pid != INVALID_PID)
					{
						cas_info.m_wEcmPid = cas_ecm_pid.video_ecm_pid;
						CDCASTB_SetEcmPid(CDCA_LIST_ADD, &cas_info);
					}
					if (cas_ecm_pid.audio_ecm_pid != INVALID_PID)
					{
						cas_info.m_wEcmPid = cas_ecm_pid.audio_ecm_pid;
						CDCASTB_SetEcmPid(CDCA_LIST_ADD, &cas_info);
					}
				}
				
				CDCASTB_SetEcmPid(CDCA_LIST_OK, NULL);
				osal_mutex_unlock(cas_struct.mutex_id);
				break;
			case CAS_MSG_SEC_CAT:
				cas_free_filter(&cas_cat_channel_id);
				break;				
			case CAS_MSG_SMC_INSERT:
#ifdef CHCHG_TICK_PRINTF
				libc_printf("####CHCHG %s():line%d, smc insert, tick=%d\n", __FUNCTION__,__LINE__, osal_get_tick());
#endif				
				OTHER_PRINTF("smc_thread: smc insert!\n");
				//M3701E not use bypass_CSA but IO_SET_DEC_STATUS
				if (sys_ic_get_chip_id() >= ALI_S3602F)
					dmx_io_control(dmx_dev, IO_SET_DEC_STATUS, (UINT32)1);
				//dmx_io_control(dmx_dev, DMX_BYPASS_CSA, 0);
				cas_struct.disp_callback(CAS_MSG_SMC_IN, 0);
				osal_mutex_lock(cas_struct.mutex_id, OSAL_WAIT_FOREVER_TIME);
				CDCASTB_SCInsert();
				osal_mutex_unlock(cas_struct.mutex_id);
				break;
			case CAS_MSG_SMC_REMOVE:
#ifdef CHCHG_TICK_PRINTF
				libc_printf("####CHCHG %s():line%d, smc remove, tick=%d\n", __FUNCTION__,__LINE__, osal_get_tick());
#endif					
				OTHER_PRINTF("smc_thread: smc remove!\n");
				cas_struct.disp_callback(CAS_MSG_SMC_OUT, 0);
				//to stop decrypt immediately
				//M3701E not use bypass_CSA but IO_SET_DEC_STATUS
				if (sys_ic_get_chip_id() >= ALI_S3602F)
					dmx_io_control(dmx_dev, IO_SET_DEC_STATUS, (UINT32)0);
				dmx_io_control(dmx_dev, DMX_BYPASS_CSA, 1);
				CDCASTB_SCRemove();
				break;
			default:
				CAS_DEBUG_PRINTF("smc_thread: unknow msg\n");
				break;
		}

		//check if nit data is requred
		cas_nit_callback();
		//check if writing flash is requred
		if (CDCA_TRUE == CAS_WriteEnable())
			CAS_WriteFlash();
	}
}
cas_disp_func_t cas_disp_cb =NULL;
void cas_register_disp_cb(cas_disp_func_t mcas_disp_func)
{
    if(NULL == mcas_disp_func)
    {
        cas_struct.disp_callback = cas_disp_cb;
    }
    else
    {
        cas_struct.disp_callback = mcas_disp_func;
    }
}

void cas_monitor_init(cas_disp_func_t mcas_disp_func)
{
	OSAL_T_CTSK t_ctsk;	
	OSAL_T_CMBF t_cmbf;
	
	MEMSET(&cas_struct, 0, sizeof(mcas_t));
    cas_disp_cb = mcas_disp_func;
	cas_struct.disp_callback = mcas_disp_func;
	cas_struct.msgque_id = OSAL_INVALID_ID;
	cas_struct.mutex_id = OSAL_INVALID_ID;
	cas_demux_init();
	
	/* create cas massage buffer */
	t_cmbf.bufsz = 0x800;//512*4
	t_cmbf.maxmsz = sizeof(struct cas_msg);	//8;
	cas_struct.msgque_id = osal_msgqueue_create(&t_cmbf);
	if (OSAL_INVALID_ID == cas_struct.msgque_id)
	{
		CAS_ERROR_PRINTF("Can not create cas message buffer!!!\n");
		ASSERT(0);
	}

	cas_struct.mutex_id = osal_mutex_create();
	if (OSAL_INVALID_ID == cas_struct.mutex_id)
	{
		CAS_ERROR_PRINTF("Can not create cas message buffer!!!\n");
		ASSERT(0);
	}

	t_ctsk.task = (OSAL_T_TASK_FUNC_PTR)cas_monitor_thread;
	t_ctsk.itskpri = OSAL_PRI_NORMAL;
	t_ctsk.stksz = 0x1000;
	t_ctsk.quantum = 10;
	t_ctsk.name[0] = 'S';
	t_ctsk.name[1] = 'M';
	t_ctsk.name[2] = 'C';
	cas_struct.thread_id = osal_task_create(&t_ctsk);
	if (OSAL_INVALID_ID == cas_struct.thread_id)
	{
		CAS_ERROR_PRINTF("Can not create monitor thread!!!\n");
		ASSERT(0);
	}
}



/************************Actual API implementation *******************/

//-------------------- 授权信息管理 --------------------
extern void CDSTBCA_EntitleChanged(CDCA_U16 wTvsID)
{
	CAS_DEBUG_PRINTF("@@%s: wTvsID = 0x%x\n", __FUNCTION__, wTvsID);
	
	cas_struct.disp_callback(CAS_MSG_ENTITLE_CHANGED, wTvsID);
	return;
}
#if INCLUDE_DETITLE_APP
extern void CDSTBCA_DetitleReceived(CDCA_U16 wTvsID, CDCA_U8 bstatus)
{
	CAS_DEBUG_PRINTF("@@@@%s: wTvsID = 0x%x, bstatus=0x%x\n", __FUNCTION__, wTvsID, bstatus);
	cas_struct.disp_callback(CAS_MSG_DETITLE_RECEIVED, (wTvsID<<8)|bstatus);
	return;
}
#endif

//-------------------- 安全控制 --------------------
static UINT32 cas_uniqueID = 0;
#ifndef SERIAL_NUM_OFF
#define SERIAL_NUM_OFF					10
#endif
#ifndef SERIAL_NUM_LEN
#define SERIAL_NUM_LEN					7
#endif
static UINT32 asc_str_to_i(UINT8* buf,UINT8 len)
{
	UINT32 ret = 0;
	UINT8 i = 0;
	for(i=0;i<len;i++)
	{
		ret = ret*10+buf[i]-'0';
	}
	return ret;
}

extern void     CDSTBCA_GetSTBID( CDCA_U16* pwPlatformID,  CDCA_U32* pdwUniqueID)
{
	INT32 ret;
	UINT32 id = 0;
	UINT32 chunkadd = 0, offset = 60, uniqueid = 0;
	CHUNK_HEADER chuck_hdr;
    char serial_buf[SERIAL_NUM_OFF+SERIAL_NUM_LEN];
	
	*pwPlatformID = CDCASTB_GetPlatformID();
	//*pdwUniqueID = 0x00000001; //for debug only
	//return;
	if (0 == cas_uniqueID)//the first time to get ID, got it from flash
	{
		#if 0
		//from bootload header, the last four bytes of time are used for unique ID
		uniqueid = sto_fetch_long(chunkadd + offset);
		if (0 == uniqueid )
		{
			uniqueid = sto_fetch_long(chunkadd + offset);
			if (0 == uniqueid)
			{
				CAS_ERROR_PRINTF("$$%s: Got stbid error!\n", __FUNCTION__);
			}
		}

		cas_uniqueID = uniqueid;	
		#else
		//STB_HWINFO locate in STB_HWINFO_BASE_ADDR(0x1E000), defined in stb_hwinfo.h
		struct sto_device *sto = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
		if(NULL != sto)
		{	
			//STB_HWINFO_BASE_ADDR = 0x1E000
			MEMSET(serial_buf,0,SERIAL_NUM_OFF+SERIAL_NUM_LEN);
			sto_get_data(sto, serial_buf, 0x1E000, SERIAL_NUM_OFF+SERIAL_NUM_LEN);
			cas_uniqueID = asc_str_to_i(&serial_buf[SERIAL_NUM_OFF],SERIAL_NUM_LEN);
		}
		#endif
	}

	*pdwUniqueID = cas_uniqueID;
	//CAS_DEBUG_PRINTF("%s: pwPlatformID=0x%x, pdwUniqueID=0x%x\n", __FUNCTION__, *pwPlatformID, *pdwUniqueID);
	return;
}
extern CDCA_U16 CDSTBCA_SCFunction( CDCA_U8* pData)
{
	CAS_DEBUG_PRINTF("%s: pdata=0x%x\n", __FUNCTION__, *pData);
	return 0x9100;//No security module.
}

//-------------------- IPPV应用 --------------------
#if INCLUDE_IPPV_APP
extern void CDSTBCA_HideIPPVDlg(CDCA_U16 wEcmPid)
{
	CAS_DEBUG_PRINTF("@@@@%s:wEcmPid=0x%04x\n", __FUNCTION__, wEcmPid);
	cas_struct.disp_callback(CAS_MSG_HIDE_IPPVDLG, wEcmPid);
	return;
}
extern void CDSTBCA_StartIppvBuyDlg(CDCA_U8 byMessageType, CDCA_U16 wEcmPid,  SCDCAIppvBuyInfo *pIppvProgram)
{
	CAS_DEBUG_PRINTF("@@@@%s:byMessageType=0x%x, wEcmPid=0x%x, \n", __FUNCTION__, byMessageType, wEcmPid);
	MEMCPY(&(cas_struct.Ippvbuyinfo), pIppvProgram, sizeof(SCDCAIppvBuyInfo));
	cas_struct.ippvstatus = (wEcmPid<<8)|byMessageType;
	cas_struct.disp_callback(CAS_MSG_START_IPPVDLG,cas_struct.ippvstatus);
	return;
}
#endif

//-------------------- 邮件/OSD显示管理 --------------------
#if INCLUDE_OSD_MAIL_APP
extern void CDSTBCA_ShowOSDMessage(CDCA_U8 byStyle,  char* szMessage)
{
	UINT16 len = 0;
	CAS_DEBUG_PRINTF("@@%s: byStyle=0x%x, szMessage=%s", __FUNCTION__, byStyle, szMessage);

	len = STRLEN(szMessage);
	if (len > CDCA_MAXLEN_OSD)
	{
		CAS_ERROR_PRINTF("!!!!ShowOSDMessage, input string size is bigger than defined!\n");
		len = CDCA_MAXLEN_OSD;
	}
	MEMSET(cas_struct.cas_osdmsg_buf, 0, sizeof(cas_struct.cas_osdmsg_buf));
	convert_gb2312_to_unicode(szMessage, len, cas_struct.cas_osdmsg_buf, CDCA_MAXLEN_OSD);
	//MEMCPY(cas_struct.cas_osdmsg_buf, szMessage, STRLEN(szMessage));
	cas_struct.disp_callback(CAS_MSG_SHOW_OSD_MESSAGE,byStyle);
	return;
}
extern void CDSTBCA_HideOSDMessage(CDCA_U8 byStyle)
{
	CAS_DEBUG_PRINTF("@@%s: byStyle=0x%x", __FUNCTION__, byStyle);
	cas_struct.disp_callback(CAS_MSG_HIDE_OSD_MESSAGE,byStyle);
	
	return;
}
extern void CDSTBCA_EmailNotifyIcon(CDCA_U8 byShow, CDCA_U32 dwEmailID)
{
	CAS_DEBUG_PRINTF("@@%s:byShow=0x%2x, EmailID=0x%08x\n", __FUNCTION__, byShow, dwEmailID);
	cas_struct.disp_callback(CAS_MSG_EMAIL_NOTIFY,byShow);

	return;
}
#endif

//-------------------- 子母卡应用 --------------------
#if INCLUDE_CHILDCARD_APP
extern void  CDSTBCA_RequestFeeding(CDCA_U16 wTVSID)
{
	CAS_DEBUG_PRINTF("@@@@%s: wTVSID=0x%x", __FUNCTION__,wTVSID);
	cas_struct.disp_callback(CAS_MSG_FEEDING_REQUEST,wTVSID);
	return;
}
#endif

//-------------------- 强制切换频道 --------------------
#if INCLUDE_LOCK_SERVICE
extern void CDSTBCA_LockService( SCDCALockService* pLockService)
{
	CAS_DEBUG_PRINTF("%s: is called\n", __FUNCTION__);
	MEMCPY(&cas_struct.lockinfo, pLockService, sizeof(SCDCALockService));
	cas_struct.disp_callback(CAS_MSG_LOCK_SERVICE, 0);
	return;
}

extern void CDSTBCA_UNLockService(void)
{
	CAS_DEBUG_PRINTF("@@@@%s is called\n", __FUNCTION__);
	cas_struct.disp_callback(CAS_MSG_UNLOCK_SERVICE, 0);
	return;
}
#endif

//-------------------- 显示界面管理 --------------------
extern void CDSTBCA_ShowProgressStrip(CDCA_U8 byProgress, CDCA_U8 byMark)
{
	CAS_DEBUG_PRINTF("@@%s: byProgress=0x%x, byMark=%x", __FUNCTION__, byProgress, byMark);
	cas_struct.disp_callback(CAS_MSG_SHOW_PROGRESS,(byProgress<<8)|byMark);
	return;
}
extern void CDSTBCA_ShowBuyMessage(CDCA_U16 wEcmPID, CDCA_U8 byMessageType)
{
	struct smc_device *smc_dev = NULL;

	smc_dev = (struct smc_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_SMC);
	if(smc_dev == NULL)
	{		
		CAS_DEBUG_PRINTF("STBCA_SCReset: Get Smart card reader failed\n");
		return ;
	}
	CAS_DEBUG_PRINTF("@@%s:wEcmPID=0x%04x, byMessageType=0x%02x\n",__FUNCTION__, wEcmPID, byMessageType);
	if(smc_card_exist(smc_dev) != SUCCESS)
	{
		if(byMessageType == CDCA_MESSAGE_BADCARD_TYPE)
			return ;
	}
	cas_struct.disp_callback(CAS_MSG_BUYMSG, (wEcmPID<<8)|byMessageType);
	return;	
}
extern void CDSTBCA_ShowFingerMessage(CDCA_U16 wEcmPID, CDCA_U32 dwCardID)
{
	CAS_DEBUG_PRINTF("@@%s: wEcmPID=0x%x, dwCardID=%d\n", __FUNCTION__, wEcmPID, dwCardID);
	cas_struct.CardID = dwCardID;
	cas_struct.disp_callback(CAS_MSG_FINGER_PRINT, dwCardID);
	return;
}
extern void  CDSTBCA_ShowCurtainNotify( CDCA_U16 wEcmPID,CDCA_U16  wCurtainCode)
{
    cas_struct.disp_callback(CAS_MSG_CURTAIN, wCurtainCode);
    CAS_DEBUG_PRINTF("@@%s: wEcmPID=0x%x, wCurtainCode=%d\n", __FUNCTION__, wEcmPID, wCurtainCode);
    return;
}

/************************UP layer get info *******************/
//AP call this function to get the IPPV dialog info
UINT8 CDSTBCA_GetIppvBuyInfo(SCDCAIppvBuyInfo *Ippv, UINT32 *status)
{
	if ((NULL == Ippv)||(NULL == status))
	{
		CAS_ERROR_PRINTF("@@@@GetIppvBuyInfo: NULL pointer!\n");
		return CDCA_FALSE;
	}
	//copy or set value??
	MEMCPY(Ippv, &(cas_struct.Ippvbuyinfo), sizeof(SCDCAIppvBuyInfo));	
	*status = cas_struct.ippvstatus;
	return CDCA_TRUE;
}
//AP call this to get Finger print
UINT32 CDSTBCA_GetFingerPrint()
{
	CAS_DEBUG_PRINTF("@@@@GetFingerPrint:CardID:%d\n", cas_struct.CardID);
	return cas_struct.CardID;
}

//This function is used to get MSG content by AP
UINT8 CDSTBCA_GetOSDMessage(UINT8* Message)
{
	if (NULL == Message)
	{
		CAS_ERROR_PRINTF("!!!!GotOSDMessage: NULL pointer!!\n");
		return CDCA_FALSE;
	}
	MEMCPY(Message, cas_struct.cas_osdmsg_buf, sizeof(cas_struct.cas_osdmsg_buf));
	return CDCA_TRUE;
}

//AP call this to get current lock service info
UINT8 CDSTBCA_GetLockSeviceinfo(SCDCALockService* plockservice)
{
	if (NULL == plockservice)
		return CDCA_FALSE;

	MEMCPY(plockservice, &cas_struct.lockinfo, sizeof(SCDCALockService));
	return CDCA_TRUE;
}

//AP call this to get current ecmpid info, rorw: 0-read info, 1-write info
UINT8 CDSTBCA_GetECMPIDinfo(UINT8 rorw,UINT16 *uni, UINT16 *video, UINT16 *audio)
{
	if (!rorw)
	{
		*uni = cas_ecm_pid.uniform_ecm_pid;
		*video = cas_ecm_pid.video_ecm_pid;
		*audio = cas_ecm_pid.audio_ecm_pid;
	}
	else
	{
		cas_ecm_pid.uniform_ecm_pid = *uni;
		cas_ecm_pid.video_ecm_pid = *video;
		cas_ecm_pid.audio_ecm_pid = *audio;
	}
}

INT32 api_mcas_stop_cas()
{
	CDCASTB_SetEmmPid(0);
	CDCASTB_SetEcmPid(CDCA_LIST_FIRST, NULL);
	CDCASTB_SetEcmPid(CDCA_LIST_OK, NULL);
	cas_demux_free(CAS_DMX_ECM_REQUESTID);
	cas_demux_free(CAS_DMX_EMM_REQUESTID);
}

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
		lib_cas_start();
		start_transponder = TRUE;
	}
}


#ifdef NEW_DEMO_FRAME
INT32 api_mcas_stop_service(UINT32 sim_id)
#else
INT32 api_mcas_stop_service()
#endif
{
	//lib_cas_stop();
	CDCASTB_SetEcmPid(CDCA_LIST_FIRST, NULL);
	CDCASTB_SetEcmPid(CDCA_LIST_OK, NULL);
	cas_demux_free(CAS_DMX_ECM_REQUESTID);
}

BOOL tf_enter_menu_or_smc_out = FALSE;

void api_tf_set_tp_status(BOOL value)
{
	tf_enter_menu_or_smc_out = value;
}

BOOL api_tf_get_tp_status()
{
	return tf_enter_menu_or_smc_out;
}


INT32 api_mcas_start_transponder()
{
	if(TRUE == api_tf_get_tp_status())
	{
		return SUCCESS;
	}
	if(start_transponder == FALSE)
	{
		lib_cas_start();
		start_transponder = TRUE;
	}
}
INT32 api_mcas_stop_transponder()
{
	if(TRUE == api_tf_get_tp_status())
	{
		return SUCCESS;
	}
	
   	CDCASTB_SetEmmPid(0);
    cas_demux_free(CAS_DMX_EMM_REQUESTID);    
	lib_cas_stop();
	start_transponder = FALSE;
}
INT32 api_mcas_switch_audiopid(UINT16 a_pid)
{
	
}


