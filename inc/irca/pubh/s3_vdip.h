/**
 * $Id: s3_vdip.h,v 1.11 2007/09/21 03:20:42 nchen Exp $
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
 * This file is used by all virtual drivers that require the use of IP addresses.
 * It contains all definitions for the IP address types.
 **/

#ifndef S3_VDIP_H
#define S3_VDIP_H

#include "s3_portable_types.h"


/* Address family (eDomain): VD_IP_FAM_IPV4. Note that all member fields are in BIG endian order! */
typedef struct
{
	ia_word16  wInPort;
	ia_word32  wInAddr;
} vd_ip_sockaddr_ipv4_st;


/* Address family (eDomain): VD_IP_FAM_IPV6. Note that all member fields are in BIG endian order! */
typedef struct
{
	ia_word16  wIn6Port;
	ia_byte    abIn6Addr[ 16 ];
	ia_word32  wIn6FlowInfo;
	ia_word32  wIn6ScopeId;
} vd_ip_sockaddr_ipv6_st;


typedef enum
{
	VD_IP_FAM_IPV4 = 1,
	VD_IP_FAM_IPV6,
	VD_CMMB_SERVICE_ID
} vd_ip_domain_type_e;


#endif /* S3_VDIP_H */
