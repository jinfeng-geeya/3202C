/**
* $Id: ca_task.h,v 1.62 2008/11/12 02:50:57 ayang Exp $
*
* Copyright (c) 2004 Irdeto Access B.V., All rights reserved.
*
* Header file for the Embedded CA Task 3.
**/

#ifndef _CA_TASK_H_
#define _CA_TASK_H_

#include <irca/pubh/s3_portable_types.h>
#include <irca/pubh/s3_status.h>
#include <irca/pubh/s3_smartcard.h>
#include <irca/pubh/s3_pvr_drm.h>
#include <irca/pubh/s3_service.h>
#include <irca/pubh/s3_vd_copycontrol.h>
#include <irca/pubh/s3_hgpc_msg.h>
#include <irca/pubh/s3_general_cfg.h>
#include <api/libc/printf.h>

#ifndef CA_TASK_VERSION
#define CA_TASK_VERSION "3.6.19"
#endif

#ifndef CA_TASK_PRIORITY
#define CA_TASK_PRIORITY			11
#endif

#ifndef CA_TASK_STACK
#define CA_TASK_STACK				(1024*10)
#endif

#ifndef CA_TASK_QUEUE_DEPTH
#define CA_TASK_QUEUE_DEPTH		(32*3)
#endif

/* mli: 3.3.11 */
#ifndef	MAX_SECTOR_NUMBER
#define	MAX_SECTOR_NUMBER			10
#endif

#ifndef SC_SLOT_MAX
#define SC_SLOT_MAX					2 /*support duel smartcards*/
#endif

#ifndef SC_SLOT_0
#define SC_SLOT_0						0
#endif
#ifndef SC_SLOT_1
#define SC_SLOT_1						1
#endif
#define SC_SLOT_INVALID				0xFF

/* for CA_CLIENT__SERVICE_GROUP_DEFINE message */
#define CA_CLIENT_DESCRAMBLE_SERVICE_TYPE	0x00000001
#define CA_CLIENT_RECORD_SERVICE_TYPE 		0x00000002
#define CA_CLIENT_PLAYBACK_SERVICE_TYPE 	0x00000004
#define CA_CLIENT_DMB_DESCRAMBLE_SERVICE_TYPE   0x00000008

#define CLIENT_DEFINE_EMM_SOURCE_CAT		0x00000008
#define CLIENT_DEFINE_EMM_SOURCE_OOB		0x00000010
#define CLIENT_DEFINE_DMB_EMM_SOURCE		0x00000020

#define CLIENT_DEFINE_DMB_MPEG_TS			0x01
#define CLIENT_DEFINE_DMB_DATAGROUP			0x02

/* limits */
#define MAX_SOFTCELL_STREAMS			32
/* ayang: 2008-03-13 change 255 to 512 according to doc705410 */
#define MAX_EPG_MSG_LENGTH				512 /* the max length of Mail/Announcement, (254 + '\0'). */
#define MAX_CLIENT_NAME_STR_LEN		32
#define MAX_BUILD_INFO_STR_LEN		128
#define MAX_CA_TASK_VER_STR_LEN		21 /* the max length of CA Task version string + '\0'. */
#ifndef	MAX_SC_SER_NO_LEN
#define MAX_SC_SER_NO_LEN				15 /* the max length of smartcard serial number. */
#endif
#define MAX_EMM_MONITOR_LEN			64
#define MAX_ECM_MONITOR_LEN			64
#define MAX_SERVICE_RESOURCES			32
/* jyou: 3.4.2 multiple smart cards */
#define MAX_DEMUX_COUNTS				4

/* invalid values in CA STATUS screen
Note: if getting an invalid value, you should display nothing on the associated item. */
#define INVALID_SC_TYPE           	0xFFFF
#define INVALID_SC_VERSION        	0x00
#define INVALID_SC_NAT_CHAR       	0xFF

/* Smart Card Sector type*/
#define IPPV_SUPPORT					0x00
#define PPT_SUPPORT						0x01
#define LPPV_SUPPORT                    0x02
#define SECTOR_FAIL						0xFF

#define INVALID_SC_SECTOR		  		0xFF
#define INVALID_SC_SMARTCARDID	  	0xFFFFFFFF

/*invalid values in CA TASK for current TS NIT information*/
#define  INVALID_TS_NIT_ID         	0xFFFF

/* ca client -> ca task request type */
#define	REQUEST_TYPE_QUERY			0x01
#define REQUEST_TYPE_REPLY				0x02


#define MAX_PINCODE_BYTE_LENGTH		2
#define PPT_LABLE_LEN           		11

#define EMM_USER_PAYLOAD_MAIL_MAX_LENGTH 256

typedef enum
{
	CA_DBG_NONE,        /* lowest */
	CA_DBG_CRITICAL,
	CA_DBG_MSG,
	CA_DBG_VERBOSE      /* highest */
}CA_DBG_LEVEL_EN;

typedef enum
{
	CA_QUERY_SUCCESS,
	CA_QUERY_NOT_AVAILABLE,
	CA_QUERY_FAIL
}CA_QUERY_STATUS_EN;

/* CA CLIENT action */
typedef enum
{
	CA_CLIENT__DEFINE_SERVICE_GROUP,
	CA_CLIENT__STOP_SERVICE_GROUP,
	CA_CLIENT__DEFINE_EMM_SERVICE,
	CA_CLIENT__STOP_EMM_SERVICE,
	CA_CLIENT__CURRENT_TS_NIT_INFO,
	CA_CLIENT__TERMINATION,
		CA_CLIENT__MAP_RESOURCES
}MSG_ACTION_EN;

/******************************************************************************/
/*                              QueryControl Type                             */
/******************************************************************************/

