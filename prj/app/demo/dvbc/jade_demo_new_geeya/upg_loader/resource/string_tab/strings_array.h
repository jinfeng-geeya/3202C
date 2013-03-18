
#include "str_English.h"
#include "str_s_chinese.h"

const char* strings_infor[]=
{
	English_strs_array,
	S_Chinese_strs_array,
};

const LANG_TABLE g_LangRscMapTable[] __attribute__((section(".flash")))=
{
	{ENGLISH_ENV, 	LIB_STRING_ENGLISH, LIB_CHAR_ASCII,		LIB_FONT_DEFAULT},
	{CHINESE_ENV, 	LIB_STRING_CHINESE, LIB_CHAR_ASCII,		LIB_FONT_DEFAULT},
};

#define	SIZE_LANG_TABLE	sizeof(g_LangRscMapTable)/sizeof(LANG_TABLE)

