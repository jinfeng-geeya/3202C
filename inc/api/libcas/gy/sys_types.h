/**************************************************************************
  * Description
  *   This file define all basic data types used in this software tree.
  *
  * Remarks
  *   None.
  *
  * Bugs
  *   None.
  *
  * TODO
  *     * None
  *
  * History
  *   <table>
  *   \Author     Date         Change Description
  *   ----------  -----------  -------------------
  *   Justin Wu   2006.10.17   Initialize.
  *   panzheng    2010.02.05   Add system hardware resource descriptor.   
  *   </table>
  *
  **************************************************************************/
#ifndef __SYS_TYPES_H__
#define __SYS_TYPES_H__
#include <sys_define.h>

#ifdef __SYS_ENV_WIN32__
#include <basetsd.h>
#endif

/* Null pointer type */
#ifndef NULL
#if defined(__cplusplus)
#define NULL 				0
#else
#define NULL 				((void *)0)
#endif // __cplusplus
#endif

/* Signed integer, weight is 8 bits. In most case it equal to char. */
typedef signed char			INT8;
/* Unsigned integer, weight is 8 bits. In most case it equal to unsigned char. */
typedef unsigned char		UINT8;

/* Signed integer, weight is 16 bits. In most case it equal to short. */
typedef signed short		INT16;
/* Unsigned integer, weight is 16 bits. In most case it equal to unsigned short. */
typedef unsigned short		UINT16;


#ifndef _BASETSD_H_ /* Windows defined following */
#ifndef _BASETSD_H /* MinGW defined following */
/* Signed integer, weight is 32 bits. In most case it equal to int or long. */
typedef signed int			INT32;
/* Unsigned integer, weight is 32 bits. In most case it equal to unsigned int or unsigned long. */
typedef unsigned int		UINT32;
#endif	/* ndef _BASETSD_H */
#endif	/* ndef _BASETSD_H_ */

/* Boolean type, the value should be FALSE and TRUE.
   Recommand replace BOOL by RET_CODE as return value. */
typedef int					BOOL;
#ifndef FALSE
#define	FALSE				(0)				/* Boolean false */
#define	TRUE				(!FALSE)		/* Boolean false */
#endif


/* Return code type. Please reference return code macro for the values. */
typedef INT32				RET_CODE;
#define SUCCESS				((INT32) 0)		/* Success return */
#define ERR_FAILURE			((INT32)-101)	/* Fail for common reason */
#define ERR_TIMEOUT			((INT32)-102)   /* Fail for waiting timeout */
#define ERR_PARAM			((INT32)-103)	/* Fail for function param invalid */
#define ERR_STATUS			((INT32)-104)   /* Fail for module status invalid */
#define ERR_BUSY			((INT32)-105)	/* Fail for module busy */
#define ERR_NO_MEM			((INT32)-106)	/* Fail for no enough memory */
#define ERR_NO_RSRC			((INT32)-107)	/* Fail for no enough resource */
#define ERR_HARDWARE		((INT32)-108)	/* Fail for hardware error */
#define ERR_NOFEATURE		((INT32)-109)	/* Fail for feature not support */
#define ERR_RESET			((INT32)-110)	/* Fail for device is reset */
#define ERR_DEV_TYPE		((INT32)-111)	/* Not support device type */

/*
 * Union which converts two 8bit values into 16bit value.
 */
typedef	union {
	UINT16	uint_16;
	struct {
		UINT8	low_8;		/* LSB */
		UINT8	hig_8;		/* MSB */
    } uint_8;
} UINT16TO8;

/*
 * Union which converts two 16bit values into 32bit value.
 */
typedef	union {
	UINT32	uint_32;
	struct {
		UINT16	low_16;		/* LSW */
		UINT16	hig_16;		/* MSW */
    } uint_16;
} UINT32TO16;

/*
 * Union which converts two 16bit values into 32bit value.
 */
typedef	union {
	UINT32	uint_32;
	struct {
		UINT8	low_80;		/* LSB0 */
		UINT8	low_81;		/* LSB1 */
		UINT8	hig_80;		/* MSB0 */
		UINT8	hig_81;		/* MSB1 */
    } uint_8;
} UINT32TO8;

/* Array data type support operation */
#define itemsof(a)      	(sizeof(a) / sizeof(a[0]))

/* System hardware resource descriptor */
enum sys_hw_rsc_type {
	SYS_HW_MEM_RSC_SEG = 0,
	SYS_HW_BIT_RSC_SEG,
	SYS_HW_IRQ_RSC_SEG
};

#define SYS_HW_RSC_HEAD \
	enum sys_hw_rsc_type type;\
	const char* name;\
	struct sys_hw_rsc* next


/**
 * system hardware resource descriptor base class
 */
struct sys_hw_rsc {
	SYS_HW_RSC_HEAD;
};

/**
 * system hardware resource -- memory segment descriptor
 */
struct sys_hw_rsc_mem {
	SYS_HW_RSC_HEAD;
	UINT32 head_addr;
	UINT32 bytes_num;
};

/**
 * system hardware resource -- bit segment descriptor
 */
struct sys_hw_rcs_bit {
	SYS_HW_RSC_HEAD;
	UINT32 addr;
	UINT16 head_bit;
	UINT16 bits_num;
};

/**
 * system hardware resource -- irq segment descriptor
 */
struct sys_hw_rsc_irq {
	SYS_HW_RSC_HEAD;
	UINT32 head_irq;
	UINT32 irqs_num;
};

#endif	/*__SYS_TYPES_H__ */

