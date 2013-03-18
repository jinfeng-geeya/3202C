#ifndef _CD_OS_H
#define _CD_OS_H


//#define DEBUG_CAS_OS
//#define ERROR_CAS_OS
#ifdef DEBUG_CAS_OS
#define CAS_OS_PRINTF	 libc_printf
#define CAS_OS_DUMP(data,len) { const int l=(len); int i;\
                         for(i=0 ; i<l ; i++) CAS_OS_PRINTF(" %02x",*((data)+i)); \
                         CAS_OS_PRINTF("\n"); }
#else
#define CAS_OS_PRINTF(...)	do{}while(0)
#define CAS_OS_DUMP(data,len) ;
#endif
#ifdef ERROR_CAS_OS
#define CAS_OSERR_PRINTF	libc_printf
#define CAS_OSERR_DUMP(data,len) { const int l=(len); int i;\
                         for(i=0 ; i<l ; i++) CAS_OSERR_PRINTF(" %02x",*((data)+i)); \
                         CAS_OSERR_PRINTF("\n"); }
#else
#define CAS_OSERR_PRINTF(...)	do{}while(0)
#define CAS_OSERR_DUMP(data,len) ;
#endif


#endif
