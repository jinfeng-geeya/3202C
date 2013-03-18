/*********************************************************************
    Copyright (c) 2005 Embedded Internet Solutions, Inc
    All rights reserved. You are not allowed to copy or distribute
    the code without permission.
    There are the Porting Events needed by iPanel MiddleWare. 
    
    Note: the "int" in the file is 32bits
    
    $ver0.0.0.1 $author Zouxianyun 2005/04/28
*********************************************************************/

#ifndef _IPANEL_MIDDLEWARE_PORTING_EVENT_H_
#define _IPANEL_MIDDLEWARE_PORTING_EVENT_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * messages same in Event[0] area.
 */
enum {
    EIS_EVENT_TYPE_TIMER	= 0,
    EIS_EVENT_TYPE_NETWORK	= 4,
    EIS_EVENT_TYPE_CHAR     = 5,
    EIS_EVENT_TYPE_MOUSE    = 6,
    EIS_EVENT_TYPE_IRKEY	= 7,
    EIS_EVENT_TYPE_DVB      = 11,
	EIS_EVENT_TYPE_MESSAGE  = 15,
    EIS_EVENT_TYPE_KEYDOWN  = 16,
    EIS_EVENT_TYPE_KEYUP    = 17,
    EIS_EVENT_TYPE_FILTER   = 0x100,
    EIS_EVENT_TYPE_AUDIO    = 0x101,
    EIS_EVENT_TYPE_HWDEVICE = 0x102,
    EIS_EVENT_TYPE_CA       = 0x103
};

/**
 * wpara messages same in Event[1] area, but effect only message = EIS_EVENT_TYPE_TIMER.
 */
enum {
	EIS_TIMER_TIMER = 0
};

/**
 * wpara messages same in Event[1] area, but effect only message = EIS_EVENT_TYPE_NETWORK.
 */
enum {
	SOCKET_EVENT_TYPE_CONNECT = 1,
	SOCKET_EVENT_TYPE_CLOSE,
	SOCKET_EVENT_TYPE_READ
};

/**
 * wpara messages same in Event[1] area (ASCII), but effect only message = EIS_EVENT_TYPE_CHAR.
 */

/**
 * wpara messages same in Event[1] area, but effect only message = EIS_EVENT_TYPE_MOUSE.
 */
enum {
    EIS_MOUSE_MOUSEMOVE,
    EIS_MOUSE_LBUTTONDOWN,
    EIS_MOUSE_LBUTTONUP,
    EIS_MOUSE_RBUTTONDOWN,
    EIS_MOUSE_RBUTTONUP
};
#define MAKE_MOUSE_POS(t,x,y)    (((x)&0xffff)<<16) | (((y)&0xffff)))
#define GET_MOUSE_XPOS(e)          ((e>>16)&0xffff)
#define GET_MOUSE_YPOS(e)          (e&0xffff)

/**
 * wpara messages same in Event[1] area, but effect only message = EIS_EVENT_TYPE_IRKEY.
 */
enum {
		EIS_IRKEY_NULL      = 0x0100/* bigger than largest ASCII*/,
    EIS_IRKEY_POWER = 0x101,
    EIS_IRKEY_NUM0  = 0x103,
    EIS_IRKEY_NUM1,
    EIS_IRKEY_NUM2,
    EIS_IRKEY_NUM3,
    EIS_IRKEY_NUM4,
    EIS_IRKEY_NUM5,
    EIS_IRKEY_NUM6,
    EIS_IRKEY_NUM7,
    EIS_IRKEY_NUM8,
    EIS_IRKEY_NUM9,
    EIS_IRKEY_UP,
    EIS_IRKEY_DOWN,
    EIS_IRKEY_LEFT,
    EIS_IRKEY_RIGHT,
    EIS_IRKEY_SELECT,
    EIS_IRKEY_LAST_CHANNEL = 0x114,
    EIS_IRKEY_VOLUME_UP,
    EIS_IRKEY_VOLUME_DOWN,
    EIS_IRKEY_VOLUME_MUTE,
    EIS_IRKEY_MENU,
    EIS_IRKEY_EXIT = 0x11A,
    EIS_IRKEY_BACK,
    EIS_IRKEY_HELP,
    EIS_IRKEY_PLAY = 0x11E,
    EIS_IRKEY_STOP,
    EIS_IRKEY_PAUSE,
    EIS_IRKEY_REC,
    EIS_IRKEY_FASTFORWARD,
    EIS_IRKEY_REWIND,
    EIS_IRKEY_HOMEPAGE = 0x126,
    EIS_IRKEY_REFRESH,
    EIS_IRKEY_PAGE_UP = 0x12D,
    EIS_IRKEY_PAGE_DOWN,
    EIS_IRKEY_VK_F1 = 0x135,
    EIS_IRKEY_VK_F2,
    EIS_IRKEY_VK_F3,
    EIS_IRKEY_VK_F4,
    EIS_IRKEY_VK_F5,
    EIS_IRKEY_VK_F6,
    EIS_IRKEY_VK_F7,
    EIS_IRKEY_VK_F8,
    EIS_IRKEY_VK_F9,
    EIS_IRKEY_VK_F10,
    EIS_IRKEY_VK_F11,
    EIS_IRKEY_VK_F12,

	EIS_IRKEY_ESC = 0x206,
	EIS_IRKEY_INFO = 0x20c,
	EIS_IRKEY_PROGRAM_LIST,		

    EIS_IRKEY_CHANNEL_UP = 0x30A,
    EIS_IRKEY_CHANNEL_DOWN,

