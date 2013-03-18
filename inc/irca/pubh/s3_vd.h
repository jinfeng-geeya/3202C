/*
 * $Id: s3_vd.h,v 1.35 2008/04/17 12:37:02 ehietbrink Exp $
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
 */

/*
 * This file is the generic header for the 'virtual driver API' version 1.09.03 and newer.
 */

#ifndef S3_VD_H
#define S3_VD_H

#include "s3_portable_types.h"
#include "s3_status.h"

/* Typedefs. */
typedef ia_word32	vd_ref_id;
typedef ia_word32	vd_rsc_id;

/* Definitions for resource IDs */
#define VD_RSC_ID_INVALID ((vd_rsc_id)0xFFFFFFFF)

/* Definitions for reserved reference IDs. */
#define VD_REF_ID_MAX ((vd_ref_id)0xFFFEFFFF)	/* For return on VD_Open(). */
#define VD_REF_ID_ERROR ((vd_ref_id)0xFFFFFFFE)	/* For return on VD_Open(). */
#define VD_REF_ID_GLOBAL ((vd_ref_id)( VD_REF_ID_MAX + 1 )) /* For global IO operations. */

/* For return on VD_GetDriverVersion(). */
#define VD_NOT_IMPLEMENTED	0

/* 
 * Additional values to the ones defined if s3_status.h that can be returned on VD_GetResources(),
 * VD_Close(), and VD_IO(). 
 * Specific drivers may add additional status values. 
 */
enum
{
	VD_WRONG_REF_ID = IA_FIRST_USER_RESULT_VALUE,
	VD_HARDWARE_ERROR,
	VD_NOT_SUPPORTED,
	VD_OS_ERROR,
	VD_NO_ACCESS,
	VD_WRITE_INCOMPLETE,
	VD_READ_INCOMPLETE,
	VD_OUT_OF_MEMORY,
	VD_FIRST_DRIVER_RESULT_VALUE = IA_FIRST_USER_RESULT_VALUE + 0x0100
};

/* The driver types supported accross ALL SoftCell 3 client types. */
typedef enum
{
	VD_DRIVER_SMARTCARD = 1,
	VD_DRIVER_PERSISTENT_STORAGE,
	VD_DRIVER_DVB_FILTER,
	VD_DRIVER_DVB_DESCRAMBLER,
	VD_DRIVER_IP_SOCKET,
	VD_DRIVER_TIMER,
	VD_DRIVER_IP_DECRYPTION_UNIT,
	VD_DRIVER_LOADER,
	VD_DRIVER_MACROVISION,
	/* Macrovision virtual driver becomes a generic copy control virtual driver. */
	VD_DRIVER_COPYCONTROL = VD_DRIVER_MACROVISION,
	VD_DRIVER_STREAM_CRYPTO,
	VD_DRIVER_EXTERNAL_STORAGE,
	VD_DRIVER_DVB_OOB,
	VD_DRIVER_IPDC_DESCRAMBLER,
	VD_DRIVER_DMB_FILTER,
	VD_DRIVER_DMB_DESCRAMBLER,
	VD_DRIVER_DRM_SCRAMBLER,
	VD_DRIVER_HOMEGATEWAY_BROADCAST,
	VD_DRIVER_ISMACRYP_DESCRAMBLER,
	VD_DRIVER_FLO_DESCRAMBLER
} vd_driver_type_e;


/* Function definitions. */
#ifndef IRDETO_TFA1

/* Initialise the virtual driver. */
ia_result VD_Initialise( void );

/* Terminate the virtual driver. */
void VD_Terminate( void );

/* Return the driver version for that virtual driver. */
/* Version numbers start with 1. 0 (VD_NOT_IMPLEMENTED) Means driver not implemented. */
ia_word16 VD_GetDriverVersion( vd_driver_type_e eDriverType );

/* Return the resources available for that virtual driver. */
ia_result VD_GetResources( vd_driver_type_e eDriverType, void **ppvList, ia_word16 *pwCount );

/* Open a resource. */
vd_ref_id VD_Open( vd_driver_type_e eDriverType, vd_rsc_id wResourceID, void *pvData );

/* Close a resource. */
ia_result VD_Close( vd_driver_type_e eDriverType, vd_ref_id wRefId );

/* send control messages to a resource. */
ia_result VD_IO( vd_driver_type_e eDriverType, vd_ref_id wRefId, ia_word16 wOpCode,
	ia_word32 wDataWriteSize, void *pvDataWrite, ia_word32 *pwDataReadSize, void *pvDataRead );

#endif  /* IRDETO_TFA1 */

#endif  /* S3_VD_H */

