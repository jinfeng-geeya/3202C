#ifndef __GB_2312_H__
#define __GB_2312_H__

#ifdef GB2312_SUPPORT 
/*
 * name		: convert_gb2312_to_unicode
 * description	: convert the string from gb2312 to unicode.
 * parameter	: 4
 *	IN	UINT8 *gb2312: the gb2312 string.
 *	IN	INT32 length: the gb2312 string length.
 *	OUT	UINT16 *unicode: the generated unicode.
 *	IN	INT32 maxlen: the unicode string maxiam length.
 * return value : INT32
 *	SUCCESS	:	gb2312 string is converted, and stored in unicode.
 *	ERR_FAILUE:	
 */
INT32 convert_gb2312_to_unicode(UINT8 *gb2312, INT32 length, UINT16 *unicode, INT32 maxlen);
#endif

#endif /* __GB_2312_H__ */
