#include <sys_config.h>
#include <api/libge/osd_lib.h>
#include <api/libge/osd_common_draw.h>
#include <api/libge/osd_primitive.h>
#include <api/libge/osd_window.h>
#include "osd_lib_internal.h"

#define GELIB_M3602_DUALOUT

GUI_RSC_FNC g_gui_rscfunc;
#ifdef GELIB_M3602_DUALOUT
static TEXT_FIELD slvtxt;
static BITMAP slvbmp;
static CONTAINER slvcon;
static MULTI_TEXT slvmtxt;
static OBJLIST slvolst;
static MULTISEL slvmsel;
static EDIT_FIELD slvedf;
static PROGRESS_BAR slvpbar;
static SCROLL_BAR slvsbar;
static LIST slvlst;
static MATRIX_BOX slvmbox;

static void *pslvObj[] = 
{
	(void *)&slvtxt,
	(void *)&slvedf,
	(void *)&slvbmp,
	(void *)&slvmsel,
	(void *)&slvpbar,
	(void *)&slvsbar,
	(void *)&slvmtxt,
	(void *)&slvlst,
	(void *)&slvmbox,
	(void *)&slvolst,
	(void *)&slvcon,
};

static UINT8 slvojb_size[] = 
{
	sizeof(slvtxt),
	sizeof(slvedf),
	sizeof(slvbmp),
	sizeof(slvmsel),
	sizeof(slvpbar),
	sizeof(slvsbar),
	sizeof(slvmtxt),
	sizeof(slvlst),
	sizeof(slvmbox),
	sizeof(slvolst),
	sizeof(slvcon),
};

/*static UINT32 obj_slvtail[]=
{
	(slvtxt.slave),
	(slvedf.slave),
	(slvbmp.slave),
	(slvmsel.slave),
	(slvpbar.slave),
	(slvsbar.slave),
	(slvmtxt.slave),
	(slvlst.slave),
	(slvmbox.slave),
	(slvolst.slave),
	(slvcon.slave),
};*/
#define SLV_OBJS_LEN	(OT_MAX)
#endif
OSD_NotifyFunc g_ObjectOpenNotify, g_ObjectCloseNotify;


#define C_MAX_OBJECT_RECORD     20
//#define DEO_VIDEO_QUALITY_IMPROVE
static UINT32 m_object_count;
static POBJECT_HEAD m_object_record[C_MAX_OBJECT_RECORD];

static BOOL bClearbackFlag = TRUE;
static BOOL bClearbackFlag_bakeup = TRUE;

void add_object_record(POBJECT_HEAD obj)
{
#ifdef DEO_VIDEO_QUALITY_IMPROVE
    int i, pos;

	if(obj->pRoot != NULL)
        return;
    for(i=0, pos=-1; i<C_MAX_OBJECT_RECORD; i++)
    {
        if(m_object_record[i] == NULL && pos < 0)
            pos = i;
        if(m_object_record[i] == obj)
            break;
    }
    if(i >= C_MAX_OBJECT_RECORD)
    {
        m_object_record[pos] = obj;
        m_object_count++;
    }

    // Call DE to disable "COPY FRAME", and enable VCAP
	ge_gma_show_onoff((struct ge_device *)dev_get_by_id(HLD_DEV_TYPE_GE, 0),0,1);
     
#endif
}

static void del_object_record(POBJECT_HEAD obj)
{
#ifdef DEO_VIDEO_QUALITY_IMPROVE
    int i;

	if(obj->pRoot != NULL)
        return;

    for(i=0; i<C_MAX_OBJECT_RECORD; i++)
    {
        if(m_object_record[i] == obj)
        {
            m_object_record[i] = NULL;
            m_object_count--;
        }
        if(m_object_record[i] != NULL)
        {
            if(OSD_RectInRect(&obj->frame, &(m_object_record[i]->frame)))
            {
                m_object_record[i] = NULL;
                m_object_count--;
            }
        }
    }
    
    
    if(!m_object_count)
    {
        ge_gma_show_onoff((struct ge_device *)dev_get_by_id(HLD_DEV_TYPE_GE, 0),0,0);
        // Call DE to enable "COPY FRAME", and disable VCAP
    }
#endif
}

void reset_object_record()
{
#ifdef DEO_VIDEO_QUALITY_IMPROVE
    m_object_count = 0;
    MEMSET(m_object_record, 0, sizeof(m_object_record));
    // Call DE to enable "COPY FRAME", and disable VCAP
#endif
}

void OSD_RegOpenNotifyFunc(OSD_NotifyFunc func)
{
	g_ObjectOpenNotify = func;
}

void OSD_RegCloseNotifyFunc(OSD_NotifyFunc func)
{
	g_ObjectCloseNotify = func;
}
BOOL OSD_RectInRect(const PGUI_RECT R,const PGUI_RECT r)
{
	if(r->uStartX >= R->uStartX && r->uStartY>= R->uStartY
		&& (r->uStartX + r->uWidth)<=(R->uStartX + R->uWidth)
		&& (r->uStartY + r->uHeight)<=(R->uStartY + R->uHeight))
		return TRUE;
	else
		return FALSE;
}

void OSD_GetRectsCross(const PGUI_RECT backR,const PGUI_RECT forgR,PGUI_RECT relativR)
{
	short ltx,lty,rbx,rby;
	short dw,dh;

	ltx = forgR->uStartX >=  backR->uStartX ? forgR->uStartX :  backR->uStartX;
	lty = forgR->uStartY >=  backR->uStartY ? forgR->uStartY :  backR->uStartY;
	rbx = (forgR->uStartX + forgR->uWidth) <= (backR->uStartX + backR->uWidth) ? (forgR->uStartX + forgR->uWidth) : (backR->uStartX + backR->uWidth);
	rby = (forgR->uStartY + forgR->uHeight) <= (backR->uStartY + backR->uHeight) ? (forgR->uStartY + forgR->uHeight) : (backR->uStartY + backR->uHeight);

	dw = rbx > ltx ? (rbx - ltx) : 0;
	dh = rby > lty ? (rby - lty) : 0;
	relativR->uWidth  = dw;
	relativR->uHeight = dh;
	relativR->uStartX = ltx;
  	relativR->uStartY  = lty;	
}

void OSD_SetWholeObjAttr(POBJECT_HEAD	pObj, UINT8 bAttr)
{
	POBJECT_HEAD	pNext;

	pObj->bAttr = bAttr;	
	if(OT_CONTAINER == pObj->bType)
	{
		pNext = ((CONTAINER*)pObj)->pNextInCntn;
		while(NULL != pNext)
		{
			pNext->bAttr = bAttr;
			if(OT_CONTAINER == pNext->bType)
				OSD_SetWholeObjAttr(pNext, bAttr);
				
			pNext = pNext->pNext;
		}
	}
}

POBJECT_HEAD OSD_GetTopRootObject(POBJECT_HEAD pObj)
{
	while(pObj)
		pObj = pObj->pRoot;
	return pObj;
}

