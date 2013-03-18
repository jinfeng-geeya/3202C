/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2008 Copyright (C)
 *
 *  File: dvt_sc.c
 *
 *  Description: This file fills the NVM related API of DVT CA library.
 *
 *  History:
 *           Date            Author         Version       Comment
 *      ========    ======      ======     =========
 *  1.  2008.01.22      Grady Xu       0.1.000       Create this file
 *
 ****************************************************************************/
#include <sys_config.h>
#include <types.h>
#include <retcode.h>
#include <hld/hld_dev.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/alloc.h>
#include <api/libc/string.h>
#include <hld/sto/sto.h>
#include <hld/sto/sto_dev.h>
#include <api/libchunk/chunk.h>

#include <api/libcas/gy/cas_gy.h>
#include "gyca_nvm.h"

static UINT32 CAS_flash_start_addr = 0;
static UINT8 *cas_flash_buff = NULL;
static UINT32 cas_flashwrite_time = 0;
static UINT8  cas_flashwrite_enable = FALSE;



/***********************************************************************************************
** 	函数名称:GYCA_GetBuffer
************************************************************************************************
**	提 供 者：  	机顶盒。
**	使 用 者：	CA模块。
**	功能描述：	获得机顶盒提供给CA模块保存信息的起始地址和尺寸。
**	输入参数：无
**	输出参数：	lStartAddr   分配空间的起始地址
**                                lSize          分配空间的大小
**	返 回 值：	成功GYTRUE  失败 GYFALSE
**	其他说明：
**                                推荐机顶盒厂商分配不少于2K的EEPROM空间
*************************************************************************************************/

GYS32 GYCA_GetBuffer (GYS32 *lStartAddr, GYS32* lSize)
{

#if 1
	UINT32 chunk_id = CAS_CHUNK_ID;
	CHUNK_HEADER chuck_hdr;
	struct sto_device *sto_flash_dev = NULL;

	/* get table info base addr by using chunk */
	if(sto_get_chunk_header(chunk_id, &chuck_hdr) == 0)
	{
		GYCASDRV_PRINTF("Err: %s get chunk header fail!\n", __FUNCTION__);
		return GYFAILURE;
	}
	
	CAS_flash_start_addr = sto_chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
	CAS_flash_start_addr += CHUNK_HEADER_SIZE;

	*lStartAddr = CAS_flash_start_addr;
	*lSize = CAS_FLASH_MAXSIZE;//CAS_CHUNK_SIZE/2;

	if (NULL == cas_flash_buff)
		cas_flash_buff = MALLOC(*lSize);
	if (NULL == cas_flash_buff)
		return GYFAILURE;

	//copy the CA flash data to mem
	sto_flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
	if(NULL == sto_flash_dev)
	{
		GYCASDRV_PRINTF("TFSTBCA_WriteBuffer: dev_get_by_name failed!\n");
		return GYFAILURE;
	}
	if (sto_open(sto_flash_dev) != SUCCESS)
	{
		GYCASDRV_PRINTF("%s: sto_open failed!\n", __FUNCTION__);
		return GYFAILURE;
	}
	if( (INT32) CAS_flash_start_addr  != sto_lseek(sto_flash_dev, (INT32) CAS_flash_start_addr, STO_LSEEK_SET))
	{
		GYCASDRV_PRINTF("%s: lseek failed!\n", __FUNCTION__);
		return GYFAILURE;
	}
	if (CAS_FLASH_MAXSIZE  != sto_read(sto_flash_dev, cas_flash_buff, CAS_FLASH_MAXSIZE))
	{
		GYCASDRV_PRINTF("%s: sto_read failed!\n", __FUNCTION__);
		return GYFAILURE;
	}
		
	GYCASDRV_PRINTF("%s: flash StartAddr=%08x, Len:%08x\n", __FUNCTION__, *lStartAddr, *lSize);
	return GYSUCCESS;
#endif	
}

/*************************************************************************************************
** 	函数名称:GYCA_ReadBuffer
**************************************************************************************************
**	提 供 者：  	机顶盒。
**	使 用 者：	CA模块。
**	功能描述：从指定的地址读出指定数量字节放在指定的空间中
**	输入参数：	pStartAddr：		要读取数据的存储空间的地址。
**	 				pbData：			被读取数据的存放地址指针。
**	 				pBytes：		       要读的数据的长度
**	输出参数：	pbData：			被读出的数据。
**	返 回 值：	GYTRUE 成功， GYFALSE 失败。
**	其他说明：
**************************************************************************************************/
GYS32 GYCA_ReadBuffer(const GYS32 lStartAddr, GYU8* pData, GYS32* nLen)
{

	INT32 ret;
	UINT32 offset = 0;

	GYCASDRV_PRINTF("%s: lStartAddr=%08x, * nLen:%08x\n", __FUNCTION__, lStartAddr, *nLen);
	if((lStartAddr == NULL)||(pData == NULL)||(*nLen == 0))
	{
		GYCASDRV_PRINTF("Err: %s parameter error!\n", __FUNCTION__);
		return GYFAILURE;
	}
	if(((UINT32)lStartAddr-CAS_flash_start_addr+*nLen)> CAS_FLASH_MAXSIZE)
	{
		GYCASDRV_PRINTF("Err: %s overflow!\n", __FUNCTION__);
		*nLen = CAS_FLASH_MAXSIZE;
	}
	
	offset= (UINT32)lStartAddr-CAS_flash_start_addr;
	MEMCPY(pData, &cas_flash_buff[offset], *nLen);
	return SUCCESS;
}