typedef enum
{
	CA_QC_SOFTCELL_INFO			=0,
	CA_QC_SMARTCARD_INFO,
	CA_QC_EMM_SERVICE_INFO,
	CA_QC_SERVICE_GROUP_INFO,
	CA_QC_SMARTCARD_ACCESS,
	CA_QC_PIN_CODE_CHANGE		= 5,
	CA_QC_PIN_CODE_CHECK,
	CA_QC_MONITOR_ECM,
	CA_QC_MONITOR_EMM,
	CA_QC_MAIL,
	CA_QC_ANNOUNCE				= 10,
	CA_QC_HASH_ID,
	CA_QC_EMM_DOWNLOAD,
	CA_QC_EMM_CHANGEPIN,
	CA_QC_EMM_DECODER_RECOVER,
	CA_QC_EMM_USER_PAYLOAD		= 15,
	CA_QC_NOTIFY_ENABLE,
	CA_QC_REMOTE_CHANNEL,
	CA_QC_NOTIFY_CURRENT_ERROR,
	CA_QC_NOTIFY_MEMORY_ERROR,
	CA_QC_COMPONENTS_INFO		= 20,
	CA_QC_ATTRIBUTED_DISPLAY_MESSAGE,
	CA_QC_IPPV_PREVIEW,
	CA_QC_IPPV_PURCHASE,
	CA_QC_IPPV_SET_THRESHOLD,
	CA_QC_IPPV_SECTOR_INFO		= 25,
	CA_QC_IPPV_EVENTS_LIST,
	CA_QC_IPPV_EVENT_CHECK,
	CA_QC_SMARTCARD_NUMBER_OF_SECTORS,		/*query control for smart crad sector number*/
	CA_QC_REGION,							/*query local region data*/
	CA_QC_PPT_INFO				= 30,		/*query ppt data*/
	CA_QC_SURFLOCK,							/*query surf lock data*/
	CA_QC_EXTENDED_PRODUCT_LIST,            /*query extended product list reply*/
	CA_QC_SMARTCARD_USER_INFO,              /*query smart card user data */
	CA_QC_PRODUCT_BLOCK,
	CA_QC_SMARTCARD_UNIQUE_ADDRESS= 35,		/* Query Controls added in ECT 3.4-1 Smartcard diagnose */
	CA_QC_SMARTCARD_VENDOR_ID,
	CA_QC_SMARTCARD_SECTOR_ADDRESS,
	CA_QC_SMARTCARD_SECTOR_DATECODE,		/* End adding 3.4-1*/
	CA_QC_SMARTCARD_OPERATOR_ID,			/* sren: Add in 3.4-2*/
	CA_QC_IPPV_CALLBACK_REQUEST	=40,		/* jyou: 3.4.2 STB Reporting */
	CA_QC_PVR_MUST_EXPIRE_NOTIFY,			/* mli: 3.3.11 */
	CA_QC_PVR_PRODUCT_BLOCK,				/* mli: 3.3.11 queryproduct */
	CA_QC_LPPV_AWARE,                       /* LPPV QC */
	CA_QC_LPPV_PREVIEW,
	CA_QC_LPPV_READ_TICKET      = 45,
	CA_QC_LPPV_DELETE_TICKET,
	CA_QC_LPPV_READ_CREDIT,
	CA_QC_LPPV_READ_ORDER,
	CA_QC_LPPV_PLACE_ORDER,
	CA_QC_LPPV_CANCEL_ORDER     = 50,
	CA_QC_MR_CAPABILITY,
	CA_QC_MR_CONFIGURATION,
	CA_QC_MR_PROFILE,
	CA_QC_MR_TRIGGER_PIN,
	CA_QC_RF_CONFIG				= 55,
	CA_QC_RF_SET,
	CA_QC_HGPC_SMARTCARD_DATA,
	CA_QC_HGPC_SOFTCELL_DATA,
	CA_QC_HGPC_MSG_STATUS,
	CA_QC_HGPC_EVENT			= 60,
	CA_QC_MONITOR_DMB_EMM,
	CA_QC_MONITOR_DMB_ECM,
	CA_QC_DMB_EMM_SERVICE_INFO,
	CA_QC_DMB_DESCRAMBLE_SERVICE_INFO,
	CA_QC_IPPV_PHONE_NUMBER		= 65,
	CA_QC_FATAL_ERROR,
	CA_QC_ATTRIBUTED_DISPLAY_MESSAGE_2,
	CA_MAX_QC_QUERIES
} CA_QUERY_CONTROL_EN;

typedef struct
{
	ia_bool			fUsed;
	ia_word32		wLength;
	ia_word32		wSmartCardId;
	ia_word32		wMacrovisionId;
	ia_word32		wDemuxId;
	ia_word32		wOOBId;
	ia_word32		wResX;
}CA_RESOURCE_CONFIG_ST;

typedef struct
{
	ia_word32		wLength;
	ia_word16		wNumGroups;
	vd_crypto_algorithm_e eCryptoAlgorithm;
}CA_INIT_CONFIG_ST;


typedef struct
{
		ia_byte         abMRPin[ 2 ];
		vd_rsc_id       wResourceId;
}CA_MR_MAP_ST;


typedef struct
{
	msg_config_descriptor_st stConfigDescTL;
	ia_byte *bData;
}CA_GENERAL_CONFIG_DESC_ST;


typedef struct
{
	msg_general_configuration_st bConfigCnt;
	CA_GENERAL_CONFIG_DESC_ST *stConfigDesc;
}CA_GENERAL_CONFIG_ST;

typedef struct
{
		CA_MR_MAP_ST    stMRMap;
	CA_GENERAL_CONFIG_ST stGeneralConfig;
}CA_RESOURCE_MAP_ST;
/* EPG message Types */
enum
{
	TEXT_MAILBOX = 0,   /* Message to be stored in mail box */
	TEXT_ANNOUNCEMENT,  /* Message to be displayed on screen only */
	CLUB_NUMBERS        /* Not supported by Headend */
		/* Values 0x03 to 0x0f are spare */
};

/* EPG message classes */
enum
{
	NORMAL=0,         /* Mail to mailbox, text to screen or temp store */
	TIMED,            /* Automatic erase after date / time*/
	FORCED_DISPLAY    /* Interrupt program and display immediate */
		/* Values 0x03 to 0x07 are spare */
};


/***** CA_QC_SOFTCELL_INFO Query Control *****/

typedef struct
{
	ia_byte	bMajor;
	ia_byte	bMinor;
	ia_uint32	uSubVersion;
	ia_char	acNameString[MAX_CLIENT_NAME_STR_LEN];
	ia_char	acBuildInfoString[MAX_BUILD_INFO_STR_LEN];
	ia_char	acCaTaskVerString[MAX_CA_TASK_VER_STR_LEN];
}CA_SOFTCELL_INFO_T;

typedef struct
{
	ia_byte	bComCount;
	CA_SOFTCELL_INFO_T* pstComponents;
}CA_COMPONENTS_INFO_T;

/***** CA_QC_SMARTCARD_INFO Query Control *****/

#define	SMARTCARD_INFO_MASK_RESOURCE_ID		0x0001
#define	SMARTCARD_INFO_MASK_STATUS			0x0002
#define	SMARTCARD_INFO_MASK_GENERAL_DATA	0x0004
#define	SMARTCARD_INFO_MASK_NATIONALITY		0x0008

#ifdef _AUSTAR_
typedef	struct
{
	ia_uint32	wResourceId;
	ia_word16	wStatus;
	ia_status_st stSmartCardStatus ;
	ia_word16	wType;
}CA_SMARTCARD_STATUS;	/* To get the SC's status */
#endif

typedef struct
{
	ia_byte		bRequestType;
	ia_uint32	wResourceId;
	ia_word16	wStatus;
	ia_status_st stSmartCardStatus ;
	ia_word16	wType;
	ia_byte		bMajorVersion;
	ia_byte		bMinorVersion;
	ia_byte		acNationality[3];
	ia_char		acSerialNo[ MAX_SC_SER_NO_LEN ]; /* Including check digit. */
	ia_word16	wPatchLevel;
	ia_word16	wOwnerId;
	ia_byte     bVariant;
	ia_byte     bBuild;
}CA_SMARTCARD_INFO_T;

/***** CA_QC_EMM_SERVICE_INFO Query Control *****/

typedef struct
{
	ia_word32		wEmmHandle;
	ia_word16		wServiceHandle;
	ia_status_st	stGlobalStatus;
#ifndef TO_SOFTCELL_3_0 /*only for softcell 3.1.x or higher*/
	ia_status_st	stServiceStatus;
#endif
	ia_word16		wEmmPid;
	ia_bool			fEmmServiceDefine ;
	ia_byte			bRscCount;
	ia_word32       awRscType[MAX_SERVICE_RESOURCES];
	ia_status_st astRscStatus[MAX_SERVICE_RESOURCES];
}CA_EMM_SERVICE_INFO_T;

