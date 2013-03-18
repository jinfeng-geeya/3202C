/*
 * $Id: s3_vd_copycontrol.h,v 1.5 2007/12/12 09:43:59 hqian Exp $
 *
 * Copyright Irdeto Access B.V.
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
 * This file is part of the 'Copy Control driver'.
 * It contains all definitions for the copy control virtual driver version 3.
 */


#ifndef S3_VD_COPYCONTROL_H
#define S3_VD_COPYCONTROL_H

#include "s3_vd_macrovision.h"

/* Opcodes for VD_IO() call */
enum
{
	/* Set the Macrovision mode byte used as part of the Macrovision protection scheme. */
	VD_COPYCONTROL_MACROVISION_SET_MODE   = VD_MACROVISION_SET_MODE,
	/* Set the (global) Macrovision configuration data. */
	VD_COPYCONTROL_MACROVISION_SET_CONFIG = VD_MACROVISION_SET_CONFIG,
	VD_COPYCONTROL_DIGITAL_SET_CCI,
	VD_COPYCONTROL_ANALOG_SET_CCI
};

typedef enum 
{
	VD_COPYCONTROL_MACROVISION = 1,
	VD_COPYCONTROL_DIGITAL,
	VD_COPYCONTROL_ANALOG
} vd_copycontrol_type_e;

typedef	struct
{
	vd_copycontrol_type_e   eCopyControlType;
} vd_copycontrol_resource_st;

typedef struct
{
	vd_copycontrol_type_e   eCopyControlType;
} vd_copycontrol_open_st;

typedef enum 
{
	VD_COPYCONTROL_COPYFREE,
	VD_COPYCONTROL_COPYNONE,
	VD_COPYCONTROL_COPYONCE,
	VD_COPYCONTROL_COPYNEVER
} vd_copycontrol_cci_e;

typedef struct
{
	vd_copycontrol_cci_e    eCopyControlInfo;
} vd_copycontrol_digital_cci_st;

typedef struct
{
	vd_copycontrol_cci_e    eCopyControlInfo;
} vd_copycontrol_analog_cci_st;

#endif /* S3_VD_COPYCONTROL_H */
