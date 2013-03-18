/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2008 Copyright (C)
 *  (C)
 *  File: png.c
 *  (I)
 *  Description: Entry of the png decoder.It contains all the APIs
 *  (S)
 *  History:(M)
 *      	Date        			Author         	Comment
 *      	====        			======		=======
 * 0.		2008.2.1			Sam		Create
 ****************************************************************************/
#include <sys_config.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <os/tds2/itron.h>
#include <api/libc/printf.h>
#include <osal/osal_timer.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <api/libmp/png.h>
#include "png_core.h"
#include "png_debug.h"

//global variables
struct png_instance g_png_instance[PNG_MAX_INSTANCE];
static UINT8 png_init = 0;

PNG_RET png_dec(struct png_instance *png,struct png_file *file,struct png_image *img,
	enum PNG_MODE mode)
{
	PNG_ENTER;
	png->par = (UINT32)file;
	if(png_core_start(png,img,mode))
	{
		PNG_EXIT;
		return PNG_TRUE;
	}
	return PNG_FALSE;
}

void png_stop(struct png_instance *png)
{
	enum PNG_STATUS status = PNG_IDLE;

	PNG_ENTER;
	png_core_finish(png);
	PNG_EXIT;
}

PNG_RET png_poll(struct png_instance *png,enum PNG_STATUS *status)
{
	PNG_ENTER;
	if(png_core_status(png,status))
	{
		PNG_EXIT;
		return PNG_TRUE;
	}	
	PNG_PRINTF("png dec poll fail\n");
	return PNG_FALSE;
}

PNG_RET png_view(struct png_instance *png,struct png_image *img,struct png_view_par *par
	,png_view_routine routine)
{
	PNG_ENTER;
	png->par = (UINT32)par;
	png->view_routine = routine;
	png_core_show_img(png,img);
	PNG_EXIT;
	return PNG_TRUE;
}

PNG_RET png_control(struct png_instance *png, UINT32 cmd, UINT32 par)
{
	switch(cmd)
	{
		case PNG_IOCMD_POLL_STATUS:
		{
			enum PNG_STATUS *pstatus = (enum PNG_STATUS *)par;
			
			if(!png_core_status(png,pstatus))
			{
				return PNG_FALSE;
			}	
			break;
		}
		default:
			break;
	}
	
	return PNG_TRUE;
}

struct png_instance *png_open(struct png_cfg *cfg)	
{
	struct png_instance *png = NULL;
	int i = 0;

	PNG_ENTER;
	if(0 == png_init)
	{
		png_init = 1;
		MEMSET((void *)g_png_instance,0,PNG_MAX_INSTANCE * sizeof(struct png_instance));
	}
	for(i = 0;i < PNG_MAX_INSTANCE;i++)
	{
		if(0 == g_png_instance[i].busy)
		{
			png = g_png_instance + i;
			png->busy = 1;
			png->id = i;
			png->par = (UINT32)cfg;
			if(png_core_init(png))
			{
				PNG_EXIT;
				return png;
			}
		}
	}
	PNG_PRINTF("png dec open fail\n");
	return NULL;	
}

void png_close(struct png_instance *png)
{
	PNG_ENTER;
	png_core_free(png);
	png->busy = 0;
	PNG_EXIT;
}

