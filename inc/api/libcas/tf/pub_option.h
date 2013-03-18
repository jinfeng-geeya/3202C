/* ================================================================= 
	Version:		4.0.0
	Modify Date:		2007.09.29
	Author:			KFM
================================================================= */

#ifndef	_CALIB21_OPTION_H
#define _CALIB21_OPTION_H

/*-------------------------------------功能模块开关---------------------------------------*/
#define	INCLUDE_DETITLE			1		/*反授权确认码功能*/
#define	INCLUDE_IPPV_APP		1		/*IPPV功能*/
#define	INCLUDE_IPPT_APP		1		/*IPPT功能*/
#define	INCLUDE_ADDRESSING_APP		1		/*OSD和邮件功能*/
#define	INCLUDE_CHILDCARD_APP		1		/*子母卡功能*/
#define	INCLUDE_LOCK_SERVICE		1		/*强制切换频道功能*/
#define	INCLUDE_PATCHING_APP		1		/*智能卡升级功能*/
#define	 INCLUDE_SAFECONTROL		1		/*安全控制模块*/

#define	DEBUG_STB			1
#define	DEBUG_MUST_OUT			1

#define	SUPPORT_SCVER20			1
#define	SUPPORT_SCVER33_ONLY		0		/*仅适用安全芯片*/


#endif
