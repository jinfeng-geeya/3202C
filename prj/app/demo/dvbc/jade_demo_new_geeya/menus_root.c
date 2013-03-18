#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
//#include <api/libosd/osd_lib.h>
#include "win_com.h"
#include "menus_root.h"



rootmenuhandle_t rootwinpool_1[] /*__attribute__((section(".flash")))*/ =
{
        {CTRL_MSG_SUBTYPE_KEY,   V_KEY_MENU,	FALSE,  ( POBJECT_HEAD ) &g_win_mainmenu},
        {CTRL_MSG_SUBTYPE_KEY,   V_KEY_UP,		FALSE,   ( POBJECT_HEAD ) &g_win_progname},
        {CTRL_MSG_SUBTYPE_KEY,   V_KEY_DOWN,	FALSE,   ( POBJECT_HEAD ) &g_win_progname},
        {CTRL_MSG_SUBTYPE_KEY,   V_KEY_C_UP,	FALSE,   ( POBJECT_HEAD ) &g_win_progname},
        {CTRL_MSG_SUBTYPE_KEY,   V_KEY_C_DOWN,	FALSE,   ( POBJECT_HEAD ) &g_win_progname},
        {CTRL_MSG_SUBTYPE_KEY,   V_KEY_P_UP,	FALSE,   ( POBJECT_HEAD ) &g_win_progname},
        {CTRL_MSG_SUBTYPE_KEY,   V_KEY_P_DOWN,	FALSE,   ( POBJECT_HEAD ) &g_win_progname},
        {CTRL_MSG_SUBTYPE_KEY,   V_KEY_F_UP,	FALSE,   ( POBJECT_HEAD ) &g_win_progname},
        {CTRL_MSG_SUBTYPE_KEY,   V_KEY_F_DOWN,	FALSE,   ( POBJECT_HEAD ) &g_win_progname},
        {CTRL_MSG_SUBTYPE_KEY,   V_KEY_TVRADIO,	FALSE,   ( POBJECT_HEAD ) &g_win_progname},
        {CTRL_MSG_SUBTYPE_KEY,   V_KEY_INFOR,	FALSE,  ( POBJECT_HEAD ) &g_win_progname},
        {CTRL_MSG_SUBTYPE_KEY,   V_KEY_RECALL,	FALSE,  ( POBJECT_HEAD ) &g_win_progname},
		{CTRL_MSG_SUBTYPE_KEY,   V_KEY_FIND,	FALSE,   (POBJECT_HEAD)&g_win_reserve},
        {CTRL_MSG_SUBTYPE_KEY,   V_KEY_LEFT,	TRUE,   ( POBJECT_HEAD ) &g_win_volume},
        {CTRL_MSG_SUBTYPE_KEY,   V_KEY_RIGHT,	TRUE,   ( POBJECT_HEAD ) &g_win_volume},
        {CTRL_MSG_SUBTYPE_KEY,   V_KEY_V_DOWN,	TRUE,   ( POBJECT_HEAD ) &g_win_volume},
        {CTRL_MSG_SUBTYPE_KEY,   V_KEY_V_UP,	TRUE,   ( POBJECT_HEAD ) &g_win_volume},
        {CTRL_MSG_SUBTYPE_KEY,   V_KEY_AUDIO,	FALSE,   ( POBJECT_HEAD ) &g_win_audio},
        {CTRL_MSG_SUBTYPE_KEY,   V_KEY_ENTER,	FALSE,   ( POBJECT_HEAD ) &g_win_chanlist/*g_win_light_chanlist*/},
        {CTRL_MSG_SUBTYPE_KEY,   V_KEY_EPG,		FALSE,   ( POBJECT_HEAD ) &g_win_epg},
		{CTRL_MSG_SUBTYPE_KEY,   V_KEY_FAV,		FALSE,   ( POBJECT_HEAD ) &g_win_chanlist/*g_win_light_chanlist*/},
#if ((SUBTITLE_ON == 1)||(TTX_ON == 1)) 
		{CTRL_MSG_SUBTYPE_KEY,   V_KEY_SUBTITLE, FALSE,  ( POBJECT_HEAD ) &g_win_subtitle},
#endif



	//        {CTRL_MSG_SUBTYPE_KEY,   V_KEY_0,		TRUE,   ( POBJECT_HEAD ) &g_win_proginput},
        {CTRL_MSG_SUBTYPE_KEY,   V_KEY_1,		TRUE,   ( POBJECT_HEAD ) &g_win_proginput},
        {CTRL_MSG_SUBTYPE_KEY,   V_KEY_2,		TRUE,   ( POBJECT_HEAD ) &g_win_proginput},
        {CTRL_MSG_SUBTYPE_KEY,   V_KEY_3,		TRUE,   ( POBJECT_HEAD ) &g_win_proginput},
        {CTRL_MSG_SUBTYPE_KEY,   V_KEY_4,		TRUE,   ( POBJECT_HEAD ) &g_win_proginput},
        {CTRL_MSG_SUBTYPE_KEY,   V_KEY_5,		TRUE,   ( POBJECT_HEAD ) &g_win_proginput},
        {CTRL_MSG_SUBTYPE_KEY,   V_KEY_6,		TRUE,   ( POBJECT_HEAD ) &g_win_proginput},
        {CTRL_MSG_SUBTYPE_KEY,   V_KEY_7,		TRUE,   ( POBJECT_HEAD ) &g_win_proginput},
        {CTRL_MSG_SUBTYPE_KEY,   V_KEY_8,		TRUE,   ( POBJECT_HEAD ) &g_win_proginput},
        {CTRL_MSG_SUBTYPE_KEY,   V_KEY_9,		TRUE,   ( POBJECT_HEAD ) &g_win_proginput},
};


