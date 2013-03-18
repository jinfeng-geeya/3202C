/**
 * $Id: s3_service.h,v 1.68 2007/10/23 11:07:20 hqian Exp $
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

#ifndef S3_SERVICE_H
#define S3_SERVICE_H

#include "s3_cadrv.h"
#include "s3_vdip.h"
#include "s3_vd_stream_crypto.h"

#define IA_INVALID_DVB_PID          0xFFFF
#define IA_INVALID_DMB_SUBCHID      0xFF
#define IA_INVALID_DMB_PA           0xFFFF
#define IA_UNUSED_DMB_PA            0x0000		/* Set when only Sub-channel CA is used. */
#define IA_INVALID_CMMB_SERVICE_ID  0xFFFF
#define IA_INVALID_CMMB_DATA_TYPE   0xFF


/* Service types. These values may *never* change! Always add to the end of the list! */
enum
{
	/* DVB services. */
	SERVICE_DVB_DESCRAMBLE        = 1,
	SERVICE_DVB_EMM               = 2,
	/* IPTV services. */
	SERVICE_IPTV_REGISTRATION_EMM = 3,
	SERVICE_IPTV_DESCRAMBLE_VOD   = 4,
	/* CypherCast services. */
	SERVICE_CC_DESCRAMBLER        = 5,
	SERVICE_CC_OPERATOR           = 6,
	/* PVR services. */
	SERVICE_PVR_RECORD            = 7,
	SERVICE_PVR_PLAYBACK          = 8,
	SERVICE_PVR_DRM               = 9,
	/* IPDC services. */
	SERVICE_IPDC_DESCRAMBLE       = 10,
	SERVICE_IPDC_EMM              = 11,
	/* DMB services. */
	SERVICE_DMB_DESCRAMBLE        = 12,
	SERVICE_DMB_EMM               = 13,
	/* CMMB services. */
	SERVICE_CMMB_DESCRAMBLE       = 14,
	SERVICE_CMMB_EMM              = 15
};


/* Resource types. These values may *never* change! Always add to the end of the list!*/
enum
{
	RESOURCE_ECM_SOURCE             = 1,
	RESOURCE_EMM_SOURCE             = 2,
	RESOURCE_DVB_DESCRAMBLER        = 3,
	RESOURCE_SMARTCARD              = 4,
	RESOURCE_IPTV_STB_SOCKET        = 5,
	RESOURCE_IPTV_HEADEND_SOCKET    = 6,
	RESOURCE_IPTV_MROUTER_ADDRESS   = 7,
	RESOURCE_CC_SOURCE              = 8,
	RESOURCE_MONITORING             = 9,
	RESOURCE_CC_DECRYPTOR           = 10,
	RESOURCE_REGISTRATION_HANDLE    = 11,
	RESOURCE_SESSION_MANAGEMENT     = 12,
	RESOURCE_CRYPTO_OPERATION       = 13,
	RESOURCE_MACROVISION            = 14,
	RESOURCE_COPY_CONTROL           = RESOURCE_MACROVISION,
	RESOURCE_STB_MAC_ADDRESS        = 15,
	RESOURCE_DVB_OOB                = 16,
	RESOURCE_SESSION_MANAGEMENT_DRM = 17,
	RESOURCE_IPDC_ECM               = 18,
	RESOURCE_IPDC_EMM               = 19,
	RESOURCE_IPDC_DESCRAMBLER       = 20,
	RESOURCE_DMB_ECM                = 21,
	RESOURCE_DMB_EMM                = 22,
	RESOURCE_DMB_DESCRAMBLER        = 23,
	RESOURCE_DRM_EVENT              = 24,
	RESOURCE_MATURITY_RATING        = 25,
	RESOURCE_CMMB_ECM               = 26,
	RESOURCE_CMMB_EMM               = 27,
	RESOURCE_CMMB_DESCRAMBLER       = 28,
	RESOURCE_CMMB_SDP_ECM           = 29,	
	RESOURCE_GENERAL_CONFIGURATION	= 30
};


/* Resource management values. */
enum
{
	MANAGE_CLOSE = 1,
	MANAGE_ADD,
	MANAGE_UPDATE
};


/* Message structures. */
typedef struct
{
	ia_word16            wServiceType;
} msg_service_open_request_data_st;

