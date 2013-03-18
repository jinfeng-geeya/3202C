/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 Copyright (C)
*
*    File:    string.c
*
*    Description:    This file contains all functions definition
*		             of string and memory operations.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	Dec.23.2002       Justin Wu       Ver 0.1    Create file.
*	2.
*****************************************************************************/

#include <osal/osal.h>
#include <basic_types.h>
#include <api/libc/string.h>
#include <sys_config.h>

/*
 * 	Name		:   libc_memcpy()
 *	Description	:
 *	Parameter	:
 *
 *	Return		:
 *
 */

/*****************************************************************************
 Function     : GetSysRandom
 Description  : Get a random value based the given Maxvalue
 Arguments    : None
 Return value : UINT32
*****************************************************************************/
/* Knuth's TAOCP section 3.6 */
#define	M	((1U<<31) -1)
#define	A	48271
#define	Q	44488		// M/A
#define	R	3399		// M%A; R < Q !!!

// FIXME: ISO C/SuS want a longer period

int rand_r(unsigned int* seed)
{
	int X;
    X = *seed;
    X = A*(X%Q) - R * (int)(X/Q);
    if (X < 0)
		X += M;
	
    *seed = X;
    return X;
}


static unsigned int seed=1;
/*void srand(unsigned int i)
{
	if(i!=0)
		seed=i;
}

int rand(void)
{
  return rand_r(&seed);
}*/

UINT32 ali_rand(UINT32 MaxVal)
{
	UINT32 dwTick;
	if (MaxVal == 0)
		return 0;
	dwTick = read_tsc();
	return (dwTick % MaxVal);
}
//UINT32 rand(UINT32 MaxVal) __attribute__((alias("ali_rand")));

#ifndef __mips16
void MemSet4(void *dest, int c, UINT32 len)
{
	__asm__ __volatile__(
		"	.set noreorder			; \
		Loop1:						; \
		sw		$5, 0($4)		; \
		sw		$5, 4($4)		; \
		sw		$5, 8($4)		; \
		sw		$5, 12($4)		; \
		subu		$6, 16		; \
		bgtz		$6, Loop1	; \
		addu	$4, 16			; \
		.set reorder"
	: : : "$8", "$9", "$10", "$11", "$12", "$13", "$14", "$15" );
}
#endif

#ifndef __mips16
void MemCopy4(void *dest, const void *src, UINT32 len)
{
	__asm__ __volatile__(
		"	.set noreorder			; \
		Loop2:						; \
		lw		$8, 0($5)		; \
		lw		$9, 4($5)		; \
		sw		$8, 0($4)		; \
		sw		$9, 4($4)		; \
		lw		$10, 8($5)		; \
		lw		$11, 12($5)		; \
		sw		$10, 8($4)		; \
		sw		$11, 12($4)		; \
		addu	$4, 16			; \
		subu		$6, 16		; \
		bgtz		$6, Loop2	; \
		addu	$5, 16			; \
		.set reorder"
	: : : "$8", "$9", "$10", "$11", "$12", "$13", "$14", "$15" );
}
#endif

