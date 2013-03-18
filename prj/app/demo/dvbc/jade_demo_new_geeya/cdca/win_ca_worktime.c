#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
//#include <api/libosd/osd_lib.h>

#include "win_cas_com.h"
#include "win_ca_worktime.h"

/*******************************************************************************
*	Local vriable & function declare
*******************************************************************************/

static void worktime_set_password_str (UINT8 display_type);
static void worktime_input_number ( UINT8 number );
static UINT8 worktime_check_time_valid(date_time start, date_time end);

/*******************************************************************************
*	key mapping and event callback definition
*******************************************************************************/
#define WORKTIME_START			1
#define WORKTIME_END			2
#define WORKTIME_INPUT_PWD	3

extern UINT8	pin_pwd_password_str[MAX_PIN_PWD_ARRAY][MAX_PIN_PWD_LEN];
static PRESULT work_time_item_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	UINT8	unact, valid = 0;
	UINT8	id = OSD_GetObjID ( pObj );
	UINT16   wt_rtn;

	date_time start = {0,0,0,0,0,0,0,0};
	date_time end = {0,0,0,0,0,0,0,0};

	switch ( event )
	{
		case EVN_FOCUS_PRE_GET:
			break;

		case EVN_FOCUS_PRE_LOSE:
			break;

		case EVN_UNKNOWN_ACTION:
			unact = ( VACTION ) ( param1 >> 16 ) ;
			if ( unact <= VACT_NUM_9 )
			{
				if(id == WORKTIME_INPUT_PWD)
				{
					pin_pwd_input(PIN_ARRAY_0, (UINT8)(unact - VACT_NUM_0));
					pin_pwd_set_str(PIN_ARRAY_0);
					OSD_TrackObject ( pObj, C_UPDATE_ALL );
				}
			}
			else if(unact == VACT_ENTER)
			{
				if((id == WORKTIME_START) ||(id == WORKTIME_END))
				{
					OSD_SetContainerFocus(&win_work_time, id+1);
					OSD_TrackObject((POBJECT_HEAD)&win_work_time,C_UPDATE_ALL);
				}
				else if(id == WORKTIME_INPUT_PWD)
				{
					//get the time from edit field
					OSD_GetEditFieldTimeDate(&work_time_edt1, &start);
					OSD_GetEditFieldTimeDate(&work_time_edt2, &end);
					valid = worktime_check_time_valid(start, end);

					if ((valid == 0xFE) ||(valid == 0xFD))
					{
						win_popup_msg(NULL, NULL, RS_CAERR_TIME_ERROR);
						OSD_SetContainerFocus(&win_work_time,WORKTIME_START);
						OSD_TrackObject((POBJECT_HEAD)&win_work_time,C_UPDATE_ALL);
						break;
					}		

					wt_rtn = CDCASTB_SetWorkTime(pin_pwd_password_str[PIN_ARRAY_0], start.hour, start.min,start.sec, end.hour, end.min, end.sec);
					if ((CDCA_RC_CARD_INVALID == wt_rtn)||(CDCA_RC_POINTER_INVALID == wt_rtn))
					{
						win_popup_msg(NULL, NULL, RS_CAERR_NOSMC);
						ret = PROC_LEAVE; //return from current window
						break;
					}
					else if (CDCA_RC_PIN_INVALID == wt_rtn)
					{
						win_popup_msg(NULL, NULL, RS_CAERR_PIN_ERROR_TF);
						pin_pwd_init(1UL<<PIN_ARRAY_0);
						pin_pwd_set_status(0, FALSE);
						pin_pwd_set_str(PIN_ARRAY_0);
						OSD_TrackObject ( pObj, C_UPDATE_ALL );
						break;
					}
					else if (CDCA_RC_WORKTIME_INVALID == wt_rtn)
					{
						win_popup_msg(NULL, NULL, RS_CAERR_TIME_ERROR);
						OSD_SetContainerFocus(&win_work_time,WORKTIME_START);
						OSD_TrackObject((POBJECT_HEAD)&g_win_work_time,C_UPDATE_ALL);
						ret = PROC_LOOP;
						break;
					}
					else if (CDCA_RC_OK == wt_rtn)
					{
						win_popup_msg(NULL, NULL, RS_CAERR_SET_SUCCESS);
						UINT16 msg_id = ap_get_cas_dispstr(0);
						if(( msg_id== RS_SHOWMSG_OUTWORKTIME))
						{
							ap_cas_message_setid(CDCA_MESSAGE_CANCEL_TYPE);	//close rating msg
						}
						ret = PROC_LEAVE; 
						break;
					}
					else
					{
						win_popup_msg(NULL, NULL, RS_CAERR_UNKNOWN);
						break;
					}

				}
			}
			else if ((unact == VACT_EDIT_LEFT) && (id == WORKTIME_INPUT_PWD))
			{
				pin_pwd_drawback(PIN_ARRAY_0);
				OSD_TrackObject (pObj, C_UPDATE_ALL );
			}			
			ret = PROC_LOOP;
			break;
	       default:
	            break;
	}
	return ret;

}

