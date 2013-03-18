/*-----------------------------------------------------------------------------
    Copyright (C) 2010 ALi Corp. All rights reserved.
    File: obj_XXX.c
-----------------------------------------------------------------------------*/
#include <sys_config.h>

#include <hld/ge/ge.h>
#include <api/libge/osd_lib.h>
#include <api/libge/ge_primitive_init.h>
#include <api/libge/osd_primitive.h>
#include "osd_lib_internal.h"

static void OSDObjListShowItem(OBJLIST *ol,UINT16 itemIdx,UINT32 nCmdDraw)
{
	UINT8	drawType;
	BOOL	bSelect;
	UINT32	nItemCmdDraw,listplotstyle;
	short	itemXOffset,itemYOffset;
	short	markXOffset,markYOffset;
	POBJECT_HEAD item,pItem0,pItemTop;
	
	listplotstyle  = OSD_GetDrawType(nCmdDraw);
	nItemCmdDraw = nCmdDraw & 0xFFFFFF0F;
	OSD_SetUpdateType(nItemCmdDraw, C_UPDATE_ALL);
	
	if(itemIdx<ol->wTop)
		return;
	if(itemIdx>=(ol->wTop + ol->wDep))
		return;
	
	if(itemIdx>=ol->wCount)
	{
		if(ol->bListStyle & LIST_GRID)
		{
			if(ol->bListStyle & LIST_ITEMS_COMPLETE)
				item = ol->pListField[ol->wCount-1];
			else
				item = ol->pListField[itemIdx - ol->wTop];
			OSD_DrawObject(item,  nCmdDraw);
		}
		return;
	}

	pItem0 = ol->pListField[0];    /*Item 0*/
	if(ol->bListStyle & LIST_ITEMS_COMPLETE)
	{
		pItemTop = ol->pListField[ol->wTop];	/*Top Item*/
		item = ol->pListField[itemIdx];		/*Current Item*/
	}
	else
	{
		pItemTop = ol->pListField[0];
		item = ol->pListField[itemIdx - ol->wTop];
	}
    
	if(ol->bListStyle & LIST_VER)
	{
		itemXOffset = markXOffset = 0;
		itemYOffset = pItemTop->frame.uStartY - pItem0->frame.uStartY;
		markYOffset = item->frame.uStartY - pItem0->frame.uStartY;
	}
	else
	{
		itemYOffset = markYOffset = 0;
		itemXOffset = pItemTop->frame.uStartX -  pItem0->frame.uStartX;
		markXOffset = item->frame.uStartX - pItem0->frame.uStartX;
	}

	/*Move object's position*/
	if(ol->pSelMarkObj!=NULL)
		OSD_MoveObject(ol->pSelMarkObj, markXOffset - itemXOffset, markYOffset - itemYOffset, TRUE);
	OSD_MoveObject(item, -itemXOffset, -itemYOffset, TRUE);
	
	drawType =  0;//Show
	bSelect = 0;
	if(ol->bListStyle & LIST_SINGLE_SLECT)
	{
		if(ol->wSelect < ol->wCount && ol->wSelect == itemIdx)
			bSelect = 1;
	}
	else if(ol->bListStyle & LIST_MULTI_SLECT)
		bSelect = OSD_CheckObjListMultiSelect(ol,itemIdx);
	

	if(ol->bListStyle & LIST_SELECT_FIRST)
	{
		if(bSelect)
		{
			if(ol->bListStyle & LIST_KEEP_SELECT \
			|| listplotstyle == C_DRAW_TYPE_HIGHLIGHT \
			|| listplotstyle == C_DRAW_TYPE_SELECT)
				drawType = 2;
		}
		else if(itemIdx== ol->wCurPoint)
		{
			if((ol->bListStyle & LIST_KEEP_FOUCS) \
			|| listplotstyle == C_DRAW_TYPE_HIGHLIGHT)
				drawType = 1;
		}
	}
	else
	{
		if(itemIdx== ol->wCurPoint)
		{
			if((ol->bListStyle & LIST_KEEP_FOUCS) || listplotstyle == C_DRAW_TYPE_HIGHLIGHT)
				drawType = 1;
		}
		if(bSelect && drawType==0)
		{
			if((ol->bListStyle & LIST_KEEP_SELECT) || listplotstyle == C_DRAW_TYPE_HIGHLIGHT)
				drawType = 2;
		}
	}
	if(!bSelect && ol->pSelMarkObj!=NULL)
	{
		if(!OSD_RectInRect(&item->frame, &ol->pSelMarkObj->frame))
			OSD_HideObject(ol->pSelMarkObj,nItemCmdDraw);
	}

	if( !OSD_CheckAttr(item, C_ATTR_ACTIVE))
		drawType = 0;

	/*Draw item*/
	if(drawType==0)
		OSD_DrawObject(item,  nItemCmdDraw);
	else if(drawType==1)
		OSD_TrackObject(item, nItemCmdDraw);
	else if(drawType==2)
		OSD_SelObject(item,   nItemCmdDraw);

	/*Draw select mark*/
	if(bSelect && ol->pSelMarkObj!=NULL)
	{
		if(drawType==0)
			OSD_DrawObject(ol->pSelMarkObj,  nItemCmdDraw);
		else if(drawType==1)
			OSD_TrackObject(ol->pSelMarkObj, nItemCmdDraw);
		else if(drawType==2)
			OSD_SelObject(ol->pSelMarkObj,   nItemCmdDraw);
	}

	/*Restore object's position*/
	if(ol->pSelMarkObj!=NULL)
		OSD_MoveObject(ol->pSelMarkObj, -(markXOffset - itemXOffset), -(markYOffset - itemYOffset), TRUE);
	OSD_MoveObject(item, itemXOffset, itemYOffset, TRUE);
}

