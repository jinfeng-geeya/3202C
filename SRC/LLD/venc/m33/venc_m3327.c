#include <osal/osal.h>
#include <basic_types.h>

#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>

#include <hld/hld_dev.h>
#include <hld/venc/venc.h>
#include <hld/venc/venc_dev.h>
#include <hld/decv/vdec_driver.h>

#include "venc_m3327.h"

static char venc_m3327_name[HLD_MAX_NAME_SIZE] = "VENC_M3327_0";

INT32 venc_m3327_init()
{
	struct venc_device *dev;
	struct venc_m3327_private *tp;
	void *priv_mem;
	dev = dev_alloc(venc_m3327_name,HLD_DEV_TYPE_VENC,sizeof(struct venc_device));
	if (dev == NULL)
	{
		VENC_PRINTF("Error: Alloc video vencplay device error!\n");
		return RET_FAILURE;
	}

	/* Alloc structure space of private */
	priv_mem = (void *)MALLOC(sizeof(struct venc_m3327_private));
	if (priv_mem == NULL)
	{
		dev_free(dev);
		VENC_PRINTF("Alloc vencplay device prive memory error!/n");
		return RET_FAILURE;
	}

	MEMSET(priv_mem, 0, sizeof(struct venc_m3327_private));
	dev->priv =  tp = priv_mem;


       dev->next = NULL;
       dev->flags = 0;
	/* Function point init */
	dev->init = venc_m3327_init;

	dev->open = venc_m3327_open;
	dev->close = venc_m3327_close;

	//dev->ioctl = venc_m3327_ioctl;
	//dev->i_frame = venc_m3327_i_frame;
	//dev->start = venc_m3327_start;
	//dev->stop = venc_m3327_stop;

	dev->mp2enc = venc_m3327_mp2enc;
	/* Add this device to queue */
	if (dev_register(dev) != RET_SUCCESS)
	{
		VENC_PRINTF("Error: Register vencplay device error!\n");
		FREE(priv_mem);
		dev_free(dev);
		return RET_FAILURE;
	}
	
	VENC_PRINTF("VENC Attached!\n");
	
	return RET_SUCCESS;
}

/***************************************************************/
INT32 venc_m3327_open(struct venc_device* dev)
{
	struct venc_m3327_private * priv = (struct venc_m3327_private*)(dev->priv) ;
	/*
	priv->width = width;
	priv->height = height;
	priv->y_addr = y_addr;
	priv->c_addr = c_addr;
	priv->out_stream_buf = out_stream_buf;
	priv->out_stream_size = out_stream_size;
	*/
	return RET_SUCCESS;
}


/***************************************************************/
INT32 venc_m3327_close(struct venc_device* dev)
{
	return RET_SUCCESS;
}

void venc_m3327_mp2enc(struct venc_device *dev,UINT8* y_addr,UINT8* c_addr,UINT16 width,UINT16 height,UINT8* out_stream_buf,UINT32* out_stream_size,UINT8* f_y_addr,UINT8* f_c_addr)
{
	if(NULL != dev)
	{
		mpeg2enc(width,
			height,
			y_addr,
			c_addr,
			f_y_addr,
			f_c_addr,
			out_stream_buf,
			out_stream_size
			);
	}
}

#if 0
/***************************************************************/
INT32 venc_m3327_start(struct venc_device* dev)
{
	return RET_SUCCESS;
}

/***************************************************************/
INT32 venc_m3327_stop(struct venc_device* dev)
{
	return RET_SUCCESS;
}
#endif

