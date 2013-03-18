
#ifndef __LLD_VENC_M3327_H__
#define __LLD_VENC_M3327_H__



#include <basic_types.h>

#include <hld/venc/venc.h>
#include <hld/venc/venc_dev.h>

#include "global.h"


struct venc_m3327_private
{
	UINT8* y_addr;
	UINT8* c_addr;
	UINT16 width;
	UINT16 height;
	UINT8* out_stream_buf;
	UINT32* out_stream_size;
};

#ifdef __cplusplus
extern "C"
{
#endif

INT32 venc_m3327_open(struct venc_device* dev);
INT32  venc_m3327_close(struct venc_device *dev);
INT32 venc_m3327_i_frame(struct venc_device* dev,UINT8* y_addr,UINT8* c_addr,UINT16 width,UINT16 height,UINT8* out_stream_buf,UINT32* out_stream_size);

//INT32  venc_m3327_start(struct venc_device *dev);
//INT32  venc_m3327_stop(struct venc_device *dev);
void venc_m3327_mp2enc(struct venc_device *dev,UINT8* y_addr,UINT8* c_addr,UINT16 width,UINT16 height,UINT8* out_stream_buf,UINT32* out_stream_size,UINT8* f_y_addr,UINT8* f_c_addr);

#ifdef __cplusplus
}
#endif

#endif /*__LLD_SDEC_M3327_H__*/