static void OSD_UpdateObjListVerScrollBar(OBJLIST *ol, UINT32 nCmdDraw)
{
	SCROLL_BAR *sb;
	UINT16 wListCount = ol->wCount;
	UINT16 wListCur = ol->wCurPoint;
	UINT32 nSbCmdDraw = nCmdDraw & 0xFFFFFF0F;
	UINT32 objDrawType  = OSD_GetDrawType(nCmdDraw);
	
	sb = ol->scrollBar;
	if(sb==NULL)
	    return;
	if(ol->bListStyle&LIST_BARSCROLL_COMPLETE)
		OSD_SetScrollBarPos(sb,ol->wCurPoint);
	else	
		OSD_SetScrollBarPos(sb,ol->wTop);
	OSD_SetScrollBarMax(sb,ol->wCount);
    
    /* Never sign event to UI*/
	if(objDrawType == C_DRAW_TYPE_HIGHLIGHT)
		OSD_TrackObject((POBJECT_HEAD)sb,nCmdDraw & 0xFF);   
	else if(objDrawType == C_DRAW_TYPE_SELECT)
		OSD_SelObject((POBJECT_HEAD)sb,nCmdDraw & 0xFF);
	else
		OSD_DrawObject((POBJECT_HEAD)sb,nCmdDraw & 0xFF);		 	
}

//---------------------------- PUBLIC FUNCTIONS -----------------------------//
void OSD_GetObjListAspect(OBJLIST *ol,UINT16 *wDep, UINT16 *wCount)
{
	*wDep	= ol->wDep;
	*wCount	= ol->wCount;
}

