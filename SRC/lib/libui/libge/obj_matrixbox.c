/*-----------------------------------------------------------------------------
    Copyright (C) 2010 ALi Corp. All rights reserved.
    File: obj_XXX.c
-----------------------------------------------------------------------------*/
#include <sys_config.h>
#include <api/libge/osd_lib.h>
#include "osd_lib_internal.h"

static UINT8 GetShowIndex(PMATRIX_BOX mb, UINT16 nPos)
{
	UINT8 bAttr = C_ATTR_ACTIVE;
	UINT8 sty = 0;
	POBJECT_HEAD pObj = &mb->head;	
	PMATRIX_BOX_NODE pNode = (PMATRIX_BOX_NODE)mb->pCellTable;

	if(C_ATTR_SELECT == ((pNode[nPos].bAttr)&C_ATTR_LMASK))
		sty = pObj->style.bSelIdx;
	else if(C_ATTR_INACTIVE == (C_ATTR_LMASK&(pNode[nPos].bAttr)))
		sty = pObj->style.bGrayIdx;//mb->bBackColor;
	else if(C_ATTR_ACTIVE == (C_ATTR_LMASK&(pNode[nPos].bAttr)))
		sty = pObj->style.bShowIdx;

	return sty;
}

void OSD_DrawMatrixBoxItem(PMATRIX_BOX mb, UINT16 nCol, UINT16 nRow, UINT32 nCmdDraw)
{//SZMK
	UINT8 mtxfont,bShIdx,bMixer;
	UINT16 nPos;
	unsigned short  wsBuf[8];
	UINT32 color_param, style_param,draw_type,update_type;
	GUI_POINT pos;
	GUI_RECT rcCell, rcText;
	PGUI_VSCR	pScr;
	PWINSTYLE pws;
	POBJECT_HEAD pObj;
	
	bMixer = 0;
	pObj = &mb->head;
	draw_type = OSD_GetDrawType(nCmdDraw);
	update_type = OSD_GetUpdateType(nCmdDraw);
	nPos = nRow * mb->nCol + nCol;

	bShIdx = GetShowIndex(mb, nPos);
	if(mb->nCount && (nPos == mb->nPos))
	{
		if(draw_type == C_DRAW_TYPE_HIGHLIGHT)
			bShIdx = pObj->style.bHLIdx;
		else if(draw_type == C_DRAW_TYPE_SELECT)
			bShIdx = pObj->style.bSelIdx;
	}	
	pws = g_gui_rscfunc.osd_get_win_style(bShIdx);
	
	MEMCPY(&rcCell, &mb->head.frame,sizeof(GUI_RECT));
	rcCell.uStartX += nCol * (rcCell.uWidth + mb->bIntervalX);
	rcCell.uStartY += nRow * (rcCell.uHeight + mb->bIntervalY);

	if(nPos >= mb->nCount)
	{
		return;
	}

	if(OSD_GetAttrSlave(mb))
	{
		mtxfont = (pObj->bFont)|C_FONT_SLVMASK;
		pScr = OSD_DrawSlaveFrame(&rcCell, bShIdx);
	}
	else
	{
		mtxfont = (pObj->bFont)|0;
		pScr = OSD_DrawObjectFrame(&rcCell, bShIdx);
	}
	OSD_SetVscrModified(pScr);

	MEMCPY(&rcText, &rcCell,sizeof(GUI_RECT));
	rcText.uStartX += mb->bIntervalTl;
	rcText.uStartY += mb->bIntervalTt;
	rcText.uWidth -= mb->bIntervalTl; // not *2
	UINT16* pText;
	UINT8 ctype = mb->bCellType;
	
	if(ctype & MATRIX_TYPE_NUMBER)
	{
		ComInt2UniStr(wsBuf, ((PMATRIX_BOX_NODE)(mb->pCellTable))[nPos].num, 0);
		pText = wsBuf;
		OSD_DrawText(&rcText, (UINT8*)pText, pws->wFgIdx, mb->bAssignType,mtxfont, pScr);
	}
	else if(ctype & MATRIX_TYPE_BITMAP)
	{
		OBJECTINFO obj;
		GUI_RECT rcIcon;		
		MEMCPY(&rcIcon, &rcCell,sizeof(GUI_RECT));
		rcIcon.uStartX += mb->bIntervalIl;
		rcIcon.uStartY += mb->bIntervalIt;
		rcIcon.uWidth -= (mb->bIntervalIl * 2);
		rcIcon.uHeight -= (mb->bIntervalIt * 2);
		g_gui_rscfunc.osd_get_obj_info(LIB_ICON, ((PMATRIX_BOX_NODE)(mb->pCellTable))[nPos].num, &obj);
		if(obj.m_objAttr.m_wActualWidth < rcIcon.uWidth)
		{
			if(GET_HALIGN(mb->bAssignType) == C_ALIGN_CENTER)
				rcIcon.uStartX += ((rcIcon.uWidth - obj.m_objAttr.m_wActualWidth) / 2);
			else if(GET_HALIGN(mb->bAssignType) == C_ALIGN_RIGHT)
				rcIcon.uStartX += (rcIcon.uWidth - obj.m_objAttr.m_wActualWidth);
		}
		if(obj.m_objAttr.m_wHeight < rcIcon.uHeight)
		{
			if(GET_VALIGN(mb->bAssignType) == C_ALIGN_VCENTER)
				rcIcon.uStartY += ((rcIcon.uHeight - obj.m_objAttr.m_wHeight) / 2);
			else if(GET_VALIGN(mb->bAssignType) == C_ALIGN_BOTTOM)
				rcIcon.uStartY += (rcIcon.uHeight - obj.m_objAttr.m_wHeight);
		}

		pos.uX = rcIcon.uStartX;
		pos.uY = rcIcon.uStartY;
		OSD_DrawBmp2Vscr(&pos, ((PMATRIX_BOX_NODE)(mb->pCellTable))[nPos].num,pScr);
	}

	if(ctype & MATRIX_TYPE_STRID)
	{
		pText = (UINT16*)OSD_GetUnicodeString(((PMATRIX_BOX_NODE)(mb->pCellTable))[nPos].str);
		OSD_DrawText(&rcText, (UINT8*)pText, pws->wFgIdx, mb->bAssignType,mtxfont, pScr);
	}
	else if(ctype & MATRIX_TYPE_WCHAR)
	{
		pText =(UINT16*) (((PMATRIX_BOX_NODE)(mb->pCellTable))[nPos].str);
		OSD_DrawText(&rcText, (UINT8*)pText, pws->wFgIdx, mb->bAssignType,mtxfont,pScr);
	}
}