typedef struct
{
	ia_word16            wServiceType;
	ia_word16            wServiceHandle;
} msg_service_open_reply_data_st;

typedef struct
{
	ia_word16            wServiceHandle;
} msg_service_close_data_st;

typedef struct
{
	ia_word16            wServiceHandle;
} msg_service_status_request_data_st;

typedef struct
{
	ia_word16            wServiceHandle;
	ia_byte              bResourceCount;
} msg_service_define_data_st;

typedef struct
{
	ia_word16            wResourceType;
	vd_rsc_id            wResourceId;
	ia_word16            wResourceMgnt;
} msg_resource_st;

typedef struct
{
	ia_byte              bGlobalCount;
	ia_byte              bStreamCount;
} msg_ecm_source_st;

typedef struct
{
	ia_byte              bTag;
	ia_byte              bDataLength;
	/* Structure is followed by ia_byte bData[ bDataLength ]. */
} msg_descriptor_st;

typedef struct
{
	ia_word16            wPid;
	ia_byte              bDescrCount;
	/* Structure is followed by msg_descriptor_st MPEG_Descr[ bDescrCount ]. */
} msg_dvb_stream_st;

typedef struct
{
	ia_byte              bDescrCount;
	/* Structure is followed by msg_descriptor_st MPEG_Descr[ bDescrCount ]. */
} msg_emm_source_st;

typedef struct
{
	ia_word16            wServiceHandle;
	ia_status_st         stStatus;
	ia_status_st         stServiceStatus;
	ia_byte              bResourceCount;
} msg_service_status_data_st;

typedef struct
{
	ia_word16            wType;
	vd_rsc_id            wId;
	ia_status_st         stStatus;
} msg_rsc_status_st;

typedef struct
{
	ia_byte              bStreamCount;
} msg_ecm_source_status_st;

typedef struct
{
	ia_word16            wEmmPid;
} msg_emm_source_status_st;

typedef struct
{
	ia_word16            wEsPid;
	ia_word16            wEcmPid;
	ia_status_st         stStatus;
} msg_stream_status_st;

typedef struct
{
	ia_bool              fSocketIsValid;
} msg_iptv_socket_status_st;


/* IPDC Services structures. */
typedef struct
{
	ia_word16            wSdpLength;
} msg_ipdc_sdp_st;

typedef struct
{
	ia_byte              bStreamCount;
} msg_ipdc_descrambler_status_st;

typedef struct
{
	ia_bool              fHasMid;
	ia_byte              bMid;
	ia_bool              fHasKILength;
	ia_byte              bKILength;
	ia_bool              fHasKmsId;
	ia_word32            wKmsId;
	ia_word32            wIPDCStreamId;
	ia_byte              abSalt[ 8 ];
	vd_ip_domain_type_e  eDomain;
	ia_bool              fHasEcmStream;
	ia_status_st         stStatus;
} msg_ipdc_content_stream_status_st;
	/* Structure is followed by Content_IP_Socket_Address, its type depending on eDomain.
		Structure is then followed by ECM_IP_Socket_Address, its type depending on eDomain and only
		if fHasEcmStream is IA_TRUE. */

typedef struct
{
	ia_byte              bStreamCount;
} msg_ipdc_emm_status_st;

typedef struct
{
	ia_word32            wKmsId;
	vd_ip_domain_type_e  eDomain;
	ia_status_st         stStatus;
} msg_ipdc_kmm_stream_status_st;


/* DMB Services structures. */
typedef struct
{
	ia_byte              bMappingTableSubChId;
	ia_byte              bStreamCount;
} msg_dmb_ecm_st;

typedef struct
{
	ia_byte              bContentSubChId;
	ia_word16            wContentPA;
} msg_dmb_content_streamid_st;

typedef struct
{
	ia_byte              bStreamCount;
} msg_dmb_ecm_status_st;

typedef struct
{
	ia_byte              bContentSubChId;
	ia_word16            wContentPA;
	ia_byte              bEcmSubChId;
	ia_word16            wEcmPA;
	ia_status_st         stStatus;
} msg_dmb_stream_status_st;

typedef struct
{
	ia_byte              bMappingTableSubChId;
} msg_dmb_emm_st;

typedef struct
{
	ia_byte              bEmmSubChId;
	ia_word16            wEmmPA;
} msg_dmb_emm_streamid_st;


