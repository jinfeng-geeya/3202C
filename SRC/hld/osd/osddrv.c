#include <sys_config.h>
#include <basic_types.h>
#include <Mediatypes.h>	
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <osal/osal.h>

#include <hld/hld_dev.h>
#include <hld/osd/osddrv.h>


#ifdef GE_SIMULATE_OSD

#include <hld/ge/ge.h>

#define GMA_SW_LAYER_ID ((UINT32)dev->priv)
#define GE_SIMU_MUTEX_LOCK()  osal_mutex_lock(dev->sema_opert_osd, OSAL_WAIT_FOREVER_TIME)
#define GE_SIMU_MUTEX_UNLOCK()  osal_mutex_unlock(dev->sema_opert_osd)
#define m_cmd_list      m_osd_cmd_list[GMA_SW_LAYER_ID]

static struct ge_device *m_osddrv_ge_dev;
static ge_cmd_list_hdl  m_osd_cmd_list[2];

void osd_dev_api_attach(struct ge_device *ge_dev, UINT32 osd_layer_id)
{
    struct osd_device *dev;
    const char *name;

    if (ge_dev)
        m_osddrv_ge_dev = ge_dev;

    if (osd_layer_id >= 3)
        return;

    if (osd_layer_id == 0)
        name = "OSD_DEV_0";
    else if (osd_layer_id == 1)
        name = "OSD_DEV_1";
    else
        name = "OSD_DEV_2";

    dev = (struct osd_device *)dev_alloc((INT8 *)name,HLD_DEV_TYPE_OSD,sizeof(struct osd_device));
    if (dev == NULL)
    {
        return ;
    }

    dev->priv = (void *)osd_layer_id;

    dev->next = NULL;
    dev->flags = 0;
    dev->sema_opert_osd = osal_mutex_create();
    if(OSAL_INVALID_ID == dev->sema_opert_osd)
    {
		PRINTF("OSD Can not create mutex!!!\n");
		ASSERT(0);
    }


    /* Function point init */
#if 0
    dev->open = OSDDrv_Open;
    dev->close = OSDDrv_Close;
    dev->ioctl = OSDDrv_IoCtl;
    dev->get_para = OSDDrv_GetPara;
    dev->show_onoff = OSDDrv_ShowOnOff;   
    dev->region_show = OSDDrv_RegionShow;   
    dev->set_pallette = OSDDrv_SetPallette;
    dev->get_pallette = OSDDrv_GetPallette;
    dev->modify_pallette = OSDDrv_ModifyPallette;
    dev->create_region = OSDDrv_CreateRegion;
    dev->delete_region = OSDDrv_DeleteRegion;
    dev->set_region_pos = OSDDrv_SetRegionPos;
    dev->get_region_pos = OSDDrv_GetRegionPos;

    dev->region_write = OSDDrv_RegionWrite;
    dev->region_read = OSDDrv_RegionRead;
    dev->region_fill = OSDDrv_RegionFill;
    dev->region_write2 = OSDDrv_RegionWrite2;

    dev->draw_hor_line = OSDDrv_DrawHorLine;

    dev->scale = OSDDrv_Scale;

    dev->set_clip = OSDDrv_SetClip;
    dev->clear_clip = OSDDrv_ClearClip;
    dev->get_region_addr = OSDDrv_GetRegionAddr;
#endif

    /* Add this device to queue */
    if (dev_register(dev) != RET_SUCCESS)
    {
        dev_free(dev);
        return ;
    }

    return ;
}

static enum GE_PIXEL_FORMAT osddrv_color_mode_to_ge(UINT8 bColorMode)
{
    enum GE_PIXEL_FORMAT color_format = GE_PF_CLUT8;

    switch (bColorMode)
    {
        case OSD_256_COLOR:
            color_format = GE_PF_CLUT8;
            break;
        case OSD_HD_ARGB1555:
 	    color_format	 = GE_PF_ARGB1555;
            break;
        case OSD_HD_ARGB4444:
            color_format = GE_PF_ARGB4444;
            break;
        case OSD_HD_AYCbCr8888:
        case OSD_HD_ARGB8888:
            color_format = GE_PF_ARGB8888;
            break;
        default:
            break;
    }

    return color_format;
}

static UINT32 osddrv_get_pitch(UINT8 bColorMode, UINT16 uWidth)
{
    switch (bColorMode)
    {
        case OSD_HD_ARGB1555:
        case OSD_HD_ARGB4444:
            uWidth <<= 1;
            break;
        case OSD_HD_AYCbCr8888:
        case OSD_HD_ARGB8888:
            uWidth <<= 2;
            break;
        case OSD_256_COLOR:
        default:
            break;
    }

    return uWidth;
}

static UINT32 osddrv_get_pitch_by_ge_format(UINT8 ge_pixel_fmt, UINT16 uWidth)
{
    switch (ge_pixel_fmt)
    {
        case GE_PF_ARGB1555:
        case GE_PF_ARGB4444:
            uWidth <<= 1;
            break;
        case GE_PF_RGB888:
        case GE_PF_ARGB8888:
            uWidth <<= 2;
            break;
        case GE_PF_CLUT8:
        case GE_PF_CK_CLUT8:
        default:
            break;
    }

    return uWidth;
}

#endif

