/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2006 Copyright (C)
 *
 *  File: flash_raw_sl.c
 *
 *  Description: Provide remote serial flash raw operations.
 *
 *  History:
 *      Date        Author      Version  Comment
 *      ====        ======      =======  =======
 *  1.  2006.4.24   Justin Wu   0.1.000  Initial
 *
 ****************************************************************************/
#include <types.h>
#include <sys_config.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/sto/sto.h>
#include <hld/sto/sto_dev.h>
#include <bus/flash/flash.h>
#include <bus/erom/erom.h>
#include "sto_flash.h"

#define soc_printf(x,...)
#define US_TICKS       (sys_ic_get_cpu_clock()*1000000 / 2000000)

#if 	1   //(SYS_PROJECT_FE == PROJECT_FE_DVBT && SYS_SDRAM_SIZE == 8)
#define MS_CNT  (US_TICKS*1000)
void DELAY_MS(unsigned long ms)
{
	volatile unsigned long start_cnt, end_cnt;
	
	//__asm volatile ("mfc0 %0,$9;nop":"=r"(start_cnt));
	start_cnt = read_tsc();
		
	while(1){
		//__asm volatile ("mfc0 %0,$9;nop":"=r"(end_cnt));
		end_cnt = read_tsc();
		if(end_cnt-start_cnt>(ms*MS_CNT))
			break;
	}	
}
void DELAY_US(unsigned long us) 
{
	volatile unsigned long start_cnt, end_cnt;
	
	//__asm volatile ("mfc0 %0,$9;nop":"=r"(start_cnt));
	start_cnt = read_tsc();	
	
	while(1){
		//__asm volatile ("mfc0 %0,$9;nop":"=r"(end_cnt));
		end_cnt = read_tsc();
		
		if(end_cnt-start_cnt>(us*US_TICKS))
			break;
	}	
}		
#else
#define DELAY_MS(ms)  		osal_task_sleep(ms) //osal_delay(ms*1000) //dly_tsk(ms) //
#define DELAY_US(us)	 	osal_delay(us)
#endif

#define	SF_SUCCESS		0
#define SF_FAILURE		(-1)
#define SF_READ_TMO		(50*1000)	// 50ms

#define SF_BASE_ADDR		0xb8000000
#define	SF_INS			(SF_BASE_ADDR + 0x98)
#define	SF_FMT			(SF_BASE_ADDR + 0x99)
#define	SF_DUM			(SF_BASE_ADDR + 0x9A)
#define	SF_CFG			(SF_BASE_ADDR + 0x9B)

#define SF_HIT_DATA		0x01
#define SF_HIT_DUMM		0x02
#define SF_HIT_ADDR		0x04
#define SF_HIT_CODE		0x08
#define SF_CONT_RD		0x40
#define SF_CONT_WR		0x80

static unsigned long srflash_read_tmo = SF_READ_TMO;
static unsigned long srflash_mode;

static struct sto_device *srflash_dev = NULL;

static unsigned char  srflash_status;
static unsigned short srflash_devid = FLASHTYPE_UNKNOWN;

static void write_uint8(UINT32 addr, UINT8 val)
{
	erom_wm(addr, &val, 1, 0);
}

static UINT8 read_uint8(UINT32 addr)
{
	UINT8 data;

	erom_rm(addr, &data, 1, 0);
	return data;
}
static int read_uint8_tmo(UINT32 addr,UINT8 *buf,UINT32 tmo)
{
	return erom_read_tmo(addr,buf,1,0,tmo);
}	

