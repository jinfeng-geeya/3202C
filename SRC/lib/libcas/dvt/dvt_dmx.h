#ifndef DVT_DMX_H
#define DVT_DMX_H


/*****************************Print Info Control*******************************************/
#define DEBUG_CAS_DMX
#define ERROR_CAS_DMX
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
/*****************************Print Info Control End*******************************************/

//#define INVALID_PID	0x1FFF
#define CAS_INVALID_REQ_ID	0
#define CAS_MAX_CHAN_NUM	3
#define CAS_MAX_SEC_SIZE	4096

#define DVT_ECM_REQUEST_ID	DVTCA_STB_ECM_SLOT //which request id is used for cas lib
#define DVT_EMM_REQUEST_ID	2 //which request id is used for cas lib

#define DVT_ECM_CHANNEL_ID	0//which cas channel is used for ECM

struct cas_channel_status
{
	UINT8 enable;
	UINT32 tick;
	UINT32 timeout;
};
struct cas_channel
{
	struct get_section_param get_sec_param;
	struct restrict mask_value;
	struct cas_channel_status status[CAS_MAX_CHAN_NUM];
	UINT8 req_id[MAX_MV_NUM];
	UINT8 flt_id;
	UINT8 sec_buf[CAS_MAX_SEC_SIZE];
};

struct ecm_pid
{
	UINT16 service_id;
	//UINT16 uniform_ecm_pid;
	INT32 uniform_ecm_pid;
	UINT16 video_ecm_pid;
	UINT16 audio_ecm_pid;
};

#endif //end: DVT_DMX_H 