RET_CODE OSDDrv_Open(HANDLE hDev,struct OSDPara*pOpenPara)
{
	INT32 result = RET_SUCCESS;
	struct osd_device *dev = (struct osd_device *)hDev;

#ifdef GE_SIMULATE_OSD
    ge_gma_region_t region_param;

    ge_open(m_osddrv_ge_dev);

    if (m_cmd_list == 0)
        m_cmd_list = ge_cmd_list_create(m_osddrv_ge_dev, 2);

    if (pOpenPara)
    {
        ge_gma_get_region_info(m_osddrv_ge_dev, GMA_SW_LAYER_ID, 0, &region_param);

        region_param.galpha_enable = pOpenPara->uGAlphaEnable;
        region_param.global_alpha = pOpenPara->uGAlpha;
        region_param.pallette_sel = pOpenPara->uPalletteSel;
        region_param.color_format = osddrv_color_mode_to_ge(pOpenPara->eMode);

        ge_gma_move_region(m_osddrv_ge_dev, GMA_SW_LAYER_ID, 0, &region_param);
    }

    return RET_SUCCESS;
#endif

    if (dev == NULL)
    {
        return RET_SUCCESS;
    }

	/* If openned already, exit */
	if(dev->flags & HLD_DEV_STATS_UP)
	{
		PRINTF("osddrv_open: warning - device %s openned already!\n", dev->name);
		return RET_SUCCESS;
	}

	/* Open this device */
	if (dev->open)
	{
		result = dev->open(dev,pOpenPara);
	}
	
	/* Setup init work mode */
	if (result == RET_SUCCESS)
	{
		dev->flags |= (HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);
		
	}
	
	return result;
}
RET_CODE OSDDrv_Close(HANDLE hDev)
{
	INT32 result = RET_SUCCESS;
	struct osd_device *dev = (struct osd_device *)hDev;

#ifdef GE_SIMULATE_OSD
    ge_cmd_list_destroy(m_osddrv_ge_dev, m_cmd_list);
    ge_gma_close(m_osddrv_ge_dev, GMA_SW_LAYER_ID);
    m_cmd_list = 0;

    return RET_SUCCESS;
#endif
	
    if (dev == NULL)
        return RET_SUCCESS;
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		PRINTF("osddrv_close: warning - device %s closed already!\n", dev->name);
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

__ATTRIBUTE_RAM_
RET_CODE OSDDrv_IoCtl(HANDLE hDev,UINT32 dwCmd,UINT32 dwParam)
{
	struct osd_device *dev = (struct osd_device *)hDev;

#ifdef GE_SIMULATE_OSD
    switch (dwCmd)
    {
        case OSD_IO_SET_GLOBAL_ALPHA:
            dwCmd = GE_IO_SET_GLOBAL_ALPHA;
            break;
        case OSD_IO_SET_TRANS_COLOR:
            dwCmd = GE_IO_SET_TRANS_COLOR;
            break;
        case OSD_IO_ENABLE_ANTIFLICK:
            dwCmd = GE_IO_ENABLE_ANTIFLICK;
            dwParam = TRUE;
            break;
        case OSD_IO_DISABLE_ANTIFLICK:
            dwCmd = GE_IO_ENABLE_ANTIFLICK;
            dwParam = FALSE;
            break;
        case OSD_IO_SET_AUTO_CLEAR_REGION:
            dwCmd = GE_IO_SET_AUTO_CLEAR_REGION;
            break;
        case OSD_IO_GET_ON_OFF:
            dwCmd = GE_IO_GET_LAYER_ON_OFF;
            break;
        case OSD_IO_CREATE_REGION:
        case OSD_IO_MOVE_REGION:
        {
            pcosd_region_param posd_rgn_info = (pcosd_region_param)dwParam;
            ge_gma_region_t ge_rgn_info;
            RET_CODE ret;
            if (posd_rgn_info == NULL)
                return RET_FAILURE;
            ge_rgn_info.color_format = posd_rgn_info->color_format;
            ge_rgn_info.galpha_enable = posd_rgn_info->galpha_enable;
            ge_rgn_info.global_alpha = posd_rgn_info->global_alpha;
            ge_rgn_info.pallette_sel = posd_rgn_info->pallette_sel;
            ge_rgn_info.region_x = posd_rgn_info->region_x;
            ge_rgn_info.region_y = posd_rgn_info->region_y;
            ge_rgn_info.region_w = posd_rgn_info->region_w;
            ge_rgn_info.region_h = posd_rgn_info->region_h;
            ge_rgn_info.bitmap_addr = posd_rgn_info->bitmap_addr;
            ge_rgn_info.pixel_pitch = posd_rgn_info->pixel_pitch;
            ge_rgn_info.bitmap_x = posd_rgn_info->bitmap_x;
            ge_rgn_info.bitmap_y = posd_rgn_info->bitmap_y;
            ge_rgn_info.bitmap_w = posd_rgn_info->bitmap_w;
            ge_rgn_info.bitmap_h = posd_rgn_info->bitmap_h;
            if (dwCmd == OSD_IO_CREATE_REGION)
                ret = ge_gma_create_region(m_osddrv_ge_dev, GMA_SW_LAYER_ID, posd_rgn_info->region_id, &ge_rgn_info);
            else
                ret = ge_gma_move_region(m_osddrv_ge_dev, GMA_SW_LAYER_ID, posd_rgn_info->region_id, &ge_rgn_info);
            return ret;
        }
        case OSD_IO_GET_REGION_INFO:
        {
            posd_region_param posd_rgn_info = (posd_region_param)dwParam;
            ge_gma_region_t ge_rgn_info;
            RET_CODE ret;
            if (posd_rgn_info == NULL)
                return RET_FAILURE;
            ret = ge_gma_get_region_info(m_osddrv_ge_dev, GMA_SW_LAYER_ID, posd_rgn_info->region_id, &ge_rgn_info);
            posd_rgn_info->color_format = ge_rgn_info.color_format;
            posd_rgn_info->galpha_enable = ge_rgn_info.galpha_enable;
            posd_rgn_info->global_alpha = ge_rgn_info.global_alpha;
            posd_rgn_info->pallette_sel = ge_rgn_info.pallette_sel;
            posd_rgn_info->region_x = ge_rgn_info.region_x;
            posd_rgn_info->region_y = ge_rgn_info.region_y;
            posd_rgn_info->region_w = ge_rgn_info.region_w;
            posd_rgn_info->region_h = ge_rgn_info.region_h;
            posd_rgn_info->bitmap_addr = ge_rgn_info.bitmap_addr;
            posd_rgn_info->pixel_pitch = ge_rgn_info.pixel_pitch;
            posd_rgn_info->bitmap_x = ge_rgn_info.bitmap_x;
            posd_rgn_info->bitmap_y = ge_rgn_info.bitmap_y;
            posd_rgn_info->bitmap_w = ge_rgn_info.bitmap_w;
            posd_rgn_info->bitmap_h = ge_rgn_info.bitmap_h;
            return ret;
        }
        default:
            return RET_SUCCESS;
            break;
    }
    return ge_io_ctrl_ext(m_osddrv_ge_dev, GMA_SW_LAYER_ID, dwCmd, dwParam);
#endif

    if (dev == NULL)
        return RET_SUCCESS;

	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->ioctl)
	{
		return dev->ioctl(dev, dwCmd, dwParam);
	}
	
	return !RET_SUCCESS;
}
RET_CODE OSDDrv_GetPara(HANDLE hDev,struct OSDPara* ptPara)
{
	struct osd_device *dev = (struct osd_device *)hDev;
	
    if (dev == NULL)
        return RET_SUCCESS;

	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->get_para)
	{
		return dev->get_para(dev, ptPara);
	}
	
	return !RET_SUCCESS;
}
RET_CODE OSDDrv_ShowOnOff(HANDLE hDev,UINT8 uOnOff)
{
	struct osd_device *dev = (struct osd_device *)hDev;

#ifdef GE_SIMULATE_OSD
    GE_SIMU_MUTEX_LOCK();	
    RET_CODE ret = ge_gma_show_onoff(m_osddrv_ge_dev, GMA_SW_LAYER_ID, uOnOff);
    GE_SIMU_MUTEX_UNLOCK();
    return ret;	
#endif

    if (dev == NULL)
        return RET_SUCCESS;

	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->show_onoff)
	{
		return dev->show_onoff(dev, uOnOff);
	}
	
	return !RET_SUCCESS;
}
RET_CODE OSDDrv_SetPallette(HANDLE hDev,UINT8 *pPallette,UINT16 wColorN,UINT8 bType)
{
	struct osd_device *dev = (struct osd_device *)hDev;

#ifdef GE_SIMULATE_OSD

    static const ge_pal_attr_t m_rgb_pal_attr =
    {GE_PAL_RGB, GE_RGB_ORDER_ARGB, GE_ALPHA_RANGE_0_255, GE_ALPHA_POLARITY_0};

    GE_SIMU_MUTEX_LOCK();
    RET_CODE ret = ge_gma_set_pallette(m_osddrv_ge_dev, GMA_SW_LAYER_ID, pPallette, wColorN, (bType == OSDDRV_RGB) ? &m_rgb_pal_attr : NULL);
    GE_SIMU_MUTEX_UNLOCK();
    return ret;
#endif

    if (dev == NULL)
        return RET_SUCCESS;

	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->set_pallette)
	{
		return dev->set_pallette(dev, pPallette,wColorN,bType);
	}
	
	return !RET_SUCCESS;
}
RET_CODE OSDDrv_GetPallette(HANDLE hDev,UINT8 *pPallette,UINT16 wColorN,UINT8 bType)
{
	struct osd_device *dev = (struct osd_device *)hDev;

#ifdef GE_SIMULATE_OSD
    GE_SIMU_MUTEX_LOCK();	
    RET_CODE ret = ge_gma_get_pallette(m_osddrv_ge_dev, GMA_SW_LAYER_ID, pPallette, wColorN, NULL);
    GE_SIMU_MUTEX_UNLOCK();
    return ret;	
#endif

    if (dev == NULL)
        return RET_SUCCESS;

	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->get_pallette)
	{
		return dev->get_pallette(dev, pPallette,wColorN,bType);
	}
	
	return !RET_SUCCESS;
}
RET_CODE OSDDrv_ModifyPallette(HANDLE hDev,UINT8 uIndex,UINT8 uY,UINT8 uCb,UINT8 uCr,UINT8 uK)
{
	struct osd_device *dev = (struct osd_device *)hDev;

#ifdef GE_SIMULATE_OSD
    GE_SIMU_MUTEX_LOCK();	
    RET_CODE ret = ge_gma_modify_pallette(m_osddrv_ge_dev, GMA_SW_LAYER_ID, uIndex, uK, uY, uCb, uCr, NULL);
    GE_SIMU_MUTEX_UNLOCK();
    return ret;	
#endif

    if (dev == NULL)
        return RET_SUCCESS;

	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->modify_pallette)
	{
		return dev->modify_pallette(dev, uIndex,uY,uCb,uCr,uK);
	}
	
	return !RET_SUCCESS;
}

