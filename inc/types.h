/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2002 Copyright (C)
 *
 *  File: types.h
 *
 *  Description: This file define the common data types which may be used
 *               throughout the project.
 *  History:
 *      Date        Author      Version Comment
 *      ====        ======      ======= =======
 *  1.  2002.10.28  Liu Lan     0.1.000 Initial
 *  2.  2006.1.13   Justin Wu   Clean up.
 ****************************************************************************/
#ifndef _TYPES_H_
#define _TYPES_H_

#include <basic_types.h>

/*
 * union which converts two 8bit values into 16bit value.
 */
typedef	union
{
	UINT16	uint_16;
	struct
	{
		UINT8	uint80;	/* LSB */
		UINT8	uint81; /* MSB */
    } uint_8;
} UINT16TO8;

/*
 * union which converts two 16bit values into 32bit value.
 */
typedef	union
{
	UINT32	uint_32;
	struct
	{
		UINT16	low_16;
		UINT16	hi_16;
    } uint_16;
} UINT32TO16;


/* Support macros */
#define itemsof(a)      (sizeof(a) / sizeof(a[0]))

#endif
