/*-----------------------------------------------------------------------------
    Copyright (C) 2010 ALi Corp. All rights reserved.
    File: obj_XXX.c
-----------------------------------------------------------------------------*/
#include <sys_config.h>

#include <api/libc/string.h>
#include <api/libge/osd_lib.h>
#include <api/libge/osd_common_draw.h>
#include <api/libge/osd_primitive.h>
#include "osd_lib_internal.h"
void OSD_DrawBitmapCell(PBITMAP pCtrl,UINT8 bStyleIdx,UINT32 nCmdDraw)
{
	UINT8 bAlign,hAligin,vAlign;
	UINT16 x,y,w,h;
	UINT32 bmpstyle;
	GUI_POINT pos;
	PGUI_RECT objframe;
	PGUI_VSCR pVscr;
	ID_RSC RscId;
	OBJECTINFO RscLibInfo;
	
	if(OSD_GetUpdateType(nCmdDraw) == C_UPDATE_CONTENT)
		bmpstyle = (UINT32)(~0);
	else
		bmpstyle = bStyleIdx;
	
	RscId  = LIB_ICON;
	bAlign = pCtrl->bAlign;	
	objframe	= &pCtrl->head.frame;
	if(OSD_GetAttrSlave(pCtrl))
		pVscr = OSD_DrawSlaveFrame(objframe,bStyleIdx);
	else
		pVscr = OSD_DrawObjectFrame(objframe,bmpstyle);
	
	if(pCtrl->wIconID)
	{
		g_gui_rscfunc.osd_get_obj_info(RscId, pCtrl->wIconID, &RscLibInfo);
		w = RscLibInfo.m_objAttr.m_wActualWidth;
		h = RscLibInfo.m_objAttr.m_wHeight;
		hAligin = GET_HALIGN(bAlign);
		vAlign  = GET_VALIGN(bAlign);
		if(hAligin==C_ALIGN_LEFT)
			x = objframe->uStartX + pCtrl->bX;
		else if(hAligin==C_ALIGN_RIGHT)
			x = objframe->uStartX + objframe->uWidth - pCtrl->bX - w;
		else
		{
			if(w < (objframe->uWidth - (pCtrl->bX<<1) ) )
				x = objframe->uStartX + ((objframe->uWidth - w)>>1);
			else
				x = objframe->uStartX + pCtrl->bX;				
		}
		
		if(vAlign == C_ALIGN_TOP)
			y = objframe->uStartY + pCtrl->bY;
		else if(vAlign == C_ALIGN_BOTTOM)
			y = objframe->uStartY + objframe->uHeight - pCtrl->bY - h;
		else
		{
			if( h < (objframe->uHeight - (pCtrl->bY<<1) ) )
				y = objframe->uStartY + ((objframe->uHeight - h)>>1);
			else
				y = objframe->uStartY + pCtrl->bY;				
		}		
		
		g_gui_rscfunc.osd_get_obj_info(RscId,pCtrl->wIconID,&RscLibInfo);

		pos.uX = x;
		pos.uY = y;
		OSD_DrawBmp2Vscr(&pos,pCtrl->wIconID,pVscr);
	}
}

 
VACTION OSD_BitmapKeyMap(POBJECT_HEAD pObj, UINT32 vkey)
{
 	VACTION Action = VACT_PASS;

    switch(vkey)
    {
    case V_KEY_ENTER:
        Action = VACT_ENTER;
        break;
    default:
        break;
    }

    return Action;
}


PRESULT OSD_BitmapProc(POBJECT_HEAD pObj, UINT32 msg_type, UINT32 msg,UINT32 param1)
{
	PRESULT Result;

	if(msg_type == MSG_TYPE_KEY)
	{
		VACTION Action;
		UINT32 vkey;
		BOOL bContinue;    	

		Result = OSD_ObjCommonProc(pObj,msg,param1,&vkey,&Action,&bContinue);
		if(!bContinue)
			goto CHECK_LEAVE;

		switch(Action)
		{
			case VACT_CLOSE:
CLOSE_OBJECT:    		
				Result = OSD_ObjClose(pObj,C_CLOSE_CLRBACK_FLG);
				goto EXIT;
				break;
			default:
				Result = OSD_SIGNAL(pObj, EVN_UNKNOWN_ACTION, (Action<<16) | vkey, param1);
				break;
		}
	}
	else// if(msg_type==MSG_TYPE_EVNT)
	{
		Result = OSD_SIGNAL(pObj,msg_type, msg, param1);
	}

CHECK_LEAVE:
	if(!(EVN_PRE_OPEN == msg_type))
		CHECK_LEAVE_RETURN(Result,pObj);

EXIT:
	return Result;		
}

