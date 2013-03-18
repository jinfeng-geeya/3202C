/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2002 Copyright (C)
 *
 *  File: sec_pat.h
 *
 *  Description: the  head file of data types and functions for pat section.
 *               
 *  History:
 *      Date        Author         Version   Comment
 *      ====        ======         =======   =======
 *  1.  2002.12.20  David Wang     0.1.000   Initial
 *  2.  2003.02.11  David Wang	   0.1.001   Modification for new sw tree
 ****************************************************************************/

#ifndef _SEC_PAT_H_
#define _SEC_PAT_H_

#include <types.h>
//#include <api/libtsi/dmx_types.h>

#define	PAT_PID                         0x0000	/* program association */

typedef struct pat_info
{
    UINT16 prog_number;
    UINT16 pid;
    struct pat_info *next;
}PAT_INFO;

#ifdef __cplusplus
extern "C"
{
#endif

UINT16 pat_get_xstream_id(UINT8 *buff,UINT16 buff_len);

PAT_INFO * pat_get_pmt_list(UINT8 *buff,INT16 buff_len); 

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*_SEC_PAT_H_*/