#ifndef __mips16
void* ali_memcpy(void *dest, const void *src, unsigned int len)
{
	if(len == 0)
	{
		return dest;
	}
	//the source and destination overlap, this function does not ensure that the original source bytes in the overlapping region are copied before being overwritten.
#ifdef DUAL_ENABLE
#ifndef SEE_CPU
   if(osal_dual_is_private_address(dest))
   {	
        osal_cache_flush((void*)(src), len);
        if(*(unsigned char *)(src+len-1) != *(volatile unsigned char*)((UINT32)(src+len-1)|0xa0000000))
        {
			libc_printf("len=%d\n", len);
            //make sure data is flushed into cache before send to SEE
            ASSERT(0);
        }
  	    hld_dev_memcpy(dest, ((UINT32)(src)&0xfffffff)|0xa0000000, len);
		return dest;
	}
#endif
#endif

	if (!((UINT32)dest&0xf) && !((UINT32)src&0xf) && (0x0f < len))
	{
		MemCopy4(dest, src, len&0xfffffff0);
		if (len&0x0f)
		{
			UINT32 dwCount;
			UINT8 *DestBuf, *SrcBuf;

			DestBuf = (UINT8 *)dest + (len & 0xfffffff0);
			SrcBuf = (UINT8 *)src + (len & 0xfffffff0);

			for (dwCount = 0;dwCount < (len&0x0f);dwCount++)
				*(DestBuf + dwCount) = *(SrcBuf + dwCount);
		}

	}
	else if(!((UINT32)dest&0x3) && !((UINT32)src&0x3) && (0x03 < len))
	{
		UINT32 dwCount, dwLen;
		UINT32 *DestBuf,*SrcBuf;
		UINT8 * DestBufByte, *SrcBufByte;

		dwLen = (len&0xfffffffc)>>2;
		DestBuf = (UINT32 *)dest;
		SrcBuf = (UINT32 *)src;
		for(dwCount=0;dwCount<dwLen;dwCount++) 
			*(DestBuf+dwCount)=*(SrcBuf+dwCount);
		dwLen = len&0x3;
		DestBufByte = (UINT8 *)(dest + (dwCount<<2));
		SrcBufByte = (UINT8 *)(src +(dwCount<<2));
		for(dwCount=0;dwCount<dwLen;dwCount++) 
			*(DestBufByte+dwCount)=*(SrcBufByte+dwCount);
	}
	else if(!((UINT32)dest&0x1) && !((UINT32)src&0x1) && (0x01 < len))
	{
		UINT32 wCount, wLen;
		UINT16 *DestBuf,*SrcBuf;
		UINT8 * DestBufByte, *SrcBufByte;		

		wLen = (len&0xfffffffe)>>1;
		DestBuf=(UINT16 *)dest;
		SrcBuf=(UINT16 *)src;
		for(wCount=0;wCount<wLen;wCount++) 
			*(DestBuf+wCount)=*(SrcBuf+wCount);
		wLen = len&0x1;
		DestBufByte = (UINT8 *)(dest +(wCount<<1));
		SrcBufByte = (UINT8 *)(src +(wCount<<1));
		for(wCount=0;wCount<wLen;wCount++) 
			*(DestBufByte+wCount)=*(SrcBufByte+wCount);		
	}
	else
	{
		UINT32 dwCount;
		UINT8 *DestBuf, *SrcBuf;

		DestBuf = (UINT8 *)dest,
		SrcBuf = (UINT8 *)src;

		for (dwCount = 0;dwCount < len;dwCount++)
			*(DestBuf + dwCount) = *(SrcBuf + dwCount);
	}

	return dest;
}
void* memcpy(void *dest, const void *src, unsigned int len) __attribute__((alias("ali_memcpy")));
#else
void* memcpy(void *dest, const void *src, unsigned int len)
{
	return ali_memcpy(dest, src, len);
}

__ATTRIBUTE_RAM_
void* ali_memcpy(void *dest, const void *src, unsigned int len)
{
	//the source and destination overlap, this function does not ensure that the original source UINT8s in the overlapping region are copied before being overwritten.
	UINT32 dwCount;
	UINT8 *DestBuf, *SrcBuf;

	DestBuf = (UINT8 *)dest,
			  SrcBuf = (UINT8 *)src;

	for (dwCount = 0;dwCount < len;dwCount++)
		*(DestBuf + dwCount) = *(SrcBuf + dwCount);

	return dest;
}
#endif

#ifndef __mips16
void* ali_memset(void *dest, int c, unsigned int  len)
{
	if (!((UINT32)dest&0x3) && (0x0f < len))
	{
		c = (UINT8)c;
		MemSet4(dest, (c + (c << 8) + (c << 16) + (c << 24)), len&0xfffffff0);
		if (len&0x0f)
		{
			UINT32 dwCount;
			UINT8 *DestBuf;
			DestBuf = (UINT8 *)dest + (len & 0xfffffff0);
			for (dwCount = 0;dwCount < (len&0x0f);dwCount++)
				*(DestBuf + dwCount) = (UINT8)c;
		}
	}
	else
	{
		UINT32 dwCount;
		UINT8 *DestBuf;
		DestBuf = (UINT8 *)dest;
		for (dwCount = 0;dwCount < len;dwCount++)
			*(DestBuf + dwCount) = (UINT8)c;
	}

	return dest;
}
void* memset(void *dest, int c, unsigned int  len) __attribute__((alias("ali_memset")));
#else
void* memset(void *dest, int c, unsigned int  len)
{
	return ali_memset(dest, c, len);
}

