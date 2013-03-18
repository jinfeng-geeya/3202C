#include <sys_config.h>
#ifdef NVOD_FEATURE
#include <types.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/decv/vdec_driver.h>
#include <api/libpub/lib_hde.h>
#include <api/libpub/lib_pub.h>
#include <api/libsi/lib_epg.h>

#include <api/libsi/si_eit.h>
//#include <api/libosd/osd_lib.h>
#include <api/libsi/lib_nvod.h>

#include "osdobjs_def.h"
#include "images.id"
#include "string.id"
#include "osd_config.h"

#include "win_com.h"
#include "menus_root.h"

#include "win_com_popup.h"
#include "win_com_list.h"

#include "control.h"
/*******************************************************************************
*	Objects definition
*******************************************************************************/
CONTAINER 	g_win_nvod_event_name;
TEXT_FIELD 	nvod_event_name;
TEXT_FIELD 	nvod_event_length;
TEXT_FIELD 	nvod_event_time_span;
TEXT_FIELD 	nvod_event_cur_time;
PROGRESS_BAR nvod_vent_bar;
TEXT_FIELD 	nvod_event_bar_txt;

TEXT_FIELD 	nvod_event_str_val;



TEXT_FIELD 	nvod_event_bar_itle;

static VACTION win_nvod_event_keymap ( POBJECT_HEAD pObj, UINT32 key );
static PRESULT win_nvod_event_callback ( POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2 );
static void nvod_event_name_draw(OSD_RECT* pRect,char* str,UINT16* unistr,UINT16 id);
PRESULT nvod_event_unknow_proc(VACTION act);
void win_nvod_event_draw_infor(void);
PRESULT time_splice();

#define PROG_WIN_SH_IDX         WSTL_POP_WIN_1
#define PROG_TXT_WHITE_IDX      WSTL_TXT_3
#define PROG_TXT_BLACK_IDX      WSTL_TXT_14

#define W_L	((OSD_MAX_WIDTH-350)/2)
#define W_T	150//260
#define W_W	400
#define W_H	270

#define TXT_NAME_L  (W_L+20)
#define TXT_BASE_TOP	(W_T+60)
#define TXT_W	(130)
#define TXT_H	(24)
#define TXT_VERTIAL_GAP	(12)
#define BAR_WIDTH	(230)
#define BAR_H	(12)
#define	TXT_BAR_L	(TXT_NAME_L+BAR_WIDTH+10)
#define	TXT_BAR_WIDTH	(W_W-BAR_WIDTH-10)


#define TXT_VAL_L  (W_L+145)
#define TXT_VAL_W  (200)


#define PSBAR_SH_IDX		    WSTL_BARBG_02
#define PSBAR_MID_SH_IDX		WSTL_NOSHOW_IDX

#define LDEF_PROGRESS_BAR(root,varBar,nxtObj,l,t,w,h,style,rl,rt,rw,rh,fgColor)	\
	DEF_PROGRESSBAR(varBar, root, nxtObj, C_ATTR_ACTIVE, 0, \
		0, 0, 0, 0, 0, l, t, w, h, PSBAR_SH_IDX, PSBAR_SH_IDX, PSBAR_SH_IDX, PSBAR_SH_IDX,\
		NULL, NULL, style, 0, 0, PSBAR_MID_SH_IDX, fgColor, \
		rl,rt , rw, rh, 1, 100, 100, 1)

#define LDEF_TXT(root,varTxt,nxtObj,l,t,w,h,shidx)		\
	DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
	    0,0,0,0,0, l,t,w,h, shidx,shidx,shidx,shidx,   \
	    NULL,NULL,  \
	    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0,len_display_str)

#define LDEF_TXT_NAME(root,varTxt,nxtObj,l,t,w,h,shidx,strid)		\
	DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
	    0,0,0,0,0, l,t,w,h, shidx,shidx,shidx,shidx,   \
	    NULL,NULL,  \
	    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,strid,NULL)

#define LDEF_WIN(varWnd,nxtObj,l,t,w,h,sh,focusID)		\
    DEF_CONTAINER(varWnd,NULL,NULL,C_ATTR_ACTIVE,0, \
	    0,0,0,0,0, l,t,w,h, sh,sh,sh,sh,   \
	    win_nvod_event_keymap,win_nvod_event_callback,  \
	    nxtObj, focusID,1)




