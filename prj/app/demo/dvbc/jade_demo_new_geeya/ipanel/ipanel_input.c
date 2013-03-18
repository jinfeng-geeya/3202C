/**************************************************************
 * Copyright 2004 by Embedded Internet Solutions, Inc.
 * All rights reserved. This is a input implenment demo file
 *
 * notified by sharp 2004/10/19
 **************************************************************/

#include <udi/ipanel/ipanel_porting_event.h>
#include <stdio.h>
#include <hld/pan/pan.h>
#include <hld/pan/pan_key.h>

#define dprintf ipanel_porting_dprintf
//#define time_ms ipanel_porting_time_ms

#if (SYS_IRP_MOUDLE == ALI25C00)

#define IR_KEY_NUM1      0x807f22dd
#define IR_KEY_NUM2      0x807FA25D
#define IR_KEY_NUM3      0x807F629D
#define IR_KEY_NUM4      0x807FE21D
#define IR_KEY_NUM5      0x807F12ED
#define IR_KEY_NUM6      0x807F926D
#define IR_KEY_NUM7      0x807F52AD
#define IR_KEY_NUM8      0x807FD22D
#define IR_KEY_NUM9      0x807F32CD
#define IR_KEY_NUM0      0x807FB24D

#define IR_KEY_LEFT      0x807F9867
#define IR_KEY_RIGHT     0x807F58A7
#define IR_KEY_UP        0x807F18E7
#define IR_KEY_DOWN      0x807FD827
#define IR_KEY_OK        0x807FF20D
#define IR_KEY_RETURN    0x807F728D
#define IR_KEY_RED       0x807F02FD
#define IR_KEY_GREEN     0x807F827D
#define IR_KEY_YELLOW    0x807F42BD
#define IR_KEY_BLUE      0x807FC23D

#define IR_KEY_CHANNEL_UP       0x807FA857
#define IR_KEY_CHANNEL_DOWN     0x807FE817
#define IR_KEY_VOLUME_UP        0x807F28D7
#define IR_KEY_VOLUME_DOWN      0x807F6897
#define IR_KEY_RECORD           0x807F38C7
#define IR_KEY_PAUSE_STEP       0x807FB847
#define IR_KEY_PLAY             0x807F7887
#define IR_KEY_STOP             0x807FF807

#define IR_KEY_TV_STB           0x807F00FF
#define IR_KEY_TV_RADIO         0x807F807F
#define IR_KEY_AUDIO            0x807F40BF
#define IR_KEY_POWER            0x807FC03F
#define IR_KEY_INFO             0x807F20DF
#define IR_KEY_HELP             0x807FA05F
#define IR_KEY_TSHIFT           0x807F609F
#define IR_KEY_LANGUAGE         0x807FE01F
#define IR_KEY_FAV              0x807F10EF
#define IR_KEY_MP               0x807F906F
#define IR_KEY_SUBTITLE         0x807F50AF
#define IR_KEY_TIMER            0x807FD02F

#define IR_KEY_MUTE             0x807F30CF
#define IR_KEY_CHLIST           0x807FB04F
#define IR_KEY_TEXT             0x807F708F
#define IR_KEY_PAGEUP           0x807FF00F
#define IR_KEY_PAGEDOWN         0x807FC837
#define IR_KEY_EPG              0x807F08F7
#define IR_KEY_MENU             0x807F8877
#define IR_KEY_EXIT             0x807F48B7

//Panel physical key code
#define IPANEL_PAN_KEY_POWER	0xFFFF0002
#define IPANEL_PAN_KEY_MENU	0xFFFF0080
#define IPANEL_PAN_KEY_OK	        0xFFFF0008
#define IPANEL_PAN_KEY_EXIT		0xFFFF0020
#define IPANEL_PAN_KEY_UP		0xFFFF0040
#define IPANEL_PAN_KEY_DOWN	0xFFFF0010
#define IPANEL_PAN_KEY_LEFT		0xFFFF0004
#define IPANEL_PAN_KEY_RIGHT	0xFFFF0001

#elif (SYS_IRP_MOUDLE == ALI25C01)

#define IR_KEY_NUM1		0x60dfc837
#define IR_KEY_NUM2		0x60df08f7
#define IR_KEY_NUM3		0x60df8877
#define IR_KEY_NUM4		0x60dff00f
#define IR_KEY_NUM5		0x60df30cf
#define IR_KEY_NUM6		0x60dfb04f
#define IR_KEY_NUM7		0x60dfd02f
#define IR_KEY_NUM8		0x60df10ef
#define IR_KEY_NUM9		0x60df906f
#define IR_KEY_NUM0		0x60df926d

