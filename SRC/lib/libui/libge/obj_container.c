/*-----------------------------------------------------------------------------
    Copyright (C) 2010 ALi Corp. All rights reserved.
    File: obj_XXX.c
-----------------------------------------------------------------------------*/
#include <sys_config.h>
#include <api/libge/osd_lib.h>
#include "osd_lib_internal.h"

void OSD_DrawContainerCell(PCONTAINER c,UINT8 bStyleIdx,UINT32 nCmdDraw)
{
	PGUI_VSCR  pVscr;
	PGUI_RECT objframe;
	
	POBJECT_HEAD oh  = (POBJECT_HEAD)(c->pNextInCntn);	
	PRESULT		bResult = PROC_PASS;
	
	UINT32 	draw_type 	= OSD_GetDrawType(nCmdDraw);
	UINT32	update_type = OSD_GetUpdateType(nCmdDraw);
	UINT32	nSubCmdDraw;

	if(update_type==C_UPDATE_ALL)
	{
			objframe = &c->head.frame;
			if(OSD_GetAttrSlave(c))
				pVscr = OSD_DrawSlaveFrame(objframe, bStyleIdx);
			else
				pVscr = OSD_DrawObjectFrame(objframe, bStyleIdx);

			if (oh)
			{
				OSD_SetVscrModified(pVscr);
			}
	}//SZTODO,slave here

	if(OSD_GetAttrSlave(c))
		return;

	nSubCmdDraw = nCmdDraw;
	//OSD_SetUpdateType(nSubCmdDraw,C_UPDATE_ALL);
	while(oh != NULL)
	{
  		if((update_type == C_UPDATE_ALL) ||(update_type == C_UPDATE_CONTENT)||(c->FocusObjectID==oh->bID))
        		{
	    		if((draw_type == C_DRAW_TYPE_HIGHLIGHT) && (0 != c->FocusObjectID)			
	    			&& (c->FocusObjectID==oh->bID || c->bHiliteAsWhole) )
	    			OSD_TrackObject(oh, nSubCmdDraw);
	    		else if( draw_type == C_DRAW_TYPE_SELECT)
	    			OSD_SelObject(oh,nSubCmdDraw);
	    		else
	    			OSD_DrawObject(oh, nSubCmdDraw);
        		}
						
		oh = oh->pNext;
	}
}

PRESULT OSD_ContainerChgFocus(CONTAINER *c, UINT8 bNewFocusID,UINT32 parm)
{
	UINT8 bFocusID;
 	UINT32 submsg;
	PRESULT Result,Retproc;
	GUI_RECT rctCon;
	PGUI_VSCR pgvsr;
	POBJECT_HEAD pObj,pFocus, pNewFocus;
	
	Result = PROC_PASS;
	Retproc = PROC_PASS;
	pObj = (POBJECT_HEAD)c;
	bFocusID = OSD_GetFocusID(pObj);
	pFocus   	= OSD_GetObject(pObj, bFocusID);   
	pNewFocus 	= OSD_GetObject(pObj, bNewFocusID);
	if(bFocusID == bNewFocusID
		|| pFocus == pNewFocus
		|| pNewFocus == NULL)
		return PROC_LOOP;

	if(((pNewFocus->bAttr)&&C_ATTR_LMASK) != C_ATTR_ACTIVE)
		return PROC_LOOP;

	pgvsr = NULL;
	MEMCPY(&rctCon,&(c->head.frame),sizeof(GUI_RECT));
	if(OSD_GetAttrSlave(c))
		OSD_GetSlvscr(&rctCon,VSCR_NULL);
	else
		pgvsr = OSD_GetVscr(&rctCon,VSCR_NULL);
	if(parm & C_DRAW_SIGN_EVN_FLG)
	{
		Result = OSD_SIGNAL(pObj,EVN_ITEM_PRE_CHANGE,(UINT32)pNewFocus,(UINT32)pFocus);
		if(Result != PROC_PASS)
			return Result;
	}

	if(parm & C_DRAW_SIGN_EVN_FLG)
	{
		Result = OSD_ObjProc(pFocus,(MSG_TYPE_EVNT<<16) |EVN_FOCUS_PRE_LOSE,(UINT32)pNewFocus, 0);		        		        
		if(Result != PROC_PASS)
			return Result;
	}

	if(parm & C_UPDATE_FOCUS)
	{
		//Retproc = OSD_SIGNAL(pObj,EVN_FOCUS_SLIDE,(UINT32)pFocus,(UINT32)pNewFocus);
#ifndef FOCUS_SLIDE_DISABLE		
		Retproc = (gelib_item_focuslide(pFocus,pNewFocus,0) == 0) ? PROC_SKIP : PROC_PASS;
#else
		Retproc = PROC_PASS;
#endif
		//if(Retproc != PROC_SKIP)
		{
			submsg = (parm & C_DRAW_SIGN_EVN_FLG) | C_UPDATE_ALL;
			if(!c->bHiliteAsWhole)
				OSD_DrawObject(pFocus, submsg);
			else
				OSD_TrackObject(pFocus, submsg);
		}
	}

	if(parm & C_DRAW_SIGN_EVN_FLG)
	{
		Result = OSD_ObjProc(pFocus,(MSG_TYPE_EVNT<<16) |EVN_FOCUS_POST_LOSE,(UINT32)pNewFocus,0);
		if(Result != PROC_PASS)
			return Result;
	}

	if(parm & C_DRAW_SIGN_EVN_FLG)
	{
		Result = OSD_ObjProc(pNewFocus, (MSG_TYPE_EVNT<<16) |EVN_FOCUS_PRE_GET,(UINT32)pFocus,0);
		if(Result != PROC_PASS)
			return Result;							
	}

	if(parm & C_UPDATE_FOCUS)
	{
		//if(PROC_SKIP != Retproc)
		{
			submsg = (parm & C_DRAW_SIGN_EVN_FLG) | C_UPDATE_ALL;
			OSD_TrackObject(pNewFocus,submsg);
		}
	}

	c->FocusObjectID = pNewFocus->bID;
	if(parm & C_DRAW_SIGN_EVN_FLG)					
	{
		Result = OSD_ObjProc(pNewFocus,(MSG_TYPE_EVNT<<16) |EVN_FOCUS_POST_GET,(UINT32)pFocus,0);
		if(Result != PROC_PASS)
			return Result;

		OSD_SIGNAL(pObj,EVN_ITEM_POST_CHANGE,(UINT32)pNewFocus,(UINT32)pFocus);
		if(Result != PROC_PASS)
			return Result;	
	}

	return Result;	
}

