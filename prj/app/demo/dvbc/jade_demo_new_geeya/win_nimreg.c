/*****************************************************************************
 *    Copyright (C)2007 Ali Corporation. All Rights Reserved.
 *
 *    File:    win_nimreg.c
 *    Description:    this file describes a window to show nim related registers.
 *    History: 
 *           Date            Athor        Version          Reason
 *	  ========================================================================
 *	1. 07/05/27		Grady			create file
 *	2. 07/07/25		penghui			Modify to two cloume, add 8 functions, add BerPer, infoBar.
 *****************************************************************************/


#include <sys_config.h>

#ifdef NIM_REG_ENABLE

#include <types.h>
#include <osal/osal.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
//#include <api/libosd/osd_lib.h>
//#include <api/libosd/osd_common.h>
#include <hld/nim/nim_dev.h>
#include "win_com.h"

#include "win_signal.h"
#include "win_com_popup.h"
#include "copper_common/com_api.h"
#include <api/libosd/osd_lib.h>

#include "string.id"
#include "osdobjs_def.h"
#include "osd_config.h"

/*******************************************************************************
*	Global variables
*******************************************************************************/

#define NIMREG_FOR_INTER_TEST 0
#define NIMREG_FOR_OUTER_USE 1

#define NIMREG_MODE NIMREG_FOR_OUTER_USE

extern CONTAINER g_win_nimreg;
extern CONTAINER g_win_signal;
extern INT32 nim_s3202_read(UINT8 bMemAdr, UINT8 *pData, UINT8 bLen);
extern INT32 nim_s3202_write(UINT8 bMemAdr, UINT8 *pData, UINT8 bLen);
static VACTION nimreg_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT nimreg_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);
static VACTION nimreg_edit_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT nimreg_edit_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);
static VACTION nimreg_sel_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT nimreg_sel_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

static PRESULT nimreg_message_proc ( UINT32 msg_type, UINT32 msg_code );
BOOL nimRegFuncFlag(UINT8 indx, BOOL val, BOOL dir);
static void nimreg_ber_close (void);
static void nimreg_signal_close (void);

void 	  display_dynamic_vision(void);
static void display_static_vision(void);
static void update_register(EDIT_FIELD *edf, UINT32 param);
static void update_bit(EDIT_FIELD *edf, UINT32 param);
static void wincom_named_funcs(void);
static UINT32 ComUniStr2Hex(const UINT16* string);

typedef struct
{
	UINT16  p_number;
	struct 
	{
		UINT8 p_page;
		UINT8 p_pos;
	} p_param[256] ;

} typeRegPrint, *PtypeRegPrint;

 typeRegPrint  RegPrint ;

static void RegPrintProc(PtypeRegPrint  pRegPrint);
void nimRegPrint();

/*******************************************************************************
*	Objects definition
*******************************************************************************/
// 608 * 430 -> 480 * 360

#define WIN_SH_IDX		WSTL_POP_WIN_1
#define WIN_HL_IDX		WSTL_POP_WIN_1
#define WIN_SL_IDX		WSTL_POP_WIN_1
#define WIN_GRY_IDX		WSTL_POP_WIN_1

#define CON_SH_IDX		WSTL_TXT_POPWIN
#define CON_HL_IDX		WSTL_BUTTON_1
#define CON_SL_IDX		WSTL_BUTTON_1
#define CON_GRY_IDX		WSTL_TXT_POPWIN

#define TXT_SH_IDX		WSTL_TXT_POPWIN
#define TXT_HL_IDX		WSTL_TXT_5
#define TXT_SL_IDX		WSTL_TXT_5
#define TXT_GRY_IDX		WSTL_TXT_10

#define EDT_SH_IDX		WSTL_TXT_POPWIN
#define EDT_HL_IDX		WSTL_BUTTON_5
#define EDT_SL_IDX		WSTL_BUTTON_3
#define EDT_GRY_IDX		WSTL_TXT_10

#define SEL_SH_IDX		WSTL_TXT_POPWIN
#define SEL_HL_IDX		WSTL_BUTTON_5
#define SEL_SL_IDX		WSTL_BUTTON_3
#define SEL_GRY_IDX		WSTL_TXT_10

#define	W_L		60//30
#define	W_T	30
#define	W_W	490
#define	W_H	240
#define   GP 		5

#define SubW_L  65//35
#define SubW_T  65
#define SubW_W 480
#define SubW_H  200

#define CON_L	(SubW_L )
#define CON_T	(SubW_T )
#define CON_W	(SubW_W /3 )//160
#define CON_H	(SubW_H / 8 )//25


#define VACT_EDIT_INCREASE (VACT_PASS + 1)
#define VACT_EDIT_DECREASE (VACT_EDIT_INCREASE + 1)
#define VACT_SWITH_CURSOR (VACT_EDIT_DECREASE + 1)
#define VACT_SWITH_SELECT (VACT_SWITH_CURSOR + 1)


