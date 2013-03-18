#ifndef _KEY_H_
#define _KEY_H_

#include <types.h>

#define INVALID_HK		0xFFFFFFFF
#define INVALID_MSG		0x00000000

typedef enum
{
	//key
	CTRL_MSG_SUBTYPE_KEY = 0,
	CTRL_MSG_SUBTYPE_KEY_UI,
	CTRL_MSG_TYPE_KEY = 2,
 }BootMsgType_t;

typedef struct
{
	BootMsgType_t	msgType;
	UINT32				msgCode;
	UINT8			priority;
}BootMsg_t, *pBootMsg_t;

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

struct ir_key_map_t
{
//BYTE0							BYTE1					BYTE2			BYTE3
//PAN_KEY_TYPE_REMOTE  	PAN_KEY_PRESSED 	press_cont	key
	UINT32			key_info;
	UINT32			ui_vkey;
};  

#define  PANEL_TYPE_NORMAL    0
#define  PANEL_TYPE_M51        1

#define IRP_NUM			    3

/* ALI25C00 key code, 48 keys */
#define IR_ALI00_KEY_ADDR		0x807f0000
#define IR_ALI00_KEY_MASK		0xffff0000
#define IRP_ALI00_PROTOCOL		IRP_NEC
#define IRP_ALI00_TYPE			0

#define IR_ALI00_HKEY_0			0x807fb24d
#define IR_ALI00_HKEY_1			0x807f22dd
#define IR_ALI00_HKEY_2			0x807fa25d
#define IR_ALI00_HKEY_3			0x807f629d
#define IR_ALI00_HKEY_4			0x807fe21d
#define IR_ALI00_HKEY_5			0x807f12ed
#define IR_ALI00_HKEY_6			0x807f926d
#define IR_ALI00_HKEY_7			0x807f52ad
#define IR_ALI00_HKEY_8			0x807fd22d
#define IR_ALI00_HKEY_9			0x807f32cd

#define IR_ALI00_HKEY_LEFT		0x807f9867
#define IR_ALI00_HKEY_RIGHT	0x807f58a7
#define IR_ALI00_HKEY_UP		0x807f18e7
#define IR_ALI00_HKEY_DOWN	0x807fd827

#define IR_ALI00_HKEY_V_UP		0x807f28d7
#define IR_ALI00_HKEY_V_DOWN	0x807f6897

#define IR_ALI00_HKEY_C_UP		0x807fa857
#define IR_ALI00_HKEY_C_DOWN	0x807fe817

#define IR_ALI00_HKEY_MP		0x807f906f
#define IR_ALI00_HKEY_ENTER	0x807ff20d
#define IR_ALI00_HKEY_P_UP		0x807fc837
#define IR_ALI00_HKEY_P_DOWN	0x807ff00f

#define IR_ALI00_HKEY_TEXT		0x807fc837
#define IR_ALI00_HKEY_POWER	0x807fc03f
#define IR_ALI00_HKEY_PREV		0x807f6897
#define IR_ALI00_HKEY_NEXT		0x807fe817

#define IR_ALI00_HKEY_AUDIO	0x807f40bf
#define IR_ALI00_HKEY_TIMER	0x807fd02f
#define IR_ALI00_HKEY_SUBTITLE	0x807f50af
#define IR_ALI00_HKEY_LIST		0x807fb04f
#define IR_ALI00_HKEY_NEWS		HKEY_NULL
#define IR_ALI00_HKEY_VOD		HKEY_NULL
#define IR_ALI00_HKEY_STOCK	HKEY_NULL

#define IR_ALI00_HKEY_SLEEP	0x807ff00f
#define IR_ALI00_HKEY_FIND		0x807f609f
#define IR_ALI00_HKEY_MUTE		0x807f30cf
#define IR_ALI00_HKEY_PAUSE	0x807fb847
#define IR_ALI00_HKEY_HELP		0x807fa05f
#define IR_ALI00_HKEY_INFOR	0x807f20df
#define IR_ALI00_HKEY_EXIT		0x807f48b7
#define IR_ALI00_HKEY_TVSAT	0x807f00ff
#define IR_ALI00_HKEY_TVRADIO	0x807f807f
#define IR_ALI00_HKEY_FAV		0x807f10ef
#define IR_ALI00_HKEY_LANG		0x807fe01f
#define IR_ALI00_HKEY_ZOOM		0x807f708f
#define IR_ALI00_HKEY_EPG		0x807f08f7
#define IR_ALI00_HKEY_MENU		0x807f8877
#define IR_ALI00_HKEY_RECALL	0x807f728d
                                      
#define IR_ALI00_HKEY_RED		0x807f02fd
#define IR_ALI00_HKEY_GREEN	0x807f827d
#define IR_ALI00_HKEY_YELLOW	0x807f42bd
#define IR_ALI00_HKEY_BLUE		0x807fc23d

#ifdef USB_MP_SUPPORT
#define IR_ALI00_HKEY_PLAY		0x807f7887
#define IR_ALI00_HKEY_STOP		0x807ff807	
#endif

/* ALI25C01 key code, 60keys */
#define IR_ALI01_KEY_ADDR		0x60df0000
#define IR_ALI01_KEY_MASK		0xffff0000
#define IRP_ALI01_PROTOCOL		IRP_NEC
#define IRP_ALI01_TYPE			1

#define IR_ALI01_HKEY_0			0x60df926d
#define IR_ALI01_HKEY_1			0x60dfc837
#define IR_ALI01_HKEY_2			0x60df08f7
#define IR_ALI01_HKEY_3			0x60df8877
#define IR_ALI01_HKEY_4			0x60dff00f
#define IR_ALI01_HKEY_5			0x60df30cf
#define IR_ALI01_HKEY_6			0x60dfb04f
#define IR_ALI01_HKEY_7			0x60dfd02f
#define IR_ALI01_HKEY_8			0x60df10ef
#define IR_ALI01_HKEY_9			0x60df906f

