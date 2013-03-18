#include <sys_config.h>
#include <retcode.h>
#include <types.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/sci/sci.h>
#include <hld/hld_dev.h>
#include <hld/decv/decv.h>
#include <hld/pan/pan_dev.h>
#include <hld/pan/pan.h>
#include <hld/pan/pan_key.h>
#include <api/libtsi/sec_tdt2.h>
#include <api/libpub/lib_pub.h>
#include <hld/pan/pan_dev.h>
#include <hld/pan/pan.h>
//#include <api/libosd/osd_vkey.h>

#include "sys_config.h"
#include "control.h"
#include "key.h"
#include "win_com.h"

#define SIGNAL_PRINTF		PRINTF

static OSAL_ID INPUT_TASK_ID;

#define INPUT_TASK_PRIORITY		OSAL_PRI_NORMAL
#define INPUT_TASK_STACKSIZE		0x1000
#define INPUT_TASK_QUANTUM		10

#define IR_KEY_MASK	0xFFFFFFFF
#define IR_NEC_KEY_MASK	0xFFFF0000
#define IR_KF_KEY_MASK		0xFFFF0000

/***************************************************/
/*      ALL kinds of RCU code defintion             */
/***************************************************/
enum signal_flag signal_detect_flag;
UINT8 upgrade_detect_flag = 0;
UINT32 key_address_mask[IRP_MAX_NUM];
UINT32 key_address[IRP_MAX_NUM];
UINT8 key_enable_flag = TRUE;

extern sys_state_t system_state;
//extern UINT8 get_fp51_time;
#define KEY_DETECT_INTERVAL     50
#define SIGNAL_DETECT_DELAY_TIMES (300/KEY_DETECT_INTERVAL)///ms
#define DELAY_SIGNAL_DETECT_DELAY_TIMES (SIGNAL_DETECT_DELAY_TIMES*6)

