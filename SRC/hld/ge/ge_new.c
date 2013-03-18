/****************************************************************************
 *
 *  ALi Corporation, All Rights Reserved. 2008 Copyright (C)
 *
 *  File: ge_new.c
 *
 *  Description: This file defines new graphics engine API.
 *
 *  History:
 *      Date          Author        Version         Comment
 *      ====          ======        =======         =======
 *  1.  2008.11.23   Jerry Long     0.1.000         Initial
 *  2.  2009.2.6	Sam 		0.1.001	     seperate some APIs to ge.c
  ****************************************************************************/

RET_CODE ge_io_ctrl_ext(struct ge_device *dev, UINT32 layer_id, UINT32 dwCmd, UINT32 dwParam)
{
    if(NULL == dev)return RET_FAILURE;

    /* If device not running, exit */
    if ((dev->flags & HLD_DEV_STATS_UP) == 0)
    {
        return RET_FAILURE;
    }

    if (dev->ioctl_ext)
    {
        return dev->ioctl_ext(dev, layer_id, dwCmd, dwParam);
    }

    return RET_FAILURE;
}

ge_cmd_list_hdl ge_cmd_list_create(struct ge_device *dev, UINT32 init_cmd_buf_size)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return 0;
	}

	if (dev->cmd_list_create)
	    return dev->cmd_list_create(dev, init_cmd_buf_size);

	return 0;
}

RET_CODE ge_cmd_list_destroy(struct ge_device *dev, ge_cmd_list_hdl cmd_list)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->cmd_list_destroy)
	    return dev->cmd_list_destroy(dev, cmd_list);

	return RET_FAILURE;
}

BOOL ge_cmd_list_is_valid(struct ge_device *dev, ge_cmd_list_hdl cmd_list)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return FALSE;
	}

	if (dev->cmd_list_is_valid)
	    return dev->cmd_list_is_valid(dev, cmd_list);

	return FALSE;
}

UINT32 ge_cmd_list_get_base(struct ge_device *dev, ge_cmd_list_hdl cmd_list)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->cmd_list_get_base)
	    return dev->cmd_list_get_base(dev, cmd_list);

	return RET_FAILURE;
}

RET_CODE ge_cmd_list_new(struct ge_device *dev, ge_cmd_list_hdl cmd_list, enum GE_CMD_LIST_MODE list_mode)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->cmd_list_new)
	    return dev->cmd_list_new(dev, cmd_list, list_mode);

	return RET_FAILURE;
}

RET_CODE ge_cmd_list_end(struct ge_device *dev, ge_cmd_list_hdl cmd_list)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->cmd_list_end)
	    return dev->cmd_list_end(dev, cmd_list);

	return RET_FAILURE;
}


UINT32 ge_cmd_list_get_num(struct ge_device *dev, ge_cmd_list_hdl cmd_list)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return 0;
	}

	if (dev->cmd_list_get_num)
	    return dev->cmd_list_get_num(dev, cmd_list);

	return 0;
}

UINT32 ge_cmd_list_get_first(struct ge_device *dev, ge_cmd_list_hdl cmd_list)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return GE_INVALID_CMD;
	}

	if (dev->cmd_list_get_first)
	    return dev->cmd_list_get_first(dev, cmd_list);

	return GE_INVALID_CMD;
}

UINT32 ge_cmd_list_get_last(struct ge_device *dev, ge_cmd_list_hdl cmd_list)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return GE_INVALID_CMD;
	}

	if (dev->cmd_list_get_last)
	    return dev->cmd_list_get_last(dev, cmd_list);

	return GE_INVALID_CMD;
}

UINT32 ge_cmd_list_get_next(struct ge_device *dev, UINT32 cmd_pos)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return GE_INVALID_CMD;
	}

	if (dev->cmd_list_get_next)
	    return dev->cmd_list_get_next(dev, cmd_pos);

	return GE_INVALID_CMD;
}

