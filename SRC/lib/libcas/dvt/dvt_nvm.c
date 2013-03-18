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
#include <api/libcas/dvt/DVTCAS_STBDataPublic.h>
#include <api/libcas/dvt/cas_dvt.h>
#include "dvt_nvm.h"

static UINT32 CAS_flash_start_addr = 0;
static UINT8 *cas_flash_buff = NULL;
static UINT32 cas_flashwrite_time = 0;
static UINT8  cas_flashwrite_enable = FALSE;

 HRESULT DVTSTBCA_GetDataBufferAddr(long * lSize, char ** ppStartAddr)
{
	UINT32 chunk_id = CAS_CHUNK_ID;
	CHUNK_HEADER chuck_hdr;
	struct sto_device *sto_flash_dev = NULL;

	/* get table info base addr by using chunk */
	if(sto_get_chunk_header(chunk_id, &chuck_hdr) == 0)
	{
		CAS_NVMERR_PRINTF("Err: %s get chunk header fail!\n", __FUNCTION__);
		return CAS_FAIL;
	}
	
	CAS_flash_start_addr = sto_chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
	CAS_flash_start_addr += CHUNK_HEADER_SIZE;
	
	*ppStartAddr = (UINT8 *)CAS_flash_start_addr;
	*lSize = CAS_FLASH_MAXSIZE;//CAS_CHUNK_SIZE/2;

	if (NULL == cas_flash_buff)
		cas_flash_buff = MALLOC(*lSize);
	if (NULL == cas_flash_buff)
		return CAS_FAIL;

	//copy the CA flash data to mem
	sto_flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
	if(NULL == sto_flash_dev)
	{
		CAS_NVMERR_PRINTF("TFSTBCA_WriteBuffer: dev_get_by_name failed!\n");
		return CAS_FAIL;
	}
	if (sto_open(sto_flash_dev) != SUCCESS)
	{
		CAS_NVMERR_PRINTF("%s: sto_open failed!\n", __FUNCTION__);
		return CAS_FAIL;
	}
	if( (INT32) CAS_flash_start_addr  != sto_lseek(sto_flash_dev, (INT32) CAS_flash_start_addr, STO_LSEEK_SET))
	{
		CAS_NVMERR_PRINTF("%s: lseek failed!\n", __FUNCTION__);
		return CAS_FAIL;
	}
	if (CAS_FLASH_MAXSIZE  != sto_read(sto_flash_dev, cas_flash_buff, CAS_FLASH_MAXSIZE))
	{
		CAS_NVMERR_PRINTF("%s: sto_read failed!\n", __FUNCTION__);
		return CAS_FAIL;
	}
		
	CAS_NVM_PRINTF("%s: flash StartAddr=%08x, Len:%08x\n", __FUNCTION__, *ppStartAddr, *lSize);
	return CAS_SUCCESS;
}


HRESULT DVTSTBCA_ReadDataBuffer(const char * pStartAddr, long * plDataLen, BYTE * pData)
{
	INT32 ret;
	UINT32 offset = 0;

	CAS_NVM_PRINTF("%s: pStartAddr=%08x, plDataLen:%08x\n", __FUNCTION__, pStartAddr, *plDataLen);
	if((pStartAddr == NULL)||(pData == NULL)||(*plDataLen == 0))
	{
		CAS_NVMERR_PRINTF("Err: %s parameter error!\n", __FUNCTION__);
		return CAS_FAIL;
	}
	if(((UINT32)pStartAddr-CAS_flash_start_addr+*plDataLen)> CAS_FLASH_MAXSIZE)
	{
		CAS_NVMERR_PRINTF("Err: %s overflow!\n", __FUNCTION__);
		*plDataLen = CAS_FLASH_MAXSIZE;
	}
	
	offset= (UINT32)pStartAddr-CAS_flash_start_addr;
	MEMCPY(pData, &cas_flash_buff[offset], *plDataLen);
	
	return CAS_SUCCESS;
}