RET_CODE OSDDrv_CreateRegion(HANDLE hDev,UINT8 uRegionId,struct OSDRect* rect,struct OSDPara*pOpenPara)
{
	struct osd_device *dev = (struct osd_device *)hDev;

#ifdef GE_SIMULATE_OSD
    ge_gma_region_t region_param;
    RET_CODE ret;
    GE_SIMU_MUTEX_LOCK();
    if (pOpenPara == NULL && rect == NULL)
    {
        ret = ge_gma_create_region(m_osddrv_ge_dev, GMA_SW_LAYER_ID, uRegionId, NULL);
        GE_SIMU_MUTEX_UNLOCK();
        return ret;		
    }
    if (pOpenPara == NULL)
    {
        ge_gma_get_region_info(m_osddrv_ge_dev, GMA_SW_LAYER_ID, uRegionId, &region_param);
        region_param.region_x = rect->uLeft;
        region_param.region_y = rect->uTop;
        region_param.region_w = rect->uWidth;
        region_param.region_h = rect->uHeight;

        region_param.bitmap_addr = 0;
        region_param.pixel_pitch = 0;
    }
    else
    {
        region_param.region_x = rect->uLeft;
        region_param.region_y = rect->uTop;
        region_param.region_w = rect->uWidth;
        region_param.region_h = rect->uHeight;

        region_param.galpha_enable = pOpenPara->uGAlphaEnable;
        region_param.global_alpha = pOpenPara->uGAlpha;
        region_param.pallette_sel = pOpenPara->uPalletteSel;
        region_param.bitmap_addr = 0;
        region_param.pixel_pitch = 0;
        region_param.color_format = osddrv_color_mode_to_ge(pOpenPara->eMode);
    }

    ret =  ge_gma_create_region(m_osddrv_ge_dev, GMA_SW_LAYER_ID, uRegionId, &region_param);
    GE_SIMU_MUTEX_UNLOCK();
    return ret;		
	
#endif

    if (dev == NULL)
        return RET_SUCCESS;

	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->create_region)
	{
		return dev->create_region(dev, uRegionId,rect,pOpenPara);
	}
	
	return !RET_SUCCESS;
}
RET_CODE OSDDrv_DeleteRegion(HANDLE hDev,UINT8 uRegionId)
{
	struct osd_device *dev = (struct osd_device *)hDev;

#ifdef GE_SIMULATE_OSD
    GE_SIMU_MUTEX_LOCK();	
    RET_CODE ret = ge_gma_delete_region(m_osddrv_ge_dev, GMA_SW_LAYER_ID, uRegionId);
    GE_SIMU_MUTEX_UNLOCK();		
    return ret;	
#endif

    if (dev == NULL)
        return RET_SUCCESS;

	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->delete_region)
	{
		return dev->delete_region(dev, uRegionId);
	}
	
	return !RET_SUCCESS;
}

