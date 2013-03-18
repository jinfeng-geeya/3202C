#include <sys_config.h>
#include <types.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/snd/snd_dev.h>
#include <hld/snd/snd.h>
#include <hld/decv/vdec_driver.h>
#include <hal/hal_gpio.h>
#include <api/libpub/lib_hde.h>
#include <api/libpub/lib_pub.h>


//#include <api/libosd/osd_lib.h>

#include "images.id"
#include "string.id"

#include "osdobjs_def.h"

#include "osd_config.h"
#include "win_com.h"

#include "win_signalstatus.h"

#define ICON_ID_SIGNAL_NO			IM_SIGNAL_NO
#define ICON_ID_SIGNAL_LNBSHORT		IM_PAUSE_S
#define ICON_ID_SIGNAL_SCRAMBLED	IM_NINE_MONEY
#define ICON_ID_SIGNAL_PARRENTLOCK	IM_LOCK


#define STRING_ID_SIGNAL_NO				RS_MSG_NO_SIGNAL
#define STRING_ID_SIGNAL_LNBSHORT		RS_SYSTEM_KEY_AUDIO_WARNING
#define STRING_ID_SIGNAL_SCRAMBLED		RS_SYSTME_CHANNEL_SCRAMBLED
#define STRING_ID_SIGNAL_PARRENTLOCK		RS_SYSTME_PARENTAL_LOCK
#define STRING_ID_SIGNAL_STOP   RS_SIGNAL_STOPPED

/*******************************************************************************
 * WINDOW's objects declaration
 *******************************************************************************/
extern CONTAINER g_win_sigstatus;
extern BITMAP sigstatus_bmp1; //,sigstatus_bmp2,sigstatus_bmp3,sigstatus_bmp4;
extern TEXT_FIELD sigstatus_txt;

static BOOL NoSignal_Open = FALSE;

/*******************************************************************************
 *	WINDOW's objects defintion MACRO
 *******************************************************************************/
#define WIN_NOSIG_IDX			WSTL_TRANS_IX
#define NOSIG_TXT_IDX		    WSTL_TXT_4
#define NOSIG_BMP_IDX		    WSTL_TXT_3

#define LDEF_TXT(root,varTxt,nxtObj,l,t,w,h,resID,str)	\
	DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
    	0,0,0,0,0, l,t,w,h, NOSIG_TXT_IDX, NOSIG_TXT_IDX, NOSIG_TXT_IDX,NOSIG_TXT_IDX,   \
    	NULL, NULL,  \
    	C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,resID,str)

#define LDEF_BMP(root,varBmp,nxtObj,l,t,w,h,icon)		\
	DEF_BITMAP(varBmp,root,nxtObj,C_ATTR_ACTIVE,0, \
    	0,0,0,0,0, l,t,w,h, NOSIG_BMP_IDX,NOSIG_BMP_IDX,NOSIG_BMP_IDX,NOSIG_BMP_IDX,   \
    	NULL,NULL,  \
    	C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,icon)

#define LDEF_WIN(varWnd,nxtObj,l,t,w,h,focusID)		\
	DEF_CONTAINER(varWnd,NULL,NULL,C_ATTR_ACTIVE,0, \
    	0,0,0,0,0, l,t,w,h, WSTL_TRANS_IX,WSTL_TRANS_IX,WSTL_TRANS_IX,WSTL_TRANS_IX,   \
    	NULL,NULL,  \
    	nxtObj, focusID,0)

#define W_L	(TV_OFFSET_L +(OSD_MAX_WIDTH-W_W)/2)//204//192
#define W_T	(TV_OFFSET_T+(OSD_MAX_HEIGHT-W_H)/2)//114
#define W_W	200
#define W_H	128

#define POP_OFFSET_L	60

#define BMP1_L	(W_L+52)//246
#define BMP1_T	W_T
#define BMP1_W	94
#define BMP1_H	94

#define TXT_L	W_L//192
#define TXT_T	(W_T+BMP1_H)//212
#define TXT_W	200
#define TXT_H	26

