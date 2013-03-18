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

void OSD_SetSDTxtContent(PSDTXT pCtrl, UINT32 StringType, UINT32 Value)
{
    UINT16 len;
	char str[16];

    switch(StringType)
    {
        case STRING_NUMBER:
            ComInt2UniStr(pCtrl->pString, Value, 0);
            break;
        case STRING_NUM_PERCENT:
			sprintf(str,"%d%%",Value);
			ComAscStr2Uni(str, pCtrl->pString); 
            break;
        case STRING_ANSI:
        	ComAscStr2Uni((UINT8*)Value,pCtrl->pString);
            break;
        case STRING_UTF8:
        	ComUTF8Str2Uni((UINT8*)Value,pCtrl->pString);
            break;
        case STRING_UNICODE:
        	ComUniStrCopyChar((UINT8 *)pCtrl->pString, (UINT8 *)Value);
            break;
        default:
            return;
    }
}

void OSD_SetTextFieldContent(PTEXT_FIELD pCtrl, UINT32 StringType, UINT32 Value)
{
    UINT16 len;
	char str[16];

    switch(StringType)
    {
        case STRING_ID:
            pCtrl->wStringID = (UINT16)Value;
            return;
        case STRING_NUMBER:
            ComInt2UniStr(pCtrl->pString, Value, 0);
            break;
        case STRING_NUM_PERCENT:
			sprintf(str,"%d%%",Value);
			ComAscStr2Uni(str, pCtrl->pString); 
            break;
        case STRING_ANSI:
        	ComAscStr2Uni((UINT8*)Value,pCtrl->pString);
            break;
        case STRING_UTF8:
        	ComUTF8Str2Uni((UINT8*)Value,pCtrl->pString);
            break;
        case STRING_UNICODE:
        	ComUniStrCopyChar((UINT8 *)pCtrl->pString, (UINT8 *)Value);
            break;
        default:
            return;
    }
}

void OSD_DrawTextFieldCell(PTEXT_FIELD pCtrl,UINT8 bStyleIdx,UINT32 nCmdDraw)
{
	PGUI_VSCR 		pVscr;
	PGUI_RECT objframe;	
	PWINSTYLE  	lpWinSty;
	UINT32 txtstyle;
	UINT8 txtfont,*pStr;
	OBJECTINFO	RscLibInfo;
	GUI_RECT r;
	
	if(OSD_GetUpdateType(nCmdDraw) == C_UPDATE_CONTENT)
		txtstyle = (UINT32)(~0);
	else
		txtstyle = bStyleIdx;
	lpWinSty  = g_gui_rscfunc.osd_get_win_style(bStyleIdx);
	if( pCtrl->pString != NULL)
		pStr = (UINT8*)pCtrl->pString;
	else
		pStr = OSD_GetUnicodeString(pCtrl->wStringID);
	
	objframe	= &pCtrl->head.frame;
	if(OSD_GetAttrSlave(pCtrl))
	{
		txtfont = (pCtrl->head.bFont)|C_FONT_SLVMASK;
		pVscr = OSD_DrawSlaveFrame(objframe,bStyleIdx);
	}
	else
	{
		txtfont = (pCtrl->head.bFont)|0;
		pVscr = OSD_DrawObjectFrame(objframe,txtstyle);
	}
	r = *objframe;
	
	if(!(pCtrl->bAlign&C_ALIGN_SCROLL))
	{
		r.uStartX += pCtrl->bX;
		r.uWidth -= pCtrl->bX<<1;
	}
	r.uStartY += pCtrl->bY;
	r.uHeight -= pCtrl->bY<<1;	

	if(pCtrl->bAlign&C_ALIGN_SCROLL)
	{
		OSD_DrawText_ext(&r,pStr,lpWinSty->wFgIdx,pCtrl->bAlign,txtfont,pVscr,pCtrl->bX);
	}
	else
	{
		OSD_DrawText(&r,pStr,lpWinSty->wFgIdx,pCtrl->bAlign,txtfont,pVscr);
	}
}

VACTION OSD_TextFieldKeyMap(POBJECT_HEAD pObj, UINT32 vkey)
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

PRESULT OSD_TextFieldProc(POBJECT_HEAD pObj, UINT32 msg_type, UINT32 msg,UINT32 param1)
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

