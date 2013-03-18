/*****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2004 Copyright (C)
 *
 *  Filename: Vdec_driver.h
 *
 *  Contents: 	This file gives out the video decode module interface
 *
 *  History:
 *		Date		By      			Reason
 *		======	====	==================	===================
 *  1.  13/05/2004  Lei Wang     		Create it
 *
 *****************************************************************************/
 
 
#ifndef __VDEC_DRIVER_H__
#define __VDEC_DRIVER_H__

#include <basic_types.h>
#include <mediatypes.h>

#include <sys_config.h>

#include "decv.h"

//#include <../../../src/app/test/M3327VER/avtest/ldscript.ld>
//#define	VDEC_DROP_B	


#ifndef USE_NEW_VDEC

#define	VDEC_DECODE_ALL			0x01
#define	VDEC_DECODE_IP			0x02



#if(WINDOW_MODE==WIN_MP_PIP)
#define	VDEC_TEST_DVIEW
#endif

//2Old Version : Dbg Info-b
#ifdef	FUNC_MONITOR
//#define	VDEC_FUNC_PRINTF	DEBUGPRINTF
#define	VDEC_FUNC_PRINTF(...)

#else
#define	VDEC_FUNC_PRINTF(...)
#endif

//#define	VDEC_USE_TDS2

#define	VDEC_DGB_ORDER
#ifdef	VDEC_DGB_ORDER
#define	VDEC_ORDER_MONITOR	DEBUGPRINTF
#endif

void vdec_dbg_asm(UINT8 uEntryType,UINT8 uEntryPosi);
//Rache Run Monitor
#ifdef VDEC_RUN_MONITOR
#define	VDEC_IN_FUNCTION		0x01
#define	VDEC_LEAVE_FUNCTION	0x02

//Entry Indnex
#define	VDEC_ENTRY_FINISH			0x01
#define	VDEC_ENTRY_FEEDDATA		0x02
#define	VDEC_ENTRY_REQUEST		0x03
#define	VDEC_ENTRY_UPDATA		0x04
#define	VDEC_ENTRY_RELEASE		0x06
#define	VDEC_ENTRY_LINEMEET		0x07
#define	VDEC_ENTRY_VBLANK			0x08

//end
//Process Index
//Finish Process
#define	VDEC_IN_FINISH				0
#define	VDEC_IN_FINISH_STOP		1

#define	VDEC_TRY_DECFRAME		2
#define	VDEC_BE_DECODING_FL		3
#define	VDEC_FIRCNT_FL				4
#define	VDEC_HWBUSY_FL			5
#define	VDEC_IN_DECFRAME			6
	
#define	VDEC_TRY_HEADER			7
#define	VDEC_HEADER_OK			8
#define	VDEC_HEADER_FL			9

#define	VDEC_B_REFCNT_FL			10

#define	VDEC_B_BINDQ_FL			11

#define	VDEC_DATACNT_FL			12

#define	VDEC_GETFRM_OK			13
#define	VDEC_GETFRM_FL			14

#define	VDEC_TRY_DECDATA			15

#define	VDEC_CONFIGHW				16
#define	VDEC_STARTBYTE			17
#define	VDEC_DE_VE_SAMEBUF		18

#define	VDEC_DECDATA_OK			19
#define	VDEC_DECDATA_FL			20

//FeedData Process
#define	VDEC_TRY_FEEDDATA		21
#define	VDEC_FEEDDATA_OK			22
#define	VDEC_FEEDDATA_FL			23
	
//No used. Just for old version *.c
#define	VDEC_TRY_GETFRM			0x0a
#define	VDEC_TRY_DMXUPDATE		0x12
#define	VDEC_TRY_DMXREQ			0x13
#define	VDEC_DMXREQ_OK			0x14
#define	VDEC_DMXREQ_FL			0x15


// end

//Status Index
#define	VDEC_DBG_IDLE	0
#define	VDEC_DBG_RUN	1
#define	VDEC_DBG_IDLE2RUN	2
#define	VDEC_DBG_RUN2IDLE	3
#define	VDEC_DBG_STOP		4
//end
#define	VDEC_MAX_PROCESS			15
#define	VDEC_MAX_ENTRY			300