/***** 3.5	CA_QC_EMM_TLV_NOTIFY Query Control *****/
typedef struct
{
	ia_byte			bRequestType; 		/* Out */
	ia_uint32		wResourceId;			/* Out */
	ia_byte			bLength; 				/* Out */
	ia_byte			*pstEMMTLV; 			/* Out */
}CA_EMM_TLV_NOTIFY_T;
/***** CA_QC_DMB_EMM_SERVICE_INFO Query Control *****/
typedef struct
{
	ia_word32       wEmmHandle;
	ia_word16       wServiceHandle;			/* OUT */
	ia_status_st	stGlobalStatus;			/* OUT */
	#ifndef TO_SOFTCELL_3_0 /*only for softcell 3.1.x or higher*/
			ia_status_st    stServiceStatus;			/* OUT */
	#endif
	ia_byte			bEmmSubChId;			/* OUT */
	ia_word32		wEmmPA;					/* OUT */
	ia_byte			bRscCount;				/* OUT */
	ia_status_st	astRscStatus[MAX_SERVICE_RESOURCES]; /* OUT */
	ia_word32        awRscType[MAX_SERVICE_RESOURCES];
} CA_DMB_EMM_SERVICE_INFO_T;
/***** CA_QC_DMB_ECM_SERVICE_INFO Query Control *****/
typedef struct
{
	ia_byte			bContentSubChId;
	ia_word16		wContentPA;
	ia_byte			bEcmSubChId;
	ia_word16		wEcmPA;
	ia_status_st	stStatus;
} DMB_STREAM_INFO_T;
typedef struct
{
		ia_word32       	wGroupHandle;
		ia_word16       	wServiceHandle;
		ia_status_st    	stGlobalStatus;
#ifndef TO_SOFTCELL_3_0 /*only for softcell 3.1.x or higher*/
		ia_status_st    	stServiceStatus;
#endif
		ia_byte         		bStreamCount;
		DMB_STREAM_INFO_T   astStreamInfo[MAX_SOFTCELL_STREAMS];
		ia_byte         		bRscCount;
		ia_word32       	awRscType[MAX_SERVICE_RESOURCES];
		ia_status_st    	astRscStatus[MAX_SERVICE_RESOURCES];
}CA_DMB_DESCRAMBLE_INFO_T;
/***** CA_QC_DESCRAMBLE_SERVICE_INFO Query Control *****/

typedef struct
{
	ia_word16		wStreamPid;
	ia_word16		wEcmPid;
	ia_status_st	stStatus;
}STREAM_INFO_T;

/*sren add CA_COPYCONTROL_INFO_T 2006.09.11*/
typedef struct
{
	ia_word16					wTypeMask;
	ia_word16					wLength;
	ia_byte						bMode;
	vd_copycontrol_cci_e		eCopyControlInfoDigital;
	vd_copycontrol_cci_e		eCopyControlInfoAnalog;
} CA_COPYCONTROL_INFO_T;

/* the structure of the substatus of the SESSION_MANAGEMENT_DRM resource */
typedef struct
{
	ia_status_st	stStatus;				 /* sub status */
	ia_word32		wSessionResourceId;      /* Resource ID */
	ia_word32     	wRightsRecId;			 /* Rights Record ID */
	ia_word32		wSequenceRecId;			 /* Sequence Record ID */
	ia_bool			fRecordFlagsUpdated;
	ia_bool			fRightsRecordUpdated;
	ia_bool			fSequenceRecordUpdated;
	ia_bool			fNonPVRable;
	ia_bool			fMustExpire;
	ia_word32		wCRID;
	msg_sm_drm_copycontrol_e eCopyControl;
	ia_word32		wPlaybackWindowStartTime;
	ia_word32		wPlaybackWindowDuration;
	ia_byte			bNrOfPlaybacks;
} CA_PVR_SERVICE_INFO_T;

typedef struct
{
	ia_word32		wGroupHandle;
	ia_word32		wServiceType;
	ia_word16		wServiceHandle;
	ia_status_st	stGlobalStatus;
#ifndef TO_SOFTCELL_3_0 /*only for softcell 3.1.x or higher*/
	ia_status_st	stServiceStatus;
#endif
	ia_byte			bStreamCount;
	STREAM_INFO_T	astStreamInfo[MAX_SOFTCELL_STREAMS];
	ia_byte			bRscCount;
	ia_word32       awRscType[MAX_SERVICE_RESOURCES];
	ia_status_st	astRscStatus[MAX_SERVICE_RESOURCES];
	CA_PVR_SERVICE_INFO_T stCaPvrServiceInfo;
#ifdef COPY_CONTROL_SUBSTATUS_AVAILABLE
	CA_COPYCONTROL_INFO_T    stCopyControlInfo;/*sren*/
#endif
}SERVICE_GROUP_INFO_T;

/***** CA_QC_MR_TRIGGER_PIN Query Control *****/
typedef struct
{
		ia_word32       wGroupHandle;
		ia_word32       wServiceType;
		ia_byte         abMRPin[ 2 ];
		vd_rsc_id       wResourceId;
}CA_MR_PIN_INFO_ST;

/***** CA_QC_NOTIFY_ENABLE Query Control *****/

/* bit-mask for the corresponding notification. */
#define NOTIFY_MASK_SMARTCARD_INFO	0x00000001
#define NOTIFY_MASK_EMM_SVC_INFO		0x00000002
#define NOTIFY_MASK_DES_SVC_INFO		0x00000004
#define NOTIFY_MASK_MONITOR_EMM		0x00000008
#define NOTIFY_MASK_MONITOR_ECM		0x00000010
#define NOTIFY_MASK_PVR_SVC_INFO	0x00000020
#define NOTIFY_MASK_IPPV_INFO	 	0x00000040
#define NOTIFY_MASK_LPPV_INFO       0x00000080
#define NOTIFY_MASK_MR_TRIGGER_PIN  0x00000100
#define NOTIFY_MASK_HGPC_SMARTCARD_DATA	0x00000200
#define NOTIFY_MASK_HGPC_SOFTCELL_DATA	0x00000400
#define NOTIFY_MASK_HGPC_MSG_STATUS		0x00000800
#define NOTIFY_MASK_HGPC_EVENT			0x00001000
#define NOTIFY_MASK_DMB_EMM_SVC     		0x00002000
#define NOTIFY_MASK_DMB_ECM_SVC     		0x00004000
#define NOTIFY_MASK_MONITOR_DMB_EMM     0x00008000
#define NOTIFY_MASK_MONITOR_DMB_ECM     	0x00010000
#define NOTIFY_MASK_FATAL_ERROR     	0x00020000

#define NOTIFY_MASK_DISABLE_ALL  0x00000000 /* switch OFF all associated notifications in CA Status Screen. (DEFAULT)*/
#define NOTIFY_MASK_ENABLE_ALL   0xFFFFFFFF /* switch ON  all associated notifications in CA Status Screen. */

/***** CA_QC_PIN_CODE_CHANGE Query Control *****/
/***** CA_QC_PIN_CODE_CHECK Query Control *****/

#define CA_PARENTAL_PIN					0 /* The checking and changing of this pin are achieved by the manufacturer. */
#define CA_IPPV_PIN						1
#define CA_HOME_SHOP_PIN				2
#define CA_GENERAL_PIN					3

#define CA_PIN_FAIL_CARD_OUT			0
#define CA_PIN_CHECK_OK					0x50
#define CA_PIN_FAILED					0x51
#define CA_PIN_CHANGED					0x52
#define CA_PIN_TYPE_INVALID				0x53
#define CA_PIN_BLOCK					0x54

#define INVALID_SERVICE_SLOT			0xFF
#define INVALID_DEMUX					0xFF
#define INVALID_SMARTCARDID				0xFFFFFFFF
#define INVALID_MARCROVISIONID			0xFF
#define INVALID_SERVICETYPE	            0xFFFFFFFF

#define CA_PIN_CODE_BCD_MAX_SIZE		2

typedef struct
{
	ia_word32 wResourceId;
	ia_byte   bCodeIndex;
	ia_byte   abOldPin[CA_PIN_CODE_BCD_MAX_SIZE]; /* CA_PIN_CODE_BCD_MAX_SIZE BCD numbers */
	ia_byte   abNewPin[CA_PIN_CODE_BCD_MAX_SIZE]; /* CA_PIN_CODE_BCD_MAX_SIZE BCD numbers */
	ia_byte   bCodeStatus;
}CA_PIN_CODE_ACCESS_T;