static int srflash_wait_free()
{
	int ret = SF_FAILURE;
	UINT8 data;

	/* Set CODE_HIT to 1, ADDR_HIT, DATA_HIT and DUMMY_HIT to 0. */
	write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA);
	/* Set SFLASH_INS to RDSR instruction */
	write_uint8(SF_INS, 0x05);
	
	if(srflash_mode == SIMPLEX_MODE) 
	{
		if(FLASHTYPE_SF_SST25VF016B==srflash_devid) {		
			DELAY_US(10);
		} else {	
			DELAY_MS(10);
		}	
		ret = SF_SUCCESS;
	}
	else
	{
		if(remote_flash_read_tmo == 0){
		while (1)
		{
			if ((read_uint8(SYS_FLASH_BASE_ADDR) & 0x01) == 0){
			    ret = SF_SUCCESS;
			    break;
			}    
			osal_delay(10);
		}
		}else{
		while(1){
			if(0!=read_uint8_tmo(
				SYS_FLASH_BASE_ADDR,&data,srflash_read_tmo))
				return SF_FAILURE;

			if((data & 0x01)==0){
			    ret = SF_SUCCESS;
			    break;
			}

			osal_delay(10);
		}
		};/*REMOTE_FLASH_READ_TMO*/		
	}
	
	/* Reset sflash to common read mode */
	write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA);
	write_uint8(SF_INS, 0x03);
	
	return ret;
}

static void srflash_write_enable(int en)
{
	/* Set CODE_HIT to 1, ADDR_HIT, DATA_HIT and DUMMY_HIT to 0. */
	write_uint8(SF_FMT, SF_HIT_CODE);
	/* Set SFLASH_INS to WREN instruction. */
	if (en)
		write_uint8(SF_INS, 0x06);
	else
		write_uint8(SF_INS, 0x04);
	/* Write to any address of serial flash. */
	write_uint8(SYS_FLASH_BASE_ADDR, 0);

//    srflash_wait_free();
}

static int srflash_sec_protect(int en, UINT32 addr)
{
	UINT8 status;
	unsigned long flash_base_addr = SYS_FLASH_BASE_ADDR;

	/*Support M3329E and M3202 SFlash addr mapping in 8M or 16M*/
	if(ALI_M3329E==sys_ic_get_chip_id()||1==sys_ic_is_M3202())
	{
		flash_base_addr = SYS_FLASH_BASE_ADDR - (addr&0xc00000);
		addr &= 0x3fffff;
	}
	write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR);
	/* Set SFLASH_INS to WREN instruction. */
	if (en)
		write_uint8(SF_INS, 0x36);
	else
		write_uint8(SF_INS, 0x39);
	/* Write to any address of serial flash. */
	write_uint8(flash_base_addr + addr, 0);
		
		
	return 	SF_SUCCESS;

}

int srflash_control(unsigned long ctrl_cmd, unsigned long ctrl_val)
{
	if( FLASH_CONTROL==ctrl_cmd) {

		erom_wm(SF_INS,&ctrl_val,4,0);

	} else if (UPG_MODE_CONTROL==ctrl_cmd ) {

		srflash_mode =ctrl_val;

	} else if (FLASH_READ_TMO == ctrl_cmd ) {
	
		srflash_read_tmo = ctrl_val;

	} else {	
		
		srflash_devid = (unsigned short)ctrl_val;

	}

	return SF_SUCCESS;
}

/* Use continue-read to get ID of remote serial flash. */
void srflash_get_id(UINT32 *result, UINT32 cmdaddr)
{
	/* 1. Try ATMEL format get ID command */
	/* Set SFLASH_INS to RDID instruction. */
	write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA |  SF_CONT_RD);
	write_uint8(SF_INS, 0x9f);
	/* Read from any address of serial flash. */
	if(remote_flash_read_tmo == 0)
		erom_rm(SYS_FLASH_BASE_ADDR, &result[0], 4, 1);	
	else
	{	
		if(SUCCESS!=erom_read_tmo(SYS_FLASH_BASE_ADDR, &result[0], 4, 1,5000))
		{
			result[0] = 0;result[1] = 0;
			return;
		}		
	};/*REMOTE_FLASH_READ_TMO*/	

	/* recover to non-cont-rd mode for next operation*/
	write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA);
	srflash_wait_free();

	/* 2. Try EON format get ID command */
	/* Set SFLASH_INS to RDID instruction. */
	write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA |  SF_CONT_RD);
	write_uint8(SF_INS, 0xAB);  
	/* Read from any address of serial flash. */
	if(remote_flash_read_tmo == 0)
		erom_rm(SYS_FLASH_BASE_ADDR, &result[1], 4, 1);	
	else
	{
		if(SUCCESS!=erom_read_tmo(SYS_FLASH_BASE_ADDR, &result[1], 4, 1,5000))
		{
			result[0] = 0;result[1] = 0;
			return;
		}		
	};/*REMOTE_FLASH_READ_TMO*/	
	
	/* Reset sflash to common read mode */
	write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA);
	write_uint8(SF_INS, 0x03);

	/* Apply to SST25VF080B & SST25VF016B*/
	if( ((result[0]>>0)&0xFF)==0xBF &&
		((result[0]>>8)&0xFF)==0x25 )
		srflash_devid = FLASHTYPE_SF_SST25VF016B;		
}

