#ifndef _GE_NEW_H_
#define _GE_NEW_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "gma_new.h"

RET_CODE ge_m36f_attach(const ge_layer_config_t *layer_config, UINT8 layer_num);

RET_CODE ge_io_ctrl_ext(struct ge_device *dev, UINT32 layer_id, UINT32 dwCmd, UINT32 dwParam);
ge_cmd_list_hdl ge_cmd_list_create(struct ge_device *dev, UINT32 init_cmd_buf_size);
RET_CODE ge_cmd_list_destroy(struct ge_device *dev, ge_cmd_list_hdl cmd_list);
BOOL ge_cmd_list_is_valid(struct ge_device *dev, ge_cmd_list_hdl cmd_list);
UINT32 ge_cmd_list_get_base(struct ge_device *dev, ge_cmd_list_hdl cmd_list);
RET_CODE ge_cmd_list_new(struct ge_device *dev, ge_cmd_list_hdl cmd_list, enum GE_CMD_LIST_MODE list_mode);
RET_CODE ge_cmd_list_end(struct ge_device *dev, ge_cmd_list_hdl cmd_list);

UINT32 ge_cmd_list_get_num(struct ge_device *dev, ge_cmd_list_hdl cmd_list);
UINT32 ge_cmd_list_get_first(struct ge_device *dev, ge_cmd_list_hdl cmd_list);
UINT32 ge_cmd_list_get_last(struct ge_device *dev, ge_cmd_list_hdl cmd_list);
UINT32 ge_cmd_list_get_next(struct ge_device *dev, UINT32 cmd_pos);
UINT32 ge_cmd_list_get_prev(struct ge_device *dev, UINT32 cmd_pos);

UINT32 ge_cmd_begin(struct ge_device *dev, ge_cmd_list_hdl cmd_list, enum GE_PRIMITIVE_MODE prim_mode);
// All other ge_set_xxx() can be invoked between ge_cmd_begin() and ge_cmd_end()
RET_CODE ge_cmd_end(struct ge_device *dev, UINT32 cmd_pos);

RET_CODE ge_cmd_list_set_param(struct ge_device *dev, ge_cmd_list_hdl cmd_list, UINT32 param);
UINT32 ge_cmd_list_get_param(struct ge_device *dev, ge_cmd_list_hdl cmd_list);

RET_CODE ge_cmd_list_start(struct ge_device *dev, ge_cmd_list_hdl cmd_list, BOOL bPriority, BOOL sync);
RET_CODE ge_cmd_list_stop(struct ge_device *dev, ge_cmd_list_hdl cmd_list, BOOL bPriority, BOOL sync);


// All other ge_set_xxx() can be invoked between ge_cmd_begin() and ge_cmd_end()

// Set GE_IO_REG, affect all new IO_CMD
// Set cmd_base (by ge_get_cmd_base()), affect all new command in CMD_Q
// Set a cmd in CMD_Q, only affect the specified command

// for all data path
RET_CODE ge_set_base_addr(struct ge_device *dev, UINT32 cmd_pos, enum GE_DATA_PATH_TYPE, const ge_base_addr_t *pbase_addr);
RET_CODE ge_get_base_addr(struct ge_device *dev, UINT32 cmd_pos, enum GE_DATA_PATH_TYPE, ge_base_addr_t *pbase_addr);

#define ge_set_operating_entity 	ge_set_base_addr
#define ge_get_operating_entity	ge_get_base_addr

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
RET_CODE ge_set_src_mode(struct ge_device *dev, UINT32 cmd_pos, enum GE_SRC_MODE src_mode);
RET_CODE ge_set_msk_mode(struct ge_device *dev, UINT32 cmd_pos, enum GE_MSK_MODE msk_mode);
RET_CODE ge_get_primitive_mode(struct ge_device *dev, UINT32 cmd_pos, enum GE_PRIMITIVE_MODE *pprim_mode);
RET_CODE ge_get_src_mode(struct ge_device *dev, UINT32 cmd_pos, enum GE_SRC_MODE *psrc_mode);
RET_CODE ge_get_msk_mode(struct ge_device *dev, UINT32 cmd_pos, enum GE_MSK_MODE *pmsk_mode);

RET_CODE ge_set_xy(struct ge_device *dev, UINT32 cmd_pos, enum GE_DATA_PATH_TYPE, INT32 x, INT32 y);
RET_CODE ge_set_wh(struct ge_device *dev, UINT32 cmd_pos, enum GE_DATA_PATH_TYPE, INT32 w, INT32 h);
RET_CODE ge_get_xy(struct ge_device *dev, UINT32 cmd_pos, enum GE_DATA_PATH_TYPE, INT32 *px, INT32 *py);
RET_CODE ge_get_wh(struct ge_device *dev, UINT32 cmd_pos, enum GE_DATA_PATH_TYPE, INT32 *pw, INT32 *ph);

