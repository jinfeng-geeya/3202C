#ifndef _KEY_H_
#define _KEY_H_

#include <types.h>

enum signal_flag
{
	SIGNAL_CHECK_NORMAL,	
	SIGNAL_CHECK_PAUSE,
	SIGNAL_CHECK_RESET,
};

typedef enum
{
	IRP_NEC = 0,
	IRP_ITT,
	IRP_NRC17,
	IRP_SHARP,
	IRP_SIRC,
	IRP_RC5,
	IRP_RC6,    
	IRP_KF,
	IRP_LAB,
	IRP_56560,
	IRP_LOGIC,
	IRP_NSE,
} irp_protocol_t;

typedef struct
{
	const UINT32 irp_moudle;
	const irp_protocol_t irp_protocol;
	const UINT8 irp_type;
	const UINT32 irkey_addr;
	const UINT32 irkey_mask;
	const struct ir_key_map_t *key_map;
	const UINT8 irkey_num;
} IR_Key_Map_t;


#define UPGRADE_CHECK_PAUSE	0
#define UPGRADE_CHECK_HOST 	0x1
#define UPGRADE_CHECK_SLAVE	0x2


#define HKEY_NULL				0xFFFFFFFF

extern IR_Key_Map_t ir_key_maps[];
#define IRP_NUM					3 //(sizeof(ir_key_maps)/sizeof(IR_Key_Map_t))
#define IRP_MAX_NUM			6

/*Geeya key code*/
#define IR_GEEYA_KEY_ADDR		0x01ff0000
#define IRP_GEEYA_TYPE          1   
/* ALI25C00 key code */

#define IR_ALI00_KEY_ADDR		0x60df0000
#define IR_ALI00_KEY_MASK		0xffff0000
#define IRP_ALI00_PROTOCOL		IRP_NEC
#define IRP_ALI00_TYPE			0

#define IR_ALI00_HKEY_0			0x60df926d
#define IR_ALI00_HKEY_1			0x60dfc837
#define IR_ALI00_HKEY_2			0x60df08f7
#define IR_ALI00_HKEY_3			0x60df8877
#define IR_ALI00_HKEY_4			0x60dff00f
#define IR_ALI00_HKEY_5			0x60df30cf
#define IR_ALI00_HKEY_6			0x60dfb04f
#define IR_ALI00_HKEY_7			0x60dfd02f
#define IR_ALI00_HKEY_8			0x60df10ef
#define IR_ALI00_HKEY_9			0x60df906f

#define IR_ALI00_HKEY_LEFT		0x60df38c7
#define IR_ALI00_HKEY_RIGHT		0x60df12ed
#define IR_ALI00_HKEY_UP		0x60df22dd
#define IR_ALI00_HKEY_DOWN		0x60dfb847

#define IR_ALI00_HKEY_V_UP		0x60df48b7//0x60df12ed
#define IR_ALI00_HKEY_V_DOWN	0x60df01fe//0x60df38c7
//no channel key, use pre ,next
#define IR_ALI00_HKEY_C_UP		0x60df20df
#define IR_ALI00_HKEY_C_DOWN	0x60df0af5

#define IR_ALI00_HKEY_ENTER		0x60df3ac5
#define IR_ALI00_HKEY_P_UP		0x60dfd22d
#define IR_ALI00_HKEY_P_DOWN	0x60dfe01f

#define IR_ALI00_HKEY_TEXT		0x60df827d
#define IR_ALI00_HKEY_POWER		0x60df708f
#define IR_ALI00_HKEY_PREV		0x60df20df
#define IR_ALI00_HKEY_NEXT		0x60df0af5

#define IR_ALI00_HKEY_AUDIO		0x60df629d
#define IR_ALI00_HKEY_TIMER		HKEY_NULL
#define IR_ALI00_HKEY_SUBTITLE	0x60df807f
#define IR_ALI00_HKEY_LIST		HKEY_NULL
#define IR_ALI00_HKEY_STOCK		HKEY_NULL

