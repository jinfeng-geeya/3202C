/**********************************************************************
*    Copyright (C)2011 Ali Corporation. All Rights Reserved.
*
*    File:    This file contains log function.
*
*    Description:    Header file of log function.
*    History:
*       Date            Author        Version          Reason
*    ============   =============   =========    =================
*     2011-12-16       Lorin          0.0.1          Added
************************************************************************/
#ifndef	__UTIL_LOG_H
#define __UTIL_LOG_H

/*
 * Notice: 如果需要添加新的模块，需要同时修改本文件中的两个地方:
 * 1) 'enum amslog_mod_e'中的'add here'前一行
 * 2) 'static amslog_t amslog[LOG_NUM+1] ='中的'add here'前一行
 */


#if	(SYS_OS_MODULE != ALI_TDS2&&SYS_OS_MODULE != NO_OS)
#include <sys/syslog.h>
#endif
#include <basic_types.h>


/* temp for open log macro */
#define AMS_LOG 1

/* Default log level is error messages only. */
#define DEFAULT_LOG_LEVEL        LOG_ERR
/* Default log destination is standard error */
#if	(SYS_OS_MODULE != ALI_TDS2&&SYS_OS_MODULE != NO_OS)
#define DEFAULT_LOG_DESTINATION  LOG_DEST_STDERR
#else
#define DEFAULT_LOG_DESTINATION  LOG_DEST_TDS
#endif

/* Maxmimu length of a single log line. */
#define MAX_LOG_LINE_LENGTH      512

/* Maxmimu length of a single log name. */
#define MAX_LOG_NAME_LENGTH      12

#if	(SYS_OS_MODULE == ALI_TDS2||SYS_OS_MODULE == NO_OS)
#define LOG_EMERG       0       /* system is unusable */
#define LOG_ALERT       1       /* action must be taken immediately */
#define LOG_CRIT        2       /* critical conditions */
#define LOG_ERR         3       /* error conditions */
#define LOG_WARNING     4       /* warning conditions */
#define LOG_NOTICE      5       /* normal but significant condition */
#define LOG_INFO        6       /* informational */
#define LOG_DEBUG       7       /* debug-level messages */
#endif

typedef enum
{
#if	(SYS_OS_MODULE != ALI_TDS2&&SYS_OS_MODULE != NO_OS)
	LOG_DEST_STDERR  = 1,  /**< Message output to stderr. */
	LOG_DEST_SYSLOG  = 2,  /**< Message output to syslog. */
	LOG_DEST_TELNET  = 3,  /**< Message output to telnet clients. */
#else
	LOG_DEST_TDS     = 4   /**< Message output to TDS. */
#endif
} amslog_dest_e;

typedef struct
{
	char mod_name[MAX_LOG_NAME_LENGTH];
	UINT8 log_level;
} amslog_t;

typedef enum
{
	LOG_APP = 0, /* Log for application */
	LOG_AMS,     /* Log for AMS(other modules) */
	LOG_EPG,     /* Log for EPG module */
	LOG_DB,     /* Log for DB module */
	//, add here
	LOG_NUM = 32
} amslog_mod_e;


#ifdef AMS_LOG
#define amslog_error(args...)	amslog_log(LOG_ERR, __FUNCTION__, __LINE__, args)
#define amslog_notice(args...)	amslog_log(LOG_NOTICE, __FUNCTION__, __LINE__, args)
#define amslog_debug(args...)	amslog_log(LOG_DEBUG, __FUNCTION__, __LINE__, args)
#else
#define amslog_error(args...)
#define amslog_notice(args...)
#define amslog_debug(args...)
#endif

// *****************************************************************
// Summary:
//     Print a log message.
// Description:
//     The function print a log message according the module & level.
// Returns:
//     None
// Parameters:
//     level :      [in] Log level, now support 3 levels of LOG_DEBUG/LOG_NOTICE/LOG_ERR
//     func :     [in] function name calling this log funcion.
//     line_num :  [in] line number of the file calling this log function.
//     log_mod :  [in] Module of calling this log function. see amslog_mod_e.
//	   pFmt : [in] log messages.
// 
// See Also:
//		amslog_error, amslog_notice, amslog_debug
//
void amslog_log(UINT8 level, const char *func, UINT32 line_num,
		amslog_mod_e log_mod, const char *pFmt, ... );
void amslog_init(void);
RET_CODE amslog_reg(INT32 mod_index, amslog_t *plog);
void amslog_cleanup(void);
void amslog_set_level(amslog_mod_e log_mod, UINT8 log_level);
UINT8 amslog_get_level(amslog_mod_e log_mod);
void amslog_set_dest(amslog_dest_e dest);
amslog_dest_e amslog_get_dest(void);

#endif	/* __UTIL_LOG_H */

