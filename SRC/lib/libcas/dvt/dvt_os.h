#ifndef DVT_OS_H
#define DVT_OS_H


/*****************************Print Info Control*******************************************/
//#define ERROR_CAS_OS
#ifdef ERROR_CAS_OS
#define CAS_OSERR_PRINTF	libc_printf
#define CAS_OSERR_DUMP(data,len) { const int l=(len); int i;\
                         for(i=0 ; i<l ; i++) CAS_OSERR_PRINTF(" %02x",*((data)+i)); \
                         CAS_OSERR_PRINTF("\n"); }
#else
#define CAS_OSERR_PRINTF(...)	do{}while(0)
#define CAS_OSERR_DUMP(data,len) ;
#endif
/*****************************Print Info Control End*******************************************/

#define CAS_STACK_SIZE	(1024*4 + 1024)
#endif //end: DVT_OS_H 