RET_CODE ge_set_xy2(struct ge_device *dev, UINT32 cmd_pos, enum GE_DATA_PATH_TYPE, INT32 x, INT32 y, INT32 x_end, INT32 y_end);
RET_CODE ge_get_xy2(struct ge_device *dev, UINT32 cmd_pos, enum GE_DATA_PATH_TYPE, INT32 *px, INT32 *py, INT32 *px_end, INT32 *py_end);

RET_CODE ge_set_back_color(struct ge_device *dev, UINT32 cmd_pos, UINT32 color);
RET_CODE ge_set_font_color(struct ge_device *dev, UINT32 cmd_pos, UINT32 color);
RET_CODE ge_set_draw_color(struct ge_device *dev, UINT32 cmd_pos, UINT32 color);
RET_CODE ge_get_back_color(struct ge_device *dev, UINT32 cmd_pos, UINT32 *pcolor);
RET_CODE ge_get_font_color(struct ge_device *dev, UINT32 cmd_pos, UINT32 *pcolor);
RET_CODE ge_get_draw_color(struct ge_device *dev, UINT32 cmd_pos, UINT32 *pcolor);
RET_CODE ge_set_color_format(struct ge_device *dev, UINT32 cmd_pos, enum GE_PIXEL_FORMAT color_format);

RET_CODE ge_set_rop_mode(struct ge_device *dev, UINT32 cmd_pos, enum GE_ROP_MODE);
RET_CODE ge_set_bop_mode(struct ge_device *dev, UINT32 cmd_pos, enum GE_BOP_MODE);
RET_CODE ge_get_rop_mode(struct ge_device *dev, UINT32 cmd_pos, enum GE_ROP_MODE *);
RET_CODE ge_get_bop_mode(struct ge_device *dev, UINT32 cmd_pos, enum GE_BOP_MODE *);

RET_CODE ge_set_global_alpha(struct ge_device *dev, UINT32 cmd_pos, UINT32 value);
RET_CODE ge_set_global_alpha_mode(struct ge_device *dev, UINT32 cmd_pos, UINT32 mode);
RET_CODE ge_set_alpha_blend_mode(struct ge_device *dev, UINT32 cmd_pos, enum GE_ALPHA_BLEND_MODE);
RET_CODE ge_set_global_alpha_sel(struct ge_device *dev, UINT32 cmd_pos, enum GE_GLOBAL_ALPHA_SEL);
RET_CODE ge_set_alpha_out_mode(struct ge_device *dev, UINT32 cmd_pos, enum GE_ALPHA_OUT_MODE mode);
RET_CODE ge_set_dst_alpha_out(struct ge_device *dev, UINT32 cmd_pos, UINT32 value);
RET_CODE ge_set_global_alpha_layer(struct ge_device *dev, UINT32 cmd_pos, enum GE_GLOBAL_ALPHA_LAYER layer);
RET_CODE ge_set_bitmap_alpha_mode(struct ge_device *dev, UINT32 cmd_pos, enum GE_DATA_PATH_TYPE data_path, enum GE_BITMAP_ALPHA_MODE mode);
RET_CODE ge_get_global_alpha(struct ge_device *dev, UINT32 cmd_pos, UINT32 *pvalue);
RET_CODE ge_get_global_alpha_mode(struct ge_device *dev, UINT32 cmd_pos, UINT32 *pmode);
RET_CODE ge_get_global_alpha_sel(struct ge_device *dev, UINT32 cmd_pos, enum GE_GLOBAL_ALPHA_SEL *psel);
RET_CODE ge_get_alpha_out_mode(struct ge_device *dev, UINT32 cmd_pos, enum GE_ALPHA_OUT_MODE *pmode);
RET_CODE ge_get_dst_alpha_out(struct ge_device *dev, UINT32 cmd_pos, UINT32 *pvalue);

RET_CODE ge_set_colorkey_mode(struct ge_device *dev, UINT32 cmd_pos, enum GE_COLOR_KEY_MODE);
RET_CODE ge_set_colorkey_match_mode(struct ge_device *dev, UINT32 cmd_pos, enum GE_COLOR_KEY_CHANNEL, enum GE_COLOR_KEY_MATCH_MODE);
RET_CODE ge_set_colorkey_range(struct ge_device *dev, UINT32 cmd_pos, UINT32 low, UINT32 high);
RET_CODE ge_get_colorkey_mode(struct ge_device *dev, UINT32 cmd_pos, enum GE_COLOR_KEY_MODE *);
RET_CODE ge_get_colorkey_match_mode(struct ge_device *dev, UINT32 cmd_pos, enum GE_COLOR_KEY_CHANNEL, enum GE_COLOR_KEY_MATCH_MODE *);
RET_CODE ge_get_colorkey_range(struct ge_device *dev, UINT32 cmd_pos, UINT32 *plow, UINT32 *phigh);


