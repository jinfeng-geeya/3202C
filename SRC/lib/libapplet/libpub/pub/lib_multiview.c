/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2004 (C)
 *
 *  File: lib_multiview.c
 *
 *  Description: This file implement the MultiView on M3329
 *         
 *
 *  History:
 *      Date         		Author   		Version  		Comment
 *      ====         	=========       =======  	=======
  ****************************************************************************/
#include <hld/decv/vdec_driver.h>
#include <api/libc/printf.h>
#include <hld/hld_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/dis/vpo.h>
#include <mediatypes.h>
#include <api/libpub/lib_pub.h>
#include <api/libpub/lib_hde.h>
#include <api/libpub/lib_mv.h>
#include <api/libpub/lib_frontend.h>


/*******************************************************
* macro define
********************************************************/  
#define 	MV_DEBUG PRINTF// libc_printf
#ifdef SPEEDUP_DV //to speed up DV search speed --Michael Xie 2006/2/24
#define  TIMES_FOR_FIRST_PIP	40
#else
#define  TIMES_FOR_FIRST_PIP	80
#endif


/*******************************************************
* extern  declare
********************************************************/
extern BOOL	g_vdec_FirstPIPInChanGot;
extern OSAL_ID libpub_flag_id;
extern void cc_play_channel(UINT32 cmd, struct ft_frontend * ft, struct cc_param * param);
extern void cc_stop_channel(UINT32 cmd, struct cc_es_info *es, struct cc_device_list *dev_list);


/*******************************************************
* struct and variable
********************************************************/
//UINT16 l_mv_uIndex[MV_BOX_NUM];
static struct cc_es_info *l_mv_es[MV_BOX_NUM];
static struct cc_device_list *l_mv_dev_list[MV_BOX_NUM];
static struct ft_frontend *l_mv_ft[MV_BOX_NUM];
static struct MVInitPara l_MVinitPara;
static struct MultiViewPara gMvpara;
static struct MVInitPara g_MVinitPara;
   
static UINT8 l_mv_status;
static UINT8 uPrecision;
static UINT8 l_dview_h_precision;
static UINT8 l_dview_v_precision;
static UINT16	l_time_for_first_pip = TIMES_FOR_FIRST_PIP;
static UINT16 	uCurrentActiveIndex;
static BOOL g_bPlayInSpecialWin = FALSE;


/*******************************************************
* internal function
********************************************************/
/* Set Multiview Flag */
static void set_mv(UINT32 flag)
{
    	osal_flag_set(libpub_flag_id, flag);
}

/* Clear Multiview Flag */
static void clear_mv(UINT32 flag)
{
    	osal_flag_clear(libpub_flag_id, flag);
}

/* Get Multiview Flag */
static BOOL get_mv(UINT32 flag)
{
     	UINT32 flgptn;

    	osal_flag_wait(&flgptn, libpub_flag_id, flag, OSAL_TWF_ORW, 0);
    	if((flgptn & flag) == flag )
    	{
        	return TRUE;
    	}
    	else
    	{
        	return FALSE;
    	}
}