int srflash_rwbp(unsigned char *sta_val, int is_read)
{
	volatile unsigned long j;

	//RDSR
	if(is_read!=0) {
		srflash_write_enable(1);
		write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA);
		write_uint8(SF_INS, 0x05);
		/* Read from any address of serial flash. */
		if(remote_flash_read_tmo == 0)
		*sta_val = read_uint8(SYS_FLASH_BASE_ADDR);
		else{
		if(0!=read_uint8_tmo(SYS_FLASH_BASE_ADDR,sta_val,srflash_read_tmo))
			return SF_FAILURE;
		DELAY_MS(10);
		};/*REMOTE_FLASH_READ_TMO*/	
	} else {

		unsigned char data8; 
		data8 = *((unsigned char *)SF_CFG);
		write_uint8(SF_CFG,(data8|0x10));
		DELAY_MS(10);
				
		//EWSR
		srflash_write_enable(1);
		write_uint8(SF_FMT, SF_HIT_CODE);
		write_uint8(SF_INS, 0x50);
		DELAY_MS(10);	
		
		//WRSR
		srflash_write_enable(1);
		write_uint8(SF_FMT, SF_HIT_DATA | SF_HIT_CODE);
		write_uint8(SF_INS, 0x01);
		write_uint8(SYS_FLASH_BASE_ADDR, *sta_val);
		DELAY_MS(10);
		
		write_uint8(SF_CFG,data8);
		DELAY_MS(10);
	}

	/* Reset sflash to common read mode */
	write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA);
	write_uint8(SF_INS, 0x03);
	
	return SUCCESS;	
}

int srflash_open( void *arg )
{
	unsigned char val = 0x80;

	if(FLASHTYPE_SF_SST25VF016B==srflash_devid) {

		if(srflash_mode == DUPLEX_MODE)
			srflash_rwbp(&srflash_status,1);
		srflash_rwbp(&val,0);	
	}
	
	srflash_dev = (struct sto_device *)arg;
	return SUCCESS;
}

int srflash_close( void *arg )
{
	if(FLASHTYPE_SF_SST25VF016B==srflash_devid) {
		if(srflash_mode == DUPLEX_MODE)
			srflash_rwbp(&srflash_status,0);
		srflash_devid = FLASHTYPE_UNKNOWN;		
	}
		
	srflash_dev = NULL;
	return SUCCESS;
}

