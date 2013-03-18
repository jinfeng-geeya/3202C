#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
//#include <api/libosd/osd_lib.h>

#include "win_cas_com.h"

/*******************************************************************************
*	Objects definition
*******************************************************************************/
static UINT16 x_pos[] = {430,  8,   500,   100,   80,   450,   8,   550,   390,   260};

static UINT16 y_pos[] = {70,   180,   320,   540,   90,   230,   350,   150,   540,   30};

#define X_POS_NUM   (sizeof(x_pos)/sizeof(x_pos[0]))
#define y_pos_NUM   (sizeof(y_pos)/sizeof(y_pos[0]))

static UINT8 pos_index = 0;
extern TEXT_FIELD txt_fingerprint;
extern TEXT_FIELD txt_urgent;

#define FP_L		295
#define FP_T		100
#define FP_W	130
#define FP_H		24

#define UB_L		30
#define UB_T		40
#define UB_W	150
#define UB_H		28

UINT16 str_fingerprint[15];
UINT16 str_urgent[15];

#define WSTL_FINGER		WSTL_PREVIEW_TXT

#define LDEF_TXTIDX(root,varTxt,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,resID,str)		\
    DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, WSTL_FINGER,WSTL_PREVIEW_TXT,WSTL_PREVIEW_TXT,WSTL_PREVIEW_TXT,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,resID,str)

LDEF_TXTIDX(NULL,txt_fingerprint,NULL,0,0,0,0,0,\
    FP_L, FP_T,FP_W,FP_H,0,str_fingerprint)
LDEF_TXTIDX(NULL,txt_urgent,NULL,0,0,0,0,0,\
    UB_L, UB_T,UB_W,UB_H,0,str_urgent)
    
static void win_finger_print_draw_info(char* str, UINT16* unistr, UINT8 index)
{
	TEXT_FIELD* txt;

	txt = &txt_fingerprint;

    if((pos_index != index)&&((str != NULL)||(unistr != NULL)))
    {
        OSD_ClearObject( (POBJECT_HEAD)txt, C_UPDATE_ALL);

        pos_index = index;
        
        txt->head.frame.uLeft = x_pos[pos_index];
        txt->head.frame.uTop  = y_pos[pos_index];
    }
    
    
	if(str != NULL)
		OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)str);
	else if (unistr != NULL)
		OSD_SetTextFieldContent(txt, STRING_UNICODE, (UINT32)unistr);
	//OSD_SetColor(txt,shidx,shidx,shidx,shidx);
	if((str != NULL)||(unistr != NULL))	
	{		
		OSD_DrawObject( (POBJECT_HEAD)txt, C_UPDATE_ALL);
	}
	else	    
        	OSD_ClearObject( (POBJECT_HEAD)txt, C_UPDATE_ALL);
}

//flag: 0 clear, 1 show finger print
void show_finger_print(UINT8 flag,UINT32 dwCardID)
{
	UINT8 i;
	UINT8 index = pos_index;
	static UINT32 show_tick = 0;
    
	UINT8 temp[15];

	if (flag)
	{
        if(osal_get_tick()- show_tick > 5000)
        { 
            show_tick = osal_get_tick();
            index = (pos_index + 1)%X_POS_NUM;
        }
		MEMSET(temp, 0, sizeof(temp));
		MEMCPY(temp, "ID: ",4);
		sprintf(&temp[4], "%d", dwCardID);
		win_finger_print_draw_info(temp, NULL, index);
	}
	else
	{
		win_finger_print_draw_info(NULL, NULL, index);
        show_tick = 0;
	}
		
	
}
void urgent_disp(UINT8 flag, UINT16 strid)
{
	TEXT_FIELD* txt = &txt_urgent;

	if (flag)
	{
		txt->pString = NULL;
		OSD_SetTextFieldContent(txt, STRING_ID, (UINT32)strid);
	}
	else
	{
		if (NULL == txt->pString)
			txt->pString = str_urgent;
		OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)"");
	}
		
	OSD_DrawObject( (POBJECT_HEAD)txt, C_UPDATE_ALL);
}


