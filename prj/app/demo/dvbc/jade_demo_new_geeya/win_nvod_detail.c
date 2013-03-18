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
#include <api/libsi/lib_nvod.h>

#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"
#include "win_com.h"
#include "menus_root.h"

#include "win_com_popup.h"
#include "win_com_list.h"

#include "copper_common/com_epg.h"
#include "control.h"

/**************************************************************************/
CONTAINER g_win_detail;

MULTI_TEXT nvod_mtext_detail;
TEXT_CONTENT nvod_mtext_content;

TEXT_FIELD nvod_detail_title;
TEXT_FIELD nvod_detail_ref_name;


static VACTION nvod_detail_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT nvod_detail_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION nvod_detail_win_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT nvod_detail_win_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);


void win_nvod_detail_draw_infor();
void win_nvod_detail_draw_refname();
void win_nvod_detail_set_refname();
void win_nvod_detail_set_infor();
/*************************************************************************/
#define WIN_L	(TV_OFFSET_L +100)
#define	WIN_T	(TV_OFFSET_T +100)
#define	WIN_W	(480)
#define	WIN_H	(TV_OFFSET_T +350)

#define WIN_MTEXT_L	(WIN_L +20)
#define	WIN_MTEXT_T	(WIN_T +65)
#define	WIN_MTEXT_W	(WIN_W -40)
#define	WIN_MTEXT_H	(270)

#define WIN_TITLE_L	(WIN_L +20)
#define	WIN_TITLE_T	(WIN_T)
#define	WIN_TITLE_W	(160)
#define	WIN_TITLE_H	(35)

#define WIN_TITLE_NAME_L	(WIN_L +(WIN_W-WIN_TITLE_NAME_W)/2)
#define	WIN_TITLE_NAME_T	(WIN_T+35)
#define	WIN_TITLE_NAME_W	(200)
#define	WIN_TITLE_NAME_H	(30)


/*************************************************************************/
TEXT_CONTENT nvod_mtext_content =
{
    STRING_UNICODE, 0
};

#define NVOD_DTL_LDEF_WIN(varCon,nxtObj,l,t,w,h,focusID)		\
    DEF_CONTAINER(varCon,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WSTL_POP_WIN_1,WSTL_POP_WIN_1,WSTL_POP_WIN_1,WSTL_POP_WIN_1,   \
    nvod_detail_win_keymap,nvod_detail_win_callback,  \
    nxtObj, focusID,0)

#define LDEF_MTXT(root,varMtxt,nextObj,ID,l,t,w,h,sh,cnt,sb,content)	\
	DEF_MULTITEXT(varMtxt,&root,nextObj,C_ATTR_ACTIVE,0, \
    	ID,ID,ID,ID,ID, l,t,w,h, sh,sh,sh,sh,   \
	    nvod_detail_keymap,nvod_detail_callback,  \
    	C_ALIGN_LEFT | C_ALIGN_TOP, cnt,  4,4,w-8,h-8,sb,content)

#define LDEF_TXT(root,varTxt,nxtObj,l,t,w,h,shidx,strid,pstring)		\
	DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
	    0,0,0,0,0, l,t,w,h, shidx,shidx,shidx,shidx,   \
	    NULL,NULL,  \
	    C_ALIGN_CENTER| C_ALIGN_VCENTER, 0,0,strid,pstring)

extern UINT16 refEventBuf[256];
UINT32 nr_count	= 0;
/*************************************************************************/

NVOD_DTL_LDEF_WIN(g_win_detail,&nvod_detail_title,WIN_L,WIN_T,WIN_W,WIN_H,1)

LDEF_TXT(&g_win_detail,nvod_detail_title,&nvod_detail_ref_name,WIN_TITLE_L,WIN_TITLE_T,WIN_TITLE_W,WIN_TITLE_H,WSTL_TXT_16,RS_PROG_BRIEF,NULL)

LDEF_TXT(&g_win_detail,nvod_detail_ref_name,&nvod_mtext_detail,WIN_TITLE_NAME_L,WIN_TITLE_NAME_T,WIN_TITLE_NAME_W,WIN_TITLE_NAME_H,WSTL_TXT_16,0,display_strs[30])

LDEF_MTXT(g_win_detail,nvod_mtext_detail,NULL,1,WIN_MTEXT_L,WIN_MTEXT_T,WIN_MTEXT_W,WIN_MTEXT_H,WSTL_TXT_16,1,NULL,&nvod_mtext_content)

