#ifndef __IRCA_SYS_H__
#define __IRCA_SYS_H__
#include <os/tds2/alitypes.h>
#include <basic_types.h>
#include <api/libtsi/sie.h>
#include <irca/ca_task.h>

typedef enum
{
	CA_STATUS_E                    = 0x00000001,
	CA_EMMSVC_E                   = 0x00000002,
	CA_ECMSVC_E                  = 0x00000004,
	CA_NEMAIL_E					= 0x00000008,
	CA_FEMAIL_E					= 0x00000010,
	CA_BANNER_E					= 0x00000020,
	CA_HASHID_E					= 0x00000040,
	CA_HGPC_E					= 0x00000080,
	CA_PRODUCT_E				= 0x00000100,
	CA_NORMAIL_E				= 0x00000200,
	
}CA_INFO_EVENT_E;

typedef struct
{
	ia_char EcmString1[MAX_ECM_MONITOR_LEN];
	ia_char	EcmString2[MAX_ECM_MONITOR_LEN];
}CA_ECM_STRING;


typedef struct
{
	BYTE *pFPStrint;
	BYTE ShowTime; 		/*in second*/
}FP_SHOW_INFO;


void ap_cas_call_back(CA_INFO_EVENT_E event_type);
void IRCA_STA_EnableCAInfo( void **pInfoStr1, void **pInfoStr2, CA_INFO_EVENT_E EventType) ;/*请注意:第一和第二个参数的类型由EventType决定*/
void IRCA_STA_GetBanStatus(ia_status_st **pGstatus);
void IRCA_STA_GetFPShow( FP_SHOW_INFO **pstFPShow);

void IRCA_CurProgNo(UINT16 CurProgNo);
void IRCA_ParsePMT(UINT8 *PmtData, INT32 length, UINT16 param);
UINT8 IRCA_ParseCAT(UINT16 pid, struct si_filter_t *filter, UINT8 reason, UINT8 *CatData, INT32 length);
void IRCA_ParseNIT(UINT8 *NitBuf, INT32 length, UINT16 param);
void TP_Desc_AfterStopProg(void);
BOOL IRCA_Init( void );

BOOL IR_SC_GetCardIn(void);

#endif