UINT32 ge_cmd_list_get_prev(struct ge_device *dev, UINT32 cmd_pos)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return GE_INVALID_CMD;
	}

	if (dev->cmd_list_get_prev)
	    return dev->cmd_list_get_prev(dev, cmd_pos);

	return GE_INVALID_CMD;
}

UINT32 ge_cmd_begin(struct ge_device *dev, ge_cmd_list_hdl cmd_list, enum GE_PRIMITIVE_MODE prim_mode)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return 0;
	}

	if (dev->cmd_begin)
	    return dev->cmd_begin(dev, cmd_list, prim_mode);

	return 0;
}

RET_CODE ge_cmd_end(struct ge_device *dev, UINT32 cmd_pos)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->cmd_end)
	    return dev->cmd_end(dev, cmd_pos);

	return RET_FAILURE;
}

RET_CODE ge_cmd_list_set_param(struct ge_device *dev, ge_cmd_list_hdl cmd_list, UINT32 param)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->cmd_list_set_param)
	    return dev->cmd_list_set_param(dev, cmd_list, param);

	return RET_FAILURE;
}

UINT32 ge_cmd_list_get_param(struct ge_device *dev, ge_cmd_list_hdl cmd_list)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return 0;
	}

	if (dev->cmd_list_get_param)
	    return dev->cmd_list_get_param(dev, cmd_list);

	return 0;
}

RET_CODE ge_cmd_list_start(struct ge_device *dev, ge_cmd_list_hdl cmd_list, BOOL bPriority, BOOL sync)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->cmd_list_start)
	    return dev->cmd_list_start(dev, cmd_list, bPriority, sync);

	return RET_FAILURE;
}

RET_CODE ge_cmd_list_stop(struct ge_device *dev, ge_cmd_list_hdl cmd_list, BOOL bPriority, BOOL sync)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->cmd_list_stop)
	    return dev->cmd_list_stop(dev, cmd_list, bPriority, sync);

	return RET_FAILURE;
}


// All other ge_set_xxx() can be invoked between ge_cmd_begin() and ge_cmd_end()

// Set GE_IO_REG, affect all new IO_CMD
// Set cmd_base (by ge_get_cmd_base()), affect all new command in CMD_Q
// Set a cmd in CMD_Q, only affect the specified command

// for all data path
RET_CODE ge_set_base_addr(struct ge_device *dev, UINT32 cmd_pos, enum GE_DATA_PATH_TYPE data_path, const ge_base_addr_t *pbase_addr)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->set_operating_entity)
	    return dev->set_operating_entity(dev, cmd_pos, data_path, pbase_addr);

	return RET_FAILURE;
}

RET_CODE ge_get_base_addr(struct ge_device *dev, UINT32 cmd_pos, enum GE_DATA_PATH_TYPE data_path, ge_base_addr_t *pbase_addr)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->get_operating_entity)
	    return dev->get_operating_entity(dev, cmd_pos, data_path, pbase_addr);

	return RET_FAILURE;
}

/*
    ge_cmd_begin(dev, cmd_list, prim_mode) is the start point of all command.

    After ge_cmd_begin(cmd_list, prim_mode), the command state will be initialized to:
        PRIM/PTN mode is user specified,
        SRC Mode is set to GE_SRC_BITBLT or GE_SRC_DISABLED according to prim_mode,
        MSK state is disabled,
        ROP mode is GE_ROP_PTN_BYPASS or GE_ROP_ALPHA_BLENDING, bool operation is disabled,
        IO_cmd: Color Key is disabled, cmd_Q: copyed from GE_CMD_BASE,
        IO_cmd: PTN CLUT op is disabled, cmd_Q: copyed from GE_CMD_BASE,
        IO_cmd: Bitmask state is unchanged, cmd_Q: Bitmask state is same as GE_CMD_BASE,
        IO_cmd: Clip state is unchanged, cmd_Q: Clip state is same as GE_CMD_BASE,
        All other things is unchanged.

    API user should call ge_set_xxx() to set the necessary parameters or enable other functions.
*/

