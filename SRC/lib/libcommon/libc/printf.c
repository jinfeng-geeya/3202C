/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 Copyright (C)
*
*    File:    prinf.c
*
*    Description:    This file contains all functions definition
*		             of printf operations.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	Dec.23.2002       Justin Wu       Ver 0.1    Create file.
*	2.
*****************************************************************************/

#include <stdarg.h>
#include <stddef.h>

#include <osal/osal.h>
#include <sys_config.h>
#include <osal/osal.h>
#include <osal/osal_task.h>
#include <api/libc/string.h>
#include <bus/sci/sci.h>
#include <api/libc/printf.h>

#ifdef NETWORK_SUPPORT
#if (SYS_NETWORK_MODULE == NET_USB_WIFI)
#define WANT_FLOATING_POINT_IN_PRINTF	// WiFi Support (Wireless Tool need this...)
#endif
#endif
//**********************************************************************
// Print debug message through debug (serial) port
//**********************************************************************
#define	to_digit(c)	((c) - '0')

#define yes_digit(c)	(((c) >= '0') && ((c) <= '9'))
#define	to_char(n)	((n) + '0')
#define BUF	40

/*
 * Flags used during conversion.
 */
#define	LONGINT		0x01		/* long integer */
#define	LONGDBL		0x02		/* long double; unimplemented */
#define	SHORTINT	0x04		/* short integer */
#define	ALT		0x08		/* alternate form */
#define	LADJUST		0x10		/* left adjustment */
#define	ZEROPAD		0x20		/* zero (as opposed to blank) pad */
#define	HEXPREFIX	0x40		/* add 0x or 0X prefix */

///
//#define outbyte(a)		sci_write(SCI_FOR_RS232, a)
///

#if ((defined SEE_CPU)|| (defined SUPPORT_LINUX))
	// SEE can not use macro "__MM_DBG_MEM_ADDR", becasue it is configed by CPU.
	// so use a internal array as printf memory base.
	#define RPINTF_TO_MEM
	#define MEM_PRINTF_SIZE 	0x4000
	UINT8 see_printf_memory[MEM_PRINTF_SIZE];
	#define MEM_PRINTF_BASE 	see_printf_memory
	static UINT8 *m_PrintMem = (UINT8 *)(MEM_PRINTF_BASE);
	static UINT8 *m_CurPrintPos = (UINT8 *)(MEM_PRINTF_BASE);
#else

#ifdef RPINTF_TO_MEM
	//#define MEM_PRINTF_BASE (__MM_DBG_MEM_ADDR+(__MM_DBG_MEM_LEN>>1))
	//#define MEM_PRINTF_SIZE (__MM_DBG_MEM_LEN>>1)
	#define MEM_PRINTF_BASE (__MM_DBG_MEM_ADDR)
	#define MEM_PRINTF_SIZE (__MM_DBG_MEM_LEN)
	static UINT8 *m_PrintMem = (UINT8 *)(MEM_PRINTF_BASE);
	static UINT8 *m_CurPrintPos = (UINT8 *)(MEM_PRINTF_BASE);
#endif
#endif

static UINT8 libc_printf_uart_id = SCI_FOR_RS232;
void libc_printf_uart_id_set(UINT8 id) //id = SCI_FOR_RS232 or id = SCI_FOR_MDM
{
	libc_printf_uart_id = id;
}
#ifndef INVALID_ID
#define INVALID_ID 0xFFFF
#endif

static ID dsh_task_id = INVALID_ID;
static UINT8 switch_display_mode = 1; // 0 ON; 1 OFF
void set_dsh_task_id(ID task_id)
{
	dsh_task_id = task_id;
}
void set_switch_display_mode(UINT8 display_mode)
{
	switch_display_mode = display_mode;
}


int outbyte(char ch)
{
#if (defined(RPINTF_TO_MEM))
	static BOOL first_run = TRUE;
	if (first_run == TRUE)
	{
		MEMSET(m_PrintMem, 0, MEM_PRINTF_SIZE);
		first_run = FALSE;
	}
	*m_CurPrintPos = ch;
	m_CurPrintPos ++;
	if (m_CurPrintPos >= m_PrintMem + MEM_PRINTF_SIZE)
		m_CurPrintPos = (UINT8 *)(MEM_PRINTF_BASE);
#elif (defined(PRINTF_FROM_MDM))
	sci_write(SCI_FOR_MDM, ch);
#else
	if(dsh_task_id == INVALID_ID ||
		dsh_task_id == osal_task_get_current_id() ||
		switch_display_mode == 0)
		sci_write(libc_printf_uart_id, ch);
#endif
	return 1;
}

/*
 * BEWARE, these `goto error' on error, and PAD uses `n'.
 */
#ifdef REFACTOR_PRINTF
#define	PRINT(ptr, len) for(i = 0;i < len;i++){ if('\n'==*ptr) output(&handle, '\r');output(&handle, *ptr++);}
#define PAD_SP(howmany) for(i = 0;i < howmany;i++) output(&handle, ' ');
#define PAD_0(howmany) for(i = 0;i < howmany;i++) output(&handle, '0');
#else
#define	PRINT(ptr, len) for(i = 0;i < len;i++){ if('\n'==*ptr) outbyte('\r');outbyte(*ptr++);}
#define PAD_SP(howmany) for(i = 0;i < howmany;i++) outbyte(' ');
#define PAD_0(howmany) for(i = 0;i < howmany;i++) outbyte('0');
#endif