BOOL OSD_GetObjListShift(OBJLIST *ol,  short wShift,UINT16* wNewTop,UINT16* wNewPoint)
{
	INT16 wPoint,wTop;
	UINT16 top,dep;
    UINT16 page_point;
    UINT32 page_moving;
	UINT16 check_cnt = 0;
	POBJECT_HEAD pItem;

    if(ol->wNewPoint < ol->wTop)
        ol->wNewPoint = ol->wCurPoint = ol->wTop;    
    if(ol->wNewPoint > ol->wCount)
        ol->wTop = ol->wNewPoint = ol->wCurPoint = 0;
        
    
    page_point = ol->wNewPoint - ol->wTop;//ol->wNewPoint % ol->wDep;
    
	if(ol->wCount==0)
		return FALSE;

	if (wShift==0)
		return FALSE;

	wPoint = ol->wNewPoint;
    wTop   = ol->wTop;

	do{
		
	    page_moving = (wShift == ol->wDep || wShift == -ol->wDep)? 1 : 0;
	    wPoint += wShift;

	    /* If move out of current page, the top point also need to move.*/
	    if( (wPoint < wTop) || (wPoint >= (wTop + ol->wDep) ) )
	    {
		if((ol->bListStyle & LIST_FIX_PAGE)&&(page_moving==0))
		{
			if(wShift>0)
				wTop += ol->wDep;
			else
				wTop -= ol->wDep;
		}
		else
		{
		        wTop += wShift;
		}
	    }

	    if(wTop == ol->wTop && wPoint < ol->wCount)
	    {
	    }
		else if(wTop < 0)
	    {
	        if(ol->wTop > 0)
	        {
	            wTop = 0;
	            
	            if(page_moving && (ol->bListStyle & LIST_PAGE_KEEP_CURITEM))
	                wPoint = page_point;
	            else
	            {
	            	wTop = wPoint;
	            }
	        }
	        else//ol->wTop == 0
	        {   
	            if (!(ol->bListStyle & LIST_SCROLL))
	                return FALSE;
	            
	            if(ol->bListStyle & LIST_FULL_PAGE)
	            {
	                wTop = ol->wCount - ol->wDep;
	                if(wTop < 0)
	                    wTop = 0;
	            }
	            else
	                wTop = (ol->wCount-1) / ol->wDep * ol->wDep;

	            if(page_moving && (ol->bListStyle & LIST_PAGE_KEEP_CURITEM))
	                wPoint = wTop + page_point;
	            else
	                wPoint = ol->wCount - 1;
	            if(wPoint >= ol->wCount)
	                wPoint = ol->wCount - 1;
	        }
	    }	    
	    else if(wPoint >= ol->wCount)
	    {
	        if(ol->wTop + ol->wDep < ol->wCount)
	        {
	            if(ol->bListStyle & LIST_FULL_PAGE)
	            {
	                wTop = ol->wCount - ol->wDep;
	                if(wTop < 0)
	                    wTop = 0;
	            }
	            else
	                wTop = (ol->wCount-1) / ol->wDep * ol->wDep;
	            
	            if(page_moving && (ol->bListStyle & LIST_PAGE_KEEP_CURITEM))
	                wPoint = wTop + page_point;
	            else
	                wPoint = ol->wCount - 1;
	            if(wPoint >= ol->wCount)
	                wPoint = ol->wCount - 1;
	        }
	        else
	        {
	            if (!(ol->bListStyle & LIST_SCROLL))
	                return FALSE;
	            wTop = 0;
	            if(page_moving && (ol->bListStyle & LIST_PAGE_KEEP_CURITEM))
	                wPoint = page_point;
	            else
	                wPoint = 0;   
	        }
	    }

		if(ol->bListStyle & LIST_ITEMS_COMPLETE)
			pItem = ol->pListField[wPoint];
		else
			pItem = ol->pListField[wPoint - wTop];

		if(OSD_CheckAttr(pItem, C_ATTR_ACTIVE))
			break;
		
		check_cnt++;
	}while(check_cnt<ol->wCount);

    *wNewPoint  = wPoint;
    *wNewTop    = wTop;
    return TRUE;
}

UINT16 OSD_GetObjListMultiSelItemNum(OBJLIST *ol)
{
	int i,j;
	UINT32 dw;
	UINT16 sum = 0;
	
	if(ol->dwSelect == NULL)
		return 0;
	
	for(i=0;i<ol->wCount/32 + 1;i++)
	{
		if(ol->dwSelect[i]==0)
			continue;
		dw = ol->dwSelect[i];
		for(j=0;j<32;j++)
		{
			if( (dw >> j) & 0x01)
				sum++;
		}
	}

	return sum;
}

BOOL OSD_CheckObjListMultiSelect(OBJLIST *ol, UINT16 wSelectIdx)
{
	UINT16	wIndex;
	UINT8	bShiftBit;

	wIndex		= wSelectIdx/32;
	bShiftBit	= (UINT8)wSelectIdx%32;
	if((ol->dwSelect[wIndex])&(0x00000001<<bShiftBit))
		return TRUE;
	else
		return FALSE;
}

void OSD_SwitchObjListMultiSelect(OBJLIST *ol, UINT16 wSwitchIdx)
{
	UINT16	wIndex;
	UINT8	bShiftBit;
	UINT16 	selItem;

	if(!(ol->bListStyle & LIST_MULTI_SLECT) )
		return;
	wIndex		= wSwitchIdx/32;
	bShiftBit	= wSwitchIdx%32;
	
	ol->dwSelect[wIndex] ^= (0x00000001<<bShiftBit);
}

