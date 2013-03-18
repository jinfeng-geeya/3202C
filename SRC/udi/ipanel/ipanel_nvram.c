/*********************************************************************
    Copyright (c) 2005 Embedded Internet Solutions, Inc
    All rights reserved. You are not allowed to copy or distribute
    the code without permission.
    This is the demo implenment of the NVRAM Porting APIs needed by iPanel MiddleWare. 
    Maybe you should modify it accorrding to Platform.
    
    Note: the "int" in the file is 32bits
    
    $ver0.0.0.1 $author Zouxianyun 2005/04/28
********************************************************************/
#include <stdio.h>
#include <udi/ipanel/ipanel_nvram.h>


#include <osal/osal.h>
#include <hld/hld_dev.h>
#include <hld/sto/sto_dev.h>
#include <api/libchunk/chunk.h>
#include <api/libc/string.h>


//#define IPANEL_NVM_DEBUG
#ifdef IPANEL_NVM_DEBUG
#define IPANEL_NVM_PRINTF	libc_printf
#else
#define IPANEL_NVM_PRINTF(...)		do{}while(0)
#endif

#define IPANEL_NVM_CHUNK_ID	0x08F70200



static UINT8 ipanel_nvm_buf[0x10000];

//0 -- writing, 1 -- success and complete, -1 - fail.
static int ipanel_nvram_status  = 1;
static unsigned int ipanel_nvm_addr = 0;
static int ipanel_nvm_len = 0;
static unsigned int ipanel_nvm_write_addr = 0;
static int ipanel_nvm_write_len = 0;

static ID ipanel_nvm_task_id = OSAL_INVALID_ID;
static ID ipanel_nvm_semaphore = OSAL_INVALID_ID;

static void ipanel_nvram_task()
{
	INT32 ret = 0;
	UINT32 flash_cmd;
	UINT32 param;
	UINT32 tmp_param[2];
	struct sto_device *sto = (struct sto_device*)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
	
	while(1)
	{
		/*need write to flash*/
		if(ipanel_nvram_status==0)
		{
			IPANEL_NVM_PRINTF("%s(): write len=%d\n",__FUNCTION__,ipanel_nvm_write_len);
			osal_semaphore_capture(ipanel_nvm_semaphore, OSAL_WAIT_FOREVER_TIME);
			ret = sto_put_data(sto, ipanel_nvm_addr, ipanel_nvm_buf, ipanel_nvm_write_len);
			osal_semaphore_release(ipanel_nvm_semaphore);
			/*write fail, need erase flash*/
			IPANEL_NVM_PRINTF("%s(): write ret=%d\n",__FUNCTION__,ret);
			if(ret!=ipanel_nvm_write_len)
			{
				/*erase nvm sector*/
				if(sto->totol_size <= 0x400000)
				{
					param = (ipanel_nvm_addr<<10) + ( 0x10000>>10);
					flash_cmd = STO_DRIVER_SECTOR_ERASE;
				}
				else
				{
					tmp_param[0] = ipanel_nvm_addr;
					tmp_param[1] =  0x10000>>10;
					param = (UINT32)tmp_param;
					flash_cmd = STO_DRIVER_SECTOR_ERASE_EXT;
				}
				IPANEL_NVM_PRINTF("%s(): erase flash addr=0x%x 64K sector\n",__FUNCTION__,ipanel_nvm_addr);
				if(sto_io_control(sto, flash_cmd, param) != SUCCESS)
				{
					IPANEL_NVM_PRINTF("%s(): erase nvm fail!\n",__FUNCTION__);
					ipanel_nvram_status = -1;
					continue;
				}
				osal_semaphore_capture(ipanel_nvm_semaphore, OSAL_WAIT_FOREVER_TIME);
				ret = sto_put_data(sto, ipanel_nvm_addr, &ipanel_nvm_buf[0], sizeof(ipanel_nvm_buf));
				osal_semaphore_release(ipanel_nvm_semaphore);
				IPANEL_NVM_PRINTF("%s(): after erase, write 64k, ret=%d\n",__FUNCTION__,ret);
				if(ret==sizeof(ipanel_nvm_buf))
					ipanel_nvram_status = 1;
				else
				{
					IPANEL_NVM_PRINTF("%s(): write nvm all buf data fail!\n",__FUNCTION__);
					ipanel_nvram_status = -1;
					continue;
				}

			}

			ipanel_nvram_status = 1;
		}
		else
		{
			osal_task_sleep(50);			
		}

	}

}


