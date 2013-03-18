#ifndef GYCA_SC_H
#define GYCA_SC_H

/*****************************Print Info Control*******************************************/
#if 0
#define GYCASDRV_PRINTF	libc_printf
#else
#define GYCASDRV_PRINTF(param...)	do{}while(0)
#endif

#define GYCASDRV_DUMP(data,len) { const int l=(len); int i;\
                         for(i=0 ; i<l ; i++) \
                         {\
						 	GYCASDRV_PRINTF("0x%02x,",*((data)+i)); \
						 	if((i&0x0f) == 0x0f) GYCASDRV_PRINTF("\n");\
                         }\
                         GYCASDRV_PRINTF("\n"); }
/*****************************Print Info Control End*******************************************/

#define CAS_SC_RETRY_TIMES		2 
#endif //end: GYCA_SC_H 