/*Notice:The function can not be call before OSD_SetObjListCount,otherwise will cause bit flag residual*/
void 	OSD_ObjListDelItemUpdateSelect(OBJLIST *ol, UINT16 item)
{
	UINT32 i,n,sw,sw1;
	UINT32	wIndex,wIndex1;
	UINT32	bShiftBit,bShiftBit1;
	
	if(item >= ol->wCount)
		return ;
	
	if(ol->bListStyle & LIST_SINGLE_SLECT )
	{
		if(ol->wSelect > ol->wCount)
			return;
		
		if(ol->wSelect ==  item)
			ol->wSelect = 0xFFFF;
		else if(ol->wSelect >  item)
			ol->wSelect -= 1;
	}
	else if(ol->bListStyle & LIST_MULTI_SLECT )
	{
		if(ol->dwSelect == NULL)
			return;
		
		for(i = item; i<  (UINT32)(ol->wCount - 1); i++)
		{
		/*Moving following bits,but the last bit can not be cleared*/
			wIndex = i / 32;
			bShiftBit	= i % 32;

			wIndex1 = (i+1)/32;
			bShiftBit1 = (i+1)%32;

			sw = ol->dwSelect[wIndex];
			sw &=  ~(0x00000001<<bShiftBit); /*Only(Notice:only) Clear current bit*/
			
			sw1 = ol->dwSelect[wIndex1];
			sw1 = (sw1>>bShiftBit1) & 0x00000001;/*load next bit value*/

			sw |= (sw1<<bShiftBit);/*move back next bit to current position*/
			ol->dwSelect[wIndex] = sw;/*merge the bit value,but the last one will be residual*/
		}
/*Patch for can not clear the last dirty bit,when middle items was deleted*/	
		wIndex = (ol->wCount - 1)/32;
		bShiftBit = (ol->wCount - 1)%32;

		sw = ol->dwSelect[wIndex];
		sw &=  ~(0x00000001<<bShiftBit); /*Only(Notice:only) Clear current bit*/
		ol->dwSelect[wIndex] = sw;
	}

}

void OSD_SetObjListVerScrollBar(OBJLIST * ol, SCROLL_BAR* sb)
{
    UINT16 n;
    
	ol->scrollBar = sb;
	if(sb != NULL)
	{
		sb->head.pRoot = (POBJECT_HEAD)ol;
		if(ol->wCount<=ol->wDep)
			n = 0;
		else
			n = ol->wCount - 1;

		if(ol->bListStyle&LIST_BARSCROLL_COMPLETE)
			OSD_SetScrollBarPos(sb,ol->wCurPoint);
		else
			OSD_SetScrollBarPos(sb,ol->wTop);
		
        OSD_SetScrollBarMax(sb,n);
        OSD_SetScrollBarPage(sb,ol->wDep);
    }
}

void    OSD_SetObjListAspect(OBJLIST *ol,UINT16 cnt, UINT16 page,UINT16 top,  UINT16 focus)
{
	OSD_SetObjListCount(ol, cnt);
	OSD_SetObjListTop(ol, top);
	OSD_SetObjListCurPoint(ol, focus);
	OSD_SetObjListNewPoint(ol, focus);
	OSD_SetObjListVerScrollBar(ol,ol->scrollBar);	
}