/* Play One Special Box */
static BOOL MW_SwitchPIPWin(struct Position *pPos, struct cc_es_info* es, struct ft_frontend* ft, struct cc_device_list* dev_list, UINT32 uBoxStatus, BOOL spe_win_flg)
{
	UINT8 ret_val;
	UINT16 counter;
	UINT8 unlockTimes = 0;
	UINT8 lock= 0;
	P_NODE p_node;
	struct Rect PipWin;
	UINT32 cmd = 0;
	struct cc_param param;
	
	MEMSET(&param, 0, sizeof(struct cc_param));
	
	MV_DEBUG("MV PIP 1 ---- Param Init.\n");
	
	if(get_mv_cancel())
	{
		uBoxStatus = BOX_PLAY_ABORT;
	}
	
	MV_DEBUG("MV PIP 2 ---- Check Cancel Multiview Or Not.\n");

	if(uBoxStatus != BOX_PLAY_ABORT)
	{
		//2Configure the position of output box from VP
	    	if(g_bPlayInSpecialWin && spe_win_flg)
	    	{
			MEMCPY(&PipWin, &l_MVinitPara.PlayWin, sizeof(PipWin));
	    	}
		else
		{
			PipWin.uHeight = l_MVinitPara.tPipSize.uHeight;
			PipWin.uWidth = l_MVinitPara.tPipSize.uWidth;
			PipWin.uStartX = pPos->uX;
			PipWin.uStartY = pPos->uY;
		}
		
		ret_val = vdec_switch_pip_ext(dev_list->vdec, &PipWin);

		MV_DEBUG("MV PIP 3 ---- Config Box Position.\n");

		// Play this Box
		MEMCPY(&(param.es), es, sizeof(struct cc_es_info));
		MEMCPY(&(param.dev_list), dev_list, sizeof(struct cc_device_list));
		cmd &= ~(CC_CMD_CLOSE_VPO|CC_CMD_FILL_FB);
    		cc_play_channel(cmd, ft, &param);	/* Get SI Monitor ID in this function */
		es->sim_id = param.es.sim_id;		/* Set SI Monitor ID to Local Variable */

		MV_DEBUG("MV PIP 4 ---- Play Box.\n");

		counter=0;
		while(g_vdec_FirstPIPInChanGot == FALSE)
		{
			if(get_mv_cancel())
			{
				return FALSE;
			}
			osal_task_sleep(10);

			counter++;
#ifdef SPEEDUP_DV
            		if(counter == (l_time_for_first_pip*2))//reach the longest time, to speed up DV search speed --Michael Xie 2006/2/24
#else
			if(counter == (l_time_for_first_pip*5))//reach the longest time
#endif			
			{
				break;
			}

			if(counter > l_time_for_first_pip)
			{ 
				nim_get_lock((struct nim_device*)dev_get_by_id(HLD_DEV_TYPE_NIM, 0), &lock);
				if(lock == 0)
				{
					unlockTimes++;
				}
				else
				{
					unlockTimes=0;
				}

				if(unlockTimes > l_time_for_first_pip/2)
				{
					MV_DEBUG("MV___MW_SwitchPIPWin leave with unlock>>>>>>>>>%d>>>>>>>>.\n",counter);
					break;		
				}
			} 
			
			struct dmx_device *dmx;
			dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);
			if(RET_SUCCESS == dmx_io_control(dmx, IS_AV_SCRAMBLED, (UINT32)NULL) && (counter >= (l_time_for_first_pip*2)))// the channel is scrambled
			{
				MV_DEBUG("MV___MW_SwitchPIPWin leave with scrambled>>>>>>>>>%d>>>>>>>>.\n",counter);
				break;
			} 
			if(counter % (l_time_for_first_pip*2) == 0 && counter >= l_time_for_first_pip)//reset qpsk
			{ 
			//	cc_reset_qpsk(0);
				MV_DEBUG("MV___MW_SwitchPIPWin reset qpsk>>>>>>>>>%d>>>>>>>>.\n",counter);				
			}
		}
		MV_DEBUG("MV PIP 5 ---- Last.\n");
	}
	
    	return TRUE;
}