UINT8 OSD_GetFocusID(POBJECT_HEAD pObj)
{
	UINT8 id = 0;
	POBJECT_HEAD pFocusObj;
	OBJLIST *ol;
	CONTAINER *con;		
	
	switch(pObj->bType)
	{
	case OT_OBJLIST:
		ol = (OBJLIST*)pObj;
		if(ol->bListStyle & LIST_ITEMS_COMPLETE)
			pFocusObj = ol->pListField[ol->wCurPoint];
		else
			pFocusObj = ol->pListField[ol->wCurPoint - ol->wTop];
		id = pFocusObj->bID;		
		break;
	case OT_CONTAINER:
		con = (CONTAINER*)pObj;
		id = con->FocusObjectID;
		break;
	case OT_TEXTFIELD:	
	case OT_BITMAP:
	case OT_EDITFIELD:
	case OT_MULTISEL:
	case OT_PROGRESSBAR:
	case OT_SCROLLBAR:
	case OT_MULTITEXT:	 	
	case OT_LIST:
	case OT_MATRIXBOX:	 		 	
	default:
		;		 			
	}	
		
	return id;	
}

POBJECT_HEAD OSD_GetObject(POBJECT_HEAD pObj, UINT8 bID)
{
	if(!bID)
		return NULL;     

	UINT32 i,n;
	POBJECT_HEAD oh = NULL;
	OBJLIST *ol;
	CONTAINER *con;
	UINT16 cnt;

	switch(pObj->bType)
	{
	case OT_OBJLIST:		
		ol = (OBJLIST*)pObj;
		cnt = OSD_GetObjListCount(ol);
		for(i=0;i<cnt;i++)
		{
			oh = ol->pListField[i];
			if(oh!=NULL && oh->bID == bID)
				return oh;
		}
		break;
	case OT_CONTAINER:
		con = (CONTAINER*)pObj;
		oh = (POBJECT_HEAD )(con->pNextInCntn);
		while(oh!=NULL)
		{
			if(oh->bID == bID)
				return oh;
			oh = oh->pNext;
		}
		break;
	case OT_TEXTFIELD:	
	case OT_BITMAP:
	case OT_EDITFIELD:
	case OT_MULTISEL:
	case OT_PROGRESSBAR:
	case OT_SCROLLBAR:
	case OT_MULTITEXT:	 	
	case OT_LIST:
	case OT_MATRIXBOX:	 		 	
	default:
		;		 			
	}	
			
	return oh;
}

POBJECT_HEAD OSD_GetFocusObject(POBJECT_HEAD pObj)
{
	UINT8 bID;
	
	bID = OSD_GetFocusID(pObj);
	return OSD_GetObject(pObj,bID);
}

POBJECT_HEAD OSD_GetAdjacentObject(POBJECT_HEAD pObj, VACTION action)
{
	POBJECT_HEAD pBox;
	POBJECT_HEAD pNewObj;
	UINT8		 bNewId, bNextNewId,bCount=0;	// for safety control
	
	if(pObj == NULL)
		return NULL;
		
	pBox = pObj->pRoot;
	if(pBox == NULL)
		return NULL;
					
	bNextNewId = bNewId = OSD_GetAdjacentObjID(pObj,action);	
	while ((pObj->bID != bNewId) && ((bCount++)<255))
	{
		pNewObj = OSD_GetObject(pBox,bNewId);
		if(pNewObj == NULL)
			return NULL;
				
		if((pNewObj->bAttr & C_ATTR_LMASK) == C_ATTR_ACTIVE)
			return pNewObj;//SZMK
		else
		{ 
			bNextNewId = OSD_GetAdjacentObjID(pNewObj,action);
			if(bNewId != bNextNewId)
				bNewId = bNextNewId;
			else
				break;						
		}
	}
	
	return pObj;
}

PRESULT OSD_ChangeFocus(POBJECT_HEAD pObj, UINT16 bNewFocusID,UINT32 parm)
{
	OBJLIST *ol;
	CONTAINER *con;		
	PRESULT Result = PROC_LOOP;
	
	switch(pObj->bType)
	{
	case OT_OBJLIST:
		ol = (OBJLIST*)pObj;
		Result = OSD_ObjListChgFocus(ol,bNewFocusID,parm);
		break;
	case OT_CONTAINER:
		con = (CONTAINER*)pObj;
		Result = OSD_ContainerChgFocus(con, (UINT8)bNewFocusID,parm);
		break;
	case OT_TEXTFIELD:	
	case OT_BITMAP:
	case OT_EDITFIELD:
	case OT_MULTISEL:
	case OT_PROGRESSBAR:
	case OT_SCROLLBAR:
	case OT_MULTITEXT:	 	
	case OT_LIST:
	case OT_MATRIXBOX:	 		 	
	default:
		;
	}

	return Result;	
}

void OSD_MoveObject(POBJECT_HEAD pObj, short x, short y, BOOL IsRelative)
{
	OBJLIST *ol;
	CONTAINER* con;
	POBJECT_HEAD oh;
	short ShiftX, ShiftY,rx,ry;
	UINT32 i,n;

	if(IsRelative)
	{
		ShiftX = x;
		ShiftY = y;
		rx = x;
		ry = y;
	}
	else
	{
		ShiftX = x - pObj->frame.uStartX;
		ShiftY = y - pObj->frame.uStartY;
		rx = ShiftX;
		ry = ShiftY;
	}

	switch(pObj->bType)
	{
	case OT_OBJLIST:
		ol = (OBJLIST*)pObj;
		if(ol->bListStyle & LIST_ITEMS_COMPLETE)
			n = ol->wCount;
		else
			n = ol->wDep;
		
		for(i=0;i<n;i++)
			OSD_MoveObject(ol->pListField[i],rx,ry,TRUE);  
		
		if(ol->pSelMarkObj != NULL)
			OSD_MoveObject(ol->pSelMarkObj,rx,ry,TRUE); 
		if(ol->scrollBar != NULL)
			OSD_MoveObject((POBJECT_HEAD)ol->scrollBar,rx,ry,TRUE);
		break;
	case OT_CONTAINER:
		con = (CONTAINER*)pObj;
		oh = con->pNextInCntn;
		while(oh)
		{
			OSD_MoveObject(oh,rx,ry,TRUE);
			oh = oh->pNext;
		}
		break;
	case OT_TEXTFIELD:	
	case OT_BITMAP:
	case OT_EDITFIELD:
	case OT_MULTISEL:
	case OT_PROGRESSBAR:
	case OT_SCROLLBAR:
	case OT_MULTITEXT:	 	
	case OT_LIST:
	case OT_MATRIXBOX:
		break;            
	}

    pObj->frame.uStartX += ShiftX;
    pObj->frame.uStartY +=  ShiftY;    
}

UINT8 OSD_GetAdjacentObjID(POBJECT_HEAD pObj, VACTION action)
{
	if(action == VACT_CURSOR_UP)
		return pObj->bUpID;
	else if(action == VACT_CURSOR_DOWN)
		return pObj->bDownID;
	else if(action == VACT_CURSOR_LEFT)
		return pObj->bLeftID;
	else if(action == VACT_CURSOR_RIGHT)
		return pObj->bRightID;
	else
		return pObj->bID;
}