// GE_PRIMITIVE_MODE is set by ge_cmd_begin(dev, cmd_list, prim_mode);
RET_CODE ge_set_src_mode(struct ge_device *dev, UINT32 cmd_pos, enum GE_SRC_MODE src_mode)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->set_src_mode)
	    return dev->set_src_mode(dev, cmd_pos, src_mode);

	return RET_FAILURE;
}

RET_CODE ge_set_msk_mode(struct ge_device *dev, UINT32 cmd_pos, enum GE_MSK_MODE msk_mode)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->set_msk_mode)
	    return dev->set_msk_mode(dev, cmd_pos, msk_mode);

	return RET_FAILURE;
}

RET_CODE ge_get_primitive_mode(struct ge_device *dev, UINT32 cmd_pos, enum GE_PRIMITIVE_MODE *pprim_mode)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->get_primitive_mode)
	    return dev->get_primitive_mode(dev, cmd_pos, pprim_mode);

	return RET_FAILURE;
}

RET_CODE ge_get_src_mode(struct ge_device *dev, UINT32 cmd_pos, enum GE_SRC_MODE *psrc_mode)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->get_src_mode)
	    return dev->get_src_mode(dev, cmd_pos, psrc_mode);

	return RET_FAILURE;
}

RET_CODE ge_get_msk_mode(struct ge_device *dev, UINT32 cmd_pos, enum GE_MSK_MODE *pmsk_mode)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->get_msk_mode)
	    return dev->get_msk_mode(dev, cmd_pos, pmsk_mode);

	return RET_FAILURE;
}

RET_CODE ge_set_xy(struct ge_device *dev, UINT32 cmd_pos, enum GE_DATA_PATH_TYPE data_path, INT32 x, INT32 y)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->set_xy)
	    return dev->set_xy(dev, cmd_pos, data_path, x, y);

	return RET_FAILURE;
}

RET_CODE ge_set_wh(struct ge_device *dev, UINT32 cmd_pos, enum GE_DATA_PATH_TYPE data_path, INT32 w, INT32 h)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->set_wh)
	    return dev->set_wh(dev, cmd_pos, data_path, w, h);

	return RET_FAILURE;
}

RET_CODE ge_get_xy(struct ge_device *dev, UINT32 cmd_pos, enum GE_DATA_PATH_TYPE data_path, INT32 *px, INT32 *py)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->get_xy)
	    return dev->get_xy(dev, cmd_pos, data_path, px, py);

	return RET_FAILURE;
}

RET_CODE ge_get_wh(struct ge_device *dev, UINT32 cmd_pos, enum GE_DATA_PATH_TYPE data_path, INT32 *pw, INT32 *ph)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->get_wh)
	    return dev->get_wh(dev, cmd_pos, data_path, pw, ph);

	return RET_FAILURE;
}

RET_CODE ge_set_xy2(struct ge_device *dev, UINT32 cmd_pos, enum GE_DATA_PATH_TYPE data_path, INT32 x, INT32 y, INT32 x_end, INT32 y_end)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->set_xy2)
	    return dev->set_xy2(dev, cmd_pos, data_path, x, y, x_end, y_end);

	return RET_FAILURE;
}

RET_CODE ge_get_xy2(struct ge_device *dev, UINT32 cmd_pos, enum GE_DATA_PATH_TYPE data_path, INT32 *px, INT32 *py, INT32 *px_end, INT32 *py_end)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->get_xy2)
	    return dev->get_xy2(dev, cmd_pos, data_path, px, py, px_end, py_end);

	return RET_FAILURE;
}

RET_CODE ge_set_back_color(struct ge_device *dev, UINT32 cmd_pos, UINT32 color)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->set_back_color)
	    return dev->set_back_color(dev, cmd_pos, color);

	return RET_FAILURE;
}

