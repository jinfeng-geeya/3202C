#include <sys_config.h>
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
#include <hld/decv/vdec_driver.h>

#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"
#include "win_com.h"
#include "menus_root.h"
#include "copper_common/com_api.h"
#include "win_com_popup.h"
#include "win_com_list.h"

#include "copper_common/com_epg.h"
#include "win_epg.h"
#include "win_signalstatus.h"

//include the header from xform 
#include "win_epg_vega.h"

/*******************************************************************************
 *	Objects definition
 *******************************************************************************/
 
#define LDEF_CONT(root, varCon,nxtObj,l,t,w,h,conobj,style)		\
    DEF_CONTAINER(varCon,root,nxtObj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, style,style,style,style,   \
    NULL,NULL,  \
    conobj, 1,1)
#define LDEF_TXT(root,varTxt,nxtObj,l,t,w,h,sh,align,resID,str)		\
    DEF_TEXTFIELD(varTxt,&root,nxtObj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, sh,sh,sh,sh,   \
    NULL,NULL,  \
    align, 0,0,resID,str)
#define LDEF_BMP(root,varBmp,nxtObj,l,t,w,h,icon)	\
  DEF_BITMAP(varBmp,&root,nxtObj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, NULL,NULL,NULL,NULL,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,icon)

CONTAINER g_win_epg;

///UINT16 my[256];
///UINT16 j=0;

BITMAP epg_cur_prog_bmp;

CONTAINER prg_con;
OBJLIST epg_prg_ol;
SCROLL_BAR epg_prg_scb;

CONTAINER epg_prg_item0;
CONTAINER epg_prg_item1;
CONTAINER epg_prg_item2;
CONTAINER epg_prg_item3;
CONTAINER epg_prg_item4;

TEXT_FIELD epg_prg_idx0;
TEXT_FIELD epg_prg_idx1;
TEXT_FIELD epg_prg_idx2;
TEXT_FIELD epg_prg_idx3;
TEXT_FIELD epg_prg_idx4;

TEXT_FIELD epg_prg_name0;
TEXT_FIELD epg_prg_name1;
TEXT_FIELD epg_prg_name2;
TEXT_FIELD epg_prg_name3;
TEXT_FIELD epg_prg_name4;

//CONTAINER epg_sch_title;
//TEXT_FIELD epg_sch_title_time;
//TEXT_FIELD epg_sch_title_event_name;
TEXT_FIELD epg_sch_weekday;

CONTAINER sch_con;
OBJLIST epg_sch_ol;
SCROLL_BAR epg_sch_scb;

CONTAINER epg_sch_item0;
CONTAINER epg_sch_item1;
CONTAINER epg_sch_item2;
CONTAINER epg_sch_item3;
CONTAINER epg_sch_item4;

BITMAP epg_book_bmp0;
BITMAP epg_book_bmp1;
BITMAP epg_book_bmp2;
BITMAP epg_book_bmp3;
BITMAP epg_book_bmp4;

TEXT_FIELD epg_sch_time0;
TEXT_FIELD epg_sch_time1;
TEXT_FIELD epg_sch_time2;
TEXT_FIELD epg_sch_time3;
TEXT_FIELD epg_sch_time4;

TEXT_FIELD epg_sch_name0;
TEXT_FIELD epg_sch_name1;
TEXT_FIELD epg_sch_name2;
TEXT_FIELD epg_sch_name3;
TEXT_FIELD epg_sch_name4;

TEXT_FIELD epg_grp_name;
BITMAP epg_grp_left_arrow;
BITMAP epg_grp_right_arrow;

static void epg_draw_cur_prg_bmp();
static PRESULT win_epg_unkown_act_proc(VACTION act);
/*static VACTION epg_sch_list_item_con_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT epg_sch_list_item_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION epg_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT epg_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION epg_list_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT epg_sch_list_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);
static PRESULT epg_prg_list_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);*/
static void win_epg_prg_set_display(void);
static UINT16 win_epg_prg_list_load();
static UINT32 win_epg_sch_list_load(BOOL reset_focus);
static void win_epg_update_sch();
static PRESULT win_epg_message_proc(UINT32 msg_type, UINT32 msg_code);
static void win_epg_convert_time(date_time *utc_time, date_time *dest_dt);
static void win_epg_get_time_len(date_time *dt1, date_time *dt2, INT32 *len);
static void epg_set_cur_serice();
static UINT8 EPGEventBookedOrNot(eit_event_info_t *sch_event);
static void epg_change_week_high_light(INT8 offset);
static void epg_draw_sch_week();
static UINT8 epg_get_week_high_light();
static void epg_reset_week_high_light();

/*#define WIN_IDX	  WSTL_COMMON_BACK_2

#define PRG_CON_IDX WSTL_WIN_1
#define SCH_CON_IDX WSTL_WIN_1

#define LIST_BAR_SH_IDX  WSTL_SCROLLBAR_1
#define LIST_BAR_HL_IDX  WSTL_SCROLLBAR_1

#define LIST_BAR_MID_RECT_IDX  	    WSTL_NOSHOW_IDX
#define LIST_BAR_MID_THUMB_IDX  	WSTL_SCROLLBAR_2

#define TXTI_IDX   WSTL_TXT_4
#define TXTI_HL_IDX   WSTL_BUTTON_3//WSTL_TXT_3

#define TITLE_SCH_SH_IDX WSTL_TXT_4

#define SCH_DATE_IDX WSTL_VOLUME_CON_BG

#define SCH_WEEKDAY_SH_IDX WSTL_VOLUME_CON_BG
#define SCH_WEEKDAY_HL_IDX WSTL_BUTTON_1
#define SCH_WEEKDAY_SL_IDX WSTL_VOLUME_CON_BG
#define SCH_WEEKDAY_GRY_IDX WSTL_VOLUME_CON_BG

#define SCH_DETAIL_SH_IDX   WSTL_WIN_2

#define CON_SH_IDX   WSTL_BUTTON_2
#define CON_HL_IDX   WSTL_BUTTON_1
#define CON_SL_IDX   WSTL_BUTTON_1
#define CON_GRY_IDX  WSTL_BUTTON_2

#define LST_IDX		WSTL_NOSHOW_IDX*/

#define PREVIEW_IDX WSTL_MY_EPGRPEW

/*#define W_L     	0
#define W_T     	90
#define W_W		    720
#define W_H     	400*/

#define LSTCON_PRG_L 66
#define LSTCON_PRG_T 98
/*#define LSTCON_PRG_W 364
#define LSTCON_PRG_H 187

#define PRG_NAME_L (LSTCON_PRG_L + 30)
#define PRG_NAME_T (LSTCON_PRG_T + 4)
#define PRG_NAME_W (LSTCON_PRG_W -60)
#define PRG_NAME_H 24

#define GRP_BMP_L	(LSTCON_PRG_L+10)
#define GRP_BMP_T	(LSTCON_PRG_T+8)
#define GRP_BMP_W	20
#define GRP_BMP_H	20*/

#define LSTCON_SCH_L LSTCON_PRG_L
//#define LSTCON_SCH_T 290
#define LSTCON_SCH_W 604
//#define LSTCON_SCH_H 192

#define LST_PRG_L (LSTCON_PRG_L+4)*/
#define LST_PRG_T (LSTCON_PRG_T+32)
/*#define LST_PRG_W (LSTCON_PRG_W-4*2)
#define LST_PRG_H (ITEM_H*5)

#define LST_SCH_L (LSTCON_SCH_L+4)
#define LST_SCH_T (LSTCON_SCH_T+34)
#define LST_SCH_W (LSTCON_SCH_W-4*2)
#define LST_SCH_H (ITEM_H*5)

#define ITEM_L	    LST_PRG_L
#define ITEM_PRG_T	LST_PRG_T
#define ITEM_SCH_T	LST_SCH_T

#define ITEM_PRG_W	(LST_PRG_W-SCB_PRG_W)
#define ITEM_SCH_W	(LST_SCH_W-SCB_SCH_W)*/

#define ITEM_H	        30
#define ITEM_PRG_GAP	0
/*#define ITEM_SCH_GAP	0

#define SCB_PRG_L (ITEM_L + ITEM_PRG_W)
#define SCB_PRG_T LST_PRG_T
#define SCB_PRG_W 12*/
#define SCB_PRG_H (ITEM_H*5+0*4)

/*#define SCB_SCH_L (ITEM_L + ITEM_SCH_W)
#define SCB_SCH_T LST_SCH_T
#define SCB_SCH_W 12
#define SCB_SCH_H (ITEM_H*5+ITEM_SCH_GAP*4)

#define CUR_PRG_BMP_W    24*/
#define CUR_PRG_BMP_H    24
/*#define CUR_PRG_BMP_L    ITEM_L
#define CUR_PRG_BMP_T    0

#define SCH_TITLE_L (LSTCON_SCH_L+5)
#define SCH_TITLE_W (LSTCON_SCH_W-5*2)
#define SCH_TITLE_T LSTCON_SCH_T
#define SCH_TITLE_H 30

#define SCH_DTL_INFO_L  (SCB_SCH_L+SCB_SCH_W)
#define SCH_DTL_INFO_W  (W_W-LSTCON_SCH_W)
#define SCH_DTL_INFO_T  SCB_SCH_T
#define SCH_DTL_INFO_H  SCB_SCH_H

#define ITEM_PRG_IDX_L     50
#define ITEM_PRG_IDX_W     55
#define ITEM_PRG_NAME_L    (ITEM_PRG_IDX_L+ITEM_PRG_IDX_W)
#define ITEM_PRG_NAME_W    140

#define ITEM_SCH_BMP_L     2
#define ITEM_SCH_BMP_W     24   //18
#define ITEM_SCH_TIME_L     26   //20
#define ITEM_SCH_TIME_W     170
#define ITEM_SCH_NAME_L     (ITEM_SCH_TIME_L+ITEM_SCH_TIME_W)
#define ITEM_SCH_NAME_W     (ITEM_SCH_W-ITEM_SCH_TIME_W-46)//(ITEM_SCH_W-ITEM_SCH_TIME_W-40)*/
/*
#define SCH_DATE_L  (ITEM_SCH_TIME_L+70)
#define SCH_DATE_T  (LSTCON_SCH_T+3)
#define SCH_DATE_W  100
#define SCH_DATE_H  24
*/
#define SCH_WEEKDAY_L  (LSTCON_SCH_L+6)
//#define SCH_WEEKDAY_T  (LSTCON_SCH_T+5)
#define SCH_WEEKDAY_W  (LSTCON_SCH_W-6*2)/7
//#define SCH_WEEKDAY_H  24

