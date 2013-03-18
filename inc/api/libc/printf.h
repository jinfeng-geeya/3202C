/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2003 Copyright (C)
*
*    File:    printf.h
*
*    Description:    This file contains all functions definition
*		             of LIBC printf.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	2002.??.??      Justin Wu       Ver 0.1     Create file.
*   2.  2003.12.05      Justin Wu       Ver 0.2     Support wDVD Standard TDS
*													-- Common Service:
*													.ERRMSG & DEBUGPRINTF
*													.ASSERTMSG & ASSERT_ADDR
*****************************************************************************/


#ifndef _LIB_C_PRINTF_H
#define _LIB_C_PRINTF_H

#include <sys_config.h>
#include <osal/osal.h>
#include <hal/machine.h>
#include <stdarg.h>



//================================================================================================
//printf group functions
void soc_printf(const char *fmt, ...);
void libc_printf(const char *fmt, ...);


struct arg_printf
{
	void *data;
	int (*put)(void*, unsigned int, void*);
};


int __v_printf(struct arg_printf* fn, const char *format, va_list arg_ptr);

//int vsnprintf(char* str, unsigned int size, const char *format, va_list arg_ptr);
int sprintf(char *dest, const char *format, ...);
int vprintf(const char *format, va_list ap);
//int snprintf(char *str, unsigned int size, const char *format, ...);
char* aprintf(const char *format, ...);
char* vaprintf(const char *fmt, va_list arg_ptr);
//int asprintf(char **s, const char *format, ...);



struct arg_scanf {
  void *data;
  int (*getch)(void*);
  int (*putch)(int,void*);
};

int __v_scanf(struct arg_scanf* fn, const char *format, va_list arg_ptr);

int vsscanf(const char* str, const char* format, va_list arg_ptr);
int sscanf(const char *str, const char *format, ...);


//================================================================================================
//macro define for the printf functons
#define FIXED_PRINTF			libc_printf
#define ERRMSG					DEBUGPRINTF

#ifdef _DEBUG_VERSION_
#ifdef _DEBUG_PRINTF_
#define PRINTF					libc_printf
#define DEBUGPRINTF				libc_printf
#else
#define PRINTF(...)				do{}while(0)
#define DEBUGPRINTF(...)		do{}while(0)
#endif  /* _DEBUG_PRINTF_ */

#else	/* _DEBUG_VERSION_ */
//extern PRINTF(const char *fmt, ...);	/* This is for Able project only */
#ifdef _DEBUG_PRINTF_
#define PRINTF					libc_printf
#define DEBUGPRINTF				libc_printf
#else
#define PRINTF(...)				do{}while(0)
#define DEBUGPRINTF(...)		do{}while(0)
#endif  /* _DEBUG_PRINTF_ */

#define ENTER_FUNCTION	DEBUGPRINTF("Function %s: Begin.\n",__FUNCTION__)
#define LEAVE_FUNCTION	DEBUGPRINTF("Function %s: End From %d line. \n", __FUNCTION__, __LINE__)

#endif	/* _DEBUG_VERSION_ */


//================================================================================================
//ASSERT defines
#ifdef ASSERT
#undef ASSERT
#endif

#ifdef _DEBUG_PRINTF_
#define	ASSERT(expression)				\
	{									\
		if (!(expression))				\
		{								\
			DEBUGPRINTF("assertion(%s) failed: file \"%s\", line %d\n",	\
				#expression, __FILE__, __LINE__);	\
			SDBBP();					\
		}								\
	}

#define ASSERTMSG(expression, message)	\
	{									\
		if (!(expression))				\
		{								\
			DEBUGPRINTF("assertion(%s) failed: file \"%s\", line %d, message %s\n", \
				#expression, __FILE__, __LINE__, #message);	\
			SDBBP();					\
		}								\
	}

#define	ASSERT_ADDR(address)			\
	{									\
		if((UINT32)(address)<0x010||((UINT32)(address)>=0x0180&&(UINT32)(address)<0x01a0)) \
		{								\
			DEBUGPRINTF("assertion address  failed: [%s]=[0x%08x] in file \"%s\", line %d\n", \
					#address,(address),__FILE__, __LINE__); \
			SDBBP();					\
		}								\
	}
#else

#ifdef _ALI_SYSDBG_ENABLE_
#define	ASSERT(expression)				\
	{									\
		if (!(expression))				\
		{								\
			sysdbg_out_cb("assertion(%s) failed: file \"%s\", line %d\n",	\
				#expression, __FILE__, __LINE__);	\
			SDBBP();					\
		}								\
	}

#define ASSERTMSG(expression, message)	\
	{									\
		if (!(expression))				\
		{								\
			sysdbg_out_cb("assertion(%s) failed: file \"%s\", line %d, message %s\n", \
				#expression, __FILE__, __LINE__, #message);	\
			SDBBP();					\
		}								\
	}

#define	ASSERT_ADDR(address)			\
	{									\
		if((UINT32)(address)<0x010||((UINT32)(address)>=0x0180&&(UINT32)(address)<0x01a0)) \
		{								\
			sysdbg_out_cb("assertion address  failed: [%s]=[0x%08x] in file \"%s\", line %d\n", \
					#address,(address),__FILE__, __LINE__); \
			SDBBP();					\
		}								\
	}
#else

#define	ASSERT(expression)				\
	{									\
		if (!(expression))				\
		{								\
			sys_watchdog_reboot();		\
		}								\
	}

#define ASSERTMSG(expression, message)	\
	{									\
		if (!(expression))				\
		{								\
			sys_watchdog_reboot();		\
		}								\
	}

#define	ASSERT_ADDR(address)			\
	{									\
		if((UINT32)(address)<0x010||((UINT32)(address)>=0x0180&&(UINT32)(address)<0x01a0)) \
		{								\
			sys_watchdog_reboot();		\
		}								\
	}
#endif /*_ALI_SYSDBG_ENABLE_*/

#endif /*_DEBUG_PRINTF_*/

#endif /* _LIB_C_PRINTF_H */
