/*-----------------------------------------------------------------------------
    
    Copyright (C) 2010 ALi Corp. All rights reserved.
    
    File: obj_multitext.c revised by Shine Zhou
-----------------------------------------------------------------------------*/
#include <sys_config.h>
#include <api/libc/string.h>
#include <api/libge/osd_lib.h>
#include <api/libge/osd_common_draw.h>
#include <api/libge/osd_primitive.h>
#include "osd_lib_internal.h"

#define C_LINE_GAP  4

static char word_end_char[] = 
{
    ' ', ',', '.', ':', ';','?','!','/','-','(',')','{','}','[',']','|','=','+','&','@','$','_',      
};

BOOL IS_WORD_END(UINT16  wc)
{
    UINT32 i,n;

    n = sizeof(word_end_char);
    for(i=0;i<n;i++)
    {
        if(wc == (UINT16)word_end_char[i])
            return TRUE;
    }

    return FALSE;
    
}

UINT16 OSD_GetWordLen(UINT8* pText, UINT8 font,UINT16* wordwidth,UINT16* wordheight ,UINT16 width)
{
	UINT16 wc,wordlen = 0;
	UINT16 w,h,mh = 0,ww = 0;
	UINT32 cnt;
	
	while( (wc = ComMB16ToWord(pText)) != 0)
	{
		if(IS_NEWLINE(wc))
		{
			*wordwidth = ww;
			*wordheight = mh;
			return wordlen;
		}
		else if((wc > 0x4e00 && wc < 0x9f45)||
		(wc > 0x3000 && wc < 0x303f)||  
		(wc > 0xff00 && wc < 0xffef)||              
		(wc > 0x2e80 && wc < 0x2eff))//chinese
		{
			if(ww==0)
			{
				//ww += 24;
				//mh = 24;
				OSD_GetCharWidthHeight(wc ,font, &w, &h);
				ww = w;
				mh = h;
				wordlen += 1;
				pText += 2;
			}
			break;
		}
		else if(IS_WORD_END(wc) && wordlen>0)
			break;
		w = 10;
		h = 0;
		cnt = 2;
		if(is_thai_unicode(wc))
		{
			struct thai_cell cell;

			cnt = thai_get_cell(pText, &cell);
			if(cnt == 0)
			break;

			OSD_GetThaiCellWidthHeight(&cell, font,&w,&h);
		}
		else
		{
			OSD_GetCharWidthHeight(wc,font, &w, &h);
		}
		if(mh < h)
			mh = h;
		if((ww+w)<=width)
			ww += w;
		else
			break;
		
		wordlen += cnt/2;
		pText += cnt;

		if(IS_WORD_END(wc))
			break;
	}
	*wordwidth = ww;
	*wordheight = mh;

	return wordlen;
}

UINT16 OSD_GetTextTotalLine(UINT8* pText,UINT8 font,UINT16 width,
                              INT16 lineidx,UINT8**  lineStr, UINT8* linesheght)
{
	UINT16 l,wc,wordlen,chl,wordw,wordh,line;

	l = 0;
	chl = 0;
	line = 0;
	if(pText==NULL)
		return 0;
    
	wc = ComMB16ToWord(pText);
	while(wc!=0 && line<MAX_LINE_NUM)
	{       
	    if(l==0 && line == lineidx  && lineStr!=NULL)
	        *lineStr = pText;
	    
	    if(IS_NEWLINE(wc))
	    {            
	        if(chl== 0)
	            chl = OSD_FONT_HEIGHT;
	        if(linesheght != NULL)
	            linesheght[line] = chl;
			
	        line++;
	        l = 0;
	        chl = 0;
	        pText += 2;
	    }
	    else
	    {
	        wordlen = OSD_GetWordLen(pText,font,&wordw,&wordh,width);
	        if(wordlen > 0)
	        {
	            if(chl== 0)
	                chl = wordh;

	            if(l + wordw <= width || l == 0)
	            {
	                l += wordw;
	                pText += wordlen*2;
	            }
	            else    /* Next line*/
	            {
	                if(chl < wordh)
	                    chl = wordh;
	                if(chl== 0)
	                    chl = OSD_FONT_HEIGHT;
	                if(linesheght != NULL)
	                    linesheght[line] = chl;
					
	                line++;
	                l = 0;
	                chl = 0;
	            }
	        }
	    }
	    
		wc = ComMB16ToWord(pText);
		if(l !=0 && wc==0)
		{//end of the multitext content
			if(chl== 0)
				chl = OSD_FONT_HEIGHT;
			
			linesheght[line] = chl;
			line++;
		}
	}

	return line;
}

