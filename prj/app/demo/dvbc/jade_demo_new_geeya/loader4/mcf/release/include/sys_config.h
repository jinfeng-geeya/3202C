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

/*****************************
*SDRAM mapping in bootloader
******************************
	________________________
	|					¡ý	| ¡ú0X82000000 (32M)
	|FRAME BUFF			       |
	|-----------------------| ¡ú0X81D00000 (29M)
	|					 	|						
	|CHUNK BUFF			¡ü	|						//#define SYS_FLASH_SIZE      0x800000
	|-----------------------| ¡ú0X81500000 (21M)	//a chunks image which maps from flash for program check, expand, and ota
	|						|
	|BOOTLOADER CODE	¡ü	|
	|-----------------------| ¡ú0X80A0000 (10M)		//bootloader code start addr
	|						|
	|MAIN CODE			¡ü	|	
	|-----------------------| ¡ú0X80000200			//MAIN_ENTRY: maincode start addr
	|_______________________|
*/

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

#define __MM_FB0_Y_LEN			0X65400	//405k
#define __MM_FB0_C_LEN			0X32C00	//203k
#define __MM_FB1_Y_LEN			0//0X65400
#define __MM_FB1_C_LEN			0//0X32C00
#define __MM_FB2_Y_LEN			0//0X65400
#define __MM_FB2_C_LEN			0//0X32C00

#define __MM_FB_TOP_ADDR		(__MM_BUF_TOP_ADDR)

#define __MM_FB0_Y_START_ADDR		((__MM_FB_TOP_ADDR - __MM_FB0_Y_LEN)&0XFFFFFF00)
#define __MM_FB0_C_START_ADDR		((__MM_FB0_Y_START_ADDR - __MM_FB0_C_LEN)&0XFFFFFF00)
#define __MM_FB1_Y_START_ADDR		((__MM_FB0_C_START_ADDR - __MM_FB1_Y_LEN)&0XFFFFFF00)
#define __MM_FB1_C_START_ADDR		((__MM_FB1_Y_START_ADDR - __MM_FB1_C_LEN)&0XFFFFFF00)
#define __MM_FB2_Y_START_ADDR		((__MM_FB1_C_START_ADDR - __MM_FB2_Y_LEN)&0XFFFFFF00)
#define __MM_FB2_C_START_ADDR		((__MM_FB2_Y_START_ADDR - __MM_FB2_C_LEN)&0XFFFFFF00)

#define __MM_MAF_LEN			0XC00
#define	__MM_EXTRA_LEN			0 //0x25200// not use now
#define __MM_DVW_LEN			0//0X25FA0 //only used in multi-view, can be 0 if no multi-view

#if (SYS_SDRAM_SIZE == 8)
	#define __MM_VBV_LEN	0X48000 //for 2+8 mode, if memory enough, 0x6D000 better
#else
	#define __MM_VBV_LEN	0X6D000
#endif

#define __MM_MAF_START_ADDR			((__MM_FB2_C_START_ADDR - __MM_MAF_LEN)&0XFFFFFC00)
#define __MM_DVW_START_ADDR			((__MM_MAF_START_ADDR - __MM_DVW_LEN)&0XFFFFFFF0)

#if(8==SYS_SDRAM_SIZE)
#define __MM_VBV_START_ADDR			((__MM_DVW_START_ADDR - __MM_VBV_LEN)&0XFFFFFF00)
#define __MM_FB_END_ADDR		(__MM_VBV_START_ADDR)
#else
#define __MM_FB_END_ADDR		(__MM_DVW_START_ADDR)
#endif

// this addr must larger than bootloader code start addr, refer to __RAM_BASE in ldscript_a.ld
#define __MM_HEAP_TOP_ADDR              __MM_FB_END_ADDR

#endif