RET_CODE OSDDrv_SetRegionPos(HANDLE hDev,UINT8 uRegionId,struct OSDRect* rect)
{
	struct osd_device *dev = (struct osd_device *)hDev;

#ifdef GE_SIMULATE_OSD
    ge_gma_region_t region_param;
    GE_SIMU_MUTEX_LOCK();		

    ge_gma_get_region_info(m_osddrv_ge_dev, GMA_SW_LAYER_ID, uRegionId, &region_param);

    region_param.region_x = rect->uLeft;
    region_param.region_y = rect->uTop;
    region_param.region_w = rect->uWidth;
    region_param.region_h = rect->uHeight; 

//    region_param.bitmap_addr = 0;

    RET_CODE ret = ge_gma_move_region(m_osddrv_ge_dev, GMA_SW_LAYER_ID, uRegionId, &region_param);
    GE_SIMU_MUTEX_UNLOCK();	
    return ret;

#endif

    if (dev == NULL)
        return RET_SUCCESS;

	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->set_region_pos)
	{
		return dev->set_region_pos(dev, uRegionId,rect);
	}
	
	return !RET_SUCCESS;
}
RET_CODE OSDDrv_GetRegionPos(HANDLE hDev,UINT8 uRegionId,struct OSDRect* rect)
{
	struct osd_device *dev = (struct osd_device *)hDev;

#ifdef GE_SIMULATE_OSD
    ge_gma_region_t region_param;
    GE_SIMU_MUTEX_LOCK();		

    RET_CODE ret = ge_gma_get_region_info(m_osddrv_ge_dev, GMA_SW_LAYER_ID, uRegionId, &region_param);

    rect->uLeft = region_param.region_x;
    rect->uTop = region_param.region_y;
    rect->uWidth = region_param.region_w;
    rect->uHeight = region_param.region_h;
    GE_SIMU_MUTEX_UNLOCK();	
    return ret;
#endif

    if (dev == NULL)
        return RET_SUCCESS;

	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->get_region_pos)
	{
		 return dev->get_region_pos(dev, uRegionId,rect);
	}
	
	return !RET_SUCCESS;
}
RET_CODE OSDDrv_RegionShow(HANDLE hDev,UINT8 uRegionId,BOOL bOn)
{
	struct osd_device *dev = (struct osd_device *)hDev;

#ifdef GE_SIMULATE_OSD
    GE_SIMU_MUTEX_LOCK();	
    RET_CODE ret = ge_gma_show_region(m_osddrv_ge_dev, GMA_SW_LAYER_ID, uRegionId, bOn);
    GE_SIMU_MUTEX_UNLOCK();		
    return ret;
#endif

    if (dev == NULL)
        return RET_SUCCESS;

	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->region_show)
	{
		return dev->region_show(dev,uRegionId,bOn);
	}
	
	return !RET_SUCCESS;
}
RET_CODE OSDDrv_RegionWrite(HANDLE hDev,UINT8 uRegionId,VSCR *pVscr,struct OSDRect *rect)
{
	struct osd_device *dev = (struct osd_device *)hDev;

#ifdef GE_SIMULATE_OSD
    RET_CODE ret;
    GE_SIMU_MUTEX_LOCK();		

    ge_cmd_list_new(m_osddrv_ge_dev, m_cmd_list, GE_COMPILE_AND_EXECUTE);
    ret = ge_gma_set_region_to_cmd_list(m_osddrv_ge_dev, GMA_SW_LAYER_ID, uRegionId, m_cmd_list);
    if (ret != RET_SUCCESS)
    {
	 GE_SIMU_MUTEX_UNLOCK();				
        return ret;
    }
    UINT32 byte_pitch = osddrv_get_pitch(pVscr->bColorMode, pVscr->vR.uWidth);
    UINT32 ptn_x, ptn_y;

    ptn_x = rect->uLeft - pVscr->vR.uLeft;
    ptn_y = rect->uTop - pVscr->vR.uTop;
    osal_cache_flush(pVscr->lpbScr + byte_pitch * ptn_y, byte_pitch * rect->uHeight);

    ge_cmd_list_hdl cmd_list = m_cmd_list;
    ge_base_addr_t base_addr;

    base_addr.color_format = (enum GE_PIXEL_FORMAT)osddrv_color_mode_to_ge(pVscr->bColorMode);
    base_addr.base_address = (UINT32)pVscr->lpbScr;
    base_addr.data_decoder = GE_DECODER_DISABLE;
    base_addr.pixel_pitch = pVscr->vR.uWidth;
    base_addr.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;

    UINT32 cmd_hdl = ge_cmd_begin(m_osddrv_ge_dev, cmd_list, GE_DRAW_BITMAP);
    ge_set_base_addr(m_osddrv_ge_dev, cmd_hdl, GE_PTN, &base_addr);
    ge_set_wh(m_osddrv_ge_dev, cmd_hdl, GE_DST_PTN, rect->uWidth, rect->uHeight);
    ge_set_xy(m_osddrv_ge_dev, cmd_hdl, GE_DST, rect->uLeft, rect->uTop);
    ge_set_xy(m_osddrv_ge_dev, cmd_hdl, GE_PTN, ptn_x, ptn_y);

    ge_cmd_end(m_osddrv_ge_dev, cmd_hdl);
    ge_cmd_list_end(m_osddrv_ge_dev, cmd_list);
    GE_SIMU_MUTEX_UNLOCK();		

    return RET_SUCCESS;
#endif

    if (dev == NULL)
        return RET_SUCCESS;

	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->region_write)
	{
		return dev->region_write(dev, uRegionId,pVscr,rect);
	}
	
	return  !RET_SUCCESS;
}
RET_CODE OSDDrv_RegionRead(HANDLE hDev,UINT8 uRegionId,VSCR *pVscr,struct OSDRect *rect)
{
	struct osd_device *dev = (struct osd_device *)hDev;

#ifdef GE_SIMULATE_OSD
    RET_CODE ret;
    GE_SIMU_MUTEX_LOCK();	

    ge_cmd_list_new(m_osddrv_ge_dev, m_cmd_list, GE_COMPILE_AND_EXECUTE);
    ret = ge_gma_set_region_to_cmd_list(m_osddrv_ge_dev, GMA_SW_LAYER_ID, uRegionId, m_cmd_list);
    if (ret != RET_SUCCESS)
    {
        GE_SIMU_MUTEX_UNLOCK();			
        return ret;
    }
    UINT32 byte_pitch = osddrv_get_pitch(pVscr->bColorMode, pVscr->vR.uWidth);
    UINT32 ptn_x, ptn_y;

    ptn_x = rect->uLeft - pVscr->vR.uLeft;
    ptn_y = rect->uTop - pVscr->vR.uTop;
    osal_cache_invalidate(pVscr->lpbScr + byte_pitch * ptn_y, byte_pitch * rect->uHeight);

    ge_cmd_list_hdl cmd_list = m_cmd_list;
    ge_base_addr_t base_addr;

    base_addr.color_format = (enum GE_PIXEL_FORMAT)osddrv_color_mode_to_ge(pVscr->bColorMode);
    base_addr.base_address = (UINT32)pVscr->lpbScr;
    base_addr.data_decoder = GE_DECODER_DISABLE;
    base_addr.pixel_pitch = pVscr->vR.uWidth;
    base_addr.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;

    UINT32 cmd_hdl = ge_cmd_begin(m_osddrv_ge_dev, cmd_list, GE_PRIM_DISABLE);
    ge_set_base_addr(m_osddrv_ge_dev, cmd_hdl, GE_DST, &base_addr);
    ge_set_xy(m_osddrv_ge_dev, cmd_hdl, GE_DST, ptn_x, ptn_y);
    ge_set_wh(m_osddrv_ge_dev, cmd_hdl, GE_DST, rect->uWidth, rect->uHeight);
    ge_set_xy(m_osddrv_ge_dev, cmd_hdl, GE_SRC, rect->uLeft, rect->uTop);

    ge_cmd_end(m_osddrv_ge_dev, cmd_hdl);
    ge_cmd_list_end(m_osddrv_ge_dev, cmd_list);
    GE_SIMU_MUTEX_UNLOCK();	

    return RET_SUCCESS;
#endif

    if (dev == NULL)
        return RET_SUCCESS;

	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->region_read)
	{
		return dev->region_read(dev, uRegionId,pVscr,rect);
	}
	
	return  !RET_SUCCESS;
}
RET_CODE OSDDrv_RegionFill(HANDLE hDev,UINT8 uRegionId,struct OSDRect *rect, UINT32 uColorData)
{
	struct osd_device *dev = (struct osd_device *)hDev;

#ifdef GE_SIMULATE_OSD
    RET_CODE ret;
    GE_SIMU_MUTEX_LOCK();	

    ge_cmd_list_new(m_osddrv_ge_dev, m_cmd_list, GE_COMPILE_AND_EXECUTE);
    ret = ge_gma_set_region_to_cmd_list(m_osddrv_ge_dev, GMA_SW_LAYER_ID, uRegionId, m_cmd_list);
    if (ret != RET_SUCCESS)
    {
       GE_SIMU_MUTEX_UNLOCK();	
	return ret;
    }
    UINT32 cmd_hdl = ge_cmd_begin(m_osddrv_ge_dev, m_cmd_list, GE_FILL_RECT_DRAW_COLOR);
    ge_set_draw_color(m_osddrv_ge_dev, cmd_hdl, uColorData);
    ge_set_xy(m_osddrv_ge_dev, cmd_hdl, GE_DST, rect->uLeft, rect->uTop);
    ge_set_wh(m_osddrv_ge_dev, cmd_hdl, GE_DST, rect->uWidth, rect->uHeight);
    ge_cmd_end(m_osddrv_ge_dev, cmd_hdl);

    ge_cmd_list_end(m_osddrv_ge_dev, m_cmd_list);
    GE_SIMU_MUTEX_UNLOCK();	

    return RET_SUCCESS;
#endif

    if (dev == NULL)
        return RET_SUCCESS;

	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->region_fill)
	{
		return dev->region_fill(dev, uRegionId,rect,uColorData);
	}
	
	return !RET_SUCCESS;
}
//#ifdef BIDIRECTIONAL_OSD_STYLE
RET_CODE OSDDrv_RegionWriteInverse(HANDLE hDev,UINT8 uRegionId,VSCR *pVscr,struct OSDRect *rect)
{
	struct osd_device *dev = (struct osd_device *)hDev;

#ifdef GE_SIMULATE_OSD
    RET_CODE ret;
    struct OSDRect rr;

    ret = OSDDrv_GetRegionPos(hDev, uRegionId, &rr);
    if(ret != RET_SUCCESS)
        return ret;
    
    GE_SIMU_MUTEX_LOCK();		

    ge_cmd_list_new(m_osddrv_ge_dev, m_cmd_list, GE_COMPILE_AND_EXECUTE);
    ret = ge_gma_set_region_to_cmd_list(m_osddrv_ge_dev, GMA_SW_LAYER_ID, uRegionId, m_cmd_list);
    if (ret != RET_SUCCESS)
    {
	 GE_SIMU_MUTEX_UNLOCK();				
        return ret;
    }
    UINT32 byte_pitch = osddrv_get_pitch(pVscr->bColorMode, pVscr->vR.uWidth);
    UINT32 ptn_x, ptn_y;

    ptn_x = rect->uLeft - pVscr->vR.uLeft;
    ptn_y = rect->uTop - pVscr->vR.uTop;
    osal_cache_flush(pVscr->lpbScr + byte_pitch * ptn_y, byte_pitch * rect->uHeight);

    ge_cmd_list_hdl cmd_list = m_cmd_list;
    ge_base_addr_t base_addr;

    base_addr.color_format = (enum GE_PIXEL_FORMAT)osddrv_color_mode_to_ge(pVscr->bColorMode);
    base_addr.base_address = (UINT32)pVscr->lpbScr;
    base_addr.data_decoder = GE_DECODER_DISABLE;
    base_addr.pixel_pitch = pVscr->vR.uWidth;
    base_addr.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;

    UINT32 cmd_hdl = ge_cmd_begin(m_osddrv_ge_dev, cmd_list, GE_DRAW_BITMAP);
    ge_set_base_addr(m_osddrv_ge_dev, cmd_hdl, GE_PTN, &base_addr);
    ge_set_wh(m_osddrv_ge_dev, cmd_hdl, GE_DST_PTN, rect->uWidth, rect->uHeight);
    ge_set_xy(m_osddrv_ge_dev, cmd_hdl, GE_DST, (rr.uWidth - rect->uLeft - rect->uWidth), rect->uTop);
    ge_set_xy(m_osddrv_ge_dev, cmd_hdl, GE_PTN, ptn_x, ptn_y);
    ge_set_scan_order(m_osddrv_ge_dev, cmd_hdl, GE_PTN, GE_SCAN_TOP_TO_BOTTOM, GE_SCAN_RIGHT_TO_LEFT);

    ge_cmd_end(m_osddrv_ge_dev, cmd_hdl);
    ge_cmd_list_end(m_osddrv_ge_dev, cmd_list);
    GE_SIMU_MUTEX_UNLOCK();		

    return RET_SUCCESS;
#endif

	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->region_write_inverse)
	{
		return dev->region_write_inverse(dev, uRegionId,pVscr,rect);
	}
	
	return  !RET_SUCCESS;
}
RET_CODE OSDDrv_RegionReadInverse(HANDLE hDev,UINT8 uRegionId,VSCR *pVscr,struct OSDRect *rect)
{
	struct osd_device *dev = (struct osd_device *)hDev;

#ifdef GE_SIMULATE_OSD
    RET_CODE ret;
    struct OSDRect rr;

    ret = OSDDrv_GetRegionPos(hDev, uRegionId, &rr);
    if(ret != RET_SUCCESS)
        return ret;

    GE_SIMU_MUTEX_LOCK();	

    ge_cmd_list_new(m_osddrv_ge_dev, m_cmd_list, GE_COMPILE_AND_EXECUTE);
    ret = ge_gma_set_region_to_cmd_list(m_osddrv_ge_dev, GMA_SW_LAYER_ID, uRegionId, m_cmd_list);
    if (ret != RET_SUCCESS)
    {
        GE_SIMU_MUTEX_UNLOCK();			
        return ret;
    }
    UINT32 byte_pitch = osddrv_get_pitch(pVscr->bColorMode, pVscr->vR.uWidth);
    UINT32 ptn_x, ptn_y;

    ptn_x = rect->uLeft - pVscr->vR.uLeft;
    ptn_y = rect->uTop - pVscr->vR.uTop;
    osal_cache_invalidate(pVscr->lpbScr + byte_pitch * ptn_y, byte_pitch * rect->uHeight);

    ge_cmd_list_hdl cmd_list = m_cmd_list;
    ge_base_addr_t base_addr;

    base_addr.color_format = (enum GE_PIXEL_FORMAT)osddrv_color_mode_to_ge(pVscr->bColorMode);
    base_addr.base_address = (UINT32)pVscr->lpbScr;
    base_addr.data_decoder = GE_DECODER_DISABLE;
    base_addr.pixel_pitch = pVscr->vR.uWidth;
    base_addr.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;

    UINT32 cmd_hdl = ge_cmd_begin(m_osddrv_ge_dev, cmd_list, GE_PRIM_DISABLE);
    ge_set_base_addr(m_osddrv_ge_dev, cmd_hdl, GE_DST, &base_addr);
    ge_set_xy(m_osddrv_ge_dev, cmd_hdl, GE_DST, ptn_x, ptn_y);
    ge_set_wh(m_osddrv_ge_dev, cmd_hdl, GE_DST, rect->uWidth, rect->uHeight);
    ge_set_xy(m_osddrv_ge_dev, cmd_hdl, GE_SRC, (rr.uWidth - rect->uLeft - rect->uWidth), rect->uTop);
    ge_set_scan_order(m_osddrv_ge_dev, cmd_hdl, GE_SRC, GE_SCAN_TOP_TO_BOTTOM, GE_SCAN_RIGHT_TO_LEFT);

    ge_cmd_end(m_osddrv_ge_dev, cmd_hdl);
    ge_cmd_list_end(m_osddrv_ge_dev, cmd_list);
    GE_SIMU_MUTEX_UNLOCK();	

    return RET_SUCCESS;
#endif

	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->region_read_inverse)
	{
		return dev->region_read_inverse(dev, uRegionId,pVscr,rect);
	}
	
	return  !RET_SUCCESS;
}
RET_CODE OSDDrv_RegionFillInverse(HANDLE hDev,UINT8 uRegionId,struct OSDRect *rect, UINT32 uColorData)
{
	struct osd_device *dev = (struct osd_device *)hDev;

#ifdef GE_SIMULATE_OSD
    RET_CODE ret;
    struct OSDRect rr;

    ret = OSDDrv_GetRegionPos(hDev, uRegionId, &rr);
    if(ret != RET_SUCCESS)
        return ret;

    GE_SIMU_MUTEX_LOCK();	

    ge_cmd_list_new(m_osddrv_ge_dev, m_cmd_list, GE_COMPILE_AND_EXECUTE);
    ret = ge_gma_set_region_to_cmd_list(m_osddrv_ge_dev, GMA_SW_LAYER_ID, uRegionId, m_cmd_list);
    if (ret != RET_SUCCESS)
    {
       GE_SIMU_MUTEX_UNLOCK();	
	return ret;
    }
    UINT32 cmd_hdl = ge_cmd_begin(m_osddrv_ge_dev, m_cmd_list, GE_FILL_RECT_DRAW_COLOR);
    ge_set_draw_color(m_osddrv_ge_dev, cmd_hdl, uColorData);
    ge_set_xy(m_osddrv_ge_dev, cmd_hdl, GE_DST, (rr.uWidth - rect->uLeft - rect->uWidth), rect->uTop);
    ge_set_wh(m_osddrv_ge_dev, cmd_hdl, GE_DST, rect->uWidth, rect->uHeight);
    ge_set_scan_order(m_osddrv_ge_dev, cmd_hdl, GE_PTN, GE_SCAN_TOP_TO_BOTTOM, GE_SCAN_RIGHT_TO_LEFT);
    ge_cmd_end(m_osddrv_ge_dev, cmd_hdl);

    ge_cmd_list_end(m_osddrv_ge_dev, m_cmd_list);
    GE_SIMU_MUTEX_UNLOCK();	

    return RET_SUCCESS;
#endif

	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->region_fill_inverse)
	{
		return dev->region_fill_inverse(dev, uRegionId,rect,uColorData);
	}
	
	return !RET_SUCCESS;
}
//#endif
	


