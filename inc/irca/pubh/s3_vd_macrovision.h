/*
 * $Id: s3_vd_macrovision.h,v 1.6 2007/12/12 09:43:59 hqian Exp $
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
 * This file is part of the 'Macrovision driver'.
 * It contains all definitions for the Macrovision virtual driver version 2 and replaces the 
 * header file s3_vdmv.h
 */


#ifndef S3_VD_MACROVISION_H
#define S3_VD_MACROVISION_H


/* Opcodes for VD_IO() call */
enum
{
	/* Set the Macrovision mode byte used as part of the Macrovision protection scheme. */
	VD_MACROVISION_SET_MODE,
	/* Set the (global) Macrovision configuration data. */
	VD_MACROVISION_SET_CONFIG
};


/* Data structure for VD_MACROVISION_SET_MODE. */
typedef struct
{
	ia_byte	bMode;	/* Only lower 6 bits are valid. */
} vd_macrovision_mode_st;


/* Data structure for VD_MACROVISION_SET_CONFIG. */
typedef struct
{
	ia_byte		bVersion;
	ia_word16	wConfigLength;
} vd_macrovision_config_st;
/* Structure is followed by ia_byte abConfig[ wConfigLength ]. */


#endif /* S3_VD_MACROVISION_H */
