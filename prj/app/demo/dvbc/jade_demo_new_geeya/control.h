#ifndef _CONTROL_H_
#define _CONTROL_H_

#include "win_com.h"



typedef struct
{
	UINT32 msg_type;
	UINT32 param;
} cti_msg_t;


struct ir_key_map_t
{
	//BYTE0							BYTE1					BYTE2			BYTE3
	//PAN_KEY_TYPE_REMOTE  	PAN_KEY_PRESSED 	press_cont	key
	UINT32 key_info;
	UINT32 ui_vkey;
};


struct channel_info
{
	UINT8 mode;
	UINT8 internal_group_idx;
	P_NODE p_node;
};

#define NR_BACKMSG_BASE 	0XA000  /*bigger than it means it is a backed msg*/
#define NR_NORMALMSG_BASE 	0X0000

typedef struct msg_backlog
{
	struct msg_backlog * next;
	struct msg_backlog * prev;
	ControlMsg_t		 msg_data;
}msg_backlog_t;

extern msg_backlog_t * msg_back_log_head;



BOOL ap_send_msg(ControlMsgType_t msg_type, UINT32 msg_code, BOOL if_clear_buffer);
void ap_clear_all_message(void);

#define INVALID_HK		0xFFFFFFFF
#define INVALID_MSG		0x00000000

/*
return:
INVALID_HK
INVALID_MSG
 */
UINT32 ap_get_key_msg(void);
UINT32 ap_hk_to_vk(UINT32 start, UINT32 key_info, UINT32 *vkey);
UINT32 ap_vk_to_hk(UINT32 start, UINT32 vkey, UINT32 *key_info);
UINT8 ap_get_vk(void);
UINT32 ap_get_hk(void);

#endif //_CONTROL_H_