BOOL CheckCell(PMATRIX_BOX m, UINT16 nPos)
{
	if(nPos >= m->nCount)
		return FALSE;

	PMATRIX_BOX_NODE pNode = (PMATRIX_BOX_NODE)m->pCellTable;
	return (C_ATTR_INACTIVE!= (C_ATTR_LMASK&(pNode[nPos].bAttr)));
}

static PRESULT NotifyCursorEvent(PMATRIX_BOX mb, UINT16 nPos)
{
	UINT16 nOldPos;
	UINT16 nCol, nRow;
	PRESULT Result;

	nOldPos = mb->nPos;
	Result = OSD_SIGNAL((POBJECT_HEAD)mb, EVN_ITEM_PRE_CHANGE, nOldPos, nPos);
	if(Result != PROC_PASS)
		return Result;
		
	mb->nPos = nPos;
	OSD_GetMtrxBoxCellLocation(mb, nOldPos, &nCol, &nRow);	
	OSD_DrawMatrixBoxItem(mb, nCol, nRow, C_UPDATE_FOCUS);
	OSD_GetMtrxBoxCellLocation(mb, nPos, &nCol, &nRow);		
	OSD_DrawMatrixBoxItem(mb, nCol, nRow, C_UPDATE_FOCUS);
	OSD_TrackObject((POBJECT_HEAD)mb, C_UPDATE_CONTENT);	
	OSD_SIGNAL((POBJECT_HEAD)mb, EVN_ITEM_POST_CHANGE, nPos, nOldPos);

	return Result;
}

//---------------------------- PUBLIC FUNCTIONS -----------------------------//
void OSD_GetMtrxBoxCellLocation(PMATRIX_BOX m, UINT16 nPos, UINT16 *pCol, UINT16 *pRow)
{
	*pCol = nPos % m->nCol;
	*pRow = nPos / m->nCol;
}

void OSD_DrawMatrixBoxCell(PMATRIX_BOX mb, UINT8 bStyleIdx, UINT32 nCmdDraw)
{
	UINT16 i, j;
	UINT32 update_type;
	UINT32	nSubCmdDraw;
	
	nSubCmdDraw = nCmdDraw;	
	update_type = OSD_GetUpdateType(nCmdDraw);
	if(C_UPDATE_CURSOR == update_type)
	{
		OSD_GetMtrxBoxCellLocation(mb, mb->nPos, &i, &j);
		OSD_DrawMatrixBoxItem(mb, i, j, nSubCmdDraw);
	}
	else
	{
		for(i=0; i<mb->nRow; i++)
		{
			for(j=0; j<mb->nCol; j++)
				OSD_DrawMatrixBoxItem(mb, j, i, nSubCmdDraw);
		}
	}
}

