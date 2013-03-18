/*
 * Copyright (C) ALi Shanghai Corp. 2005
 *
 * File name	: lib_char.c
 * Description	: a DVB stream charset conversion to unicode utility.
 * History	:
 *    Date		Version		Author		Comment
 *   +-----------------+---------------+---------------+------------------------
 *  1.20050410		0.1		Zhengdao Li	Initialize.
 *  2.20050423		0.2		Zhengdao Li	Preliminary support for Chinese.
 *							Support more ISO8859 charset.
 *  3.20060112		0.3		Zhengdao Li	Merge 2M solution
 *  4.20060705		0.3.001		Zhengdao Li	Polish language patch.
 */
#include <types.h>
#include <sys_config.h>
#include <retcode.h>

#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libsi/si_config.h>
#include <api/libchar/lib_char.h>

#include "char_map.h"
#include "iso_5937.h"

extern void arabic_process(unsigned short *str, unsigned int size);
extern void hebrew_punctuation_process(unsigned short *str, unsigned int size);
static void Convert_UniStr_Pos(UINT16 * str);
static INT32 _iso8859_to_unicode(const UINT8 *dvb, INT32 length, UINT16 *unicode, INT32 maxlen, UINT32 code);
static INT32 _vbi_iso8859_to_unicode(const UINT8 *dvb, INT32 length, UINT16 *unicode, INT32 maxlen, UINT32 code);
static INT32 _big5_to_unicode(const UINT8 *dvb, INT32 length, UINT16 *unicode, INT32 maxlen);
static INT32 _utf8_to_unicode(const UINT8* src, INT32 srcLen, UINT16* dst, INT32 maxlen);
static INT32 _dvb_to_unicode(UINT8 *dvb, INT32 length, UINT16 *unicode, INT32 maxlen, UINT32 refcode, UINT8 vbi);

#if defined(GB2312_SUPPORT)
#include "gb_2312.h"
#endif

#if defined(ISO8859_SUPPORT)
#define ISO_8859_2_SUPPORT
#define ISO_8859_3_SUPPORT
#define ISO_8859_4_SUPPORT
#define ISO_8859_5_SUPPORT
#define ISO_8859_6_SUPPORT
#define ISO_8859_7_SUPPORT
#define ISO_8859_8_SUPPORT
#define ISO_8859_9_SUPPORT
#define ISO_8859_10_SUPPORT
#define ISO_8859_11_SUPPORT
#define ISO_8859_13_SUPPORT
#define ISO_8859_14_SUPPORT
#define ISO_8859_15_SUPPORT
#define ISO_8859_16_SUPPORT

#include "iso_8859_2.h"
#include "iso_8859_3.h"
#include "iso_8859_4.h"
//#include "iso_8859_5.h"
#include "iso_8859_6.h"
#include "iso_8859_7.h"
#include "iso_8859_8.h"
#include "iso_8859_9.h"
#include "iso_8859_10.h"
#include "iso_8859_11.h"
//#include "iso_8859_12.h"
#include "iso_8859_13.h"
#include "iso_8859_14.h"
#include "iso_8859_15.h"
#include "iso_8859_16.h"

static UINT16 *iso8859_convert_map[] = {
	NULL,
	NULL, /* same as ASCII code map */
	iso8859_2_map_A0_FF,
	iso8859_3_map_A0_FF,
	iso8859_4_map_A0_FF,
	NULL, /* use code to lookup since it has some rules to follow */
	iso8859_6_map_A0_FF,
	iso8859_7_map_A0_FF,
	iso8859_8_map_A0_FF,
	iso8859_9_map_A0_FF,
	iso8859_10_map_A0_FF,
	iso8859_11_map_A0_FF,
	NULL, /* indian charset, have no clue of this. */
	iso8859_13_map_A0_FF,
	iso8859_14_map_A0_FF,
	iso8859_15_map_A0_FF,
	iso8859_16_map_A0_FF,
};
#endif

//<<<
enum ISO_639_LANG_ID
{
	ISO_639_None 	= 0,
	ISO_639_Arabic 	= 1,
	ISO_639_Chinese = 2,
	ISO_639_Czech 	= 3,
	ISO_639_Danish,
	ISO_639_German,
	ISO_639_Dutch,
	ISO_639_Greek,
	ISO_639_English,
	ISO_639_Spanish,
	ISO_639_Estonian,
	ISO_639_Finnish,
	ISO_639_French,
	ISO_639_Irish,
	ISO_639_Hebrew,
	ISO_639_Hungarian,
	ISO_639_Icelandic,
	ISO_639_Italian,
	ISO_639_Kurdish,
	ISO_639_Norwegian,
	ISO_639_Persian,
	ISO_639_Polish,
	ISO_639_Portuguese,
	ISO_639_Romanian,
	ISO_639_Russian,
	ISO_639_Scots,
	ISO_639_Slovak,
	ISO_639_Slovenian,
	ISO_639_Swedish,
	ISO_639_Turkish,
	ISO_639_Sorbian
};

struct ISO_639_LANG
{
	UINT8 lang[3];	//iso 639 language code, no matter small letter and capital letter
	UINT16 id;		//unique id, see enum ISO_639_LANG_ID
};