struct VDec_ConfigInfo
{
	UINT32  VLDByteStartAddr		;
	BOOL	DataIsValid			;
	BOOL	isLastPart			;
	BOOL	isFirstPart			;
	UINT32 VLDBitOffset			;
	UINT32 VLDBitLength			;
	UINT32 VLDByteEndAddr		; 
	
};

struct VDec_Monitor
{
	UINT8	uEntryType;
	BOOL	bConfigHW;
	UINT8	aProcess[VDEC_MAX_PROCESS];
	struct 	VDec_ConfigInfo		tConfigInfo;

	UINT32	uMoni_Index;
	UINT8	uVdec_Status;

	UINT8	uISRCause;
	BOOL	bStartNewDecode;
	UINT8	uProcessCount;
	
	UINT32	uEn_HWOffset;
	UINT32	uEn_HWLength;
	UINT32	uEn_WritePoint;
	UINT32	uEn_ReadPoint;
	UINT32	uEn_CurPoint;
	UINT32	uEn_Using;
	UINT32	uEn_Empty;

	UINT32	uEx_HWOffset;
	UINT32	uEx_HWLength;
	UINT32	uEx_WritePoint;
	UINT32	uEx_ReadPoint;
	UINT32	uEx_CurPoint;
	UINT32	uEx_Using;
	UINT32	uEx_Empty;
};

extern void vdec_dbg_PrintfMonitor(UINT16 uLatestEntryCount);
extern void vdec_dbg_RecProcess(UINT8 uProcessIndex);

extern void vdec_update_entry(void);
extern void vdec_finish_entry(UINT8 uISRCause);
extern void vdec_feeddata_entry(UINT8 uISRCause);
extern void vdec_release_entry(void);
#endif

//2Old Version : Dbg Info-end


#define FOR_MERGE_DMX
/******************************************************************************************
*
*			Video Decoder Driver Data Structures
*
*******************************************************************************************/

#define	VDEC_ON_M3327		0x01
#define	VDEC_ON_M3357		0x02
#define	VDEC_PROJECT		VDEC_ON_M3327
#ifdef FOR_MERGE_DMX
#if(VDEC_PROJECT==VDEC_ON_M3327)
#define	CONTROLBIT_STDID		0x01
#define	CONTROLBIT_PTSVALID	0x02
#define	CONTROLBIT_VOBUVALID	0x10
#elif(VDEC_PROJECT==VDEC_ON_M3357)
#define	CONTROLBIT_STDID		0x01
#define	CONTROLBIT_PTSVALID	0x04
#define	CONTROLBIT_NUMTIMEVALID	0x08
#define	CONTROLBIT_VOBUVALID	0x10
#endif
#endif



// Video setting from high layer
struct VidSetting
{
	enum VidSpecFmt	eVidSpecFmt;
	enum FrameRate 	eFrameRate;
	enum TVSystem	eTVSystem;
	enum AspRatio	eAspRatio;
	UINT16	uPicWidth;
	UINT16	uPicHeight;
};


// enum type to specify which VOBU to request
enum VDecVobuReq
{
	REQ_NEXT_VOBU,	// request next VOBU
	REQ_CURRENT_VOBU,	// request current VOBU	
	REQ_PREV_VOBU		// request previous VOBU
};



enum VDecStopType	
{
	VDEC_STOP_IM = 0,		// Stop immediately after decoding current picture (if some).
	VDEC_STOP_BY_FRAME,	// Stop when the picture with desired frame (specified by uStopVobuNum and uStopPicNum) was decoded.
	VDEC_STOP_AFTER,		// Stop after all data in buffer was processed. This means there should be no more data will be sent.
	VDEC_STOP_RESET
};

// Vdec command, how to decode the video data
enum VDecCmd
{
	ALL_PIC = 0,	// decode all pictures
	SKIP_B,		// decode I, P pictures only
	SKIP_BP		// decode I pictures only
};

