/*-----------------------------------------------------------------------------
    Copyright (C) 2010 ALi Corp. All rights reserved.
    File: obj_XXX.c
-----------------------------------------------------------------------------*/
#include <sys_config.h>
#include <api/libge/osd_lib.h>
#include "osd_lib_internal.h"

#define LS_PRINTF(...)

BOOL CheckItem(PLIST l, INT16 nPos)
{
	UINT16	wIndex;
	UINT8	bShiftBit;
	UINT32	tmp,tmp2;

	if(nPos >= l->nCount)
		return TRUE;
	if(l->item_status == NULL)
		return TRUE;
	wIndex	= nPos / 16;
	bShiftBit	= (UINT8)nPos % 16;
	bShiftBit *= 2;
	tmp = (l->item_status[wIndex]) & (0x03 << bShiftBit);
	tmp2 = 0x02 << bShiftBit;
	if(tmp == tmp2) // 10=hide,11=sel
		return FALSE;
	else
		return TRUE;
}

void OSD_DrawListItem(PLIST l, INT16 nItem, UINT32 nCmdDraw)
{//MKSZ,need tail override if use list
	UINT8 lstfont,halign,valign,draw_type,update_type,bShIdx;
	INT16 k,nIndex,wNextX;
	unsigned short wsNum[8];
	UINT16** pTextTable;
	UINT16 *pValue,*pText ;
	GUI_POINT pos;
	GUI_RECT rcField,rcItem, rcContent;
	PGUI_VSCR	pScr;
	lpLISTFIELD pf;
	OBJECTINFO obj;
	PWINSTYLE pws;
	POBJECT_HEAD pObj;
	
	wNextX = 0;
	pText = NULL;
	pObj = &l->head;
	pf = l->pFieldTable;
	draw_type = OSD_GetDrawType(nCmdDraw);
	update_type = OSD_GetUpdateType(nCmdDraw);
	bShIdx = pObj->style.bShowIdx;
	if(nItem < 0)
		return;

	nIndex = nItem - l->nTop;

	if(l->nCount)
	{
		if(nItem == l->nPos)
		{
			if(OSD_CheckListSelect(l, nItem))
				bShIdx = pObj->style.bSelIdx;
			if(draw_type == C_DRAW_TYPE_HIGHLIGHT)
				bShIdx = pObj->style.bHLIdx;

		}
		else
		{
			if(OSD_CheckListSelect(l, nItem))
				bShIdx = pObj->style.bSelIdx;
			else
				bShIdx = pObj->style.bShowIdx;

		}
		if(!CheckItem(l, nItem))
			bShIdx = pObj->style.bGrayIdx;
	}
	pws = g_gui_rscfunc.osd_get_win_style(bShIdx);

	MEMCPY(&rcItem,&l->head.frame,sizeof(GUI_RECT));
	rcItem.uStartY += (nItem - l->nTop) * (rcItem.uHeight + l->bIntervalY);

	if(nItem >= l->nCount)
	{
		if(l->bListStyle&LIST_GRID) // ONLY for this style
		{
			if(OSD_GetAttrSlave(l))
				pScr = OSD_DrawSlaveFrame(&rcItem, bShIdx);
			else
				pScr = OSD_DrawObjectFrame(&rcItem, bShIdx);//MKSZ
			OSD_SetVscrModified(pScr);
		}
		return;
	}

	if(OSD_GetAttrSlave(l))
	{
		lstfont = (pObj->bFont)|C_FONT_SLVMASK;
		pScr = OSD_DrawSlaveFrame(&rcItem, bShIdx);
	}
	else	
	{
		lstfont = pObj->bFont;
		pScr = OSD_DrawObjectFrame(&rcItem, bShIdx);
	}
	OSD_SetVscrModified(pScr);

 	for(k = 0; k < l->bNumField; k++)
	{
		MEMCPY(&rcContent, &rcItem,sizeof(GUI_RECT));
		rcContent.uStartX += pf[k].bIntervalL;
		rcContent.uStartY += pf[k].bIntervalT;
		rcContent.uHeight -= (pf[k].bIntervalT * 2);	
		rcContent.uWidth -= pf[k].bIntervalL;	
		MEMCPY(&rcField, &rcContent,sizeof(GUI_RECT));
		rcField.uStartX += wNextX;
		rcField.uWidth = pf[k].nWidth;
		wNextX += rcField.uWidth;

		if(!OSD_CheckListSelect(l, nItem))
		{
			if(OSD_GetAttrSlave(l))
				pScr = OSD_DrawSlaveFrame(&rcItem, bShIdx);
			else	
				pScr = OSD_DrawObjectFrame(&rcField, pf[k].bSHStyle);
			OSD_SetVscrModified(pScr);
		}

		pTextTable = (UINT16 **)pf[k].pContent;
		pValue = (UINT16 *)pf[k].pContent;
		switch (pf[k].bType)
		{
		case FIELD_IS_ICON:
			if(pValue[nIndex])  // != NULL
			{
				halign = GET_HALIGN(pf[k].bAssignType);
				valign = GET_VALIGN(pf[k].bAssignType);
				g_gui_rscfunc.osd_get_obj_info(LIB_ICON, pValue[nIndex], &obj);				
				if(obj.m_objAttr.m_wActualWidth < rcField.uWidth)
				{
					if(halign == C_ALIGN_RIGHT)
						rcField.uStartX += (rcField.uWidth - obj.m_objAttr.m_wActualWidth);
					else if(halign == C_ALIGN_CENTER)
						rcField.uStartX += ((rcField.uWidth - obj.m_objAttr.m_wActualWidth) / 2);
				}
				if(obj.m_objAttr.m_wHeight < rcField.uHeight)
				{
					if(valign == C_ALIGN_BOTTOM)
						rcField.uStartY += (rcField.uHeight - obj.m_objAttr.m_wHeight);
					else if(valign == C_ALIGN_VCENTER)
						rcField.uStartY += ((rcField.uHeight - obj.m_objAttr.m_wHeight) / 2);
				}

				pos.uX = rcField.uStartX;
				pos.uY = rcField.uStartY;
				OSD_DrawBmp2Vscr(&pos,pValue[nIndex],pScr);
			}
			break;
		case FIELD_IS_NUMBER:
			if(!pValue || 0xFFFF == pValue[nIndex])
				wsNum[0] = 0;
			else
				ComInt2UniStr(wsNum, pValue[nIndex], 0);
			pText = wsNum;
			goto draw_item_text;
		case FIELD_IS_COUNT:
			ComInt2UniStr(wsNum, nItem+1, 0);
			pText = wsNum;
			goto draw_item_text;
		case FIELD_IS_STRING_ID:
			pText = (UINT16*)OSD_GetUnicodeString((UINT16)(pValue[nIndex]));
			goto draw_item_text;
		case FIELD_IS_TEXT:
			pText = pTextTable[nIndex];
draw_item_text:
			OSD_DrawText(&rcField, (UINT8*)pText, pws->wFgIdx, pf[k].bAssignType,lstfont, pScr);
			break;
		}
	}
}

