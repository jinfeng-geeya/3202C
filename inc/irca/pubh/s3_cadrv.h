/**
 * $Id: s3_cadrv.h,v 1.75 2008/04/17 12:35:38 ehietbrink Exp $
 *
 * Copyright 2001-2008 Irdeto Access B.V.
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

#ifndef S3_CADRV_H
#define S3_CADRV_H

/* Required to export public SoftCell APIs. Define externally for Windows DLLs etc.. */
#ifndef S3_EXPORT
# define S3_EXPORT extern
#endif

#include "s3_portable_types.h"
#include "s3_status.h"
#include "s3_vd.h"
#include "s3_vd_loader.h"

/* defines. */
#define CA_TASK_OP_SEQ_LOW    (0x0000)
#define CA_TASK_OP_SEQ_HIGH   (0x7FFF)
#define SOFTCELL_OP_SEQ_LOW   (0x8000)
#define SOFTCELL_OP_SEQ_HIGH  (0xFFFF)


/* typedefs. */


/* CA-Task message opcodes. Their values may *never* change! */
typedef enum
{
	/* Client identification. */
	MSG_CLIENT_VERSION_QUERY               = 1,
	MSG_COMPONENTS_VERSION_QUERY           = 2,
	MSG_VERSION_REPLY                      = 3,

	/* Generic errors. */
	MSG_ERROR                              = 4,

	/* Service control messages. */
	MSG_SERVICE_OPEN_REQUEST               = 5,
	MSG_SERVICE_OPEN_REPLY                 = 6,
	MSG_SERVICE_CLOSE                      = 7,
	/* MSG_SERVICE_CLOSE_REPLY = 112 defined below. */
	MSG_SERVICE_DEFINE                     = 8,
	MSG_SERVICE_STATUS_REQUEST             = 9,
	MSG_SERVICE_STATUS                     = 10,

	/* Smartcard messages - Identification. */
	MSG_SMARTCARD_STATUS                   = 11,
	MSG_NATIONALITY_QUERY                  = 12,
	MSG_NATIONALITY                        = 13,
	MSG_SMARTCARD_ID                       = 14,
	MSG_SMARTCARD_DATA_QUERY               = 15,
	MSG_SMARTCARD_DATA_REPLY               = 16,
	MSG_SMARTCARD_HOMING_CH_QUERY          = 17,
	MSG_SMARTCARD_HOMING_CH_REPLY          = 18,

	/* Decoder messages for the user. */
	MSG_DECODER                            = 19,

	/* Monitoring messages. */
	MSG_DVB_EMM_MONITOR                    = 20,
	MSG_DVB_ECM_MONITOR                    = 21,
	MSG_IPTV_EMM_MONITOR                   = 22,
	MSG_IPTV_REGISTRATION_ATTEMPTS         = 23,

	/* TMS info queries and replies. */
	MSG_SERVICE_INFO_QUERY                 = 24,
	MSG_SERVICE_INFO_REPLY                 = 25,
	MSG_TRANSPORT_INFO_QUERY               = 26,
	MSG_TRANSPORT_INFO_REPLY               = 27,
	MSG_NETWORK_INFO_QUERY                 = 28,
	MSG_NETWORK_INFO_REPLY                 = 29,

	/* IPPV messages. */
	MSG_IPPV_EVENT_NOTIFY                  = 30,
	MSG_IPPV_BUY_QUERY                     = 31,
	MSG_IPPV_BUY_REPLY                     = 32,
	MSG_IPPV_SECTOR_INFORMATION_QUERY      = 33,
	MSG_IPPV_SECTOR_INFORMATION_REPLY      = 34,
	MSG_IPPV_SET_DEBIT_THRESHOLD           = 35,
	MSG_IPPV_CALLBACK_REQUEST              = 36,
	MSG_IPPV_EVENT_LIST_QUERY              = 37,
	MSG_IPPV_EVENT_LIST_REPLY              = 38,
	MSG_IPPV_EVENT_CHECK_QUERY             = 39,
	MSG_IPPV_EVENT_CHECK_REPLY             = 40,

	/* Smartcard messages - PIN Management. */
	MSG_PIN_CODE_QUERY                     = 41,
	MSG_PIN_CODE_REPLY                     = 42,

	/* Smartcard messages - Sector Information. */
	MSG_NUMBER_OF_SECTORS_NOTIFY           = 43,

	/* Smartcard messages - Remote channel to card. */
	MSG_REMOTE_MESSAGE_QUERY               = 44,
	MSG_REMOTE_MESSAGE_REPLY               = 45,

	/* Smartcard messages - Get CAM user data. */
	MSG_SMARTCARD_USER_DATA_QUERY          = 46,
	MSG_SMARTCARD_USER_DATA_REPLY          = 47,

	/* Reserved message, for future use. */
	MSG_RFU1                               = 48,

	/* Registration service info. */
	MSG_REGISTRATION_USE_NEW_HEADEND       = 49,

	/* Smartcard RSA crypto. */
	MSG_SMARTCARD_RSA_SIGN_REQUEST         = 50,
	MSG_SMARTCARD_RSA_SIGN_REPLY           = 51,
	MSG_SMARTCARD_RSA_VERIFY_REQUEST       = 52,
	MSG_SMARTCARD_RSA_VERIFY_REPLY         = 53,
	MSG_SMARTCARD_RSA_ENCRYPT_REQUEST      = 54,
	MSG_SMARTCARD_RSA_ENCRYPT_REPLY        = 55,
	MSG_SMARTCARD_RSA_DECRYPT_REQUEST      = 56,
	MSG_SMARTCARD_RSA_DECRYPT_REPLY        = 57,

	/* Product interface messages. */
	MSG_PRODUCT_CHECK_QUERY                = 58,
	MSG_PRODUCT_CHECK_REPLY                = 59,
	MSG_ANY_PRODUCT_QUERY                  = 60,
	MSG_ANY_PRODUCT_REPLY                  = 61,
	MSG_PRODUCT_LIST_QUERY                 = 62,
	MSG_PRODUCT_LIST_REPLY                 = 63,

	/* ClientId messages. DEFUNCT! DO NOT USE! */
	MSG_CLIENT_ID_QUERY                    = 64,
	MSG_CLIENT_ID_REPLY                    = 65,

	/* CypherCast Service Info messages. */
	MSG_CC_SERVICE_ADD                     = 66,
	MSG_CC_SERVICE_DELETE                  = 67,
	MSG_CC_SERVICE_UPDATE                  = 68,

	/* Product interface messages (extension). */
	MSG_EXTENDED_PRODUCT_LIST_QUERY        = 69,
	MSG_EXTENDED_PRODUCT_LIST_REPLY        = 70,

	/* Smartcard messages - Pay Per Time. */
	MSG_PPT_INFO_QUERY                     = 71,
	MSG_PPT_INFO_REPLY                     = 72,

	/* Smartcard messages - Surf Locking. */
	MSG_SURF_LOCK_QUERY                    = 73,
	MSG_SURF_LOCK_REPLY                    = 74,

	/* Smartcard messages - Get Region. */
	MSG_REGION_QUERY                       = 75,
	MSG_REGION_REPLY                       = 76,

	/* Product interface messages (expiration). */
	MSG_EXPIRED_PRODUCT_QUERY              = 77,
	MSG_EXPIRED_PRODUCT_REPLY              = 78,

	/* IPDC Monitoring */
	MSG_IPDC_EMM_MONITOR                   = 79,
	MSG_IPDC_ECM_MONITOR                   = 80,

	/* Registration Parameter messages. */
	MSG_REGISTRATION_PARAMETER_QUERY       = 81,
	MSG_REGISTRATION_PARAMETER_REPLY       = 82,

	/* SoftClient Download proxy. DEFUNCT! DO NOT USE! */
	MSG_SC_DOWNLOAD_PROXY_QUERY            = 83,
	MSG_SC_DOWNLOAD_PROXY_REPLY            = 84,

	/* SoftClient Security proxy. DEFUNCT! DO NOT USE!*/
	MSG_SC_SECURITY_PROXY_QUERY            = 85,
	MSG_SC_SECURITY_PROXY_REPLY            = 86,

	/* SoftClient card-ip proxy. DEFUNCT! DO NOT USE!*/
	MSG_SC_CARDIP_PROXY_QUERY              = 87,
	MSG_SC_CARDIP_PROXY_REPLY              = 88,

	MSG_SC_MONITOR_PROXY_QUERY             = 89,
	MSG_SC_MONITOR_PROXY_REPLY             = 90,

	/* IPDC CBMS ESG Purchase Data messages. */
	MSG_EVALUATE_PURCHASE_DATA_QUERY       = 91,
	MSG_EVALUATE_PURCHASE_DATA_REPLY       = 92,
	MSG_ENTITLEMENT_UPDATE_TRIGGER         = 93,

	MSG_DRM_SYSTEM_QUERY                   = 94,
	MSG_DRM_SYSTEM_REPLY                   = 95,

	MSG_SMARTCARD_UNIQUE_ADDRESS_QUERY     = 96,
	MSG_SMARTCARD_UNIQUE_ADDRESS_REPLY     = 97,
	MSG_SMARTCARD_VENDOR_ID_QUERY          = 98,
	MSG_SMARTCARD_VENDOR_ID_REPLY          = 99,

	MSG_SMARTCARD_SECTOR_ADDRESS_QUERY     = 100,
	MSG_SMARTCARD_SECTOR_ADDRESS_REPLY     = 101,
	MSG_SMARTCARD_SECTOR_DATECODE_QUERY    = 102,
	MSG_SMARTCARD_SECTOR_DATECODE_REPLY    = 103,

	/* OOB emm. */
	MSG_OOB_EMM_DOWNLOAD                   = 104,
	MSG_OOB_EMM_RESPONSE                   = 105,

	/* Smartcard messages - Operator Id. */
	MSG_SMARTCARD_SECTOR_OPERATORID_QUERY  = 106,
	MSG_SMARTCARD_SECTOR_OPERATORID_REPLY  = 107,

	/* Smartcard messages - Marriage status. */
	MSG_SMARTCARD_MARRIAGE_STATUS_QUERY    = 108,
	MSG_SMARTCARD_MARRIAGE_STATUS_REPLY    = 109,

	/* DMB Monitoring. */
	MSG_DMB_EMM_MONITOR                    = 110,
	MSG_DMB_ECM_MONITOR                    = 111,

	/* Service control messages (extension). */
    MSG_SERVICE_CLOSE_REPLY				   = 112,

	/* Smartcard messages - Multiple smartcard state notification. */
	MSG_SMARTCARD_MULTICARD_STATE_NOTIFY   = 120,

	/* LPPV messages */
	MSG_LPPV_AWARE_QUERY                   = 121,
	MSG_LPPV_AWARE_REPLY                   = 122,
	MSG_LPPV_EVENT_NOTIFY                  = 123,
	MSG_LPPV_READ_TICKET_QUERY             = 124,
	MSG_LPPV_READ_TICKET_REPLY             = 125,
	MSG_LPPV_DELETE_TICKET_QUERY           = 126,
	MSG_LPPV_DELETE_TICKET_REPLY           = 127,
	MSG_LPPV_READ_CREDIT_QUERY             = 128,
	MSG_LPPV_READ_CREDIT_REPLY             = 129,
	MSG_LPPV_READ_ORDER_QUERY              = 130,
	MSG_LPPV_READ_ORDER_REPLY              = 131,
	MSG_LPPV_PLACE_ORDER_QUERY             = 132,
	MSG_LPPV_PLACE_ORDER_REPLY             = 133,
	MSG_LPPV_CANCEL_ORDER_QUERY            = 134,
	MSG_LPPV_CANCEL_ORDER_REPLY            = 135,

	MSG_EMM_TLV_NOTIFY                     = 136,

	/* Maturity Rating. */
	MSG_MATURITY_RATING_CAPABILITY_QUERY   = 137,
	MSG_MATURITY_RATING_CAPABILITY_REPLY   = 138,
	MSG_MATURITY_RATING_CONFIG_QUERY       = 139,
	MSG_MATURITY_RATING_CONFIG_REPLY       = 140,
	MSG_MATURITY_RATING_USER_PROFILE_QUERY = 141,
	MSG_MATURITY_RATING_USER_PROFILE_REPLY = 142,

	/* Usage Monitoring. */
	MSG_UMC_CONFIG                         = 143,
	MSG_UMC_TRIGGER                        = 144,

	MSG_REGION_FILTER_CONFIG               = 145,
	MSG_REGION_FILTER_SET_INFO             = 146,

	MSG_HGPC_SMARTCARD_DATA_QUERY          = 147,
	MSG_HGPC_SMARTCARD_DATA_REPLY          = 148,
	MSG_HGPC_SCELL_DATA_QUERY              = 149,
	MSG_HGPC_SCELL_DATA_REPLY              = 150,
	MSG_HGPC_MESSAGE_STATUS                = 151,
	MSG_HGPC_EVENT_NOTIFY                  = 152,

	/* CMMB Monitoring */
	MSG_CMMB_EMM_MONITOR                   = 153,
	MSG_CMMB_ECM_MONITOR                   = 154,

	MSG_IPPV_PHONE_NUMBER_QUERY            = 155,
	MSG_IPPV_PHONE_NUMBER_REPLY            = 156,
	
	MSG_LPPV_PURCHASE_DATA_NOTIFY          = 157,
	MSG_LPPV_OFFLINE_RECHARGE_CODE_QUERY   = 158,
	MSG_LPPV_OFFLINE_RECHARGE_CODE_REPLY   = 159,

	/* FLO Monitoring */
	MSG_FLO_EMM_MONITOR                    = 160,
	MSG_FLO_ECM_MONITOR                    = 161
} ca_drv_message_type_e;