static UINT8 key_task_suspend_flag = 0;
UINT32 CHCHG_TICK_PRINTF_time=0;
extern char g_cc_step_name[][23];
extern UINT32 g_cc_play_step;
// ALI25C00
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
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_RIGHT&0xFFFF),		V_KEY_RIGHT},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_UP&0xFFFF),		V_KEY_UP},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_DOWN&0xFFFF),		V_KEY_DOWN},

	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_V_UP&0xFFFF),		V_KEY_V_UP},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_V_DOWN&0xFFFF),	V_KEY_V_DOWN},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_C_UP&0xFFFF),		V_KEY_C_UP},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_C_DOWN&0xFFFF),	V_KEY_C_DOWN},

	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_ENTER&0xFFFF),		V_KEY_ENTER},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_P_UP&0xFFFF),		V_KEY_P_UP},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_P_DOWN&0xFFFF),	V_KEY_P_DOWN},

	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_TEXT&0xFFFF),		V_KEY_TEXT},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_POWER&0xFFFF),		V_KEY_POWER},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_PREV&0xFFFF),		V_KEY_PREV},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_NEXT&0xFFFF),		V_KEY_NEXT},

	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_AUDIO&0xFFFF),		V_KEY_AUDIO},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_TIMER&0xFFFF),		V_KEY_TIMER},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_SUBTITLE&0xFFFF),	V_KEY_SUBTITLE},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_LIST&0xFFFF),		V_KEY_LIST},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_NEWS&0xFFFF),		V_KEY_DATARADIO},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_VOD&0xFFFF),		V_KEY_VOD},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_STOCK&0xFFFF),		V_KEY_STOCK},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_MAIL&0xFFFF),		V_KEY_MAIL},

	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_FIND&0xFFFF),		V_KEY_FIND},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_MUTE&0xFFFF),		V_KEY_MUTE},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_PAUSE&0xFFFF),		V_KEY_PAUSE},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_SUBTITLE&0xFFFF),	V_KEY_SUBTITLE},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_HELP&0xFFFF),		V_KEY_HELP},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_INFOR&0xFFFF),		V_KEY_INFOR},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_EXIT&0xFFFF),		V_KEY_EXIT},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_TVSAT&0xFFFF),		V_KEY_TVSAT},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_TVRADIO&0xFFFF),	V_KEY_TVRADIO},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_FAV&0xFFFF),		V_KEY_FAV},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_LANG&0xFFFF),		V_KEY_LANGUAGE},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_ZOOM&0xFFFF),		V_KEY_ZOOM},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_EPG&0xFFFF),		V_KEY_EPG},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_MENU&0xFFFF),		V_KEY_MENU},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_RECALL&0xFFFF),	V_KEY_RECALL},

	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_RED&0xFFFF),		V_KEY_RED},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_GREEN&0xFFFF),		V_KEY_GREEN},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_YELLOW&0xFFFF),	V_KEY_YELLOW},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_BLUE&0xFFFF),		V_KEY_BLUE},

	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_PLAY&0xFFFF),		V_KEY_PLAY},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_ALI00_TYPE<<24)|(IR_ALI00_HKEY_STOP&0xFFFF),		V_KEY_STOP},
};
// Geeya remote device
const struct ir_key_map_t geeya_itou_key_tab[] =
{
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_GEEYA_TYPE<<24)|(0x01ff08f7&0xFFFF),			V_KEY_0},	
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_GEEYA_TYPE<<24)|(0x01ff8877&0xFFFF),			V_KEY_1},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_GEEYA_TYPE<<24)|(0x01ff48b7&0xFFFF),			V_KEY_2},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_GEEYA_TYPE<<24)|(0x01ffc837&0xFFFF),			V_KEY_3},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_GEEYA_TYPE<<24)|(0x01ff28d7&0xFFFF),			V_KEY_4},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_GEEYA_TYPE<<24)|(0x01ffa857&0xFFFF),			V_KEY_5},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_GEEYA_TYPE<<24)|(0x01ff6897&0xFFFF),			V_KEY_6},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_GEEYA_TYPE<<24)|(0x01ffe817&0xFFFF),			V_KEY_7},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_GEEYA_TYPE<<24)|(0x01ff18e7&0xFFFF),			V_KEY_8},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_GEEYA_TYPE<<24)|(0x01ff9867&0xFFFF),			V_KEY_9},

	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_GEEYA_TYPE<<24)|(0x01ffc03f&0xFFFF),		V_KEY_LEFT},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_GEEYA_TYPE<<24)|(0x01ff40bf&0xFFFF),		V_KEY_RIGHT},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_GEEYA_TYPE<<24)|(0x01ff00ff&0xFFFF),		V_KEY_UP},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_GEEYA_TYPE<<24)|(0x01ff807f&0xFFFF),		V_KEY_DOWN},


	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_GEEYA_TYPE<<24)|(0x01fff807&0xFFFF),	V_KEY_ENTER},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_GEEYA_TYPE<<24)|(0x01ff5aa5&0xFFFF),		V_KEY_V_UP},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_GEEYA_TYPE<<24)|(0x01fffa05&0xFFFF),	V_KEY_V_DOWN},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_GEEYA_TYPE<<24)|(0x01ff1ae5&0xFFFF),		V_KEY_P_UP},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_GEEYA_TYPE<<24)|(0x01ffba45&0xFFFF),	V_KEY_P_DOWN},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_GEEYA_TYPE<<24)|(0x01ff50af&0xFFFF),	V_KEY_POWER},

	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_GEEYA_TYPE<<24)|(0x01ff02fd&0xFFFF),		V_KEY_AUDIO},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_GEEYA_TYPE<<24)|(0x01ff609f&0xFFFF),		V_KEY_LIST},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_GEEYA_TYPE<<24)|(0x01ffb04f&0xFFFF),		V_KEY_MUTE},

	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_GEEYA_TYPE<<24)|(0x01ffb847&0xFFFF),	V_KEY_MAIL},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_GEEYA_TYPE<<24)|(0x01ff10ef&0xFFFF),	V_KEY_INFOR},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_GEEYA_TYPE<<24)|(0x01ff38c7&0xFFFF),	V_KEY_EXIT},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_GEEYA_TYPE<<24)|(0x01ffd827&0xFFFF),		V_KEY_TVRADIO},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_GEEYA_TYPE<<24)|(0x01ff20df&0xFFFF),		V_KEY_TVRADIO},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_GEEYA_TYPE<<24)|(0x01ff30cf&0xFFFF),		V_KEY_FAV},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_GEEYA_TYPE<<24)|(0x01ff7887&0xFFFF),	V_KEY_EPG},

	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_GEEYA_TYPE<<24)|(0x01ff827d&0xFFFF),		V_KEY_MP},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_GEEYA_TYPE<<24)|(0x01ffe01f&0xFFFF),		V_KEY_FIND},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_GEEYA_TYPE<<24)|(0x01ff708f&0xFFFF),		V_KEY_FB},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_GEEYA_TYPE<<24)|(0x01ff58a7&0xFFFF),	V_KEY_MENU},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_GEEYA_TYPE<<24)|(0x01ffd02f&0xFFFF),	V_KEY_RECALL},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_GEEYA_TYPE<<24)|(0x01ffa05f&0xFFFF),		V_KEY_RED},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_GEEYA_TYPE<<24)|(0x01ff906f&0xFFFF),	V_KEY_GREEN},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_GEEYA_TYPE<<24)|(0x01ff42bd&0xFFFF),		V_KEY_YELLOW},
	{(PAN_KEY_TYPE_REMOTE<<29)|(PAN_KEY_PRESSED<<28)|(IRP_GEEYA_TYPE<<24)|(0x01ffc23d&0xFFFF),	V_KEY_BLUE},
		
	{(PAN_KEY_TYPE_PANEL<<29)|(PAN_KEY_PRESSED<<28)|(IRP_GEEYA_TYPE<<24)|(0xffff0004&0xFFFF),	V_KEY_ENTER },
	{(PAN_KEY_TYPE_PANEL<<29)|(PAN_KEY_PRESSED<<28)|(IRP_GEEYA_TYPE<<24)|(0xffff0002&0xFFFF),	V_KEY_MENU },
	{(PAN_KEY_TYPE_PANEL<<29)|(PAN_KEY_PRESSED<<28)|(IRP_GEEYA_TYPE<<24)|(0xFFFF0040&0xFFFF),	V_KEY_LEFT },
	{(PAN_KEY_TYPE_PANEL<<29)|(PAN_KEY_PRESSED<<28)|(IRP_GEEYA_TYPE<<24)|(0xffff0010&0xFFFF),	V_KEY_UP },
	{(PAN_KEY_TYPE_PANEL<<29)|(PAN_KEY_PRESSED<<28)|(IRP_GEEYA_TYPE<<24)|(0xffff0020&0xFFFF),	V_KEY_DOWN },
	{(PAN_KEY_TYPE_PANEL<<29)|(PAN_KEY_PRESSED<<28)|(IRP_GEEYA_TYPE<<24)|(0xffff0080&0xFFFF),	V_KEY_RIGHT },
	#ifdef STANDBY_BY_PANL_KEY
	{(PAN_KEY_TYPE_PANEL<<29)|(PAN_KEY_PRESSED<<28)|(IRP_GEEYA_TYPE<<24)|(0xffff0001&0xFFFF),	V_KEY_POWER },
	#endif
};
const struct ir_key_map_t panel_itou_key_tab[] =
{
	//Panel keys
	{(PAN_KEY_TYPE_PANEL<<29)|(PAN_KEY_PRESSED<<28)|(PAN_ALI_HKEY_POWER&0xFFFF),		V_KEY_POWER},
	{(PAN_KEY_TYPE_PANEL<<29)|(PAN_KEY_PRESSED<<28)|(PAN_ALI_HKEY_MENU&0xFFFF),		 	V_KEY_MENU},
	{(PAN_KEY_TYPE_PANEL<<29)|(PAN_KEY_PRESSED<<28)|(PAN_ALI_HKEY_ENTER&0xFFFF),		V_KEY_ENTER },
	{(PAN_KEY_TYPE_PANEL<<29)|(PAN_KEY_PRESSED<<28)|(PAN_ALI_HKEY_EXIT&0xFFFF),		 	V_KEY_EXIT},
	{(PAN_KEY_TYPE_PANEL<<29)|(PAN_KEY_PRESSED<<28)|(PAN_ALI_HKEY_UP&0xFFFF),		 	V_KEY_UP},
	{(PAN_KEY_TYPE_PANEL<<29)|(PAN_KEY_PRESSED<<28)|(PAN_ALI_HKEY_DOWN&0xFFFF),		 	V_KEY_DOWN},
	{(PAN_KEY_TYPE_PANEL<<29)|(PAN_KEY_PRESSED<<28)|(PAN_ALI_HKEY_LEFT&0xFFFF),		 	V_KEY_LEFT},
	{(PAN_KEY_TYPE_PANEL<<29)|(PAN_KEY_PRESSED<<28)|(PAN_ALI_HKEY_RIGHT&0xFFFF),		V_KEY_RIGHT},
};

