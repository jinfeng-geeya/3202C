#ifndef __LIB_CHAR_H__
#define __LIB_CHAR_H__

// forward char: NON, CONS, LV, FV1, FV2, FV3
// dead char: CTRL, BV1, BV2, BD, TONE, AD1, AD2, AD3, AV1, AV2, AV3
struct thai_cell
{
	UINT16 char_num;	// 0~3
	UINT16 unicode[3];	// unicode[0]: forward char; unicode[1]: dead char; unicode[2]: dead char
};

/**
 * name		: get_iso639_refcode
 * description	: return reference code of ISO8859/ISO6937 according to iso639 code.
 * parameter	: 1
 *	IN	UINT8 *iso639:	the ISO-639 string, could be 2 or 3 letters.
 * return value	: UINT32
 *	0	: no reference code.
 *	1~15	: the ISO-8859 code page value.
 *	6937	: the ISO-6937 reference code.
 *	other	: reserved.
 */
UINT32 get_iso639_refcode(UINT8 *iso639);

/**
 * name		: dvb_to_unicode
 * description	: Convert DVB SI code to unicode, ignore what it doesn't support.
 * parameter	: 4
 *	IN	UINT8 *dvb:	the DVB si code string.
 *	IN	INT32 length:	the DVB si code length.
 *	OUT	UINT16 *unicode: generated unicode string.
 *	IN	INT32 maxlen:	the unicode maximum length.
 * return value : INT32
 *	the length of the converted string!
 */

INT32 dvb_to_unicode(UINT8 *dvb, INT32 length, UINT16 *unicode, INT32 maxlen, UINT32 refcode);

/*
 * compare 2 iso639 language codes
 * example:
 *		"ENG" == "eng",	not care small letter and capital letter;
 *		"deu" == "ger",	some language may have multi-abbreviatives.
 */
INT32 compare_iso639_lang(const UINT8 *iso639_1, const UINT8 *iso639_2);


#endif /* __LIB_CHAR_H__ */
