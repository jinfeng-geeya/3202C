#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>

//#include <api/libosd/osd_lib.h>
//#include <hld/osd/osddrv_dev.h>

#include "../osdobjs_def.h"

#include "../string.id"
#include "../images.id"
#include "../osd_config.h"

#include "../win_com.h"
#include "../control.h"
#include "win_cas_com.h"

static VACTION pwd_con_keymap(POBJECT_HEAD pObj, UINT32 vkey);
static PRESULT pwd_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

//include the header from xform 
#include "win_ca_password.h"

static UINT8 pwd_len = 6;
static UINT8 pwd_char[7] = {0};
static UINT32 input_pwd_len = 0;
extern CONTAINER g_win_progname;//program name window when display a program

static UINT32 default_key_exit[] = {V_KEY_EXIT, V_KEY_MENU};

#define VACT_PWD_BACK (VACT_PASS+2)


/*******************************************************************************
 *	key mapping and event callback definition
 *******************************************************************************/
static VACTION pwd_con_keymap(POBJECT_HEAD pObj, UINT32 vkey)
{
	VACTION act = VACT_PASS;
	
	switch (vkey)
	{
		case V_KEY_0: case V_KEY_1: case V_KEY_2: case V_KEY_3: case V_KEY_4:	
		case V_KEY_5: case V_KEY_6: case V_KEY_7: case V_KEY_8: case V_KEY_9:
			act = vkey - V_KEY_0 + VACT_NUM_0;//cause the unknown event in the callback function
			break;
		case V_KEY_ENTER:
			act = VACT_ENTER;//cause the unknown event in the callback function
			break;
        case V_KEY_LEFT:
            act = VACT_PWD_BACK;//cause the unknown event in the callback function
            break;
		default:
			break;
	}

	return act;

}


static PRESULT password_bitmap_process(POBJECT_HEAD pObj, VACTION vact);

static PRESULT pwd_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	UINT32 *pParam;
	UINT16 vact;
	PRESULT ret = PROC_PASS;
	switch (event)
	{
		case EVN_UNKNOWN_ACTION:
			vact = (VACTION)(param1 >> 16);
			ret = password_bitmap_process(pObj, vact);
		case EVN_PRE_CLOSE:
			pParam = (UINT32*)param2;
			*pParam = 0;
			break;
		default:
			;
	}

	return ret;
}


static void add_bitmap_highlight(PCONTAINER pContainer, UINT8 bFocusID);
static void subtract_bitmap_highlight(PCONTAINER pContainer, UINT8 bFocusID);

static PRESULT password_bitmap_process(POBJECT_HEAD pObj, VACTION vact)
{	
	PRESULT ret = PROC_PASS;
	
	UINT8 bFocusID = OSD_GetFocusID(pObj);//the last focus id in password container, i.e. the id of current focused bitmap
	SYSTEM_DATA *sys_data = sys_data_get();

	UINT32 i;//be used to as index in case VACT_ENTER
	
	switch(vact)
	{
		case VACT_NUM_0: case VACT_NUM_1: case VACT_NUM_2: case VACT_NUM_3: case VACT_NUM_4: 
		case VACT_NUM_5: case VACT_NUM_6: case VACT_NUM_7: case VACT_NUM_8: case VACT_NUM_9:
			//if(input_pwd_len < pwd_len)
			{
				pin_pwd_input(PIN_ARRAY_0, (UINT8)(vact - VACT_NUM_0));
				//pwd_char[input_pwd_len++] = vact - VACT_NUM_0;
				add_bitmap_highlight((PCONTAINER) pObj, bFocusID);
			}
			break;
		case VACT_PWD_BACK:
/*			if(input_pwd_len > 0)
			{
				pwd_char[input_pwd_len--] = 0;
				subtract_bitmap_highlight((PCONTAINER) pObj, bFocusID);
			}*/
			break;
		case VACT_ENTER:			
#if 1
			for(i=6; i>0; i--)
			{
				subtract_bitmap_highlight((PCONTAINER) pObj, i);
			}
			ret = PROC_LEAVE;//password is correct, and so it will exit from the password window
#else			
			if(input_pwd_len==pwd_len && memcmp(pwd_char, sys_data->menu_password, pwd_len)==0)
			{
				//require the focus ID again in order to clear the show winstyle of the four bitmaps
				bFocusID = OSD_GetFocusID(pObj);
				for(i=bFocusID; i>1; --i)
				{
					pwd_char[input_pwd_len--] = 0;
					subtract_bitmap_highlight((PCONTAINER) pObj, i);
				}
				//ca_pwd_wrong_result.wStringID = 0;
				ret = PROC_LEAVE;//password is correct, and so it will exit from the password window
			}
			else
			{
				ca_pwd_wrong_result.wStringID = RS_WRONG_PASSWORD;
				
				//require the focus ID again in order to clear the show winstyle of the four bitmaps
				bFocusID = OSD_GetFocusID(pObj);
				for(i=bFocusID; i>1; --i)
				{
					pwd_char[input_pwd_len--] = 0;
					subtract_bitmap_highlight((PCONTAINER) pObj, i);
				}				
				OSD_DrawObject((POBJECT_HEAD)&ca_pwd_wrong_result, C_UPDATE_ALL);
				ca_pwd_wrong_result.wStringID = 0;
			}
#endif
			break;
		default:
			break;				
	}

	return ret;
}


