#ifndef _CD_DMX_H
#define _CD_DMX_H

//#define DEBUG_CAS_DMX
//#define ERROR_CAS_DMX
#ifdef DEBUG_CAS_DMX
#define CAS_DMX_PRINTF	 libc_printf
#define CAS_DMX_DUMP(data,len) { const int l=(len); int i;\
                         for(i=0 ; i<l ; i++) CAS_DMX_PRINTF(" %02x",*((data)+i)); \
                         CAS_DMX_PRINTF("\n"); }
#else
#define CAS_DMX_PRINTF(...)	do{}while(0)
#define CAS_DMX_DUMP(data,len) ;
#endif
#ifdef ERROR_CAS_DMX
#define CAS_DMXERR_PRINTF	libc_printf
#define CAS_DMXERR_DUMP(data,len) { const int l=(len); int i;\
                         for(i=0 ; i<l ; i++) CAS_DMXERR_PRINTF(" %02x",*((data)+i)); \
                         CAS_DMXERR_PRINTF("\n"); }
#else
#define CAS_DMXERR_PRINTF(...)	do{}while(0)
#define CAS_DMXERR_DUMP(data,len) ;
#endif

#define CAS_INVALID_REQ_ID	0
#define CAS_MAX_CHAN_NUM	3
#define CAS_MAX_SEC_SIZE	4096

#define CAS_DMX_ECM_REQUESTID		1
#define CAS_DMX_EMM_REQUESTID		2

//#define INVALID_PID	0x1FFF

struct cas_channel
{
	struct get_section_param get_sec_param;
	struct restrict mask_value;
	UINT8 flt_id;
	UINT8 req_id[MAX_MV_NUM];
	UINT8 sec_buf[CAS_MAX_SEC_SIZE];
};


#endif

