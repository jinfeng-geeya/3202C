/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2006 Copyright (C)
 *
 *  File: tf_nvm.c
 *
 *  Description: This file fills the Demux related API of Tongfang CA library.
 *
 *  History:
 *      Date            Author            Version   Comment
 *      ====        ======      =======  =======
 *  1. 2006.3.19  Gushun Chen     0.1.000    Initial
 *  2. 2007.7.25  Grady Xu		 0.2.000	  update the flash write method to avoid blocking 
 *                                                           the thread and separate the MW and AP by Callbck
 *  3.2007.10.22 Grady Xu		0.3.000	  re-org the Tongfang CA lib.
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
#include <hld/dmx/dmx.h>

//#include <api/libcas/cdca/pub_st.h>
#include <api/libcas/cdca/CDCAS30.h>
//#include <api/libcas/cdca/cas_cd.h>
#include "cd_nvm.h"

//#define CAS_NVM_MEMONLY
#ifdef CAS_NVM_MEMONLY
static UINT8 *CAS_NVM_MEMADDR = NULL;
#endif
extern mcas_t cas_struct;
static UINT32 CAS_flashwrite_time;
static UINT32 CAS_flash_start_addr = 0;
static UINT8 * CAS_flash_buf = NULL;
static UINT8 CAS_write_enable = FALSE;

/*------------------------Internal function----------------------------------*/
UINT8 CAS_WriteEnable()
{
	UINT32 temp;
#ifdef CAS_NVM_MEMONLY
	return CDCA_FALSE;
#else
	if (FALSE ==CAS_write_enable)
		return CDCA_FALSE;
	else 
	{
		temp = osal_get_tick() - CAS_flashwrite_time;
		if (temp >CAS_FLASHWRITE_INTERVAL)
			return CDCA_TRUE;
		else
			return CDCA_FALSE;
	}
#endif
}

void clear_ca_data()
{
	INT32 ret;
	struct sto_device *sto_flash_dev;
	UINT32 param;
	UINT32 flash_cmd;
	CHUNK_HEADER chuck_hdr;
	UINT32 chunk_id = 0x08F70100;	

	ret = sto_get_chunk_header(chunk_id, &chuck_hdr);
	if (ret == 0)
	{
		libc_printf("%s(): CHUNK ID not found\n",__FUNCTION__,chunk_id);
		return;
	}
	
	sto_flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
	if(sto_flash_dev == NULL)
	{
		libc_printf("STBCA_WriteBuffer: dev_get_by_name failed!\n");
		return;
	}
	if (sto_open(sto_flash_dev) != SUCCESS)
	{
		libc_printf("STBCA_WriteBuffer: sto_open failed!\n");
		return;
	}
	

	UINT32 tmp_param[2];
	tmp_param[0] = (UINT32)sto_chunk_goto(&chunk_id, 0xFFFFFFFF, 1) + CHUNK_HEADER_SIZE;;
	tmp_param[1] = 0x20000 >> 10;
	param = (UINT32)tmp_param;
	flash_cmd = STO_DRIVER_SECTOR_ERASE_EXT;
	ret = sto_io_control(sto_flash_dev, flash_cmd, param);

	if(ret != SUCCESS)
	{
		libc_printf("STBCA_WriteBuffer: Erase failed!\n");
		return;
	}

}

void CAS_WriteFlash()
{
	INT32 ret;
	struct sto_device *sto_flash_dev;
	UINT32 param;
	UINT32 tick = CAS_flashwrite_time;  //record the last time that libcore write
	UINT32 pdwStartAddr = CAS_flash_start_addr;
	UINT32 dwLen = CAS_CHUNK_SIZE;
	UINT8 * pbyData = CAS_flash_buf;
	

	CAS_NVM_PRINTF("@@@@CAS_WriteFlash:Start to write flash!\n");
	sto_flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
	if(sto_flash_dev == NULL)
	{
		CAS_NVMERR_PRINTF("STBCA_WriteBuffer: dev_get_by_name failed!\n");
		return;
	}
	if (sto_open(sto_flash_dev) != SUCCESS)
	{
		CAS_NVMERR_PRINTF("STBCA_WriteBuffer: sto_open failed!\n");
		return;
	}
	
	UINT32 flash_cmd;
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
		CAS_NVMERR_PRINTF("STBCA_WriteBuffer: Erase failed!\n");
		return;
	}

	ret = sto_lseek(sto_flash_dev, (INT32) pdwStartAddr, STO_LSEEK_SET);
	if (ret != (INT32) pdwStartAddr)
	{
		CAS_NVMERR_PRINTF("STBCA_WriteBuffer: seek base failed!\n");
		return;
	}
	ret = sto_write(sto_flash_dev, pbyData, dwLen);
	if((UINT32) ret != dwLen)
	{
		CAS_NVMERR_PRINTF("STBCA_WriteBuffer: here is not blank at:%x!\n", pbyData);
	}

	if (tick == CAS_flashwrite_time)
		CAS_write_enable = FALSE; //if no new write during flash writing, we can disable the flag
	else
		CAS_write_enable = TRUE;
}