BOOL menus_find_root(UINT32 msgType, UINT32 msgCode, BOOL *bFlag, POBJECT_HEAD *winhandle)
{
	int i, n;
	rootmenuhandle_t *roothandle;
	UINT32 vkey;

	ap_hk_to_vk(0, msgCode, &vkey);

	n = sizeof(rootwinpool_1) / sizeof(rootwinpool_1[0]);
	for (i = 0; i < n; i++)
	{
		roothandle = &rootwinpool_1[i];
		if ( roothandle->msgType == msgType
		        && roothandle->msgCode == vkey )
		{
			*bFlag = roothandle->processMsg; //bflag;
			*winhandle = roothandle->root;
			return TRUE;
		}
	}

	*winhandle = NULL;

	return FALSE;
}



rootmenuhandle_t rootwinpool_2[] /*__attribute__((section(".flash")))*/ =
{
	{CTRL_MSG_SUBTYPE_KEY,   V_KEY_LEFT,	TRUE,   ( POBJECT_HEAD ) &g_win_volume},
	{CTRL_MSG_SUBTYPE_KEY,   V_KEY_RIGHT,	TRUE,   ( POBJECT_HEAD ) &g_win_volume},
	{CTRL_MSG_SUBTYPE_KEY,   V_KEY_V_DOWN,	TRUE,   ( POBJECT_HEAD ) &g_win_volume},
	{CTRL_MSG_SUBTYPE_KEY,   V_KEY_V_UP,	TRUE,   ( POBJECT_HEAD ) &g_win_volume},
	{CTRL_MSG_SUBTYPE_KEY,   V_KEY_AUDIO,	FALSE,   ( POBJECT_HEAD ) &g_win_audio},
#ifdef NVOD_FEATURE
	{CTRL_MSG_SUBTYPE_KEY,   V_KEY_EXIT,	FALSE,   ( POBJECT_HEAD ) &g_win_nvod_warn},
	{CTRL_MSG_SUBTYPE_KEY,   V_KEY_INFOR,	FALSE,   ( POBJECT_HEAD ) &g_win_nvod_event_name},
#endif
};



BOOL menus_find_root_nvodmode(UINT32 msgType, UINT32 msgCode, BOOL *bFlag, POBJECT_HEAD *winhandle)
{
	int i, n;
	rootmenuhandle_t *roothandle;
	UINT32 vkey;

	ap_hk_to_vk(0, msgCode, &vkey);

	n = sizeof(rootwinpool_2) / sizeof(rootwinpool_2[0]);
	for (i = 0; i < n; i++)
	{
		roothandle = &rootwinpool_2[i];
		if ( roothandle->msgType == msgType
		        && roothandle->msgCode == vkey )
		{
			*bFlag = roothandle->processMsg; //bflag;
			*winhandle = roothandle->root;
			return TRUE;
		}
	}

	*winhandle = NULL;

	return FALSE;
}

