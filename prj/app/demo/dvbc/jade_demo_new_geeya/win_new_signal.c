#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <hld/nim/nim.h>
//#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"
//#include "menus_root.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"
#include "osd_rsc.h"

#include "copper_common/system_data.h"
#include "copper_common/menu_api.h"
//#include "win_com_menu_define.h"

#include "win_com.h"
//include the header from xform 
#include "win_new_signal.h"

extern CONTAINER g_win_sys_management;
#define DVBC_QUICKSCAN_EDIT_LEN  0x06 // add .xx
//this values is restore the values setting
static UINT32 i_qs_freq = 0; //0x74CC;
static UINT32 i_qs_symbol = 0; //6875;
static UINT8 i_qs_mode = 0; //QAM64;


#define VACT_SET_DECREASE	(VACT_PASS + 1)
#define VACT_SET_INCREASE	(VACT_PASS + 2)



/*******************************************************************************
*	Callback and Keymap Functions
*******************************************************************************/


static VACTION signal_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

	switch (key)
	{   
	    case V_KEY_MENU:
			act = VACT_CLOSE;
			break;
		case V_KEY_EXIT:
			
			dm_set_onoff(1); //new add
			win_qs_update_param(1);  //new add
            win_mainFeq_save_data(); //new add
            sys_data_save(1);        //new add
			BackToFullScrPlay();
			break;
	
		default:
			break;
	}

	return act;
}

static PRESULT signal_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	SYSTEM_DATA *pSysData = NULL;

	pSysData = sys_data_get();

	switch (event)
	{
		case EVN_PRE_OPEN:    //new add
           
			win_mainFeq_load_data();
			dvbc_quickscan_set_def_val();
			dm_set_onoff(0);
			//win_new_signal_load_data(FALSE);  //yuanlin
			break;
		case EVN_POST_OPEN:
		//	win_new_signal_load_data();
		    new_win_signal_open(pObj);
		    new_win_signal_update(); 
			//win_qs_update_param(1);
			break;
		case EVN_PRE_CLOSE:
		*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
			break;
		case EVN_POST_CLOSE:
			//win_new_signal_close();
			dm_set_onoff(1);       //new add
			//win_qs_update_param(1);
			//win_mainFeq_save_data();
			new_win_signal_close();
			//sys_data_save(1);
			break;
		case EVN_MSG_GOT:
			ret=win_new_signal_message_proc(param1, param2);
			
			break;
	}
	return ret;
}


/*******************************************************************************
 *	HELPER FUNCTIONS DEFINE
 *******************************************************************************/

static PRESULT win_new_signal_message_proc(UINT32 msg_type, UINT32 msg_code)
{
	PRESULT ret = PROC_LOOP;

	switch (msg_type)
	{   
		case CTRL_MSG_SUBTYPE_STATUS_SIGNAL:
			new_win_signal_update();
            break;
		//case CTRL_MSG_SUBTYPE_STATUS_SIGNAL:
			//win_new_signal_load_data(TRUE);
			//break;
		default:
			break;
	}

	

}

