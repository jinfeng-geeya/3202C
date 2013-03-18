#ifndef _MENUS_ROOT_H_
#define _MENUS_ROOT_H_

////////////////ad///////////
//#include "./ali_ad/ad_public.h"
////////////////////////////////
typedef struct
{
	UINT32 msgType;
	UINT32 msgCode;
	BOOL processMsg; // TRUE send msgType & msgCode to root when open
	POBJECT_HEAD root;
} rootmenuhandle_t;

BOOL menus_find_root(UINT32 msgType, UINT32 msgCode, BOOL *bFlag, POBJECT_HEAD *winhandle);
BOOL menus_find_root_nvodmode(UINT32 msgType, UINT32 msgCode, BOOL *bFlag, POBJECT_HEAD *winhandle);
extern CONTAINER g_win_signal0;
extern CONTAINER g_win_mainmenu;
extern CONTAINER g_win_chanedit;
extern CONTAINER g_win_reserve;
extern CONTAINER g_win_binding;
#ifndef JINGJIN
extern CONTAINER g_win_chan_detail;
extern CONTAINER g_win_chan_manage;
extern CONTAINER g_win_chanedit_action;
extern OBJLIST chanedit_ol;
extern CONTAINER g_win_advset;
extern CONTAINER g_win_displayset;
extern CONTAINER g_win_audioset;
extern CONTAINER g_win_factoryset;
extern CONTAINER g_win_freqset;
extern CONTAINER g_win_manual_update;
extern CONTAINER g_win_languageset;
extern CONTAINER g_win_childlock;
extern CONTAINER g_win_childlock_list;
extern CONTAINER g_win_pwdset_alert;
#endif
extern CONTAINER g_win_book_detail;
extern CONTAINER g_win_chanlist;
extern CONTAINER g_win_progname;
extern CONTAINER g_win_search;
extern CONTAINER g_win_sys_management;
extern CONTAINER g_win_light_chanlist;
extern CONTAINER g_win_chan_management;
extern CONTAINER g_win_prog_search;
extern CONTAINER g_win_sys_items;
extern CONTAINER g_win_sys_information;
extern CONTAINER g_win_condition_access;
extern CONTAINER g_win_games;
extern CONTAINER win_game_box;
extern CONTAINER g_win_common_item;
extern CONTAINER game_con;
extern CONTAINER g_win_pwd;
extern CONTAINER g_win_signal_test;
extern CONTAINER g_win_signal0;
extern CONTAINER g_ca_message;//  ----yuanlin
extern CONTAINER g_win_fullband_scan;
extern CONTAINER g_win_epg;
extern CONTAINER g_win_epg_detail;
extern CONTAINER g_win_main_freq;
extern CONTAINER g_win_manual_search;
extern CONTAINER g_win_factoryset_con;
extern CONTAINER g_win_channel_info;
extern CONTAINER g_win_stb_info;
extern CONTAINER g_win_volume;
extern CONTAINER g_win_audio;
extern CONTAINER g_win_proginput;    //yuanlin
//extern TEXT_FIELD g_win_proginput;
#ifdef USB_UPG_DVBC 
extern CONTAINER g_win_usbupg;
#endif

#ifdef NIM_REG_ENABLE
extern CONTAINER g_win_nimreg;
#endif

#ifdef REG_ENABLE
extern CONTAINER g_win_reg;
#endif

#ifdef NVOD_FEATURE
extern CONTAINER g_win_nvod;
extern CONTAINER g_win_nvod_warn;
extern CONTAINER g_win_nvod_event_name;
extern CONTAINER g_win_detail;
#endif

#if ((SUBTITLE_ON == 1)||(TTX_ON == 1)) 
extern CONTAINER	g_win_subtitle;
#endif


#ifdef SHOW_PICS
#if(SHOW_PICS == SHOW_MULTI_GIF)
extern CONTAINER g_win_show_multi_gifs;
#elif(SHOW_PICS == SHOW_SINGLE_PIC)
extern CONTAINER g_win_show_single_pic_ad;
#endif
#endif

#ifdef DATA_BROADCAST
#if(DATA_BROADCAST==DATA_BROADCAST_IPANEL)
    extern CONTAINER g_win_ipanel;
#elif(DATA_BROADCAST==DATA_BROADCAST_ENREACH)
    extern CONTAINER g_win_enreach;
#endif
#endif

#ifdef USB_MP_SUPPORT
extern CONTAINER g_win_usb_filelist;
extern CONTAINER g_win_imagepreview;
extern CONTAINER g_win_imageslide;
extern CONTAINER g_win_slidesetup;
extern CONTAINER g_win_usb_playlist;
#endif

