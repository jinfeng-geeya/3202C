/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2006 Copyright (C)
 *
 *  File: flash_raw_pr.c
 *
 *  Description: Provide remote paral flash raw operations.
 *
 *  History:
 *      Date        Author      Version  Comment
 *      ====        ======      =======  =======
 *  1.  2006.4.xx Shipman Yuan  0.1.000  Initial
 *
 ****************************************************************************/
#include <types.h>
#include <sys_config.h>
#include <retcode.h>
#include <sys_parameters.h>
#include <osal/osal.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hal/hal_mem.h>
#include <hld/hld_dev.h>
#include <hld/sto/sto.h>
#include <hld/sto/sto_dev.h>
#include <bus/erom/erom.h>
#include "sto_flash.h"

/*Debug Switch*/
#define soc_printf(x,...) 		// debug only

#define DELAY_US(us)	 		osal_delay(us)
#define DELAY_MS(ms)  			osal_task_sleep(ms)

#define FLASH_CTRL_IO			(0xB8000090)

#define ERASE_CHIP_WAIT_TIME    	(10000)  //in unit of ms
#define ERASE_SECTOR_WAIT_TIME    	(3000)   //in unit of ms

static unsigned long remote_flash_mode;
static unsigned long remote_flash_cmdaddr;
static unsigned long old_baudrate;
static unsigned long cmd_buf[6] = {0,0xAA,0,0x55,0,0};



static int remote_flash_send_cmd(unsigned long *cmd, unsigned long cnt);

int remote_flash_open( void *arg )
{
	/*construct the command buffer for later use*/
	cmd_buf[0]=remote_flash_cmdaddr;
	cmd_buf[2]=remote_flash_cmdaddr>>1;
	cmd_buf[4]=remote_flash_cmdaddr;

	remote_flash_read_tmo = RF_READ_TMO;
	
	return SUCCESS;
}

int remote_flash_close ( void* arg )
{
//	return uart_high_speed_config(115200);
	return SUCCESS;
}

int remote_flash_control(unsigned long ctrl_cmd, unsigned long ctrl_val)
{
	if( FLASH_CONTROL==ctrl_cmd) {

		erom_wm(FLASH_CTRL_IO,&ctrl_val,4,0);

	} else if (UPG_MODE_CONTROL==ctrl_cmd ) {

		remote_flash_mode =ctrl_val;

	} else if (FLASH_SET_CMDADDR == ctrl_cmd) {

		remote_flash_cmdaddr = ctrl_val;

	} else if (FLASH_READ_TMO == ctrl_cmd) {

		remote_flash_read_tmo = ctrl_val;
	
	}	

	return SUCCESS;
}

void remote_flash_get_id(unsigned long *id_buf, unsigned long cmd_addr)
{
	unsigned long cmd;
	unsigned long base_addr = SYS_FLASH_BASE_ADDR;

	remote_flash_cmdaddr = cmd_addr;

	cmd_buf[0]=remote_flash_cmdaddr;
	cmd_buf[2]=remote_flash_cmdaddr>>1;
	cmd_buf[4]=remote_flash_cmdaddr;
	cmd_buf[5]=0x90;
	remote_flash_send_cmd(cmd_buf,3);

	if(remote_flash_read_tmo == 0){
	erom_rm(base_addr,(void *)id_buf,sizeof(unsigned long),0);
	erom_rm(base_addr+0x100,(void *)(id_buf+1),sizeof(unsigned long),0);
	}else{
	if(SUCCESS!=erom_read_tmo(base_addr,
		(void *)id_buf, 4, 0,remote_flash_read_tmo) ||
			SUCCESS!=erom_read_tmo(base_addr,
				(void *)(id_buf+1), 4, 0,remote_flash_read_tmo) )
	{
		id_buf[0] = 0;id_buf[1] = 0;
		return;
	}
	}/*REMOTE_FLASH_READ_TMO */

	cmd = 0xF0;
	erom_wm(base_addr,(void *)(&cmd),1,0);
}

int remote_flash_erase_chip(void)
{
	unsigned long retry, cmd;
    	unsigned long base_addr = SYS_FLASH_BASE_ADDR;
	unsigned long cnt,tmo_cnt;

	cmd_buf[5]= 0x80;
	remote_flash_send_cmd(cmd_buf,3);
	cmd_buf[5]= 0x10;
	remote_flash_send_cmd(cmd_buf,3);

	if(remote_flash_mode == SIMPLEX_MODE) {
		DELAY_MS(ERASE_CHIP_WAIT_TIME);
		soc_printf("erase chip (SIMPLEX_MODE)\n");
		return SUCCESS;
	}

	if(remote_flash_read_tmo == 0){
	retry = (ERASE_CHIP_WAIT_TIME+4)/5;
	while ((retry--)!=0) {
		DELAY_MS(5);
		erom_rm(base_addr,(void *)(&cmd),1,0);
		if(cmd == 0xFF)
			return SUCCESS ;
	}
	}else{
	cmd =0; 
	tmo_cnt = ERASE_CHIP_WAIT_TIME * MS_TICKS;
	cnt = read_tsc();
	while (1) {
		if(SUCCESS!=erom_read_tmo(base_addr,
			(void *)(&cmd),1,0,remote_flash_read_tmo))
			return ERR_FAILUE;
		if(cmd == 0xFF)
			return SUCCESS ;
		if((read_tsc()-cnt)>=tmo_cnt)
			break;	
		DELAY_MS(5);
	}
	};/*REMOTE_FLASH_READ_TMO */
	
	soc_printf("erase chip (DUPLEX_MODE)\n");
	return ERR_FAILUE;
}