#define IR_ALI01_HKEY_LEFT		0x60df38c7
#define IR_ALI01_HKEY_RIGHT	0x60df12ed
#define IR_ALI01_HKEY_UP		0x60df22dd
#define IR_ALI01_HKEY_DOWN	0x60dfb847
//no vulumn key, use left, right
#define IR_ALI01_HKEY_V_UP		0x60df12ed
#define IR_ALI01_HKEY_V_DOWN	0x60df38c7
//no channel key, use pre ,next
#define IR_ALI01_HKEY_C_UP		0x60df20df
#define IR_ALI01_HKEY_C_DOWN	0x60df0af5

#define IR_ALI01_HKEY_MP		0x60df50af
#define IR_ALI01_HKEY_ENTER	0x60df3ac5
#define IR_ALI01_HKEY_P_UP		0x60dfd22d
#define IR_ALI01_HKEY_P_DOWN	0x60dfe01f

#define IR_ALI01_HKEY_TEXT		0x60df827d
#define IR_ALI01_HKEY_POWER	0x60df708f
#define IR_ALI01_HKEY_PREV		0x60df20df
#define IR_ALI01_HKEY_NEXT		0x60df0af5

#define IR_ALI01_HKEY_AUDIO	0x60df629d
#define IR_ALI01_HKEY_TIMER	HKEY_NULL
#define IR_ALI01_HKEY_SUBTITLE	0x60df807f
#define IR_ALI01_HKEY_LIST		HKEY_NULL
#define IR_ALI01_HKEY_NEWS		HKEY_NULL
#define IR_ALI01_HKEY_VOD		HKEY_NULL
#define IR_ALI01_HKEY_STOCK	HKEY_NULL

#define IR_ALI01_HKEY_SLEEP	0x60df9867
#define IR_ALI01_HKEY_FIND		0x60dfe21d
#define IR_ALI01_HKEY_MUTE		0x60dfa05f
#define IR_ALI01_HKEY_PAUSE	0x60df7a85
#define IR_ALI01_HKEY_HELP		HKEY_NULL
#define IR_ALI01_HKEY_INFOR	0x60df6897
#define IR_ALI01_HKEY_EXIT		0x60df42bd
#define IR_ALI01_HKEY_TVSAT	0x60df52ad
#define IR_ALI01_HKEY_TVRADIO	0x60df02fd
#define IR_ALI01_HKEY_FAV		0x60dfc23d
#define IR_ALI01_HKEY_LANG		HKEY_NULL
#define IR_ALI01_HKEY_ZOOM		0x60df40bf
#define IR_ALI01_HKEY_EPG		0x60df00ff
#define IR_ALI01_HKEY_MENU		0x60df2ad5
#define IR_ALI01_HKEY_RECALL	0x60dfc03f
                                      
#define IR_ALI01_HKEY_RED		0x60df609f
#define IR_ALI01_HKEY_GREEN	0x60df7887
#define IR_ALI01_HKEY_YELLOW	0x60dff807
#define IR_ALI01_HKEY_BLUE		0x60dfba45

#define IR_ALI01_HKEY_V_FORMAT 0x60dfa25d
#define IR_ALI01_HKEY_FAV_PUP  0x60df28d7
#define IR_ALI01_HKEY_FAV_PDOWN  0x60df48b7
#ifdef USB_MP_SUPPORT
#define IR_ALI01_HKEY_PLAY		0x60df18e7
#define IR_ALI01_HKEY_STOP		0x60dfe817	
#endif
#define IR_ALI01_HKEY_RECORD        0x60dfa857
#define IR_ALI01_HKEY_PVR_INFO		0x60dfca35	
#define IR_ALI01_HKEY_FILELIST		0x60dfb24d	
#define IR_ALI01_HKEY_DVRLIST		0x60df8a75	
#define IR_ALI01_HKEY_USBREMOVE	    0x60df1ae5	
#define IR_ALI01_HKEY_PIP			0x60df6a95	
#define IR_ALI01_HKEY_PIP_LIST		0x60df9a65	
#define IR_ALI01_HKEY_SWAP			0x60df5aa5	
#define IR_ALI01_HKEY_MOVE			0x60dfda25	
#define IR_ALI01_HKEY_REPEATAB		0x60dfea15	
#define IR_ALI01_HKEY_FB			0x60df58a7	
#define IR_ALI01_HKEY_FF		    0x60dfd827	
#define IR_ALI01_HKEY_B_SLOW  0x60df4ab5


/* front panel key code, 8 keys */
#define PAN_ALI_HKEY_POWER	0xFFFF0001//0xFFFF0002
#define PAN_ALI_HKEY_MENU		0xFFFF0002//0xFFFF0080
#define PAN_ALI_HKEY_ENTER	0xFFFF0004//0xFFFF0008
#define PAN_ALI_HKEY_EXIT		0xFFFFFFFF//0xFFFF0020
#define PAN_ALI_HKEY_UP		0xFFFF0010//0xFFFF0040
#define PAN_ALI_HKEY_DOWN		0xFFFF0020//0xFFFF0010
#define PAN_ALI_HKEY_LEFT		0xFFFF0040//0xFFFF0004
#define PAN_ALI_HKEY_RIGHT	0xFFFF0080//0xFFFF0001

UINT8 key_Init();

void key_pan_display(char *data, UINT32 len);
void key_pan_set_power_mode(UINT8 mode);
void key_pan_set_key_table(IR_Key_Map_t* key_tab,UINT32 tab_cnt);
#endif//_INPUT_H_

