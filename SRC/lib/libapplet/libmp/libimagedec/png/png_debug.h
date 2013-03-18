
#ifndef _PNG_DEBUG_
#define _PNG_DEBUG_

#include <sys_config.h>
#include <basic_types.h>
#include <mediatypes.h>

#if 0
#define PNG_PRINTF libc_printf
#define PNG_ASSERT(code)				\
	{									\
		if (!(code))				\
		{								\
			PNG_PRINTF("error code<%d>: file \"%s\", line %d\n",	\
				code, __FILE__, __LINE__);	\
		}								\
		SDBBP();						\
	}
#else
#define PNG_PRINTF(...) 	do{}while(0)
#define PNG_ASSERT(...)	do{}while(0)
#endif

#define PNG_ENTER									\
		{\
			PNG_PRINTF("/***************/ \n");		   \
			PNG_PRINTF("      entrance <%s>\n",__FUNCTION__);\
			PNG_PRINTF("/***************/ \n");		   \
		}
		
#define PNG_EXIT										\
		{\
			PNG_PRINTF("/**************/ \n");		\
			PNG_PRINTF("      exit <%s>\n",__FUNCTION__);	\
			PNG_PRINTF("/**************/ \n");		\
		}


#define PNG_ERR_MALLOC			(-1)
#define PNG_ERR_INF				(-2)
#define PNG_ERR_FILE			(-3)

#define PNG_ERR_INF_STATUS		(-200)
#define PNG_ERR_NO_NEW_BIT	(-201)
#define PNG_ERR_BLK_TYPE		(-202)


#endif

