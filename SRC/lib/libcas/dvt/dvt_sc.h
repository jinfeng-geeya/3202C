#ifndef DVT_SC_H
#define DVT_SC_H

/*****************************Print Info Control*******************************************/
//#define DEBUG_CAS_SC
#define ERROR_CAS_SC
#ifdef DEBUG_CAS_SC
#define CAS_SC_PRINTF	 libc_printf
#define CAS_SC_DUMP(data,len) { const int l=(len); int i;\
                         for(i=0 ; i<l ; i++) CAS_SC_PRINTF(" %02x",*((data)+i)); \
                         CAS_SC_PRINTF("\n"); }
#else
#define CAS_SC_PRINTF(...)	do{}while(0)
#define CAS_SC_DUMP(data,len) do{}while(0)
#endif
#ifdef ERROR_CAS_SC
#define CAS_SCERR_PRINTF	libc_printf
#define CAS_SCERR_DUMP(data,len) { const int l=(len); int i;\
                         for(i=0 ; i<l ; i++) CAS_SCERR_PRINTF(" %02x",*((data)+i)); \
                         CAS_SCERR_PRINTF("\n"); }
#else
#define CAS_SCERR_PRINTF(...)	do{}while(0)
#define CAS_SCERR_DUMP(data,len) do{}while(0)
#endif
/*****************************Print Info Control End*******************************************/

#define CAS_SC_RETRY_TIMES		2 
#endif //end: DVT_SC_H 
