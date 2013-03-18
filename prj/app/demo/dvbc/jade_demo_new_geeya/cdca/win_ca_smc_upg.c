#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
//#include <api/libosd/osd_lib.h>

#if (CAS_TYPE == CAS_CDCA)
#include "win_cas_com.h"
#include "tf3_cas_api.h"

//for progress window
extern CONTAINER smcupg_info_con;
extern TEXT_FIELD smcupg_title;
extern TEXT_FIELD smcupg_txt_msg;
extern PROGRESS_BAR smcupg_bar_progress;
extern TEXT_FIELD smcupg_txt_progress;

#define CHANNEL_BAR_HANDLE		((POBJECT_HEAD)&g_win_progname)

#define SMC_UPG_CONX	WSTL_POP_WIN_1//WSTL_TXT_3//WSTL_POP_WIN_01
#define TITLE_SH_IDX 	WSTL_TXT_POPWIN

#define SMCUPG_INFO_SH_IDX	WSTL_TXT_POPWIN

#define PROGRESS_TXT_SH_IDX			WSTL_TXT_POPWIN
#define PROGRESSBAR_SH_IDX			WSTL_BARBG_01
#define PROGRESSBAR_MID_SH_IDX		WSTL_NOSHOW_IDX
#define PROGRESSBAR_PROGRESS_SH_IDX	WSTL_BAR_02

#define TXTN_SH_IDX   WSTL_TXT_POPWIN//WSTL_TXT_3
#define TXTN_HL_IDX   WSTL_NOSHOW_IDX
#define TXTN_SL_IDX   WSTL_NOSHOW_IDX
#define TXTN_GRY_IDX  WSTL_NOSHOW_IDX

#define W_L		100
#define W_T		100
#define W_W		410
#define W_H		150

#define TITLE_L (W_L + 10)
#define TITLE_T	 (W_T + 10)
#define TITLE_W (W_W - 20)
#define TITLE_H 30

#define BAR_L	(W_L + 20)
#define BAR_T	(W_T + 110)
#define BAR_W	(W_W - 150)
#define BAR_H	12

#define BAR_TXT_L (BAR_L + BAR_W + 10)
#define BAR_TXT_W	60
#define BAR_TXT_T ( BAR_T - 2)
#define BAR_TXT_H	24 

#define TXTN_L  	(W_L+10)
#define TXTN_T		(W_T+50)
#define TXTN_W  	360
#define TXTN_H		28


#define LDEF_CON(root, varCon,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,conobj)		\
    DEF_CONTAINER(varCon,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, SMC_UPG_CONX,SMC_UPG_CONX,SMC_UPG_CONX,SMC_UPG_CONX,   \
    NULL,NULL,  \
    conobj, ID,1)
    
#define LDEF_TXT(root,varTxt,nxtObj,l,t,w,h,resID)		\
    DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXTN_SH_IDX,TXTN_HL_IDX,TXTN_SL_IDX,TXTN_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,resID,NULL)

#define LDEF_PROGRESS_BAR(root,varBar,nxtObj,l,t,w,h,style,rl,rt,rw,rh)	\
	DEF_PROGRESSBAR(varBar, &root, nxtObj, C_ATTR_ACTIVE, 0, \
		0, 0, 0, 0, 0, l, t, w, h, PROGRESSBAR_SH_IDX, PROGRESSBAR_SH_IDX, PROGRESSBAR_SH_IDX, PROGRESSBAR_SH_IDX, \
		NULL, NULL, style, 0, 0, PROGRESSBAR_MID_SH_IDX, PROGRESSBAR_PROGRESS_SH_IDX, \
		rl,rt , rw, rh, 1, 100, 100, 0)
   
#define LDEF_PROGRESS_TXT(root,varTxt,nxtObj,l,t,w,h)		\
    DEF_TEXTFIELD(varTxt,&root,nxtObj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXTN_SH_IDX,TXTN_HL_IDX,TXTN_SL_IDX,TXTN_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_TOP, 5,0,0,display_strs[1])

#define LDEF_TITLE(root,varTxt,nxtObj,l,t,w,h,resID)		\
    DEF_TEXTFIELD(varTxt,&root,nxtObj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TITLE_SH_IDX,TITLE_SH_IDX,TITLE_SH_IDX,TITLE_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,resID,NULL)

#define LDEF_INFOCON(varCon,nxtObj,l,t,w,h)		\
    DEF_CONTAINER(varCon,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, SMCUPG_INFO_SH_IDX,SMCUPG_INFO_SH_IDX,SMCUPG_INFO_SH_IDX,SMCUPG_INFO_SH_IDX,   \
    NULL,NULL,  \
    &nxtObj, 0,0)


//LDEF_INFOCON(smcupg_info_con, smcupg_bar_progress,W_L-9,W_T, W_W, W_H)
LDEF_CON(NULL, smcupg_info_con,NULL,1,1,1,1,1,W_L,W_T, W_W, W_H,&smcupg_title)

