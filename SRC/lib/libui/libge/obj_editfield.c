/*-----------------------------------------------------------------------------
    Copyright (C) 2010 ALi Corp. All rights reserved.
    File: obj_XXX.c
-----------------------------------------------------------------------------*/
#include <sys_config.h>
#include <api/libc/string.h>
#include <api/libge/osd_lib.h>
#include <api/libge/osd_common_draw.h>
#include <api/libge/osd_primitive.h>
#include <api/libge/ge_primitive_init.h>
#include "osd_lib_internal.h"

#define C_MAX_EDIT_LENGTH   32
typedef struct _EDIT_PATTERN
{
    UINT8 bTag;
    UINT8 bLen;         // length without seperator
    UINT8 bMaxLen;      // length with seperator
    UINT16 wCursorMap;
    union
    {
        struct
        {
            INT8 bSub;
        }f;

        struct
        {
            UINT8 bInit;
            UINT8 bMask;						
        }p;

        struct
        {
            UINT8 bFormat;
            UINT8 bSep;
        }dt;

        struct
        {
            INT32 nMin;
            INT32 nMax;
        }r;
		struct
		{
			UINT16 *pString;
		}s;
		struct
		{
			UINT8 bFormat;
			UINT8 bSep;
		}ip;
		
		struct
		{
			UINT8 bInit;
			UINT8 bMask;
		}m;
    };
}EDIT_PATTERN, *PEDIT_PATTERN;


#define C_TIME_FMT_OFFWSET  5// 4
static char *m_fmtDT[] = 
{
    "Y4M2D2", 
    "Y2M2D2", 
    "M2D2Y4", 
    "M2D2Y2", 
    "D2M2Y4", 	
    "h2m2s2", 
    "h2m2", 
    "m2s2", 
};
#define C_TIME_SEP          3
static char *m_Sep = ".-/:";
static UINT8 key_input_changed; // only for pattern == 'm'

static UINT32 ConvertIP2int(UINT16 *pString);
static void ConvertValue2String(PEDIT_FIELD pCtrl,UINT16* pString, UINT32 dwValue, PEDIT_PATTERN pPattern);
static PRESULT NotifyChangeEvent(PEDIT_FIELD pCtrl, UINT16* pText, PEDIT_PATTERN pPattern);

static BOOL IsEditString(UINT8 bTag)
{
	return ('p' == bTag || 's' == bTag || 'm' == bTag);
}

static void GetPattern(PEDIT_FIELD pCtrl, PEDIT_PATTERN pPattern)
{
    char* pcsString = pCtrl->pcsPattern;
    char* pfmt;
    UINT32 i, j;
    UINT8 bLen = 0;

    MEMSET(pPattern, 0, sizeof(EDIT_PATTERN));
    pPattern->bLen = pCtrl->bMaxLen;
    pPattern->bMaxLen = pCtrl->bMaxLen;
    if(!pcsString)
        return;
    switch(pcsString[0])
    {
    case 'f':
       	pPattern->f.bSub = pcsString[2] - '0';
		OSD_ASSERT(pPattern->f.bSub);
		pPattern->bLen = pcsString[1] - '0' + pPattern->f.bSub;
		pPattern->wCursorMap |= (1 << pPattern->f.bSub);
		pPattern->bMaxLen = pPattern->bLen + 1;
		pPattern->bTag = 'f';
        break;

    case 'p':
        pPattern->p.bInit 	= pcsString[1];			
        pPattern->p.bMask 	= pcsString[2];
        pPattern->bLen 		= pcsString[3] - '0';
        pPattern->bMaxLen	= pPattern->bLen;
        pPattern->bTag 		= 'p';
        break;

    case 'r':
        pPattern->wCursorMap = 0;
        pPattern->r.nMin = pPattern->r.nMax = 0;
        for(i=1; pcsString[i]!='~'; i++)
            pPattern->r.nMin = pPattern->r.nMin * 10 + (pcsString[i] - '0');
        for(j=i+1; pcsString[j]; j++)
            pPattern->r.nMax = pPattern->r.nMax * 10 + (pcsString[j] - '0');
        pPattern->bLen = j - i - 1;
        pPattern->bMaxLen = pPattern->bLen;
        pPattern->bTag = 'r';
        break;

#if EDITFIELD_SUPPORT_TIME
    case 't':
        pPattern->dt.bFormat = pcsString[1] - '0' + C_TIME_FMT_OFFWSET;
        pPattern->dt.bSep = C_TIME_SEP;
        goto get_time_pattern;
    case 'd':
        pPattern->dt.bFormat = pcsString[1] - '0';
        pPattern->dt.bSep = pcsString[2] - '0';
get_time_pattern:
        pfmt = m_fmtDT[pPattern->dt.bFormat];
        bLen = 0;
        pPattern->bLen = 0;
        for(i=0; i<STRLEN(pfmt); i+=2)
        {
            bLen = pfmt[i+1] - '0';
            if(i)
            {
                pPattern->wCursorMap <<= (bLen + 1);
                pPattern->wCursorMap |= (1 << bLen);
            }
            pPattern->bLen += bLen;
        }
        pPattern->bMaxLen = pPattern->bLen + (i/2 - 1);
        pPattern->bTag = 't';
        break;
#endif
	case 's':
		pPattern->s.pString = pCtrl->pString;
		pPattern->bLen = 0;
		for(i=1; pcsString[i]; i++)
			pPattern->bLen = pPattern->bLen * 10 + (pcsString[i] - '0');
		pPattern->bMaxLen = pPattern->bLen;
		pPattern->bTag = 's';
			break;
		case 'i':
			pPattern->wCursorMap = 0x888;
			pPattern->ip.bSep = pcsString[1] - '0';
			pPattern->bTag = 'i';
			pPattern->bMaxLen = pPattern->bLen = 15;
			break;
		case 'm':
			pPattern->m.bInit 	= pcsString[1];
			pPattern->m.bMask 	= pcsString[2];
			pPattern->bLen = 0;
			for(i=3; pcsString[i]; i++)
				pPattern->bLen = pPattern->bLen * 10 + (pcsString[i] - '0');

			if(pPattern->bLen > pCtrl->bMaxLen)
				pPattern->bLen = pCtrl->bMaxLen;
			pPattern->bMaxLen	= pPattern->bLen;
			pPattern->bTag 		= 'm';		
		break;
    }
}