RET_CODE ge_set_font_color(struct ge_device *dev, UINT32 cmd_pos, UINT32 color)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->set_font_color)
	    return dev->set_font_color(dev, cmd_pos, color);

	return RET_FAILURE;
}

RET_CODE ge_set_draw_color(struct ge_device *dev, UINT32 cmd_pos, UINT32 color)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->set_draw_color)
	    return dev->set_draw_color(dev, cmd_pos, color);

	return RET_FAILURE;
}

RET_CODE ge_get_back_color(struct ge_device *dev, UINT32 cmd_pos, UINT32 *pcolor)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->get_back_color)
	    return dev->get_back_color(dev, cmd_pos, pcolor);

	return RET_FAILURE;
}

RET_CODE ge_get_font_color(struct ge_device *dev, UINT32 cmd_pos, UINT32 *pcolor)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->get_font_color)
	    return dev->get_font_color(dev, cmd_pos, pcolor);

	return RET_FAILURE;
}

RET_CODE ge_get_draw_color(struct ge_device *dev, UINT32 cmd_pos, UINT32 *pcolor)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->get_draw_color)
	    return dev->get_draw_color(dev, cmd_pos, pcolor);

	return RET_FAILURE;
}

RET_CODE ge_set_color_format(struct ge_device *dev, UINT32 cmd_pos, enum GE_PIXEL_FORMAT color_format)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->set_color_format)
	    return dev->set_color_format(dev, cmd_pos, color_format);

	return RET_FAILURE;
}

RET_CODE ge_set_rop_mode(struct ge_device *dev, UINT32 cmd_pos, enum GE_ROP_MODE rop_mode)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->set_rop_mode)
	    return dev->set_rop_mode(dev, cmd_pos, rop_mode);

	return RET_FAILURE;
}

RET_CODE ge_set_bop_mode(struct ge_device *dev, UINT32 cmd_pos, enum GE_BOP_MODE bop_mode)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->set_bop_mode)
	    return dev->set_bop_mode(dev, cmd_pos, bop_mode);

	return RET_FAILURE;
}

RET_CODE ge_get_rop_mode(struct ge_device *dev, UINT32 cmd_pos, enum GE_ROP_MODE *prop_mode)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->get_rop_mode)
	    return dev->get_rop_mode(dev, cmd_pos, prop_mode);

	return RET_FAILURE;
}

RET_CODE ge_get_bop_mode(struct ge_device *dev, UINT32 cmd_pos, enum GE_BOP_MODE *pbop_mode)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->get_bop_mode)
	    return dev->get_bop_mode(dev, cmd_pos, pbop_mode);

	return RET_FAILURE;
}


RET_CODE ge_set_global_alpha(struct ge_device *dev, UINT32 cmd_pos, UINT32 value)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->set_global_alpha)
	    return dev->set_global_alpha(dev, cmd_pos, value);

	return RET_FAILURE;
}

RET_CODE ge_set_global_alpha_mode(struct ge_device *dev, UINT32 cmd_pos, UINT32 mode)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->set_global_alpha_mode)
	    return dev->set_global_alpha_mode(dev, cmd_pos, mode);

	return RET_FAILURE;
}

RET_CODE ge_set_alpha_blend_mode(struct ge_device *dev, UINT32 cmd_pos, enum GE_ALPHA_BLEND_MODE mode)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->set_global_alpha_mode)
	    return dev->set_global_alpha_mode(dev, cmd_pos, mode);

	return RET_FAILURE;
}

RET_CODE ge_set_global_alpha_sel(struct ge_device *dev, UINT32 cmd_pos, enum GE_GLOBAL_ALPHA_SEL sel)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->set_global_alpha_sel)
	    return dev->set_global_alpha_sel(dev, cmd_pos, sel);

	return RET_FAILURE;
}

RET_CODE ge_set_global_alpha_layer(struct ge_device *dev, UINT32 cmd_pos, enum GE_GLOBAL_ALPHA_LAYER layer)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->set_global_alpha_layer)
	    return dev->set_global_alpha_layer(dev, cmd_pos, layer);

	return RET_FAILURE;
}