/*
 * To extend shorts properly, we need both signed and unsigned
 * argument extraction methods.
 */
#define	SARG() \
	(flags&LONGINT ? va_arg(ap, long) : \
	    flags&SHORTINT ? (long)(short)va_arg(ap, int) : \
	    (long)va_arg(ap, int))
#define	UARG() \
	(flags&LONGINT ? va_arg(ap, unsigned long) : \
	    flags&SHORTINT ? (unsigned long)(unsigned short)va_arg(ap, int) : \
	    (unsigned long)va_arg(ap, unsigned int))


// Finds characters in a buffer.
//If successful, memchr returns a pointer to the first location of c in buf. Otherwise it returns NULL.

UINT8 * ComMemFindChar( const UINT8 *buf, char c, int count )
{
	int iCount = 0;
	if (buf == NULL)
		return NULL;
	if (count <= 0)
		return NULL;
	for (iCount = 0;iCount < count;iCount++)
	{
		if (*(UINT8 *)((UINT32)buf + iCount) == c)
			return (UINT8 *)((UINT32)buf + iCount);
	}
	return NULL;
}

#ifdef REFACTOR_PRINTF
typedef void *(*BYTE_OUTPUT)(void **handle, char ch);
static void *libc_vprintf(void *handle, BYTE_OUTPUT output, const char *fmt, va_list ap)
{
	int ch;	/* character from fmt */
	int n;		/* handy integer (short term usage) */
	char *cp;	/* handy char pointer (short term usage) */
	const char *fmark;	/* for remembering a place in fmt */
	int flags;	/* flags as above */
	int width;		/* width from format (%8d), or 0 */
	int prec;		/* precision from format (%.3d), or -1 */
	char sign;		/* sign prefix (' ', '+', '-', or \0) */
	unsigned long _ulong;	/* integer arguments %[diouxX] */
	enum { OCT, DEC, HEX } base;/* base for [diouxX] conversion */
	int dprec;		/* a copy of prec if [diouxX], 0 otherwise */
	int dpad;		/* extra 0 padding needed for integers */
	int fieldsz;		/* field size expanded by sign, dpad etc */
	/* The initialization of 'size' is to suppress a warning that
	   'size' might be used unitialized.  It seems gcc can't
	   quite grok this spaghetti code ... */
	int size = 0;		/* size of converted field or string */
	char buf[BUF];		/* space for %c, %[diouxX], %[eEfgG] */
	int i;

	/*
	 * Scan the format for conversions (`%' character).
	 */
	for (;;)
	{
		for (fmark = fmt; (ch = *fmt) != '\0' && ch != '%'; fmt++)
			/* void */;
		if ((n = fmt - fmark) != 0)
		{
			PRINT(fmark, n);
		}
		if (ch == '\0')
			goto done;
		fmt++;		/* skip over '%' */

		flags = 0;
		dprec = 0;
		width = 0;
		prec = -1;
		sign = '\0';

rflag:		ch = *fmt++;
reswitch:	switch (ch)
		{
		case ' ':
			/*
			 * ``If the space and + flags both appear, the space
			 * flag will be ignored.''
			 *	-- ANSI X3J11
			 */
			if (!sign)
				sign = ' ';
			goto rflag;
		case '#':
			flags |= ALT;
			goto rflag;
		case '*':
			/*
			 * ``A negative field width argument is taken as a
			 * - flag followed by a positive field width.''
			 *	-- ANSI X3J11
			 * They don't exclude field widths read from args.
			 */
			if ((width = va_arg(ap, int)) >= 0)
				goto rflag;
			width = -width;
			/* FALLTHROUGH */
		case '-':
			flags |= LADJUST;
			flags &= ~ZEROPAD; /* '-' disables '0' */
			goto rflag;
		case '+':
			sign = '+';
			goto rflag;
		case '.':
			if ((ch = *fmt++) == '*')
			{
				n = va_arg(ap, int);
				prec = n < 0 ? -1 : n;
				goto rflag;
			}
			n = 0;
			while (yes_digit(ch))
			{
				n = 10 * n + to_digit(ch);
				ch = *fmt++;
			}
			prec = n < 0 ? -1 : n;
			goto reswitch;
		case '0':
			/*
			 * ``Note that 0 is taken as a flag, not as the
			 * beginning of a field width.''
			 *	-- ANSI X3J11
			 */
			if (!(flags & LADJUST))
				flags |= ZEROPAD; /* '-' disables '0' */
			goto rflag;
case '1': case '2': case '3': case '4':
case '5': case '6': case '7': case '8': case '9':
			n = 0;
			do
			{
				n = 10 * n + to_digit(ch);
				ch = *fmt++;
			}
			while (yes_digit(ch));
			width = n;
			goto reswitch;
		case 'h':
			flags |= SHORTINT;
			goto rflag;
		case 'l':
			flags |= LONGINT;
			goto rflag;
		case 'c':
			*(cp = buf) = va_arg(ap, int);
			size = 1;
			sign = '\0';
			break;
		case 'D':
			flags |= LONGINT;
			/*FALLTHROUGH*/
		case 'd':
		case 'i':
			_ulong = SARG();
			if ((long)_ulong < 0)
			{
				_ulong = -_ulong;
				sign = '-';
			}
			base = DEC;
			goto number;
		case 'O':
			flags |= LONGINT;
			/*FALLTHROUGH*/
		case 'o':
			_ulong = UARG();
			base = OCT;
			goto nosign;
		case 'p':
			/*
			 * ``The argument shall be a pointer to void.  The
			 * value of the pointer is converted to a sequence
			 * of printable characters, in an implementation-
			 * defined manner.''
			 *	-- ANSI X3J11
			 */
			/* NOSTRICT */
			_ulong = (unsigned long)va_arg(ap, void *);
			base = HEX;
			flags |= HEXPREFIX;
			ch = 'x';
			goto nosign;
		case 's':
			if ((cp = va_arg(ap, char *)) == NULL)
				cp = "(null)";
			if (prec >= 0)
			{
				/*
				 * can't use strlen; can only look for the
				 * NUL in the first `prec' characters, and
				 * strlen() will go further.
				 */
//					char *p = (char*)memchr(cp, 0, prec);	//jeff wu modify
				char *p = (char*)ComMemFindChar(cp, 0, prec);
				if (p != NULL)
				{
					size = p - cp;
					if (size > prec)
						size = prec;
				}
				else
					size = prec;
			}
			else
				size = STRLEN(cp);
			sign = '\0';
			break;
		case 'U':
			flags |= LONGINT;
			/*FALLTHROUGH*/
		case 'u':
			_ulong = UARG();
			base = DEC;
			goto nosign;
		case 'X':
		case 'x':
			_ulong = UARG();
			base = HEX;
			/* leading 0x/X only if non-zero */
			if (flags & ALT && _ulong != 0)
				flags |= HEXPREFIX;

			/* unsigned conversions */
nosign:			sign = '\0';
			/*
			 * ``... diouXx conversions ... if a precision is
			 * specified, the 0 flag will be ignored.''
			 *	-- ANSI X3J11
			 */
number:			if ((dprec = prec) >= 0)
				flags &= ~ZEROPAD;

			/*
			 * ``The result of converting a zero value with an
			 * explicit precision of zero is no characters.''
			 *	-- ANSI X3J11
			 */
			cp = buf + BUF;
			if (_ulong != 0 || prec != 0)
			{
				char *xdigs; /* digits for [xX] conversion */
				/*
				 * unsigned mod is hard, and unsigned mod
				 * by a constant is easier than that by
				 * a variable; hence this switch.
				 */
				switch (base)
				{
				case OCT:
					do
					{
						*--cp = to_char(_ulong & 7);
						_ulong >>= 3;
					}
					while (_ulong);
					/* handle octal leading 0 */
					if (flags & ALT && *cp != '0')
						*--cp = '0';
					break;

				case DEC:
					/* many numbers are 1 digit */
					while (_ulong >= 10)
					{
						*--cp = to_char(_ulong % 10);
						_ulong /= 10;
					}
					*--cp = to_char(_ulong);
					break;

				case HEX:
					if (ch == 'X')
						xdigs = "0123456789ABCDEF";
					else /* ch == 'x' || ch == 'p' */
						xdigs = "0123456789abcdef";
					do
					{
						*--cp = xdigs[_ulong & 15];
						_ulong >>= 4;
					}
					while (_ulong);
					break;

				default:
					cp = "bug in vform: bad base";
					goto skipsize;
				}
			}
			size = buf + BUF - cp;
skipsize:
			break;
		default:	/* "%?" prints ?, unless ? is NUL */
			if (ch == '\0')
				goto done;
			/* pretend it was %c with argument ch */
			cp = buf;
			*cp = ch;
			size = 1;
			sign = '\0';
			break;
		}

		/*
		 * All reasonable formats wind up here.  At this point,
		 * `cp' points to a string which (if not flags&LADJUST)
		 * should be padded out to `width' places.  If
		 * flags&ZEROPAD, it should first be prefixed by any
		 * sign or other prefix; otherwise, it should be blank
		 * padded before the prefix is emitted.  After any
		 * left-hand padding and prefixing, emit zeroes
		 * required by a decimal [diouxX] precision, then print
		 * the string proper, then emit zeroes required by any
		 * leftover floating precision; finally, if LADJUST,
		 * pad with blanks.
		 */

		/*
		 * compute actual size, so we know how much to pad.
		 */
		fieldsz = size;
		dpad = dprec - size;
		if (dpad < 0)
			dpad = 0;

		if (sign)
			fieldsz++;
		else if (flags & HEXPREFIX)
			fieldsz += 2;
		fieldsz += dpad;

		/* right-adjusting blank padding */
		if ((flags & (LADJUST | ZEROPAD)) == 0)
			PAD_SP(width - fieldsz);

		/* prefix */
		if (sign)
		{
			output(&handle, sign);
		}
		else if (flags & HEXPREFIX)
		{
			output(&handle, '0');
			output(&handle, ch);
		}

		/* right-adjusting zero padding */
		if ((flags & (LADJUST | ZEROPAD)) == ZEROPAD)
			PAD_0(width - fieldsz);

		/* leading zeroes from decimal precision */
		PAD_0(dpad);

		/* the string or number proper */
		PRINT(cp, size);


		/* left-adjusting padding (always blank) */
		if (flags & LADJUST)
			PAD_SP(width - fieldsz);
	}
done:
//		outbyte(0);		//this is the end of string
	/*	__asm("
			mtc0 $0,$23
			li	$9,%0
			mtc0	$9,$24

			la	$9,_counter_save
			lw	$8,0($9)
			slti $9,$8,%1
			bne $9,$0,1f
			nop
			li $8,(%1-1)
		1:
			mtc0	$8,$23		#restore counter
			"
			::"i"(ENA_COMP_FLAG),"i"(COMP_FLAG)
			);*/
	return handle;
}

 void output_to_uart(void **handle, char ch)
{
	outbyte(ch);
}