static UINT32 get_edit_cursor_type(PEDIT_FIELD pCtrl,PEDIT_PATTERN pPattern)
{
    UINT32 cursor_type;
    
	switch(pPattern->bTag)
	{
		case 'r':
	    		cursor_type = pCtrl->bCursorMode;
			break;
		case 'm':
		case 's':
			if (CURSOR_NO != pCtrl->bCursorMode)
				cursor_type = CURSOR_NORMAL;	
			else
				cursor_type = pCtrl->bCursorMode;
			break;
		case 'f':
		case 'i':
#if EDITFIELD_SUPPORT_TIME
		case 't':
		case 'd':
#endif
			cursor_type = CURSOR_NORMAL;
			break;
		case 'p':
			cursor_type = CURSOR_NO;
	        	break;
		default:
			cursor_type = CURSOR_NO;
			break;				
		}

    return cursor_type;    
}

static UINT32 get_focus_cursor_type(PEDIT_FIELD pCtrl,PEDIT_PATTERN pPattern)
{
    UINT32 cursor_type;
    UINT32 mode,status; 
    
    mode   = OSD_GetEditFieldMode(pCtrl);
    status = OSD_GetEditFieldStatus(pCtrl);

    cursor_type = get_edit_cursor_type(pCtrl,pPattern);
    
    if(cursor_type == CURSOR_NORMAL)
    {
        if(status == EDIT_STATUS
            || mode == NORMAL_EDIT_MODE)
            return CURSOR_NORMAL;
    }
    else if(cursor_type == CURSOR_SPECIAL)
    {
        if(status == EDIT_STATUS)
            return CURSOR_SPECIAL;
    }

    return CURSOR_NO;
}

static BOOL check_focus_can_input09(PEDIT_FIELD pCtrl,PEDIT_PATTERN pPattern)
{
    UINT32 mode,status; 
    
    mode   = OSD_GetEditFieldMode(pCtrl);
    status = OSD_GetEditFieldStatus(pCtrl);

    if(status == EDIT_STATUS
        || mode == NORMAL_EDIT_MODE)
        return TRUE;
    else    /* SELECT_EDIT_MODE && status != EDIT_STATUS*/
        return FALSE;
}

static BOOL check_focus_can_addsub(PEDIT_FIELD pCtrl,PEDIT_PATTERN pPattern)
{
    BOOL b;
    UINT32 cursor_type;
    UINT32 mode,status; 
    
    mode   = OSD_GetEditFieldMode(pCtrl);
    status = OSD_GetEditFieldStatus(pCtrl);
    if(pPattern->bTag != 'r')
        return FALSE;

    cursor_type = pCtrl->bCursorMode;
    
    if(cursor_type == CURSOR_NO)
        return TRUE;
    else if(cursor_type == CURSOR_NORMAL)
        return FALSE;
    else    // CURSOR_SPECIAL
    {
        if(status == SELECT_STATUS)
            return TRUE;
        else
            return FALSE;
    }
    
}

static void check_cursor_pos(PEDIT_FIELD pCtrl,PEDIT_PATTERN pPattern)
{
    UINT32 cursor_type;
    INT32 i;
	BOOL b;   
	UINT32 mode,status; 
    INT8 Cursor;
    UINT8 bMaxLen,strLen;

    bMaxLen = pPattern->bMaxLen;
    if(pPattern->bTag == 'p')
        bMaxLen += 1;
	else if (pPattern->bTag == 's' || pPattern->bTag == 'm')
    {
        strLen =  ComUniStrLen(pCtrl->pString);
        if(strLen < bMaxLen)
            strLen = strLen + 1;
    }
    
    if(pCtrl->bCursor >= bMaxLen)
        pCtrl->bCursor = 0;

    cursor_type = get_edit_cursor_type(pCtrl,pPattern);
    if(cursor_type != CURSOR_NORMAL)
        return;

	if(pPattern->bTag != 's' && pPattern->bTag != 'm')
	{
	    Cursor = pCtrl->bCursor;
	    for(i=0;i<pPattern->bMaxLen;i++)
        {
		    Cursor = pCtrl->bCursor;            
		    if(pPattern->wCursorMap & (1 << (pPattern->bMaxLen - Cursor - 1)))
	        {
                if(Cursor < (pPattern->bMaxLen - 1))
                    Cursor ++;
                else
                    Cursor = 0;
	        }
        }
        pCtrl->bCursor = Cursor;
	}
    

    return ;
}

INT8 get_next_cursor_pos(PEDIT_FIELD pCtrl,PEDIT_PATTERN pPattern,INT8 Step)
{
    INT8 Cursor;
    UINT8 bMaxLen,strLen;

    bMaxLen = pPattern->bMaxLen;
    if(pPattern->bTag == 'p')
        bMaxLen += 1;
	else if (pPattern->bTag == 's' || pPattern->bTag == 'm')
    {
        strLen =  ComUniStrLen(pCtrl->pString);
        if(strLen < bMaxLen)
            strLen = strLen + 1;
    }
    
    Cursor = (pCtrl->bCursor + Step + bMaxLen)%bMaxLen;
    if( !( pPattern->bTag == 's' && pPattern->bTag == 'p') )
    {
        if(pPattern->wCursorMap & (1 << (pPattern->bMaxLen - Cursor - 1)))
            Cursor += Step;
    }

    return Cursor;

}

static BOOL check_number_value(PEDIT_FIELD pCtrl,PEDIT_PATTERN pPattern)
{
    UINT32 mode,status,dval; 
    UINT32 value,valbak,min,max;
    UINT16 wsText[C_MAX_EDIT_LENGTH];
    
    mode   = OSD_GetEditFieldMode(pCtrl);
    status = OSD_GetEditFieldStatus(pCtrl);
	dval = OSD_GetEditFieldDefaultVal(pCtrl);

    if(pPattern->bTag == 'r' 
        && pCtrl->bCursorMode == CURSOR_SPECIAL)
    {
        value = ComUniStr2Int(pCtrl->pString);
        min = pPattern->r.nMin;
        max = pPattern->r.nMax;
        valbak = pCtrl->valbak;
        if(value < min || value > max)
        {
        	if(dval==FORCE_TO_VALBAK)
			value = valbak;
		else
		{
	            if(valbak>= min && valbak <= max)
	                value = valbak;
	            else 
	                value = (value < min)? min : max;
		}
            ConvertValue2String(pCtrl,wsText,value,pPattern);
            NotifyChangeEvent(pCtrl,wsText,pPattern);
            return TRUE;
        }        
    }

    return FALSE;
}

