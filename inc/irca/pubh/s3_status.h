/**
 * $Id: s3_status.h,v 1.52 2008/02/15 15:51:46 ehietbrink Exp $.
 *
 * Copyright 2003-2008 Irdeto Access B.V., All rights reserved.
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

#ifndef S3_STATUS_H
#define S3_STATUS_H

#include "s3_portable_types.h"


/*
 * Sources of the status. Their values may *never* change! Always add to the end of the list!
 */
typedef enum
{
	SOURCE_UNKNOWN                     = 1,  /* Use only for initialisation and error-checking. */
	SOURCE_SOFTCELL                    = 2,  /* Core components. */
	SOURCE_SERVICES                    = 3,  /* Core service and modules components. */
	SOURCE_SMARTCARD                   = 4,  /* Main component of the smartcard security device. */
	SOURCE_DVB_EMM_SERVICE             = 5,  /* DVB EMM service. */
	SOURCE_MOD_EMM_SOURCE              = 6,  /* EMM source (CAT) module. */
	SOURCE_MOD_DVB_EMM_FILTER          = 7,  /* DVB EMM filter module. */
	SOURCE_DVB_DESCRAMBLE_SERVICE      = 8,  /* DVB DESCRAMBLE service. */
	SOURCE_MOD_ECM_SOURCE              = 9,  /* ECM source (PMT) module. */
	SOURCE_MOD_DVB_DESCRAMBLER         = 10, /* DVB Descrambler module. */
	SOURCE_MOD_MONITORING              = 11, /* Monitoring module. */
	SOURCE_IPPV                        = 12, /* IPPV smartcard option. */
	SOURCE_SMARTCARD_MARRIAGE          = 13, /* Smartcard Marriage option. */
	SOURCE_IPTV_REGISTRATION_EMM       = 14, /* IPTV Registration and EMM service. */
	SOURCE_MOD_IPTV_STB_SOCKET         = 15, /* IPTV STB Socket module. */
	SOURCE_MOD_IPTV_HEADEND_SOCKET     = 16, /* IPTV HeadEnd Socket module. */
	SOURCE_MOD_IPTV_MROUTER_ADDRESS    = 17, /* IPTV MRouter Address module. */
	SOURCE_MOD_CC_EMM                  = 18, /* CC EMM Filter module in operator service. */
	SOURCE_MOD_CC_SOURCE               = 19, /* CC Source in CC services. */
	SOURCE_CC_OPERATOR_SERVICE         = 20, /* CC Operator service. */
	SOURCE_MOD_CC_ECM                  = 21, /* CC ECM Parser module in operator service. */
	SOURCE_CC_DESCRAMBLER_SERVICE      = 22, /* CC Descrambler service. */
	SOURCE_MOD_CC_DESCRAM              = 23, /* CC Descrambler module. */
	SOURCE_MOD_CC_DECRYPTOR            = 24, /* CC Decryptor source module. */
	SOURCE_MOD_REGISTRATION_HANDLE     = 25, /* IPTV Registration service handle. */
	SOURCE_IPTV_DESCRAMBLE_VOD_SERVICE = 26, /* IPTV Descramble VOD service. */
	SOURCE_PVR_RECORD_SERVICE          = 27, /* PVR Record service. */
	SOURCE_PVR_PLAYBACK_SERVICE        = 28, /* PVR Playback service. */
	SOURCE_MOD_SESSION_MANAGEMENT      = 29, /* Session Management resource module. */
	SOURCE_MOD_CRYPTO_OPERATION        = 30, /* Crypto Operation resource module. */
	SOURCE_MOD_MACROVISION             = 31, /* Macrovision resource module. */
	SOURCE_MOD_COPYCONTROL             = SOURCE_MOD_MACROVISION,
	SOURCE_APPLICATION                 = 32, /* To be used by the application to report non-SoftCell
                                               status. */
	SOURCE_MOD_STB_MAC_ADDRESS         = 33, /* STB MAC address module. */
	SOURCE_MOD_DVB_OOB                 = 34, /* DVB OOB resource module. */
	SOURCE_CUSTOM_CA                   = 35, /* CustomCA module */
	SOURCE_PVR_DRM_SERVICE             = 36, /* PVR DRM service. */
	SOURCE_MOD_SESSION_MANAGEMENT_DRM  = 37, /* DRM Session Management resource module. */
	SOURCE_MOD_IPDC_EMM                = 38, /* Status of the IPDC EMM resource module. */
	SOURCE_MOD_IPDC_ECM                = 39, /* Status of the IPDC ECM resource module. */
	SOURCE_RESOURCES                   = 40, /* Common status of resources. */
	SOURCE_SDP_PARSER                  = 41, /* Common status of SDP parser modules. */
	SOURCE_SVC_IPDC_DESCRAMBLE         = 42, /* IPDC Descramble service. */
	SOURCE_SVC_IPDC_EMM                = 43, /* IPDC EMM service. */
	SOURCE_MOD_DMB_MAPTBL              = 44, /* DMB Mapping Table parser module. */
	SOURCE_MOD_DMB_ECM                 = 45, /* DMB ECM service. */
	SOURCE_MOD_DMB_EMM                 = 46, /* DMB EMM service. */
	SOURCE_MOD_DRM_EVENT               = 47, /* DRM Event module. */
	SOURCE_LPPV                        = 48, /* LPPV module. */
	SOURCE_MOD_MATURITY_RATING         = 49, /* Maturity Rating resource module. */
	SOURCE_RSA_CRYPTO                  = 50, /* Used by the new S/C option RSA Crypto. */
	SOURCE_EADT_PARSER                 = 51, /* Common status of CMMB EATD parser modules. */
	SOURCE_MOD_CMMB_EMM                = 52, /* Status of the CMMB EMM resource module. */
	SOURCE_MOD_CMMB_ECM                = 53, /* Status of the CMMB ECM resource module. */
	SOURCE_SVC_CMMB_DESCRAMBLE         = 54, /* CMMB Descramble service. */
	SOURCE_SVC_CMMB_EMM                = 55, /* CMMB EMM service. */
	SOURCE_CMMB_SDP_PARSER             = 56, /* CMMB common SDP Parser. */
	SOURCE_MOD_CMMB_SDP_ECM            = 57, /* CMMB SDP ECM Parser. */
	SOURCE_MOD_GENERAL_CONFIGURATION   = 58, /* General Configuration*/
	SOURCE_MOD_FLO_EMM                 = 59, /* Status of the FLO EMM resource module. */
	SOURCE_MOD_FLO_ECM                 = 60, /* Status of the FLO ECM resource module. */
	SOURCE_SVC_FLO_DESCRAMBLE          = 61, /* FLO Descramble service. */
	SOURCE_SVC_FLO_EMM                 = 62  /* FLO EMM service. */
} ia_status_source_e;


/* The severity of the status. Statuses of severity STATUS_INFO and STATUS_INFO_DND should never
	result in a banner. */
typedef enum
{
	STATUS_FATAL = 1,
	STATUS_ERROR,
	STATUS_WARNING,
	STATUS_INFO,
	STATUS_INFO_DND
} ia_status_severity_e;


typedef struct
{
	ia_status_source_e    eSource;
	ia_word16             wStatus;
	ia_status_severity_e  eSeverity;
} ia_status_st;

#endif /* S3_STATUS_H. */