int srflash_erase_chip(void)
{
	int i, sectors;
	unsigned long sec_addr;
	struct flash_private *tp;

	soc_printf("sflash erase chip!\n");

	if(FLASHTYPE_SF_SST25VF016B!=srflash_devid){
			
		/*** "srflash_dev" is a flag which may indicate that
		STO API is used by application.  ***/ 
		if(srflash_dev) {
			tp = (struct flash_private *)srflash_dev->priv ;
			sectors = tp->flash_sectors;
			for (i = 0; i < sectors; i++)
			{
				srflash_write_enable(1);
				sec_addr = sto_flash_sector_start(srflash_dev,i);
				if(SF_SUCCESS!=srflash_sec_protect(0, sec_addr))
					return SF_FAILURE;			
			}
		} else {
			for (i = 0; i < flash_info.flash_sectors; i++)
			{
				srflash_write_enable(1);
				if(SF_SUCCESS!=srflash_sec_protect(0, flash_sector_start(i)))
					return SF_FAILURE;
			}
		}

	}

	srflash_write_enable(1);
	/* Set CODE_HIT to 1, ADDR_HIT, DATA_HIT and DUMMY_HIT to 0. */
	write_uint8(SF_FMT, SF_HIT_CODE);
	/* Set SFLASH_INS to SE instruction. */
	write_uint8(SF_INS, 0xC7);
	/* Write to any address of serial flash. */
	write_uint8(SYS_FLASH_BASE_ADDR, 0);

	if(srflash_mode == SIMPLEX_MODE)
	{	
		DELAY_MS(8000);
	}	

    	return srflash_wait_free();
}

int srflash_erase_sector(UINT32 sector_addr)
{
	unsigned long flash_base_addr = SYS_FLASH_BASE_ADDR;

	soc_printf("sflash erase sector(%x)!\n",sector_addr);	
	
	srflash_write_enable(1);
	if(SF_SUCCESS!=srflash_sec_protect(0, sector_addr))
		return SF_FAILURE;
	/*Support M3329E and M3202 SFlash addr mapping in 8M or 16M*/
	if(ALI_M3329E==sys_ic_get_chip_id()||1==sys_ic_is_M3202())
	{
		flash_base_addr = SYS_FLASH_BASE_ADDR - (sector_addr&0xc00000);
		sector_addr &= 0x3fffff;
	}
	srflash_write_enable(1);
	/* Set CODE_HIT and ADDR_HIT to 1, DATA_HIT and DUMMY_HIT to 0. */
	write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR);
	/* Set SFLASH_INS to SE instruction. */
	write_uint8(SF_INS, 0xd8);
	/* Write to typical sector start address of serial flash. */
	write_uint8(flash_base_addr + sector_addr, 0);

	if(srflash_mode == SIMPLEX_MODE)
	{
		DELAY_MS(1000);
	}	

    	return srflash_wait_free();
}

