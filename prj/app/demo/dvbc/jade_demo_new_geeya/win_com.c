#include <sys_config.h>
#include <types.h>
#include <api/libc/printf.h>
#include <osal/osal.h>
//#include <api/libosd/osd_lib.h>
#include <api/libtsi/db_3l.h>
#include <api/libpub/lib_hde.h>
#include <api/libpub/lib_pub.h>
#include <api/libpub/lib_as.h>
#include <hld/dis/vpo.h>
#include "osdobjs_def.h"
//#include <api/libosd/osd_vkey.h>

#include "osd_config.h"
#include "string.id"
#include "images.id"

#include "key.h"
#include "control.h"
#include "menus_root.h"
#include "win_com.h"
#include "win_signalstatus.h"
#include "win_com_popup.h"

#ifdef MULTI_CAS
#if(CAS_TYPE==CAS_CONAX)
#include <api/libcas/mcas.h>
#endif
#endif

/****************************************************************************/
//Tuner index
UINT8 cur_tuner_idx = 0;

static UINT32 pre_channel = 0;
static UINT8 mode = 0;
static UINT32 freq, symbol;
static UINT8 modulation;
static BOOL mute_flag, pause_flag;
BOOL VolumeOn=FALSE;

#ifdef NVOD_FEATURE
#define NVOD_HANDLE			((POBJECT_HEAD)&g_win_nvod)

extern BOOL Get_NVOD_Scan_status();
#endif

/****************************************************************************/
/* Title and help                                                           */
/****************************************************************************/
// Title
extern CONTAINER title_con;
extern BITMAP title_bmp;
TEXT_FIELD title_txt;
extern  TEXT_FIELD   epg_title_time_txt;
  extern CONTAINER epg_sch_item3;
static UINT16 title_time_buf[30];

#define TITLE_CON_L	0
#define TITLE_CON_T 0
#define TITLE_CON_W 720
#define TITLE_CON_H 90

#define TITLE_BMP_L	(TV_OFFSET_L+66)
#define TITLE_BMP_T (TV_OFFSET_T+40)
#define TITLE_BMP_W 48
#define TITLE_BMP_H 40

#define TITLE_TXT_L	(TITLE_BMP_L+TITLE_BMP_W)
#define TITLE_TXT_T (TV_OFFSET_T+52)
#define TITLE_TXT_W 250
#define TITLE_TXT_H 24

#define TITLE_CON_IDX   WSTL_TITLE_CON_IDX
#define TITLE_BMP_IDX	WSTL_TXT_3
#define TITLE_TXT_IDX	WSTL_TXT_TITLE

DEF_CONTAINER ( title_con, NULL, NULL, C_ATTR_ACTIVE, 0, \
                1, 0, 0, 0, 0, TITLE_CON_L, TITLE_CON_T, TITLE_CON_W, TITLE_CON_H, TITLE_CON_IDX, TITLE_CON_IDX, TITLE_CON_IDX, TITLE_CON_IDX,   \
                NULL, NULL,  \
                ( POBJECT_HEAD ) &title_bmp, 0, 0 )

DEF_BITMAP(title_bmp,&title_con,&title_txt,C_ATTR_ACTIVE,0, \
    1,0,0,0,0, TITLE_BMP_L,TITLE_BMP_T,TITLE_BMP_W,TITLE_BMP_H, TITLE_BMP_IDX,TITLE_BMP_IDX,TITLE_BMP_IDX,TITLE_BMP_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,IM_TITLE_TEXT_01)

DEF_TEXTFIELD ( title_txt, &title_con, NULL, C_ATTR_ACTIVE, 0, \
                0, 0, 0, 0, 0, TITLE_TXT_L, TITLE_TXT_T, TITLE_TXT_W, TITLE_TXT_H, TITLE_TXT_IDX, TITLE_TXT_IDX, TITLE_TXT_IDX, TITLE_TXT_IDX,   \
                NULL, NULL,  \
                C_ALIGN_LEFT| C_ALIGN_VCENTER, 0, 0, 0, NULL )


void wincom_open_title (UINT16 title_strid, UINT16 title_bmpid )
{
    BITMAP *bmp;
	TEXT_FIELD *txt;

    bmp = &title_bmp;
	txt = &title_txt;
    OSD_SetBitmapContent(bmp,title_bmpid);
	OSD_SetTextFieldContent ( txt, STRING_ID, ( UINT32 ) title_strid );
	OSD_DrawObject ( ( POBJECT_HEAD ) &title_con, C_UPDATE_ALL );
}

UINT16 weekday_id[] = 
{
    RS_SUN,
    RS_MON,
    RS_TUES,
    RS_WED,
    RS_THURS,
    RS_FRI,
    RS_SAT
};

//flag==1   Increase time;   flag==2  Decrease time;
void timer_InOrDecrease(UINT8 flag,UINT16 time)
{
	UINT8 i;
	SYSTEM_DATA *sys_data= sys_data_get();
	TIMER_SET_CONTENT *timer1,*timer2;
	date_time time1,time2;
	for(i=0; i<MAX_TIMER_NUM; i++)
	{
		//if(i == sys_data->timer_set.timer_num-1) continue;

		if(sys_data->timer_set.TimerContent[i].timer_mode == TIMER_MODE_OFF) continue;

		if(flag==1){
			// sys_data->timer_set.TimerContent[i].wakeup_time+=time;
			 time1.year= sys_data->timer_set.TimerContent[i].wakeup_year;
			 time1.month= sys_data->timer_set.TimerContent[i].wakeup_month;
			 time1.day= sys_data->timer_set.TimerContent[i].wakeup_day;
			 time1.hour= sys_data->timer_set.TimerContent[i].wakeup_time/3600;
			 time1.min=(sys_data->timer_set.TimerContent[i].wakeup_time-time1.hour*3600)/60;
			 time1.sec=sys_data->timer_set.TimerContent[i].wakeup_time-time1.hour*3600-time1.min*60;
			 convert_time_by_offset(&time2, &time1, time, 0);

			 sys_data->timer_set.TimerContent[i].wakeup_year=time2.year;
			 sys_data->timer_set.TimerContent[i].wakeup_month=time2.month;
			 sys_data->timer_set.TimerContent[i].wakeup_day=time2.day;
			 sys_data->timer_set.TimerContent[i].wakeup_time=time2.hour*3600+time2.min*60+time2.sec;
		}
		else if(flag==2){
			 time1.year= sys_data->timer_set.TimerContent[i].wakeup_year;
			 time1.month= sys_data->timer_set.TimerContent[i].wakeup_month;
			 time1.day= sys_data->timer_set.TimerContent[i].wakeup_day;
			 time1.hour= sys_data->timer_set.TimerContent[i].wakeup_time/3600;
			 time1.min=(sys_data->timer_set.TimerContent[i].wakeup_time-time1.hour*3600)/60;
			 time1.sec=sys_data->timer_set.TimerContent[i].wakeup_time-time1.hour*3600-time1.min*60;
			 convert_time_by_offset(&time2, &time1, -time, 0);

			 sys_data->timer_set.TimerContent[i].wakeup_year=time2.year;
			 sys_data->timer_set.TimerContent[i].wakeup_month=time2.month;
			 sys_data->timer_set.TimerContent[i].wakeup_day=time2.day;
			 sys_data->timer_set.TimerContent[i].wakeup_time=time2.hour*3600+time2.min*60+time2.sec;
		}
	}
	sys_data_save(1);
}

void wincom_draw_epg_title_time()	//hw.wang
{
	TEXT_FIELD *txt;
	date_time dt;
	char str[40];
	UINT8 *src;
	UINT16 strLen;
	UINT16 preLeft;

	txt = &epg_title_time_txt;
	preLeft = txt->head.frame.uLeft;
	//preLeft = txt->head.frame.uLeft;
	//->head.frame.uLeft = txt->head.frame.uLeft + txt->head.frame.uWidth + 50;
	txt->pString = title_time_buf;

	get_local_time(&dt);
	sprintf(str, "%4d.%02d.%02d ", dt.year, dt.month, dt.day);
	ComAscStr2Uni(str, len_display_str);
	strLen = ComUniStrLen(len_display_str);
	src = OSD_GetUnicodeString(weekday_id[dt.weekday%7]);
	ComUniStrCopyChar((UINT8*) &len_display_str[strLen], src);

	strLen = ComUniStrLen(len_display_str);
	MEMSET(str, 0, sizeof(str));
	sprintf(str, " %02d:%02d ", dt.hour, dt.min);
	ComAscStr2Uni(str, &len_display_str[strLen]);

	OSD_SetTextFieldContent(txt, STRING_UNICODE, (UINT32)len_display_str);
	//OSD_DrawObject((POBJECT_HEAD)txt, C_UPDATE_ALL);
	//txt->head.frame.uLeft = preLeft;
	//txt->pString = NULL;
}

void wincom_draw_time_ext(PTEXT_FIELD pctrl) 	//J.Jing
{
	date_time dt;
	char str[40];
	UINT8 *src;
	UINT16 strLen;

	pctrl->pString = display_strs[32];

	get_local_time(&dt);
	sprintf(str, "%4d.%02d.%02d ", dt.year, dt.month, dt.day);
	ComAscStr2Uni(str, len_display_str);
	strLen = ComUniStrLen(len_display_str);
	src = OSD_GetUnicodeString(weekday_id[dt.weekday%7]);
	ComUniStrCopyChar((UINT8*) &len_display_str[strLen], src);

	strLen = ComUniStrLen(len_display_str);
	MEMSET(str, 0, sizeof(str));
	sprintf(str, " %02d:%02d ", dt.hour, dt.min);
	ComAscStr2Uni(str, &len_display_str[strLen]);

	OSD_SetTextFieldContent(pctrl, STRING_UNICODE, (UINT32)len_display_str);
}