#define EPG_PREVIEW_L 30///434 26
#define EPG_PREVIEW_T 56  //94  64
#define EPG_PREVIEW_W 272////238 283
#define EPG_PREVIEW_H 194///194

#define PREVIEW_L 41//452  42
#define PREVIEW_T 69///114 69
#define PREVIEW_W 255////255
#define PREVIEW_H 169///168

#define WEEK_DAY_TIEM_CNT 7
/*#define MAX_SCH_INFO_DAY_NUM 7

#define LEFT_BMP(root,varBmp,nxtObj,ID,l,t,w,h,icon,IDX)	\
  DEF_BITMAP(varBmp,&root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, IDX,IDX,IDX,IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER,0,0,icon)


#define LDEF_TITLE_TXT(root,varTxt,nxtObj,l,t,w,h,resID,str)		\
    DEF_TEXTFIELD(varTxt,&root,nxtObj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, SCH_WEEKDAY_SH_IDX,SCH_WEEKDAY_HL_IDX,SCH_WEEKDAY_SL_IDX,SCH_WEEKDAY_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER| C_ALIGN_VCENTER, 0,0,resID,str)

#define LDEF_CON_SCH(root, varCon,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,conobj,focusID)		\
    DEF_CONTAINER(varCon,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    epg_sch_list_item_con_keymap,epg_sch_list_item_con_callback,  \
    conobj, 1,1)

#define LDEF_CONT_SCH_TITLE(root, varCon,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,conobj,focusID)		\
    DEF_CONTAINER(varCon,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, SCH_DETAIL_SH_IDX,SCH_DETAIL_SH_IDX,SCH_DETAIL_SH_IDX,SCH_DETAIL_SH_IDX,   \
    NULL,NULL,  \
    conobj, 1,1)

#define LDEF_CONT(root, varCon,nxtObj,l,t,w,h,conobj,style)		\
    DEF_CONTAINER(varCon,root,nxtObj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, style,style,style,style,   \
    NULL,NULL,  \
    conobj, 1,1)

#define LDEF_TXT1(root,varTxt,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,resID,str,xAlign)		\
    DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, TXTI_IDX,TXTI_HL_IDX,TXTI_HL_IDX,TXTI_IDX,   \
    NULL,NULL,  \
    xAlign  | C_ALIGN_VCENTER, 10,0,resID,str)

#define LDEF_TXT2(root,varTxt,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,resID,str,xAlign)		\
    DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, TITLE_SCH_SH_IDX,TITLE_SCH_SH_IDX,TITLE_SCH_SH_IDX,TITLE_SCH_SH_IDX,   \
    NULL,NULL,  \
    xAlign | C_ALIGN_VCENTER, 10,0,resID,str)

#define LDEF_TXT3(root,varTxt,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,resID,str)		\
    DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, TXTI_IDX,TXTI_HL_IDX,TXTI_HL_IDX,TXTI_IDX,   \
    NULL,NULL,  \
    C_ALIGN_RIGHT | C_ALIGN_VCENTER, 10,0,resID,str)

#define LDEF_TXT4(root,varTxt,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,resID,str,xAlign)		\
    DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, TXTI_IDX,TXTI_HL_IDX,TXTI_HL_IDX,TXTI_IDX,   \
    NULL,NULL,  \
    xAlign  | C_ALIGN_VCENTER |C_ALIGN_SCROLL, 0,0,resID,str)

#define LDEF_TXT_GRP_NAME(root,varTxt,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,resID,str)		\
    DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, WSTL_VOLUME_CON_BG,WSTL_VOLUME_CON_BG,WSTL_VOLUME_CON_BG,WSTL_VOLUME_CON_BG,   \
    NULL,NULL,  \
    C_ALIGN_CENTER| C_ALIGN_VCENTER, 10,0,resID,str)

#define LDEF_BMP_GRP(root,varBmp,nxtObj,l,t,w,h,icon,IDX) \
    	DEF_BITMAP(varBmp,&root,nxtObj,C_ATTR_ACTIVE,0, \
	    0,0,0,0,0, l,t,w,h, IDX,IDX,IDX,IDX,   \
	    NULL,NULL,  \
	    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,icon)*/
/*
#define LDEF_SCH_TITLE(root,varCon,varnxt,varTxt1,varTxt2,ID,l,t,w,h,id1,id2)	\
	LDEF_CONT_SCH_TITLE(&root,varCon,varnxt,ID,ID,ID,ID,ID,l,t,w,h,&varTxt1,1)	\
	LDEF_TXT2(&varCon,varTxt1,&varTxt2 ,0,0,0,0,0,l + ITEM_SCH_TIME_L, t,ITEM_SCH_TIME_W,h,id1,NULL,C_ALIGN_LEFT)	\
	LDEF_TXT2(&varCon,varTxt2,NULL ,1,1,1,1,1,l + ITEM_SCH_NAME_L, t,ITEM_SCH_NAME_W,h,id2,NULL,C_ALIGN_CENTER)
*/
/*#define LDEF_LIST_ITEM_PRG(root,varCon,varTxt1,varTxt2,ID,l,t,w,h,str1,str2)	\
	LDEF_CON_SCH(&root,varCon,NULL,ID,ID,ID,ID,ID,l,t,w,h,&varTxt1,1)	\
	LDEF_TXT3(&varCon,varTxt1,&varTxt2 ,0,0,0,0,0,l + ITEM_PRG_IDX_L, t,ITEM_PRG_IDX_W,h,0,str1)	\
	LDEF_TXT1(&varCon,varTxt2,NULL ,1,1,1,1,1,l + ITEM_PRG_NAME_L, t,ITEM_PRG_NAME_W,h,0,str2,C_ALIGN_CENTER)

#define LDEF_LIST_ITEM_SCH(root,varCon,bmp,varTxt1,varTxt2,ID,l,t,w,h,str1,str2)	\
	LDEF_CON_SCH(&root,varCon,NULL,ID,ID,ID,ID,ID,l,t,w,h,&bmp,1)	\
	LEFT_BMP(varCon,bmp,&varTxt1,0,l + ITEM_SCH_BMP_L, t,ITEM_SCH_BMP_W,h,IM_EPG_COLORBUTTON_BLUE,WSTL_NOSHOW_IDX)\
	LDEF_TXT1(&varCon,varTxt1,&varTxt2 ,0,0,0,0,0,l + ITEM_SCH_TIME_L, t,ITEM_SCH_TIME_W,h,0,str1,C_ALIGN_LEFT)	\
	LDEF_TXT4(&varCon,varTxt2,NULL,1,1,1,1,1,l + ITEM_SCH_NAME_L, t,ITEM_SCH_NAME_W,h,0,str2,C_ALIGN_LEFT)

#define LDEF_LISTBAR(root,varScb,page,l,t,w,h)	\
	DEF_SCROLLBAR(varScb, &root, NULL, C_ATTR_ACTIVE, 0, \
		0, 0, 0, 0, 0, l, t, w, h, LIST_BAR_SH_IDX, LIST_BAR_HL_IDX, LIST_BAR_SH_IDX, LIST_BAR_SH_IDX, \
		NULL, NULL, BAR_VERT_AUTO | SBAR_STYLE_RECT_STYLE, page, LIST_BAR_MID_THUMB_IDX, LIST_BAR_MID_RECT_IDX, \
		0, 10, w, h - 20, 100, 1)

#define LDEF_OL(root,varOl,nxtObj,id,l,t,w,h,style,dep,count,flds,sb,mark,selary,callback)	\
  DEF_OBJECTLIST(varOl,&root,nxtObj,C_ATTR_ACTIVE,0, \
    id,id,id,id,id, l,t,w,h,LST_IDX,LST_IDX,LST_IDX,LST_IDX,   \
    epg_list_keymap,callback,    \
    flds,sb,mark,style,dep,count,selary)

#define LDEF_MTXT(root,varMtxt,nextObj,ID,l,t,w,h,sh,cnt,sb,content)	\
	DEF_MULTITEXT(varMtxt,&root,nextObj,C_ATTR_ACTIVE,0, \
    	ID,ID,ID,ID,ID, l,t,w,h, sh,sh,sh,sh,   \
	    NULL,NULL,  \
    	C_ALIGN_LEFT | C_ALIGN_TOP, cnt,  4,4,w-8,h-8,sb,content)

#define LDEF_WIN(varCon,nxtObj,l,t,w,h,focusID)		\
    DEF_CONTAINER(varCon,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_IDX,WIN_IDX,WIN_IDX,WIN_IDX,   \
    epg_keymap,epg_callback,  \
    nxtObj, focusID,0)

LEFT_BMP ( epg_prg_ol, epg_cur_prog_bmp, NULL, 	0, CUR_PRG_BMP_L,\
        CUR_PRG_BMP_T, CUR_PRG_BMP_W, CUR_PRG_BMP_H, IM_EPG_COLORBUTTON_BLUE,WSTL_TXT_4)

LDEF_TITLE_TXT ( g_win_epg, epg_sch_weekday, NULL,SCH_WEEKDAY_L, SCH_WEEKDAY_T, SCH_WEEKDAY_W, SCH_WEEKDAY_H, 0, NULL)

LDEF_LIST_ITEM_PRG ( epg_prg_ol, epg_prg_item0, epg_prg_idx0, epg_prg_name0,1, \
                 ITEM_L, ITEM_PRG_T + ( ITEM_H + ITEM_PRG_GAP ) *0, ITEM_PRG_W, ITEM_H,\
                 display_strs[15], display_strs[16])

LDEF_LIST_ITEM_PRG ( epg_prg_ol, epg_prg_item1, epg_prg_idx1, epg_prg_name1,2, \
                 ITEM_L, ITEM_PRG_T + ( ITEM_H + ITEM_PRG_GAP ) *1, ITEM_PRG_W, ITEM_H,\
                 display_strs[18], display_strs[19])

LDEF_LIST_ITEM_PRG ( epg_prg_ol, epg_prg_item2, epg_prg_idx2, epg_prg_name2,3, \
                 ITEM_L, ITEM_PRG_T + ( ITEM_H + ITEM_PRG_GAP ) *2, ITEM_PRG_W, ITEM_H,\
                 display_strs[21], display_strs[22])

LDEF_LIST_ITEM_PRG ( epg_prg_ol, epg_prg_item3, epg_prg_idx3, epg_prg_name3,4, \
                 ITEM_L, ITEM_PRG_T + ( ITEM_H + ITEM_PRG_GAP ) *3, ITEM_PRG_W, ITEM_H,\
                 display_strs[24], display_strs[25])

LDEF_LIST_ITEM_PRG ( epg_prg_ol, epg_prg_item4, epg_prg_idx4, epg_prg_name4,5, \
                 ITEM_L, ITEM_PRG_T + ( ITEM_H + ITEM_PRG_GAP ) *4, ITEM_PRG_W, ITEM_H,\
                 display_strs[27], display_strs[28])

LDEF_LISTBAR ( epg_prg_ol, epg_prg_scb, 5, SCB_PRG_L, SCB_PRG_T, SCB_PRG_W, SCB_PRG_H )*/