IR_Key_Map_t ir_key_maps[] =
{
	{ALI25C00, IRP_ALI00_PROTOCOL, IRP_GEEYA_TYPE, IR_GEEYA_KEY_ADDR, IR_ALI00_KEY_MASK, geeya_itou_key_tab, sizeof(geeya_itou_key_tab)/sizeof(struct ir_key_map_t)},
	{ALI25C00, IRP_ALI00_PROTOCOL, IRP_ALI00_TYPE, IR_ALI00_KEY_ADDR, IR_ALI00_KEY_MASK, panel_itou_key_tab, sizeof(panel_itou_key_tab)/sizeof(struct ir_key_map_t)},

	{ALI25C00, IRP_ALI00_PROTOCOL, IRP_ALI00_TYPE, IR_ALI00_KEY_ADDR, IR_ALI00_KEY_MASK, ali00_itou_key_tab, sizeof(ali00_itou_key_tab)/sizeof(struct ir_key_map_t)},
};


/***************************************************/
void key_task_suspend()
{
	key_task_suspend_flag = 1;
}

void key_task_resume()
{
	key_task_suspend_flag = 0;
}



void key_set_upgrade_check_flag(UINT8 upgrade_flag)
{
	upgrade_detect_flag = upgrade_flag;
}

static UINT8 set_signal_check_flag = 0;
void key_set_signal_check_flag(enum signal_flag flag)
{
	signal_detect_flag = flag;
	set_signal_check_flag = 1;
}