/* Enter Multiview */
static RET_CODE MultiView_Enter(UINT32 cmd, struct MultiViewPara *mvPara)
{
	UINT8 i;
	UINT8 ret_val;
	UINT32 boxStatus = BOX_PLAY_NORMAL;
	struct VDecPIPInfo  PIPInitInfo;
	struct MPSource_CallBack MPCallBack;
	struct PIPSource_CallBack PIPCallBack;
	UINT8 curActiveIdx;
	UINT16 prog_pos = 0xffff;

	MV_DEBUG("MV------MultiView_Enter\n");

	// Set Multiview Status
	l_mv_status = MV_ACTIVE | MV_SCAN_SCREEN | MV_SCAN_BOX;


	//If View Mode is Multiview, then Stop the Current Channel of Last Multiview
	if(hde_get_mode() == VIEW_MODE_MULTI)
	{
		cc_stop_channel(cmd, l_mv_es[uCurrentActiveIndex], l_mv_dev_list[uCurrentActiveIndex]);
	}

	MEMCPY(&gMvpara, mvPara, sizeof(struct MultiViewPara));
	
	set_mv(LIBPUB_FLG_MV_ENTER);

	curActiveIdx = gMvpara.uBoxActiveIdx-1;
		
   	MV_DEBUG("MV 1 ----- Switch Multiview Mode.\n");

	//3Switch to PIP
	if(hde_get_mode() != VIEW_MODE_MULTI)
	{
		// Set Multiview Mode
	       hde_set_mode(VIEW_MODE_MULTI);
		PIPInitInfo.pipStaPos.uX=l_MVinitPara.puBoxPosi[0].uX;
		PIPInitInfo.pipStaPos.uY=l_MVinitPara.puBoxPosi[0].uY;
		PIPInitInfo.mpSize.uHeight=l_MVinitPara.tMpSize.uHeight;
		PIPInitInfo.mpSize.uWidth=l_MVinitPara.tMpSize.uWidth;
		PIPInitInfo.pipSize.uHeight=l_MVinitPara.tPipSize.uHeight;
		PIPInitInfo.pipSize.uWidth=l_MVinitPara.tPipSize.uWidth;

		PIPInitInfo.bUseBgColor=1;
		PIPInitInfo.bgColor.uCb=0x80;
		PIPInitInfo.bgColor.uCr=0x80;
		PIPInitInfo.bgColor.uY=0x10;

		// Stop Current Channel
		cmd |= (CC_CMD_CLOSE_VPO | CC_CMD_FILL_FB);
		cc_stop_channel(cmd, l_mv_es[curActiveIdx], l_mv_dev_list[curActiveIdx]);

		// Set Video Decoder Output Mode
		osal_task_sleep(30);
		ret_val = vdec_set_output(l_mv_dev_list[curActiveIdx]->vdec, (enum VDecOutputMode)PIP_MODE, &PIPInitInfo, &MPCallBack, &PIPCallBack);		
		if (ret_val != RET_SUCCESS)
		{
			MV_DEBUG("MV ---VDec_SetOutput return false.\n",i);
		}

		// Set VPO Window Mode
		vpo_win_mode(l_mv_dev_list[curActiveIdx]->vpo, (BYTE)(VPO_MAINWIN|VPO_PIPWIN), &MPCallBack, &PIPCallBack);	   
	}
	else
	{
		osal_task_sleep(100);
	}

	// Process by Upper Layer (Show Box Scan Start)
	if(l_MVinitPara.callback != NULL)
	{
		boxStatus = l_MVinitPara.callback(BOX_SCAN_START);
	}

//1Draw 9 windows
	for(i = 0; i < gMvpara.uBoxNum; i++)
	{
		// Play Box Start Callback
		MV_DEBUG("MV --- %d -- Show Box Start Callback.\n",i);
		//090403, move to here, for AP will decide whether to play the Parent Locked program
		boxStatus=BOX_PLAY_NORMAL;
		if(l_MVinitPara.callback != NULL)
		{
			boxStatus = l_MVinitPara.callback(i);
		}
		
		// Stop Last Box Channel
		MV_DEBUG("MV ---%d -- Stop Last Box Channel.\n",i);
//090403
//		boxStatus=BOX_PLAY_NORMAL;		
		cmd &= ~ (CC_CMD_CLOSE_VPO | CC_CMD_FILL_FB);
		if( i != 0)
		{
			cc_stop_channel(cmd, l_mv_es[i - 1], l_mv_dev_list[i - 1]);
		}		

		// Play Box Channel
		MV_DEBUG("MV --- %d -- Show Box Channel.\n",i);
		ret_val = TRUE;
		ret_val = MW_SwitchPIPWin(&l_MVinitPara.puBoxPosi[i], l_mv_es[i], l_mv_ft[i], l_mv_dev_list[i], boxStatus, 0);

		// Play Box Finish Callback
		MV_DEBUG("MV --- %d -- Show Box Finish Callback.\n", i);
		if(l_MVinitPara.callback != NULL)
		{
			l_MVinitPara.callback(i + BOX_FINISHED);
		}
        	if(ret_val == FALSE)
		{
			MV_DEBUG("MV ---- Receive a Message to Quit.\n");
			break;
		}
		MV_DEBUG("Show Box %d OK!\n", i);
		
	}

	// If Not Finish Show All Boxes or Error, then Return
 	if(i != gMvpara.uBoxNum)
	{
		l_mv_status = MV_ACTIVE;

		if (i != 0)
		{
			uCurrentActiveIndex = i-1;
		}
		
		MV_DEBUG("MV ---- Receive a Message to Return:%d.\n",i);
		return RET_FAILURE;
	}

	// Set Multiview Status
	l_mv_status = MV_ACTIVE | MV_SCAN_SCREEN;
	osal_task_sleep(1000);//this may need some information from Ch_Chg and VDEC

	//1Switch to the special box	
	if((gMvpara.uBoxNum != (gMvpara.uBoxActiveIdx)) || g_bPlayInSpecialWin)
	{
		// Stop Last Box Channel
		MV_DEBUG("MV Switch to Special Box ---- Stop Last Box Channel.\n");

		// Show Special Box Start by Callback
		MV_DEBUG("MV Switch to Special Box ---- Show Special Box Start by Callback.\n");
		if(l_MVinitPara.callback != NULL)
		{
			boxStatus = l_MVinitPara.callback(curActiveIdx);
		}

		cc_stop_channel(cmd, l_mv_es[i - 1], l_mv_dev_list[i - 1]);
		
		// Show Special Box
		MV_DEBUG("MV Switch to Special Box ---- Show Special Box.\n");
		ret_val = MW_SwitchPIPWin(&l_MVinitPara.puBoxPosi[curActiveIdx], l_mv_es[curActiveIdx], l_mv_ft[curActiveIdx], l_mv_dev_list[curActiveIdx], boxStatus, 1);

		// Show Special Box Finish by Callback
		MV_DEBUG("MV Switch to Special Box ---- Show Special Box Finish by Callback.\n");
		if(l_MVinitPara.callback != NULL)
		{
			l_MVinitPara.callback(curActiveIdx + BOX_FINISHED + BOX_FOCUS);
		}
		if(ret_val == FALSE)
		{
			MV_DEBUG("MV----Receive a Message to Quit2.\n");
			l_mv_status = MV_ACTIVE;
			uCurrentActiveIndex = gMvpara.uBoxNum - 1;
			return RET_FAILURE;
		}
		MV_DEBUG("MV Switch to Special Box ---- Show Special Box Ok.\n");
	}

	uCurrentActiveIndex = gMvpara.uBoxActiveIdx-1;
	l_mv_status = MV_ACTIVE;
	MV_DEBUG("MultiView_Enter success~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~`!.\n");
	return RET_SUCCESS;
}