int ipanel_nvram_init()
{
	INT32 ret;
	CHUNK_HEADER chuck_hdr;
	UINT32 chunk_id = IPANEL_NVM_CHUNK_ID;
	OSAL_T_CTSK t_ctsk;

	ret = sto_get_chunk_header(chunk_id, &chuck_hdr);
	if (ret == 0)
	{
		IPANEL_NVM_PRINTF("%s(): CHUNK ID not found\n",__FUNCTION__,chunk_id);
		return -1;
	}
	ipanel_nvm_addr = (UINT32)sto_chunk_goto(&chunk_id, 0xFFFFFFFF, 1) + CHUNK_HEADER_SIZE;
	ipanel_nvm_len = chuck_hdr.len - CHUNK_HEADER_SIZE + CHUNK_NAME;
	IPANEL_NVM_PRINTF("%s(): nvram addr=0x%x, len=0x%x\n",__FUNCTION__,ipanel_nvm_addr,ipanel_nvm_len);

	if(ipanel_nvm_semaphore==OSAL_INVALID_ID)
		ipanel_nvm_semaphore = osal_semaphore_create(1);
	if (OSAL_INVALID_ID == ipanel_nvm_semaphore)
	{
		IPANEL_NVM_PRINTF("%s(): create nvram semaphore fail!\n",__FUNCTION__);
		return -1;
	}
	MEMSET(ipanel_nvm_buf, 0, sizeof(ipanel_nvm_buf));
	MEMSET(&t_ctsk, 0, sizeof(t_ctsk));
	t_ctsk.stksz = 0x1000;
	t_ctsk.quantum = 10;
	t_ctsk.itskpri = OSAL_PRI_NORMAL;
	t_ctsk.name[0] = 'N';
	t_ctsk.name[1] = 'V';
	t_ctsk.name[2] = 'M';
	t_ctsk.task = (FP)ipanel_nvram_task;
	if(ipanel_nvm_task_id==OSAL_INVALID_ID)
		ipanel_nvm_task_id = osal_task_create(&t_ctsk);
	if (OSAL_INVALID_ID == ipanel_nvm_task_id)
	{
		IPANEL_NVM_PRINTF("%s(): create nvram task fail!\n",__FUNCTION__);
		return -1;
	}
	return 0;
}


INT32 ipanel_nvram_exit()
{
	while(ipanel_nvram_status==0)
	{
		osal_task_sleep(10);
	}
	if(ipanel_nvm_task_id!=OSAL_INVALID_ID)
	{
		osal_task_delete(ipanel_nvm_task_id);
		ipanel_nvm_task_id = OSAL_INVALID_ID;
	}
	if(ipanel_nvm_semaphore!=OSAL_INVALID_ID)
	{
		osal_semaphore_delete(ipanel_nvm_semaphore);
		ipanel_nvm_semaphore = OSAL_INVALID_ID;
	}

	ipanel_nvram_status  = 1;
	ipanel_nvm_addr = 0;
	ipanel_nvm_len = 0;
	ipanel_nvm_write_addr = 0;
	ipanel_nvm_write_len = 0;
	IPANEL_NVM_PRINTF("%s(): finish!\n",__FUNCTION__);
	return 0;
}



/******************************************************************/
/*Description: Get Flash Memory Address information.              */
/*Input      : NVRAM Address Pointer, section number&Size Pointer */
/*Output     : NVRAM Address, number of sections, section size.   */
/*     sect size must be 32k/64k/128k/256k.                       */
/*Return     : 0 -- success, -1 -- fail.                          */
/******************************************************************/
/**the base NVRAM address which give to iPanel MiddleWare         */
int ipanel_porting_nvram_info(unsigned char **addr, int *numberofsections, int *sect_size)
{
	if((ipanel_nvm_addr==0)&&(ipanel_nvm_len==0))
	{
		IPANEL_NVM_PRINTF("%s(): invalid nvm addr or len\n",__FUNCTION__);
		return -1;
	}
	*addr = (unsigned char *)&ipanel_nvm_buf[0];
	*numberofsections = 1;
	*sect_size = 0x10000;
	
	return 0;
}