void key_set_enable_flag(UINT8 en)
{
	key_enable_flag = en;
}

extern UINT8 led_data_len;
UINT8 key_device_init()
{
	UINT8 b;
	INT32 retVal;

	led_data_len = g_pan_dev->led_num;

	PRINTF("---------UIKeyInit() ,led display ----\n");
	return TRUE;
}


BOOL key_get_key(struct pan_key *key_struct, UINT32 timeout)
{
	UINT32 controlMsgCode;
	BOOL b_valid_key = FALSE;
	struct pan_key *pan_key;

	pan_key = pan_get_key(g_pan_dev, timeout);
	if (pan_key == NULL)
		return FALSE;

	if (FALSE == key_enable_flag)
		return FALSE;

	MEMCPY(key_struct, pan_key, sizeof(struct pan_key));
	
	if (PAN_KEY_INVALID == key_struct->code)
		return FALSE;

	return TRUE;
}

UINT32 multi_key_to_hk_msg(struct pan_key *key)
{
	int i;
	
	UINT8 irp_type = IRP_ALI00_TYPE;
	if (PAN_KEY_TYPE_REMOTE == key->type)
	{
  
        	for (i = 0; i < IRP_NUM; i++)
		{
			if ((key->code & ir_key_maps[i].irkey_mask) == ir_key_maps[i].irkey_addr)
			{
				irp_type = ir_key_maps[i].irp_type;
				break;
			}
		}

		if (i == IRP_NUM)
			return (UINT32)(-1);
	}
    
	//[31:29] pan_key_type, [28] key_status, [27:24], irp_type, [23 :16] key_cnt, [15:0] key_value
	return (((key->type & 0x7) << 29) | ((key->state & 0x1) << 28) | \
		((irp_type & 0xf) << 24) | ((key->count & 0xff) << 16) | (key->code &0xFFFF));	
}

extern UINT8 play_chan_nim_busy;

