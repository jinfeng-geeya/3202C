#include <sys_config.h>
#include <sys_parameters.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>

#include <api/libloader/ld_osd_lib.h>
#include "ld_osd_lib_internal.h"

BOOL OSD_RectInRect(const struct OSDRect* R,const struct OSDRect* r)
{
	if(r->uLeft >= R->uLeft && r->uTop>= R->uTop
		&& (r->uLeft + r->uWidth)<=(R->uLeft + R->uWidth)
		&& (r->uTop + r->uHeight)<=(R->uTop + R->uHeight))
		return TRUE;
	else
		return FALSE;
}

void OSD_GetRectsCross(const struct OSDRect *backR,const struct OSDRect *forgR, struct OSDRect* relativR)
{
	short ltx,lty,rbx,rby;
	short dw,dh;

	ltx = forgR->uLeft >=  backR->uLeft ? forgR->uLeft :  backR->uLeft;
	lty = forgR->uTop >=  backR->uTop ? forgR->uTop :  backR->uTop;
	rbx = (forgR->uLeft + forgR->uWidth) <= (backR->uLeft + backR->uWidth) ? (forgR->uLeft + forgR->uWidth) : (backR->uLeft + backR->uWidth);
	rby = (forgR->uTop + forgR->uHeight) <= (backR->uTop + backR->uHeight) ? (forgR->uTop + forgR->uHeight) : (backR->uTop + backR->uHeight);

	dw = rbx > ltx ? (rbx - ltx) : 0;
	dh = rby > lty ? (rby - lty) : 0;
	relativR->uWidth  = dw;
	relativR->uHeight = dh;
	relativR->uLeft = ltx;
  	relativR->uTop  = lty;	
}

void OSD_init(struct OSD_InitInfo* pOSDInfo, UINT8* p_pallette)
{
	OSD_SetDeviceHandle((HANDLE)pOSDInfo->p_handle);
	OSDDrv_Open((HANDLE)pOSDInfo->p_handle, &(pOSDInfo->tOpenPara));
	OSDDrv_CreateRegion((HANDLE)pOSDInfo->p_handle, 0, &(pOSDInfo->osdrect), NULL);    
	pOSDInfo->osdrect.uLeft = pOSDInfo->osdrect.uTop = 0;
	OSDDrv_SetPallette((HANDLE)pOSDInfo->p_handle, p_pallette, COLOR_N, OSDDRV_YCBCR);
	OSDDrv_RegionFill((HANDLE)pOSDInfo->p_handle,0,&(pOSDInfo->osdrect),OSD_TRANSPARENT_COLOR);
	OSDDrv_ShowOnOff((HANDLE)pOSDInfo->p_handle,OSDDRV_ON);
}

void OSD_close(struct osd_device *dev)
{
	OSDDrv_ShowOnOff((HANDLE)dev,OSDDRV_OFF);
}