extern CONTAINER nimreg_con1;
extern CONTAINER nimreg_con2;
extern CONTAINER nimreg_con3;
extern CONTAINER nimreg_con4;
extern CONTAINER nimreg_con5;
extern CONTAINER nimreg_con6;
extern CONTAINER nimreg_con7;
extern CONTAINER nimreg_con8;
extern CONTAINER nimreg_con9;
extern CONTAINER nimreg_con10;
extern CONTAINER nimreg_con11;
extern CONTAINER nimreg_con12;
extern CONTAINER nimreg_con13;
extern CONTAINER nimreg_con14;
extern CONTAINER nimreg_con15;
extern CONTAINER nimreg_con16;
extern CONTAINER nimreg_con17;
extern CONTAINER nimreg_con18;
extern CONTAINER nimreg_con19;
extern CONTAINER nimreg_con20;
extern CONTAINER nimreg_con21;
extern CONTAINER nimreg_con22;
extern CONTAINER nimreg_con23;
extern CONTAINER nimreg_con24;
extern TEXT_FIELD nimreg_txt1;
extern TEXT_FIELD nimreg_txt2;
extern TEXT_FIELD nimreg_txt3;
extern TEXT_FIELD nimreg_txt4;
extern TEXT_FIELD nimreg_txt5;
extern TEXT_FIELD nimreg_txt6;
extern TEXT_FIELD nimreg_txt7;
extern TEXT_FIELD nimreg_txt8;
extern TEXT_FIELD nimreg_txt9;
extern TEXT_FIELD nimreg_txt10;
extern TEXT_FIELD nimreg_txt11;
extern TEXT_FIELD nimreg_txt12;
extern TEXT_FIELD nimreg_txt13;
extern TEXT_FIELD nimreg_txt14;
extern TEXT_FIELD nimreg_txt15;
extern TEXT_FIELD nimreg_txt16;
extern TEXT_FIELD nimreg_txt17;
extern TEXT_FIELD nimreg_txt18;
extern TEXT_FIELD nimreg_txt19;
extern TEXT_FIELD nimreg_txt20;
extern TEXT_FIELD nimreg_txt21;
extern TEXT_FIELD nimreg_txt22;
extern TEXT_FIELD nimreg_txt23;
extern TEXT_FIELD nimreg_txt24;
extern EDIT_FIELD nimreg_edt1;
extern EDIT_FIELD nimreg_edt2;
extern EDIT_FIELD nimreg_edt3;
extern EDIT_FIELD nimreg_edt4;
extern EDIT_FIELD nimreg_edt5;
extern EDIT_FIELD nimreg_edt6;
extern EDIT_FIELD nimreg_edt7;
extern EDIT_FIELD nimreg_edt8;
extern EDIT_FIELD nimreg_edt9;
extern EDIT_FIELD nimreg_edt10;
extern EDIT_FIELD nimreg_edt11;
extern EDIT_FIELD nimreg_edt12;
extern EDIT_FIELD nimreg_edt13;
extern EDIT_FIELD nimreg_edt14;
extern EDIT_FIELD nimreg_edt15;
extern EDIT_FIELD nimreg_edt16;

extern MULTISEL nimreg_sel1;
extern MULTISEL nimreg_sel2;
extern MULTISEL nimreg_sel3;
extern MULTISEL nimreg_sel4;
extern MULTISEL nimreg_sel5;
extern MULTISEL nimreg_sel6;
extern MULTISEL nimreg_sel7;
extern MULTISEL nimreg_sel8;

extern CONTAINER g_con_ber;
extern TEXT_FIELD g_txt_ber0;
extern TEXT_FIELD g_txt_ber1;
extern TEXT_FIELD g_txt_ber2;
extern TEXT_FIELD g_txt_ber3;

CONTAINER *pCons[] = 
{
	&nimreg_con1,
	&nimreg_con2,
	&nimreg_con3,
	&nimreg_con4,
	&nimreg_con5,
	&nimreg_con6,
	&nimreg_con7,
	&nimreg_con8,
	&nimreg_con9,
	&nimreg_con10,
	&nimreg_con11,
	&nimreg_con12,
	&nimreg_con13,
	&nimreg_con14,
	&nimreg_con15,
	&nimreg_con16,
	&nimreg_con17,
	&nimreg_con18,
	&nimreg_con19,
	&nimreg_con20,
	&nimreg_con21,
	&nimreg_con22,
	&nimreg_con23,
	&nimreg_con24,
};

MULTISEL *pSels[] = 
{
	&nimreg_sel1,
	&nimreg_sel2,
	&nimreg_sel3,
	&nimreg_sel4,
	&nimreg_sel5,
	&nimreg_sel6,
	&nimreg_sel7,
	&nimreg_sel8,
};


UINT16 edit_str[16][5];
UINT16 text_str[24][10];
UINT16 ber_str[4][20];

char win_nimreg_pat[] = "s2";
UINT16 onoff_ids[] =
{
   RS_COMMON_OFF,
   RS_COMMON_ON,
};
static UINT32 m_nPos = 0;
static UINT32 m_pagenum = 0;
static ID	 nimreg_timer = OSAL_INVALID_ID;
PMULTISEL pNimReg_Time_Msel;


#define LDEF_WIN(varCon,nxtObj,l,t,w,h,focusID)		\
    DEF_CONTAINER(varCon,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    nimreg_keymap,nimreg_callback,  \
    nxtObj, focusID,0)

#define LDEF_CON(root, varCon,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,conobj,focusID)		\
    DEF_CONTAINER(varCon,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    NULL,NULL,  \
    conobj, focusID,1)

#define LDEF_TXT(root,varTxt,nxtObj,l,t,w,h,txtStr) \
   DEF_TEXTFIELD(varTxt, root, nxtObj, C_ATTR_ACTIVE, 0, \
   0, 0, 0, 0, 0, l, t, w, h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX, \
   NULL, NULL, \
   C_ALIGN_CENTER|C_ALIGN_VCENTER, 0, 0, 0, txtStr)