RET_CODE OSDDrv_RegionWrite2(HANDLE hDev,UINT8 uRegionId,UINT8* pSrcData,UINT16 uSrcWidth,UINT16 uSrcHeight,struct OSDRect* pSrcRect,struct OSDRect* pDestRect)
{
	struct osd_device *dev = (struct osd_device *)hDev;

#ifdef GE_SIMULATE_OSD
    RET_CODE ret;
    ge_gma_region_t region_param;

    UINT32 vscr_width = 1920, vscr_height = 1080; // get from subtitle
    UINT32 reg_width, reg_height;
    GE_SIMU_MUTEX_LOCK();

    ret = ge_gma_get_region_info(m_osddrv_ge_dev, GMA_SW_LAYER_ID, uRegionId, &region_param);
    if (ret != RET_SUCCESS)
    {
	GE_SIMU_MUTEX_UNLOCK();		
        return ret;
    }
    if (uSrcWidth > 1280)
    {
        vscr_width = 1920;
        vscr_height = 1080;
    }
    else if (uSrcWidth > 720)
    {
        vscr_width = 1280;
        vscr_height = 720;
    }
    else
    {
        vscr_width = 720;
        vscr_height = 576;
    }

    reg_width = region_param.region_w;
    reg_height = region_param.region_h;


    if (vscr_width > reg_width || vscr_height > reg_height)
    {
        region_param.region_x = 0;
        region_param.region_y = 0;
        region_param.region_w = vscr_width;
        region_param.region_h = vscr_height;
        region_param.bitmap_x = 0;
        region_param.bitmap_y = 0;
        region_param.bitmap_w = vscr_width;
        region_param.bitmap_h = vscr_height;
        region_param.pixel_pitch = vscr_width;
        region_param.bitmap_addr = 0;
        ret = ge_gma_delete_region(m_osddrv_ge_dev, GMA_SW_LAYER_ID, uRegionId);
        if (ret != RET_SUCCESS)
        {
            SDBBP();
        }
        ret = ge_gma_create_region(m_osddrv_ge_dev, GMA_SW_LAYER_ID, uRegionId, &region_param);
        if (ret != RET_SUCCESS)
        {
            SDBBP();
            return ret;
        }
        ret = ge_gma_scale(m_osddrv_ge_dev, GMA_SW_LAYER_ID, GE_VSCALE_TTX_SUBT, PAL);
        if (ret != RET_SUCCESS)
        {
            SDBBP();
        }
    }

    ge_cmd_list_new(m_osddrv_ge_dev, m_cmd_list, GE_COMPILE_AND_EXECUTE);
    ret = ge_gma_set_region_to_cmd_list(m_osddrv_ge_dev, GMA_SW_LAYER_ID, uRegionId, m_cmd_list);
    if (ret != RET_SUCCESS)
    {
	GE_SIMU_MUTEX_UNLOCK();				
        return ret;
    }
    UINT32 byte_pitch = osddrv_get_pitch(OSD_256_COLOR, uSrcWidth);

    osal_cache_flush(pSrcData + byte_pitch * pSrcRect->uTop, byte_pitch * pSrcRect->uHeight);

    ge_cmd_list_hdl cmd_list = m_cmd_list;
    ge_base_addr_t base_addr;

    base_addr.color_format = (enum GE_PIXEL_FORMAT)osddrv_color_mode_to_ge(OSD_256_COLOR);
    base_addr.base_address = (UINT32)pSrcData;
    base_addr.data_decoder = GE_DECODER_DISABLE;
    base_addr.pixel_pitch = uSrcWidth;
    base_addr.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;

    UINT32 cmd_hdl = ge_cmd_begin(m_osddrv_ge_dev, cmd_list, GE_DRAW_BITMAP);
    ge_set_base_addr(m_osddrv_ge_dev, cmd_hdl, GE_PTN, &base_addr);
    ge_set_wh(m_osddrv_ge_dev, cmd_hdl, GE_DST_PTN, pSrcRect->uWidth, pSrcRect->uHeight);
    ge_set_xy(m_osddrv_ge_dev, cmd_hdl, GE_DST, pDestRect->uLeft, pDestRect->uTop);
    ge_set_xy(m_osddrv_ge_dev, cmd_hdl, GE_PTN, pSrcRect->uLeft, pSrcRect->uTop);

    ge_cmd_end(m_osddrv_ge_dev, cmd_hdl);
    ge_cmd_list_end(m_osddrv_ge_dev, cmd_list);
	GE_SIMU_MUTEX_UNLOCK();

    return RET_SUCCESS;
#endif

    if (dev == NULL)
        return RET_SUCCESS;

	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->region_write2)
	{
		 return dev->region_write2(dev, uRegionId,pSrcData,uSrcWidth,uSrcHeight,pSrcRect,pDestRect);
	}
	
	return  !RET_SUCCESS;
}

