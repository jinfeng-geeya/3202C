#ifndef _CD_SC_H
#define _CD_SC_H

#define DEBUG_CAS_SC
#define ERROR_CAS_SC
#ifdef DEBUG_CAS_SC
#define CAS_SC_PRINTF	 libc_printf
#define CAS_SC_DUMP(data,len) { const int l=(len); int i;\
                         for(i=0 ; i<l ; i++) CAS_SC_PRINTF(" %02x",*((data)+i)); \
                         CAS_SC_PRINTF("\n"); }
#else
#define CAS_SC_PRINTF(...)	do{}while(0)
#define CAS_SC_DUMP(data,len) ;
#endif
#ifdef ERROR_CAS_SC
#define CAS_SCERR_PRINTF	libc_printf
#define CAS_SCERR_DUMP(data,len) { const int l=(len); int i;\
                         for(i=0 ; i<l ; i++) CAS_SCERR_PRINTF(" %02x",*((data)+i)); \
                         CAS_SCERR_PRINTF("\n"); }
#else
#define CAS_SCERR_PRINTF(...)	do{}while(0)
#define CAS_SCERR_DUMP(data,len) ;
#endif

#endif