RET_CODE ge_set_bitmap_alpha_mode(struct ge_device *dev, UINT32 cmd_pos, enum GE_DATA_PATH_TYPE data_path, enum GE_BITMAP_ALPHA_MODE mode)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->set_bitmap_alpha_mode)
		return dev->set_bitmap_alpha_mode(dev, cmd_pos, data_path, mode);
	
	return RET_FAILURE;
};

RET_CODE ge_set_alpha_out_mode(struct ge_device *dev, UINT32 cmd_pos, enum GE_ALPHA_OUT_MODE mode)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}

	if(dev->set_alpha_out_mode)
		return dev->set_alpha_out_mode(dev, cmd_pos, mode);

	return RET_FAILURE;
}

RET_CODE ge_set_dst_alpha_out(struct ge_device *dev, UINT32 cmd_pos, UINT32 value)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if(dev->set_dst_alpha_out)
		return dev->set_dst_alpha_out(dev, cmd_pos, value);
	
	return RET_FAILURE;
}

RET_CODE ge_get_global_alpha(struct ge_device *dev, UINT32 cmd_pos, UINT32 *pvalue)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->get_global_alpha)
	    return dev->get_global_alpha(dev, cmd_pos, pvalue);

	return RET_FAILURE;
}

RET_CODE ge_get_global_alpha_mode(struct ge_device *dev, UINT32 cmd_pos, UINT32 *pmode)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->get_global_alpha_mode)
	    return dev->get_global_alpha_mode(dev, cmd_pos, pmode);

	return RET_FAILURE;
}

RET_CODE ge_get_global_alpha_sel(struct ge_device *dev, UINT32 cmd_pos, enum GE_GLOBAL_ALPHA_SEL *psel)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->get_global_alpha_sel)
	    return dev->get_global_alpha_sel(dev, cmd_pos, psel);

	return RET_FAILURE;
}

RET_CODE ge_get_alpha_out_mode(struct ge_device *dev, UINT32 cmd_pos, enum GE_ALPHA_OUT_MODE *pmode)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->get_alpha_out_mode)
		return dev->get_alpha_out_mode(dev, cmd_pos, pmode);
	
	return RET_FAILURE;
};

RET_CODE ge_get_dst_alpha_out(struct ge_device *dev, UINT32 cmd_pos, UINT32 *pvalue)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->get_dst_alpha_out)
		return dev->get_dst_alpha_out(dev, cmd_pos, pvalue);
	
	return RET_FAILURE;
};


RET_CODE ge_set_colorkey_mode(struct ge_device *dev, UINT32 cmd_pos, enum GE_COLOR_KEY_MODE mode)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->set_colorkey_mode)
	    return dev->set_colorkey_mode(dev, cmd_pos, mode);

	return RET_FAILURE;
}

RET_CODE ge_set_colorkey_match_mode(struct ge_device *dev, UINT32 cmd_pos, enum GE_COLOR_KEY_CHANNEL channel, enum GE_COLOR_KEY_MATCH_MODE mode)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->set_colorkey_match_mode)
	    return dev->set_colorkey_match_mode(dev, cmd_pos, channel, mode);

	return RET_FAILURE;
}

RET_CODE ge_set_colorkey_range(struct ge_device *dev, UINT32 cmd_pos, UINT32 low, UINT32 high)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->set_colorkey_range)
	    return dev->set_colorkey_range(dev, cmd_pos, low, high);

	return RET_FAILURE;
}

RET_CODE ge_get_colorkey_mode(struct ge_device *dev, UINT32 cmd_pos, enum GE_COLOR_KEY_MODE *pmode)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->get_colorkey_mode)
	    return dev->get_colorkey_mode(dev, cmd_pos, pmode);

	return RET_FAILURE;
}

