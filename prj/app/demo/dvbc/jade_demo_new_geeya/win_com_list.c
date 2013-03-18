#include <types.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/osd/osddrv.h>
//#include <api/libosd/osd_lib.h>
//#include <api/libosd/osd_vkey.h>

#include "osdobjs_def.h"

#include "osd_config.h"
#include "string.id"
#include "images.id"

#include "key.h"
#include "control.h"

#include "win_com.h"

#include "win_com_list.h"

#define CLST_PRINTF	PRINTF

/*******************************************************************************
 *	WINDOW's objects declaration
 *******************************************************************************/


////////////////////////////////////////////////////////////////////////////////////////////

extern CONTAINER g_win_com_lst;
extern OBJLIST g_ol_com_list;

extern TEXT_FIELD list_title;
extern TEXT_FIELD list_txt_fld0;
extern TEXT_FIELD list_txt_fld1;
extern TEXT_FIELD list_txt_fld2;
extern TEXT_FIELD list_txt_fld3;
extern TEXT_FIELD list_txt_fld4;
extern TEXT_FIELD list_txt_fld5;
extern TEXT_FIELD list_txt_fld6;
extern TEXT_FIELD list_txt_fld7;
extern TEXT_FIELD list_txt_fld8;
extern TEXT_FIELD list_txt_fld9;
extern TEXT_FIELD list_txt_fld10;
extern TEXT_FIELD list_txt_fld11;
#if 0
extern TEXT_FIELD list_txt_fld12;
extern TEXT_FIELD list_txt_fld13;
extern TEXT_FIELD list_txt_fld14;
extern TEXT_FIELD list_txt_fld15;
#endif

extern BITMAP list_mark;
extern SCROLL_BAR list_bar;


UINT32 comlist_sl_array[(MAX_MULTI_SL_ITEMS + 31) / 32];
UINT16 comlist_item_str[COMLIST_MAX_DEP][MAX_ITEM_STR_LEN];
UINT16 title_string[MAX_ITEM_STR_LEN];

static VACTION comlist_item_text_defkeymap(POBJECT_HEAD pObj, UINT32 vkey);
static PRESULT comlist_item_text_defcallback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION comlist_list_defkeymap(POBJECT_HEAD pObj, UINT32 vkey);
static PRESULT comlist_list_defcallback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION comlist_con_defkeymap(POBJECT_HEAD pObj, UINT32 vkey);
static PRESULT comlist_con_defcallback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

//static UINT16 item_str[COMLIST_MAX_DEP][MAX_STR_LEN];

/*******************************************************************************
 * WINDOW's objects defintion MACRO
 *******************************************************************************/

#define WIN_SH_IDX	    WSTL_POP_WIN_1
#define LST_SH_IDX	    WSTL_NOSHOW_IDX
#define SH_IDX		    WSTL_TXT_POPWIN
#define HL_IDX		    WSTL_BUTTON_1
#define SEL_IDX		    WSTL_TXT_POPWIN
#define MARK_SH_IDX	0
#define MARK_HL_IDX	0
#define MARK_SEL_IDX	0


#define LIST_BAR_SH_IDX  WSTL_SCROLLBAR_1
#define LIST_BAR_HL_IDX  WSTL_SCROLLBAR_1

#define LIST_BAR_MID_RECT_IDX  	WSTL_NOSHOW_IDX
#define LIST_BAR_MID_THUMB_IDX  	WSTL_SCROLLBAR_2

#define TITLE_SH_IDX    0

#define TOX 0
#define TH		26
#define TGAP	0

#define LDEF_TXT(root,varTxt,nxtObj,ID,t,str)		\
  DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,0,0,0,0,  0,t,170,TH, SH_IDX,HL_IDX,SEL_IDX,0,   \
    comlist_item_text_defkeymap,comlist_item_text_defcallback,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 20,0,0,str)

#define LDEF_MARKBMP(root,varBmp,l,t,w,h,icon)		\
  DEF_BITMAP(varBmp,root,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, MARK_SH_IDX,MARK_HL_IDX,MARK_SEL_IDX,0,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,icon)