/**********************************************************************/
/*Description: Read Data From Flash Memory.                           */
/*Input      : NVRAM Address, Buffer, and numberofbytes need to read. */
/*Output     : Save Data to Buffer                                    */
/*Return     : Read real nbytes, -1 -- fail.                          */
/**********************************************************************/
int ipanel_porting_nvram_read(unsigned int address, unsigned char *buf,  int nbytes)
{
	INT32 ret = 0;
	INT32 read_byte = nbytes;
	struct sto_device *sto = (struct sto_device*)dev_get_by_id(HLD_DEV_TYPE_STO, 0);

	if((address<(unsigned int)&ipanel_nvm_buf[0])||(address>=(unsigned int)(&ipanel_nvm_buf[0]+0x10000))||(buf==NULL))
	{
		IPANEL_NVM_PRINTF("%s(): invalid param\n",__FUNCTION__);
		return -1;
	}
	IPANEL_NVM_PRINTF("%s(): addr=0x%x, len=%d\n",__FUNCTION__,address,nbytes);
	if((address+nbytes)>(unsigned int)(&ipanel_nvm_buf[0]+0x10000))
		read_byte =(unsigned int) &ipanel_nvm_buf[0]+0x10000 - address;

	//nvm writing
	if(ipanel_nvram_status==0)
	{
		osal_semaphore_capture(ipanel_nvm_semaphore, OSAL_WAIT_FOREVER_TIME);
		MEMCPY(buf, ipanel_nvm_buf, read_byte);
		osal_semaphore_release(ipanel_nvm_semaphore);
		return read_byte;
	}
	else
	{
		ret = sto_get_data(sto, buf, ipanel_nvm_addr, read_byte);
		if(ret==read_byte)
			return ret;
		else 
			return -1;
	}

}

/******************************************************************/
/*Description: Write Flash Memory, and no block.                  */
/*Input      : NVRAM Address, data, length need to write to nvram */
/*Output     : No                                                 */
/*Return     : Write real nbytes. and  -1 -- fail.                */
/******************************************************************/
int ipanel_porting_nvram_burn(unsigned int address, const char *to, int len)
{
	INT32 ret = 0;
	INT32 write_byte = len;
	struct sto_device *sto = (struct sto_device*)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
	
	if((address<(unsigned int)&ipanel_nvm_buf[0])||(address>=(unsigned int)(&ipanel_nvm_buf[0]+0x10000))||(to==NULL))
	{
		IPANEL_NVM_PRINTF("%s(): invalid param\n",__FUNCTION__);
		return -1;
	}
	IPANEL_NVM_PRINTF("%s(): addr=0x%x, len=%d\n",__FUNCTION__,address,len);
	if((address+len)>((unsigned int)&ipanel_nvm_buf[0]+0x10000))
		write_byte = ipanel_nvm_addr+0x10000-address;

	osal_semaphore_capture(ipanel_nvm_semaphore, OSAL_WAIT_FOREVER_TIME);
	//burn data changed
	if(MEMCMP(ipanel_nvm_buf, to, write_byte)!=0)
	{
		MEMCPY(&ipanel_nvm_buf, to, write_byte);
		ipanel_nvram_status = 0;
	}
	//burn data not change
	else
		ipanel_nvram_status = 1;
	osal_semaphore_release(ipanel_nvm_semaphore);

	ipanel_nvm_write_addr = address;
	ipanel_nvm_write_len = write_byte;
	
	return write_byte;
}

/******************************************************************/
/*Description: Check burn is finished.                            */
/*Input      : NVRAM address, len need to write                   */
/*Output     : No                                                 */
/*Return     : 0 -- writing, 1 -- success and complete, -1 - fail.*/
/******************************************************************/
int ipanel_porting_nvram_status(unsigned int address, int len)
{
    return ipanel_nvram_status;
}