LDEF_WIN ( g_win_nvod_event_name, &nvod_event_bar_itle, W_L, W_T, W_W, W_H, PROG_WIN_SH_IDX, 1)
LDEF_TXT_NAME ( &g_win_nvod_event_name, nvod_event_bar_itle, &nvod_event_name, TXT_NAME_L, W_T+10, TXT_W, TXT_H, WSTL_TXT_16,RS_PROG_INFO)
LDEF_TXT_NAME ( &g_win_nvod_event_name, nvod_event_name, &nvod_event_length, TXT_NAME_L, TXT_BASE_TOP+(TXT_H+TXT_VERTIAL_GAP)*0, TXT_W, TXT_H, WSTL_TXT_16,RS_PROGRAM_NAME )
LDEF_TXT_NAME ( &g_win_nvod_event_name, nvod_event_length, &nvod_event_time_span, TXT_NAME_L, TXT_BASE_TOP+(TXT_H+TXT_VERTIAL_GAP)*1, TXT_W, TXT_H, WSTL_TXT_16,RS_TIME_LEN )
LDEF_TXT_NAME ( &g_win_nvod_event_name, nvod_event_time_span, &nvod_vent_bar, TXT_NAME_L, TXT_BASE_TOP+(TXT_H+TXT_VERTIAL_GAP)*2, TXT_W, TXT_H, WSTL_TXT_16,RS_PROGRAM_TIME )
LDEF_PROGRESS_BAR ( &g_win_nvod_event_name, nvod_vent_bar, &nvod_event_cur_time, TXT_NAME_L,\
	TXT_BASE_TOP+(TXT_H+TXT_VERTIAL_GAP)*3+(TXT_H-BAR_H)/2, BAR_WIDTH, BAR_H,  PROGRESSBAR_HORI_NORMAL|PBAR_STYLE_RECT_STYLE,0,1,\
	BAR_WIDTH,BAR_H-2,0)
//LDEF_TXT( &g_win_nvod_event_name, nvod_event_bar_txt, &nvod_event_cur_time, TXT_BAR_L, TXT_BASE_TOP+(TXT_H+TXT_VERTIAL_GAP)*3, TXT_BAR_WIDTH, TXT_H, WSTL_TXT_14 )
LDEF_TXT_NAME ( &g_win_nvod_event_name, nvod_event_cur_time, NULL, TXT_NAME_L, TXT_BASE_TOP+(TXT_H+TXT_VERTIAL_GAP)*4, TXT_W, TXT_H, WSTL_TXT_16,RS_SYSTEM_EVENTSET_CURRENT_TIME )

LDEF_TXT( NULL, nvod_event_str_val, NULL, TXT_VAL_L, TXT_BASE_TOP+(TXT_H+TXT_VERTIAL_GAP)*0, TXT_VAL_W, TXT_H, WSTL_TXT_16 )


#define VACT_DETAIL_SHOW	(VACT_PASS+1)
static UINT32 life_time =0;
static BOOL exit_flag=FALSE;

extern  CONTAINER g_win_detail;
/*******************************************************************************
*	Local vriable & function declare
*******************************************************************************/
extern void GetCurEvent(struct nvod_event* event);

UINT16 NvodGetCurRefEventPos();

