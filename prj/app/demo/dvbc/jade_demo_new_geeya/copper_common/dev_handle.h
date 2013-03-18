#ifndef _DEV_HANDLE_H_
#define _DEV_HANDLE_H_

extern struct vdec_device *g_decv_dev;
extern struct deca_device *g_deca_dev;
extern struct dmx_device *g_dmx_dev;
extern struct nim_device *g_nim_dev;
extern struct nim_device *g_nim_dev2;
extern struct snd_device *g_snd_dev;
extern struct sdec_device *g_sdec_dev;
extern struct vbi_device *g_vbi_dev;
extern struct osd_device *g_osd_dev;
extern struct sto_device *g_sto_dev;
extern struct vpo_device *g_vpo_dev;
extern struct rfm_device *g_rfm_dev;
extern struct pan_device *g_pan_dev;
#ifdef MULTI_CAS
#if (CAS_TYPE == CAS_ABEL)
extern struct osd_device *g_osd_dev2;
#endif
#endif


#endif //_DEV_HANDLE_H_