void libc_printf(const char *format, ...)
{
#ifndef DISABLE_PRINTF
	va_list arg;

	va_start(arg, format);
	libc_vprintf(NULL, (BYTE_OUTPUT)output_to_uart, format, arg);
	va_end(arg);
#endif
}

static void output_to_mem(void **handle, char ch)
{
	char *buffer = (char *)(*handle);
	*buffer = ch;
	*handle = (void **)(++buffer);
}

int sprintf(char *s, const char *format, ...)
{
	va_list arg;

	va_start(arg, format);
	s = libc_vprintf((void *)s, (BYTE_OUTPUT)output_to_mem, format, arg);
	*s = 0;
	va_end(arg);

	return STRLEN(s);
}

#else


#define MAX_WIDTH 10*1024

static unsigned long skip_to(const char *format)
{
	unsigned long nr;
	for (nr = 0; format[nr] && (format[nr] != '%'); ++nr);
	return nr;
}

#define A_WRITE(fn,buf,sz)	((fn)->put((void*)(buf),(sz),(fn)->data))
#define B_WRITE(fn,buf,sz)	{ if ((unsigned long)(sz) > (((unsigned long)(int)(-1))>>1) || len+(int)(sz)<len) return -1; A_WRITE(fn,buf,sz); } while (0)

