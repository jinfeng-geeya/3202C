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

/***************************************************************/
INT32 venc_m3327_i_frame(struct venc_device* dev,UINT8* y_addr,UINT8* c_addr,UINT16 width,UINT16 height,UINT8* out_stream_buf,UINT32* out_stream_size)
{
	struct venc_m3327_private * priv = (struct venc_m3327_private*)(dev->priv) ;

	struct vdec_io_get_frm_para tpara;
	tpara.ufrm_mode = VDEC_UN_DISPLAY;
	vdec_io_control((struct vdec_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DECV), VDEC_IO_GET_FRM, (UINT32)(&tpara));

	mpeg2enc(width,
		height,
		y_addr,
		c_addr,
		tpara.tFrmInfo.uY_Addr,
		tpara.tFrmInfo.uC_Addr,
		out_stream_buf,
		out_stream_size
		);
	return RET_SUCCESS; 
}

void venc_m3327_enable_i_frm_api(struct venc_device* dev)
{
	dev->i_frame = venc_m3327_i_frame;	
}
