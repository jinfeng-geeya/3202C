/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2006 Copyright (C)
 *
 *  File: win_reg.c
 *
 *  Description: register debug tools for UI.it is a memory dump liked,
 *
 *  History:
 *  Date                           Author             Version         Comment
 *  1.  2008.05.09                 vincent_zhang      0.1.000         init
 ****************************************************************************/

/*******************************************************************************
*	Objects definition
*******************************************************************************/
#include <sys_config.h>
#ifdef REG_ENABLE

#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>

//#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"

#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"



extern CONTAINER g_win_reg;
extern CONTAINER con_reg;

extern CONTAINER con_reg_item_add;
extern CONTAINER con_reg_item_txt;
extern CONTAINER con_reg_item1;
extern CONTAINER con_reg_item2;
extern CONTAINER con_reg_item3;
extern CONTAINER con_reg_item4;

extern TEXT_FIELD txt_title_add;
extern TEXT_FIELD txt_title_button;

extern EDIT_FIELD	edit_reg_add;
extern EDIT_FIELD	edit_reg_value1;
extern EDIT_FIELD	edit_reg_value2;
extern EDIT_FIELD	edit_reg_value3;
extern EDIT_FIELD	edit_reg_value4;


char win_reg_add_pat[] = "s8"; 
#define P_BUF_BASE_IDX	20

static VACTION win_reg_keymap ( POBJECT_HEAD pObj, UINT32 key );
static PRESULT win_reg_callback ( POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2 );
static VACTION con_keymap ( POBJECT_HEAD pObj, UINT32 key );
static PRESULT con_callback ( POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2 );
static VACTION item_keymap ( POBJECT_HEAD pObj, UINT32 key );
static PRESULT item_callback ( POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2 );
static VACTION reg_edit_keymap ( POBJECT_HEAD pObj, UINT32 key );
static PRESULT reg_edit_callback ( POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2 );


#define WIN_SH_IDX	WSTL_SEARCH_BG_CON

#define GAP 10
#define W_L  (TV_OFFSET_L+105)
#define W_T  (TV_OFFSET_T+100)
#define W_W  505
#define W_H  300

#define CON_L  (W_L+GAP)
#define CON_T  (W_T+GAP)
#define CON_W  (W_W-2*GAP)
#define CON_H  (W_H-2*GAP)

#define ITEM_L  CON_L
#define ITEM_T  CON_T
#define ITEM_W  CON_W
#define ITEM_H  30

#define REG_ADD_L  (ITEM_L+GAP)
#define REG_ADD_W  ((ITEM_W-3*GAP)/2)
#define REG_ADD_H  ITEM_H

#define REG_VALUE_L  (REG_ADD_L + GAP +REG_ADD_W)
#define REG_VALUE_W  REG_ADD_W
#define REG_VALUE_H  ITEM_H

#define LDEF_EDIT(root, varNum, nxtObj, ID,l, t, w, h,style,cursormode,pat,pre,str)	\
    DEF_EDITFIELD(varNum,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, WSTL_TXT_11,WSTL_BUTTON_3,WSTL_TXT_11,WSTL_TXT_11,   \
    reg_edit_keymap,reg_edit_callback, \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,pat,10,cursormode,pre,NULL,str)

#define LDEF_EDIT_S(root, varNum, nxtObj, ID,idl,idr,idup,iddown,l, t, w, h,style,cursormode,pat,pre,str)	\
    DEF_EDITFIELD(varNum,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idup,iddown,l,t,w,h, WSTL_TXT_11,WSTL_BUTTON_3,WSTL_TXT_11,WSTL_TXT_11,   \
    reg_edit_keymap,reg_edit_callback, \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,pat,10,cursormode,pre,NULL,str)


#define LDEF_TXT(root,varTxt,nxtObj,id,idl,idr,idup,iddown,l,t,w,h,resID,pstring)		\
    DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
    id,idl,idr,idup,iddown,l,t,w,h,WSTL_TXT_4,WSTL_TXT_3,WSTL_TXT_4,WSTL_TXT_4,\
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,resID,pstring)