//infact, only multi-abbreviatives of the language needs this table.
static const struct ISO_639_LANG iso639_lang[] = 
{
//{"ang", ISO_639_English, Old (ca. 450-1100)           },	//English, Old (ca. 450-1100) 
{"ara", ISO_639_Arabic                                },	//Arabic 
{"chi", ISO_639_Chinese                               },	//Chinese 
{"cze", ISO_639_Czech                                 },	//Czech 
{"dan", ISO_639_Danish                                },	//Danish 
{"deu", ISO_639_German                                },	//German 
//{"dum", ISO_639_Dutch, Middle (ca. 1050-1350)         },	//Dutch, Middle (ca. 1050-1350) 
{"dut", ISO_639_Dutch                                 },	//Dutch 
{"ell", ISO_639_Greek                                 },	//Greek, Modern (1453-) 
{"eng", ISO_639_English                               },	//English 
//{"enm", ISO_639_English, Middle (ca. 1100-1500)       },	//English, Middle (ca. 1100-1500) 
{"esl", ISO_639_Spanish                               },	//Spanish 
{"est", ISO_639_Estonian                              },	//Estonian 
{"fas", ISO_639_Persian								  },	//Persian 
{"fin", ISO_639_Finnish                               },	//Finnish 
{"fra", ISO_639_French                                },	//French 
{"fre", ISO_639_French                                },	//French 
//{"frm", ISO_639_French, Middle (ca. 1400-1600)        },	//French, Middle (ca. 1400-1600) 
//{"fro", ISO_639_French, Old (842- ca. 1400)           },	//French, Old (842- ca. 1400) 
{"gai", ISO_639_Irish                                 },	//Irish 
//{"gem", ISO_639_Germanic (Other)                      },	//Germanic (Other) 
{"ger", ISO_639_German                                },	//German 
//{"gmh", ISO_639_German, Middle High (ca. 1050-1500)   },	//German, Middle High (ca. 1050-1500) 
//{"goh", ISO_639_German, Old High (ca. 750-1050)       },	//German, Old High (ca. 750-1050) 
//{"grc", ISO_639_Greek, Ancient (to 1453)              },	//Greek, Ancient (to 1453) 
{"gre", ISO_639_Greek                                 },	//Greek, Modern (1453-) 
{"heb", ISO_639_Hebrew                                },	//Hebrew 
{"hun", ISO_639_Hungarian                             },	//Hungarian 
{"ice", ISO_639_Icelandic                             },	//Icelandic 
{"iri", ISO_639_Irish                                 },	//Irish 
{"isl", ISO_639_Icelandic                             },	//Icelandic 
{"ita", ISO_639_Italian                               },	//Italian 
//{"jpn", ISO_639_Japanese                              },	//Japanese 
//{"kor", ISO_639_Korean                                },	//Korean 
{"kur", ISO_639_Kurdish								  },	//Kurdish 
//{"mga", ISO_639_Irish, Middle (900 - 1200)            },	//Irish, Middle (900 - 1200) 
{"nla", ISO_639_Dutch                                 },	//Dutch 
{"nno", ISO_639_Norwegian                             },	//Norwegian (Nynorsk) 
{"nor", ISO_639_Norwegian                             },	//Norwegian 
//{"ota", ISO_639_Turkish, Ottoman (1500 - 1928)        },	//Turkish, Ottoman (1500 - 1928) 
{"per", ISO_639_Persian                               },	//Persian
{"pol", ISO_639_Polish                                },	//Polish 
{"por", ISO_639_Portuguese                            },	//Portuguese 
{"ron", ISO_639_Romanian                              },	//Romanian 
{"rum", ISO_639_Romanian                              },	//Romanian 
{"rus", ISO_639_Russian                               },	//Russian 
{"sco", ISO_639_Scots                                 },	//Scots 
//{"sga", ISO_639_Irish, Old (to 900)                   },	//Irish, Old (to 900) 
//{"sla", ISO_639_Slavic (Other)                        },	//Slavic (Other) 
{"slk", ISO_639_Slovak                                },	//Slovak 
{"slo", ISO_639_Slovak                                },	//Slovak 
{"slv", ISO_639_Slovenian                             },	//Slovenian 
{"spa", ISO_639_Spanish                               },	//Spanish 
{"sve", ISO_639_Swedish                               },	//Swedish 
{"swe", ISO_639_Swedish                               },	//Swedish 
{"tur", ISO_639_Turkish                               },	//Turkish 
{"wen", ISO_639_Sorbian                               },	//Sorbian Languages 
{"zho", ISO_639_Chinese                               },	//Chinese 
{"ira", ISO_639_Persian                               },	//Iranian
};
static const INT32 iso639_lang_cnt = sizeof(iso639_lang)/sizeof(iso639_lang[0]);



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