void wincom_draw_title_time()
{
	TEXT_FIELD *txt;
	date_time dt;
	char str[40];
	UINT8 *src;
	UINT16 strLen;
	UINT16 preLeft;

	txt = &title_txt;
	preLeft = txt->head.frame.uLeft;
	txt->head.frame.uLeft = txt->head.frame.uLeft + txt->head.frame.uWidth + 50;
	txt->pString = display_strs[49];

	get_local_time(&dt);
	sprintf(str, "%4d.%02d.%02d ", dt.year, dt.month, dt.day);
	ComAscStr2Uni(str, len_display_str);
	strLen = ComUniStrLen(len_display_str);
	src = OSD_GetUnicodeString(weekday_id[dt.weekday%7]);
	ComUniStrCopyChar((UINT8*) &len_display_str[strLen], src);

	strLen = ComUniStrLen(len_display_str);
	MEMSET(str, 0, sizeof(str));
	sprintf(str, " %02d:%02d ", dt.hour, dt.min);
	ComAscStr2Uni(str, &len_display_str[strLen]);

	OSD_SetTextFieldContent(txt, STRING_UNICODE, (UINT32)len_display_str);
	OSD_DrawObject((POBJECT_HEAD)txt, C_UPDATE_ALL);
	txt->head.frame.uLeft = preLeft;
	txt->pString = NULL;
}

void wincom_open_title_ext(UINT16 title_strid, UINT16 title_bmpid)
{
	wincom_open_title(title_strid, title_bmpid);
	wincom_draw_title_time();
}

void wincom_open_title_by_str(UINT8* title_str,UINT16 title_bmpid)
{
    BITMAP *bmp;
	TEXT_FIELD *txt;

    bmp = &title_bmp;
	txt = &title_txt;
    OSD_SetBitmapContent(bmp,title_bmpid);
	txt->pString = display_strs[49];
	OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32) title_str);
	OSD_DrawObject ( ( POBJECT_HEAD ) &title_con, C_UPDATE_ALL );
	txt->pString = NULL;

	wincom_draw_title_time();
}

void wincom_close_title(void)
{
	OSD_ClearObject((POBJECT_HEAD) &title_con, 0);
}

////////////////////////////////////////////////////////
// Help
  #define HELP_BTN_T 396


#define HELP_SH_IDX
extern TEXT_FIELD menu_help_back;
extern BITMAP menu_help_bmp;
extern TEXT_FIELD menu_help_txt;

#define HELP_BACK_IDX	WSTL_HELP_BAR_BACK
#define HLEP_BMP_SH_IDX	WSTL_TXT_3
#define HLEP_TXT_IDX	WSTL_TXT_3

#define HELP_BAR_L      0
#define HELP_BAR_T      490
#define HELP_BAR_W      OSD_MAX_WIDTH
#define HELP_BAR_H      (OSD_MAX_HEIGHT-HELP_BAR_T)

#define HELP_CONTENT_L      64
#define HELP_CONTENT_T      510
#define HELP_CONTENT_W      592
#define HELP_CONTENT_H      24

#define HELP_BMP_W      24
#define HELP_BMP_H      24

#define HELP_ITEM_LEN       160

DEF_TEXTFIELD(menu_help_back, NULL, NULL, C_ATTR_ACTIVE, 0,  \
	0, 0, 0, 0, 0, HELP_BAR_L, HELP_BAR_T, HELP_BAR_W, HELP_BAR_H, HELP_BACK_IDX, HELP_BACK_IDX, HELP_BACK_IDX, HELP_BACK_IDX,  \
	NULL, NULL,  \
	C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, 0, 0)

DEF_BITMAP(menu_help_bmp, NULL, NULL, C_ATTR_ACTIVE, 0,  \
	0, 0, 0, 0, 0, 0, 0, HELP_BMP_W, HELP_BMP_H, HLEP_BMP_SH_IDX, HLEP_BMP_SH_IDX, HLEP_BMP_SH_IDX, HLEP_BMP_SH_IDX,  \
	NULL, NULL,  \
	C_ALIGN_LEFT | C_ALIGN_VCENTER, 0, 0, 0)


DEF_TEXTFIELD(menu_help_txt, NULL, NULL, C_ATTR_ACTIVE, 0,  \
	0, 0, 0, 0, 0, 0, 0, HELP_ITEM_LEN, HELP_BMP_H, HLEP_TXT_IDX, HLEP_TXT_IDX, HLEP_TXT_IDX, HLEP_TXT_IDX,  \
	NULL, NULL,  \
	C_ALIGN_LEFT | C_ALIGN_VCENTER, 0, 0, 0, display_strs[0])



typedef struct
{
	UINT16 l;
	UINT16 t;
}
help_item_pos_t;

#define HELP_ITEM_CNT 8

#if 0
help_item_pos_t item_1_pos[] =
{
    {(HELP_CONTENT_W/2-HELP_ITEM_LEN/2),0},
};

help_item_pos_t item_2_pos[] =
{
    {(HELP_CONTENT_W/2-HELP_ITEM_LEN),0},
    {(HELP_CONTENT_W/2),0},
};

help_item_pos_t item_3_pos[] =
{
    {(HELP_CONTENT_W/2-HELP_ITEM_LEN*3/2),0},
    {(HELP_CONTENT_W/2-HELP_ITEM_LEN/2),0},
    {(HELP_CONTENT_W/2+HELP_ITEM_LEN/2),0},
};

help_item_pos_t item_4_pos[] =
{
    {0,0},
    {HELP_ITEM_LEN,0},
    {HELP_ITEM_LEN*2,0},
    {HELP_ITEM_LEN*3,0},
};

help_item_pos_t *item_pos[] =
{
	item_1_pos, item_2_pos, item_3_pos, item_4_pos
};

void wincom_open_help(struct help_item_resource *helpinfo, UINT8 item_count)
{
	UINT32 i;
	struct help_item_resource *helpitem;
	help_item_pos_t *btnPoses, btnPos;
	BITMAP *bmp;
	TEXT_FIELD *txt;
	UINT16 strLen = 0;
	UINT8 *src;

	if ((item_count <= 0) || (item_count >= (HELP_ITEM_CNT + 1)))
		return ;

	OSD_DrawObject((POBJECT_HEAD) &menu_help_back, C_UPDATE_ALL);

	btnPoses = item_pos[item_count - 1];

	for (i = 0; i < item_count; i++)
	{
		btnPos = btnPoses[i];
		bmp = &menu_help_bmp;
		txt = &menu_help_txt;
		helpitem = &helpinfo[i];

		//draw txt
		MEMSET(len_display_str, 0, sizeof(len_display_str));
		strLen = 0;

		ComAscStr2Uni("[", &len_display_str[strLen]);

		if (helpitem->part1_type == 0)
		//txt
		{
			src = OSD_GetUnicodeString(helpitem->part1_id);
			ComUniStrCopyChar((UINT8*) &len_display_str[1], src);
		}
		else if (helpitem->part1_type == 1)
		//bmp
		{
			ComAscStr2Uni("  ", &len_display_str[strLen + 1]);

		}

		strLen = ComUniStrLen(len_display_str);
		ComAscStr2Uni("]", &len_display_str[strLen]);
		src = OSD_GetUnicodeString(helpitem->part2_id);
		strLen = ComUniStrLen(len_display_str);
		ComUniStrCopyChar((UINT8*) &len_display_str[strLen], src);

		txt->head.frame.uLeft = btnPos.l + HELP_CONTENT_L;
		txt->head.frame.uTop = btnPos.t + HELP_CONTENT_T;
		OSD_SetTextFieldContent(txt, STRING_UNICODE, (UINT32)len_display_str);
		OSD_DrawObject((POBJECT_HEAD)txt, C_UPDATE_ALL);

		//draw bmp
		if (helpitem->part1_type == 1)
		//bmp
		{
			bmp->head.frame.uLeft = btnPos.l + HELP_CONTENT_L + 5;
			bmp->head.frame.uTop = btnPos.t + HELP_CONTENT_T;
			OSD_SetBitmapContent(bmp, helpitem->part1_id);
			OSD_DrawObject((POBJECT_HEAD)bmp, C_UPDATE_ALL);
		}
	}
}
#else
void wincom_open_help(struct help_item_resource *helpinfo, UINT8 item_count)
{
	UINT32 i;
	struct help_item_resource *helpitem;
	BITMAP *bmp;
	TEXT_FIELD *txt;
	UINT16 strLen = 0,totalW,mw,mh;
	UINT8 *src;
    UINT16 bmpL[HELP_ITEM_CNT];

	if ((item_count <= 0) || (item_count >= (HELP_ITEM_CNT + 1)))
		return ;

	OSD_DrawObject((POBJECT_HEAD) &menu_help_back, C_UPDATE_ALL);

	MEMSET(len_display_str, 0, sizeof(len_display_str));
	strLen = 0;

	bmp = &menu_help_bmp;
	txt = &menu_help_txt;

	//draw txt
	for (i = 0; i < item_count; i++)
	{
    	helpitem = &helpinfo[i];
		ComAscStr2Uni("[", &len_display_str[strLen]);
		if (helpitem->part1_type == 0)
		//txt
		{
			src = OSD_GetUnicodeString(helpitem->part1_id);
			ComUniStrCopyChar((UINT8*) &len_display_str[strLen+1], src);
		}
		else if (helpitem->part1_type == 1)
		//bmp
		{
            bmpL[i] = OSD_MultiFontLibStrMaxHW((UINT8 *)len_display_str,
                ((POBJECT_HEAD)txt)->bFont, &mw, &mh, 0);
			ComAscStr2Uni("   ", &len_display_str[strLen + 1]);

		}

		strLen = ComUniStrLen(len_display_str);
		ComAscStr2Uni("]", &len_display_str[strLen]);
		src = OSD_GetUnicodeString(helpitem->part2_id);
		strLen = ComUniStrLen(len_display_str);
		ComUniStrCopyChar((UINT8*) &len_display_str[strLen], src);
        strLen = ComUniStrLen(len_display_str);
        ComAscStr2Uni(" ", &len_display_str[strLen]);
        strLen = ComUniStrLen(len_display_str);
	}
    
    totalW = OSD_MultiFontLibStrMaxHW((UINT8 *)len_display_str, ((POBJECT_HEAD)txt)->bFont, &mw, &mh, 0);
    txt->head.frame.uWidth = (totalW<HELP_CONTENT_W)?totalW : HELP_CONTENT_W;
	txt->head.frame.uLeft = HELP_CONTENT_L+(HELP_CONTENT_W-txt->head.frame.uWidth)/2;
	txt->head.frame.uTop = HELP_CONTENT_T;
	OSD_SetTextFieldContent(txt, STRING_UNICODE, (UINT32)len_display_str);
	OSD_DrawObject((POBJECT_HEAD)txt, C_UPDATE_ALL); 

    //draw bmp
    for (i = 0; i < item_count; i++)
    {
        helpitem = &helpinfo[i];
		bmp->head.frame.uLeft =bmpL[i]+txt->head.frame.uLeft;
		bmp->head.frame.uTop = HELP_CONTENT_T;        
        if (helpitem->part1_type == 1)
        {
			OSD_SetBitmapContent(bmp, helpitem->part1_id);
			OSD_DrawObject((POBJECT_HEAD)bmp, C_UPDATE_ALL);        
        }
    }
    
}
#endif