/***** CA_QC_MONITOR_EMM Query Control *****/
typedef struct
{
	ia_word32	wEmmHandle;
	ia_word16   wServiceHandle;
	/* abEmmData is deprecated - do not use abEmmData from version 3.2.6 PVCS 23471 */
	/* acEmmString is a NULL terminated string that should be displayed in
	the CAStatus screen AS IS. The string has been formated as per Irdeto
	requirements */
	ia_char		acEmmString[MAX_EMM_MONITOR_LEN];
}CA_EMM_MONITOR_T;

/***** CA_QC_MONITOR_ECM Query Control *****/
typedef struct
{
	ia_word32	wGroupHandle;
	ia_word16   wServiceHandle;
	/* abEcmData is deprecated - do not use abEcmData from version 3.4.2 PVCS 25272 */
	/* acEcmString is a NULL terminated string that should be displayed in
	the CAStatus screen AS IS. The string has been formated as per Irdeto
	requirements */
	ia_char		acEcmString[MAX_ECM_MONITOR_LEN];
}CA_ECM_MONITOR_T;

/***** CA_QC_SMARTCARD_ACCESS Query Control *****/

/* bAccessOp */
#define CA_SC_ACCESS_READ   0
#define CA_SC_ACCESS_WRITE  1

typedef struct
{
	ia_word32 wResourceId;
	ia_byte   bAccessOp;
	ia_byte   bLength;
	ia_byte  *pbData;
}CA_SC_ACCESS_T;

/***** CA_QC_HASH_ID Query Control *****/

typedef struct
{
	ia_word32 wResourceId;
	ia_word32 wClientHandle;
	ia_byte   bHashedNoLen;
	ia_byte  *pbHashedNo;
}CA_HASH_ID_T;

/***** CA_QC_MAIL Query Control *****/

/* message Type */
#define CA_MAIL     0
#define CA_ANNOUNCE 1

/* message Class */
#define CA_MAIL_NORMAL   0
#define CA_MAIL_TIMED    1  /* automatic erase after date/time */
#define CA_MAIL_FORCED   2  /* forced display */

/* message Priority */
#define CA_MAIL_PRIORITY_LOW    0
#define CA_MAIL_PRIORITY_MED    1
#define CA_MAIL_PRIORITY_HIGH   2
#define CA_MAIL_PRIORITY_URGENT 3

typedef struct
{
	ia_byte   bYear;      /* 95 to 99 for 1995 - 1999 or 00 to 94 for 2000 to 2094 */
	ia_byte   bMonth;     /* 1 to 12 */
	ia_byte   bDay;       /* 1 to 31 */
	ia_byte   bHour;      /* 0 to 23 */
	ia_byte   bMinute;    /* 0 to 5, a 10 minute interval. 0 for 0--9, 1 for 10--19 etc */
}TIMESTAMP;

typedef struct
{
	ia_byte   bIndex;
	ia_word32 wEmmHandle;                   /* OUT */
	ia_byte   bMsgLength;
	ia_word16 wServiceHandle;               /* OUT */
	ia_byte   bMessageData[MAX_EPG_MSG_LENGTH];
	ia_byte   bType;
	ia_byte   bClass;
	ia_byte   bPriority;
	ia_bool   fTimedDelete; /* FALSE means stDeleteTime is meaningless. */
	TIMESTAMP   stDeleteTime;
}CA_MAIL_T;

/***** CA_QC_EMM_DOWNLOAD Query Control *****/

typedef struct
{
	ia_word32 wEmmHandle;					/*IN*/
	ia_bool   fDownloadAllowed;
	ia_bool   fForcedDownload;
	ia_bool   fProfdecForcedDownload;
}CA_EMM_DOWNLOAD_T;

/***** CA_QC_EMM_CHANGEPIN Query Control *****/

typedef struct
{
	ia_word32 wGroupHandle;					/*IN*/
	ia_byte   bParentalPINIndex;    /* currently it is always set to zero. */
	ia_byte   abParentalPINCode[4]; /* 4 BCD numbers */
}CA_EMM_CHANGE_PIN_T;

/***** CA_QC_EMM_DECODER_RECOVER Query Control *****/

/* Decoder Recovery Types */
enum
{
	RESET_IRD = 0,
	RESET_CHANNEL_ORDER,
	RE_SCAN_SI_RESET_ORDER,
	ERASE_SI_RESET_RETUNE,
	RECONNECT_TO_SERVICE,
	FORCE_TUNE_TO_SERVICE,
	FORCE_SET_PIN_CODE
	/* Values 0x07 to 0x0f are spare */
};

typedef struct
{
	ia_word32	wGroupHandle;
	ia_word16   wNetworkID;
	ia_word16   wTransportStreamID;
	ia_word16   wServiceID;
}FORCE_TUNE_TO_SVC_T;

typedef struct
{
	ia_byte   bRecoveryType;  /* Decoder Recovery Types */
	ia_word32 wGroupHandle;					/*IN*/
	ia_word16   wBouquetID;                       /* for RESET_CHANNEL_ORDER   */
	FORCE_TUNE_TO_SVC_T   stForceTuneToSvc;  /* for FORCE_TUNE_TO_SERVICE */
	ia_byte   abInstallerPINCode[4];          /* for FORCE_SET_PIN_CODE    */
}CA_EMM_DECODER_RECOVER_T;

/***** CA_QC_NOTIFY_CURRENT_ERROR Query Control *****/
/* Channel Status Types */
enum
{
	POWER_ON = 0,
	NEW_SERVICE,
	CLEAR_SERVICE,
	SCRAMBLE_SERVICE
};

typedef struct
{
	ia_word32		wGroupHandle;
	ia_word32		wServiceType;
	ia_word16       wServiceHandle;
	ia_status_st	stGlobalStatus ;
}CA_ERROR_INFO_T;

/***** CA_QC_ATTRIBUTED_DISPLAY_MESSAGE Query Control *****/
typedef struct
{
	ia_byte   bIndex;                           /* IN */
	ia_word32 wEmmHandle;                       /*OUT*/
	ia_word16 wServiceHandle;                   /*OUT*/
	ia_word16 wMsgLength;
	ia_byte   bMessageData[MAX_EPG_MSG_LENGTH];
	ia_byte   bType;
	ia_byte   bDisplayMethod;
	ia_word16 wDuration;
} CA_ATTRIB_T;

typedef struct
{
	ia_byte   bIndex;                           /* IN */
	ia_word32 wEmmHandle;                       /*OUT*/
	ia_word16 wServiceHandle;                   /*OUT*/
	ia_word16 wMsgLength;
	ia_byte   bMessageData[MAX_EPG_MSG_LENGTH];
	ia_byte   bType;
	ia_byte   bDisplayMethod;
	ia_word16 wDuration;
	ia_byte   bFingerprintType;
	ia_byte   bRFU;
} CA_ATTRIB_T_2;

/***** CA_QC_IPPV_PREVIEW Query Control *****/
typedef struct
{
	ia_word32   wResourceId;
	ia_byte     bSector;
	ia_word16	wServiceCount;
	ia_word32	*wGroupHandle/*[CA_SERVICE_MAX]*/;
	ia_word16	wIPPVEventID;
	ia_word16	wEventCost;
	ia_byte		bClockValue;
	ia_byte		bPreviewValue;
}CA_IPPV_PREVIEW_T;

/***** CA_IPPV_PURCHASE Query Control *****/
typedef struct
{
	ia_byte		bRequestType;
	ia_word32   wResourceId;
	ia_byte     bSector;
	ia_word32	wGroupHandle;
	ia_word16	wIPPVEventID;
	ia_byte		bIPPVPin[2];
	ia_byte		bSCpinstatus;
} CA_IPPV_PURCHASE_T;