// for SRC, PTN, and MSK
RET_CODE ge_set_byte_endian(struct ge_device *dev, UINT32 cmd_pos, enum GE_DATA_PATH_TYPE, enum GE_BYTE_ENDIAN byte_endian);
RET_CODE ge_set_subbyte_endian(struct ge_device *dev, UINT32 cmd_pos, enum GE_DATA_PATH_TYPE, enum GE_SUBBYTE_ENDIAN subbyte_endian);
RET_CODE ge_set_rgb_order(struct ge_device *dev, UINT32 cmd_pos, enum GE_DATA_PATH_TYPE, enum GE_RGB_ORDER rgb_order);
RET_CODE ge_set_alpha_attr(struct ge_device *dev, UINT32 cmd_pos, enum GE_DATA_PATH_TYPE, enum GE_ALPHA_RANGE alpha_range, enum GE_ALPHA_POLARITY alpha_pol);
RET_CODE ge_set_rgb_expansion(struct ge_device *dev, UINT32 cmd_pos, enum GE_DATA_PATH_TYPE, enum GE_RGB_COLOR_EXPANSION color_expan);

// Only for font PTN
RET_CODE ge_set_font_data_format(struct ge_device *dev, UINT32 cmd_pos, enum GE_FONT_DATA_FORMAT font_format);
RET_CODE ge_set_font_stretch(struct ge_device *dev, UINT32 cmd_pos, UINT32 stretch_h, UINT32 stretch_v);

// Only PTN can do CLUT operation
RET_CODE ge_set_clut_mode(struct ge_device *dev, UINT32 cmd_pos, enum GE_CLUT_MODE, BOOL color_cvt_enable);

// clut address must be 8bytes aligned(struct ge_device *dev, address[2:0] == 0)
RET_CODE ge_set_clut_addr(struct ge_device *dev, UINT32 cmd_pos, UINT32 clut_addr);

// ARGB == AYCbCr, BGRA = CrCbYA
RET_CODE ge_set_clut_rgb_order(struct ge_device *dev, UINT32 cmd_pos, enum GE_RGB_ORDER argb_order);
RET_CODE ge_set_clut_update(struct ge_device *dev, UINT32 cmd_pos, BOOL bUpdate);
RET_CODE ge_set_clut_color_cvt(struct ge_device *dev, UINT32 cmd_pos, enum GE_COLOR_CVT_SYSTEM color_btx, enum GE_COLOR_SPACE_MATRIX color_matrix);

// Only for DST
RET_CODE ge_set_clip_mode(struct ge_device *dev, UINT32 cmd_pos, enum GE_CLIP_MODE);
RET_CODE ge_set_clip_rect(struct ge_device *dev, UINT32 cmd_pos, UINT32 x, UINT32 y, UINT32 w, UINT32 h);

RET_CODE ge_enable_bitmask(struct ge_device *dev, UINT32 cmd_pos, BOOL bEnable);
RET_CODE ge_set_bitmask(struct ge_device *dev, UINT32 cmd_pos, UINT32 bitmask);

RET_CODE ge_enable_dither(struct ge_device *dev, UINT32 cmd_pos, BOOL bEnable);

// for SRC is overlapped with DST
RET_CODE ge_set_scan_order(struct ge_device *dev, UINT32 cmd_pos, enum GE_DATA_PATH_TYPE, UINT32 vscan_order, UINT32 hscan_order);
RET_CODE ge_set_scale_info(struct ge_device *dev, UINT32 cmd_pos, ge_scale_info_t *pscale_info);


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

#ifdef __cplusplus
}
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(_array)      (sizeof(_array)/sizeof(_array[0]))
#endif

#if 0
typedef struct 
{
	UINT16	width;		// width of input surface
	UINT16	height;		// height of surface to be created    
	INT32	pitch;		// distance to start of next line (return value only)
	UINT8 	surf_id;        //indicate HW or memory surface
	UINT8 	cur_region; //region id for current operation
} ge_surface_desc_t;
#endif

enum GE_INTERSECT_RESULT
{
	GE_INTERSECT_NULL,     // ra not intersect rb
	GE_INTERSECT_EQUAL,    // ra = rb
	GE_INTERSECT_SMALL,    // ra < rb
	GE_INTERSECT_BIG,      // ra > rb
	GE_INTERSECT_PART      // ra intersect rb
};

#if 0
enum GE_INTERSECT_RESULT ge_rect_intersect_rect(const ge_rect_t *ra, const ge_rect_t *rb, ge_rect_t *r);
BOOL ge_rect_is_inside_rect(const ge_rect_t *ra, const ge_rect_t *rb);
BOOL ge_rect_is_inside_size(const ge_rect_t *rect, UINT32 width, UINT32 height);
#endif

#endif //_GE_API_H_