#if 0
static void win_new_signal_load_data(BOOL Flag)
{   
     UINT8 str[30];
	 CONTAINER *con;
	 CONTAINER *pObj;
	 pObj = &g_win_signal01;
     PROGRESS_BAR *bar;
	 INT32 temp = 0;
	 UINT32 dot = 0;
	 UINT16 cn_noise, level;
	 UINT32 ber;
	 UINT8 lock,cn_noise_maped = 0, level_maped = 0;;
	 UINT16 level_shIdx,quality_shIdx,ber_idx,index,cnt;
     struct nim_device *nim_dev;

     UINT32 valid_idx;

	 P_NODE p_node;
     T_NODE tp;
	 TEXT_FIELD *txt;
	 UINT16 qam[5] = {16,32,64,128,256};


    UINT8  av_mode;
    UINT32 ret;
	UINT16 cur_channel;
	/*get the info of the current video */
	display_strs_init(3, 5);
	
	av_mode = sys_data_get_cur_chan_mode();
	cur_channel = sys_data_get_cur_group_cur_mode_channel();
	ret = get_prog_at(cur_channel, &p_node);
	
	/*fill velues for each display_strs[]*/
 
	if (cur_tuner_idx == 0)
		nim_dev = g_nim_dev;
	else
		nim_dev = g_nim_dev2;

	nim_get_lock(nim_dev, &lock);
    nim_ioctl_ext(nim_dev, NIM_DRIVER_GET_RF_LEVEL, (void*) &level);  
      
 	if (lock)
	{
		level_shIdx = WSTL_YU_ORANGE;
		quality_shIdx = WSTL_YU_ORANGE;
		ber_idx = WSTL_YU_ORANGE;
        nim_ioctl_ext(nim_dev, NIM_DRIVER_GET_CN_VALUE, (void*) &cn_noise);    
		nim_ioctl_ext(nim_dev, NIM_DRIVER_GET_BER_VALUE, (void*) &ber);
	}
	else
	{
		level_shIdx = WSTL_YU_ORANGE;
		quality_shIdx = WSTL_YU_ORANGE;
		ber_idx = WSTL_YU_ORANGE;
		cn_noise = 0;
		ber = 0;
	}
	  
     //update the text value & uint for each item

     if (level % 10)
       {
		temp = (107-level / 10) - 1; //107
		dot = 10-level % 10;
	   }
	else
	  {
		temp = 107-level / 10; //107
		dot = 0;
	  }

    if(temp<0)
        temp= -temp;  //temp=0; //avoid the negative value

	sprintf(str, "%d.%d", temp,dot);
	txt = &signal_txt_level0;    //信号强度
	OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)str);

	  
	if(lock)
	 { 
		sprintf(str, "%d.%d", cn_noise / 10, cn_noise % 10);
	 }
	else
	 {
		sprintf(str, "0 ");
	  }
	txt = &signal_txt_CN1;   //信号品质
	OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)str);


	sprintf(str, "%d ", ber);
	txt = &signal_txt_BER2;     //误码率
	OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)str);



  

	//draw SNR bar
	nim_get_SNR(nim_dev, &cn_noise_maped);
	bar = &sig_bar_CN_11;      //信号品质
    if(0==cn_noise)
    {
        OSD_SetProgressBarPos(bar, cn_noise_maped);
        bar->wTickFg = quality_shIdx;
    }
    else
    {
	    OSD_SetProgressBarPos(bar, cn_noise_maped); 
	    bar->wTickFg = quality_shIdx;
    }


//Draw the signal error bar
	bar = &sig_bar_BER22;     //误码率
	ber = map_new_ber_value(&ber);
	OSD_SetProgressBarPos(bar, ber);
	bar->wTickFg = ber_idx;
	
//Draw the signal intensity bar
    level_maped = temp *100 / 107;  
//    if((level_maped>=0)&&(level_maped<30))
    if(level_maped<30)
    {
        level_maped = 5*level_maped/2;  
    }
    else if((level_maped>=30)&&(level_maped<60))
    {
        level_maped = 75 + (level_maped - 30)/3; 
    }
    else
    {
        level_maped = 95 + (level_maped - 60)/20;
    }
    
	if (level_maped > 100)
	{
		level_maped = 100;
//        osal_task_sleep(10);
    } 
    
	bar = &sig_bar_level01;      //信号 强度
	OSD_SetProgressBarPos(bar, level_maped);
	bar->wTickFg = level_shIdx;   //错误
//draw them all
	if(Flag)
	OSD_DrawObject((POBJECT_HEAD)pObj, C_UPDATE_ALL);

	return TRUE;


}
#endif
/*zoom the value to 0 - 100*/
static UINT32 map_new_ber_value(UINT32 *ber)
{
	if (*ber > 50000)
	{
		return 0; /* bad ber*/
	}

	/*if ber > 800  PER will not be zero.*/
	if (*ber < 8000)
	 /* zoom to 0-40*/
	{
		*ber =  *ber / 200;
		return  *ber;
	}
	if (*ber < 50000)
	{
		*ber = 29+ *ber / 700;
		return  *ber;
	}
	return 0;
}
static VACTION item_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

	switch (key)
	{  
		case V_KEY_LEFT:
			act = VACT_SET_DECREASE;
			break;
		case V_KEY_RIGHT:
			act = VACT_SET_INCREASE;
			break;
		case V_KEY_UP:
			act = VACT_CURSOR_UP;
			break;
		case V_KEY_DOWN:
			act = VACT_CURSOR_DOWN;
			break;
		case V_KEY_ENTER:
			act = VACT_ENTER;
			break;
		default:
			break;
	}
	return act;

}
static PRESULT item_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	VACTION act;
	 UINT8 id= OSD_GetFocusID(pObj); 

	switch (event)
	{
case EVN_ITEM_POST_CHANGE:
			if (1 == qs_check_value())
			 win_qs_update_param(1);
				break;
		case EVN_UNKNOWN_ACTION:
			act = (VACTION)(param1 >> 16);
			if (act == VACT_ENTER)
			{
				if (1 == qs_check_value())
				{ 
				win_qs_update_param(1);
					
				}
				ret = PROC_LOOP;
			}
			break;
		default:
			break;
	}
	return ret;
}
static VACTION main_signal_item_keymap(POBJECT_HEAD pObj, UINT32 key)
{
		VACTION Action = VACT_PASS;

	switch (key)
	{   
		case V_KEY_LEFT:
			Action = VACT_EDIT_LEFT;
			break;
		case V_KEY_RIGHT:
			Action = VACT_EDIT_RIGHT;
			break;
		case V_KEY_0:
		case V_KEY_1:
		case V_KEY_2:
		case V_KEY_3:
		case V_KEY_4:
		case V_KEY_5:
		case V_KEY_6:
		case V_KEY_7:
		case V_KEY_8:
		case V_KEY_9:
			Action = key - V_KEY_0 + VACT_NUM_0;
			break;
		default:
			break;
	}

	return Action;

}
static VACTION main_signal_sel_item_keymap(POBJECT_HEAD pObj, UINT32 key)
{
    VACTION act = VACT_PASS;
	switch (key)
	{
		case V_KEY_LEFT:
			act = VACT_DECREASE;
			break;
		case V_KEY_RIGHT:
			act = VACT_INCREASE;
			break;
		case V_KEY_ENTER:
			break;
		default:
			act = VACT_PASS;
			break;
	}

	return act;

}

