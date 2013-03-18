#ifndef __IMAGEOSD_HEADER
#define __IMAGEOSD_HEADER

#include <api/libimagedec/imagedec.h>
#include <mediatypes.h>

#define MAX_WIDTH_BY_OSD			(720)	
#define MAX_HEIGHT_BY_OSD			(576)

// paramter of yuv to rgb porcess 
#define 	YUVTOY_PAR				0x129FB// 1.164
#define	YUVTOR_PAR				0x19893// 1.596
#define 	YUVTOG_PAR_U			0x6418// 0.391
#define 	YUVTOG_PAR_R			0xd020// 0.813
#define	YUVTOB_PAR				0x2049B// 2.018

typedef struct imagedec_api_cmd_t
{
	UINT32 on:1;
	UINT32 res:31;

	struct Rect dis_rect;
	UINT32 bitmap;
	UINT32 buf_size;
}imagedec_api_cmd,*pimagedec_api_cmd;

typedef struct imagedec_osd_raw_pic_t
{
	UINT8 *y_buf;
	UINT8 *c_buf;

	UINT32 y_size;
	UINT32 c_size;
	
	UINT16 width;
	UINT16 height; 	
	UINT32 stride;
}imagedec_osd_raw_pic,*pimagedec_osd_raw_pic;

typedef struct imagedec_osd_out_t
{
	void *handle;
	UINT32 frmt;
	void (*blt)(void *,pImagedec_Out_Image,struct Rect *);
	struct Rect dis_rect;

	UINT32 bitmap;
	UINT32 buf_size;	
	UINT32 stride;
}imagedec_osd_out,*pimagedec_osd_out;

typedef struct imagedec_osd_ins_t
{
	UINT32 on:1;
	UINT32 res:31;
	
	imagedec_api_cmd api_cmd;
	Imagedec_Out_Image image;
	
	imagedec_osd_raw_pic pic;
	imagedec_osd_out out;
	void (*update_osd)(void *);
}imagedec_osd_ins,*pimagedec_osd_ins;

#endif