void wincom_close_help(void)
{
	OSD_ClearObject((POBJECT_HEAD) &menu_help_back, C_UPDATE_ALL);
}

/////////////////////////////////////////////////////////
//preview window handle
////////////////////////////////////////////////////////
UINT16 mtxt_strid = 0;
POBJECT_HEAD preview_pre_win = NULL;
UINT8 preview_mute_status = FALSE;
UINT8 preview_pause_status = FALSE;

static 	UINT16 PrewviewStr[100];
static  UINT8 CasMsg_Show = 0;


TEXT_CONTENT mtxt_preview_str=
{
	STRING_ID, 0
};
TEXT_FIELD txt_preview;// preview main frame
extern MULTI_TEXT info_preview;//display the strings, such as"no signal", "please insert the smc"....
BITMAP preview_mute_bmp;
BITMAP preview_pause_bmp;
#define PREVIEW_BMP_SH_IDX	WSTL_TXT_4

#define LDEF_PREVIEW_WIN(varTxt,nxtObj,l,t,w,h)		\
DEF_TEXTFIELD(varTxt,NULL,nxtObj,C_ATTR_ACTIVE,0, \
0,0,0,0,0, l,t,w,h, WSTL_PREVIEW_TXT,WSTL_PREVIEW_TXT,WSTL_PREVIEW_TXT,WSTL_PREVIEW_TXT,   \
NULL,NULL,  \
C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,0,NULL)

#define LDEF_PREVIEW_INFO(varMtxt,l,t,w,h,cnt,content)	\
	DEF_MULTITEXT(varMtxt,NULL,NULL,C_ATTR_ACTIVE,0, \
    	1,1,1,1,1, l,t,w,h, WSTL_PREVIEW_TXT,WSTL_PREVIEW_TXT,WSTL_PREVIEW_TXT,WSTL_PREVIEW_TXT, NULL,NULL,  \
    	C_ALIGN_CENTER | C_ALIGN_VCENTER, cnt,  4,4,0,0,NULL,content)

#define LEFT_PREVIEW_BMP(root,varBmp,nxtObj,l,t,w,h,icon)	\
DEF_BITMAP(varBmp,&root,nxtObj,C_ATTR_ACTIVE,0, \
0,0,0,0,0, l,t,w,h, PREVIEW_BMP_SH_IDX,PREVIEW_BMP_SH_IDX,PREVIEW_BMP_SH_IDX,PREVIEW_BMP_SH_IDX,   \
NULL,NULL,  \
C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,icon)

LDEF_PREVIEW_WIN(txt_preview, NULL, 0, 0, 0, 0 )
LDEF_PREVIEW_INFO(info_preview, 0,0,0,0, 1,&mtxt_preview_str)
LEFT_PREVIEW_BMP ( txt_preview, preview_mute_bmp , NULL, 0, 0, 0, 0, IM_MUTE_S )
LEFT_PREVIEW_BMP ( txt_preview, preview_pause_bmp, NULL, 0, 0, 0, 0, IM_PAUSE_S )

//preview volume
CONTAINER preview_volume;
BITMAP preview_vol_bmp;
PROGRESS_BAR preview_vol_bar;

#define VOL_H   30

#define VOL_BMP_W   30
#define VOL_BMP_H   30

#define VOL_PRO_H   30

#define VOL_TXT_W   0

#define VOL_MIN         0
#define VOL_MAX         100
#define VOL_STEP_NUM    20
#define VOL_STEP_LEN    ((VOL_MAX-VOL_MIN)/VOL_STEP_NUM)

#define VOLUME_SH_IDX			WSTL_NOSHOW_IDX
#define VOLUME_MID_SH_IDX		WSTL_VOLUME_CON_BG
#define VOLUME_FG_SH_IDX		WSTL_VOLUME_CON_FG

#define LDEF_PROGRESS_BAR(root,varBar,nxtObj,l,t,w,h,style,rl,rt,rw,rh)	\
DEF_PROGRESSBAR(varBar, root, nxtObj, C_ATTR_ACTIVE, 0, \
1, 1, 1, 1, 1, l, t, w, h, VOLUME_SH_IDX, VOLUME_SH_IDX, VOLUME_SH_IDX, VOLUME_SH_IDX,\
NULL, NULL, style, 4, 0, VOLUME_MID_SH_IDX, VOLUME_FG_SH_IDX, \
rl,rt , rw, rh, VOL_MIN, VOL_MAX, VOL_STEP_NUM, 1)

#define LEFT_BMP(root,varBmp,nxtObj,ID,l,t,w,h,icon,IDX)	\
DEF_BITMAP(varBmp,&root,nxtObj,C_ATTR_ACTIVE,0, \
ID,ID,ID,ID,ID, l,t,w,h, IDX,IDX,IDX,IDX,   \
NULL,NULL,  \
C_ALIGN_LEFT | C_ALIGN_VCENTER,0,0,icon)

#define LDEF_CONT(root, varCon,nxtObj,l,t,w,h,conobj,style)		\
DEF_CONTAINER(varCon,root,nxtObj,C_ATTR_ACTIVE,0, \
0,0,0,0,0, l,t,w,h, style,style,style,style,   \
NULL,NULL,  \
conobj, 1,1)

LEFT_BMP(preview_volume, preview_vol_bmp, &preview_vol_bar, 0, 0,  \
	0, 0, 0, IM_VOLUME_ICON_S, WSTL_TXT_4)

LDEF_PROGRESS_BAR(&preview_volume, preview_vol_bar, NULL, 0, 0, 0, 0, PBAR_STYLE_RECT_STYLE | PROGRESSBAR_HORI_NORMAL,  \
	0, 0, 0, 0)

LDEF_CONT(NULL, preview_volume, NULL, 0, 0, 0, 0, &preview_vol_bmp, WSTL_VOLUME_BG)

void PreviewVolProc(INT8 shift)
{
	PROGRESS_BAR *bar;
	SYSTEM_DATA *sys_data;
	INT16 vol;

	sys_data = sys_data_get();
	bar = &preview_vol_bar;
	vol = OSD_GetProgressBarPos(bar);
	vol += VOL_STEP_LEN * shift;

	if (vol < VOL_MIN)
		vol = VOL_MIN;
	if (vol > VOL_MAX)
		vol = VOL_MAX;

	sys_data->volume = vol;
	OSD_SetProgressBarPos(bar, vol);
	api_audio_set_volume(sys_data->volume);
	if (sys_data->volume > 0 && GetMuteState())
		SetMuteOnOff(FALSE);

	OSD_DrawObject((POBJECT_HEAD) &preview_volume, C_UPDATE_ALL);
}

//preview volume end

#define MUTE_S_W 30
#define MUTE_S_H 30
#define PAUSE_S_W 30
#define PAUSE_S_H 30


void setMtxt(UINT16 sid)
{
	mtxt_strid=sid;
	#ifdef MULTI_CAS 
#if(CAS_TYPE==CAS_IRDETO)
	if(sid==0){
		IRCA_BAN_SetBannerShow(FALSE);
		MEMSET(PrewviewStr,0,sizeof(PrewviewStr));
	}
#endif
#endif
}

#ifdef MULTI_CAS
#if (CAS_TYPE == CAS_CDCA)
/*
#define PREVIEW_L 34
#define PREVIEW_T 82
#define PREVIEW_W 320
#define PREVIEW_H 260

#define PREVIEW_INNER_L 54
#define PREVIEW_INNER_T 90
#define PREVIEW_INNER_W 280
#define PREVIEW_INNER_H 212
*/
#define PREVIEW_L  (TV_OFFSET_L+64)
#define PREVIEW_T  (TV_OFFSET_T+96)
#define PREVIEW_W  290
#define PREVIEW_H  234

#define PREVIEW_INNER_L  (PREVIEW_L+24)
#define PREVIEW_INNER_T  (PREVIEW_T+25)
#define PREVIEW_INNER_W  (PREVIEW_W-24-27)
#define PREVIEW_INNER_H  (PREVIEW_H-25-29)

void wincom_open_preview_default()
{
    OSD_RECT rc;
	OSD_RECT info_rc;
    OSD_RECT rc_con;
	OSD_RECT rc_preview;
    
    rc_con.uLeft = PREVIEW_L;
	rc_con.uTop = PREVIEW_T;
	rc_con.uWidth = PREVIEW_W;
	rc_con.uHeight = PREVIEW_H;

	rc_preview.uLeft = PREVIEW_INNER_L;
	rc_preview.uTop = PREVIEW_INNER_T;
	rc_preview.uWidth = PREVIEW_INNER_W;
	rc_preview.uHeight = PREVIEW_INNER_H;
    
	OSD_SetRect2(&info_rc, &rc_preview);
	info_rc.uTop += MUTE_S_H;
	info_rc.uHeight -= (MUTE_S_H*2);
	
	OSD_SetRect2(&txt_preview.head.frame, &rc_con);
	OSD_SetRect2(&info_preview.head.frame, &info_rc);
	OSD_SetRect(&(info_preview.rcText), 4, 4, info_rc.uWidth-8, info_rc.uHeight-8);

    txt_preview.head.style.bShowIdx = WSTL_SUBMENU_PREVIEW;
	txt_preview.head.style.bHLIdx = WSTL_SUBMENU_PREVIEW;
	txt_preview.head.style.bSelIdx = WSTL_SUBMENU_PREVIEW;
	txt_preview.head.style.bGrayIdx = WSTL_SUBMENU_PREVIEW;

	rc.uTop = rc_preview.uTop+2;
	rc.uWidth = MUTE_S_W;
	rc.uHeight = MUTE_S_H;
	rc.uLeft = rc_preview.uLeft + rc_preview.uWidth - 5-rc.uWidth;
	OSD_SetRect2(&preview_mute_bmp.head.frame, &rc);

	rc.uLeft -= 5+rc.uWidth;
	OSD_SetRect2(&preview_pause_bmp.head.frame, &rc);

	OSD_DrawObject((POBJECT_HEAD) &txt_preview, C_UPDATE_ALL);

	wincom_draw_preview();

	hde_config_preview_window(rc_preview.uLeft, rc_preview.uTop, rc_preview.uWidth, rc_preview.uHeight, 1); //always PAL(When change to N,hardware scale.)
	hde_set_mode(VIEW_MODE_PREVIEW);
	vpo_ioctl(g_vpo_dev, VPO_IO_DIRECT_ZOOM, 0);    
}
#endif
#endif