static const char pad_line[2][16] = { "                ", "0000000000000000", };
static int write_pad(unsigned int* dlen, struct arg_printf* fn, unsigned int len, int padwith)
{
	int nr = 0;
	if ((int)len <= 0) return 0;
	if (*dlen + len < len) return -1;
	for (;len > 15;len -= 16, nr += 16)
	{
		A_WRITE(fn, pad_line[(padwith=='0')?1:0], 16);
	}
	if (len > 0)
	{
		A_WRITE(fn, pad_line[(padwith=='0')?1:0], (unsigned int)len); nr += len;
	}
	*dlen += nr;
	return 0;
}

#ifdef WANT_FLOATING_POINT_IN_PRINTF	
static int copystring(char* buf, int maxlen, const char* s)
{
	int i;
	for (i = 0; i < 3 && i < maxlen; ++i)
		buf[i] = s[i];
	if (i < maxlen)
	{
		buf[i] = 0; ++i;
	}
	return i;
}

int __dtostr(double d, char *buf, unsigned int maxlen, unsigned int prec, unsigned int prec2, int g)
{
#if 1
	union {
		unsigned long long l;
		double d;
	} u = { .d = d };
	/* step 1: extract sign, mantissa and exponent */
	signed long e = ((u.l >> 52) & ((1 << 11) - 1)) - 1023;
#else
#if __BYTE_ORDER == __LITTLE_ENDIAN
	signed long e = (((((unsigned long*) & d)[1]) >> 20) & ((1 << 11) - 1)) - 1023;
#else
	signed long e = (((*((unsigned long*) & d)) >> 20) & ((1 << 11) - 1)) - 1023;
#endif
#endif
	/*  unsigned long long m=u.l & ((1ull<<52)-1); */
	/* step 2: exponent is base 2, compute exponent for base 10 */
	signed long e10;
	/* step 3: calculate 10^e10 */
	unsigned int i;
	double backup = d;
	double tmp;
	char *oldbuf = buf;

	if ((i = isinf(d))) return copystring(buf, maxlen, i > 0 ? "inf" : "-inf");
	if (isnan(d)) return copystring(buf, maxlen, "nan");
	e10 = 1 + (long)(e * 0.30102999566398119802); /* log10(2) */
	/* Wir iterieren von Links bis wir bei 0 sind oder maxlen erreicht
	 * ist.  Wenn maxlen erreicht ist, machen wir das nochmal in
	 * scientific notation.  Wenn dann von prec noch was übrig ist, geben
	 * wir einen Dezimalpunkt aus und geben prec2 Nachkommastellen aus.
	 * Wenn prec2 Null ist, geben wir so viel Stellen aus, wie von prec
	 * noch übrig ist. */
	if (d == 0.0)
	{
		prec2 = prec2 == 0 ? 1 : prec2 + 2;
		prec2 = prec2 > maxlen ? 8 : prec2;
		i = 0;
		if (prec2 && (long long)u.l < 0)
		{
			buf[0] = '-'; ++i;
		}
		for (; i < prec2; ++i) buf[i] = '0';
		buf[buf[0] == '0' ? 1 : 2] = '.'; buf[i] = 0;
		return i;
	}

	if (d < 0.0)
	{
		d = -d; *buf = '-'; --maxlen; ++buf;
	}

	/*
	   Perform rounding. It needs to be done before we generate any
	   digits as the carry could propagate through the whole number.
	*/

	tmp = 0.5;
	for (i = 0; i < prec2; i++)
	{
		tmp *= 0.1;
	}
	d += tmp;

	if (d < 1.0)
	{
		*buf = '0'; --maxlen; ++buf;
	}
	/*  printf("e=%d e10=%d prec=%d\n",e,e10,prec); */
	if (e10 > 0)
	{
		int first = 1;	/* are we about to write the first digit? */
		tmp = 10.0;
		i = e10;
		while (i > 10)
		{
			tmp = tmp * 1e10; i -= 10;
		}
		while (i > 1)
		{
			tmp = tmp * 10; --i;
		}
		/* the number is greater than 1. Iterate through digits before the
		 * decimal point until we reach the decimal point or maxlen is
		 * reached (in which case we switch to scientific notation). */
		while (tmp > 0.9)
		{
			char digit;
			double fraction = d / tmp;
			digit = (int)(fraction);		/* floor() */
			if (!first || digit)
			{
				first = 0;
				*buf = digit + '0'; ++buf;
				if (!maxlen)
				{
					/* use scientific notation */
					int len = __dtostr(backup / tmp, oldbuf, maxlen, prec, prec2, 0);
					int initial = 1;
					if (len == 0) return 0;
					maxlen -= len; buf += len;
					if (maxlen > 0)
					{
						*buf = 'e';
						++buf;
					}
					--maxlen;
					for (len = 1000; len > 0; len /= 10)
					{
						if (e10 >= len || !initial)
						{
							if (maxlen > 0)
							{
								*buf = (e10 / len) + '0';
								++buf;
							}
							--maxlen;
							initial = 0;
							e10 = e10 % len;
						}
					}
					if (maxlen > 0) goto fini;
					return 0;
				}
				d -= digit * tmp;
				--maxlen;
			}
			tmp /= 10.0;
		}
	}
	else
	{
		tmp = 0.1;
	}

	if (buf == oldbuf)
	{
		if (!maxlen) return 0; --maxlen;
		*buf = '0'; ++buf;
	}
	if (prec2 || prec > (unsigned int)(buf - oldbuf) + 1)
	{	/* more digits wanted */
		if (!maxlen) return 0; --maxlen;
		*buf = '.'; ++buf;
		if (g)
		{
			if (prec2) prec = prec2;
			prec -= buf - oldbuf - 1;
		}
		else
		{
			prec -= buf - oldbuf - 1;
			if (prec2) prec = prec2;
		}
		if (prec > maxlen) return 0;
		while (prec > 0)
		{
			char digit;
			double fraction = d / tmp;
			digit = (int)(fraction);		/* floor() */
			*buf = digit + '0'; ++buf;
			d -= digit * tmp;
			tmp /= 10.0;
			--prec;
		}
	}
fini:
	*buf = 0;
	return buf -oldbuf;
}