/* CMMB Services structures. */
typedef struct
{
	ia_word16            wEadtLength;
} msg_cmmb_eadt_st;


typedef struct
{
	ia_byte              bStreamCount;
} msg_cmmb_descrambler_status_st;

typedef struct
{
	ia_bool              fHasMid;
	ia_byte              bMid;
	ia_bool              fHasKILength;
	ia_byte              bKILength;
	ia_bool              fHasKmsId;
	ia_word32            wKmsId;
	ia_word32            wCMMBStreamId;
	ia_byte              abSalt[ 8 ];
	vd_ip_domain_type_e  eDomain;
	ia_bool              fHasEcmStream;
	ia_status_st         stStatus;
} msg_cmmb_content_stream_status_st;
	/* Structure is followed by Content_IP_Socket_Address, its type depending on eDomain.
		Structure is then followed by ECM_IP_Socket_Address, its type depending on eDomain and only
		if fHasEcmStream is IA_TRUE. */

typedef struct
{
	ia_byte              bStreamCount;
} msg_cmmb_status_st ;

typedef struct
{
	ia_word16            wCaSystemId;
	ia_word16            wServiceId;
	ia_byte              bDataType;
	ia_status_st         stStatus;
} msg_cmmb_stream_status_st;


/* Monitoring structures. */
typedef struct
{
	ia_word16            wNumberOfMonitors;
} msg_monitoring_st;

typedef struct
{
	ia_word16            wServiceHandle;
	vd_rsc_id            wResourceId;
	ia_byte              bScStatus1;
	ia_byte              bScStatus2;
	ia_word16            wEmmPid;
	ia_byte              bAddressControl;
	ia_byte              abEmmPayload[ 3 ];
	ia_word32            wEmmCount;
} msg_dvb_emm_monitor_data_st;

typedef struct
{
	ia_word16            wServiceHandle;
	vd_rsc_id            wResourceId;
	ia_byte              bScStatus1;
	ia_byte              bScStatus2;
	ia_word16            wEcmPid;
	ia_byte              bPage;
	ia_byte              bVersion;
	ia_byte              bAlgorithm;
	ia_byte              bOddEven;
	ia_bool              fNextKey;
	ia_byte              abScReply[ 6 ];
	ia_word32            wEcmCount;
} msg_dvb_ecm_monitor_data_st;

typedef struct
{
	ia_word16            wServiceHandle;
	vd_rsc_id            wResourceId;
	ia_byte              bScStatus1;
	ia_byte              bScStatus2;
	ia_word16            wEmmPid;
	ia_byte              bAddressControl;
	ia_byte              abEmmPayload[ 3 ];
	ia_word32            wEmmCount;
	ia_word32            wAckId;
} msg_iptv_emm_monitor_data_st;

typedef struct
{
	ia_word16            wServiceHandle;
	ia_word32            wAttemptCount;
} msg_iptv_attempts_monitor_data_st;

typedef struct
{
	ia_word16            wServiceHandle;
	vd_rsc_id            wResourceId;
	ia_byte              bScStatus1;
	ia_byte              bScStatus2;
	ia_byte              bAddressControl;
	ia_byte              abEmmPayload[ 3 ];
	ia_word32            wEmmCount;
	vd_ip_domain_type_e  eDomain;
} msg_ipdc_emm_monitor_data_st;

typedef struct
{
	ia_word16            wServiceHandle;
	vd_rsc_id            wResourceId;
	ia_byte              bScStatus1;
	ia_byte              bScStatus2;
	ia_byte              bPage;
	ia_byte              bVersion;
	ia_byte              bAlgorithm;
	ia_byte              bOddEven;
	ia_bool              fNextKey;
	ia_byte              abScReply[ 6 ];
	ia_word32            wEcmCount;
	vd_ip_domain_type_e  eDomain;
} msg_ipdc_ecm_monitor_data_st;

typedef struct
{
	ia_word16            wServiceHandle;
	vd_rsc_id            wResourceId;
	ia_byte              bScStatus1;
	ia_byte              bScStatus2;
	ia_byte              bAddressControl;
	ia_byte              abEmmPayload[ 3 ];
	ia_word32            wEmmCount;
	ia_byte              bEmmSubChId;
	ia_word16            wEmmPA;
} msg_dmb_emm_monitor_data_st;

