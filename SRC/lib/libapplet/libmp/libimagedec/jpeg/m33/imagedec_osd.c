
#include <basic_types.h>
#include <mediatypes.h>
#include <os/tds2/itron.h>
#include <api/libc/printf.h>
#include <osal/osal_timer.h>

#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <api/libimagedec/imagedec.h>
#include "imagedec_main.h"
#include "imagedec_osd.h"

//#define OSD_TEST_32BITS
extern Imagedec_hdl g_imagedec_hdl[IMAGEDEC_MAX_INSTANCE];
extern imagedec_osd_ins g_imagedec_osd_ins;

static INT32 Y = 0, Cb = 0, Cr = 0;
static INT32 Y_2 = 0,Cr_R = 0, Cr_G = 0, Cb_G = 0, Cb_B = 0;
static INT32  Red = 0, Green = 0, Blue = 0;
static UINT16  RGB; //16bits
#if 0
IMAGEDEC_INLINE void YUVTORGB()
{
	Red = Y + ((Cr * YUVTOR_PAR)>>16);
	Green = Y - ((Cr * YUVTOG_PAR_R)>>16) - ((Cb * YUVTOG_PAR_U)>>16);
	Blue = Y + ((Cb * YUVTOB_PAR)>>16);
	Red = Red < 0 ? 0 :(Red > 0xFF ? 0xFF : Red);
	Green = Green < 0 ? 0 :(Green > 0xFF ? 0xFF : Green);
	Blue = Blue < 0 ? 0 :(Blue > 0xFF ? 0xFF : Blue);
	RGB = (UINT16)(0x8000 | ((Red<<7) & 0x7C00) | ((Green<<2) & 0x3E0)  | (Blue>>3));	
}
#endif
#define YUVTORGB_TRANSFER_YUV	{		\
		Y = Y < 16 ? 16 : (Y > 235 ? 235 : Y);	\
		Y -= 16;	\
		Y_2 = (Y * YUVTOY_PAR)>>16;	\
		Cb = Cb < 16 ? 16 : (Cb > 240 ? 240 : Cb);	 \
		Cr = Cr < 16 ? 16 : (Cr > 240 ? 240 : Cr);	\
		Cb -= 128;	\
		Cr -= 128;	\
		Cr_R = (Cr * YUVTOR_PAR)>>16;	\
		Cr_G = (Cr * YUVTOG_PAR_R)>>16;	\
		Cb_G = (Cb * YUVTOG_PAR_U)>>16;	\
		Cb_B = (Cb * YUVTOB_PAR)>>16;	\
	}

#define YUVTORGB_GEN_RGB		{		\
		Red = Y + Cr_R;	\
		Green = Y - Cr_G - Cb_G;	\
		Blue = Y + Cb_B;	\
		Red = Red < 0 ? 0 :(Red > 0xFF ? 0xFF : Red);		\
		Green = Green < 0 ? 0 :(Green > 0xFF ? 0xFF : Green);		\
		Blue = Blue < 0 ? 0 :(Blue > 0xFF ? 0xFF : Blue);		\
		RGB = (UINT16)(0x8000 | ((Red>>3)<<10) | ((Green>>3)<<5)  | (Blue>>3));\
	}