void OSD_GetMTextInfor(PMULTI_TEXT pCtrl,mtxtinfo_t * pInfo)
{
	UINT32 i;
	UINT8 mtfont,*pStr;
	PTEXT_CONTENT pText;
	UINT16 lidx,total_lines,page_lines,lines,width,height;

	page_lines = 0;
	total_lines = 0;
	width  = pCtrl->rcText.uWidth;
	MEMSET(&(pInfo->line_height[0]),0x0,MAX_LINE_NUM*sizeof((pInfo->line_height[0])));
	
	if(OSD_GetAttrSlave(pCtrl))
		mtfont = (pCtrl->head.bFont)|C_FONT_SLVMASK;
	else
		mtfont = pCtrl->head.bFont;
		
	for(i=0; i<pCtrl->bCount; i++)
	{
		pText = &pCtrl->pTextTable[i];

		if(pText == NULL)
			continue;

		if(pText->bTextType == (UINT8)STRING_ID)
			pStr = OSD_GetUnicodeString(pText->text.wStringID);
		else
			pStr = (UINT8*)pText->text.pString;
		if(pStr== NULL)
			continue;
        
		lidx = (INT16)pInfo->topline_idx - (INT16)total_lines;
		lines = OSD_GetTextTotalLine(pStr,mtfont,width,lidx,&pInfo->topline_pstr,&pInfo->line_height[total_lines]);
		if(pInfo->topline_idx >= total_lines&& pInfo->topline_idx< lines + total_lines)
			pInfo->topline_tblidx = i;
		
		total_lines += lines;
    }

	pInfo->total_lines = total_lines;
	pInfo->total_height = 0;
	for(i=0;i<total_lines;i++)
	{
		pInfo->total_height += pInfo->line_height[i];
		if( i> 0)
			pInfo->total_height += C_LINE_GAP;
	}

	pInfo->page_height = pCtrl->rcText.uHeight;
	if(pInfo->topline_idx >= total_lines)
		pInfo->topline_idx = 0;

    /* Check the topline_idx */
CHECK_TOP_LINE: 
    height = 0;
    for(i=pInfo->topline_idx;i<total_lines;i++)
    {
        height += pInfo->line_height[i];
        if(i != pInfo->topline_idx)
            height += C_LINE_GAP;
    }
    if(pInfo->topline_idx> 0 && height < pInfo->page_height)
    {
        if(height + pInfo->line_height[total_lines - 1] + C_LINE_GAP <= pInfo->page_height)
        {
            pInfo->topline_idx -= 1;        
            goto CHECK_TOP_LINE;
        }
    }

    /* Get Current Page's lines */
    height = 0;
    for(i=pInfo->topline_idx;i<total_lines;i++)
    {
        height += pInfo->line_height[i];
        if(i != pInfo->topline_idx)
            height += C_LINE_GAP;
        if(height > pInfo->page_height)
            break;
    }

    page_lines = i - pInfo->topline_idx;
    pInfo->page_lines  = page_lines;    
}

void OSD_SetMultiTextContent(PMULTI_TEXT pCtrl, PTEXT_CONTENT pTextTable)
{
        pCtrl->pTextTable = pTextTable;
}

