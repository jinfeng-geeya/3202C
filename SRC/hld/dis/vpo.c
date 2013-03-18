/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2002 Copyright (C)
 *
 *  File: decv.c
 *
 *  Description: This file define the functions for video decoder device management.
 *
 *  History:
 *      Date        Author         Version   Comment
 *      ====        ======         =======   =======
 *  1.  2003.04.11  David Wang     0.1.000   Initial
 ****************************************************************************/

#include <osal/osal.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <sys_config.h>
#include <mediatypes.h>
#include <hld/dis/vpo.h>
__ATTRIBUTE_REUSE_
RET_CODE vpo_open(struct vpo_device *dev,struct VP_InitInfo *pInitInfo)
{
	return dev->open(dev,pInitInfo);
}

RET_CODE vpo_close(struct vpo_device *dev)
{	
	return dev->close(dev);
}
RET_CODE vpo_win_onoff(struct vpo_device *dev,BOOL bOn)
{
	return dev->win_onoff(dev,bOn);
}
RET_CODE vpo_win_mode(struct vpo_device *dev, BYTE bWinMode, struct MPSource_CallBack *pMPCallBack,struct PIPSource_CallBack *pPIPCallBack)
{
	return dev->win_mode(dev,bWinMode,pMPCallBack,pPIPCallBack);
}
RET_CODE vpo_zoom(struct vpo_device *dev, struct Rect *pSrcRect , struct Rect *pDstRect)
{
	return dev->zoom(dev,pSrcRect,pDstRect);
}
RET_CODE vpo_aspect_mode(struct vpo_device *dev, enum TVMode eTVAspect, enum DisplayMode e169DisplayMode)
{
	return dev->aspect_mode(dev,eTVAspect,e169DisplayMode);
}
RET_CODE vpo_tvsys(struct vpo_device *dev, enum TVSystem eTVSys)
{
	return dev->tvsys(dev,eTVSys,FALSE);
}
RET_CODE vpo_tvsys_ex(struct vpo_device *dev, enum TVSystem eTVSys, BOOL bProgressive)
{
	return dev->tvsys(dev,eTVSys,bProgressive);
}
RET_CODE vpo_ioctl(struct vpo_device *dev, UINT32 dwCmd, UINT32 dwParam)
{
	return dev->ioctl(dev,dwCmd,dwParam);
}
RET_CODE vpo_config_source_window(struct vpo_device *dev, struct vp_win_config_para *pwin_para)
{
	if(dev->config_source_window)
		return dev->config_source_window(dev,pwin_para);
	else
		return RET_FAILURE;
}
RET_CODE vpo_set_progres_interl(struct vpo_device *dev, BOOL bProgressive)
{
	if(dev->set_progres_interl)
		return dev->set_progres_interl(dev,bProgressive);
	else
		return RET_FAILURE;
}

RET_CODE tvenc_open(struct tve_device *dev)
{
	return dev->open(dev);
}
RET_CODE tvenc_close(struct tve_device *dev)
{
	return dev->close(dev);
}
RET_CODE tvenc_set_tvsys(struct tve_device *dev,enum TVSystem eTVSys)
{
	return dev->set_tvsys(dev,eTVSys,FALSE);
}

RET_CODE tvenc_set_tvsys_ex(struct tve_device *dev,enum TVSystem eTVSys, BOOL bProgressive)
{
	return dev->set_tvsys(dev,eTVSys,bProgressive);
}
RET_CODE tvenc_register_dac(struct tve_device *dev,enum DacType eDacType, struct VP_DacInfo *pInfo)
{
	return dev->register_dac(dev,eDacType,pInfo);
}
RET_CODE tvenc_unregister_dac(struct tve_device *dev,enum DacType eDacType)
{
	return dev->unregister_dac(dev,eDacType);
}
RET_CODE tvenc_write_wss(struct tve_device *dev,UINT16 Data)
{
	return dev->write_wss(dev,Data);
}
RET_CODE tvenc_write_cc(struct tve_device *dev,UINT8 FieldParity, UINT16 Data)
{
	return dev->write_cc(dev,FieldParity,Data);
}
RET_CODE tvenc_write_ttx(struct tve_device *dev,UINT8 LineAddr, UINT8 Addr, UINT8 Data)
{
	return dev->write_ttx(dev,LineAddr,Addr,Data);
}