void wincom_open_preview(OSD_RECT rc_con, OSD_RECT rc_preview, UINT8 idx)
{
	OSD_RECT rc;
	OSD_RECT info_rc;

	OSD_SetRect2(&info_rc, &rc_preview);
	info_rc.uTop += MUTE_S_H;
	info_rc.uHeight -= (MUTE_S_H*2);
	
	OSD_SetRect2(&txt_preview.head.frame, &rc_con);
	OSD_SetRect2(&info_preview.head.frame, &info_rc);
	OSD_SetRect(&(info_preview.rcText), 4, 4, info_rc.uWidth-8, info_rc.uHeight-8);

	txt_preview.head.style.bShowIdx = idx;
	txt_preview.head.style.bHLIdx = idx;
	txt_preview.head.style.bSelIdx = idx;
	txt_preview.head.style.bGrayIdx = idx;

	rc.uTop = rc_preview.uTop;//rc_preview.uTop+2;
	rc.uWidth = rc_preview.uWidth;///MUTE_S_W;
	rc.uHeight = rc_preview.uHeight+1;///MUTE_S_H;
	rc.uLeft = rc_preview.uLeft ;///+ rc_preview.uWidth - 5-rc.uWidth;
	OSD_SetRect2(&preview_mute_bmp.head.frame, &rc);

	rc.uHeight-= 2;
	OSD_SetRect2(&preview_pause_bmp.head.frame, &rc);

	OSD_DrawObject((POBJECT_HEAD) &txt_preview, C_UPDATE_ALL);

	wincom_draw_preview();
	#ifdef AD_SANZHOU
	wincom_preview_proc();
	#endif

	hde_config_preview_window(rc_preview.uLeft, rc_preview.uTop, rc_preview.uWidth, rc_preview.uHeight, 1); //always PAL(When change to N,hardware scale.)
	hde_set_mode(VIEW_MODE_PREVIEW);
	vpo_ioctl(g_vpo_dev, VPO_IO_DIRECT_ZOOM, 0);
}

void wincom_open_preview_ext(OSD_RECT rc_con, OSD_RECT rc_preview, UINT8 idx)
{
	OSD_RECT r;

	wincom_open_preview(rc_con, rc_preview, idx);
	/*
	OSD_SetRect(&r, rc_preview.uLeft, rc_preview.uTop + rc_preview.uHeight-30, rc_preview.uWidth, VOL_H);
	//OSD_SetRect(&r, rc_preview.uLeft, rc_preview.uTop + rc_preview.uHeight, rc_preview.uWidth, VOL_H);
	OSD_SetRect2(&preview_volume.head.frame, &r);
	OSD_SetRect(&r, rc_preview.uLeft, rc_preview.uTop + rc_preview.uHeight-30, VOL_BMP_W, VOL_BMP_H);
	OSD_SetRect2(&preview_vol_bmp.head.frame, &r);
	OSD_SetRect(&r, rc_preview.uLeft + VOL_BMP_W, rc_preview.uTop + rc_preview.uHeight-30, rc_preview.uWidth - VOL_BMP_W - VOL_TXT_W, VOL_H);
	OSD_SetRect2(&preview_vol_bar.head.frame, &r);
	r.uLeft = 0;
	r.uTop = 5;//
	r.uHeight -= r.uTop * 2;
	OSD_SetRect2(&preview_vol_bar.rcBar, &r);

	wincom_draw_volume_bar();
	OSD_DrawObject((POBJECT_HEAD) &preview_volume, C_UPDATE_ALL);
	*/
}


/* to keep the npos of progress bar is equal between volume and preview */
static void wincom_draw_volume_bar()
{
	PROGRESS_BAR *bar = &preview_vol_bar;
	SYSTEM_DATA *sys_data = NULL;

	sys_data = sys_data_get();
	OSD_SetProgressBarPos(bar, sys_data->volume);
}

void wincom_draw_preview_frame()
{
	OSD_DrawObject((POBJECT_HEAD) &txt_preview, C_UPDATE_ALL);
	OSD_DrawObject((POBJECT_HEAD) &preview_volume, C_UPDATE_ALL);
}

static void wincom_draw_preview(void)
{
	MULTI_TEXT *mtxt;
	BITMAP *bmp;
	
	mtxt = &info_preview;
	if(CasMsg_Show)
	{
		mtxt_preview_str.bTextType = STRING_UNICODE;
		mtxt_preview_str.text.wStringID= 0;
		mtxt_preview_str.text.pString = PrewviewStr;
	}
	else
	{
		mtxt_preview_str.bTextType =STRING_ID;
		mtxt_preview_str.text.wStringID= mtxt_strid;
	}
	OSD_DrawObject((POBJECT_HEAD)mtxt, C_UPDATE_ALL);
	
	if (GetMuteState())
	{
		bmp = &preview_mute_bmp;
		OSD_DrawObject((POBJECT_HEAD)bmp, C_UPDATE_ALL);
	}
	else
	{
		bmp = &preview_mute_bmp;
		OSD_ClearObject((POBJECT_HEAD)bmp, C_UPDATE_ALL);
	}
	
	if (GetPauseState())
	{
		bmp = &preview_pause_bmp;
		OSD_DrawObject((POBJECT_HEAD)bmp, C_UPDATE_ALL);
	}
	else
	{
		bmp = &preview_pause_bmp;
		OSD_ClearObject((POBJECT_HEAD)bmp, C_UPDATE_ALL);
	}
}

#ifdef MULTI_CAS
#if (CAS_TYPE == CAS_DVT)
	extern UINT16 urgent_broadcast_tip;
#endif
#endif

void wincom_preview_proc()
{
	UINT16 wMsgId;
	signal_lock_status lock_status;
	signal_scramble_status scramble_status;
	signal_parentlock_status parrentlock_flag;

	UINT8 mode,group_index;
	UINT32 prog_num;

#ifdef NVOD_FEATURE
    group_index = sys_data_get_cur_intgroup_index();
    if(group_index != NVOD_GROUP_TYPE)
    {
		mode = sys_data_get_cur_chan_mode();
		prog_num = get_prog_num(VIEW_ALL | mode, 0);    	
    }
    else
		prog_num = get_prog_num(VIEW_ALL | PROG_NVOD_MODE, 0);

#else
	mode = sys_data_get_cur_chan_mode();
	prog_num = get_prog_num(VIEW_ALL | mode, 0);
#endif

	CasMsg_Show = 0;
	GetSignalStatus(&lock_status, &scramble_status, &parrentlock_flag);
	if (lock_status == SIGNAL_STATUS_UNLOCK)
    {
#ifdef NVOD_FEATURE
		if((FALSE == Get_NVOD_Scan_status())
           &&(NVOD_HANDLE == menu_stack_get_top()))
           wMsgId = 0;
        else
        {
        	#ifdef MIS_AD
		if(MIS_GetIsMisControl())
			wMsgId = 0;
		else
		#endif
		#ifdef AD_TYPE
		if(AD_OSDIgnore())
			wMsgId = 0;
		else
		#endif
           	wMsgId = RS_MSG_NO_SIGNAL; 
        }
#else
//    	libc_printf(" %s():  RS_MSG_NO_SIGNAL \n ",__FUNCTION__);
		#ifdef MIS_AD
		if(MIS_GetIsMisControl())
			wMsgId = 0;
		else
		#endif
		#ifdef AD_TYPE
		if(AD_OSDIgnore())
			wMsgId = 0;
		else
		#endif
		wMsgId = RS_MSG_NO_SIGNAL;
#endif
    }
	else if (parrentlock_flag == SIGNAL_STATUS_PARENT_LOCK)
		wMsgId = RS_SYSTME_PARENTAL_LOCK;
    else if(GetSignalStoped())
    	{
		#ifdef MIS_AD
		if(MIS_GetIsMisControl())
			wMsgId = 0;
		else
		#endif
		#ifdef AD_TYPE
		if(AD_OSDIgnore())
			wMsgId = 0;
		else
		#endif
        	wMsgId = RS_SIGNAL_STOPPED;
    	}
#ifdef MULTI_CAS //CAS messages
#if (CAS_TYPE == CAS_DVT)
	else if (urgent_broadcast_tip)
		wMsgId = urgent_broadcast_tip;
#elif(CAS_TYPE==CAS_IRDETO)
	else if(IRCA_BAN_GetBannerShow()==TRUE)
	{
				IRCA_BAN_GetPreViewStr(PrewviewStr,&wMsgId);
				//libc_printf("wMsgId=%d\n",wMsgId);
				CasMsg_Show = 1;
	}
#elif(CAS_TYPE==CAS_ABEL)
	else if(api_abel_osd_get_cas_msg_num())
	{
		UINT8 test_str[256];
		if(api_abel_osd_get_cas_msg_disp_string(test_str, 256, &wMsgId))
		{
			CasMsg_Show = 1;
			//libc_printf("wincom_preview_proc(): \"%s\"\n", test_str);
			ComAscStr2Uni(test_str, PrewviewStr);
		}
	}	
#endif
	else
		wMsgId = ap_get_cas_dispstr(0);
#else //no CAS messages
	else if (scramble_status == SIGNAL_STATUS_SCRAMBLED)
		wMsgId = RS_SYSTME_CHANNEL_SCRAMBLED;
	else
		wMsgId = 0;
#endif

	if (0 == prog_num&&wMsgId!=0&&wMsgId!=RS_DVNCA_PAIR_START&&
		wMsgId!=RS_DVNCA_PAIR_COMPLETE&&wMsgId!=RS_DVNCA_PAIR_FAIL)
		return ;
	//info_preview.wStringID = wMsgId;
	//avoid to draw the preview window frequently
	if ((wMsgId != mtxt_strid) 
		|| (preview_pre_win != menu_stack_get_top())
		||(preview_mute_status != GetMuteState())
		||(preview_pause_status != GetPauseState()))
	{
		mtxt_strid = wMsgId;
		preview_pre_win = menu_stack_get_top();
		preview_mute_status = GetMuteState();
		preview_pause_status = GetPauseState();
		wincom_draw_preview();
	}
}