#define LDEF_SEL(root, varSel, nxtObj, ID, l, t, w, h,style,cur,cnt,ptabl)	\
    DEF_MULTISEL(varSel,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, SEL_SH_IDX,SEL_HL_IDX,SEL_SL_IDX,SEL_GRY_IDX,   \
    nimreg_sel_keymap,nimreg_sel_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,style,ptabl,cur,cnt)

#define LDEF_EDT(root, varEdt, nxtObj, ID, l, t, w, h,style,cursormode,pat,pre,sub,edtStr)	\
    DEF_EDITFIELD(varEdt,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, EDT_SH_IDX,EDT_HL_IDX,EDT_SL_IDX,EDT_GRY_IDX,   \
    nimreg_edit_keymap,nimreg_edit_callback, \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,style,pat,SELECT_EDIT_MODE,cursormode,pre,sub,edtStr)

#define LDEF_MENU_ITEM_EDT(root,varCon,nxtObj,varTxt,varEdt,ID,IDl,IDr,IDu,IDd,l,t,w,h,txtStr,edtStr)	\
    LDEF_CON(&root,varCon,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,&varTxt,ID)	\
    LDEF_TXT(&varCon,varTxt,&varEdt,l ,t ,w/2,h,txtStr)	\
    LDEF_EDT(&varCon,varEdt,NULL,ID,l+w/2 ,t ,w/2,h,NORMAL_EDIT_MODE, CURSOR_NO, win_nimreg_pat,NULL,NULL, edtStr)

#define LDEF_MENU_ITEM_SEL(root,varCon,nxtObj,varTxt,varSel,ID,IDl,IDr,IDu,IDd,l,t,w,h,txtStr,ptabl) \
    LDEF_CON(&root,varCon,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,&varTxt,ID)   \
    LDEF_TXT(&varCon,varTxt,&varSel,l ,t ,w/2,h,txtStr)    \
    LDEF_SEL(&varCon,varSel,NULL ,ID, l+w/2 ,t ,w/2,h,STRING_ID,0,2,ptabl )


#define LDEF_CON_BER(varCon,l,t,w,h,conobj)		\
    DEF_CONTAINER(varCon,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    NULL,NULL,  \
    conobj, 0,1)
    
#define LDEF_TXT_BER(root,varTxt,nxtObj,l,t,w,h,sh_idx,txtStr) \
   DEF_TEXTFIELD(varTxt, root, nxtObj, C_ATTR_ACTIVE, 0, \
   0, 0, 0, 0, 0, l, t, w, h, TXT_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX, \
   NULL, NULL, \
   C_ALIGN_RIGHT|C_ALIGN_VCENTER, 0, 0, 0, txtStr)

#define LDEF_BER(varCon,l,t) \
    LDEF_CON_BER(varCon,l,t,480,30,&g_txt_ber0)   \
    LDEF_TXT_BER(&varCon,g_txt_ber0,&g_txt_ber1,l+20,t+2,100,26,WIN_SH_IDX,ber_str[0])    \
    LDEF_TXT_BER(&varCon,g_txt_ber1,&g_txt_ber2,l+120,t+2,100,26,WSTL_TXT_14,ber_str[1])    \
    LDEF_TXT_BER(&varCon,g_txt_ber2,&g_txt_ber3,l+220,t+2,140,26,WIN_SH_IDX,ber_str[2])    \
    LDEF_TXT_BER(&varCon,g_txt_ber3,NULL,l+360,t+2,100,26,WSTL_TXT_14,ber_str[3]) 




/*******************************************************************************
*	Draw the windows , include BerPer. (the infoBar are draw outside this file)
*******************************************************************************/
#define WIN g_win_nimreg

LDEF_BER(g_con_ber,35,270)

LDEF_WIN(WIN,&nimreg_con1,W_L,W_T,W_W,W_H,1)

LDEF_MENU_ITEM_EDT(WIN,nimreg_con1 ,&nimreg_con2 ,nimreg_txt1 ,nimreg_edt1 ,\
		1 ,24, 9 , 24,2 ,CON_L,CON_T + CON_H * 0,CON_W,CON_H,text_str[0],edit_str[0])
LDEF_MENU_ITEM_EDT(WIN,nimreg_con2 ,&nimreg_con3 ,nimreg_txt2 ,nimreg_edt2 ,\
		2 ,17, 10, 1 ,3 ,CON_L,CON_T + CON_H * 1,CON_W,CON_H,text_str[1],edit_str[1])
LDEF_MENU_ITEM_EDT(WIN,nimreg_con3 ,&nimreg_con4 ,nimreg_txt3 ,nimreg_edt3 ,\
		3 ,18, 11, 2 ,4 ,CON_L,CON_T + CON_H * 2,CON_W,CON_H,text_str[2],edit_str[2])
LDEF_MENU_ITEM_EDT(WIN,nimreg_con4 ,&nimreg_con5 ,nimreg_txt4 ,nimreg_edt4 ,\
		4 ,19, 12, 3 ,5 ,CON_L,CON_T + CON_H * 3,CON_W,CON_H,text_str[3],edit_str[3])           
LDEF_MENU_ITEM_EDT(WIN,nimreg_con5 ,&nimreg_con6 ,nimreg_txt5 ,nimreg_edt5 ,\
		5 ,20, 13, 4 ,6 ,CON_L,CON_T + CON_H * 4,CON_W,CON_H,text_str[4],edit_str[4])           
