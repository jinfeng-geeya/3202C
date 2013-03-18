/* ================================================================= 
	Title:				STB和CA_LIB公用的数据结构及宏定义
	Create Date:		2003.07.28
	Author:				GZY
	Description:		此文件符合ANSI C的语法	
================================================================= */

/* ================================================================= 
	Version:			4.0.0
	Modify Date:		2007.09.06
	Author:				KFM
	Description:		对原文件进行整理，使与移植说明文档相对应	
================================================================= */

#ifndef	_PUB21EX_ST_H
#define _PUB21EX_ST_H

#include	"pub_option.h"
/*-------------------------------------功能模块开关---------------------------------------
#define	INCLUDE_DETITLE			1		//反授权确认码功能
#define	INCLUDE_IPPV_APP		1		//IPPV功能
#define	INCLUDE_IPPT_APP		1		//IPPT功能
#define	INCLUDE_ADDRESSING_APP	1		//OSD和邮件功能
#define	INCLUDE_CHILDCARD_APP	1		//子母卡功能
#define	INCLUDE_LOCK_SERVICE	1		//强制切换频道功能
#define	INCLUDE_PATCHING_APP	1		//智能卡升级功能
*/

#ifdef  __cplusplus
extern "C" {
#endif


/*-------------------------------------基本数据类型---------------------------------------*/
#define		ULONG			unsigned  long   
#define		WORD			unsigned  short  
#define		BYTE			unsigned  char 
#ifndef     bool
	#define		bool			BYTE
#endif

#ifndef		NULL
	#define		NULL	0
#endif

#define true	1                                          
#define false	0   

//-----函数调用时，指针参数所指向的空间的使用方式
#define		OUT				/*- 被调用者初始化并返回数据*/
#define		IN				/*- 调用者初始化并输入数据*/						
#define		INOUT			/*- 调用者初始化并输入数据，调用后被调用者返回数据*/	


/*----------------------------------------宏定义-----------------------------------------*/

//---------- 1. 智能卡相关限制
#define		TFCA_MAXLEN_SN					16			/*智能卡序列号的长度*/
#define		TFCA_MAXLEN_PRICE				2			/*最多的IPPV价格个数*/
#define		TFCA_MAXLEN_OPERATOR			4			/*最多的运营商个数*/
#define		TFCA_MAXLEN_PINCODE		   		6			/*PIN码的长度*/     
#define		TFCA_MAXLEN_ACLIST				18			/*智能卡内保存的每个运营商的用户特征个数*/
#define		TFCA_MAXLEN_SLOT				20			/*卡存储的最大钱包数*/
#define		TFCA_MAXLEN_TVSPRIINFO			32			/*运营商私有信息的长度*/
#define		TFCA_MAXLEN_IPPVP				300			/*智能卡保存最多IPPV节目的个数*/
#define		TFCA_MAXLEN_ENTITLE				300			/*智能卡保存最多授权产品的个数*/


//---------- 2. 机顶盒相关限制
#define		TFCA_MAXLEN_PROGRAMNUM			4			/*一个控制字最多解的节目数*/
#define		TFCA_MAXLEN_ECM					8			/*同时接收处理的ECMPID的最大数量*/
#if INCLUDE_DETITLE
	#define TFCA_MAXLEN_DETITLE				5			/*每个运营商下可保存的反授权码个数*/
#endif


//---------- 3. CAS提示信息
#define		MESSAGE_CANCEL_TYPE				0x00		/*取消当前的显示*/
#define		MESSAGE_BADCARD_TYPE			0x01		/*无法识别卡，不能使用*/
#define		MESSAGE_EXPICARD_TYPE			0x02		/*智能卡已经过期，请更换新卡*/
#define		MESSAGE_INSERTCARD_TYPE			0x03		/*加扰节目，请插入智能卡*/
#define		MESSAGE_NOOPER_TYPE				0x04		/*卡中不存在节目运营商*/
#define		MESSAGE_BLACKOUT_TYPE			0x05		/*条件禁播*/
#define		MESSAGE_OUTWORKTIME_TYPE		0x06		/*不在工作时段内*/
#define		MESSAGE_WATCHLEVEL_TYPE			0x07		/*节目级别高于设定观看级别*/
#define		MESSAGE_PAIRING_TYPE			0x08		/*节目要求机卡对应*/
#define		MESSAGE_NOENTITLE_TYPE			0x09		/*没有授权*/
#define		MESSAGE_DECRYPTFAIL_TYPE		0x0A		/*节目解密失败*/
#define		MESSAGE_NOMONEY_TYPE			0x0B		/*卡内金额不足*/
#define		MESSAGE_ERRREGION_TYPE          0x0C		/*区域不正确*/
#define		MESSAGE_NEEDFEED_TYPE           0x0D		/*子卡需要和母卡对应*/
#define		MESSAGE_ERRCARD_TYPE            0x0E		/*智能卡校验失败，请联系运营商，同步智能卡*/
#define		MESSAGE_UPDATE_TYPE             0x0F	    /*系统升级，请不要拔卡或者关机*/
#define		MESSAGE_LOWCARDVER_TYPE			0x10		/*请升级智能卡*/
#define		MESSAGE_VIEWLOCK_TYPE			0x11		/*请勿频繁切换频道*/
#define		MESSAGE_MAXRESTART_TYPE			0x12		/*智能卡已受损*/
#define		MESSAGE_FREEZE_TYPE				0x13		/*智能卡已冻结，请联系运营商*/
#define		MESSAGE_CALLBACK_TYPE			0x14		/*回传失败*/
#define		MESSAGE_STBLOCKED_TYPE			0x20		/*请关机重启机顶盒*/

//---------- 4. 功能调用返回值定义
#define		TFCAS_OK						0x00		/*- 成功*/
#define		TFCAS_UNKNOWN					0x01		/*- 未知错误*/
#define		TFCAS_POINTER_INVALID			0x02		/*- 指针无效*/
#define		TFCAS_CARD_INVALID				0x03		/*- 智能卡无效*/
#define		TFCAS_PIN_INVALID				0x04		/*- PIN码无效*/
#define		TFCAS_DATASPACE_SMALL			0x06		/*- 所给的空间不足*/
#define		TFCAS_CARD_PAIROTHER			0x07		/*- 智能卡已经对应别的机顶盒*/
#define		TFCAS_DATA_NOT_FIND				0x08		/*- 没有找到所要的数据*/
#define		TFCAS_PROG_STATUS_INVALID		0x09		/*- 要购买的节目状态无效*/
#define		TFCAS_CARD_NO_ROOM				0x0A		/*- 智能卡没有空间存放购买的节目*/
#define		TFCAS_WORKTIME_INVALID			0x0B		/*- 设定的工作时段无效*/
#define		TFCAS_IPPV_CANNTDEL				0x0C		/*- IPPV节目不能被删除*/
#define		TFCAS_CARD_NOPAIR				0x0D		/*- 智能卡没有对应任何的机顶盒*/
#define		TFCAS_WATCHRATING_INVALID		0x0E		/*- 设定的观看级别无效*/
#define		TFCAS_CARD_NOTSUPPORT		    0x0F		/*- 当前智能卡不支持此功能*/
#define		TFCAS_DATA_ERROR				0x10		/*- 数据错误，智能卡拒绝*/
#define		TFCAS_FEEDTIME_NOT_ARRIVE		0x11		/*- 喂养时间未到，子卡不能被喂养*/
#define		TFCAS_CARD_TYPEERROR			0x12		/*- 子母卡喂养失败，插入智能卡类型错误*/


//---------- 5. ECM_PID设置的操作类型
#define		TFCAS_LIST_OK					0x00
#define		TFCAS_LIST_FIRST				0x01	
#define		TFCAS_LIST_ADD					0x02


//---------- 6. 邮件应用相关定义
//------------ 6.1. 邮件大小及数量限制
#define		TFCA_MAXLEN_EMAIL_TITLE			30			/*邮件标题的长度*/
#define		TFCA_MAXLEN_EMAIL				100			/*机顶盒保存的最大邮件个数*/
#define		TFCA_MAXLEN_EMAIL_CONTENT		160			/*邮件内容的长度*/

//------------ 6.2. 邮件图标显示方式
#define		Email_IconHide					0x00		/*隐藏邮件通知图标*/
#define		Email_New						0x01		/*新邮件通知，显示新邮件图标*/
#define		Email_SpaceExhaust				0x02		/*磁盘空间以满，图标闪烁。*/


//---------- 7. OSD应用相关定义
//------------ 7.1. OSD的长度限制
#define		TFCA_MAXLEN_OSD					180			/*OSD内容的最大长度*/

//------------ 7.2. OSD显示类型
#define		OSD_TOP							0x01		/*OSD风格：显示在屏幕上方*/
#define		OSD_BOTTOM						0x02		/*OSD风格：显示在屏幕下方*/


//---------- 8. IPPV/IPPT应用相关定义
//------------ 8.1. IPPV/IPPT不同购买阶段提示
#define		IPPV_FREEVIEWED_SEGMENT			0x00        /*免费预览阶段，是否购买*/
#define		IPPV_PAYVIEWED_SEGMENT			0x01        /*收费阶段，是否购买*/
#define		IPPT_PAYVIEWED_SEGMENT			0x02		/*IPPT收费段，是否购买*/

//------------ 8.2. IPPV价格类型
#define		TFCA_IPPVPRICETYPE_TPPVVIEW			0x0		/*不回传，不录像类型*/
#define		TFCA_IPPVPRICETYPE_TPPVVIEWTAPING	0x1		/*不回传，可录像类型*/

//------------ 8.3. IPPV节目的状态
#define		TFCA_IPPVSTATUS_TOKEN_BOOKING		0x01	/*预定*/
#define		TFCA_IPPVSTATUS_TOKEN_VIEWED		0x03	/*已看*/

//---------- 9. 频道锁定应用相关定义
#define   TFCA_MAXLEN_COMPONENT  5
#define   TFCA_MAXLEN_LOCKMESS   40


//---------- 10. 反授权确认码应用相关定义
#if INCLUDE_DETITLE
	#define Detitle_All_Readed				0x00		/*所有反授权确认码已经被读，隐藏图标*/
	#define Detitle_Received				0x01		/*收到新的反授权码，显示反授权码图标*/
	#define Detitle_Space_Small				0x02		/*反授权码空间不足，改变图标状态提示用户*/
	#define Detitle_Ignore					0x03		/*收到重复的反授权码，可忽略，不做处理*/
#endif
	

//---------- 10. 进度条提示信息
#define		TFCA_RECEIVEPATCH				1			/*升级数据接收中*/
#define		TFCA_PATCHING					2			/*智能卡升级中*/

/*-------------------------------------end of 宏定义--------------------------------------*/



/*----------------------------------------数据结构----------------------------------------*/

//---------- 1. 系统时间
typedef ULONG  TFTIME;
typedef WORD   TFDATE;

//---------- 2. 信号量定义（不同的操作系统可能不一样）
typedef ULONG  TFCA_Semaphore;

//---------- 3. 运营商信息
typedef struct _TFCAOperatorInfo{
	char	m_szTVSPriInfo[TFCA_MAXLEN_TVSPRIINFO+1];   /*运营商私有信息*/
}STFCAOperatorInfo;

//---------- 4. 节目信息
typedef struct _TFCASServiceInfo{
	WORD	m_wEcmPid;								/*节目相应控制信息的PID*/				
	BYTE	m_byServiceNum;							/*当前PID下的节目个数*/
	WORD 	m_wServiceID[TFCA_MAXLEN_PROGRAMNUM];	/*当前PID下的节目ID列表*/	
}STFCASServiceInfo;

//---------- 5. 普通授权信息
//------------ 5.1. 授权信息
typedef struct _TFCAEntitle{
	ULONG 		m_dwProductID;		/*普通授权的节目ID*/
	BYTE		m_bCanTape;			/*用户是否购买录像：1－可以录像；0－不可以录像*/
	TFDATE		m_tBeginDate;		/*授权的起始时间*/
	TFDATE		m_tExpireDate;		/*授权的过期时间*/
}STFCAEntitle;

//------------ 5.2. 授权信息集合
typedef struct _TFCAEntitles{
	WORD			m_wProductCount;					/*普通授权的个数*/
	STFCAEntitle	m_Entitles[TFCA_MAXLEN_ENTITLE];		/*普通授权列表*/
}STFCAEntitles;

//---------- 6. IPPV信息
#if INCLUDE_IPPV_APP 
//------------ 6.1. 钱包信息
	typedef struct _TFCASTVSSlotInfo{
		ULONG	m_wCreditLimit;					    /*信用度（点数）*/
		ULONG	m_wBalance;						    /*已花的点数*/
	}STFCATVSSlotInfo;

//------------ 6.2. IPPV节目的价格
	typedef struct _TFCAIPPVPrice{
		BYTE	m_byPriceCode;						/*节目价格类型*/
		WORD	m_wPrice;							/*节目价格（点数）*/	
	}STFCAIPPVPrice; 

//------------ 6.3. IPPV节目购买提示信息
	typedef struct _TFCAIppvBuyInfo{
		WORD			m_wTVSID;					/*运营商ID*/
		BYTE			m_bySlotID;					/*钱包ID*/
		ULONG 			m_dwProductID;				/*节目的ID*/
		BYTE			m_byPriceNum;				/*节目价格个数*/
		STFCAIPPVPrice  m_Price[TFCA_MAXLEN_PRICE];	/*节目价格*/
		TFDATE			m_wExpiredDate;				/*节目过期时间*/
	}STFCAIppvBuyInfo;

//------------ 6.4. IPPV节目信息
	typedef struct _TFCAIppvfo{
		ULONG 			m_dwProductID;				/*节目的ID*/
		BYTE			m_byBookEdFlag;				/*产品状态：BOOKING，VIEWED*/ 
		BYTE			m_bCanTape;					/*是否可以录像：1－可以录像；0－不可以录像*/
		WORD			m_wPrice;					/*节目价格*/
		TFDATE			m_wExpiredDate;				/*节目过期时间*/
		BYTE			m_bySlotID;					/*钱包ID*/
	}STFCAIppvInfo;

#endif

//---------- 7. 邮件信息
#if INCLUDE_ADDRESSING_APP
//------------ 7.1. 邮件头
	typedef struct _TFCAEmailHead{
		ULONG		m_dwActionID;								/*Email ID*/
		BYTE		m_bNewEmail;								/*新邮件标记：0－不是新邮件；1－新邮件*/
		char   		m_szEmailHead[TFCA_MAXLEN_EMAIL_TITLE+1];	/*邮件标题，最长为30*/
		WORD 		m_wImportance;								/*重要性： 0－普通，1－重要*/
		ULONG		m_tCreateTime;								/*EMAIL创建的时间*/
	}STFCAEmailHead;

//------------ 7.2. 邮件内容
	typedef struct _TFCAEmailContent{
		char		m_szEmail[TFCA_MAXLEN_EMAIL_CONTENT+1];		/*Email的正文*/
	}STFCAEmailContent;

#endif

//---------- 8. 频道锁定信息
#if INCLUDE_LOCK_SERVICE
//------------ 8.1. 节目组件信息
	typedef struct _TFComponent{		/*组件用于通知机顶盒节目类型及PID等信息，一个节目可能包含多个组件*/
		BYTE		m_CompType;         /*组件类型*/   
		WORD		m_wCompPID;         /*组件PID*/  
		WORD 		m_wECMPID;			/*组件对应的ECM包的PID，如果组件是不加扰的，则应取0。*/
	}STFComponent;

//------------ 8.2. 频道参数信息
	typedef struct _TFLockService{
		BYTE		m_fec_outer;							/*前项纠错外码*/          
		BYTE        m_fec_inner;							/*前项纠错内码*/          
		BYTE        m_Modulation;							/*调制方式*/
		ULONG		m_dwFrequency;							/*频率，BCD码*/
		ULONG 		m_symbol_rate;							/*符号率，BCD码*/
		WORD		m_wPcrPid;								/*PCR PID*/
		BYTE        m_ComponentNum;							/*节目组件个数*/
		STFComponent m_CompArr[TFCA_MAXLEN_COMPONENT];		/*节目组件列表*/     
		char        m_szBeforeInfo[TFCA_MAXLEN_LOCKMESS+1]; /*保留*/
		char        m_szQuitInfo[TFCA_MAXLEN_LOCKMESS+1];   /*保留*/
		char        m_szEndInfo[TFCA_MAXLEN_LOCKMESS+1];    /*保留*/
	}STFLockService;
	
/*-----------------------------------------------------------------------------------
a. 本系统中，参数m_dwFrequency和m_symbol_rate使用BCD码，编码前取MHz为单位。
   编码时，前4个4-bit BCD码表示小数点前的值，后4个4-bit BCD码表示小数点后的值。
   例如：
		若频率为642000KHz，即642.0000MHz，则对应的m_dwFrequency的值应为0x06420000；
		若符号率为6875KHz，即6.8750MHz，则对应的m_symbol_rate的值应为0x00068750。

b. 本系统中，m_Modulation的取值如下：
	0		Reserved
	1		QAM16
	2		QAM32
	3		QAM64
	4		QAM128
	5		QAM256
	6～255	Reserved
------------------------------------------------------------------------------------*/ 

#endif

/*------------------------------------end of 数据结构-------------------------------------*/


#ifdef  __cplusplus
}
#endif

#endif		

