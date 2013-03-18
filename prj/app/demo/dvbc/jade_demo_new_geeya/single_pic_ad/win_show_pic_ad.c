#include <sys_config.h>

#ifdef SHOW_PICS
#if(SHOW_PICS == SHOW_SINGLE_PIC)

#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>

#include <api/libosd/osd_lib.h>

#include "../osdobjs_def.h"
#include "../string.id"
#include "../images.id"
#include "../osd_config.h"

#include "../menus_root.h"
#include "../win_com.h"
#include "../win_com_popup.h"
#include "../win_com_list.h"

#include "ad_pic.h"
/*******************************************************************************
 *	Objects & callback functions declaration *
 *******************************************************************************/
extern CONTAINER g_win_show_single_pic_ad;

/*******************************************************************************
callback & keymap functions declaration
 *******************************************************************************/
static VACTION win_show_single_pic_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT win_show_single_pic_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

/*******************************************************************************
 *	Objects & marco declaration
 *******************************************************************************/
/*rectangle parameters & style define for g_win_main_freq*/
#define WIN_BG_IDX   WSTL_SEARCH_BG_CON

#define WIN_BG_L     (TV_OFFSET_L+0)
#define WIN_BG_T     (TV_OFFSET_T+90)
#define WIN_BG_W     720
#define WIN_BG_H     400

#define LDEF_WIN(varCon,root,nxtObj,l,t,w,h,nxtInObj,focusID) \
    DEF_CONTAINER(varCon,root,nxtObj,C_ATTR_ACTIVE,0, \
    1,0,0,0,0,l,t,w,h, WIN_BG_IDX,WIN_BG_IDX,WIN_BG_IDX,WIN_BG_IDX,\
    win_show_single_pic_keymap, win_show_single_pic_callback,  \
    nxtInObj, 1,0)

/*******************************************************************************
*	Objects define
*******************************************************************************/
LDEF_WIN(g_win_show_single_pic_ad,NULL,NULL,WIN_BG_L,WIN_BG_T,WIN_BG_W,WIN_BG_H,NULL,0)

/*******************************************************************************
*	local variable define
*******************************************************************************/
struct help_item_resource show_pic_ad_help[3] = 
{
    {0,RS_MENU,RS_HELP_BACK},
    {0,RS_HELP_EXIT,RS_HELP_EXIT},  
    {1,IM_EPG_COLORBUTTON_RED,RS_HELP_SELECT},
};

/*******************************************************************************
 *	helper functions define
 *******************************************************************************/
static UINT8 gif_show_on;  //1,AD pic is to been showed ; 0, AD pic has been be showed
static UINT8 i=0;
static BOOL busy = FALSE;
static UINT8 pic_index = 0;
static void show_pic_ad(void)
{
	if(busy == TRUE)
		return;
	busy = TRUE;
    gif_show_on = (gif_show_on + 1)%2;
    if(gif_show_on==1)
    {
        //Init the GE for showing pic ad
        ad_pic_init(0);
//        show_flash_pic(0);  //show png        
//        show_flash_pic(1); //show gif
//        show_flash_pic(2);  //show jpg
//        show_flash_pic(3);  //show bmp

        show_flash_pic(i);
		pic_index = i;
        i++;
        if(i==4)
            i=0;
    }
    else
    {
        ad_pic_exit();
    }
	busy = FALSE;

}

void refresh_show_pic_view(void)
{
	if(menu_stack_get_top() != (POBJECT_HEAD)&g_win_show_single_pic_ad)
		return;
	if(busy == TRUE)
		return;
	busy = TRUE;
	if(gif_show_on == 1)
	{
		//ad_pic_exit();
		//ad_pic_init(0);
		show_flash_pic(pic_index);
	}
	busy = FALSE;
}

static VACTION win_show_single_pic_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

	switch (key)
	{
		case V_KEY_MENU:
			act = VACT_CLOSE;
			break;
		case V_KEY_EXIT:
            if(1==gif_show_on)
            	ad_pic_exit();
            restore_scene();
			BackToFullScrPlay();
			break;
        case V_KEY_RED:
            show_pic_ad();
            break;
                
		default:
			break;
	}

	return act;
}

static PRESULT win_show_single_pic_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;

	switch (event)
	{
		case EVN_PRE_OPEN:
            record_scene();  //close the porgram to show the ad pic without flicking
          
			wincom_open_title_ext(RS_SHOW_PIC, IM_TITLE_ICON_SYSTEM);
			wincom_open_help(show_pic_ad_help, 3);  
            gif_show_on = 0;
			break;
		case EVN_POST_OPEN:             
			break;
		case EVN_PRE_CLOSE:
			*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
			break;
		case EVN_POST_CLOSE:
            if(1==gif_show_on)
            	ad_pic_exit();
            restore_scene();
			break;
		case EVN_MSG_GOT:
			//main_req_message_proc(param1, param2);
			break;
	}

	return ret;
}

#endif
#endif