#define LDEF_OL(root,varOl,nxtObj,l,t,w,h,style,dep,count,flds,sb,mark)	\
  DEF_OBJECTLIST(varOl,root,nxtObj,C_ATTR_ACTIVE,0, \
    1,1,1,1,1, l,t,w,h,LST_SH_IDX,LST_SH_IDX,0,0,   \
    comlist_list_defkeymap,comlist_list_defcallback,    \
    flds,sb,mark,style,dep,count,comlist_sl_array)

#define LDEF_WIN(varCon,nxtObj,l,t,w,h,focusID)		\
    DEF_CONTAINER(varCon,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_SH_IDX,0,0,   \
    comlist_con_defkeymap,comlist_con_defcallback,  \
    nxtObj, focusID,0)

/*******************************************************************************
*   WINDOW's objects defintion
*******************************************************************************/
//#define WND (&g_win_com_lst)
//#define LST	(&g_ol_com_list)
//#define TEXT list_txt_fld1

#define LS_GAP  4
#define WS_GAP  4

LDEF_TXT ( &g_ol_com_list, list_txt_fld0, NULL, 0, TOX,					comlist_item_str[0] )
LDEF_TXT ( &g_ol_com_list, list_txt_fld1, NULL, 0, TOX + ( TGAP + TH ) *1,	comlist_item_str[1] )
LDEF_TXT ( &g_ol_com_list, list_txt_fld2, NULL, 0, TOX + ( TGAP + TH ) *2,	comlist_item_str[2] )
LDEF_TXT ( &g_ol_com_list, list_txt_fld3, NULL, 0, TOX + ( TGAP + TH ) *3,	comlist_item_str[3] )
LDEF_TXT ( &g_ol_com_list, list_txt_fld4, NULL, 0, TOX + ( TGAP + TH ) *4,	comlist_item_str[4] )
LDEF_TXT ( &g_ol_com_list, list_txt_fld5, NULL, 0, TOX + ( TGAP + TH ) *5,	comlist_item_str[5] )
LDEF_TXT ( &g_ol_com_list, list_txt_fld6, NULL, 0, TOX + ( TGAP + TH ) *6,	comlist_item_str[6] )
LDEF_TXT ( &g_ol_com_list, list_txt_fld7, NULL, 0, TOX + ( TGAP + TH ) *7,	comlist_item_str[7] )
LDEF_TXT ( &g_ol_com_list, list_txt_fld8, NULL, 0, TOX + ( TGAP + TH ) *8,	comlist_item_str[8] )
LDEF_TXT ( &g_ol_com_list, list_txt_fld9, NULL, 0, TOX + ( TGAP + TH ) *9,	comlist_item_str[9] )
LDEF_TXT ( &g_ol_com_list, list_txt_fld10, NULL, 0, TOX + ( TGAP + TH ) *10, comlist_item_str[10] )
LDEF_TXT ( &g_ol_com_list, list_txt_fld11, NULL, 0, TOX + ( TGAP + TH ) *11, comlist_item_str[11] )
#if 0
LDEF_TXT ( &g_ol_com_list, list_txt_fld12, NULL, 0, TOX + ( TGAP + TH ) *12, ( UINT8* ) display_strs[12] )
LDEF_TXT ( &g_ol_com_list, list_txt_fld13, NULL, 0, TOX + ( TGAP + TH ) *13, ( UINT8* ) display_strs[13] )
LDEF_TXT ( &g_ol_com_list, list_txt_fld14, NULL, 0, TOX + ( TGAP + TH ) *14, ( UINT8* ) display_strs[14] )
LDEF_TXT ( &g_ol_com_list, list_txt_fld15, NULL, 0, TOX + ( TGAP + TH ) *15, ( UINT8* ) display_strs[15] )
#endif