static void add_bitmap_highlight(PCONTAINER pContainer, UINT8 bFocusID)
{
	if(1 == bFocusID)
	{
		ca_pwd_bitmap1.wIconID = IM_PASSWORD_BUTTON_01_HI;
		++pContainer->FocusObjectID;
		OSD_DrawObject((POBJECT_HEAD)&ca_pwd_bitmap1, C_UPDATE_ALL);
	}
	else if(2 == bFocusID)
	{
		ca_pwd_bitmap2.wIconID = IM_PASSWORD_BUTTON_01_HI;
		++pContainer->FocusObjectID;
		OSD_DrawObject((POBJECT_HEAD)&ca_pwd_bitmap2, C_UPDATE_ALL);
	}
	else if(3 == bFocusID)
	{
		ca_pwd_bitmap3.wIconID = IM_PASSWORD_BUTTON_01_HI;
		++pContainer->FocusObjectID;
		OSD_DrawObject((POBJECT_HEAD)&ca_pwd_bitmap3, C_UPDATE_ALL);
	}
	else if(4 == bFocusID)
	{
		ca_pwd_bitmap4.wIconID = IM_PASSWORD_BUTTON_01_HI;
		++pContainer->FocusObjectID;
		OSD_DrawObject((POBJECT_HEAD)&ca_pwd_bitmap4, C_UPDATE_ALL);
	}
	else if(5 == bFocusID)
	{
		ca_pwd_bitmap5.wIconID = IM_PASSWORD_BUTTON_01_HI;
		++pContainer->FocusObjectID;
		OSD_DrawObject((POBJECT_HEAD)&ca_pwd_bitmap5, C_UPDATE_ALL);
	}
	else if(6 == bFocusID)
	{
		ca_pwd_bitmap6.wIconID = IM_PASSWORD_BUTTON_01_HI;
		++pContainer->FocusObjectID;
		OSD_DrawObject((POBJECT_HEAD)&ca_pwd_bitmap6, C_UPDATE_ALL);
	}
}


static void subtract_bitmap_highlight(PCONTAINER pContainer, UINT8 bFocusID)
{
	if(1 == bFocusID)
	{
		ca_pwd_bitmap1.wIconID = IM_PASSWORD_BUTTON_01;
		--pContainer->FocusObjectID;
		OSD_DrawObject((POBJECT_HEAD)&ca_pwd_bitmap1, C_UPDATE_ALL);
	}
	else if(2 == bFocusID)
	{
		ca_pwd_bitmap2.wIconID = IM_PASSWORD_BUTTON_01;
		--pContainer->FocusObjectID;
		OSD_DrawObject((POBJECT_HEAD)&ca_pwd_bitmap2, C_UPDATE_ALL);
	}
	else if(3 == bFocusID)
	{
		ca_pwd_bitmap3.wIconID = IM_PASSWORD_BUTTON_01;
		--pContainer->FocusObjectID;
		OSD_DrawObject((POBJECT_HEAD)&ca_pwd_bitmap3, C_UPDATE_ALL);
	}
	else if(4 == bFocusID)
	{
		ca_pwd_bitmap4.wIconID = IM_PASSWORD_BUTTON_01;
		--pContainer->FocusObjectID;
		OSD_DrawObject((POBJECT_HEAD)&ca_pwd_bitmap4, C_UPDATE_ALL);
	}
	else if(5 == bFocusID)
	{
		ca_pwd_bitmap5.wIconID = IM_PASSWORD_BUTTON_01;
		--pContainer->FocusObjectID;
		OSD_DrawObject((POBJECT_HEAD)&ca_pwd_bitmap5, C_UPDATE_ALL);
	}
	else if(6 == bFocusID)
	{
		ca_pwd_bitmap6.wIconID = IM_PASSWORD_BUTTON_01;
		--pContainer->FocusObjectID;
		OSD_DrawObject((POBJECT_HEAD)&ca_pwd_bitmap6, C_UPDATE_ALL);
	}
}


