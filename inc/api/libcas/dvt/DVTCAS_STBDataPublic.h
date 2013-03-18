#ifndef _DVTCAS_STBDataPublic_H_2004_12_31_
#define _DVTCAS_STBDataPublic_H_2004_12_31_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef  __cplusplus
extern "C" {
#endif

/*用DVTCA_VERSION_NEW宏来区分新、老集成的机顶盒。由数码视讯人员控制。2009-12-02
  新集成的SDVTCAIpp、SDVTCAViewedIpp结构中多了成员变量，DVTSTBCA_ShowOSDMsg参数有变化.
*/
#define	DVTCA_VERSION_NEW		//如果是新集成的机顶盒,需要定义；如果是原来集成的机顶盒，注释掉此行??????????????????????

/*用DVTCA_SECURITYCHIP宏来区分是不是开通了安全功能的安全芯片.由数码视讯人员控制。2009-12-14
*/
#define DVTCA_SECURITYCHIP   (BYTE)0	//1:安全芯片,并且打开了安全功能; 0:非安全芯片或者是安全芯片但没打开芯片的安全功能???????????

#ifndef BYTE //8bit
#define BYTE unsigned char
#endif
#ifndef WORD //16bit
#define WORD unsigned short
#endif
#ifndef HRESULT
#define	HRESULT long
#endif
#ifndef DWORD
#define DWORD unsigned long
#endif
#ifndef bool //8bit
#define bool unsigned char
#endif
#ifndef NULL
#define NULL 0
#endif
#ifndef false
#define false 0
#endif
#ifndef true
#define true 1
#endif
	
//信号量定义
typedef long  DVTCA_Semaphore ;

//线程函数地址类型
typedef void (*pThreadFunc) (void);
	
/*----------------------------------------------------------------------*/
/*                       下面是长度值定义                               */
/*----------------------------------------------------------------------*/

#define DVTCA_MAXLEN_PINCODE					6//8		//最长的PIN码长度
#define DVTCA_LEN_SMARTCARD_SN					64		//最长只能卡序列号长度（外部卡号）

#define DVTCA_MAXLEN_SERVICEPERECM				10		//每个ECM支持的最大Service数目

#define DVTCA_MAXLEN_TVSNAME					21		//最长运营商名称长度+\0
#define DVTCA_MAXLEN_MANUFACTURERNAME			21		//最长的CAS厂商名字长度+\0 
#define DVTCA_MAXLEN_PRODUCT_NAME				21		//最长的产品名称,最多20个字符+\0
#define DVTCA_MAXLEN_SERVICE_NAME				21		//最长的业务名称,最多20个字符+\0
#define	DVTCA_MAXLEN_EMAIL_SENDERNAME			11		//E-mail发送者姓名，最多11个字符+\0
#define	DVTCA_MAXLEN_EMAIL_CONTENT				1025	//E-mail最大长度，最多1024个字符+\0
#define	DVTCA_MAXLEN_EMAIL_SHORT_CONTENT		21		//E-mail标题长度，最多20个字符+\0

#define	DVTCA_MAXNUMBER_TVSID					5		//支持的运营商最大个数

#define DVTCA_MAX_IPP_COUNT						250		//最多的IPP产品个数，暂定为250

//请求收取私有数据的请求号
#define DVTCA_STB_ECM_SLOT						1
#define DVTCA_STB_EMM_Private_SLOT				2
#define DVTCA_STB_EMM_Public_SLOT				3
#define DVTCA_STB_EMM_Group_SLOT				4  //for Group Address 

//智能卡状态定义
#define DVTCAS_SC_OUT							0		//智能卡不在读卡器中
#define DVTCAS_SC_INIT							1		//智能卡在读卡器中，正在初始化
#define DVTCAS_SC_IN							2		//智能卡在读卡器中，工作正常

//机卡配对状态定义
#define DVTCAS_SC_PAIR_OK						0		//智能卡与当前机顶盒配对
#define DVTCAS_SC_NO_Pairing					1		//智能卡没有与任何机顶盒配对
#define DVTCAS_SC_PAIR_OTHER					2		//智能卡与当前机顶盒没有配对

//收看级别定义
#define DVTCAS_Rating_3							3		//3岁
#define DVTCAS_Rating_4							4		//4岁
#define DVTCAS_Rating_5							5		//5岁
#define DVTCAS_Rating_6							6		//6岁
#define DVTCAS_Rating_7							7		//7岁
#define DVTCAS_Rating_8							8		//8岁
#define DVTCAS_Rating_9							9		//9岁
#define DVTCAS_Rating_10						10		//10岁
#define DVTCAS_Rating_11						11		//11岁
#define DVTCAS_Rating_12						12		//12岁
#define DVTCAS_Rating_13						13		//13岁
#define DVTCAS_Rating_14						14		//14岁
#define DVTCAS_Rating_15						15		//15岁
#define DVTCAS_Rating_16						16		//16岁
#define DVTCAS_Rating_17						17		//17岁
#define DVTCAS_Rating_18						18		//18岁

//email 显示信息。
#define DVTCAS_EMAIL_NEW						0		//新邮件通知
#define DVTCAS_EMAIL_NEW_NO_ROOM				1		//有新邮件，但Email空间不够
#define DVTCAS_EMAIL_NONE						0xFF	//隐藏标志

//email 状态。	
#define DVTCAS_EMAIL_STATUS_INI					0		//初始值
#define DVTCAS_EMAIL_STATUS_READ				2		//被读过了

//价格类型定义。
#define	TPP_TAP_PRICE							0		//不回传可录像
#define	TPP_NOTAP_PRICE							1		//不回传不可录像
#define	CPP_TAP_PRICE							2		//回传可录像
#define	CPP_NOTAP_PRICE							3		//回传不可录像


/*----------------------------------------------------------------------*/
/*                       下面是返回值定义                               */
/*----------------------------------------------------------------------*/

//机顶盒返回值基值定义
#define DVTCAERR_BASE_STB						0xF1000000

//返回值定义
#define DVTCA_OK								0 

#define DVTCAERR_STB_POINTER_INVALID			(DVTCAERR_BASE_STB + 1)		//输入的指针无效
#define DVTCAERR_STB_DATA_LEN_ERROR				(DVTCAERR_BASE_STB + 2)		//输入的数据长度不合法
#define DVTCAERR_STB_TVS_NOT_FOUND				(DVTCAERR_BASE_STB + 3)		//没有找到符合要求的运营商
#define DVTCAERR_STB_EXCEED_MAX_COUNT			(DVTCAERR_BASE_STB + 4)		//超过了允许的最大的个数
#define DVTCAERR_STB_KEY_NOT_FOUND				(DVTCAERR_BASE_STB + 5)		//没有找到解密密钥
#define DVTCAERR_STB_PIN_INVALID				(DVTCAERR_BASE_STB + 6)		//输入的PIN码无效
#define DVTCAERR_STB_PIN_LOCKED					(DVTCAERR_BASE_STB + 7)		//PIN码被锁定
#define DVTCAERR_STB_CMD_INVALID				(DVTCAERR_BASE_STB + 8)		//MAC校验错误
#define DVTCAERR_STB_VER_EXPIRED				(DVTCAERR_BASE_STB + 9)		//输入的版本没有IC卡中的大，即数据已经过期
#define DVTCAERR_STB_EXCEED_MAX_LEN				(DVTCAERR_BASE_STB + 10)	//运营商的名称超过最大的长度
#define DVTCAERR_STB_NO_SPACE					(DVTCAERR_BASE_STB + 11)	//已经没有空间
//#define DVTCAERR_STB_IS_PAIRED					(DVTCAERR_BASE_STB + 12)	//智能卡与当前机顶盒对应
#define DVTCAERR_STB_NOT_PAIRED					(DVTCAERR_BASE_STB + 13)	//智能卡与当前机顶盒没有对应
//#define DVTCAERR_STB_COUNT_ISZERO				(DVTCAERR_BASE_STB + 14)	//输入的Count为0. 
#define DVTCAERR_STB_WORKTIME_INVALID			(DVTCAERR_BASE_STB + 15)	//工作时段设置不合法，可能是时间不在00:00:00~23:59:59之间，或者开始时间大于结束时间。
//#define	DVTCAERR_STB_OVERFLOW					(DVTCAERR_BASE_STB + 16)	
#define	DVTCAERR_STB_CARD_INVALID				(DVTCAERR_BASE_STB + 17)	//智能卡无效
#define	DVTCAERR_STB_Rating_LOW					(DVTCAERR_BASE_STB + 18)	//智能卡设置的收看级别比当前收看的节目低
#define DVTCAERR_STB_UNKNOWN					(DVTCAERR_BASE_STB + 19)	//未知错误，STB不做任何提示，可通过卡复位恢复。

//#define	DVTCAERR_STB_EMAILBOX_EMPTY				(DVTCAERR_BASE_STB + 20)	//E-mail信箱为空
#define	DVTCAERR_STB_NO_EMAIL					(DVTCAERR_BASE_STB + 21)	//没有此E-mail
#define	DVTCAERR_STB_EMAILBOX_FULL				(DVTCAERR_BASE_STB + 22)	//E-mail信箱满
//#define	DVTCAERR_STB_NO_SECTION					(DVTCAERR_BASE_STB + 23)	//没有收到完整Section

#define DVTCAERR_STB_MONEY_LACK					(DVTCAERR_BASE_STB + 24)	//余额不多，只在预订时会返回该错误，需要提示用户"预订成功，但余额不多，请及时充值"。
#define DVTCAERR_STB_PROD_NOT_FOUND				(DVTCAERR_BASE_STB + 25)	//产品未找到
#define DVTCAERR_STB_OPER_INVALID				(DVTCAERR_BASE_STB + 26)	//产品已存在，不能操作
#define DVTCAERR_STB_NEED_PIN_PASS				(DVTCAERR_BASE_STB + 27)	//需要PIN验证通过
#define DVTCAERR_STB_IC_COMMUNICATE				(DVTCAERR_BASE_STB + 28)	//与IC卡通讯错误
#define DVTCAERR_STB_SLOT_NOT_FOUND				(DVTCAERR_BASE_STB + 29)	//运营商钱包没有发现
#define DVTCAERR_STB_PRICE_INVALID				(DVTCAERR_BASE_STB + 30)	//价格无效
#define DVTCAERR_STB_PRODUCT_EXPIRED			(DVTCAERR_BASE_STB + 31)	//产品已经过期

//xb:20050617 ++
#define DVTCAERR_STB_EXPIRED					(DVTCAERR_BASE_STB + 33)	//产品已经过期
#define DVTCAERR_IPPREC_NOT_FOUND				(DVTCAERR_BASE_STB + 34)	//IPP记录不存在
#define DVTCAERR_TIME_INVALID					(DVTCAERR_BASE_STB + 35)	//当前时间无效
#define DVTCAERR_NOT_BOOKED						(DVTCAERR_BASE_STB + 37)	//产品未预定
#define DVTCAERR_CONFIRMED						(DVTCAERR_BASE_STB + 38)	//产品已经确认
#define DVTCAERR_UNCONFIRMED					(DVTCAERR_BASE_STB + 39)	//产品已经取消确认
#define DVTCAERR_INPUT_DATA_INVALID				(DVTCAERR_BASE_STB + 40)	//输入的数据无效

//xb:20050617 --

//机顶盒遥控器不能取消的显示信息列表
#define	DVTCA_RATING_TOO_LOW					0			//收看级别不够
#define DVTCA_NOT_IN_WATCH_TIME					1			//不在收看时段内
#define	DVTCA_NOT_PAIRED						2			//没有机卡对应
#define	DVTCA_IS_PAIRED_OTHER					3			//IC卡与其它机顶盒对应
#define	DVTCA_PLEASE_INSERT_CARD				4			//请插卡
#define	DVTCA_NO_ENTITLE						5			//没有购买此节目
#define	DVTCA_PRODUCT_RESTRICT					6			//运营商限制观看该节目
#define	DVTCA_AREA_RESTRICT						7			//运营商限制区域观看
#define	DVTCA_DECRYPT_FAILURE					0xFF			//STB不做任何提示。卡通讯问题，通过卡复位可解决
//V2.1新增的提示信息
#define DVTCA_MOTHER_RESTRICT					8			//此卡为子卡，已经被限制收看，请与母卡配对
#define DVTCA_NO_MONEY							9			//余额不足，不能观看此节目，请及时充值
#define DVTCA_IPPV_NO_CONFIRM					10			//此节目为IPPV节目，请到IPPV节目确认/取消购买菜单下确认购买此节目
#define DVTCA_IPPV_NO_BOOK						11			//此节目为IPPV节目，您没有预订和确认购买，不能观看此节目
#define DVTCA_IPPT_NO_CONFIRM					12			//此节目为IPPT节目，请到IPPT节目确认/取消购买菜单下确认购买此节目
#define DVTCA_IPPT_NO_BOOK						13			//此节目为IPPT节目，您没有预订和确认购买，不能观看此节目
#define DVTCA_IPPV_NO_MONEY						14			//此节目为IPPV节目，余额不足，不能观看，请及时充值
#define DVTCA_IPPT_NO_MONEY						15			//此节目为IPPT节目，余额不足，不能观看，请及时充值
//xb:20050617
#define DVTCA_DATA_INVALID						16			//数据无效，STB不做任何提示。卡密钥问题。
#define DVTCA_SLOT_NOT_FOUND					17			//钱包不存在
#define DVTCA_SC_NOT_SERVER						18			//IC卡被禁止服务
#define DVTCA_TVS_NOT_FOUND						19			//运营商不存在
#define DVTCA_KEY_NOT_FOUND						20			//此卡未被激活，请联系运营商
#define DVTCA_IPPNEED_CALLBACK					21			//请联系运营商回传IPP节目信息
#define DVTCA_FREE_PREVIEWING					22			//用户您好，此节目您尚未购买，正在免费预览中
//wy:20071022－用户自定义数据类型
#define DVTCA_BOUQUETID							254			//自定义数据类型-Bouquet_id

//kfd:2007-11-16---字符串语言类型
#define DVTCA_LANG_CHN_SIM						1			//简体中文(默认)
#define DVTCA_LANG_ENG							2			//英文

/*----------------------------------------------------------------------*/
/*                       下面是一些数据结构定义                         */
/*----------------------------------------------------------------------*/

typedef struct _SDVTCAPin{
	BYTE	m_byLen;
	BYTE	m_byszPin[DVTCA_MAXLEN_PINCODE];
}SDVTCAPin;

typedef struct _SDVTCAManuInfo{
	DWORD	m_dwCardID;											//卡内部ID
	BYTE	m_byszSerialNO[DVTCA_LEN_SMARTCARD_SN];				//外部卡号
	DWORD	m_dwSTBCASVer;										//机顶盒CAS模块的版本号
	char	m_szSCCASManuName[DVTCA_MAXLEN_MANUFACTURERNAME];	//CAS供应商的名称
	DWORD	m_dwSCCASVer;										//智能卡CAS的版本号
	DWORD	m_dwSCCOSVer;										//SC的版本号，也就是SC中COS的版本号
	DWORD	m_tSCExpireDate;									//智能卡的有效期，由STB初始化时判断，如果过期，给用户一个提示，但可以继续使用。time_t时间格式。
}SDVTCAManuInfo;

typedef struct _SDVTCATvsInfo{
	WORD	m_wTVSID; 											//运营商的外部编号
	char	m_szTVSName[30];					//运营商名称
}SDVTCATvsInfo;

typedef struct _SDVTCAServiceEntitle{
	WORD 	m_wProductID;										//产品ID
	DWORD	m_tEntitleTime;										//授权时间，time_t格式。
	DWORD	m_tStartTime;										//开始时间，time_t格式。
	DWORD	m_tEndTime;											//结束时间，time_t格式。
	char	m_szProductName[DVTCA_MAXLEN_PRODUCT_NAME];			//产品名称
	bool	m_bTapingFlag;										//录像标识。
}SDVTCAServiceEntitle;											//普通产品授权

typedef struct _SDVTCAServiceInfo{
	WORD 	m_wEcmPid;											//加扰的ECMPID
	WORD 	m_wServiceID;										//所处频道的ServiceID
}SDVTCAServiceInfo;

typedef struct _SDVTCAEmailInfo{
	DWORD	m_dwVersion;										//发送时间
	char	m_szSenderName[DVTCA_MAXLEN_EMAIL_SENDERNAME];		//发送者姓名
	BYTE	m_Status;											//邮件的状态，为DVTCAS_EMAIL_STATUS_宏中的一种
	WORD	m_wEmailLength;										//邮件内容长度
	char	m_szTitle[DVTCA_MAXLEN_EMAIL_SHORT_CONTENT];		//邮件短内容
}SDVTCAEmailInfo;

typedef struct _SDVTCAEmailContent{
	DWORD	m_dwVersion;										//发送时间
	WORD	m_wEmailLength;										//邮件内容长度
	char	m_szEmail[DVTCA_MAXLEN_EMAIL_CONTENT];				//邮件内容
}SDVTCAEmailContent;

typedef struct _SDVTCAIpp{
	WORD	m_wTVSID;					//运营商编号
	WORD	m_wProdID;					//产品ID
	BYTE	m_bySlotID;					//钱包ID
	char	m_szProdName[DVTCA_MAXLEN_PRODUCT_NAME];	//产品名称
	DWORD	m_tStartTime;				//开始时间，time_t格式。
	DWORD	m_dwDuration;				//持续秒数
	char	m_szServiceName[DVTCA_MAXLEN_SERVICE_NAME];	//业务名称
	WORD	m_wCurTppTapPrice;			//当前的不回传、能录像价格(分)，价格类型值为0
	WORD	m_wCurTppNoTapPrice;		//当前的不回传、不可录像价格(分)，价格类型值为1
	WORD	m_wCurCppTapPrice;			//当前的要回传、能录像价格(分)，价格类型值为2
	WORD	m_wCurCppNoTapPrice;		//当前的要回传、不可录像价格(分)，价格类型值为3
	WORD	m_wBookedPrice;			//已经预订的价格(分)
	BYTE	m_byBookedPriceType;		//已经预订的价格类型，取值范围0~3
	BYTE	m_byBookedInterval;		//预订收费间隔
	BYTE	m_byCurInterval;			//当前收费间隔
	BYTE	m_byIppStatus;				//Ipp产品状态
#ifdef DVTCA_VERSION_NEW
	BYTE	m_byUnit;					//收费间隔的单位0 -分钟1-小时2-天3-月4-年
	WORD	m_wIpptPeriod; 			//用户订购IPPT的观看周期数,for Philippines LongIPPT。
#endif
}SDVTCAIpp;

typedef struct _SDVTCAViewedIpp{
	//WORD	m_wTVSID;											//运营商编号
	char	m_szoperdName[30];
	char	m_szProdName[DVTCA_MAXLEN_PRODUCT_NAME];		//产品名称
	DWORD	m_tStartTime;						//开始时间，time_t格式。
	DWORD	m_dwDuration;						//持续秒数
	WORD	m_wBookedPrice;						//预订价格(分)
	BYTE	m_byBookedPriceType;					//预订价格类型：0:TppTap;1:TppNoTap;2:CppTap;3:CppNoTap;
	BYTE	m_byBookedInterval;					//预订收费间隔
	char   	m_szOtherInfo[44];					//ippv时为“此产品为ippv产品”，ippt时为“观看总时间：？分钟，扣钱总数：？分”
#ifdef DVTCA_VERSION_NEW
	BYTE	m_byUnit;						//收费单位，0 -分钟1-小时2-天3-月4-年
#endif
}SDVTCAViewedIpp;

typedef struct _SDVTCAAreaInfo{
	DWORD		m_dwCardArea;			//卡的区域码
	DWORD		m_tSetCardAreaTime;		//设定区域时间。
	BYTE		m_byStartFlag;			//启动标志。1:启动；0:没有。
	DWORD		m_tSetFlagTime;			//设置标志时间。
	DWORD		m_tSetStreamTime;		//设置特征码流的时间。(收到区域码流时间)
	DWORD		m_dwIntervalTime;		//卡中当前时间减去特征码时间的最大值。
	DWORD		m_dwStreamMinArea;		//码流的最小区域码
	DWORD		m_dwStreamMaxArea;		//码流的最大区域码
}SDVTCAAreaInfo;

#ifdef  __cplusplus
}
#endif

#endif	