void win_nvod_event_draw_infor(void)
{
    OSD_RECT rc;
    struct nvod_event curEvent;
    struct nvod_ref_event ref;
    date_time start_time,end_time;
	date_time localTime;
    INT32 h,m,sec;
    char str[40];
    UINT16 curRefEventPos = 0;
    UINT32 strLen = 0;
	INT32 total=0;
	INT32 time_pasd=0;


	/*this curEvent always hold the right content no matter normal 
	 *and timer full screen nvod broswer*/
    GetCurEvent(&curEvent);

 	MEMSET((UINT8*)len_display_str,0,sizeof(len_display_str));
	if(!GetNvodFromTimerFlag())
	{
	    curRefEventPos = NvodGetCurRefEventPos();
	    nvod_get_ref_event(curRefEventPos,&ref);
	    //convert_gb2312_to_unicode(ref.name,(INT32)ref.name_len,&len_display_str,30);
        dvb_to_unicode(ref.name,(INT32)ref.name_len,(UINT16*)&len_display_str,30,2312);
	}
	else
	{
		if(SUCCESS==nvod_get_ref_event_byid(curEvent.ref_event_id,curEvent.ref_service_id,&ref))
		{
			//convert_gb2312_to_unicode(ref.name,(INT32)ref.name_len,&len_display_str,30);
			dvb_to_unicode(ref.name,(INT32)ref.name_len,(UINT16*)&len_display_str,30,2312);
		}		
	}

	 /*nvod title*/
    rc.uLeft = TXT_VAL_L;
    rc.uTop = TXT_BASE_TOP;//W_T+3;
    rc.uWidth = TXT_VAL_W;
    rc.uHeight = TXT_H;
    nvod_event_name_draw(&rc,NULL,len_display_str,0);

    /*event time and total time*/

    GetRefEventStartEndTime(&curEvent,&start_time,&end_time,NULL);
	get_STC_offset(&h,&m,&sec);
    convert_time_by_offset2(&start_time,&start_time,h,m,sec);
    convert_time_by_offset2(&end_time,&end_time,h,m,sec);

	/*total time*/
	MEMSET(str,0,sizeof(str));
	api_get_time_len(&start_time, &end_time, &total);
	if(total<0)
		total = -total;
	
	//sprintf(str,"%d min",total/60);
	ali_itoa(str,total/60);
	rc.uTop += (TXT_VERTIAL_GAP +TXT_H);
	nvod_event_name_draw(&rc,str,NULL,0);

	/*event time*/
	MEMSET(str,0,sizeof(str));
	sprintf ( str, "%02d:%02d~%02d:%02d  ", start_time.hour,start_time.min,\
	        end_time.hour,end_time.min);
	rc.uTop += (TXT_VERTIAL_GAP +TXT_H);
	nvod_event_name_draw(&rc,str,NULL,0);

	/**/
	get_local_time(&localTime);
	api_get_time_len(&start_time, &localTime, &time_pasd);
	if(time_pasd<0)
		time_pasd= -time_pasd;

	if(time_pasd>=total)
		time_pasd = total;

	MEMSET(str,0,sizeof(str));
    TEXT_FIELD* txt;
	txt = &nvod_event_str_val;
    OSD_SetTextFieldContent(txt, STRING_NUM_PERCENT, (time_pasd*100)/total);
	txt->head.frame.uLeft = TXT_BAR_L;
	txt->head.frame.uWidth = 50;
	txt->head.frame.uTop += (TXT_VERTIAL_GAP +TXT_H);
    txt->head.frame.uHeight = TXT_H;
    OSD_DrawObject((POBJECT_HEAD)txt,C_UPDATE_ALL);
	
	PROGRESS_BAR * pbar = &nvod_vent_bar;
	pbar->wTickFg = WSTL_BAR_02;
	OSD_SetProgressBarPos(pbar, (time_pasd*100)/total);
	OSD_DrawObject((POBJECT_HEAD)pbar, C_UPDATE_ALL);


	/*current time*/
	    
    rc.uLeft = TXT_VAL_L;
	 rc.uWidth = TXT_VAL_W;
  	rc.uTop += 2*(TXT_VERTIAL_GAP +TXT_H);

	MEMSET(str,0,sizeof(str));
	sprintf(str ,"%02d:%02d",localTime.hour,localTime.min);
    nvod_event_name_draw(&rc,str,NULL,0);

}

static UINT16 audioChannelID[] = 
{
    RS_AUDIO_MODE_LEFT_CHANNEL,
    RS_AUDIO_MODE_RIGHT_CHANNEL,
    RS_AUDIO_MODE_STEREO,
    RS_AUDIO_MODE_MONO
};

