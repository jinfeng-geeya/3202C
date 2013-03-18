#include <sys_config.h>
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
#include "control.h"
#include "win_password.h"

static UINT8 pwd_len = 4;
/*******************************************************************************
 *	Objects definition
 *******************************************************************************/

extern UINT16 pwd_char[5];
extern CONTAINER g_win_progname;
#include "win_password_obj.h"

void set_pwd_len(UINT8 len);
void PwdChangePos(OSD_RECT *rc);
static void PwdPosBack();

#define PWD_W_W		355//200
#define PWD_W_H		164//130
#define PWD_W_L		302//((OSD_MAX_WIDTH - PWD_W_W)/2)
#define PWD_W_T		178//((OSD_MAX_HEIGHT - PWD_W_H)/2-50)

#define PWD_TITLE_L	 ((PWD_W_W-PWD_TITLE_W)/2)
#define PWD_TITLE_T	 13
#define PWD_TITLE_W  342//150
#define PWD_TITLE_H	 34//26

#define PWD_BMP_L	 PWD_TITLE_L
#define PWD_BMP_T	 (PWD_TITLE_T+50)
#define PWD_BMP_W	 29//40
#define PWD_BMP_H	 33//40

#define PWD_EDT_L	 (PWD_BMP_L+50)
#define PWD_EDT_T	 PWD_BMP_T
#define PWD_EDT_W    239//70
#define PWD_EDT_H	 29//30

/*******************************************************************************
 *	Local variables define
 *******************************************************************************/
static UINT16 pwd_char[5];

static UINT32 default_key_exit[] =
{
	V_KEY_EXIT, V_KEY_MENU, V_KEY_UP, V_KEY_DOWN
};

#define VACT_PWD_BACK (VACT_PASS+1)
/*******************************************************************************
 *	key mapping and event callback definition
 *******************************************************************************/
 static PRESULT pwd_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	switch (event)
	{
		case EVN_PRE_CLOSE:
			*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
			break;
	       default:
		   	break;
	}
	return ret;
}
static VACTION pwd_edf_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act;
	UINT8 bID;

	bID = OSD_GetObjID(pObj);

	switch (key)
	{
		case V_KEY_0:	case V_KEY_1:	case V_KEY_2:	case V_KEY_3:
		case V_KEY_4:	case V_KEY_5:	case V_KEY_6:	case V_KEY_7:
		case V_KEY_8:	case V_KEY_9:
			act = key - V_KEY_0 + VACT_NUM_0;
			break;
		case V_KEY_ENTER:
			act = VACT_ENTER;
			break;
        case V_KEY_LEFT:
            act = VACT_PWD_BACK;
            break;
		default:
			act = VACT_PASS;
			break;
	}

	return act;

}


cb_pwd_check PwdCheck = NULL;

void SetPwdCheckCB(cb_pwd_check cb)
{
    
    PwdCheck = cb;
}

static void PwdPosBack()
{
    EDIT_FIELD *edt = &pwd_edt;

    if(edt->bCursor>=1)
    {
        edt->bCursor -= 1;
        OSD_DrawObject((POBJECT_HEAD)edt,C_UPDATE_ALL);
    }
}