void check_number_edit_mode(PEDIT_FIELD pCtrl,PEDIT_PATTERN pPattern)
{
    UINT32 mode,status; 
    UINT8 len;
    
    mode   = OSD_GetEditFieldMode(pCtrl);
    status = OSD_GetEditFieldStatus(pCtrl);

    if(pPattern->bTag == 'r' 
        && status==EDIT_STATUS 
        && pCtrl->bCursorMode == CURSOR_SPECIAL)
    {
        len = ComUniStrLen(pCtrl->pString);
        if(len >=  pPattern->bMaxLen)
        {
            pCtrl->bStyle &= ~EDIT_STATUS;
            check_number_value(pCtrl, pPattern);
        }        
    }    
}

static void GetEditText(PEDIT_FIELD pCtrl, UINT16* pBuf, int nSize, PEDIT_PATTERN Pattern)
{
	int i;
    UINT16 wstext[C_MAX_EDIT_LENGTH];
    UINT8 len;
    
    switch(Pattern->bTag)
    {
    case 'p':   // password
    	ComUniStrMemSet(pBuf,Pattern->p.bMask,pCtrl->bCursor);
    	ComUniStrMemSet(&pBuf[pCtrl->bCursor],Pattern->p.bInit,Pattern->bLen - pCtrl->bCursor);
        ComUniStrMemSet(&pBuf[Pattern->bLen],' ',1);
        break;
		case 'm':
			len = ComUniStrLen(pCtrl->pString);
			if(pCtrl->bCursor > len)
			{
				pCtrl->bCursor = len - 1;
				ASSERT(0);
			}
			ComUniStrMemSet(pBuf,Pattern->m.bMask,len);
			if(key_input_changed)
				pBuf[pCtrl->bCursor] = pCtrl->pString[pCtrl->bCursor];
			ComUniStrMemSet(&pBuf[len],' ',1);
			break;
    default:
        ComUniStrCopyEx(pBuf, pCtrl->pString, nSize-1);
        break;
    }
}

static UINT32 CheckRangeValue(INT32 Value, INT32 nMin, INT32 nMax, BOOL fLoop)
{
    if(Value < nMin)
        Value = fLoop ? nMax : nMin;
    else if(Value > nMax)
        Value = fLoop ? nMin : nMax;
    return Value;
}

static BOOL GetValue(PEDIT_FIELD pCtrl, UINT32 *pValue)
{
    EDIT_PATTERN Pattern;
    UINT32 value,dval;

    GetPattern(pCtrl, &Pattern);
	if('p' == Pattern.bTag || 's' == Pattern.bTag || 'm' == Pattern.bTag)
        return FALSE;
	if('i' == Pattern.bTag)
		*pValue = ConvertIP2int(pCtrl->pString);
	else		
    *pValue = ComUniStr2Int(pCtrl->pString);
    if('r' == Pattern.bTag)
    {
CHECK_RANGE:    
        value = CheckRangeValue(*pValue, Pattern.r.nMin, Pattern.r.nMax, FALSE);
        if(value != *pValue)
        {
            value = pCtrl->valbak;
            *pValue = value;
            dval = OSD_GetEditFieldDefaultVal(pCtrl);
            if(dval!=FORCE_TO_VALBAK)
                goto CHECK_RANGE;
        }
    }
    return TRUE;
}

static BOOL ConvertIP2int2(UINT16 *pString, UINT32 *pValue)
{
	//strings: "192.168.9.1"
	char asc_str[32];
	UINT32 i, j, data,temp;
	UINT32 pos[4];
	
	ComUniStrToAsc((UINT8*)pString, asc_str);
	data = 0;
	for(i = 0, j = 0; i < STRLEN(asc_str); i++)
	{
		if(!IS_NUMBER_CHAR(asc_str[i]))
			pos[j++] = i; 
	}

	ASSERT(j == 3);
	for(i = 0, j = 0, data = 0; i < 4; i++)
	{
		temp = ATOI(&asc_str[j]);
		if(temp > 0xff)
		{
			*pValue = temp;
			return FALSE;
		}
		data += (temp<<((3-i)*8));
		j = pos[i]+1;
	}

	//libc_printf("asc_str = %s, data = %x\n", asc_str, data);
	*pValue = data;
	return TRUE;
}
static UINT32 ConvertIP2int(UINT16 *pString)
{
	UINT32 value;
	ConvertIP2int2(pString, &value);
	return value;
}

static UINT32 AutoCorrectIP(UINT16 *pString, UINT16 pos, UINT8 num)
{
	UINT32 value;
	UINT32 offset = pos%4;
	
	if(!ConvertIP2int2(pString, &value))
	{
		if(offset == 0)
		{
			if((value/100) > 3)
			{
				// error, do nothing.
				return FALSE;
			}
			else
			{
				SetUniStrCharAt(pString,'0',pos+1);
				SetUniStrCharAt(pString,'0',pos+2);
			}
		}
		else if(offset == 1)
		{
			UINT32 tens = (value%100)/10;
			if(tens > 5)
			{
				// error, do nothing.
				return FALSE;
			}
			else
			{
				SetUniStrCharAt(pString,'0',pos+1);
			}
		}
		return TRUE;
	}
	return FALSE;
}
static void ConvertValue2String(PEDIT_FIELD pCtrl,UINT16* pString, UINT32 dwValue, PEDIT_PATTERN pPattern)
{
    UINT32 i, j, len, cnt;
    UINT32 dwTemp = 1;
    UINT16 wSep;

#if EDITFIELD_SUPPORT_TIME
    if('t' == pPattern->bTag)
    {
        UINT16 wsNum[8], wsSep[2];// = {0};
        char *pfmt = m_fmtDT[pPattern->dt.bFormat];

        for(i=0; i<pPattern->bLen; i++)
            dwTemp *= 10;
        pString[0] = 0;
        wSep = (UINT16)(m_Sep[pPattern->dt.bSep]);
        for(i=0, len=0; i<STRLEN(pfmt); i+=2)
        {
            cnt = pfmt[i+1] - '0';
            for(j=0; j<cnt; j++)
                dwTemp /= 10;
            len += ComInt2UniStr(&pString[len], dwValue/dwTemp, cnt);
            if(dwTemp > 1)
                SetUniStrCharAt(&pString[len++], wSep,0);
            dwValue %= dwTemp;
        }
        pString[len++] = 0;
    }
    else
#endif
	if(pPattern->bTag == 'i')
	{
		wSep = (UINT16)(m_Sep[pPattern->ip.bSep]);
		for(i = 0,len = 0; i < 4; i++)
		{
			len += ComInt2UniStr(&pString[len], (UINT8)(dwValue>>((3-i)*8)), 3);
			if(i < 3)
				SetUniStrCharAt(&pString[len++], wSep,0);
		}
		pString[len++] = 0;
	}
	else
    {
        UINT32 dwQuotient, dwRemainder;
        UINT8 bSub = 0;
        UINT32 len;

        if('f' == pPattern->bTag)
            bSub = pPattern->f.bSub;
        for(i=0; i<bSub; i++)
            dwTemp *= 10;
        dwQuotient = dwValue/dwTemp;
        dwRemainder = dwValue%dwTemp;
        
        len = pPattern->bLen-bSub;
        if(pPattern->bTag == 'r')
        {
            if(pCtrl->bCursorMode == CURSOR_NO
                || pCtrl->bCursorMode == CURSOR_SPECIAL)
                len = 0;        
        }
        ComInt2UniStr(pString, dwQuotient, len);
        if(bSub)
        {
            len = ComUniStrLen(pString);
            SetUniStrCharAt(pString, '.',len);
            len++;
            ComInt2UniStr(&(pString[len]), dwRemainder, bSub);
        }
    }
}

