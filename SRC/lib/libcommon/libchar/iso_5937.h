#ifndef __ISO_5937_H__
#define __ISO_5937_H__

#include <types.h>
#include <sys_config.h>

/*
 * name		: iso5937_convert_ch
 * description	: convert the first character from iso5937 to unicode.
 * parameter	: 3
 *	IN	UINT8 *iso5937: the ISO5937 string.
 *	IN	INT32 length: the ISO5937 string length.
 *	OUT	UINT16 *unicode: the generated unicode.
 * return value : INT32
 *	1	: 1 iso5937 character is detected, and stored in 1 unicode.
 *	2	: 2 iso5937 combined character is detected and stored in 1 unicode.
 *	other	: the iso5937 character is invalid, or should be skipped.
 */
INT32 iso6937_convert_ch(UINT8 *iso5937, INT32 length, UINT16 *unicode, INT32 maxlen);

#endif /* __ISO_5937_H__ */

