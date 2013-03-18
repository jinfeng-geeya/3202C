/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2002 Copyright (C)
 *
 *  File: decv.c
 *
 *  Description: This file define the functions for video decoder device management.
 *
 *  History:
 *      Date        Author         Version   Comment
 *      ====        ======         =======   =======
 *  1.  2003.04.11  David Wang     0.1.000   Initial
 ****************************************************************************/

#include <osal/osal.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <sys_config.h>
#include <mediatypes.h>
#include <hld/decv/vdec_driver.h>
#include <hld/dis/vpo.h>

VDecCBFunc pAVC_CB = NULL;

#ifdef USE_NEW_VDEC
__ATTRIBUTE_REUSE_
RET_CODE vdec_open(struct vdec_device *dev)
{
    if (dev == NULL)
        return RET_SUCCESS;
    return dev->open(dev);
}

RET_CODE vdec_close(struct vdec_device *dev)
{
    if (dev == NULL)
        return RET_SUCCESS;
    return dev->close(dev);
}
RET_CODE vdec_start(struct vdec_device *dev)
{
    if (dev == NULL)
        return RET_SUCCESS;
    return dev->start(dev);
}
RET_CODE vdec_stop(struct vdec_device *dev,BOOL bclosevp,BOOL bfillblack)
{
    if (dev == NULL)
        return RET_SUCCESS;
    return dev->stop(dev,bclosevp,bfillblack);
}
/*
RET_CODE vdec_pause(struct vdec_device *dev)
{
    if (dev == NULL)
        return RET_SUCCESS;
    return dev->pause(dev);
}

RET_CODE vdec_resume(struct vdec_device *dev)
{
    if (dev == NULL)
        return RET_SUCCESS;
    return dev->resume(dev);
}
*/
RET_CODE vdec_vbv_request(void *dev, UINT32 uSizeRequested, void ** ppVData, UINT32 * puSizeGot, struct control_block * ctrl_blk)
{
    struct vdec_device *decv_dev = (struct vdec_device *)dev;
    if (dev == NULL)
    {
        *ppVData = NULL;
        *puSizeGot = 0; //__MM_VBV_LEN;
        return RET_FAILURE;
    }
    return decv_dev->vbv_request(decv_dev,uSizeRequested,ppVData,puSizeGot,ctrl_blk);
}

void vdec_vbv_update(void *dev, UINT32 uDataSize)
{
    struct vdec_device *decv_dev = (struct vdec_device *)dev;
    if (dev == NULL)
        return;
    return decv_dev->vbv_update(decv_dev,uDataSize);
}

RET_CODE vdec_set_output(struct vdec_device *dev, enum VDecOutputMode eMode,struct VDecPIPInfo *pInitInfo,  struct MPSource_CallBack *pMPCallBack, struct PIPSource_CallBack *pPIPCallBack)
{
    if (dev == NULL)
    {
        return RET_SUCCESS;
    }
    if(is_cur_decoder_avc())
    {
        struct vdec_device * ret;
        ret = (struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0);
        if(ret != NULL)
            ret->set_output(ret,eMode,pInitInfo,pMPCallBack,pPIPCallBack);
    }
    else
    {
        struct vdec_device * ret;
        ret = (struct vdec_device *)dev_get_by_name("DECV_AVC_0");
        if(ret != NULL)
            ret->set_output(ret,eMode,pInitInfo,pMPCallBack,pPIPCallBack);
    }

    return dev->set_output(dev,eMode,pInitInfo,pMPCallBack,pPIPCallBack);
}

RET_CODE vdec_switch_pip(struct vdec_device *dev, struct Position *pPIPPos)
{
    if (dev == NULL)
        return RET_SUCCESS;
    return dev->switch_pip(dev,pPIPPos);
}

RET_CODE vdec_switch_pip_ext(struct vdec_device *dev, struct Rect *pPIPWin)
{
    if (dev == NULL)
        return RET_SUCCESS;
    return dev->switch_pip_ext(dev,pPIPWin);
}