/*
LDEF_SCH_TITLE ( g_win_epg, epg_sch_title,&epg_sch_ol,epg_sch_title_time,\
                 epg_sch_title_event_name,1,\
                 0, SCH_TITLE_T, W_W, SCH_TITLE_H,\
                 RS_SYSTEM_TIME, RS_SYSTEM_TIMERSET_CHANNEL)
*/

/*LDEF_LIST_ITEM_SCH ( epg_sch_ol, epg_sch_item0, epg_book_bmp0,epg_sch_time0, epg_sch_name0, 1, \
                 ITEM_L, ITEM_SCH_T + ( ITEM_H + ITEM_SCH_GAP ) *0, ITEM_SCH_W, ITEM_H,\
                 display_strs[5], display_strs[6])

LDEF_LIST_ITEM_SCH ( epg_sch_ol, epg_sch_item1, epg_book_bmp1,epg_sch_time1, epg_sch_name1,2, \
                 ITEM_L, ITEM_SCH_T + ( ITEM_H + ITEM_SCH_GAP ) *1, ITEM_SCH_W, ITEM_H,\
                 display_strs[7], display_strs[8])

LDEF_LIST_ITEM_SCH ( epg_sch_ol, epg_sch_item2, epg_book_bmp2,epg_sch_time2, epg_sch_name2,3, \
                 ITEM_L, ITEM_SCH_T + ( ITEM_H + ITEM_SCH_GAP ) *2, ITEM_SCH_W, ITEM_H,\
                 display_strs[9], display_strs[10])

LDEF_LIST_ITEM_SCH ( epg_sch_ol, epg_sch_item3, epg_book_bmp3,epg_sch_time3, epg_sch_name3,4, \
                 ITEM_L, ITEM_SCH_T + ( ITEM_H + ITEM_SCH_GAP ) *3, ITEM_SCH_W, ITEM_H,\
                 display_strs[11], display_strs[12])

LDEF_LIST_ITEM_SCH ( epg_sch_ol, epg_sch_item4, epg_book_bmp4,epg_sch_time4, epg_sch_name4,5, \
                 ITEM_L, ITEM_SCH_T + ( ITEM_H + ITEM_SCH_GAP ) *4, ITEM_SCH_W, ITEM_H,\
                 display_strs[13], display_strs[14])

LDEF_LISTBAR ( epg_sch_ol, epg_sch_scb, 5, SCB_SCH_L, SCB_SCH_T, SCB_SCH_W, SCB_SCH_H )*/

POBJECT_HEAD epg_prg_items[] =
{
    ( POBJECT_HEAD ) &epg_prg_item0,
    ( POBJECT_HEAD ) &epg_prg_item1,
    ( POBJECT_HEAD ) &epg_prg_item2,
    ( POBJECT_HEAD ) &epg_prg_item3,
    ( POBJECT_HEAD ) &epg_prg_item4,
    ( POBJECT_HEAD ) &epg_prg_item5,
    ( POBJECT_HEAD ) &epg_prg_item6,
};

POBJECT_HEAD epg_sch_items[] =
{
    ( POBJECT_HEAD ) &epg_sch_item0,
    ( POBJECT_HEAD ) &epg_sch_item1,
    ( POBJECT_HEAD ) &epg_sch_item2,
    ( POBJECT_HEAD ) &epg_sch_item3,
    ( POBJECT_HEAD ) &epg_sch_item4,
    ( POBJECT_HEAD ) &epg_sch_item5,
    ( POBJECT_HEAD ) &epg_sch_item6,
    ( POBJECT_HEAD ) &epg_sch_item7,
    ( POBJECT_HEAD ) &epg_sch_item8,
    ( POBJECT_HEAD ) &epg_sch_item9,
    ( POBJECT_HEAD ) &epg_sch_item10,
    ( POBJECT_HEAD ) &epg_sch_item11,
    ( POBJECT_HEAD ) &epg_sch_item12,
    ( POBJECT_HEAD ) &epg_sch_item13,
};

POBJECT_HEAD epg_sch_names[] =
{
    ( POBJECT_HEAD ) &epg_sch_name0,
    ( POBJECT_HEAD ) &epg_sch_name1,
    ( POBJECT_HEAD ) &epg_sch_name2,
    ( POBJECT_HEAD ) &epg_sch_name3,
    ( POBJECT_HEAD ) &epg_sch_name4,
    ( POBJECT_HEAD ) &epg_sch_name5,
    ( POBJECT_HEAD ) &epg_sch_name6,
    ( POBJECT_HEAD ) &epg_sch_name7,
    ( POBJECT_HEAD ) &epg_sch_name8,
    ( POBJECT_HEAD ) &epg_sch_name9,
    ( POBJECT_HEAD ) &epg_sch_name10,
    ( POBJECT_HEAD ) &epg_sch_name11,
    ( POBJECT_HEAD ) &epg_sch_name12,
    ( POBJECT_HEAD ) &epg_sch_name13,
};

/*#define LIST_STYLE (LIST_VER | LIST_NO_SLECT     | LIST_ITEMS_NOCOMPLETE |  LIST_SCROLL | LIST_GRID | LIST_FOCUS_FIRST | LIST_PAGE_KEEP_CURITEM | LIST_FULL_PAGE)

LDEF_OL(g_win_epg, epg_sch_ol, NULL, 2, LST_SCH_L, LST_SCH_T, LST_SCH_W, LST_SCH_H, LIST_STYLE, 5, 10,  \
	epg_sch_items, &epg_sch_scb, NULL, NULL, epg_sch_list_callback)

LDEF_CONT(&g_win_epg, sch_con, &epg_sch_ol, LSTCON_SCH_L, LSTCON_SCH_T, LSTCON_SCH_W, LSTCON_SCH_H, NULL, SCH_CON_IDX)

LDEF_OL(g_win_epg, epg_prg_ol, &sch_con, 1, LST_PRG_L, LST_PRG_T, LST_PRG_W, LST_PRG_H, LIST_STYLE, 5, 6,  \
	epg_prg_items, &epg_prg_scb, NULL, NULL, epg_prg_list_callback)

LDEF_CONT(&g_win_epg, prg_con, &epg_prg_ol, LSTCON_PRG_L, LSTCON_PRG_T, LSTCON_PRG_W, LSTCON_PRG_H, &epg_grp_left_arrow, PRG_CON_IDX)

LDEF_TXT_GRP_NAME(NULL,epg_grp_name,NULL ,1,1,1,1,1,\
        PRG_NAME_L,PRG_NAME_T,PRG_NAME_W,PRG_NAME_H,0,display_strs[4])

LDEF_BMP_GRP(prg_con, epg_grp_left_arrow, &epg_grp_right_arrow, \
	GRP_BMP_L, GRP_BMP_T, GRP_BMP_W, GRP_BMP_H, IM_GROUP_LEFT, WSTL_TXT_4)
	
LDEF_BMP_GRP(prg_con, epg_grp_right_arrow, NULL, \
	GRP_BMP_L + GRP_BMP_W + PRG_NAME_W, GRP_BMP_T, GRP_BMP_W, GRP_BMP_H, IM_GROUP_RIGHT, WSTL_TXT_4)

LDEF_WIN(g_win_epg, &prg_con, W_L, W_T, W_W, W_H, 1)*/

/*******************************************************************************
 *	Local vriable & function declare
 *******************************************************************************/
static UINT8 last_valid_grp_idx = 0;	/* Last valid group index */
static UINT8 cur_chan_grp_idx = 0; /* Current channel group index */
static UINT8 exit_grp_idx = 0;
static void win_epg_sch_set_display(void);
static UINT16 win_epg_prg_list_load();
static void win_epg_update_time();

static UINT8 week_day_high_light = 0,first_week_day = 0;
static UINT8 pre_sch_cnt = 0;

extern UINT16 weekday_id[];
extern UINT8 show_and_playchannel;

#define isinragne(i,low,high) 	((i)>=(low) && (i)<(high))
#define calc_wt(i)  (UINT32)((i.hour)*3600 + (i.min)*60 + i.sec)
/*******************************************************************************
 *	key mapping and event callback definition
 *******************************************************************************/