static double __dtoe(double d, int *pe)
{
	double nd = (d >= 0.0) ? d : -d;
	int e = 0;
	
	int sign = (d >= 0.0) ? 1 : -1;
	if(nd == 0.0)
	{
		e = 0; 
		nd = 0.0;
	}
	else if(nd < 1.0)
	{
		e = 0;
		while(nd < 1.0)
		{
			nd = nd * 10;
			e++;
		}

		e = -e;
	}
	else
	{
		e = 0;
		while(nd >= 10 )
		{
			nd = nd / 10.0;
			e++;

		}
	}

	*pe = e;
	return nd * sign;
}



#endif // WANT_FLOATING_POINT_IN_PRINTF	

static int __lltostr(char *s, int size, unsigned long long i, int base, char UpCase)
{
	char *tmp;
	unsigned int j = 0;

	s[--size] = 0;

	tmp = s + size;

	if ((base == 0) || (base > 36)) base = 10;

	j = 0;
	if (!i)
	{
		*(--tmp) = '0';
		j = 1;
	}

	while ((tmp > s) && (i))
	{
		tmp--;
		if ((*tmp = i % base + '0') > '9') * tmp += (UpCase ? 'A' : 'a') - '9' -1;
		i = i / base;
		j++;
	}
	MEMMOVE(s, tmp, j + 1);

	return j;
}

static int __ltostr(char *s, int size, unsigned long i, int base, char UpCase)
{
	char *tmp;
	unsigned int j = 0;

	s[--size] = 0;

	tmp = s + size;

	if ((base == 0) || (base > 36)) base = 10;

	j = 0;
	if (!i)
	{
		*(--tmp) = '0';
		j = 1;
	}

	while ((tmp > s) && (i))
	{
		tmp--;
		if ((*tmp = i % base + '0') > '9') * tmp += (UpCase ? 'A' : 'a') - '9' -1;
		i = i / base;
		j++;
	}
	MEMMOVE(s, tmp, j + 1);

	return j;
}


