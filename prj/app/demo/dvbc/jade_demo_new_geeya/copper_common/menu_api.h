#ifndef _MENU_API_H_
#define _MENU_API_H_

#include "../win_com.h"
/* common display string array: used by all display str of TextField/Numsel */
#define MAX_DISP_STR_ITEMS 	50
#define MAX_DISP_STR_LEN		50
extern UINT16 display_strs[MAX_DISP_STR_ITEMS][MAX_DISP_STR_LEN];

#define MAX_LEN_DISP_STR		300
extern UINT16 len_display_str[MAX_LEN_DISP_STR];

void menu_stack_push(POBJECT_HEAD w);
void menu_stack_pop(void);
void menu_stack_pop_all(void);
POBJECT_HEAD menu_stack_get_top(void);
POBJECT_HEAD menu_stack_get(int offset);

/*
PRESULT menu_enter_root(POBJECT_HEAD p_handle, UINT32	para);
PRESULT menu_proc(UINT32 msg_type, UINT32 key);
 */

#endif //_MENU_API_H_
