#include <sys_config.h>
#include <retcode.h>
#include <types.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
//#include <bus/sci/sci.h>
//#include <hld/hld_dev.h>
#include <hld/pan/pan_dev.h>
#include <hld/pan/pan.h>
#include <hld/pan/pan_key.h>
//#include <api/libtsi/sec_tdt2.h>
#include <hld/pan/pan_dev.h>
#include <hld/pan/pan.h>
//#include <hld/decv/Decv.h>
//#include <api/libdsh/lib_dsh.h>
#include <api/libosd/osd_vkey.h>
#include "dev_handle.h"
#include "key.h"

//#define DUMP_KEY_CODE

#define 	INPUT_TASK_PRIORITY		OSAL_PRI_NORMAL
#define 	INPUT_TASK_STACKSIZE		0x1000
#define 	INPUT_TASK_QUANTUM		10
#define	IR_KEY_MASK	0xFFFFFFFF
#define	IR_NEC_KEY_MASK	0xFFFF0000
#define KEY_DETECT_INTERVAL     50

static OSAL_ID INPUT_TASK_ID;
UINT32	key_address_mask;
UINT32	key_address[2];	

// ALI25C00 48keys
const struct ir_key_map_t ali00_itou_key_tab[] =
{
	//IR keys
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_0&0xFFFF),			V_KEY_0},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_1&0xFFFF),			V_KEY_1},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_2&0xFFFF),			V_KEY_2},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_3&0xFFFF),			V_KEY_3},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_4&0xFFFF),			V_KEY_4},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_5&0xFFFF),			V_KEY_5},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_6&0xFFFF),			V_KEY_6},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_7&0xFFFF),			V_KEY_7},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_8&0xFFFF),			V_KEY_8},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_9&0xFFFF),			V_KEY_9},

	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_LEFT&0xFFFF),		V_KEY_LEFT},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_RIGHT&0xFFFF),	V_KEY_RIGHT},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_UP&0xFFFF),		V_KEY_UP},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_DOWN&0xFFFF),	V_KEY_DOWN},

	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_ENTER&0xFFFF),	V_KEY_ENTER},

	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_POWER&0xFFFF),	V_KEY_POWER},

#ifdef USB_MP_SUPPORT
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_PLAY&0xFFFF),		V_KEY_PLAY},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_STOP&0xFFFF),		V_KEY_STOP},
#endif
};

// ALI25C01 60keys
const struct ir_key_map_t ali01_itou_key_tab[] =
{
	//IR keys
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI01_TYPE<<24)|(IR_ALI01_HKEY_0&0xFFFF),			V_KEY_0},	
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI01_TYPE<<24)|(IR_ALI01_HKEY_1&0xFFFF),			V_KEY_1},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI01_TYPE<<24)|(IR_ALI01_HKEY_2&0xFFFF),			V_KEY_2},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI01_TYPE<<24)|(IR_ALI01_HKEY_3&0xFFFF),			V_KEY_3},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI01_TYPE<<24)|(IR_ALI01_HKEY_4&0xFFFF),			V_KEY_4},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI01_TYPE<<24)|(IR_ALI01_HKEY_5&0xFFFF),			V_KEY_5},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI01_TYPE<<24)|(IR_ALI01_HKEY_6&0xFFFF),			V_KEY_6},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI01_TYPE<<24)|(IR_ALI01_HKEY_7&0xFFFF),			V_KEY_7},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI01_TYPE<<24)|(IR_ALI01_HKEY_8&0xFFFF),			V_KEY_8},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI01_TYPE<<24)|(IR_ALI01_HKEY_9&0xFFFF),			V_KEY_9},

	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI01_TYPE<<24)|(IR_ALI01_HKEY_LEFT&0xFFFF),		V_KEY_LEFT},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI01_TYPE<<24)|(IR_ALI01_HKEY_RIGHT&0xFFFF),	V_KEY_RIGHT},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI01_TYPE<<24)|(IR_ALI01_HKEY_UP&0xFFFF),		V_KEY_UP},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI01_TYPE<<24)|(IR_ALI01_HKEY_DOWN&0xFFFF),	V_KEY_DOWN},

	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI01_TYPE<<24)|(IR_ALI01_HKEY_ENTER&0xFFFF),	V_KEY_ENTER},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI01_TYPE<<24)|(IR_ALI01_HKEY_POWER&0xFFFF),	V_KEY_POWER},
};

const struct ir_key_map_t panel_itou_key_tab[] =
{
	//Panel keys
	{(PAN_KEY_TYPE_PANEL<<29)|(PAN_KEY_PRESSED<<28)|(PAN_ALI_HKEY_POWER&0xFFFF),		V_KEY_POWER},
	{(PAN_KEY_TYPE_PANEL<<29)|(PAN_KEY_PRESSED<<28)|(PAN_ALI_HKEY_MENU&0xFFFF),		 	V_KEY_MENU},
	{(PAN_KEY_TYPE_PANEL<<29)|(PAN_KEY_PRESSED<<28)|(PAN_ALI_HKEY_ENTER&0xFFFF),		 	V_KEY_ENTER },
	{(PAN_KEY_TYPE_PANEL<<29)|(PAN_KEY_PRESSED<<28)|(PAN_ALI_HKEY_EXIT&0xFFFF),		 	V_KEY_EXIT},
	{(PAN_KEY_TYPE_PANEL<<29)|(PAN_KEY_PRESSED<<28)|(PAN_ALI_HKEY_UP&0xFFFF),		 	V_KEY_UP},
	{(PAN_KEY_TYPE_PANEL<<29)|(PAN_KEY_PRESSED<<28)|(PAN_ALI_HKEY_DOWN&0xFFFF),		 	V_KEY_DOWN},
	{(PAN_KEY_TYPE_PANEL<<29)|(PAN_KEY_PRESSED<<28)|(PAN_ALI_HKEY_LEFT&0xFFFF),		 	V_KEY_LEFT},
	{(PAN_KEY_TYPE_PANEL<<29)|(PAN_KEY_PRESSED<<28)|(PAN_ALI_HKEY_RIGHT&0xFFFF),		 	V_KEY_RIGHT},
};

