
#ifndef _GIF_DEBUG_
#define _GIF_DEBUG_


//DEBUG
#if 0
#define GIF_PRINTF libc_printf
#define GIF_ASSERT(code)				\
	{									\
		if (!(code))				\
		{								\
			GIF_PRINTF("error code<%d>: file \"%s\", line %d\n",	\
				code, __FILE__, __LINE__);	\
		}								\
		SDBBP();						\
	}
#else
#define GIF_PRINTF(...) 	do{}while(0)
#define GIF_ASSERT(...)	do{}while(0)
#endif

#define GIF_ENTER									\
		{\
			GIF_PRINTF("/***************/ \n");		   \
			GIF_PRINTF("      entrance <%s>\n",__FUNCTION__);\
			GIF_PRINTF("/***************/ \n");		   \
		}
		
#define GIF_EXIT										\
		{\
			GIF_PRINTF("/**************/ \n");		\
			GIF_PRINTF("      exit <%s>\n",__FUNCTION__);	\
			GIF_PRINTF("/**************/ \n");		\
		}


#define GIF_ERR(value)			do {g_gif_error_flag = value; return;} while(0)
#define GIF_ERR_RE(value)		do {g_gif_error_flag = value; return 0;} while(0)

//ERROR CODE
#define GIF_ERR_MALLOC							(-1)
#define GIF_ERR_DEC_START						(-2)
#define GIF_ERR_INFO_INIT						(-3)
#define GIF_ERR_LZW_INIT						(-4)
#define GIF_ERR_OUT_INIT						(-5)
#define GIF_ERR_ALLOCATE_CORE_BUF				(-6)
#define GIF_ERR_IN_INIT							(-7)

#define GIF_ERR_CORE_STATUS					(-20)
#define GIF_ERR_CORE_LOST_HEADER_INFO		(-21)
#define GIF_ERR_CORE_LOST_PALLETE				(-22)
#define GIF_ERR_CORE_FIRST_CODE				(-23)
#define GIF_ERR_UNSUP_OSD_FORMAT				(-24)

#define GIF_ERR_OTHERS							(-255)

#endif