HRESULT DVTSTBCA_WriteDataBuffer(const char * pStartAddr, long * plDataLen, const BYTE * pData)
{
	INT32 ret;
	UINT32 offset = 0;//StartAddr = (UINT32)pStartAddr;

	CAS_NVM_PRINTF("%s: pStartAddr=%08x, *plDataLen:%08x\n", __FUNCTION__, pStartAddr, *plDataLen);
	if((pStartAddr == NULL)||(pData == NULL)||(*plDataLen == 0))
	{
		CAS_NVMERR_PRINTF("Err: %s parameter error!\n", __FUNCTION__);
		return CAS_FAIL;
	}
	if(((UINT32)pStartAddr-CAS_flash_start_addr+*plDataLen)> CAS_FLASH_MAXSIZE)
	{
		CAS_NVMERR_PRINTF("Err: %s overflow!\n", __FUNCTION__);
		*plDataLen = CAS_FLASH_MAXSIZE;
	}

	offset= (UINT32)pStartAddr-CAS_flash_start_addr;
	MEMCPY(&cas_flash_buff[offset], pData, *plDataLen);
	cas_flashwrite_enable = TRUE;
	cas_flashwrite_time = osal_get_tick();
	
	return CAS_SUCCESS;
}

 
HRESULT DVTSTBCA_GetDataFromEeprom(long * plDataLen, BYTE * pData)
{
	return (HRESULT)-1;
}


HRESULT DVTSTBCA_SaveDataToEeprom(long * plDataLen, const BYTE * pData)
{
	return (HRESULT)-1;
}

/*****************************************************/
#if 0
UINT8 cas_flash_status_check()
{
	INT32 ret;
	UINT32 param,flash_cmd;
	UINT32 tick = cas_flashwrite_time;  //record the last time that libcore write
	UINT32 pdwStartAddr = CAS_flash_start_addr;
	struct sto_device *sto_flash_dev = NULL;

	if (FALSE == cas_flashwrite_enable)
		return CAS_FAIL;
	if (osal_get_tick()-tick < CAS_FLASHWRITE_INTERVAL)
		return CAS_FAIL;

	CAS_NVM_PRINTF("%s: Start to write flash!\n", __FUNCTION__);
	sto_flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
	if(NULL == sto_flash_dev)
	{
		CAS_NVMERR_PRINTF("TFSTBCA_WriteBuffer: dev_get_by_name failed!\n");
		return CAS_FAIL;
	}
	if (sto_open(sto_flash_dev) != SUCCESS)
	{
		CAS_NVMERR_PRINTF("%s: sto_open failed!\n", __FUNCTION__);
		return CAS_FAIL;
	}

	if(sto_flash_dev->totol_size <= 0x400000)
	{
		param = pdwStartAddr << 10;
		param |= CAS_CHUNK_SIZE >> 10;
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
	ret = sto_io_control(sto_flash_dev, flash_cmd, param);
	if(ret != SUCCESS)
	{
		CAS_NVMERR_PRINTF("%s: Erase failed!\n", __FUNCTION__);
		return CAS_FAIL;
	}
	
	ret = sto_lseek(sto_flash_dev, (INT32) pdwStartAddr, STO_LSEEK_SET);
	if (ret != (INT32) pdwStartAddr)
	{
		CAS_NVMERR_PRINTF("%s: lseek failed!\n", __FUNCTION__);
		return CAS_FAIL;
	}
	if (CAS_FLASH_MAXSIZE  != sto_write(sto_flash_dev, cas_flash_buff, CAS_FLASH_MAXSIZE))
	{
		CAS_NVMERR_PRINTF("%s: sto_write failed!\n", __FUNCTION__);
		return CAS_FAIL;
	}

	if (tick == cas_flashwrite_time)
		cas_flashwrite_enable = FALSE; //if no new write during flash writing, we can disable the flag
	else
		cas_flashwrite_enable = TRUE;
	
	return CAS_SUCCESS;
}
#endif
