/**
 * $Id: s3_vdsc.h,v 1.14 2008/07/18 09:03:03 ehietbrink Exp $
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
 * This file is part of the 'smartcard virtual driver'.
 * It contains all definitions for the smartcard virtual driver.
 **/

#ifndef S3_VDSC_H
#define S3_VDSC_H

#include "s3_portable_types.h"
#include "s3_vd.h"

typedef enum
{
	/* A card is inserted into the slot. */
	VD_SC_CARD_IN,
	/* A card is removed from the slot. */
	VD_SC_CARD_OUT,
	/* An error has occurred on insertion of the card. */
	VD_SC_CARD_IN_ERROR,
	/* A JavaCard / SIM smartcard has been inserted. */
	VD_SC_JAVACARD_CHANNEL_OPENED,
	/* The Irdeto JavaCard Applet was not found or could not be selected. */
	VD_SC_JAVACARD_APPLET_NOT_FOUND
} vd_sc_status_e;

typedef enum
{
	/* An error has happened on communications. */
	VD_SC_COMM_ERROR,
	/* Some hardware error occurred. */
	VD_SC_HARDWARE_ERROR,
	/* The card has been removed from the reader. */
	VD_SC_CARD_OUT_ERROR,
	/* The comuniations have timed-out. */
	VD_SC_TIME_OUT,
	/* The ATR received from the card is not up to the ISO7816-3 spec. */
	VD_SC_ATR_INVALID_FORMAT,
	/* The ATR received from the card is not an Irdeto Access ATR. */
	VD_SC_ATR_INVALID_HISTORY,
	/* JavaCard does not support Logical Channels. (ISO/IEC 7816-4:1995 §8.3.6) ==> TODO: remove.*/
	VD_SC_NO_LOGICAL_CHAN_SUPPORT
} vd_sc_error_e;

/* Smartcard driver VD_IO opcodes. */
enum
{
	/* Interface with the smartcard. */
	VD_SC_COMMUNICATE = ( VD_DRIVER_SMARTCARD * 10 ),
	/* Reset the smartcard. */
	VD_SC_RESET,
	/* Trigger Softclient event. */
	VD_SC_SOFTCLIENT_EVENT,
	/* Get form of the smartcard. */
	VD_SC_GET_CARD_FORM
};

/* The smartcard notify function. */
typedef void (* vd_sc_status_notify_fn)(vd_ref_id wRefId, ia_word32 wSoftCellVal,
	vd_sc_status_e eReason, void *pvInfo);

typedef struct
{
	vd_sc_status_notify_fn	pfnStatusNotify;
	ia_word32 				wSoftCellVal;
} vd_sc_open_st;

typedef struct
{
	/* Length of the ATR that follows this struct. */
	ia_word16	wAtrLength;
} vd_sc_atr_st;

typedef struct
{
	/* Length of the ATR that follows this struct. */
	ia_word16	wAtrLength;
	/* Logical Channel number. */
	ia_byte		bChannelNumber;
} vd_sc_atr_java_channel_st;

typedef struct
{
	vd_sc_error_e	eCardError;
} vd_sc_card_in_error_st;

typedef struct
{
	/* Define the length of the header that follows this struct. */
	ia_byte		bScHeaderLen;
	/* Define the length of the payload that follows the header. */
	ia_byte		bScPayloadLen;
} vd_sc_write_st;

typedef struct
{
	/* Size of the data to follow. */
	ia_word16	wSize;
} vd_sc_return_st;

typedef struct
{
	/* Last reported error. */
	vd_sc_error_e	eError;
} vd_sc_read_error_st;

typedef enum
{
	/* Indicate eResetCode is not used in VD_SC_RESET command. */
	VD_SC_RESETCODE_NOT_USED = 0,
	/* Probe other smartcard on mobile device. */
	VD_SC_JAVACARD_SWAP
} vd_sc_reset_code_e;

typedef enum
{
	VD_SC_CARD_FORM_NORMAL,
	VD_SC_CARD_FORM_SMD,
	VD_SC_CARD_FORM_SIM,
	VD_SC_CARD_FORM_SDCARD
} vd_sc_card_form_e;

typedef struct
{
	vd_sc_card_form_e eScForm;
} vd_sc_get_card_form_st;

#endif /* S3_VDSC_H */