void key_check_signal_status(void)
{
	UINT8 level, quality, lock;
	UINT8 level_cur, quality_cur;
	static UINT8 level_pre = 0, quality_pre = 0;
	UINT32 msgcode;
	static UINT32 delay_time = 0, wait_time = DELAY_SIGNAL_DETECT_DELAY_TIMES;
	struct nim_device *nim_dev;
	static BOOL reset_status = FALSE;

	/* Signal detect */
	if (cur_tuner_idx == 0)
		nim_dev = g_nim_dev;
	else
		nim_dev = g_nim_dev2;

	nim_get_lock(nim_dev, &lock);

	delay_time++;
	
	//Send a msg for scrolling textfield.
	ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_SCROLL_STRING, 0, FALSE);

	if (SIGNAL_CHECK_PAUSE == signal_detect_flag)
	{
		osal_task_sleep(10);
		return ;
	}
	else if (SIGNAL_CHECK_RESET == signal_detect_flag)
	{
		//signal_detect_flag = SIGNAL_CHECK_NORMAL;
		//wait_time = DELAY_SIGNAL_DETECT_DELAY_TIMES;
		//delay_time = 0;
		if (set_signal_check_flag)
		{
			wait_time = DELAY_SIGNAL_DETECT_DELAY_TIMES;
			delay_time = 0;
		}
	}

	if (set_signal_check_flag)
		set_signal_check_flag = 0;

	if (delay_time == wait_time)
	{
		if (wait_time == DELAY_SIGNAL_DETECT_DELAY_TIMES)
			signal_detect_flag = SIGNAL_CHECK_NORMAL;

		nim_get_lock(nim_dev, &lock);
		nim_get_AGC(nim_dev, &level_cur);
		nim_get_SNR(nim_dev, &quality_cur);

		level = (level_cur + level_pre) / 2;
		quality = (quality_cur + quality_pre) / 2;
		level_pre = level_cur;
		quality_pre = quality_cur;


		dem_signal_to_display(lock, &level, &quality);
		msgcode = (cur_tuner_idx << 24) + (level << 16) + (quality << 8) + lock;
		ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_SIGNAL, msgcode, FALSE);

		if (play_chan_nim_busy)
		{
			win_signal_set_level_quality(0, 0, 0);
			lock = 0;
		}
		else
			win_signal_set_level_quality(level, quality, lock);

		key_pan_display_lock(lock);

		wait_time = SIGNAL_DETECT_DELAY_TIMES;
		delay_time = 0;
	}
}

UINT32 key_monitor_com()
{
	UINT8 i;
	UINT8 ch = 0xff;
	UINT8 *comtest_command = "comtest";
	INT32 comtest_command_len = STRLEN(comtest_command) - 1;
	UINT8 *apptest_command = "APP  init ok";
	INT32 apptest_command_len = STRLEN(apptest_command) - 1;
	UINT8 sync = 0;

	i = 0;
	if (UPGRADE_CHECK_HOST &upgrade_detect_flag)
	{
		while (sci_read_tm(SCI_FOR_RS232, &ch, 10) == SUCCESS)
		{
			if (apptest_command[i] == ch)
			{
				i++;
				if (i > apptest_command_len)
				{
					//system_state = SYS_STATE_UPGRAGE_HOST;
					return 1;
				}
			}
		}
	}

	if (UPGRADE_CHECK_SLAVE &upgrade_detect_flag)
	{
		i = 0;
		UINT32 timeout = 1000;
		while (sci_read_tm(SCI_FOR_RS232, &ch, timeout) == SUCCESS)
		{
			if (i <= comtest_command_len && comtest_command[i] == ch)
			{
				timeout = 1000 * 100;
				sci_write(SCI_FOR_RS232, ch);
				i++;
				if (i > comtest_command_len)
				 /* Is a comtest command string, wait for upgrade */
				{
					//system_state = SYS_STATE_UPGRAGE_SLAVE;
					//power_off_process();
					//power_on_process();
					return 2;
				}
			}
			else
				break;
		}
	}

	return 0;
}

#ifdef MULTI_CAS
#if (CAS_TYPE == CAS_CONAX)
#include <hld/dmx/dmx.h>
#include <api/libcas/mcas.h>
#include "conaxca/win_ca_mmi.h"