LDEF_MENU_ITEM_EDT(WIN,nimreg_con6 ,&nimreg_con7 ,nimreg_txt6 ,nimreg_edt6 ,\
		6 ,21, 14, 5 ,7 ,CON_L,CON_T + CON_H * 5,CON_W,CON_H,text_str[5],edit_str[5])           
LDEF_MENU_ITEM_EDT(WIN,nimreg_con7 ,&nimreg_con8 ,nimreg_txt7 ,nimreg_edt7 ,\
		7 ,22, 15, 6 ,8 ,CON_L,CON_T + CON_H * 6,CON_W,CON_H,text_str[6],edit_str[6])           
LDEF_MENU_ITEM_EDT(WIN,nimreg_con8 ,&nimreg_con9 ,nimreg_txt8 ,nimreg_edt8 ,\
		8 ,23, 16, 7 ,9 ,CON_L,CON_T + CON_H * 7,CON_W,CON_H,text_str[7],edit_str[7])           
LDEF_MENU_ITEM_EDT(WIN,nimreg_con9 ,&nimreg_con10,nimreg_txt9 ,nimreg_edt9 ,\
		9 ,1 , 17, 8 ,10,CON_L + CON_W,CON_T + CON_H * 0,CON_W,CON_H,text_str[8] ,edit_str[8] ) 
LDEF_MENU_ITEM_EDT(WIN,nimreg_con10,&nimreg_con11,nimreg_txt10,nimreg_edt10,\
		10,2 , 18, 9 ,11,CON_L + CON_W,CON_T + CON_H * 1,CON_W,CON_H,text_str[9] ,edit_str[9] ) 
LDEF_MENU_ITEM_EDT(WIN,nimreg_con11,&nimreg_con12,nimreg_txt11,nimreg_edt11,\
		11,3 , 19, 10,12,CON_L + CON_W,CON_T + CON_H * 2,CON_W,CON_H,text_str[10],edit_str[10]) 
LDEF_MENU_ITEM_EDT(WIN,nimreg_con12,&nimreg_con13,nimreg_txt12,nimreg_edt12,\
		12,4 , 20, 11,13,CON_L + CON_W,CON_T + CON_H * 3,CON_W,CON_H,text_str[11],edit_str[11]) 
LDEF_MENU_ITEM_EDT(WIN,nimreg_con13,&nimreg_con14,nimreg_txt13,nimreg_edt13,\
		13,5 , 21, 12,14,CON_L + CON_W,CON_T + CON_H * 4,CON_W,CON_H,text_str[12],edit_str[12]) 
LDEF_MENU_ITEM_EDT(WIN,nimreg_con14,&nimreg_con15,nimreg_txt14,nimreg_edt14,\
		14,6 , 22, 13,15,CON_L + CON_W,CON_T + CON_H * 5,CON_W,CON_H,text_str[13],edit_str[13]) 
LDEF_MENU_ITEM_EDT(WIN,nimreg_con15,&nimreg_con16,nimreg_txt15,nimreg_edt15,\
		15,7 , 23, 14,16,CON_L + CON_W,CON_T + CON_H * 6,CON_W,CON_H,text_str[14],edit_str[14]) 
LDEF_MENU_ITEM_EDT(WIN,nimreg_con16,&nimreg_con17,nimreg_txt16,nimreg_edt16,\
		16,8 , 24, 15,17,CON_L + CON_W,CON_T + CON_H * 7,CON_W,CON_H,text_str[15],edit_str[15])
LDEF_MENU_ITEM_SEL(WIN,nimreg_con17,&nimreg_con18,nimreg_txt17,nimreg_sel1 ,\
		17,9 , 2 , 16,18,CON_L + CON_W * 2,CON_T + CON_H * 0,CON_W,CON_H,text_str[16],onoff_ids)
LDEF_MENU_ITEM_SEL(WIN,nimreg_con18,&nimreg_con19,nimreg_txt18,nimreg_sel2 ,\
		18,10, 3 , 17,19,CON_L + CON_W * 2,CON_T + CON_H * 1,CON_W,CON_H,text_str[17],onoff_ids)
LDEF_MENU_ITEM_SEL(WIN,nimreg_con19,&nimreg_con20,nimreg_txt19,nimreg_sel3 ,\
		19,11, 4 , 18,20,CON_L + CON_W * 2,CON_T + CON_H * 2,CON_W,CON_H,text_str[18],onoff_ids)
LDEF_MENU_ITEM_SEL(WIN,nimreg_con20,&nimreg_con21,nimreg_txt20,nimreg_sel4 ,\
		20,12, 5 , 19,21,CON_L + CON_W * 2,CON_T + CON_H * 3,CON_W,CON_H,text_str[19],onoff_ids)
LDEF_MENU_ITEM_SEL(WIN,nimreg_con21,&nimreg_con22,nimreg_txt21,nimreg_sel5 ,\
		21,13, 6 , 20,22,CON_L + CON_W * 2,CON_T + CON_H * 4,CON_W,CON_H,text_str[20],onoff_ids)
LDEF_MENU_ITEM_SEL(WIN,nimreg_con22,&nimreg_con23,nimreg_txt22,nimreg_sel6 ,\
		22,14, 7 , 21,23,CON_L + CON_W * 2,CON_T + CON_H * 5,CON_W,CON_H,text_str[21],onoff_ids)
LDEF_MENU_ITEM_SEL(WIN,nimreg_con23,&nimreg_con24,nimreg_txt23,nimreg_sel7 ,\
		23,15, 8 , 22,24,CON_L + CON_W * 2,CON_T + CON_H * 6,CON_W,CON_H,text_str[22],onoff_ids)