void wincom_close_preview()
{
	OSD_ClearObject((POBJECT_HEAD) &txt_preview, C_UPDATE_ALL);
	hde_set_mode(VIEW_MODE_FULL);
	vpo_ioctl(g_vpo_dev, VPO_IO_DIRECT_ZOOM, 0);
}



//preview end
////////////////////////////////////////////////////////

UINT8 satlist_tuner_idx;
UINT32 tplist_sat_id;

PRESULT comlist_satlist_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	UINT16 i, wTop;
	char str_len;
	OBJLIST *ol;
	UINT16 unistr[50];

	PRESULT cbRet = PROC_PASS;

	ol = (OBJLIST*)pObj;

	if (event == EVN_PRE_DRAW)
	{
		wTop = OSD_GetObjListTop(ol);
		for (i = 0; i < ol->wDep && (i + wTop) < ol->wCount; i++)
		{
			get_tuner_sat_name(satlist_tuner_idx, wTop + i, unistr);
			win_comlist_set_str(i + wTop, NULL, (char*)unistr, 0);
		}
	}
	else if (event == EVN_POST_CHANGE)
		cbRet = PROC_LEAVE;

	return cbRet;
}


PRESULT comlist_tplist_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	UINT16 i, wTop;
	char str_len;
	OBJLIST *ol;
	UINT16 unistr[50];

	PRESULT cbRet = PROC_PASS;

	ol = (OBJLIST*)pObj;

	if (event == EVN_PRE_DRAW)
	{
		wTop = OSD_GetObjListTop(ol);
		for (i = 0; i < ol->wDep && (i + wTop) < ol->wCount; i++)
		{
			get_tp_name(tplist_sat_id, wTop + i, unistr);
			win_comlist_set_str(i + wTop, NULL, (char*)unistr, 0);
		}
	}
	else if (event == EVN_POST_CHANGE)
		cbRet = PROC_LEAVE;

	return cbRet;
}



PRESULT comlist_chgrpall_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	UINT16 i, wTop;
	OBJLIST *ol;
	UINT16 unistr[50];
	UINT8 group_type;

	PRESULT cbRet = PROC_PASS;

	ol = (OBJLIST*)pObj;

	if (event == EVN_PRE_DRAW)
	{
		wTop = OSD_GetObjListTop(ol);
		for (i = 0; i < ol->wDep && (i + wTop) < ol->wCount; i++)
		{
			get_chan_group_name((UINT8*)unistr, i + wTop, &group_type);
			win_comlist_set_str(i + wTop, NULL, unistr, NULL);
		}
	}
	else if (event == EVN_POST_CHANGE)
		cbRet = PROC_LEAVE;

	return cbRet;
}


PRESULT comlist_chgrpfav_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	UINT16 i, wTop;
	OBJLIST *ol;
	UINT16 unistr[50];
	UINT8 group_type;
	UINT8 fav_grp_start;
	UINT8 av_flag;

	PRESULT cbRet = PROC_PASS;

	ol = (OBJLIST*)pObj;

	if (event == EVN_PRE_DRAW)
	{
		wTop = OSD_GetObjListTop(ol);
		av_flag = sys_data_get_cur_chan_mode();
		fav_grp_start = sys_data_get_sate_group_num(av_flag) + 1;

		for (i = 0; i < ol->wDep && (i + wTop) < ol->wCount; i++)
		{
			get_chan_group_name((UINT8*)unistr, i + wTop + fav_grp_start, &group_type);
			win_comlist_set_str(i + wTop, NULL, unistr, NULL);
		}
	}
	else if (event == EVN_POST_CHANGE)
		cbRet = PROC_LEAVE;

	return cbRet;
}

PRESULT comlist_chanlist_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT cbRet = PROC_PASS;
	UINT16 i, wTop;
	OBJLIST *ol;
	P_NODE p_node;
	UINT16 unistr[30];


	ol = (OBJLIST*)pObj;

	if (event == EVN_PRE_DRAW)
	{
		wTop = OSD_GetObjListTop(ol);
		for (i = 0; i < ol->wDep && (i + wTop) < ol->wCount; i++)
		{
			get_prog_at(i + wTop, &p_node);
			if (p_node.ca_mode)
				ComAscStr2Uni("$", unistr);
			ComUniStrCopyChar((UINT8*) &unistr[p_node.ca_mode], p_node.service_name);
			win_comlist_set_str(i + wTop, NULL, (UINT16*)unistr, 0);
		}
	}
	else if (event == EVN_POST_CHANGE)
		cbRet = PROC_LEAVE;

	return cbRet;
}


extern PRESULT comlist_menu_language_osd_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);
extern PRESULT comlist_menu_language_stream_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);
extern PRESULT comlist_tvsys_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);
extern PRESULT comlist_menu_osd_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);
extern PRESULT comlist_menu_time_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);
extern PRESULT comlist_menu_keylist_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);
#ifdef MULTI_CAS
#if (CAS_TYPE == CAS_TF || CAS_TYPE == CAS_CDCA)
extern PRESULT comlist_menu_ca_operator_callback ( POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2 );
extern VACTION comlist_menu_ca_operator_keymap ( POBJECT_HEAD pObj, UINT32 vkey );
#endif
#endif
// sub list (SAT / TP / LNB / DisEqC1.0 )
UINT16 win_com_open_sub_list(COM_POP_LIST_TYPE_T type, OSD_RECT *pRect, COM_POP_LIST_PARAM_T *param)
{
	//struct OSDRect rect;
    OSD_RECT rect;
	UINT16 i, count = 0, dep, cur_idx;
	UINT16 style;
	UINT8 mark_align, offset;
	PFN_KEY_MAP winkeymap = NULL;
	PFN_CALLBACK callback = NULL;
	UINT8 back_saved;
	UINT av_mode;

	dep = (pRect->uHeight - 20) / 26;

	switch (type)
	{
		case POP_LIST_TYPE_SAT:
			satlist_tuner_idx = param->id;
			count = get_tuner_sat_cnt(satlist_tuner_idx);
			callback = comlist_satlist_callback;
			break;
		case POP_LIST_TYPE_TP:
			tplist_sat_id = param->id;
			count = get_tp_num_sat(tplist_sat_id);
			callback = comlist_tplist_callback;
			break;
			//			case POP_LIST_TYPE_LNB:
			//				count = get_lnb_type_cnt();
			//				callback = comlist_lnblist_callback;
			//				break;

		case POP_LIST_TYPE_CHGRPALL:
			count = sys_data_get_group_num();
			callback = comlist_chgrpall_callback;
			break;
		case POP_LIST_TYPE_CHGRPFAV:
			av_mode = sys_data_get_cur_chan_mode();
			count = sys_data_get_fav_group_num(av_mode);
			callback = comlist_chgrpfav_callback;
			break;
		case POP_LIST_TYPE_CHGRPSAT:
			av_mode = sys_data_get_cur_chan_mode();
			count = sys_data_get_sate_group_num(av_mode) + 1;
			callback = comlist_chgrpall_callback;
			break;

		case POP_LIST_TYPE_CHANLIST:
			av_mode = sys_data_get_cur_chan_mode();
			count = get_prog_num(VIEW_ALL | av_mode, 0);
			callback = comlist_chanlist_callback;
			break;

		case POP_LIST_MENU_LANGUAGE_OSD:
			//count = win_language_get_menu_language_osd_num();
			//callback = comlist_menu_language_osd_callback;
			break;
		case POP_LIST_MENU_LANGUAGE_STREAM:
			//count = win_language_get_menu_language_stream_num();
			//callback = comlist_menu_language_stream_callback;
			break;

		case POP_LIST_MENU_TVSYS:
			//count = win_language_get_tvsys_num();
			//callback = comlist_tvsys_callback;
			break;
		case POP_LIST_MENU_OSDSET:
			//count = win_osd_get_num();
			//callback = comlist_menu_osd_callback;
			break;
		case POP_LIST_MENU_CA_OPERATOR_INFO:
#ifdef MULTI_CAS
#if (CAS_TYPE == CAS_TF || CAS_TYPE == CAS_CDCA)
			count = win_ca_operator_get_num();
			callback = comlist_menu_ca_operator_callback;
#endif
#endif
			break;
	    case POP_LIST_MENU_MATURITY_RATING:
#ifdef MULTI_CAS
#if (CAS_TYPE == CAS_CONAX)
	        count = 4;
//	        callback = comlist_maturity_callback;
#endif
#endif
		case POP_LIST_MENU_TIME:
			//unt = win_time_get_num();
			//llback = comlist_menu_time_callback;
			break;
#ifdef KEY_EDIT
		case POP_LIST_MENU_KEYLIST:
			count = win_keylist_get_num();
			callback = comlist_menu_keylist_callback;
			break;
#endif

		default:
			return 0;
	}

	cur_idx = param->cur;
	OSD_SetRect2(&rect, pRect);
	offset = 30;


	style = LIST_VER | LIST_SCROLL | LIST_FOCUS_FIRST | LIST_PAGE_KEEP_CURITEM | LIST_FULL_PAGE;
	if (type == POP_LIST_TYPE_SAT || type == POP_LIST_TYPE_FAVSET)
	{
		if (param->selecttype == POP_LIST_SINGLESELECT)
			style |= LIST_SINGLE_SLECT;
		else if (param->selecttype == POP_LIST_MULTISELECT)
			style |= LIST_MULTI_SLECT;
	}
	else
		style |= LIST_SINGLE_SLECT;

#ifdef MULTI_CAS
#if (CAS_TYPE == CAS_TF || CAS_TYPE == CAS_CDCA)
	if (type == POP_LIST_MENU_CA_OPERATOR_INFO)
		winkeymap = comlist_menu_ca_operator_keymap;
#endif
#endif	

	mark_align = C_ALIGN_RIGHT;
	win_comlist_reset();
	win_comlist_set_sizestyle(count, dep, style);
	if (style &LIST_MULTI_SLECT)
		win_comlist_set_align(30, 0, C_ALIGN_LEFT | C_ALIGN_VCENTER);
	else
		win_comlist_set_align(10, 0, C_ALIGN_LEFT | C_ALIGN_VCENTER);
	win_comlist_ext_enable_scrollbar(TRUE);
	win_comlist_set_frame(rect.uLeft, rect.uTop, rect.uWidth, rect.uHeight);
	win_comlist_set_mapcallback(NULL, winkeymap, callback);

	win_comlist_ext_set_ol_frame(rect.uLeft + 10, rect.uTop + 10 /*30*/, rect.uWidth - 20, rect.uHeight - 20 /*40*/);
	win_comlist_ext_set_selmark_xaligen(mark_align, offset);
	//win_comlist_ext_set_win_style(win_idx);
	//win_comlist_ext_set_ol_items_style( sh_idx,  hi_idx,  sel_idx);
	//win_comlist_ext_set_title(/*title_str*/NULL,NULL,title_id);
	win_comlist_ext_set_selmark(0);
	if (style &LIST_SINGLE_SLECT)
		win_comlist_ext_set_item_sel(cur_idx);
	else if (style &LIST_MULTI_SLECT)
	{
		for (i = 0; i < count && param->select_array; i++)
		{
			if (param->select_array[i])
				win_comlist_ext_set_item_sel(i);
		}

		win_comlist_ext_set_selmark(IM_PAUSE_S);
		win_comlist_ext_set_selmark_xaligen(C_ALIGN_LEFT, 2);
	}
	win_comlist_ext_set_item_cur(cur_idx);

	win_comlist_popup_ext(&back_saved);

	return win_comlist_get_selitem();
}






