/****
add by Jarod for DVTCA free previewing
****/

#include "win_cas_com.h"

extern  MULTI_TEXT cas_freeprewing_info;
static TEXT_CONTENT cas_reeprewing_txt;
static UINT8 CAS_POPUP_FREE_PREVIEWING_STATUS = 0;
#define LDEF_FREEPREVIEW_INFO(varMtxt,l,t,w,h,cnt,content)	\
	DEF_MULTITEXT(varMtxt,NULL,NULL,C_ATTR_ACTIVE,0, \
    	1,1,1,1,1, l,t,w,h, WSTL_FREEPREVIEWING_TXT,WSTL_FREEPREVIEWING_TXT,WSTL_FREEPREVIEWING_TXT,WSTL_FREEPREVIEWING_TXT, NULL,NULL,  \
    	C_ALIGN_CENTER | C_ALIGN_VCENTER, cnt,  4,4,0,0,NULL,content)
    	
LDEF_FREEPREVIEW_INFO(cas_freeprewing_info, 0,0,0,0, 1,&cas_reeprewing_txt)

void ap_cas_free_previewing_open(UINT16 strID)
{
	static UINT8 i=0;
	OSD_RECT free_prew_rc;
	free_prew_rc.uLeft = 350;
	free_prew_rc.uTop = 0;
	free_prew_rc.uWidth = 360;
	free_prew_rc.uHeight = 120;

	OSD_SetRect2(&cas_freeprewing_info.head.frame, &free_prew_rc);
	OSD_SetRect(&(cas_freeprewing_info.rcText), 4, 4, free_prew_rc.uWidth-8, free_prew_rc.uHeight-8);	

	cas_reeprewing_txt.bTextType =STRING_ID;
	cas_reeprewing_txt.text.wStringID= strID;
	OSD_DrawObject((POBJECT_HEAD)&cas_freeprewing_info, C_UPDATE_ALL);
	CAS_POPUP_FREE_PREVIEWING_STATUS = 1;
}
void ap_cas_free_previewing_close()
{
	if (CAS_POPUP_FREE_PREVIEWING_STATUS)
	{
		OSD_ClearObject((POBJECT_HEAD)&cas_freeprewing_info,C_UPDATE_ALL);
		CAS_POPUP_FREE_PREVIEWING_STATUS = 0;
	}
}