RET_CODE ge_get_colorkey_match_mode(struct ge_device *dev, UINT32 cmd_pos, enum GE_COLOR_KEY_CHANNEL channel, enum GE_COLOR_KEY_MATCH_MODE *pmode)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->get_colorkey_match_mode)
	    return dev->get_colorkey_match_mode(dev, cmd_pos, channel, pmode);

	return RET_FAILURE;
}

RET_CODE ge_get_colorkey_range(struct ge_device *dev, UINT32 cmd_pos, UINT32 *plow, UINT32 *phigh)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->get_colorkey_range)
	    return dev->get_colorkey_range(dev, cmd_pos, plow, phigh);

	return RET_FAILURE;
}

// for SRC, PTN, and MSK
RET_CODE ge_set_byte_endian(struct ge_device *dev, UINT32 cmd_pos, enum GE_DATA_PATH_TYPE data_path, enum GE_BYTE_ENDIAN byte_endian)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->set_byte_endian)
	    return dev->set_byte_endian(dev, cmd_pos, data_path, byte_endian);

	return RET_FAILURE;
}

RET_CODE ge_set_subbyte_endian(struct ge_device *dev, UINT32 cmd_pos, enum GE_DATA_PATH_TYPE data_path, enum GE_SUBBYTE_ENDIAN subbyte_endian)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->set_subbyte_endian)
	    return dev->set_subbyte_endian(dev, cmd_pos, data_path, subbyte_endian);

	return RET_FAILURE;
}

RET_CODE ge_set_rgb_order(struct ge_device *dev, UINT32 cmd_pos, enum GE_DATA_PATH_TYPE data_path, enum GE_RGB_ORDER rgb_order)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->set_rgb_order)
	    return dev->set_rgb_order(dev, cmd_pos, data_path, rgb_order);

	return RET_FAILURE;
}

RET_CODE ge_set_alpha_attr(struct ge_device *dev, UINT32 cmd_pos, enum GE_DATA_PATH_TYPE data_path, enum GE_ALPHA_RANGE alpha_range, enum GE_ALPHA_POLARITY alpha_pol)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->set_alpha_attr)
	    return dev->set_alpha_attr(dev, cmd_pos, data_path, alpha_range, alpha_pol);

	return RET_FAILURE;
}

RET_CODE ge_set_rgb_expansion(struct ge_device *dev, UINT32 cmd_pos, enum GE_DATA_PATH_TYPE data_path, enum GE_RGB_COLOR_EXPANSION color_expan)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->set_rgb_expansion)
	    return dev->set_rgb_expansion(dev, cmd_pos, data_path, color_expan);

	return RET_FAILURE;
}


// Only for font PTN
RET_CODE ge_set_font_data_format(struct ge_device *dev, UINT32 cmd_pos, enum GE_FONT_DATA_FORMAT font_format)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->set_font_data_format)
	    return dev->set_font_data_format(dev, cmd_pos, font_format);

	return RET_FAILURE;
}

RET_CODE ge_set_font_stretch(struct ge_device *dev, UINT32 cmd_pos, UINT32 stretch_h, UINT32 stretch_v)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->set_font_stretch)
	    return dev->set_font_stretch(dev, cmd_pos, stretch_h, stretch_v);

	return RET_FAILURE;
}


// Only PTN can do CLUT operation
RET_CODE ge_set_clut_mode(struct ge_device *dev, UINT32 cmd_pos, enum GE_CLUT_MODE clut_mode, BOOL color_cvt_enable)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->set_clut_mode)
	    return dev->set_clut_mode(dev, cmd_pos, clut_mode, color_cvt_enable);

	return RET_FAILURE;
}


// clut address must be 8bytes aligned(struct ge_device *dev, address[2:0] == 0)
RET_CODE ge_set_clut_addr(struct ge_device *dev, UINT32 cmd_pos, UINT32 clut_addr)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->set_clut_addr)
	    return dev->set_clut_addr(dev, cmd_pos, clut_addr);

	return RET_FAILURE;
}