RET_CODE vdec_sync_mode(struct vdec_device *dev, UINT8 uSyncMode,UINT8 uSyncLevel)
{
    if (dev == NULL)
        return RET_SUCCESS;
    return dev->sync_mode(dev,uSyncMode,uSyncLevel);
}

RET_CODE vdec_extrawin_store_last_pic(struct vdec_device *dev, struct Rect *pPIPWin)
{
    if (dev == NULL)
        return RET_SUCCESS;
    return dev->extrawin_store_last_pic(dev,pPIPWin);
}

#if (SYS_CHIP_MODULE == ALI_S3601 || SYS_CHIP_MODULE == ALI_S3602)
RET_CODE vdec_profile_level(struct vdec_device *dev, UINT8 uProfileLevel,VDEC_BEYOND_LEVEL cb_beyond_level)
{
    if (dev == NULL)
        return RET_SUCCESS;
    return dev->profile_level(dev,uProfileLevel,cb_beyond_level);
}
#endif
RET_CODE vdec_io_control(struct vdec_device *dev, UINT32 io_code, UINT32 param)
{
    if (dev == NULL)
        return RET_SUCCESS;
    return dev->ioctl(dev,io_code,param);
}

RET_CODE vdec_playmode(struct vdec_device *dev, enum VDecDirection direction, enum VDecSpeed speed)
{
    if (dev == NULL)
        return RET_SUCCESS;
    if(dev->playmode == NULL)
        return RET_SUCCESS;
    return dev->playmode(dev,direction,speed);
}

RET_CODE vdec_dvr_set_param(struct vdec_device *dev, struct VDec_DvrConfigParam param)
{
    if (dev == NULL)
        return RET_SUCCESS;
    if(dev->dvr_set_param == NULL)
        return RET_SUCCESS;
    return dev->dvr_set_param(dev,param);
}

RET_CODE vdec_dvr_pause(struct vdec_device *dev)
{
    if (dev == NULL)
        return RET_SUCCESS;
    if(dev->dvr_pause == NULL)
        return RET_SUCCESS;

    return dev->dvr_pause(dev);
}
RET_CODE vdec_dvr_resume(struct vdec_device *dev)
{
    if (dev == NULL)
        return RET_SUCCESS;
    if(dev->dvr_resume == NULL)
        return RET_SUCCESS;

    return dev->dvr_resume(dev);
}


RET_CODE vdec_step(struct vdec_device *dev)
{
    if (dev == NULL)
        return RET_SUCCESS;
    return dev->step(dev);
}

#else
RET_CODE vdec_open(struct vdec_device *dev)
{
    return VDec_Open(0, NULL);
}

RET_CODE vdec_close(struct vdec_device *dev)
{
    return VDec_Close(0);
}
RET_CODE vdec_start(struct vdec_device *dev)
{
    return VDec_Decode(0,   ALL_PIC,    FALSE,0,FALSE,FALSE,FALSE   );
 }
RET_CODE vdec_stop(struct vdec_device *dev)
{
    return VDec_Stop((HANDLE)0, VDEC_STOP_IM, 0, 0, FALSE);
}

RET_CODE vdec_vbv_request(void *dev, UINT32 uSizeRequested, void ** ppVData, UINT32 * puSizeGot, struct control_block * ctrl_blk)
{
    return decv_request_write(0,  uSizeRequested, ppVData, puSizeGot, ctrl_blk);
}

void vdec_vbv_update(void *dev, UINT32 uDataSize)
{
    return decv_update_write(0, uDataSize);
}

RET_CODE vdec_set_output(struct vdec_device *dev,enum VDecOutputMode eMode,struct VDecPIPInfo *pInitInfo,   struct MPSource_CallBack *pMPCallBack, struct PIPSource_CallBack *pPIPCallBack)
{
    return VDec_SetOutput(eMode, pInitInfo, pMPCallBack, pPIPCallBack, 0,NULL);
}