#define TUNERSAT_API_DEBUG	PRINTF//soc_printf

UINT32 get_tuner_cnt(void)
{
	UINT32 tuner_cnt = 1;

	return tuner_cnt;
}

UINT16 get_tuner_sat_cnt(UINT16 tuner_idx)
{
	UINT16 sat_cnt = 1;

	return sat_cnt;
}




void get_tuner_sat(UINT32 tuner_idx, UINT32 sat_idx, S_NODE *snode)
{
#ifdef SELECT_SAT_ONLY
	get_sat_at(sat_idx, SET_SELECTED, snode);
#else
	get_sat_at(sat_idx, VIEW_ALL, snode);
#endif
}

void get_tuner_sat_name(UINT32 tuner_idx, UINT32 sat_idx, UINT16 *unistr)
{
	S_NODE s_node;
	UINT32 sat_cnt;
	UINT16 sat_select_mode = 0;
	char str[30];
	UINT32 str_len;
#ifdef SELECT_SAT_ONLY
	sat_select_mode = SET_SELECTED;
#else
	sat_select_mode = VIEW_ALL;
#endif
	sat_cnt = get_sat_num(sat_select_mode);
	if (sat_idx < sat_cnt)
	{
		get_sat_at(sat_idx, sat_select_mode, &s_node);
		sprintf(str, "(%d/%d) ", sat_idx + 1, sat_cnt);
		ComAscStr2Uni(str, unistr);
		str_len = ComUniStrLen(unistr);
		ComUniStrCopyChar((UINT8*)(&unistr[str_len]), s_node.sat_name);
	}
	else
		ComAscStr2Uni("No Sat", unistr);
}

void get_tp_name(UINT32 sat_id, UINT32 tp_pos, UINT16 *unistr)
{
	T_NODE t_node;
	UINT32 tp_cnt;
	char str[30];

	tp_cnt = get_tp_num_sat(sat_id);
	if (tp_pos < tp_cnt)
	{
		get_tp_at(sat_id, tp_pos, &t_node);
		sprintf(str, "(%d/%d) %d %c %d", tp_pos + 1, tp_cnt, t_node.frq, (t_node.pol == 0) ? 'H' : 'V', t_node.sym);
	}
	else
		sprintf(str, "No TP");
	ComAscStr2Uni(str, unistr);
}


void get_chan_group_name(char *group_name, UINT8 group_idx, UINT8 *group_type)
{
	UINT8 group_pos, str_len;
	UINT16 cur_channel;
	S_NODE s_node;
	char str[50];
	SYSTEM_DATA *sys_data;
	UINT8 *scr;

	sys_data = sys_data_get();

	sys_data_get_cur_mode_group_infor(group_idx, group_type, &group_pos, &cur_channel);
	if (*group_type == ALL_GROUP_TYPE)
		STRCPY(str, "All Group");
	else if (*group_type == ALL_GROUP_TYPE)
	{
		scr = OSD_GetUnicodeString(RS_CHANNEL_TV_CHANNEL_LIST);
		ComUniStrCopyChar((UINT8*)group_name, scr);
		return ;
		/*		get_sat_at ( group_pos, VIEW_ALL, &s_node );
		#ifndef DB_USE_UNICODE_STRING
		STRCPY ( str, s_node.sat_name );
		#else
		ComUniStrCopyChar ( group_name, s_node.sat_name );
		return ;
		#endif
		 */
	}
	else
	{
#ifdef FAV_GROP_RENAME
		scr = OSD_GetUnicodeString(RS_FAVORITE_GROUP);
		ComUniStrCopyChar((UINT8*)group_name, scr);
		return ;
		//		sprintf ( str, "%s", sys_data->favgrp_names[group_pos] );
#else
		STRCPY(str, "FAV Group ");
		str_len = STRLEN(str);
		sprintf(&str[str_len], "%d", group_pos + 1);
#endif
	}

#ifndef DB_USE_UNICODE_STRING
	STRCPY(group_name, str);
#else
	ComAscStr2Uni(str, (UINT16*)group_name);
#endif
}

void get_cur_group_name(char *group_name, UINT8 *group_type)
{
	UINT8 cur_group;

	cur_group = sys_data_get_cur_group_index();
	get_chan_group_name(group_name, cur_group, group_type);
}


void remove_menu_item(CONTAINER *root, OBJECT_HEAD *item, UINT32 height)
{
	OBJECT_HEAD *obj,  *preobj,  *nxtobj;
	OBJECT_HEAD *head,  *tail,  *newhead,  *newtail;
	short y;
	y =  - height;


	head = OSD_GetContainerNextObj(root); /*get head node from CONTAINER structure*/
	if (head == NULL)
		return ;

	obj = head;
	while (obj != NULL)
	{
		tail = obj;
		obj = OSD_GetObjpNext(obj);
	} /*find tail*/

	preobj = obj = head;
	while (obj != NULL && obj != item)
	{
		preobj = obj;
		obj = OSD_GetObjpNext(obj);
	} /*find item position*/
	if (obj == NULL)
		return ;

	/* Relink object (Remove object of "item" ) */
	nxtobj = OSD_GetObjpNext(obj);
	if (item != head)
		OSD_SetObjpNext(preobj, nxtobj);
	else
		OSD_SetContainerNextObj(root, nxtobj);
	 /* Remove head */

	/* Move objects' postion from start of next item of "item" */
	obj = nxtobj;
	while (obj != NULL)
	{
		OSD_MoveObject(obj, 0, y, TRUE);
		obj = OSD_GetObjpNext(obj);
	}

	newhead = newtail = OSD_GetContainerNextObj(root);
	if (newhead == NULL)
	 /* All the items have already been removed */
		return ;
	obj = newhead;
	while (obj != NULL)
	{
		newtail = obj;
		obj = OSD_GetObjpNext(obj);
	}

	/* Set UP/DOWN ID */
	if (item == head || item == tail)
	{
		newhead->bUpID = newtail->bID;
		newtail->bDownID = newhead->bID;
	}
	else
	{
		preobj->bDownID = nxtobj->bID;
		nxtobj->bUpID = preobj->bID;
	}

	obj = newhead;
	while (obj->bID == 0)
	{
		obj = OSD_GetObjpNext(obj);
	} /*Xian_Zhou added,to avoid the newhead ID is 0,set Container focus id as the first non-zero object*/
	OSD_SetContainerFocus(root, obj->bID);
}



/**********************************************************
 * copy from win_com.c  of  dvbc
 ***********************************************************/
UINT8 ComStrCat(UINT8 *Target, const UINT8 *Source)
{
	UINT8 i, j;
	if (Target == NULL)
		return 0;
	i = STRLEN(Target);
	if (Source == NULL)
		return i;
	j = 0;
	while (Source[j])
		Target[i++] = Source[j++];
	Target[i] = 0;
	return i;
}

void ComInt2Str(UINT8 *lpStr, UINT32 dwValue, UINT8 bLen)
{
	char *p;
	char *first_dig;
	char temp;
	unsigned val;
	p = lpStr;
	first_dig = p;

	if (bLen > 0)
	{
		do
		{
			//从后向前取数字
			val = (unsigned)(dwValue % 0x0a);
			dwValue /= 0x0a;
			*p++ = (char)(val + '0');
			bLen--;
		}
		while (bLen > 0); //按长度bLen取
	}
	else
	{
		do
		{
			val = (unsigned)(dwValue % 0x0a);
			dwValue /= 0x0a;
			*p++ = (char)(val + '0');
		}
		while (dwValue > 0); //按实际数位取
	}
	*p-- = '\0';

	//逆序转置
	do
	{
		temp =  *p;
		*p =  *first_dig;
		*first_dig = temp;
		--p;
		++first_dig;
	}
	while (first_dig < p);
}


