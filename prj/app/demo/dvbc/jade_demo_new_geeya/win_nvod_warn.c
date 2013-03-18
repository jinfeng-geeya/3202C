#include <sys_config.h>
#ifdef NVOD_FEATURE
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <api/libpub/lib_pub.h>
#include <api/libpub/lib_hde.h>
#include <hld/dis/vpo.h>
//#include <api/libosd/osd_lib.h>
#include <api/libsi/si_eit.h>
#include <api/libsi/lib_nvod.h>
#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"

#include "win_com.h"
#include "menus_root.h"

#include "win_com_popup.h"
#include "win_com_list.h"
/*******************************************************************************
*	Objects definition
*******************************************************************************/
static VACTION nvod_warn_keymap ( POBJECT_HEAD pObj, UINT32 key );
static PRESULT nvod_warn_callback ( POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2 );
static void nvod_warn_draw_info();
static void nvod_warn_draw_id();
static void nvod_warn_draw(OSD_RECT* pRect,char* str,UINT16* unistr,UINT16 id);
static PRESULT nvod_warn_message_proc(UINT32 msg_type, UINT32 msg_code);
UINT32 GetNvodProgID(UINT32 progID);
void SetNvodProgID(UINT32 progID);


CONTAINER g_win_nvod_warn;

TEXT_FIELD nvod_warn_txt;

//extern void nvod_clear_current_timer();
#define NVOD_WARN_W_L (TV_OFFSET_L +(OSD_MAX_WIDTH-NVOD_WARN_W_W)/2)
#define NVOD_WARN_W_T (TV_OFFSET_T+(OSD_MAX_HEIGHT-NVOD_WARN_W_H)/2)
#define NVOD_WARN_W_W 350
#define NVOD_WARN_W_H ((ITEM_H+ITEM_GAP)*4+ITEM_H+10*2+20)//350




#define ITEM_H  26
#define ITEM_GAP    4

#define LDEF_WIN(varCon,nxtObj,l,t,w,h,focusID)		\
    DEF_CONTAINER(varCon,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WSTL_POP_WIN_1,WSTL_POP_WIN_1,WSTL_POP_WIN_1,WSTL_POP_WIN_1,   \
    nvod_warn_keymap,nvod_warn_callback,  \
    nxtObj, focusID,0)

#define LDEF_TXT(root,varTxt,nxtObj,l,t,w,h,resID,str,IDX)		\
    DEF_TEXTFIELD(varTxt,&root,nxtObj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, IDX,IDX,IDX,IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,resID,str)

LDEF_TXT ( g_win_nvod_warn, nvod_warn_txt, NULL, 		NVOD_WARN_W_L+5, \
        NVOD_WARN_W_T+5, NVOD_WARN_W_W-10, NVOD_WARN_W_H-10, RS_EVENT_LEN, display_strs[0] ,WSTL_TXT_WIN_POP)

LDEF_WIN ( g_win_nvod_warn, &nvod_warn_txt, NVOD_WARN_W_L, NVOD_WARN_W_T, NVOD_WARN_W_W, NVOD_WARN_W_H, 1 )

static OSD_RECT nvod_warn_rcs[] = 
{
    {NVOD_WARN_W_L+(NVOD_WARN_W_W-60)/2,  NVOD_WARN_W_T+10,60,  ITEM_H},

    {NVOD_WARN_W_L+10,   NVOD_WARN_W_T+10+(ITEM_H+ITEM_GAP),   80,  ITEM_H},
    {NVOD_WARN_W_L+10+80,NVOD_WARN_W_T+10+(ITEM_H+ITEM_GAP),   150, ITEM_H},

    {NVOD_WARN_W_L+10,   NVOD_WARN_W_T+10+(ITEM_H+ITEM_GAP)*2, 80,  ITEM_H},
    {NVOD_WARN_W_L+10+80,NVOD_WARN_W_T+10+(ITEM_H+ITEM_GAP)*2, 150, ITEM_H},

    {NVOD_WARN_W_L+10,   NVOD_WARN_W_T+10+(ITEM_H+ITEM_GAP)*3, 150,  ITEM_H},

    {NVOD_WARN_W_L+10,   NVOD_WARN_W_T+10+(ITEM_H+ITEM_GAP)*4, 150,  ITEM_H},
};

static UINT16 nvod_warn_ids[] = 
{
    RS_SYSTEM_KEY_AUDIO_WARNING,
    RS_NVOD_CONFIRM,
    RS_NVOD_CANCEL_NVOD,
    RS_NVOD_CANCEL,
    RS_COMMON_CANCEL,
    RS_EVENT_LEN,
    RS_TIME_PASSED
};

static UINT32 nvod_timer_ProgID = 0xFFFFFFFF;

void SetNvodProgID(UINT32 progID)
{

    nvod_timer_ProgID = progID;
}

UINT32 GetNvodProgID(UINT32 progID)
{

    return nvod_timer_ProgID;
}

static BOOL backfromNVOD=FALSE;
BOOL getBackfromNVOD()
{
    return backfromNVOD;
}
void setBackfromNVOD(BOOL flag)
{
    backfromNVOD=flag;
}
static VACTION nvod_warn_keymap ( POBJECT_HEAD pObj, UINT32 key )
{
	VACTION act = VACT_PASS;
    POBJECT_HEAD menu = NULL;

	switch ( key )
	{
        case V_KEY_ENTER:
            menu_stack_pop();
            if(GetNvodFromTimerFlag())
            {
                menu_stack_pop_all();
				//nvod_clear_current_timer();
            }
			

            menu = (POBJECT_HEAD)&g_win_nvod;
            setBackfromNVOD(TRUE);
        	if (OSD_ObjOpen(menu, 0xFFFFFFFF) != PROC_LEAVE)
        	{
        		menu_stack_push(menu);            
        	}
            nvod_timer_ProgID = 0xFFFF;
            
            break;
		case V_KEY_EXIT:
			act = VACT_CLOSE;
            break;
		default:
			act = VACT_PASS;
	}

	return act;

}