/*************************************************************************************************
** 	函数名称:GYCA_WriteBuffer
**************************************************************************************************
**	提 供 者：	机顶盒。
**	使 用 者：	CA模块。
**	功能描述：	向机顶盒的存储空间写信息
**	输入参数：	lStartAddr：		要写的存储空间的目标地址。
**					pData：	要写的数据
** 					nLen：		要写的数据的长度
**	输出参数：	无。
**	返 回 值：    GYTRUE 成功， FALSE： 失败。
**	其他说明：
*************************************************************************************************/
GYS32 GYCA_WriteBuffer(const GYS32 lStartAddr, const GYU8* pData, GYS32 nLen)
{
	INT32 ret;
	UINT32 offset = 0;//StartAddr = (UINT32)pStartAddr;

	GYCASDRV_PRINTF("%s: lStartAddr=%08x, nLen:%08x\n", __FUNCTION__, lStartAddr, nLen);
	if((lStartAddr == NULL)||(pData == NULL)||(nLen == 0))
	{
		GYCASDRV_PRINTF("Err: %s parameter error!\n", __FUNCTION__);
		return GYFAILURE;
	}
	if(((UINT32)lStartAddr-CAS_flash_start_addr+nLen)> CAS_FLASH_MAXSIZE)
	{
		GYCASDRV_PRINTF("Err: %s overflow!\n", __FUNCTION__);
		nLen = CAS_FLASH_MAXSIZE;
	}

	offset= (UINT32)lStartAddr-CAS_flash_start_addr;
	MEMCPY(&cas_flash_buff[offset], pData, nLen);
	cas_flashwrite_enable = TRUE;
	cas_flashwrite_time = osal_get_tick();
	
	return GYSUCCESS;
}

/*****************************************************/
UINT8 cas_flash_status_check()
{
	INT32 ret;
	UINT32 param,flash_cmd;
	UINT32 tick = cas_flashwrite_time;  //record the last time that libcore write
	UINT32 pdwStartAddr = CAS_flash_start_addr;
	struct sto_device *sto_flash_dev = NULL;
	UINT8 *buff = NULL;

	if (FALSE == cas_flashwrite_enable)
		return CAS_FAIL;
	if (osal_get_tick()-tick < CAS_FLASHWRITE_INTERVAL)
		return CAS_FAIL;

	GYCASDRV_PRINTF("%s: Start to write flash!\n", __FUNCTION__);
	sto_flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
	if(NULL == sto_flash_dev)
	{
		GYCASDRV_PRINTF("TFSTBCA_WriteBuffer: dev_get_by_name failed!\n");
		return CAS_FAIL;
	}
	if (sto_open(sto_flash_dev) != SUCCESS)
	{
		GYCASDRV_PRINTF("%s: sto_open failed!\n", __FUNCTION__);
		return CAS_FAIL;
	}

	if(sto_flash_dev->totol_size <= 0x400000)
	{
		param = pdwStartAddr << 10;
		param |= CAS_FLASH_MAXSIZE >> 10;//CAS_CHUNK_SIZE >> 10;
		flash_cmd = STO_DRIVER_SECTOR_ERASE;
	}
	else
	{
		UINT32 tmp_param[2];
		tmp_param[0] = pdwStartAddr;
		tmp_param[1] = CAS_CHUNK_SIZE >> 10;
		param = (UINT32)tmp_param;
		flash_cmd = STO_DRIVER_SECTOR_ERASE_EXT;
	}
	buff = (UINT8 *)MALLOC(SECTOR_SIZE);
	sto_io_control(sto_flash_dev, STO_DRIVER_SECTOR_BUFFER, (UINT32)buff);
	ret = sto_io_control(sto_flash_dev, flash_cmd, param);
	if(buff != NULL)
		free(buff);
	if(ret != SUCCESS)
	{
		GYCASDRV_PRINTF("%s: Erase failed!\n", __FUNCTION__);
		return CAS_FAIL;
	}
	
	ret = sto_lseek(sto_flash_dev, (INT32) pdwStartAddr, STO_LSEEK_SET);
	if (ret != (INT32) pdwStartAddr)
	{
		GYCASDRV_PRINTF("%s: lseek failed!\n", __FUNCTION__);
		return CAS_FAIL;
	}
	if (CAS_FLASH_MAXSIZE  != sto_write(sto_flash_dev, cas_flash_buff, CAS_FLASH_MAXSIZE))
	{
		GYCASDRV_PRINTF("%s: sto_write failed!\n", __FUNCTION__);
		return CAS_FAIL;
	}

	if (tick == cas_flashwrite_time)
		cas_flashwrite_enable = FALSE; //if no new write during flash writing, we can disable the flag
	else
		cas_flashwrite_enable = TRUE;
	
	return CAS_SUCCESS;
}