#ifdef GELIB_M3602_DUALOUT
static POBJECT_HEAD OSD_GetSlaveObject(POBJECT_HEAD pmst)
{
	void *pslvadr;
	PSDBMP pslvbmp;
	PSDCON pslvcon;
	PSDMTXBOX pslvmbox;
	PSDTXT pslvtxt;
	PSD_EDF pslvedf;
	PSD_LST pslvlst;
	PSD_MULSEL pslvmsel;
	PSD_MULTXT pslvmtxt;
	PSD_OBJLST pslvol;
	PSD_PBAR pslvpbar;
	PSD_SBAR pslvsbar;
	POBJECT_HEAD pslv;

	pslv = NULL;
	pslvbmp = NULL;
	pslvcon = NULL;
	pslvmbox = NULL;
	pslvtxt = NULL;
	pslvedf = NULL;
	pslvlst = NULL;
	pslvmsel = NULL;
	pslvmtxt = NULL;
	pslvol = NULL;
	pslvpbar = NULL;
	pslvsbar = NULL;
	if((pmst != NULL)&&(gui_rctvalid(&pmst->frame))&&(pmst->bType < OT_MAX))
	{
		pslvadr = pslvObj[pmst->bType];
		MEMCPY(pslvadr,pmst,slvojb_size[pmst->bType]);
		switch(pmst->bType)
		{
			case OT_TEXTFIELD:
				if(slvtxt.slave)
				{
					pslvtxt = (PSDTXT)slvtxt.slave;
					
					slvtxt.slave = 0;
					slvtxt.bX = pslvtxt->bX;
					slvtxt.bY = pslvtxt->bY;
                    if((pslvtxt->pString) != NULL)
                        slvtxt.pString = pslvtxt->pString;
					MEMCPY(&(slvtxt.head.frame),&(pslvtxt->frame),sizeof(GUI_RECT));
					MEMCPY(&(slvtxt.head.style),&(pslvtxt->style),sizeof(COLOR_STYLE));
				}
				else
				{
					return pslv;
				}
				break;
			case OT_BITMAP:
				if(slvbmp.slave)
				{
					pslvbmp = (PSDBMP)slvbmp.slave;

					slvbmp.slave = 0;
					slvbmp.bX = pslvbmp->bX;
					slvbmp.bY = pslvbmp->bY;
					slvbmp.wIconID = pslvbmp->wIconID;
					MEMCPY(&(slvbmp.head.frame),&(pslvbmp->frame),sizeof(GUI_RECT));
					MEMCPY(&(slvbmp.head.style),&(pslvbmp->style),sizeof(COLOR_STYLE));
				}
				else
				{
					return pslv;
				}
				break;
			case OT_CONTAINER:
				if(slvcon.slave)
				{//SZTODO
					pslvcon = (PSDCON)slvcon.slave;

					slvcon.slave = 0;
					slvcon.pNextInCntn = NULL;
					MEMCPY(&(slvcon.head.frame),&(pslvcon->frame),sizeof(GUI_RECT));
					MEMCPY(&(slvcon.head.style),&(pslvcon->style),sizeof(COLOR_STYLE));
				}
				else
				{
					return pslv;
				}
				break;
			case OT_OBJLIST:
				if(slvolst.slave)
				{
					pslvol = (PSD_OBJLST)slvolst.slave;

					slvolst.slave = 0;
					MEMCPY(&(slvolst.head.frame),&(pslvol->frame),sizeof(GUI_RECT));
					MEMCPY(&(slvolst.head.style),&(pslvol->style),sizeof(COLOR_STYLE));
				}
				else
				{
					return pslv;
				}
				break;
			case OT_EDITFIELD:
				if(slvedf.slave)
				{
					pslvedf = (PSD_EDF)slvedf.slave;

					slvedf.slave = 0;
					slvedf.bX = pslvedf->bX;
					slvedf.bY = pslvedf->bY;
					MEMCPY(&(slvedf.head.frame),&(pslvedf->frame),sizeof(GUI_RECT));
					MEMCPY(&(slvedf.head.style),&(pslvedf->style),sizeof(COLOR_STYLE));
				}
				else
				{
					return pslv;
				}
				break;
			case OT_MULTISEL:
				if(slvmsel.slave)
				{
					pslvmsel = (PSD_MULSEL)slvmsel.slave;

					slvmsel.slave = 0;
					slvmsel.bX = pslvmsel->bX;
					slvmsel.bY = pslvmsel->bY;
					MEMCPY(&(slvmsel.head.frame),&(pslvmsel->frame),sizeof(GUI_RECT));
					MEMCPY(&(slvmsel.head.style),&(pslvmsel->style),sizeof(COLOR_STYLE));
				}
				else
				{
					return pslv;
				}
				break;
			case OT_PROGRESSBAR:
				if(slvpbar.slave)
				{
					pslvpbar = (PSD_PBAR)slvpbar.slave;

					slvpbar.slave = 0;
					slvpbar.bX = pslvpbar->bX;
					slvpbar.bY = pslvpbar->bY;
					slvpbar.wTickBg = pslvpbar->wTickBg;
					slvpbar.wTickFg = pslvpbar->wTickFg;
					MEMCPY(&(slvpbar.rcBar),&(pslvpbar->rcBar),sizeof(GUI_RECT));
					MEMCPY(&(slvpbar.head.frame),&(pslvpbar->frame),sizeof(GUI_RECT));
					MEMCPY(&(slvpbar.head.style),&(pslvpbar->style),sizeof(COLOR_STYLE));
				}
				else
				{
					return pslv;
				}
				break;
			case OT_SCROLLBAR:
				if(slvsbar.slave)
				{
					pslvsbar = (PSD_SBAR)slvsbar.slave;

					slvsbar.slave = 0;
					slvsbar.wTickBg = pslvsbar->wTickBg;
					slvsbar.wThumbID = pslvsbar->wThumbID;
					MEMCPY(&(slvsbar.rcBar),&(pslvsbar->rcBar),sizeof(GUI_RECT));
					MEMCPY(&(slvsbar.head.frame),&(pslvsbar->frame),sizeof(GUI_RECT));
					MEMCPY(&(slvsbar.head.style),&(pslvsbar->style),sizeof(COLOR_STYLE));
				}
				else
				{
					return pslv;
				}
				break;
			case OT_MULTITEXT:
				if(slvmtxt.slave)
				{
					pslvmtxt = (PSD_MULTXT)slvmtxt.slave;

					slvmtxt.slave = 0;
					MEMCPY(&(slvmtxt.rcText),&(pslvmtxt->rcText),sizeof(GUI_RECT));
					MEMCPY(&(slvmtxt.head.frame),&(pslvmtxt->frame),sizeof(GUI_RECT));
					MEMCPY(&(slvmtxt.head.style),&(pslvmtxt->style),sizeof(COLOR_STYLE));
				}
				else
				{
					return pslv;
				}
				break;
			case OT_MATRIXBOX:
				if(slvmbox.slave)
				{
					pslvmbox = (PSDMTXBOX)slvmbox.slave;

					slvmbox.slave = 0;
					slvmbox.bIntervalX = pslvmbox->bIntervalX;
					slvmbox.bIntervalY = pslvmbox->bIntervalY;
					MEMCPY(&(slvmbox.head.frame),&(pslvmbox->frame),sizeof(GUI_RECT));
					MEMCPY(&(slvmbox.head.style),&(pslvmbox->style),sizeof(COLOR_STYLE));
				}
				else
				{
					return pslv;
				}
				break;
			case OT_LIST:
				if(slvlst.slave)
				{
					pslvlst = (PSD_LST)slvlst.slave;

					slvlst.slave = 0;
					slvlst.bIntervalY = pslvlst->bIntervalY;
					MEMCPY(&(slvlst.head.frame),&(pslvlst->frame),sizeof(GUI_RECT));
					MEMCPY(&(slvlst.head.style),&(pslvlst->style),sizeof(COLOR_STYLE));
				}
				else
				{
					return pslv;
				}
				break;
			default:
				return pslv;
		}

		pslv = (POBJECT_HEAD)pslvadr;
		
		pslv->pNext = NULL;
		pslv->pRoot = NULL;

        if(pmst->bType != OT_MULTISEL)
            pslv->pfnCallback = NULL;
		pslv->pfnKeyMap = NULL;
		pslv->bAttr = (pslv->bAttr) | C_ATTR_SLAVE;
	}

#if 1
	if(NULL != pslv)
	{
		if((gui_rctvalid(&(pslv->frame)))==0)
		{
			return NULL;
			ASSERT(0);
		}
		else if(((pslv->frame.uStartX)+(pslv->frame.uWidth))>720)
		{
			return NULL;
			ASSERT(0);
		}
		else if(((pslv->frame.uStartY)+(pslv->frame.uHeight))>576)
		{
			return NULL;
			ASSERT(0);
		}
	}
#endif
	
	return pslv;
}