static void update(void *ins)
{
	pImagedec_hdl phdl = &g_imagedec_hdl[0];
	Imagedec_Out_Image image;
	pimagedec_osd_out pout = &(((pimagedec_osd_ins)ins)->out);
	pimagedec_osd_raw_pic ppic = &(((pimagedec_osd_ins)ins)->pic);
	UINT8 *y_buf = ppic->y_buf;
	UINT8 *c_buf = ppic->c_buf;
#ifdef OSD_TEST_32BITS
	UINT32 *bitmap = (UINT32 *)((pout->bitmap + 3) & 0xFFFFFFFC);
#else
	UINT16 *bitmap = (UINT16 *)((pout->bitmap + 1) & 0xFFFFFFFE);
#endif
	UINT32 y_line = 0, c_line = 0;
	UINT32 bit_line = 0;
	UINT16 w = 0, h = 0;
	UINT16 i = 0,j = 0;
	UINT8 flag = 0;

	UINT32 MB_LINE_SIZE = 0;

	MEMSET((void *)&image,0,sizeof(Imagedec_Out_Image));

	pout->bitmap = (UINT32)bitmap;

	if((pout->stride * ppic->height) > pout->buf_size)
	{
		jpeg_printf("Don't have enough bitmap buf\n");
		terminate(JPGD_OSD_NO_ENOUGH_BITMAP_BUF);
	}
	
	w = (ppic->width < pout->dis_rect.uWidth) ? ppic->width : pout->dis_rect.uWidth;
	h = (ppic->height < pout->dis_rect.uHeight) ? ppic->height : pout->dis_rect.uHeight;

	w &= ~1;
	h &= ~1;
	
	pout->dis_rect.uWidth = w;
	pout->dis_rect.uHeight = h;

#ifdef OSD_TEST_32BITS
	pout->stride = pout->dis_rect.uWidth<<2;	
#else
	pout->stride = pout->dis_rect.uWidth<<1;
#endif

	if((0 == w) || (0 == h))
		terminate(JPGD_OSD_SHOW_RECT_FAIL);

	MEMSET((void *)bitmap,0,pout->buf_size);
	
	MB_LINE_SIZE = (ppic->stride<<4) - 256;

	y_line = (UINT32)y_buf;
	c_line = (UINT32)c_buf;
	bit_line = (UINT32)bitmap;
	// osal_cache_flush((void *)y_buf,ppic->y_size);
	// osal_cache_flush((void *)c_buf,ppic->c_size);
	for(i = 0;i < h;i++)
	{
		for(j = 0;j < w;j++)
		{
			Y = (INT32)*y_buf++;
			if(0xF == (j &0xF))
				y_buf += 240;
			
			if(0 == (j & 0x1))
			{
				if(0xE == (j & 0xF))
					flag = 1;
				else
					flag = 0;
				Cb = (INT32)*c_buf++;
				Cr = (INT32)*c_buf++;			
				if(flag)
					c_buf += 240;
				
#ifdef OSD_TEST_32BITS

#else
				YUVTORGB_TRANSFER_YUV;	
#endif				
			}
#ifdef OSD_TEST_32BITS

#else
			YUVTORGB_GEN_RGB;
#endif			

#ifdef OSD_TEST_32BITS
			*bitmap++ = 0xFF000000 | (Cr<<16) | (Y<<8) | Cb;
#else
			*bitmap++ = RGB;
#endif
		}

		bit_line += pout->stride;

#ifdef OSD_TEST_32BITS
		bitmap = (UINT32 *)bit_line;
#else
		bitmap = (UINT16 *)bit_line;
#endif

		y_line += 16;
		if(1 == (i & 0x1))
		{
			c_line += 16;
			if(0xF == (i & 0xF))
			{
				y_line += MB_LINE_SIZE;// - 256;
				if(0x1F == (i & 0x1F))
					c_line += MB_LINE_SIZE;// - 256;				
			}
		}
		y_buf = (UINT8 *)y_line;
		c_buf = (UINT8 *)c_line;
	}

	// output the image by the blt call back function. only support 16Bits OSD
	image.bpp = 16;
	image.start_buf = pout->bitmap;
	image.stride = pout->stride;
	image.width = w;
	image.height = h;
	jpeg_printf("osd update: buf %x stride %d width %d height %d\n",image.start_buf
		,image.stride,image.width,image.height);	

	if(NULL != pout->blt)
		pout->blt(pout->handle,&image,&pout->dis_rect);
}
	
void imagedec_osd_init(pImagedec_Osd_Config config)
{
	MEMSET((void *)&g_imagedec_osd_ins,0,sizeof(imagedec_osd_ins));
	
	g_imagedec_osd_ins.out.handle= config->handle;
	g_imagedec_osd_ins.out.frmt = config->pic_frmt;
	g_imagedec_osd_ins.out.blt = config->blt;
	g_imagedec_osd_ins.update_osd = update;
}

