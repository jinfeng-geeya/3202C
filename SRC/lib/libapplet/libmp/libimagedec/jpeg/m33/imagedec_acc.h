
#ifndef __JH_M3329E_ACCELERATOR__
#define __JH_M3329E_ACCELERATOR__

#include <api/libimagedec/imagedec.h>
#include "imagedec_main.h"

//jpeg HW accumulator hal operation
#define JPEG_HW_ACUM_REG_BASE_29E 	0xB8004200
#define JPEG_HW_ACUM_REG_BASE_29E5 	0xb8013200 

#define JPEG_HW_VLD_MIN_DATA_SIZE	0x1000

#define JPEG_HW_MAX_MCU_VALUE		1024

//register offset
#define JPEG_HW_ACUM_CONTROL_29E		0x10
#define JPEG_HW_ACUM_START_29E		0x14
#define JPEG_HW_ACUM_STATUS_29E   	0x18
#define JPEG_HW_VBV_START_ADDR		0X24
#define JPEG_HW_VBV_OFFSET			0X28
#define JPEG_HW_VBV_VLD_LENGTH		0X2C
#define JPEG_HW_VBV_END_ADDR			0X30
#define JPEG_HW_ACUM_MEM_BASE_29E 	0x78
#define JPEG_HW_ACUM_VLD_HUF_TAB_29E	0x7C
#define JPEG_HW_ACUM_QUT_INDEX_29E	0x80
#define JPEG_HW_ACUM_EXTRA_FUN_29E	0xA0
#define JPEG_HW_ACUM_EXTRA_COT_29E	0xA4
#define JPEG_HW_ACUM_INPUT_29E		0xB0

//jpeg hw extra function index
#define JPEG_HW_EXT_MEM_BASE_Y		0x00
#define JPEG_HW_EXT_MEM_BASE_C		0x01	
#define JPEG_HW_EXT_CONTROL			0x02
#define JPEG_HW_EXT_MCU_CORD			0x03

#define JPEG_HW_VLD_FIRST				0x04
#define JPEG_HW_VLD_LAST				0x02
#define JPEG_HW_VLD_VALID				0x01

//config parameter
#define JPEG_HW_MAX_SCALED_WIDTH  720
#define JPEG_HW_MAX_SCALED_WIDTH2 896
#define JPEG_HW_MAX_SCALED_HEIGHT 900
#define JPEG_HW_DEC_BUF_STRIDE	  720

#define JPEG_HW_MAX_SCALED_SIZE_EXT2  1440

#define JPEG_HW_MacroBlock_NUM	1 // 16x row for one sw scale operation

#define JPEG_HW_MAX_SCALED_WIDTH_EXT  (JPGD_MAX_WIDTH>>3)
#define JPEG_HW_MAX_SCALED_HEIGHT_EXT (JPGD_MAX_HEIGHT>>3)

#define JPEG_HW_MIN_OUT_WIDTH_THU 480
#define JPEG_HW_MIN_OUT_HEIGHT_THU 360

inline void jh_reg_mcu_coord_y(UINT32 y_mcu);
inline void jh_reg_mcu_coord_x(UINT32 x_mcu);
inline UINT8 jh_hw_jpeg_busy(void);
inline UINT8 jh_hw_iqis_buf_full(void);
int jh_hw_init(pImagedec_hdl phdl);
inline void jh_input_rlc_data(UINT32 run, int level);
inline void jh_reset_dc_value(void);
inline void jh_mcu_start(void);
inline void jh_unit_start(void);
#endif
