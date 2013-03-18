#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <api/libpub/lib_pub.h>
#include <api/libpub/lib_as.h>

//#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"

#include "win_com.h"
#include "menus_root.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_signalstatus.h"

#include "win_signal.h"

#include "win_new_signal_test.h"

#define WIN_SH_IDX                  WSTL_WIN_2
#define LOCK_IDX                    WSTL_BAR_05
#define UNLOCK_IDX                  WSTL_BAR_05//WSTL_BARBG_01
#define PROGRESS_COLOR_BLUE         WSTL_BAR_05
#define PROGRESS_COLOR_GREEN        WSTL_BAR_05
#define PROGRESSBAR_SH_IDX		    WSTL_BARBG_02
#define PROGRESSBAR_MID_SH_IDX		WSTL_NOSHOW_IDX
#define PROGRESS_TXT_IDX    WSTL_TXT_11


/*******************************************************************************
 *	key mapping and event callback definition
 *******************************************************************************/


/*this interface is keep for "key_check_signal_status()" in key.c*/
BOOL new_win_signal_set_level_quality(UINT32 level, UINT32 quality, UINT32 lock)
{
	PROGRESS_BAR *bar;
	POBJECT_HEAD pObj;
	UINT16 level_shIdx;
	UINT16 quality_shIdx;
	UINT16 ber_idx;
	UINT32 ber_value;

	if (lock)
	{
		level_shIdx = PROGRESS_COLOR_GREEN;
		quality_shIdx = LOCK_IDX;
		ber_idx = PROGRESS_COLOR_BLUE;
	}
	else
	{
		level_shIdx = UNLOCK_IDX;
		quality_shIdx = UNLOCK_IDX;
		ber_value = 0;
	}

	if (level < 4)
		level_shIdx = WSTL_NOSHOW_IDX;
	if (quality < 4)
		quality_shIdx = WSTL_NOSHOW_IDX;

	bar = &new_sig_bar_level;//sig_bar_level;
	OSD_SetProgressBarPos(bar, level);
	bar->wTickFg = level_shIdx;

	bar = &new_sig_bar_CN;//sig_bar_CN;
	OSD_SetProgressBarPos(bar, quality);
	bar->wTickFg = quality_shIdx;


	/* If signal display is on */
	pObj = (POBJECT_HEAD) &g_new_signal_test;//g_win_signal;
	if (pObj->pRoot == NULL)
		return FALSE;
	else
		return TRUE;
}



void new_win_signal_open(POBJECT_HEAD w)
{
	POBJECT_HEAD pObj;
	UINT8 level, quality, lock;
	struct nim_device *nim_dev;
	if (cur_tuner_idx == 0)
		nim_dev = g_nim_dev;
	else
		nim_dev = g_nim_dev2;

	if (w->bType != OT_CONTAINER)
		return ;

	pObj = (POBJECT_HEAD) &g_new_signal_test;//g_win_signal;
	pObj->pRoot = w;

	nim_get_lock(nim_dev, &lock);
	nim_get_AGC(nim_dev, &level);
	nim_get_SNR(nim_dev, &quality);

	new_dem_signal_to_display(lock, &level, &quality);
	new_win_signal_set_level_quality(level, quality, lock);
	new_win_signal_update();

}



/*this interface is nor used yet, can delete it???*/
void new_win_signal_open_tplist(POBJECT_HEAD w)
{
	//POBJECT_HEAD pObj;
	UINT8 level, quality, lock;
	struct nim_device *nim_dev;
	if (cur_tuner_idx == 0)
		nim_dev = g_nim_dev;
	else
		nim_dev = g_nim_dev2;

	if (w->bType != OT_CONTAINER)
		return ;

	nim_get_lock(nim_dev, &lock);
	nim_get_AGC(nim_dev, &level);
	nim_get_SNR(nim_dev, &quality);

	new_dem_signal_to_display(lock, &level, &quality);
	new_win_signal_set_level_quality(level, quality, lock);

}


void new_win_signal_close(void)
{
	POBJECT_HEAD pObj;
    pObj = (POBJECT_HEAD) &g_new_signal_test;
	pObj->pRoot = NULL;
}