static UINT32 OSD_DrawSlvObject(POBJECT_HEAD pObj, UINT32 nCmdDraw,UINT32 param )
{
	UINT32 drawType;
	UINT8  bStyleIdx;

	drawType = param;
	switch(param)
	{
	case C_DRAW_TYPE_NORMAL:
		if(((pObj->bAttr)&C_ATTR_LMASK) == C_ATTR_ACTIVE)
		{
			bStyleIdx = pObj->style.bShowIdx;
		}
		else if(((pObj->bAttr)&C_ATTR_LMASK) == C_ATTR_INACTIVE)
		{
			drawType = C_DRAW_TYPE_GRAY;
			bStyleIdx = pObj->style.bGrayIdx;
		}    
		else
		{
			drawType = C_DRAW_TYPE_HIDE;
			bStyleIdx = 0;
		}
		break;
	case C_DRAW_TYPE_HIGHLIGHT:
		if(((pObj->bAttr)&C_ATTR_LMASK) == C_ATTR_ACTIVE)
			bStyleIdx = pObj->style.bHLIdx;
		else
			return 1;
		break;
	case C_DRAW_TYPE_SELECT:
		if(((pObj->bAttr)&C_ATTR_LMASK) == C_ATTR_ACTIVE)
			bStyleIdx = pObj->style.bSelIdx;
		else
			return 1;
		break;
	default:
		bStyleIdx = 0;
		return 1;
	}

	OSD_SetDrawType(nCmdDraw,drawType);
	if(drawType == C_DRAW_TYPE_HIDE)
		OSD_HideObject(pObj,nCmdDraw);
	else
		OSD_DrawObjectCell(pObj,bStyleIdx,nCmdDraw);

	return 0;
}
#endif

PGUI_VSCR OSD_DrawSlaveFrame(PGUI_RECT prct,UINT32 bStyleIdx)
{	
	UINT32 bFlag;
	PGUI_VSCR pvscr;
	PWINSTYLE  lpWinSty;

	bFlag = VSCR_NULL;
	if(bStyleIdx != (UINT32)(~0))
	{
		lpWinSty = g_gui_rscfunc.osd_get_win_style(bStyleIdx);
		switch((lpWinSty->bWinStyleType)&C_WS_TYPE_MASK)
		{
			case (C_WS_LINE_CIRCL_DRAW&C_WS_TYPE_MASK):
			case C_WS_PIC_DRAW:
				bFlag = VSCR_GET_CORNER;
				break;
			default:
				bFlag = VSCR_GET_BACK;
		}
	}

	pvscr = OSD_GetSlvscr(prct, bFlag);
	
	if(bStyleIdx != (UINT32)(~0))
		OSD_DrawStyleRect(prct,bStyleIdx,pvscr);

	return pvscr;
}

PGUI_VSCR OSD_DrawObjectFrame(PGUI_RECT pRect,UINT32 bStyleIdx)
{	
	UINT32 bFlag;
	PGUI_VSCR pGVscr;
	PWINSTYLE  lpWinSty;

	bFlag = VSCR_NULL;
	if(bStyleIdx != (UINT32)(~0))
	{
		lpWinSty = g_gui_rscfunc.osd_get_win_style(bStyleIdx);
		switch((lpWinSty->bWinStyleType)&C_WS_TYPE_MASK)
		{
			case C_WS_LINE_CIRCL_DRAW:
			case C_WS_PIC_DRAW:
				bFlag = VSCR_GET_CORNER;
				break;
			default:
				bFlag = VSCR_GET_BACK;
		}
	}

	pGVscr = OSD_GetVscr(pRect, bFlag);
	if(bStyleIdx != (UINT32)(~0))
		OSD_DrawStyleRect(pRect,bStyleIdx,pGVscr);
	
	return pGVscr;
}

UINT8 OSD_GetSlaveWstyle(PCOLOR_STYLE pstyle,POBJECT_HEAD pobj,UINT8 colorstyle)
{
	UINT8 i,slvstyle;
	PCOLOR_STYLE pmstyle;

	pmstyle  = &(pobj->style);
	if(colorstyle == pmstyle->bShowIdx)
		slvstyle = pstyle->bShowIdx;
	else if(colorstyle == pmstyle->bSelIdx)
		slvstyle = pstyle->bSelIdx;
	else if(colorstyle == pmstyle->bHLIdx)
		slvstyle = pstyle->bHLIdx;
	else
		slvstyle = pstyle->bGrayIdx;

	return slvstyle;
}