PRESULT OSD_ListMoveCursor(PLIST l, INT16 nStep)
{
	INT16 nPos, nTop, nTemp;
	PRESULT Result = PROC_PASS;
	INT16 nOldPos = l->nPos;

	nTemp = (nStep < 0) ? -nStep : nStep;
	if(l->nCount <= nTemp)
		return PROC_LOOP;

	nTemp = l->nPos + nStep;
	nPos = (nTemp + l->nCount) % l->nCount;
	while(!CheckItem(l, nPos))
	{
		nTemp += (nStep < 0) ? -1 : 1;
		nPos = (nTemp + l->nCount) % l->nCount;
	}
		
	if((l->nCount <= l->bPage) || (nTemp >= l->nTop && nTemp < (l->nTop + l->bPage)))
	{
		nTop = l->nTop;
		Result = OSD_SIGNAL((POBJECT_HEAD)l, EVN_ITEM_PRE_CHANGE, nOldPos, nPos);
		if(Result != PROC_PASS)
			return Result;
		l->nTop = nTop;
		l->nPos = nPos;

		OSD_DrawListItem(l, nOldPos, C_UPDATE_FOCUS); //C_UPDATE_CURSOR
		OSD_TrackObject((POBJECT_HEAD)l, C_UPDATE_FOCUS);
		Result = OSD_SIGNAL((POBJECT_HEAD)l,  EVN_ITEM_POST_CHANGE, nPos, nOldPos);
		return Result;
	}

	if(nStep < 0)
	{
		if(!l->nTop)
			nTop = l->nCount - l->bPage;
		else
		{
			nTop = l->nTop + nStep;
			if(nTop < 0)
			{
				nTop = 0;
				nPos = l->nPos- l->nTop;
			}
		}
		while(nPos - nTop < 0)
			nTop--;			
	}
	else
	{
		if(l->nTop == (l->nCount - l->bPage))
			nTop = 0;
		else
		{
			nTop = l->nTop + nStep;
			if(nTop > (l->nCount - l->bPage))
			{
				nTop = l->nCount - l->bPage;
				nPos = nTop + l->nPos - l->nTop;
			}
		}
		while(nPos - nTop >= l->bPage)
			nTop++;
	}
	Result = OSD_SIGNAL((POBJECT_HEAD)l,  EVN_ITEM_PRE_CHANGE,  nOldPos, nPos);
	if(Result != PROC_PASS)
		return Result;
	l->nTop = nTop;
	l->nPos = nPos;

	Result = OSD_SIGNAL((POBJECT_HEAD)l, EVN_REQUEST_DATA, nPos, nTop);
	if(Result != PROC_PASS)
		return Result;
	OSD_TrackObject((POBJECT_HEAD)l, C_UPDATE_ALL);
	Result = OSD_SIGNAL((POBJECT_HEAD)l,  EVN_ITEM_POST_CHANGE, nPos, 0);
	return Result;
}