static PRESULT NotifyChangeEvent(PEDIT_FIELD pCtrl, UINT16* pText, PEDIT_PATTERN pPattern)
{
    PRESULT Result;

	if('p' == pPattern->bTag || 's' == pPattern->bTag || 'm' == pPattern->bTag)
    {
        Result = OSD_SIGNAL((POBJECT_HEAD)pCtrl, EVN_PRE_CHANGE, (UINT32)pText,0);
		if(Result == PROC_PASS)
		{
            ComUniStrCopy(pCtrl->pString, pText);
        	Result = OSD_SIGNAL((POBJECT_HEAD)pCtrl, EVN_POST_CHANGE, (UINT32)pText, 0);
        }
    }
    else
    {
		UINT32 dwValue;

		if('i' == pPattern->bTag)
		{
			if(!ConvertIP2int2(pText, &dwValue))
			{
				Result = OSD_SIGNAL((POBJECT_HEAD)pCtrl, EVN_DATA_INVALID, (UINT32)&dwValue, ConvertIP2int(pCtrl->pString));
				return Result;
			}
		}
		else
		{
			dwValue = ComUniStr2Int(pText);
		}
        Result = OSD_SIGNAL((POBJECT_HEAD)pCtrl, EVN_PRE_CHANGE, (UINT32)&dwValue, 0);
        if(Result == PROC_PASS)
        {
            ConvertValue2String(pCtrl,pCtrl->pString, dwValue, pPattern);
        	Result = OSD_SIGNAL((POBJECT_HEAD)pCtrl, EVN_POST_CHANGE, dwValue, 0);
        }
        check_number_edit_mode(pCtrl, pPattern);		
	}

    return Result;
}

static void MoveEditCursor(PEDIT_FIELD pCtrl, INT8 Step, PEDIT_PATTERN pPattern)
{
	PRESULT Result; 
    INT8 Cursor;

    Cursor = get_next_cursor_pos(pCtrl,pPattern,Step);
    
    Result = OSD_SIGNAL((POBJECT_HEAD)pCtrl, EVN_CURSOR_PRE_CHANGE, (UINT32)&Cursor, 0);
    if( Result == PROC_PASS)
    {
    	pCtrl->bCursor = Cursor;
    	OSD_SIGNAL((POBJECT_HEAD)pCtrl, EVN_CURSOR_POST_CHANGE, Cursor, 0);
    	OSD_TrackObject((POBJECT_HEAD)pCtrl, C_UPDATE_ALL/*C_MOVE_CURSOR*/);
	}
}

static PRESULT ChangeEditValue(PEDIT_FIELD pCtrl, INT8 Num, PEDIT_PATTERN pPattern, BOOL fMoveCursor)
{
	PRESULT Result = PROC_LOOP;
	UINT16 wsText[C_MAX_EDIT_LENGTH];
	UINT strLen;
	UINT32 dwValue;

    strLen = ComUniStrLen(pCtrl->pString);

    if(pPattern->bTag == 'p'
        && pCtrl->bCursor == pPattern->bMaxLen)
        pCtrl->bCursor = 0;
    
    if(pPattern->bTag != 'p' )    //make more key can be password    
       Num = (Num + 10)%10;

    ComUniStrCopy(wsText, pCtrl->pString);

	if(pPattern->bTag == 'r' && fMoveCursor && pCtrl->bCursor && pCtrl->bCursorMode == CURSOR_SPECIAL)
	{
		dwValue = ComUniStr2Int(wsText);
		if(dwValue == 0)
			pCtrl->bCursor = 0;
	}
	
    SetUniStrCharAt(wsText,(Num + '0'),pCtrl->bCursor);
    if(pPattern->bTag == 'p')
    {
        wsText[pCtrl->bCursor + 1] = 0;
        fMoveCursor = TRUE;
    }
	else if(pPattern->bTag == 's' || pPattern->bTag == 'm')
    {
        if(pCtrl->bCursor == strLen)
        {
            if(strLen < pCtrl->bMaxLen)
                wsText[pCtrl->bCursor + 1] = 0;
        }
    }
    else if(pPattern->bTag == 'r')
    {
        if(pCtrl->bCursorMode != CURSOR_NORMAL)
            wsText[pCtrl->bCursor + 1] = 0;
	}
	else if(pPattern->bTag == 'i')
	{
		AutoCorrectIP(wsText, pCtrl->bCursor, Num+'0');
    }

    Result = NotifyChangeEvent(pCtrl, wsText, pPattern);
    if(PROC_PASS == Result)
    {
	if((pPattern->bTag == 'p') && ((pCtrl->bCursor+1)>pPattern->bMaxLen))
	{
		pCtrl->bCursor = 0;
		fMoveCursor = FALSE;
	}

        OSD_TrackObject((POBJECT_HEAD)pCtrl, C_UPDATE_ALL);
        if(fMoveCursor)
            MoveEditCursor(pCtrl, 1, pPattern);
    }
	return Result;
}