LDEF_MENU_ITEM_SEL(WIN,nimreg_con24,NULL, nimreg_txt24,nimreg_sel8 ,\
		24,16, 1 , 23,1 ,CON_L + CON_W * 2,CON_T + CON_H * 7,CON_W,CON_H,text_str[23],onoff_ids)



/*******************************************************************************
*	Functions 
*******************************************************************************/
static UINT8 GetByte(UINT32 nOffset)
{
#if 1
    UINT8 bValue;
    if(nOffset >= 256)
        return 0xFF;
	nim_s3202_read(nOffset, &bValue, 1);
    return bValue;
#else
	return 0xff;
#endif
}

static BOOL SetByte(UINT32 nOffset, UINT8 bValue)
{
#if 1
    if(nOffset >= 256)
        return FALSE;
	nim_s3202_write(nOffset, &bValue, 1);
    return TRUE;
#else 
	return TRUE;
#endif
}


static VACTION nimreg_keymap(POBJECT_HEAD pObj, UINT32 key)
{

	VACTION act = VACT_PASS;

	switch (key)
	{
		case V_KEY_UP:
			act = VACT_CURSOR_UP;
			m_nPos = (m_nPos == 0 ) ? 23 : m_nPos -1;
			break;
		case V_KEY_DOWN:
			act = VACT_CURSOR_DOWN;
			m_nPos = (m_nPos == 23 ) ? 0 : m_nPos +1;
			break;
		case V_KEY_LEFT:
			act = VACT_CURSOR_LEFT;
			m_nPos = (m_nPos < 8 ) ? ((m_nPos==0)?23:(m_nPos + 15)) : m_nPos -8;
			break;
		case V_KEY_RIGHT:
			act = VACT_CURSOR_RIGHT;
			m_nPos = (m_nPos > 15 ) ? ((m_nPos==23)?0:(m_nPos - 15)) : m_nPos + 8;
			break;
		case V_KEY_EXIT:
		case V_KEY_MENU:
			act = VACT_CLOSE;
			break;
		case V_KEY_P_UP:
			m_pagenum = (m_pagenum == 0 ) ? 15 : m_pagenum -1;
			display_dynamic_vision();
			break;
		case V_KEY_P_DOWN:
			m_pagenum = (m_pagenum == 15 ) ? 0 : m_pagenum +1;
			display_dynamic_vision();
			break;
		default:
			act = VACT_PASS;
	}

	return act;
}


static PRESULT nimreg_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	EDIT_FIELD *Pedf;
	UINT32 i;
	Pedf = ( EDIT_FIELD* )pObj;
	switch (event)
	{
		case EVN_PRE_OPEN:
			{
			display_static_vision();
			display_dynamic_vision();
			}
			break;
		case EVN_POST_OPEN:
			break;


#if (NIMREG_MODE == NIMREG_FOR_INTER_TEST)		
		case EVN_PRE_CLOSE:
			//nimreg_signal_close();
			nimreg_ber_close ();
			break;
		case EVN_POST_CLOSE:
			{
			for (i = 0; i < 8 ; i++)
				{
				OSD_SetMultiselSel(pSels[i],FALSE);
				nimRegFuncFlag(i, FALSE, TRUE);
				}
			}
			break;
		//case EVN_MSG_GOT:
			//if(nimRegFuncFlag(6, FALSE, FALSE)== TRUE)
			//nimreg_message_proc(param1, param2 );
			//if (CTRL_MSG_SUBTYPE_CMD_UPDATE_VIEW == param1)
			//	{
			//		OSD_SetMultiselSel(pNimReg_Time_Msel,FALSE);
			//		OSD_TrackObject((POBJECT_HEAD)pNimReg_Time_Msel, C_UPDATE_ALL);
			//	}
			//break;
#endif
		default:
			break;
			
	}
	return ret;
}



static VACTION nimreg_edit_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION Action = VACT_PASS;
	EDIT_FIELD * edf = (EDIT_FIELD *)pObj;

	switch (key)
	{
		case V_KEY_UP:
			if (edf->bCursorMode == CURSOR_NORMAL)
				Action = VACT_EDIT_INCREASE;
			else
				Action = VACT_PASS;			
			break;
		case V_KEY_DOWN:
			if (edf->bCursorMode == CURSOR_NORMAL)
				Action = VACT_EDIT_DECREASE;
			else
				Action = VACT_PASS;
			break;
		case V_KEY_LEFT:
			if (edf->bCursorMode == CURSOR_NORMAL)
				Action = VACT_EDIT_LEFT;
			else
				Action = VACT_PASS;
			break;
		case V_KEY_RIGHT:
			if (edf->bCursorMode == CURSOR_NORMAL)
				Action = VACT_EDIT_RIGHT;
			else
				Action = VACT_PASS;
			break;
		case V_KEY_0:	case V_KEY_1:	case V_KEY_2:	case V_KEY_3:
		case V_KEY_4:	case V_KEY_5:	case V_KEY_6:	case V_KEY_7:
		case V_KEY_8:	case V_KEY_9:
			Action = key - V_KEY_0 + VACT_NUM_0;
			break;
		case V_KEY_ENTER:
			Action = VACT_SWITH_CURSOR;
			break;
			
#if (NIMREG_MODE == NIMREG_FOR_INTER_TEST)
		case V_KEY_FAV:
			Action = VACT_SWITH_SELECT;
			break;
#endif
		
		default:
			break;
	}

	return Action;
}