#define LDEF_CON_BG(varCon,root,nxtObj,nxtInObj,l,t,w,h)		\
    DEF_CONTAINER(varCon,root,nxtObj,C_ATTR_ACTIVE,0, \
    1,0,0,0,0, l,t,w,h, WSTL_WIN_2,WSTL_WIN_2,WSTL_WIN_2,WSTL_WIN_2,   \
    con_keymap,con_callback,  \
    nxtInObj, 1,0)

#define LDEF_CON(varCon,root,nxtObj,nxtInObj,id,idl,idr,idUp,idDown,l,t,w,h,styleNormal,styleHi,styleSel,styleGary)		\
    DEF_CONTAINER(varCon,root,nxtObj,C_ATTR_ACTIVE,0, \
    id,idl,idr,idUp,idDown,l,t,w,h,styleNormal,styleHi,styleSel,styleGary,  \
    item_keymap,item_callback,  \
    nxtInObj, id,1)

#define LDEF_ITEM(root,var,nxtobj,edt_add,id,idl,idr,idup,iddown,l,t,w,h,styleNormal,styleHi,styleSel,styleGary,pat,pstring)\
    LDEF_CON(var,root,nxtobj,&edt_add,id,idl,idr,idup,iddown,l,t,w,h,styleNormal,styleHi,styleSel,styleGary)\
    LDEF_EDIT(&var,edt_add,NULL,id,l,t,w,h,NORMAL_EDIT_MODE,CURSOR_NORMAL,pat,NULL,pstring)\
	
#define LDEF_ITEM_TXT(root,var,nxtobj,txt,id,idl,idr,idup,iddown,l,t,w,h,styleNormal,styleHi,styleSel,styleGary,pstring)\
    LDEF_CON(var,root,nxtobj,&txt,id,idl,idr,idup,iddown,l,t,w,h,styleNormal,styleHi,styleSel,styleGary)\
    LDEF_TXT(&var,txt,NULL,id,idl,idr,idup,iddown,l,t,w,h,0,pstring)


#define LDEF_WIN(varWnd,nxtObj,l,t,w,h,focusID)		\
    DEF_CONTAINER(varWnd,NULL,NULL,C_ATTR_ACTIVE,0, \
	    1,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,   \
	    win_reg_keymap,win_reg_callback,  \
	    nxtObj, focusID,0)

LDEF_WIN (g_win_reg, &con_reg, W_L, W_T, W_W, W_H, 1 )

LDEF_CON_BG (con_reg,&g_win_reg,NULL,&con_reg_item_add,CON_L,CON_T,CON_W,CON_H)

LDEF_ITEM (&con_reg,con_reg_item_add,&con_reg_item_txt,edit_reg_add,1,2,2,5,3,REG_ADD_L,ITEM_T,REG_ADD_W,ITEM_H,
	WSTL_LCL_ITEM_NORMAL,WSTL_BUTTON_1,WSTL_LCL_ITEM_NORMAL,WSTL_LCL_ITEM_NORMAL,win_reg_add_pat,display_strs[P_BUF_BASE_IDX])

LDEF_ITEM_TXT (&con_reg,con_reg_item_txt,&con_reg_item1,txt_title_button,2,1,1,6,4,REG_VALUE_L,ITEM_T,REG_ADD_W,ITEM_H,WSTL_LCL_ITEM_NORMAL,\
	WSTL_BUTTON_1,WSTL_LCL_ITEM_NORMAL,WSTL_LCL_ITEM_NORMAL,display_strs[P_BUF_BASE_IDX+5])

LDEF_ITEM(&con_reg,con_reg_item1,&con_reg_item2,edit_reg_value1,3,4,4,1,5,REG_ADD_L,ITEM_T+(2*ITEM_H),REG_ADD_W,ITEM_H,\
	WSTL_LCL_ITEM_NORMAL,WSTL_BUTTON_1,WSTL_LCL_ITEM_NORMAL,WSTL_LCL_ITEM_NORMAL,win_reg_add_pat,display_strs[P_BUF_BASE_IDX+1])

LDEF_ITEM(&con_reg,con_reg_item2,&con_reg_item3,edit_reg_value2,4,3,3,2,6,REG_VALUE_L,ITEM_T+(2*ITEM_H),REG_ADD_W,ITEM_H,\
	WSTL_LCL_ITEM_NORMAL,WSTL_BUTTON_1,WSTL_LCL_ITEM_NORMAL,WSTL_LCL_ITEM_NORMAL,win_reg_add_pat,display_strs[P_BUF_BASE_IDX+2])

