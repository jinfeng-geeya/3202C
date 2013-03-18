/**
 * $Id: s3_smartcard.h,v 1.29 2007/08/01 09:02:10 axu Exp $
 *
 * Copyright 2001-2007 Irdeto Access B.V.
 *
 * This file and the information contained herein are the subject of copyright
 * and intellectual property rights under international convention. All rights
 * reserved. No part of this file may be reproduced, stored in a retrieval
 * system or transmitted in any form by any means, electronic, mechanical or
 * optical, in whole or in part, without the prior written permission of Irdeto
 * Access B.V.
 *
 * This file is part of the SoftCell 3 Integration library.
 **/

/**
 * This file is part of the 'Host CA-Task driver'.
 **/

#ifndef S3_SMARTCARD_H
#define S3_SMARTCARD_H

#include "s3_cadrv.h"
#include "s3_vdsc.h"

typedef struct
{
	vd_rsc_id     wResourceId;
	ia_status_st  stStatus;
} msg_smartcard_status_data_st;

typedef struct
{
	vd_rsc_id     wResourceId;
	ia_word16     wServiceHandle;
	ia_byte       bIdStringLength;
} msg_smartcard_id_data_st;

typedef struct
{
	vd_rsc_id     wResourceId;
} msg_smartcard_query_data_st;

typedef struct
{
	vd_rsc_id     wResourceId;
	ia_byte       acNationality[ 3 ];
} msg_nationality_data_st;

typedef struct
{
	vd_rsc_id     wResourceId;
	ia_byte       bSerialNoLength;
	ia_byte       bCheckDigit;
	ia_word16     wSmartcardType;
	ia_byte       bMajorVersion;
	ia_byte       bMinorVersion;
	ia_byte       bBuild;
	ia_byte       bVariant;
	ia_word16     wPatchLevel;
	ia_word16     wOwnerId;
} msg_smartcard_data_reply_data_st;

#define HOMING_CHANNEL_LENGTH	12
typedef struct
{
	vd_rsc_id     wResourceId;
	ia_byte       abHomingChannel[ HOMING_CHANNEL_LENGTH ];
} msg_smartcard_homing_reply_data_st;

/* CAM user data reply. */
#define USER_DATA_LENGTH	28
typedef struct
{
	vd_rsc_id     wResourceId;
	ia_byte       abUserData[ USER_DATA_LENGTH ];
} msg_smartcard_user_data_reply_data_st;

/* CBMS ESG DRM System Id string. */
typedef struct
{
	vd_rsc_id     wResourceId;
	ia_byte       bIdStringLength;
} msg_drm_system_data_st;

/* Pay Per Time. */
typedef struct
{
	vd_rsc_id     wResourceId;
	ia_byte       bSector;
} msg_ppt_info_query_st;

typedef enum
{
	PPT_GS_OK_V41 = 1,
	PPT_GS_OK_V5PLUS,
	PPT_GS_ERROR,
	PPT_GS_CARD_VERSION_INVALID,
	PPT_GS_V41_PPT_NOT_SUPPORTED
} msg_ppt_global_status_e;

typedef enum
{
	PPT_SS_OK = 1,
	PPT_SS_PPT_NOT_SUPPORTED,
	PPT_SS_NOT_ACTIVE,
	PPT_SS_ERROR
} msg_ppt_sector_status_e;

typedef struct
{
	vd_rsc_id               wResourceId;
	ia_byte                 bSector;
	msg_ppt_global_status_e ePPTGlobalStatus;
	msg_ppt_sector_status_e ePPTSectorStatus;
} msg_ppt_info_reply_st;

typedef struct
{
	ia_word16     wViewingTimeLeft;
	ia_byte       bPPTStatus;
} msg_ppt_record_v41_st;

typedef struct
{
	ia_byte       abPPTLabel[ 11 ];
	ia_word16     wViewingTimeLeft;
	ia_byte       bPPTStatus;
} msg_ppt_record_v5plus_st;

/* Surf Locking. */
typedef struct
{
	vd_rsc_id     wResourceId;
	ia_bool       fSurfLocked;
} msg_surf_lock_reply_st;

/* Get Region. */
typedef struct
{
	vd_rsc_id     wResourceId;
	ia_byte       bSector;
} msg_region_query_st;

typedef struct
{
	vd_rsc_id     wResourceId;
	ia_byte       bSector;
	ia_bool       fSectorValid;
	ia_byte       bRegion;
	ia_byte       bSubRegion;
} msg_region_reply_st;

/*Get Unique Address*/
typedef struct
{
	vd_rsc_id     wResourceId;
} msg_unique_addr_query_st;

typedef struct
{
	vd_rsc_id     wResourceId;
	ia_byte       abUniqueAddress[ 3 ];
	ia_byte       bEMMFilterNum;
} msg_unique_addr_reply_st;

/*Get Vendor IDs*/
typedef struct
{
	vd_rsc_id     wResourceId;
} msg_vendor_id_query_st;

typedef struct
{
	ia_word16     wVendorId;
	ia_bool       fActive;
} msg_vendor_id_st;

typedef struct
{
	vd_rsc_id     wResourceId;
	ia_byte       bNumOfVendorId;
} msg_vendor_id_reply_st;

/*Get Group Address*/
typedef struct
{
	vd_rsc_id     wResourceId;
	ia_byte       bSector;
} msg_sector_addr_query_st;