/*******************************************************************************
 *	External functions
 *******************************************************************************/

extern UINT8 *osd_vscr_buffer[];

static UINT8 *g_back_buf = NULL;
static UINT32 g_back_buf_size;

#define MAX_BACK_BUF_SIZE (2*1024)
static UINT8 back_buf[MAX_BACK_BUF_SIZE];

BOOL win_ca_pwd_open(UINT32 *vkey_exist_array, UINT32 vkey_exist_cnt, OSD_RECT *rc)
{
	PRESULT bResult = PROC_LOOP;
	
	POBJECT_HEAD win_pwd = (POBJECT_HEAD) &g_win_ca_pwd_con;	
	
//	MEMSET(pwd_char, 0, sizeof(pwd_char));
	pin_pwd_init(1UL<<PIN_ARRAY_0);
	pin_pwd_set_status(0, FALSE);
//	pin_pwd_set_str(PIN_ARRAY_0);
	
	UINT32 *exit_pwd_keys;
	UINT32 exit_pwd_keycnt;
	
	if (NULL==vkey_exist_array || 0==vkey_exist_cnt)
	{
		exit_pwd_keys = default_key_exit;
		exit_pwd_keycnt = sizeof(default_key_exit) / sizeof(default_key_exit[0]);
	}
	else
	{
		exit_pwd_keys = vkey_exist_array;
		exit_pwd_keycnt = vkey_exist_cnt;
	}

//	if(rc != NULL)
//		PwdChangePos(rc);

	/* Start to try to save backgrand */
	OSD_RECT *frame = &win_pwd->frame;
    VSCR vscr;
	VSCR *lpVscr = OSD_GetTaskVscr(osal_task_get_current_id());
	OSD_UpdateVscr(lpVscr);
    lpVscr->lpbScr = NULL;

#ifdef USE_LIB_GE
	UINT32 vscr_size =(rc->uWidth*rc->uHeight)<<1;
	UINT8 *pBuf;// = (UINT8 *)MALLOC(vscr_size);
	
	UINT8 save_back_ok;//1:ok; 0:failed

    if (vscr_size < MAX_BACK_BUF_SIZE)
	{
		pBuf = back_buf;
		save_back_ok = 1;
	}
	else
	{
        if (g_back_buf != NULL && g_back_buf_size >= vscr_size)
			pBuf = g_back_buf;
		else
		{
    		if(g_back_buf!=NULL)
    		{
    			FREE(g_back_buf);
    			g_back_buf = NULL;
    		}

    		pBuf = (UINT8*)MALLOC(vscr_size);
    		MEMSET(pBuf,0,vscr_size);
    			
    		if (pBuf != NULL)
    		{
    			g_back_buf = pBuf;
                g_back_buf_size=vscr_size;
    			save_back_ok = 1;
    		}
    		else
    		{
    			libc_printf("\npop up malloc fail!\n");
    			save_back_ok = 0;
    			return -2;
    		}
		}
	}

	if (save_back_ok)
	{
		OSD_RectBackUp(lpVscr,pBuf,frame);
	}	
#else
	UINT32 vscr_size = (((frame->uWidth) >> FACTOR) + 1) *frame->uHeight;
	UINT8 *pBuf;// = (UINT8 *)MALLOC(vscr_size);
	
	UINT8 save_back_ok;//1:ok; 0:failed

    if (vscr_size < OSD_VSRC_MEM_MAX_SIZE / 2)
	{
		pBuf = &osd_vscr_buffer[osal_task_get_current_id() - 1][vscr_size];
		save_back_ok = 1;
	}
	else
	{
		if (g_back_buf != NULL && g_back_buf_size >= vscr_size)
			pBuf = g_back_buf;
		else
		{
			if (g_back_buf != NULL)
				FREE(g_back_buf);
			pBuf = (UINT8*)MALLOC(vscr_size);
			g_back_buf = pBuf;
			if (g_back_buf != NULL)
				g_back_buf_size = vscr_size;
		}
		if (pBuf != NULL)
			save_back_ok = 1;
		else
			save_back_ok = 0;
	}
    
	if (NULL == pBuf)
	{
		save_back_ok = 0;
		return FALSE;
	}
	save_back_ok = 1;//save background successfully
	
	//OSD_RectBackUp(lpVscr, pBuf, frame);
	OSD_SetRect2(&vscr.vR, frame);
	vscr.lpbScr = pBuf;
	OSD_GetRegionData(&vscr, &vscr.vR);
    
#endif

	/* End of try to save backgrand */

	OSD_ChangeFocus(win_pwd,1, 0);
	OSD_DrawObject(win_pwd, C_UPDATE_ALL);//draw the password window after the vitual screen's backup
	
	BOOL ret = TRUE;
	UINT32 hkey=INVALID_HK, vkey;
	UINT32 i;//index for the following loop
	
	while (bResult != PROC_LEAVE)//when inputted password is correct, it will exit from the password window
	{
		hkey = ap_get_key_msg();
		if (INVALID_HK == hkey || INVALID_MSG == hkey)
			continue;//invalid hardware key and invalid message will be ignored

		ap_hk_to_vk(0, hkey, &vkey);
		for (i=0; i<exit_pwd_keycnt; ++i)//check if the inputted key belongs to exit keys
		{
			if (vkey == exit_pwd_keys[i])
			{
				UINT8 bFocusID = OSD_GetFocusID(win_pwd);
				//clear the temporary data of password
				//pin_pwd_init(1UL<<PIN_ARRAY_0);
				//pin_pwd_set_status(0, FALSE);
				UINT32 i;
				for(i=6; i>0; i--)
				{
					subtract_bitmap_highlight((PCONTAINER) win_pwd, i);
				}
				ret = FALSE;				
				goto EXIT;
			}
		}

		bResult = OSD_ObjProc((POBJECT_HEAD)win_pwd, (MSG_TYPE_KEY << 16), hkey, 0);
		//process the win_pwd object through its corresponding callback function
	}

EXIT:
	if(save_back_ok)
	{
#ifdef USE_LIB_GE
		OSD_RectRestore(lpVscr, pBuf, frame);
#else
		OSD_RegionWrite(&vscr, &vscr.vR);
#endif
        if (g_back_buf != NULL)
		{
			FREE(g_back_buf);
			g_back_buf = NULL;
		}
	}
    lpVscr->lpbScr = NULL;
    
	if ((!ret) && ((vkey != V_KEY_EXIT) && (vkey != V_KEY_MENU)))
		ap_send_msg(CTRL_MSG_SUBTYPE_KEY, hkey, FALSE); /*if exit key got,repatch the messages again*/

	POBJECT_HEAD top_menu = menu_stack_get_top();
	if((!ret) && (NULL == top_menu || top_menu == (POBJECT_HEAD)&g_win_progname)
		&& (V_KEY_MENU == vkey))
		ap_send_msg(CTRL_MSG_SUBTYPE_KEY, hkey, FALSE); /*repatch the messages again*/

	return ret;
}

