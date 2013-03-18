/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 Copyright (C)
*
*    File:    hal_mem.h
*
*    Description:    This file contains all functions about memory interface.
*					 Current suport M6303, M3325 Flash port only.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	Feb.13.2003       Justin Wu       Ver 0.1    Create file.
*	2.
*****************************************************************************/

#ifndef	__HAL_MEM_H__
#define __HAL_MEM_H__

#include <types.h>
#include <sys_config.h>


/* HAL_ROM_MODE_SET() param:
 * sw:	Write operation speed;
 * sr:	Read operation speed;
 * wr:	Bus write / read mode;
 * wd:	Bus width;
 */

#if ((SYS_CHIP_MODULE == ALI_M3327) || (SYS_CHIP_MODULE==ALI_M3327C)||(SYS_CHIP_MODULE==ALI_M3329E))
/* Flash operate mode in M3327:
 * bit0: 			1 for flash port write enable, 0 for disable;
 * bit1:			0 for flash port access enable, 1 for disable;
 * bit2:			1 for HW control enable, 0 for SW control enable;
 * bit20 - bit16: 	Flash operation cycle value in clock of SDRAM_clock;
 * others: Reserved
 */
#define HAL_ROM_MODE_SET(sw, sr, wr, wd)	\
	do										\
	{										\
		 if(ALI_M3329E==sys_ic_get_chip_id()&&sys_ic_get_rev_id()>=IC_REV_5)\
			*(UINT32 *)0xb8008090 = (((sw) << 16) | (wr & 1) | 0x04|(0x4<<24));\
		else																\
			*(UINT32 *)0xb8000090 = (((sw) << 16) | (wr & 1) | 0x04|(0x4<<24));\
	}while(0)

#define HAL_ROM_BUS_WIDTH_GET()				8

#elif(SYS_CHIP_MODULE == ALI_S3602)
#define HAL_ROM_MODE_SET(sw, sr, wr, wd)	\
	do										\
	{										\
		if(ALI_M3101 == sys_ic_get_chip_id() && (sys_ic_get_rev_id()>=IC_REV_6))\
			*(UINT32 *)0xb8000090 = (((sw) << 16) | (wr & 1) | 0x04);\
	}while(0)
	
#define HAL_ROM_BUS_WIDTH_GET()				8

#else
#define HAL_ROM_MODE_SET(...)
#define HAL_ROM_BUS_WIDTH_GET()				8

#endif

#endif /* __HAL_MEM_H__ */