static PRESULT pwd_edf_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	EDIT_FIELD *edt = (EDIT_FIELD*)pObj;
	UINT8 input_pwd_len;
	SYSTEM_DATA *sys_data;
	UINT16 *pwd_str;
	UINT32 i;
	VACTION unact;
	INT32 temp;
	char tempStr[9];
	UINT32 strLen = 0;
    UINT8 defaultPwd[4] = {2,0,4,6};
	BOOL rightPwd = FALSE;
	UINT8 strTemp[10];

	switch (event)
	{
		case EVN_POST_CHANGE:
			input_pwd_len = edt->bCursor;

			if (input_pwd_len + 1 >= pwd_len)
			//PWD_LENGTH			
			{
				sys_data = sys_data_get();
				pwd_str = (UINT16*)OSD_GetEditFieldContent(edt);

		                if(PwdCheck!=NULL)
		                {
		                    if(PwdCheck((UINT32)pwd_str,0))
		                    {
		                        ret = PROC_LEAVE;
		                    }
		                    break;
		                }

				strLen = ComUniStrLen(pwd_str);

				if (strLen < pwd_len)
					break;

				//=======================================
				ComUniStrToAsc((UINT8*)pwd_str, strTemp);
				for (i = 0; i < strLen; i++)
				{
					//strTemp[i] -= '0';
					if (strTemp[i] != sys_data->menu_password[i])
						break;
				}
				if (i == pwd_len)
					rightPwd = TRUE;
				//=======================================
				
				if (rightPwd ||  \
					MEMCMP(sys_data->menu_password, defaultPwd, sizeof(sys_data->menu_password)) == 0)
				{
					ret = PROC_LEAVE;
				}
			}
			break;
		case EVN_KEY_GOT:
			unact = (VACTION)(param1 >> 16);
			if (unact == VACT_ENTER)
			{
				sys_data = sys_data_get();
				pwd_str = (UINT16*)OSD_GetEditFieldContent(edt);

		               	 if(PwdCheck!=NULL)
		                {
		                    if(PwdCheck((UINT32)pwd_str,0))
		                    {
		                        ret = PROC_LEAVE;
		                    }
		                    else
		                    {
		                        edt->bCursor = 0;
		                        OSD_DrawObject(pObj,C_UPDATE_ALL);                        
		                    }
		                    break;
		                }                

				strLen = ComUniStrLen(pwd_str);
				if (strLen < pwd_len)
					break;

				ComUniStrToAsc((UINT8*)pwd_str, strTemp);
				for (i = 0; i < strLen; i++)
				{
					//strTemp[i] -= '0';
					if (strTemp[i] != sys_data->menu_password[i])
						break;
				}

				if (i == pwd_len)
					rightPwd = TRUE;

				if (rightPwd ||  \
					MEMCMP(sys_data->menu_password, defaultPwd, sizeof(sys_data->menu_password)) == 0)
				{
					ret = PROC_LEAVE;
				}

			}
            else if(unact == VACT_PWD_BACK)
                PwdPosBack();
			break;
		default:
			break;
	}

	return ret;
}

/*******************************************************************************
 *	External functions
 *******************************************************************************/

extern UINT8 *osd_vscr_buffer[];