static void UpdateListVerScrollBar(LIST *l, UINT32 nCmdDraw)
{
	SCROLL_BAR *sb;	

	UINT32 nSbCmdDraw = nCmdDraw & 0xFFFFFF0F;
	UINT32 objDrawType  = OSD_GetDrawType(nCmdDraw);
	
	sb = l->scrollBar;
	if(sb==NULL)
	    return;

	OSD_SetScrollBarMax(sb, l->nCount);
	OSD_SetScrollBarPos(sb, l->nTop);
    
	if(objDrawType == C_DRAW_TYPE_HIGHLIGHT)
		OSD_TrackObject((POBJECT_HEAD)sb, nCmdDraw & 0xFF);   
	else if(objDrawType == C_DRAW_TYPE_SELECT)
		OSD_SelObject((POBJECT_HEAD)sb, nCmdDraw & 0xFF);
	else
		OSD_DrawObject((POBJECT_HEAD)sb, nCmdDraw & 0xFF);		 	
}

void OSD_SetListVerScrollBar(LIST * l, SCROLL_BAR* sb)
{
	UINT16 n;

	l->scrollBar = sb;
	if(sb != NULL)
	{
		sb->head.pRoot = (POBJECT_HEAD)l;
		if(l->nCount <= l->bPage)
			n = 0;
		else
			n = l->nCount - 1;
		OSD_SetScrollBarPos(sb, l->nTop);
		OSD_SetScrollBarMax(sb, n);
	}
}

UINT16 OSD_GetListPageSize(LIST *l)
{
	return l->bPage;
}

void OSD_SetListPageSize(LIST *l, UINT8 cur_page)	
{
	l->bPage = cur_page;
}

UINT16 OSD_GetListCurPoint(LIST *l)	
{
	return l->nPos;
}

void OSD_SetListCurPoint(LIST *l, UINT16 cur_pos)
{
	if(cur_pos < l->nCount)
	{
		l->nPos = cur_pos;
	}
	else
	{
		if(l->nCount > 0)
			l->nPos =  l->nCount -1;
		else
			l->nPos = 0;
	}
	
	if(l->nPos < l->nTop || l->nPos >= (l->nTop + l->bPage))
	{
		if(l->nCount >= l->bPage)
			l->nTop = l->nPos;
		else
			l->nTop = 0;
	}
}

