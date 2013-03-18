#include "palette1.h"
#ifdef MULTI_CAS
#if (CAS_TYPE == CAS_ABEL)		
#include "palette2.h"
#endif
#endif

static unsigned char*	palette_array[] = 
{
	(unsigned char*)&palette_1[0][0],
#ifdef MULTI_CAS
#if (CAS_TYPE == CAS_ABEL)		
	(unsigned char*)&palette_2[0][0],
#endif
#endif
};