typedef struct
{
	vd_rsc_id     wResourceId;
	ia_byte       bSector;
	ia_bool       fSectorValid;
	ia_byte       abGroupAddress[ 3 ];
} msg_sector_addr_reply_st;

/*Get Datacode Address*/
typedef struct
{
	vd_rsc_id     wResourceId;
	ia_byte       bSector;
} msg_datecode_query_st;

typedef struct
{
	vd_rsc_id     wResourceId;
	ia_byte       bSector;
	ia_bool       fSectorValid;
	ia_word16     wDateCode;
} msg_datecode_reply_st;

/* Global card status values in ia_status_st structure. */
enum
{
	STATUS_CARD_IN          = 0,
	STATUS_CARD_OUT         = 4,
	STATUS_CARD_UNKNOWN     = 5,
	STATUS_CARD_ERROR       = 6,
	STATUS_CARD_VERIFYING   = 7,

	/* softclient support */
	/* download proxy */
	STATUS_CARD_NO_DLL,             /* need to download partial*/
	STATUS_CARD_NO_TABLE_AND_DLL,   /* need to download full */
	STATUS_CARD_DOWNLOAD_PART,
	STATUS_CARD_DOWNLOAD_FULL,
	/* security proxy */
	STATUS_CARD_IPTV_DONE,
	STATUS_CARD_LEASE_INVALID,
	STATUS_CARD_LEASE_RECEIVED,

	STATUS_JAVACARD_APPLET_NOT_FOUND = 125,
	STATUS_CARD_IN_WRONG_NETWORK = 144
};

/* Smartcard PIN action to perform. */
typedef enum
{
	IA_PIN_CHECK            = 0,
	IA_PIN_CHANGE,
	IA_PIN_CANCEL
} ia_pin_action_e;

/* Smartcard PIN types. */
typedef enum
{
	IA_PARENTAL_PIN         = 0,
	IA_IPPV_PIN             = 1,
	IA_HOME_SHOPPING_PIN	= 2,
	IA_GENERAL_PIN			= 3
} ia_pin_type_e;

typedef struct
{
	vd_rsc_id               wResourceId;
	ia_pin_action_e         ePinAction;
	ia_pin_type_e           ePinType;
	ia_byte                 abPinCode[ 2 ];
	ia_byte                 abNewPinCode[ 2 ];
} msg_pin_code_query_st;

typedef enum
{
	PIN_FAIL_CARD_OUT       = 0,
	PIN_CHECK_OK            = 0x50,
	PIN_FAILED              = 0x51,
	PIN_CHANGED             = 0x52,
	PIN_TYPE_INVALID        = 0x53,
	PIN_BLOCK               = 0x54,
	PIN_TYPE_NOT_ALLOW_ACCESS
} msg_pin_code_status_e;

typedef struct
{
	vd_rsc_id               wResourceId;
	ia_pin_action_e         ePinAction;
	ia_pin_type_e           ePinType;
	msg_pin_code_status_e   eResult;
} msg_pin_code_reply_st;

typedef struct
{
	vd_rsc_id               wResourceId;
	ia_byte                 bLength;
} msg_remote_channel_message_st;
/* ia_byte abPayload[bLength]; */

/* Number of sectors notification. */
typedef struct
{
	vd_rsc_id               wResourceId;
	ia_byte                 bSectors;
} msg_number_of_sectors_reply_data_st;

typedef struct
{
	ia_byte                 bSector;
	ia_bool                 fActive;
} msg_number_of_sectors_sector_st;

/*Get Operator Id from smartcard. */
typedef struct
{
	vd_rsc_id               wResourceId;
	ia_byte                 bSector;
} msg_operator_id_query_st;

typedef struct
{
	vd_rsc_id               wResourceId;
	ia_byte                 bSector;
	ia_bool                 fSectorValid;
	ia_word16               wOperatorId;
} msg_operator_id_reply_st;

typedef enum
{
	MARRY_MONOGAMY_ENABLED = 0,
	MARRY_BIGAMY_ENABLED,
	MARRY_NONE
} msg_marriage_status_e;

typedef struct
{
	vd_rsc_id               wResourceId;
} msg_marriage_status_query_st;

typedef struct
{
	vd_rsc_id               wResourceId;
	ia_bool                 fSTBMarriageEnabled;
	msg_marriage_status_e   eSTBMarriageType;	/* Monogamy or Bigamy or none */
	ia_bool                 fSCMarried;
	ia_byte                 abSCAddress[ 3 ];
	ia_byte                 abSCMarryData[ 4 ];
	ia_bool                 fSTBMarried;
	ia_byte                 abSTBAddress[ 3 ];
	ia_byte                 abSTBMarryData[ 4 ];
} msg_marriage_status_reply_st;

typedef struct
{
	vd_rsc_id               wResourceId;
	ia_bool                 fPrimary;
	ia_bool                 fValid;
	vd_sc_card_form_e       eForm;
} msg_smartcard_multicard_state_notify_st;

typedef enum
{
	SC_LPPV_ORDER_PLACE = 1,
	SC_LPPV_ORDER_CANCEL
} msg_lppv_ticket_order_action_e;

typedef struct 
{
	ia_byte	bDescrCount;
} msg_region_filter_set_info_st;

typedef enum
{
	REGION_FILTER_ON = 1,
	REGION_FILTER_OFF
} msg_region_filter_config_e;

typedef struct 
{
	msg_region_filter_config_e    eConfig;
} msg_region_filter_config_st;

#endif /* S3_SMARTCARD_H. */