extern void CDSTBCA_GetBuffer(CDCA_U32* pdwStartAddr, CDCA_U32* pdwSize)
{
	INT32 ret;
	UINT32 chunk_id = CAS_CHUNK_ID;
	CHUNK_HEADER chuck_hdr;

	/* get table info base addr by using chunk */
	if(sto_get_chunk_header(chunk_id, &chuck_hdr) == 0)
	{
		CAS_NVMERR_PRINTF("STBCA_GetBuffer: getbuffer failed!\n");
		return;
	}
	*pdwStartAddr = sto_chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
	*pdwStartAddr += CHUNK_HEADER_SIZE;
	CAS_flash_start_addr = *pdwStartAddr;
	*pdwSize = CAS_CHUNK_SIZE;
	CAS_NVM_PRINTF("STBCA_GetBuffer: pdwStartAddr=%08x, dwLen:%08x\n", *pdwStartAddr, *pdwSize);
#ifdef CAS_NVM_MEMONLY
	if (NULL == CAS_NVM_MEMADDR)
		CAS_NVM_MEMADDR = (UINT8 *)MALLOC(0x10000);
	if (NULL == CAS_NVM_MEMADDR)
	{
		CAS_NVMERR_PRINTF("!!!STBCA_WriteBuffer: Malloc Failed!!");
		return;
	}
	MEMSET(CAS_NVM_MEMADDR, 0xff, 0x10000);
#endif
	return;
}


extern void CDSTBCA_ReadBuffer(CDCA_U32 dwStartAddr, CDCA_U8* pbyData, CDCA_U32* pdwLen)
{
	INT32 ret;
	struct sto_device *sto_flash_dev;
	
#ifdef CAS_NVM_MEMONLY
	MEMCPY(pbyData, CAS_NVM_MEMADDR, *pdwLen);
	return;
#else

	CAS_NVM_PRINTF("STBCA_ReadBuffer: pdwStartAddr=%08x, dwLen:%08x\n", dwStartAddr, *pdwLen);
	if((dwStartAddr == 0)||(pbyData == NULL)||(*pdwLen == 0))
	{
		CAS_NVMERR_PRINTF("STBCA_ReadBuffer: read buffer param error!\r\n");
		return;
	}
	if(*pdwLen > CAS_CHUNK_SIZE)
	{
		*pdwLen = CAS_CHUNK_SIZE;
	}
	sto_flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
	if(sto_flash_dev == NULL)
	{
		CAS_NVMERR_PRINTF("STBCA_WriteBuffer: dev_get_by_name failed!\n");
		return;
	}
	
	if (sto_open(sto_flash_dev) != SUCCESS)
	{
		CAS_NVMERR_PRINTF("STBCA_WriteBuffer: sto_open failed!\n");
		return;
	}
	
	ret = sto_lseek(sto_flash_dev, (INT32)dwStartAddr, STO_LSEEK_SET);
	if (ret != (INT32)dwStartAddr)
	{
		CAS_NVMERR_PRINTF("STBCA_WriteBuffer: seek base failed!\n");
		return ;
	}
	
	if (sto_read(sto_flash_dev, pbyData, *pdwLen) != (INT32)*pdwLen)
	{
		CAS_NVMERR_PRINTF("read_from_flash: sto_read failed!\n");
		return;
	}
	return;
#endif
}


extern void CDSTBCA_WriteBuffer(CDCA_U32 dwStartAddr, const CDCA_U8* pbyData, CDCA_U32 dwLen)
{
	UINT32 offset = 0;
#ifdef CAS_NVM_MEMONLY
	offset = dwStartAddr -CAS_flash_start_addr;
	MEMCPY(&CAS_NVM_MEMADDR[offset], pbyData, dwLen);
#else
	CAS_NVM_PRINTF("STBCA_WriteBuffer: pdwStartAddr=%08x, dwLen:%08x\n", dwStartAddr, dwLen);

	if((dwStartAddr == 0)||(pbyData == NULL)||(dwLen == 0))
	{
		CAS_NVMERR_PRINTF("STBCA_WriteBuffer: write buffer param error!\r\n");
		return;
	}	
	if (NULL == CAS_flash_buf)
	{
		CAS_flash_buf = (UINT8 *)MALLOC(0x10000);
		if (NULL == CAS_flash_buf)
		{
			CAS_NVMERR_PRINTF("!!!STBCA_WriteBuffer: Malloc Failed!!");
			return;
		}
		
		MEMSET(CAS_flash_buf, 0xff, 0x10000);
	}
	if (CAS_flash_start_addr != 0)
	{
		if (CAS_flash_start_addr > dwStartAddr)
		{
			ASSERT(0);
		}
		offset = dwStartAddr -CAS_flash_start_addr;
		
	}
	
	MEMCPY(&CAS_flash_buf[offset], pbyData, dwLen);
	CAS_write_enable = TRUE;
	CAS_flashwrite_time = osal_get_tick();
	
	return;
#endif
}


