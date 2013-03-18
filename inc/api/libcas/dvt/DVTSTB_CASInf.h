//本文件定义要求机顶盒实现并提供给CA模块使用的接口。所有HRESULT返回值为DVTCA_OK都表示成功，其他值表示失败。

#ifndef _DVTSTB_CASINTERFACE_H_2004_12_31_
#define _DVTSTB_CASINTERFACE_H_2004_12_31_

#include "DVTCAS_STBDataPublic.h"

#ifdef  __cplusplus
extern "C" {
#endif 

/*----------------------------------以下接口是STB提供给CAS调用--------------------------------------------*/
/*++
功能：重置智能卡。
--*/
HRESULT DVTSTBCA_SCReset(void);

/*++
功能：获得机顶盒分配给CAS的flash空间的起点地址和大小（以字节为单位）。
参数：
	ppStartAddr:			机顶盒分配给CAS的flash空间的开始地址。
	lSize:					输出机顶盒分配给CAS的flash空间的大小。
--*/
HRESULT DVTSTBCA_GetDataBufferAddr(long * lSize, char ** ppStartAddr);

/*++
功能：读取保存在机顶盒flash中的信息。
参数：
	pStartAddr:				要读取的存储空间的开始地址。
	plDataLen:				输入为要读取的最长数据值；输出为实际读取的大小。
	pData:					存放输出数据。
--*/
HRESULT DVTSTBCA_ReadDataBuffer(const char * pStartAddr, long * plDataLen, BYTE * pData);

/*++
功能：向机顶盒的存储空间写信息。
参数：
	lStartAddr:				要写的存储空间的开始地址。
	plDataLen:				输入为要写的数据的长度；输出为写入的实际长度。
	pData:					要写的数据。
--*/
HRESULT DVTSTBCA_WriteDataBuffer(const char * pStartAddr, long * plDataLen, const BYTE * pData);

/*++
功能：读取保存在机顶盒eeprom中的信息。
参数：
	plDataLen:				输入为要读取的最长数据值；输出为实际读取的大小。
	pData:					存放输出数据。
--*/
HRESULT DVTSTBCA_GetDataFromEeprom(long * plDataLen, BYTE * pData);

/*++
功能：向机顶盒的eeprom存储空间写信息。
参数：
	plDataLen:				输入为要写的数据的长度；输出为写入的实际长度。
	pData:					要写的数据。
--*/
HRESULT DVTSTBCA_SaveDataToEeprom(long * plDataLen, const BYTE * pData);

/*++
功能：挂起当前线程。
参数：
	dwMicroseconds:			毫秒。
--*/
HRESULT DVTSTBCA_Sleep( int dwMicroseconds );

/*++
功能：初始化信号量,初始化后信号量没有信号。
参数：
	pSemaphore:				指向信号量的指针。
--*/
HRESULT DVTSTBCA_SemaphoreInit( DVTCA_Semaphore * pSemaphore );

/*++
功能：给信号量加信号。
参数：
	pSemaphore: 			指向信号量的指针。
--*/
HRESULT DVTSTBCA_SemaphoreSignal( DVTCA_Semaphore * pSemaphore );

/*++
功能：等待信号量,等待成功之后,信号量为无信号。
参数：
	pSemaphore: 			指向信号量的指针。
--*/
HRESULT DVTSTBCA_SemaphoreWait( DVTCA_Semaphore * pSemaphore );

/*++
功能：CAS模块向机顶盒注册线程。
参数：
	szName:					注册的线程名称。
	pTaskFun:				要注册的线程函数。
--*/
HRESULT DVTSTBCA_RegisterTask(const char * szName, pThreadFunc pTaskFun);

/*++
功能：设置过滤器接收CAS私有数据。
参数：
	byReqID:				请求收表的请求号。机顶盒应该保存该数据，接收到CAS私有数据后调用DVTCASTB_StreamGuardDataGot()时应该将该数据返回。
	wEcmPID:				需要过滤的流的PID。
	szFilter:				过滤器的值，为一个8个字节的数组。
	szMask:					过滤器的掩码，为一个8个字节的数组，与过滤器对应，当szMask的某一bit位为0时表示要过滤的表对应的bit位不需要与szFilter里对应的bit位相同，当szMask某一位为1时表示要过滤的表对应的bit位一定要与szFilter里对应的bit位相同。
	byLen:					过滤器的长度，为8。
	nWaitSeconds:			收此表最长的等待时间，如果为0则无限等待，单位秒。
--*/
HRESULT DVTSTBCA_SetStreamGuardFilter(BYTE byReqID, WORD wPID, const BYTE * szFilter, const BYTE * szMask, BYTE byLen, int nWaitSeconds);

/*++
功能：CA程序用此函数设置解扰器。将当前周期及下一周期的CW送给解扰器。
参数：
	wEcmPID:				CW所属的ECMPID。
	szOddKey:				奇CW的数据。
	szEvenKey:				偶CW的数据。
	byKeyLen:				CW的长度。
	bTapingControl:			true：允许录像,false：不允许录像。
--*/
HRESULT DVTSTBCA_SetDescrCW(WORD wEcmPID, BYTE byKeyLen, const BYTE * szOddKey, const BYTE * szEvenKey, bool bTapingControl);

/*++
功能：打印调试信息。
参数：
	pszMsg:					调试信息内容。
--*/
void DVTSTBCA_AddDebugMsg(const char * pszMsg);

/*++
功能：实现与智能卡之间的通讯。采用7816标准。
参数：
	byReaderNo:				对应的读卡器编号。
	pbyLen:					输入为待发送命令字节的长度;输出为返回字节长度。
	byszCommand:			待发送的一串命令字节。
	szReply:				返回的数据，空间长度恒为256字节。
--*/
HRESULT DVTSTBCA_SCAPDU(BYTE byReaderNo, BYTE * pbyLen, const BYTE * byszCommand, BYTE * byszReply);

/*--------------------------- 类型1的界面：不允许用户通过遥控器取消显示的界面 -----------------------*/

/*
功能：显示用户不能观看节目等提示信息，下面两个函数对应使用。
参数：	
	byMesageNo:				要显示的消息编号。
--*/
void   DVTSTBCA_ShowPromptMessage(BYTE byMesageNo);
void   DVTSTBCA_HidePromptMessage(void);

/*
功能：显示指纹信息。
参数：
	dwCardID:				卡号。
	wDuration:				持续秒数。
--*/
void	DVTSTBCA_ShowFingerPrinting(DWORD dwCardID, WORD wDuration);

#ifdef DVTCA_VERSION_NEW
/*
功能：CA模块用此函数通知机顶盒按CA前端的发送的OSD优先级（0~3）来有区别地显示普通、重要OSD。重要OSD用大号字体和特殊颜色显示。可以是同步显示，也可以是异步显示。
参数:	
	byPriority:	OSD优先级，0~3，优先级递增。
	szOSD:		用于显示的OSD信息。
说明：
	用户可以通过遥控器取消当前OSD显示。
--*/
void	DVTSTBCA_ShowOSDMsg(BYTE byPriority,const char * szOSD);

#else 
/*
功能：以左滚字幕的方式显示一次OSD消息，可以是同步显示，也可以是异步显示。
参数:	
	szOSD:					用于显示的OSD信息。
说明：
	用户可以通过遥控器取消当前OSD显示。
--*/
void   DVTSTBCA_ShowOSDMsg(const char * szOSD);

#endif

/*--------------------------- 类型2的界面：用户可以取消显示的界面 -----------------------*/
/*++
功能：显示新邮件的标志。
参数：
	byShow:					标志类型。含义如下：
							DVTCAS_NEW_EAMIL				新邮件通知。
							DVTCAS_NEW_EMAIL_NO_ROOM		有新邮件，但Email空间不够。
							DVTCAS_EMAIL_NONE				没有新邮件。
--*/
void DVTSTBCA_EmailNotify(BYTE byShow);

/*--------------------------- 应急广播 -----------------------*/
/*++
功能：应急广播, 切换到指定频道。
参数：
	wOriNetID:				原始网络ID。
	wTSID:					传送流ID。
	wServiceID:				业务ID。
	wDuration:				持续秒数。
--*/
void DVTSTBCA_UrgencyBroadcast(WORD wOriNetID, WORD wTSID, WORD wServiceID, WORD wDuration);

/*++
功能：取消应急广播，切换回应急广播前用户观看的频道。
参数：无。
--*/
void DVTSTBCA_CancelUrgencyBroadcast( void );

/*--------------------------- IPPV/IPPT -----------------------*/
/*++
功能：弹出框询问用户是否预订当前IPP节目。
参数：
	wEcmPid:				IPP对应的EcmPID
	pIpp:					当前IPP的信息。
--*/
void DVTSTBCA_InquireBookIpp(WORD wEcmPid, const SDVTCAIpp * pIpp );

/*++
功能：通知机顶盒需要到特征码流频点接收区域特征码流。(单频点区域锁定使用)
参数：
	wWaitSeconds:			超时时间，单位秒。
返回值：
	DVTCA_OK				成功
	其他					失败
--*/
HRESULT DVTSTBCA_SwitchChannel(WORD wWaitSeconds);

/*++
功能：通知机顶盒区域锁定特征码流接收完毕。(单频点区域锁定使用)
--*/
void DVTSTBCA_AreaLockOk(void);

/*++
功能：获取机顶盒软件版本。
返回值：机顶盒软件版本号。
--*/
DWORD DVTSTBCA_GetSoftVer(void);


/*++
功能:获取芯片的CPUID。
参数:
	pbyCPUID:	芯片提供的CPUID，最多20个字节
返回值:
	0:		获取CPUID成功
	其他值:	获取CPUID失败
--*/
HRESULT DVTSTBCA_GetCPUID(BYTE * pbyCPUID);

/*++
功能:获取NIT表中的PDSD值，用于NIT区域锁定功能
参数:
	pdwData:	机顶盒提供的，从NIT表中获取的PDSD值
返回值:
	0:		获取成功
	其它值:	获取失败
	
使用说明:
	如果不实现NIT区域锁定功能，此函数返回-1且(*pdwData)置为0即可
--*/
HRESULT	DVTSTBCA_GetNitValue(DWORD * pdwData);

/*------------------------------------------以上接口是STB供给CAS调用---------------------------------------------------*/

#ifdef  __cplusplus
}
#endif
#endif  