/*******************************************************************************
 *	WINDOW's objects defintion
 *******************************************************************************/
LDEF_BMP ( &g_win_sigstatus, sigstatus_bmp1, &sigstatus_txt,  BMP1_L, BMP1_T, BMP1_W, BMP1_H, ICON_ID_SIGNAL_NO )
LDEF_TXT ( &g_win_sigstatus, sigstatus_txt, NULL, TXT_L, TXT_T, TXT_W, TXT_H, STRING_ID_SIGNAL_NO, NULL )

LDEF_WIN(g_win_sigstatus, &sigstatus_bmp1, W_L, W_T, W_W, W_H, 1)

/*******************************************************************************
 *	Local vriable & function declare
 *******************************************************************************/
BOOL block_status = FALSE;

void set_block_status(BOOL b);
BOOL get_block_status();

/*******************************************************************************
 *  WINDOW's  keymap, proc and callback
 *******************************************************************************/



/*******************************************************************************
 *	Menu Item ---  callback, keymap and proc
 *******************************************************************************/



/*********************************************************************************
 *	Window handle's open,proc and handle
 **********************************************************************************/


signal_lock_status lv_lock_status = SIGNAL_STATUS_LOCK;
signal_scramble_status lv_scramble_status = SIGNAL_STATUS_UNSCRAMBLED;
signal_lnbshort_status lv_lnbshort_status = SIGNAL_STATUS_LNBNORMAL;
signal_parentlock_status lv_parrentlock_status = SIGNAL_STATUS_PARENT_UNLOCK;
signal_stop_status lv_signalstop_status=SIGNAL_STATUS_SIGNAL_START;

static void SetSingnalStr(void)
{
	TEXT_FIELD *txt;
	BITMAP *bmp;

	UINT16 strID;
	UINT16 imageID;

	txt = &sigstatus_txt;
	bmp = &sigstatus_bmp1;
	
	/*UINT8 lock;
	struct nim_device *nim_dev;
	if (cur_tuner_idx == 0)
		nim_dev = g_nim_dev;
	else
		nim_dev = g_nim_dev2;

	nim_get_lock(nim_dev, &lock);

	if(get_block_status())
	{
		//libc_printf("test nim lock is %d\n",lock);
		if(!lock)
			return;
		set_block_status(FALSE);
		lv_lock_status = SIGNAL_STATUS_LOCK;
		
	}
	*/

	if (lv_lock_status == SIGNAL_STATUS_UNLOCK)
	{
		imageID = ICON_ID_SIGNAL_NO;
		strID = STRING_ID_SIGNAL_NO;
	}
	else if (lv_scramble_status == SIGNAL_STATUS_SCRAMBLED)
	{
		imageID = ICON_ID_SIGNAL_SCRAMBLED;
		strID = STRING_ID_SIGNAL_SCRAMBLED;
	}
	else if (lv_lnbshort_status == SIGNAL_STATUS_LNBSHORT)
	{
		imageID = ICON_ID_SIGNAL_LNBSHORT;
		strID = STRING_ID_SIGNAL_LNBSHORT;
	}
	else if (lv_parrentlock_status == SIGNAL_STATUS_PARENT_LOCK)
	{
		imageID = ICON_ID_SIGNAL_PARRENTLOCK;
		strID = STRING_ID_SIGNAL_PARRENTLOCK;
	}
    else if(lv_signalstop_status == SIGNAL_STATUS_SIGNAL_STOP)
    {
        imageID = 0;
        strID=STRING_ID_SIGNAL_STOP;
    }
	else
	{
		imageID = 0;
		strID = 0;
	}
	
	//libc_printf("now the lv_lock_status is %d; the lv_scramble_status is %d, the lv_parrentlock_status is %d \n",lv_lock_status,lv_scramble_status,lv_parrentlock_status);
	OSD_SetTextFieldContent(txt, STRING_ID, strID);
	OSD_SetBitmapContent(bmp, imageID);
}

