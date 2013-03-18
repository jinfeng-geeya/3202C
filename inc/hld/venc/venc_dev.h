#ifndef	__HLD_VENC_DEV_H__
#define __HLD_VENC_DEV_H__

#include <basic_types.h>
#include <Mediatypes.h>	

struct venc_device
{
	struct venc_device  *next;  /*next device */
       INT32  type;
	INT8  name[32];
	INT32  flags;

	INT32 busy;
	
	void *priv;		/* Used to be 'private' but that upsets C++ */
	
	INT32 (*init) ();
	INT32 (*open) (struct venc_device *);
	INT32 (*close) (struct venc_device *);
	INT32 (*i_frame) (struct venc_device *,UINT8*,UINT8*,UINT16,UINT16,UINT8*,UINT32*);
	//INT32 (*start) (struct venc_device *);
	//INT32 (*stop) (struct venc_device *); 

	// new added api to break the relation between the vdec and venc during the compile process
	void (*mp2enc)(struct venc_device *,UINT8*,UINT8* ,UINT16 ,UINT16 ,UINT8* ,UINT32* ,UINT8*,UINT8*);

	INT32 (*img_enc)(struct venc_device *, struct vdec_picture *, struct venc_img *);
	
};

#endif /*__HLD_VENC_DEV_H__*/
