#ifndef _OSD_VKEY_H_
#define _OSD_VKEY_H_

typedef enum
{
	V_KEY_0 = 0,
	V_KEY_1,
	V_KEY_2,
	V_KEY_3,
	V_KEY_4,
	V_KEY_5,
	V_KEY_6,
	V_KEY_7,
	V_KEY_8,
	V_KEY_9,

	V_KEY_LEFT,
	V_KEY_RIGHT,
	V_KEY_UP,
	V_KEY_DOWN,
	V_KEY_ENTER,

	V_KEY_MENU,
	V_KEY_RETURN,
	V_KEY_EXIT,
	
	V_KEY_POWER,
	
	//key cause play state changed
	V_KEY_MUTE,
	V_KEY_PAUSE,
	V_KEY_TVRADIO,
	V_KEY_RECALL,
	V_KEY_AUDIO,
	V_KEY_AUDIOCH,
	V_KEY_ZOOM,
	V_KEY_HELP,
	
	//key show informatin
	V_KEY_SIGNAL, //show signal
	V_KEY_INFOR,  //show channel bar
	V_KEY_PRO_INFOR,
	V_KEY_DVR_INFOR,
	V_KEY_TIMER,
	V_KEY_LANGUAGE,
	
	//extend up/down control key
	V_KEY_C_UP,//ch+
	V_KEY_C_DOWN,//ch-
	V_KEY_V_UP,//vol+
	V_KEY_V_DOWN,//vol-
	V_KEY_P_UP,//page+
	V_KEY_P_DOWN,//page-	
	V_KEY_F_UP,//fav+
	V_KEY_F_DOWN,//fav-
	
	//EPG,TTX,SUBTITLE keys
	V_KEY_EPG,
	V_KEY_TEXT,
	V_KEY_SUBTITLE,
	
	//Sat,Fav key
	V_KEY_FAV,
	V_KEY_SAT,
	
	//scart output control keys
	V_KEY_TVSAT,
	V_KEY_RGBCVBS,
	
	//sleep
	V_KEY_SLEEP,
	
	//find
	V_KEY_FIND,
	
	//show channel list, 9 pictures
	V_KEY_LIST,
	V_KEY_MP,
	
	//color keys
	V_KEY_RED,
	V_KEY_GREEN,
	V_KEY_YELLOW,
	V_KEY_BLUE,
	V_KEY_WHITE,
	
	//character input control keys
	V_KEY_CAPS,
	V_KEY_DEL,
	
	//DVR
	V_KEY_RECORD,
	V_KEY_STOP,
	V_KEY_TSHIFT,
	V_KEY_PLAY,
	V_KEY_SEARCH,
	V_KEY_PREV,
	V_KEY_NEXT,
	V_KEY_FB,
	V_KEY_FF,
	V_KEY_B_SLOW,
	V_KEY_STEP,
	V_KEY_SLOW,
	V_KEY_TVMODE,
	V_KEY_DVRLIST,
	V_KEY_TIMESHIFT,
	// HDTV
	V_KEY_VIDEO_FORMAT,

	//for PIP
	V_KEY_PIP,
	V_KEY_SWAP,
	V_KEY_MOVE,
	V_KEY_PIP_LIST,

	V_KEY_BOOKMARK,
	V_KEY_JUMPMARK,

	V_KEY_PVR_INFO,
	V_KEY_PVR_CFG, // DVR Configuration

	V_KEY_RED_RELEASE,
	V_KEY_GREEN_RELEASE,

	V_KEY_FILELIST,
	V_KEY_USBREMOVE,
	V_KEY_REPEATAB,
	V_KEY_SET,
	V_KEY_BACK_STEP,
	V_KEY_STATE,
	V_KEY_MAIL,
	V_KEY_VOD,
	/*add by jasing*/
	V_KEY_GAME,
	V_KEY_DATARADIO,/*Êý¾Ý¹ã²¥*/
	
	V_KEY_ASPECT, // 4:3PS, 4:3LB, 16:9

	V_KEY_MUSIC, // For ALi J00 project
	V_KEY_ALBUM, // For ALi J00 project

	V_KEY_CHADD,
	V_KEY_RATIO,
	V_KEY_DASH,
	V_KEY_CC,
	V_KEY_STOCK,
	V_KEY_NAVIGT,
	V_KEY_F3,
	V_KEY_F4,
	
	//Don't change, keep it last	
	V_KEY_OSDLAST,	
	V_KEY_NULL = 0xff,
	
}OSD_VKEY_t;

#endif//_OSD_VKEY_H_