static void SetSignalLockStatus(signal_lock_status lock_flag)
{
	lv_lock_status = lock_flag;
	SetSingnalStr();
}

static void SetSignalScrambleStatus(signal_lnbshort_status lnbshort_flag)
{
	lv_scramble_status = lnbshort_flag;
	SetSingnalStr();
}


void SetChannelParrentLock(signal_parentlock_status parrentlock_flag)
{
	lv_parrentlock_status = parrentlock_flag;
	SetSingnalStr();
}

void SetSignalStopFlag(signal_stop_status signalstop_flag)
{
	lv_signalstop_status = signalstop_flag;
	SetSingnalStr();
}
BOOL GetSignalStoped()
{
    return (lv_signalstop_status == SIGNAL_STATUS_SIGNAL_STOP) ? TRUE : FALSE;
}
BOOL GetChannelParrentLock(void)
{
	return (lv_parrentlock_status == SIGNAL_STATUS_PARENT_LOCK) ? TRUE : FALSE;
}


void set_block_status(BOOL b)
{
	block_status = b;
}

BOOL get_block_status()
{
	return block_status;
}


BOOL GetSignalStatus (  signal_lock_status* lock_flag,
                        signal_scramble_status* scramble_flag,
                        signal_parentlock_status* parrentlock_flag )
{
	UINT8 lock;
	struct dmx_device *dmx_dev;

	SYSTEM_DATA *sys_data;
	sys_data = sys_data_get();
	UINT8 scrabled, scrable_typ;
	P_NODE p_node;
	UINT16 cur_channel;
	struct VDec_StatusInfo curStatus;
	static UINT16 prechan;
	static UINT32 descDetTime = 0;
	struct nim_device *nim_dev;
	struct snd_device* snd_dev;
	static UINT8 snd_stopped = FALSE;
	snd_dev = (struct snd_device*)dev_get_by_type(NULL, HLD_DEV_TYPE_SND);

	if (cur_tuner_idx == 0)
		nim_dev = g_nim_dev;
	else
		nim_dev = g_nim_dev2;


	SetSingnalStr();

	dmx_dev = (struct dmx_device*)dev_get_by_name("DMX_M3327_0");

	scrabled = 0;
	if (dmx_io_control(dmx_dev, IS_AV_SCRAMBLED, (UINT32)(&scrable_typ)) == RET_SUCCESS)
	{
		if (scrable_typ &(VDE_TS_SCRBL | VDE_PES_SCRBL))
			scrabled = 1;
		else
			scrabled = 0;
	}
#ifdef MULTI_CAS
#if (CAS_TYPE == CAS_ABEL)
	scrabled = 0;
#endif
#endif
	SetSignalScrambleStatus ( scrabled ? SIGNAL_STATUS_SCRAMBLED : SIGNAL_STATUS_UNSCRAMBLED );

	nim_get_lock(nim_dev, &lock);

    static UINT8 dma_stop_running=0;

    SetSignalLockStatus(lock ? SIGNAL_STATUS_LOCK : SIGNAL_STATUS_UNLOCK);
    if(lock)
    {
        if (dmx_io_control(dmx_dev, IS_DMA_RUNNING, (UINT32)(&scrable_typ)) != RET_SUCCESS)
            dma_stop_running++;
        else
            dma_stop_running=0;
    }
    if(lock&&dma_stop_running>=5&&lv_parrentlock_status!=SIGNAL_STATUS_PARENT_LOCK)
        SetSignalStopFlag(SIGNAL_STATUS_SIGNAL_STOP);
    else
        SetSignalStopFlag(SIGNAL_STATUS_SIGNAL_START);
	
	if(!lock)
	{
		if(!snd_stopped)
		{
			snd_stop(snd_dev);
			snd_stopped = TRUE;
		}
	}
	else
		snd_stopped = FALSE;

	/*Never in In multi-view mode*/
	if (lock && !scrabled && hde_get_mode() != VIEW_MODE_MULTI)
	{
		cur_channel = sys_data_get_cur_group_cur_mode_channel();
		p_node.ca_mode = 0;
		get_prog_at(cur_channel, &p_node);
		if (p_node.ca_mode && sys_data_get_cur_chan_mode() == TV_CHAN)
		{
			vdec_io_control(g_decv_dev,  \
				VDEC_IO_GET_STATUS, (UINT32) &curStatus);
			if (!curStatus.uFirstPicShowed && curStatus.uCurStatus != VDEC_PAUSED)
			{
				descDetTime++;
				if (descDetTime == 1)
					prechan = cur_channel;
				if (descDetTime >= 3 && prechan == cur_channel)
					scrabled = 1;
				else
					descDetTime = 0;
			}
			else
				descDetTime = 0;
		}
	}
	if (lock_flag != NULL)
		*lock_flag = lv_lock_status;
	if (scramble_flag != NULL)
		*scramble_flag = lv_scramble_status;
	if (parrentlock_flag != NULL)
		*parrentlock_flag = lv_parrentlock_status;


	if ( lv_lock_status == SIGNAL_STATUS_LOCK
	        && lv_scramble_status == SIGNAL_STATUS_UNSCRAMBLED
	        && lv_lnbshort_status == SIGNAL_STATUS_LNBNORMAL
	        && lv_parrentlock_status == SIGNAL_STATUS_PARENT_UNLOCK 
	        && lv_signalstop_status == SIGNAL_STATUS_SIGNAL_START)
		return FALSE;
	else
		return TRUE;
}

