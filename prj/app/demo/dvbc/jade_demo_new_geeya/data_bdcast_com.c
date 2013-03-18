#include <sys_config.h>
#include <basic_types.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libsi/si_module.h>
#include <api/libsi/si_service_type.h>
#include <api/libpub/lib_pub.h>
#include <api/libsi/si_eit.h>
#include <hld/decv/decv.h>

#include <api/libsi/sie_monitor.h>
#include <irca/irca_sys.h>



#include "win_com_popup.h"
#include "string.id"
#include "copper_common/system_data.h"
#include "menus_root.h"
#include "win_com.h"






BOOL _generic__dbcast_message_send(OSAL_ID mbf_id, ControlMsgType_t msg_type, UINT32 msg_code, BOOL if_clear_buffer)
{
	ER ret_val;
	ControlMsg_t control_msg;

	if(OSAL_INVALID_ID==mbf_id)
		return FALSE;

	control_msg.msgType = msg_type;
	control_msg.msgCode = msg_code;

	ret_val = osal_msgqueue_send(mbf_id, &control_msg, sizeof(ControlMsg_t), 0);
	if (OSAL_E_OK != ret_val)
	{
		if (if_clear_buffer)
		{
			ap_clear_all_message();
			ret_val = osal_msgqueue_send(mbf_id, &control_msg, sizeof(ControlMsg_t), 0);
		}
		else
			return FALSE;
	}
	else
		return TRUE;	
	

}








