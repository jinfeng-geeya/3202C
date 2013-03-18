/*****************************************************************************************************
  * Description
  *  This file implement tongfang CAS internal struct
  * Remarks
  *   None.
  * Bugs
  *   None.
  * TODO
  *    * None.
  * History
  *   <table>
  *   \Author         Date        Change Description
  *   ----------    ----------  -------------------
  *   Qingtao Wang   2008.06.18  Initialize
  *   </table>
  *
*****************************************************************************************************/
#ifndef  __CAS_GY_H__
#define  __CAS_GY_H__

#include <sys_define.h>
//#include <api/libcas/gy/sys_types.h>
#include <basic_types.h>
#include <api/libc/list.h>
//#include <mdl/algo/link_list.h>
//#include <mdl/cas/cas_ui.h>
//#include <mdl/cas/cas.h>
//#include <mdl/db/db_nvram_ca_mail.h>
#include <api/libcas/gy/gyca_def.h>
#include <api/libcas/gy/gyca_interface.h>
#include <api/libcas/gy/gyca_osp.h>
#include <api/libcas/gy/gyca_pub.h>
//#include <mdl/algo/fast_crc.h>

#include <hld/dmx/dmx.h>

#define EMM_TASK

#if 0
#define GYCAS_ERR	PRINTF
#else
#define GYCAS_ERR(...)	do{}while(0)
#endif

#define GYCAS_STO
//#define OSAL_SEMA_TFSEMA

#define GYCAS_MAX_ECM_NUM	 2
#define GYCAS_FILTER_NUM	8
#define GYCAS_ELEM_NUM	2
#define GYCAS_CH_BUF_ECM_SIZE	0x1000
#define GYCAS_CH_BUF_EMM_SIZE	0x10000
#define GYCAS_MAX_SEMA_NUMBER	10
#define GYCAS_MAX_TASK_NUMBER	8
#define GYCAS_MAX_MSGQ_NUMBER 5
#define GYSTB_BUF_SIZE	0x10000

#define GYCAS_FILTER_MAX_LEN	15
#define GYCAS_MAX_MAIL			52

#define GYCAS_MAIL_RECORD_ID	0
#define GYCAS_LIB_FLASH_ID	1
#if 1
typedef struct
{
	UINT32 mail_id;
	UINT8 status;  /*1:new  0:old*/
	T_CAMailInfo mail_data;
}gycas_mail_t;

typedef struct {
	struct list_head list;
	void *ptr;
}gycas_buf_link_t;

typedef struct {
	INT32 chid;
	INT32 desc_id;
	UINT16 pid;
	UINT8 cw[16];
	UINT8 *ch_buf;
}gycas_ch_info_t;

typedef struct  {
	INT32 filter_id;
	INT32 req_id;
	UINT8 filter[MAX_SEC_MASK_LEN];
	UINT8 mask[MAX_SEC_MASK_LEN];
}gycas_req_param_t;

typedef struct  {
	UINT16 service_id;
	UINT16 ca_system_id;
	UINT16 ecm_pid;
	UINT16 es_pid;
	INT32 es_channel_id;
}cas_desc_ecm_t;

typedef struct  {
	UINT16 network_id;
	UINT16 ts_id;
	UINT16 ca_system_id;
	UINT16 emm_pid;
}cas_desc_emm_t;

typedef struct  {
	UINT32 sto_offset;
	UINT32 sto_len;
	UINT16 ecm_pid[GYCAS_ELEM_NUM];

	#ifdef EMM_TASK
	UINT8 *emmbuf;
	UINT32 emmbuf_read;
	UINT32 emmbuf_write;
	UINT32 emmbuf_end;
	OSAL_ID emmtask;
	#endif
	
	cas_desc_emm_t descemm;
	cas_desc_ecm_t descecm[GYCAS_MAX_ECM_NUM];

	struct list_head buf_list;
	OSAL_ID TaskTable[GYCAS_MAX_TASK_NUMBER];
	OSAL_ID SemaTable[GYCAS_MAX_SEMA_NUMBER];
	OSAL_ID msgq_table[GYCAS_MAX_MSGQ_NUMBER];

	//OSAL_MUTEX_ID MutexTable[GYCAS_MAX_SEMA_NUMBER];
	gycas_ch_info_t emm_ch_info;
	gycas_ch_info_t ecm_ch_info[GYCAS_MAX_ECM_NUM];
	gycas_req_param_t req_param[MAX_MV_NUM];
	UINT8 req_filter_value[MAX_MV_NUM][GYCAS_FILTER_MAX_LEN];

	struct sto_device *sto_dev;
	struct dmx_device *dmx_dev;
	struct smc_device *smc_dev;
	OSAL_ID mutex;
	UINT8 stb_id[20];

	/***********************************/
	SC_Status_Notify_Fct_t smc_notify;
	UINT16 service_id;
	UINT16 atr_size;
	UINT8 atr[33];

	OSAL_ID osd_bottom_timer;
	OSAL_ID osd_top_timer;
	OSAL_ID osd_mid_timer;
	UINT8 first_play_flag ;
	UINT8 nit_got;
	//cas_adv_t  cas_adv;
	UINT8 gycas_msg_str[100];
}gycas_info_priv_t;

gycas_info_priv_t* gycas_get_priv(void);
//RET_CODE gycas_ui_attach(void);
//RET_CODE gycas_ui_detach(void);

//extern void smc_callback(UINT32 param);
//extern casui_info_t* casui_get_info(void);
//extern cas_info_t* cas_get_info(void);
#endif
#endif  /* __CAS_TF_H__*/
