/*
 * Copyright (C) ALi Shanghai Corp. 2006
 */
#include <types.h>
#include <sys_config.h>
#include <retcode.h>

#include "char_map.h"

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
UINT16 u8u16_lookup(UINT16 key, struct u8u16 *table, INT32 count)
{
	UINT16 ret = 0xFFFF;
	INT32 mid, low=-1, high=count;
	if (table == NULL)
		return ret;
	do {
		mid = (low+high)>>1;
		if (table[mid].key>key) {
			high = mid;
		} else if (table[mid].key<key) {
			low = mid;
		} else {
			ret = table[mid].value;
			break;
		}
	}while(high-low>1);
	
	return ret;
}

/*
 * name		: reverse_lookup
 * description	: reverse search in u8u16 table.
 * parameter	: 3
 *	IN	UINT16 value : the u8 value.
 *	IN	struct u8u16 *table: the table.
 *	IN	INT32 count : the table count. 
 * return value : UINT16
 *	0xFFFF:	 no match.
 *	other :	 the key that matches the value.
 */
UINT16 reverse_lookup(UINT16 value, struct u8u16 *table, INT32 count)
{
	INT32 i;
	UINT16 ret = 0xFFFF;
	if (table == NULL)
		return ret;

	for(i=0; i<count; i++)
	{
		if(value == table[i].value)
		{
			ret = table[i].key;
			break; 
		}
	}

	return ret;
}

