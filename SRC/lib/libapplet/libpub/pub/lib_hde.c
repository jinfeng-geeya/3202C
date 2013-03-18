/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2003, 2004 (C)
 *
 *  File: lib_hde.c
 *
 *  Description: This file implement the hardware display engine on
 *               ALi M3327 chip.
 *
 *  History:
 *      Date        Author      Version     Comment
 *      ==========  ==========      =======     ===================
 ****************************************************************************/
#include <sys_config.h>
#include <basic_types.h>
#include <retcode.h>
#include <mediatypes.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <hld/hld_dev.h>
#include <api/libtsi/db_3l.h>
#include <hld/dmx/dmx.h>
#include <hld/dis/vpo.h>
#include <hld/decv/vdec_driver.h>
#include <api/libpub/lib_hde.h>

/*******************************************************
* macro define
********************************************************/
#define LEFT_MARGIN     32
#define TOP_MARGIN      96

#define VE_MODE_SWITCH_TIME     2000 // ms
#define H264_VE_MODE_SWITCH_TIME    8000 // ms
#define VE_MODE_SWITCH_TIME_29E 500 // ms


/*******************************************************
* extern  declare
********************************************************/
extern struct vdec_device * get_selected_decoder(void);

extern UINT16 l_cc_sema_id;
#define ENTER_CCHDE_API()   osal_semaphore_capture(l_cc_sema_id,TMO_FEVR);
#define LEAVE_CCHDE_API()   osal_semaphore_release(l_cc_sema_id)

/*******************************************************
* struct and variable
********************************************************/
static UINT32 l_hde_mode = VIEW_MODE_FULL;
static UINT32 l_hde_mode_initialized = FALSE;
static UINT32 l_hde_uIsPreviewResize = 0;

static struct Rect  l_hde_rectScrnPreview = {SCREEN_WIDTH>>2, SCREEN_HEIGHT>>2, SCREEN_WIDTH>>1, SCREEN_HEIGHT>>1 };
static struct Rect  l_hde_rectSrcFull = {0, 0, PICTURE_WIDTH, PICTURE_HEIGHT};
static struct Rect  l_hde_rectSrcFull_adjusted = {LEFT_MARGIN, TOP_MARGIN, PICTURE_WIDTH-LEFT_MARGIN*2, PICTURE_HEIGHT-TOP_MARGIN*2};
static struct Rect  l_hde_rectScrnFull = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};

static UINT8 l_hde_preview_method = 0;

/*******************************************************
* internal function
********************************************************/


/*******************************************************
* external API
********************************************************/
/* Set Background Color of VPO for Single Color Background */
void hde_back_color(UINT8 bY, UINT8 bCb, UINT8 bCr)
{
    struct  YCbCrColor color;

    color.uY = bY;
    color.uCb = bCb;
    color.uCr = bCr;
    vpo_ioctl((struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS), VPO_IO_SET_BG_COLOR, (UINT32)(&color));
#ifdef DUAL_VIDEO_OUTPUT
    vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 1), VPO_IO_SET_BG_COLOR, (UINT32)&color);
#endif
}
extern BOOL enable_blk_screen;

/* Set the View Mode of VPO Window */
void hde_set_mode_smoothly_switch(UINT32 mode)
{
    struct MPSource_CallBack MPCallBack;
    struct PIPSource_CallBack PIPCallBack;
    struct VDec_StatusInfo vdec_info;
    struct VDecPIPInfo tInitInfo;
    UINT32 waittime=0, wait_total_time = 0;
    struct vdec_device *pDecvDevice = (struct vdec_device *)get_selected_decoder();
    struct vdec_device *pMpeg2Device = (struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0);
    struct vdec_device *pAvcDevice = (struct vdec_device *)dev_get_by_name("DECV_AVC_0");
    BOOL need_to_wait = FALSE;
    BOOL need_to_set_output = FALSE;
    UINT8 ulock = 0;
    struct nim_device *nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 0);
    struct vpo_device *pDisDevice = (struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS);
#ifdef DUAL_VIDEO_OUTPUT
    struct vpo_device *pDisDevice_O = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 1);
    enum VDecOutputMode preview_mode = DUAL_PREVIEW_MODE;
    enum VDecOutputMode full_mode = DUAL_MODE;
#else
    struct vpo_device *pDisDevice_O = NULL;
    enum VDecOutputMode preview_mode = PREVIEW_MODE;
    enum VDecOutputMode full_mode = MP_MODE;