BOOL new_win_signal_update()
{
	POBJECT_HEAD pObj;
	PROGRESS_BAR *bar;
	TEXT_FIELD *txt;

	UINT16 cn_noise, level;
	UINT32 ber;
	INT32 temp = 0;
	UINT32 dot = 0;
	char str[20];
	struct nim_device *nim_dev;
	UINT8 lock, cn_noise_maped = 0, level_maped = 0;
	UINT16 level_shIdx,quality_shIdx,ber_idx;
    
	pObj = (POBJECT_HEAD) &g_new_signal_test;//lin
	if (pObj->pRoot == NULL)
		return FALSE;

	if (cur_tuner_idx == 0)
		nim_dev = g_nim_dev;
	else
		nim_dev = g_nim_dev2;

	nim_get_lock(nim_dev, &lock);
    nim_ioctl_ext(nim_dev, NIM_DRIVER_GET_RF_LEVEL, (void*) &level);
        
	if (lock)
	{
		level_shIdx = PROGRESS_COLOR_GREEN;
		quality_shIdx = LOCK_IDX;
		ber_idx = PROGRESS_COLOR_BLUE;
        nim_ioctl_ext(nim_dev, NIM_DRIVER_GET_CN_VALUE, (void*) &cn_noise);    
		nim_ioctl_ext(nim_dev, NIM_DRIVER_GET_BER_VALUE, (void*) &ber);
	}
	else
	{
		level_shIdx = UNLOCK_IDX;
		quality_shIdx = UNLOCK_IDX;
		ber_idx = UNLOCK_IDX;
		cn_noise = 0;
		ber = 0;
	}

	//update the text value & uint for each item
	if(lock)
	{
		sprintf(str, "%d.%d ", cn_noise / 10, cn_noise % 10);
	}
	else
	{
		sprintf(str, "0");
	}
	txt = &new_sig_txt_CN0;//sig_txt_CN_uint;
	OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)str);


	sprintf(str, "%d ", ber);
	txt = &new_sig_txt_BER0;//sig_txt_BER_uint;
	OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)str);
            
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

	sprintf(str, "%d.%d", temp, dot);
	txt = &new_sig_txt_level0;//sig_txt_level_uint;
	OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)str);

//draw SNR bar
	nim_get_SNR(nim_dev, &cn_noise_maped);
	bar = &new_sig_bar_CN;//sig_bar_CN;  
    if(0==cn_noise)
    {
        OSD_SetProgressBarPos(bar, cn_noise_maped);
        bar->wTickFg = quality_shIdx;
    }
    else
    {
	    OSD_SetProgressBarPos(bar, cn_noise_maped); //OSD_SetProgressBarPos(bar, cn_noise_maped);
	    bar->wTickFg = quality_shIdx;
    }


//Draw the signal error bar
	bar = &new_sig_bar_BER;//sig_bar_BER;
	ber = new_map_ber_value(&ber);
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
    
	bar = &new_sig_bar_level;//sig_bar_level; 
	OSD_SetProgressBarPos(bar, level_maped);
	bar->wTickFg = level_shIdx;

//draw them all
	OSD_DrawObject(pObj, C_UPDATE_ALL);

	return TRUE;
}


/*zoom the value to 0 - 100*/
static UINT32 new_map_ber_value(UINT32 *ber)
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

void new_dem_signal_to_display(UINT8 lock, UINT8 *level, UINT8 *quality)
{
#if 0
	if (*level > 90)
		*level = 90;
	if (*level < 10)
		*level =  *level * 11 / 2;
	else
		*level =  *level / 2+50;
	if (*level > 90)
		*level = 90;
	if (!lock)
		*level /= 2;
	if (lock)
	{
		if (*quality < 30)
			*quality =  *quality * 7 / 3;
		else
			*quality =  *quality / 3+60;
	}
	else
	{
		if (*quality >= 20)
			*quality =  *quality * 3 / 7;
	}

	if (*quality > 90)
		*quality = 90;
#endif


}