void* ali_memset(void *dest, int c, unsigned int  len)
{
	UINT32 dwCount;
	UINT8 *DestBuf;

	DestBuf = (UINT8 *)dest;

	for (dwCount = 0;dwCount < len;dwCount++)
		*(DestBuf + dwCount) = c;

	return dest;
}
#endif


int ali_memcmp(const void *buf1, const void *buf2, unsigned int  len)
{
	UINT32 dwCount;
	UINT8* cmp1, *cmp2;

	cmp1 = (UINT8*)buf1;
	cmp2 = (UINT8*)buf2;

	for (dwCount = 0;dwCount < len;dwCount++)
	{
		if (*(cmp1 + dwCount) < *(cmp2 + dwCount))
			return -1;	//buf1 less than buf2
		else if (*(cmp1 + dwCount) > *(cmp2 + dwCount))
			return 1;	//buf1 greater than buf2
	}

	return 0;	//buf1 identical to buf2
}
int memcmp(const void *buf1, const void *buf2, unsigned int  len) __attribute__((alias("ali_memcmp")));

void * ali_memmove(void * dest, const void * src, unsigned int n)
{
	unsigned long dstp = (unsigned long) dest;
	unsigned long srcp = (unsigned long) src;

	UINT32 temp;

	if (!n)
		return dest;

	//if no overlap, just call memcpy
	if((dstp + n < srcp) || (srcp + n < dstp))
	{
		return ali_memcpy(dest, src, n);
	}
	else
	{
		char *a = dest;
		const char *b = src;
		if (src != dest)
		{
			if (src > dest)
			{
				while (n--) *a++ = *b++;
			}
			else
			{
				a += n - 1;
				b += n - 1;
				while (n--) *a-- = *b--;
			}
		}
		return dest;
	}
	
}
void * memmove(void * dest, const void * src, unsigned int n) __attribute__((alias("ali_memmove")));
/*
 * 	Name		:   libc_strcpy()
 *	Description	:
 *	Parameter	:
 *
 *	Return		:
 *
 */
char* ali_strcpy(char *dest, const char *src)
{
	unsigned int i;

	for (i = 0; src[i] != '\0'; i++)
	{
		dest[i] = src[i];
	}

	dest[i] = '\0';

	return dest;
}
char* strcpy(char *dest, const char *src) __attribute__((alias("ali_strcpy")));
/*
 * 	Name		:   libc_strcmp()
 *	Description	:
 *	Parameter	:
 *
 *	Return		:
 *
 */
int ali_strcmp(const char *s1, const char *s2)
{
	while (*s1 != '\0' && *s1 == *s2)
	{
		s1++;
		s2++;
	}

	return (*(unsigned char *) s1) - (*(unsigned char *) s2);
}
int strcmp(const char *s1, const char *s2) __attribute__((alias("ali_strcmp")));

/*
 * 	Name		:   libc_strlen()
 *	Description	:
 *	Parameter	:
 *
 *	Return		:
 *
 */
unsigned long ali_strlen(const char *src)
{
	unsigned long i;

	for (i = 0; src[i] != '\0'; i++);

	return i;
}
unsigned long strlen(const char *src) __attribute__((alias("ali_strlen")));

int ali_itoa(char * str, unsigned long val)
{
	char *p;
	char *first_dig;
	char temp;
	unsigned t_val;
	int len = 0;
	p = str;
	first_dig = p;

	do
	{
		t_val = (unsigned)(val % 0x0a);
		val   /= 0x0a;
		*p++ = (char)(t_val + '0');
		len++;
	}
	while (val > 0);//按实际数位取
	*p-- = '\0';

	//逆序转置
	do
	{
		temp = *p;
		*p   = *first_dig;
		*first_dig = temp;
		--p;
		++first_dig;
	}
	while (first_dig < p);
	return len;
}
//int itoa(char * str, unsigned long val) __attribute__((alias("ali_itoa")));