static PRESULT nimreg_edit_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	VACTION unact;
	EDIT_FIELD* edf;
	
	edf = ( EDIT_FIELD* )pObj;
	switch ( event )
	{
		case EVN_UNKNOWN_ACTION:
			unact = ( VACTION)(param1 >> 16 );
			if(unact == VACT_EDIT_INCREASE)
			{
				update_bit(edf, VACT_EDIT_INCREASE);
				ret  = PROC_LOOP;
			}
			else if(unact == VACT_EDIT_DECREASE)
			{
				update_bit(edf, VACT_EDIT_DECREASE);
				ret  = PROC_LOOP;
			}
			else if(unact == VACT_SWITH_CURSOR)
			{
				if(edf->bCursorMode == CURSOR_NORMAL)
				{
					edf->bCursorMode = CURSOR_NO;
					update_register(edf,0);
				}
				else					
					edf->bCursorMode = CURSOR_NORMAL;

				OSD_TrackObject(pObj,C_UPDATE_ALL);
			}

			
			#if (NIMREG_MODE == NIMREG_FOR_INTER_TEST)
			else if (unact = VACT_SWITH_SELECT)
			{
				RegPrintProc(&RegPrint);
			}
			#endif
			
			break;
		case EVN_FOCUS_PRE_LOSE:
			OSD_SetEditFieldCursorMode(edf,CURSOR_NO);
			break;
		default:
			break;
	}
	return ret;
}

static UINT32 ComUniStr2Hex(const UINT16* string)
{
	UINT8 i,len,c;
	UINT32 val;

	if(string == NULL)
		return 0;

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
		else if (c>='A' && c<='F')
			val = val*16 + c - 55;
		else if (c>='a' && c<='f')
			val = val*16 + c - 87;
	}
	
	return val;	
}


static void update_bit(EDIT_FIELD *edf, UINT32 param)
{
	UINT16 *pstring;
	UINT32 i;
	UINT8 tmp[2];
	UINT8 cursor_pos = edf->bCursor;
	if (0 == cursor_pos)//update the high bit , ex. 0x5a, here update bit 5
	{
		pstring = (UINT16*)OSD_GetEditFieldContent(edf);
		i = ComUniStr2Hex(pstring);
		if (VACT_EDIT_INCREASE == param)
		{
			
			if (i/16 == 0x0f)
				i = i%16;
			else
				i+=16;
		}
		else 
		{
			if (i/16 == 0x00)
				i = i%16+0xf0;
			else
				i-=16;
		}
		sprintf(tmp, "%02x", (i));	
		wincom_AscToMBUni(tmp, edit_str[m_nPos]);
		OSD_TrackObject((POBJECT_HEAD)edf,C_UPDATE_ALL);
	}
	else if (1 == cursor_pos) //update the low bit , ex. 0x7d, here update bit d
	{
		pstring = (UINT16*)OSD_GetEditFieldContent(edf);
		i = ComUniStr2Hex(pstring);
		if (VACT_EDIT_INCREASE == param)
		{
			if (i%16 == 0x0f)
				i=i-0x0f;
			else
				i +=1;
		}
		else 
		{
			if (i%16 == 0x00)
				i+=0x0f;
			else
				i-=1;
		}
		sprintf(tmp, "%02x", (i));	
		wincom_AscToMBUni(tmp, edit_str[m_nPos]);
		OSD_TrackObject((POBJECT_HEAD)edf,C_UPDATE_ALL);
	}
	else  //should not come here
	{
		ASSERT(0);
	}
}


static void update_register(EDIT_FIELD *edf, UINT32 param)
{
	UINT32 edit_value,i;
	UINT16 *pstring;
	UINT32 num;

	pstring = (UINT16*)OSD_GetEditFieldContent(edf);
	num = ComUniStr2Hex(pstring);
	SetByte((m_pagenum*16 + m_nPos), num);

}

static void display_static_vision(void)
{
	UINT8 i;
	for (i=0; i<16; i++)
	wincom_AscToMBUni("CR_  ", text_str[i]);
	wincom_named_funcs();
	for (i = 0; i<24;i ++)
	OSD_DrawObject((POBJECT_HEAD)pCons[i],C_UPDATE_ALL);
}


void display_dynamic_vision(void)
{
	UINT8 bID;
	UINT32 i,k;
	UINT8 tmp[2];
	bID = OSD_GetContainerFocus(&g_win_nimreg);
	k = m_pagenum*16;	
	for (i=0;i<16;i++)
	{
		sprintf(tmp, "%02x", (k+i));	
		wincom_AscToMBUni(tmp, &text_str[i][3]);

		sprintf(tmp, "%02x", GetByte(k+i));
		wincom_AscToMBUni(tmp, edit_str[i]);

		if(bID != (i+1))
			OSD_DrawObject((POBJECT_HEAD)pCons[i],C_UPDATE_ALL);
		else
			OSD_TrackObject((POBJECT_HEAD)pCons[i],C_UPDATE_ALL);
	}
}


static VACTION nimreg_sel_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION Action = VACT_PASS;
	switch (key)
	{
		case V_KEY_ENTER:
			Action = VACT_INCREASE;
			break;
		default:
			Action = VACT_PASS;
			break;
	}
	return Action;
}

/*******************************************************************************
*	To add or modifiy the functions displayed in the REGMENU, you need to modified 
* 	following two functions.
*******************************************************************************/