/* Exit Multiview */
static RET_CODE MultiView_Exit()
{
	UINT8 ret_val;
	struct VDecPIPInfo  PIPInitInfo;
	struct MPSource_CallBack MPCallBack;
	struct PIPSource_CallBack PIPCallBack;
	UINT32 cmd = 0;
	
	if(hde_get_mode() != VIEW_MODE_MULTI)
	{
		return RET_FAILURE;
	}
	else
	{
		MV_DEBUG("MV______MultiView_Exit\n");
		
		cmd |= (CC_CMD_CLOSE_VPO | CC_CMD_FILL_FB);
		cc_stop_channel(cmd, l_mv_es[uCurrentActiveIndex], l_mv_dev_list[uCurrentActiveIndex]);
	
		ret_val = vdec_set_output(l_mv_dev_list[uCurrentActiveIndex]->vdec, (enum VDecOutputMode)MP_MODE, &PIPInitInfo, &MPCallBack, &PIPCallBack);
		vpo_win_mode(l_mv_dev_list[uCurrentActiveIndex]->vpo, VPO_MAINWIN, &MPCallBack, &PIPCallBack);
		vpo_win_onoff(l_mv_dev_list[uCurrentActiveIndex]->vpo, FALSE);
		
		osal_task_sleep(100);
    
		hde_set_mode(VIEW_MODE_FULL);
    
   		l_mv_status=MV_IDLE;
		return RET_SUCCESS;
	}
}