#endif
    struct vpo_io_get_info dis_info;

    switch(mode)
    {
    case VIEW_MODE_PREVIEW:
        if ((l_hde_mode != mode) || (l_hde_uIsPreviewResize == 1) || l_hde_mode_initialized == FALSE)
        {
            l_hde_mode_initialized = TRUE;
            ENTER_CCHDE_API();
            #if(VDEC27_PREVIEW_SOLUTION == VDEC27_PREVIEW_VE_SCALE)

            #ifdef HDTV_SUPPORT
            /* Check Frame Size, If Input Frame is HD, Set VE to Dview Mode */
            vpo_ioctl(pDisDevice, VPO_IO_GET_INFO, (UINT32) &dis_info);
            if(pDisDevice_O)
                vpo_ioctl(pDisDevice_O, VPO_IO_GET_INFO, (UINT32) &dis_info);

            tInitInfo.adv_setting.init_mode = 1;
            tInitInfo.adv_setting.switch_mode = 1; /* mp<=>preview switch smoothly */
            tInitInfo.adv_setting.out_sys = dis_info.tvsys;
            tInitInfo.adv_setting.bprogressive = dis_info.bprogressive;
            tInitInfo.src_rect.uStartX = l_hde_rectSrcFull_adjusted.uStartX;
            tInitInfo.src_rect.uStartY= l_hde_rectSrcFull_adjusted.uStartY;
            tInitInfo.src_rect.uWidth = l_hde_rectSrcFull_adjusted.uWidth;
            tInitInfo.src_rect.uHeight = l_hde_rectSrcFull_adjusted.uHeight;
            tInitInfo.dst_rect.uStartX = l_hde_rectScrnPreview.uStartX;
            tInitInfo.dst_rect.uStartY= l_hde_rectScrnPreview.uStartY;
            tInitInfo.dst_rect.uWidth = l_hde_rectScrnPreview.uWidth;
            tInitInfo.dst_rect.uHeight = l_hde_rectScrnPreview.uHeight;

            vdec_io_control(pMpeg2Device, VDEC_IO_SET_OUTPUT_RECT, (UINT32)&tInitInfo);
            vdec_io_control(pAvcDevice, VDEC_IO_SET_OUTPUT_RECT, (UINT32)&tInitInfo);
            vdec_set_output(pDecvDevice, preview_mode, &tInitInfo, &MPCallBack, &PIPCallBack);
            #if (!defined(_BUILD_OTA_E_) && defined(SUPPORT_MPEG4_TEST))
            /* for PE module, set other VDEC output mode, such as MPEG4 */
            video_set_output(preview_mode, &tInitInfo);
            #endif

            vdec_io_control(pDecvDevice, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);
            if(vdec_info.uCurStatus == VDEC_STOPPED)
                wait_total_time = 0;
            else
                wait_total_time = H264_VE_MODE_SWITCH_TIME;

            /* wait ve switch finish */
    		while(waittime<wait_total_time)
    		{
    			vdec_io_control(pDecvDevice, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);
    			if(vdec_info.output_mode == PREVIEW_MODE || vdec_info.output_mode == DUAL_PREVIEW_MODE)
    			{
    				if(vdec_info.uCurStatus == VDEC_DECODING)
    					osal_task_sleep(50);
    				break;
    			}
    			osal_task_sleep(1);
    			waittime++;
    		}

            #else   // else of #ifdef HDTV_SUPPORT
            /* Set Video Decoder Output Mode (PREVIEW_MODE) */
            tInitInfo.src_rect.uStartX = l_hde_rectSrcFull_adjusted.uStartX>>1;
            tInitInfo.src_rect.uStartY= l_hde_rectSrcFull_adjusted.uStartY>>1;
            tInitInfo.src_rect.uWidth = PICTURE_WIDTH - 2*tInitInfo.src_rect.uStartX;
            tInitInfo.src_rect.uHeight = PICTURE_HEIGHT - 2*tInitInfo.src_rect.uStartY;
            tInitInfo.dst_rect.uStartX = l_hde_rectScrnPreview.uStartX;
            tInitInfo.dst_rect.uStartY= l_hde_rectScrnPreview.uStartY;
            tInitInfo.dst_rect.uWidth = l_hde_rectScrnPreview.uWidth;
            tInitInfo.dst_rect.uHeight = l_hde_rectScrnPreview.uHeight;
            vdec_set_output(pDecvDevice,PREVIEW_MODE, &tInitInfo, &MPCallBack,&PIPCallBack);

            /* Wait Video Decoder Output Mode Switch to PREVIEW_MODE */
            vdec_info.output_mode = VIEW_MODE_FULL;
            while((vdec_info.output_mode != PREVIEW_MODE)&&(waittime<VE_MODE_SWITCH_TIME))
            {
                nim_get_lock(nim, &ulock);
                if(!ulock)
                    break;
                osal_task_sleep(1);
                waittime++;
                vdec_io_control(pDecvDevice, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);
            }

            /* Switch Video Decoder Output Mode to PREVIEW_MODE Forcely */
            if(vdec_info.output_mode != PREVIEW_MODE)
            {
                PRINTF("switch to preview forcely\n");
                vdec_stop(pDecvDevice,sys_data_get_cur_chan_mode() == PROG_TV_MODE,FALSE);
                vdec_set_output(pDecvDevice,PREVIEW_MODE, &tInitInfo, &MPCallBack,&PIPCallBack);
                vdec_start(pDecvDevice);
            }
            #endif  // end of #ifdef HDTV_SUPPORT
            #elif(VDEC27_PREVIEW_SOLUTION == VDEC27_PREVIEW_DE_SCALE) // end of #if(VDEC27_PREVIEW_SOLUTION == VDEC27_PREVIEW_VE_SCALE)
            /* If is DE Scale Operation, then adjust VPO Full Srceen to Preview Screen directly */
            if(0 == l_hde_preview_method)
                vpo_zoom(pDisDevice, &l_hde_rectSrcFull_adjusted, &l_hde_rectScrnPreview);
            /*
               If is VE Scale Operation, first set Video Decoder Output Mode(PIP_MODE),
               then adjust VPO Full Srceen to Preview Screen
            */
            else if(1 == l_hde_preview_method)
            {
                if(l_hde_mode == VIEW_MODE_PREVIEW)
                {
                    vpo_zoom(pDisDevice, &l_hde_rectSrcFull_adjusted, &l_hde_rectScrnPreview);
                }
                else
                {
                    /* Set Video Decoder Output Mode (IND_PIP_MODE) */
                    tInitInfo.src_rect.uStartX = l_hde_rectSrcFull_adjusted.uStartX>>1;
                    tInitInfo.src_rect.uStartY= l_hde_rectSrcFull_adjusted.uStartY>>1;
                    tInitInfo.src_rect.uWidth = PICTURE_WIDTH - 2*tInitInfo.src_rect.uStartX;
                    tInitInfo.src_rect.uHeight = PICTURE_HEIGHT - 2*tInitInfo.src_rect.uStartY;
                    tInitInfo.dst_rect.uStartX = l_hde_rectScrnPreview.uStartX;
                    tInitInfo.dst_rect.uStartY= l_hde_rectScrnPreview.uStartY;
                    tInitInfo.dst_rect.uWidth = l_hde_rectScrnPreview.uWidth;
                    tInitInfo.dst_rect.uHeight = l_hde_rectScrnPreview.uHeight;
                    tInitInfo.pipSize.uWidth=l_hde_rectScrnPreview.uWidth;
                    tInitInfo.pipSize.uHeight=l_hde_rectScrnPreview.uHeight/5;
                    vdec_set_output(pDecvDevice,IND_PIP_MODE, &tInitInfo, &MPCallBack,&PIPCallBack);

                    /* After Video Decoder Output Mode switch to PIP_MODE, then adjust VPO */
                    vdec_io_control(pDecvDevice, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);
                    if(VDEC27_STOPPED == vdec_info.uCurStatus)
                    {
                        vpo_zoom(pDisDevice, &l_hde_rectSrcFull_adjusted, &l_hde_rectScrnPreview);
                    }
                    else
                    {
                        waittime = 0;
                        while((vdec_info.output_mode != PIP_MODE)&&(waittime<VE_MODE_SWITCH_TIME_29E))
                        {
                            nim_get_lock(nim, &ulock);
                            if(!ulock)
                                break;
                            osal_task_sleep(20);
                            waittime += 20;
                            vdec_io_control(pDecvDevice, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);
                        }
                        if(vdec_info.output_mode != PIP_MODE)
                        {
                            vdec_stop(pDecvDevice,TRUE,FALSE);
                            vdec_set_output(pDecvDevice,IND_PIP_MODE, &tInitInfo, &MPCallBack,&PIPCallBack);
                            vdec_start(pDecvDevice);
                            vpo_zoom(pDisDevice, &l_hde_rectSrcFull_adjusted, &l_hde_rectScrnPreview);
                        }
                        else
                        {
                            vpo_zoom(pDisDevice, &l_hde_rectSrcFull_adjusted, &l_hde_rectScrnPreview);
                        }
                    }
                    vpo_ioctl(pDisDevice, VPO_IO_ADJUST_LM_IN_PREVIEW, 1);
                }
            }
            #endif  // end of #elif(VDEC27_PREVIEW_SOLUTION == VDEC27_PREVIEW_DE_SCALE)
        }
        l_hde_uIsPreviewResize = 0;
        LEAVE_CCHDE_API();
        break;
    case VIEW_MODE_MULTI:
        break;
    case VIEW_MODE_FULL:
    default:
        if (l_hde_mode != mode || l_hde_mode_initialized == FALSE)
        {
            l_hde_mode_initialized = TRUE;
            ENTER_CCHDE_API();
            #if(VDEC27_PREVIEW_SOLUTION == VDEC27_PREVIEW_VE_SCALE)
            #ifdef HDTV_SUPPORT
            /* convert VE to MP mode */
            vpo_ioctl(pDisDevice, VPO_IO_GET_INFO, (UINT32) &dis_info);
            if(pDisDevice_O)
                vpo_ioctl(pDisDevice_O, VPO_IO_GET_INFO, (UINT32) &dis_info);
            need_to_set_output = TRUE;

            tInitInfo.adv_setting.init_mode = 0;
            tInitInfo.adv_setting.switch_mode = 1; /* mp<=>preview switch smoothly */
            tInitInfo.adv_setting.out_sys = dis_info.tvsys;
            tInitInfo.adv_setting.bprogressive = dis_info.bprogressive;
            tInitInfo.src_rect.uStartX = l_hde_rectSrcFull.uStartX;
            tInitInfo.src_rect.uStartY= l_hde_rectSrcFull.uStartY;
            tInitInfo.src_rect.uWidth = l_hde_rectSrcFull.uWidth;
            tInitInfo.src_rect.uHeight = l_hde_rectSrcFull.uHeight;
            tInitInfo.dst_rect.uStartX = l_hde_rectScrnFull.uStartX;
            tInitInfo.dst_rect.uStartY= l_hde_rectScrnFull.uStartY;
            tInitInfo.dst_rect.uWidth = l_hde_rectScrnFull.uWidth;
            tInitInfo.dst_rect.uHeight = l_hde_rectScrnFull.uHeight;

            vdec_io_control(pMpeg2Device, VDEC_IO_SET_OUTPUT_RECT, (UINT32)&tInitInfo);
            vdec_io_control(pAvcDevice, VDEC_IO_SET_OUTPUT_RECT, (UINT32)&tInitInfo);
            vdec_set_output(pDecvDevice, full_mode, &tInitInfo, &MPCallBack, &PIPCallBack);
            #if (!defined(_BUILD_OTA_E_) && defined(SUPPORT_MPEG4_TEST))
            /* for PE module, set other VDEC output mode, such as MPEG4 */
            video_set_output(full_mode, &tInitInfo);
            #endif

            vdec_io_control(pDecvDevice, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);
            if(vdec_info.uCurStatus == VDEC_STOPPED)
                wait_total_time = 0;
            else
                wait_total_time = H264_VE_MODE_SWITCH_TIME;

            /* wait ve switch finish */
			while(waittime<wait_total_time)
			{
				 vdec_io_control(pDecvDevice, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);
				 if(vdec_info.output_mode == MP_MODE || vdec_info.output_mode == DUAL_MODE)
				 {
                     if(vdec_info.uCurStatus == VDEC_DECODING)
				         osal_task_sleep(50);
				     break;
				 }
				 osal_task_sleep(1);
				 waittime++;
			}

            #else   // else of #ifdef HDTV_SUPPORT
            /* Set Video Decoder Output Mode (MP_MODE) */
            tInitInfo.buse_sml_buf = FALSE;
            tInitInfo.src_rect.uStartX = l_hde_rectSrcFull.uStartX;
            tInitInfo.src_rect.uStartY= l_hde_rectSrcFull.uStartY;
            tInitInfo.src_rect.uWidth = l_hde_rectSrcFull.uWidth;
            tInitInfo.src_rect.uHeight = l_hde_rectSrcFull.uHeight;
            tInitInfo.dst_rect.uStartX = l_hde_rectScrnFull.uStartX;
            tInitInfo.dst_rect.uStartY= l_hde_rectScrnFull.uStartY;
            tInitInfo.dst_rect.uWidth = l_hde_rectScrnFull.uWidth;
            tInitInfo.dst_rect.uHeight = l_hde_rectScrnFull.uHeight;
            vdec_set_output(pDecvDevice,MP_MODE, &tInitInfo, &MPCallBack,&PIPCallBack);

            /* Wait Video Decoder Output Mode Switch to MP_MODE */
            vdec_io_control(pDecvDevice, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);
            while(((vdec_info.output_mode != MP_MODE)||(vdec_info.use_sml_buf))&&(waittime<VE_MODE_SWITCH_TIME))
            {
                nim_get_lock(nim, &ulock);
                if(!ulock)
                    break;
                osal_task_sleep(1);
                waittime++;
                vdec_io_control(pDecvDevice, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);
            }
            /* Switch Video Decoder Output Mode to MP_MODE Forcely */
            if((vdec_info.output_mode != MP_MODE)||(vdec_info.use_sml_buf))
            {
                PRINTF("switch to mp forcely\n");

                vdec_stop(pDecvDevice,TRUE,FALSE);
                vdec_set_output(pDecvDevice,MP_MODE, &tInitInfo, &MPCallBack,&PIPCallBack);
                vdec_start(pDecvDevice);
            }
            #endif  // end of #ifdef HDTV_SUPPORT
            #elif(VDEC27_PREVIEW_SOLUTION == VDEC27_PREVIEW_DE_SCALE)   // end of #if(VDEC27_PREVIEW_SOLUTION == VDEC27_PREVIEW_VE_SCALE)
            /* If is DE Scale Operation, then adjust VPO to Full Srceen directly */
            if(0 == l_hde_preview_method)
                vpo_zoom(pDisDevice, &l_hde_rectSrcFull, &l_hde_rectScrnFull);
            /*
               If is VE Scale Operation, first set Video Decoder Output Mode(MP_MODE),
               then adjust VPO to Full Srceen
            */
            else if(1 == l_hde_preview_method)
            {
                /* Set Video Decoder Output Mode (MP_MODE) */
                tInitInfo.src_rect.uStartX = l_hde_rectSrcFull.uStartX;
                tInitInfo.src_rect.uStartY= l_hde_rectSrcFull.uStartY;
                tInitInfo.src_rect.uWidth = l_hde_rectSrcFull.uWidth;
                tInitInfo.src_rect.uHeight = l_hde_rectSrcFull.uHeight;
                tInitInfo.dst_rect.uStartX = l_hde_rectScrnFull.uStartX;
                tInitInfo.dst_rect.uStartY= l_hde_rectScrnFull.uStartY;
                tInitInfo.dst_rect.uWidth = l_hde_rectScrnFull.uWidth;
                tInitInfo.dst_rect.uHeight = l_hde_rectScrnFull.uHeight;
                vdec_set_output(pDecvDevice,MP_MODE, &tInitInfo, &MPCallBack,&PIPCallBack);

                /* After Video Decoder Output Mode switch to MP_MODE, then adjust VPO */
                vdec_io_control(pDecvDevice, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);
                if(VDEC27_STOPPED == vdec_info.uCurStatus)
                {
                    vpo_zoom(pDisDevice, &l_hde_rectSrcFull, &l_hde_rectScrnFull);
                }
                else
                {
                    waittime = 0;
                    while((vdec_info.output_mode != MP_MODE)&&(waittime<VE_MODE_SWITCH_TIME_29E))
                    {
                        nim_get_lock(nim, &ulock);
                        if(!ulock)
                            break;
                        osal_task_sleep(20);
                        waittime += 20;
                        vdec_io_control(pDecvDevice, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);
                    }
                    if(vdec_info.output_mode != MP_MODE)
                    {
                        vdec_stop(pDecvDevice,TRUE,FALSE);
                        vdec_set_output(pDecvDevice,MP_MODE, &tInitInfo, &MPCallBack,&PIPCallBack);
                        vdec_start(pDecvDevice);
                        vpo_zoom(pDisDevice, &l_hde_rectSrcFull, &l_hde_rectScrnFull);
                    }
                    else
                    {
                        vpo_zoom(pDisDevice, &l_hde_rectSrcFull, &l_hde_rectScrnFull);
                    }
                }

                vpo_ioctl(pDisDevice, VPO_IO_ADJUST_LM_IN_PREVIEW, 0);
            }
            #endif  // end of #elif(VDEC27_PREVIEW_SOLUTION == VDEC27_PREVIEW_DE_SCALE)
            LEAVE_CCHDE_API();
        }
    }
        l_hde_mode = mode;
}

