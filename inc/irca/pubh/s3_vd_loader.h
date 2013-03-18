/**
 * $Id: s3_vd_loader.h,v 1.4 2006/11/29 14:04:57 ehietbrink Exp $
 *
 * Copyright © 2004-2006 Irdeto Access B.V.
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
 * This file is part of the 'loader virtual driver'.
 **/

#ifndef S3_VD_LOADER_H
#define S3_VD_LOADER_H

#include "s3_portable_types.h"
#include "s3_vd.h"

/* VD_IO opcodes. */
enum
{
	VD_LOADER_GET_SYSTEM_ID = ( VD_DRIVER_LOADER * 10 ),
	VD_LOADER_GET_KEY_VERSION,
	VD_LOADER_GET_SIGNATURE_VERSION,
	VD_LOADER_GET_VARIANT,
	VD_LOADER_GET_MANUFACTURER_ID,
	VD_LOADER_GET_HARDWARE_VERSION,
	VD_LOADER_GET_LOAD_VERSION,
	VD_LOADER_GET_SECURE_CHIP_CSSN
};

/* Structure used with VD_LOADER_GET_SYSTEM_ID. */
typedef struct
{
	ia_word16	wSystemId;
} vd_loader_system_id_st;

/* Structure used with VD_LOADER_GET_KEY_VERSION. */
typedef struct
{
	ia_word16	wKeyVersion;
} vd_loader_key_version_st;

/* Structure used with VD_LOADER_GET_SIGNATURE_VERSION. */
typedef struct
{
	ia_word16	wSignatureVersion;
} vd_loader_signature_version_st;

/* Structure used with VD_LOADER_GET_VARIANT. */
typedef struct
{
	ia_word16	wVariant;
} vd_loader_variant_st;

/* Structure used with VD_LOADER_GET_MANUFACTURER_ID. */
typedef struct
{
	ia_word16	wManufacturerId;
} vd_loader_manufacturer_id_st;

/* Structure used with VD_LOADER_GET_HARDWARE_VERSION. */
typedef struct
{
	ia_word16	wHardwareVersion;
} vd_loader_hardware_version_st;

/* Structure used with VD_LOADER_GET_LOAD_VERSION. */
typedef struct
{
	ia_word16	wLoadVersion;
} vd_loader_load_version_st;

/* Structure used with VD_LOADER_GET_SECURE_CHIP_CSSN. */
typedef struct
{
	ia_word32	wCSSN;
	ia_byte		bCrypto;
	ia_word32	wRsaBoot;
	ia_word32	wJtag;
	ia_word32	wCwMode;
} vd_loader_secure_chip_st;

#endif /* S3_VD_LOADER_H */