OBJECT_HEAD* list_flds[] =
    {
        ( OBJECT_HEAD* ) &list_txt_fld0,
        ( OBJECT_HEAD* ) &list_txt_fld1,
        ( OBJECT_HEAD* ) &list_txt_fld2,
        ( OBJECT_HEAD* ) &list_txt_fld3,
        ( OBJECT_HEAD* ) &list_txt_fld4,
        ( OBJECT_HEAD* ) &list_txt_fld5,
        ( OBJECT_HEAD* ) &list_txt_fld6,
        ( OBJECT_HEAD* ) &list_txt_fld7,
        ( OBJECT_HEAD* ) &list_txt_fld8,
        ( OBJECT_HEAD* ) &list_txt_fld9,
        ( OBJECT_HEAD* ) &list_txt_fld10,
        ( OBJECT_HEAD* ) &list_txt_fld11,
#if 0
        ( OBJECT_HEAD* ) &list_txt_fld12,
        ( OBJECT_HEAD* ) &list_txt_fld13,
        ( OBJECT_HEAD* ) &list_txt_fld14,
        ( OBJECT_HEAD* ) &list_txt_fld15,
#endif

    };

LDEF_OL ( &g_win_com_lst, g_ol_com_list, NULL, 0, 0, 200, 300, LIST_VER , 16, 16, &list_flds[0], NULL, ( OBJECT_HEAD* ) &list_mark )

LDEF_MARKBMP ( &g_ol_com_list, list_mark, 5, 0, 30, TH, 0/*IM_MTG10_38*/ )


DEF_SCROLLBAR ( list_bar, &g_ol_com_list, NULL, C_ATTR_ACTIVE, 0, \
                0, 0, 0, 0, 0, 200 + LS_GAP, 0, 20, 300, LIST_BAR_SH_IDX, LIST_BAR_HL_IDX, LIST_BAR_SH_IDX, LIST_BAR_SH_IDX, \
                NULL, NULL, BAR_VERT_AUTO | SBAR_STYLE_RECT_STYLE, 10, LIST_BAR_MID_THUMB_IDX, LIST_BAR_MID_RECT_IDX, \
                0, 10, 20, 300 - 20, 100, 1 )

/*
DEF_SCROLLBAR(list_bar, &g_ol_com_list, NULL, C_ATTR_ACTIVE, 0, \
	3, 2, 1, 3, 3, 200 + LS_GAP, 0, 20, 300, LIST_BAR_SH_IDX, LIST_BAR_HL_IDX, LIST_BAR_SH_IDX, LIST_BAR_SH_IDX, \
	NULL, NULL, BAR_VERT_AUTO, 0, LIST_BAR_MID_THUMB_IDX, LIST_BAR_MID_RECT_IDX, \
	LS_GAP + 2, 2, 16, 296, 100, 2)
*/

DEF_TEXTFIELD ( list_title, &g_ol_com_list, &g_ol_com_list, C_ATTR_ACTIVE, 0, \
                1, 0, 0, 0, 0, 310, 100, 200, 134, WSTL_TXT_5, WSTL_BUTTON_1, WSTL_TXT_5, WSTL_TXT_5,   \
                NULL, NULL,  \
                C_ALIGN_VCENTER | C_ALIGN_VCENTER, 10, 10, 0, title_string )

//LISTDEF_BAR(&g_ol_com_list,list_bar,200 + LS_GAP,5,12,290,8,8,0,100)

LDEF_WIN ( g_win_com_lst, &g_ol_com_list, 0, 0, 300, 300, 1 )


/*******************************************************************************
 *	Local vriable & function declare
 *******************************************************************************/

static UINT8 save_back_ok;

///
void set_obj_wh(OBJECT_HEAD *obj, UINT16 w, UINT16 h);

/*******************************************************************************
 *  WINDOW's  keymap, proc and callback
 *******************************************************************************/

static VACTION comlist_item_text_defkeymap(POBJECT_HEAD pObj, UINT32 vkey)
{
	return VACT_PASS;
}

static PRESULT comlist_item_text_defcallback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	return PROC_PASS;
}

static VACTION comlist_list_defkeymap(POBJECT_HEAD pObj, UINT32 vkey)
{
	VACTION act = VACT_PASS;

	switch (vkey)
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
		case V_KEY_ENTER:
			act = VACT_SELECT;
			break;
	}

	return act;
}