UINT16 OSD_GetListTopPoint(LIST *l)
{
	return l->nTop;
}

void OSD_SetListTopPoint(LIST *l, UINT16 wTop)
{
	if((wTop < l->nCount) && (l->nCount > l->bPage))
	{
		UINT8 tmp;
		tmp = l->nPos - l->nTop;
		l->nTop = ((l->nCount - wTop) < l->bPage) ? (l->nCount - l->bPage) : wTop;
		l->nPos = l->nTop + tmp;
	}
	else
	{
		if(l->nCount > l->bPage)
			l->nTop = l->nCount - l->bPage;
		else
			l->nTop = 0;
	}
}

UINT16 OSD_GetListCount(LIST *l)
{
	return l->nCount;
}

void OSD_SetListCount(LIST *l, UINT16 count)
{
	l->nCount = count;
	if((l->nPos >= count)&& (0 != count))
		l->nPos = count-1;
}

void OSD_SetListContent(LIST * l, UINT8 bFieldN, lpLISTFIELD pListField)
{
	l->bNumField = bFieldN;
	l->pFieldTable = pListField;
}

void OSD_SetListStyle(LIST *l, UINT16  bListStyle)
{
	if(bListStyle != 0xFF)
		l->bListStyle = bListStyle;
}

void  OSD_SwitchListSelect(LIST *l, UINT16 uSelPoint)
{
	UINT16	wIndex;
	UINT8	bShiftBit;

	if(uSelPoint < l->nCount && l->item_status != NULL)
	{
		wIndex	= uSelPoint / 16;
		bShiftBit	= (UINT8)uSelPoint % 16;
		bShiftBit *= 2;
		(l->item_status[wIndex]) ^= (0x03 << bShiftBit);
	}
}

PRESULT  OSD_SetListCurSel(LIST *l, UINT16 uSelPoint)
{
	UINT16	wIndex;
	UINT8	bShiftBit;
	PRESULT Result;
	BOOL bSelect;

	bSelect = OSD_CheckListSelect(l, uSelPoint);
	Result = OSD_SIGNAL((POBJECT_HEAD)l, EVN_PRE_CHANGE,  (UINT32)&uSelPoint, (UINT32)&bSelect);
	if(PROC_PASS == Result)
	{
		OSD_SwitchListSelect(l,uSelPoint);
		Result = OSD_SIGNAL((POBJECT_HEAD)l, EVN_POST_CHANGE, uSelPoint, (UINT32)&bSelect);
	}
	return Result;
}

void OSD_SetListCurHide(LIST *l, UINT16 uHidePoint)
{
	UINT16	wIndex;
	UINT8	bShiftBit;

	if(uHidePoint < l->nCount && l->item_status != NULL)
	{
		wIndex	= uHidePoint / 16;
		bShiftBit	= (UINT8)uHidePoint % 16;
		bShiftBit *= 2;
		(l->item_status[wIndex]) |= (0x02 << bShiftBit);
	}
}

BOOL OSD_CheckListSelect(LIST *l, UINT16 wSelectIdx)
{
	UINT16	wIndex;
	UINT8	bShiftBit;
	UINT32	tmp,tmp2;

	if(l->item_status == NULL)
		return FALSE;
	wIndex	= wSelectIdx / 16;
	bShiftBit	= (UINT8)wSelectIdx % 16;
	bShiftBit *= 2;
	tmp = (l->item_status[wIndex]) & (0x03 << bShiftBit);
	tmp2 = 0x03 << bShiftBit;
	if(tmp == tmp2) // 10=hide,11=sel
		return TRUE;
	else
		return FALSE;
}

