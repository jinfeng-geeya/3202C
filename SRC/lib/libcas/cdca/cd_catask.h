#ifndef _CD_CATASK_H
#define _CD_CATASK_H


//#define DEBUG_CAS
//#define ERROR_CAS
#ifdef DEBUG_CAS
#define CAS_DEBUG_PRINTF	 libc_printf
#define CAS_DEBUG_DUMP(data,len) { const int l=(len); int i;\
                         for(i=0 ; i<l ; i++) CAS_DEBUG_PRINTF(" %02x",*((data)+i)); \
                         TFCAS_DEBUG_PRINTF("\n"); }
#else
#define CAS_DEBUG_PRINTF(...)	do{}while(0)
#define CAS_DEBUG_DUMP(data,len) ;
#endif
#ifdef ERROR_CAS
#define CAS_ERROR_PRINTF	libc_printf
#define OTHER_PRINTF	libc_printf 
#define CAS_ERROR_DUMP(data,len) { const int l=(len); int i;\
                         for(i=0 ; i<l ; i++) CAS_ERROR_PRINTF(" %02x",*((data)+i)); \
                         CAS_ERROR_PRINTF("\n"); }
#else
#define CAS_ERROR_PRINTF(...)	do{}while(0)
#define OTHER_PRINTF(...)	do{}while(0)
#define CAS_ERROR_DUMP(data,len) ;
#endif


struct ecm_pid
{
	UINT16 service_id;
	UINT16 uniform_ecm_pid;
	UINT16 video_ecm_pid;
	UINT16 audio_ecm_pid;
	UINT16 ttx_ecm_pid;
	UINT16 subt_ecm_pid;
};

extern struct ecm_pid cas_ecm_pid;


//#define CAS_WITHOUT_UI


#endif

