#ifndef _CD_NVM_H
#define _CD_NVM_H


//#define DEBUG_CAS_NVM
//#define ERROR_CAS_NVM
#ifdef DEBUG_CAS_NVM
#define CAS_NVM_PRINTF	 libc_printf
#define CAS_NVM_DUMP(data,len) { const int l=(len); int i;\
                         for(i=0 ; i<l ; i++) CAS_NVM_PRINTF(" %02x",*((data)+i)); \
                         CAS_NVM_PRINTF("\n"); }
#else
#define CAS_NVM_PRINTF(...)	do{}while(0)
#define CAS_NVM_DUMP(data,len) ;
#endif
#ifdef ERROR_CAS_NVM
#define CAS_NVMERR_PRINTF	libc_printf
#define CAS_NVMERR_DUMP(data,len) { const int l=(len); int i;\
                         for(i=0 ; i<l ; i++) CAS_NVMERR_PRINTF(" %02x",*((data)+i)); \
                         CAS_NVMERR_PRINTF("\n"); }
#else
#define CAS_NVMERR_PRINTF(...)	do{}while(0)
#define CAS_NVMERR_DUMP(data,len) ;
#endif

#define CAS_FLASHWRITE_INTERVAL 2000
#define CAS_CHUNK_ID	0x08F70100
#define CAS_CHUNK_SIZE	0x10000

#endif