static unsigned long int __strtoul(const char *ptr, char **endptr)
{
	int neg = 0;
	unsigned long int v = 0;
	const char* orig;
	const char* nptr = ptr;
	int base = 10;
	while(' ' == (*nptr)) ++nptr;

	if (*nptr == '-')
	{
		neg = 1; nptr++;
	}
	else if (*nptr == '+') ++nptr;
	orig = nptr;
	while (*nptr)
	{
		unsigned char c = *nptr;
		c = (c >= 'a' ? c - 'a' + 10 : c >= 'A' ? c - 'A' + 10 : c <= '9' ? c - '0' : 0xff);
		if ((c >= base)) break;
		v = v * base + c;
		++nptr;
	}
	if (nptr == orig)
	{
		nptr = ptr;
		v = 0;
	}
	if (endptr) *endptr = (char *)nptr;

	return (neg ? -v : v);
}

//#define WANT_FLOATING_POINT_IN_PRINTF	
#define WANT_LONGLONG_PRINTF
#define __WORDSIZE 32
#define WANT_NULL_PRINTF

int __v_printf(struct arg_printf* fn, const char *format, va_list arg_ptr)
{
	unsigned int len = 0;
#ifdef WANT_ERROR_PRINTF
	int _errno = errno;
#endif

	while (*format)
	{
		unsigned long sz = skip_to(format);
		if (sz)
		{
			B_WRITE(fn, format, sz); len += sz;
			format += sz;
		}
		if (*format == '%')
		{
			char buf[128];
			union { char*s; } u_str;
#define s u_str.s

			int retval;
			unsigned char ch, padwith = ' ', precpadwith = ' ';

			char flag_in_sign = 0;
			char flag_upcase = 0;
			char flag_hash = 0;
			char flag_left = 0;
			char flag_space = 0;
			char flag_sign = 0;
			char flag_dot = 0;
			signed char flag_long = 0;

			unsigned int base;
			unsigned int width = 0, preci = 0;

			long number = 0;
#ifdef WANT_LONGLONG_PRINTF
			long long llnumber = 0;
#endif

			++format;
inn_printf:
			switch (ch = *format++)
			{
			case 0:
				return -1;
				break;

				/* FLAGS */
			case '#':
				flag_hash = -1;

			case 'h':
				--flag_long;
				goto inn_printf;
#if __WORDSIZE != 64
			case 'j':
#endif
			case 'q':		/* BSD ... */
			case 'L':
				++flag_long; /* fall through */
#if __WORDSIZE == 64
			case 'j':
#endif
			case 'z':
			case 'l':
				++flag_long;
				goto inn_printf;

			case '-':
				flag_left = 1;
				goto inn_printf;

			case ' ':
				flag_space = 1;
				goto inn_printf;

			case '+':
				flag_sign = 1;
				goto inn_printf;

			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				if (flag_dot) return -1;
				width = strtoul(format - 1, (char**) & s, 10);
				if (width > MAX_WIDTH) return -1;
				if (ch == '0' && !flag_left) padwith = '0';
				format = s;
				goto inn_printf;

			case '*':
			{
				/* A negative field width is taken as a '-' flag followed by
				 * a positive field width
				 **/
				int tmp;
				if ((tmp = va_arg(arg_ptr, int)) < 0)
				{
					flag_left = 1;
					tmp = -tmp;
				}
				if ((width = (unsigned long)tmp) > MAX_WIDTH) return -1;
				goto inn_printf;
			}
			case '.':
				flag_dot = 1;
				if (*format == '*')
				{
					int tmp = va_arg(arg_ptr, int);
					preci = tmp < 0 ? 0 : tmp;
					++format;
				}
				else
				{
					long int tmp = strtol(format, (char**) & s, 10);
					preci = tmp < 0 ? 0 : tmp;
					format = s;
				}
				if (preci > MAX_WIDTH) return -1;
				goto inn_printf;

				/* print a char or % */
			case 'c':
				ch = (char)va_arg(arg_ptr, int);
			case '%':
				B_WRITE(fn, &ch, 1); ++len;
				break;

#ifdef WANT_ERROR_PRINTF
				/* print an error message */
			case 'm':
				s = strerror(_errno);
				sz = STRLEN(s);
				B_WRITE(fn, s, sz); len += sz;
				break;
#endif
				/* print a string */
			case 's':
				s = va_arg(arg_ptr, char *);
#ifdef WANT_NULL_PRINTF
				if (!s) s = "(null)";
#endif
				sz = STRLEN(s);
				if (flag_dot && sz > preci) sz = preci;
				preci = 0;
				flag_dot ^= flag_dot;
				padwith = precpadwith = ' ';

print_out:
				{
					char *sign = s;
					int todo = 0;
					int vs;

					if (! (width || preci) )
					{
						B_WRITE(fn, s, sz); len += sz;
						break;
					}

					if (flag_in_sign) todo = 1;
					if (flag_hash > 0)  todo = flag_hash;
					if (todo)
					{
						s += todo;
						sz -= todo;
						width -= todo;
					}

					/* These are the cases for 1234 or "1234" respectively:
					      %.6u -> "001234"
					      %6u  -> "  1234"
					      %06u -> "001234"
					      %-6u -> "1234  "
					      %.6s -> "1234"
					      %6s  -> "  1234"
					      %06s -> "  1234"
					      %-6s -> "1234  "
					      %6.5u -> " 01234"
					      %6.5s -> "  1234"
					       In this code, for %6.5s, 6 is width, 5 is preci.
					   flag_dot means there was a '.' and preci is set.
					   flag_left means there was a '-'.
					   sz is 4 (strlen("1234")).
					   padwith will be '0' for %06u, ' ' otherwise.
					   precpadwith is '0' for %u, ' ' for %s.
					 */

					if (flag_dot && width == 0) width = preci;
					if (!flag_dot) preci = sz;
					if (!flag_left && padwith == ' ') /* do left-side padding with spaces */
					{
						if (write_pad(&len, fn, width - preci, padwith))
							return -1;
					}
					if (todo)
					{
						B_WRITE(fn, sign, todo);
						len += todo;
					}
					if (!flag_left && padwith != ' ') /* do left-side padding with '0' */
					{
						if (write_pad(&len, fn, width - preci, padwith))
							return -1;
					}
					/* do preci padding */
					if (write_pad(&len, fn, preci - sz, precpadwith))
						return -1;
					/* write actual string */
					B_WRITE(fn, s, sz); len += sz;
					if (flag_left)
					{
						if (write_pad(&len, fn, width - preci, padwith))
							return -1;
					}

					break;
				}

				/* print an integer value */
			case 'b':
				base = 2;
				sz = 0;
				goto num_printf;
			case 'p':
				flag_hash = 2;
				flag_long = 1;
				ch = 'x';
			case 'X':
				flag_upcase = (ch == 'X');
			case 'x':
				base = 16;
				sz = 0;
				if (flag_hash)
				{
					buf[1] = '0';
					buf[2] = ch;
					flag_hash = 2;
					sz = 2;
				}
				if (preci > width) width = preci;
				goto num_printf;
			case 'd':
			case 'i':
				flag_in_sign = 1;
			case 'u':
				base = 10;
				sz = 0;
				goto num_printf;
			case 'o':
				base = 8;
				sz = 0;
				if (flag_hash)
				{
					buf[1] = '0';
					flag_hash = 1;
					++sz;
				}

num_printf:
				s = buf + 1;

				if (flag_long > 0)
				{
#ifdef WANT_LONGLONG_PRINTF
					if (flag_long > 1)
						llnumber = va_arg(arg_ptr, long long);
					else
#endif
						number = va_arg(arg_ptr, long);
				}
				else
				{
					number = va_arg(arg_ptr, int);
					if (sizeof(int) != sizeof(long) && !flag_in_sign)
						number &= ((unsigned int) - 1);
				}

				if (flag_in_sign)
				{
#ifdef WANT_LONGLONG_PRINTF
					if ((flag_long > 1) && (llnumber < 0))
					{
						llnumber = -llnumber;
						flag_in_sign = 2;
					}
					else
#endif
						if (number < 0)
						{
							number = -number;
							flag_in_sign = 2;
						}
				}
				if (flag_long < 0) number &= 0xffff;
				if (flag_long < -1) number &= 0xff;
#ifdef WANT_LONGLONG_PRINTF
				if (flag_long > 1)
					retval = __lltostr(s + sz, sizeof(buf) - 5, (unsigned long long) llnumber, base, flag_upcase);
				else
#endif
					retval = __ltostr(s + sz, sizeof(buf) - 5, (unsigned long) number, base, flag_upcase);

				/* When 0 is printed with an explicit precision 0, the output is empty. */
				if (flag_dot && retval == 1 && s[sz] == '0')
				{
					if (preci == 0 || flag_hash > 0)
					{
						sz = 0;
					}
					flag_hash = 0;
				}
				else sz += retval;

				if (flag_in_sign == 2)
				{
					*(--s) = '-';
					++sz;
				}
				else if ((flag_in_sign) && (flag_sign || flag_space))
				{
					*(--s) = (flag_sign) ? '+' : ' ';
					++sz;
				}
				else flag_in_sign = 0;

				precpadwith = '0';

				goto print_out;

#ifdef WANT_FLOATING_POINT_IN_PRINTF
				/* print a floating point value */
			case 'f':
			case 'g':
			{
				int g = (ch == 'g');
				double d = va_arg(arg_ptr, double);
				s = buf + 1;
				if (width == 0) width = 1;
				if (!flag_dot) preci = 6;
				if (flag_sign || d < + 0.0) flag_in_sign = 1;

				sz = __dtostr(d, s, sizeof(buf) - 1, width, preci, g);

				if (flag_dot)
				{
					char *tmp;
					if ((tmp = strchr(s, '.')))
					{
						if (preci || flag_hash) ++tmp;
						while (preci > 0 && *++tmp) --preci;
						*tmp = 0;
					}
					else if (flag_hash)
					{
						s[sz] = '.';
						s[++sz] = '\0';
					}
				}

				if (g)
				{
					char *tmp, *tmp1;	/* boy, is _this_ ugly! */
					if ((tmp = strchr(s, '.')))
					{
						tmp1 = strchr(tmp, 'e');
						while (*tmp) ++tmp;
						if (tmp1) tmp = tmp1;
						while (*--tmp == '0') ;
						if (*tmp != '.') ++tmp;
						*tmp = 0;
						if (tmp1) strcpy(tmp, tmp1);
					}
				}

				if ((flag_sign || flag_space) && d >= 0)
				{
					*(--s) = (flag_sign) ? '+' : ' ';
					++sz;
				}

				sz = strlen(s);
				if (width < sz) width = sz;
				precpadwith = '0';
				flag_dot = 0;
				flag_hash = 0;
				goto print_out;
			}
			case 'e':
			case 'E':
			{				
				double d = va_arg(arg_ptr, double);
				s = buf + 1;
				if (width == 0) width = 1;
				if (!flag_dot) preci = 6;
				if (flag_sign || d < + 0.0) flag_in_sign = 1;
				
				int exp;
				double nd;
				nd = __dtoe(d, &exp);
				sz = __dtostr(nd, s, sizeof(buf) - 1, width, preci, 0);

				if (flag_dot)
				{
					char *tmp;
					if ((tmp = strchr(s, '.')))
					{
						if (preci || flag_hash) ++tmp;
						while (preci > 0 && *++tmp) --preci;
						*tmp = 0;
					}
					else if (flag_hash)
					{
						s[sz] = '.';
						s[++sz] = '\0';
					}
				}

				if ((flag_sign || flag_space) && d >= 0)
				{
					*(--s) = (flag_sign) ? '+' : ' ';
					++sz;
				}

				sz = STRLEN(s);
				//append the exponent part
				s[sz++] = ch; s[sz] = 0;
				char sexp[16];
				int esign = 1;
				if(exp < 0)
				{
					s[sz++] = '-'; s[sz] = 0;
					exp = -exp;
				}
				else
				{
					s[sz++] = '+'; s[sz] = 0;
				}
				__ltostr(sexp, 15, exp, 10, 0);
				
				int sl = STRLEN(sexp);
				int j = 0;
				for(j = 0; j < 3 - sl; j++)
				{
					s[sz++] = '0'; s[sz] = 0;
				}
				strcat(s, sexp);
				sz = STRLEN(s);
				if (width < sz) width = sz;
				precpadwith = '0';
				flag_dot = 0;
				flag_hash = 0;
				goto print_out;
			}
			
#endif

			default:
				break;
			}
		}
	}
	return len;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////
//sprintf serial functions

struct str_data
{
	char* str;
	unsigned int len;
	unsigned int size;
};

static int swrite(void*ptr, unsigned int nmemb, struct str_data* sd)
{
	unsigned int tmp = sd->size - sd->len;
	if (tmp > 0)
	{
		unsigned int len = nmemb;
		if (len > tmp) len = tmp;
		if (sd->str)
		{
			MEMCPY(sd->str + sd->len, ptr, len);
			sd->str[sd->len+len] = 0;
		}
		sd->len += len;
	}
	return nmemb;
}

int vsnprintf(char* str, unsigned int size, const char *format, va_list arg_ptr)
{
	int n;
	struct str_data sd =
	{
		str, 0, size ? size - 1 : 0
	};
	struct arg_printf ap =
	{
		&sd, (int(*)(void*, unsigned int, void*)) swrite
	};
	n = __v_printf(&ap, format, arg_ptr);
	if (str && size && n >= 0)
	{
		if (size != (unsigned int) - 1 && ((unsigned int)n >= size)) str[size-1] = 0;
		else str[n] = 0;
	}
	return n;
}



int sprintf(char *dest, const char *format, ...)
{
	int n;
	va_list arg_ptr;
	va_start(arg_ptr, format);
	n = vsnprintf(dest, (unsigned int) - 1, format, arg_ptr);
	va_end (arg_ptr);
	return n;
}


//////////////////////////////////////////////////////////////
// printfs serial functions
static int __stdio_outs(const char *str, int len, void* ignore)
{
	int i;
	for (i = 0;i < len;i++)
	{
		if ('\n' == *str)
			outbyte('\r');
		outbyte(*str++);
	}
	return 1;
}

int vprintf(const char *format, va_list ap)
{
	struct arg_printf _ap =
	{
		0, (int(*)(void*, unsigned int, void*)) __stdio_outs
	};
	return __v_printf(&_ap, format, ap);
}


void libc_printf(const char *format, ...)
{
#ifndef DISABLE_PRINTF
	int n;
	va_list arg_ptr;
	va_start(arg_ptr, format);
	n = vprintf(format, arg_ptr);
	va_end(arg_ptr);
	//return n;
#endif
}

int snprintf(char *str, unsigned int size, const char *format, ...)
{
	int n;
	va_list arg_ptr;
	va_start(arg_ptr, format);
	n = vsnprintf(str, size, format, arg_ptr);
	va_end (arg_ptr);
	return n;
}

int vsprintf(char *dest, const char *format, va_list arg_ptr)
{
	return vsnprintf(dest, (size_t) - 1, format, arg_ptr);
}

int printf(const char *format, ...) __attribute__((alias("libc_printf")));
#endif