#define IR_KEY_LEFT		0x60df38c7
#define IR_KEY_RIGHT    	0x60df12ed
#define IR_KEY_UP		0x60df22dd
#define IR_KEY_DOWN		0x60dfb847

#define IR_KEY_OK		0x60df3ac5
#define IR_KEY_RETURN	0x60dfc03f

#define IR_KEY_RED		0x60df609f
#define IR_KEY_GREEN		0x60df7887
#define IR_KEY_YELLOW	0x60dff807
#define IR_KEY_BLUE		0x60dfba45

#define IR_KEY_CHANNEL_UP		0x60df20df
#define IR_KEY_CHANNEL_DOWN	0x60df0af5
#define IR_KEY_VOLUME_UP		0x60df12ed
#define IR_KEY_VOLUME_DOWN	0x60df38c7
#define IR_KEY_RECORD		0x60dfa857
#define IR_KEY_PAUSE_STEP	0x60df7a85
#define IR_KEY_PLAY             0x60df18e7
#define IR_KEY_STOP             0x60dfe817

#define IR_KEY_TV_STB	0x60df52ad
#define IR_KEY_TV_RADIO	0x60df02fd
#define IR_KEY_AUDIO		0x60df629d
#define IR_KEY_POWER	0x60df708f
#define IR_KEY_INFO		0x60df6897
#define IR_KEY_HELP		0xFFFFFFFF
#define IR_KEY_TSHIFT	0xFFFFFFFF
#define IR_KEY_LANGUAGE	0xFFFFFFFF
#define IR_KEY_FAV		0x60dfc23d
#define IR_KEY_MP		0x60df50af
#define IR_KEY_SUBTITLE	0x60df807f
#define IR_KEY_TIMER		0xFFFFFFFF

#define IR_KEY_MUTE		0x60dfa05f
#define IR_KEY_CHLIST	0xFFFFFFFF
#define IR_KEY_TEXT		0x60df827d
#define IR_KEY_PAGEUP		0x60dfd22d
#define IR_KEY_PAGEDOWN	0x60dfe01f
#define IR_KEY_EPG              0x60df00ff
#define IR_KEY_MENU           0x60df2ad5
#define IR_KEY_EXIT             0x60df42bd

//Panel physical key code
#define IPANEL_PAN_KEY_POWER	0xFFFF0002
#define IPANEL_PAN_KEY_MENU	0xFFFF0080
#define IPANEL_PAN_KEY_OK	        0xFFFF0008
#define IPANEL_PAN_KEY_EXIT		0xFFFF0020
#define IPANEL_PAN_KEY_UP		0xFFFF0040
#define IPANEL_PAN_KEY_DOWN	0xFFFF0010
#define IPANEL_PAN_KEY_LEFT		0xFFFF0004
#define IPANEL_PAN_KEY_RIGHT	0xFFFF0001

#endif

extern struct pan_device *g_pan_dev;