/* Draw One Special Box */
static RET_CODE MultiView_DrawOneBox(UINT16 uActiveIndex)
{
	UINT32 uindex;
	UINT32 boxStatus;
	UINT32 cmd = 0;
	UINT16 prog_pos = 0xffff;

	if(hde_get_mode() != VIEW_MODE_MULTI)
	{
		MV_DEBUG("Alert!!! Not in multiview mode\n");	
		return RET_FAILURE;
	}
	if(g_bPlayInSpecialWin)
	{
		struct Rect PIPWin;
		PIPWin.uStartX = l_MVinitPara.puBoxPosi[uCurrentActiveIndex].uX;
		PIPWin.uStartY = l_MVinitPara.puBoxPosi[uCurrentActiveIndex].uY;
		PIPWin.uWidth = l_MVinitPara.tPipSize.uWidth;
		PIPWin.uHeight = l_MVinitPara.tPipSize.uHeight;			
		vdec_extrawin_store_last_pic(l_mv_dev_list[uCurrentActiveIndex]->vdec, &PIPWin);	
	}
	
	boxStatus = BOX_PLAY_NORMAL;
	//1Switch to the special box	
	if(l_MVinitPara.callback != NULL)
	{
		boxStatus = l_MVinitPara.callback(uActiveIndex);
	}
	cc_stop_channel(cmd, l_mv_es[uCurrentActiveIndex], l_mv_dev_list[uCurrentActiveIndex]);
	MW_SwitchPIPWin(&l_MVinitPara.puBoxPosi[uActiveIndex], l_mv_es[uActiveIndex], l_mv_ft[uActiveIndex], l_mv_dev_list[uActiveIndex], boxStatus, 1);
	if(l_MVinitPara.callback != NULL)
	{
		(l_MVinitPara.callback)(uActiveIndex + BOX_FINISHED);
	}
	uCurrentActiveIndex = uActiveIndex;
	return RET_SUCCESS;
}

/* Implement Multiview Command */
static void mv_cmd_implement_wrapped(struct libpub_cmd *cmd)
{
	struct MultiViewPara *mvPara = NULL;
	int i;

	switch(cmd->cmd_type)
	{
	case LIBPUB_CMD_MV_ENTER:
		MV_DEBUG("A message to enter!.\n");
		MV_DEBUG("After Rcv MultiviewEnter Cmd. Address is 0x%x\n", cmd->para1);
		mvPara = (struct MultiViewPara*)cmd->para1;
		set_mv(LIBPUB_FLG_MV_BUSY);
		clear_mv(LIBPUB_FLG_MV_CANCEL);
		MultiView_Enter(cmd->cmd_bit, mvPara);
		clear_mv(LIBPUB_FLG_MV_BUSY | LIBPUB_FLG_MV_CANCEL);
 		break;
	case LIBPUB_CMD_MV_DRAWONEBOX:
		MV_DEBUG("A message to drawonebox!.\n");		
		set_mv(LIBPUB_FLG_MV_BUSY);
		clear_mv(LIBPUB_FLG_MV_CANCEL);
		MultiView_DrawOneBox((UINT16)(cmd->para1));
		clear_mv(LIBPUB_FLG_MV_BUSY | LIBPUB_FLG_MV_CANCEL);
 		break;
	case LIBPUB_CMD_MV_EXIT:	
		MV_DEBUG("A message to exit!.\n");		
		set_mv(LIBPUB_FLG_MV_BUSY);
		clear_mv(LIBPUB_FLG_MV_CANCEL);
		MultiView_Exit();
		clear_mv(LIBPUB_FLG_MV_ACTIVE | LIBPUB_FLG_MV_BUSY | LIBPUB_FLG_MV_CANCEL | LIBPUB_FLG_MV_EXIT);                    
 		break;
	default:
		ASSERT(0);
	}
}