UINT32 get_iso639_refcode(UINT8 *iso639)
{
	UINT32 refcode = 15;/*1;, 8859-1 can be replaced by 8859-15*/

	if (iso639 == NULL) {
		refcode = 0;
	} else if ((MEMCMP(iso639, "slk", 3)==0) || (MEMCMP(iso639, "SLK", 3)==0) \
				||(MEMCMP(iso639, "pol", 3)==0) || (MEMCMP(iso639, "POL", 3)==0) \
				||(MEMCMP(iso639, "slo", 3)==0) || (MEMCMP(iso639, "SLO", 3)==0)){
		refcode = 0x02; // ISO/IEC 8859-2 
	} else if ((MEMCMP(iso639, "rus", 3)==0) || (MEMCMP(iso639, "RUS", 3)==0)\
				||(MEMCMP(iso639, "wen", 3)==0) || (MEMCMP(iso639, "WEN", 3)==0)) {
		refcode = 0x05; // ISO/IEC 8859-5 
	} else if ((MEMCMP(iso639, "ara", 3)==0) || (MEMCMP(iso639, "ARA", 3)==0) \
				||(MEMCMP(iso639, "per", 3)==0) || (MEMCMP(iso639, "PER", 3)==0) \
				||(MEMCMP(iso639, "fas", 3)==0) || (MEMCMP(iso639, "FAS", 3)==0) \
				||(MEMCMP(iso639, "ira", 3)==0) || (MEMCMP(iso639, "IRA", 3)==0)) {
		refcode = 0x06; // ISO/IEC 8859-6 Latin/Arabic alphabet
	} else if ((MEMCMP(iso639, "gre", 3)==0) || (MEMCMP(iso639, "GRE", 3)==0)) {
		refcode = 0x07; // ISO/IEC 8859-7 Latin/Greek alphabet
	} else if ((MEMCMP(iso639, "heb", 3)==0) || (MEMCMP(iso639, "HEB", 3)==0)) {
		refcode = 0x08; // ISO/IEC 8859-8 Latin/Hebrew alphabet
	} else if ((MEMCMP(iso639, "tur", 3)==0) || (MEMCMP(iso639, "TUR", 3)==0)\
				||(MEMCMP(iso639, "kur", 3)==0) || (MEMCMP(iso639, "KUR", 3)==0)) {
		refcode = 0x09; // ISO/IEC 8859-9 
	} else if ((MEMCMP(iso639, "dan", 3)==0) || (MEMCMP(iso639, "DAN", 3)==0) \
				||(MEMCMP(iso639, "ice", 3)==0) || (MEMCMP(iso639, "ICE",3)==0) \
				||(MEMCMP(iso639, "isl", 3)==0) || (MEMCMP(iso639, "ISL",3)==0) \
				||(MEMCMP(iso639, "nor", 3)==0) || (MEMCMP(iso639, "NOR",3)==0) \
				||(MEMCMP(iso639, "nno", 3)==0) || (MEMCMP(iso639, "NNO",3)==0) \
				||(MEMCMP(iso639, "sve", 3)==0) || (MEMCMP(iso639, "SVE", 3)==0) \
				||(MEMCMP(iso639, "swe", 3)==0) || (MEMCMP(iso639, "SWE", 3)==0)){
		refcode = 0x0a; // ISO/IEC 8859-10
	} else if ((MEMCMP(iso639, "fin", 3)==0) || (MEMCMP(iso639, "FIN", 3)==0) \
				||(MEMCMP(iso639, "est", 3)==0) || (MEMCMP(iso639, "EST",3)==0)){
		refcode = 0x0d; // ISO/IEC 8859-13 
	} else if ((MEMCMP(iso639, "por", 3)==0) || (MEMCMP(iso639, "POR",3)==0) \
				||(MEMCMP(iso639, "spa", 3)==0) || (MEMCMP(iso639, "SPA",3)==0) \
				||(MEMCMP(iso639, "esl", 3)==0) || (MEMCMP(iso639, "ESL",3)==0) \
				||(MEMCMP(iso639, "fra", 3)==0) || (MEMCMP(iso639, "fre",3)==0) \
				||(MEMCMP(iso639, "ger", 3)==0) || (MEMCMP(iso639, "GER",3)==0) \
				||(MEMCMP(iso639, "sco", 3)==0) || (MEMCMP(iso639, "SCO",3)==0) \
				||(MEMCMP(iso639, "iri", 3)==0) || (MEMCMP(iso639, "IRI",3)==0) \
				||(MEMCMP(iso639, "dan", 3)==0) || (MEMCMP(iso639, "DAN",3)==0) \
				||(MEMCMP(iso639, "dut", 3)==0) || (MEMCMP(iso639, "DUT",3)==0) \
				||(MEMCMP(iso639, "nla", 3)==0) || (MEMCMP(iso639, "NLA",3)==0) \
				||(MEMCMP(iso639, "deu", 3)==0) || (MEMCMP(iso639, "DEU",3)==0)	) {
		refcode = 0x0f; // ISO/IEC 8859-15 
	} else if ((MEMCMP(iso639, "hun", 3)==0) || (MEMCMP(iso639, "HUN", 3)==0) \
				||(MEMCMP(iso639, "ron", 3)==0) || (MEMCMP(iso639, "RON", 3)==0) \
				||(MEMCMP(iso639, "rum", 3)==0) || (MEMCMP(iso639, "RUM", 3)==0) \
				||(MEMCMP(iso639, "ita", 3)==0) || (MEMCMP(iso639, "ITA", 3)==0) \
				||(MEMCMP(iso639, "slv", 3)==0) || (MEMCMP(iso639, "SLV", 3)==0)) {
		refcode = 0x10; // ISO/IEC 8859-16 
#if defined( GB2312_SUPPORT ) || defined( CHINESE_SUPPORT ) 
	} else if ((MEMCMP(iso639, "chi", 3)==0) || (MEMCMP(iso639, "zho", 3)==0)) {
	#ifdef CHINESE_SUPPORT
		refcode = 0x15; // for utf-8 chinese
	#else
		refcode = 0x13; // GB2312
	#endif
#endif
	} else if ((MEMCMP(iso639, "cze", 3)==0) || (MEMCMP(iso639, "CZE", 3)==0)
				||(MEMCMP(iso639, "eng", 3)==0) || (MEMCMP(iso639, "wel", 3)==0)
				||(MEMCMP(iso639, "ces", 3)==0) || (MEMCMP(iso639, "ces", 3)==0)) {
		refcode = 6937;
	} 
	return refcode;
}