static PRESULT nvod_warn_callback ( POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2 )
{
	PRESULT ret = PROC_PASS;
	VACTION unact;

	switch ( event )
	{
		case EVN_PRE_OPEN:
            //enterNvodWin = FALSE;
            MEMSET(display_strs[0],0,sizeof(display_strs[0]));
			break;
		case EVN_POST_OPEN:
            nvod_warn_draw_id();
            nvod_warn_draw_info();
			break;
		case EVN_UNKNOWNKEY_GOT:
			break;
		case EVN_UNKNOWN_ACTION:
			unact = ( VACTION ) ( param1 >> 16 );
			break;
		case EVN_MSG_GOT:
			ret= nvod_warn_message_proc(param1, param2);
			break;
		case EVN_PRE_CLOSE:
			break;
		case EVN_POST_CLOSE:
			break;
	}

	return ret;
}

static void nvod_warn_draw_id()
{
    UINT8 i = 0;

    for(i = 0;i<7;i++)
    {
        nvod_warn_draw(&nvod_warn_rcs[i],NULL,NULL,nvod_warn_ids[i]);
    }
}


void GetCurEvent(struct nvod_event* event);
extern void GetRefEventStartEndTime(struct nvod_event* event,date_time *start_dt,date_time *end_dt,date_time* duration_dt);
extern void CovertByOffset(date_time *dt);
extern BOOL rise_from_standby;

static void nvod_warn_draw_info()
{
    UINT8 i = 0;
    OSD_RECT info_rc;
    char event_len[8]={0},event_passed_time[8]={0};
    UINT8* id_str = NULL;
    UINT32 str_len = 0;
    UINT16 uni_str[20];
    struct nvod_event curEvent;
    date_time localTime,start_time,duration;
    char strTemp[5];
    INT32 day,h,m,sec;
    P_NODE pNode;
    T_NODE tNode;
	INT32 time_len = 0;

	
	if(rise_from_standby)
		return;
    info_rc.uTop = nvod_warn_rcs[5].uTop;
    info_rc.uLeft = nvod_warn_rcs[2].uLeft;
    info_rc.uWidth = nvod_warn_rcs[2].uWidth;
    info_rc.uHeight= nvod_warn_rcs[2].uHeight;

    GetCurEvent(&curEvent);
    
    get_local_time(&localTime);
    GetRefEventStartEndTime(&curEvent,&start_time,NULL,&duration);
    CovertByOffset(&start_time);
    get_time_offset(&start_time,&localTime,&day,&h,&m,&sec);
    
    sprintf(event_len,"%d",60*duration.hour+duration.min);
	time_len = 60*24*day+60*h+m;
	if(time_len<=0)
		time_len = 0;
    sprintf(event_passed_time,"%d",time_len);
    
    MEMSET(uni_str,0,sizeof(uni_str));
    ComAscStr2Uni(event_len,uni_str);
    str_len = ComUniStrLen(uni_str);
    id_str = OSD_GetUnicodeString(RS_SYSTME_AUTO_SHUT_DOWN_MINUTE);
    ComUniStrCopyChar((UINT8*)&uni_str[str_len], id_str);
    nvod_warn_draw(&info_rc,NULL,uni_str,0);

    info_rc.uTop = nvod_warn_rcs[6].uTop;
    MEMSET(uni_str,0,sizeof(uni_str));
    ComAscStr2Uni(event_passed_time,uni_str);
    str_len = ComUniStrLen(uni_str);
    id_str = OSD_GetUnicodeString(RS_SYSTME_AUTO_SHUT_DOWN_MINUTE);
    ComUniStrCopyChar((UINT8*)&uni_str[str_len], id_str);
    nvod_warn_draw(&info_rc,NULL,uni_str,0);
}

static void nvod_warn_draw(OSD_RECT* pRect,char* str,UINT16* unistr,UINT16 id)
{
	TEXT_FIELD* txt;

	txt = &nvod_warn_txt;
	OSD_SetRect2(&txt->head.frame,pRect);
    if(str != NULL)
    {
        if(txt->pString == NULL)
            txt->pString = display_strs[0];
		OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)str);
    }
    else if(unistr != NULL)
    {
        if(txt->pString == NULL)
            txt->pString = display_strs[0];
        OSD_SetTextFieldContent(txt, STRING_UNICODE, (UINT32)unistr);
    }
    else
    {
		OSD_SetTextFieldContent(txt, STRING_ID, (UINT32)id);
        txt->pString = NULL;
    }

	OSD_DrawObject( (POBJECT_HEAD)txt, C_UPDATE_ALL);
}



static PRESULT nvod_warn_message_proc(UINT32 msg_type, UINT32 msg_code)
{
	PRESULT ret = PROC_LOOP;
	static date_time this_time,last_time;

	switch (msg_type)
	{
		case CTRL_MSG_SUBTYPE_CMD_TIMEDISPLAYUPDATE:
			MEMSET(&this_time,0,sizeof(date_time));
			get_local_time(&this_time);
			if(is_larger_than_1m(&this_time,&last_time))
			{
				MEMSET(&last_time,0,sizeof(date_time));
				last_time = this_time;
				nvod_warn_draw_info();
			}
			break;
		case CTRL_MSG_SUBTYPE_STATUS_SIGNAL:	
			break;
		default:
			break;
	}
	
	return ret;
	
}
#endif