/***** CA_IPPV_SECTOR_INFO Query Control *****/
typedef struct
{
	ia_byte		bRequestType;       /* IN */
	ia_word32	wResourceId;       /* IN */
	ia_byte		bSector;            /* IN */
	ia_word16	wDebitLimit;        /* OUT */
	ia_word16	wDebitThreshold;    /* OUT */
	ia_word16	wOperatorPurse;		/* OUT */
	ia_word16   wCustomerPurse;		/* OUT */
	ia_byte	    bMaxNoIppvEvents;   /* OUT */
}CA_IPPV_SECTOR_INFO_T;

/***** CA_QC_IPPV_PURSE Query Control *****/
typedef struct
{
	ia_byte		bRequestType;
	ia_word32	wResourceId;
	ia_byte		bSector;
	ia_word16	wOperatorPurse;
	ia_word16	wCustomerPurse;
}CA_IPPV_PURSE_T;

/***** CA_IPPV_SET_THRESHOLD Query Control *****/
typedef struct
{
	ia_word32	wResourceId;
	ia_byte		bSector;
	ia_word16	wDebitThreshold;
}CA_IPPV_SET_THRESHOLD_T;

/***** CA_EVENT_RECORDS Query Control *****/
typedef struct
{
	ia_word16	wIPPVEventID;
	ia_bool		fEventReported;
	ia_word16	wDateCode;
	ia_word16	wEventCost;
}CA_IPPV_EVENT_RECORDS_T;

/***** CA_IPPV_IPPV_EVENTS_LIST Query Control *****/
typedef struct
{
	ia_byte		bRequestType;
	ia_word32	wResourceId;
	ia_byte		bSector;
	ia_byte		bDaysGrace;
	ia_byte		bStatus;
	ia_byte		bIPPVEventCount;
	CA_IPPV_EVENT_RECORDS_T *pstEventRecords;
	void		*next;
}CA_IPPV_EVENTS_LIST_T;

/***** CA_IPPV_EVENT_CHECK Query Control *****/
typedef struct
{
	ia_byte		bSector;
	ia_bool		fSectorValid;
	ia_bool		fEventValid;
	ia_word16	wEvent;
	ia_word16	wDateCode;
	ia_bool		fReported;
	ia_word16	wCost;
}CA_IPPV_EVENT_CHECK_T;

/***** CA_IPPV_EVENTS_CHECK Query Control *****/
typedef struct
{
	ia_byte		bRequestType;
	ia_word32	wResourceId;
	ia_byte		bDaysGrace;
	ia_byte		bStatus;
	ia_byte		bIPPVEventCount;
	CA_IPPV_EVENT_CHECK_T	*pstEventCheck;
}CA_IPPV_EVENTS_CHECK_T;

/* jyou: 3.4.2 STB Reporting */
/***** CA_CALLBACK_REQUEST *****/
typedef struct{
	ia_word32	wResourceId;
	ia_byte		bSectorID;
	ia_byte		bPhoneNumberLength;
	ia_byte		bPhoneControl;
	ia_word16	wDelay;
	ia_byte		bDelayMultiplier;
	ia_byte		*pabPhoneNumber;
}CA_IPPV_CALLBACK_REQUEST_T;

/* CA_IPPV_PHONE_NUMBER */
typedef struct{
	ia_byte		bRequestType;		/* In */
	ia_word32	wResourceId;		/* In */
	ia_byte		bSectorID;			/* In */
	ia_byte		bPhoneNumberLength; /* Out */
	ia_byte		*pabPhoneNumber;	/* Out */
}CA_IPPV_PHONE_NUMBER_T;

/* Add LPPV support */
/***** 1. CA_QC_LPPV_AWARE Query Control *****/
typedef struct
{
		ia_byte     bSector;
		ia_byte     bSCPPVMode;
}CA_LPPV_PPV_MODE_T;

typedef struct
{
		ia_byte     bRequestType;       /* In */
		vd_rsc_id   wResourceId;        /* In */
		ia_byte     bSTBPPVMode;        /* In */
		ia_byte     bSCPPVMode;         /* Out */
		ia_byte     bSectorCount;       /* Out */
		CA_LPPV_PPV_MODE_T *pstLPPVMode ;       /* Out */
}CA_LPPV_AWARE_T;

/***** 2. CA_QC_LPPV_PREVIEW Query Control *****/
typedef struct
{
		ia_word16   wTicketId;
		ia_word16   wCost;
		ia_word16   wExpiryDataCode;
		ia_byte     bPurchaseStatus;
}CA_LPPV_PPV_EVENT_TICKET_T;

typedef struct
{
		vd_rsc_id   wResourceId;        /* Out */
		ia_byte     bSector;            /* Out */
		ia_byte     bPreviewWindow;     /* Out */
		ia_byte     bBuyWindow;         /* Out */
		ia_byte     bTicketCount;        /* Out */
		CA_LPPV_PPV_EVENT_TICKET_T *pstEventTickets ;   /* Out */
}CA_LPPV_PREVIEW_T;

/***** 3. CA_QC_LPPV_READ_TICKET Query Control *****/
typedef struct
{
		ia_word16       wTicketId;
		ia_word16       wCost;
		ia_word16       wDateCode;
		ia_word16       wExpiryDataCode;
}CA_LPPV_PPV_READ_TICKET_INFO_T;

typedef struct
{
		ia_byte     bRequestType;       /* In */
		vd_rsc_id   wResourceId;        /* In */
		ia_byte     bSector;            /* In */
		ia_byte     bTicketMode;        /* In */
		ia_byte     bTicketCount;       /* Out */
		CA_LPPV_PPV_READ_TICKET_INFO_T *pstReadTickets ;    /* Out */
}CA_LPPV_READ_TICKET_T;

/***** 4. CA_QC_LPPV_DELETE_TICKET Query Control *****/
typedef struct
{
		vd_rsc_id   wResourceId;        /* In */
		ia_byte     bSector;            /* In */
		ia_word16   wTicketId;          /* In */
		ia_byte     abPIN[2];           /* In */
}CA_LPPV_DELETE_TICKET_T;

/***** 5. CA_QC_LPPV_READ_CREDIT Query Control *****/
typedef struct
{
		ia_byte     bRequestType;           /* In */
		vd_rsc_id   wResourceId;            /* In */
		ia_byte     bSector;                /* In */
		ia_word16   wCredit;                /* Out */
		ia_word16   wCurrentDC;             /* Out */
		ia_word16   wExpiryDC;              /* Out */
		ia_bool     fClearOnExpiry;         /* Out */
		ia_word16   wLastIncreaseCredits;   /* Out */
		ia_word16   wLastIncreaseDC;        /* Out */
}CA_LPPV_CREDIT_INFO_T;

/***** 6. CA_QC_LPPV_READ_ORDER Query Control *****/
typedef struct
{
		ia_word16   wTicketId;
		ia_bool     fIgnoreBuyWindow;
		ia_word16   wMaxCost;
		ia_word16   wExpiryDataCode;
}CA_LPPV_ORDER_INFO_T;

typedef struct
{
		ia_byte     bRequestType;           /* In */
		vd_rsc_id   wResourceId;            /* In */
		ia_byte     bSector;                /* In */
		ia_byte     bOrderCount;            /* Out */
	ia_byte     bStatus;                /* Out */
		CA_LPPV_ORDER_INFO_T *pstOrders;    /* Out */
}CA_LPPV_READ_ORDER_T;

/***** 7. CA_QC_LPPV_PLACE_ORDER Query Control *****/
typedef struct
{
		vd_rsc_id   wResourceId;            /* In */
		ia_byte     bSector;                /* In */
		ia_word16   wTicketId;              /* In */
		ia_bool     fIgnoreBuyWindow;       /* In */
		ia_word16   wMaxCost;               /* In */
		ia_word16   wExpiryDataCode;        /* In */
		ia_byte     abPIN[2];               /* In */
}CA_LPPV_PLACE_ORDER_T;

