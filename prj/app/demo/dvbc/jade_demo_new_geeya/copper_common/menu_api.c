#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
//#include <api/libosd/osd_lib.h>

#include "menu_api.h"
#include "com_api.h"

UINT16 display_strs[MAX_DISP_STR_ITEMS][MAX_DISP_STR_LEN];
UINT16 len_display_str[MAX_LEN_DISP_STR];

#define MAX_WINS_ON	20
POBJECT_HEAD g_menuStack[MAX_WINS_ON];
static UINT8 curOnWinNum = 0;

INT16 window_pos_in_stack(POBJECT_HEAD w)
{
	UINT8 i = 0;

	if (w == NULL)
		return  - 1;

	for (i = 0; i < curOnWinNum; i++)
	{
		if (g_menuStack[i] == w)
		{
			return i; //the window is in the stack.
		}
	}

	return  - 1; //do not find the window in stack
}

BOOL menu_stack_pop_to(POBJECT_HEAD w)
{
	INT16 pos;

	if (w == NULL)
		return FALSE;

	pos = window_pos_in_stack(w);
	if (pos ==  - 1)
	//do not find the window in stack
	{
		return FALSE;
	}
	else
	{
		curOnWinNum = pos + 1;
		return TRUE;
	}
}

#ifdef AD_TYPE
AD_Menu menu_reg[AD_MAX];
SCENE_TYPE cur_type;
void menu_reg_ad(AD_Menu *ad, UINT32 count)
{
	UINT32 i;
	for(i=0; i<count && i<AD_MAX; i++, ad++)
		menu_reg[ad->type]=*ad;
}

void menu_open_callback(POBJECT_HEAD pobj)
{
	UINT32 i;
	if(curOnWinNum == 0)
		AD_EnterApp();
	
	if(cur_type!=AD_NULL)
		AD_EraseAD(cur_type<<16);
	
	for(i=0;i<AD_MAX;i++)
	{
		if(menu_reg[i].pobj == pobj)
		{
			AD_DrawAD(menu_reg[i].type<<16);
			cur_type=menu_reg[i].type;
		}
	}
}

void menu_close_callback(POBJECT_HEAD pobj)
{
	UINT32 i;
	for(i=0;i<AD_MAX;i++)
	{
		if(menu_reg[i].pobj == pobj)
		{
			AD_EraseAD(menu_reg[i].type<<16);
			cur_type=AD_NULL;
		}
	}
}
#endif

void menu_stack_push(POBJECT_HEAD w)
{
	if (curOnWinNum >= MAX_WINS_ON)
		return ;
	g_menuStack[curOnWinNum++] = w;
}


void menu_stack_pop(void)
{
	if (curOnWinNum > 0)
		curOnWinNum--;
	#ifdef AD_TYPE
	if(curOnWinNum == 0)
		AD_QuitApp();
	#endif	
}

void menu_stack_pop_all(void)
{
	curOnWinNum = 0;
	#ifdef AD_TYPE
		AD_QuitApp();
	#endif	
}

UINT8 GetCurWinNum(void)
{
	return curOnWinNum;
}

POBJECT_HEAD menu_stack_get_top(void)
{
	if (curOnWinNum == 0)
		return NULL;

	return g_menuStack[curOnWinNum - 1];
}


POBJECT_HEAD menu_stack_get(int offset)
{
	if (offset >= 0)
	{
		if (offset < curOnWinNum)
			return g_menuStack[offset];
		else
			return NULL;
	}
	else
	//offset<0
	{
		if (curOnWinNum + offset > 0)
			return g_menuStack[curOnWinNum - 1+offset];
		else
			return NULL;
	}

}

PRESULT menu_enter_root(POBJECT_HEAD p_handle, UINT32 para)
{
	PRESULT proc_ret = PROC_PASS;
	ControlMsg_t msg;
	static POBJECT_HEAD p_prev_handle;

	if ((p_handle != NULL) && (p_handle != menu_stack_get_top()))
	{
		if (NULL == menu_stack_get_top())
			api_osd_mode_change(OSD_WINDOW);
		//ap_mute_hide(p_handle);
		if (NULL == menu_stack_get_top())
		{
			//ap_time_display_hide(p_handle);
		}
		proc_ret = OSD_ObjOpen(p_handle, para);
		if (PROC_LEAVE != proc_ret)
		{
			menu_stack_push(p_handle);
		}
	}

	return proc_ret;
}