PRESULT OSD_ObjListChgTopFocus(OBJLIST *ol, UINT16 newTop, UINT16 newPoint, UINT32 parm)
{
	UINT16 oldPoint;
	UINT32 sign_evnt, draw_obj,nCmdDraw,bUpdateAll;
	PRESULT Result,Retproc;
	POBJECT_HEAD pItem,pnewItem,pSelItem,pObj;
	
	bUpdateAll = 0;
	Result = PROC_PASS;
	Retproc = PROC_PASS;
	pObj = (POBJECT_HEAD)ol;
	oldPoint 	= ol->wCurPoint;
	sign_evnt = (parm & C_DRAW_SIGN_EVN_FLG) ? 1 : 0;
	draw_obj  = (OSD_GetUpdateType(parm)  == C_UPDATE_FOCUS) ? 1 : 0;
	if(ol->bListStyle & LIST_ITEMS_COMPLETE)
		pItem = ol->pListField[ol->wCurPoint];
	else
		pItem = ol->pListField[ol->wCurPoint - ol->wTop];
	

	if(newPoint == oldPoint)
		return PROC_LOOP;

	if(sign_evnt)
	{
		Result = OSD_SIGNAL(pObj, EVN_ITEM_PRE_CHANGE, oldPoint, newPoint);
		if(Result != PROC_PASS)
			return Result;
	}
		
	if(newTop != ol->wTop)
		bUpdateAll = 1;
	else
		bUpdateAll = 0;
	if(ol->bListStyle & LIST_ITEMS_COMPLETE)
		pnewItem = ol->pListField[newPoint];
	else
		pnewItem = ol->pListField[newPoint - newTop];
		
	if(pnewItem != pItem)
	{
		/* Previous item losting focus */
		if(sign_evnt)
		{
			Result = OSD_ObjProc(pItem,(MSG_TYPE_EVNT<<16)|EVN_FOCUS_PRE_LOSE,(UINT32)pnewItem, 0);
			if(Result != PROC_PASS)
				return Result;
		}

		if(!bUpdateAll)
		{
			//Retproc = OSD_SIGNAL(pObj,EVN_FOCUS_SLIDE,(UINT32)pItem,(UINT32)pnewItem);
#ifndef FOCUS_SLIDE_DISABLE					
			Retproc = (gelib_olst_focuslide(pItem,pnewItem,0) == 0) ? PROC_SKIP : PROC_PASS;
#else
			Retproc = PROC_PASS;
#endif
//			if(Retproc != PROC_SKIP)
			{
				nCmdDraw = sign_evnt? C_DRAW_SIGN_EVN_FLG : 0;
				nCmdDraw |= (C_UPDATE_FOCUS | C_DRAW_TYPE_NORMAL);
				if(draw_obj)
					OSD_DrawObject((POBJECT_HEAD)ol, nCmdDraw);
			}
		}
		if(sign_evnt)
		{
			Result = OSD_ObjProc(pItem,(MSG_TYPE_EVNT<<16) |EVN_FOCUS_POST_LOSE,(UINT32)pnewItem,0);
			if(Result != PROC_PASS)
				return Result;
		}
		ol->wCurPoint = ol->wNewPoint = newPoint;
		ol->wTop = newTop;
		if(sign_evnt)
		{
			Result = OSD_ObjProc(pnewItem,(MSG_TYPE_EVNT<<16)|EVN_FOCUS_PRE_GET,(UINT32)pItem, 0);
			if(Result != PROC_PASS)
				return Result;
		}
        
		if(draw_obj)
		{
			nCmdDraw = sign_evnt? C_DRAW_SIGN_EVN_FLG : 0;
			nCmdDraw |= bUpdateAll? C_UPDATE_ALL : C_UPDATE_FOCUS;
			nCmdDraw |= C_DRAW_TYPE_HIGHLIGHT;		
			OSD_TrackObject((POBJECT_HEAD)ol, nCmdDraw);
		}

		if(sign_evnt)
		{
			Result = OSD_ObjProc(pnewItem,(MSG_TYPE_EVNT<<16)|EVN_FOCUS_POST_GET,(UINT32)pItem,0);
			if(Result != PROC_PASS)
				return Result;       
		}
	}
	else
	{
		ol->wCurPoint = ol->wNewPoint = newPoint;
		ol->wTop = newTop;

		if(draw_obj)
		{
			nCmdDraw = sign_evnt? C_DRAW_SIGN_EVN_FLG : 0;
			nCmdDraw |= bUpdateAll? C_UPDATE_ALL : C_UPDATE_FOCUS;
			nCmdDraw |= C_DRAW_TYPE_HIGHLIGHT;		

			OSD_TrackObject((POBJECT_HEAD)ol, nCmdDraw);
		}
	}

	if(sign_evnt)
	{
		Result = OSD_SIGNAL(pObj, EVN_ITEM_POST_CHANGE, oldPoint, newPoint);
		if(Result != PROC_PASS)
			return Result;
	}

	return PROC_PASS;
}


PRESULT OSD_ObjListChgFocus(OBJLIST *ol, UINT16 newItem,UINT32 parm)
{
	BOOL b;
	short wShift;
	UINT16 newTop, newPoint;
		
	wShift = newItem - ol->wNewPoint;
	b = OSD_GetObjListShift(ol,wShift,&newTop,&newPoint);
	if(!b || wShift == 0)
		return PROC_LOOP;	

	return OSD_ObjListChgTopFocus(ol,newTop,newPoint,parm);	
}