#if (NIMREG_MODE == NIMREG_FOR_OUTER_USE)
static PRESULT nimreg_sel_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	return PROC_PASS;
}
static void wincom_named_funcs(void)
{	
	wincom_AscToMBUni("Func0", text_str[16]);
	wincom_AscToMBUni("Func1", text_str[17]);
	wincom_AscToMBUni("Func2", text_str[18]);
	wincom_AscToMBUni("Func3", text_str[19]);
	wincom_AscToMBUni("Func4", text_str[20]);
	wincom_AscToMBUni("Func5", text_str[21]);
	wincom_AscToMBUni("Func6", text_str[22]);
	wincom_AscToMBUni("Func7", text_str[23]);
}

#else

static void delay_timer_proc();
static void delay_timer_func(PMULTISEL pObj);
static void nimreg_ber_open ( POBJECT_HEAD w);
void 	  nimreg_ber_refresh (UINT32 err_count,UINT32 RsUbc); //
static void prcNimRegFunc0(BOOL result);
static void prcNimRegFunc1(BOOL result);
static void prcNimRegFunc2(BOOL result);
static void prcNimRegFunc3(BOOL result);
static void prcNimRegFunc4(BOOL result);
static void prcNimRegFunc5(BOOL result);
static void prcNimRegFunc6(BOOL result);
static void prcNimRegFunc7(BOOL result);

BOOL  nimreg_flag[8];  
static UINT32 nimreg_mutex_id = OSAL_INVALID_ID;

/***********************************
indx: Indicate the reg NO.
val:  The value of the write data
dir:   Indicate it is read or write operation. (TRUE for write; FALSE for read)
return - The read result.
************************************/
BOOL nimRegFuncFlag(UINT8 indx, BOOL val, BOOL dir)
{
	BOOL bRet;
	bRet = FALSE;
	//if(nimreg_mutex_id ==OSAL_INVALID_ID)	
	//{
	//	nimreg_mutex_id=osal_mutex_create();
	//	if(nimreg_mutex_id==OSAL_INVALID_ID)	
	//	{
	//		return -1;
	//	}
	//}
	//osal_mutex_lock(nimreg_mutex_id, OSAL_WAIT_FOREVER_TIME);
	if (dir == TRUE)  // Write
	{
		if (indx > 7 | indx < 0)
			bRet = FALSE;
		else
		{
			nimreg_flag[indx]= val;
			bRet = TRUE;
		}
	}
	else 			// read
		bRet =  nimreg_flag[indx];
	//osal_mutex_unlock(nimreg_mutex_id);
	return bRet;
	
}


static void RegPrintProc( PtypeRegPrint  pRegPrint)
{
	UINT8 i,j;
	
	PtypeRegPrint Rp = pRegPrint; 

	for (i = 0,Rp->p_number!=0; i< Rp->p_number;i++)
		{
		if ((Rp->p_param[i].p_page==m_pagenum) && (Rp->p_param[i].p_pos == m_nPos))
			{
				for (j=i;j<Rp->p_number-1 ; j++)
					{
					Rp->p_param[j].p_page = Rp->p_param[j+1].p_page;
					Rp->p_param[j].p_pos = Rp->p_param[j+1].p_pos;
					}
				Rp->p_number -= 1;
				return;
			}
		}
	Rp->p_param[Rp->p_number].p_page = m_pagenum;
	Rp->p_param[Rp->p_number].p_pos = m_nPos;
	Rp->p_number += 1;

}

void nimRegPrint()
{
	UINT32 prt_addr;
	UINT8 data,i;
	for (i = 0,RegPrint.p_number!=0; i< RegPrint.p_number;i++)
	{
		//if (i ==0) 
		//	libc_printf("------------\n");
		prt_addr = RegPrint.p_param[i].p_page * 16 + RegPrint.p_param[i].p_pos;
		data = GetByte(prt_addr);
		libc_printf("CR_%02x = 0x%02x \n",prt_addr,data);
	}
}

static PRESULT nimreg_message_proc ( UINT32 msg_type, UINT32 msg_code )
{
	PRESULT ret = PROC_LOOP;

	switch ( msg_type )
	{
		case CTRL_MSG_SUBTYPE_STATUS_SIGNAL:
			win_signal_refresh();
			break;
	}

	return ret;
}


void delay_timer_func(PMULTISEL pObj)
{
	pNimReg_Time_Msel = pObj;
	nimreg_timer = api_start_timer( "nimreg",100,delay_timer_proc);
}

void delay_timer_proc()
{
	api_stop_timer ( &nimreg_timer );
	ap_send_msg ( CTRL_MSG_SUBTYPE_CMD_UPDATE_VIEW, 1, TRUE );
	//OSD_SetMultiselSel(pNimReg_Time_Msel,FALSE);
	//OSD_TrackObject((POBJECT_HEAD)pNimReg_Time_Msel, C_UPDATE_ALL);
}

static void nimreg_ber_open ( POBJECT_HEAD w)
{
	POBJECT_HEAD pObj;
	UINT8 level, quality, lock ;
	UINT8 data[4];

	if ( w->bType != OT_CONTAINER )
		return ;

	pObj = ( POBJECT_HEAD ) & g_con_ber;
	pObj->pRoot = w;
	
	wincom_AscToMBUni("BER:", ber_str[0]);
	wincom_AscToMBUni("*10^7;  PER:", ber_str[2]);
	OSD_DrawObject ((POBJECT_HEAD)(&g_con_ber), C_UPDATE_ALL );
	
}