void OSD_DrawListCell(PLIST l, UINT8 bStyleIdx, UINT32 nCmdDraw)
{
	UINT16 i;
	UINT32 lstyle,update_type,nSubCmdDraw;
	PGUI_VSCR pScr;
	if(OSD_GetUpdateType(nCmdDraw) == C_UPDATE_CONTENT)
		lstyle = (UINT32)(~0);
	else
		lstyle = bStyleIdx;
	
	pScr = NULL;
	nSubCmdDraw = nCmdDraw;
	if(OSD_GetAttrSlave(&l))
		OSD_GetSlvscr(&(l->head.frame),VSCR_GET_BACK);
	else
		pScr = OSD_GetVscr(&(l->head.frame),VSCR_GET_BACK);
	update_type = OSD_GetUpdateType(nCmdDraw);
	if(C_UPDATE_ALL == update_type || C_UPDATE_CONTENT == update_type)
	{
		for(i=l->nTop; i<(l->nTop+l->bPage); i++)
		{
            if(i != l->nPos)
			    OSD_DrawListItem(l, i, nSubCmdDraw);
		}
        OSD_DrawListItem(l, l->nPos, nSubCmdDraw);
	}
	else if(C_UPDATE_CURSOR == update_type)
		OSD_DrawListItem(l, l->nSel, nSubCmdDraw);
	else if(C_UPDATE_FOCUS == update_type)
		OSD_DrawListItem(l, l->nPos, nSubCmdDraw);
	if(update_type == C_UPDATE_ALL && l->scrollBar!=NULL && !OSD_CheckAttr((POBJECT_HEAD)l,C_ATTR_HIDDEN) )
		UpdateListVerScrollBar(l, nSubCmdDraw);
}

VACTION OSD_ListKeyMap(POBJECT_HEAD pObj, UINT32 vkey)
{
	VACTION	Vact = VACT_PASS;
	LIST		*l = (LIST *)pObj;

	switch(vkey)
	{
		case V_KEY_UP:
			if((l->bListStyle & LIST_SCROLL) || (l->nPos != 0))			
				Vact = VACT_CURSOR_UP;
			else
				Vact = VACT_PASS;
			break;
		case V_KEY_DOWN:
			if((l->nPos != (l->nCount - 1)) || (l->bListStyle & LIST_SCROLL))
				Vact = VACT_CURSOR_DOWN;
			else
				Vact = VACT_PASS;
			break;
		case V_KEY_P_UP:
			Vact = VACT_CURSOR_PGUP;
			break;
		case V_KEY_P_DOWN:
			Vact = VACT_CURSOR_PGDN;
			break;
		case V_KEY_ENTER:
			Vact = VACT_SELECT;
			break;
		default:
			Vact = VACT_PASS;
	}
	return Vact;
}

PRESULT OSD_ListProc(POBJECT_HEAD pObjHead, UINT32 msg_type, UINT32 msg, UINT32 param1)
{
	PRESULT Result = PROC_PASS;
	LIST		*l = (LIST*)pObjHead;
	INT16 nStep, oldPoint, newTop, newPoint;	
	UINT16 oldSelect,newSelect;
	BOOL b, bSelect;
	UINT8 bUpdateAll;
	UINT32 submsg, nCmdDraw;
	
	if(msg_type == MSG_TYPE_KEY)
	{
		VACTION Action;
		UINT32 vkey;
		BOOL bContinue;

		Result = OSD_ObjCommonProc(pObjHead, msg, param1, &vkey, &Action, &bContinue);
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
			        nStep = -l->bPage;
			        goto list_move_cursor;
			case VACT_CURSOR_PGDN:
			        nStep = l->bPage;
list_move_cursor:	
				Result = OSD_ListMoveCursor(l, nStep);
				UpdateListVerScrollBar(l, TRUE);
				break;
			case VACT_SELECT:
				Result = OSD_SetListCurSel(l, l->nPos);
				UpdateListVerScrollBar(l, TRUE);
				break;
		    	case VACT_CLOSE:
CLOSE_OBJECT:
		    		Result = OSD_ObjClose(pObjHead, C_CLOSE_CLRBACK_FLG);
		    		goto EXIT;
		    		break;
			default:
				Result = OSD_SIGNAL(pObjHead, EVN_UNKNOWN_ACTION, (Action<<16) | vkey, param1);
				break;
		}
	}
	else// if(msg_type==MSG_TYPE_EVNT)
		Result = OSD_SIGNAL(pObjHead, msg_type, msg, param1);

CHECK_LEAVE:
	if(!(EVN_PRE_OPEN == msg_type))
		CHECK_LEAVE_RETURN(Result, pObjHead);
EXIT:	
	return Result;
}

