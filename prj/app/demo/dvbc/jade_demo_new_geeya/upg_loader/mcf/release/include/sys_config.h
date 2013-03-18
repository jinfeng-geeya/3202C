/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 Copyright (C)
*
*    File:    sys_config.h
*
*    Description:    This file contains all system configuration switches and
*						parameter definations.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*****************************************************************************/

#ifndef _SYS_CONFIG_H_
#define _SYS_CONFIG_H_

#include <sys_define.h>
#include "common_config.h"
//#include "autoconf.h"

/***********************************************************************
Memory map for hardware buffer related marco define
*************************************************************************/
#if(SYS_SDRAM_SIZE==8)
#define __MM_BUF_TOP_ADDR		0XA0800000
#elif(SYS_SDRAM_SIZE==16)
#define __MM_BUF_TOP_ADDR		0XA1000000
#elif(SYS_SDRAM_SIZE==32)
#define __MM_BUF_TOP_ADDR		0XA2000000
#elif(SYS_SDRAM_SIZE==64)
#define __MM_BUF_TOP_ADDR		0XA4000000 
#else
#define __MM_BUF_TOP_ADDR		0XA1000000  //For databroadcast,mem should bigger than 16M. 
#endif

/****************************************************************
//    >2MB -- Video Decoder buffer
*****************************************************************/
#define __MM_FB0_Y_LEN			0X65400	//405k
#define __MM_FB0_C_LEN			0X32C00	//203k
#define __MM_FB1_Y_LEN			0X65400
#define __MM_FB1_C_LEN			0X32C00
#define __MM_FB2_Y_LEN			0X65400
#define __MM_FB2_C_LEN			0X32C00

#define __MM_MAF_LEN			0XC00
#define	__MM_EXTRA_LEN			0 //0x25200// not use now
#define __MM_DVW_LEN			0//0X25FA0 //only used in multi-view, can be 0 if no multi-view

#if (SYS_SDRAM_SIZE == 8)
	#define __MM_VBV_LEN	0X48000 //for 2+8 mode, if memory enough, 0x6D000 better
#else
	#define __MM_VBV_LEN	0X6D000
#endif

#define __MM_FB_TOP_ADDR		(__MM_BUF_TOP_ADDR)

#define __MM_FB0_Y_START_ADDR		((__MM_FB_TOP_ADDR - __MM_FB0_Y_LEN)&0XFFFFFF00)
#define __MM_FB0_C_START_ADDR		((__MM_FB0_Y_START_ADDR - __MM_FB0_C_LEN)&0XFFFFFF00)
#define __MM_FB1_Y_START_ADDR		((__MM_FB0_C_START_ADDR - __MM_FB1_Y_LEN)&0XFFFFFF00)
#define __MM_FB1_C_START_ADDR		((__MM_FB1_Y_START_ADDR - __MM_FB1_C_LEN)&0XFFFFFF00)
#define __MM_FB2_Y_START_ADDR		((__MM_FB1_C_START_ADDR - __MM_FB2_Y_LEN)&0XFFFFFF00)
#define __MM_FB2_C_START_ADDR		((__MM_FB2_Y_START_ADDR - __MM_FB2_C_LEN)&0XFFFFFF00)

#define __MM_MAF_START_ADDR			((__MM_FB2_C_START_ADDR - __MM_MAF_LEN)&0XFFFFFC00)
#define __MM_DVW_START_ADDR			((__MM_MAF_START_ADDR - __MM_DVW_LEN)&0XFFFFFFF0)

/*VBV buffer must < 16M, for 32M solution will malloc VBV buffer*/
#if(8==SYS_SDRAM_SIZE)
#define __MM_VBV_START_ADDR			((__MM_DVW_START_ADDR - __MM_VBV_LEN)&0XFFFFFF00)
#define __MM_FB_END_ADDR		(__MM_VBV_START_ADDR)
#else
#define __MM_FB_END_ADDR		(__MM_DVW_START_ADDR)
#endif
/*********************** end Video Decoder buffer******************************/

/****************************************************************
    16MB -- Device Buffer: DMX, GE, OSD, LWIP, etc.
*****************************************************************/
#if(SYS_SDRAM_SIZE>=16)
#define __MM_DMX_SI_LEN			(80*188)
#else
#define __MM_DMX_SI_LEN			(80*188) ////if memory enough, (20*188) better
#endif
#define __MM_DMX_SI_TOTAL_LEN	(__MM_DMX_SI_LEN*28)
#define __MM_SI_VBV_OFFSET		__MM_DMX_SI_TOTAL_LEN
#define __MM_DMX_DATA_LEN		(30*188)
#define __MM_DMX_PCR_LEN		(10*188)
#if(SYS_SDRAM_SIZE>=16)
#define __MM_DMX_AUDIO_LEN		(256*188)
#define __MM_DMX_VIDEO_LEN		(1024*188)
#else
#define __MM_DMX_AUDIO_LEN		(32*188)
#define __MM_DMX_VIDEO_LEN		(384*188) //if memory enough, (1024*188) better
#endif
#define __MM_DMX_AVP_LEN		(__MM_DMX_VIDEO_LEN+__MM_DMX_AUDIO_LEN+__MM_DMX_PCR_LEN)
#define __MM_DMX_REC_LEN		0

/*osd layer buf len should be modified according to osd region width,hight and bytes per pixel*/
#if (SYS_SDRAM_SIZE == 8)
	#define __MM_OSD_LAYER0_LEN	0X68100  //for 2+8 mode,  720*(28+28)*2+720*240*2,16bit