// ARGB == AYCbCr, BGRA = CrCbYA
RET_CODE ge_set_clut_rgb_order(struct ge_device *dev, UINT32 cmd_pos, enum GE_RGB_ORDER argb_order)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->set_clut_rgb_order)
	    return dev->set_clut_rgb_order(dev, cmd_pos, argb_order);

	return RET_FAILURE;
}

RET_CODE ge_set_clut_update(struct ge_device *dev, UINT32 cmd_pos, BOOL bUpdate)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->set_clut_update)
	    return dev->set_clut_update(dev, cmd_pos, bUpdate);

	return RET_FAILURE;
}

RET_CODE ge_set_clut_color_cvt(struct ge_device *dev, UINT32 cmd_pos, enum GE_COLOR_CVT_SYSTEM color_btx, enum GE_COLOR_SPACE_MATRIX color_matrix)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->set_clut_color_cvt)
	    return dev->set_clut_color_cvt(dev, cmd_pos, color_btx, color_matrix);

	return RET_FAILURE;
}


// Only for DST
RET_CODE ge_set_clip_mode(struct ge_device *dev, UINT32 cmd_pos, enum GE_CLIP_MODE clip_mode)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->set_clip_mode)
	    return dev->set_clip_mode(dev, cmd_pos, clip_mode);

	return RET_FAILURE;
}

RET_CODE ge_set_clip_rect(struct ge_device *dev, UINT32 cmd_pos, UINT32 x, UINT32 y, UINT32 w, UINT32 h)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->set_clip_rect)
	    return dev->set_clip_rect(dev, cmd_pos, x, y, w, h);

	return RET_FAILURE;
}


RET_CODE ge_enable_bitmask(struct ge_device *dev, UINT32 cmd_pos, BOOL bEnable)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->enable_bitmask)
	    return dev->enable_bitmask(dev, cmd_pos, bEnable);

	return RET_FAILURE;
}

RET_CODE ge_set_bitmask(struct ge_device *dev, UINT32 cmd_pos, UINT32 bitmask)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->set_bitmask)
	    return dev->set_bitmask(dev, cmd_pos, bitmask);

	return RET_FAILURE;
}


RET_CODE ge_enable_dither(struct ge_device *dev, UINT32 cmd_pos, BOOL bEnable)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->enable_dither)
	    return dev->enable_dither(dev, cmd_pos, bEnable);

	return RET_FAILURE;
}


// for SRC is overlapped with DST
RET_CODE ge_set_scan_order(struct ge_device *dev, UINT32 cmd_pos, enum GE_DATA_PATH_TYPE data_path, UINT32 vscan_order, UINT32 hscan_order)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->set_scan_order)
	    return dev->set_scan_order(dev, cmd_pos, data_path, vscan_order, hscan_order);

	return RET_FAILURE;
}



/*
Use the following 5 functions to retrieve a cmd list, for example:

ge_cmd_list_hdl cmd_list;
UINT32 cmd;
for (cmd = get_first(cmd_list); cmd != get_last(cmd_list); cmd = get_next(cmd))
{
    get_xxx(cmd);
    set_xxx(cmd);
}
or
for (cmd = get_last(cmd_list); cmd != get_first(cmd_list); )
{
    cmd = get_prev(cmd);
    get_xxx(cmd);
    set_xxx(cmd);
}

*/

// Begin GMA interfaces
RET_CODE ge_gma_show_onoff(struct ge_device *dev, UINT32 layer_id, BOOL bOnOff)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->gma_show_onoff)
	    return dev->gma_show_onoff(layer_id, bOnOff);

	return RET_FAILURE;
}

RET_CODE ge_gma_scale(struct ge_device *dev, UINT32 layer_id, UINT32 scale_cmd, UINT32 scale_param)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->gma_scale)
	    return dev->gma_scale(layer_id, scale_cmd, scale_param);

	return RET_FAILURE;
}

