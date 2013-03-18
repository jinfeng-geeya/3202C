//本文件定义CA模块实现提供给STB调用的接口

#ifndef _DVTCAS_STBINTERFACE_H_2004_12_31_
#define _DVTCAS_STBINTERFACE_H_2004_12_31_

#include "DVTCAS_STBDataPublic.h"

#ifdef  __cplusplus
extern "C" {
#endif 

/*-----------------------------------以下接口是CAS提供给STB调用----------------------------------*/

/*++
功能：机顶盒启动的时候调用，对CAS模块初始化。
返回值：
	true				初始化成功。
	false				初始化失败。
--*/
bool DVTCASTB_Init(void);

/*++
功能：机顶盒获得CASID后调用该接口判断CASID是否有效，如果有效则返回true，否则返回false。
参数：
	wCaSystemID:		机顶盒分析SI/PSI信息获得的CASID。
返回值：
	true				CASID有效
	false				CASID无效
--*/
bool DVTCASTB_CASIDVerify(WORD wCaSystemID);

/*++
功能：当用户插卡时机顶盒主程序调用，通知CAS模块
参数：
	byReaderNo:			读卡器的编号
--*/
void DVTCASTB_SCInsert(BYTE byReaderNo);

/*++
功能：当用户拔出智能卡时机顶盒主程序调用，通知CAS模块。
--*/
void DVTCASTB_SCRemove(void);

/*++
功能：设置当前正在收看的节目的详细信息。
参数：
	pServiceInfo:		SDVTCAServiceInfo结构，描述节目的详细信息。
说明：
	当用户换频道时，机顶盒必须要调用此函数。如果当前频道为不加扰的频道，传过来的m_wEcmPid设为0。
--*/
void DVTCASTB_SetCurEcmInfo(const SDVTCAServiceInfo * pServiceInfo);

/*++
功能：设置EMMPID，CAS模块将根据设置的EMMPID信息进行EMM数据接收处理。
参数：
	wEmmPid:			CAT表中包含的descriptor中将有CA_system_ID及CA_PID，调用DVTCASTB_CASIDVerify()校验有效的CA_PID即为EMM PID。
说明:
	当机顶盒收到CAT表后，机顶盒需要调用此函数，将EMM PID传过来。
--*/
void DVTCASTB_SetEmmPid(WORD wEmmPid);

/*++
功能：机顶盒主程序在收到CAS私有数据或者Time_Out，调用这个接口将数据提供给CAS模块处理。
参数：
	byReqID：				当前请求收取私有表的请求号，与DVTSTBCA_SetStreamGuardFilter里的byReqID对应
	bSuccess：				收取数据有没有成功，成功为true，失败为false
	wPID:					接收的数据的流的PID。
	byszReceiveData：		收取的私有数据
	wLen：					收取到的私有数据的长度
--*/
void DVTCASTB_StreamGuardDataGot(BYTE byReqID, bool bSuccess, WORD wPID, WORD wLen, const BYTE * byszReceiveData);

/*++
功能：判断智能卡的PIN码
参数：
	pPin:					PIN码
返回值：
	DVTCA_OK          				成功
	DVTCAERR_STB_PIN_ERR			输入PIN码错误
	DVTCAERR_STB_PIN_LOCKED			PIN被锁定
--*/
HRESULT DVTCASTB_VerifyPin(const SDVTCAPin * pPIN);

/*++
功能：判断卡是否被锁住。
参数：
	pbLocked:				输出参数，true表示锁住了，false表示没有。
返回值：
	DVTCA_OK				成功
	其他					失败
--*/
HRESULT DVTCASTB_IsPinLocked(bool * pbLocked);

/*
功能：判断用户是否通过pin码验证。
参数：
	pbPass:				输出时true表示通过了了；false，表示没有。
*/
HRESULT DVTCASTB_IsPinPass(bool * pbPass);

/*++
功能：修改智能卡的PIN码。
参数：
	pNewPin:				新的PIN码。	
返回值：
	DVTCA_OK          				成功
	DVTCAERR_STB_PIN_LOCKED			PIN被锁定
说明:
	调用该函数之前要求确保调用过VerifyPin，并且成功。
--*/
HRESULT DVTCASTB_ChangePin(const SDVTCAPin * pNewPin);

/*++
功能：获得智能卡目前最高观看级别。
参数：
	pbyRating:				保存观看级别。
返回值：
	DVTCA_OK				成功
	其他					失败
说明:
	观看级别为3~18，共15级。初始值为18，即可以观看所有级别的节目。
--*/
HRESULT DVTCASTB_GetRating(BYTE * pbyRating);

/*++
功能：设置智能卡观看级别。
参数：
	byRating:				要设置的新的观看级别
返回值：
	DVTCA_OK				成功
	其他					失败
说明:
	调用该函数之前要求确保调用过VerifyPin，并且成功。
--*/
HRESULT DVTCASTB_SetRating(BYTE byRating);

/*++
功能：获得智能卡当前设置的工作时间段。
参数：
	pbyStartHour			开始工作的小时
	pbyStartMinute			开始工作的分钟
	pbyEndHour				结束工作的小时
	pbyEndMinute			结束工作的分钟
返回值：
	DVTCA_OK				成功
	其他					失败
说明:
	工作时段初始值为00:00~23:59，即全天都能收看。
--*/
HRESULT DVTCASTB_GetWorkTime(BYTE * pbyStartHour, BYTE * pbyStartMinute, BYTE * pbyEndHour, BYTE * pbyEndMinute);

/*++
功能：设置智能卡工作时段。
参数：
	byStartHour			开始工作的小时
	byStartMinute			开始工作的分钟
	byEndHour				结束工作的小时
	byEndMinute			结束工作的分钟
返回值：
	DVTCA_OK				成功
	其他					失败
说明:
	调用该函数之前要求确保调用过VerifyPin，并且成功。
--*/
HRESULT DVTCASTB_SetWorkTime(BYTE byStartHour, BYTE byStartMinute, BYTE byEndHour, BYTE byEndMinute);


/*++
功能：获得CAS供应商信息。
参数：
	pManu					指向保存返回信息的数组地址。
返回值：
	DVTCA_OK				成功
	其他					失败
--*/
HRESULT DVTCASTB_GetStreamGuardManuInfo(SDVTCAManuInfo * pManu);

/*++
功能：获得CAS运营商信息。
参数：
	wTVSID:					CAS运营商的ID  如果是－1 则返回所有的运营商信息
	pOperatorInfo:			指向保存返回信息的数组地址
	pbyCount:				传进来的是数组的长度，返回时是实际返回运营商信息的个数
返回值：
	DVTCA_OK				成功
	其他					失败
--*/
HRESULT DVTCASTB_GetOperatorInfo(WORD wTVSID, BYTE * pbyCount, SDVTCATvsInfo * pOperatorInfo);

/*++
功能：获得普通授权节目购买的情况，用于在机顶盒界面上显示。
参数：
	wTVSID:					运营商的内部编号。
	pEntitleCount:			输入时为数组的最大个数，输出时为实际获得的个数。
	psEntitles:				返回的授权数组。						
返回值：
	DVTCA_OK					成功
	DVTCAERR_STB_TVS_NOT_FOUND	没有找到符合要求的运营商
	其他						失败
--*/
HRESULT DVTCASTB_GetServiceEntitles(WORD wTVSID, BYTE * pEntitleCount, SDVTCAServiceEntitle * psEntitles);

/*++
功能：打开或关闭调试信息。
参数：
	bDebugMsgSign:			打开调试信息标志，1:打开调试信息;0:关闭调试信息。
--*/
void DVTCASTB_AddDebugMsgSign(bool bDebugMsgSign);

/*
功能：机顶盒显示完一个OSD消息后，用这个接口来通知CA模块。
参数:	
	wDuration:				消息显示的具体时间，单位为秒。
--*/
void DVTCASTB_ShowOSDMsgOver(WORD wDuration);

/*++
功能:机顶盒获取E-mail个数接口。
参数:
	*pEmailCount:			机顶盒中E-mail总个数。
	*pNewEmailCount:		机顶盒中未读过的E-mail个数。
返回值：
	DVTCA_OK				成功
	其他					失败
--*/
HRESULT DVTCASTB_GetEmailCount(WORD * pEmailCount, WORD * pNewEmailCount);

/*++
功能:机顶盒获取多个E-mail简要信息接口。
参数:
	*pEmailCount:			输入时为数组总个数，输出时为机顶盒中E-mail总个数。
	*pEmail:				获取的E-mail数组。
返回值:
	DVTCA_OK:				获取E-mail成功。
	DVTCAERR_STB_EMAILBOX_EMPTY:	E-mail信箱为空。
--*/
HRESULT DVTCASTB_GetEmailHead(WORD * pEmailCount, SDVTCAEmailInfo * pEmail);

/*++
功能:机顶盒获取一个E-mail的内容。
参数:
	tVersion:				E-mail版本。
	*pEmail:				获取的E-mail结构。
返回值:
	DVTCA_OK:				获取E-mail成功。
	DVTCAERR_STB_NO_EMAIL:	没有此E-mail。
--*/
HRESULT DVTCASTB_GetEmailContent(DWORD tVersion, SDVTCAEmailContent * pEmail);

/*++
功能:机顶盒看完E-mail后，通知CAS的接口。
参数:
	tVersion:				E-mail版本。
返回值:
	DVTCA_OK:				设置E-mail为已读成功。
	其他					失败
--*/
HRESULT DVTCASTB_EmailRead(DWORD tVersion);

/*++
功能:机顶盒删除E-mail接口。
参数:
	tVersion:				E-mail版本。如果为-1，表示删除邮箱中所有email，否则，为具体的id。
返回值:
	DVTCA_OK:				删除E-mail成功。
	DVTCAERR_STB_NO_EMAIL:	要删除的E-mail不存在。
	其他					失败
--*/
HRESULT DVTCASTB_DelEmail(DWORD tVersion);

/*
功能：读出母卡配对信息。
参数：
	pLen:					输入时为最大的数据长度，输出时，为实际的输出长度。
	pData:					输出的数据。
返回值：
	DVTCA_OK				成功
	其他					失败
*/
HRESULT DVTCASTB_GetCorrespondInfo(BYTE * pLen, BYTE * pData);

/*
功能：设置子卡配对信息
参数：
	DataLen:				输入的数据长度。
	pData:					输入的数据。此数据是用DVTCASTB_GetCorrespondInfo得到数据。
返回值：
	DVTCA_OK				成功
	其他					失败
*/
HRESULT DVTCASTB_SetCorrespondInfo(BYTE DataLen, const BYTE * pData);

//--------------------------------------------------------ipp begin----------------------------------------------------

/*
功能：得到所有可预订/退订的Ipp节目。
参数：
	pbyCount:				输入为数组指针指向的数组可容纳节目个数；输出为得到的节目个数。
	pIppvs:					ipp节目数组指针。
返回值：
	DVTCA_OK				成功
	其他					失败
*/
HRESULT DVTCASTB_GetBookIpps(BYTE * pbyCount, SDVTCAIpp * pIpps);

/*
功能：预订Ipp节目
参数：
	pIpp:						要预定并确认购买的ipp节目信息。
	
返回值：
	DVTCA_OK					成功
	DVTCAERR_STB_MONEY_LACK		钱不多，需要提示用户"预订成功，但余额不多，请及时充值"
	DVTCAERR_STB_NEED_PIN_PASS	需要通过PIN验证
	DVTCAERR_STB_IC_COMMUNICATE	与IC卡通讯参数错误
	DVTCAERR_STB_TVS_NOT_FOUND	运营商ID无效
	DVTCAERR_STB_SLOT_NOT_FOUND	钱包没有发现
	DVTCAERR_STB_VER_EXPIRED	产品已经过期
	DVTCAERR_STB_OPER_INVALID	产品已经存在，不能操作
	DVTCAERR_STB_NO_SPACE		没有空间
	DVTCAERR_STB_PROD_NOT_FOUND	产品没有发现
	DVTCAERR_STB_PRICE_INVALID	价格无效
	DVTCAERR_STB_UNKNOWN		未知错误
*/
HRESULT DVTCASTB_BookIpp(const SDVTCAIpp * pIpp);

/*
功能：得到所有已观看的Ipp节目。
参数：
	pbyCount:				输入为数组指针指向的数组可容纳节目个数；输出为得到的节目个数。
	pIpps:					ippv节目数组指针。
返回值：
	DVTCA_OK				成功
	其他					失败
*/
HRESULT DVTCASTB_GetViewedIpps(BYTE * pbyCount, SDVTCAViewedIpp * pIpps);

/*++
功能：当退出提示用户预订IPP产品的框时，机顶盒调用此接口通知CA。
参数：
	wEcmPid:				IPP对应的EcmPID
--*/
void DVTCASTB_InquireBookIppOver(WORD wEcmPid);

//--------------------------------------------------------ipp end----------------------------------------------------

/*
功能：得到某个运营商的已花费钱数，剩余钱数。
参数：
	wTVSID:					运营商的内部编号。
	pdwAllBalance:			输出已花费钱数，单位分。
	pdwRemainder:			输出剩余钱数，单位分。
返回值：
	DVTCA_OK				成功
	其他					失败
*/
HRESULT DVTCASTB_GetMoneyInfo(WORD wTVSID, DWORD * pdwAllBalance, DWORD * pdwRemainder);

/*
功能：通知CA模块，DVTSTBCA_SwitchChannel完毕。(单频点区域锁定使用)
参数：
	flag==1:超时
	flag==2:没有默认频点、或者没有节目
*/
void DVTCASTB_SwitchChannelOver(BYTE byFlag);

/*
功能: 获取区域信息
参数:
	psAreaInfo				区域信息
返回值:
	0:						成功
	其他值:					失败
*/	
HRESULT DVTCASTB_GetAreaInfo(SDVTCAAreaInfo * psAreaInfo);//weiye:2007.05.24

/*
功能:获取用户自定义数据
参数:
	byType					自定义数据类型
	pdwUserDefData			用户自定义数据的DWORD类型指针
返回值:
	0:						成功
	其他值:					失败
*/
HRESULT DVTCASTB_GetUserDefData(BYTE byType, DWORD *pdwUserDefData);

/*
功能:切换提示语言，机顶盒CA模块初始化后调用
参数:
	byLanguage               语言类型DVTCA_LANG_CHN_SIM为简体中文(默认) 	DVTCA_LANG_ENG为英文
*/
void DVTCASTB_ChangeLanguage(BYTE byLanguage);

/*
功能:得到子母卡配对的信息
参数:
	pdwMotherCardID    获取母卡卡号，当pdwMotherCardID的值为0，则表示当前卡为母卡， 如果值不为0，表示当前卡为子卡，值为母卡卡号。
	                   
返回值:
	0:						成功
	其他值:					失败
*/
HRESULT DVTCASTB_GetMotherInfo(DWORD *pdwMotherCardID);


/*----------------------------------以上接口是CAS提供给STB调用--------------------------------------------*/

#ifdef  __cplusplus
}
#endif
#endif  

