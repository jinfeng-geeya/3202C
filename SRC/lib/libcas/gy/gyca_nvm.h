#ifndef GY_NVM_H
#define GY_NVM_H

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

#define SECTOR_SIZE 64*1024
#define CAS_FLASHWRITE_INTERVAL 2000
#define CAS_CHUNK_ID	0x08F70100
#define CAS_CHUNK_SIZE	0x10000
#define CAS_FLASH_MAXSIZE	(1024*64)//(1024*32) //DVT defines the maxsize of the flash

#endif //end: GY_NVM_H 