RET_CODE OSDDrv_DrawHorLine(HANDLE hDev, UINT8 uRegionId, UINT32 x, UINT32 y, UINT32 width, UINT32 color)
{
	struct osd_device *dev = (struct osd_device *)hDev;

#ifdef GE_SIMULATE_OSD
    ge_cmd_list_hdl cmd_list = m_cmd_list;
	GE_SIMU_MUTEX_LOCK();
    ge_cmd_list_new(m_osddrv_ge_dev, cmd_list, GE_COMPILE_AND_EXECUTE);

    RET_CODE ret = ge_gma_set_region_to_cmd_list(m_osddrv_ge_dev, GMA_SW_LAYER_ID, uRegionId, m_cmd_list);
    if (ret != RET_SUCCESS)
    {
	GE_SIMU_MUTEX_UNLOCK();				
        return ret;
    }

    UINT32 cmd_hdl = ge_cmd_begin(m_osddrv_ge_dev, cmd_list, GE_FILL_RECT_DRAW_COLOR);
    ge_set_draw_color(m_osddrv_ge_dev, cmd_hdl, color);
    ge_set_wh(m_osddrv_ge_dev, cmd_hdl, GE_DST, width, 1);
    ge_set_xy(m_osddrv_ge_dev, cmd_hdl, GE_DST, x, y);

    ge_cmd_end(m_osddrv_ge_dev, cmd_hdl);
    ge_cmd_list_end(m_osddrv_ge_dev, cmd_list);
	GE_SIMU_MUTEX_UNLOCK();

    return RET_SUCCESS;
#endif

    if (dev == NULL)
        return RET_SUCCESS;

	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->draw_hor_line)
	{
		return dev->draw_hor_line(dev, uRegionId, x, y, width, color);
	}	

	return !RET_SUCCESS;
}