/*
void PwdChangePos(GUI_RECT *rc)
{
	GUI_RECT r;
	UINT16 l, t, w, h;
	
	l = rc->uStartX;
	t = rc->uStartY;
	w = rc->uWidth;
	h = rc->uHeight;
	
	OSD_SetRect(&r, l, t, w, h);
	OSD_SetRect2(&g_win_ca_pwd_con.head.frame, &r);
	
	OSD_SetRect(&r, l+20, t+20, 230, 30);
	OSD_SetRect2(&ca_pwd_title.head.frame, &r);
	
	OSD_SetRect(&r, l+76, t+100, 30, 30);
	OSD_SetRect2(&ca_pwd_bitmap1.head.frame, &r);
	
	OSD_SetRect(&r, l+106, t+100, 30, 30);
	OSD_SetRect2(&ca_pwd_bitmap2.head.frame, &r);

	OSD_SetRect(&r, l+136, t+100, 30, 30);
	OSD_SetRect2(&ca_pwd_bitmap3.head.frame, &r);

	OSD_SetRect(&r, l+166, t+100, 30, 30);
	OSD_SetRect2(&ca_pwd_bitmap4.head.frame, &r);

	OSD_SetRect(&r, l+196, t+100, 30, 30);
	OSD_SetRect2(&ca_pwd_bitmap5.head.frame, &r);

	OSD_SetRect(&r, l+226, t+100, 30, 30);
	OSD_SetRect2(&ca_pwd_bitmap6.head.frame, &r);

	OSD_SetRect(&r, l+20, t+50, 230, 30);
	OSD_SetRect2(&ca_pwd_wrong_result.head.frame, &r);				
}
*/
