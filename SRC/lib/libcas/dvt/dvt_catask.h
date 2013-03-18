#ifndef DVT_CATASK_H
#define DVT_CATASK_H


/*****************************Print Info Control*******************************************/
#define DEBUG_CAS_CATSK
//#define ERROR_CAS_CATSK

#ifdef DEBUG_CAS_CATSK
#define CAS_CATSK_PRINTF	 libc_printf
#define CAS_CATSK_DUMP(data,len) { const int l=(len); int i;\
                         for(i=0 ; i<l ; i++) CAS_CATSK_PRINTF(" %02x",*((data)+i)); \
                         CAS_CATSK_PRINTF("\n"); }
#else
#define CAS_CATSK_PRINTF(...)	do{}while(0)
#define CAS_CATSK_DUMP(data,len) ;
#endif
#ifdef ERROR_CAS_CATSK
#define CAS_CATSKERR_PRINTF	libc_printf
#define CAS_CATSKERR_DUMP(data,len) { const int l=(len); int i;\
                         for(i=0 ; i<l ; i++) CAS_CATSKERR_PRINTF(" %02x",*((data)+i)); \
                         CAS_CATSKERR_PRINTF("\n"); }
#else
#define CAS_CATSKERR_PRINTF(...)	do{}while(0)
#define CAS_CATSKERR_DUMP(data,len) ;
#endif
/*****************************Print Info Control End*******************************************/

#define CAS_CHANNEL_INVALID	0xFF
#define CAS_PMT_SEC_LEN	4096
#define CAS_CAT_SEC_LEN	1024


enum  //monitor task msg
{
	CAS_MSG_SEC_PMT = 0,
	CAS_MSG_SEC_CAT,
	CAS_MSG_SMC_REMOVE,
	CAS_MSG_SMC_INSERT,
};

struct cas_msg
{
	UINT8 type;
	UINT32 param;
};
#endif //end: DVT_CATASK_H 