int srflash_copy(UINT32 flash_addr, UINT8 *data, UINT32 len)
{
	UINT32 i;
	UINT32 end_cnt;
	UINT32 flash_base_addr = SYS_FLASH_BASE_ADDR;
	UINT32 original_flash_addr = flash_addr;
	soc_printf("sflash write(add:%x len:%x)!\n",flash_addr,len);
	if(!len)
		return SUCCESS;
	if(ALI_M3329E==sys_ic_get_chip_id()||1==sys_ic_is_M3202())
	{
		UINT32 cur_seg = (flash_addr&0xc00000)>>22;
		UINT32 tge_seg = ((flash_addr+len-1)&0xc00000)>>22;
		if(cur_seg!=tge_seg)
		{
			UINT32 inter_seg_len;

			inter_seg_len = ((cur_seg+1)<<22) - flash_addr;
			if(SUCCESS!=srflash_copy(flash_addr, data, inter_seg_len))
					return !SUCCESS;
			flash_addr += inter_seg_len;
			data += inter_seg_len;
			len -= inter_seg_len;
			cur_seg++;
			
			while(tge_seg != cur_seg)
			{
				if(SUCCESS!=srflash_copy(flash_addr, data, 0x400000))
					return !SUCCESS;
				flash_addr += 0x400000;
				data += 0x400000;
				len -= 0x400000;
				cur_seg++;
			}
			
			return srflash_copy(flash_addr, data, len);
		}
		else
		{
			flash_base_addr = SYS_FLASH_BASE_ADDR - (flash_addr&0xc00000);
			flash_addr &= 0x3fffff;
		}
	}
    if(FLASHTYPE_SF_SST25VF016B==srflash_devid) {

//#### AAI mode of SST serial flash    	
#if  (defined(USE_SW_EROM) && defined(SST_AAI_ENABLE))	
	
	//address unalignment
	if((flash_addr&0x1)!=0) {
   		srflash_write_enable(1);
    		/* Set CODE_HIT, ADDR_HIT, DATA_HIT to 1, DUMMY_HIT to 0. */
    		write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA);
    		/* Set SFLASH_INS to PP. */
    		write_uint8(SF_INS, 0x02);
    		/* Write to typical address of serial flash. */
		write_uint8(flash_base_addr + flash_addr, *data);	
		flash_addr ++; 	data ++; len--; original_flash_addr ++;
	}
	
	//address alignment and length is bigger than one byte
	if(len>2){

		//start AAI mode
		srflash_write_enable(1);
		write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA | SF_HIT_ADDR);
		write_uint8(SF_INS, 0xAD);	   	
	   	erom_wm(flash_base_addr + flash_addr,data,2,1);
	   	flash_addr += 2;
		original_flash_addr += 2;
	   	data +=2;
	   	len -=2;
	
	  	DELAY_US(10);
	   	
	   	//continue AAI
	   	end_cnt = len /2 ;
	   	len = len%2;
	   	for(i=0;i<end_cnt;i++){
			write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA);
			write_uint8(SF_INS, 0xAD);	   	
		   	erom_wm(flash_base_addr + flash_addr,data,2,1);   	
		   	flash_addr += 2; data +=2; original_flash_addr += 2;
	  		DELAY_US(10);
	   	}
	
		//terminate AAI mode by issuing WRDI command
		srflash_write_enable(0);  		
	
	   	//wait AAI finished
		if(SUCCESS!=srflash_wait_free())  	
	   		return SF_FAILURE;
	}   		
   	
	//any byte left ?
	if(len!=0) {
   		srflash_write_enable(1);
    		/* Set CODE_HIT, ADDR_HIT, DATA_HIT to 1, DUMMY_HIT to 0. */
    		write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA);
    		/* Set SFLASH_INS to PP. */
    		write_uint8(SF_INS, 0x02);
    		/* Write to typical address of serial flash. */
		write_uint8(flash_base_addr + flash_addr, *data);	

	     	if(SF_SUCCESS!=srflash_wait_free())
	    		return SF_FAILURE;  
	}   	
   	
   	return SUCCESS;	
   	
//#### #### ####   	
#else	/*SST_AAI_ENABLE*/
  	
   	// CONT-Read NOT support now!
        for(i = 0; i < len; i++)
        {
    		srflash_write_enable(1);
    		/* Set CODE_HIT, ADDR_HIT, DATA_HIT to 1, DUMMY_HIT to 0. */
    		write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA);
    		/* Set SFLASH_INS to PP. */
    		write_uint8(SF_INS, 0x02);
    		/* Write to typical address of serial flash. */
		write_uint8(flash_base_addr + flash_addr + i, *data++);

        }

     	if(SF_SUCCESS!=srflash_wait_free())
    		return SF_FAILURE;  
        
        return SUCCESS;