PRESULT OSD_ShiftMatrixBoxSelHor(PMATRIX_BOX m, short sShiftX)
{
	UINT16 nCol, nRow;
	UINT16 nPos;
	UINT8 bMode = MATRIX_ROW_MODE(m->bStyle);
	BOOL fUnderFlow, fOverFlow;

	OSD_GetMtrxBoxCellLocation(m, m->nPos, &nCol, &nRow);
	while(1)
	{
		fUnderFlow = (sShiftX > 0) && (nCol == (m->nCol - 1));
		fOverFlow = (sShiftX < 0) && !nCol;
		nCol = (nCol + sShiftX + m->nCol) % m->nCol;
		nPos = nRow * m->nCol + nCol;
		if(MATRIX_ROW_STAY == bMode)
		{
			if(fUnderFlow || fOverFlow || nPos >= m->nCount)
				return PROC_LOOP;
		}
		else if(MATRIX_ROW_WRAP == bMode)
		{
			if(fUnderFlow)
			{
				nRow = (nRow + 1 + m->nRow) % m->nRow;
			}
			else if(fOverFlow)
			{
				nRow = (nRow - 1 + m->nRow) % m->nRow;
			}
			nPos = nRow * m->nCol + nCol;
		}
		if(nPos == m->nPos)
			return PROC_LOOP;
		if(CheckCell(m, nPos))
			break;
	}
	return NotifyCursorEvent(m, nPos);
}

PRESULT OSD_ShiftMatrixBoxSelVer(PMATRIX_BOX m, short sShiftY)
{
	UINT16 nCol, nRow;
	UINT16 nPos;
	UINT8 bMode = MATRIX_COL_MODE(m->bStyle);
	BOOL fUnderFlow, fOverFlow;

	OSD_GetMtrxBoxCellLocation(m, m->nPos, &nCol, &nRow);
	while(1)
	{
		fUnderFlow = (sShiftY > 0) && (nRow == (m->nRow - 1));
		fOverFlow = (sShiftY < 0) && !nRow;
		nRow = (nRow + sShiftY + m->nRow) % m->nRow;
		nPos = nRow * m->nCol + nCol;
		if(MATRIX_COL_STAY == bMode)
		{
			if(fUnderFlow || fOverFlow || nPos >= m->nCount)
				return PROC_LOOP;
		}
		if(nPos == m->nPos)
			return PROC_LOOP;
		if(CheckCell(m, nPos))
			break;
	}
	return NotifyCursorEvent(m, nPos);
}

void OSD_SetMtrxboxCount(PMATRIX_BOX m, UINT16 count, UINT16 row, UINT16 col)
{
	if(row * col >= count)
	{
		m->nCount = count;
		m->nRow = row;
		m->nCol = col;
	}
}

UINT16 OSD_GetMtrxboxCurPos(PMATRIX_BOX m)
{
	return m->nPos;
}

void OSD_SetMtrxboxCurPos(PMATRIX_BOX m, UINT16 nPos)
{
	if(nPos < m->nCount)
	{
		m->nPos = nPos;
	}
}

VACTION OSD_MatrixboxKeyMap(POBJECT_HEAD pObj, UINT32 vkey)
{
	VACTION		Vact = VACT_PASS;
	MATRIX_BOX	*mb = (MATRIX_BOX *)pObj;

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
		case V_KEY_ENTER:
			Vact = VACT_ENTER;
			break;
		default:
			Vact = VACT_PASS;
	}
	return Vact;
}

PRESULT OSD_MatrixboxProc(POBJECT_HEAD pObjHead, UINT32 msg_type, UINT32 msg,UINT32 param1)
{
	PRESULT Result = PROC_PASS;
	OBJECT_HEAD*	pObject;
	MATRIX_BOX	*m = (MATRIX_BOX*)pObjHead;
    
	if(msg_type == MSG_TYPE_KEY)
	{
		VACTION Action;
		UINT32 vkey;
		BOOL bContinue;
		Result = OSD_ObjCommonProc(pObjHead, msg, param1, &vkey, &Action, &bContinue);
		if(Result != PROC_PASS)
			return Result;

		switch (Action)
		{
			case VACT_CURSOR_UP:
				Result = OSD_ShiftMatrixBoxSelVer(m, -1);//MKSZ tail here
				break;
			case VACT_CURSOR_DOWN:
				Result= OSD_ShiftMatrixBoxSelVer(m, 1);//MKSZ tail here
				break;
			case VACT_CURSOR_LEFT:
				Result = OSD_ShiftMatrixBoxSelHor(m, -1);//MKSZ tail here
				break;
			case VACT_CURSOR_RIGHT:
				Result= OSD_ShiftMatrixBoxSelHor(m, 1);//MKSZ tail here
				break;
CLOSE_OBJECT:
		    	case VACT_CLOSE:
		    		Result = OSD_ObjClose(pObjHead, C_CLOSE_CLRBACK_FLG);
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
		CHECK_LEAVE_RETURN(Result,pObjHead);

EXIT:	
	return Result;
}

