#include <basic_types.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <osal/osal.h>

#include <hld/hld_dev.h>
#include <hld/venc/venc.h>



/*
 * 	Name		:   venc_open()   	
 *	Description	:   Open a venc device
 *	Parameter	:	struct venc_device *dev		: Device to be openned
 *	Return		:	INT32 						: Return value
 *
 */
INT32 venc_open(struct venc_device* dev)
{
	INT32 result=RET_FAILURE;
	
	/* If openned already, exit */
	if (dev->flags & HLD_DEV_STATS_UP)
	{
		PRINTF("venc_open: warning - device %s openned already!\n", dev->name);
		return RET_SUCCESS;
	}
	
	/* Open this device */
	if (dev->open)
	{
		result = dev->open(dev);
	}
	
	/* Setup init work mode */
	if (result == RET_SUCCESS)
	{
		dev->flags |= (HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);
		
	}
	
	return result;
}

/*
 * 	Name		:   venc_close()   	
 *	Description	:   Close a venc device
 *	Parameter	:	struct venc_device *dev		: Device to be closed
 *	Return		:	INT32 						: Return value
 *
 */
INT32 venc_close(struct venc_device *dev)
{
	INT32 result=RET_FAILURE;

	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		PRINTF("venc_close: warning - device %s closed already!\n", dev->name);
		return RET_SUCCESS;
	}
	
	/* Stop device */
	if (dev->close)
	{
		result = dev->close(dev);
	}
	
	/* Update flags */
	dev->flags &= ~(HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);
	
	return result;
}
INT32  venc_i_frame(struct venc_device *dev,UINT8* y_addr,UINT8* c_addr,UINT16 width,UINT16 height,UINT8* out_stream_buf,UINT32* out_stream_size)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->i_frame)
	{
		return dev->i_frame(dev,y_addr,c_addr,width,height,out_stream_buf,out_stream_size);
	}
	return RET_FAILURE;
}

/*
	Description: new added api to enc the i frm
	Arguments:
			dev 			-> enc device
			y_addr		-> src y buffer address
			c_addr		-> src c buffer address
			width 		-> src video width
			height		-> src video height
			out_stream_buf -> encoded stream buffer address
			out_stream_size -> encoded stream buffer size
			f_y_addr , f_c_addr -> temporally used buffer from vdec frame buffer
*/
void venc_mp2enc(struct venc_device *dev,UINT8* y_addr,UINT8* c_addr,UINT16 width,UINT16 height,UINT8* out_stream_buf,UINT32* out_stream_size,UINT8* f_y_addr,UINT8* f_c_addr)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return;
	}
	
	if (dev->mp2enc)
	{
		dev->mp2enc(dev,y_addr,c_addr,width,height,out_stream_buf,out_stream_size,f_y_addr,f_c_addr);
	}
	return;
}

INT32 venc_img_enc(struct venc_device *dev, struct vdec_picture *ppic, struct venc_img *pimg)
{
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->img_enc)
	{
		return dev->img_enc(dev, ppic, pimg);
	}
	
	return RET_FAILURE;
}

#if 0
INT32  venc_start(struct venc_device *dev)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->start)
	{
		return dev->start(dev);
	}
	return RET_FAILURE;
}

INT32  venc_stop(struct venc_device *dev)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->stop)
	{
		return dev->stop(dev);
	}
	return RET_FAILURE;

}
#endif

