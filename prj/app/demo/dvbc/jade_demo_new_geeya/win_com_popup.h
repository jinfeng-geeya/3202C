#ifndef _WIN_COM_POPUP_H_
#define _WIN_COM_POPUP_H_
//win_com_popup.h

#include <types.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>

//#include <api/libosd/osd_lib.h>

#include "win_com.h"
typedef enum
{
	WIN_POPUP_TYPE_SMSG = 0,
#if(defined(MULTI_CAS)&&(CAS_TYPE == CAS_ABEL))	
	WIN_POPUP_TYPE_INPUTPIN,
#endif		
	WIN_POPUP_TYPE_OK,
	WIN_POPUP_TYPE_OKNO,
	WIN_POPUP_TYPE_OKNOCANCLE,
} win_popup_type_t;


typedef enum
{
	WIN_POP_CHOICE_NULL = 0,
	WIN_POP_CHOICE_YES,
	WIN_POP_CHOICE_NO,
	WIN_POP_CHOICE_CANCEL,
} win_popup_choice_t;


void win_compopup_init(win_popup_type_t type);
void win_compopup_set_title(char *str, char *unistr, UINT16 strID);
void win_compopup_set_msg(char *str, char *unistr, UINT16 strID);
void win_compopup_set_msg_ext(char *str, char *unistr, UINT16 strID);
void win_compopup_set_btnstr(UINT32 btn_idx, UINT16 str_id);
void win_compopup_set_frame(UINT16 x, UINT16 y, UINT16 w, UINT16 h);
void win_compopup_set_pos(UINT16 x, UINT16 y);
void win_compopup_set_default_choice(win_popup_choice_t choice);


win_popup_choice_t win_compopup_open(void);

void win_compopup_close(void);

win_popup_choice_t win_com_popup_open(win_popup_type_t type, char *str, UINT16 strID);

win_popup_choice_t win_compopup_open_ext(UINT8 *back_saved);
void win_compopup_smsg_restoreback(void);
INT32 win_com_backup_rect(OSD_RECT *prc);
void win_com_restore_rect(OSD_RECT *prc);

void wincom_dish_move_popup_open(UINT16 pre_sat_id, UINT16 cur_sat_id, UINT32 tuner_idx, UINT8 *back_saved);

UINT8 win_compopup_get_status();
void win_compopup_set_status(UINT8 flag);

#endif //_WIN_COM_SMSG_H_
