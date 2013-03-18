#ifndef _CAS_GY_H
#define _CAS_GY_H

//#include <api/libc/time.h>
#include <api/libcas/gy/gyca_def.h>
#include <api/libc/list.h>

#define CAS_SUCCESS	0
#define CAS_FAIL		0xff
#define GY_OSDMSG_LEN	(210+2)//150//(210+2) 

#define GYCAS_MAX_SEMA_NUMBER	10
#define GYCAS_MAX_TASK_NUMBER	8
#define GYCAS_MAX_MSGQ_NUMBER 5
/*
#ifdef CT1642_WITHOUT_DIGITAL_TUBE 
typedef enum 
	{
		STANDBY_LED =0,
		IR_LED,
		EMAIL_LED,
		EXTENDED_LED,
	}Lit_Led_Type;
#endif
*/
typedef enum
{
	CAS_MSG_SHOWPROMPTMSG = 1,
	CAS_MSG_HIDEPROMPTMSG,	
	CAS_MSG_FINGER_PRINT,
	CAS_MSG_FINGER_HID,
	CAS_MSG_LOCK_SERVICE,
	CAS_MSG_SHOW_OSD_MESSAGE,
	CAS_MSG_HID_TOP_OSD_MESSAGE,
	CAS_MSG_HID_BTM_OSD_MESSAGE,
	CAS_MSG_HID_MID_OSD_MESSAGE,
	CAS_RESTART_CHAN,
	CAS_MSG_EMAIL_NOTIFY,
	CAS_MSG_START_IPPVDLG,
	CAS_MSG_HID_IPPVDLG,
	CAS_MSG_SMC_OUT,
	CAS_MSG_SMC_IN,
	CAS_MSG_NOSIGNAL,
	CAS_MSG_UNLOCK_SERVICE,
	CAS_MSG_URGENT_BROADCAST,
	CAS_MSG_FORCE_TRANSPARENT,/*强制换台或强制搜台的消息*/
}CAS_MSG_TYPE_T;

typedef enum
{
	CAS_IPPV_FREE 		= 0, /*免费PPV节目*/
	CAS_IPPV_PAY 		= 1, /*IPPV 付费*/
	CAS_IPPT_PAY 		= 2, /*IPPT付费*/
	CAS_IPPV_BOOK		= 3,	/*已预订*/
} cas_ippv_type;

typedef enum
{
	CAS_IPPV_STATUS_BOOKING,
	CAS_IPPV_STATUS_VIEWED,
	CAS_IPPV_STATUS_OTHER
}cas_ipp_status;

typedef struct {
	UINT16 year;
	UINT8 month;
	UINT8 day;
}cas_date;

typedef struct {
	UINT8 hour;
	UINT8 minute;
	UINT8 second;
}cas_time;

typedef struct {
	cas_date date;
	cas_time time;
}cas_datetime;


typedef struct
{
	cas_ippv_type ippvtype;			/*PPV类型,CAS_IPPV_FREE,CAS_IPPV_PAY,CAS_IPPT_PAY*/
	UINT32 product_id;				/*PPV产品ID*/
	UINT32 operator_id;				/*PPV运营商ID*/
	cas_ipp_status ipp_status;			/*PPV节目的状态*/
	UINT8 data[0];
	struct{
				UINT16 service_id;
				UINT16 event_id;
				UINT16 price;
				UINT16 platform_id;
				UINT32 ecm_pid;
				cas_datetime startdate;
				cas_datetime expiredate;
				cas_datetime buydate;
			};
}cas_ippv_info;


typedef void (*cas_disp_func_t)(CAS_MSG_TYPE_T type, unsigned long param);

typedef struct
{
	UINT16 network_id;
	UINT16 ts_id;
	UINT16 service_id;
	UINT16 duration;
}gy_urgent_info;

typedef struct
{
	UINT16 state;
	OSAL_ID thread_id;
	OSAL_ID msgque_id;
	OSAL_ID mutex_id;
	cas_disp_func_t disp_callback;
	//UINT32 finger_info[2]; //finger_info[0]: card_id, finger_info[1]: duration
	//UINT16 gy_osdbuf[GY_OSDMSG_LEN];
	//SDVTCAIpp ippinfo;
	//gy_urgent_info urgent;
} mcas_t;


typedef struct {
	struct list_head list;
	void *ptr;
}gycas_buf_link_t;

typedef struct  {
	UINT32 sto_offset;
	UINT32 sto_len;
	UINT16 ecm_pid;

	struct list_head buf_list;
	OSAL_ID TaskTable[GYCAS_MAX_TASK_NUMBER];
	OSAL_ID SemaTable[GYCAS_MAX_SEMA_NUMBER];
	OSAL_ID msgq_table[GYCAS_MAX_MSGQ_NUMBER];

	struct sto_device *sto_dev;
	struct dmx_device *dmx_dev;
	struct smc_device *smc_dev;
	OSAL_ID mutex;
	UINT8 stb_id[20];
	UINT8 stb_id_len;

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

typedef struct _gycamail{
	UINT8 flg;    /*1:valid  0:invalid*/
	UINT8 status;    /*1:new  0:old*/
	T_CAMailInfo  info;
}gycamail;

typedef struct _gycamail_head{
	UINT8 status;    /*1:new  0:old*/
	UINT8 level;			/*邮件等级 0-普通 1-紧急*/
	UINT8 create_Time[5]; 	/*创建时间 格式：MJD+HHMMSS*/
	UINT8 length;    	    /*邮件标题长度*/
	UINT8 title[40];     		/*邮件标题内容*/
}gycamail_head;

typedef struct _gycamail_content{
	UINT8 length;  	/*邮件内容长度*/
	UINT8 content[112]; 	 /*邮件内容*/
}gycamail_content;

typedef enum {
	MAIL_OLD_FULL = 0,
	MAIL_NEW ,
	MAIL_SPACE_FULL,
	MAIL_OTHERS,
}mail_status;

typedef struct _gycain{
	BYTE	m_byLen;
	BYTE	m_byszPin[6];
}gycapin;

typedef struct _gyca_forcechgchl{
	T_CARemoteControlInfo m_para;
	UINT32 m_len;
}T_gy_chgchl_f,*PT_gy_chgchl_f;

INT32 mjd_to_tm(UINT32 mjd, struct tm *tm);
gycas_info_priv_t* gycas_get_priv(void);	

#define GYCA_EMAIL_MAX_CNT      50
void gyca_check_writemail_to_flash(void);
UINT8 gyca_readmail_one_from_ram(UINT8 index, gycamail *data);
UINT8 gyca_setmail_one_oldstatus_ram(UINT8 index);
UINT8 gyca_delmail_all_from_ram(void);
UINT8 gyca_delmail_one_from_ram(UINT8 index);
UINT8 gyca_addmail_one_to_ram(T_CAMailInfo *data);
UINT8 gyca_mail_init(void);
UINT8 gyca_getmail_condition_from_ram(UINT8 *m_total, UINT8 *m_new);
void gyca_setmail_status(mail_status status);
mail_status gyca_getmail_status(void);
void gyca_check_new_mail(void);

#endif