void OSD_DrawMultiTextCell(PMULTI_TEXT pCtrl, UINT8 bStyleIdx,UINT32 nCmdDraw)
{
	PGUI_VSCR 		pVscr;
	PGUI_RECT objframe;	
	PWINSTYLE  	lpWinSty;
	UINT32		mtxtstyle,bgColor;
	UINT8 			bAlign,hAligin,vAlign;
	OBJECTINFO	RscLibInfo;
	GUI_RECT r;

	PTEXT_CONTENT pText;
	UINT16 width,height,left,right,bottom;
	INT16  lidx;
	UINT32 i,k;
	UINT8 *pStr,*pDrawStr;
	UINT16  wc;
	UINT16 wordlen,wordw,wordh,maxW,maxH,charw,charh;
	UINT8 font;
	UINT16 ox,oy,x,y,l,t;
	mtxtinfo_t mtxtinfo;
	UINT32 bNewLine;
	BOOL reverse_flag;
    
	if(OSD_GetUpdateType(nCmdDraw) == C_UPDATE_CONTENT)
		mtxtstyle = (UINT32)(~0);
	else
		mtxtstyle = bStyleIdx;
	objframe	= &pCtrl->head.frame;
	lpWinSty 	= g_gui_rscfunc.osd_get_win_style(bStyleIdx);
	bgColor     = lpWinSty->wBgIdx;
	bAlign 		= pCtrl->bAlign;
	hAligin     = GET_HALIGN(bAlign);
	vAlign      = GET_VALIGN(bAlign);
	
	if(OSD_GetAttrSlave(pCtrl))
	{
		font  = ((pCtrl->head.bFont) |C_FONT_SLVMASK);
		pVscr = OSD_DrawSlaveFrame(objframe,bStyleIdx);
	}
	else
	{
		font  = pCtrl->head.bFont;
		pVscr = OSD_DrawObjectFrame(objframe,mtxtstyle);
	}
    
	r = *objframe;
	r.uStartX += pCtrl->rcText.uStartX;
	r.uStartY  += pCtrl->rcText.uStartY;
	r.uWidth    = pCtrl->rcText.uWidth;
	r.uHeight   = pCtrl->rcText.uHeight;
	width  = r.uWidth;
	height = r.uHeight;

	GET_MULTITEXT_INFOR:
	mtxtinfo.topline_idx = (UINT8)pCtrl->nLine;
	mtxtinfo.topline_pstr = NULL;
	OSD_GetMTextInfor(pCtrl,&mtxtinfo);
	if(mtxtinfo.topline_pstr == NULL && pCtrl->nLine!=0)
	{
		pCtrl->nLine = 0;
		goto GET_MULTITEXT_INFOR;
	}
	if(mtxtinfo.topline_pstr == NULL)
	return;

	ox = 0;
	if(mtxtinfo.total_lines == 1)
	{
		maxW = OSD_MultiFontLibStrMaxHW(mtxtinfo.topline_pstr,font,&maxW,&maxH,0);
		if(maxW<width)
		{
			if(hAligin==C_ALIGN_RIGHT)
				ox = width - maxW;
			else if(hAligin == C_ALIGN_CENTER)
				ox = (width - maxW)>>1;
		}        
	}
    
	
	if(vAlign == C_ALIGN_TOP)
		oy = 0;
	else if(vAlign == C_ALIGN_BOTTOM)
	{
		if(mtxtinfo.total_height < height)
			oy = height - mtxtinfo.total_height;
		else
			oy = 0;
	}
	else
	{
		if(mtxtinfo.total_height < height)
			oy = (height - mtxtinfo.total_height)>>1;
		else
			oy = 0;
	}
	x = r.uStartX + ox;
	y = r.uStartY + oy;
	l = x;
	t = y;
	bottom = r.uStartY + height;
	right  = r.uStartX + width;
	left = r.uStartX;

	bNewLine = 0;
	lidx  = mtxtinfo.topline_idx ;
	for(i=0; i<pCtrl->bCount; i++)
	{
		/* Draw from the top line*/
		if(i<mtxtinfo.topline_tblidx)
			continue;

		pText = &pCtrl->pTextTable[i];
		if(pText == NULL)
			continue;

		reverse_flag = FALSE;
		if(pText->bTextType == STRING_ID)
			pStr = OSD_GetUnicodeString(pText->text.wStringID);
		else
		{
			pStr = (UINT8*)pText->text.pString;
			if(pText->bTextType == STRING_REVERSE)
				reverse_flag = TRUE;
		}
		if(pStr == NULL)
			continue;

		if(i == mtxtinfo.topline_tblidx)    /* The top line's string pointer.*/
			pDrawStr = mtxtinfo.topline_pstr;
		else
			pDrawStr = pStr;

		if(reverse_flag)
		{
			//force to right align
			x = r.uStartX+r.uWidth;
			l = x;
		}

		while(1)
		{
			wc = ComMB16ToWord(pDrawStr);
			if(wc == 0)
			{   
				/* If the last character is not a new line 
				character, then set next line from the start.*/
				if(!bNewLine)
				{
					l = x;
					t += mtxtinfo.line_height[lidx] + C_LINE_GAP;
				}
				break;
			}
			else if(IS_NEWLINE(wc)) /* New line characters */
			{
				l = x;
				t += mtxtinfo.line_height[lidx++] + C_LINE_GAP;
				pDrawStr += 2;
				wordlen = 0;
				bNewLine = 1;
			}
			else
			{   
				/* Get a word */
				wordlen = OSD_GetWordLen(pDrawStr,font,&wordw,&wordh,width);
				if(wordlen>0 && l!=x)
				{
					if((!reverse_flag&&(l + wordw > right))||(reverse_flag&&(l<left+wordw)))
					{
						l = x;
						t += mtxtinfo.line_height[lidx++] + C_LINE_GAP;
						bNewLine = 1;
					}
				}
				else
					bNewLine = 0;
			}

			if(t>bottom)
				break;

			/* Draw the word's every characters.*/
			for(k=0;k<wordlen;k++)
			{
				wc = ComMB16ToWord(pDrawStr);
				if(is_thai_unicode(wc))
				{
					struct thai_cell cell;
					UINT32 cnt;

					cnt = thai_get_cell(pDrawStr, &cell);
					if(cnt == 0)
						break;
					if((t + mtxtinfo.line_height[lidx]) <= bottom)
						charw = OSD_DrawThaiCell(l,t ,lpWinSty->wFgIdx,bgColor,&cell, font,pVscr);

					l+= charw;
					pDrawStr += cnt;
					k += cnt/2-1;		
				}
				else
				{
					if(reverse_flag)
					{
						OSD_GetCharWidthHeight(wc, font, &charw, &charh);
						l -= charw;
					}
					if((t + mtxtinfo.line_height[lidx]) <= bottom)
						charw = OSD_DrawChar2Vscr(l,t ,lpWinSty->wFgIdx,bgColor,wc,font,pVscr);
					if(!reverse_flag)
						l+= charw;

					pDrawStr += 2;
				}
			}
		}
		
		if(t>=bottom)
			break;
	}
	OSD_SetVscrModified(pVscr);

	if(pCtrl->scrollBar != NULL)
	{
		OSD_SetScrollBarMax(pCtrl->scrollBar, mtxtinfo.total_lines);
		OSD_SetScrollBarPage(pCtrl->scrollBar, mtxtinfo.page_lines);
		OSD_SetScrollBarPos(pCtrl->scrollBar,mtxtinfo.topline_idx);
		OSD_DrawObject((POBJECT_HEAD)pCtrl->scrollBar, C_DRAW_SIGN_EVN_FLG);
	}
}