static PRESULT comlist_list_defcallback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	OBJLIST *ol = (OBJLIST*)pObj;

	switch (event)
	{
		case EVN_ITEM_POST_CHANGE:
			if (ol->bListStyle &LIST_SINGLE_SLECT)
				ret = PROC_LEAVE;
			break;
	}

	return ret;
}

static VACTION comlist_con_defkeymap(POBJECT_HEAD pObj, UINT32 vkey)
{
	PRESULT ret = VACT_PASS;
	switch (vkey)
	{
		case V_KEY_MENU:
		case V_KEY_EXIT:
			ret = VACT_CLOSE;
	}

	return ret;
}

static PRESULT comlist_con_defcallback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	return PROC_PASS;
}


void win_comlist_clear_sel(void)
{
	OBJLIST *ol;
	ol = &g_ol_com_list;

	MEMSET(comlist_sl_array, 0, sizeof(comlist_sl_array));
	OSD_SetObjListSingleSelect(ol, LIST_INVALID_SEL_IDX);
}

/*******************************************************************************
 *	Common exported functions
 *******************************************************************************/

void win_comlist_reset(void)
{
	CONTAINER *win;
	OBJLIST *ol;
	UINT16 i;

	win = win = &g_win_com_lst;
	ol = &g_ol_com_list;
	win->pNextInCntn = (POBJECT_HEAD)ol;

	OSD_SetObjListTop(ol, 0);
	OSD_SetObjListCurPoint(ol, 0);
	OSD_SetObjListNewPoint(ol, 0);

	win_comlist_clear_sel();
	win_comlist_ext_set_win_style(WIN_SH_IDX);
	win_comlist_ext_set_ol_items_style(SH_IDX, HL_IDX, SH_IDX);
	win_comlist_ext_set_title_style(SH_IDX);

	win_comlist_ext_set_title(NULL, NULL, 0);
	win_comlist_ext_set_selmark(0 /*IM_MTG10_38*/);
	win_comlist_ext_set_selmark_xaligen(C_ALIGN_LEFT, 0);
	win_comlist_ext_enable_scrollbar(FALSE);
	win_comlist_set_mapcallback(NULL, NULL, NULL);

	for (i = 0; i < COMLIST_MAX_DEP; i++)
		win_comlist_ext_set_item_attr(i, C_ATTR_ACTIVE);
}



void win_comlist_set_frame(UINT16 l, UINT16 t, UINT16 w, UINT16 h)
{
	CONTAINER *win;
	OBJLIST *ol;
	OBJECT_HEAD *item;
	UINT8 i;
	UINT16 oldCount, oldDep;
	BOOL b;
	POBJECT_HEAD obj;

	win = &g_win_com_lst;
	ol = &g_ol_com_list;
	if (ol->scrollBar == NULL)
	{
		b = FALSE;
		ol->scrollBar = &list_bar;
	}
	else
		b = TRUE;

	obj = (POBJECT_HEAD) &list_title;

	OSD_SetRect(&obj->frame, l + 4, t + 4, w - 10, 26);

	/* Move all objects defined in list_flds*/
	oldCount = ol->wCount;
	oldDep = ol->wDep;
	ol->wDep = COMLIST_MAX_DEP;
	if (oldCount < ol->wDep)
		ol->wCount = ol->wDep;
	OSD_MoveObject(win, l, t, FALSE);
	ol->wCount = oldCount;
	ol->wDep = oldDep;
	if (h < ((ol->wDep *(TGAP + TH)) + 8))
		h = ol->wDep *(TGAP + TH) + 8;

	set_obj_wh((OBJECT_HEAD*)win, w, h);

	if ( !b )  ol->scrollBar = NULL;
	CLST_PRINTF("Window Frame(%d,%d,%d,%d)\n", win->head.frame.uLeft, win->head.frame.uTop, win->head.frame.uWidth, win->head.frame.uHeight);


	win_comlist_ext_set_ol_frame(l + 4, t + 4, w - 8, h - 8);

}