void hde_set_mode(UINT32 mode)
{
#ifdef MP_PREVIEW_SWITCH_SMOOTHLY
    return hde_set_mode_smoothly_switch(mode);
#endif

    struct MPSource_CallBack MPCallBack;
    struct PIPSource_CallBack PIPCallBack;
    struct VDec_StatusInfo vdec_info;
    struct VDecPIPInfo tInitInfo;
    UINT32 waittime=0, wait_total_time = 0;
    struct vdec_device *pDecvDevice = (struct vdec_device *)get_selected_decoder();
    BOOL need_to_wait = FALSE;
    BOOL need_to_set_output = FALSE;
    UINT8 ulock = 0;
    struct nim_device *nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 0);
    struct vpo_device *pDisDevice = (struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS);
#ifdef DUAL_VIDEO_OUTPUT
    struct vpo_device *pDisDevice_O = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 1);
#endif
    struct vpo_io_get_info dis_info;

    switch(mode)
    {
    case VIEW_MODE_PREVIEW:
        if ((l_hde_mode != mode) || (l_hde_uIsPreviewResize == 1))
        {
            ENTER_CCHDE_API();
            #if(VDEC27_PREVIEW_SOLUTION == VDEC27_PREVIEW_VE_SCALE)

            #ifdef HDTV_SUPPORT
            /* Check Frame Size, If Input Frame is HD, Set VE to Dview Mode */
            vpo_ioctl(pDisDevice, VPO_IO_GET_INFO, (UINT32) &dis_info);
            #ifdef DUAL_VIDEO_OUTPUT
            vpo_ioctl(pDisDevice_O, VPO_IO_GET_INFO, (UINT32) &dis_info);
            #endif //end of #ifdef DUAL_VIDEO_OUTPUT
            if(is_cur_decoder_avc())
            {
                vdec_io_control(pDecvDevice, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);
                if(vdec_info.pic_width > 720 || vdec_info.pic_height > 576)
                {
                    need_to_wait = TRUE;
                    if(vdec_info.uCurStatus == VDEC_DECODING)
                    {
                        vpo_win_onoff(pDisDevice, FALSE);
                        #ifdef DUAL_VIDEO_OUTPUT
                        vpo_win_onoff(pDisDevice_O, FALSE);
                        #endif
                    }
                }
                else
                    need_to_wait = FALSE;
            }
            else
            {
                need_to_wait = TRUE;
                if(enable_blk_screen)
                    vdec_stop(pDecvDevice,TRUE,TRUE);
                else
                    vdec_stop(pDecvDevice,TRUE,FALSE);
            }

            tInitInfo.adv_setting.init_mode = 1;
            tInitInfo.adv_setting.out_sys = dis_info.tvsys;
            tInitInfo.adv_setting.bprogressive = dis_info.bprogressive;
            tInitInfo.src_rect.uStartX = l_hde_rectSrcFull_adjusted.uStartX>>1;
            tInitInfo.src_rect.uStartY= l_hde_rectSrcFull_adjusted.uStartY>>1;
            tInitInfo.src_rect.uWidth = PICTURE_WIDTH - 2*tInitInfo.src_rect.uStartX;
            tInitInfo.src_rect.uHeight = PICTURE_HEIGHT - 2*tInitInfo.src_rect.uStartY;
            tInitInfo.dst_rect.uStartX = l_hde_rectScrnPreview.uStartX;
            tInitInfo.dst_rect.uStartY= l_hde_rectScrnPreview.uStartY;
            tInitInfo.dst_rect.uWidth = l_hde_rectScrnPreview.uWidth;
            tInitInfo.dst_rect.uHeight = l_hde_rectScrnPreview.uHeight;

            //if(need_to_wait)
            {

                #if 0 // I am confused by the following code. -- Jerry Long @2010-3-4

                #ifdef DUAL_VIDEO_OUTPUT
                if(is_cur_decoder_avc())
                #else
                if(0)
                #endif
                {
                    vdec_set_output(pDecvDevice,DUAL_PREVIEW_MODE, &tInitInfo, &MPCallBack, &PIPCallBack);
                }
                else
                {
                    vdec_set_output(pDecvDevice,    DUAL_PREVIEW_MODE, &tInitInfo, &MPCallBack, &PIPCallBack);
                }
                #else   // else of #if 0
                #ifdef DUAL_VIDEO_OUTPUT
                vdec_set_output(pDecvDevice,    DUAL_PREVIEW_MODE, &tInitInfo, &MPCallBack, &PIPCallBack);
                #else   // else of #ifdef DUAL_VIDEO_OUTPUT
                vdec_set_output(pDecvDevice,    PREVIEW_MODE, &tInitInfo, &MPCallBack, &PIPCallBack);
                #endif  // end of #ifdef DUAL_VIDEO_OUTPUT
                #endif  //  end of #if 0
            }

            #ifndef _BUILD_OTA_E_
            #ifdef DUAL_VIDEO_OUTPUT
            #if defined(SUPPORT_MPEG4_TEST)
            // for PE module, set other VDEC output mode, such as MPEG4
            video_set_output(DUAL_PREVIEW_MODE, &tInitInfo);
            #endif  // end of #if defined(SUPPORT_MPEG4_TEST)
            #else // else of #ifdef DUAL_VIDEO_OUTPUT
            #if defined(SUPPORT_MPEG4_TEST)
            // for PE module, set other VDEC output mode, such as MPEG4
            video_set_output(PREVIEW_MODE, &tInitInfo);
            #endif  // end of #if defined(SUPPORT_MPEG4_TEST)
            #endif  // end of #ifdef DUAL_VIDEO_OUTPUT
            #endif  // end of #ifndef _BUILD_OTA_E_
            if(!is_cur_decoder_avc())
            {
                vdec_start(pDecvDevice);
            }
            if(is_cur_decoder_avc() && need_to_wait)
            {
                struct dmx_device *dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);

                //For H264, we have to wait for 1st picture decoded, then call vpo_zoom, otherwise, it could cause below issue:
                //1 1. DE under run, because DE can scale down HD full size picture to preview size
                //1 2. In full screen mode, VE mapping is H264 mapping mode, in preview mode, VE mapping is MPEG2 mapping mode
                //--Michael Xie 2007/8/29
                wait_total_time = VE_MODE_SWITCH_TIME;
                while(waittime<wait_total_time)
                {
                    nim_get_lock(nim, &ulock);
                    if(!ulock)
                        break;
                    if(RET_SUCCESS == dmx_io_control(dmx, IS_AV_SCRAMBLED, 0))
                        break;
                    vdec_io_control(pDecvDevice, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);
                    if(vdec_info.uFirstPicShowed)
                    {
                        wait_total_time = H264_VE_MODE_SWITCH_TIME;
                    }

                    if(vdec_info.output_mode == PREVIEW_MODE || vdec_info.output_mode == DUAL_PREVIEW_MODE)
                    {
                        if(vdec_info.uCurStatus == VDEC_DECODING)
                            osal_task_sleep(50);
                        break;
                    }
                    osal_task_sleep(1);
                    waittime++;
                }
            }

            vpo_zoom(pDisDevice, &l_hde_rectSrcFull_adjusted, &l_hde_rectScrnPreview);
            #ifdef DUAL_VIDEO_OUTPUT
            vpo_zoom(pDisDevice_O, &l_hde_rectSrcFull_adjusted, &l_hde_rectScrnPreview);
            vpo_ioctl(pDisDevice_O, VPO_IO_SET_PREVIEW_MODE, 1); // make vpo do not notify HDMI if source AR is changed in preview to avoid black screen when change channel
            #endif  // end of #ifdef DUAL_VIDEO_OUTPUT
            vpo_ioctl(pDisDevice, VPO_IO_SET_PREVIEW_MODE, 1); // make vpo do not notify HDMI if source AR is changed in preview to avoid black screen when change channel

            #else   // else of #ifdef HDTV_SUPPORT
            //Set Video Decoder Output Mode (PREVIEW_MODE)
            tInitInfo.src_rect.uStartX = l_hde_rectSrcFull_adjusted.uStartX>>1;
            tInitInfo.src_rect.uStartY= l_hde_rectSrcFull_adjusted.uStartY>>1;
            tInitInfo.src_rect.uWidth = PICTURE_WIDTH - 2*tInitInfo.src_rect.uStartX;
            tInitInfo.src_rect.uHeight = PICTURE_HEIGHT - 2*tInitInfo.src_rect.uStartY;
            tInitInfo.dst_rect.uStartX = l_hde_rectScrnPreview.uStartX;
            tInitInfo.dst_rect.uStartY= l_hde_rectScrnPreview.uStartY;
            tInitInfo.dst_rect.uWidth = l_hde_rectScrnPreview.uWidth;
            tInitInfo.dst_rect.uHeight = l_hde_rectScrnPreview.uHeight;
            vdec_set_output(pDecvDevice,PREVIEW_MODE, &tInitInfo, &MPCallBack,&PIPCallBack);

            //Wait Video Decoder Output Mode Switch to PREVIEW_MODE
            vdec_info.output_mode = VIEW_MODE_FULL;
            while((vdec_info.output_mode != PREVIEW_MODE)&&(waittime<VE_MODE_SWITCH_TIME))
            {
                nim_get_lock(nim, &ulock);
                if(!ulock)
                    break;
                osal_task_sleep(1);
                waittime++;
                vdec_io_control(pDecvDevice, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);
            }

            //Switch Video Decoder Output Mode to PREVIEW_MODE Forcely
            if(vdec_info.output_mode != PREVIEW_MODE)
            {
                PRINTF("switch to preview forcely\n");
                vdec_stop(pDecvDevice,sys_data_get_cur_chan_mode() == PROG_TV_MODE,FALSE);
                vdec_set_output(pDecvDevice,PREVIEW_MODE, &tInitInfo, &MPCallBack,&PIPCallBack);
                vdec_start(pDecvDevice);
            }
            #endif  // end of #ifdef HDTV_SUPPORT
            #elif(VDEC27_PREVIEW_SOLUTION == VDEC27_PREVIEW_DE_SCALE) // end of #if(VDEC27_PREVIEW_SOLUTION == VDEC27_PREVIEW_VE_SCALE)
            //If is DE Scale Operation, then adjust VPO Full Srceen to Preview Screen directly
            if(0 == l_hde_preview_method)
                vpo_zoom(pDisDevice, &l_hde_rectSrcFull_adjusted, &l_hde_rectScrnPreview);
            //If is VE Scale Operation, first set Video Decoder Output Mode(PIP_MODE),
            //then adjust VPO Full Srceen to Preview Screen
            else if(1 == l_hde_preview_method)
            {
                if(l_hde_mode == VIEW_MODE_PREVIEW)
                {
                    vpo_zoom(pDisDevice, &l_hde_rectSrcFull_adjusted, &l_hde_rectScrnPreview);
                }
                else
                {
                    //Set Video Decoder Output Mode (IND_PIP_MODE)
                    tInitInfo.src_rect.uStartX = l_hde_rectSrcFull_adjusted.uStartX>>1;
                    tInitInfo.src_rect.uStartY= l_hde_rectSrcFull_adjusted.uStartY>>1;
                    tInitInfo.src_rect.uWidth = PICTURE_WIDTH - 2*tInitInfo.src_rect.uStartX;
                    tInitInfo.src_rect.uHeight = PICTURE_HEIGHT - 2*tInitInfo.src_rect.uStartY;
                    tInitInfo.dst_rect.uStartX = l_hde_rectScrnPreview.uStartX;
                    tInitInfo.dst_rect.uStartY= l_hde_rectScrnPreview.uStartY;
                    tInitInfo.dst_rect.uWidth = l_hde_rectScrnPreview.uWidth;
                    tInitInfo.dst_rect.uHeight = l_hde_rectScrnPreview.uHeight;
                                        tInitInfo.pipSize.uWidth=l_hde_rectScrnPreview.uWidth;
                    tInitInfo.pipSize.uHeight=l_hde_rectScrnPreview.uHeight/5;
                    vdec_set_output(pDecvDevice,IND_PIP_MODE, &tInitInfo, &MPCallBack,&PIPCallBack);

                    //After Video Decoder Output Mode switch to PIP_MODE, then adjust VPO
                    vdec_io_control(pDecvDevice, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);
                    if(VDEC27_STOPPED == vdec_info.uCurStatus)
                    {
                        vpo_zoom(pDisDevice, &l_hde_rectSrcFull_adjusted, &l_hde_rectScrnPreview);
                    }
                    else
                    {
                        waittime = 0;
                        while((vdec_info.output_mode != PIP_MODE)&&(waittime<VE_MODE_SWITCH_TIME_29E))
                        {
                            nim_get_lock(nim, &ulock);
                            if(!ulock)
                                break;
                            osal_task_sleep(20);
                            waittime += 20;
                            vdec_io_control(pDecvDevice, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);
                        }
                        if(vdec_info.output_mode != PIP_MODE)
                        {
                            vdec_stop(pDecvDevice,TRUE,FALSE);
                            vdec_set_output(pDecvDevice,IND_PIP_MODE, &tInitInfo, &MPCallBack,&PIPCallBack);
                            vdec_start(pDecvDevice);
                            vpo_zoom(pDisDevice, &l_hde_rectSrcFull_adjusted, &l_hde_rectScrnPreview);
                        }
                        else
                        {
                            vpo_zoom(pDisDevice, &l_hde_rectSrcFull_adjusted, &l_hde_rectScrnPreview);
                        }
                    }
                    vpo_ioctl(pDisDevice, VPO_IO_ADJUST_LM_IN_PREVIEW, 1);
                }
            }
            #endif  // end of #elif(VDEC27_PREVIEW_SOLUTION == VDEC27_PREVIEW_DE_SCALE)
        }
        l_hde_uIsPreviewResize = 0;
        LEAVE_CCHDE_API();
        break;
    case VIEW_MODE_MULTI:
        break;
    case VIEW_MODE_FULL:
    default:
        if (l_hde_mode != mode)
        {
            ENTER_CCHDE_API();
            #if(VDEC27_PREVIEW_SOLUTION == VDEC27_PREVIEW_VE_SCALE)
            #ifdef HDTV_SUPPORT
            // convert VE to MP mode
            vpo_ioctl(pDisDevice, VPO_IO_GET_INFO, (UINT32) &dis_info);
            #ifdef DUAL_VIDEO_OUTPUT
            vpo_ioctl(pDisDevice_O, VPO_IO_GET_INFO, (UINT32) &dis_info);
            #endif  // end of #ifdef DUAL_VIDEO_OUTPUT
            need_to_set_output = FALSE;
            if(is_cur_decoder_avc())
            {
                vdec_io_control(pDecvDevice, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);
                if(vdec_info.pic_width > 720 || vdec_info.pic_height > 576)
                {
                    need_to_wait = TRUE;
                    if(vdec_info.uCurStatus == VDEC_DECODING)
                    {
                        vpo_win_onoff(pDisDevice, FALSE);
                        #ifdef DUAL_VIDEO_OUTPUT
                        vpo_win_onoff(pDisDevice_O, FALSE);
                        #endif  // end of #ifdef DUAL_VIDEO_OUTPUT
                        osal_task_sleep(50);
                    }
                }
                else if(vdec_info.pic_width == 0 || vdec_info.pic_height == 0)
                {
                    need_to_set_output = TRUE;
                }
                else
                {
                    need_to_wait = FALSE;
                }
            }
            else
            {
                need_to_wait = TRUE;
                if(enable_blk_screen)
                    vdec_stop(pDecvDevice,TRUE,TRUE);
                else
                    vdec_stop(pDecvDevice,TRUE,FALSE);
            }

            tInitInfo.adv_setting.init_mode = 0;
            tInitInfo.adv_setting.out_sys = dis_info.tvsys;
            tInitInfo.adv_setting.bprogressive = dis_info.bprogressive;
            tInitInfo.src_rect.uStartX = l_hde_rectSrcFull.uStartX;
            tInitInfo.src_rect.uStartY= l_hde_rectSrcFull.uStartY;
            tInitInfo.src_rect.uWidth = l_hde_rectSrcFull.uWidth;
            tInitInfo.src_rect.uHeight = l_hde_rectSrcFull.uHeight;
            tInitInfo.dst_rect.uStartX = l_hde_rectScrnFull.uStartX;
            tInitInfo.dst_rect.uStartY= l_hde_rectScrnFull.uStartY;
            tInitInfo.dst_rect.uWidth = l_hde_rectScrnFull.uWidth;
            tInitInfo.dst_rect.uHeight = l_hde_rectScrnFull.uHeight;

            //if(need_to_wait || need_to_set_output)
            {
                #if 0 // I am confused by the following code. -- Jerry Long @2010-3-4
                #ifdef DUAL_VIDEO_OUTPUT
                if(is_cur_decoder_avc())
                #else
                if(0)
                #endif
                {
                    vdec_set_output(pDecvDevice,DUAL_MODE, &tInitInfo, &MPCallBack, &PIPCallBack);
                }
                else
                {
                    vdec_set_output(pDecvDevice,DUAL_MODE, &tInitInfo, &MPCallBack, &PIPCallBack);
                }
                #else // else of #if 0
                #ifdef DUAL_VIDEO_OUTPUT
                vdec_set_output(pDecvDevice,DUAL_MODE, &tInitInfo, &MPCallBack, &PIPCallBack);
                #else   // else of #ifdef DUAL_VIDEO_OUTPUT
                vdec_set_output(pDecvDevice,MP_MODE, &tInitInfo, &MPCallBack, &PIPCallBack);
                #endif  // end of #ifdef DUAL_VIDEO_OUTPUT
                #endif  // end of #if 0
            }

            #ifndef _BUILD_OTA_E_
            #ifdef DUAL_VIDEO_OUTPUT
            #if defined(SUPPORT_MPEG4_TEST)
            // for PE module, set other VDEC output mode, such as MPEG4
            video_set_output(DUAL_MODE, &tInitInfo);
            #endif  // end of #if defined(SUPPORT_MPEG4_TEST)
            #else   // else of #ifdef DUAL_VIDEO_OUTPUT
            #if defined(SUPPORT_MPEG4_TEST)
            // for PE module, set other VDEC output mode, such as MPEG4
            video_set_output(MP_MODE, &tInitInfo);
            #endif  // end of #if defined(SUPPORT_MPEG4_TEST)
            #endif  // end of #ifdef DUAL_VIDEO_OUTPUT
            #endif  // end of #ifndef _BUILD_OTA_E_
            if(!is_cur_decoder_avc())
            {
                vdec_start(pDecvDevice);
            }

            vpo_zoom(pDisDevice, &l_hde_rectSrcFull, &l_hde_rectScrnFull);
            #ifdef DUAL_VIDEO_OUTPUT
            vpo_zoom(pDisDevice_O, &l_hde_rectSrcFull, &l_hde_rectScrnFull);
            vpo_ioctl(pDisDevice_O, VPO_IO_SET_PREVIEW_MODE, 0); // make vpo do not notify HDMI if source AR is changed in preview to avoid black screen when change channel
            #endif
            vpo_ioctl(pDisDevice, VPO_IO_SET_PREVIEW_MODE, 0);// set io command to make vpo notify HDMI if source AR is changed in full mode
            //  osal_task_sleep(50);
            if(is_cur_decoder_avc() && need_to_wait)
            {
                struct dmx_device *dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);

                wait_total_time = VE_MODE_SWITCH_TIME;
                while(waittime<wait_total_time)
                {
                    nim_get_lock(nim, &ulock);
                    if(!ulock)
                        break;
                    if(RET_SUCCESS == dmx_io_control(dmx, IS_AV_SCRAMBLED, 0))
                        break;
                    vdec_io_control(pDecvDevice, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);
                    if(vdec_info.uFirstPicShowed)
                    {
                        wait_total_time = H264_VE_MODE_SWITCH_TIME;
                    }

                    if(vdec_info.output_mode == MP_MODE || vdec_info.output_mode == DUAL_MODE)
                    {
                        break;
                    }
                    osal_task_sleep(1);
                    waittime++;
                }
            }

            #else   // else of #ifdef HDTV_SUPPORT
            //Set Video Decoder Output Mode (MP_MODE)
            tInitInfo.buse_sml_buf = FALSE;
            tInitInfo.src_rect.uStartX = l_hde_rectSrcFull.uStartX;
            tInitInfo.src_rect.uStartY= l_hde_rectSrcFull.uStartY;
            tInitInfo.src_rect.uWidth = l_hde_rectSrcFull.uWidth;
            tInitInfo.src_rect.uHeight = l_hde_rectSrcFull.uHeight;
            tInitInfo.dst_rect.uStartX = l_hde_rectScrnFull.uStartX;
            tInitInfo.dst_rect.uStartY= l_hde_rectScrnFull.uStartY;
            tInitInfo.dst_rect.uWidth = l_hde_rectScrnFull.uWidth;
            tInitInfo.dst_rect.uHeight = l_hde_rectScrnFull.uHeight;
            vdec_set_output(pDecvDevice,MP_MODE, &tInitInfo, &MPCallBack,&PIPCallBack);

            //Wait Video Decoder Output Mode Switch to MP_MODE
            vdec_io_control(pDecvDevice, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);
            while(((vdec_info.output_mode != MP_MODE)||(vdec_info.use_sml_buf))&&(waittime<VE_MODE_SWITCH_TIME))
            {
                nim_get_lock(nim, &ulock);
                if(!ulock)
                    break;
                osal_task_sleep(1);
                waittime++;
                vdec_io_control(pDecvDevice, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);
            }
            //Switch Video Decoder Output Mode to MP_MODE Forcely
            if((vdec_info.output_mode != MP_MODE)||(vdec_info.use_sml_buf))
            {
                PRINTF("switch to mp forcely\n");

                vdec_stop(pDecvDevice,TRUE,FALSE);
                vdec_set_output(pDecvDevice,MP_MODE, &tInitInfo, &MPCallBack,&PIPCallBack);
                vdec_start(pDecvDevice);
            }
            #endif  // end of #ifdef HDTV_SUPPORT
            #elif(VDEC27_PREVIEW_SOLUTION == VDEC27_PREVIEW_DE_SCALE)   // end of #if(VDEC27_PREVIEW_SOLUTION == VDEC27_PREVIEW_VE_SCALE)
            //If is DE Scale Operation, then adjust VPO to Full Srceen directly
            if(0 == l_hde_preview_method)
                vpo_zoom(pDisDevice, &l_hde_rectSrcFull, &l_hde_rectScrnFull);
            //If is VE Scale Operation, first set Video Decoder Output Mode(MP_MODE),
            //then adjust VPO to Full Srceen
            else if(1 == l_hde_preview_method)
            {
                //Set Video Decoder Output Mode (MP_MODE)
                tInitInfo.src_rect.uStartX = l_hde_rectSrcFull.uStartX;
                tInitInfo.src_rect.uStartY= l_hde_rectSrcFull.uStartY;
                tInitInfo.src_rect.uWidth = l_hde_rectSrcFull.uWidth;
                tInitInfo.src_rect.uHeight = l_hde_rectSrcFull.uHeight;
                tInitInfo.dst_rect.uStartX = l_hde_rectScrnFull.uStartX;
                tInitInfo.dst_rect.uStartY= l_hde_rectScrnFull.uStartY;
                tInitInfo.dst_rect.uWidth = l_hde_rectScrnFull.uWidth;
                tInitInfo.dst_rect.uHeight = l_hde_rectScrnFull.uHeight;
                vdec_set_output(pDecvDevice,MP_MODE, &tInitInfo, &MPCallBack,&PIPCallBack);

                //After Video Decoder Output Mode switch to MP_MODE, then adjust VPO
                vdec_io_control(pDecvDevice, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);
                if(VDEC27_STOPPED == vdec_info.uCurStatus)
                {
                    vpo_zoom(pDisDevice, &l_hde_rectSrcFull, &l_hde_rectScrnFull);
                }
                else
                {
                    waittime = 0;
                    while((vdec_info.output_mode != MP_MODE)&&(waittime<VE_MODE_SWITCH_TIME_29E))
                    {
                        nim_get_lock(nim, &ulock);
                        if(!ulock)
                            break;
                        osal_task_sleep(20);
                        waittime += 20;
                        vdec_io_control(pDecvDevice, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);
                    }
                    if(vdec_info.output_mode != MP_MODE)
                    {
                        vdec_stop(pDecvDevice,TRUE,FALSE);
                        vdec_set_output(pDecvDevice,MP_MODE, &tInitInfo, &MPCallBack,&PIPCallBack);
                        vdec_start(pDecvDevice);
                        vpo_zoom(pDisDevice, &l_hde_rectSrcFull, &l_hde_rectScrnFull);
                    }
                    else
                    {
                        vpo_zoom(pDisDevice, &l_hde_rectSrcFull, &l_hde_rectScrnFull);
                    }
                }

                vpo_ioctl(pDisDevice, VPO_IO_ADJUST_LM_IN_PREVIEW, 0);
            }
            #endif  // end of #elif(VDEC27_PREVIEW_SOLUTION == VDEC27_PREVIEW_DE_SCALE)
            LEAVE_CCHDE_API();
        }
    }
        l_hde_mode = mode;
}