#ifdef NEW_CLIB_FUNCTION

char * strrev(char *s)
{
    int j,k,c;
    for(k=0;s[k] != 0;k++);
    for(j=0,k--;j<k;j++,k--) {
	c = s[j];
	s[j] = s[k];
	s[k] = c;
    }
    return s;
}

static char lower[] = "0123456789abcdefghijklmnopqrstuvwxyz";
char *itoa(int n, char *s, int radix)
{
    unsigned int uval;
    int i, sign;

    if((sign = (n < 0)) && radix == 10)
	uval = -n;
    else
	uval = n;

    i=0;
    do {
      s[i++] = lower[uval % radix];
      uval /= radix;
    }while(uval>0);

    if (sign)
	s[i++] = '-';
    s[i] = '\0';
    return strrev(s);
}

#endif

int ali_atoi(const char * str)
{
	const char *pstr = str;
	int result = 0;
	int sign = 1;

	//found sign and skip blank
	while(*pstr==' ')
		pstr++;
	if(*pstr == '-')
	{
		sign = -1;
		pstr++;
	}
	if(*pstr == '+')
	{
		pstr++;
	}
/*	while(*pstr)
	{
		if(*pstr == ' ')
			pstr++;
		else if(*pstr == '-')
		{
			sign = -1;
			pstr++;
			break;
		}
		else if(*pstr == '+')
		{
			pstr++;
			break;
		}
		else
			break;
	}*/
	
	while ((unsigned int)(*pstr - '0')<10u)
	{
		result = result * 10 + *pstr - '0';
		pstr++;
	}

	result= result * sign;

	return result;
}
int atoi(const char * str) __attribute__((alias("ali_atoi")));

unsigned long ali_atoi16(const char  *str)
{
	unsigned char  *tmp = (unsigned char *)str;
	unsigned long sum = 0;
	unsigned char data = 0;

	while (*tmp != '\0')
	{
		switch (*tmp)
		{
		case 'A':
		case 'a':
			data = 10;
			break;
		case 'B':
		case 'b':
			data = 11;
			break;
		case 'C':
		case 'c':
			data = 12;
			break;
		case 'D':
		case 'd':
			data = 13;
			break;
		case 'E':
		case 'e':
			data = 14;
			break;
		case 'F':
		case 'f':
			data = 15;
			break;
		default:
			data = *tmp - '0';
			break;
		}
		sum = sum * 16 + data;
		tmp++;
	}
	return sum;
}

unsigned long com_atoi(char * str)
{
	if (*str >= '1' && *str <= '9')
		return ali_atoi(str);
	else
		return ali_atoi16(str);
}



///porting functions for the open source codes who call the ali_xxx functions
UINT32 libc_rand(UINT32 MaxVal)
{
	return ali_rand(MaxVal);
}

void* libc_memcpy(void *dest, const void *src, unsigned int len)
{
	return ali_memcpy(dest, src, len);
}

void* libc_memset(void *dest, int c, unsigned int  len)
{
	return ali_memset(dest, c, len);
}

int libc_memcmp(const void *buf1, const void *buf2, unsigned int  len)
{
	return ali_memcmp(buf1, buf2, len);
}

void * libc_memmove(void * dest, const void * src, unsigned int n)
{
	return ali_memmove(dest, src, n);
}
char* libc_strcpy(char *dest, const char *src)
{
	return ali_strcpy(dest, src);
}
int libc_strcmp(const char *dest, const char *src)
{
	return ali_strcmp(dest, src);
}
unsigned long libc_strlen(const char *src)
{
	return ali_strlen(src);
}
int libc_itoa(char * str, unsigned long val)
{
	ali_itoa(str, val);
}
int libc_atoi(const char * str)
{
	return ali_atoi(str);
}
unsigned long libc_atoi16(const char  *str)
{
	return ali_atoi16(str);
}