void win_comlist_set_sizestyle(UINT16 count, UINT16 dep, UINT16 lststyle)
{
	OBJLIST *ol;
	ol = &g_ol_com_list;

	if (dep > COMLIST_MAX_DEP)
		dep = COMLIST_MAX_DEP;

	if (count > MAX_MULTI_SL_ITEMS)
		count = MAX_MULTI_SL_ITEMS;
	ol->wCount = count;

	ol->wDep = dep;
	ol->bListStyle = lststyle;
	if (ol->scrollBar)
		win_comlist_ext_enable_scrollbar(TRUE);

}

void win_comlist_set_align(UINT16 ox, UINT16 oy, UINT8 alignStyle)
{
	UINT8 i;
	TEXT_FIELD *t;

	for (i = 0; i < COMLIST_MAX_DEP; i++)
	{
		t = (TEXT_FIELD*)list_flds[i];
		t->bX = ox;
		t->bY = oy;
		t->bAlign = alignStyle;
	}
}

void win_comlist_set_str(UINT16 itemIdx, char *str, char *unistr, UINT16 strID)
{
	OBJLIST *ol;
	TEXT_FIELD *t;
	INT i;
	UINT16 wTop;
	UINT16 *pstr;

	ol = &g_ol_com_list;

	if (ol->bListStyle &LIST_ITEMS_COMPLETE)
		wTop = 0;
	else
		wTop = OSD_GetObjListTop(ol);
	//OSD_GetObjListTopPoint(ol);//OSD_GetObjListTop(ol);

	CLST_PRINTF("itemIdx=%d,wTop=%d\n", itemIdx, wTop);

	t = (TEXT_FIELD*)list_flds[itemIdx - wTop];
	pstr = comlist_item_str[itemIdx - wTop]; //item_str[itemIdx - wTop];

	if (strID != 0)
	{
		t->pString = NULL;
		t->wStringID = strID;
	}
	else if (str != NULL || unistr != NULL)
	{
		if (unistr != NULL)
			ComUniStrCopyChar((UINT8*)pstr, unistr);
		else
			ComAscStr2Uni(str, pstr);
		t->pString = pstr;
	}
	else
	{
		t->pString = NULL;
		t->wStringID = 0;
	}

}


UINT16 win_comlist_get_selitem(void)
{
	OBJLIST *ol;
	ol = &g_ol_com_list;

	if (ol->bListStyle &LIST_SINGLE_SLECT == LIST_SINGLE_SLECT)
		return OSD_GetObjListSingleSelect(ol);
	else if (ol->bListStyle &LIST_MULTI_SLECT == LIST_MULTI_SLECT)
		return OSD_GetObjListMultiSelItemNum(ol);
	else
		return 0;
}

void win_comlist_set_mapcallback(PFN_KEY_MAP listKeyMap, PFN_KEY_MAP winKeyMap, PFN_CALLBACK lstcallback)
{
	CONTAINER *w;
	OBJLIST *ol;
	PFN_KEY_MAP kemap;
	PFN_CALLBACK callback;

	w = &g_win_com_lst;
	ol = &g_ol_com_list;

	kemap = (listKeyMap == NULL) ? comlist_list_defkeymap : listKeyMap;
	OSD_SetKeyMap(ol, kemap);

	callback = (lstcallback == NULL) ? comlist_list_defcallback : lstcallback;
	OSD_SetCallback(ol, callback);

	kemap = (winKeyMap == NULL) ? comlist_con_defkeymap : winKeyMap;
	OSD_SetKeyMap(w, kemap);
}

BOOL win_comlist_open(void)
{
	CONTAINER *w;

	CLST_PRINTF("Enter %s:\n", __FUNCTION__);

	w = &g_win_com_lst;
	OSD_ObjOpen((POBJECT_HEAD)w, 0);


	CLST_PRINTF("Exit %s:\n", __FUNCTION__);

	return TRUE;
}

void win_comlist_close(void)
{
	CONTAINER *w = &g_win_com_lst;
	UINT32 param;

	param = C_CLOSE_CLRBACK_FLG;
	OSD_ObjClose((POBJECT_HEAD)w, param);

}

void set_obj_wh(OBJECT_HEAD *obj, UINT16 w, UINT16 h)
{
	obj->frame.uWidth = w;
	obj->frame.uHeight = h;
}