#else
	#define __MM_OSD_LAYER0_LEN	0XCA800  //720*576*2,16bit
#endif

#define __MM_OSD_LAYER1_LEN	0  //720*576,clut8
#define __MM_GE_CMD_LEN		0x1000
#define __MM_OSD_BLK_LEN		0x1200 // 0X480

#if(SYS_SDRAM_SIZE>=32)
#define __MM_LWIP_MEMP_LEN		(1532*256)//Lwcp need this buffer.Set to 0 if lwcp don't need.
#else
#define __MM_LWIP_MEMP_LEN		0
#endif

#define __MM_DEV_TOP_ADDR		(__MM_FB_END_ADDR)

/* DMX buffer note: can not stride 8M */
#define __MM_DMX_AVP_START_ADDR		((__MM_DEV_TOP_ADDR - __MM_SI_VBV_OFFSET - __MM_DMX_DATA_LEN - __MM_DMX_AVP_LEN)&0XFFFFFFFC)
#define __MM_DMX_REC_START_ADDR		((__MM_DMX_AVP_START_ADDR - __MM_DMX_REC_LEN)&0XFFFFFFFC)

#define __MM_OSD_START_ADDR			((__MM_DMX_REC_START_ADDR - __MM_OSD_LAYER0_LEN -__MM_OSD_LAYER1_LEN - __MM_GE_CMD_LEN)&0XFFFFFFF0)
#define __MM_OSD_BLOCK_ADDR			((__MM_OSD_START_ADDR - __MM_OSD_BLK_LEN)&0XFFFFFFF0)

#define __MM_LWIP_MEMP_ADDR         (__MM_OSD_BLOCK_ADDR - __MM_LWIP_MEMP_LEN)

#define __MM_DEV_END_ADDR		(__MM_LWIP_MEMP_ADDR)

/****************************************************************
    UPG buffer
*****************************************************************/
#if(SYS_FLASH_SIZE==0x200000)
#define __MM_UPG_COMPRESSED_BUFFER_LEN	(2*1024*1024)//(8*1024*1024)//
#define __MM_UPG_UNCOMPRESSED_BUFFER_LEN	(2*1024*1024)//(8*1024*1024)//
#define __MM_UPG_IMAGE_BUFFER_LEN	(2*1024*1024)//(8*1024*1024)//
#elif(SYS_FLASH_SIZE==0x400000)
#define __MM_UPG_COMPRESSED_BUFFER_LEN	(4*1024*1024)//(8*1024*1024)//
#define __MM_UPG_UNCOMPRESSED_BUFFER_LEN	(4*1024*1024)//(8*1024*1024)//
#define __MM_UPG_IMAGE_BUFFER_LEN	(4*1024*1024)//(8*1024*1024)//
#elif(SYS_FLASH_SIZE==0x800000)
#define __MM_UPG_COMPRESSED_BUFFER_LEN	(8*1024*1024)//(8*1024*1024)//
#define __MM_UPG_UNCOMPRESSED_BUFFER_LEN	(8*1024*1024)//(8*1024*1024)//
#define __MM_UPG_IMAGE_BUFFER_LEN	(8*1024*1024)//(8*1024*1024)//
#endif 

#define __MM_UPG_TOP_ADDR		(__MM_DEV_END_ADDR)

#define __MM_UPG_COMPRESSED_BUF_ADDR	(__MM_UPG_TOP_ADDR - __MM_UPG_COMPRESSED_BUFFER_LEN)
#define __MM_UPG_IMAGE_BUF_ADDR	 __MM_UPG_COMPRESSED_BUF_ADDR//use the same buffer as compressed_buf  (__MM_UPG_COMPRESSED_BUF_ADDR - __MM_UPG_IMAGE_BUFFER_LEN)//
#define __MM_UPG_UNCOMPRESSED_BUF_ADDR	(__MM_UPG_IMAGE_BUF_ADDR - __MM_UPG_UNCOMPRESSED_BUFFER_LEN)

#define __MM_UPG_END_ADDR     __MM_UPG_UNCOMPRESSED_BUF_ADDR 

/****************************************************************
    HEAP TOP
*****************************************************************/
#define __MM_HEAP_TOP_ADDR			__MM_UPG_END_ADDR


#if (256 == COLOR_N)	
#define	BIT_PER_PIXEL		8
#define 	OSD_TRANSPARENT_COLOR		0xFF
#define 	OSD_TRANSPARENT_COLOR_BYTE 	0xFF
#define	IF_GLOBAL_ALPHA	FALSE
#elif (16 == COLOR_N)	
#define	BIT_PER_PIXEL		4
#define 	OSD_TRANSPARENT_COLOR		15
#define 	OSD_TRANSPARENT_COLOR_BYTE 	0xFF
#define	IF_GLOBAL_ALPHA	FALSE
#elif(4 == COLOR_N)	
#define	BIT_PER_PIXEL		2
#define 	OSD_TRANSPARENT_COLOR		3
#define 	OSD_TRANSPARENT_COLOR_BYTE 0xFF
#define	IF_GLOBAL_ALPHA	TRUE
#endif

#if		(BIT_PER_PIXEL	== 2)
#define	FACTOR					2					
#elif	(BIT_PER_PIXEL    == 4)
#define	FACTOR					1					
#elif	(BIT_PER_PIXEL     == 8)
#define	FACTOR					0					
#endif

#endif