static void NvodDrawAudioChannel(INT8 nextStatus)
{
    OSD_RECT rc;
    struct nvod_event curEvent;
    P_NODE pNode;
    
    rc.uLeft = W_L+200;
    rc.uTop = W_T+(40-TXT_H)/2;//W_T+3;
    rc.uWidth = 120;
    rc.uHeight = TXT_H;
    GetCurEvent(&curEvent);
    //nvod_get_channel(curEvent.service_id,&pNode);
    UINT32 prog_id;

    nvod_get_channel_progid(curEvent.service_id, curEvent.t_s_id, &prog_id);
    get_prog_by_id(prog_id, &pNode);
    if(nextStatus)
    {
        if((pNode.audio_channel==0)&&(nextStatus==-1))
            pNode.audio_channel = 3;
        else
            pNode.audio_channel = (pNode.audio_channel+nextStatus)%4;
        //nvod_modify_channel(curEvent.service_id,&pNode);
        modify_prog(pNode.prog_id, &pNode);
    }
    //nvod_event_name_draw(&rc,NULL,NULL,audioChannelID[pNode.audio_channel]);
}

void win_nvod_event_redraw ( void )
{
	POBJECT_HEAD pObj = ( POBJECT_HEAD ) & g_win_nvod_event_name;
	OSD_DrawObject ( pObj, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL );
}

static VACTION win_nvod_event_keymap ( POBJECT_HEAD pObj, UINT32 key )
{
	VACTION act = VACT_PASS;

	switch ( key )
	{
		case V_KEY_AUDIO:
        case V_KEY_LEFT:
        case V_KEY_RIGHT:
            //win_nvod_event_redraw();
           	win_nvod_event_draw_infor();
            if(key == V_KEY_LEFT)
                NvodDrawAudioChannel(-1);
            else
                NvodDrawAudioChannel(1);
			break;
		case V_KEY_INFOR:
			act = VACT_DETAIL_SHOW;
			break;
		case V_KEY_EXIT:
	    case V_KEY_MENU:
			act = VACT_CLOSE;
			break;
		default:
			act = VACT_PASS;
	}
	return act;
}

static PRESULT win_nvod_event_callback ( POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2 )
{
	PRESULT ret = PROC_PASS;
	UINT32 i;
	VACTION	unact;
	ID taskID;
	UINT8 *screenBuf;

	switch ( event )
	{
		case EVN_PRE_OPEN:
			taskID = osal_task_get_current_id();
			screenBuf = OSD_GetTaskVscrBuffer(taskID);
			MEMSET(screenBuf, OSD_TRANSPARENT_COLOR, OSD_VSRC_MEM_MAX_SIZE);
			/*
			if(IRCA_BAN_GetWinpopShow() == TRUE)
			{
				win_compopup_close();
				IRCA_BAN_SetWinpopShow(FALSE);
			}
			Set_Show_BeforEnter(TRUE);//added by wm 090112
			*/
			break;
		case EVN_POST_OPEN:
			exit_flag= FALSE;
			win_nvod_event_draw_infor();
           // NvodDrawAudioChannel(0);
			break;
		case EVN_PRE_CLOSE:
			OSD_ClearObject((POBJECT_HEAD)&g_win_nvod_event_name, C_UPDATE_ALL);
			break;
		case EVN_POST_CLOSE:
			/*if(IRCA_BAN_GetBannerShow())
			{
				ap_cas_call_back(0x00000020);
			}
			else if(Get_HDBannerShow())
			{
				DIsplay_HD_Info();
			}
			else if(Get_No_Video_BannerShow())
			{
				ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_NOVIDEO_DIRECT, 0, FALSE);
			}
			Set_Show_BeforEnter(FALSE);
			*/
			break;
		case EVN_PRE_DRAW:
			break;
		case EVN_POST_DRAW:
			break;
		case EVN_UNKNOWNKEY_GOT:
			break;
		case EVN_UNKNOWN_ACTION:
			unact = ( VACTION ) ( param1 >> 16 ) ;
			ret = nvod_event_unknow_proc(unact);
			break;
		case EVN_MSG_GOT:
			ret= time_splice();
			break;
		default:
			break;
	}

	return ret;

}

