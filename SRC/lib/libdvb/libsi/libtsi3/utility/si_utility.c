#include <types.h>
#include <sys_config.h>
#include <api/libsi/si_utility.h>

#if (SI_DEBUG_LEVEL>0)
#include <api/libc/printf.h>
#define SI_PRINTF				libc_printf
#else
#define SI_PRINTF(...)			do{}while(0)
#endif
/**
 * function bcd2int related enum values.
 */
enum {
 	BCD_WIDTH				= 4,
	BCD_POWER				= 10,
	BCD_MASK				= 0x0F,
};

/**
 * bcd2int
 *
 * convert bcd code to interger.
 */
UINT32 bcd2integer(UINT8 *ptr, UINT8 digits, INT32 offset)
{
	UINT32 ret=0;
	INT32 i;
	UINT8 bcd;
	UINT8 offt;
	for(i=0; i<digits; i++) {
		offt = i+offset;
		bcd = ptr[offt>>1];
		bcd = (offt&0x01)? bcd&BCD_MASK: bcd>>BCD_WIDTH;
		ret = ret*BCD_POWER+bcd;
	}
	return ret;
}


#if (SI_DEBUG_LEVEL>0)
void DUMP_SECTION(UINT8 *data, INT32 len, INT32 level)
{
	if ((data!=NULL)&&(len>=8)) {
		SI_PRINTF("[%02x][%02x%02x][%d/%d]\n", 
			data[0], data[1]&0x0f, data[2], data[6], data[7]);

		if (level>0) {
			for(i=0; i<len; i++) {
				SI_PRINTF("%02x ", data[i]);
				if ((0x0f&i) == 0x0f) {
					SI_PRINTF("\n");
				}
			}
			SI_PRINTF("\n");
		}
	}
}
#endif