void OSD_DrawObjectListCell(OBJLIST *ol,UINT8 bStyleIdx,UINT32 nCmdDraw)
{
	UINT8	dflag;
	UINT16	wNewTop;	
	UINT32	i,itemidx,update_type;
	PGUI_VSCR  pvscr;
	GUI_RECT *objframe,rctbak;
	PGUI_REGION prgn;

	update_type = OSD_GetUpdateType(nCmdDraw);
	objframe = &ol->head.frame;
	if(update_type == C_UPDATE_ALL)
	{
		if(OSD_GetAttrSlave(ol))
			pvscr = OSD_DrawSlaveFrame(objframe, bStyleIdx);
		else
			pvscr = OSD_DrawObjectFrame(objframe, bStyleIdx);
		
		ol->wCurPoint = ol->wNewPoint;
	}
	else if(OSD_GetAttrSlave(ol))
	{
		pvscr = OSD_GetTaskSlvscr(osal_task_get_current_id());
	}
	else
	{
		pvscr = OSD_GetTaskVscr(osal_task_get_current_id());
	}

	if(OSD_GetAttrSlave(ol))
		return;

	for(i=0;i<ol->wDep;i++)
	{
		itemidx = ol->wTop + i;
		if(update_type == C_UPDATE_FOCUS)
		{
			if(itemidx != ol->wCurPoint && itemidx != ol->wNewPoint)
			continue;
		}

		if(update_type != C_UPDATE_ALL || itemidx != ol->wCurPoint)
		{
			OSDObjListShowItem(ol,ol->wTop + i,nCmdDraw);
		}
	}

	if(update_type == C_UPDATE_ALL
		&& ol->scrollBar!=NULL 
		&& !OSD_CheckAttr((POBJECT_HEAD)ol,C_ATTR_HIDDEN) )
		OSD_UpdateObjListVerScrollBar(ol, nCmdDraw);

	if(update_type == C_UPDATE_ALL)
	{
		OSDObjListShowItem(ol, ol->wCurPoint, nCmdDraw);
	}

	/*if(update_type == C_UPDATE_FOCUS)
	{
		dflag = pvscr->dirty_flag;
		
		pvscr->dirty_flag = 1;
		MEMCPY(&rctbak,&(pvscr->frm),sizeof(GUI_RECT));
		MEMCPY(&(pvscr->frm),objframe,sizeof(GUI_RECT));
		OSD_UpdateVscr(pvscr);
		
		pvscr->dirty_flag = dflag;
		MEMCPY(&(pvscr->frm),&rctbak,sizeof(GUI_RECT));
	}*/
}

/////////////////////////////////////////////////////////////////////
//keymap & proc
VACTION    OSD_ObjListDefMap(OBJECT_HEAD* ObjHead, UINT32 dInput)
{
    VACTION       Vact = VACT_PASS;
    OBJLIST       *ol = (OBJLIST *)ObjHead;


    switch(dInput)
    {
    case V_KEY_UP:
        Vact = VACT_CURSOR_UP;
        break;
    case V_KEY_P_UP:
    	Vact = VACT_CURSOR_PGUP;
        break;
    case V_KEY_DOWN:
        Vact = VACT_CURSOR_DOWN;
        break;
    case V_KEY_P_DOWN:
    	Vact = VACT_CURSOR_PGDN;
    	break;
    case V_KEY_ENTER:
        Vact = VACT_SELECT;
        break;
    default:
        ;
    }
    
    return Vact;

}

