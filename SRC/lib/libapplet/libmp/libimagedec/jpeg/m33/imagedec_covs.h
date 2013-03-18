#ifndef __JPEG_HW_M3329E_CONVS__
#define __JPEG_HW_M3329E_CONVS__

#include <api/libimagedec/imagedec.h>
#include <mediatypes.h>

#if	0
#define JHCON_PRINTF soc_printf
#define	JHCON_ASSERT(...)			SDBBP()
#else
#define JHCON_PRINTF(...) do{}while(0)
#define	JHCON_ASSERT(...) do{}while(0)
#endif
#define JHCONVS_INLINE static inline

#define JHCONVS_MAX_OUT_WIDTH 720
#define JHCONVS_MAX_OUT_HEIGHT 576 

#define JHCONVS_MIN_OUT_WIDTH_THU 80
#define JHCONVS_MIN_OUT_HEIGHT_THU 60

struct jhconvs_mode
{
	enum IMAGE_ANGLE angle;
	UINT32 dis_mode;
	UINT32 res_bits;
	UINT32 res_pot;
};

struct jhconvs_rect
{
	UINT32 x;
	UINT32 y;
	UINT32 w;
	UINT32 h;
};

enum jhconvs_chro_format
{
	JHCONVS_420,
	JHCONVS_422,
	JHCONVS_411
};

struct jhconvs_image_info
{	
	UINT32 y_addr;
	UINT32 c_addr;
	UINT32 stride;
	struct jhconvs_rect rect;
	enum jhconvs_chro_format format;
};

struct jhconvs_control
{
	UINT32 res_bits;
	UINT32 res_pot;
};
struct jhconvs_main
{
	struct jhconvs_mode mode;
	struct jhconvs_control control;
	struct jhconvs_image_info src_info;
	struct jhconvs_image_info dst_info;
	void (*y_convert_op)(struct jhconvs_main *);
	void (*c_convert_op)(struct jhconvs_main *);
};

#endif