UINT32 wincom_AscToMBUni(UINT8 *Ascii_str, UINT16 *Uni_str)
{
	int i = 0;

	if ((Ascii_str == NULL) || (Uni_str == NULL))
		return 0;
	while (Ascii_str[i])
	{
		Uni_str[i] = (UINT16)Ascii_str[i];
		i++;
	}

	Uni_str[i] = 0;

#if (SYS_CPU_ENDIAN==ENDIAN_BIG)
	return ComUniStrLen(Uni_str);
#else
	i = 0;
	while (Uni_str[i])
	{
		Uni_str[i] = (UINT16)(((Uni_str[i] &0x00ff) << 8) | ((Uni_str[i] &0xff00) >> 8));
		i++;
	}
	return i;
#endif
}

void wincom_i_to_mbs(UINT16 *str, UINT32 num, UINT32 len)
{
	UINT8 str_buf[20];

	// convert
	ComInt2Str(str_buf, num, len);
	wincom_AscToMBUni(str_buf, str);
}


UINT32 wincom_mbs_to_i(UINT16 *str)
{
	UINT32 total = 0;

	while (*str != '\0')
		total = 10 * total + (ComMB16ToUINT16((UINT8*)str++) - '0');
	 /* accumulate digit */

	return total;
}

void wincom_i_to_mbs_with_dot(UINT16 *str, UINT32 num, UINT32 len /*without dot*/, UINT8 dot_pos)
{
	UINT8 str_buf[20];
	UINT8 tmp_buf[20];
	UINT32 cnt = 0x0;
	UINT32 tmp = 0x1;

	if (dot_pos == 0x0)
	{
		ComInt2Str(str_buf, num, len);
	}
	else
	{
		while (dot_pos--)
		{
			tmp = tmp * 10;
			cnt++;
		}

		if (tmp > num)
			ComInt2Str(str_buf, 0x0, len);
		else
		{
			ComInt2Str(str_buf, num / tmp, len < cnt ? 0 : len - cnt);
			ComStrCat(str_buf, ".");
			ComInt2Str(tmp_buf, num % tmp, cnt);
			ComStrCat(str_buf, tmp_buf);
		}
	}
	// byte order
	wincom_AscToMBUni(str_buf, str);
}

void wincom_mbs_to_i_with_dot(UINT16 *str, UINT32 *num, UINT8 *dot_pos)
{
	BOOL is_find_dot = FALSE;
	UINT16 tmp;

	// init
	*num = 0,  *dot_pos = 0;

	while (*str != '\0')
	{
		tmp = ComMB16ToUINT16((UINT8*)str++);

		if (is_find_dot)
			*dot_pos++;

		if (tmp != '.')
			*num = 10 *(*num) + (tmp - '0');
		 /* accumulate digit */
		else
			is_find_dot = TRUE;
	}
}

void display_str_init(UINT8 max)
{
	UINT8 i = 0;

	for (i = 0; i <= max; i++)
	{
		MEMSET(display_strs[i], 0, sizeof(display_strs[i]));
	}
}

extern CONTAINER g_win_mainmenu;



void ForceToScrnDonothing()
{
	OSD_RECT rect;
	menu_stack_pop_all();

	hde_set_mode(VIEW_MODE_FULL);
	vpo_ioctl(g_vpo_dev, VPO_IO_DIRECT_ZOOM, 0);
	/* Clear OSD */
#ifdef USE_LIB_GE
    OSD_ClearScrn(NULL);
#else
	OSD_GetRectOnScreen(&rect);
	OSDDrv_RegionFill((HANDLE)g_osd_dev, 0, &rect, OSD_TRANSPARENT_COLOR);
#endif
	/* show mute & pause status*/
	ShowMuteOnOff();
	ShowPauseOnOff();
}


/* make sure save any changes in each window(current window, or all windows in menu stack)
 * before call this function.
 */
void BackToFullScrPlay()
{
	OSD_RECT rect;
	UINT16 cur_channel;
	UINT8 no_channel = 0, back_saved;

#ifdef ALI_DEMO_AD
    #ifdef ALI_AD_DEMO_ON
    menu_close_cb(menu_stack_get_top());
    #endif
#endif
	menu_stack_pop_all();
#ifdef MIS_AD
	MIS_Set_EnterMainMenu_Full(FALSE);
	Mis_Set_SameChan_AdvShowOnce(TRUE);
#endif	
	cur_channel = sys_data_get_cur_group_cur_mode_channel();
	if (get_prog_num(VIEW_ALL | PROG_TV_MODE, 0) == 0)
	{
		no_channel = 1;
	}
	if (get_prog_num(VIEW_ALL | PROG_RADIO_MODE, 0) > 0)
	{
		if (no_channel)
			sys_data_set_cur_chan_mode(RADIO_CHAN);
		no_channel = 0;
	}
	if (no_channel)
	{
		if (OSD_ObjOpen((POBJECT_HEAD) &g_win_mainmenu, MENU_OPEN_TYPE_STACK) != PROC_LEAVE)
			menu_stack_push((POBJECT_HEAD) &g_win_mainmenu);
	}
	else
	{
		hde_set_mode(VIEW_MODE_FULL);
		vpo_ioctl(g_vpo_dev, VPO_IO_DIRECT_ZOOM, 0);
		/* Clear OSD */
#ifdef USE_LIB_GE
        OSD_ClearScrn(NULL);
#else
		OSD_GetRectOnScreen(&rect);
		OSDDrv_RegionFill((HANDLE)g_osd_dev, 0, &rect, OSD_TRANSPARENT_COLOR);
#endif
		/* show mute & pause status*/
		ShowMuteOnOff();
		ShowPauseOnOff();
        
#if ((SUBTITLE_ON == 1)||(TTX_ON == 1)) 
		api_osd_mode_change(OSD_SUBTITLE); 
#endif
        
#ifdef MULTI_CAS
#if((CAS_TYPE != CAS_CONAX) && (CAS_TYPE != CAS_ABEL))
		ap_cas_message_show();
#elif(CAS_TYPE==CAS_CONAX)
        if((get_mmi_showed()== 0x02) && (ap_get_cas_dispstr(0)==RS_CONAX_NO_CARD))
	ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MCAS, MCAS_DISP_CARD_NON_EXIST<<16, FALSE);
	 set_mmi_showed(10);
#endif
#endif
		/* show channel index */
		key_pan_display_channel(cur_channel);

		if(GetChannelParrentLock())
		{
			api_play_channel(cur_channel,TRUE,TRUE,FALSE);
		}
#ifdef MULTI_CAS
#if(CAS_TYPE==CAS_IRDETO)
		if(IRCA_BAN_GetBannerShow())
		{
			//ap_cas_call_back(0x00000020);
			//ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MCAS,0x00200000 , FALSE);
			ap_send_msg_expand(CTRL_MSG_SUBTYPE_STATUS_MCAS,0x00000020 ,0, FALSE);
		}
#endif
#endif
	}
}


void restore_scene()
{

	/*cause mode could be changed, so we need restore pre mode and recreate_prog_view*/
	recreate_prog_view(VIEW_ALL | mode, 0);
	sys_data_set_cur_chan_mode(mode);
	api_play_channel(pre_channel, TRUE, FALSE, FALSE);

	/* restore mute pause status , we can ingore mute status because
	 *api_play_channel not change the mute status but pause status have changed
	 */

	if (pause_flag)
		SetPauseOnOff(FALSE);

}

void record_scene()
{
	struct nim_device *nim_dev = (struct nim_device*)dev_get_by_id(HLD_DEV_TYPE_NIM, 0);

	pre_channel = sys_data_get_cur_group_cur_mode_channel();
	mode = sys_data_get_cur_chan_mode();


	mute_flag = GetMuteState();
	pause_flag = GetPauseState();
	api_stop_play(0);
}



/* if a timer binded program is deleted, we should disable this timer*/
void del_from_timer(UINT32 prg_id)
{
	UINT32 index = 0;
	TIMER_SET_CONTENT *timer = NULL;

	for (; index < MAX_TIMER_NUM; index++)
	{
		timer = &system_config.timer_set.TimerContent[index];
		if (prg_id == timer->wakeup_channel)
		{
			timer->timer_mode = TIMER_MODE_OFF;
		}
	}
}

/*find the program id from the index in current mode*/
inline INT32 find_prg_id_from_index(UINT16 index)
{
	P_NODE p_node;
	INT32 ret = !SUCCESS;

	ret = get_prog_at(index, &p_node);
	if (SUCCESS == ret)
		return p_node.prog_id;
	return PROG_ID_NOTFOUND;

}

/*draw title for different group & av mode*/
void DrawGrpName(TEXT_FIELD * P_text, UINT8 grp_idx)
{
	TEXT_FIELD *txt = P_text;
	UINT16 *txt_str = NULL;
	UINT8 av_mode;
	UINT16 strID;

	if (grp_idx > 2)
	{
		strID = RS_UNKNOW_GROUP; /* this code should not be reached*/
	}
	else
	{
		if (2 == grp_idx)
		{
			strID = RS_FAVORITE_GROUP;
		}
		else
		{
			av_mode = sys_data_get_cur_chan_mode();
			strID = (av_mode == TV_CHAN) ? RS_CHANNEL_TV_CHANNEL_LIST : RS_CHANNEL_RADIO_CHANNEL_LIST;
		}
	}
	txt_str = txt->pString;
	txt->pString = NULL;
	OSD_SetTextFieldContent(txt, STRING_ID, strID);
	OSD_DrawObject((POBJECT_HEAD)txt, C_UPDATE_ALL);
	txt->pString = txt_str;
}

