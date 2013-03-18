/*********************************************************************
    Copyright (c) 2005 Embedded Internet Solutions, Inc
    All rights reserved. You are not allowed to copy or distribute
    the code without permission.
    There are the Demux Porting APIs needed by iPanel MiddleWare. 
    
    Note: the "int" in the file is 32bits
    
    $ver0.0.0.1 $author Zouxianyun 2005/04/28
*********************************************************************/

/*****************************************************************************/
/**
 *
 * @file ipanel_demux.h
 *
 * Implements porting layer for ipanel STB middle layer running on top of ALI
 * chips.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00a joy  08/10/11 First release
 * </pre>
 *
 ******************************************************************************/

#ifndef _IPANEL_MIDDLEWARE_PORTING_DEMUX_API_FUNCTOTYPE_H_
#define _IPANEL_MIDDLEWARE_PORTING_DEMUX_API_FUNCTOTYPE_H_

#ifdef __cplusplus
extern "C" {
#endif


#define IPANEL_PORTING_CHANNEL_TOTAL    32
#define IPANEL_PORTING_FILTER_TOTAL     32

typedef void (*NOTIFY_FUNC)(UINT32 ch, UINT32 filter, UINT32 buf, INT32 len);


enum ipanel_dmx_channel_stat
{
    IPANEL_DMX_CHANNEL_STAT_DETACH = 0,

    IPANEL_DMX_CHANNEL_STAT_ATTACH,

    IPANEL_DMX_CHANNEL_STAT_PLAY,

    IPANEL_DMX_CHANNEL_STAT_PAUSE
};


enum ipanel_dmx_filter_stat
{
    IPANEL_DMX_FILTER_STAT_DETACH = 0,

    IPANEL_DMX_FILTER_STAT_ATTACH,

    IPANEL_DMX_FILTER_STAT_PLAY,

    IPANEL_DMX_FILTER_STAT_PAUSE
};


INT32 ipanel_porting_set_demux_notify(NOTIFY_FUNC func);

UINT32 ipanel_porting_create_demux_channel(INT32 pool_size, UINT16 pid);

INT32 ipanel_porting_start_demux_channel(UINT32 idx);

INT32 ipanel_porting_stop_demux_channel(UINT32 idx);

INT32 ipanel_porting_destroy_demux_channel(UINT32 idx);

UINT32 ipanel_porting_create_demux_filter(UINT32 channel, UINT32 wide, UINT8 * coef, UINT8 * mask, UINT8 * excl);

INT32 ipanel_porting_enable_demux_filter(UINT32 idx);

INT32 ipanel_porting_disable_demux_filter(UINT32 idx);

INT32 ipanel_porting_destroy_demux_filter(UINT32 idx);


#ifdef __cplusplus
}
#endif

#endif