static int maxExponent = 511;	/* Largest possible base 10 exponent.  Any
				 * exponent larger than this will already
				 * produce underflow or overflow, so there's
				 * no need to worry about additional digits.
				 */
static double powersOf10[] = {	/* Table giving binary powers of 10.  Entry */
    10.,			/* is 10^2^i.  Used to convert decimal */
    100.,			/* exponents into floating-point numbers. */
    1.0e4,
    1.0e8,
    1.0e16,
    1.0e32,
    1.0e64,
    1.0e128,
    1.0e256
};

/*
 *----------------------------------------------------------------------
 *
 * strtod --
 *
 *	This procedure converts a floating-point number from an ASCII
 *	decimal representation to internal double-precision format.
 *
 * Results:
 *	The return value is the double-precision floating-point
 *	representation of the characters in string.  If endPtr isn't
 *	NULL, then *endPtr is filled in with the address of the
 *	next character after the last one that was part of the
 *	floating-point number.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

double
strtod(string, endPtr)
    const char *string;		/* A decimal ASCII floating-point number,
				 * optionally preceded by white space.
				 * Must have form "-I.FE-X", where I is the
				 * integer part of the mantissa, F is the
				 * fractional part of the mantissa, and X
				 * is the exponent.  Either of the signs
				 * may be "+", "-", or omitted.  Either I
				 * or F may be omitted, or both.  The decimal
				 * point isn't necessary unless F is present.
				 * The "E" may actually be an "e".  E and X
				 * may both be omitted (but not just one).
				 */
    char **endPtr;		/* If non-NULL, store terminating character's
				 * address here. */
{
	int sign, expSign = 0;
	double fraction, dblExp, *d;
	register const char *p;
	register int c;
	int exp = 0;		/* Exponent read from "EX" field. */
	int fracExp = 0;		/* Exponent that derives from the fractional
	* part.  Under normal circumstatnces, it is
	* the negative of the number of digits in F.
	* However, if I is very long, the last digits
	* of I get dropped (otherwise a long I with a
	* large negative exponent could cause an
	* unnecessary overflow on I alone).  In this
	* case, fracExp is incremented one for each
	* dropped digit. */
	int mantSize;		/* Number of digits in mantissa. */
	int decPt;			/* Number of mantissa digits BEFORE decimal
	* point. */
	const char *pExp;		/* Temporarily holds location of exponent
	* in string. */

	/*
	* Strip off leading blanks and check for a sign.
	*/

	p = string;
	while (isspace(*p))
	{
		p += 1;
	}
	if (*p == '-')
	{
		sign = 1;
		p += 1;
	}
	else
	{
		if (*p == '+')
		{
			p += 1;
		}
		sign = 0;
	}

	/*
	* Count the number of digits in the mantissa (including the decimal
	* point), and also locate the decimal point.
	*/

	decPt = -1;
	for (mantSize = 0; ; mantSize += 1)
	{
		c = *p;
		if (!isdigit(c))
		{
			if ((c != '.') || (decPt >= 0))
			{
				break;
			}
			decPt = mantSize;
		}
		p += 1;
	}

	/*
	* Now suck up the digits in the mantissa.  Use two integers to
	* collect 9 digits each (this is faster than using floating-point).
	* If the mantissa has more than 18 digits, ignore the extras, since
	* they can't affect the value anyway.
	*/

	pExp  = p;
	p -= mantSize;
	if (decPt < 0) 
	{
		decPt = mantSize;
	} else 
	{
		mantSize -= 1;			/* One of the digits was the point. */
	}
	if (mantSize > 18)
	{
		fracExp = decPt - 18;
		mantSize = 18;
	} else
	{
		fracExp = decPt - mantSize;
	}
	if (mantSize == 0)
	{
		fraction = 0.0;
		p = string;
		goto done;
	} else 
	{
		int frac1, frac2;
		frac1 = 0;
		for ( ; mantSize > 9; mantSize -= 1)
		{
			c = *p;
			p += 1;
			if (c == '.')
			{
				c = *p;
				p += 1;
			}
			frac1 = 10*frac1 + (c - '0');
		}
		frac2 = 0;
		for (; mantSize > 0; mantSize -= 1)
		{
			c = *p;
			p += 1;
			if (c == '.')
			{
				c = *p;
				p += 1;
			}
			frac2 = 10*frac2 + (c - '0');
		}
		fraction = (1.0e9 * frac1) + frac2;
	}

	/*
	* Skim off the exponent.
	*/

	p = pExp;
	if ((*p == 'E') || (*p == 'e'))
	{
		p += 1;
		if (*p == '-')
		{
			expSign = 1;
			p += 1;
		} 
		else
		{
			if (*p == '+')
			{
				p += 1;
			}
			expSign = 0;
		}
		if(isdigit(*p))
		{
			while (isdigit(*p))
			{
				exp = exp * 10 + (*p - '0');
				p += 1;
			}
		}
		else
		{
			p = pExp;
		}
	}
	if (expSign) {
		exp = fracExp - exp;
	} else {
		exp = fracExp + exp;
	}

	/*
	* Generate a floating-point number that represents the exponent.
	* Do this by processing the exponent one bit at a time to combine
	* many powers of 2 of 10. Then combine the exponent with the
	* fraction.
	*/

	if (exp < 0)
	{
		expSign = 1;
		exp = -exp;
	}
	else
	{
		expSign = 0;
	}
	if (exp > maxExponent)
	{
		exp = maxExponent;
	}
	dblExp = 1.0;
	for (d = powersOf10; exp != 0; exp >>= 1, d += 1)
	{
		if (exp & 01)
		{
			dblExp *= *d;
		}
	}
	if (expSign)
	{
		fraction /= dblExp;
	}
	else
	{
		fraction *= dblExp;
	}

	done:
	if (endPtr != NULL)
	{
		*endPtr = (char *) p;
	}

	if (sign)
	{
		return -fraction;
	}
	return fraction;
}