void set_bar_h(SCROLL_BAR *b, UINT16 h)
{
	struct OSDRect *b_frame;

	b_frame = &b->head.frame;
	b_frame->uHeight = h;
}

//extend APIs
void win_comlist_ext_set_ol_frame(UINT16 l, UINT16 t, UINT16 w, UINT16 h)
{
	CONTAINER *win;
	OBJLIST *ol;
	OBJECT_HEAD *item;
	UINT8 i;
	struct OSDRect *w_frame;
	UINT16 oldCount, oldDep;
	BOOL b;
	UINT16 or, wr;

	win = &g_win_com_lst;
	ol = &g_ol_com_list;

	if (ol->scrollBar == NULL)
	{
		b = FALSE;
		ol->scrollBar = &list_bar;
	}
	else
		b = TRUE;

	w_frame = &win->head.frame;

	if ( l < w_frame->uLeft )		l = w_frame->uLeft;
	if ( t < w_frame->uTop )			t = w_frame->uTop;
	if ( w > w_frame->uWidth )		w = w_frame->uWidth;
	if (b)
	{
		or = l + w;
		wr = w_frame->uLeft + w_frame->uWidth;
		if (or >= (wr - LS_GAP - WS_GAP - ol->scrollBar->head.frame.uWidth))
		{
			or = wr - LS_GAP - WS_GAP - ol->scrollBar->head.frame.uWidth;
			w = or - l;
		}
	}
	if ( h > w_frame->uHeight )		h = w_frame->uHeight;


	oldCount = ol->wCount;
	oldDep = ol->wDep;
	ol->wDep = COMLIST_MAX_DEP;
	if (oldCount < ol->wDep)
		ol->wCount = ol->wDep;
	OSD_MoveObject(ol, l, t, FALSE);
	ol->wCount = oldCount;
	ol->wDep = oldDep;

	OSD_MoveObject(ol->scrollBar, l + w + LS_GAP, ol->scrollBar->head.frame.uTop, FALSE);

	set_obj_wh((OBJECT_HEAD*)ol, w, h);

	set_bar_h(ol->scrollBar, ol->wDep *(TH + TGAP) - TGAP);

	CLST_PRINTF("OL Frame(%d,%d,%d,%d)\n", ol->head.frame.uLeft, ol->head.frame.uTop, ol->head.frame.uWidth, ol->head.frame.uHeight);
	CLST_PRINTF("Scroll bar Frame(%d,%d,%d,%d)\n", ol->scrollBar->head.frame.uLeft, ol->scrollBar->head.frame.uTop, ol->scrollBar->head.frame.uWidth, ol->scrollBar->head.frame.uHeight);

	if ( !b )  ol->scrollBar = NULL;

	for (i = 0; i < COMLIST_MAX_DEP; i++)
	{
		CLST_PRINTF("Item %d\n", i);
		item = list_flds[i];
		set_obj_wh(item, w, item->frame.uHeight);
		CLST_PRINTF("Item %d Frame(%d,%d,%d,%d)\n", i, item->frame.uLeft, item->frame.uTop, item->frame.uWidth, item->frame.uHeight);
	}


}

void win_comlist_ext_set_title(char *str, char *unistr, UINT16 strID)
{
	TEXT_FIELD *txt;
	txt = &list_title;
	UINT8 title_exist;
	CONTAINER *win;
	OBJLIST *ol;

	title_exist = 1;
	if (str != NULL)
		OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)str);
	else if (unistr != NULL)
		OSD_SetTextFieldContent(txt, STRING_UNICODE, (UINT32)str);
	else if (strID != 0)
		OSD_SetTextFieldContent(txt, STRING_ID, (UINT32)strID);
	else
		title_exist = 0;

	win = win = &g_win_com_lst;
	ol = &g_ol_com_list;

	if (title_exist)
		win->pNextInCntn = (POBJECT_HEAD)txt;
	else
		win->pNextInCntn = (POBJECT_HEAD)ol;

}

void win_comlist_ext_set_win_style(UINT8 idx)
{
	CONTAINER *win;

	win = &g_win_com_lst;
	OSD_SetColor(win, idx, idx, idx, idx);
}