IR_Key_Map_t ir_key_maps[] =
{
	{ALI25C00, IRP_ALI00_PROTOCOL, IRP_ALI00_TYPE, IR_ALI00_KEY_ADDR, IR_ALI00_KEY_MASK, panel_itou_key_tab, sizeof(panel_itou_key_tab)/sizeof(struct ir_key_map_t)},

	{ALI25C00, IRP_ALI00_PROTOCOL, IRP_ALI00_TYPE, IR_ALI00_KEY_ADDR, IR_ALI00_KEY_MASK, ali00_itou_key_tab, sizeof(ali00_itou_key_tab)/sizeof(struct ir_key_map_t)},
	{ALI25C01, IRP_ALI01_PROTOCOL, IRP_ALI01_TYPE, IR_ALI01_KEY_ADDR, IR_ALI01_KEY_MASK, ali01_itou_key_tab, sizeof(ali01_itou_key_tab)/sizeof(struct ir_key_map_t)},
};

static UINT8 key_task_suspend_flag = 0;

void key_task_suspend()
{
	key_task_suspend_flag = 1;
}

void key_task_resume()
{
	key_task_suspend_flag = 0;
}

BOOL key_get_key(struct pan_key *key_struct, UINT32 timeout)
{
	UINT32		controlMsgCode;
	BOOL 			b_valid_key = FALSE;
	struct pan_key *	pan_key;
	pan_key = pan_get_key(g_pan_dev,timeout);
	if (pan_key == NULL) 
		return FALSE;
		
	MEMCPY(key_struct, pan_key, sizeof(struct pan_key));
	if(PAN_KEY_INVALID == key_struct->code)
	{
		return FALSE;
	}
	else if(PAN_KEY_TYPE_REMOTE == key_struct->type)
	{
		if(((key_address_mask & key_struct->code) != key_address[0]) \
			&&((key_address_mask & key_struct->code) != key_address[1]))
		{
#ifndef DUMP_KEY_CODE
			return FALSE;
#endif
		}        

	}
	return TRUE;
}


static void key_task()
{
	UINT32	msg_code;
	struct pan_key key_struct;
	UINT32  ret;
	UINT32 i;

	while(1)
	{
		osal_task_sleep(KEY_DETECT_INTERVAL);

		if (key_task_suspend_flag == 1)
			continue;
            
		if(key_get_key(&key_struct, 1))
		{
#ifdef DUMP_KEY_CODE
	            if(PAN_KEY_PRESSED== key_struct.state)
	            {
	                libc_printf("key = 0x%x\n", key_struct.code);
	            }
	            continue;
#endif
//		msg_code = ((key_struct.type&0x7f)<<25) | ((key_struct.state&0x1)<<24) | ((key_struct.count&0xFF) <<16) |((key_struct.code>>16) & 0xff00)  | (key_struct.code & 0xFF);

			UINT8 irp_type = 0;
			if (PAN_KEY_TYPE_REMOTE == key_struct.type)
			{
				for (i = 0; i < IRP_NUM; i++)
				{
					if ((key_struct.code & ir_key_maps[i].irkey_mask) == ir_key_maps[i].irkey_addr)
					{
						irp_type = ir_key_maps[i].irp_type;
						break;
					}
				}

				if (i == IRP_NUM)
					continue;
			}
		msg_code = ((key_struct.type & 0x7) << 29) | ((key_struct.state & 0x1) << 28) | \
				((irp_type & 0xf) << 24) | ((key_struct.count & 0xff) << 16) | (key_struct.code &0xFFFF);

		boot_send_msg(CTRL_MSG_SUBTYPE_KEY, msg_code, FALSE);
		}

	}//while(1)
}

UINT8 key_Init()
{	
	OSAL_T_CTSK		t_ctsk;
	INT32	retVal;
	UINT32	i;

	t_ctsk.stksz	= 		INPUT_TASK_STACKSIZE;
	t_ctsk.quantum	= 	INPUT_TASK_QUANTUM;
	t_ctsk.itskpri	= 		INPUT_TASK_PRIORITY;
	t_ctsk.name[0]	= 'K';
	t_ctsk.name[1]	= 'E';
	t_ctsk.name[2]	= 'Y';
	t_ctsk.task = (FP)key_task;
	INPUT_TASK_ID = osal_task_create(&t_ctsk);
	if(INPUT_TASK_ID==OSAL_INVALID_ID)
		return FALSE;

	return TRUE;
}