static void StepEditValue(PEDIT_FIELD pCtrl, INT8 Num, PEDIT_PATTERN pPattern)
{
    UINT32 Value,newValue;
    
	if('i' == pPattern->bTag)
		Value = ConvertIP2int(pCtrl->pString);
	else
    Value = ComUniStr2Int(pCtrl->pString);
    newValue = Value + Num;
    
    newValue = CheckRangeValue(newValue, pPattern->r.nMin, pPattern->r.nMax, TRUE);
    OSD_SIGNAL((POBJECT_HEAD)pCtrl, EVN_PRE_CHANGE,(UINT32)&newValue,Value);
    ConvertValue2String(pCtrl,pCtrl->pString,newValue,pPattern);
    OSD_TrackObject((POBJECT_HEAD)pCtrl, C_UPDATE_ALL);
    OSD_SIGNAL((POBJECT_HEAD)pCtrl, EVN_POST_CHANGE, newValue, Value);
}

static void DevideEditValue(PEDIT_FIELD pCtrl, PEDIT_PATTERN pPattern)
{
    UINT32 Value,newValue;
    
	if('i' == pPattern->bTag)
		Value = ConvertIP2int(pCtrl->pString);
	else
    Value = ComUniStr2Int(pCtrl->pString);
    newValue = Value / 10;
    if(Value == newValue)
        return;
    OSD_SIGNAL((POBJECT_HEAD)pCtrl, EVN_PRE_CHANGE,(UINT32)&newValue,Value);

    ConvertValue2String(pCtrl,pCtrl->pString,newValue,pPattern);
    if(pCtrl->bCursor > 0)
		pCtrl->bCursor -= 1;
    
    OSD_SIGNAL((POBJECT_HEAD)pCtrl, EVN_POST_CHANGE, newValue, Value);
    OSD_TrackObject((POBJECT_HEAD)pCtrl, C_UPDATE_ALL);
}

static BOOL BackupRestoreValue(PEDIT_FIELD pCtrl,PEDIT_PATTERN pPattern,BOOL bBackup)
{
    if(pPattern->bTag != 'r' 
        && pPattern->bTag != 'd'  
		&& pPattern->bTag != 'i' 
        && pPattern->bTag != 't'
        && pPattern->bTag != 'f')
        return FALSE;

    if(bBackup)
    {
		if('i' == pPattern->bTag)
			pCtrl->valbak = ConvertIP2int(pCtrl->pString);
		else
        pCtrl->valbak = ComUniStr2Int(pCtrl->pString);
        gui_printf("Backup value : %d \n",pCtrl->valbak);
    }
    else
    {
        ConvertValue2String(pCtrl,pCtrl->pString,pCtrl->valbak,pPattern);
        gui_printf("Restore value : %d \n",pCtrl->valbak);
    }

    return TRUE;
}

BOOL OSD_SetEditFieldContent(PEDIT_FIELD pCtrl, UINT32 StringType, UINT32 dwValue)
{
    UINT32 len;
    UINT8* pStr;
    switch(StringType)
    {
    case STRING_ID:
    	pStr = OSD_GetUnicodeString(dwValue);
    	len = ComUniStrLenExt(pStr);
    	MEMCPY((UINT8*)pCtrl->pString,pStr,len);
        break;
    case STRING_ANSI:
    	ComAscStr2Uni((UINT8*)dwValue,pCtrl->pString);
    	break;
    case STRING_UNICODE:
        ComUniStrCopy(pCtrl->pString, (UINT16*)dwValue);
        break;
    case STRING_NUMBER:
    {
        EDIT_PATTERN Pattern;
        GetPattern(pCtrl, &Pattern);
        ConvertValue2String(pCtrl,pCtrl->pString, dwValue, &Pattern);
        break;
    }
    default:
        return FALSE;
    }
    return TRUE;
}

BOOL OSD_SetEditFieldPrefix(PEDIT_FIELD pCtrl, UINT32 StringType, UINT32 dwValue)
{
    UINT32 len;
    UINT8* pStr;
    switch(StringType)
    {
    case STRING_ID:
    	pStr = OSD_GetUnicodeString(dwValue);
    	len = ComUniStrLenExt(pStr);
    	MEMCPY((UINT8*)pCtrl->pString,pStr,len);
        break;
    case STRING_ANSI:
    	ComAscStr2Uni((UINT8*)dwValue,pCtrl->pPrefix);
    	break;        	
    case STRING_UNICODE:
        ComUniStrCopy(pCtrl->pPrefix, (UINT16*)dwValue);
        break;
    default:
        return FALSE;
    }
    return TRUE;
}

BOOL OSD_SetEditFieldSuffix(PEDIT_FIELD pCtrl, UINT32 StringType, UINT32 dwValue)
{
    UINT32 len;
    UINT8* pStr;
    switch(StringType)
    {
    case STRING_ID:
    	pStr = OSD_GetUnicodeString(dwValue);
    	len = ComUniStrLenExt(pStr);
    	MEMCPY((UINT8*)pCtrl->pString,pStr,len);
        break;
    case STRING_ANSI:
    	ComAscStr2Uni((UINT8*)dwValue,pCtrl->pSuffix);
    	break;
    case STRING_UNICODE:
        ComUniStrCopy(pCtrl->pSuffix, (UINT16*)dwValue);
        break;
    default:
        return FALSE;
    }
    
    return TRUE;
}

UINT32 OSD_GetEditFieldContent(PEDIT_FIELD pCtrl)
{
    EDIT_PATTERN Pattern;
    UINT32 dwValue;

    GetPattern(pCtrl, &Pattern);
	if('p' == Pattern.bTag || 's' == Pattern.bTag || 'm' == Pattern.bTag)
        return (UINT32)pCtrl->pString;
	else if('i' == Pattern.bTag)
	{
		dwValue = ConvertIP2int(pCtrl->pString);
		return dwValue;
	}
    else
    {
        dwValue = ComUniStr2Int(pCtrl->pString);
        return dwValue;
    }
}


BOOL OSD_GetEditFieldIntValue(PEDIT_FIELD pCtrl,UINT32 *pValue)
{
	EDIT_PATTERN Pattern;

    GetPattern(pCtrl, &Pattern);
    if('r' == Pattern.bTag \
        || 'f' == Pattern.bTag \
        || 't' == Pattern.bTag)
    	*pValue = ComUniStr2Int(pCtrl->pString);
	else if('i' == Pattern.bTag)
	{
		*pValue = ConvertIP2int(pCtrl->pString);
		//*pValue = ComUniStr2Int(pCtrl->pString);
	}
	else
		return FALSE;
					
	return TRUE;
}


