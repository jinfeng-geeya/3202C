/*
 * PSI_DB.H
 *
 * Interfacing function for PSI and DATABASE conversions.
 *
 * History
 * 1. 20060516	Zhengdao Li	0.0.1		Initialize.
 */
#ifndef __PSI_DB_H__
#define __PSI_DB_H__

#include <types.h>
#include <sys_config.h>

#include <api/libtsi/db_3l.h>

#include <api/libtsi/si_types.h>

#if(defined( _MHEG5_ENABLE_) ||defined(_LCN_ENABLE_) ||defined(_MHEG5_V20_ENABLE_))
#define INVALID_LCN_NUM		0xFFFF
#endif

typedef INT32 (*prog_node_return)(P_NODE *node );


/*
 * name		: pg2db
 * description	: convert PSI program information into database node.
 * parameter	: 2
 * @pg		: the database node.
 * @info	: the PSI program information.
 * return value	: INT32
 * @SI_SUCCESS
 */
INT32 pd_pg2db(P_NODE *pg, PROG_INFO *info);

/*
 * name		: update_pids
 * description	: copy all PID information from source database node to destination.
 * parameter	: 2
 * @dest	: the target database node.
 * @src		: the source database node.
 * return value	: INT32
 * @SI_SUCCESS
 */
INT32 pd_update_pids(P_NODE *dest, P_NODE *src);

/*
 * name		: update_name
 * description	: copy all name information from source database node to destination.
 * parameter	: 2
 * @dest	: the target database node.
 * @src		: the source database node.
 * return value : INT32
 * @SI_SUCCESS.
 */
INT32 pd_update_name(P_NODE *dest, P_NODE *src);

INT32 prog_callback_register(prog_node_return callback);
INT32 prog_callback_unregister( );
UINT32 get_prog_flg();

#endif /* __PSI_DB_H__ */