struct VDecDataCtrlBlk
{
	// Bit map to specify which parameter in the structure is valid: "1" valid, "0" not invlid .
	// Bit0: STC_ID section valid or not;
	// bit 1: Data discontinue or not
	// Bit2: PTS section valid or not;
	// Bit3: uVobuNum and uVobuBaseTime section valid or not.
	// Bit4: Video trick mode or not
	UINT8	uCtrlByte;
	UINT8	uStcId;
	UINT32	uPTS;
#ifdef FOR_MERGE_DMX
#if(VDEC_PROJECT == VDEC_ON_M3357)
	INT32	nVobuNum;
	UINT32	uVobuBaseTime;
#endif
#endif
};

struct VidFrameInfo
{
	INT32	nVobuNum;
	UINT8	uPicNum;
	UINT32	uFrameTime;
	UINT32	uPTS;
};



//~wl add
// ~wl

/******************************************************************************************
*
*			Video Decoder Driver APIs
*
*******************************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
enum Output_Frame_Ret_Code VDec_Output_Frame(struct Display_Info *pDisplay_info);
BOOL VDec_Release_FrBuf(UINT8 top_index, UINT8 bottom_index);


HANDLE VDec_Attach(char* pcName);
RET_CODE VDec_Detach(HANDLE hDev);
RET_CODE VDec_Open(HANDLE hDev, struct VidSetting	*pVidSetting);
RET_CODE VDec_Close(HANDLE hDev);
RET_CODE VDec_RegisterCB(HANDLE hDev, enum VDecCBType eCBType, VDecCBFunc pfCB);
RET_CODE VDec_Decode(HANDLE hDev,
						enum VDecCmd eDecCmd, 
						BOOL bSyncMode,
						UINT32 uTargetPic,
						BOOL bReportI,
						BOOL bPauseAtTargetFrame,
						BOOL bRptTargetFramePTS
						);
RET_CODE VDec_Reverse(HANDLE hDev, 
						enum VDecCmd eDecCmd, 
						BOOL bSyncMode,
						UINT32 uTargetFrame,
						UINT8 uRepeatTimes
						);
RET_CODE VDec_Pause(HANDLE hDev);
RET_CODE VDec_Step(HANDLE hDev, enum VDecDirection eVDecDirection);
RET_CODE VDec_Stop(HANDLE hDev,
						enum VDecStopType eStopType, 
						INT32 nStopVobuNum,  
						UINT32 uStopPicNum,
						BOOL bBlock
						);
RET_CODE VDec_SetDecCmd(HANDLE hDev,enum VDecCmd eDecCmd, BOOL bReportI );
RET_CODE VDec_SetSyncMode(HANDLE hDev, BOOL bSyncMode);
RET_CODE VDec_SetRepeatTimes(HANDLE hDev, UINT8 uRepeatTimes);
RET_CODE VDec_Reverse(HANDLE hDev, 
						enum VDecCmd eDecCmd, 
						BOOL bSyncMode,
						UINT32 uTargetPic,
						UINT8 uRepeatTimes);
RET_CODE VDec_DataEnd(HANDLE hDev);
#ifdef FOR_MERGE_DMX
RET_CODE decv_request_write(void * hDev, UINT32 uSizeRequested, 
	                                        void ** ppVData, UINT32 * puSizeGot,
	                                        struct control_block * ctrl_blk);
void decv_update_write(void * hDev,UINT32 uDataSize);
#else
RET_CODE decv_request_write(HANDLE hVDec,
						UINT32 uSizeRequested,
				
						UINT8** ppuData,
						UINT32* puSizeGot
						);
RET_CODE decv_update_write(HANDLE hVDec, UINT32 uDataSize);

#endif
RET_CODE VDec_GetCurFrameInfo(HANDLE hDev, struct VidFrameInfo *pFrameInfo);
RET_CODE VDec_Abort(HANDLE	hDev);
RET_CODE VDec_IoControl(HANDLE hDev, UINT32 uCmd, UINT32 uParam);

//wl add
RET_CODE VDec_SetOutput(enum VDecOutputMode eMode, 
						struct VDecPIPInfo *pInitInfo, // if MP_MODE, pls set NULL
						struct MPSource_CallBack *pMPCallBack, 
						struct PIPSource_CallBack *pPIPCallBack,
						BOOL bPullDownDetect,
						struct VDec_PullDown_Opr *pPullDownOpr);
RET_CODE VDec_SwitchPIPWin(struct Position *pPIPPos);
enum Output_Frame_Ret_Code VDec_Output_MP_Frame(struct Display_Info *pDisplay_info);
enum Output_Frame_Ret_Code VDec_Output_PIP_Frame(struct PIP_Dislay_Info *pDisplay_info);
BOOL VDec_Release_PIP_FrBuf(UINT8 buf_index);
void VDec_Set_Sync_Mode(HANDLE hDev, UINT8 uSyncMode);
void VDec_Set_Sync_Level(HANDLE hDec, UINT8 uSyncLevel);
void VDec_FillDisplayFrm(HANDLE hDev,struct YCbCrColor *pColor);
void VDec_GetStatusInfo(HANDLE hDev,struct VDec_StatusInfo *pCurStatus);
void VDec_Dbg_WhyUIdle(void);
//~wl add

#ifdef __cplusplus
}
#endif

#define vdec_m3327_attach()		VDec_Attach(0)
#define vdec_m3327_dettach(x1)	VDec_Detach(x1)

/*
#ifdef USE_NEW_VDEC
RET_CODE decv_request_write_2(void * hDev, UINT32 uSizeRequested, 
	                                        void ** ppVData, UINT32 * puSizeGot,
	                                        struct control_block * ctrl_blk);
void decv_update_write_2(void * hDev,UINT32 uDataSize);
enum Output_Frame_Ret_Code vdec_m3327_de_mp_request(struct Display_Info *pDisplay_info);
BOOL vdec_m3327_de_mp_release(UINT8 utop_idx,UINT8 ubot_idx);
enum Output_Frame_Ret_Code vdec_m3327_de_bg_request(struct Display_Info *pDisplay_info);
BOOL vdec_m3327_de_bg_release(UINT8 utop_idx,UINT8 ubot_idx);
enum Output_Frame_Ret_Code vdec_m3327_de_pip_request(struct PIP_Dislay_Info *pDisplay_info);
BOOL vdec_m3327_de_pip_release(UINT8 pip_idx);

#define	VDec_Attach(x) 		vdec_m3327_attach()
#define	VDec_Detach(x1)		vdec_m3327_dettach(x1)

#define	VDec_Open(x1, x2) 	decv_open(x1)
#define	VDec_Close(x1)		decv_close(x1)

#define	VDec_Decode(x1,x2,x3,x4,x5,x6,x7) decv_start(x1)
#define	VDec_Stop(x1,x2,x3,x4,x5) decv_stop(x1)
#define	VDec_Pause(x1)		decv_pause(x1)
#define	VDec_Resume(x1)		decv_resume(x1)

#define	VDec_SetOutput(x1,x2,x3,x4,x5,x6) decv_set_output(x1,x2,x3,x4)
#define	VDec_SwitchPIPWin(x1)		decv_switch_pip(0,x1)

#define	VDec_Set_Sync_Mode(x1, x2) decv_sync_mode(x1, x2)
#define	VDec_Set_Sync_Level(x1, x2) decv_sync_level(x1, x2)

#define	VDec_FillDisplayFrm(x1,x2) 	decv_fill_all_frm(x1, x2)
#define	VDec_RegisterCB(x1,x2,x3)	decv_register_cb(x1,x2,x3)
#define	VDec_GetStatusInfo(x1,x2)	decv_get_status(x1,x2)	
#define	VDec_Dbg_WhyUIdle()		decv_dbg_why_idle(0)
#define	VDec_GetSourceMode(x1,x2)	decv_get_src_mode(x1,x2)
#define	VDec_Get_Frm(x1,x2)		decv_get_frm(x1,x2)

//show logo call this function 
#define	decv_request_write(x1,x2,x3,x4,x5) 	decv_request_write_2(x1,x2,x3,x4,x5)
#define	decv_update_write(x1,x2)			decv_update_write_2(x1,x2)	
#endif
*/
#endif

#endif	//__VDEC_DRIVER_H__




