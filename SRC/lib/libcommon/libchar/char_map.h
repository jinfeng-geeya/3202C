/*
 * Copyright (C) ALi Shanghai Corp. 2006
 */
#ifndef __CHAR_MAP_H__
#define __CHAR_MAP_H__

#include <types.h>
#include <sys_config.h>

struct u8u16 {
	UINT16 key;
	UINT16 value;
};

struct lookup_table {
	struct u8u16 *table;
	UINT8 count;
};

/*
 * name		: u8u16_lookup
 * description	: standard procedure of binary search in u8u16 table.
 * parameter	: 3
 *	IN	UINT8 key : the u8 value.
 *	IN	struct u8u16 *table: the table.
 *	IN	INT32 count : the table count. 
 * return value : UINT16
 *	0xFFFF:	 no match.
 *	other :	 the value that matches the key.
 */
UINT16 u8u16_lookup(UINT16 key, struct u8u16 *table, INT32 count);

static inline UINT16 cpu_to_be16(UINT16 x)
{
#if (SYS_CPU_ENDIAN == ENDIAN_LITTLE)	
	return (x>>8)|((x&0xFF)<<8);
#elif (SYS_CPU_ENDIAN == ENDIAN_BIG)
	return x;
#else
#error "please check SYS_CPU_ENDIAN in <sys_config.h>"
#endif
}

static inline UINT16 cpu_to_le16(UINT16 x)
{
#if (SYS_CPU_ENDIAN == ENDIAN_LITTLE)
	return x;
#elif (SYS_CPU_ENDIAN == ENDIAN_BIG)
	return (x>>8)|((x&0xFF)<<8);
#endif
}

#define le16_to_cpu(x)			cpu_to_le16(x)
#define be16_to_cpu(x)			cpu_to_be16(x)

/*
 * redirection Marco
 */
#if ((SYS_CHIP_MODULE == ALI_M3327C) && (SYS_PROJECT_FE == PROJECT_FE_DVBT) && (SYS_SDRAM_SIZE == 2))
#define U16_TO_UNICODE(x)	cpu_to_le16(x)
#else
#define U16_TO_UNICODE(x)	cpu_to_be16(x)
#endif

#endif /* __CHAR_MAP_H__ */