RET_CODE vdec_switch_pip(struct vdec_device *dev, struct Position *pPIPPos)
{
    return VDec_SwitchPIPWin(pPIPPos);
}

RET_CODE vdec_sync_mode(struct vdec_device *dev, UINT8 uSyncMode,UINT8 uSyncLevel)
{
    VDec_Set_Sync_Mode(0, uSyncMode);
    VDec_Set_Sync_Level(0, uSyncLevel);
    return RET_SUCCESS;
}


RET_CODE vdec_io_control(struct vdec_device *dev, UINT32 io_code, UINT32 param)
{
    switch(io_code)
    {
    case VDEC_IO_FILL_FRM:
        {
            VDec_FillDisplayFrm(0, (struct YCbCrColor *)param);
            return RET_SUCCESS;
        }
    case VDEC_IO_REG_CALLBACK:
        {
            struct vdec_io_reg_callback_para *ppara = (struct vdec_io_reg_callback_para *)param;
            return VDec_RegisterCB(0,ppara->eCBType,ppara->pCB);

        }
    case VDEC_IO_GET_STATUS:
        {
            VDec_GetStatusInfo(0, (struct VDec_StatusInfo *)(param));
            return RET_SUCCESS;
        }
    case VDEC_IO_GET_MODE:
        {
            return VDec_GetSourceMode(0,(enum TVSystem *)(param));
        }
    case VDEC_IO_GET_FRM:
        {
            struct vdec_io_get_frm_para *ppara = (struct vdec_io_get_frm_para *)(param);
            VDec_Get_Frm(ppara->ufrm_mode,&(ppara->tFrmInfo));
            return RET_SUCCESS;
        }
    case VDEC_IO_WHY_IDLE:
        {
            VDec_Dbg_WhyUIdle();
            return RET_SUCCESS;
        }
    case VDEC_IO_SMLBUF:
    case VDEC_IO_GET_FREEBUF:
        {
            DEBUGPRINTF("This function is not supported on old vdec driver\n");
            return RET_FAILURE;
        }
    case VDEC_IO_FILLRECT_INBG:
        {
            struct Rect  *fill_rect = (struct Rect *)(param);
            struct Position rect_position;
            rect_position.uX = fill_rect->uStartX;
            rect_position.uY = fill_rect->uStartY;
            SwScale_FillBlack(&rect_position);
            return RET_SUCCESS;
        }
    }
}
#endif

#if ((defined HDTV_SUPPORT) || ((SYS_CHIP_MODULE == ALI_S3602) && (SYS_CPU_MODULE == CPU_M6303) && (SYS_PROJECT_FE == PROJECT_FE_DVBT)))//for 3105
extern struct  vdec_control *g_top_control;
extern void vdec_s3601_isr(UINT32 para);
/*
extern void avc_isr(UINT32 para);
extern enum Output_Frame_Ret_Code vdec_avc_de_release(void *pdev, void *fs);
extern enum Output_Frame_Ret_Code vdec_avc_de_request(void *pdev,struct Display_Info *pDisplay_info,struct Request_Info *pRequest_Info);
extern enum Output_Frame_Ret_Code vdec_s3601_release(void *pdev, void *fs);
extern enum Output_Frame_Ret_Code vdec_avc_s3601_request(void *pdev,struct Display_Info *pDisplay_info,struct Request_Info *pRequest_Info);
*/
static int status = 0;
#ifdef AVC_FIRST_HEAD_PARSE_SUPPORT
extern UINT32 callback_avc_first_head_parse(UINT32 uParam1, UINT32 uParam2);
#endif
void h264_decoder_select(int select, BOOL in_preivew)
{
    struct VDecPIPInfo vInitInfo;
    struct MPSource_CallBack vMPCallBack;
    struct PIPSource_CallBack vPIPCallBack;
    struct vp_win_config_para win_para;
    struct vdec_io_reg_callback_para tpara;
    struct vdec_device*  g_decv_dev = (struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0);
    struct vdec_device*  g_avc_dev = (struct vdec_device *)dev_get_by_name("DECV_AVC_0");
    struct vpo_device*   g_vpo_dev = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0);
    struct vpo_device*   g_sd_vpo_dev = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 1);
    struct vpo_io_get_info dis_info;
    struct Rect src_rect, dst_rect;
    int ret;
    UINT32 vdec_irq_num;