//MP(60key RC)
#define IR_ALI00_HKEY_NEWS		0x60df50af
//SLEEP(60key RC)
#define IR_ALI00_HKEY_VOD		0x60df9867
//PIP(60key RC)
#define IR_ALI00_HKEY_MAIL		0x60df6a95
#define IR_ALI00_HKEY_LOCALPLAY	    0x60df7887	

#define IR_ALI00_HKEY_FIND		0x60dfe21d
#define IR_ALI00_HKEY_MUTE		0x60dfa05f
#define IR_ALI00_HKEY_PAUSE		0x60df7a85
#define IR_ALI00_HKEY_HELP		HKEY_NULL
#define IR_ALI00_HKEY_INFOR		0x60df6897
#define IR_ALI00_HKEY_EXIT		0x60df42bd
#define IR_ALI00_HKEY_TVSAT		0x60df52ad
#define IR_ALI00_HKEY_TVRADIO	0x60df02fd
#define IR_ALI00_HKEY_FAV		0x60dfc23d
#define IR_ALI00_HKEY_LANG		HKEY_NULL
#define IR_ALI00_HKEY_ZOOM		0x60df40bf
#define IR_ALI00_HKEY_EPG		0x60df00ff
#define IR_ALI00_HKEY_MENU		0x60df2ad5
#define IR_ALI00_HKEY_RECALL	0x60dfc03f
                                      
#define IR_ALI00_HKEY_RED		0x60df609f
#define IR_ALI00_HKEY_GREEN		0x60df7887
#define IR_ALI00_HKEY_YELLOW	0x60dff807
#define IR_ALI00_HKEY_BLUE		0x60dfba45

#define IR_ALI00_HKEY_PLAY		0x60df18e7
#define IR_ALI00_HKEY_STOP		0x60dfe817	

#if (SYS_MAIN_BOARD == BOARD_S3202C_DEMO_01V01)
#define PAN_ALI_HKEY_POWER	0xFFFF0001//0xFFFF0002
#define PAN_ALI_HKEY_MENU		0xFFFF0002//0xFFFF0080
#define PAN_ALI_HKEY_ENTER		0xFFFF0004//0xFFFF0008
#define PAN_ALI_HKEY_EXIT		HKEY_NULL//0xFFFF0020
#define PAN_ALI_HKEY_UP		0xFFFF0010//0xFFFF0040
#define PAN_ALI_HKEY_DOWN		0xFFFF0020//0xFFFF0010
#define PAN_ALI_HKEY_LEFT		0xFFFF0040//0xFFFF0004
#define PAN_ALI_HKEY_RIGHT		0xFFFF0080//0xFFFF0001
#else
#define PAN_ALI_HKEY_POWER	0xFFFF0001//0xFFFF0002
#define PAN_ALI_HKEY_MENU		0xFFFF0002//0xFFFF0080
#define PAN_ALI_HKEY_ENTER		0xFFFF0004//0xFFFF0008
#define PAN_ALI_HKEY_EXIT		HKEY_NULL//0xFFFF0020
#define PAN_ALI_HKEY_UP		0xFFFF0010//0xFFFF0040
#define PAN_ALI_HKEY_DOWN		0xFFFF0020//0xFFFF0010
#define PAN_ALI_HKEY_LEFT		0xFFFF0040//0xFFFF0004
#define PAN_ALI_HKEY_RIGHT		0xFFFF0080//0xFFFF0001
#endif

UINT8 key_Init();

void key_pan_display(char *data, UINT32 len);
void key_pan_display_channel(UINT16 channel);
void key_pan_display_lock(UINT8 sw);
void key_pan_set_power_mode(UINT8 mode);
void key_pan_set_key_table(IR_Key_Map_t *key_tab);
#endif //_KEY_H_