extern BOOL mmi_new_msg;
extern UINT32 g_vdec_DisplayIndex;
void key_check_mmi()
{
	struct VDec_StatusInfo CurStatus;
	static UINT32 mmi_tick;
	UINT32 duration;
	UINT32 vde_dipy_idx ;
	UINT8 av_ok = 0;

	if(get_mmi_showed()!=1&&get_mmi_showed()!=6)
		return;
	
	if(mmi_new_msg)
	{
		mmi_new_msg = FALSE;
		mmi_tick = osal_get_tick();
	}

	duration = osal_get_tick() - mmi_tick;
	if(duration>=3000)
	{
		vdec_io_control((struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0), VDEC_IO_GET_STATUS, (UINT32)&CurStatus);
		if(CurStatus.uFirstPicShowed==TRUE)
		{
			vde_dipy_idx = g_vdec_DisplayIndex;
			osal_task_sleep(200);
			if(vde_dipy_idx != g_vdec_DisplayIndex)
			{
				vde_dipy_idx = g_vdec_DisplayIndex;
				osal_task_sleep(200);
				if(vde_dipy_idx != g_vdec_DisplayIndex)
					av_ok = 1;
			}
		}
		if(av_ok==1)
		{
			if(get_mmi_showed() == 1)
			{
				MMI_PRINTF("video ok, MCAS_DISP_CARD_IN\n");
				ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MCAS, MCAS_DISP_CARD_IN<<16, FALSE);
			}
			else if(get_mmi_showed() == 6)
			{
				MMI_PRINTF("video ok, MCAS_DISP_NO_ACCESS_CLR\n");
				ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MCAS, MCAS_DISP_NO_ACCESS_CLR<<16, FALSE);			
			}
		}
	}
}
#endif
#endif

#ifdef MULTI_CAS
#if (CAS_TYPE == CAS_ABEL)
static BOOL abel_key_exist = FALSE;
BOOL abel_key_existed()
{
	return abel_key_exist;
}

void abel_key_is_existed()
{
	abel_key_exist = TRUE;
}

UINT32 abel_upgrade_monitor_com()
{
	UINT8 i;
	UINT8 ch = 0xff;
	UINT8 *comtest_command = "sertest";
	INT32 comtest_command_len = STRLEN(comtest_command) - 1;
	UINT8 *apptest_command = "APP  init ok";
	INT32 apptest_command_len = STRLEN(apptest_command) - 1;
	UINT8 sync = 0;

	i = 0;
	UINT32 timeout = 1000;
	while (sci_read_tm(SCI_FOR_RS232, &ch, timeout) == SUCCESS)
	{
		if (i <= comtest_command_len && comtest_command[i] == ch)
		{
			timeout = 1000 * 100;
			sci_write(SCI_FOR_RS232, ch);
			i++;
			if (i > comtest_command_len)
			 /* Is a comtest command string, wait for upgrade */
			{
				return 1;
			}
		}
		else
			break;
	}

	return 0;
}
#endif
#endif