LDEF_ITEM(&con_reg,con_reg_item3,&con_reg_item4,edit_reg_value3,5,6,6,3,1,REG_ADD_L,ITEM_T+(3*ITEM_H),REG_ADD_W,ITEM_H,\
	WSTL_LCL_ITEM_NORMAL,WSTL_BUTTON_1,WSTL_LCL_ITEM_NORMAL,WSTL_LCL_ITEM_NORMAL,win_reg_add_pat,display_strs[P_BUF_BASE_IDX+3])

LDEF_ITEM(&con_reg,con_reg_item4,NULL,edit_reg_value4,6,5,5,4,2,REG_VALUE_L,ITEM_T+(3*ITEM_H),REG_ADD_W,ITEM_H,\
	WSTL_LCL_ITEM_NORMAL,WSTL_BUTTON_1,WSTL_LCL_ITEM_NORMAL,WSTL_LCL_ITEM_NORMAL,win_reg_add_pat,display_strs[P_BUF_BASE_IDX+4])




/*******************************************************************************
*	Local vriable & function declare
*******************************************************************************/
#define DEBUG_READ	(VACT_PASS + 1)
#define MOVE_LEFT	(VACT_PASS + 2)
#define MOVE_RIGHT	(VACT_PASS + 3)
#define MOVE_UP		(VACT_PASS + 4)
#define MOVE_DOWN	(VACT_PASS + 5)
#define VAL_INC     (VACT_PASS + 6)
#define VAL_DEC     (VACT_PASS + 7)


#define write_value(add,val) (*(volatile UINT32 *)(add)) = (val)
#define read_value(add) (*(volatile UINT32 *)(add))

#define KSEG0_BASE 0X80000000
#define KSEG1_HIGH 0XC0000000

#define MAX_OBJ_ID 6
#define REG_ALIGN_MARK 0xFFFFFFFC /*4 byte aligned*/
#define adjust_addr(add) (add = add & REG_ALIGN_MARK)

#define MODE_BROSWER 0 
#define MODE_EDIT 1


/*we asume that the broswer mode is the default mode
 *when user entry ok, entry into the edit mode,re-entry ok back to broswer mode 
 */
static POBJECT_HEAD pre_tr = NULL;
static UINT32 read_base_add;
static UINT32 mode_flag = 0;  


static UINT32 ComUniStr2Hex(const UINT16* string);
static void do_read_memory();
static PRESULT edit_proc(POBJECT_HEAD pObj,VACTION act,UINT32 item_id,UINT32 pos);
static void set_default_val();
/*******************************************************************************
*	key mapping and event callback definition
*******************************************************************************/

static VACTION reg_edit_keymap(POBJECT_HEAD pObj, UINT32 key )
{
	VACTION Action = VACT_PASS;
	if(pre_tr != pObj)
	{
		mode_flag = MODE_BROSWER;
	}

	switch (key)
	{
		case V_KEY_LEFT:
			Action = (MODE_BROSWER==mode_flag)?MOVE_LEFT:VACT_EDIT_LEFT;
			break;
		case V_KEY_RIGHT:
			Action = (MODE_BROSWER==mode_flag)?MOVE_RIGHT:VACT_EDIT_RIGHT;
			break;
		case V_KEY_UP:
			Action = (MODE_BROSWER==mode_flag)?MOVE_UP:VAL_INC;
			break;
		case V_KEY_DOWN:
			Action = (MODE_BROSWER==mode_flag)?MOVE_DOWN:VAL_DEC;
			break;
		case V_KEY_0:
		case V_KEY_1:
		case V_KEY_2:
		case V_KEY_3:
		case V_KEY_4:
		case V_KEY_5:
		case V_KEY_6:
		case V_KEY_7:
		case V_KEY_8:
		case V_KEY_9:
			Action = key - V_KEY_0 + VACT_NUM_0;
			break;
		case V_KEY_ENTER:
			mode_flag = (MODE_BROSWER==mode_flag)?MODE_EDIT:MODE_BROSWER;
			break;

		default:
			break;
	}
	pre_tr = pObj;

	return Action;
}