typedef struct
{
	ia_word16            wServiceHandle;
	vd_rsc_id            wResourceId;
	ia_byte              bScStatus1;
	ia_byte              bScStatus2;
	ia_byte              bPage;
	ia_byte              bVersion;
	ia_byte              bAlgorithm;
	ia_byte              bOddEven;
	ia_bool              fNextKey;
	ia_byte              abScReply[ 6 ];
	ia_word32            wEcmCount;
	ia_byte              bEcmSubChId;
	ia_word16            wEcmPA;
} msg_dmb_ecm_monitor_data_st;

typedef struct
{
	ia_word16            wServiceHandle;
	vd_rsc_id            wResourceId;
	ia_byte              bScStatus1;
	ia_byte              bScStatus2;
	ia_byte              bAddressControl;
	ia_byte              abEmmPayload[ 3 ];
	ia_word32            wEmmCount;
} msg_emm_monitor_data_st;
/* Structure is directly followed by transport specific structure depending on message type. */
/* E.g. for MSG_CMMB_EMM_MONITOR the msg_cmmb_streamid_st structure is used. */

typedef struct
{
	ia_word16            wServiceId;
	ia_byte              bDataType;
} msg_cmmb_streamid_st;

typedef struct
{
	ia_word16            wServiceHandle;
	vd_rsc_id            wResourceId;
	ia_byte              bScStatus1;
	ia_byte              bScStatus2;
	ia_byte              bPage;
	ia_byte              bVersion;
	ia_byte              bAlgorithm;
	ia_byte              bOddEven;
	ia_bool              fNextKey;
	ia_byte              abScReply[ 6 ];
	ia_word32            wEcmCount;
} msg_ecm_monitor_data_st;
/* Structure is directly followed by transport specific structure depending on message type. */
/* E.g. for MSG_CMMB_ECM_MONITOR the msg_cmmb_streamid_st structure is used. */

typedef struct
{
	vd_ip_domain_type_e  eDomain;
} msg_socket_st;
/* Structure is followed by a socket in struct type defined by eDomain. */

typedef struct
{
	ia_word16            wServiceId;	/* Used only in cc descrambler svce. */
	ia_bool              fIsSrcAddr;	/* Used only in cc descrambler svce. */
	vd_ip_domain_type_e  eDomain;
} msg_cc_source_st;
/* Structure is followed by a socket struct type defined by eDomain. */

typedef struct
{
	ia_bool              wAddrValid;
} msg_cc_source_status_st;

typedef struct
{
	ia_word16            wServiceHandle;
} msg_registration_handle_st;

typedef struct
{
	vd_crypto_algorithm_e   eAlgorithm;
} msg_crypto_operation_source_st;

typedef struct
{
	ia_status_st         stStatus;
} msg_session_source_substatus_st;

typedef struct
{
	ia_byte              abMacAddress[ 6 ];
} msg_mac_address_st;

typedef struct
{
	ia_word16            wTypeMask;
	ia_word16            wLength;
} msg_copycontrol_substatus_st;

enum
{
	MSG_COPYCONTROL_MACROVISION = 0x01,
	MSG_COPYCONTROL_DIGITAL     = 0x02,
	MSG_COPYCONTROL_ANALOG      = 0x04
};


typedef struct
{
	ia_byte              bEventCount;
} msg_drm_event_list_st;

typedef struct
{
	ia_byte              *pbEventId;
	ia_word16            wEsgContentIdLength;
	ia_word16            wBaseDrmRIUrlLength;
} msg_drm_event_st;
/* Structure is followed by ia_byte abEsgContentId[ wEsgContentIdLength ]. */
/* Further followed by ia_byte abBaseDrmRIUrl[ wBaseDrmRIUrlLength ]. */

typedef struct
{
	ia_byte              *pbEventId;
} msg_drm_event_status_st;

typedef struct
{
	vd_rsc_id            wResourceId;
	ia_byte              bLength;
} msg_emm_tlv_notify_st;


typedef struct
{
	ia_word16            wServiceId;
	ia_word16            wSdpLength;
} msg_cmmb_sdp_ecm_st;
/* Structure is followed by ia_byte abSdpData[ wSdpLength ]. */

#endif /* S3_SERVICE_H. */