static VACTION epg_sch_list_item_con_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

	return act;
}

static PRESULT epg_sch_list_item_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;

	return ret;
}

#define VACT_FAV_EDIT		(VACT_PASS + 1)
#define VACT_CHANGE_FOCUS	(VACT_PASS + 2)
#define VACT_MUTE 			(VACT_PASS + 3)
#define VACT_PAUSE 			(VACT_PASS + 4)
#define VACT_TIMER 			(VACT_PASS + 5)
#define VACT_GRP_DECREASE  (VACT_PASS + 6)
#define VACT_GRP_INCREASE	(VACT_PASS + 7)
#define VACT_SCH_DAY_DEC	(VACT_PASS + 8)
#define VACT_SCH_DAY_INC	(VACT_PASS + 9)
#define VACT_TV_RADIO_SW	(VACT_PASS + 10)

static VACTION epg_list_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;
#ifdef MULTI_CAS
#if(CAS_TYPE==CAS_IRDETO)
			if(getStopChannelChange()&&(key==V_KEY_UP||key==V_KEY_DOWN||key==V_KEY_P_UP||key==V_KEY_P_DOWN))//check whether stop channel change
				return act;
#endif
#endif
	switch (key)
	{
		case V_KEY_UP:
			act = VACT_CURSOR_UP;
			break;
		case V_KEY_DOWN:
			act = VACT_CURSOR_DOWN;
			break;
		case V_KEY_P_UP:
			act = VACT_CURSOR_PGUP;
			break;
		case V_KEY_P_DOWN:
			act = VACT_CURSOR_PGDN;
			break;
		case V_KEY_RED:
			act = VACT_ENTER;
			break;
		case V_KEY_ENTER:
			act = VACT_TIMER;
			break;
		case V_KEY_BLUE:
		case V_KEY_YELLOW:
			if (OSD_GetContainerFocus(&g_win_epg) == 1)
			{
				act = (key == V_KEY_BLUE) ? VACT_GRP_INCREASE : VACT_GRP_DECREASE;
			}
			else
			{
				act = (key == V_KEY_YELLOW) ? VACT_SCH_DAY_INC : VACT_SCH_DAY_DEC;
			}
			break;
            case V_KEY_TVRADIO:
                    act = VACT_TV_RADIO_SW;
                    break;
                
		default:
			act = VACT_PASS;
	}

	return act;
}