LDEF_TITLE(smcupg_info_con, smcupg_title, &smcupg_txt_msg, \
		TITLE_L, TITLE_T, TITLE_W, TITLE_H, RS_TF3_SMC_UPGRADE)//

LDEF_PROGRESS_BAR(smcupg_info_con,smcupg_bar_progress,&smcupg_txt_progress,	\
	BAR_L, BAR_T, BAR_W, BAR_H, PROGRESSBAR_HORI_NORMAL | PBAR_STYLE_RECT_STYLE,0,3,BAR_W,BAR_H-6)

LDEF_PROGRESS_TXT(smcupg_info_con, smcupg_txt_progress, NULL, \
		BAR_TXT_L, BAR_TXT_T, BAR_TXT_W, BAR_TXT_H)

LDEF_TXT(&smcupg_info_con,smcupg_txt_msg,&smcupg_bar_progress,TXTN_L,TXTN_T,TXTN_W,TXTN_H,RS_TF3_SHOWMSG_UPDATE)


/*cjj 010509 add  >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
extern UINT16 g_smc_no[17];

void ap_tf_cas_process_update(UINT8 process, UINT8 mark)
{
	date_time dt;
	SYSTEM_DATA* pSysData = NULL;
	static tem_process = 0; //
	static BOOL g_smc_read = FALSE;

	//libc_printf("smc upd process:%d, %d\n", process, mark);
	ap_tf_cas_clear_screen(CLEAR_CA_MSG);

	if(CDCA_SCALE_RECEIVEPATCH == mark)
	{
		smcupg_txt_msg.wStringID = RS_DISPLAY_DOWNLOADING;
	}
	else if(CDCA_SCALE_PATCHING == mark)
	{
		smcupg_txt_msg.wStringID = RS_TF3_SHOWMSG_UPDATE;
	}

	pSysData = sys_data_get();

	if (CDCA_SCALE_PATCHING == mark && (process == 0 ||(FALSE == g_smc_read) ))
	{
			if(FALSE == g_smc_read)
			{
				if(g_smc_no[0] != 0)
				{
					MEMCPY(pSysData->upd_smc_no, g_smc_no, sizeof(g_smc_no));
					g_smc_read = TRUE;
				}
			}
	}
		
	if(process > 100)
	{
		if (CDCA_SCALE_PATCHING == mark)
		{
			get_local_time(&dt);			
			pSysData->smc_update_dt = dt;

			if (tem_process == 100)
			{
				pSysData->bUpd_status = TRUE;
			}
			else
			{
				pSysData->bUpd_status = FALSE;
			}
		}
		
		sys_data_save(0);
#if 1
		OSD_ClearObject((POBJECT_HEAD)&smcupg_info_con, C_UPDATE_ALL);
        
		if((POBJECT_HEAD)&smcupg_info_con == menu_stack_get_top())
		{
			menu_stack_pop();
		}
        BackToFullScrPlay();
#else
		OSD_ClearObject( (POBJECT_HEAD)&smcupg_txt_msg, C_UPDATE_ALL);
		OSD_ClearObject( (POBJECT_HEAD)&smcupg_bar_progress, C_UPDATE_ALL);
		OSD_ClearObject( (POBJECT_HEAD)&smcupg_txt_progress, C_UPDATE_ALL);
#endif
	}
	else
	{
		if((POBJECT_HEAD)CHANNEL_BAR_HANDLE == menu_stack_get_top())
		{
			OSD_ObjClose((POBJECT_HEAD)CHANNEL_BAR_HANDLE, C_DRAW_SIGN_EVN_FLG|C_CLOSE_CLRBACK_FLG);
			menu_stack_pop();
		}
		
		if(-1 == window_pos_in_stack((POBJECT_HEAD)&smcupg_info_con))
		{
			menu_stack_push((POBJECT_HEAD)&smcupg_info_con);
		}
		
		OSD_SetProgressBarPos(&smcupg_bar_progress,(INT16)process);
		OSD_SetTextFieldContent(&smcupg_txt_progress, STRING_NUM_PERCENT, (UINT32)process);
#if 1
		OSD_DrawObject((POBJECT_HEAD)&smcupg_info_con, C_UPDATE_ALL);
#else
		OSD_DrawObject( (POBJECT_HEAD)&smcupg_txt_msg, C_UPDATE_ALL);
		OSD_DrawObject( (POBJECT_HEAD)&smcupg_bar_progress, C_UPDATE_ALL);
		OSD_ClearObject( (POBJECT_HEAD)&smcupg_txt_progress, C_UPDATE_ALL);
		OSD_DrawObject( (POBJECT_HEAD)&smcupg_txt_progress, C_UPDATE_ALL);
#endif
	}

    tem_process = process;
}
/*cjj 010509 add  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/


#endif

