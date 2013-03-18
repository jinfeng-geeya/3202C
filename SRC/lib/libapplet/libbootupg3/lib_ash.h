/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2003 Copyright (C)
*
*    File:    lib_ash.h
*
*    Description:    This file contains all globe micros and functions declare
*		             of ALi shell moduler.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	Jul.22.2003       Justin Wu       Ver 0.1    Create file.
*	2.
*****************************************************************************/

#ifndef __LIB_ASH_H__
#define __LIB_ASH_H__

#include <types.h>
#include <bus/sci/sci.h>

extern UINT32 boot_uart_id;

//#define TRANSFER_FORMAT3_ONLY

#define LIB_ASH_COMMAND_NUM     9
#define LIB_ASH_COMMAND_LST			8
#define LIB_ASH_COMMAND_LEN			128

#define LIB_ASH_COMMAND_MAX_NUM     16
extern UINT32 lib_ash_command_num;

#ifdef DTTM
#define LIB_DIAGNOSTIC_COMMAND_NUM     3
#define LIB_DOWNLOAD_COMMAND_NUM     10
#endif

#define LIB_ASH_OC(a)			sci_write(boot_uart_id, a)
#define LIB_ASH_IC()			sci_read(boot_uart_id)

struct ash_cmd
{
    unsigned char *command;
    UINT32 (*handler)(unsigned int argc, unsigned char *argv[]);
};

extern struct ash_cmd lib_ash_command[];
#ifndef TRANSFER_FORMAT3_ONLY
	#define LIB_ASH_OLDCOMMAND_NUM     10
	extern struct ash_cmd lib_ash_command_old[];
#endif
#ifdef DTTM
extern struct ash_cmd lib_diag_command[];
extern struct ash_cmd lib_download_command[];
#endif

#ifdef __cplusplus
extern "C"
{
#endif

extern void lib_ash_shell();
extern void upgrade_buf_addr_set(unsigned char *image_addr, unsigned char *zip_addr);	

 typedef UINT32(*ash_cmd_handler)(unsigned int argc, unsigned char *argv[]);
extern BOOL cmd_reg_callback(struct ash_cmd cmd);

#ifdef __cplusplus
}
#endif

#endif /* __LIB_ASH_H__ */
