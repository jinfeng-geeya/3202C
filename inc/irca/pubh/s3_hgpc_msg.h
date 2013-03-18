/**
 * $Id: s3_hgpc_msg.h,v 1.3 2007/12/04 09:32:17 axu Exp $
 *
 * Copyright 2007 Irdeto Access B.V.
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

#ifndef S3_HGPC_MSG_H
#define S3_HGPC_MSG_H

typedef enum
{
	HGPC_CARD_OK = 1,
	HGPC_CARD_ERROR ,
	HGPC_CARD_NOT_SUPPORT
} msg_hgpc_card_status_e;

typedef enum
{
	HGPC_METHOD_INACTIVE = 0,
	HGPC_METHOD_HNA_HNA = 1,
	HGPC_METHOD_HNR_HNR,
	HGPC_METHOD_HNR_HNA
} msg_hgpc_method_e;

typedef enum
{
	HGPC_MSG_TYPE_HNA = 1,
	HGPC_MSG_TYPE_HNR
} msg_hgpc_message_type_e;

typedef enum
{
	HGPC_MSG_ACTION_CREATION = 1,
	HGPC_MSG_ACTION_WRITE_TO_CARD,
	HGPC_MSG_ACTION_SEND_OVER_NET,
	HGPC_MSG_ACTION_RECEIVE_OVER_NET,
	HGPC_MSG_ACTION_FIND_IN_STORE
} msg_hgpc_message_action_e;

typedef enum
{
	HGPC_MSG_STATUS_OK = 1,
	HGPC_MSG_STATUS_CARD_ERROR,
	HGPC_MSG_STATUS_CARD_UNSUPPORT,
	HGPC_MSG_STATUS_CARD_INACTIVE,
	HGPC_MSG_STATUS_CARD_OP_INVALID,
	HGPC_MSG_STATUS_AUTH_FAILED,
	HGPC_MSG_STATUS_ADDR_MISMATCH,
	HGPC_MSG_STATUS_TIMESTAMP_STALE,
	HGPC_MSG_STATUS_STORE_EMPTY,
	HGPC_MSG_STATUS_STORE_NO_MATCH
} msg_hgpc_message_status_e;


typedef enum
{
	HGPC_ROLE_INACTIVE = 1,
	HGPC_ROLE_PRIMARY,
	HGPC_ROLE_SECONDARY
} msg_hgpc_role_e	;


typedef struct 
{
	ia_byte			abUniqueAddr[ 3 ]; //pyh:这是卡地址，无需理会
	ia_byte			bSerialNoLength;
	ia_byte			bCheckDigit;
} msg_hgpc_smartcard_id_st;

typedef struct 
{
	msg_hgpc_method_e	eMethod;
	ia_word16			wHNARefreshTimeout;
	ia_word16			WHNARepeatInterval;
	ia_byte				bSecCardCount;
} msg_hgpc_card_data_primary_st;


typedef struct 
{
	msg_hgpc_method_e				eMethod;
	ia_word16						wHNRTimeLeft;
	ia_word16						wActivationCode;
	msg_hgpc_smartcard_id_st	stPrimCardId;
} msg_hgpc_card_data_secondary_st;


typedef struct 
{
	vd_rsc_id			wResourceId;
} msg_hgpc_card_data_query_st;

typedef struct 
{
	vd_rsc_id					wResourceId;
	msg_hgpc_card_status_e	eCardStatus;
	msg_hgpc_role_e			eCardRole;
} msg_hgpc_card_data_reply_st;

typedef struct 
{
	msg_hgpc_role_e    eScellRole;
} msg_hgpc_scell_data_reply_st;


typedef struct
{
	msg_hgpc_smartcard_id_st	stPrimCardId;
} msg_hgpc_scell_data_st;


typedef struct 
{
	vd_rsc_id						wResourceId;
	msg_hgpc_message_type_e		eType;
	msg_hgpc_message_action_e	eAction;
	msg_hgpc_message_status_e	eStatus;
} msg_hgpc_message_status_st;


typedef enum
{
	HGPC_EVENT_ACTIVATION,
	HGPC_EVENT_FORCE_RENEW,
	HGPC_EVENT_HNR_REQUIRED,
	HGPC_EVENT_HNA_REQUIRED,
	HGPC_EVENT_HNA_FALLBACK_TO_HNR
} msg_hgpc_event_type_e;

typedef struct 
{
	vd_rsc_id					wResourceId;
	msg_hgpc_event_type_e	eEventType;
} msg_hgpc_event_notify_st;

#endif /* S3_HGPC_MSG_H. */