#endif	/*SST_AAI_ENABLE*/
    }	

    if (flash_addr & 0x03)
    {
        for(i = 0; i < (4 - (flash_addr & 0x03)) && len > 0; i++)
        {
    	
    		srflash_write_enable(1);
    		/* Set CODE_HIT, ADDR_HIT, DATA_HIT to 1, DUMMY_HIT to 0. */
    		write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA);
    		/* Set SFLASH_INS to PP. */
    		write_uint8(SF_INS, 0x02);
    		/* Write to typical address of serial flash. */
		write_uint8(flash_base_addr + flash_addr + i, *data++);
            	len--;
    		if(SF_SUCCESS!=srflash_wait_free())
    			return SF_FAILURE;

        }
        flash_addr += (4 - (flash_addr & 0x03));
	 original_flash_addr += (4 - (original_flash_addr & 0x03));	
		
    }
    for (i = 0; len > 0; flash_addr += end_cnt, original_flash_addr += end_cnt, len -= end_cnt)
    {
    	end_cnt = ((flash_addr + 0x100) & ~0xff) - flash_addr;	/* Num to align */
    	end_cnt = end_cnt > len ? len : end_cnt;
		srflash_write_enable(1);
		if(SF_SUCCESS!=srflash_sec_protect(0, original_flash_addr)){
			return SF_FAILURE;
		}	
		srflash_write_enable(1);
		/* Set CODE_HIT, ADDR_HIT, DATA_HIT to 1, DUMMY_HIT to 0. */
		write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA | SF_CONT_WR);
		//Special patch for s-flash chip from ESI
		if(ALI_M3329E==sys_ic_get_chip_id())
		{
			write_uint8(0xb8000094, (end_cnt&0xff));
			write_uint8(0xb8000095, ((end_cnt>>8)&0xff));
			write_uint8(0xb8000096,  read_uint8(0xb8000096)|1);
		}
		/* Set SFLASH_INS to PP. */
		write_uint8(SF_INS, 0x02);
		/* Write to typical address of serial flash. */
		for (i = 0; i < end_cnt; i++)
		{
			write_uint8(flash_base_addr + flash_addr + i, *data++);
	    	}
	    	//PRINTF("out\n");
		write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA);
		//Special patch for s-flash chip from ESI
		if(ALI_M3329E==sys_ic_get_chip_id())
		{
			write_uint8(0xb8000096,  read_uint8(0xb8000096)&(~1));
		}	
   		if(SF_SUCCESS!=srflash_wait_free()){
    			return SF_FAILURE;
    		}	
    }

    return len != 0;
}

int srflash_read(void *buffer, void *flash_addr, UINT32 len)
{
	UINT32 i, l;
	UINT32 flash_base_addr = SYS_FLASH_BASE_ADDR;
	UINT32 flash_offset = (UINT32)flash_addr-SYS_FLASH_BASE_ADDR;
	
	soc_printf("srflash read (addr %x, len %x)\n",(UINT32)flash_addr,len);
	if(!len)
		return SUCCESS;
	if(srflash_mode == SIMPLEX_MODE)
		return SF_FAILURE;	
	if(ALI_M3329E==sys_ic_get_chip_id()||1==sys_ic_is_M3202())
	{
		UINT32 cur_seg = (flash_offset&0xc00000)>>22;
		UINT32 tge_seg = ((flash_offset+len-1)&0xc00000)>>22;
		if(cur_seg!=tge_seg)
		{
			UINT32 inter_seg_len;

			inter_seg_len = ((cur_seg+1)<<22) - flash_offset;
			srflash_read(buffer, (void *)(flash_offset+SYS_FLASH_BASE_ADDR), inter_seg_len);
			flash_offset += inter_seg_len;
			buffer = (void *)((UINT32)buffer+inter_seg_len);
			len -= inter_seg_len;
			cur_seg++;
			
			while(tge_seg != cur_seg)
			{
				srflash_read(buffer, (void *)(flash_offset+SYS_FLASH_BASE_ADDR), 0x400000);
				flash_offset += 0x400000;
				buffer = (void *)((UINT32)buffer+0x400000);
				len -= 0x400000;
				cur_seg++;
			}
			
			return srflash_read(buffer, (void *)(flash_offset+SYS_FLASH_BASE_ADDR), len);
		}
		else
		{
			flash_base_addr = SYS_FLASH_BASE_ADDR - (flash_offset&0xc00000);
			flash_offset &= 0x3fffff;
		}	
	}
	flash_addr = (void *)(flash_base_addr+flash_offset);
	
	/* Read data in head not align with 4 bytes */
	if ((UINT32)flash_addr & 3)
	{
		l = 4 - ((UINT32)flash_addr & 3);
		l = l > len ? len : l;
		if(remote_flash_read_tmo == 0){ 			
		if(SUCCESS!=erom_rm(
			 (UINT32)flash_addr, 
					(void *)(buffer), l, 0))
			return ERR_FAILUE;
		}else{
		if(SUCCESS!=erom_read_tmo(
			 (UINT32)flash_addr, 
				(void *)(buffer), l, 0,srflash_read_tmo))
			return ERR_FAILUE;		
		};/*REMOTE_FLASH_READ_TMO*/					

		buffer=(void *)((UINT32)buffer + l);
		flash_addr=(void *)((UINT32)flash_addr +l);
		len -= l;
	}
	/* Read data in body align with 4 bytes */
	if (len > 0)
	{
		/* Set CODE_HIT, ADDR_HIT and DATA_HIT to 1, DUMMY_HIT to 0 */
		write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA | SF_CONT_RD);
		/* Set SFLASH_INS to SE insturciton. */
		write_uint8(SF_INS, 0x03);
		/* Read data */
		if(remote_flash_read_tmo == 0){ 					
		if(SUCCESS!=erom_rm(
			 (UINT32)flash_addr,
				 (void *)(buffer), len, 0))
			return ERR_FAILUE;	
		}else{
		if(SUCCESS!=erom_read_tmo(
			 (UINT32)flash_addr, 
				(void *)(buffer), len, 0,srflash_read_tmo))
		write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA);
  		if(SF_SUCCESS!=srflash_wait_free())
    			return SF_FAILURE;
		};/*REMOTE_FLASH_READ_TMO*/
	}

	return SF_SUCCESS;
}

