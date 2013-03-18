/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2002 Copyright (C)
 *
 *  File: sec_sdt.h
 *
 *  Description: the head file of data types and functions for sdt section.
 *               
 *  History:
 *      Date        Author         Version   Comment
 *      ====        ======         =======   =======
 *  1.  2003.01.13  David Wang     0.1.000   Initial
 *  2.  2003.02.11  David Wang	   0.1.001   Modification for new sw tree
 *  3.  2005.01.25  Zhengdao Li	   0.1.002   modify return value to UINT8
 ****************************************************************************/

#ifndef _SEC_SDT_H_
#define _SEC_SDT_H_

#include <types.h>
#include <sys_config.h>

#include <api/libtsi/db_3l.h>

#define	SDT_PID                         	0x0011	/* service description */
	#ifdef DB_USE_UNICODE_STRING
	#define SDT_DEFAULT_NAME				"\0N\0o\0 \0N\0a\0m\0e\0"
	#define SDT_DEFAULT_PROV_NAME		"\0U\0n\0k\0n\0o\0w\0n\0 \0P\0P\0"
	#else
	#define SDT_DEFAULT_NAME				"No Name"
	#define SDT_DEFAULT_PROV_NAME			"Unknown PP"
	#endif
//#define	MAX_SERVICE_NAME_LENGTH			15
#define SDT_ACTUAL_TS_TABLE_ID			0x42
#define SDT_OTHER_TS_TABLE_ID			0x46
#define	SERVICE_DESC				0x48

/*
 * name		: sdt_get_service_name
 * description	:
 * parameter	:
 * return value :
 *	0	: SDT said the program is free.
 *	1	: SDT said the program is CA-scrambled.
 */
UINT8 sdt_get_service_name(UINT8 *buff,UINT16 buff_len,UINT8 *service_name, 
#if (SERVICE_PROVIDER_NAME_OPTION>0)	
			UINT8 *service_provider_name,
#endif			
			UINT8 *service_type, UINT16 service_id);

#endif /* _SEC_SDT_H_ */