static void nvod_event_name_draw(OSD_RECT* pRect,char* str,UINT16* unistr,UINT16 id)
{
	TEXT_FIELD* txt;

	txt = &nvod_event_str_val;
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


PRESULT nvod_event_unknow_proc(VACTION act)
{
	PRESULT ret = PROC_LOOP;
	
	if(act==VACT_DETAIL_SHOW)
	{
		if(life_time)
		{
			ret =  PROC_LEAVE;
			ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32)&g_win_detail, TRUE );
			life_time =0;
			exit_flag = TRUE;
		}
		else
		{
			ret= PROC_LEAVE;
		}
	}
	return ret;
}

PRESULT time_splice()
{
	PRESULT ret = PROC_LOOP;
	
	if(exit_flag)
		return ret;

	life_time++;
	if(life_time>50)
	{
		life_time=0;	
		ap_send_msg(CTRL_MSG_SUBTYPE_CMD_EXIT_ROOT, (UINT32)&g_win_nvod_event_name, TRUE );
		
	}	
		
	return ret;
}










#if 0
/*******************************************************************************
*	Objects definition
*******************************************************************************/
CONTAINER 	g_win_nvod_event_name;
BITMAP 		nvod_event_bmp;
TEXT_FIELD 	nvod_event_text;

static VACTION win_nvod_event_keymap ( POBJECT_HEAD pObj, UINT32 key );
static PRESULT win_nvod_event_callback ( POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2 );
static void nvod_event_name_draw(OSD_RECT* pRect,char* str,UINT16* unistr,UINT16 id);

#define PROG_WIN_SH_IDX         WSTL_PROG_NAME
#define PROG_BMP_SH_IDX         WSTL_TXT_3
#define PROG_TXT_WHITE_IDX      WSTL_TXT_3
#define PROG_TXT_BLACK_IDX      WSTL_TXT_3

#define W_L	100
#define W_T	400
#define W_W	600
#define W_H	144
#define TXT_VERTIAL_GAP	(12)

#define ITEM_H	26
#define TXT_VAL_L  (W_L+145)
#define TXT_VAL_W  (200)
#define TXT_H	(24)

#define LDEF_BMP(root,varBmp,nxtObj,l,t,w,h,sh,icon)		\
	DEF_BITMAP(varBmp,root,NULL,C_ATTR_ACTIVE,0, \
	    0,0,0,0,0, l,t,w,h, sh,sh,sh,sh,   \
	    NULL,NULL,  \
	    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,icon)

#define LDEF_TXT(root,varTxt,nxtObj,l,t,w,h,shidx)		\
	DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
	    0,0,0,0,0, l,t,w,h, shidx,shidx,shidx,shidx,   \
	    NULL,NULL,  \
	    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0,len_display_str)

#define LDEF_WIN(varWnd,nxtObj,l,t,w,h,sh,focusID)		\
    DEF_CONTAINER(varWnd,NULL,NULL,C_ATTR_ACTIVE,0, \
	    0,0,0,0,0, l,t,w,h, sh,sh,sh,sh,   \
	    win_nvod_event_keymap,win_nvod_event_callback,  \
	    NULL, focusID,1)


LDEF_BMP ( &g_win_nvod_event_name, nvod_event_bmp, NULL, 0, 0, 0, 0, PROG_BMP_SH_IDX, 0 )
LDEF_TXT ( &g_win_nvod_event_name, nvod_event_text, NULL, 0, 0, 0, ITEM_H, WSTL_TXT_3 )

LDEF_WIN ( g_win_nvod_event_name, NULL, W_L, W_T, W_W, W_H, PROG_WIN_SH_IDX, 1 )

/*******************************************************************************
*	Local vriable & function declare
*******************************************************************************/

static UINT32 life_time =0;
static BOOL exit_flag=FALSE;

extern void GetCurEvent(struct nvod_event* event);
UINT16 NvodGetCurRefEventPos();