/* Object drawing functions*/
void OSD_DrawObjectCell(POBJECT_HEAD pObj, UINT8 bStyleIdx,	UINT32 nCmdDraw)
{
	PGUI_VSCR pvscr;
	
	switch(pObj->bType)
	{
	case OT_TEXTFIELD:
		OSD_DrawTextFieldCell((PTEXT_FIELD)pObj,bStyleIdx,nCmdDraw);
		break;
	case OT_BITMAP:
		OSD_DrawBitmapCell((PBITMAP)pObj,bStyleIdx,nCmdDraw);
		break;
	case OT_EDITFIELD:
		OSD_DrawEditFieldCell((PEDIT_FIELD)pObj,bStyleIdx,nCmdDraw);
		break;
	case OT_MULTISEL:
		OSD_DrawMultiselCell((PMULTISEL)pObj,bStyleIdx,nCmdDraw);
		break;
	case OT_PROGRESSBAR:
		OSD_DrawProgressBarCell((PPROGRESS_BAR)pObj,bStyleIdx,nCmdDraw);
		break;
	case OT_SCROLLBAR:
		OSD_DrawScrollBarCell((PSCROLL_BAR)pObj,bStyleIdx,nCmdDraw);
		break;
	case OT_MULTITEXT:
		OSD_DrawMultiTextCell((PMULTI_TEXT)pObj, bStyleIdx,nCmdDraw);
		break;
	case OT_LIST:
		OSD_DrawListCell((PLIST)pObj, bStyleIdx,nCmdDraw);
		break;
	case OT_MATRIXBOX:
		OSD_DrawMatrixBoxCell((PMATRIX_BOX)pObj, bStyleIdx,nCmdDraw);
		break;
	case OT_OBJLIST:
		OSD_DrawObjectListCell((POBJLIST)pObj,bStyleIdx,nCmdDraw);
		break;
	case OT_CONTAINER:
		OSD_DrawContainerCell((PCONTAINER)pObj,bStyleIdx,nCmdDraw);
		break;
	default:
		break;
	}

	if(OSD_GetAttrSlave(pObj))
		pvscr = OSD_GetTaskSlvscr(osal_task_get_current_id());
	else
		pvscr = OSD_GetTaskVscr(osal_task_get_current_id());
	
	OSD_SetVscrModified(pvscr);	
	if( (!(nCmdDraw & C_DRAW_SIGN_EVN_FLG)) && (pObj->pRoot == NULL) &&(!OSD_GetAttrSlave(pObj)))
		OSD_UpdateVscr(pvscr);
}

PRESULT OSD_DrawObject(POBJECT_HEAD pObj, UINT32 nCmdDraw )
{
	PRESULT Result;
	UINT32 drawType;
	UINT8  bStyleIdx;
	UINT32 msg;

	if(((pObj->bAttr)&C_ATTR_LMASK) == C_ATTR_ACTIVE)
	{
		drawType = C_DRAW_TYPE_NORMAL;
		bStyleIdx = pObj->style.bShowIdx;
	}
	else if(((pObj->bAttr)&C_ATTR_LMASK) == C_ATTR_INACTIVE)
	{
		drawType = C_DRAW_TYPE_GRAY;
		bStyleIdx = pObj->style.bGrayIdx;
	}    
	else
	{
		drawType = C_DRAW_TYPE_HIDE;
		bStyleIdx = 0;
	}

	OSD_SetDrawType(nCmdDraw,drawType);
	if(nCmdDraw & C_DRAW_SIGN_EVN_FLG)    
	{   	
		Result = OSD_ObjProc(pObj,(MSG_TYPE_EVNT<<16)|EVN_PRE_DRAW,drawType,nCmdDraw);
		if(Result != PROC_PASS)
			return Result;            
	}

	
    add_object_record(pObj);

	if(drawType == C_DRAW_TYPE_HIDE)
		OSD_HideObject(pObj,nCmdDraw);
	else
	{
		if((gelib_getdual_support())&&(!OSD_GetAttrSlave(pObj)))
		{//recursive
			UINT32 slvcmd;
			POBJECT_HEAD pslv;

			slvcmd = nCmdDraw;
			slvcmd &= (~C_DRAW_SIGN_EVN_FLG);
			slvcmd &= (~C_DRAW_SIGN_EVN_FLG);
			pslv = OSD_GetSlaveObject(pObj);
			if(NULL != pslv)
				OSD_DrawSlvObject(pslv,slvcmd,C_DRAW_TYPE_NORMAL);
		}

		OSD_DrawObjectCell(pObj,bStyleIdx,nCmdDraw);
	}

	if(nCmdDraw & C_DRAW_SIGN_EVN_FLG)
	{
		OSD_ObjProc(pObj,(MSG_TYPE_EVNT<<16)|EVN_POST_DRAW,drawType,nCmdDraw);
	}

	if((gelib_objwnd_valid(pObj))&&(!OSD_GetAttrSlave(pObj)) && (gelib_get_wndmonitor()))
	{
		osdwnd_showwnd(NULL,(UINT32)pObj);
	}

	return PROC_PASS;  
}
 
PRESULT OSD_TrackObject(POBJECT_HEAD pObj, UINT32 nCmdDraw )
{
	PRESULT Result;
	UINT32 drawType;
	UINT8  bStyleIdx;

	if(((pObj->bAttr)&C_ATTR_LMASK) == C_ATTR_ACTIVE)
	{
		drawType = C_DRAW_TYPE_HIGHLIGHT;
		bStyleIdx = pObj->style.bHLIdx;
	}
	else
	{
		return PROC_PASS;
	}

	OSD_SetDrawType(nCmdDraw,drawType);		
	if(nCmdDraw & C_DRAW_SIGN_EVN_FLG)    
	{
		Result = OSD_ObjProc(pObj,(MSG_TYPE_EVNT<<16) |EVN_PRE_DRAW, drawType, nCmdDraw);
		if(Result != PROC_PASS)
			return Result;
	}
	
	if((gelib_getdual_support())&&(!OSD_GetAttrSlave(pObj)))
	{
		UINT32 slvcmd;
		POBJECT_HEAD pslv;

		slvcmd = nCmdDraw;
		slvcmd &= (~C_DRAW_SIGN_EVN_FLG);
		slvcmd &= (~C_DRAW_SIGN_EVN_FLG);
		pslv = OSD_GetSlaveObject(pObj);
		if(NULL != pslv)
			OSD_DrawSlvObject(pslv,slvcmd,C_DRAW_TYPE_HIGHLIGHT);
	}
	
    add_object_record(pObj);

	OSD_DrawObjectCell(pObj,bStyleIdx,nCmdDraw);
	
	if(nCmdDraw & C_DRAW_SIGN_EVN_FLG)
	{
		OSD_ObjProc(pObj,(MSG_TYPE_EVNT<<16) |EVN_POST_DRAW,drawType,nCmdDraw);
	}

	if(gelib_objwnd_valid(pObj))
	{
		osdwnd_showwnd(NULL,(UINT32)pObj);
	}
	return PROC_PASS;
}