double atof(const char *nptr)
{
  double tmp=strtod(nptr,0);
  return tmp;
}


int strspn(const char *s, const char *accept)
{
	int l=0;
	int a=1,i;
	int al=strlen(accept);

	while((a)&&(*s))
	{
		for(a=i=0;(!a)&&(i<al);i++)
			if (*s==accept[i]) a=1;
				if (a) l++;
					s++;
	}
	return l;
}

int strcspn(const char *s, const char *reject)
{
  int l=0;
  int a=1,i,al=strlen(reject);

  while((a)&&(*s))
  {
    for(i=0;(a)&&(i<al);i++)
      if (*s==reject[i]) a=0;
    if (a) l++;
    s++;
  }
  return l;
}

char*strtok_r(char*s,const char*delim,char**ptrptr)
{
	char*tmp=0;

	if (s==0)
		s=*ptrptr;
	s+=strspn(s,delim);		/* overread leading delimiter */
	if(*s)
	{
		tmp=s;
		s+=strcspn(s,delim);
		if(*s) *s++=0;	/* not the end ? => terminate it */
	}
	
	*ptrptr=s;
	
	return tmp;
}

static char *strtok_pos;
char *strtok(char *s, const char *delim)
{
	return strtok_r(s,delim,&strtok_pos);
}


#ifdef NEW_CLIB_FUNCTION

int my_tolower(char ch)

{

   if(ch >= 'A' && ch <= 'Z')

      return (ch + 0x20);

   return ch;

}

int memicmp(const void *buffer1,const void *buffer2,int count)
{

    int f = 0;

    int l = 0;

    while (count--)

    {

        if ( (*(unsigned char *)buffer1 == *(unsigned char *)buffer2) ||

                ((f = my_tolower( *(unsigned char *)buffer1 )) ==

                (l =  my_tolower( *(unsigned char *)buffer2 ))) )

        {

        buffer1 = (char *)buffer1 + 1;

        buffer2 = (char *)buffer2 + 1;

        }

        else

            break;

    }

    return ( f - l );

}

#endif