BOOL OSD_GetEditFieldTimeDate(PEDIT_FIELD pCtrl,date_time* dt)
{
	char f,ch,*pfmt;
	UINT16* pString;
	UINT32 i, j, val,pos, cnt;
	EDIT_PATTERN Pattern;
	
	
	GetPattern(pCtrl, &Pattern);
	if('t' != Pattern.bTag)
		return FALSE;

	pfmt = m_fmtDT[Pattern.dt.bFormat];
	pString = pCtrl->pString;
	pos = 0;
	for(i=0; i<STRLEN(pfmt); i+=2)
    {
    	f 	= pfmt[i];
        cnt	= (UINT32)(pfmt[i+1] - '0');
        if(!ComUniStr2IntExt(&pString[pos],cnt,&val))
        	return FALSE;
        	
        switch(f)
        {
        case 'Y':
			dt->year = (UINT16)val;
        	break;
		case 'M':			
			dt->month = (UINT8)val;				
			break;
		case 'D':
			dt->day = (UINT8)val;				        	
			break;
		case 'h':
			dt->hour = (UINT8)val;
			break;
		case 'm':
			dt->min = (UINT8)val;
			break;
		case 's':
			dt->sec = (UINT8)val;
			break;
		default:
			return FALSE;
        }
        pos += cnt + 1;       
    }
    
    return TRUE;
}

void OSD_DrawEditFieldCell(PEDIT_FIELD pCtrl, UINT8 bStyleIdx, UINT32 nCmdDraw)
{
	UINT8	edffont,pos,bAssignType,bCursor,*pStr;
	UINT16	wsText[C_MAX_EDIT_LENGTH*2];
	UINT16	wYpos,wXpos,totalW,partW,chH, chW,wMaxHeight;
	UINT32 	hAligin,vAlign,edfstyle,draw_type,shiftChars,cursorMode;
	OBJECTINFO	RscLibInfo;
	EDIT_PATTERN Pattern;
	GUI_RECT	r;
	GUI_PEN	pen_dot;
	PGUI_VSCR	pVscr;
	PGUI_RECT	objframe;
	PGUI_REGION	prgn;
	PWINSTYLE	lpWinSty;

	if(OSD_GetUpdateType(nCmdDraw) == C_UPDATE_CONTENT)
		edfstyle = (UINT32)(~0);
	else
		edfstyle = bStyleIdx;
		
	pos = 0;
	bAssignType = pCtrl->bAlign;
	hAligin = GET_HALIGN(bAssignType);
	vAlign = GET_VALIGN(bAssignType);
	objframe	= &pCtrl->head.frame;
	lpWinSty 	= g_gui_rscfunc.osd_get_win_style(bStyleIdx);
	
	if(OSD_GetAttrSlave(pCtrl))
	{
		edffont = (pCtrl->head.bFont)|C_FONT_SLVMASK;
		pVscr = OSD_DrawSlaveFrame(objframe,bStyleIdx);
	}
	else
	{
		edffont = (pCtrl->head.bFont);
		pVscr = OSD_DrawObjectFrame(objframe,edfstyle);
	}
	
	GetPattern(pCtrl, &Pattern);
	draw_type = OSD_GetDrawType(nCmdDraw);	
	if(draw_type==C_DRAW_TYPE_HIGHLIGHT)
		cursorMode = get_focus_cursor_type(pCtrl,&Pattern);
	else
		cursorMode = CURSOR_NO;
	bCursor = pCtrl->bCursor + 1;

	wsText[0] = 0;
	if(pCtrl->pPrefix != NULL)
	{
		ComUniStrCopy(wsText,pCtrl->pPrefix);
		pos = ComUniStrLen(wsText);
	}		
	GetEditText(pCtrl, &wsText[pos], ARRAY_SIZE(wsText) - pos, &Pattern);	
	if(cursorMode==CURSOR_SPECIAL)
	{
	    pos = ComUniStrLen(wsText);
		SetUniStrCharAt(wsText,'_',pos);
		wsText[pos + 1] = 0;
	}
	
	if(pCtrl->pSuffix != NULL)
	{
		pos = ComUniStrLen(wsText);
		ComUniStrCopy(&wsText[pos],pCtrl->pSuffix);
	}
	pStr = (UINT8*)wsText;
	r = *objframe;
	r.uStartX += pCtrl->bX;
	r.uStartY	+= pCtrl->bY;
	r.uWidth -= pCtrl->bX<<1;
	r.uHeight -= pCtrl->bY<<1;	

	shiftChars = 0;
	if((Pattern.bTag == 's' || Pattern.bTag == 'm')&& cursorMode == CURSOR_NORMAL)
	{
		totalW = OSD_MultiFontLibStrMaxHW(pStr,edffont,&wMaxHeight,&chW,bCursor);
		while(r.uWidth < totalW && shiftChars < pCtrl->bCursor)
		{
			 shiftChars ++;
			 totalW = OSD_MultiFontLibStrMaxHW(pStr + shiftChars*2,edffont,&wMaxHeight,&chW,bCursor - shiftChars);			 
		}
	}

	pStr = (UINT8*)(wsText + shiftChars);
	OSD_DrawText(&r,pStr,lpWinSty->wFgIdx,pCtrl->bAlign,edffont,pVscr);
	
	if(cursorMode == CURSOR_NORMAL && pStr!=NULL)
	{		
		totalW = OSD_MultiFontLibStrMaxHW(pStr,edffont,&wMaxHeight,&chW,0);
		
		pos = (pCtrl->pPrefix != NULL)? ComUniStrLen(pCtrl->pPrefix) : 0;
		partW = OSD_MultiFontLibStrMaxHW(pStr,edffont,&chH,&chW,bCursor +pos - shiftChars);

		wYpos = r.uStartY;
		wXpos = r.uStartX;
		if (r.uHeight > wMaxHeight)
		{
			if (vAlign == C_ALIGN_VCENTER)
				wYpos += (r.uHeight - wMaxHeight)>>1;
			else if (vAlign == C_ALIGN_BOTTOM)
				wYpos += r.uHeight - wMaxHeight;
		}

		{
			if (hAligin == C_ALIGN_CENTER)
			{
				if (totalW<r.uWidth)
					wXpos += (r.uWidth-totalW)>>1;				
			}	
			else if(hAligin == C_ALIGN_RIGHT)
			{
				if (totalW < r.uWidth)
					wXpos += r.uWidth-totalW-4;
			}
		}						
		if(ComUniStrLen(pStr) == 0)
		{
			chW = 10;
			chH = 12;
		}
		else if( (UINT8)(bCursor - shiftChars)> ComUniStrLen(pStr) )
		{
			wXpos += partW;
			chW = 10;
		}
		else
			wXpos += partW - chW;
		wYpos += chH - 2;

		if(1)
		{
			GUI_POINT strt,end;
			
			strt.uX = wXpos - 1;
			end.uX = wXpos - 1 + chW + 2;
			strt.uY = end.uY = wYpos;
			MEMSET(&pen_dot,0,sizeof(GUI_PEN));
			pen_dot.thick = 2;
			pen_dot.color = lpWinSty->wFgIdx;
			prgn = gelib_getdstrgn(pVscr->root_layer,pVscr->root_rgn);
			gelib_drawline(prgn,&strt,&end,(UINT32)&pen_dot);
			strt.uX = wXpos - 1;
			end.uX = wXpos - 1 + chW + 2;
			strt.uY = end.uY =	wYpos+1;
			gelib_drawline(prgn,&strt,&end,(UINT32)&pen_dot);
		}
	}
}