int remote_flash_erase_sector(unsigned long sector_addr)
{
	unsigned long retry, cmd;
	unsigned long base_addr = SYS_FLASH_BASE_ADDR;
	unsigned long cnt,tmo_cnt;

	soc_printf("erase sector %x\n",sector_addr);
	cmd_buf[5]= 0x80;
	remote_flash_send_cmd(cmd_buf,3);
	cmd_buf[4]=sector_addr;
	cmd_buf[5]= 0x30;
	remote_flash_send_cmd(cmd_buf,3);
	cmd_buf[4]=remote_flash_cmdaddr;

	if(remote_flash_mode == SIMPLEX_MODE ) 	{
		DELAY_MS(ERASE_SECTOR_WAIT_TIME);
		soc_printf("erase sector 0x%x (SIMPLEX_MODE)\n",sector_addr);
		return SUCCESS;
	}

	if(remote_flash_read_tmo == 0){
	retry = ERASE_SECTOR_WAIT_TIME;
	while ((retry--)!=0) {
		DELAY_MS(1);
		erom_rm(base_addr+sector_addr,(void *)(&cmd),1,0);
		if( cmd == 0xFF)
			return SUCCESS ;
	}
	}else{
	cmd =0; 
	tmo_cnt = ERASE_SECTOR_WAIT_TIME * MS_TICKS;
	cnt = read_tsc();
	while (1) {
		if(SUCCESS!=erom_read_tmo(base_addr+sector_addr,
			(void *)(&cmd),1,0,remote_flash_read_tmo))
			return ERR_FAILUE;
		if(cmd == 0xFF)
			return SUCCESS ;
		if((read_tsc()-cnt)>=tmo_cnt)
			break;	
		DELAY_MS(1);	
	}
	};
	soc_printf("erase sector 0x%x (DUPLEX_MODE)\n",sector_addr);
	return ERR_FAILUE;
}

int remote_flash_copy(unsigned long flash_addr, unsigned char *data, unsigned long len)
{
	unsigned long cmd_buf0[2];

        soc_printf("write 0x%x len %x\n",flash_addr,len) ;
	for (; len > 0; flash_addr++, len--) {
		cmd_buf[5]= 0xA0;
		remote_flash_send_cmd(cmd_buf,3);
		cmd_buf0[0]=flash_addr;
		cmd_buf0[1]= *data++;
		remote_flash_send_cmd(cmd_buf0,1);

    	}

    	return SUCCESS;
}

static int remote_flash_read(unsigned long flash_addr, unsigned char *data, unsigned long len)
{
	unsigned long base_addr = SYS_FLASH_BASE_ADDR;

	if(remote_flash_read_tmo == 0)	
	return erom_rm(base_addr+flash_addr,(void *)(data),len,0);
	else
	return erom_read_tmo(base_addr+flash_addr,(void *)(data),len,0,remote_flash_read_tmo);
	/*REMOTE_FLASH_READ_TMO*/
}

int remote_flash_read2( void *des, void* src, UINT32 len)
{
	unsigned long addr =(unsigned long)src-SYS_FLASH_BASE_ADDR;
	return remote_flash_read(addr,(unsigned char *)des,len);
}

int remote_flash_verify(unsigned long flash_addr, unsigned char *data, unsigned long len)
{
	int ret = SUCCESS;
	unsigned char dst, src;
	unsigned long base_addr = SYS_FLASH_BASE_ADDR;

	for (; len > 0; flash_addr++, len--) {
	if(remote_flash_read_tmo == 0){
		erom_rm(base_addr+flash_addr,(void *)(&dst),1,0);
	}else{
		if(SUCCESS!=erom_read_tmo(base_addr+flash_addr,
				(void *)(&dst),1,0,remote_flash_read_tmo))
			return 3; 
	};/*REMOTE_FLASH_READ_TMO*/		
		src = *data++;
		dst ^= src;
		if (dst == 0)
		    	continue;
		ret = 1;                // no need to erase
		if (dst & src) 	{
		    	ret = 2;	// must erase
		    	break;
		}
	}

	return ret;
}

int remote_flash_send_cmd(unsigned long *cmd,unsigned long cnt)
{
	unsigned long base_addr = SYS_FLASH_BASE_ADDR;

	while(cnt>0) {
		erom_wm(base_addr+(*cmd),(void *)(cmd+1),1,0);
		cmd += 2;
		cnt--;
	};

	return SUCCESS;
}