/* Config the Position and Size of Preview Window, work on next hde_set_mode */
void hde_config_preview_window(UINT16 x, UINT16 y, UINT16 w, UINT16 h, BOOL IsPAL)
{
    l_hde_rectScrnPreview.uStartX = x;
    l_hde_rectScrnPreview.uStartY = y*(SCREEN_HEIGHT/(IsPAL?576:480));
    l_hde_rectScrnPreview.uWidth = w;
    l_hde_rectScrnPreview.uHeight = h*(SCREEN_HEIGHT/(IsPAL?576:480));
    l_hde_uIsPreviewResize = 1;
}

/* Get the Current View Mode of VPO Window*/
UINT32 hde_get_mode(void)
{
    return l_hde_mode;
}

/*
    Function:       Config the Method of Preview Display Output Scale Operation
    Argument:   method --   0 : DE(Display Engine) scale
                            1 : VE(Video Decoder Engine) scale with the independent pip mode
                            default value is 1
*/
void hde_config_preview_method(UINT8 method)
{
    l_hde_preview_method = method;
}

void hde_set_mode_HD_dec_restart(UINT32 mode)
{
    struct MPSource_CallBack MPCallBack;
    struct PIPSource_CallBack PIPCallBack;
    struct VDec_StatusInfo vdec_info;
    struct VDecPIPInfo tInitInfo;
    UINT32 waittime=0, wait_total_time = 0;
    struct vdec_device *pDecvDevice = (struct vdec_device *)get_selected_decoder();
    BOOL need_to_wait = FALSE;
    BOOL need_to_set_output = FALSE;
    UINT8 ulock = 0;
    struct nim_device *nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 0);
    struct vpo_device *pDisDevice = (struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS);
    #ifdef DUAL_VIDEO_OUTPUT
    struct vpo_device *pDisDevice_O = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 1);
    #endif
    struct vpo_io_get_info dis_info;

    switch(mode)
    {
    case VIEW_MODE_PREVIEW:
        if ((l_hde_mode != mode) || (l_hde_uIsPreviewResize == 1))
        {
            ENTER_CCHDE_API();

            /* Check Frame Size, If Input Frame is HD, Set VE to Dview Mode */
            vpo_ioctl(pDisDevice, VPO_IO_GET_INFO, (UINT32) &dis_info);
            #ifdef DUAL_VIDEO_OUTPUT
            //vpo_ioctl(pDisDevice_O, VPO_IO_GET_INFO, (UINT32) &dis_info);
            #endif  // end of #ifdef DUAL_VIDEO_OUTPUT

            vdec_stop(pDecvDevice,TRUE,FALSE);

            tInitInfo.adv_setting.init_mode = 1;
            tInitInfo.adv_setting.out_sys = dis_info.tvsys;
            tInitInfo.adv_setting.bprogressive = dis_info.bprogressive;
            #ifdef DUAL_VIDEO_OUTPUT
            vdec_set_output(pDecvDevice,DUAL_PREVIEW_MODE, &tInitInfo, &MPCallBack, &PIPCallBack);
            #else   // else of #ifdef DUAL_VIDEO_OUTPUT
            vdec_set_output(pDecvDevice,PREVIEW_MODE, &tInitInfo, &MPCallBack, &PIPCallBack);
            #endif  // end of #ifdef DUAL_VIDEO_OUTPUT
            osal_task_sleep(100);
            vdec_start(pDecvDevice);

//          libc_printf("vpz\n");
//          osal_task_sleep(100);
            vpo_zoom(pDisDevice, &l_hde_rectSrcFull_adjusted, &l_hde_rectScrnPreview);
            #ifdef DUAL_VIDEO_OUTPUT
            vpo_zoom(pDisDevice_O, &l_hde_rectSrcFull_adjusted, &l_hde_rectScrnPreview);
            #endif  // end of #ifdef DUAL_VIDEO_OUTPUT
        }
        l_hde_uIsPreviewResize = 0;
        LEAVE_CCHDE_API();
        break;
    case VIEW_MODE_MULTI:
        break;
    case VIEW_MODE_FULL:
    default:
        if (l_hde_mode != mode)
        {
            ENTER_CCHDE_API();
            // convert VE to MP mode
            vpo_ioctl(pDisDevice, VPO_IO_GET_INFO, (UINT32) &dis_info);
            #ifdef DUAL_VIDEO_OUTPUT
            //vpo_ioctl(pDisDevice_O, VPO_IO_GET_INFO, (UINT32) &dis_info);
            #endif  // end of #ifdef DUAL_VIDEO_OUTPUT
            vdec_stop(pDecvDevice,TRUE,FALSE);

            tInitInfo.adv_setting.init_mode = 0;
            tInitInfo.adv_setting.out_sys = dis_info.tvsys;
            tInitInfo.adv_setting.bprogressive = dis_info.bprogressive;
            #ifdef DUAL_VIDEO_OUTPUT
            vdec_set_output(pDecvDevice,DUAL_MODE, &tInitInfo, &MPCallBack, &PIPCallBack);
            #else   // else of #ifdef DUAL_VIDEO_OUTPUT
            vdec_set_output(pDecvDevice,MP_MODE, &tInitInfo, &MPCallBack, &PIPCallBack);
            #endif  // end of #ifdef DUAL_VIDEO_OUTPUT
            vdec_start(pDecvDevice);

            vpo_zoom(pDisDevice, &l_hde_rectSrcFull, &l_hde_rectScrnFull);
            #ifdef DUAL_VIDEO_OUTPUT
            vpo_zoom(pDisDevice_O, &l_hde_rectSrcFull, &l_hde_rectScrnFull);
            #endif  // end of #ifdef DUAL_VIDEO_OUTPUT

            LEAVE_CCHDE_API();
        }
    }
    l_hde_mode = mode;
}