/* Error values for CA_DRV_FatalError. */
enum
{
	IA_ERROR_MEMORY = 1,
	IA_ERROR_TASK,
	IA_ERROR_RESOURCE,
	IA_ERROR_MESSAGE,
	IA_ERROR_ASSERT,
	IA_ERROR_FATAL,
	IA_ERROR_RULE
};


/* General messages */
typedef struct
{
	ia_word16  wVersionCount;
} msg_version_reply_data_st;


typedef struct
{
	ia_byte    bMajor;
	ia_byte    bMinor;
	ia_word32  wSubVersion;
	ia_word16  wNameLength;
	ia_word16  wBuildInfoLength;
} msg_version_data_st;


typedef struct
{
	ca_drv_message_type_e  eOpcode;
	ia_word16              wOpSeqNo;
	ia_status_st           stStatus;
} msg_error_data_st;


typedef struct
{
	ia_word16                 wSystemId;
	ia_word16                 wKeyVersion;
	ia_word16                 wSignatureVersion;
	ia_word16                 wVariant;
	ia_word16                 wManufacturerId;
	ia_word16                 wHardwareVersion;
	ia_word16                 wLoadVersion;
	vd_loader_secure_chip_st  stSecureChipInfo;
} msg_registration_parameter_reply_data_st;


/* Function definitions exported by SoftCell. */
S3_EXPORT
ia_result SCELL_Initialise( void );
S3_EXPORT
void SCELL_Close( void );
S3_EXPORT
ia_result SCELL_Message( ca_drv_message_type_e eOpcode, ia_word16 wOpSeqNo, ia_word16 wLength,
	void *pvMessage );


#ifndef IRDETO_TFA1
/* Function definitions exported by manufacturer CA Task Driver. */
ia_result CA_DRV_Message( ca_drv_message_type_e eOpcode, ia_word16 wOpSeqNo, ia_word16 wLength,
	void *pvMessage );
void CA_DRV_FatalError( ia_word16 wError );
#endif

#endif /* S3_CADRV_H. */