#ifdef MULTI_CAS
#if (CAS_TYPE == CAS_TF || CAS_TYPE == CAS_CDCA)
extern CONTAINER g_win_pin_pwd;
extern CONTAINER g_win_maturity_rating;
extern CONTAINER g_win_work_time;
extern CONTAINER g_win_STBIDlst;
extern CONTAINER g_win_operator_info;
extern CONTAINER g_win_operator_message; //yuanlin
extern CONTAINER g_win_ca_set;       //yuanlin
extern CONTAINER g_win_service_entitle;
extern CONTAINER g_win_walletinfo;
extern CONTAINER g_win_IPPVinfo;
extern CONTAINER g_win_feedcard;
extern CONTAINER g_win_detitle;
extern CONTAINER g_win_mail;
extern CONTAINER g_win_mail_content;
extern CONTAINER g_win_book_detail;
extern CONTAINER g_win_smc_info;
extern CONTAINER g_win_ca_message;   //---yuanlin
extern CONTAINER g_win_ippv_popup;
extern CONTAINER g_win_eigenvalue_info;
extern CONTAINER g_win_smc_upgrade_info;
#elif(CAS_TYPE == CAS_GY)
extern CONTAINER g_win_pin_pwd;
extern CONTAINER g_win_maturity_rating;
extern CONTAINER g_win_work_time;
extern CONTAINER g_win_STBIDlst;
extern CONTAINER g_win_operator_info;
extern CONTAINER g_win_operator_message; //yuanlin
extern CONTAINER g_win_ca_set;       //yuanlin
extern CONTAINER g_win_service_entitle;
extern CONTAINER g_win_walletinfo;
extern CONTAINER g_win_IPPVinfo;
extern CONTAINER g_win_feedcard;
extern CONTAINER g_win_detitle;
extern CONTAINER g_win_mail;
extern CONTAINER g_win_mail_content;
extern CONTAINER g_win_book_detail;
extern CONTAINER g_win_smc_info;
extern CONTAINER g_win_ca_message;   //---yuanlin
extern CONTAINER g_win_ippv_popup;
extern CONTAINER g_win_eigenvalue_info;
extern CONTAINER g_win_smc_upgrade_info;
extern CONTAINER g_win_ippbuy;
#elif (CAS_TYPE == CAS_DVT)
extern CONTAINER g_win_operator;
extern CONTAINER g_win_service_entitle;
extern CONTAINER g_win_smc_info;
extern CONTAINER g_win_ca_message;  //---yuanlin
extern CONTAINER g_win_pin_pwd;
extern CONTAINER g_win_maturity_rating;
extern CONTAINER g_win_work_time;
extern CONTAINER g_win_mail;
extern CONTAINER g_win_mail_content;
extern CONTAINER g_win_book_detail;
extern CONTAINER g_win_feedcard;
extern CONTAINER g_win_walletinfo;
extern CONTAINER g_win_ippview;
extern CONTAINER g_win_ippbuy;
extern CONTAINER g_win_smallpin;
extern CONTAINER g_win_ipporder;
extern CONTAINER g_win_conceal_container;
extern CONTAINER g_win_open_close_con;
extern CONTAINER g_win_nit_value_con;
#elif (CAS_TYPE == CAS_DVN)
extern CONTAINER g_win_operator;
extern CONTAINER g_win_service_entitle;
extern CONTAINER g_win_smc_info;
extern CONTAINER g_win_ca_message;  //---yuanlin
extern CONTAINER g_win_pin_pwd;
extern CONTAINER g_win_maturity_rating;
extern CONTAINER g_win_work_time;
extern CONTAINER g_win_mail;
extern CONTAINER g_win_mail_content;
extern CONTAINER g_win_book_detail;
extern CONTAINER g_win_feedcard;
extern CONTAINER g_win_walletinfo;
extern CONTAINER g_win_ippview;
extern CONTAINER g_win_ippbuy;
extern CONTAINER g_win_smallpin;
extern CONTAINER g_win_ipporder;
extern CONTAINER g_win_transaction_info;
extern CONTAINER g_win_maturity_rating;
#elif (CAS_TYPE == CAS_IRDETO)
extern CONTAINER g_win_info_submenu;
extern CONTAINER g_win_mail_submenu;
extern CONTAINER g_win_mail;
extern CONTAINER g_win_mail_content;
extern CONTAINER g_win_book_detail;
extern CONTAINER g_win_ca_prog;
extern CONTAINER g_win_smc_info;
extern CONTAINER g_win_ca_message;  //---yuanlin
extern CONTAINER g_win_ecm_info;
extern CONTAINER g_win_emm_info;
extern CONTAINER g_win_service_entitle;
extern CONTAINER g_win_entitle_list;
extern CONTAINER g_win_irca_cin;
extern CONTAINER g_win_ca_force_dsp;
extern CONTAINER g_win_pin_pwd;
#elif (CAS_TYPE == CAS_MG)
extern CONTAINER g_win_mscard_info;
extern CONTAINER g_win_smc_info;
extern CONTAINER g_win_ca_message;  //---yuanlin
extern CONTAINER g_win_ippv_popup;
extern CONTAINER g_win_mail;
extern CONTAINER g_win_mail_content;
extern CONTAINER g_win_book_detail;
#elif (CAS_TYPE == CAS_CONAX)
extern CONTAINER	g_win_ca_pin;
extern CONTAINER	g_win_ca_about;
extern CONTAINER	g_win_token;
extern CONTAINER	g_win_purse;
extern CONTAINER	g_win_debit;
extern CONTAINER	g_win_credit;
extern CONTAINER	g_win_subscription;
extern CONTAINER	g_win_event;
extern CONTAINER	g_win_maturity;
extern CONTAINER	g_win_mail;
extern CONTAINER	g_win_mail_content;
extern CONTAINER g_win_book_detail;
#elif (CAS_TYPE == CAS_ABEL)
extern CONTAINER	g_win_subscription_rights;
extern CONTAINER	g_win_ppv_rights;
extern CONTAINER	g_win_parental_control;
extern CONTAINER	g_win_change_tv_pin;
extern CONTAINER	g_win_system_info;
extern CONTAINER	g_win_mail;
extern CONTAINER	g_win_mail_content;
extern CONTAINER g_win_book_detail;
#endif
#endif

#ifdef IRD_OTA_SUPPORT
extern CONTAINER g_win_ird_ota_upgrade;
#endif
#ifdef AD_SANZHOU
extern TEXT_FIELD g_win_ad_txt;
extern CONTAINER g_win_ad_freq;
#endif

extern CONTAINER g_win_information;

#endif //_MENUS_ROOT_H_