/**
 * name		: dvb_to_unicode
 * description	: Convert DVB SI code to unicode, ignore what it doesn't support.
 * parameter	: 4
 *	IN	UINT8 *dvb:	the DVB si code string.
 *	IN	INT32 length:	the DVB si code length.
 *	OUT	UINT16 *unicode: generated unicode string.
 *	IN	INT32 maxlen:	the unicode maximum length.
 * return value : INT32
 *	SUCCESS	: the DVB string is partially or wholy converted.
 *	other	: the DVB string can not be converted at all!
 */
INT32 dvb_to_unicode(UINT8 *dvb, INT32 length, UINT16 *unicode, INT32 maxlen, UINT32 refcode)
{
	return _dvb_to_unicode( dvb, length, unicode, maxlen, refcode, 0 );
}
static INT32 _dvb_to_unicode(UINT8 *dvb, INT32 length, UINT16 *unicode, INT32 maxlen, UINT32 refcode, UINT8 vbi)
{
	INT32 si =0, di = 0;
	UINT8 code;
	UINT16 code_8859 = 0;
	UINT16 tmp;

	if (length<1) 
	{
		*unicode = 0x0000;
		return 0;
	}
	code = dvb[0];	// first byte, select character code table. EN300 468: Annex A

	if(code>=0x20)	
	{/*use refcode or ISO-6937, because the first byte does not indicate character table */
		if( refcode >0 && refcode <= 0x10 )
		{
			si--;
			if( refcode==0x01 ) // use 8859-15 replace 8859-1
				refcode = 0x0F;
			 /* compatible with dvb spec */
			code = refcode >= 0x05 ? ( refcode - 0x04 ) : ( refcode + 0xb );
		}
		else if( refcode == 0 || refcode == 6937 )
		{/* refcode==0, when search programme */
			return iso6937_convert_ch(dvb, length, unicode, maxlen); // set ISO-6937 as default character coding table
		}
		else
		{ // code > 0x10, so : 0x15(unicode), 0x13 or 2312
			code = refcode;
#ifdef CHINESE_SUPPORT
			/*!
			 * Because some tradition chinese stream use UTF-8, but
			 * first byte not indicate about this, so we withdrawal one byte
			 */
			if (refcode == 0x15)
				si--;
#endif
#ifdef GB2312_SUPPORT
			if (refcode == 0x13 || refcode==2312 ) // gb2312
				return convert_gb2312_to_unicode(dvb, length, unicode, maxlen);
#endif
		}
	}
	else if(code == 0x0E ) // utf-8
    {
		code = 0x15;
	}
	else if (code==1 && refcode==2)
	{	/* some Polish stream give misleading code = 1 that incicate selecting Russian character table
		 * but infact that stream use ISO 6937 character set, so here will patch for Polish.
		 */
		return iso6937_convert_ch(dvb+1, length-1, unicode, maxlen); // set ISO-6937 as default character coding table
	}
	switch(code)
	{
		case 0x0d:	/*8859-2*/
		case 0x0e:	/*8859-3*/
		case 0x0f:	/*8859-4*/
			code -= 0x0b;
			si += 1;
#ifdef EPG_LATIN_SUBSETS_SUPPORT
			if( vbi == 1 )
			{
				di = _vbi_iso8859_to_unicode(dvb+si, length-si, unicode, maxlen, code);
			}
#endif
			if( vbi == 0 )
			{
				di = _iso8859_to_unicode(dvb+si, length-si, unicode, maxlen, code);
			}
			break;
		case 0x01:	/*8859-5*/
		case 0x02:	/*8859-6*/
		case 0x03:	/*8859-7*/
		case 0x04:	/*8859-8*/
		case 0x05:	/*8859-9*/		
		case 0x06:	/*8859-10*/
		case 0x07:	/*8859-11*/
		case 0x09:	/*8859-13*/
		case 0x0a:	/*8859-14*/
		case 0x0b:	/*8859-15*/
		case 0x0c:	/*8859-16*/
			code += 4;
			si += 1;
#ifdef EPG_LATIN_SUBSETS_SUPPORT
			if( vbi == 1 )
			{
				di = _vbi_iso8859_to_unicode(dvb+si, length-si, unicode, maxlen, code);
			}
#endif
			if( vbi == 0 )
			{
				di = _iso8859_to_unicode(dvb+si, length-si, unicode, maxlen, code);
			}
			break;

		case 0x10:	/* ISO/IEC 8859 */
			code_8859 = (dvb[1]<<8)|dvb[2];
			si += 3;
#ifdef EPG_LATIN_SUBSETS_SUPPORT
			if( vbi == 1 )
			{
				di = _vbi_iso8859_to_unicode(dvb+si, length-si, unicode, maxlen, code_8859);
			}
#endif
			if( vbi == 0 )
			{
				di = _iso8859_to_unicode(dvb+si, length-si, unicode, maxlen, code_8859);
			}
			break;
		
		case 0x11:  /* ISO-10646-1 Basic Multilingual Plane*/
			for(di=0, si=1; (si<length-1)&&(di<maxlen); si+=2)
		    {
		    	tmp = (UINT16)(dvb[si]<<8|dvb[si+1]); // maybe have bug here
				if(((tmp!=0)&&(tmp<0xE080))||(tmp>0xE09F))
					unicode[di++] = U16_TO_UNICODE(tmp);
				else if(tmp==0xE08A)
					unicode[di++] = U16_TO_UNICODE((UINT16)'\n');
			}
		    unicode[di] = 0x0000;
			break;
			
#ifdef GB2312_SUPPORT
		case 0x13:   /*GB-2312-1980*/
			di = convert_gb2312_to_unicode(&dvb[1], length-1, unicode, maxlen);
			break;
#endif

		case 0x14:	/*ISO/IEC 10646-1, Big5*/
			si += 1;
			di = _big5_to_unicode(dvb+si, length-si, unicode, maxlen);
			break;

		case 0x15:	//UTF-8 encode in single byte when 0x15
		    si += 1;		
			di = _utf8_to_unicode(dvb+si, length-si, unicode, maxlen);
    		break;

		default:
			*unicode = 0x0000;
			di = 0;
			break;
	}

	if(	code == 6 || code == 8 	 \
        || (code == 0x15 && refcode == 0x06 ) /* Arabic(Persian) with UTF8 encoding */	\
		|| (code == 10 && (code_8859 == 6 || code_8859 == 8)))	/*8859-6 for Arabic & 8859-8 for Hebrew*/
	{
        #if (SYS_CPU_ENDIAN == ENDIAN_LITTLE)
	        Convert_UniStr_Pos(unicode);
        #endif
        
		if (code == 8 || code_8859 == 8) // modify punctuation marks for Hebrew -> <> & () & [] & {}
		{
			hebrew_punctuation_process(unicode,di);
		}
		if( code == 0x15 && refcode == 0x06 )
			persian_process(unicode,di);
		else
			arabic_process(unicode,di);
        
        #if (SYS_CPU_ENDIAN == ENDIAN_LITTLE)
    		Convert_UniStr_Pos(unicode);
        #endif
		di = ComUniStrLen(unicode);
	}
		
	return di;
}



