/**
 * $Id: s3_general_cfg.h,v 1.1 2007/10/31 07:19:15 hqian Exp $
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

#ifndef S3_GENERAL_CFG_H
#define S3_GENERAL_CFG_H

typedef struct
{
	ia_byte            bTag;
	ia_word16          wDataLength;
	/* Structure is followed by ia_byte bData[ wDataLength ]. */
} msg_config_descriptor_st;

typedef struct
{
	
	ia_byte              bConfigCount;
	/* Structure is followed by msg_config_descriptor_st Config_Desc[ bConfigCount ]. */
} msg_general_configuration_st;

#endif
