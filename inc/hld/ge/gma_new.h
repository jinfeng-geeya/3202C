/*
  Important:
    This header file(GMA API) is a part of GE API and will be included in ge.h,
    application please don't include this file directly.
*/

#ifndef _GMA_NEW_H_
#define _GMA_NEW_H_

RET_CODE ge_gma_show_onoff(struct ge_device *dev, UINT32 layer_id, BOOL bOnOff);
RET_CODE ge_gma_scale(struct ge_device *dev, UINT32 layer_id, UINT32 scale_cmd, UINT32 scale_param);

RET_CODE ge_gma_set_pallette(struct ge_device *, UINT32 layer_id, const UINT8 *pal, UINT16 color_num, const ge_pal_attr_t *pattr);
RET_CODE ge_gma_get_pallette(struct ge_device *, UINT32 layer_id, UINT8 *pal,       UINT16 color_num, const ge_pal_attr_t *pattr);
RET_CODE ge_gma_modify_pallette(struct ge_device *, UINT32 layer_id, UINT8 uIndex, UINT8 alpha, UINT8 uY, UINT8 uCb, UINT8 uCr, const ge_pal_attr_t *pattr);
//RET_CODE ge_gma_modify_pallette(struct ge_device *, UINT32 layer_id, UINT8 uIndex, UINT8 alpha, UINT8 uR, UINT8 uG, UINT8 uB, const ge_pal_attr_t *pattr);

/*
   ge_gma_create_region(): bitmap_addr and pixel_pitch determines the region bitmap address, total 4 cases:

   Case 1: if bitmap_addr is 0, and pixel_pitch is 0, it will use region_w as pixel_pitch,
       and region bitmap addr will be allocated from uMemBase dynamically.
   Case 2: if bitmap_addr is 0, and pixel_pitch is not 0, the region bitmap addr will be fixed:
       uMemBase + (pixel_pitch * bitmap_y + bitmap_x) * byte_per_pixel

   Case 3: if bitmap_addr is not 0, and pixel_pitch is 0, the region bitmap addr will be:
       bitmap_addr + (bitmap_w * bitmap_y + bitmap_x) * byte_per_pixel
   Case 4: if bitmap_addr is not 0, and pixel_pitch is not 0, the region bitmap addr will be:
       bitmap_addr + (pixel_pitch * bitmap_y + bitmap_x) * byte_per_pixel
*/
RET_CODE ge_gma_create_region(struct ge_device *, UINT32 layer_id, UINT32 region_id, const ge_gma_region_t *pregion_param);
RET_CODE ge_gma_delete_region(struct ge_device *, UINT32 layer_id, UINT32 region_id);

/*
   ge_gma_move_region(): region using internal memory can only change region_x, region_y, pal_sel;
   ge_gma_move_region(): region using external memory can change everyting in ge_gma_region_t;
*/
RET_CODE ge_gma_move_region(struct ge_device *, UINT32 layer_id, UINT32 region_id, const ge_gma_region_t *pregion_param);
RET_CODE ge_gma_show_region(struct ge_device *, UINT32 layer_id, UINT32 region_id, BOOL bOnOff);
RET_CODE ge_gma_get_region_info(struct ge_device *, UINT32 layer_id, UINT32 region_id, ge_gma_region_t *pregion_param);

RET_CODE ge_gma_set_region_clip_mode(struct ge_device *, UINT32 layer_id, enum GE_CLIP_MODE clip_mode);
RET_CODE ge_gma_set_region_clip_rect(struct ge_device *, UINT32 layer_id, UINT32 clip_x, UINT32 clip_y, UINT32 clip_w, UINT32 clip_h);

RET_CODE ge_gma_set_region_to_cmd_list(struct ge_device *, UINT32 layer_id, UINT32 region_id, ge_cmd_list_hdl cmd_list);

#if 0
UINT8 gma_get_byte_per_pixel(UINT8 color_mode);
UINT16 gma_get_pitch(UINT8 color_mode, UINT16 width);
#define gma_get_pitch_by_color_mode gma_get_pitch
#endif

#endif //_GMA_API_H_