//-->20060316 by steven
//	2010-11-06 modified by Davy Wu
#ifdef EPG_LATIN_SUBSETS_SUPPORT

/*
 *  ETS 300 706 Table 36: Latin National Option Sub-sets
 *
 *  Latin G0 character code to Unicode mapping per national subset,
 *  unmodified codes (NO_SUBSET) in row zero.
 *
 *  [13][0] Turkish currency symbol not in Unicode, use private code U+E800
 */
static const UINT16
national_subset[14][13] = {
  { 0x0023u, 0x0024u, 0x0040u, 0x005Bu, 0x005Cu, 0x005Du, 0x005Eu, 0x005Fu, 0x0060u, 0x007Bu, 0x007Cu, 0x007Du, 0x007Eu },
  { 0x0023u, 0x016Fu, 0x010Du, 0x0165u, 0x017Eu, 0x00FDu, 0x00EDu, 0x0159u, 0x00E9u, 0x00E1u, 0x011Bu, 0x00FAu, 0x0161u },
  { 0x00A3u, 0x0024u, 0x0040u, 0x2190u, 0x00BDu, 0x2192u, 0x2191u, 0x0023u, 0x2014u, 0x00BCu, 0x2016u, 0x00BEu, 0x00F7u },
  { 0x0023u, 0x00F5u, 0x0160u, 0x00C4u, 0x00D6u, 0x017Du, 0x00DCu, 0x00D5u, 0x0161u, 0x00E4u, 0x00F6u, 0x017Eu, 0x00FCu },
  { 0x00E9u, 0x00EFu, 0x00E0u, 0x00EBu, 0x00EAu, 0x00F9u, 0x00EEu, 0x0023u, 0x00E8u, 0x00E2u, 0x00F4u, 0x00FBu, 0x00E7u },
  { 0x0023u, 0x0024u, 0x00A7u, 0x00C4u, 0x00D6u, 0x00DCu, 0x005Eu, 0x005Fu, 0x00B0u, 0x00E4u, 0x00F6u, 0x00FCu, 0x00DFu },
  { 0x00A3u, 0x0024u, 0x00E9u, 0x00B0u, 0x00E7u, 0x2192u, 0x2191u, 0x0023u, 0x00F9u, 0x00E0u, 0x00F2u, 0x00E8u, 0x00ECu },
  { 0x0023u, 0x0024u, 0x0160u, 0x0117u, 0x0229u, 0x017Du, 0x010Du, 0x016Bu, 0x0161u, 0x0105u, 0x0173u, 0x017Eu, 0x012Fu },
  { 0x0023u, 0x0144u, 0x0105u, 0x01B5u, 0x015Au, 0x0141u, 0x0107u, 0x00F3u, 0x0119u, 0x017Cu, 0x015Bu, 0x0142u, 0x017Au },
  { 0x00E7u, 0x0024u, 0x00A1u, 0x00E1u, 0x00E9u, 0x00EDu, 0x00F3u, 0x00FAu, 0x00BFu, 0x00FCu, 0x00F1u, 0x00E8u, 0x00E0u },
  { 0x0023u, 0x00A4u, 0x0162u, 0x00C2u, 0x015Eu, 0x01CDu, 0x00CDu, 0x0131u, 0x0163u, 0x00E2u, 0x015Fu, 0X01CEu, 0x00EEu },
  { 0x0023u, 0x00CBu, 0x010Cu, 0x0106u, 0x017Du, 0x00D0u, 0x0160u, 0x00EBu, 0x010Du, 0x0107u, 0x017Eu, 0x00F0u, 0x0161u },
  { 0x0023u, 0x00A4u, 0x00C9u, 0x00C4u, 0x00D6u, 0x00C5u, 0x00DCu, 0x005Fu, 0x00E9u, 0x00E4u, 0x00F6u, 0x00E5u, 0x00FCu },
  { 0xE800u, 0x011Fu, 0x0130u, 0x015Eu, 0x00D6u, 0x00C7u, 0x00DCu, 0x011Eu, 0x0131u, 0x015Fu, 0x00F6u, 0x00E7u, 0x00FCu }
};
static UINT8 _g_subset = 0; // subset that selected.