/**********************************************************************************/


static VACTION nvod_detail_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

	return act;
}

static PRESULT nvod_detail_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{

	PRESULT ret = PROC_PASS;

	return ret;
}


static VACTION nvod_detail_win_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

	switch(key)
	{
		case V_KEY_EXIT:
		case V_KEY_MENU:
			act = VACT_CLOSE;
			break;
		default:
			break;
	}
	return act;
}


static PRESULT nvod_detail_win_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;

	switch ( event )
	{
		case EVN_PRE_OPEN:
			/*
			if(IRCA_BAN_GetWinpopShow() == TRUE)
			{
				win_compopup_close();
				IRCA_BAN_SetWinpopShow(FALSE);
			}
			Set_Show_BeforEnter(TRUE);//added by wm 090112
			*/
			
			win_nvod_detail_set_refname();
			win_nvod_detail_set_infor();
			break;
		case EVN_POST_OPEN:
						
			break;
		case EVN_PRE_CLOSE:
			break;
		case EVN_POST_CLOSE:
			/*
			if(IRCA_BAN_GetBannerShow())
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
		default:
			break;
	}
	return ret;
}

void win_nvod_detail_set_infor()
{
	struct nvod_event curEvent;
  	struct nvod_ref_event ref;

	
	GetCurEvent(&curEvent);
	if(!GetNvodFromTimerFlag())
	{
		/* the refEventBuf always holds the right data for each reference event*/
		nvod_mtext_content.text.pString = refEventBuf;
	}
	else
	{
		if(SUCCESS==nvod_get_ref_event_byid(curEvent.ref_event_id,curEvent.ref_service_id,&ref))
		{
			MEMSET((UINT8*)refEventBuf,0,sizeof(refEventBuf));
			//convert_gb2312_to_unicode(ref.txt,ref.txt_len,refEventBuf,255);
			dvb_to_unicode(ref.txt,ref.txt_len,refEventBuf,255,2312);
			nvod_mtext_content.text.pString = refEventBuf;
		}
		else
			nvod_mtext_content.text.pString = NULL;
	}

	if(/*nr_count==0 &&*/ GetNvodPlayFlag())
	{
		//nr_count++;
		OSD_SetMultiTextLine(&nvod_mtext_detail,0);
		//OSD_DrawObject((POBJECT_HEAD)&nvod_mtext_detail,C_UPDATE_ALL);
	}

}

void win_nvod_detail_set_refname()
{
    struct nvod_event curEvent;
    struct nvod_ref_event ref;
    UINT8 str[40];
    UINT16 curRefEventPos = 0;
	TEXT_FIELD * ptxt = &nvod_detail_ref_name;
	
	/*this curEvent always hold the right content no matter normal 
	 *and timer full screen nvod broswer
	 */
    GetCurEvent(&curEvent);
	if(!GetNvodFromTimerFlag())
	{
	    curRefEventPos = NvodGetCurRefEventPos();
	    if(SUCCESS==nvod_get_ref_event(curRefEventPos,&ref))
			goto DARW;
		else
			goto OUT;
	}
	else
	{
		if(SUCCESS==nvod_get_ref_event_byid(curEvent.ref_event_id,curEvent.ref_service_id,&ref))
			goto DARW;
		else
			goto OUT;
	}
DARW:
 	MEMSET(str,0,sizeof(str));
    //convert_gb2312_to_unicode(ref.name,(INT32)ref.name_len,(UINT16 *)str,40);
    dvb_to_unicode(ref.name,(INT32)ref.name_len,(UINT16 *)str,40,2312);
	OSD_SetTextFieldContent(ptxt, STRING_UNICODE, (UINT32)str);
	//OSD_DrawObject((POBJECT_HEAD)ptxt, C_UPDATE_ALL);
OUT:
	return;

}


/*
void clear_last_refname()
{
    UINT8 str[40];
	TEXT_FIELD * ptxt = &nvod_detail_ref_name;
	
	if(ptxt->pString)
 	MEMSET(str,0,sizeof(str));
    convert_gb2312_to_unicode(ref.name,(INT32)ref.name_len,(UINT16 *)str,40);

	OSD_SetTextFieldContent(ptxt, STRING_UNICODE, (UINT32)str);
}
*/

#endif