BOOL signal_stataus_showed = FALSE;

BOOL GetSignalStatausShow(void)
{
	return signal_stataus_showed;
}


void ShowSignalStatusOSDOnOff(UINT8 flag)
{
	OBJECT_HEAD *obj;
	obj = (OBJECT_HEAD*) &g_win_sigstatus;

	if (flag)
		OSD_DrawObject(obj, C_UPDATE_ALL);
	else
		OSD_ClearObject(obj, 0);

	signal_stataus_showed = flag ? TRUE : FALSE;
}

void ShowSignalStatusOnOff(void)
{
	UINT8 flag;
	signal_lock_status lock_flag;
	signal_scramble_status scramble_flag;
	signal_parentlock_status parrentlock_flag;

	if (GetSignalStatus(&lock_flag, &scramble_flag, &parrentlock_flag) == FALSE)
		flag = 0;
	else
		flag = 1;

	ShowSignalStatusOSDOnOff(flag);
}

void GetSignalStatusRect(OSD_RECT *rect)
{
	POBJECT_HEAD obj;

	obj = (POBJECT_HEAD) &g_win_sigstatus;
	*rect = obj->frame;
}

extern UINT8 play_chan_nim_busy;
void SetChannelNimBusy(UINT32 flag)
{

	play_chan_nim_busy = (UINT8)flag;

}

void shift_signal_osd()
{
	CONTAINER *signal_con = &g_win_sigstatus;
	TEXT_FIELD *signal_txt = &sigstatus_txt;
	BITMAP *signal_bmp = &sigstatus_bmp1;

	signal_con->head.frame.uLeft = (signal_con->head.frame.uLeft) + POP_OFFSET_L;
	signal_txt->head.frame.uLeft = (signal_txt->head.frame.uLeft) + POP_OFFSET_L;
	signal_bmp->head.frame.uLeft = (signal_bmp->head.frame.uLeft) + POP_OFFSET_L;
}

void restore_signal_osd()
{
	CONTAINER *signal_con = &g_win_sigstatus;
	TEXT_FIELD *signal_txt = &sigstatus_txt;
	BITMAP *signal_bmp = &sigstatus_bmp1;

	signal_con->head.frame.uLeft = (signal_con->head.frame.uLeft) - POP_OFFSET_L;
	signal_txt->head.frame.uLeft = (signal_txt->head.frame.uLeft) - POP_OFFSET_L;
	signal_bmp->head.frame.uLeft = (signal_bmp->head.frame.uLeft) - POP_OFFSET_L;
}