static PRESULT epg_prg_list_unkown_act_proc(VACTION act)
{
	PRESULT ret = PROC_PASS;
	UINT8 grp_cnt, grp_idx, av_mode, back_saved;
	UINT16 sel;
	OBJLIST *ol = &epg_prg_ol;;
	UINT16 channel, strID;
	UINT8 cur_grp_idx;

	if ((act == VACT_GRP_DECREASE) || (act == VACT_GRP_INCREASE))
	{
		cur_grp_idx = sys_data_get_cur_group_index();
		grp_idx = sys_data_cur_group_inc_dec((act == VACT_GRP_INCREASE), 0);
		if (cur_grp_idx != grp_idx)
		{
			sys_data_change_group(grp_idx);
			cur_grp_idx = grp_idx;
			
			channel = sys_data_get_cur_group_cur_mode_channel();
			if (channel != P_INVALID_ID)
			{
				api_play_channel(channel, TRUE, TRUE, FALSE);
				epg_set_cur_serice();
				last_valid_grp_idx = grp_idx;
			}

			///Draw_localGrp_Name((TEXT_FIELD *)&epg_grp_name,cur_grp_idx);
			//group_get_name(grp_idx, OSD_GetTextFieldStrPoint(&epg_grp_name));
			//OSD_DrawObject((POBJECT_HEAD)&epg_grp_name, C_UPDATE_ALL);
			win_epg_prg_list_load();
			sel = OSD_GetObjListCurPoint(ol);
			OSD_TrackObject((POBJECT_HEAD)ol, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
			win_epg_sch_list_load(FALSE);
			////epg_draw_sch_week();
			OSD_DrawObject((POBJECT_HEAD) &epg_sch_ol, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
		}
		/*else
		{
			OSD_TrackObject((POBJECT_HEAD)ol, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
		}
		*/
	}
       else if (VACT_TV_RADIO_SW == act)
       {
            av_mode = sys_data_get_cur_chan_mode();
		av_mode = (av_mode == TV_CHAN) ? RADIO_CHAN : TV_CHAN;
		//sys_data_set_cur_intgroup_index(0); /*force to return all group*/
		sys_data_get_group_channel(0, &channel, av_mode);
		if (channel == P_INVALID_ID)
		{	
			 /* If the opposite mode has no channel */
			win_compopup_init(WIN_POPUP_TYPE_SMSG);
			if(av_mode == TV_CHAN)
			{
				win_compopup_set_msg(NULL, NULL, RS_MSG_NO_PROGRAM_TV);
			}
			else
			{
				win_compopup_set_msg(NULL, NULL, RS_MSG_NO_RADIO_CHANNEL);
			}
			win_compopup_open_ext(&back_saved);
			osal_task_sleep(1000);
			win_compopup_smsg_restoreback();
		}
		else
		{
		       UINT16 cur_channel, max_channel;
                	P_NODE p_node;
                	UINT32 n;
                	SYSTEM_DATA *sys_data;
                	UINT8 cur_chan_mode;
                	BOOL b_ret;
                    
			sys_data_set_cur_chan_mode(av_mode);			

                	sys_data = sys_data_get();

                	sys_data_change_group(0);
                	
                	cur_chan_mode = sys_data_get_cur_chan_mode();
                	cur_channel = sys_data_get_cur_group_cur_mode_channel();
                	max_channel = get_prog_num(VIEW_ALL | cur_chan_mode, 0);
                	if (cur_channel >= max_channel)
                		cur_channel = 0;

                	n = 0;
                	get_prog_at(cur_channel, &p_node);
                	while ( ( p_node.skip_flag
                	          || ( sys_data->chan_sw == CHAN_SWITCH_FREE && p_node.ca_mode )
                	          || ( sys_data->chan_sw == CHAN_SWITCH_SCRAMBLED && !p_node.ca_mode ) )
                	        && n != max_channel )
                	{
                		cur_channel = (cur_channel + 1+max_channel) % max_channel;
                		get_prog_at(cur_channel, &p_node);
                		n++;
                	};
                    api_play_channel(cur_channel, TRUE, TRUE, FALSE);
                    epg_set_cur_serice();
                   /// Draw_localGrp_Name((TEXT_FIELD *)&epg_grp_name, cur_chan_grp_idx);
			//group_get_name(grp_idx, OSD_GetTextFieldStrPoint(&epg_grp_name));
			//OSD_DrawObject((POBJECT_HEAD)&epg_grp_name, C_UPDATE_ALL);
			win_epg_prg_list_load();
			sel = OSD_GetObjListCurPoint(ol);
			OSD_TrackObject((POBJECT_HEAD)ol, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
			win_epg_sch_list_load(FALSE);
			////epg_draw_sch_week();
			OSD_DrawObject((POBJECT_HEAD) &epg_sch_ol, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
		}
       }
#if 0
	else if (VACT_TV_RADIO_SW == act)
	{
		av_mode = (av_mode == TV_CHAN) ? RADIO_CHAN : TV_CHAN;
		sys_data_get_cur_group_channel(&channel, av_mode);
		if (channel == P_INVALID_ID)
		 /* If the opposite mode has no channel */
		{
			if (av_mode == RADIO_CHAN)
				strID = RS_MSG_NO_RADIO_CHANNEL;
			else
				strID = RS_MSG_NO_TV_CHANNEL;
			win_compopup_init(WIN_POPUP_TYPE_SMSG);
			win_compopup_set_msg(NULL, NULL, strID);
			win_compopup_open_ext(&back_saved);
			osal_task_sleep(500);
			win_compopup_smsg_restoreback();
		}
		else
		{
			title_str_id = (title_str_id == RS_DIGITAL_TV)? RS_RADIO : RS_DIGITAL_TV;
			title_bmp_id = (title_bmp_id == IM_TITLE_ICON_TV)? IM_TITLE_ICON_RADIO : IM_TITLE_ICON_TV;
			wincom_open_title_ext(title_str_id,title_bmp_id);
			sys_data_set_cur_chan_mode(av_mode);
			cur_chan_grp_idx = sys_data_get_cur_group_index();
			chanlist_load_group();
			sel = OSD_GetObjListCurPoint(ol);
			api_play_channel(sel, TRUE, TRUE, FALSE);
			OSD_TrackObject((POBJECT_HEAD)ol, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
			DrawGrpName((TEXT_FIELD *)&chanlist_grp_name,cur_chan_grp_idx);
		}
	}
#endif

	return ret;
}


static PRESULT epg_prg_list_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	VACTION unact;
	UINT16 uni_name[30];
	UINT8 *new_name;
	POBJLIST ol = &epg_prg_ol;
	POBJLIST schol = &epg_sch_ol;
	UINT16 curitem;
	P_NODE p_node;
	enum API_PLAY_TYPE ret_enum;
	SYSTEM_DATA *sys_data = sys_data_get();

	curitem = OSD_GetObjListNewPoint(ol);

	switch (event)
	{
		case EVN_PRE_DRAW:
			win_epg_prg_set_display();
			break;
		case EVN_ITEM_POST_CHANGE:
			curitem = OSD_GetObjListNewPoint((OBJLIST*)pObj);
			epg_set_cur_serice();
			SetPauseStatus(FALSE);
			if (get_prog_at((UINT16)curitem, &p_node) != SUCCESS)
			{
				ret = PROC_LOOP;
				break;
			}
			/*
			if(p_node.lock_flag)
			{
				// draw blank
				OSD_SetObjListCount(schol, 0);
				OSD_DrawObject((POBJECT_HEAD)schol, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
			}
*/
			ret_enum = api_play_channel(curitem, TRUE, TRUE, FALSE);

			/* Draw epg info for non-blocked and normal played program*/
			if(!p_node.lock_flag || API_PLAY_NORMAL == ret_enum)
			{   
				/* Every time when we change the channel, we reset the sch to init status and refesh it */
				win_epg_sch_list_load(TRUE);
				pre_sch_cnt = OSD_GetObjListCount(schol);
				////epg_draw_sch_week();
				OSD_DrawObject((POBJECT_HEAD)schol, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
			}
			break;
		case EVN_UNKNOWN_ACTION:
			unact = (VACTION)(param1 >> 16);
			epg_prg_list_unkown_act_proc(unact);
			break;
	}

	return ret;
}

static void epg_set_cur_serice()
{
	struct ACTIVE_SERVICE_INFO service;
	P_NODE pNode;
	POBJLIST ol = &epg_prg_ol;
	UINT16 curitem;

	curitem = OSD_GetObjListNewPoint(ol);

	if (SUCCESS == get_prog_at(curitem, &pNode))
	{
		service.tp_id = pNode.tp_id;
		service.service_id = pNode.prog_number;
		epg_set_active_service(&service, 1);
	}

}

static PRESULT epg_sch_list_unkown_act_proc(VACTION act)
{
	PRESULT ret =PROC_PASS;
	UINT16 uni_name[30];
	UINT8 *new_name;
	POBJLIST ol = &epg_prg_ol;
	POBJLIST schol = &epg_sch_ol;
	UINT16 curitem, channel, top;
	SYSTEM_DATA *sys_data = sys_data_get();
	eit_event_info_t *sch_event = NULL;
	struct winepginfo current_hl_item;
	INT8 timer_num;
	TIMER_SET_CONTENT timer, rettimer,*conflict_timer;
	UINT8 back_saved;
	INT32 duration;
	P_NODE p_node;
	date_time dt,dt_now;
	

	curitem = OSD_GetObjListCurPoint(schol);
	top = OSD_GetObjListTop(schol);
	channel = OSD_GetObjListCurPoint(ol);
	
	sch_event = epg_get_schedule_event(curitem);

	if (act == VACT_ENTER)
	{
		if (NULL == sch_event)
			{
			///int m=10;
			///OSD_TrackObject((POBJECT_HEAD) &g_win_epg, C_UPDATE_ALL);
			return PROC_PASS;

		}

		get_event_start_time(sch_event, &current_hl_item.start);
		get_event_end_time(sch_event, &current_hl_item.end);
		current_hl_item.event_idx = sch_event->event_id;
		win_epg_detail_open(channel, &current_hl_item);

		win_epg_sch_list_load(FALSE);
		OSD_TrackObject((POBJECT_HEAD) &g_win_epg, C_UPDATE_ALL);
	///epg_draw_cur_prg_bmp();
       //// epg_draw_sch_week();
        ///Draw_localGrp_Name((TEXT_FIELD *)&epg_grp_name,cur_chan_grp_idx);
		wincom_draw_preview_frame();        
	}
	else if (act == VACT_TIMER)
	{
		if (NULL == sch_event)
			return PROC_PASS;
		
		UINT8 timerIDX = EPGEventBookedOrNot(sch_event);
		if (timerIDX != 0)
		{
		
			sys_data->timer_set.TimerContent[timerIDX - 1].timer_mode = TIMER_MODE_OFF;
			win_epg_sch_set_display();
			OSD_TrackObject((POBJECT_HEAD)schol, C_UPDATE_ALL);

			return PROC_PASS;
		}
		
		timer_num = find_available_timer();
		if (timer_num == 0)
		{
			//not available timer
			win_compopup_init(WIN_POPUP_TYPE_SMSG);
			win_compopup_set_msg(NULL, NULL, RS_MSG_EPG_TIMER_IS_FULL);
			win_compopup_open_ext(&back_saved);
			osal_task_sleep(2000);
			win_compopup_smsg_restoreback();
			return PROC_PASS;
		}

		timer.timer_mode = TIMER_MODE_ONCE;
		timer.timer_service = TIMER_SERVICE_CHANNEL;

		get_prog_at(channel, &p_node);
		timer.wakeup_channel = p_node.prog_id;

		timer.wakeup_chan_mode = sys_data_get_cur_chan_mode();
		timer.wakeup_group_idx = sys_data_get_cur_group_index();
		timer.wakeup_state = TIMER_STATE_READY;
		timer.wakeup_message = TIMER_MSG_BIRTHDAY;

		sch_event = epg_get_schedule_event(curitem);
		get_event_start_time(sch_event, &current_hl_item.start);
		get_event_end_time(sch_event, &current_hl_item.end);
		//current_hl_item.event_idx = sch_event->event_id;

		win_epg_convert_time(&current_hl_item.start, &dt);
		timer.wakeup_year = dt.year;
		timer.wakeup_month = dt.month;
		timer.wakeup_day = dt.day;
		timer.wakeup_time = dt.hour * 3600+dt.min * 60+dt.sec;
		MEMCPY(timer.event_name,(UINT8 *)&p_node.service_name[0],40);

		win_epg_get_time_len(&current_hl_item.start, &current_hl_item.end, &duration);
		if (duration == 0)
			duration = 1;
		//The least duration time is one sec.
		timer.wakeup_duration_time = (UINT16)duration;

		/*check is that the start time is in future*/
		get_local_time(&dt_now);
		if(eit_compare_time(&dt,&dt_now)>0)
		{
			win_compopup_init(WIN_POPUP_TYPE_OK);
			win_compopup_set_msg(NULL, NULL, RS_MSG_INVALID_INPUT_CONTINUE);
			win_compopup_open_ext(&back_saved);
			return PROC_PASS;
		}
		sys_data = sys_data_get();
		sys_data->timer_set.timer_num = timer_num;
		//if (win_timerset_open(&timer, &rettimer, TRUE))
		INT32 con_timer=IsValidTimer(&timer);
		if(0==con_timer)
		{
			///my[j]=channel;
			///j++;
			MEMCPY(&sys_data->timer_set.TimerContent[timer_num - 1], &timer, sizeof(TIMER_SET_CONTENT));
			//win_epg_sch_set_display();
		}else
		{
            UINT16 book_msg[100];
	        UINT16 str_len;
	        UINT8 *s,str[5];
            win_popup_choice_t choice;
            win_compopup_init(WIN_POPUP_TYPE_OKNO);
			s = OSD_GetUnicodeString(CS_MSG_EPG_REPLACE_TIMER1);
			ComUniStrCopyChar((UINT8 *)&book_msg[0], s);
			str_len = ComUniStrLen(book_msg);
			ComUniStrCopyChar((UINT8*)&book_msg[str_len], sys_data->timer_set.TimerContent[con_timer - 1].event_name);	
			str_len = ComUniStrLen(book_msg);

			s = OSD_GetUnicodeString(CS_MSG_EPG_REPLACE_TIMER2);
			ComUniStrCopyChar((UINT8 *)&book_msg[str_len], s);

			win_compopup_set_msg(NULL, (UINT8*)book_msg, 0);
			win_compopup_set_frame(160,172,400,200);
			choice=win_compopup_open_ext(&back_saved);
			if(choice==WIN_POP_CHOICE_YES)
			{
				MEMCPY(&sys_data->timer_set.TimerContent[con_timer - 1], &timer, sizeof(TIMER_SET_CONTENT));
				sys_data->timer_set.timer_num = timer_num-1;
			}
		}
        sys_data_save(0);
//resort the timer order according to the time
		api_timer_resort();

//		OSD_TrackObject((POBJECT_HEAD) &g_win_epg, C_UPDATE_ALL);
//		epg_draw_cur_prg_bmp();
        win_epg_sch_set_display();
        OSD_TrackObject((POBJECT_HEAD)schol, C_UPDATE_ALL);
		ret =PROC_LOOP;
//		epg_draw_cur_prg_bmp();        
//		epg_draw_sch_week();
//        Draw_localGrp_Name((TEXT_FIELD *)&epg_grp_name,cur_chan_grp_idx);
//		wincom_draw_preview_frame();
	}
	else if ((act == VACT_SCH_DAY_DEC) || (act == VACT_SCH_DAY_INC))
	{
        pre_sch_cnt=0;
		epg_change_week_high_light((act == VACT_SCH_DAY_DEC) ? (-1) : 1);
		win_epg_sch_list_load(TRUE);
		////epg_draw_sch_week();
		OSD_TrackObject((POBJECT_HEAD)schol, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
	}
	
	return ret;
}


static PRESULT epg_sch_list_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	VACTION unact;

	switch (event)
	{
		case EVN_PRE_DRAW:
			win_epg_sch_set_display();
			break;
		case EVN_UNKNOWN_ACTION:
			unact = (VACTION)(param1 >> 16);
			epg_sch_list_unkown_act_proc(unact);
			break;
		default:
			break;
	}

	return ret;
}

static VACTION epg_sch_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act;

	switch (key)
	{
		case V_KEY_EXIT:
		case V_KEY_MENU:
		case V_KEY_LEFT:
			act = VACT_CLOSE;
			break;
		default:
			act = VACT_PASS;

	}

	return act;
}

static void win_epg_prg_set_display(void)
{
	POBJLIST ol = &epg_prg_ol;
	CONTAINER *item,  *chancon;
	TEXT_FIELD *txt;
	UINT32 i;
	UINT32 valid_idx;
	UINT16 top, cnt, page, index;
	P_NODE p_node;
	UINT16 unistr[30];
	char str[10];
	UINT8 focusId;

	cnt = OSD_GetObjListCount(ol);
	page = OSD_GetObjListPage(ol);
	top = OSD_GetObjListTop(ol);
//	curitem = OSD_GetObjListNewPoint(ol);
////////

	for (i = 0; i < page; i++)
	{
		item = (CONTAINER*)epg_prg_items[i];
		index = top + i;

		valid_idx = (index < cnt) ? 1 : 0;
		if (valid_idx)
		{
			get_prog_at(index, &p_node);
		}
		else
		{
			STRCPY(str, "");
			unistr[0] = 0;
		}


		/* IDX */
		//chancon = ( CONTAINER* ) OSD_GetContainerNextObj ( item );
		txt = (TEXT_FIELD*)OSD_GetContainerNextObj(item);
		if (valid_idx)
			sprintf(str, "%03d", index + 1);
		OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)str);

		/* Name */
		txt = (TEXT_FIELD*)OSD_GetObjpNext(txt);
		if (valid_idx)
		{
			if (p_node.ca_mode)
				ComAscStr2Uni("$", unistr);
			ComUniStrCopyChar((UINT8*) &unistr[p_node.ca_mode], p_node.service_name);
			OSD_SetTextFieldContent(txt, STRING_UNICODE, (UINT32)unistr);
		}
		else
			OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)str);

	}
}