int srflash_verify(unsigned int flash_addr, unsigned char *data, unsigned int len)
{
    unsigned char dst;
    unsigned char src;
    int ret = SUCCESS;
    UINT32 flash_base_addr = SYS_FLASH_BASE_ADDR;
    soc_printf("sflash verify(add:%x len:%x)!\n",flash_addr,len);
    if(!len)
		return ret;
	if(srflash_mode == SIMPLEX_MODE)
		return SF_FAILURE;	
	
	if(ALI_M3329E==sys_ic_get_chip_id()||1==sys_ic_is_M3202())
	{
		UINT32 cur_seg = (flash_addr&0xc00000)>>22;
		UINT32 tge_seg = ((flash_addr+len-1)&0xc00000)>>22;
		if(cur_seg!=tge_seg)
		{
			UINT32 inter_seg_len;

			inter_seg_len = ((cur_seg+1)<<22) - flash_addr;
			ret = srflash_verify(flash_addr, data, inter_seg_len);
			if(SUCCESS!=ret)
					return ret;
			flash_addr += inter_seg_len;
			data += inter_seg_len;
			len -= inter_seg_len;
			cur_seg++;
			
			while(tge_seg != cur_seg)
			{
				ret = srflash_verify(flash_addr, data, 0x400000);
				if(SUCCESS!=ret)
					return ret;
				flash_addr += 0x400000;
				data += 0x400000;
				len -= 0x400000;
				cur_seg++;
			}
			
			return srflash_verify(flash_addr, data, len);
		}
		else
		{
			flash_base_addr = SYS_FLASH_BASE_ADDR - (flash_addr&0xc00000);
			flash_addr &= 0x3fffff;
		}
	}	
	/* Set CODE_HIT, ADDR_HIT and DATA_HIT to 1, DUMMY_HIT to 0 */
	write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA | SF_CONT_RD);
	/* Set SFLASH_INS to SE insturciton. */
	write_uint8(SF_INS, 0x03);
	/* Read data */
	for (; len > 0; flash_addr++, len--)
	{
		if(remote_flash_read_tmo == 0)	    	
			dst=read_uint8(flash_base_addr + flash_addr);
		else{
			if(SUCCESS!=read_uint8_tmo(flash_base_addr + flash_addr,&dst,srflash_read_tmo))
		return 3;
	};
        src = *data++;
        dst ^= src;
        if (dst == 0)
            continue;
        ret = 1;
        if (dst & src)
        {
            ret = 2;
            break;
        }
    }
	write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA);

	return	srflash_wait_free();
}