void win_nvod_event_draw_infor(void)
{
    OSD_RECT rc;
    struct nvod_event curEvent;
    struct nvod_ref_event ref;
    date_time start_time,end_time;
	date_time localTime;
    INT32 h,m,sec;
    char str[40];
    UINT16 curRefEventPos = 0;
    UINT32 strLen = 0;
	INT32 total=0;
	INT32 time_pasd=0;


	/*this curEvent always hold the right content no matter normal 
	 *and timer full screen nvod broswer*/
    GetCurEvent(&curEvent);

 	MEMSET((UINT8*)len_display_str,0,sizeof(len_display_str));
	if(!GetNvodFromTimerFlag())
	{
	    curRefEventPos = NvodGetCurRefEventPos();
	    nvod_get_ref_event(curRefEventPos,&ref);
	    convert_gb2312_to_unicode(ref.name,(INT32)ref.name_len,&len_display_str,30);
	}
	else
	{
		if(SUCCESS==nvod_get_ref_event_byid(curEvent.ref_event_id,curEvent.ref_service_id,&ref))
		{
			convert_gb2312_to_unicode(ref.name,(INT32)ref.name_len,&len_display_str,30);
		}		
	}
    /*nvod title*/
    rc.uLeft = W_L+50;
    rc.uTop = W_T+(40-ITEM_H)/2;//W_T+3;
    rc.uWidth = 120;
    rc.uHeight = ITEM_H;
    nvod_event_name_draw(&rc,NULL,len_display_str,0);

    /*event time and total time*/

    GetRefEventStartEndTime(&curEvent,&start_time,&end_time,NULL);
	get_STC_offset(&h,&m,&sec);
    convert_time_by_offset2(&start_time,&start_time,h,m,sec);
    convert_time_by_offset2(&end_time,&end_time,h,m,sec);

	/*total time*/
	MEMSET(str,0,sizeof(str));
	api_get_time_len(&start_time, &end_time, &total);
	if(total<0)
		total = -total;
	
	//sprintf(str,"%d min",total/60);
	itoa(total/60,str,10);
	rc.uLeft += 80;
	rc.uTop += (TXT_VERTIAL_GAP +TXT_H);
	nvod_event_name_draw(&rc,str,NULL,0);

	/*event time*/
	MEMSET(str,0,sizeof(str));
	sprintf ( str, "%02d:%02d~%02d:%02d  ", start_time.hour,start_time.min,\
	        end_time.hour,end_time.min);
	rc.uTop += (TXT_VERTIAL_GAP +TXT_H);
	nvod_event_name_draw(&rc,str,NULL,0);

	/*
	get_local_time(&localTime);
	api_get_time_len(&start_time, &localTime, &time_pasd);
	if(time_pasd<0)
		time_pasd= -time_pasd;

	if(time_pasd>=total)
		time_pasd = total;

	MEMSET(str,0,sizeof(str));
    TEXT_FIELD* txt;
	txt = &nvod_event_str_val;
    OSD_SetTextFieldContent(txt, STRING_NUM_PERCENT, (time_pasd*100)/total);
	txt->head.frame.uLeft = TXT_BAR_L;
	txt->head.frame.uWidth = 50;
	txt->head.frame.uTop += (TXT_VERTIAL_GAP +TXT_H);
    txt->head.frame.uHeight = TXT_H;
    OSD_DrawObject((POBJECT_HEAD)txt,C_UPDATE_ALL);
	
	PROGRESS_BAR * pbar = &nvod_vent_bar;
	pbar->wTickFg = WSTL_BAR_51;
	OSD_SetProgressBarPos(pbar, (time_pasd*100)/total);
	OSD_DrawObject((POBJECT_HEAD)pbar, C_UPDATE_ALL);


	/*current time*/
	/*    
    rc.uLeft = TXT_VAL_L;
	 rc.uWidth = TXT_VAL_W;
  	rc.uTop += 2*(TXT_VERTIAL_GAP +TXT_H);

	MEMSET(str,0,sizeof(str));
	sprintf(str ,"%02d:%02d",localTime.hour,localTime.min);
    nvod_event_name_draw(&rc,str,NULL,0);
	*/
}

static UINT16 audioChannelID[] = 
{
    RS_AUDIO_MODE_LEFT_CHANNEL,
    RS_AUDIO_MODE_RIGHT_CHANNEL,
    RS_AUDIO_MODE_STEREO,
    RS_AUDIO_MODE_MONO
};

