/**
 * $Id: s3_portable_types.h,v 1.6 2005/10/31 09:53:14 ehietbrink Exp $
 *
 * Copyright © 2003-2005 Irdeto Access B.V.
 *
 * This file and the information contained herein are the subject of copyright
 * and intellectual property rights under international convention. All rights
 * reserved. No part of this file may be reproduced, stored in a retrieval
 * system or transmitted in any form by any means, electronic, mechanical or
 * optical, in whole or in part, without the prior written permission of Irdeto
 * Access B.V.
 *
 * This file is part of the SoftCell 3 library integration pack.
 **/

/**
 * As far as possible these types are target independent. Where needed
 * definitions for a compiler have to be made.
 * Floating point types are not supported as they are non-portable.
 * Currently there is no support to determine endian type of the compiler.
 *
 * All types specify in their type the required size. These sizes must be met.
 * All types are defined in the form:
 *         ia_[u|s]<type>[<size>]
 * where:
 *        - u indicates unsigned, s indicates signed, if omitted it will compiler dependant or defined by the type.
 *        - <type> indicates the type. Options are: char, int, word and bool
 *        - <size> indicates the size of the type (in bits). Options are: 8, 16 and 32
 **/

#if !defined( S3_PORTABLE_TYPES_H )
#define S3_PORTABLE_TYPES_H

/* Now for the still not defined integers. */
#if !defined( IA_INT_TYPES_DEFINED )
	/* integer types. */
#	define  IA_8_BIT_TYPE	char  /* Type char must be 8 bits long. */
#	define  IA_16_BIT_TYPE	short int  /* Type short int must be 16 bits long. */
#	define  IA_32_BIT_TYPE	long int  /* Type long int must be 32 bits long. */
#	define IA_INT_TYPES_DEFINED 1
#endif /* !defined( IA_INT_TYPES_DEFINED ) */

/* Signed and unsigned int types. */
typedef IA_8_BIT_TYPE ia_int8;
typedef unsigned IA_8_BIT_TYPE ia_uint8;
typedef signed IA_8_BIT_TYPE ia_sint8;
typedef IA_16_BIT_TYPE ia_int16;
typedef unsigned IA_16_BIT_TYPE ia_uint16;
typedef signed IA_16_BIT_TYPE ia_sint16;
typedef IA_32_BIT_TYPE ia_int32;
typedef unsigned IA_32_BIT_TYPE ia_uint32;
typedef signed IA_32_BIT_TYPE ia_sint32;

/* Char type (always signed). */
typedef signed char ia_char;

/* Unsigned byte and word types (always unsigned). */
typedef ia_uint8 ia_byte;
typedef ia_uint16 ia_word16;
typedef ia_uint32 ia_word32;

/**
 * Generic type for function return values.
 * Lower range is reserved for generic return values.
 * Upper range, starting with IA_FIRST_USER_RESULT_VALUE can be used for
 * user (class, method, virtual driver, etc.) define values.
 **/
typedef ia_word16 ia_result;

/* Generic function return values for ia_result type. */
enum
{
	/* The function has failed with an unspecified error. Meaning may depend on function. */
	IA_FAIL = 1,

	/* The function has succeeded. All is Ok. */
	IA_SUCCESS,

	/* The function has failed because a buffer or queue was full. */
	IA_FULL,

	/* The function has failed due to an out of range of some parameter. */
	IA_INVALID_PARAMETER,

	/* A task could not be created. Out of resources. */
	/* IA_TASK_ERROR, */

	/* A requested or needed resource was not available or a given resource handle is invalid. */
	/* IA_RESOURCE_ERROR, */

	/* User define values start from here. */
	IA_FIRST_USER_RESULT_VALUE = 0x0100
};


/* For boolean values. */
typedef enum
{
	IA_TRUE = 1,
	IA_FALSE
} ia_bool;

#define IA_YES	IA_TRUE
#define IA_NO	IA_FALSE

#define IA_NOT( value ) ( ( value ) == IA_TRUE ? IA_FALSE : IA_TRUE )

#endif /* !defined( S3_PORTABLE_TYPES_H ) */