PRESULT OSD_SelObject(POBJECT_HEAD pObj, UINT32 nCmdDraw )
{
	PRESULT Result;
	UINT32 drawType;
	UINT8  bStyleIdx;
	
	if(((pObj->bAttr)&C_ATTR_LMASK) == C_ATTR_ACTIVE)
	{
		drawType = C_DRAW_TYPE_SELECT;
		bStyleIdx = pObj->style.bSelIdx;
	}
	else
	{
		return PROC_PASS;
	}
		
	OSD_SetDrawType(nCmdDraw,drawType);
	if(nCmdDraw & C_DRAW_SIGN_EVN_FLG)    
	{
		Result = OSD_ObjProc(pObj,(MSG_TYPE_EVNT<<16) | EVN_PRE_DRAW,drawType, nCmdDraw);
		if(Result != PROC_PASS)
			return Result;
	}
	
	if((gelib_getdual_support())&&(!OSD_GetAttrSlave(pObj)))
	{//recursive
		UINT32 slvcmd;
		POBJECT_HEAD pslv;

		slvcmd = nCmdDraw;
		slvcmd &= (~C_DRAW_SIGN_EVN_FLG);
		slvcmd &= (~C_DRAW_SIGN_EVN_FLG);
		pslv = OSD_GetSlaveObject(pObj);
		if(NULL != pslv)
			OSD_DrawSlvObject(pslv,slvcmd,C_DRAW_TYPE_SELECT);
	}

	
    add_object_record(pObj);
	OSD_DrawObjectCell(pObj,bStyleIdx,nCmdDraw);	
	
	if(nCmdDraw & C_DRAW_SIGN_EVN_FLG)
	{
		OSD_ObjProc(pObj,(MSG_TYPE_EVNT<<16) | EVN_POST_DRAW,drawType, nCmdDraw);
	}

	return PROC_PASS;
}

void OSD_HideObject(POBJECT_HEAD pObj, UINT32 nCmdDraw )
{
	PRESULT Result;
	UINT32 drawType;
	UINT32 dwColorIdx;
	OBJECT_HEAD* pBox;
	PGUI_VSCR	pvscr,pslvscr;
	
	drawType= C_DRAW_TYPE_HIDE;
	if(nCmdDraw & C_DRAW_SIGN_EVN_FLG)    
	{
		Result = OSD_ObjProc(pObj,(MSG_TYPE_EVNT<<16) |EVN_PRE_DRAW,drawType, nCmdDraw);
		if(Result != PROC_PASS)
			return ;
	}

	pvscr = NULL;
	pBox = (OBJECT_HEAD*)(pObj->pRoot);
	if(pBox == NULL)
	{
		OSD_ClearObject(pObj,nCmdDraw);
	}
	else
	{
		pvscr = OSD_GetVscr(&pObj->frame,VSCR_NULL);		
		dwColorIdx = g_gui_rscfunc.osd_get_win_style(pBox->style.bShowIdx)->wBgIdx;
		if(wstyle_colorshow(dwColorIdx))
		{
			OSD_DrawRect2Vscr(&pObj->frame,dwColorIdx,pvscr);
		}

		if(gelib_getdual_support())//slave recurse
		{
			POBJECT_HEAD pslv;

			pslv = OSD_GetSlaveObject(pBox);
			if(pslv != NULL)
			{
				dwColorIdx = g_gui_rscfunc.osd_get_win_style(pslv->style.bShowIdx)->wBgIdx;
				
				pslv = OSD_GetSlaveObject(pObj);
				if((NULL != pslv) && (wstyle_colorshow(dwColorIdx)))
				{
					pslvscr = OSD_GetSlvscr(&pslv->frame,VSCR_NULL);
					OSD_SetVscrModified(pslvscr);
					OSD_DrawRect2Vscr(&pslv->frame,dwColorIdx,pslvscr);
				}
			}
		}
	}

	if(nCmdDraw & C_DRAW_SIGN_EVN_FLG)
	{
		OSD_ObjProc(pObj,(MSG_TYPE_EVNT<<16) |EVN_POST_DRAW,drawType, nCmdDraw);
	}
	else if(pBox != NULL)
	{
		OSD_SetVscrModified(pvscr);
		OSD_UpdateVscr(pvscr);
	}
}

void  OSD_ClearObject(POBJECT_HEAD pObj, UINT32 nCmdDraw)
{
	PRESULT Result;
	UINT32 vscr_idx,drawType;
	PGUI_VSCR pslvscr,pvscr;
	
	drawType= C_DRAW_TYPE_HIDE;
	if(nCmdDraw & C_DRAW_SIGN_EVN_FLG)    
	{
		Result = OSD_ObjProc(pObj,(MSG_TYPE_EVNT<<16) |EVN_PRE_DRAW,drawType, nCmdDraw);
		if(Result != PROC_PASS)
			return ;
	}    

	pvscr = OSD_GetVscr(&pObj->frame,VSCR_REQBUF);
	OSD_SetVscrModified(pvscr);
	OSD_DrawRect2Vscr(&pObj->frame,transcolor_const[pvscr->color_mode],pvscr);
	if(gelib_getdual_support())
	{
		POBJECT_HEAD pslv;

		pslv = OSD_GetSlaveObject(pObj);
		if(NULL != pslv)
		{
			pslvscr = OSD_GetSlvscr(&pslv->frame,VSCR_NULL);
			OSD_SetVscrModified(pslvscr);
			OSD_DrawRect2Vscr(&pslv->frame,transcolor_const[pslvscr->color_mode],pslvscr);
		}
	}

	if(nCmdDraw & C_DRAW_SIGN_EVN_FLG)
	{
		OSD_ObjProc(pObj,(MSG_TYPE_EVNT<<16) |EVN_POST_DRAW,drawType, nCmdDraw);
	}
	else if(pObj->pRoot == NULL)
	{
		OSD_UpdateVscr(pvscr);
        del_object_record(pObj);

		if(!OSD_GetAttrSlave(pObj))
			osdwnd_hidewnd(NULL,(UINT32)pObj);
	}
}

/* Object common processing function*/
PRESULT OSD_NotifyEvent(POBJECT_HEAD pObj, VEVENT Event, UINT32 Param1, UINT32 Param2)
{
    if(pObj->pfnCallback)
        return pObj->pfnCallback(pObj, Event, Param1, Param2);
    return PROC_PASS;
}

UINT8 evts_to_child[] = 
{
	EVN_FOCUS_PRE_LOSE,
	EVN_FOCUS_POST_LOSE,
	EVN_FOCUS_PRE_GET,
	EVN_FOCUS_POST_GET,	
};
#define EVT_TO_CHILD_NUM	(sizeof(evts_to_child))

UINT8 evts_to_childmap[][2] =
{
	{EVN_FOCUS_PRE_LOSE,	EVN_PARENT_FOCUS_PRE_LOSE},
	{EVN_FOCUS_POST_LOSE,	EVN_PARENT_FOCUS_POST_LOSE},
	{EVN_FOCUS_PRE_GET,	EVN_PARENT_FOCUS_PRE_GET},
	{EVN_FOCUS_POST_GET,		EVN_PARENT_FOCUS_POST_GET},
};
#define EVT_TO_CHILDMAP_NUM		(sizeof(evts_to_childmap)/2)