UINT8 get_latin_national_subset(UINT8 *lang)
{
	UINT32 i;
	struct 
	{
		UINT8 lang[3];
		UINT8 n;
	} lang_set[] = {
		{"ces", 1,},	/*Czech*/
		{"cze", 1,},	/*Czech*/
		{"slk", 1,},	/*Slovak*/
		{"slo", 1,},	/*Slovak*/
		{"eng", 2,},	/*English*/
		{"est", 3,},	/*Estonian*/
		{"fra", 4,},	/*French*/
		{"fre", 4,},	/*French*/
		{"deu", 5,},	/*German*/
		{"ger", 5,},	/*German*/
		{"ita", 6,},	/*Italian*/
		//{"???", 7,},	/*Lettish*/
		{"lit", 7,},	/*Lithuanian*/
		{"pol", 8,},	/*Polish*/
		{"por", 9,},	/*Portuguese*/
		{"esl", 9,},	/*Spanish*/
		{"spa", 9,},	/*Spanish*/
		{"ron", 10,},	/*Rumanian*/
		{"rum", 10,},	/*Rumanian*/
		//{"???", 11,},	/*Serbian*/
		{"scr", 11,},	/*Croatian*/
		{"slv", 11,},	/*Slovenian*/
		{"sve", 12,},	/*Swedish*/
		{"swe", 12,},	/*Swedish*/
		{"fin", 12,},	/*Finnish*/
		{"tur", 13,},	/*Turkish*/
	};

	if (lang == NULL)
		return 0;
	for (i=0;i<24;i++)
	{
		if (MEMCMP(lang_set[i].lang, lang, 3) == 0)
		{
			return lang_set[i].n;
		}
	}
	return 0;
}

/*
 * Translate national subset Teletext character code to Unicode.
 * n: national subset
 * c: character
 */
static UINT16 vbi_national_subset_teletext_unicode(UINT8 n, UINT8 c)
{
	INT32 i;
	/* shortcut */
	if( c>=0x20 && (0xF8000019UL & (1 << (c & 31))) )
	{
		if ( n>0 && n<14 )
		{
			for (i = 0; i < 13; i++)
			{
				if (c == national_subset[0][i])
					return national_subset[n][i];
			}
		}

		if( c == 0x24 )
			return 0x00A4u;
		else if (c == 0x7C)
			return 0x00A6u;
		else if (c == 0x7F)
			return 0x25A0u;
	}
	return U16_TO_UNICODE((UINT16)c);
}

INT32 dvb_vbi_to_unicode(UINT8 *dvb, INT32 length, UINT16 *unicode, INT32 maxlen, UINT32 refcode, UINT8 subset)
{
	_g_subset = subset;
	return _dvb_to_unicode( dvb, length, unicode, maxlen, refcode, 1 );
}