#ifdef DUAL_VIDEO_OUTPUT
    enum VDecOutputMode preview_mode = DUAL_PREVIEW_MODE;
    enum VDecOutputMode full_mode = DUAL_MODE;
#else
    enum VDecOutputMode preview_mode = PREVIEW_MODE;
    enum VDecOutputMode full_mode = MP_MODE;
#endif
    enum VDecOutputMode emode;
    if(in_preivew)
    {
        emode = preview_mode;
        vInitInfo.adv_setting.init_mode = 1;
    }
    else
    {
        emode = full_mode;
        vInitInfo.adv_setting.init_mode = 0;
    }

    if(sys_ic_is_M3101())
        vdec_irq_num = 20;
    else
        vdec_irq_num = 9;

    MEMSET(&vInitInfo,0,sizeof(struct VDecPIPInfo));

    vpo_ioctl(g_vpo_dev, VPO_IO_GET_INFO, (UINT32) &dis_info);

    vInitInfo.adv_setting.out_sys = dis_info.tvsys;
    vInitInfo.adv_setting.bprogressive = dis_info.bprogressive;

    if(select && !status)
    {
        vdec_stop(g_decv_dev, FALSE, FALSE);
        vdec_stop(g_avc_dev, FALSE, FALSE);

        if(OSAL_E_OK!=osal_interrupt_unregister_lsr(vdec_irq_num, vdec_s3601_isr))
            ASSERT(0);
        vdec_close(g_decv_dev);
        if(RET_SUCCESS!=vdec_open(g_avc_dev))
            ASSERT(0);

        tpara.eCBType = VDEC_CB_OUTPUT_MODE_CHECK;
        tpara.pCB = pAVC_CB;
        vdec_io_control(g_avc_dev, VDEC_IO_REG_CALLBACK, (UINT32)(&tpara));

#ifdef AVC_FIRST_HEAD_PARSE_SUPPORT
        tpara.eCBType = VDEC_CB_FIRST_HEAD_PARSED;
        tpara.pCB = callback_avc_first_head_parse;
        vdec_io_control(g_avc_dev, VDEC_IO_REG_CALLBACK, (UINT32)(&tpara));
#endif
        vdec_set_output(g_avc_dev, emode, &vInitInfo, &vMPCallBack, &vPIPCallBack);

        vpo_win_mode(g_vpo_dev, VPO_MAINWIN, &vMPCallBack, &vPIPCallBack);
        status = 1;

#ifdef DUAL_VIDEO_OUTPUT
        win_para.source_number = 1;
        win_para.control_source_index = 0;
        win_para.source_info[0].attach_source_index = 1;
        win_para.source_info[0].src_callback.request_callback = (T_Request)(vMPCallBack.RequestCallback);
        win_para.source_info[0].src_callback.release_callback = (T_Release)(vMPCallBack.ReleaseCallback);
        win_para.source_info[0].src_callback.vblanking_callback= vMPCallBack.vblanking_callback;
        win_para.source_info[0].src_module_devide_handle = g_avc_dev;
        win_para.source_info[0].src_path_index = 1;
        win_para.window_number = 1;
        win_para.window_parameter[0].source_index = 0;
        win_para.window_parameter[0].display_layer = VPO_LAYER_MAIN;
  /*      win_para.window_parameter[0].rect.src_rect.uStartX = 0;
        win_para.window_parameter[0].rect.src_rect.uStartY = 0;
        win_para.window_parameter[0].rect.src_rect.uWidth = PICTURE_WIDTH;
        win_para.window_parameter[0].rect.src_rect.uHeight = PICTURE_HEIGHT;	
        win_para.window_parameter[0].rect.dst_rect.uStartX = 0;
        win_para.window_parameter[0].rect.dst_rect.uStartY = 0;
        win_para.window_parameter[0].rect.dst_rect.uWidth = SCREEN_WIDTH;
        win_para.window_parameter[0].rect.dst_rect.uHeight = SCREEN_HEIGHT;*/
        vpo_config_source_window(g_sd_vpo_dev, &win_para);
#endif
    }
    if(!select)
    {
        if(status == 1)
        {
            vdec_stop(g_decv_dev, FALSE, FALSE);
            vdec_stop(g_avc_dev, FALSE, FALSE);

            if(RET_SUCCESS!=vdec_close(g_avc_dev))
                ASSERT(0);
            if (sys_ic_get_chip_id() == ALI_S3602)
                *(unsigned long *)0xb8000078 = (*(unsigned long *)0xb8000078)&(~0x00002000);

            if(OSAL_E_OK!=osal_interrupt_register_lsr(vdec_irq_num, vdec_s3601_isr, (UINT32)(g_top_control)))
                ASSERT(0);
        }

        vdec_set_output(g_decv_dev, emode, &vInitInfo, &vMPCallBack, &vPIPCallBack);

        vpo_win_mode(g_vpo_dev, VPO_MAINWIN, &vMPCallBack, &vPIPCallBack);
        status = 0;
#ifdef DUAL_VIDEO_OUTPUT
        win_para.source_number = 1;
        win_para.control_source_index = 0;
        win_para.source_info[0].attach_source_index = 1;
        win_para.source_info[0].src_callback.request_callback = (T_Request)(vMPCallBack.RequestCallback);
        win_para.source_info[0].src_callback.release_callback = (T_Release)(vMPCallBack.ReleaseCallback);
        win_para.source_info[0].src_callback.vblanking_callback= vMPCallBack.vblanking_callback;
        win_para.source_info[0].src_module_devide_handle = g_decv_dev;
        win_para.source_info[0].src_path_index = 1;
        win_para.window_number = 1;
        win_para.window_parameter[0].source_index = 0;
        win_para.window_parameter[0].display_layer = VPO_LAYER_MAIN;
/*        win_para.window_parameter[0].rect.src_rect.uStartX = 0;
        win_para.window_parameter[0].rect.src_rect.uStartY = 0;
        win_para.window_parameter[0].rect.src_rect.uWidth = PICTURE_WIDTH;
        win_para.window_parameter[0].rect.src_rect.uHeight = PICTURE_HEIGHT;	
        win_para.window_parameter[0].rect.dst_rect.uStartX = 0;
        win_para.window_parameter[0].rect.dst_rect.uStartY = 0;
        win_para.window_parameter[0].rect.dst_rect.uWidth = SCREEN_WIDTH;
        win_para.window_parameter[0].rect.dst_rect.uHeight = SCREEN_HEIGHT;*/
        vpo_config_source_window(g_sd_vpo_dev, &win_para);
#endif
    }
}

struct vdec_device * get_selected_decoder(void)
{
    struct vdec_device * ret;
    if(status == 0){
         ret = (struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0);
    }else{
        ret = (struct vdec_device *)dev_get_by_name("DECV_AVC_0");
    };

    return ret;
}

BOOL is_cur_decoder_avc(void)
{
    return ((status==1)?TRUE:FALSE);
}

void set_avc_output_mode_check_cb(VDecCBFunc pCB)
{
    pAVC_CB = pCB;
}
#else
void h264_decoder_select(int select, BOOL in_preivew)
{

}

struct vdec_device * get_selected_decoder(void)
{
    struct vdec_device * ret;
    ret = (struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0);

    return ret;
}

BOOL is_cur_decoder_avc(void)
{
    return FALSE;
}

void set_avc_output_mode_check_cb(VDecCBFunc pCB)
{
    pAVC_CB = pCB;
}

#endif