static PRESULT main_signal_sel_item_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	UINT8 bID;
	VACTION unact;



	switch (event)
	{
		case EVN_POST_CHANGE:
			win_qs_update_param(1);
			break;
		default:
			break;

	}


	return ret;
}


static void dvbc_quickscan_set_def_val(void)
{
	INT8 rule[20];
	UINT32 freq, symbol, constellation;
	P_NODE p_node;
	T_NODE t_node;
	UINT32 prog_num;
	UINT16 cur_channel;

	/* FREQ */
	wincom_i_to_mbs_with_dot(display_strs[0],
	                         i_qs_freq, DVBC_QUICKSCAN_EDIT_LEN - 1/*without dot*/, 0x02/*xxx.xx*/);
	ComAscStr2Uni("MHz", display_strs[30]);
	/* SYMBOL */
	wincom_i_to_mbs(display_strs[1],
	                i_qs_symbol, DVBC_QUICKSCAN_EDIT_LEN - 1);
	ComAscStr2Uni("KBaud", display_strs[31]);

	OSD_SetMultiselSel(&txt_signal_item22, i_qs_mode - QAM16);
	
}



// 0 not save, 1 save the parameter
static void win_qs_update_param(BOOL save)
{
	UINT8 dot_pos;
	UINT32 freq, symbol, constellation;

	//freq
	wincom_mbs_to_i_with_dot(display_strs[0], &freq, &dot_pos);
	// SYMBOL
	symbol = wincom_mbs_to_i(display_strs[1]);
	// scan mode
	constellation = QAM16 + OSD_GetMultiselSel(&txt_signal_item22);


	if (1 == save)
	{
		i_qs_freq = freq;
		i_qs_symbol = symbol;
		i_qs_mode = constellation;
	
	}
}




// 1 valid 0 invalid
static UINT8 qs_check_value()
{
	UINT32 ret1, ret2;
	UINT8 save;

	ret1 = check_freq_value(display_strs[0]);
	ret2 = check_symbol_value(display_strs[1]);
	/*if (ret1 != 0 || ret2 != 0)
	{
		win_popup_msg(NULL, NULL, RS_MSG_INVALID_INPUT_CONTINUE);
		return 0;
	}
	else*/
		return 1;
}


static INT32 check_freq_value(UINT16 *str)
{
	INT32 ret = 0;
	UINT32 val;
	UINT8 dot_pos;

	/* FREQ */
	wincom_mbs_to_i_with_dot(str, &val, &dot_pos);
	if (val < 4825 || val > 85875)
		ret =  - 1;

	return ret;
}

static INT32 check_symbol_value(UINT16 *str)
{
	INT32 ret = 0;
	UINT32 val;

	/* SYMBOL */
	val = wincom_mbs_to_i(str);
	if (val < 1000 || val > 7000)
		ret =  - 1;


	return ret;
}

static void win_mainFeq_save_data()
{
	SYSTEM_DATA *sys_data = &system_config;

	sys_data->main_frequency = i_qs_freq;
	sys_data->main_symbol = i_qs_symbol;
	sys_data->main_qam = i_qs_mode;

}
static void win_mainFeq_load_data()
{
	SYSTEM_DATA *sys_data = &system_config;

	i_qs_freq = sys_data->main_frequency;
	i_qs_symbol = sys_data->main_symbol;
	i_qs_mode = sys_data->main_qam;

}