/***** 8. CA_QC_LPPV_CANCEL_ORDER Query Control *****/
typedef struct
{
		vd_rsc_id   wResourceId;            /* In */
		ia_byte     bSector;                /* In */
		ia_word16   wTicketId;              /* In */
		ia_byte     abPIN[2];               /* In */
}CA_LPPV_CANCEL_ORDER_T;

/* LPPV sector end */


/* Add MR support */
typedef enum
{
	CA_MR_CONFIG_READ		= 0,
	CA_MR_CONFIG_UPDATE
}CA_MR_CONFIG_ACTION_EN;


typedef enum
{
	CA_MR_CONFIG_OK			= 0,
	CA_MR_CONFIG_CARD_ERROR,
	CA_MR_CONFIG_CARD_MR_INCAPABLE,
	CA_MR_CONFIG_PIN_WRONG,
	CA_MR_CONFIG_PIN_BLOCK,
	CA_MR_CONFIG_DISABLE_MR_NOT_ALLOWED
}CA_MR_CONFIG_RESULT_EN;


typedef enum
{
	CA_MR_USER_PROFILE_READ = 0,
	CA_MR_USER_PROFILE_SET,
	CA_MR_USER_PROFILE_DELETE
}CA_MR_PROFILE_ACTION_EN;


typedef struct
{
	ia_byte	bProfileID;
	ia_byte	bUserAge;
	ia_byte	abUserPin[ 2 ];
}CA_MR_USER_PROFILE_DATA_T;

typedef struct
{
	vd_rsc_id	wResourceId;		/* In */
}CA_MR_CAPABILITY_T;

typedef struct
{
	 ia_byte     bRequestType;   			/* IN */
	vd_rsc_id	wResourceId;				/* In */
	ia_byte		abSubscriberPin[ 2 ]; 	/* In */
	CA_MR_CONFIG_ACTION_EN		eAction;	/* In */
	ia_bool		fEnable;						/* In/Out */
	CA_MR_CONFIG_RESULT_EN     eResult; /* Out */
}CA_MR_CONFIGURATION_T;

typedef struct
{
	 ia_byte     bRequestType;   			/* IN */
	vd_rsc_id	wResourceId;				/* In */
	ia_byte		abSubscriberPin[ 2 ]; 	/* In */
	CA_MR_PROFILE_ACTION_EN		eAction;	/* In */
	CA_MR_USER_PROFILE_DATA_T stUserProfile;		/* In/Out */
}CA_MR_PROFILE_T;
/* MR sector end */

/* Regional Filter */
typedef struct
{
	 ia_byte     bDescrCount;   			/* IN */
	 ia_byte     *bMpegDescr;				/* IN */
}CA_RF_SET_T;
/* Regional Filter end */


#ifdef CA_DMB_CLIENT
typedef struct
{
		ia_byte         bContentSubChId;
		ia_word16       wContentPA;
		ia_byte         bEcmSubChId;
		ia_word16       wEcmPA;
		ia_status_st    stStatus;
} DMB_STREAM_INFO_T;

typedef struct
{
		ia_byte                 bMappingTableSubChId;   /* IN */
		ia_status_st            stGlobalStatus;         /* OUT */
		ia_status_st            stServiceStatus;        /* OUT */
		ia_byte                 bStreamCount;           /* OUT */
		DMB_STREAM_INFO_T       astStreamInfo[MAX_SOFTCELL_STREAMS]; /* OUT */
		ia_byte                 bRscCount;              /* OUT */
		ia_status_st            astRscStatus[MAX_SERVICE_RESOURCES]; /* OUT */
} CA_DMB_DESCRAMBLE_INFO_T;

typedef struct
{
		ia_byte         bMappingTableSubChId;   /* IN */
		ia_status_st    stGlobalStatus;         /* OUT */
		ia_status_st    stServiceStatus;        /* OUT */
		ia_word32       wEmmPA;                 /* OUT */
		ia_byte         bRscCount;              /* OUT */
		ia_status_st    astRscStatus[MAX_SERVICE_RESOURCES]; /* OUT */
} CA_DMB_EMM_SVR_INFO_T;

typedef struct
{

		ia_byte     bRequestType;           /*  IN */
		ia_uint32   wResourceId;            /* OUT */
		ia_word16   wStatus;                /* OUT */
		ia_word16   wVendorId;              /* OUT */
} CA_VENDOR_ID_T;
#endif

/***** CA_REMOTE_CHANNEL Query Control *****/
typedef struct{
	ia_byte		bRequestType;           /* IN */
	ia_word32	wResourceId;           /* IN */
	ia_byte 	bPayloadLengthMax;    /* IN/OUT */
	ia_byte 	*pbPayload;             /* IN/OUT */
}CA_REMOTE_CHANNEL_T;

/***** CA_QC_SMARTCARD_NUMBER_OF_SECTORS Query Control *****/
typedef struct
{
	ia_byte			bSector;		/* OUT  */
	ia_byte			bActive;		/* OUT  */
	ia_byte			bReserved;		/* OUT  */
}CA_SC_SECTOR_T;

typedef struct
{
	ia_word32			wResourceId;			 /* IN */
	ia_byte				bSectorCount;			/* IN  */
	CA_SC_SECTOR_T      *pstSectors;			/* OUT */
}CA_SC_SECTORLIST_T;

/***** CA_QC_REGION Query Control *****/
typedef struct
{
	ia_word32   wResourceId;			 /* IN */
	ia_byte		bRequestType;            /* IN */
	ia_byte		bSector;                 /* IN */
	ia_byte		bSectorStatus;           /* OUT */
	ia_byte		bRegion;                 /* OUT */
	ia_byte		bSubRegion;              /* OUT */
}CA_REGION_T;


/***** CA_QC_PPT_INFO Query Control *****/
typedef struct
{
	ia_byte		pbPPTLable[ PPT_LABLE_LEN ];             /* OUT */
	ia_word16	wPPTViewTimeLeft;           /* OUT */
	ia_byte		bPPTStatus;                 /* OUT */
}CA_PPT_RECORD_T;

typedef struct
{
	ia_byte				bRequestType;					/* IN */
	ia_word32			wResourceId;					/* IN */
	ia_byte       		bSector;						/* IN */
	ia_byte				bPPTGlobalStatus;				/* OUT */
	ia_byte				bPPTSectorStatus;				/* OUT */
	ia_byte				bPPTRecordCount;				/* IN */
	CA_PPT_RECORD_T     *pstPPTRecords;					/* OUT */
	void                *next;
}CA_PPT_T;

/***** CA_QC_SURFLOCK Query Control *****/
typedef struct
{
	ia_byte				bRequestType;					/* IN */
	ia_word32			wResourceId;
	ia_bool				fSurfLocked;						/*OUT*/
	/*	ia_word16			caLockedServiceID;			*/	/*OUT*/
	ia_word32           wGroupHandle;							/*OUT*/
} CA_SURFLOCK_T;

/***** CA_QC_SURFLOCK Query Control *****/
/*typedef struct
{
ia_byte				bFlexiFlashStatus;
ia_word16			wOwnerID;
ia_byte				abVersion[4];
ia_byte				bVariant;
ia_word16			wPatchLevel;
ia_word16			wPatchID;
ia_word16			wTargetPatchLevel;
ia_byte				bLastProcessedBlock;
ia_byte				bLastBlock;
} CA_FLEXIFLASH_PATCH_INFO_T;*/

/***** CA_QC_EXTENDED_PRODUCT_LIST Query Control *****/

typedef struct
{
	ia_word16          				wProduct;				/* OUT */
	ia_word16          				wStartDate; 			/* OUT */
	ia_byte        				    bDuration;				/* OUT */
}CA_SECTOR_EXTEND_PRODUCT_T;