VACTION OSD_ContainerKeyMap(POBJECT_HEAD pObj, UINT32 vkey)
{
	VACTION Vact = VACT_PASS;
    
	switch(vkey)
	{
	case V_KEY_UP:
		Vact = VACT_CURSOR_UP;		
		break;
	case V_KEY_DOWN:
		Vact = VACT_CURSOR_DOWN;	
		break;
	case V_KEY_LEFT:
		Vact = VACT_CURSOR_LEFT;	
		break;
	case V_KEY_RIGHT:
		Vact = VACT_CURSOR_RIGHT;  
		break;
	default:
		;
	}
	return Vact;
}

PRESULT OSD_ContainerProc(POBJECT_HEAD pObj, UINT32 msg_type, UINT32 msg,UINT32 param1)
{
	PRESULT Result = PROC_PASS;
	CONTAINER	*c = (CONTAINER*)pObj;
	
	POBJECT_HEAD pFocus, pNewFocus;
	UINT8 bFocusID,bNewFocusID;

    	UINT32 subEvnt;
 	UINT32 submsg;
	
	bFocusID = OSD_GetFocusID(pObj);
	pFocus   = OSD_GetObject(pObj, bFocusID);    
	
	if(msg_type == MSG_TYPE_KEY)
	{
		VACTION Action;
		UINT32 vkey;
		BOOL bContinue;	
	    
		if(pFocus!=NULL)
		{
			Result = OSD_ObjProc(pFocus,msg_type<<16, msg,param1);		
			if(Result != PROC_PASS)
	    		goto CHECK_LEAVE;
		}
	
		Result = OSD_ObjCommonProc(pObj,msg,param1,&vkey,&Action,&bContinue);
		if(!bContinue)
	    	goto CHECK_LEAVE;

		switch(Action)
		{
		case VACT_CURSOR_UP:
		case VACT_CURSOR_DOWN:
		case VACT_CURSOR_LEFT:
		case VACT_CURSOR_RIGHT:			
			pNewFocus = OSD_GetAdjacentObject(pFocus,Action);
			if(pNewFocus == NULL)
			{
				Result = PROC_LOOP;
				break;
			}
			
			bNewFocusID = OSD_GetObjID(pNewFocus);
			
			if(pNewFocus==pFocus)
				Result = PROC_LOOP;
			else
			{
				submsg = C_DRAW_SIGN_EVN_FLG | C_UPDATE_FOCUS;
				Result = OSD_ContainerChgFocus(c, bNewFocusID, submsg);
				if(Result != PROC_PASS)
	    			goto CHECK_LEAVE;
				
				Result = PROC_LOOP;
		        break;				
			}
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
	else// if(MSG_TYPE_EVNT == osd_msg_type)
	{
	 	UINT32 submsg;
		if(pFocus!= NULL && OSD_EventPassToChild(msg_type,&submsg))
			Result = OSD_ObjProc(pFocus,(MSG_TYPE_EVNT<<16)| submsg,msg, param1);
		else 
			Result = PROC_PASS;
		if(Result==PROC_PASS)
			Result = OSD_SIGNAL(pObj, msg_type, msg, param1);
	}

CHECK_LEAVE:
	if(!(EVN_PRE_OPEN == msg_type))
		CHECK_LEAVE_RETURN(Result,pObj);
	
EXIT:
	return Result;
}

