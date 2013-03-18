#ifndef _OSD_LIB_INTERNAL_H_
#define _OSD_LIB_INTERNAL_H_

#ifndef OSD_VSRC_MEM_MAX_SIZE
#define OSD_VSRC_MEM_MAX_SIZE	(30*1024)
#endif

#ifndef OSD_TRANSPARENT_COLOR_BYTE
#define OSD_TRANSPARENT_COLOR_BYTE 0xFF
#endif

#ifndef OSD_TRANSPARENT_COLOR
#define OSD_TRANSPARENT_COLOR	 0xFF
#endif

#ifndef FACTOR
#define FACTOR 0
#endif

#ifndef COLOR_N
#define COLOR_N 256
#endif

#ifndef BIT_PER_PIXEL
#define BIT_PER_PIXEL	8
#endif

#define	CIRCLE_PIX 5
					  
#if(OSD_VSCR_STRIDE==0)
	#define OSD_GET_VSCR_STIDE(pVscr)		( (pVscr)->vR.uWidth >> FACTOR)
	#define OSD_GET_VSCR_SIZE(pRect)		( (pRect)->uHeight * ( (pRect)->uWidth >> FACTOR) )
#else
	#define OSD_GET_VSCR_STIDE(pVscr)		( ( (pVscr)->vR.uWidth >> FACTOR) + 1)
	#define OSD_GET_VSCR_SIZE(pRect)		( (pRect)->uHeight * ( ( (pRect)->uWidth >> FACTOR) + 1) )
#endif
	
#endif//_OSD_LIB_INTERNAL_H_