typedef struct
{
	ia_byte        				bSector;             		/* OUT */
	ia_byte          			bSectorStatus; 				/* OUT */
	ia_word16        			wSectorDateCode;			/* OUT */
	ia_byte                     bProductCount;				/* OUT */
	ia_word16                   wMaxSizeExtendProducts;		/* OUT */
	ia_word16                   wOrgSizeExtendProducts;		/* OUT */
	CA_SECTOR_EXTEND_PRODUCT_T  *pstExtendProducts;			/* OUT */
}CA_SECTOR_EXTEND_SECTOR_T;

typedef struct
{
	ia_byte          			bRequestType;				/* IN */
	ia_word32          			wResourceId;				/* IN */
	ia_byte          			bSectorCount; 				/* IN */
	ia_word16        			wMaxSizeSectorProducts;		/* IN */
	ia_word16        			wOrgSizeSectorProducts;		/* OUT */
	CA_SECTOR_EXTEND_SECTOR_T	*pstSectorProducts;			/* OUT */
	ia_bool						fExpiredProductsExist;
} CA_EXTENDED_PRODUCT_LIST_T;

/* for CA query product check */
/* mli: 3.3.11 changed interface structures???*/
#if	0
typedef struct
{
	ia_word32          	wResourceId;				/* IN */
	ia_byte				sectorIndex;
	ia_word16			wProduct;
}CA_QUERY_PRODUCT_T;

typedef struct
{
	ia_word32          		wResourceId;				/* IN */
	ia_byte					productNumber;
	CA_QUERY_PRODUCT_T		*pstProduct;
}CA_QUERY_SECTOR_PRODUCT_T;

typedef struct
{
	ia_word32          	wResourceId;				/* IN */
	ia_byte				sectorNumber;
	CA_QUERY_SECTOR_PRODUCT_T	*pstQuerySector;
}CA_QUERY_PRODUCT_CHECK_T;
#endif
/* for CA query	product	check */
typedef	struct
{
	ia_byte	bSector;								/* IN/OUT */
	ia_bool	fAnySector;							  /* IN	*/
	ia_bool	fSectorValid;						  /* OUT */
	ia_bool	fProductValid;						 /*	OUT	*/
	ia_word16 wProduct;							  /* IN/OUT	*/
}CA_QUERY_SECTOR_PRODUCT_T;

typedef	struct
{
	ia_byte	bRequestType ;
	ia_word32	wResourceId;				/* IN/OUT */
	ia_byte	bProductCount;							/* IN/OUT */
	ia_byte	bGracePeriod;							 /*	IN */
	ia_bool	fKeepTrack;								  /* IN	*/
	CA_QUERY_SECTOR_PRODUCT_T *pstQuerySector;
}CA_QUERY_PRODUCT_CHECK_T;

typedef	struct
{
	ia_byte	bRequestType ;
	ia_word32	wResourceId;
	ia_byte	bSectorCount;
	ia_byte	sectorIndex[ MAX_SECTOR_NUMBER ];
	CA_QUERY_SECTOR_PRODUCT_T stQuerySector[MAX_SECTOR_NUMBER];
}CA_ANY_PRODUCT_CHECK_T;

#ifdef INCL_PRODUCT_BLOCKING
#ifndef ANY_FTA_PRODUCT
#define ANY_FTA_PRODUCT		0xFFFF
#endif

#ifndef MAX_FTA_PRODUCTS
#define MAX_FTA_PRODUCTS	5
#endif

typedef struct
{
	ia_word16          wProduct;
	ia_byte            bSector;
	/* mli: 3.3.11 */
	 ia_bool	fSectorValid;						  /* OUT */
	 ia_bool	fProductValid;						 /*	OUT	*/
}CA_PRODUCT_T;

/* mli: 3.3.11 */
typedef struct
{
	ia_byte		   bRequestType ;
	ia_word32      wResourceId;
	ia_byte        bFTAOnly;
	ia_byte        bNumEntries;
	CA_PRODUCT_T   astProduct[MAX_FTA_PRODUCTS];
} CA_PRODUCT_BLOCK_T;

#endif


typedef struct
{
	ia_word32          	wResourceId;				/* IN */
	ia_byte  abUserData[USER_DATA_LENGTH]; /* OUT USER_DATA_LENGTH = 28 */
}CA_SMARTCARD_USER_DATA_T;


/* PVR DRM */
typedef struct
{
	ia_word32  wSessionResourceId;
	ia_bool     fUseDrm;
	ia_word16  wDescrambleServiceHandle;
	ia_bool    fFtaCheckAnySector;
	ia_byte    bFtaCheckSectorNr;
	ia_bool    fChainRecordings;
	msg_sm_drm_pvr_purpose_e ePvrPurpose;
	ia_word32  wChainingSessionResourceId;
} CA_DRM_PVR_RECORD_T;

typedef struct
{
	ia_word32  wSessionResourceId;
	ia_bool     fUseDrm;
	ia_word32  wRightsRecordId;
	ia_word32  wSequenceRecordId;
	ia_bool    fPeekRightsOnly;
	msg_sm_drm_pvr_purpose_e ePvrPurpose;
	msg_sm_drm_playback_direction_e ePlaybackDirection;
} CA_DRM_PVR_PLAYBACK_T;


/* IRD Decoder EMM User Playload */
typedef struct
{
	ia_word32    wEmmHandle;			/*IN/OUT*/
	ia_word16    wMessageLength;
	ia_byte      *pbMessage;
} CA_EMM_USER_PAYLOAD_DATA_T;

#ifdef _AUSTAR_
extern int SynScMessage;
extern int SynSendMessage;
#endif

/******************************************************************************/
/* Definitions provided for ECT v3.4-1 Smartcard Diagnose                     */
/******************************************************************************/

#define MAX_VENDOR_COUNT		4

/***** CA_QC_SMARTCARD_UNIQUE_ADDRESS Query Control *****/
/* Smart Card Unique Address */
#define UNIQUE_ADDRESS_LENGTH 3

typedef struct
{
	ia_byte		bRequestType;							/* IN */
	ia_word32	wResourceId;							/* IN */
	ia_byte		abUniqueAddress[UNIQUE_ADDRESS_LENGTH];	/* OUT */
	ia_byte		bEMMFilterNumber;						/* OUT */
} CA_SMARTCARD_UNIQUE_ADDRESS_T;


/***** CA_QC_SMARTCARD_VENDOR_ID Query Control *****/
typedef struct
{
	ia_word16			wVendorID;
	ia_bool				fActive;
} CA_VENDOR_ID_T;

typedef struct
{
	ia_byte			bRequestType;					/* IN */
	ia_word32		wResourceId;					/* IN */
	ia_byte			bNumOfVendorIDs;				/* OUT */
	CA_VENDOR_ID_T	*pstVendorIDs;					/* OUT */
} CA_SMARTCARD_VENDOR_ID_T;

/***** CA_QC_SMARTCARD_SECTOR_ADDRESS Query Control *****/
#define SECTOR_ADDRESS_LENGTH 3
typedef struct
{
	ia_byte		bRequestType;							/* IN */
	ia_word32	wResourceId;							/* IN */
	ia_byte		bSectorID;								/* IN */
	ia_bool		fSectorValid;							/* OUT */
	ia_byte		abSectorAddress[SECTOR_ADDRESS_LENGTH]; /* OUT */
} CA_SMARTCARD_SECTOR_ADDRESS_T;

/***** CA_QC_SMARTCARD_SECTOR_DATECODE Query Control *****/
typedef struct
{
	ia_byte		bRequestType;	/* IN */
	ia_word32	wResourceId;	/* IN */
	ia_byte		bSectorID;		/* IN */
	ia_bool		fSectorValid;	/* OUT */
	ia_word16	wDateCode;		/* OUT */
} CA_SMARTCARD_SECTOR_DATECODE_T;
/********** Feature 3.4-1 Definitions End **********/

