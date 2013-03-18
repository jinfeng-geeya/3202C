#include <sys_config.h>

#include <api/libloader/ld_osd_lib.h>

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

UINT32 ComUniStrLen(const UINT16* string)
{
	UINT32 i=0;

	if(string == NULL)
		return 0;
	
	while (string[i])
		i++;
	return i;
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