/*******************************************************
* external API
********************************************************/
__ATTRIBUTE_REUSE_
/* API for Multiview Initialization: Register Command and Initialize Local Struct */
void mv_init(void)
{
	int i;
	BOOL ret_bool;
	ret_bool = libpub_cmd_handler_register(LIBPUB_CMD_MV,mv_cmd_implement_wrapped);
	if(!ret_bool)
	{
		MV_DEBUG("libpub_cmd_handler_register failed.\n");
	}
	ret_bool = libpub_overlay_cmd_register(LIBPUB_CMD_MV_ENTER);
	if(!ret_bool)
	{
		MV_DEBUG("libpub_overlay_cmd_register LIBPUB_CMD_MV_ENTER failed.\n");
	}
	ret_bool = libpub_overlay_cmd_register(LIBPUB_CMD_MV_DRAWONEBOX);
	if(!ret_bool)
	{
		MV_DEBUG("libpub_overlay_cmd_register LIBPUB_CMD_MV_DRAWONEBOX failed.\n");
	}
	ret_bool = libpub_overlay_cmd_register(LIBPUB_CMD_MV_EXIT);
	if(!ret_bool)
	{
		MV_DEBUG("libpub_overlay_cmd_register LIBPUB_CMD_MV_EXIT failed.\n");
	}

	for(i=0; i<MV_BOX_NUM; i++)
	{
		l_mv_es[i] = &(gMvpara.mv_param[i].es);
		l_mv_dev_list[i]= &(gMvpara.mv_param[i].device_list);
		l_mv_ft[i] = &(gMvpara.mv_param[i].ft);
	}
}

/* API for Wait Exit Multiview Flag */
BOOL get_mv_exit(void)
{
	return get_mv(LIBPUB_FLG_MV_EXIT);
}

/* API for Init Before Enter Multiview */
void UIMultiviewInit(struct MVInitPara *tMVinitPara, enum MVMode mvMode)
{
	int i,j;
	enum MVMode tmvMode = mvMode;
	UINT8 col = 0, row = 0;

	g_bPlayInSpecialWin = FALSE;
	MEMCPY(&g_MVinitPara, tMVinitPara, sizeof(struct MVInitPara));
	if(mvMode == MV_AUTO_BOX)
	{
		if(tMVinitPara->uBoxNum <= 9)
		{
			tmvMode = MV_9_BOX;
		}
		else if(tMVinitPara->uBoxNum > 9 || tMVinitPara->uBoxNum <= 16)
		{
			tmvMode = MV_16_BOX;
		}
		else if(tMVinitPara->uBoxNum > 16)
		{
			tmvMode = MV_25_BOX;
		}
	}
		
	switch(tmvMode)
	{
		case MV_4_BOX:
			col = 2, row = 2;
			break;
		case MV_9_BOX:
			col = 3, row = 3;
			break;
		case MV_16_BOX:
			col = 4, row = 4;
			break;
		case MV_25_BOX:
			col = 5, row = 5;
			break;
		default:
			ASSERT(FALSE);
	}
	
	if(g_MVinitPara.tPipSize.uWidth < 200)
	{
		uPrecision= 8;
	}
	else
	{
		uPrecision = 4;
	}
	
	l_dview_h_precision = (uPrecision > ((VDEC27_DVIEW_PRECISION >> 16) & 0xFF)) ? uPrecision : ((VDEC27_DVIEW_PRECISION >> 16) & 0xFF);
	l_dview_v_precision = (uPrecision > (VDEC27_DVIEW_PRECISION & 0xFF)) ? uPrecision : (VDEC27_DVIEW_PRECISION & 0xFF);

#ifndef VIDEO_SUPPORT_EXTRA_DVIEW_WINDOW
	if(l_dview_h_precision == 4)
	{
		for(j = 0; j < col; j++)
		{
			for(i = 0; i < row; i++)
			{
				g_MVinitPara.puBoxPosi[i + j * row].uX = 176 * i + g_MVinitPara.puBoxPosi[0].uX;
			}
		}
	}
	else
	{
		for(j = 0; j < col; j++)
		{
			for(i = 0; i < row; i++)
			{
				g_MVinitPara.puBoxPosi[i + j * row].uX = 80 * i + g_MVinitPara.puBoxPosi[0].uX;
				g_MVinitPara.puBoxPosi[i + j * row].uY = 72 * j + g_MVinitPara.puBoxPosi[0].uY;
			}
		}
	}		
#endif
	MV_DEBUG("UIMultiviewInit-----leave!.\n");
}

