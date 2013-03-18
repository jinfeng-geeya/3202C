/*****************************************************************************

File Name  : GYAD_Api.h

Description: geeya advertisement API.

Copyright (C) 2009 Chengdu Geeya Technology Co.,Ltd

Author: Archer 

Create Date:2010.09.09

* Date				Name			    Modification
* ---------		-----------			------------
* 2010.09.09		      Archer			     Created
*  
*****************************************************************************/

#ifndef __GYAD_CORE_H__
#define __GYAD_CORE_H__

#define USHORT_LENGTH(a,b)	(((GYADU16)(a) << 8) | ((GYADU16)(b)))
#define ULONG_LENGTH(a,b,c,d)	(((GYADU32)(a) << 24) | ((GYADU32)(b))<< 16 | ((GYADU32)(c)<< 8) | ((GYADU32)(d)))

#define GYAD_GET_DATA_TIMEOUT		20

#define GYAD_PID		0xFD

#define GYAD_RELATE_TABLE_ID	0xF1
#define GYAD_DATA_TABLE_ID	0xF0



#define GYAD_AD_INDX_DES_SECTION		0 /*广告位描述段*/
#define GYAD_RESRC_DES_SECTION		1 /*资源信息描述段*/
#define GYAD_RESRC_INFO_SECTION		2 /*资源关联段*/

#define GYAD_PER_INDX_RESRC_MAX		10  /*每个广告位最多关联的资源数量*/

#define GYAD_NEED_SAVE_TO_NVRAM		1 /*资源需要保存到NVRAM*/

#define GYAD_RESRC_RELATIVE_SERVICE		1 /*资源和节目关联*/
#define GYAD_RESRC_NOT_RELATIVE_SERVICE	0 /*资源和节目无关*/

#define GYAD_BOOT_LOGO_INDX	1 /*开机广告广告位编号*/

#define GYAD_NVRAM_HEAD_INFO_SIZE	1 /*NVRAM信息头的长度*/

#define GYAD_RESRC_PER_SEC_DATA_LEN	2032 /*每个段中所含资源数据的长度(最后一个段例外)*/

/*资源信息*/
typedef struct{
	GYADU8 wResrcId;	/*资源ID*/
	GYADU8 cAdIndx;	/*广告位编号*/
	GYADU8 cAdVer;	/*资源版本*/
	GYADU8 cRelativeFlag;	/*节目关联标识*/
	GYADU8 cSaveFlag;	/*NVRAM保存标识*/
	sGYAdInfo_t sAdInfo;	/*广告信息*/
}GYResrcInfo_t;

/*节目关联信息*/
typedef struct{
	GYADU16 wSvcId;	/*服务号ID*/
	GYADU16 wTsId;	/*传输流ID*/
	GYADU8 cResrcNum;	/*关联资源数量*/
	GYADU16 *pwResrcId; /*资源ID*/
}GYAdProg_t;

typedef struct{
	GYADU16 wSvcNum;	/*关联节目的数量*/
	GYAdProg_t *psAdProgInfo;	/*关联信息*/
}GYAdRelativeInfo_t;

typedef struct{
	GYADU8 wResrcId;	/*资源ID*/
	GYADU32 dwOffset;	/*NVRAM中的数据地址偏移*/
}GYAdDataNvramInfo_t;

#endif