static VACTION work_time_item_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION Action = VACT_PASS;

	switch (key)
	{
		case V_KEY_LEFT:
			Action = VACT_EDIT_LEFT;
			break;
		case V_KEY_RIGHT:
			Action = VACT_EDIT_RIGHT;
			break;
		case V_KEY_ENTER:
			Action = VACT_ENTER;
			break;
		case V_KEY_0:
		case V_KEY_1:
		case V_KEY_2:
		case V_KEY_3:
		case V_KEY_4:
		case V_KEY_5:
		case V_KEY_6:
		case V_KEY_7:
		case V_KEY_8:
		case V_KEY_9:
			Action = key - V_KEY_0 + VACT_NUM_0;
			break;
		default:
			break;
	}

	return Action;
}

static VACTION work_time_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;
	UINT8 bID;

	bID = OSD_GetFocusID ( ( POBJECT_HEAD ) &g_win_work_time);
	switch(key)
	{
		case V_KEY_EXIT:
			BackToFullScrPlay();
			break;
		case V_KEY_MENU:
			act = VACT_CLOSE;
			break;
		default:
			act = VACT_PASS;
	}

	return act;	
}

static VACTION work_time_con_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;
	UINT8 bID;

	bID = OSD_GetFocusID ( ( POBJECT_HEAD ) &g_win_work_time);
	switch(key)
	{
		case V_KEY_UP:
			{
				if (WORKTIME_INPUT_PWD == bID)
				{
					pin_pwd_init(1UL<<PIN_ARRAY_0);
					pin_pwd_set_status(0, FALSE);
					pin_pwd_set_str(PIN_ARRAY_0);
				}
			}
			act = VACT_CURSOR_UP;
			break;
		case V_KEY_DOWN:
			if (WORKTIME_INPUT_PWD == bID)
			{
				pin_pwd_init(1UL<<PIN_ARRAY_0);
				pin_pwd_set_status(0, FALSE);
				pin_pwd_set_str(PIN_ARRAY_0);
			}
			act = VACT_CURSOR_DOWN;
			break;
		default:
			act = VACT_PASS;
	}

	return act;	
}



static PRESULT work_time_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	UINT8 StartHour, EndHour;
  	UINT32 wt_ret;
	date_time start = {0,0,0,0,0,0,0,0};// for debug init
	date_time end = {0,0,0,0,0,0,0,0};
	struct help_item_resource win_wt_help[] =
	{
		{1,IM_HELP_ICON_TB,RS_HELP_SELECT},
    		{0,RS_HELP_ENTER,RS_HELP_ENTER_INTO},
    		{0,RS_MENU,RS_HELP_BACK},
    		{0,RS_HELP_EXIT,RS_FULL_PLAY},
	};
    
	switch(event)
	{
		case EVN_PRE_OPEN:
			wt_ret = CDCASTB_GetWorkTime(&start.hour,&start.min, &start.sec, &end.hour, &end.min, &end.sec);
			if ((CDCA_RC_CARD_INVALID == wt_ret)||(CDCA_RC_UNKNOWN == wt_ret)||(CDCA_RC_POINTER_INVALID ==wt_ret))
			{
				win_popup_msg(NULL, NULL, RS_CAERR_NOSMC);
				ret = PROC_LEAVE;
				return ret;
			}
			//else is OK
			OSD_SetContainerFocus(&win_work_time, WORKTIME_START);
			break;
		case EVN_PRE_DRAW:
			CDCASTB_GetWorkTime(&start.hour,&start.min, &start.sec, &end.hour, &end.min, &end.sec);
		
			OSD_SetEditFieldContent(&work_time_edt1, STRING_NUMBER, start.hour*10000 + start.min*100 + start.sec);
			OSD_SetEditFieldContent(&work_time_edt2, STRING_NUMBER, end.hour*10000 + end.min*100 + end.sec);
			pin_pwd_init(1UL<<PIN_ARRAY_0);
			pin_pwd_set_status(0, FALSE);
			pin_pwd_set_str(PIN_ARRAY_0);
			break;
		case EVN_PRE_CLOSE:
			*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;	
			break;
		case EVN_POST_CLOSE:
			break;
	}

	return ret;
}


/*check the work time is valid or not, return the error type:
 *0x0: input are valid ,0xFE: start time is invalid, 0xFD: end time is invalid 
 *0xFF: end time is earlier than start time(this is supported by new lib)
 */
static UINT8 worktime_check_time_valid(date_time start, date_time end)
{
	UINT8 sH, sM, sS;
	UINT8 eH, eM, eS;

	sH = start.hour;
	sM = start.min;
	sS = start.sec;
	if ((sH>23) || (sM>59) || (sS>59))
		return 0XFE; // start time is invalid
		
	eH = end.hour;
	eM = end.min;
	eS = end.sec;

	if ((eH>23) || (eM>59) || (eS>59))
		return 0XFD; //end time is invlid
#if 0
	if ((eH<sH)
		|| ((eH==sH)&&(eM<sM))
		|| ((eH==sH)&&(eM==sM)&&(eS<=sS)))
	{
		return 0XFF;
	}
#endif 
	return 0;
}