/***** CA_QC_SMARTCARD_OPERATOR_ID Query Control *****/
typedef struct
{
	ia_byte        	bRequestType;   /* IN */
	ia_word32	    wResourceId;	/* IN */
	ia_byte	        bSectorID;		/* IN */
	ia_bool			fSectorValid;		/* OUT */
	ia_word16	    wOperatorID;	/* OUT */
} CA_SMARTCARD_OPERATOR_ID_T;

/***** CA_QC_HPGC_SMARTCARD_DATA Query Control *****/
typedef struct
{
	msg_hgpc_smartcard_id_st	stSecCardId;
	/* Fixed SCR#27478:The null pointer in mih_hgpc_get_scell_data causes system crash */
	ia_byte 					abSecCardSerialNo[MAX_SC_SER_NO_LEN];
} CA_HGPC_SEC_CARD_T;

typedef struct
{
	msg_hgpc_method_e	eMethod;
	ia_word16			wHNARefreshTimeout;
	ia_word16			wHNARepeatInterval;
	ia_byte				bSecCardCount;
	CA_HGPC_SEC_CARD_T	*stSecCard;
}CA_HGPC_PRIMARY_SC_DATA_T;

typedef struct
{
	msg_hgpc_method_e			eMethod;
	ia_word16					wHNRTimeLeft;
	ia_word16					wActivationCode;
	msg_hgpc_smartcard_id_st  	stPrimCardId;
	/* Fixed SCR#27478:The null pointer in mih_hgpc_get_scell_data causes system crash */
	ia_byte 					abPrimCardSerialNo[MAX_SC_SER_NO_LEN];
}CA_HGPC_SECONDARY_SC_DATA_T;

typedef struct
{
	ia_byte						bRequestType;	/* In */
	vd_rsc_id			 		wResourceId; 	/* In */
	msg_hgpc_card_status_e	 	eCardStatus; 	/* Out */
	msg_hgpc_role_e		 		eCardRole;	 	/* Out */
	CA_HGPC_PRIMARY_SC_DATA_T   stPrimaryData;	/* Out */
	CA_HGPC_SECONDARY_SC_DATA_T stSecondaryData;/* Out */
}CA_HGPC_SMARTCARD_DATA_T;

/***** CA_QC_HPGC_SCELL_DATA Query Control *****/
typedef struct
{
	ia_byte							bRequestType;	/* In */
	msg_hgpc_role_e    				eScellRole; 	/* Out */
	msg_hgpc_scell_data_st			stScellData;	/* Out */
	/* Fixed SCR#27478:The null pointer in mih_hgpc_get_scell_data causes system crash */
	ia_byte							abPrimCardSerialNo[MAX_SC_SER_NO_LEN];/* Out */
}CA_HGPC_SCELL_DATA_T;

/***** CA_HGPC_MSG_STATUS Query Control *****/
typedef struct
{
	vd_rsc_id					wResourceId;	/* Out */
	msg_hgpc_message_type_e		eType;			/* Out */
	msg_hgpc_message_action_e	eAction;		/* Out */
	msg_hgpc_message_status_e	eStatus;		/* Out */
}CA_HGPC_MSG_STATUS_T;

/***** CA_QC_HPGC Local structure *****/
typedef struct
{
	vd_rsc_id			 		wResourceId; 	/* In/Out */
	/* SMART_DATA */
	msg_hgpc_card_status_e	 	eCardStatus; 	/* Out */
	msg_hgpc_role_e		 		eCardRole;	 	/* Out */
	CA_HGPC_PRIMARY_SC_DATA_T   stPrimarySCData;	/* Out */
	CA_HGPC_SECONDARY_SC_DATA_T stSecondarySCData;/* Out */
	/* SCELL_DATA */
	msg_hgpc_role_e    			eScellRole; 	/* Out */
	msg_hgpc_scell_data_st		stScellData;	/* Out */
	/* Fixed SCR#27478:The null pointer in mih_hgpc_get_scell_data causes system crash */
	ia_byte						abPrimCardSerialNo[MAX_SC_SER_NO_LEN];/* Out */
	/* MSG_STATUS */
	msg_hgpc_message_type_e		eType;			/* Out */
	msg_hgpc_message_action_e	eAction;		/* Out */
	msg_hgpc_message_status_e	eStatus;		/* Out */
}CA_HGPC_LOCAL_T;

/***** CA_QC_FATAL_ERROR Local structure *****/
enum
{
	CA_ERROR_MEMORY = 1,
	CA_ERROR_TASK,
	CA_ERROR_RESOURCE,
	CA_ERROR_MESSAGE,
	CA_ERROR_ASSERT,
	CA_ERROR_FATAL,
	CA_ERROR_INIT,
	CA_ERROR_POINTER,
	CA_ERROR_RULE
};

typedef enum
{
	CA_ERROR_SOURCE_SOFTCELL = 0,
	CA_ERROR_SOURCE_ECT
}CA_ERROR_SOURCE_EN;

typedef struct
{
	CA_ERROR_SOURCE_EN	eSource;
	ia_word16			wError;
}CA_FATAL_ERROR_ST;

/******************************************************************************/
/* Functions provided for the manufacturer.                                   */
/******************************************************************************/

ia_bool CA_TASK_Init(CA_DBG_LEVEL_EN eDebug, void *pstData);

typedef void(*CA_NOTIFICATION)( ia_word32 wHandle, CA_QUERY_CONTROL_EN eQuery, ia_uint32 ulParam );
void CA_TASK_RegisterCaNotify( CA_NOTIFICATION fnCallback );

CA_QUERY_STATUS_EN CA_TASK_QueryControl( ia_word32 wHandle, CA_QUERY_CONTROL_EN eQuery, void *pvParams );

CA_QUERY_STATUS_EN CA_MR_PinQueryControl(ia_word32 wHandle, void* pCAClientMsg);

ia_bool CA_TASK_SendMessage( MSG_ACTION_EN eAction, void *pvParams );

ia_bool CA_TASK_Close( void );


/******************************************************************************/
/* Functions implemented by the manufacturer.                                 */
/******************************************************************************/

#define CA_PRINTF libc_printf /**/

extern ia_bool CA_TestDebugError(void);
extern ia_bool CA_TestDebug(void);
extern ia_bool CA_TestDebugDVError(void);

#define CA_ERROR(debug)\
{ \
	if (CA_TestDebugError() == IA_TRUE) \
{ \
	CA_PRINTF debug; \
} \
} \


#define CA_DBG(debug)\
{ \
	if (CA_TestDebug() == IA_TRUE) \
{ \
	CA_PRINTF debug; \
} \
} \

#define CA_DBG_V(debug)\
{ \
	if (CA_TestDebugDVError() == IA_TRUE) \
{ \
	CA_PRINTF debug; \
} \
} \

#ifndef DEBUG_MEMORY
#	define	ALLOCATE_MEMORY(type, count)		(type *)OS_DRV_AllocateMemory((unsigned long)((unsigned long)count * (unsigned long)sizeof(type)))
#	define	FREE_MEMORY							OS_DRV_FreeMemory
#else
#	define	ALLOCATE_MEMORY(type, count)		(type *)DBG_AllocateMemory(__FILE__, __LINE__, (unsigned long)((unsigned long)count * (unsigned long)sizeof(type)))
#	define	FREE_MEMORY(ptr)			DBG_FreeMemory(__FILE__, __LINE__, ptr)
void *DBG_AllocateMemory(const char *pcFile, const int iLine, unsigned long ulSize);
void DBG_FreeMemory(const char *pcFile, const int iLine, void *pvMemory);

#endif /* DEBUG_MEMORY. */

/* Memory Protection function */
ia_bool CA_MEM_Protect( void* pvAddr, ia_uint32 uLen );

#endif /* _CA_TASK_H_ */