    EIS_IRKEY_RED = 0x402,
    EIS_IRKEY_YELLOW,
    EIS_IRKEY_GREEN,
    EIS_IRKEY_BLUE,
	EIS_IRKEY_PAGE_FORWARD,
	EIS_IRKEY_PAGE_BACKWARD,
	EIS_IRKEY_GOTO,
	EIS_IRKEY_CANCEL,
	EIS_IRKEY_AUDIO,
	EIS_IRKEY_NONAME1,
	EIS_IRKEY_A_B,
	EIS_IRKEY_DVDMENU,
	EIS_IRKEY_TITILE,	
	EIS_IRKEY_SOUNDMODE,
	EIS_IRKEY_SUBTITLE,
	EIS_IRKEY_ANGLE,
	EIS_IRKEY_ZOOM,
	EIS_IRKEY_SLOW,
	EIS_IRKEY_TVSYSTEM,
	EIS_IRKEY_EJECT,
	EIS_IRKEY_IME,
	EIS_IRKEY_RECOMMEND,			//ÍÆ¼ö¼ü
	EIS_IRKEY_VOLUMNE_ACCOMPANY,	//°éÒô¼ü 
	EIS_IRKEY_FORETELL,				//Ô¤¸æ¼ü	
	EIS_IRKEY_FAVORITE,				//ÊÕ²Ø¼ü
	EIS_IRKEY_SWITCH,				//ÇÐ»»¼ü
	EIS_IRKEY_FUN1,
	EIS_IRKEY_FUN2,  
	EIS_IRKEY_FUN3,
	EIS_IRKEY_FUN4,
	EIS_IRKEY_LANGUAGE			// ÖÐ/Ó¢
};

/**
 * wpara messages same in Event[1] area, but effect only message = EIS_EVENT_TYPE_DVB.
 */
enum {
	EIS_DVB_DELIVERY_TUNE_SUCCESS = 0x500,
	EIS_DVB_DELIVERY_TUNE_FAILED,
	EIS_DVB_PROGRAM_TUNE_SUCCESS,
	EIS_DVB_PROGRAM_TUNE_FAILED,
	EIS_DVB_SINGAL_QUALITY,
	EIS_DVB_SINGAL_STRENGTH,
	EIS_DVB_SELECT_AV_SUCCESS     = 0x600,
	EIS_DVB_SELECT_AV_FAILED
};

/**
 * wpara messages same in Event[1] area, but effect only message = EIS_EVENT_TYPE_MESSAGE.
 * This is the struction of p1message. and wpara message is the address of the struction.
 * Note: you must post this message directly, not thought a queque.
 */
typedef int (* p1messageCbf)(int id, int action, unsigned int p1, unsigned int p2);
typedef struct{
    int id;
    int type;
    int priority;
	p1messageCbf cbf; 
	unsigned long start_time; /* the seconds from 1970-1-1 00:00:00 */
    unsigned long duration;
	
  	unsigned int *pos; /*display position, if null is default position, other is a array of x,y,w and h*/
	int modifier;
    int button_lan;
    int msg_len;
    char *msg_text;
    struct Adapt *adapt_field;

	unsigned int running_status;
	void *tmpData;
} p1message;

enum p1message_Type
{
	MESSAGE_TYPE_INSTANT = 1,
	MESSAGE_TYPE_ALERT   = 2,
	MESSAGE_TYPE_CONFIRM = 3,
	MESSAGE_TYPE_PROMPT  = 4,
	MESSAGE_TYPE_INPUT   = 5,
	MESSAGE_TYPE_SELECT  = 6,
	MESSAGE_TYPE_RADIO   = 7,
	MESSAGE_TYPE_CHECK   = 8,
	MESSAGE_TYPE_STRING  = 9,
	MESSAGE_TYPE_MOVESTRING = 10
};

enum p1message_Priority
{
	PRIORITY_NONE                        = 0,		/*     0 00 00     */
	PRIORITY_TOP_WAIT_CURR               = 1,		/*     0 00 01     */
	PRIORITY_TOP_PAUSE_CURR              = 5,		/*	   0 01 01     */
	PRIORITY_TOP_CANCEL_CURR             = 9,		/*     0 10 01     */
	PRIORITY_TOP_WAIT_CURR_RESET_OTHER   = 17,		/*     1 00 01     */
	PRIORITY_TOP_PAUSE_CURR_RESET_OTHER  = 21,		/*     1 01 01     */
	PRIORITY_TOP_CANCEL_CURR_RESET_OTHER = 25,		/*     1 10 01     */
	PRIORITY_NO_CANCEL_CURR_RESET_OTHER  = 26		/*     1 10 10     */
};

enum p1message_Button_language
{
	BUTTON_LAN_CHS  = 0,
	BUTTON_LAN_CHT  = 1,
	BUTTON_LAN_ENG  = 2
};

typedef struct Adapt{
	int num_options;
	char **names;
	char **values;
} p1message_Adapt_field_when_type_is_6;


enum p1message_Cbf_parameter_action
{
	ACTION_DISCARD = 1,
	ACTION_DOMODAL = 2
};

enum p1message_Cbf_parameter_p1
{
	BUTTON_NONE   = 0x00,
	BUTTON_OK     = 0x01,
	BUTTON_CANCEL = 0x02,
	BUTTON_YES    = 0x04,
	BUTTON_NO     = 0x08
};


/**
 * wpara messages same in Event[1] area, but effect only message = EIS_EVENT_TYPE_FILTER.
 */
enum {
	EIS_FILTER_READ = 1,
	EIS_FILTER_FULL = 2
};

/**
 * wpara messages same in Event[1] area, but effect only message = EIS_EVENT_TYPE_AUDIO.
 */
enum {
	EIS_AUDIO_PLAYBUF_COMPLETE = 1
};

#ifdef __cplusplus
}
#endif

#endif

