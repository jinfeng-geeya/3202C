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

#define MAX_STR_LEN		32
static UINT8* GetSelText(PMULTISEL pCtrl, UINT16* pBuf, int nSize)
{
	PRESULT Result;
    UINT16* pUniStr = pBuf;
    UINT16 id;
    char str[MAX_STR_LEN];
    UINT8* pStr;
    
    pUniStr[0] = 0;
    switch(pCtrl->bSelType)
    {
    case STRING_ID:
        id = ((UINT16*)pCtrl->pSelTable)[pCtrl->nSel];
        pStr = OSD_GetUnicodeString(id);
        return pStr;
        break;

    case STRING_ANSI:
    	{
    		UINT8** pcstr = (UINT8**)pCtrl->pSelTable;
        	OSD_ASSERT(nSize > (INT32)STRLEN( (char*)pcstr[pCtrl->nSel]) );
        	ComAscStr2Uni((UINT8*)pcstr[pCtrl->nSel], pUniStr);
    	}
        break;

    case STRING_UNICODE:
    	{
        	UINT16** pcwstr = (UINT16**)pCtrl->pSelTable;
            pUniStr = (UINT16*)pcwstr[pCtrl->nSel];
    	}
        break;

    case STRING_NUM_TOTAL:
    	sprintf(str,"%d/%d", pCtrl->nSel, ((UINT32 *)pCtrl->pSelTable)[1]);
    	ComAscStr2Uni(str,pUniStr);        
        break;

    case STRING_NUMBER:
        ComInt2UniStr(pUniStr, pCtrl->nSel, 0);
        break;

    case STRING_PROC:
		Result = OSD_SIGNAL((POBJECT_HEAD)pCtrl, EVN_REQUEST_STRING, (UINT32)pCtrl->nSel, (UINT32)pUniStr);
		if(Result != PROC_PASS)
			pUniStr[0] = 0;
        break;
					
    default:
        OSD_ASSERT(0);
        break;
    }
    
    return (UINT8*)pUniStr;
}

void OSD_DrawMultiselCell(PMULTISEL pCtrl, UINT8 bStyleIdx, UINT32 nCmdDraw)
{	
	PGUI_VSCR 		pVscr;
	PGUI_RECT objframe;	
	PWINSTYLE  	lpWinSty;
	UINT32 mslstyle;
	UINT8 mslfont,*pStr;
	OBJECTINFO	RscLibInfo;
	UINT16 wsText[MAX_STR_LEN];
	GUI_RECT r;
	
	if(OSD_GetUpdateType(nCmdDraw) == C_UPDATE_CONTENT)
		mslstyle = (UINT32)(~0);
	else
		mslstyle = bStyleIdx;
	lpWinSty  = g_gui_rscfunc.osd_get_win_style(bStyleIdx);
	pStr = GetSelText(pCtrl, wsText, ARRAY_SIZE(wsText));
	
	objframe	= &pCtrl->head.frame;
	if(OSD_GetAttrSlave(pCtrl))
	{
		mslfont = (pCtrl->head.bFont)|C_FONT_SLVMASK;
		pVscr = OSD_DrawSlaveFrame(objframe,bStyleIdx);	
	}
	else
	{
		mslfont = (pCtrl->head.bFont);
		pVscr = OSD_DrawObjectFrame(objframe,mslstyle);	
	}

	r = *objframe;
	r.uStartX += pCtrl->bX;
	r.uStartY	+= pCtrl->bY;
	r.uWidth -= pCtrl->bX<<1;
	r.uHeight -= pCtrl->bY<<1;		

	OSD_DrawText(&r,pStr,lpWinSty->wFgIdx,pCtrl->bAlign,mslfont,pVscr);
}

VACTION OSD_MultiselKeyMap(POBJECT_HEAD pObj, UINT32 Key)
{
    VACTION Action = VACT_PASS;
    PMULTISEL pCtrl = (PMULTISEL)pObj;

	switch(Key)
	{
	case V_KEY_LEFT:
		Action = VACT_DECREASE;
		break;
	case V_KEY_RIGHT:
		Action = VACT_INCREASE;
		break;
	default:
		break;
	}

    return Action;
}


PRESULT OSD_MultiselProc(POBJECT_HEAD pObj, UINT32 msg_type, UINT32 msg,UINT32 param1)
{
	PRESULT Result = PROC_LOOP;
	PMULTISEL pCtrl = (PMULTISEL)pObj;
	INT32 nSel, nStep;

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
		case VACT_DECREASE:
		case VACT_INCREASE:
			if(Action == VACT_DECREASE)
				nStep = -1;
			else
				nStep = 1;

			if(STRING_NUMBER == pCtrl->bSelType 
			|| STRING_NUM_TOTAL == pCtrl->bSelType)
			{
				INT32 *pValue = (INT32 *)pCtrl->pSelTable;
				nSel = (INT32)pCtrl->nSel + (pValue[2]*nStep);
				if(nSel < pValue[0])
					nSel = pValue[1];
				else if(nSel > pValue[1])
					nSel = pValue[0];
			}
			else
			{
				if(pCtrl->nCount == 0)
					break;
				
				nSel = (pCtrl->nSel + nStep + pCtrl->nCount)%pCtrl->nCount;
			}
		
			Result = OSD_SIGNAL((POBJECT_HEAD)pCtrl, EVN_PRE_CHANGE, (UINT32)&nSel, 0);
			if(Result != PROC_PASS)
				goto CHECK_LEAVE;
		
			pCtrl->nSel = nSel;
					
			OSD_TrackObject((POBJECT_HEAD)pCtrl, C_UPDATE_ALL);
		
			Result = OSD_SIGNAL((POBJECT_HEAD)pCtrl, EVN_POST_CHANGE, (UINT32)nSel, 0);
			if(Result != PROC_PASS)
				goto CHECK_LEAVE;
				
			Result = PROC_LOOP;
			break;
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
		Result = OSD_SIGNAL(pObj, msg_type, msg, param1);

CHECK_LEAVE:
	if(!(EVN_PRE_OPEN == msg_type))
		CHECK_LEAVE_RETURN(Result,pObj);		

EXIT:
	return Result;
}