static void NvodDrawAudioChannel(INT8 nextStatus)
{
    OSD_RECT rc;
    struct nvod_event curEvent;
    P_NODE pNode;
    
    rc.uLeft = W_L+200;
    rc.uTop = W_T+(40-ITEM_H)/2;//W_T+3;
    rc.uWidth = 120;
    rc.uHeight = ITEM_H;
    GetCurEvent(&curEvent);
    //nvod_get_channel(curEvent.service_id,&pNode);
    UINT32 prog_id;

    nvod_get_channel_progid(curEvent.service_id, curEvent.t_s_id, &prog_id);
    get_prog_by_id(prog_id, &pNode);
    if(nextStatus)
    {
        if((pNode.audio_channel==0)&&(nextStatus==-1))
            pNode.audio_channel = 3;
        else
            pNode.audio_channel = (pNode.audio_channel+nextStatus)%4;
        //nvod_modify_channel(curEvent.service_id,&pNode);
        modify_prog(pNode.prog_id, &pNode);
    }
    nvod_event_name_draw(&rc,NULL,NULL,audioChannelID[pNode.audio_channel]);
}

void win_nvod_event_redraw ( void )
{
	POBJECT_HEAD pObj = ( POBJECT_HEAD ) & g_win_nvod_event_name;
	OSD_DrawObject ( pObj, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL );
}

static VACTION win_nvod_event_keymap ( POBJECT_HEAD pObj, UINT32 key )
{
	VACTION act = VACT_PASS;

	switch ( key )
	{
		case V_KEY_AUDIO:
        case V_KEY_LEFT:
        case V_KEY_RIGHT:
            win_nvod_event_redraw();
            win_nvod_event_draw_infor();
            if(key == V_KEY_LEFT)
                NvodDrawAudioChannel(-1);
            else
                NvodDrawAudioChannel(1);
			break;
		case V_KEY_EXIT:
	    case V_KEY_MENU:
			act = VACT_CLOSE;
			break;
		default:
			act = VACT_PASS;
	}
	return act;
}

static PRESULT win_nvod_event_callback ( POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2 )
{
	PRESULT ret = PROC_PASS;
	UINT32 i;
	VACTION	unact;
	ID taskID;
	UINT8 *screenBuf;

	switch ( event )
	{
		case EVN_PRE_OPEN:
			taskID = osal_task_get_current_id();
			screenBuf = OSD_GetTaskVscrBuffer(taskID);
			MEMSET(screenBuf, OSD_TRANSPARENT_COLOR, OSD_VSRC_MEM_MAX_SIZE);
			break;
		case EVN_POST_OPEN:
            win_nvod_event_draw_infor();
            NvodDrawAudioChannel(0);
			break;
		case EVN_PRE_CLOSE:
			OSD_ClearObject((POBJECT_HEAD)&g_win_nvod_event_name, C_UPDATE_ALL);
			break;
		case EVN_POST_CLOSE:
			break;
		case EVN_PRE_DRAW:
			break;
		case EVN_POST_DRAW:
			break;
		case EVN_UNKNOWNKEY_GOT:
			break;
		case EVN_UNKNOWN_ACTION:
			unact = ( VACTION ) ( param1 >> 16 ) ;
			ret = nvod_event_unknow_proc(unact);
			break;
		case EVN_MSG_GOT:
			ret= time_splice();
			break;
		default:
			break;
	}

	return ret;

}

static void nvod_event_name_draw(OSD_RECT* pRect,char* str,UINT16* unistr,UINT16 id)
{
	TEXT_FIELD* txt;

	txt = &nvod_event_text;
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

PRESULT nvod_event_unknow_proc(VACTION act)
{
	PRESULT ret = PROC_LOOP;
	

	if(life_time)
	{
		ret =  PROC_LEAVE;
		//ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32)&g_win_detail, TRUE );
		life_time =0;
		exit_flag = TRUE;
	}
	else
	{
		ret= PROC_LEAVE;
	}

	return ret;
}

PRESULT time_splice()
{
	PRESULT ret = PROC_LOOP;
	
	if(exit_flag)
		return ret;

	life_time++;
	if(life_time>50)
	{
		life_time=0;	
		ap_send_msg(CTRL_MSG_SUBTYPE_CMD_EXIT_ROOT, (UINT32)&g_win_nvod_event_name, TRUE );
		
	}	
		
	return ret;
}
#endif

#endif

