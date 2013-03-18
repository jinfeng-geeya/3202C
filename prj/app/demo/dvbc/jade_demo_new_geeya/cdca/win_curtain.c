#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
//#include <api/libosd/osd_lib.h>
#include <hld/decv/decv.h>

#include <api/libcas/cdca/cdcas30.h>

#include "win_cas_com.h"

/*******************************************************************************
*	Objects definition
*******************************************************************************/
extern TEXT_FIELD txt_curtain;

extern BITMAP curtain_bmp;

UINT16 str_curtain[15];

#define WSTL_CURTAIN		WSTL_TXT_2


#define LDEF_TXTIDX(root,varTxt,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,resID,str)		\
    DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, 100,100,400,28, WSTL_CURTAIN,WSTL_PREVIEW_TXT,WSTL_PREVIEW_TXT,WSTL_PREVIEW_TXT,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,resID,str)

DEF_BITMAP ( curtain_bmp, NULL, NULL, C_ATTR_ACTIVE, 0, \
             1, 0, 0, 0, 0, 150, 150, 248, 113, WSTL_TRANS_IX, WSTL_TRANS_IX, 0, 0,   \
             NULL, NULL,  \
             C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, /*IM_DEMO*/0)

LDEF_TXTIDX(NULL,txt_curtain,NULL,0,0,0,0,0,\
    FP_L, FP_T,FP_W,FP_H,0,str_curtain)

static void win_curtain_draw_info(char* str, UINT16* unistr)
{
	TEXT_FIELD* txt;

	txt = &txt_curtain;

    //OSD_ClearObject( (POBJECT_HEAD)txt, C_UPDATE_ALL);
    
    
	//if(str != NULL)
	//	OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)str);
	//else if (unistr != NULL)
	//	OSD_SetTextFieldContent(txt, STRING_UNICODE, (UINT32)unistr);

	if((str != NULL)||(unistr != NULL))	
	{		
		//OSD_DrawObject( (POBJECT_HEAD)txt, C_UPDATE_ALL);
        OSD_DrawObject ( ( OBJECT_HEAD* ) & curtain_bmp, C_UPDATE_ALL );
	}
	else	
	{
        //OSD_ClearObject( (POBJECT_HEAD)txt, C_UPDATE_ALL);
        OSD_ClearObject ( ( OBJECT_HEAD* ) & curtain_bmp, 0 );	
	}
}

#define CHANNEL_BAR_HANDLE		((POBJECT_HEAD)&g_win_progname)

//flag: 0 clear, 1 show curtain
void show_curtain_print(UINT16 flag,UINT32 dwCardID)
{
	UINT8 temp[15];
	

	if(!((NULL==menu_stack_get_top())
		||(CHANNEL_BAR_HANDLE == menu_stack_get_top())))
	{
		libc_printf("\n%s some window showed!\n",__FUNCTION__);
		return;
	}
	
	if (flag==CDCA_CURTAIN_CANCLE)
	{
		win_curtain_draw_info(NULL, NULL);
	}
	else
	{
		MEMSET(temp, 0, sizeof(temp));
		MEMCPY(temp, "CURTAIN Test!",13);
		//sprintf(&temp[8], "%d", 12345678);
		win_curtain_draw_info(temp, NULL);

        //black screen
        //if(flag!=CDCA_CURTAIN_OK)
        if(flag==CDCA_CURTAIN_OK)
    	{
			if(is_cur_decoder_avc())
				vdec_start((struct vdec_device *)dev_get_by_name("DECV_AVC_0")); 			  
			else
				vdec_start((struct vdec_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DECV));
    	}
		else
        {
			if(is_cur_decoder_avc())							
				vdec_stop((struct vdec_device *)dev_get_by_name("DECV_AVC_0"),TRUE,TRUE); 			  
			else
				vdec_stop((struct vdec_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DECV), TRUE, TRUE);
        }
	}
		
	
}

