#include <sys_config.h>

#include <api/libc/string.h>
#include <api/libc/printf.h>
#include <api/libge/osd_common.h>

UINT32 ComAscStr2Uni(UINT8* Ascii_str,UINT16* Uni_str)
{
   UINT32 i=0;

   if((Ascii_str==NULL)||(Uni_str==NULL))
       return 0;
   while(Ascii_str[i])
   {
#if(SYS_CPU_ENDIAN==ENDIAN_LITTLE)
       Uni_str[i]=(UINT16)(Ascii_str[i]<<8);
#else
       Uni_str[i]=(UINT16)Ascii_str[i];
#endif
       i++;
   }

   Uni_str[i]=0;
   return i;
}

#define utf8_to_u_hostendian(str, uni_str, err_flag) \
{\
	err_flag = 0;\
	if ((str[0]&0x80) == 0)\
		*uni_str++ = *str++;\
	else if ((str[1] & 0xC0) != 0x80) {\
		*uni_str++ = 0xfffd;\
		str+=1;\
	} else if ((str[0]&0x20) == 0) {\
		*uni_str++ = ((str[0]&31)<<6) | (str[1]&63);\
		str+=2;\
	} else if ((str[2] & 0xC0) != 0x80) {\
		*uni_str++ = 0xfffd;\
		str+=2;\
	} else if ((str[0]&0x10) == 0) {\
		*uni_str++ = ((str[0]&15)<<12) | ((str[1]&63)<<6) | (str[2]&63);\
		str+=3;\
	} else if ((str[3] & 0xC0) != 0x80) {\
		*uni_str++ = 0xfffd;\
		str+=3;\
	} else {\
		err_flag = 1;\
	}\
}

#define HOST_TO_LENDIAN_INT16(arg)	(uint16)(arg)
#define utf8_char_len(c) ((((int)0xE5000000 >> ((c >> 3) & 0x1E)) & 3) + 1)

//return -1:fail 0:success
static int utf8_to_unicode(
	const char	*src,
	int		*srcLen,
	UINT16		*dst)
{
	int srcLimit = *srcLen;
	int srcCount = 0;
	int dstCount = 0;
    INT32 j;

	while (srcCount < srcLimit)
	{
		unsigned short	unicode;
		unsigned short	*UNICODE = &unicode;
		unsigned char	*UTF8 = (unsigned char *)src + srcCount;
		int     err_flag;

		if ((srcCount + utf8_char_len(src[srcCount])) > srcLimit)
			break;

		utf8_to_u_hostendian(UTF8, UNICODE, err_flag);
		if (err_flag == 1)
			return -1;

#if(SYS_CPU_ENDIAN==ENDIAN_LITTLE)
        *dst++ = ((unicode&0x00ff)<<8) | ((unicode&0xff00)>>8);
#else
        *dst++ = unicode;
#endif
		srcCount += UTF8 - ((unsigned char *)(src + srcCount));
	}

	*srcLen = srcCount;

	return 0;
}

INT32 ComUTF8Str2Uni(UINT8* utf8,UINT16* Uni_str)
{
	INT32 result;
	unsigned long utf8len;


	utf8len = strlen(utf8) + 1;

	result = utf8_to_unicode(utf8, (int *) & utf8len,Uni_str);

	return result;
}

void ComUniStrToAsc(UINT8 * unicode,char* asc)
{
    INT32 i = 0;

	if((unicode==NULL)||(asc==NULL))
       	return;
	
    while( !(unicode[i*2+0] == 00 && unicode[i*2+1] == 0) )
    {
        asc[i] = unicode[i*2+1];
        i++;
    }
    asc[i] = '\0';
}


UINT32 ComInt2UniStr(UINT16 *pBuffer, UINT32 num, UINT32 len)
{
	char str[20];	
    UINT32 str_len;
	
	sprintf(str,"%u",num);
	if(len >0)
	{
	    str_len = STRLEN(str);

        if( len< str_len)
            str[len] = '\0';
        else if(len > str_len )
        {
            MEMSET(str,'0',len-str_len);
            sprintf(&str[len-str_len],"%u",num);
        }
    }
					
	return ComAscStr2Uni((UINT8*)str,pBuffer);	
}

UINT32 ComUniStrLen(const UINT16* string)
{
	UINT32 i=0;

	if(string == NULL)
		return 0;
	
	while (string[i])
		i++;
	return i;
}