static void win_epg_sch_set_display(void)
{
	POBJLIST ol = &epg_prg_ol;
	POBJLIST schol = &epg_sch_ol;
	CONTAINER *item,  *chancon;
	TEXT_FIELD *txt;
	BITMAP *bmp;
	UINT32 i;
	UINT32 valid_idx;
	UINT16 top, cnt, page, index; // curitem;
	P_NODE p_node;
	char str[40];
	UINT8 focusId;
	eit_event_info_t *sch_event = NULL;
	UINT32 eventNum = 0;
	date_time start_time, end_time; //event time
	UINT16 *s;
	UINT16 eventName[30];
	INT32 h, m, sec;

	cnt = OSD_GetObjListCount(schol);
	page = OSD_GetObjListPage(schol);
	top = OSD_GetObjListTop(schol);
//	curitem = OSD_GetObjListNewPoint(ol);

	//sch_event=epg_get_cur_service_event(curitem, SCHEDULE_EVENT,NULL,NULL, &eventNum, FALSE);
	for (i = 0; i < page; i++)
	{
		item = (CONTAINER*)epg_sch_items[i];
		index = top + i;

		valid_idx = (index < cnt) ? 1 : 0;
		if (valid_idx)
		{
			//get_prog_at ( index, &p_node );
			sch_event = epg_get_schedule_event(index);
		}
		else
		{
			STRCPY(str, "");
		}


		txt = (TEXT_FIELD*)OSD_GetContainerNextObj(item);



///////

		
		if (valid_idx)
        {
			get_STC_offset(&h, &m, &sec);
			get_event_start_time(sch_event, &start_time);
			get_event_end_time(sch_event, &end_time);

			convert_time_by_offset(&start_time, &start_time, h, m);
			convert_time_by_offset(&end_time, &end_time, h, m);

	        sprintf(str, "%02d:%02d~%02d:%02d", start_time.hour, start_time.min,end_time.hour,end_time.min);

			/// sprintf(str, "%02d:%02d:%02d~%02d:%02d:%02d", start_time.hour, start_time.min, start_time.sec,  \
					end_time.hour, end_time.min, end_time.sec);
        }
        else
            STRCPY(str, "");
		OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)str);

		// Name 
		txt = (TEXT_FIELD*)OSD_GetObjpNext(txt);
		// Stop scroll 
		wincom_scroll_textfield_stop(txt);
		
		if (valid_idx)
		{
			s = epg_get_event_name(sch_event, eventName, 32);
			if (s != NULL)
				OSD_SetTextFieldContent(txt, STRING_UNICODE, (UINT32)eventName);
		}
		else
			OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)str);

		bmp = (BITMAP*)OSD_GetObjpNext(txt);
		//if(bmp!=NULL)
	//	{
			if (valid_idx && (EPGEventBookedOrNot(sch_event) != 0))
			{
				OSD_SetBitmapContent(bmp, IM_ICON_CLOCK_01);
				////OSD_DrawObject((POBJECT_HEAD)bmp, C_UPDATE_ALL);
			}
			else
			{
				OSD_SetBitmapContent(bmp, 0);
			}
	///	}
		//else
			///continue;

	}
}


extern void PreviewVolProc(INT8 shift);

static VACTION epg_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;
	INT8 shift = 1;

	switch (key)
	{
		case V_KEY_EXIT:
		case V_KEY_ENTER:
			if(key == V_KEY_ENTER)
				{
			if (OSD_GetContainerFocus(&g_win_epg) == 2)
				break;
				}
            		//ad_stop(AD_EPG);
			sys_data_save(1);
			if (last_valid_grp_idx != sys_data_get_cur_group_index())
				sys_data_change_group(last_valid_grp_idx);
			BackToFullScrPlay();
			break;
		case V_KEY_MENU:
		case V_KEY_EPG:
			if (last_valid_grp_idx != sys_data_get_cur_group_index())
				sys_data_change_group(last_valid_grp_idx);
            
 #if ((SUBTITLE_ON == 1)||(TTX_ON == 1)) 
            UINT8 menu_num=0;
            menu_num=GetCurWinNum();
            if(menu_num==1)  //back to the video screen
            	api_osd_mode_change(OSD_SUBTITLE); 
 #endif
			act = VACT_CLOSE;
			break;
		case V_KEY_RIGHT:
		case V_KEY_LEFT:
			if (OSD_GetContainerFocus(&g_win_epg) == 1)
				{
				if(key == V_KEY_LEFT)
					break;

				}
			else
				{
					if(key == V_KEY_RIGHT)
					break;

				}
			
			act = VACT_CHANGE_FOCUS;
			break;
		case V_KEY_MUTE:
			act = VACT_MUTE;
			break;
		case V_KEY_PAUSE:
			act = VACT_PAUSE;
			break;
		/* Disable left/right key response
		case V_KEY_LEFT:
			shift =  - 1;
		case V_KEY_RIGHT:
			if (OSD_GetContainerFocus(&g_win_epg) == 1)
				PreviewVolProc(shift);
			break;
		*/
		default:
			act = VACT_PASS;
	}

	return act;

}

struct help_item_resource epg_help1[] =
{
    {1,IM_EPG_COLORBUTTON_GREEN,RS_SWITCH_WINDOW},
    {1,IM_HELP_ICON_LR,RS_HELP_GROUP},
    {0,RS_MENU,RS_HELP_BACK},
    {0,RS_HELP_EXIT,RS_HELP_EXIT},
};

struct help_item_resource epg_help2[] =
{
    {1,IM_EPG_COLORBUTTON_GREEN,RS_SWITCH_WINDOW},
    {1,IM_HELP_ICON_LR,RS_SWITCH_DATE},
    {1,IM_EPG_COLORBUTTON_RED,RS_EPG_DETAIL},
    {1,IM_EPG_COLORBUTTON_BLUE,RS_TIMER},
};

static PRESULT epg_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	VACTION unact;
	UINT16 x, y, w, h, chan_idx;
	UINT8 TV_Out;
	eit_event_info_t *sch_event;
	date_time tmp_date;
	INT32 event_num;
	UINT32 vkey;
	UINT8 str[2];
	UINT8 av_mode,cur_grp_idx;
	OSD_RECT rc_con, rc_preview;
	POBJECT_HEAD root = NULL;
	UINT16 ch_cnt = 0;
	UINT16 av_flag;
    date_time cur_date;

    UINT16 cur_channel;
    P_NODE p_node;

	av_mode = sys_data_get_cur_chan_mode();

	switch (event)
	{
		case EVN_PRE_OPEN:
			epg_help_name2.wStringID=RS_CTI_IPPV_VIEW;
/*make show the window will show EPG info in the case of opening the EPG widow at once after booting*/
			///libc_printf("enter EPG UI,time=%d\n",osal_get_tick()); 
          	cur_channel = sys_data_get_cur_group_cur_mode_channel();
		    get_prog_at(cur_channel, &p_node); 
		    epg_on(p_node.sat_id,p_node.tp_id,p_node.prog_number);
                  
			av_flag = sys_data_get_cur_chan_mode();
			ch_cnt = get_prog_num(VIEW_ALL | av_flag, 0);
			if (ch_cnt == 0 || sys_data_get_cur_group_index() == FAV_GROUP_TYPE)
			{
				sys_data_change_group(ALL_GROUP_TYPE);
				show_and_playchannel = 1;
			}
			last_valid_grp_idx = sys_data_get_cur_group_index();
			OSD_SetContainerFocus(&g_win_epg, 1); //set focus to prg list
			display_str_init(49);//29
			get_local_time(&cur_date);
			first_week_day = cur_date.weekday;
			epg_reset_week_high_light();
			if (win_epg_prg_list_load() == 0)
			{
				ret = PROC_LEAVE;
				break;
			}
			
			epg_set_cur_serice();
		win_epg_sch_list_load(TRUE);
			pre_sch_cnt = OSD_GetObjListCount(&epg_sch_ol);
	//		group_get_name(last_valid_grp_idx, OSD_GetTextFieldStrPoint(&epg_grp_name));
			#ifndef USE_16BIT_OSD
           /// wincom_open_title_ext(RS_EPG, IM_TITLE_ICON_EPG);
			////wincom_open_help(epg_help1, 4);
            #endif  
			//set_update_flag(TRUE);
			wincom_draw_epg_title_time();
		///con_whw.FocusObjectID=1;
			///libc_printf("enter2 EPG UI,time=%d\n",osal_get_tick());
			break;
		case EVN_POST_OPEN:
            #ifdef USE_16BIT_OSD
            ///wincom_open_title_ext(RS_EPG, IM_TITLE_ICON_EPG);
			////wincom_open_help(epg_help1, 4);
            #endif  
		//	OSD_DrawObject((POBJECT_HEAD)&epg_grp_name, C_UPDATE_ALL);
		  ////      Draw_localGrp_Name((TEXT_FIELD *)&epg_grp_name, cur_chan_grp_idx);
		 	/// libc_printf("Haved Open EPG UI,time=%d\n",osal_get_tick());
			rc_con.uLeft = EPG_PREVIEW_L;
			rc_con.uTop = EPG_PREVIEW_T;
			rc_con.uWidth = EPG_PREVIEW_W;
			rc_con.uHeight = EPG_PREVIEW_H;

			rc_preview.uLeft = PREVIEW_L;
			rc_preview.uTop = PREVIEW_T;
			rc_preview.uWidth = PREVIEW_W;
			rc_preview.uHeight = PREVIEW_H;

			wincom_open_preview_ext(rc_con, rc_preview, PREVIEW_IDX);

			////api_fill_black_screen();

			////epg_draw_sch_week();
			break;
		case EVN_UNKNOWNKEY_GOT:
			break;
		case EVN_UNKNOWN_ACTION:
			unact = (VACTION)(param1 >> 16);
			ret = win_epg_unkown_act_proc(unact);
			break;
		case EVN_MSG_GOT:
			ret = win_epg_message_proc(param1, param2);
			break;
		case EVN_PRE_CLOSE:
			if (menu_stack_get(0) == (POBJECT_HEAD) &g_win_mainmenu)
			{
				*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
			}
			sys_data_save(1);
            		//ad_stop(AD_EPG);
			break;
		case EVN_POST_CLOSE:
			
			root =menu_stack_get(0) ;
			if (root != (POBJECT_HEAD) &g_win_mainmenu)
			{
				wincom_close_preview();
				wincom_close_title();
				wincom_close_help();
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
			// av_mode = ( av_mode == TV_CHAN ) ? RADIO_CHAN : TV_CHAN;

			/*added for show pwd window if the program is locked   *******vincent*/
			/* sys_data_get_cur_group_channel ( &chan_idx, av_mode );
			get_prog_at ( chan_idx, &p_node );
			if (sys_data_get_channel_lock() && ( p_node.lock_flag || p_node.provider_lock))
			{
			api_play_channel(chan_idx,  TRUE,  TRUE,  FALSE);
			}
			 */
			break;
	}

	return ret;
}