BOOL win_pwd_open(UINT32 *vkey_exist_array, UINT32 vkey_exist_cnt, OSD_RECT *rc)
{
	POBJECT_HEAD win;
	PRESULT bResult = PROC_LOOP;
	UINT32 hkey = INVALID_HK, vkey;
	UINT32 *exit_pwd_keys;
	UINT32 exit_pwd_keycnt;
	UINT32 i;
	BOOL ret = TRUE;

	UINT32 vscr_size;
	OSD_RECT *frame;
	VSCR vscr;

	UINT8 *pbuf = NULL;
	UINT8 save_back_ok;
	//UINT8 *g_back_buf = NULL;
	UINT32 g_back_buf_size = 0;
	char pat_str[6];
 
#ifdef USE_LIB_GE   
	GUI_VSCR *lpVscr;
#else
    VSCR * lpVscr;
#endif

	POBJECT_HEAD menu;

	MEMSET(pwd_char, 0, sizeof(pwd_char));
	MEMSET(pat_str, 0, sizeof(pat_str));
	sprintf(pat_str, "p-*%d", pwd_len);

	pwd_edt.pcsPattern = pat_str;
	pwd_edt.bMaxLen = pwd_len;

	if (vkey_exist_array == NULL || vkey_exist_cnt == 0)
	{
		exit_pwd_keys = default_key_exit;
		exit_pwd_keycnt = sizeof(default_key_exit) / sizeof(default_key_exit[0]);
	}
	else
	{
		exit_pwd_keys = vkey_exist_array;
		exit_pwd_keycnt = vkey_exist_cnt;
	}

	win = (POBJECT_HEAD) &g_win_pwd_con;

	PwdChangePos(rc);

	/* Start to try to save backgrand */
	frame = &win->frame;
	lpVscr = OSD_GetTaskVscr(osal_task_get_current_id());
	OSD_UpdateVscr(lpVscr);
    lpVscr->lpbScr = NULL;

#ifdef USE_LIB_GE
    vscr_size = (frame->uWidth*frame->uHeight)<<1;//16bit osd
	pbuf = (UINT8*)MALLOC(vscr_size);    
    if (pbuf != NULL)
		save_back_ok = 1;
	else
	{
		save_back_ok = 0;
		return FALSE;
	}
    if (save_back_ok)
	{
		OSD_RectBackUp(lpVscr,pbuf,frame);
	}
#else
	UINT8 *g_back_buf = NULL;

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
#endif
	/* End of try to save backgrand */


	OSD_SetEditFieldCursor(&pwd_edt, 0);
	OSD_DrawObject(win, C_UPDATE_ALL);

	ret = TRUE;
	while (bResult != PROC_LEAVE)
	//while(1)
	{
		hkey = ap_get_key_msg();
		if (hkey == INVALID_HK || hkey == INVALID_MSG)
			continue;

		ap_hk_to_vk(0, hkey, &vkey);
		for (i = 0; i < exit_pwd_keycnt; i++)
		{
			if (vkey == exit_pwd_keys[i])
			{
				ret = FALSE;
				goto EXIT;
			}
		}

		bResult = OSD_ObjProc((POBJECT_HEAD)win, (MSG_TYPE_KEY << 16), hkey, 0);
	}

EXIT:
	if(save_back_ok)
	{
#ifdef USE_LIB_GE
            OSD_RectRestore(lpVscr, pbuf, frame);
            FREE(pbuf);            
            pbuf = NULL;
#else
		OSD_RegionWrite(&vscr, &vscr.vR);
		if (g_back_buf != NULL)
		{
			FREE(g_back_buf);
			g_back_buf = NULL;
		}
#endif
	}

	if ((!ret) && ((vkey != V_KEY_EXIT) && (vkey != V_KEY_MENU)))
	{
		ap_send_msg(CTRL_MSG_SUBTYPE_KEY, hkey, FALSE); /*if exit key got,repatch the messages again*/
	}

	menu = menu_stack_get_top();
	if(!ret && (menu == NULL || menu == (POBJECT_HEAD)&g_win_progname) && (vkey == V_KEY_MENU))
	{
		ap_send_msg(CTRL_MSG_SUBTYPE_KEY, hkey, FALSE); /*repatch the messages again*/
	}



	set_pwd_len(4); //default pwd len is 4
	return ret;
}

void set_pwd_len(UINT8 len)
{
	pwd_len = len;
}

void PwdChangePos(OSD_RECT *rc)
{
	OSD_RECT r;
	UINT16 l, t, w, h;

	if (rc == NULL)
	{
		l = PWD_W_L;
		t = PWD_W_T;
		w = PWD_W_W;
		h = PWD_W_H;
	}
	else
	{
		l = rc->uLeft;
		t = rc->uTop;
		w = rc->uWidth;
		h = rc->uHeight;
	}

	OSD_SetRect(&r, l, t, w, h);
	OSD_SetRect2(&g_win_pwd_con.head.frame, &r);
	OSD_SetRect(&r, l + PWD_TITLE_L, t + PWD_TITLE_T, PWD_TITLE_W, PWD_TITLE_H);
	OSD_SetRect2(&pwd_title.head.frame, &r);
	OSD_SetRect(&r, l + PWD_BMP_L, t + PWD_BMP_T, PWD_BMP_W, PWD_BMP_H);
	OSD_SetRect2(&pwd_bmp.head.frame, &r);
	OSD_SetRect(&r, l + PWD_EDT_L, t + PWD_EDT_T, PWD_EDT_W, PWD_EDT_H);
	OSD_SetRect2(&pwd_edt.head.frame, &r);
}