UINT32 ComUniStrLenExt(const UINT8* string)
{
	UINT32 i=0;

	if(string == NULL)
		return 0;

	while (string[2*i] || string[2*i+1] )
		i+= 2;
	return i/2;
}

void ComUniStrCopy(UINT16* Target,const UINT16* Source)
{	
	UINT32 i;
	if((NULL == Target) || (NULL == Source))
		return;
		
	for(i=0;Source[i];i++)
	{
		Target[i]=Source[i];
	}
	Target[i]=Source[i];
}


void ComUniStrCopyEx(UINT16* Target,const UINT16* Source,UINT32 len)
{	
	UINT32 i;
	if((NULL == Target) || (NULL == Source))
		return;
		
	for(i=0;Source[i] && i<len;i++)
	{
		Target[i]=Source[i];
	}
	Target[i] = 0;
}

int ComUniStrCopyChar(UINT8 *dest, UINT8 *src)
{	
     unsigned int i;

	if((NULL == dest) || (NULL == src))
		return 0;
	
     for(i=0; !((src[i] == 0x0 && src[i+1] == 0x0)&&(i%2 == 0)) ;i++)
         dest[i] = src[i];
     if(i%2)
     {
        dest[i] = src[i];
        i++;
     }
     dest[i] = dest[i+1] = 0x0;
 
     return i/2;
}

int ComUniStrCopyCharN(UINT8 *dest, UINT8 *src, UINT32 len)
{	
     unsigned int i;

	if((NULL == dest) || (NULL == src))
		return 0;

     for(i=0; !(src[i] == 0x0 && src[i+1] == 0x0) && i<(len*2); i++)
         dest[i] = src[i];
     if(i%2)
     {
        dest[i] = src[i];
        i++;
     }
     dest[i] = dest[i+1] = 0x0;
 
     return i/2;
}

void ComUniStrMemSet(UINT16* Target,UINT8 c,UINT32 len)
{
	UINT32 i;

	if(Target == NULL)
		return;
	
	for(i=0;i<len;i++)
	{
#if(SYS_CPU_ENDIAN==ENDIAN_LITTLE)
       Target[i]=(UINT16)(c<<8);
#else
       Target[i]=(UINT16)c;
#endif		
	}
	Target[i] = 0;
	
}

UINT32 ComUniStr2Int(const UINT16* string)
{
	UINT8 i,len,c;
	UINT32 val;

	if(string == NULL)
		return 0;

	val = 0;
	len = ComUniStrLen(string);
	
	for(i=0;i<len;i++)
	{
#if(SYS_CPU_ENDIAN==ENDIAN_LITTLE)
		c = (UINT8)(string[i] >> 8);
#else
       	c = (UINT8)(string[i]);
#endif
		if(c>='0' && c<='9')
			val = val*10 + c - '0';
	}
	
	return val;	
}


UINT32 ComUniAnyStr2Int(const UINT16* string)  //for password,convert any unicode to INT ,not only number
{
	UINT8 i,len,c;
	UINT32 val;

	if(string == NULL)
		return 0;

	val = 0;
	len = ComUniStrLen(string);
	
	for(i=0;i<len;i++)
	{
#if(SYS_CPU_ENDIAN==ENDIAN_LITTLE)
		c = (UINT8)(string[i] >> 8);
#else
       	c = (UINT8)(string[i]);
#endif
			val = val*10 + c - '0'; 
	}
	
	return val;	
}


char GetUniStrCharAt(const UINT16* string,UINT32 pos)
{
	UINT8 len,c;

	if(string == NULL)
		return 0;

	len = ComUniStrLen(string);
	if(len <= pos)
		return '\0';
		
#if(SYS_CPU_ENDIAN==ENDIAN_LITTLE)
	c = (UINT8)(string[pos] >> 8);
#else
	c = (UINT8)(string[pos]);
#endif

	return c;	
}

void SetUniStrCharAt(UINT16* string, char c, UINT32 pos)
{
	UINT8 len;

	if(string == NULL)
		return;

	len = ComUniStrLen(string);
	//if(len <= pos) return ;
#if(SYS_CPU_ENDIAN==ENDIAN_LITTLE)
	string[pos]=(UINT16)(c<<8);
#else
	string[pos]=(UINT16)c;
#endif			
}