BOOL OSD_EventPassToChild(UINT32 msg,UINT32* submsg)
{
	UINT32 i,n;
	UINT32	subMsgCode;
	BOOL b = FALSE; 

	subMsgCode = msg;
	for(i=0;i<EVT_TO_CHILD_NUM;i++)
		if(msg == evts_to_child[i])
		{
			b = TRUE;
			break;
		}
		
	if(b)			
	{
		for(i=0;i<EVT_TO_CHILDMAP_NUM;i++)
			if(msg == evts_to_childmap[i][0])
			{
				subMsgCode = evts_to_childmap[i][1];
				break;
			}
	}
	
	if(b)
	{
		*submsg =subMsgCode;
	}
	
	return b;		
}

PRESULT OSD_ObjOpen(POBJECT_HEAD pObj, UINT32 param)
{
	PRESULT Result;
	OBJECT_HEAD *pFoucsObj;
	UINT32 nCmdDraw;
	PGUI_VSCR  pvscr;
	
	nCmdDraw = 0;
	pFoucsObj = OSD_GetFocusObject(pObj);
	Result = OSD_ObjProc(pObj,(MSG_TYPE_EVNT<<16) | EVN_PRE_OPEN, (UINT32)pFoucsObj,param);    
	if(Result != PROC_PASS)
		return Result;

    add_object_record(pObj);
	if(g_ObjectOpenNotify) g_ObjectOpenNotify(pObj);
	nCmdDraw = 	C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL | C_DRAW_TYPE_HIGHLIGHT;
	Result = OSD_TrackObject((POBJECT_HEAD)pObj,nCmdDraw);//slave only need draw,recurse here
	pFoucsObj = OSD_GetFocusObject(pObj);
	Result = OSD_ObjProc(pObj,(MSG_TYPE_EVNT<<16) |EVN_POST_OPEN,(UINT32)pFoucsObj,param);  

	if((pObj->pRoot == NULL)&&(!OSD_GetAttrSlave(pObj)))
	{
		pvscr = OSD_GetTaskVscr(osal_task_get_current_id());
		
		OSD_UpdateVscr(pvscr);
		osdwnd_showwnd(NULL,(UINT32)pObj);
	}

	return PROC_PASS;
}

PRESULT OSD_ObjClose(POBJECT_HEAD pObj, UINT32 param)
{
	PRESULT Result;
	OBJECT_HEAD *pFoucsObj;
	UINT32 evtpara;
	PGUI_VSCR pvscr;

	if(pObj->pRoot != NULL)
		return PROC_LOOP;

	evtpara = param;
	pFoucsObj = OSD_GetFocusObject(pObj);
	Result = OSD_ObjProc(pObj,(MSG_TYPE_EVNT<<16) |EVN_PRE_CLOSE,(UINT32)pFoucsObj,(UINT32)&evtpara);    
	if(Result != PROC_PASS)
		return Result;

	if(g_ObjectCloseNotify) g_ObjectCloseNotify(pObj);
	if((param & evtpara & C_CLOSE_CLRBACK_FLG) && bClearbackFlag)
	{
		OSD_ClearObject(pObj,0);//need not slave recurse
		if(!OSD_GetAttrSlave(pObj))
			osdwnd_hidewnd(NULL,(UINT32)pObj);
	}

	Result = OSD_ObjProc(pObj,(MSG_TYPE_EVNT<<16) |EVN_POST_CLOSE,(UINT32)pFoucsObj,(UINT32)param);   
	pvscr = OSD_GetTaskVscr(osal_task_get_current_id());
	OSD_UpdateVscr(pvscr);
    del_object_record(pObj);

	return PROC_LEAVE;
}

PRESULT OSD_ObjCommonProc(POBJECT_HEAD pObj, UINT32 hkey, UINT32 Param,
						  UINT32* pvkey,VACTION* pvact,BOOL* bContinue)
{
	PRESULT Result;
	VACTION Action = VACT_PASS;
	UINT32	vkey;
	UINT32	i = 0;
	*bContinue = FALSE;

	if(pObj->pfnKeyMap == NULL)
		return OSD_SIGNAL(pObj,EVN_UNKNOWNKEY_GOT,hkey,Param);

	do
	{
		i = g_gui_rscfunc.ap_hk_to_vk(i, hkey, &vkey);
		if(vkey!=V_KEY_NULL)
			Action = pObj->pfnKeyMap(pObj,vkey);
	}while(Action == VACT_PASS && vkey!=V_KEY_NULL); 
	
	*pvkey = vkey;
	*pvact  = Action;
	if(vkey==V_KEY_NULL)
		return OSD_SIGNAL(pObj,EVN_UNKNOWNKEY_GOT,hkey,Param);
	
	if(Action == VACT_PASS)
		return PROC_PASS;

	Result = OSD_SIGNAL(pObj, EVN_KEY_GOT, (Action<<16) | vkey, Param);
	if(Result == PROC_LOOP || Result ==PROC_LEAVE)
		return Result;

	*bContinue = TRUE;            	        
	return PROC_PASS;
}

PRESULT OSD_ObjProc(POBJECT_HEAD pObj, UINT32 msg_type,UINT32 msg,UINT32 param1)
{
	PRESULT Result= PROC_PASS;
	UINT16	osd_msg_type, user_define_type;
	PGUI_VSCR pvscr;
	
	osd_msg_type = msg_type>>16;
	user_define_type = (UINT16)msg_type;
	if((MSG_TYPE_KEY == osd_msg_type)  || (MSG_TYPE_EVNT == osd_msg_type))
	{
		if(MSG_TYPE_EVNT == osd_msg_type)
			osd_msg_type = user_define_type;
			
		switch(pObj->bType)
		{
		 case OT_TEXTFIELD:
		 	Result = OSD_TextFieldProc(pObj,osd_msg_type,msg,param1);
		 	break;
		 case OT_BITMAP:
		 	Result = OSD_BitmapProc(pObj,osd_msg_type,msg,param1);
		 	break;
		case OT_EDITFIELD:
			Result = OSD_EditFieldProc(pObj,osd_msg_type,msg,param1);
		 	break;
		 case OT_MULTISEL:
		 	Result = OSD_MultiselProc(pObj,osd_msg_type,msg,param1);
		 	break;
		 case OT_PROGRESSBAR:
		 	Result = OSD_ProgressBarProc(pObj,osd_msg_type,msg,param1);
		 	break;
		 case OT_SCROLLBAR:
		 	Result = OSD_ScrollBarProc(pObj,osd_msg_type,msg,param1);
		 	break;
		 case OT_MULTITEXT:
		 	Result = OSD_MultiTextProc(pObj,osd_msg_type,msg,param1);
		 	break;
		 case OT_LIST:
		 	Result = OSD_ListProc(pObj,osd_msg_type,msg,param1);
		 	break;
		 case OT_MATRIXBOX:
		 	Result = OSD_MatrixboxProc(pObj,osd_msg_type,msg,param1);
		 	break;
		 case OT_OBJLIST:
		 	Result = OSD_ObjectListProc(pObj,osd_msg_type,msg,param1);
		 	break;
		 case OT_CONTAINER:
		 	Result = OSD_ContainerProc(pObj,osd_msg_type,msg,param1);
		 	break;
		 default:
		 	;			 			
		}
	}
	else if(osd_msg_type == MSG_TYPE_MSG)
	{
		Result = OSD_SIGNAL(pObj,EVN_MSG_GOT,user_define_type,msg);
		if(Result == PROC_LEAVE)
			Result = OSD_ObjClose(pObj,C_CLOSE_CLRBACK_FLG);
	}
	
	if((pObj->pRoot == NULL)&&(!OSD_GetAttrSlave(pObj)))
	{
		pvscr = OSD_GetTaskVscr(osal_task_get_current_id());
		OSD_UpdateVscr(pvscr);
	}

	return Result;
}