VACTION OSD_MultiTextKeyMap(POBJECT_HEAD pObj, UINT32 Key)
{
	VACTION Action = VACT_PASS;
	
	switch(Key)
    {
        case V_KEY_UP:
            Action = VACT_CURSOR_UP;
            break;
        case V_KEY_P_UP:
        	Action = VACT_CURSOR_PGUP;
            break;
        case V_KEY_DOWN:
            Action = VACT_CURSOR_DOWN;
            break;
        case V_KEY_P_DOWN:
        	Action = VACT_CURSOR_PGDN;
        	break;
        default:
			break;
	}

	return Action;
}

PRESULT OSD_MultiTextProc(POBJECT_HEAD pObj, UINT32 msg_type,UINT32 msg,UINT32 param1)
{
	PRESULT Result = PROC_LOOP;
	PMULTI_TEXT pCtrl = (PMULTI_TEXT)pObj;
	INT32 step;
	UINT16 total_line, page_line;
	INT16 line_num;


	if(msg_type == MSG_TYPE_KEY)
	{
		VACTION Action;
		UINT32 vkey;
		BOOL bContinue;
		mtxtinfo_t mtxtInfo;
	    	
		Result = OSD_ObjCommonProc(pObj,msg,param1,&vkey,&Action,&bContinue);
		if(!bContinue)
			goto CHECK_LEAVE;
	
		GET_MULTITEXT_INFOR:    
		mtxtInfo.topline_idx = pCtrl->nLine;
		OSD_GetMTextInfor(pCtrl,&mtxtInfo);
		if(mtxtInfo.total_lines > 0 && mtxtInfo.topline_pstr == NULL)
		{
			pCtrl->nLine = 0;
			goto GET_MULTITEXT_INFOR;
		}
        
		pCtrl->nLine = mtxtInfo.topline_idx;
		line_num = pCtrl->nLine;
		page_line  = mtxtInfo.page_lines;
		total_line = mtxtInfo.total_lines;

		switch(Action)
		{
		case VACT_CURSOR_UP:
			step = -1;
			goto multitext_scroll;
		case VACT_CURSOR_DOWN:
			step = 1;
			goto multitext_scroll;
		case VACT_CURSOR_PGUP:
			step = -page_line;
			goto multitext_scroll;
		case VACT_CURSOR_PGDN:
			step = page_line;
multitext_scroll:
            if(total_line == 0)
                return PROC_LOOP;
			line_num += step;

            if(line_num + page_line>total_line)
                line_num = (INT16)total_line - (INT16)page_line;
            if(line_num < 0)
                line_num = 0;
     

			if(pCtrl->nLine == (UINT16)line_num)
				return PROC_LOOP;
    		
			Result = OSD_SIGNAL(pObj, EVN_PRE_CHANGE, (UINT32)&line_num, 0);
			if(Result != PROC_PASS)
				goto CHECK_LEAVE;

			pCtrl->nLine = (UINT16)line_num;
			OSD_TrackObject(pObj, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);

			OSD_SIGNAL(pObj, EVN_POST_CHANGE, line_num, 0);
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
	else// if(osd_msg_type==MSG_TYPE_EVNT)
		Result = OSD_SIGNAL(pObj, msg_type, msg, param1);
	
CHECK_LEAVE:
	if(!(EVN_PRE_OPEN == msg_type))
		CHECK_LEAVE_RETURN(Result,pObj);

EXIT:
	return Result;
}


