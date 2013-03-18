/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2004 (C)
 *
 *  File: lib_hde.h
 *
 *  Description: This file implement the hardware display engine on
 *               ALi M3327 chip.
 *
 *  History:
 *      Date		Author		Version  	Comment
 *      ==========	==========  =======		=============================		
 *  1.  2004.10.21	Tom Gao		0.1.000		Initial
 ****************************************************************************/

#ifndef __LIB_PUB27_HDE_H__
#define __LIB_PUB27_HDE_H__

#include <basic_types.h>

#define VIEW_MODE_FULL			0
#define VIEW_MODE_PREVIEW		1
#define VIEW_MODE_MULTI		2
#define VIEW_MODE_SML			3


void hde_back_color(UINT8, UINT8, UINT8);
void hde_set_mode(UINT32);
void hde_config_preview_window(UINT16 x, UINT16 y, \
      UINT16 w, UINT16 h, BOOL IsPAL);
UINT32 hde_get_mode(void);

#endif /* __LIB_PUB27_HDE_H__ */