UINT8 app_rsc_init(PGUI_RSC_FNC p_rsc_info)
{
	g_gui_rscfunc.osd_get_font_lib = p_rsc_info->osd_get_font_lib;
	g_gui_rscfunc.osd_get_lang_env = p_rsc_info->osd_get_lang_env;
	g_gui_rscfunc.osd_get_obj_info = p_rsc_info->osd_get_obj_info;
	g_gui_rscfunc.osd_get_rsc_data =p_rsc_info->osd_get_rsc_data ;
	g_gui_rscfunc.osd_get_thai_font_data =p_rsc_info->osd_get_thai_font_data;
	g_gui_rscfunc.osd_get_str_lib_id = p_rsc_info->osd_get_str_lib_id;
	g_gui_rscfunc.osd_get_win_style = p_rsc_info->osd_get_win_style ;
	g_gui_rscfunc.osd_rel_rsc_data = p_rsc_info->osd_rel_rsc_data;
	g_gui_rscfunc.ap_hk_to_vk = p_rsc_info->ap_hk_to_vk;/*Attach Rsc functions*/
}

UINT8* OSD_GetUnicodeString(UINT16 uIndex)
{
	ID_RSC	    RscId;
	OBJECTINFO	RscLibInfo;    

	RscId = g_gui_rscfunc.osd_get_str_lib_id(g_gui_rscfunc.osd_get_lang_env());    
	return g_gui_rscfunc.osd_get_rsc_data(RscId, uIndex, &RscLibInfo);
}

BOOL OSD_GetCharWidthHeight(UINT16 uString,UINT8 font, UINT16* width, UINT16* height)
{
	OBJECTINFO	ObjInfo;
	ID_RSC		RscLibId;

	RscLibId = g_gui_rscfunc.osd_get_font_lib(uString) | font;
	if(g_gui_rscfunc.osd_get_obj_info( RscLibId, uString,&ObjInfo))
	{
		*width	= ObjInfo.m_objAttr.m_wActualWidth;
		*height = ObjInfo.m_objAttr.m_wHeight;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

UINT16 OSD_MultiFontLibStrMaxHW(UINT8* pString,UINT8 font, UINT16 *wH, UINT16 *wW,UINT16 strLen)
{
	UINT16 		wMaxH=0, wMaxW=0, wLen=0,w,h;
	OBJECTINFO	ObjInfo;
	ID_RSC		RscLibId;
	UINT16 		fontSize;
	UINT16		uString;
	UINT16		i = 0;
	UINT32		cnt;

	while( 0 != (uString = ComMB16ToWord(pString)))	// = NULL?
	{
		cnt = 2;
		if(is_thai_unicode(uString))
		{
			struct thai_cell cell;
			
			cnt = thai_get_cell(pString, &cell);
			if(cnt == 0)
				break;
			if(OSD_GetThaiCellWidthHeight(&cell,font,&w,&h))
			{
				wMaxH = MAX(wMaxH,h);
				wMaxW = MAX(wMaxW,w);
				wLen += w;
			}
		}
		else
		{
			if(OSD_GetCharWidthHeight(uString,font,&w,&h))
			{
				wMaxH = MAX(wMaxH,h);
				wMaxW = MAX(wMaxW,w);
				wLen += w;
			}
		}
		
		i++;
		pString += cnt;
		if(strLen>0 && i==strLen)
		{
			wMaxH = h;
			wMaxW = w;				
			break;
		}
	}
	*wH = wMaxH;
	*wW = wMaxW;
	
	return wLen;
}

BOOL OSD_GetThaiCellWidthHeight(struct thai_cell *cell, UINT8 font, UINT16* width, UINT16* height)
{
	UINT16 w,h;
	UINT32 i;
	OBJECTINFO	ObjInfo;
	ID_RSC		RscLibId;

	if(cell == NULL)
		return FALSE;

	w = 0;
	h = 0;
	for(i=0; i<cell->char_num; i++)
	{
		RscLibId = g_gui_rscfunc.osd_get_font_lib(cell->unicode[i]) | font;
		if(g_gui_rscfunc.osd_get_obj_info( RscLibId, cell->unicode[i],&ObjInfo))
		{
			if(w<ObjInfo.m_objAttr.m_wActualWidth)
				w = ObjInfo.m_objAttr.m_wActualWidth;
			if(h<ObjInfo.m_objAttr.m_wHeight)
				h = ObjInfo.m_objAttr.m_wHeight;
		}
		else 
			return FALSE;
	}	
	*width = w;
	*height = h;
	
	return TRUE;
}

UINT8 OSD_GetShowIndex(const PCOLOR_STYLE pColorStyle, UINT8 bAttr)
{
	UINT8 *pStyle = (UINT8 *)pColorStyle;

	//OSD_ASSERT((bAttr&C_ATTR_LMASK) <= sizeof(COLOR_STYLE));
	return (0 == (bAttr&C_ATTR_LMASK)) ? pStyle[0] : pStyle[(bAttr&C_ATTR_LMASK)-1];
}

BOOL OSD_GetLibInfoByWordIdx(UINT16 wIdx, ID_RSC RscLibId, UINT16 *wWidth, UINT16 *uHeight)
{
	UINT16 		wIdxTmp = BE_TO_LOCAL_WORD(wIdx);
	OBJECTINFO	ObjInfo;

	*wWidth = 0;
	*uHeight = 0;
	if(g_gui_rscfunc.osd_get_obj_info(RscLibId, wIdxTmp, &ObjInfo))
	{
		*wWidth = ObjInfo.m_objAttr.m_wActualWidth;
		*uHeight = ObjInfo.m_objAttr.m_wHeight;
	}
	
	return (*wWidth > 0) ? TRUE : FALSE;
}

UINT8* OSD_GetRscPallette(UINT16 pal_idx)
{
	OBJECTINFO	RscLibInfo;
	
	pal_idx = BE_TO_LOCAL_WORD(pal_idx);
	return (g_gui_rscfunc.osd_get_rsc_data(LIB_PALLETE, pal_idx, &RscLibInfo));
}

void OSD_SetCloseClrbackFlag(BOOL bClear)
{
    bClearbackFlag_bakeup = bClearbackFlag;
    bClearbackFlag = bClear;
}

void OSD_ResumeCloseClrbackFlag(void)
{
    bClearbackFlag = bClearbackFlag_bakeup;
}