/* API for Extenstion Init Before Enter Multiview */
void UIMultiviewInitExt(struct MVInitPara *tMVinitPara, enum MVMode mvMode, BOOL bPlayInSpecialWin)
{
	UIMultiviewInit(tMVinitPara, mvMode);
	g_bPlayInSpecialWin = bPlayInSpecialWin;	
}

/* API for Enter Multiview */
void UIMultiviewEnter(struct MultiViewPara *mvPara)
{
	UINT32 flgptn;
	struct libpub_cmd cmd;   

	libpub_clear_message_queue();
	flgptn = LIBPUB_FLG_MV_ACTIVE;
	if(get_mv(LIBPUB_FLG_MV_BUSY))
	{
		flgptn |= LIBPUB_FLG_MV_CANCEL;
	}
	set_mv(flgptn);
	MEMCPY(&l_MVinitPara,&g_MVinitPara,sizeof(struct MVInitPara));
	
  	MEMSET(&cmd, 0, sizeof(struct libpub_cmd));
	cmd.cmd_type=LIBPUB_CMD_MV_ENTER;
	cmd.sync = 1;
	cmd.para1 = (UINT32)mvPara;
	while(libpub_send_cmd(&cmd,OSAL_WAIT_FOREVER_TIME) != SUCCESS);
	MV_DEBUG("Send MultiviewEnter Cmd Successfully!.\n");
	
	if(cmd.sync)
	{
		libpub_wait_cmd_finish(LIBPUB_FLG_MV_ENTER, OSAL_WAIT_FOREVER_TIME);
	}
		
	l_mv_status = MV_ACTIVE|MV_SCAN_SCREEN|MV_SCAN_BOX;
	MV_DEBUG("UIMultiviewEnter -----leave!.\n");
}

/* API for Draw One Special Box */
BOOL UIMultiviewDrawOneBox(UINT16 uActiveIndex)
{
	struct libpub_cmd cmd;
	UINT32 flgptn;
	UINT8 times=0;

	libpub_clear_message_queue();
	flgptn = LIBPUB_FLG_MV_ACTIVE;
	if(get_mv(LIBPUB_FLG_MV_BUSY))
	{
	    	flgptn |= LIBPUB_FLG_MV_CANCEL;
	}
	set_mv(flgptn);

	MEMSET(&cmd, 0, sizeof(struct libpub_cmd));
	cmd.cmd_type = LIBPUB_CMD_MV_DRAWONEBOX;
	cmd.para1 = uActiveIndex;
	
	libpub_send_cmd(&cmd, OSAL_WAIT_FOREVER_TIME);
	
	return TRUE;
}

/* API for Exit Multiview */
void UIMultiviewExit()
{
	UINT32 flgptn;
	struct libpub_cmd cmd;
	
	libpub_clear_message_queue();

	flgptn = LIBPUB_FLG_MV_ACTIVE  | LIBPUB_FLG_MV_EXIT;
	if(get_mv(LIBPUB_FLG_MV_BUSY))
	{
		flgptn |= LIBPUB_FLG_MV_CANCEL;
	}
	set_mv(flgptn);

	MEMSET(&cmd, 0, sizeof(struct libpub_cmd));
	cmd.cmd_type = LIBPUB_CMD_MV_EXIT;
	
	libpub_send_cmd(&cmd, OSAL_WAIT_FOREVER_TIME);
	
	while(get_mv(LIBPUB_FLG_MV_ACTIVE))
	{
		osal_task_sleep(10);	
	}
}

/* API for Get Multiview Status */
UINT8 UIMultiviewGetStatus(void)
{
	return	l_mv_status;
}

/* API for Wait Cancel Multiview Flag */
BOOL get_mv_cancel(void)
{
    	return get_mv(LIBPUB_FLG_MV_CANCEL);
}
