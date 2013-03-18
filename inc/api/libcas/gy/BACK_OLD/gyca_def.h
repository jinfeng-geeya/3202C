/*******************************************************************************
** Notice: Copyright (c) 2008 Geeya corporation - All Rights Reserved 
********************************************************************************
** Description:
**                 The defination file of CAS library . the data structure prototype,enum and macro
**                 is included in the file. these are common definations thar used at CAS library and 
**                 STB application.
**
** File Rev     $Revision: \main\20060816_1.0_integration_branch\1 $
**
** Date         $Date:     Wednesday Augest 16 11:54:04 2006 $
** 
** File Name: gyca_def.h
**
********************************************************************************/
#ifndef _GYCAS_DEF_H_
#define _GYCAS_DEF_H_

#ifdef  __cplusplus
extern "C" {
#endif

#include "gyca_pub.h"

/*
** the definations about EMM,ECM 
*/
#define	CA_INFOR_CLEAR			       (0)
#define	CA_INFOR_ADD				(1)

#define 	CA_MAX_SERVICE_PER_ECM	(10)  /* 1个ECM可加扰的节目数*/

/*
** CA数据表过滤器类型定义*/
#define   CA_STB_FILTER1		(1)	/* ECM filter */
#define	CA_STB_FILTER2		(2)	/* EMM filter */
#define	CA_STB_FILTER3		(3)	/* EMM filter */
#define	CA_STB_FILTER4		(4)	/* EMM filter */
#define	CA_STB_FILTER5		(5)	/* EMM filter */


/*
** CA OSD显示位置定义*/
#define	CA_OSD_ON_TOP				(0)	/* OSD显示在机顶盒的上方*/
#define	CA_OSD_ON_BOTTOM			(1)	/* OSD显示在机顶盒的下方*/
#define   CA_OSD_ON_MIDDLE			(2)	/* OSD显示在机顶盒的中间*/
#define   CA_OSD_ON_FULL_SCREEN	(3)	/* OSD全屏显示,保留暂时可以不支持*/

/*  时间结构定义*/
 typedef struct _T_CA_Time{
 	GYU8  m_byHour;	/* 小时*/
 	GYU8  m_byMinute;	/* 分钟*/
 	GYU8  m_bySecond;  /* 秒钟*/
 }T_CA_Time;

/*  日期结构定义*/
typedef struct _T_CA_Date{
	GYU16  m_byDate;      /*MJD code*/
}T_CA_Date;

/* 时间日期结构定义*/
typedef struct T_CA_DateTime{
	T_CA_Date   date;
	T_CA_Time   time;
}T_CA_DateTime;

/** CAS 系统的相关信息*/
typedef struct _T_CAServiceInfo{
	GYU16 	m_wEcmPid;									/* 当前Ecm的PID*/
	GYU16 	m_wOriNetworkID;							/* 原始网络ID*/
	GYU16 	m_wTSID;									/* 当前TS流ID*/
	GYU8 	m_byServiceCount;							/* 当前相同ECM PID中的节目数量*/
	GYU8	m_reserved;			      						/* 保留*/
	GYU16	m_wServiceID[CA_MAX_SERVICE_PER_ECM];		/* 各个节目的PID*/
}T_CAServiceInfo;

/*
** 运营商的信息 
*/
typedef struct _T_CA_OperatorInfo {
	GYU8   	m_bySPNameLen;	/* 运营商名称长度*/
	GYU8     m_szSPName[30];	/* 运营商名称,0字符结束*/
	GYU8      m_bySPTelLen;		/* 运营商联系电话的长度*/
	GYU8      m_szSPTel[20];		/* 运营商联系的电话号码,0字符结束*/
	GYU8      m_bySPAddrLen;	/* 运营商联系地址长度*/
	GYU8      m_szSPAddr[60];	/* 运营商联系地址内容,0字符结束*/
} T_CA_OperatorInfo;

/*
** T_CAOSDInfo OSD显示数据信息：
** 在CAM需要机顶盒显示OSD内容调用回调函数时使用
*/
typedef struct _T_CAOSDInfo{
	GYU8	m_byOSD_Position;   	/*OSD 显示的位置0-顶部 1-底部
				   				 2-中间显示 3-全屏显示*/
	GYU16     m_wOSD_Show_Cycle;	/*OSD显示的时间，单位秒*/
	GYU8      m_byOSD_Text_length;	/*OSD文本显示的长度*/
	GYU8      m_szOSD_Text[150];	/*OSD文本*/
}T_CAOSDInfo;


/*
** T_CAMailInfo邮件信息：
** 在CAM接收到邮件，通知机顶盒进行邮件信息处理时调用
*/
typedef struct _T_CAMailInfo{
	GYU8       m_byEmail_Level;			/*邮件等级 0-普通 1-紧急*/
	GYU8       m_byEmail_Create_Time[5]; 	/*创建时间 格式：MJD+HHMMSS*/
	GYU8       m_byEmail_Title_length;    	/*邮件标题长度*/
	GYU8       m_szEmail_Title[40];     		/*邮件标题内容*/
	GYU8       m_byEmail_Content_length;  	/*邮件内容长度*/
	GYU8       m_szEmail_Content[112]; 	 /*邮件内容*/
}T_CAMailInfo;

/*
** T_CASoftwareUpdateInfo软件更新信息：
** CAM接收到机顶盒程序升级通知后将此通知机顶盒应用程序。
*/
typedef struct _T_CASoftwareUpdateInfo{
	GYU8    m_bySoft_version[4];   //软件版本
	GYU8    m_byReserved1;     		//保留字段1
	GYU32   m_lFrequency;        	//频点  单位KHz
	GYU16   m_wSymbol_rate;     //符号率 
	GYU8    m_byQAM;           	//升级软件传送流所在频点的QAM调制方式
                              		//详细参考QAM模式定义
	GYU16   m_wProgram_no;      //升级软件传送流数据服务的频道号
	GYU16   m_wPID;             //升级软件传送流的PID信息
	GYU8    m_byTable_id;          //升级软件传送流的Table_id信息
	GYU8    m_byReserved2[6];    //保留6个字节
}T_CASoftwareUpdateInfo;

/*
** 要求机顶盒显示的指纹信息*/
typedef struct _T_CAFingerInfo{
	GYU8  m_byShowTime;		/* 显示时间单位秒*/
	GYU8  m_byFingerInfoLen;	/* 指纹信息长度*/
	GYU8  m_szFinger[16];		/* 指纹信息0字符结束*/
}T_CAFingerInfo;

/*************************************************************************
** IPPV 相关结构定义
**************************************************************************/



/*
** 钱包信息*/
typedef struct{
	GYS32 dwBalanceToken;  /* 剩余的金额 ,为负数表示欠费*/
    GYS32 dwUsedToken;	/* 已花费的金额 */
}T_CA_IPPVBurseInfo;

/*IPPT购买记录信息*/
typedef struct{
	GYU16 	wEventID; /*产品的包ID	2*/
	T_CA_DateTime time; /*Mjdhms格式	5*/
	GYU8 	stbSn[8]; /*购买所在STB的序号(后8位)	8*/
	GYU8 	resevred;  /*1 byte*/
}T_CA_IPPTBoughtRecord; 

/* IPPV 价格信息 */
typedef struct _T_CA_IPPVPrice{
	GYU8    m_byEventPriceType; 	/* 0x0-一次性扣费 0x1-单位时段扣费*/
	GYU16   m_wEventPrice;			/* 当前节目的单位价格,人民币分单位*/
}T_CA_IPPVPrice;

/*要求机顶盒处理的当前ippv的信息*/
typedef struct _T_CA_IPPVInfo{
	GYU16   m_wPlatformId;
	GYU16   m_wServiceId;			/* 当前节目的服务id */
	GYU16   m_wEventId;  			/* 当前观看节目的事件id */
	GYU32	m_dwEvent_Ex_Sn;
	T_CA_IPPVPrice  price;
	T_CA_DateTime  expiredDate;
	T_CA_DateTime  validDate;	     /*增加的数据域 购买开始时间*/
	GYU16          casID;			 /*增加的数据域 购买产品包ID*/
}T_CA_IPPVInfo;

//雃安子母机方案的存入FLASH中的记录格式,仅此一条记录
typedef struct _T_CA_MotherInfo{
	GYU32 m_nsecEntitleTime; /*主卡的授权时间,写入主卡插过的机顶盒上, 时间以秒为单位便于比较*/
	GYU8 m_szSubName[20]; /*用户名称,20字节,在子卡插入里需要核对是否与主卡是同一个用户名称*/
	GYU8 m_szCUFB[64]; /*主卡的授权复制到子卡所在的机顶盒上*/
	GYU8 m_szExipreTime[5]; 
	GYBOOL m_bSonVerified; /*子卡是否已被验证, 要求在数据有效期内必须验证, 否则子卡不能收看节目*/
}T_CA_MotherInfo; 

/*挂角广告定义*/
typedef struct _T_CA_CornerAdvertise{
	GYU8 	m_bDataValid; 	/*当前FLASH存储区的内容是否有效, 1 为有效, 其他为无效*/
	GYU16 	m_wServiceID; 
	GYU8  	m_byPosition;   	/*与OSD显示位置相同*/
	GYU8	m_szBgColor[3]; /*广告的背景显示颜色，按RGB(R, G, B)表示*/
	GYU8	m_szTextColor[3]; 		/*广告的文字显示颜色，按RGB(R, G, B)表示*/
	GYU8 	m_byText_length; 	/*不多于100个字节，为0时表示不显示广告*/
	GYU8 	m_szText[101]; 	/*比要求的100个字节略多一些*/
}T_CA_CornerAdvertise; 

typedef struct _T_CARemoteControlInfo{
		GYU8	m_byOpCode; 			//操作码0: 按指定频点搜索节目；1: 跳转频道到指定频点的指定频道
		GYU32	m_dwFrequency; 		//搜索的指定频点kHz, 默认按QAM64, 符号率6875ksps. 	
		GYU16	m_wTsID; 				//跳转的指定TSID, 备用
		GYU16	m_wServiceID; 			//跳转的指定频道service_id只对OpCode==1时的频道切换有效）
}T_CARemoteControlInfo;


/********************************************************************************
** CA枚举类型定义
********************************************************************************/
#ifndef CA_ENUM_DEFINED
#define CA_ENUM_DEFINED

/********************************************************************************
** GYCAS 智能卡的事件定义
********************************************************************************/
typedef enum
{
	GYCA_CARD_IN = 0,		/* Inserting smart card event */
	GYCA_CARD_OUT,			/* pull out smart card event */
	GYCA_CARD_BAD,			/* bad card event notify when detecting a bad smc  */
	GYCA_CARD_NOTVALID,	/* invalid card event notify when detecting a invalid smc  */
	GYCA_CARD_ERROR, 		/* error card event notify when detecting a error smc */
	GYCA_CARD_READING
}GYCA_SMCSTATUS;

/*
** CA模块消息类型
*/
typedef enum
{
	GYCA_NOTICE_SHOW = 0,		/* 显示节目不能播放的提示信息*/
	GYCA_NOTICE_ERASE,		/* 擦除提示信息*/
	GYCA_OSD_ERASE,			/* 擦除OSD信息*/
	GYCA_RESTART_CHAN,		/* 请求应用程序重新播放当前节目*/
	GYCA_OSD_INFO,			/* 有EMM,PEMM OSD信息通知*/
	GYCA_MAIL_INFO,			/* 有EMM,PEMM Mail信息通知*/
	GYCA_EXECUTE_OP_INFO,	/* 要求机顶盒执行具体的机顶盒操作*/
	GYCA_SW_UPDATE_INFO,		/* 有EMM,PEMM 软件更新信息通知*/
	GYCA_TRANSPARENT_INFO,	/* CA 透传给机顶盒的私有信息*/
	GYCA_FINGER_SHOW,		/* 要求显示指纹信息*/
	GYCA_FINGER_ERASE,		/* 要求擦除指纹信息*/
	GYCA_IPPV_INFO			/* IPPV 信息*/
}GYCA_MSG_TYPE;

/*
** 智能卡用户类型定义*/
enum
{
	SMC_NORMAL_USER = 0x0,    	/* 普通用户*/
	SMC_TEST_USER = 0x1,		/* 测试用户*/
	SMC_GROUP_USER = 0x2,		/* 集团用户*/
	SMC_DEMO_USER = 0x3,		/* 演示用户*/
	SMC_FOREIGN_USER = 0x4	/* 境外用户*/
};


/*
** QAM模式*/
enum
{
	GYCA_QAM16 = 0,	/* 16QAM调制方式*/
	GYCA_QAM32,		/* 32QAM调制方式*/
	GYCA_QAM64,		/* 64QAM调制方式*/
	GYCA_QAM128,		/* 128QAM调制方式*/
	GYCA_QAM256		/* 256QAM调制方式*/
};


/*
** 智能卡授权状况定义*/
enum
{
	USER_ENS_UNKNOWNED = -1,	/* 用户状态未知，可能因为卡错误或者未插卡*/
	USER_ENS_PENDING,			/* 卡尚未被激活*/
	USER_ENS_ACTIVATE,			/* 正常激活的卡*/
	USER_ENS_ARREARAGE,		/* 欠费催缴*/
	USER_ENS_DEBT_STOP,		/* 用户欠费，几经停用*/
	USER_ENS_CALL_FOR_STOP,	/* 用户报停*/
	USER_ENS_CANCELED			/* 卡已经注销*/	
};

/*
** 函数调用的返回状态值*/
enum
{
	GYCA_ERR = -100,					/* 错误返回，原因不详*/
	GYCA_PIN_ERR,						/* PIN码输入错误*/
	GYCA_PARENTLEVEL_ERR,				/* 输入的家长控制级别超出范围*/
	GYCA_CARD_ERR,					/* 智能卡的状态出错*/
	GYCA_CARD_NOPAIR,					/* 智能卡未与机顶盒配对*/
	GYCA_CARD_MISPAIR,				/* 智能卡与其它机顶盒配对*/
	GYCA_INPUT_PARA_ERR,				/* 调用函数输入的参数错误*/
	GYCA_OK = 0						/* 正确返回*/
};

/*
** 智能卡通讯结果定义*/
enum
{
	SMC_COM_NORESP = -4,				/* 智能卡无响应*/
	SMC_COM_LEN_MISMATCH,			/* 输入长度不匹配*/
	SMC_COM_INVALIDCARDNO,			/* 输入的卡号无效*/
	SMC_COM_REMOVED,					/* 智能卡不在*/
	SMC_COM_OK = 0					/* 智能卡发送命令成功*/
};

#endif  //end of  CA_ENUM_DEFINED

/********************************************************************************
** 回调函数原型定义
********************************************************************************/
#ifndef CA_CALLBACK_FUNC_DEFINED
#define CA_CALLBACK_FUNC_DEFINED
/*
** 智能卡的状态改变通知CA模块的回调函数原型
*/
typedef void (*SC_Status_Notify_Fct_t)(GYU8,GYU8,GYU8*,GYS32);

#endif  //end of CA_CALLBACK_FUNC_DEFINED

#ifdef  __cplusplus
}
#endif

#endif		//_GYCAS_PUBS_H_