/* Draw local group name for channel list and epg window */
void Draw_localGrp_Name(TEXT_FIELD * P_text, UINT8 grp_idx)
{
	
	SYSTEM_DATA *sys_data;
	TEXT_FIELD *txt = P_text;
	UINT8 *str=NULL;
	UINT16 strID = 0;
	UINT16 channel;
	UINT8 group_type,group_pos;

	sys_data_get_cur_mode_group_infor(grp_idx, &group_type, &group_pos, &channel);

	if(group_type == ALL_GROUP_TYPE)
	{
		if(sys_data_get_cur_chan_mode()==RADIO_CHAN)
			strID = RS_INFO_RADIO;
		else
			strID = RS_INFO_TV;
	}
	else if(group_type == DATA_GROUP_TYPE)
		strID = RS_DATA_BROADCAST;
	else if(group_type == NVOD_GROUP_TYPE)
		strID = RS_NVOD;
	else if(group_type == LOCAL_GROUP_TYPE)
	{// get local group name
		sys_data = sys_data_get();
		if(!sys_data->local_group_id[group_pos-1])
			strID = RS_UNKNOW_GROUP;
		else
			str = (UINT8*)&sys_data->local_group_name[group_pos-1][0];
	}
	else if(group_type == FAV_GROUP_TYPE)
		strID = RS_FAVORITE_GROUP;//favlst_ids[group_pos-MAX_GROUP_NUM];

	if(str==NULL)
	{
		OSD_SetTextFieldStrPoint(txt, NULL);
		OSD_SetTextFieldContent(txt, STRING_ID, strID);
	}
	else
	{
		OSD_SetTextFieldStrPoint(txt, display_strs[33]);
		OSD_SetTextFieldContent(txt, STRING_UNICODE, (UINT32)str);
	}


	OSD_DrawObject((POBJECT_HEAD)txt, C_UPDATE_ALL);
}


/* clear trash when we want to reuse display_strs[XX] */
void win_clear_trash(UINT16 * start, UINT32 count)
{
	memset(start, 0, MAX_DISP_STR_LEN *count * 2);
}

#ifdef USE_LIB_GE
BOOL wincom_backup_region(POSD_RECT frame,UINT8 *backBuf)
{
	UINT32 vscr_idx,vscr_size;
	PGUI_VSCR apVscr;

	if(backBuf==NULL)
		return FALSE;
	
	vscr_idx = osal_task_get_current_id();
	apVscr = OSD_GetTaskVscr(vscr_idx);
	OSD_UpdateVscr(apVscr);

//	vscr_size = frame->uWidth*frame->uHeight*2;
//	backup_region_buf = (UINT8*)MALLOC(vscr_size);
//	if(backup_region_buf == NULL)
//		return FALSE;

	OSD_RectBackUp(apVscr,backBuf,frame);
	return TRUE;
}

void wincom_restore_region(POSD_RECT frame,UINT8 *backBuf,BOOL freeBuf)
{
	UINT32 vscr_idx;
	PGUI_VSCR apVscr;
	
	vscr_idx = osal_task_get_current_id();
	apVscr = OSD_GetTaskVscr(vscr_idx);
	if(backBuf != NULL)
	{
		//soc_printf("%x__%x__%x__%x.\n",frame->uStartX,frame->uStartY,frame->uWidth,frame->uHeight);
		OSD_RectRestore(apVscr,backBuf,frame);
		if(freeBuf)
		{
			FREE(backBuf);
		}
	}
}
#else
BOOL wincom_backup_region(POSD_RECT frame, lpVSCR pvscr)
{
	UINT8* pbuf;
	UINT32 vscr_idx,vscr_size;
	lpVSCR apVscr;
	
	pbuf = NULL;
	vscr_idx = osal_task_get_current_id();
	apVscr = OSD_GetTaskVscr(vscr_idx);
	OSD_UpdateVscr(apVscr);
	apVscr->lpbScr = NULL;

	vscr_size = ( ((frame->uWidth)>>FACTOR )+ 1)*frame->uHeight;
	MEMSET(pvscr, 0, sizeof(*pvscr));
	pbuf = (UINT8*)MALLOC(vscr_size);
	if(NULL == pbuf)
		return FALSE;

	OSD_SetRect2(&pvscr->vR, frame);
	pvscr->lpbScr = pbuf;
	OSD_GetRegionData(pvscr, &pvscr->vR);
	return TRUE;
}

void wincom_restore_region(lpVSCR pvscr)
{
	UINT32 vscr_idx;
	lpVSCR apVscr;
	
	vscr_idx = osal_task_get_current_id();
	apVscr = OSD_GetTaskVscr(vscr_idx);
	if(pvscr->lpbScr)
	{
		OSD_RegionWrite(pvscr, &pvscr->vR);
		FREE(pvscr->lpbScr);
	}
	apVscr->lpbScr = NULL;
}
#endif
#define SCROLL_DELAY_CNT	10		//Delay count for starting scroll
#define SCROLL_END_GAP		(1*24)	//means width of two fonts.

static INT8 delay_preset = SCROLL_DELAY_CNT;
static INT8 delay_cnt = 0;
static BOOL scroll_pause = FALSE;

void wincom_scroll_set_delay_cnt(INT8 cnt)
{
	delay_preset = cnt;
}

void wincom_scroll_textfield_pause(void)
{
	scroll_pause = TRUE;
}

void wincom_scroll_textfield_resume(void)
{
	scroll_pause = FALSE;
}

void wincom_scroll_textfield_stop(PTEXT_FIELD txt)
{
	txt->bAlign &= ~C_ALIGN_SCROLL;
	txt->bX = 0;
	delay_cnt = 0;
}

BOOL wincom_scroll_get_over(PTEXT_FIELD txt)
{
	if (delay_cnt >= delay_preset && txt->bX == 0)
		return TRUE;
	else
		return FALSE;
}

void wincom_scroll_textfield(PTEXT_FIELD txt)
{
	UINT16 str_width = 0,mw = 0,mh = 0;
	UINT8 *str = NULL;

	if (scroll_pause == TRUE)
		return;

	if (txt == NULL)
		return;

	if (txt->pString == NULL)
	{
		str = OSD_GetUnicodeString(txt->wStringID);        
	}
	else
	{
		str = (UINT8 *)txt->pString;
	}

	str_width = OSD_MultiFontLibStrMaxHW(str, ((POBJECT_HEAD)txt)->bFont, &mw, &mh, 0);
	if (str_width > txt->head.frame.uWidth)
	{
		if (txt->bX == 0)
		{
			if (delay_cnt < delay_preset)
			{
				delay_cnt++;
				return;
			}
		}

		txt->bAlign |= C_ALIGN_SCROLL;
		
		if ((txt->bX + txt->head.frame.uWidth) > (str_width + SCROLL_END_GAP))
		{
			if (delay_cnt)
			{
				delay_cnt--;
			}
			else
			{
				txt->bX = 0;
				delay_cnt = 0;
				OSD_TrackObject((POBJECT_HEAD)txt, C_UPDATE_ALL);
				///OSD_TrackObject(&epg_sch_item3, C_UPDATE_ALL);
			}
		}
		else
		{
			txt->bX++;
			OSD_TrackObject((POBJECT_HEAD)txt, C_UPDATE_ALL);
			///OSD_TrackObject(&epg_sch_item3, C_UPDATE_ALL);
		}

		txt->bAlign &= ~C_ALIGN_SCROLL;
	}
}

void Draw_pop_info(win_popup_type_t msg_type,UINT16 string_id)
{
	UINT8 back_save = 0;

	win_compopup_init(msg_type);
	win_compopup_set_msg(NULL, NULL, string_id);
	win_compopup_open();

	//win_compopup_init ( msg_type );
	//win_compopup_set_frame ( 200, 200, 200, 100 );
	//win_compopup_set_msg ( NULL, NULL, string_id );
	//win_compopup_open_ext(&back_save);
	osal_task_sleep ( 1000 );
	//win_compopup_close();  
	
}

/* hex increase the value in specified pos of a unicode string
 * we support low case & upper case for paramer input
 * but the output default is upper case
 */
void increase_val_inuncode(UINT16 *ptr_base,UINT32 pos)
{
	char c;

	c = GetUniStrCharAt(ptr_base,pos);
	c++;
	if(c==103 || c==71)
		c = 48;
	else if(c==58)
		c = 65;
	else
 		c = c;

	SetUniStrCharAt(ptr_base,c,pos);

}

void decrease_val_inuncode(UINT16 *ptr_base,UINT32 pos)
{
	char c;

	c = GetUniStrCharAt(ptr_base,pos);
	c--;
	
	if(c==47)	
		c = 70;
	else if(c==64 || c==96)
		c = 57;
	else
		c = c;

	SetUniStrCharAt(ptr_base,c,pos);

}

BOOL is_larger_than_1m(date_time * stime,date_time * etime)
{
	if(stime->year != etime->year)  return TRUE;
	if(stime->month != etime->month)  return TRUE;
	if(stime->day != etime->day)  return TRUE;
	if(stime->hour != etime->hour)  return TRUE;
	if((stime->min > etime->min) && (stime->min - etime->min >= 1))
		return TRUE;
	if((stime->min < etime->min) && (etime->min - stime->min >= 1))
		return TRUE;

	return FALSE;
		
}

BOOL getVolumeState()
{
	return VolumeOn;
}
void setVolumeOn(BOOL input)
{
	VolumeOn=input;
}


BOOL group_get_name(UINT8 grp_idx, UINT16 *name)
{
	BOOL get_ret = FALSE;
	SYSTEM_DATA *sys_data;
	UINT8 *str = NULL;
	UINT16 strID = 0;
	UINT16 channel;
	UINT8 group_type,group_pos;

	get_ret = sys_data_get_cur_mode_group_infor(grp_idx, &group_type, &group_pos, &channel);
	if (get_ret == FALSE)
		return FALSE;

	if (group_type == ALL_GROUP_TYPE)
	{
		if (sys_data_get_cur_chan_mode() == RADIO_CHAN)
			strID = RS_INFO_RADIO;
		else
			strID = RS_INFO_TV;
	}
	else if (group_type == DATA_GROUP_TYPE)
		strID = RS_DATA_BROADCAST;
	else if (group_type == NVOD_GROUP_TYPE)
		strID = RS_NVOD;
	else if (group_type == LOCAL_GROUP_TYPE)
	{// get local group name
		sys_data = sys_data_get();
		str = (UINT8*)&sys_data->local_group_name[group_pos-1][0];
	}
	else if(group_type == FAV_GROUP_TYPE)
		strID = RS_FAVORITE_GROUP;//favlst_ids[group_pos-MAX_GROUP_NUM];
	else
		return FALSE;

	if(str == NULL)
		str = OSD_GetUnicodeString(strID);
	ComUniStrCopyChar((UINT8 *)name, str);
	
	return TRUE;
}

void display_strs_init(UINT8 start, UINT8 num)
{
	UINT8 i = 0;

	for (i = start; i <= (start + num); i++)
	{
		MEMSET(display_strs[i], 0, sizeof(display_strs[i]));
	}
}