int eis_input_get(unsigned int event[3])
{
	unsigned int key;

	struct pan_key *prcv_key = NULL;

	event[0] = 0;
	event[1] = 0;
	event[2] = 0;

	//rcv_key.code = PAN_KEY_INVALID;

	prcv_key = pan_get_key(g_pan_dev, 5);
	if (NULL != prcv_key)
	{
		if (prcv_key->state == 0x1)
		//完整判断加上prcv_key->count==0;不等于0时，是同一键的连续按键
			event[0] = EIS_EVENT_TYPE_KEYDOWN;
		else if (prcv_key->state == 0x0)
			event[0] = EIS_EVENT_TYPE_KEYUP;

		if (event[0] == EIS_EVENT_TYPE_KEYUP)
			return 0;

		switch (prcv_key->code)
		{
			case IR_KEY_NUM0:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_NUM0;
				event[2] = 0;
				dprintf("key NUM0\n");
				return 1;

			case IR_KEY_NUM1:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_NUM1;
				event[2] = 0;
				dprintf("key NUM1\n");
				return 1;

			case IR_KEY_NUM2:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_NUM2;
				event[2] = 0;
				dprintf("key NUM2\n");
				return 1;

			case IR_KEY_NUM3:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_NUM3;
				event[2] = 0;
				dprintf("key NUM3\n");
				return 1;

			case IR_KEY_NUM4:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_NUM4;
				event[2] = 0;
				dprintf("key NUM4\n");
				return 1;

			case IR_KEY_NUM5:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_NUM5;
				event[2] = 0;
				dprintf("key NUM5\n");
				return 1;

			case IR_KEY_NUM6:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_NUM6;
				event[2] = 0;
				dprintf("key NUM6\n");
				return 1;

			case IR_KEY_NUM7:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_NUM7;
				event[2] = 0;
				dprintf("key NUM7\n");
				return 1;

			case IR_KEY_NUM8:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_NUM8;
				event[2] = 0;
				dprintf("key NUM8\n");
				return 1;

			case IR_KEY_NUM9:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_NUM9;
				event[2] = 0;
				dprintf("key NUM9\n");
				return 1;

			case IR_KEY_UP:
                        case IPANEL_PAN_KEY_UP:
				//event[0]=EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_UP;
				event[2] = 0;
				dprintf("key UP\n");
				return 1;

			case IR_KEY_DOWN:
                        case IPANEL_PAN_KEY_DOWN:
				//event[0]=EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_DOWN;
				event[2] = 0;
				dprintf("key DOWN\n");
				return 1;

			case IR_KEY_RIGHT:
                        case IPANEL_PAN_KEY_RIGHT:
				//event[0]=EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_RIGHT;
				event[2] = 0;
				dprintf("key RIGHT\n");
				return 1;

			case IR_KEY_LEFT:
                        case IPANEL_PAN_KEY_LEFT:
				//event[0]=EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_LEFT;
				event[2] = 0;
				dprintf("key LEFT\n");
				return 1;

			case IR_KEY_OK:
                        case IPANEL_PAN_KEY_OK:
				//event[0]=EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_SELECT;
				event[2] = 0;
				dprintf("key OK\n");
				return 1;

			case IR_KEY_RETURN:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_BACK;
				event[2] = 0;
				dprintf("key BACK\n");
				return 1;

			case IR_KEY_RED:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_RED;
				event[2] = 0;
				dprintf("key RED\n");
				return 1;

			case IR_KEY_GREEN:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_GREEN;
				event[2] = 0;
				dprintf("key GREEN\n");
				return 1;

			case IR_KEY_YELLOW:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_YELLOW;
				event[2] = 0;
				dprintf("key YELLOW\n");
				return 1;

			case IR_KEY_BLUE:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_BLUE;
				event[2] = 0;
				dprintf("key BLUE\n");
				return 1;
				
			case IR_KEY_MENU:
                        case IPANEL_PAN_KEY_MENU:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_MENU;
				event[2] = 0;
				dprintf("key MENU\n");
				return 1;

			case IR_KEY_EXIT:
                        case IPANEL_PAN_KEY_EXIT:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_EXIT;
				event[2] = 0;
				dprintf("key EXIT\n");
				return 1;

			case IR_KEY_PAGEUP:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_PAGE_UP;
				event[2] = 0;
				dprintf("key PAGE_UP\n");
				return 1;

			case IR_KEY_PAGEDOWN:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_PAGE_DOWN;
				event[2] = 0;
				dprintf("key PAGE_DOWN\n");
				return 1;

			case IR_KEY_MUTE:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_VOLUME_MUTE;
				event[2] = 0;
				dprintf("key MUTE\n");
				return 1;

			case IR_KEY_INFO:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_INFO;
				event[2] = 0;
				dprintf("key INFO\n");
				return 1;
			
#if (SYS_IRP_MOUDLE == ALI25C00)
			case IR_KEY_AUDIO:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_AUDIO;
				event[2] = 0;
				dprintf("key AUDIO\n");
				return 1;

			case IR_KEY_CHANNEL_UP:
				//event[0]=EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_CHANNEL_UP;
				event[2] = 0;
				dprintf("key CHANNEL_UP\n");
				return 1;

			case IR_KEY_CHANNEL_DOWN:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_CHANNEL_DOWN;
				event[2] = 0;
				dprintf("key CHANNEL_DOWN\n");
				return 1;

			case IR_KEY_VOLUME_UP:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_VOLUME_UP;
				event[2] = 0;
				dprintf("key VOLUME_UP\n");
				return 1;

			case IR_KEY_VOLUME_DOWN:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_VOLUME_DOWN;
				event[2] = 0;
				dprintf("key VOLUME_DOWN\n");
				return 1;

			case IR_KEY_PAUSE_STEP:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_PAUSE;
				event[2] = 0;
				dprintf("key PAUSE\n");
				return 1;

			case IR_KEY_PLAY:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_PLAY;
				event[2] = 0;
				dprintf("key PLAY\n");
				return 1;

			case IR_KEY_STOP:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_STOP;
				event[2] = 0;
				dprintf("key STOP\n");
				return 1;

			case IR_KEY_POWER:
                        case IPANEL_PAN_KEY_POWER:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_POWER;
				event[2] = 0;
				dprintf("key POWER\n");
				return 1;

			case IR_KEY_HELP:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_HELP;
				event[2] = 0;
				dprintf("key HELP\n");
				return 1;

			case IR_KEY_SUBTITLE:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_SUBTITLE;
				event[2] = 0;
				dprintf("key SUBTITLE\n");
				return 1;

			case IR_KEY_LANGUAGE:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_LANGUAGE;
				event[2] = 0;
				dprintf("key LANGUAGE\n");
				return 1;

			case IR_KEY_FAV:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_FAVORITE; //EIS_IRKEY_NULL;
				event[2] = 0;
				dprintf("key FAVORITE\n");
				return 1;

			case IR_KEY_TEXT:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_IME;
				event[2] = 0;
				dprintf("key IME\n");
				return 1;

			case IR_KEY_TV_STB:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_REFRESH; //EIS_IRKEY_NULL; //
				event[2] = 0;
				dprintf("key REFRESH\n");
				return 1;

				//don't know which to match ,so those match to NULL:
			case IR_KEY_EPG:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_NULL; //EIS_IRKEY_EPG;
				event[2] = 0;
				dprintf("key NULL\n");
				return 1;

			case IR_KEY_RECORD:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_NULL; //EIS_IRKEY_RECORD;
				event[2] = 0;
				dprintf("key NULL\n");
				return 1;

			case IR_KEY_TV_RADIO:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_NULL;
				event[2] = 0;
				dprintf("key NULL\n");
				return 1;

			case IR_KEY_TSHIFT:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_NULL;
				event[2] = 0;
				dprintf("key NULL\n");
				return 1;

			case IR_KEY_MP:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_NULL;
				event[2] = 0;
				dprintf("key NULL\n");
				return 1;

			case IR_KEY_TIMER:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_NULL;
				event[2] = 0;
				dprintf("key NULL\n");
				return 1;

			case IR_KEY_CHLIST:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_NULL;
				event[2] = 0;
				dprintf("key NULL\n");
				return 1;
#elif(SYS_IRP_MOUDLE == ALI25C01)
			case IR_KEY_AUDIO:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_AUDIO;
				event[2] = 0;
				dprintf("key AUDIO\n");
				return 1;

			case IR_KEY_CHANNEL_UP:
				//event[0]=EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_CHANNEL_UP;
				event[2] = 0;
				dprintf("key CHANNEL_UP\n");
				return 1;

			case IR_KEY_CHANNEL_DOWN:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_CHANNEL_DOWN;
				event[2] = 0;
				dprintf("key CHANNEL_DOWN\n");
				return 1;

			case IR_KEY_PAUSE_STEP:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_PAUSE;
				event[2] = 0;
				dprintf("key PAUSE\n");
				return 1;

			case IR_KEY_PLAY:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_PLAY;
				event[2] = 0;
				dprintf("key PLAY\n");
				return 1;

			case IR_KEY_STOP:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_STOP;
				event[2] = 0;
				dprintf("key STOP\n");
				return 1;

			case IR_KEY_POWER:
                        case IPANEL_PAN_KEY_POWER:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_POWER;
				event[2] = 0;
				dprintf("key POWER\n");
				return 1;

			case IR_KEY_SUBTITLE:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_SUBTITLE;
				event[2] = 0;
				dprintf("key SUBTITLE\n");
				return 1;

			case IR_KEY_FAV:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_FAVORITE; //EIS_IRKEY_NULL;
				event[2] = 0;
				dprintf("key FAVORITE\n");
				return 1;

			case IR_KEY_TEXT:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_IME;
				event[2] = 0;
				dprintf("key IME\n");
				return 1;

			case IR_KEY_TV_STB:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_REFRESH; //EIS_IRKEY_NULL; //
				event[2] = 0;
				dprintf("key REFRESH\n");
				return 1;

				//don't know which to match ,so those match to NULL:
			case IR_KEY_EPG:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_NULL; //EIS_IRKEY_EPG;
				event[2] = 0;
				dprintf("key NULL\n");
				return 1;

			case IR_KEY_RECORD:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_NULL; //EIS_IRKEY_RECORD;
				event[2] = 0;
				dprintf("key NULL\n");
				return 1;

			case IR_KEY_TV_RADIO:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_NULL;
				event[2] = 0;
				dprintf("key NULL\n");
				return 1;

			case IR_KEY_MP:
				//event[0] = EIS_EVENT_TYPE_IRKEY;
				event[1] = EIS_IRKEY_NULL;
				event[2] = 0;
				dprintf("key NULL\n");
				return 1;
#endif
			default:
				dprintf("Unknown key\n");
				return 0;
		}
	}

	else
		return 0;
}