PRESULT OSD_ObjectListProc(POBJECT_HEAD pObj, UINT32 msg_type, UINT32 msg,UINT32 param1)
{
	PRESULT Result;
	OBJLIST  *ol = (OBJLIST *)pObj;
	
	INT16 nSel, nStep;
	UINT16 oldPoint,newPoint,newTop;
	UINT16 oldSelect,newSelect;
	BOOL bSelect,b;
	UINT32 nCmdDraw;
	UINT32 bUpdateAll = 0;
    
	OBJECT_HEAD* pItem,*pnewItem,*pSelItem;
 	UINT32 subEvnt;
 	UINT32 submsg;
	
	if(ol->bListStyle & LIST_ITEMS_COMPLETE)
		pItem = ol->pListField[ol->wCurPoint];
	else
		pItem = ol->pListField[ol->wCurPoint - ol->wTop];
		
	if(MSG_TYPE_KEY == msg_type)
	{		

		VACTION Action;
		UINT32 vkey;
		BOOL bContinue;		
	
		Result = OSD_ObjProc(pItem,(msg_type<<16), msg,param1);
		if(Result != PROC_PASS)
			goto CHECK_LEAVE;		
	
	    Result = OSD_ObjCommonProc(pObj,msg,param1,&vkey,&Action,&bContinue);
	    if(!bContinue)
	    	goto CHECK_LEAVE;
	        
		Result = PROC_LOOP;
		switch (Action)
		{
		case VACT_CURSOR_UP:
	        nStep = -1;
	        goto list_move_cursor;
	    case VACT_CURSOR_DOWN:
	        nStep = 1;
	        goto list_move_cursor;
	    case VACT_CURSOR_PGUP:
	        nStep = -ol->wDep;
	        goto list_move_cursor;
	    case VACT_CURSOR_PGDN:
	        nStep = ol->wDep;
	list_move_cursor:
			oldPoint 	= ol->wCurPoint;
	
			b = OSD_GetObjListShift(ol,nStep,&newTop,&newPoint);
			if(newPoint == oldPoint || !b)
			{
				Result = PROC_LOOP;
				goto EXIT;
			}

			submsg = C_DRAW_SIGN_EVN_FLG | C_UPDATE_FOCUS;
			OSD_ObjListChgTopFocus(ol,newTop,newPoint,submsg);
			Result = PROC_LOOP;
			break;
		case VACT_SELECT:
			
			newSelect = ol->wCurPoint;		
			if(ol->bListStyle & LIST_SINGLE_SLECT)
			{
				oldSelect = ol->wSelect;
				
				
				if(oldSelect > ol->wCount)
					oldSelect = 0xFFFF; 
				Result = OSD_SIGNAL(pObj, EVN_PRE_CHANGE, oldSelect, newSelect);
				if(Result != PROC_PASS)
					goto CHECK_LEAVE;
					
				//If previous selection can be viewed,remove selection
				if(oldSelect>= ol->wTop && oldSelect<=ol->wCount)
				{
					if(ol->bListStyle & LIST_ITEMS_COMPLETE)
						pSelItem = ol->pListField[oldSelect];
					else
						pSelItem = ol->pListField[oldSelect - ol->wTop];
					
					ol->wSelect = 0xFFFF;
					nCmdDraw = C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL | C_DRAW_TYPE_NORMAL;
					OSDObjListShowItem(ol,oldSelect,nCmdDraw);
				}
				
				ol->wSelect = newSelect;
				nCmdDraw = C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL | C_DRAW_TYPE_HIGHLIGHT;
				OSDObjListShowItem(ol,newSelect,nCmdDraw);
				Result = OSD_SIGNAL(pObj, EVN_POST_CHANGE, oldSelect, newSelect);						
				if(Result != PROC_PASS)
					goto CHECK_LEAVE;
				
				Result = PROC_LOOP;
			}
			else if(ol->bListStyle & LIST_MULTI_SLECT)
			{
				bSelect = OSD_CheckObjListMultiSelect(ol,ol->wCurPoint);
				Result  = OSD_SIGNAL(pObj, EVN_PRE_CHANGE, ol->wCurPoint, bSelect? 0 : 1);
				if(Result != PROC_PASS)
					goto CHECK_LEAVE;
				
				OSD_SwitchObjListMultiSelect(ol,ol->wCurPoint);				
				nCmdDraw = C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL | C_DRAW_TYPE_HIGHLIGHT;
				OSDObjListShowItem(ol,ol->wCurPoint,nCmdDraw);
				
				Result  = OSD_SIGNAL(pObj, EVN_PRE_CHANGE, ol->wCurPoint, bSelect? 0 : 1);
				if(Result != PROC_PASS)
					goto CHECK_LEAVE;
				
				Result = PROC_LOOP;
			}
			else
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
	else
	{
		if(OSD_EventPassToChild(msg_type,&submsg))
			Result = OSD_ObjProc(pItem, (MSG_TYPE_EVNT<<16)|submsg,msg, param1);
		else
			Result = PROC_PASS;
			
		if(Result == PROC_PASS)
			Result = OSD_SIGNAL(pObj, msg_type, msg, param1);					
	}
	
CHECK_LEAVE:
	if(!(EVN_PRE_OPEN == msg_type))
		CHECK_LEAVE_RETURN(Result,pObj);
	
EXIT:	
	return Result;
}

