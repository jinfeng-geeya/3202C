#ifndef GYCA_OS_H
#define GYCA_OS_H


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

#define CAS_STACK_SIZE	(1024*4 + 1024)
#endif //end:GYCA_OS_H 