RET_CODE OSDDrv_Scale(HANDLE hDev,UINT32 uScaleCmd,UINT32 uScaleParam)
{
	struct osd_device *dev = (struct osd_device *)hDev;

#ifdef GE_SIMULATE_OSD

    static const ge_scale_param_t m_gma_scale_param[] =
    {
        {LINE_720_25, 720, 576, 1280, 720 },
        {LINE_720_30, 720, 480, 1280, 720 },
        {LINE_1080_25, 720, 576, 1920, 1080 },
        {LINE_1080_30, 720, 480, 1920, 1080 },
    };

    static const ge_scale_param_t m_gma_scale_param_src_1280x720[] =
    {
        {PAL,  1280, 720, 720, 576 },
        {NTSC, 1280, 720, 720, 480 },
        {LINE_720_25, 1, 1, 1, 1 },
        {LINE_720_30, 1, 1, 1, 1 },
        {LINE_1080_25, 1280, 720, 1920, 1080 },
        {LINE_1080_30, 1280, 720, 1920, 1080 },
    };

    static const ge_scale_param_t m_gma_multiview[]=
    {
        {PAL,  1, 384, 1, 510 },
        {NTSC, 1, 384, 1, 426},		
    };
    GE_SIMU_MUTEX_LOCK();
    switch (uScaleCmd)
    {
        case OSD_SCALE_WITH_PARAM:
            uScaleCmd = GE_SET_SCALE_PARAM;
            break;
        case OSD_VSCALE_OFF:
            uScaleCmd = GE_VSCALE_OFF;
            break;
        case OSD_VSCALE_TTX_SUBT:
            uScaleCmd = GE_VSCALE_TTX_SUBT;
            break;
        case OSD_HDUPLICATE_ON:
            uScaleCmd = GE_H_DUPLICATE_ON_OFF;
            uScaleParam = TRUE;
            break;
        case OSD_HDUPLICATE_OFF:
            uScaleCmd = GE_H_DUPLICATE_ON_OFF;
            uScaleParam = FALSE;
            break;
        case OSD_OUTPUT_720:
            uScaleCmd = GE_SET_SCALE_PARAM;
            if (uScaleParam == OSD_SOURCE_NTSC)
                uScaleParam = (UINT32)&m_gma_scale_param[1];
            else
                uScaleParam = (UINT32)&m_gma_scale_param[0];
            break;
        case OSD_OUTPUT_1080:
            uScaleCmd = GE_SET_SCALE_PARAM;
            if (uScaleParam == OSD_SOURCE_NTSC)
                uScaleParam = (UINT32)&m_gma_scale_param[3];
            else
                uScaleParam = (UINT32)&m_gma_scale_param[2];
            break;
        case OSD_SET_SCALE_MODE:
            uScaleCmd = GE_SET_SCALE_MODE;
            break;
        case OSD_HDVSCALE_OFF:
            uScaleCmd = GE_SET_SCALE_PARAM;
            if (uScaleParam == OSD_SOURCE_NTSC)
                uScaleParam = (UINT32)&m_gma_scale_param_src_1280x720[1];
            else
                uScaleParam = (UINT32)&m_gma_scale_param_src_1280x720[0];
            break;
	case OSD_HDOUTPUT_720:
            uScaleCmd = GE_SET_SCALE_PARAM;
            if (uScaleParam == OSD_SOURCE_NTSC)
                uScaleParam = (UINT32)&m_gma_scale_param_src_1280x720[3];
            else
                uScaleParam = (UINT32)&m_gma_scale_param_src_1280x720[2];
            break;
	case OSD_HDOUTPUT_1080:
            uScaleCmd = GE_SET_SCALE_PARAM;
            if (uScaleParam == OSD_SOURCE_NTSC)
                uScaleParam = (UINT32)&m_gma_scale_param_src_1280x720[5];
            else
                uScaleParam = (UINT32)&m_gma_scale_param_src_1280x720[4];
            break;
	case OSD_VSCALE_DVIEW:
            uScaleCmd = GE_SET_SCALE_PARAM;
            if (*(enum OSDSys *)uScaleParam == OSD_PAL)
                uScaleParam = (UINT32)&m_gma_multiview[0];
            else
                uScaleParam = (UINT32)&m_gma_multiview[1];	     
            break;
        default:
 	    GE_SIMU_MUTEX_UNLOCK();
           return RET_SUCCESS;
            break;
    }
    RET_CODE ret = ge_gma_scale(m_osddrv_ge_dev, GMA_SW_LAYER_ID, uScaleCmd, uScaleParam);
	GE_SIMU_MUTEX_UNLOCK();
	return ret;
#endif

    if (dev == NULL)
        return RET_SUCCESS;

	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->scale)
	{
		return dev->scale(dev, uScaleCmd, uScaleParam);
	}
	
	return  !RET_SUCCESS;
}
RET_CODE OSDDrv_SetClip(HANDLE hDev,enum CLIPMode clipmode,struct OSDRect *pRect)
{
	struct osd_device *dev = (struct osd_device *)hDev;

#ifdef GE_SIMULATE_OSD
    enum GE_CLIP_MODE ge_clip_mode;
	GE_SIMU_MUTEX_LOCK();

    if (clipmode == CLIP_INSIDE_RECT)
        ge_clip_mode = GE_CLIP_INSIDE;
    else if (clipmode == CLIP_OUTSIDE_RECT)
        ge_clip_mode = GE_CLIP_OUTSIDE;
    else
        ge_clip_mode = GE_CLIP_DISABLE;

    ge_gma_set_region_clip_rect(m_osddrv_ge_dev, GMA_SW_LAYER_ID,
        pRect->uLeft, pRect->uTop, pRect->uWidth, pRect->uHeight);

    RET_CODE ret = ge_gma_set_region_clip_mode(m_osddrv_ge_dev, GMA_SW_LAYER_ID, ge_clip_mode);
	GE_SIMU_MUTEX_UNLOCK();
	return ret;
#endif

    if (dev == NULL)
        return RET_SUCCESS;

	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->set_clip)
	{
		return dev->set_clip(dev, clipmode,pRect);
	}
	
	return !RET_SUCCESS;
}
RET_CODE OSDDrv_ClearClip(HANDLE hDev)
{
	struct osd_device *dev = (struct osd_device *)hDev;

#ifdef GE_SIMULATE_OSD
	GE_SIMU_MUTEX_LOCK();
       RET_CODE ret = ge_gma_set_region_clip_mode(m_osddrv_ge_dev, GMA_SW_LAYER_ID, GE_CLIP_DISABLE);
	GE_SIMU_MUTEX_UNLOCK();
	return ret;
#endif

    if (dev == NULL)
        return RET_SUCCESS;

	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->clear_clip)
	{
		return dev->clear_clip(dev);
	}
	
	return  !RET_SUCCESS;
}

RET_CODE OSDDrv_GetRegionAddr(HANDLE hDev,UINT8 region_idx,UINT16 y, UINT32 *addr)
{
	struct osd_device *dev = (struct osd_device *)hDev;

#ifdef GE_SIMULATE_OSD
    ge_gma_region_t region_param;
    GE_SIMU_MUTEX_LOCK();

    RET_CODE ret = ge_gma_get_region_info(m_osddrv_ge_dev, GMA_SW_LAYER_ID, region_idx, &region_param);

    UINT32 base_addr = region_param.bitmap_addr;
    base_addr += y * osddrv_get_pitch_by_ge_format(region_param.color_format, region_param.pixel_pitch); // todo: calc according to color_format

    *addr = base_addr;
    GE_SIMU_MUTEX_UNLOCK();

    return ret;
#endif

    if (dev == NULL)
    {
        *addr = 0;
        return RET_SUCCESS;
    }

	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->get_region_addr)
	{
		return dev->get_region_addr(dev,region_idx, y, addr);
	}
	
	return  !RET_SUCCESS;
}