static INT32 _vbi_iso8859_to_unicode(const UINT8 *dvb, INT32 length, UINT16 *unicode, INT32 maxlen, UINT32 code)
{
	INT32 di=0, si=0;
	if(code==0x01) // ISO-8859-1 is ASCII
	{
		for (di=0;(si<length)&&(di<maxlen);si++)
		{
			if (dvb[si] == 0x8A)	//CR/LF
				unicode[di++] = U16_TO_UNICODE((UINT16)'\n');
			else
				unicode[di++] = vbi_national_subset_teletext_unicode(_g_subset, dvb[si]);
		}
	}
	else if(code==0x05) // ISO-8859-5
	{
		for(di=0;(si<length)&&(di<maxlen);si++) 
		{
			if (dvb[si]<0x80 && dvb[si]>=0x20)
				if(_g_subset>0)
				unicode[di++] = vbi_national_subset_teletext_unicode(_g_subset, dvb[si]);
			else if ((dvb[si]== 0xA0)||(dvb[si]==0xAD))
				unicode[di++] = U16_TO_UNICODE((UINT16)dvb[si]);
			else if (dvb[si]==0xF0)
				unicode[di++] = U16_TO_UNICODE(0x2116);
			else if (dvb[si]==0xFD)
				unicode[di++] = U16_TO_UNICODE(0x00A7);
			else if(dvb[si]>0x9F)
				unicode[di++] = U16_TO_UNICODE(dvb[si]+0x360);
			else if(dvb[si] == 0x8A)	//CR/LF
				unicode[di++] = U16_TO_UNICODE((UINT16)'\n');
		}
		unicode[di]=0x0000;
	}
	else if(code>=0x02 && code<=0x10) // ISO-8859-[2,3,4,6,..,11,13,14,15,16]
	{
		for(di=0;(si<length)&&(di<maxlen);si++) 
		{
			if (dvb[si]<0x80 && dvb[si]>=0x20)
				unicode[di++] = vbi_national_subset_teletext_unicode(_g_subset, dvb[si]);
			else if(dvb[si]>0x9F) 
				unicode[di++] = U16_TO_UNICODE(iso8859_convert_map[code][dvb[si]-0xA0]);
			else if(dvb[si] == 0x8A)
				unicode[di++] = U16_TO_UNICODE((UINT16)'\n');			
		}
		unicode[di]=0x0000;
	}
	return di;
}

#endif	//EPG_LATIN_SUBSETS_SUPPORT


static INT32 _iso8859_to_unicode(const UINT8 *dvb, INT32 length, UINT16 *unicode, INT32 maxlen, UINT32 code)
{
	INT32 di=0, si=0;
	if(code==0x01) // ISO-8859-1 is ASCII
	{
		for (di=0;(si<length)&&(di<maxlen);si++)
		{
			if (dvb[si] == 0x8A)	//CR/LF
				unicode[di++] = U16_TO_UNICODE((UINT16)'\n');
			else
				unicode[di++] = U16_TO_UNICODE((UINT16)dvb[si]);
		}
	}
	else if(code==0x05) // ISO-8859-5
	{
		for(di=0;(si<length)&&(di<maxlen);si++) 
		{
			if (dvb[si]<0x80 && dvb[si]>=0x20)
				unicode[di++] = U16_TO_UNICODE((UINT16)dvb[si]);
			else if ((dvb[si]== 0xA0)||(dvb[si]==0xAD))
				unicode[di++] = U16_TO_UNICODE((UINT16)dvb[si]);
			else if (dvb[si]==0xF0)
				unicode[di++] = U16_TO_UNICODE(0x2116);
			else if (dvb[si]==0xFD)
				unicode[di++] = U16_TO_UNICODE(0x00A7);
			else if(dvb[si]>0x9F)
				unicode[di++] = U16_TO_UNICODE(dvb[si]+0x360);
			else if(dvb[si] == 0x8A)	//CR/LF
				unicode[di++] = U16_TO_UNICODE((UINT16)'\n');
		}
		unicode[di]=0x0000;
	}
	else if(code>=0x02 && code<=0x10) // ISO-8859-[2,3,4,6,..,11,13,14,15,16]
	{
		for(di=0;(si<length)&&(di<maxlen);si++) 
		{
			if (dvb[si]<0x80 && dvb[si]>=0x20)
				unicode[di++] = U16_TO_UNICODE((UINT16)dvb[si]);
			else if(dvb[si]>0x9F) 
				unicode[di++] = U16_TO_UNICODE(iso8859_convert_map[code][dvb[si]-0xA0]);
			else if(dvb[si] == 0x8A)
				unicode[di++] = U16_TO_UNICODE((UINT16)'\n');			
		}
		unicode[di]=0x0000;
	}
	return di;
}