// This function is the small bar below the NIM_REG main window
// When is opened ,it should be refresh every time, so it should
// be placed in a function which execute always, so I call it in 
// function of nim_s3202_sw_test_thread() in file nim_s3202.c
// when the switch of ber is opened
void nimreg_ber_refresh (UINT32 err_count,UINT32 RsUbc)
{
	UINT8 level, quality, lock ;
	UINT8 temp[15];
	
	MEMSET(temp, 0, sizeof(temp));
	sprintf(temp, "%d", err_count);
	OSD_SetTextFieldContent(&g_txt_ber1, STRING_ANSI, (UINT32)temp);
	
	MEMSET(temp, 0, sizeof(temp));
	sprintf(temp, "%d", RsUbc);
	OSD_SetTextFieldContent(&g_txt_ber3, STRING_ANSI, (UINT32)temp);

	OSD_DrawObject ((POBJECT_HEAD)(&g_txt_ber1), C_UPDATE_ALL );
	OSD_DrawObject ((POBJECT_HEAD)(&g_txt_ber3), C_UPDATE_ALL );		
}

// Close the ber bar
// it called by two function.
//  1. nimreg_sel_callback() . which is the main place that it is called.
//  2. nimreg_callback().       which is happend when all the NIM_REG window is closed
static void nimreg_ber_close () // close the ber bar
{
	if ((&g_con_ber) != NULL)
		OSD_ClearObject((POBJECT_HEAD)&g_con_ber, 0);
}


void nimreg_close () // close the ber bar
{
	if ((&g_win_nimreg) != NULL)
		OSD_ClearObject((POBJECT_HEAD)&g_win_nimreg, 0);
}


// Close the signal bar
// it called by two function.
//  1. nimreg_sel_callback() . which is the main place that it is called.
//  2. nimreg_callback().       which is happend when all the NIM_REG window is closed
static void nimreg_signal_close () // close the signal bar 
{
	if ((&g_win_signal) != NULL)
		OSD_ClearObject((POBJECT_HEAD)&g_win_signal, 0);
}



static PRESULT nimreg_sel_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;

	MULTISEL *  mSel = ( MULTISEL* )pObj;
	ID bID = OSD_GetContainerFocus(&g_win_nimreg);
	UINT8 func_number = bID - 17;//From 1 to 8.
	BOOL result = FALSE + OSD_GetMultiselSel(mSel);


	switch ( event )
	{
		case EVN_POST_CHANGE:
		{
			switch (func_number)
				{
				case 0:{	prcNimRegFunc0(result);break;}
				case 1:{ prcNimRegFunc1(result);break;}
				case 2:{ prcNimRegFunc2(result);break;}
				case 3:{ prcNimRegFunc3(result);break;}
				case 4:{ prcNimRegFunc4(result);break;}
				case 5:{ prcNimRegFunc5(result);break;}
				case 6:{ prcNimRegFunc6(result);break;}
				case 7:{ prcNimRegFunc7(result);break;}
				default:	break;
			}
		}
		default:	break;
	}
	return ret;
}


static void wincom_named_funcs(void)
{	
	wincom_AscToMBUni("0x8040", text_str[16]);
	wincom_AscToMBUni("BerPer", text_str[17]);
	wincom_AscToMBUni("Refresh",text_str[18]);
	wincom_AscToMBUni("CatchAD", text_str[19]);
	wincom_AscToMBUni("Func4", text_str[20]);
	wincom_AscToMBUni("Func5", text_str[21]);
	wincom_AscToMBUni("ClrPrint", text_str[22]);
	wincom_AscToMBUni("FastPrt", text_str[23]);

}

static void prcNimRegFunc0(BOOL result)
{
	nimRegFuncFlag(0, TRUE, TRUE);
	delay_timer_func(&nimreg_sel1);
}

static void prcNimRegFunc1(BOOL result)
{

	if(result == TRUE)
	{	
		nimRegFuncFlag(1, TRUE, TRUE);
		nimreg_ber_open((POBJECT_HEAD)(&g_win_nimreg));
	}
	else if (result == FALSE)
	{
		nimRegFuncFlag(1, FALSE, TRUE);
		nimreg_ber_close ();
	}

}

static void prcNimRegFunc2(BOOL result)
{
	nimRegFuncFlag(2, result, TRUE);
}

static void prcNimRegFunc3(BOOL result)
{
	nimRegFuncFlag(3, TRUE, TRUE);
	delay_timer_func(&nimreg_sel4);
}

static void prcNimRegFunc4(BOOL result)// agst2 20071101
{
	nimRegFuncFlag(4, TRUE, TRUE);
	delay_timer_func(&nimreg_sel5);
}

static void prcNimRegFunc5(BOOL result) // ifbw&cp2 20071101
{
	nimRegFuncFlag(5, TRUE, TRUE);
	delay_timer_func(&nimreg_sel6);
}

static void prcNimRegFunc6(BOOL result)
{
	nimRegFuncFlag(6, TRUE, TRUE);
	RegPrint.p_number = 0;
	delay_timer_func(&nimreg_sel7);
}

static void prcNimRegFunc7(BOOL result)
{
#if 0
	if(result == TRUE)
	{	
		nimRegFuncFlag(7, TRUE, TRUE);
		win_signal_open((POBJECT_HEAD)(&g_win_nimreg));
	}
	else if (result == FALSE)
	{
		nimRegFuncFlag(7, FALSE, TRUE);
		nimreg_signal_close();
	}
#endif	
	nimRegFuncFlag(7, result, TRUE);

}

#endif

#endif