/*SELECT_EDIT_MODE map*/
VACTION OSD_EditFieldKeyMap1(POBJECT_HEAD pObj, UINT32 Key)
{
	VACTION Action = VACT_PASS;
	PEDIT_FIELD pCtrl = (PEDIT_FIELD)pObj;
	UINT8 bStyle = pCtrl->bStyle;

	if(bStyle&EDIT_STATUS)
    	{
		switch(Key)
		{
			case V_KEY_UP:
				Action = VACT_EDIT_INCREASE_;
				break;
			case V_KEY_DOWN:
				Action = VACT_EDIT_DECREASE_;
				break;
			case V_KEY_LEFT:
				Action = VACT_EDIT_LEFT;
				break;
			case V_KEY_RIGHT:
				Action = VACT_EDIT_RIGHT;
				break;
			case V_KEY_0:	case V_KEY_1:	case V_KEY_2:	case V_KEY_3:
			case V_KEY_4:	case V_KEY_5:	case V_KEY_6:	case V_KEY_7:
			case V_KEY_8:	case V_KEY_9:
				Action = Key - V_KEY_0 + VACT_NUM_0;
				break;
			case V_KEY_ENTER:
				Action = VACT_EDIT_ENTER;
				break;
			case V_KEY_MENU:
			case V_KEY_EXIT:
				Action = VACT_EDIT_CANCEL_EXIT;
				break;
			default:
				break;
		}
	}
	else // SELECT_STATUS
	{
		switch(Key)
		{
			case V_KEY_ENTER:
				break;
			case V_KEY_LEFT:
				Action = VACT_EDIT_LEFT;
				break;
			case V_KEY_RIGHT:
				Action = VACT_EDIT_RIGHT;
				break;
			default:
				break;
		}
	}
	
    return Action;
}

/* NORMAL_EDIT_MODE map*/
VACTION OSD_EditFieldKeyMap2(POBJECT_HEAD pObj, UINT32 Key)
{
	VACTION Action = VACT_PASS;
	PEDIT_FIELD pCtrl = (PEDIT_FIELD)pObj;
	UINT8 bStyle = pCtrl->bStyle;

	if(bStyle&EDIT_STATUS)
	{
		switch(Key)
		{
			case V_KEY_0:	case V_KEY_1:	case V_KEY_2:	case V_KEY_3:
			case V_KEY_4:	case V_KEY_5:	case V_KEY_6:	case V_KEY_7:
			case V_KEY_8:	case V_KEY_9:
				Action = Key - V_KEY_0 + VACT_NUM_0;
				break;
			case V_KEY_ENTER:
				Action = VACT_ENTER;
				break;
			case V_KEY_MENU:
			case V_KEY_EXIT:
				Action = VACT_EDIT_CANCEL_EXIT;
				break;
			case V_KEY_LEFT:
				Action = VACT_EDIT_LEFT;
				break;				
			default:
				break;
		}
	}
	else // SELECT_STATUS
	{
		switch(Key)
		{
			case V_KEY_LEFT:
				Action = VACT_EDIT_LEFT;
				break;
			case V_KEY_RIGHT:
				Action = VACT_EDIT_RIGHT;
				break;
			case V_KEY_0:	case V_KEY_1:	case V_KEY_2:	case V_KEY_3:
			case V_KEY_4:	case V_KEY_5:	case V_KEY_6:	case V_KEY_7:
			case V_KEY_8:	case V_KEY_9:
				Action = Key - V_KEY_0 + VACT_NUM_0;
				break;
			default:
				break;
		}
	}

	return Action;	
}

VACTION OSD_EditFieldKeyMap(POBJECT_HEAD pObj, UINT32 Key)
{
	VACTION Action = VACT_PASS;
	PEDIT_FIELD pCtrl = (PEDIT_FIELD)pObj;
	UINT8 bStyle = pCtrl->bStyle;

	if(bStyle&SELECT_EDIT_MODE)
		Action = OSD_EditFieldKeyMap1(pObj, Key);
	else if(bStyle&NORMAL_EDIT_MODE)
		Action = OSD_EditFieldKeyMap2(pObj, Key);

	return Action;
}