static INT32 _big5_to_unicode(const UINT8 *dvb, INT32 length, UINT16 *unicode, INT32 maxlen)
{
	INT32 di, si=0;
#if (SYS_PROJECT_FE == PROJECT_FE_DVBT) || (SYS_PROJECT_FE == PROJECT_FE_ISDBT)
	for(di=0; (si<length)&&(di<maxlen); si++) 
	{
		if ((dvb[si]==0xE0)&&(si<length-1)
			&&(dvb[si+1]>=0x80)&&(dvb[si+1]<=0x9F)) 
		{
			si++; /* control code, ignore them now. */
		}
		else if((dvb[si] == 0xff) && (dvb[si+1]<=0x5f))//temp use because we have no full-width font
		{
			unicode[di++] = (dvb[si+1] +0x20)<< 8;
			si++;
		}
		else
		{
			unicode[di++] = ((((UINT16) dvb[si+1]) << 8) |(UINT16) dvb[si]);
			si++;
		}
	}
#else
	for(di=0; (si<length)&&(di<maxlen); si++) 
	{
		if (dvb[si]<0x80)
			unicode[di++] = U16_TO_UNICODE((UINT16)dvb[si]);
		else if ((dvb[si]==0xE0)&&(si<length-1)
					&&(dvb[si+1]>=0x80)&&(dvb[si+1]<=0x9F)) 
		{
			si++; /* control code, ignore them now. */
		} 
		else 
		{
			si++; /* wait until we have the charset map. */
		}
	}
#endif
	unicode[di]=0x0000;
	return di;
}


// Count the number of bytes of a UTF-8 character
#define utf8_char_len(c) ((((int)0xE5000000 >> ((c >> 3) & 0x1E)) & 3) + 1)
static INT32 _utf8_to_unicode(const UINT8* src, INT32 srcLen, UINT16* dst, INT32 maxlen)
{
	INT32 srcIdx, dstIdx;
	UINT16 unicode;
	UINT8 i;
	srcIdx = dstIdx = 0;

	while( srcIdx<srcLen && dstIdx<(maxlen-1) )
	{
		if( (srcIdx+utf8_char_len(src[srcIdx])) > srcLen )
			break;
		for(i=0;i<8;i++)
		{
			if( (src[srcIdx]&(0x80>>i))==0 )
				break;
		}
		switch(i)
		{
			case 0: // ascii
				unicode = U16_TO_UNICODE((UINT16)src[srcIdx++]);
				break;
			case 1: // unknow character
				unicode = 0xFFFD;
				srcIdx += 1;
				break;
			case 2: // 2 byte
				if( (src[srcIdx+1]&0xC0)!=0x80 )
				{
					unicode = 0xFFFD;
					srcIdx += 1;
					break;
				}
				unicode = ((UINT16)(src[srcIdx]&0x1F))<<6;
				unicode = unicode | (UINT16)(src[srcIdx+1]&0x3F);
				unicode = U16_TO_UNICODE(unicode);
				srcIdx += 2;
				break;
			case 3: // 3 byte
				if( (src[srcIdx+1]&0xC0)!=0x80 || (src[srcIdx+2]&0xC0)!=0x80 )
				{
					unicode = 0xFFFD;
					srcIdx += 1;
					break;
				}
				unicode = ((UINT16)(src[srcIdx]&0x0F))<<12;
				unicode = unicode | ((UINT16)(src[srcIdx+1]&0x3F))<<6;
				unicode = unicode | (UINT16)(src[srcIdx+2]&0x3F);
				unicode = U16_TO_UNICODE(unicode);
				srcIdx += 3;
				break;
			default: // no process for bytes>3 utf8 character
				unicode = 0xFFFD;
				srcIdx += i;
		}
		dst[dstIdx++] = unicode;
	}
	dst[dstIdx] = 0x0000;
	return dstIdx;
}

static void Convert_UniStr_Pos(UINT16 * str)
{
    UINT16 temp;
    if(str == NULL)
        return;
    while(*str != 0)
    {
        temp = *str;
        *str = (temp<<8)|(temp>>8);  //change between big_end and little_end
        str++;
    }
}


static UINT8* smallletter(UINT8 *dst, const UINT8 *src, UINT32 len)
{
	UINT32 i;

	for (i=0; i<len; i++)
	{
		if (src[i] >= 'A' && src[i] <= 'Z')
			dst[i] = src[i] + 0x21;
		else
			dst[i] = src[i];
	}

	return dst;
}

static UINT16 get_iso639_lang_id(UINT8 *iso639)
{
	int i;

	for (i=0; i<iso639_lang_cnt; i++)
	{
		if (MEMCMP(iso639, iso639_lang[i].lang, 3) == 0)
			return iso639_lang[i].id;
	}

	return ISO_639_None;
}

/*
 * compare 2 iso639 language codes
 * example:
 *		"ENG" == "eng",	not care small letter and capital letter;
 *		"deu" == "ger",	some language may have multi-abbreviatives.
 */
INT32 compare_iso639_lang(const UINT8 *iso639_1, const UINT8 *iso639_2)
{
	UINT8 lang1[3], lang2[3];
	INT32 lang1_id, lang2_id;
	
	ASSERT(iso639_1 != NULL && iso639_2 != NULL);

	smallletter(lang1, iso639_1, 3);
	smallletter(lang2, iso639_2, 3);

	if ((lang1_id=get_iso639_lang_id(lang1)) != ISO_639_None
		&& (lang2_id=get_iso639_lang_id(lang2)) != ISO_639_None)
		return (lang2_id - lang1_id);
	else
		return MEMCMP(iso639_1, iso639_2, 3);
}