void win_comlist_ext_set_ol_items_style(UINT8 sh_idx, UINT8 hi_idx, UINT8 sel_idx)
{
#if 0
	OBJECT_HEAD *item;
	BITMAP *mark;
	OBJLIST *ol;
	UINT8 i;

	for (i = 0; i < COMLIST_MAX_DEP; i++)
	{
		item = list_flds[i];
		OSD_SetColor(item, sh_idx, hi_idx, sel_idx, 0);
	}

	//mark = &list_mark;
	//OSD_SetColor(mark,sh_idx,hi_idx,sel_idx,0);

	ol = &g_ol_com_list;
	OSD_SetColor(ol, sh_idx, sh_idx, sh_idx, 0);
#endif
	win_comlist_ext_set_ol_items_allstyle(sh_idx, hi_idx, sel_idx, 0);

}


void win_comlist_ext_set_ol_items_allstyle(UINT8 sh_idx, UINT8 hi_idx, UINT8 sel_idx, UINT8 gry_idx)
{
	OBJECT_HEAD *item;
	BITMAP *mark;
	OBJLIST *ol;
	UINT8 i;

	for (i = 0; i < COMLIST_MAX_DEP; i++)
	{
		item = list_flds[i];
		OSD_SetColor(item, sh_idx, hi_idx, sel_idx, gry_idx);
	}

	//mark = &list_mark;
	//OSD_SetColor(mark,sh_idx,hi_idx,sel_idx,0);

	ol = &g_ol_com_list;
	OSD_SetColor(ol, sh_idx, sh_idx, sh_idx, 0);

}

void win_comlist_ext_set_title_style(UINT8 idx)
{
	POBJECT_HEAD pObj;

	pObj = (POBJECT_HEAD) &list_title;
	OSD_SetColor(pObj, idx, idx, idx, idx);
}

void win_comlist_ext_set_selmark(UINT16 iconid)
{
	BITMAP *b;

	b = &list_mark;
	OSD_SetBitmapContent(b, iconid);
}

void win_comlist_ext_set_selmark_xaligen(UINT8 style, UINT8 offset)
{
	OBJECT_HEAD *item0;
	BITMAP *b;

	item0 = list_flds[0];
	b = &list_mark;

	if (style &C_ALIGN_LEFT)
		b->head.frame.uLeft = item0->frame.uLeft + offset;
	else if (style &C_ALIGN_CENTER)
		b->head.frame.uLeft = item0->frame.uLeft + item0->frame.uWidth / 2-offset;
	else if (style &C_ALIGN_RIGHT)
		b->head.frame.uLeft = item0->frame.uLeft + item0->frame.uWidth - offset;
}

void win_comlist_ext_get_sel_items(UINT32 **dwSelect)
{
	OBJLIST *ol;

	ol = &g_ol_com_list;
	*dwSelect = ol->dwSelect;
}

BOOL win_comlist_ext_check_item_sel(UINT16 index)
{
	OBJLIST *ol;

	ol = &g_ol_com_list;

	if (ol->bListStyle &LIST_MULTI_SLECT == LIST_MULTI_SLECT)
		return OSD_CheckObjListMultiSelect(ol, index);
	else if (ol->bListStyle &LIST_SINGLE_SLECT == LIST_SINGLE_SLECT)
		return (ol->wSelect == index) ? TRUE : FALSE;
	else
		return FALSE;
}


void win_comlist_ext_set_item_sel(UINT16 index)
{
	OBJLIST *ol;
	ol = &g_ol_com_list;

	if ((ol->bListStyle &LIST_SINGLE_SLECT) == LIST_SINGLE_SLECT)
		OSD_SetObjListSingleSelect(ol, index);
	else if ((ol->bListStyle &LIST_MULTI_SLECT) == LIST_MULTI_SLECT)
		OSD_SwitchObjListMultiSelect(ol, index);
}

void win_comlist_ext_set_item_cur(UINT16 index)
{
	OBJLIST *ol;
	ol = &g_ol_com_list;

	if ( index >= ol->wCount )   return;
	ol->wTop = index / ol->wDep *ol->wDep;
	ol->wCurPoint = ol->wNewPoint = index;
}