PRESULT OSD_EditFieldProc(POBJECT_HEAD pObj, UINT32 msg_type, UINT32 msg,UINT32 param1)
{
	PRESULT Result = PROC_LOOP;
	PEDIT_FIELD pCtrl = (PEDIT_FIELD)pObj;
	INT8 Num, Cursor, Step;
	EDIT_PATTERN Pattern;
	UINT32 dwValue, EvnParam;   
	UINT32 mode,status; 
	
	mode   = OSD_GetEditFieldMode(pCtrl);
	status = OSD_GetEditFieldStatus(pCtrl);

	if(msg_type == MSG_TYPE_KEY)
	{
		VACTION Action;
		UINT32 vkey;
		BOOL bContinue;    
    	
        key_input_changed = TRUE;
		Result = OSD_ObjCommonProc(pObj,msg,param1,&vkey,&Action,&bContinue);
        key_input_changed = FALSE;
	   	if(!bContinue)
	    		goto CHECK_LEAVE;
 
		Result = PROC_LOOP;
		GetPattern(pCtrl, &Pattern);	    	   
		switch(Action)
		{
        case VACT_EDIT_ENTER:   /* Enter Edit mode */            
			if(mode == SELECT_EDIT_MODE
				&& status == SELECT_STATUS)
			{	
				pCtrl->bStyle |= EDIT_STATUS;
				gui_printf("VACT_INPUT : Change from SELECT_STATUS->EDIT_STATUS\n");
				pCtrl->bCursor = 0;
				BackupRestoreValue(pCtrl,&Pattern,TRUE);
				OSD_TrackObject(pObj, C_UPDATE_ALL);
			}
			else
				Result = PROC_PASS;
            break;

        case VACT_EDIT_CANCEL_EXIT: /* Exit Edit mode without save */
            if(status==EDIT_STATUS
				&& (mode==SELECT_EDIT_MODE \
				    || (Pattern.bTag == 'r' && pCtrl->bCursorMode == CURSOR_SPECIAL))
			)
			{	
				pCtrl->bStyle &= ~EDIT_STATUS;
                if(BackupRestoreValue(pCtrl,&Pattern,FALSE))
                {
                    UINT16 wsText[C_MAX_EDIT_LENGTH];
                    ComUniStrCopy(wsText, pCtrl->pString);
				    NotifyChangeEvent(pCtrl, wsText, &Pattern);
                }
				OSD_TrackObject(pObj, C_UPDATE_ALL);
			}
			else
				Result = PROC_PASS;            
            break;

        case VACT_EDIT_SAVE_EXIT:   /* Exit Edit mode with save */            
            if((status==EDIT_STATUS&& (mode==SELECT_EDIT_MODE || Pattern.bTag == 'r'))
                ||(mode==NORMAL_EDIT_MODE)||(Pattern.bTag == 'r'))
			{	
			    gui_printf("VACT_ENTER : Change from EDIT_STATUS->SELECT_STATUS\n");
				if(!GetValue(pCtrl, &dwValue))
					EvnParam = (UINT32)pCtrl->pString;
				else
      				EvnParam = dwValue;
                    
				if(PROC_PASS == OSD_SIGNAL(pObj, EVN_ENTER, EvnParam, 0))
				{
					pCtrl->bStyle &= ~EDIT_STATUS;
					if(!IsEditString(Pattern.bTag))
						ConvertValue2String(pCtrl,pCtrl->pString, dwValue, &Pattern);
					OSD_TrackObject(pObj, C_UPDATE_ALL);
				}
			}
			else
				Result = PROC_PASS;
            break;
        case VACT_EDIT_LEFT:
        case VACT_EDIT_RIGHT:            
            if(get_focus_cursor_type(pCtrl,&Pattern) == CURSOR_NORMAL)
            {   
                if(VACT_EDIT_LEFT == Action)
    				Step = -1;
    			else
    				Step = 1;
                check_cursor_pos(pCtrl, &Pattern);
    			MoveEditCursor(pCtrl, Step, &Pattern);	
                
            }
            else if(check_focus_can_addsub(pCtrl, &Pattern))
            {
                /* Increase / Decrease */
                if(Action == VACT_EDIT_LEFT)
    				Num = -1;
    			else
    				Num = 1;

    			StepEditValue(pCtrl, Num, &Pattern);                
            }
            else if(Pattern.bTag == 'r' \
                && pCtrl->bCursorMode == CURSOR_SPECIAL \
                && status == EDIT_STATUS)
            {
            	if(Action == VACT_EDIT_LEFT)
            	{
	                /* Delete last char for number input */
	                DevideEditValue(pCtrl, &Pattern);
				}
				else
				{
					/* Restore to previous value( exit current input) */
					BackupRestoreValue(pCtrl,&Pattern,FALSE);
					pCtrl->bStyle &= ~EDIT_STATUS;
					OSD_TrackObject(pObj, C_UPDATE_ALL);
				}
            } 
            else
                Result = PROC_PASS;
            break;
		 			
		case VACT_EDIT_INCREASE_:
		case VACT_EDIT_DECREASE_:  // add or minus the current cursor digit					    
			if(get_focus_cursor_type(pCtrl,&Pattern) != CURSOR_NORMAL)
			{
				Result = PROC_PASS;
				break;
			}
			if(mode != EDIT_STATUS)
			{
			    gui_printf(" VACT_EDIT_INCREASE_/VACT_EDIT_DECREASE_: Change from SELECT_STATUS->EDIT_STATUS\n");
				pCtrl->bStyle |= EDIT_STATUS;
				BackupRestoreValue(pCtrl,&Pattern,TRUE);
			}

            check_cursor_pos(pCtrl, &Pattern);							
			Num = GetUniStrCharAt(pCtrl->pString,pCtrl->bCursor);
			Num -= '0';
			if(Action == VACT_EDIT_INCREASE_)
				Num = (Num + 1)%10;
			else
				Num = (Num - 1 + 10)%10;
			Result = ChangeEditValue(pCtrl, Num, &Pattern, FALSE);
			break;
			
		case VACT_NUM_0: case VACT_NUM_1: case VACT_NUM_2: case VACT_NUM_3: 
		case VACT_NUM_4: case VACT_NUM_5: case VACT_NUM_6: case VACT_NUM_7: 
		case VACT_NUM_8: case VACT_NUM_9: case VACT_EDIT_PASSWORD:
			if(!check_focus_can_input09(pCtrl,&Pattern) )
			{
			    gui_printf("Can't int put 0~9\n");
				Result = PROC_PASS;
				break;
			}
			if(status != EDIT_STATUS)
			{
			    gui_printf(" 0 ~ 9: Change from SELECT_STATUS->EDIT_STATUS\n");
				pCtrl->bStyle |= EDIT_STATUS;

                if(Pattern.bTag == 'r')
                {
                    if(pCtrl->bCursorMode != CURSOR_NORMAL)
                        pCtrl->bCursor = 0;
                }
                BackupRestoreValue(pCtrl,&Pattern,TRUE);
			}

            check_cursor_pos(pCtrl, &Pattern);
            
            if(VACT_EDIT_PASSWORD == Action && Pattern.bTag == 'p')
                Num = vkey - V_KEY_0;//10~ store vkey to make more key can be password except 0~9
            else
                Num = Action - VACT_NUM_0; //0~9
			Result = ChangeEditValue(pCtrl, Num, &Pattern, TRUE);
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
	else// if(msg_type ==MSG_TYPE_EVNT)
	{
		Result = OSD_SIGNAL(pObj, msg_type, msg, param1);
        if(Result==PROC_PASS 
            && (msg_type == EVN_FOCUS_PRE_LOSE
            || msg_type == EVN_PARENT_FOCUS_PRE_LOSE))
        {
            if(status == EDIT_STATUS)
				pCtrl->bStyle &= ~EDIT_STATUS;
            
            GetPattern(pCtrl, &Pattern);                
            check_number_value(pCtrl, &Pattern);
	    }
		
	}

CHECK_LEAVE:
	if(!(EVN_PRE_OPEN == msg_type))
		CHECK_LEAVE_RETURN(Result,pObj);		

EXIT:
	return Result;
}