static PRESULT reg_edit_callback ( POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2 )
{
	PRESULT ret = PROC_PASS;
	VACTION act;
	PEDIT_FIELD pedit;
	pedit =(PEDIT_FIELD)pObj;
	UINT32 pos = pedit->bCursor;
	UINT32 item_id = pedit->head.bID;
	switch ( event )
	{
		case EVN_UNKNOWN_ACTION:
			act = (VACTION)(param1 >> 16);
			ret = edit_proc(pObj,act,item_id,pos);
		default:
			break;
	}

	return ret;
}


static VACTION item_keymap ( POBJECT_HEAD pObj, UINT32 key )
{
	VACTION act = VACT_PASS;
	switch(key)
	{
		case V_KEY_ENTER:
			act = DEBUG_READ;
			break;
		default:
			break;
	}

	return act;	
}


static PRESULT item_callback ( POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2 )
{
	PRESULT ret = PROC_PASS;
	UINT32 idx=0;
	POBJECT_HEAD phead = (POBJECT_HEAD)&con_reg;
	VACTION act;

	switch(event)
	{
		case EVN_UNKNOWN_ACTION:
			act = (VACTION)(param1 >> 16);
			if((DEBUG_READ == act) && (pObj==(POBJECT_HEAD)&con_reg_item_txt))
			{
				do_read_memory();
				/*idx = OSD_GetFocusID(phead);
				idx = get_next_fcousID(idx);
				OSD_ChangeFocus(phead,idx,C_UPDATE_FOCUS | C_DRAW_SIGN_EVN_FLG);
				*/
			}
			break;
		default:
			break;
			
	}
	
	return ret;	
}


static VACTION con_keymap ( POBJECT_HEAD pObj, UINT32 key )
{	
	VACTION act = VACT_PASS;

	switch ( key )
	{
		case V_KEY_LEFT:
			act = VACT_CURSOR_LEFT;
			break;
		case V_KEY_RIGHT:
			act = VACT_CURSOR_RIGHT;
			break;
		case V_KEY_UP:
			act = VACT_CURSOR_UP;
			break;
		case V_KEY_DOWN:
			act = VACT_CURSOR_DOWN;
			break;
		default:
			act = VACT_PASS;
	}

	return act;
}

static PRESULT con_callback ( POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2 )
{
	PRESULT ret = PROC_PASS;

	switch ( event )
	{
		case EVN_PRE_OPEN:
			break;
		default:
			break;
	}
	return ret;
}


static VACTION win_reg_keymap ( POBJECT_HEAD pObj, UINT32 key )
{
	VACTION act = VACT_PASS;
	switch(key)
	{
		case V_KEY_EXIT:
			act = VACT_CLOSE;
			break;
		default:
			break;
	}
	
	return act;
}


static PRESULT win_reg_callback ( POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2 )
{
	PRESULT ret = PROC_PASS;
	VACTION act;

	switch ( event )
	{
		case EVN_PRE_OPEN:
			set_default_val();
			OSD_SetContainerFocus((CONTAINER*)&con_reg,1);
			break;
		default:
			break;
	}

	return ret;
}


static void set_default_val()
{
	UINT32 index = P_BUF_BASE_IDX;
	for(;index<P_BUF_BASE_IDX+5;index++)
	{
		wincom_AscToMBUni("00000000",display_strs[index]);
	}
	wincom_AscToMBUni("read",display_strs[index]);
}