RET_CODE ge_set_scale_info(struct ge_device *dev, UINT32 cmd_pos, ge_scale_info_t *pscale_info)
{
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->set_scale_info)
	    return dev->set_scale_info(dev, cmd_pos, pscale_info);

	return RET_FAILURE;

}
RET_CODE ge_gma_set_pallette(struct ge_device *dev, UINT32 layer_id, const UINT8 *pal, UINT16 color_num, const ge_pal_attr_t *pattr)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->gma_set_pallette)
	    return dev->gma_set_pallette(layer_id, pal, color_num, pattr);

	return RET_FAILURE;
}

RET_CODE ge_gma_get_pallette(struct ge_device *dev, UINT32 layer_id, UINT8 *pal, UINT16 color_num, const ge_pal_attr_t *pattr)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->gma_get_pallette)
	    return dev->gma_get_pallette(layer_id, pal, color_num, pattr);

	return RET_FAILURE;
}

RET_CODE ge_gma_modify_pallette(struct ge_device *dev, UINT32 layer_id, UINT8 uIndex, UINT8 alpha, UINT8 uY, UINT8 uCb, UINT8 uCr, const ge_pal_attr_t *pattr)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->gma_modify_pallette)
	    return dev->gma_modify_pallette(layer_id, uIndex, alpha, uY, uCb, uCr, pattr);

	return RET_FAILURE;
}

RET_CODE ge_gma_create_region(struct ge_device *dev, UINT32 layer_id, UINT32 region_id, const ge_gma_region_t *pregion_param)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->gma_create_region)
	    return dev->gma_create_region(layer_id, region_id, pregion_param);

	return RET_FAILURE;
}

RET_CODE ge_gma_delete_region(struct ge_device *dev, UINT32 layer_id, UINT32 region_id)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->gma_delete_region)
	    return dev->gma_delete_region(layer_id, region_id);

	return RET_FAILURE;
}

RET_CODE ge_gma_move_region(struct ge_device *dev, UINT32 layer_id, UINT32 region_id, const ge_gma_region_t *pregion_param)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->gma_move_region)
	    return dev->gma_move_region(layer_id, region_id, pregion_param);

	return RET_FAILURE;
}

RET_CODE ge_gma_show_region(struct ge_device *dev, UINT32 layer_id, UINT32 region_id, BOOL bOnOff)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->gma_show_region)
	    return dev->gma_show_region(layer_id, region_id, bOnOff);

	return RET_FAILURE;
}

RET_CODE ge_gma_get_region_info(struct ge_device *dev, UINT32 layer_id, UINT32 region_id, ge_gma_region_t *pregion_param)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->gma_get_region_info)
	    return dev->gma_get_region_info(layer_id, region_id, pregion_param);

	return RET_FAILURE;
}

RET_CODE ge_gma_set_region_to_cmd_list(struct ge_device *dev, UINT32 layer_id, UINT32 region_id, ge_cmd_list_hdl cmd_list)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->gma_set_region_to_cmd_list)
	    return dev->gma_set_region_to_cmd_list(layer_id, region_id, cmd_list);

	return RET_FAILURE;
}

RET_CODE ge_gma_set_region_clip_mode(struct ge_device *dev, UINT32 layer_id, enum GE_CLIP_MODE clip_mode)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->gma_set_clip_mode)
	    return dev->gma_set_clip_mode(layer_id, clip_mode);

	return RET_FAILURE;
}

RET_CODE ge_gma_set_region_clip_rect(struct ge_device *dev, UINT32 layer_id, UINT32 x, UINT32 y, UINT32 w, UINT32 h)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->gma_set_clip_rect)
	    return dev->gma_set_clip_rect(layer_id, x, y, w, h);

	return RET_FAILURE;
}

RET_CODE ge_gma_close(struct ge_device *dev, UINT32 layer_id)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
	    return RET_FAILURE;
	}

	if (dev->gma_close)
	    return dev->gma_close(layer_id);

	return RET_FAILURE;
}
// End GMA interfaces

