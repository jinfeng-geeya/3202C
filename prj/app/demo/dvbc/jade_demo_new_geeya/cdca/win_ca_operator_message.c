#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
//#include <api/libosd/osd_lib.h>

#include "win_cas_com.h"

//include the header from xform 
#include "win_ca_operator_message.h"


/*******************************************************************************
*	Callback and Keymap Functions
*******************************************************************************/
static VACTION message_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;
	UINT32 hKey;

	switch (key)
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
static PRESULT message_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	VACTION unact;

	switch (event)
	{
		case EVN_PRE_OPEN:
			//TODO: add code here
			break;
		case EVN_POST_OPEN:
			//TODO: add code here
			break;
		case EVN_PRE_CLOSE:
			*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
			break;
		case EVN_POST_CLOSE:
			//TODO: add code here
			break;
		case EVN_UNKNOWNKEY_GOT:
			//TODO: add code here
			break;
		case EVN_UNKNOWN_ACTION:
			//TODO: add code here
			break;
		case EVN_MSG_GOT:
			//TODO: add code here
			break;
	}
	return ret;
}