void win_comlist_ext_enable_scrollbar(BOOL b)
{
	OBJLIST *ol;

	ol = &g_ol_com_list;
	if (b)
		OSD_SetObjListVerScrollBar(ol, &list_bar);
	else
		ol->scrollBar = NULL;
}

void win_comlist_ext_set_item_attr(UINT16 index, UINT8 attr)
{
	OBJLIST *ol;
	ol = &g_ol_com_list;

	POBJECT_HEAD item;


	if (index < ol->wTop + COMLIST_MAX_DEP)
	{
		item = list_flds[index - ol->wTop];
		OSD_SetAttr(item, attr);
	}
}

void win_comlist_popup()
{
	CONTAINER *con = &g_win_com_lst;
	UINT8 bResult = PROC_LOOP;
	UINT32 hkey;
	UINT32 msg;

	CLST_PRINTF("Enter %s:\n", __FUNCTION__);

	win_comlist_open();

	while (bResult != PROC_LEAVE)
	{
		hkey = ap_get_key_msg();
		if (hkey == INVALID_HK || hkey == INVALID_MSG)
		{
			CLST_PRINTF("Unvalid hkey\n");
			continue;
		}
		CLST_PRINTF("!!!!!hkey==%d\n", hkey);
		//msg = OSD_GetOSDMessage(MSG_TYPE_KEY,hkey);

		bResult = OSD_ObjProc((POBJECT_HEAD)con, (MSG_TYPE_KEY << 16), hkey, 0); //w->head.pProc((OBJECTHEAD *)w, msgcode, 0, save_back_ok);

		CLST_PRINTF("bResult==%d\n", bResult);
	}


	CLST_PRINTF("Exit %s:\n", __FUNCTION__);
}

extern UINT8 *osd_vscr_buffer[];

static UINT8 *g_back_buf = NULL;
static UINT32 g_back_buf_size;
void win_comlist_popup_ext(UINT8 *back_saved)
{
	UINT8 *pbufTemp,*pvscr_buff;
	CONTAINER *win;
	UINT32 vscr_size;
	OSD_RECT *frame;
	VSCR vscr,  *lpVscr;
	UINT8 *pbuf = NULL;

	win = &g_win_com_lst;
	frame = &win->head.frame;

	lpVscr = OSD_GetTaskVscr(osal_task_get_current_id());
	OSD_UpdateVscr(lpVscr);
	lpVscr->lpbScr = NULL;

#ifdef USE_LIB_GE
	win_com_backup_rect(frame);
	
	win_comlist_popup();

	win_com_restore_rect(frame);

#else
	vscr_size = (((frame->uWidth) >> FACTOR) + 1) *frame->uHeight;
	if (vscr_size < OSD_VSRC_MEM_MAX_SIZE / 2)
	{
		pbuf = &osd_vscr_buffer[osal_task_get_current_id() - 1][vscr_size];
		save_back_ok = 1;
	}
	else
	{
		if (g_back_buf != NULL && g_back_buf_size >= vscr_size)
			pbuf = g_back_buf;
		else
		{
			if (g_back_buf != NULL)
				FREE(g_back_buf);
			pbuf = (UINT8*)MALLOC(vscr_size);
			g_back_buf = pbuf;
			if (g_back_buf != NULL)
				g_back_buf_size = vscr_size;
		}
		if (pbuf != NULL)
			save_back_ok = 1;
		else
			save_back_ok = 0;
	}

	if (save_back_ok)
	{
		OSD_SetRect2(&vscr.vR, frame);
		vscr.lpbScr = pbuf;
		OSD_GetRegionData(&vscr, &vscr.vR);
	}

	CLST_PRINTF("Enter %s:\n", __FUNCTION__);


	win_comlist_popup();

	if (save_back_ok)
	{
		OSD_RegionWrite(&vscr, &vscr.vR);
		if (g_back_buf != NULL)
		{
			FREE(g_back_buf);
			g_back_buf = NULL;
		}

		lpVscr->lpbScr = NULL;
	}
#endif
	*back_saved = save_back_ok;


	CLST_PRINTF("Exit %s:\n", __FUNCTION__);
}