void SetUniStrUniCharAt(UINT16* string, UINT16 c, UINT32 pos)
{
	UINT8 len;
	UINT16 ch;

	if(string == NULL)
		return;

	len = ComUniStrLen(string);
	//if(len <= pos) return ;
#if(SYS_CPU_ENDIAN==ENDIAN_LITTLE)
	ch =  ( (UINT8)(c>>8))  |  ( ( (UINT8)c)<<8);
#else
	ch = c;
#endif			
	string[pos]=(UINT16)ch;
}


BOOL ComUniStr2IntExt(const UINT16* string,UINT8 cnt,UINT32* val)
{
	UINT8 i,ch;
	UINT32 u32 = 0;

	if(string == NULL)
		return FALSE;
		
	for(i=0;i<cnt;i++)
	{
		ch = GetUniStrCharAt(string,i);
		if(IS_NUMBER_CHAR(ch) )
			u32 = u32*10 + ch - '0';
		else
			return FALSE;
	}
	
	*val = u32;
	
	return TRUE;
}

UINT16 ComMB16ToUINT16(UINT8* pVal)
{
	UINT16 wRes=0;

	if(pVal == NULL)
		return 0;

 #if(SYS_CPU_ENDIAN==ENDIAN_BIG)
	wRes = *pVal;
#else
	wRes=(UINT16)((0xff00&(*pVal<<8))|(0x00ff&(*(pVal+1))));
#endif
	return wRes;	
}

BOOL IS_NEWLINE(UINT16  wc)
{
    if (wc == '\n')
        return TRUE;

    if (wc == '\r')
        return TRUE;

    return FALSE;
}

UINT32 ComUniStrToMB(UINT16* pwStr)
 {
 	if(pwStr == NULL)
		return 0;
	
 #if(SYS_CPU_ENDIAN==ENDIAN_BIG)
	return ComUniStrLen(pwStr);
#else
	UINT32 i=0;
	while(pwStr[i])
	{
		pwStr[i]=(UINT16)(((pwStr[i]&0x00ff)<<8) | ((pwStr[i]&0xff00)>>8));
		i++;
	}
	return i;
#endif
 }

UINT32 ComUniStrCat(UINT16 * Target, const UINT16* Source)
{
	UINT32 i,j;
	if(Target==NULL)
		return 0;	
 	i=ComUniStrLen(Target);
 	if(Source==NULL)
 		return i;
 	j=0;
 	while(Source[j])
 		Target[i++]=Source[j++];
 	Target[i]=0;
 	return i;
}

INT32 ComUniStrCmp(const UINT16 * dest, const UINT16* src)
{
	unsigned int i;

	if((NULL == dest) || (NULL == src))
		return 0;

	for (i = 0; dest[i] != '\0' && src[i] != '\0'; i++)
	{
		if (dest[i] != src[i])
		{
			return -2;
		}
	}

	if (dest[i] == '\0' && src[i] == '\0')
	{
		return 0;
	} else if (dest[i] == '\0')
	{
		return -1;
	} else
	{
		return 1;
	}
}


INT32 ComUniStrCmpExt(const UINT8 * dest, const UINT8* src)
{
     unsigned int i;

 	if((NULL == dest) || (NULL == src))
		return 0;

     for (i = 0; !(dest[i] == 0x0 && dest[i+1]== 0x0) && !(src[i] == 0x0 && src[i+1]== 0x0); i++)
     {
         if (dest[i] != src[i])
         {
             return dest[i] - src[i];
         }
     }
 
     if (dest[i] == 0x0 && dest[i+1] == 0x0 
         && src[i] == 0x0 && src[i+1] == 0x0)
     {
         return 0;
     }
     else if (dest[i] == 0x0 && dest[i+1] == 0x0)
     {
         return -1;
     } else
     {
         return 1;
     }

}

UINT16 *ComStr2UniStrExt(UINT16* uni, char* str, UINT16 maxcount)
{	
	UINT16 i;

	if (uni == NULL)
		return NULL;

	if (str == NULL || maxcount == 0)
	{
		uni[0]=(UINT16)'\0';
		return NULL;
	}

	for(i=0; (0!=str[i])&&(i<maxcount); i++)
#if (SYS_CPU_ENDIAN==ENDIAN_LITTLE)
       uni[i]=(UINT16)(str[i]<<8);
#else
       uni[i]=(UINT16)str[i];
#endif
	uni[i]=(UINT16)'\0';

	return uni;
}