static void key_task()
{
	UINT32 vk;
	UINT32 msg_code;
	struct pan_key key_struct;
	UINT32 ret;
	UINT32 i;

	signal_detect_flag = SIGNAL_CHECK_RESET;
	key_device_init();
	while (1)
	{
		/*suspend key task for ipanel mw*/
		if (key_task_suspend_flag == 1)
		{
			osal_task_sleep(100);
			continue;
		}

		osal_task_sleep(KEY_DETECT_INTERVAL);
		if ( system_state == SYS_STATE_INITIALIZING
		        || system_state == SYS_STATE_POWER_OFF
		        || system_state == SYS_STATE_UPGRAGE_HOST )
			continue;
#ifdef MULTI_CAS
#if (CAS_TYPE == CAS_ABEL)
		/* Host software upgrade detecting. */
		if(abel_key_existed() == FALSE)
		{
			ret = abel_upgrade_monitor_com();
			if (ret == 1)
			{
				ap_send_msg(CTRL_MSG_SUBTYPE_CMD_UPGRADE_SLAVE, 0, TRUE);
			}
		}
#endif
#endif

#if 0
		/* Host software upgrade detecting. */
		ret = key_monitor_com();
		if (ret == 1 || ret == 2)
		{
			if (ret == 1)
				;
			//ap_send_msg(CTRL_MSG_SUBTYPE_CMD_UPGRADE, 0, TRUE);
			else
				ap_send_msg(CTRL_MSG_SUBTYPE_CMD_UPGRADE_SLAVE, 0, TRUE);
		}
#endif
		if (key_get_key(&key_struct, 1))
		{
			UINT8 irp_type = 0;
			
			if (PAN_KEY_TYPE_PANEL == key_struct.type)
			{
				irp_type = 1;
				//libc_printf("%s() key_code %08X\n", __FUNCTION__, key_struct.code);
			}
			if (PAN_KEY_TYPE_REMOTE == key_struct.type)
			{

				//libc_printf("%s() key_code %08X\n", __FUNCTION__, key_struct.code);
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
			//[31:29] pan_key_type, [28] key_status, [27:24], irp_type, [23 :16] key_cnt, [15:0] key_value
			msg_code = ((key_struct.type & 0x7) << 29) | ((key_struct.state & 0x1) << 28) | \
				((irp_type & 0xf) << 24) | ((key_struct.count & 0xff) << 16) | (key_struct.code &0xFFFF);

			//libc_printf("%s() key_msg %08X\n", __FUNCTION__, msg_code);
#if 1//def CHCHG_TICK_PRINTF			
			if(key_struct.state)
			{
				g_cc_play_step=0;
				CHCHG_TICK_PRINTF_time=osal_get_tick();
				g_cc_play_tick[g_cc_play_step].step = GET_KEY;
				g_cc_play_tick[g_cc_play_step].start_tick = CHCHG_TICK_PRINTF_time;
				g_cc_play_tick[g_cc_play_step++].end_tick= CHCHG_TICK_PRINTF_time;
				//libc_printf("key  CHCHG_TICK_PRINTF_time=%d   \n",CHCHG_TICK_PRINTF_time);	
			}
#endif
			#ifdef AD_TYPE
			i=0;
			i=AD_GetKey(msg_code);
			if(i==0)
			#endif
			#ifdef MIS_AD
			if (MIS_GetIsMisControl())
			{
				ap_hk_to_vk(0, msg_code, &vk);
				MIS_DispatchInput(vk);
			}
			else
#endif
			ap_send_msg(CTRL_MSG_SUBTYPE_KEY, msg_code, FALSE);
		}
		else
		{
			key_check_signal_status();
#ifdef BURN_SERIAL_NUM
			api_com_monitor();
#endif
		}

#ifdef MULTI_CAS
#if (CAS_TYPE == CAS_CONAX)
		key_check_mmi();
#endif
#endif
	}//while(1) 
}

//-----------------------------------------------------------------------------
// FUNCTION:    UIKeyInit:
//
// DESCRIPTION: init the keyboard devices(including frontpanel and remote controller) hardware
//
// RETURN:      True	Successful
//              False	Error when init the hardware
//
// NOTES:
//
//-----------------------------------------------------------------------------
UINT8 key_Init()
{
	OSAL_T_CTSK t_ctsk;
	INT32 retVal;
	UINT32 i;

	t_ctsk.stksz = INPUT_TASK_STACKSIZE;
	t_ctsk.quantum = INPUT_TASK_QUANTUM;
	t_ctsk.itskpri = INPUT_TASK_PRIORITY;
	t_ctsk.name[0] = 'K';
	t_ctsk.name[1] = 'E';
	t_ctsk.name[2] = 'Y';
	t_ctsk.task = (FP)key_task;
	INPUT_TASK_ID = osal_task_create(&t_ctsk);
	if (INPUT_TASK_ID == OSAL_INVALID_ID)
		return FALSE;

	//rcu_fp_type.rcu_sw_pos = sys_data_get()->rcupos;
	return TRUE;
}


UINT8 led_display_data[10];
UINT8 led_display_flag = 0;
UINT8 led_data_len;
UINT8 lock_status;
UINT8 standby_status;
/* 0 - Normal, write both to LED and buf
1 - write to LED only
2 - write to buf only
 */

void key_pan_display(char *data, UINT32 len)
{
	UINT8 str[10];
	INT32 idx;
	if (led_display_flag == 0 || led_display_flag == 1)
	{
		str[0] = data[0];
		idx = 1;
		str[idx++] = data[1];
		str[idx++] = data[2];
		str[idx++] = data[3];
		if (len == 5)
			str[idx++] = data[4];


		pan_display(g_pan_dev, str, idx);
	}

	if (led_display_flag == 0 || led_display_flag == 2)
	{
		MEMCPY(led_display_data, data, len);
		led_display_data[len] = '\0';
	}
}

void key_pan_display_channel(UINT16 channel)
{
	UINT32 prog_num;
	char program[6];

	prog_num = channel + 1;

	if (g_pan_dev->led_num == 3)
		sprintf(program, "%03d", prog_num);
	else
		sprintf(program, "%04d", prog_num);

	key_pan_display(program, g_pan_dev->led_num);
}

void key_pan_display_lock(UINT8 flag)
{
	UINT8 uChannel[4];

	if (flag != lock_status)
	{
		uChannel[0] = 27;
		uChannel[1] = PAN_ESC_CMD_LBD;
		uChannel[2] = PAN_ESC_CMD_LBD_LOCK;
		if (flag)
			uChannel[3] = PAN_ESC_CMD_LBD_ON;
		else
			uChannel[3] = PAN_ESC_CMD_LBD_OFF;
		key_pan_display(uChannel, 4);
		lock_status = flag;
	}
}


void key_pan_display_standby(UINT8 flag)
{
	UINT8 uChannel[4];

	if (flag != standby_status)
	{
		uChannel[0] = 27;
		uChannel[1] = PAN_ESC_CMD_LBD;
		uChannel[2] = PAN_ESC_CMD_LBD_FUNCA;
		if (flag)
			uChannel[3] = PAN_ESC_CMD_LBD_ON;
		else
			uChannel[3] = PAN_ESC_CMD_LBD_OFF;
		key_pan_display(uChannel, 4);
		standby_status = flag;
	}
}

void key_pan_display_power(UINT8 flag)
{
	UINT8 uChannel[4];

	//if(flag != standby_status)
	{
		uChannel[0] = 27;
		uChannel[1] = PAN_ESC_CMD_LBD;
		uChannel[2] = PAN_ESC_CMD_LBD_POWER;
		if (flag)
			uChannel[3] = PAN_ESC_CMD_LBD_ON;
		else
			uChannel[3] = PAN_ESC_CMD_LBD_OFF;
		key_pan_display(uChannel, 4);
		//standby_status = flag;
	}
}

#if 0
void key_pan_set_key_table(IR_Key_Map_t *key_tab)
{
	UINT32 address;
	UINT32 i;

	/*
	#define SUPPORT_IR_NUM 0

	for(i=0;i<SUPPORT_IR_NUM;i++)
	{
	if(key_tab[i].irp_moudle == SYS_IRP_MOUDLE)
	{
	break;
	}
	}
	 */
	//i = 0;

	//ASSERT(i < SUPPORT_IR_NUM);

	for (i = 0; i < IRP_NUM; i++)
	{
		address = key_tab[i].irkey_addr; //LV_IR008_Address[rcu_sw_pos];
		switch (key_tab[i].irp_protocal)
		{
			case IRP_NEC:
				key_address[i] = address << 16;
				key_address_mask[i] = IR_NEC_KEY_MASK;
				break;
			case IRP_ITT:
			case IRP_NRC17:
			case IRP_SHARP:
			case IRP_SIRC:
			case IRP_RC5:
			case IRP_RC6:
			case IRP_KF:
			case IRP_LAB:
			case IRP_56560:
			case IRP_LOGIC:
			case IRP_NSE:
				key_address[i] = address << 16;
				key_address_mask[i] = IR_KF_KEY_MASK;
				break;
			default:
				return ; //hwIrKey = ((0xFF - address) << 24) + (address << 16) + ((0xFF - code) << 8) + code;
		}
	}
}
#endif

