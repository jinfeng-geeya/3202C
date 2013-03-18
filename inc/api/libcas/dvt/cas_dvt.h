#ifndef _CAS_DVT_H
#define _CAS_DVT_H

#include "DVTCAS_STBDataPublic.h"
#define CAS_SUCCESS	0
#define CAS_FAIL		0xff
#define DVT_OSDMSG_LEN	(210+2) //210 is defined by DVT, 2 is reserved.

typedef enum
{
	CAS_MSG_SHOWPROMPTMSG = 1,
	CAS_MSG_HIDEPROMPTMSG,	
	CAS_MSG_FINGER_PRINT,
	CAS_MSG_LOCK_SERVICE,
	CAS_MSG_SHOW_OSD_MESSAGE,
	CAS_MSG_EMAIL_NOTIFY,
	CAS_MSG_START_IPPVDLG,
	CAS_MSG_SMC_OUT,
	CAS_MSG_SMC_IN,
	CAS_MSG_NOSIGNAL,
	CAS_MSG_UNLOCK_SERVICE,
	CAS_MSG_URGENT_BROADCAST,
}CAS_MSG_TYPE_T;

typedef void (*cas_disp_func_t)(CAS_MSG_TYPE_T type, unsigned long param);

typedef struct
{
	UINT16 network_id;
	UINT16 ts_id;
	UINT16 service_id;
	UINT16 duration;
}dvt_urgent_info;

typedef struct
{
	UINT16 state;
	OSAL_ID thread_id;
	OSAL_ID msgque_id;
	OSAL_ID mutex_id;
	cas_disp_func_t disp_callback;
	UINT32 finger_info[2]; //finger_info[0]: card_id, finger_info[1]: duration
	UINT16 dvt_osdbuf[DVT_OSDMSG_LEN];
	SDVTCAIpp ippinfo;
	dvt_urgent_info urgent;
} mcas_t;

#endif