static PRESULT edit_proc(POBJECT_HEAD pObj,VACTION act,UINT32 item_id,UINT32 pos)
{
	PRESULT ret = PROC_LOOP;
	UINT16 * ptar = NULL;
	UINT32 index = P_BUF_BASE_IDX;
	UINT32 idx = 0;
	POBJECT_HEAD pthis = pObj;
	POBJECT_HEAD pobj = (POBJECT_HEAD)&con_reg;
	UINT32 addr = 0;
	UINT32 val = 0;
	UINT16 tmp[15]={0};
	UINT16 rep =0;
	
	if(item_id==1)
	{
		/*the con_reg_item_add is 20*/
		ptar = (UINT16 *)&display_strs[P_BUF_BASE_IDX+item_id-1];
	}
	else
	{
		/*the con_reg_item1 is from 21*/
		ptar = (UINT16 *)&display_strs[P_BUF_BASE_IDX+item_id-2];
	}

	switch(act)
	{
		case MOVE_LEFT:
			OSD_ChangeFocus(pobj,pthis->pRoot->bLeftID,C_UPDATE_FOCUS | C_DRAW_SIGN_EVN_FLG);
			goto leave_nosave;	
		case MOVE_RIGHT:
			OSD_ChangeFocus(pobj,pthis->pRoot->bRightID,C_UPDATE_FOCUS | C_DRAW_SIGN_EVN_FLG);
			goto leave_nosave;				
		case MOVE_UP:
			OSD_ChangeFocus(pobj,pthis->pRoot->bUpID,C_UPDATE_FOCUS | C_DRAW_SIGN_EVN_FLG);
			goto leave_nosave;	
		case MOVE_DOWN:
			OSD_ChangeFocus(pobj,pthis->pRoot->bDownID,C_UPDATE_FOCUS | C_DRAW_SIGN_EVN_FLG);
			goto leave_nosave;	
		case VAL_INC: 
			increase_val_inuncode(ptar,pos);
			goto leave_save;
		case VAL_DEC:    
			decrease_val_inuncode(ptar,pos);
			goto leave_save;
		default:
			break;				
	}
	OSD_TrackObject(pthis, C_UPDATE_ALL);
	return ret;

leave_nosave: /*change focus*/
	OSD_TrackObject(pobj, C_UPDATE_ALL);
	return ret;

leave_save:
	if(1 != item_id)
	{
		addr = read_base_add+(item_id-3)*sizeof(UINT32);
		val = ComUniStr2Hex((UINT16 *)&display_strs[P_BUF_BASE_IDX+item_id -2]);
		write_value(addr,val);
	}
	OSD_TrackObject(pthis, C_UPDATE_ALL);
	return ret;
}



/* default will read 16 byte data from the add user specfied*/
static void do_read_memory()
{
	UINT8 buf[40];
	UINT8 buf_msg[50];
	UINT32 val;
	UINT32 add = 0 ;
	UINT32 i=0;
	UINT8 back_saved;
	
	
	add = ComUniStr2Hex((UINT16 *)&display_strs[P_BUF_BASE_IDX+i]);
  	/*check the add is valid, we asume that is within kseg0-kseg1*/
	if((add<KSEG0_BASE)||(add>KSEG1_HIGH)|| (add==0))
		return;
	if(add&(~REG_ALIGN_MARK))
	{
		adjust_addr(add);
		sprintf(buf_msg,"%s %8X","bad address,will aligned to",add);
		sprintf(buf,"%.8X",add);
		ComAscStr2Uni(buf,(UINT16 *)&display_strs[P_BUF_BASE_IDX]);

		win_compopup_init(WIN_POPUP_TYPE_OK);
		win_compopup_set_frame(TV_OFFSET_L+(OSD_MAX_WIDTH-450)>>1,TV_OFFSET_L+(OSD_MAX_HEIGHT-100)>>1 , 450, 100);
		win_compopup_set_msg_ext((char *)buf_msg,NULL,0);
		win_compopup_open_ext(&back_saved);
	}
	read_base_add = add;
	

	for(;i<4;i++)
	{
		val = read_value(add);

		sprintf(buf,"%.8X",val);
		ComAscStr2Uni(buf,(UINT16 *)&display_strs[P_BUF_BASE_IDX+i+1]);
		add = add +sizeof(UINT32);
	}
	OSD_TrackObject((POBJECT_HEAD)&con_reg, C_UPDATE_ALL);
	
	return;
}




static UINT32 ComUniStr2Hex(const UINT16* string)
{
	UINT8 i,len,c;
	UINT32 val;
	
	val = 0;
	len = ComUniStrLen(string);
	
	for(i=0;i<len;i++)
	{
#if(SYS_CPU_ENDIAN==ENDIAN_LITTLE)
		c = (UINT8)(string[i] >> 8);
#else
       	c = (UINT8)(string[i]);
#endif
		if(c>='0' && c<='9')
			val = val*16 + c - '0';
		if(c>='a' && c<='f')
			val = val*16 + 10 + c - 'a';
		if(c>='A' && c<='F')
			val = val*16 + 10 + c - 'A';
	}
	
	return val;	
}

#endif