static PRESULT win_epg_unkown_act_proc(VACTION act)
{
	PRESULT ret = PROC_LOOP;
	UINT8 av_mode;
	CONTAINER *item;
	UINT16 cur_point, top_idx;
	POBJLIST prg_ol = &epg_prg_ol;
	POBJLIST sch_ol = &epg_sch_ol;

	switch (act)
	{
		case VACT_MUTE:
		case VACT_PAUSE:
			if (act == VACT_MUTE)
				SetMuteOnOff(FALSE);
			else
			{
				av_mode = sys_data_get_cur_chan_mode();
				if (av_mode == TV_CHAN)
					SetPauseOnOff(FALSE);
			}
			break;
		case VACT_CHANGE_FOCUS:
			if (OSD_GetContainerFocus(&g_win_epg) == 1)
			{
				if (OSD_GetObjListCount(sch_ol) == 0)
					break;
				OSD_SetContainerFocus(&g_win_epg, 2);

				cur_point = OSD_GetObjListCurPoint(prg_ol);
				top_idx = OSD_GetObjListTop(prg_ol);
				item = (CONTAINER*)epg_prg_items[cur_point - top_idx];
				OSD_DrawObject((POBJECT_HEAD)item, C_UPDATE_ALL);
				cur_point = OSD_GetObjListCurPoint(sch_ol);
				top_idx = OSD_GetObjListTop(sch_ol);
				item = (CONTAINER*)epg_sch_items[cur_point - top_idx];
				OSD_TrackObject((POBJECT_HEAD)item, C_UPDATE_ALL);
				/*
//////

			LDEF_CONT(&g_win_epg,Container3,NULL,0,512,720,51,NULL, WSTL_MY_WIN_EPG)
			LDEF_CONT(&Container3, Container4, NULL, 41, 517, 90, 31, NULL, WSTL_YELLO_ITEM)
			LDEF_CONT(&Container3, Container5, NULL, 203, 517, 70, 31, NULL, WSTL_BULE_ITEM)
			LDEF_TXT(Container4,TextField2,NULL,51,521,66,18,WSTL_TXT_3,C_ALIGN_LEFT | C_ALIGN_VCENTER,RS_BUTTON_CONFIRM,NULL)
			LDEF_TXT(Container5,TextField4,NULL,211,521,55,18,WSTL_TXT_3,C_ALIGN_LEFT | C_ALIGN_VCENTER,RS_COMMON_OK,NULL)
			LDEF_TXT(Container4,TextField5,NULL,285,521,56,17,WSTL_TXT_3,C_ALIGN_LEFT | C_ALIGN_VCENTER,RS_BUTTON_CONFIRM,NULL)
			LDEF_TXT(Container3,TextField6,NULL,390,521,61,16,WSTL_TXT_3,C_ALIGN_LEFT | C_ALIGN_VCENTER,RS_CA_ENTITLE_CLASS,NULL)
			LDEF_TXT(Container3,TextField7,NULL,495,521,48,19,WSTL_TXT_3,C_ALIGN_LEFT | C_ALIGN_VCENTER,RS_CTI_DETITLE_CODE,NULL)
			LDEF_TXT(Container3,TextField8,NULL,594,521,73,19,WSTL_TXT_3,C_ALIGN_LEFT | C_ALIGN_VCENTER,RS_CTI_DETITLE_CODE,NULL)
			
			///LDEF_TXT(Container4,TextField2,NULL,51,521,66,18,WSTL_TXT_3,C_ALIGN_LEFT | C_ALIGN_VCENTER,RS_BUTTON_CONFIRM,NULL)
			LDEF_BMP ( Container3, Bitmap1, NULL, \
           344, 520, 43, 23, IM_HELP_RED)
           LDEF_BMP ( Container3, Bitmap2, NULL, \
           443, 520, 56, 20, IM_HELP_BLUE)
           LDEF_BMP ( Container3, Bitmap3, NULL, \
           551, 520, 38, 17, IM_HELP_YELLOW)WSTL_TXT_3
           */
          /// LDEF_TXT(Container3,TextField3,NULL,133,520,69,17,NULL,C_ALIGN_LEFT | C_ALIGN_VCENTER,RS_CTI_IPPV_VIEW,NULL)



			/*
			LDEF_TXT2(Container3,TextField3,NULL,0,0,0,0,0,l,t,w,h,resID,str)
			LDEF_TXT3(root,varTxt,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,resID,str)
			LDEF_TXT4(root,varTxt,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,resID,str)
			LDEF_TXT5(root,varTxt,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,resID,str)
			LDEF_TXT6(root,varTxt,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,resID,str)
			LDEF_TXT7(root,varTxt,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,resID,str)
			
			OSD_DrawObject((POBJECT_HEAD)&Container3, C_UPDATE_ALL);
			OSD_DrawObject((POBJECT_HEAD)&Container4, C_UPDATE_ALL);
			OSD_DrawObject((POBJECT_HEAD)&Container5, C_UPDATE_ALL);
			OSD_DrawObject((POBJECT_HEAD)&TextField2, C_UPDATE_ALL);
			
			OSD_DrawObject((POBJECT_HEAD)&TextField4, C_UPDATE_ALL);
			OSD_DrawObject((POBJECT_HEAD)&TextField5, C_UPDATE_ALL);
			OSD_DrawObject((POBJECT_HEAD)&TextField6, C_UPDATE_ALL);
			OSD_DrawObject((POBJECT_HEAD)&TextField7, C_UPDATE_ALL);
			OSD_DrawObject((POBJECT_HEAD)&TextField8, C_UPDATE_ALL);
			OSD_DrawObject((POBJECT_HEAD)&Bitmap1, C_UPDATE_ALL);
			OSD_DrawObject((POBJECT_HEAD)&Bitmap2, C_UPDATE_ALL);
			OSD_DrawObject((POBJECT_HEAD)&Bitmap3, C_UPDATE_ALL);
			*/
			
				//OSD_ClearObject((POBJECT_HEAD)&TextField3, C_UPDATE_ALL);
				epg_help_name2.wStringID=RS_CTI_IPPV_STATUS_BOOKING;//RS_CTI_IPPV_VIEW;
			OSD_DrawObject((POBJECT_HEAD)&epg_help_con, C_UPDATE_ALL);





///////
				/////epg_draw_cur_prg_bmp();
				////wincom_open_help(epg_help2, 4);
				win_epg_sch_list_load(FALSE);
				////epg_draw_sch_week();
				//OSD_DrawObject((POBJECT_HEAD) &sch_ol, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG );
				OSD_TrackObject((POBJECT_HEAD) &sch_ol, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
			}
			else if (OSD_GetContainerFocus(&g_win_epg) == 2)
			{
                if(epg_get_week_high_light()!=0)
                {
                    OSD_SetObjListCurPoint(sch_ol, 0);
                    OSD_SetObjListTop(sch_ol,0);
                    OSD_SetObjListNewPoint(sch_ol, 0);
                    cur_point=0;
                    top_idx=0;
                }else{
                    cur_point = OSD_GetObjListCurPoint(sch_ol);
				    top_idx = OSD_GetObjListTop(sch_ol);
                }
                epg_reset_week_high_light();
				////wincom_open_help(epg_help1, 4);
				OSD_SetContainerFocus(&g_win_epg, 1);
				//OSD_HideObject((POBJECT_HEAD)&epg_detail,C_UPDATE_ALL);
				//redraw the highlight item in the sch list
                
				item = (CONTAINER*)epg_sch_items[cur_point - top_idx];
				OSD_DrawObject((POBJECT_HEAD)item, C_UPDATE_ALL);

				epg_help_name2.wStringID=RS_CTI_IPPV_VIEW;

				///LDEF_TXT(Container3,TextField3,NULL,133,520,69,17,WSTL_TXT_3,C_ALIGN_LEFT | C_ALIGN_VCENTER,RS_CA_ENTITLE_CLASS,NULL)
				OSD_DrawObject((POBJECT_HEAD)&epg_help_con, C_UPDATE_ALL);
                
				cur_point = OSD_GetObjListCurPoint(prg_ol);
				top_idx = OSD_GetObjListTop(prg_ol);
				item = (CONTAINER*)epg_prg_items[cur_point - top_idx];
				OSD_TrackObject((POBJECT_HEAD)item, C_UPDATE_ALL);
			}
			break;
		default:
			break;
	}
	return ret;
}

static void epg_draw_cur_prg_bmp()
{
	OSD_RECT rect;
	OBJLIST *ol = &epg_prg_ol;
	UINT16 cur_point, top_idx;

	cur_point = OSD_GetObjListCurPoint(ol);
	top_idx = OSD_GetObjListTop(ol);

	epg_cur_prog_bmp.head.frame.uTop = LST_PRG_T + (ITEM_H + ITEM_PRG_GAP)*(cur_point - top_idx) + (ITEM_H - CUR_PRG_BMP_H) / 2;
	OSD_DrawObject((POBJECT_HEAD) &epg_cur_prog_bmp, C_UPDATE_ALL);
}

static void epg_change_week_high_light(INT8 offset)
{
	offset %= WEEK_DAY_TIEM_CNT;
	week_day_high_light = (UINT8)((week_day_high_light + \
		(offset+WEEK_DAY_TIEM_CNT)) % WEEK_DAY_TIEM_CNT);
}

static UINT8 epg_get_week_high_light()
{
	return week_day_high_light;
}

static void epg_reset_week_high_light()
{
	week_day_high_light = 0;
}

static void epg_draw_sch_week()
{
	UINT8 i = 0;
	POBJECT_HEAD txt = (POBJECT_HEAD)&epg_sch_weekday;
    
	for(i=0;i<WEEK_DAY_TIEM_CNT;i++)
	{
		OSD_SetTextFieldContent(&epg_sch_weekday,STRING_ID,weekday_id[(i+first_week_day)%WEEK_DAY_TIEM_CNT]);
		txt->frame.uLeft = SCH_WEEKDAY_L + i*SCH_WEEKDAY_W;
		if(week_day_high_light == i)
		{
			OSD_TrackObject(txt,C_UPDATE_ALL);
		}
		else
		{
			OSD_DrawObject(txt,C_UPDATE_ALL);
		}
	}
}

static UINT16 win_epg_prg_list_load()
{
	UINT16 channel, ch_cnt = 0, page, top;
	UINT8 av_flag;
	P_NODE p_node;
	OBJLIST *ol;

	ol = &epg_prg_ol;

	channel = sys_data_get_cur_group_cur_mode_channel();
	if (show_and_playchannel)
	{
		//show_and_playchannel = 0;
		api_play_channel(channel, TRUE, TRUE, FALSE);
	}

	av_flag = sys_data_get_cur_chan_mode();
	ch_cnt = get_prog_num(VIEW_ALL | av_flag, 0);

	/*if (ch_cnt != 0)
		exit_grp_idx = cur_chan_grp_idx;
	*/
	page = OSD_GetObjListPage(ol);

	if (channel >= ch_cnt)
		channel = 0;

	top = channel / page * page;
	if (ch_cnt > page)
	{
		if ((top + page) > ch_cnt)
			top = ch_cnt - page;
	}

	OSD_SetObjListCount(ol, ch_cnt);
	OSD_SetObjListCurPoint(ol, channel);
	OSD_SetObjListNewPoint(ol, channel);
	OSD_SetObjListTop(ol, top);

	return ch_cnt;
}

static UINT32 win_epg_sch_list_load(BOOL reset_focus)
{
	UINT16 channel, page, top;
	P_NODE p_node;
	POBJLIST ol = &epg_sch_ol;
	eit_event_info_t *sch_event = NULL;
	UINT32 eventNum = 0;
	date_time start_dt, end_dt, tempTime, viewTime;
	INT32 hour_off, min_off, sec_off;

	channel = sys_data_get_cur_group_cur_mode_channel();

    get_local_time(&viewTime);
	viewTime.mjd += (week_day_high_light % WEEK_DAY_TIEM_CNT);
	mjd_to_ymd(viewTime.mjd, &viewTime.year, &viewTime.month, &viewTime.day, &viewTime.weekday);

	start_dt = viewTime;
	end_dt = start_dt;
	if (week_day_high_light != 0)
	{
		start_dt.hour = 0;
		start_dt.min = 0;
		start_dt.sec = 0;
	}
	end_dt.hour = 23;
	end_dt.min = 59;
	end_dt.sec = 59;

	get_STC_offset(&hour_off, &min_off, &sec_off);	
	tempTime = start_dt;
	convert_time_by_offset(&start_dt, &tempTime,  - hour_off,  - min_off);
	tempTime = end_dt;
	convert_time_by_offset(&end_dt, &tempTime,  - hour_off,  - min_off);

	sch_event = epg_get_cur_service_event(channel, SCHEDULE_EVENT, &start_dt, &end_dt, &eventNum, TRUE);
	page = OSD_GetObjListPage(ol);

	OSD_SetObjListCount(ol, eventNum);
	if (reset_focus)
	{
		OSD_SetObjListCurPoint(ol, 0);
		OSD_SetObjListNewPoint(ol, 0);
		OSD_SetObjListTop(ol, 0);
	}

	return eventNum;
}

static void win_epg_scroll_string()
{
	OBJLIST *ol = &epg_sch_ol;
	UINT16 cur_point = 0;

	if (2 == OSD_GetContainerFocus(&g_win_epg))
	{
		cur_point = OSD_GetObjListCurPoint(ol);
		cur_point -= OSD_GetObjListTop(ol);
		wincom_scroll_textfield((PTEXT_FIELD)epg_sch_names[cur_point]);
	}
}

static void win_epg_update_sch()
{
	POBJLIST schol = &epg_sch_ol;
	
	if (pre_sch_cnt != OSD_GetObjListCount(schol))
	{
		epg_draw_sch_week();
		if (OSD_GetContainerFocus(&g_win_epg) == 1)
			OSD_DrawObject((POBJECT_HEAD)schol, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
		else
			OSD_TrackObject((POBJECT_HEAD)schol, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);

        pre_sch_cnt = OSD_GetObjListCount(schol);
	}

}

static PRESULT win_epg_message_proc(UINT32 msg_type, UINT32 msg_code)
{
	PRESULT ret = PROC_LOOP;
    date_time cur_date;

	switch (msg_type)
	{
		case CTRL_MSG_SUBTYPE_CMD_TIMEDISPLAYUPDATE:
			if(is_time_inited())
			{
				////wincom_draw_title_time();
				wincom_draw_epg_title_time();
				get_local_time(&cur_date);
				first_week_day = cur_date.weekday;
				//wincom_preview_proc();
				win_epg_sch_list_load(FALSE);
				win_epg_update_sch();
			}
			break;
		case CTRL_MSG_SUBTYPE_CMD_EPG_PF_UPDATED:
		case CTRL_MSG_SUBTYPE_CMD_EPG_SCH_UPDATED:
			win_epg_sch_list_load(FALSE);
			win_epg_update_sch();
		break;
		case CTRL_MSG_SUBTYPE_STATUS_SIGNAL:
			wincom_preview_proc();
			break;
		case CTRL_MSG_SUBTYPE_STATUS_SCROLL_STRING:
			win_epg_scroll_string();
			break;
		default:
			ret = PROC_PASS;
	}

	return ret;
}

static void win_epg_convert_time(date_time *utc_time, date_time *dest_dt)
{
	INT32 h, m, s;
	get_STC_offset(&h, &m, &s);
	convert_time_by_offset2(dest_dt, utc_time, h, m, s);
}

static void win_epg_get_time_len(date_time *dt1, date_time *dt2, INT32 *len)
{
	//*len>=0 when dt2>=dt1;*len<0 when dt2<dt1
	INT32 day = 0;
	INT32 tmp = 0;
	if (eit_compare_time(dt1, dt2) >= 0)
	//dt2>dt1
	{
		day = relative_day(dt1, dt2);
		*len = 3600 *(24 *day + dt2->hour) + 60*(dt2->min)  + dt2->sec - (3600 *(dt1->hour) + 60*(dt1->min)+ dt1->sec);
	}
	else
	{
		day = relative_day(dt2, dt1);
		*len =  - (3600 *(24 *day + dt1->hour) + 60 *(dt1->min) +dt1->sec - (3600 *(dt2->hour) + 60 *(dt2->min) + dt2->sec));
	}
}

/*0 means not booked  and other value means the index of timer*/
static UINT8 EPGEventBookedOrNot(eit_event_info_t *sch_event)
{
	date_time start_time,end_time;
	UINT8 i;
	UINT32 prog_id;
	INT32 h, m, sec;
	SYSTEM_DATA *sys_data;
	TIMER_SET_CONTENT * ptimer_set=NULL;
	P_NODE pNode;
	UINT16 channel;

	channel = OSD_GetObjListCurPoint(&epg_prg_ol);
	get_prog_at(channel, &pNode);
	sys_data = sys_data_get();
	get_STC_offset(&h, &m, &sec);
	get_event_start_time(sch_event, &start_time);
	get_event_end_time(sch_event, &end_time);

	convert_time_by_offset(&start_time, &start_time, h, m);
	convert_time_by_offset(&end_time, &end_time, h, m);


	for (i = 0; i < MAX_TIMER_NUM; i++)
	{
		ptimer_set = &sys_data->timer_set.TimerContent[i];
        if((ptimer_set->timer_mode!=TIMER_MODE_OFF)
            &&(ptimer_set->wakeup_channel == pNode.prog_id)
            &&(ptimer_set->wakeup_year== start_time.year)
            &&(ptimer_set->wakeup_month == start_time.month)
            &&(ptimer_set->wakeup_day== start_time.day)
            &&(isinragne(ptimer_set->wakeup_time,calc_wt(start_time),
            (end_time.day-start_time.day)*24*3600+calc_wt(end_time))))
			return (i + 1);
	}

	return 0;
}

