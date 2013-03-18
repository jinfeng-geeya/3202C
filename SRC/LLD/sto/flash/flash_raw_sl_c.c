/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2006 Copyright (C)
 *
 *  File: flash_raw_sl.c
 *
 *  Description: Provide local serial flash raw operations.
 *
 *  History:
 *      Date        Author      Version  Comment
 *      ====        ======      =======  =======
 *  1.  2006.4.24   Justin Wu   0.1.000  Initial
 *  2.  2006.11.10  Wen   Liu   0.2.000  Cleaning
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
#include "sto_flash.h"

//#define SUPPORT_GLOBAL_PROTECT
#define FLASHTYPE_SF_SST25VF 0xf5
//#define SYS_SFLASH_FAST_READ_SUPPORT

#define DELAY_MS(ms)  		osal_task_sleep(ms)
#define DELAY_US(us)		osal_delay(us)

#define SF_BASE_ADDR		0xb8000000
unsigned sflash_reg_addr = SF_BASE_ADDR;

unsigned int  SFLASH_IDENTIFY_MODE = 0;

#define	SF_INS			(sflash_reg_addr + 0x98)
#define	SF_FMT			(sflash_reg_addr + 0x99)
#define	SF_DUM			(sflash_reg_addr + 0x9A)
#define	SF_CFG			(sflash_reg_addr + 0x9B)

#define SF_HIT_DATA		0x01
#define SF_HIT_DUMM		0x02
#define SF_HIT_ADDR		0x04
#define SF_HIT_CODE		0x08
#define SF_CONT_RD		0x40
#define SF_CONT_WR		0x80

#define write_uint8(addr, val)	*((volatile UINT8 *)(addr)) = (val)
#define read_uint8(addr)	*((volatile UINT8 *)(addr))

unsigned short sflash_devid = FLASHTYPE_UNKNOWN;
static unsigned short aai_copy_enable[4] = {0,0,0,0};
	
#ifdef SUPPORT_GLOBAL_PROTECT		
static unsigned short global_protect_enable = 0;
static unsigned long gp_en_list[] = {
	0x001540ef, 0x14ef14ef,	//Winbond		25Q16VSIC
	0x001630ef, 0x15ef15ef,	//Winbond		25X32VSIC
	0xbf4125bf, 0x41bf41bf,	//SST		SST25VF016B
	0x0000461f, 0x00000000	//ATMEL		26DF161
};
#endif

static void sflash_wait_free(UINT8 chip_idx)
{
	UINT32 sys_flash_base_addr;
	sys_flash_base_addr=SYS_FLASH_BASE_ADDR-(chip_idx*0x1000000);
	
	/* Set CODE_HIT to 1, ADDR_HIT, DATA_HIT and DUMMY_HIT to 0. */
	write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA);
	/* Set SFLASH_INS to RDSR instruction */
	write_uint8(SF_INS, 0x05);
	/* Read from any address of serial flash. */
	while (1)
	{
		if((m_EnableSoftProtection && ((((volatile UINT8 *)unpro_addr_low)[0] & 0x01) == 0)) || 
			((!m_EnableSoftProtection) && ((((volatile UINT8 *)sys_flash_base_addr)[0] & 0x01) == 0)))
		break;
		osal_delay(10);
	}

	/* Reset sflash to common read mode */
#ifdef SYS_SFLASH_FAST_READ_SUPPORT
	write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA | SF_HIT_DUMM);
	write_uint8(SF_INS, 0x0b);
#else    
	write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA);
	write_uint8(SF_INS, 0x03);
#endif    
}

static void sflash_write_enable(UINT8 chip_idx, UINT8 en)
{
	UINT32 sys_flash_base_addr;
	sys_flash_base_addr=SYS_FLASH_BASE_ADDR-(chip_idx*0x1000000);
	/* Set CODE_HIT to 1, ADDR_HIT, DATA_HIT and DUMMY_HIT to 0. */
	write_uint8(SF_FMT, SF_HIT_CODE);
	/* Set SFLASH_INS to WREN instruction. */
	if (en)
		write_uint8(SF_INS, 0x06);
	else
		write_uint8(SF_INS, 0x04);
	/* Write to any address of serial flash. */
	if(m_EnableSoftProtection)
    	{
        	((volatile UINT8 *)unpro_addr_low)[0] = 0;
    	}else{
        	((volatile UINT8 *)sys_flash_base_addr)[0] = 0;
    	};
}

static void sflash_global_protect(UINT8 chip_idx, UINT8 en)
{
	UINT32 sys_flash_base_addr;
	sys_flash_base_addr=SYS_FLASH_BASE_ADDR-(chip_idx*0x1000000);
#ifdef SUPPORT_GLOBAL_PROTECT
	UINT8 status_reg;
	if(!global_protect_enable)
		return;
	write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA);
	write_uint8(SF_INS, 0x05);
	status_reg = ((volatile UINT8 *)sys_flash_base_addr)[0] ;
	if(en)
		status_reg |= 0x3c;
	else
		status_reg &= (~0x3c);
	
	sflash_write_enable(chip_idx, 1);
	write_uint8(SF_FMT, SF_HIT_DATA | SF_HIT_CODE);
	write_uint8(SF_INS, 0x01);
	 ((volatile UINT8 *)sys_flash_base_addr)[0] = status_reg;
	sflash_wait_free(chip_idx);
#endif
}


static void pd_detect_lsr(UINT32 param)
{
#define PDCNT	3
	static INT32 pd_cnt = PDCNT;
	if(pd_cnt>0)
		pd_cnt --;
	if(pd_cnt==0)
	{	
	#ifdef _DEBUG_VERSION_	
		#ifndef NO_POWERDOWN_TESTING
		powerdown_test();
	 	#endif
	#endif 
		pd_cnt = PDCNT;
	}
}

void sflash_get_id(UINT32 *result, UINT32 chip_idx)
{
	UINT32 sys_flash_base_addr;
	sys_flash_base_addr=SYS_FLASH_BASE_ADDR-(chip_idx*0x1000000);
	UINT32 chip_id = 0;
	chip_id = sys_ic_get_chip_id();
	
//check chip idx 
	if (ALI_S3602 == chip_id)
	{
		if( chip_idx >= 4) return;//support 4 chip,0~3
	}
	else if((ALI_M3329E == chip_id) && sys_ic_get_rev_id()>=IC_REV_5)//for 29E
	{
		if(chip_idx >=2 ) return;//support max 2 chips
	}
	else	{
			if(chip_idx >= 1) return;//for others,only one chip
		}

// change base addr
	if((ALI_M3329E==sys_ic_get_chip_id()&&sys_ic_get_rev_id()>=IC_REV_5) || (sys_ic_is_M3202() && (sys_ic_get_rev_id() >= IC_REV_4)))
	{
		sflash_reg_addr |= 0x8000;
		m_EnableSoftProtection = FALSE;
	}
	
	if(m_EnableSoftProtection)
	{
		sflash_soft_protect_init();
		osal_interrupt_register_lsr(7, pd_detect_lsr, 0);
	}
#ifdef _DEBUG_VERSION_	
      #ifndef NO_POWERDOWN_TESTING
	else
	{
		powerdown_test_init();
		osal_interrupt_register_lsr(7, pd_detect_lsr, 0);
	}
	#endif
#endif
	MUTEX_ENTER();
	/* Set CODE_HIT to 1, ADDR_HIT, DATA_HIT and DUMMY_HIT to 0. */
	write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA);
	/* 1. Try ATMEL format get ID command */
	/* Set SFLASH_INS to RDID instruction. */
	write_uint8(SF_INS, 0x9f);
	/* Read from any address of serial flash. */
	if(m_EnableSoftProtection)
	{
        	result[0] = ((volatile UINT32 *)unpro_addr_low)[0];
	}else{
        	result[0] = ((volatile UINT32 *)sys_flash_base_addr)[0];
	}

	/* 2. Try EON format get ID command */
	/* Set SFLASH_INS to RDID instruction. */
	write_uint8(SF_INS, 0xAB);
	/* Read from any address of serial flash. */
    	if(m_EnableSoftProtection)
    	{
        	result[1] = ((volatile UINT32 *)unpro_addr_low)[0];
    	}else
    	{
        	result[1] = ((volatile UINT32 *)sys_flash_base_addr)[0];
	}
#ifdef SUPPORT_GLOBAL_PROTECT	
	write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA|SF_HIT_DUMM|0x20);
	write_uint8(SF_INS, 0x90);
	if(m_EnableSoftProtection)
    	{
        	result[2] = ((volatile UINT32 *)unpro_addr_low)[0];
    	}else
    	{
        	result[2] = ((volatile UINT32 *)sys_flash_base_addr)[0];
	}
	{
		unsigned long i;
		global_protect_enable = 0;
		for(i=0; i<(sizeof(gp_en_list)/(4*2));i++)
		{
			if(result[0]==gp_en_list[i*2]&&result[2]==gp_en_list[i*2+1])
				global_protect_enable = 1;
		}
	}
#endif	
	if( (((result[0]>>0)&0xFF)==0xBF && ((result[0]>>8)&0xFF)==0x25 && ((result[0]>>16)&0xFF)!=0x4b) ||
	    (((result[0]>>0)&0xFF)==0x8C && (((result[0]>>8)&0xFF)==0x20 || ((result[0]>>8)&0xFF)==0x21)))
		aai_copy_enable[chip_idx] = 1;
	else
		aai_copy_enable[chip_idx] = 0;

	/* Remove all protection bits */
	/* Now only ATMEL26 and SST25VF with default protection */
	/* Sector unprotection dedicated from Atmel is unused */

	write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA);
	write_uint8(SF_INS, 0x05);
	//if((0xffffffff!=result[0])||(0xffffffff!=result[1])||(0xffffffff!=result[2]))
	if(((0xffffffff!=result[0])&&(0!=result[0]))
                ||((0xffffffff!=result[1])&&(0!=result[1]))
                ||((0xffffffff!=result[2])&&(0!=result[1])))
	{
		if(((volatile UINT8 *)sys_flash_base_addr)[0] & 0x3c){
			sflash_write_enable(chip_idx, 1);
			write_uint8(SF_FMT, SF_HIT_DATA | SF_HIT_CODE);
			write_uint8(SF_INS, 0x01);
		    	((volatile UINT8 *)sys_flash_base_addr)[0] = 0x00;
			sflash_wait_free(chip_idx);	
		}
	}
	sflash_global_protect(chip_idx, 1);
	/* Reset sflash to common read mode */
#ifdef SYS_SFLASH_FAST_READ_SUPPORT
	write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA | SF_HIT_DUMM);
	write_uint8(SF_INS, 0x0b);
#else
	write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA);
	write_uint8(SF_INS, 0x03);
#endif
	MUTEX_LEAVE();
}



int sflash_erase_chip(void)
{
	UINT8 fls_idx, max_chip_idx;
	UINT32 sys_flash_base_addr;
#if 1
	int i, sectors;
	unsigned long sec_addr;
	struct flash_private *tp;

	if(multi_flash_info.multi_chip)
	{
		if(ALI_S3602==sys_ic_get_chip_id())
		{
			max_chip_idx=3;
		}
		else
		{
			max_chip_idx=1;
		}
		fls_idx=0;
	}
	else
	{
		sys_flash_base_addr=SYS_FLASH_BASE_ADDR;
		switch(sys_flash_base_addr)
		{
		case 0xafc00000:
			fls_idx=0;
			break;
		case 0xaec00000:
			fls_idx=1;
			break;
		case 0xadc00000:
			fls_idx=2;
			break;
		case 0xacc00000:
			fls_idx=3;
			break;
		default:
			return ERR_FAILURE;
		}
		max_chip_idx=fls_idx;
	}

	for(i=fls_idx; i<=max_chip_idx; i++)
	{
		if(0==multi_flash_info.multi_chip_info[i].base_addr)
		{
			continue;
		}
		else
		{
			sys_flash_base_addr=multi_flash_info.multi_chip_info[i].base_addr;
		}
		MUTEX_ENTER();
		sflash_global_protect(i, 0);
		sflash_write_enable(i, 1);
		/* Set CODE_HIT to 1, ADDR_HIT, DATA_HIT and DUMMY_HIT to 0. */
		write_uint8(SF_FMT, SF_HIT_CODE);
		/* Set SFLASH_INS to SE instruction. */
		write_uint8(SF_INS, 0xC7);
		/* Write to any address of serial flash. */
		if(m_EnableSoftProtection)
		{
			((volatile UINT8 *)unpro_addr_low)[0] = 0;
		}else{
			((volatile UINT8 *)sys_flash_base_addr)[0] = 0;
		}

		sflash_wait_free(i);
		sflash_global_protect(i, 1);
		MUTEX_LEAVE();
	}
	return SUCCESS;

#endif
}

int sflash_erase_sector(UINT32 sector_addr)
{
	UINT32 sys_flash_base_addr;
#if 1
	UINT8 fls_idx=0;
	unsigned char data = 0;
	unsigned long flash_base_addr = 0;// = SYS_FLASH_BASE_ADDR;
	if(multi_flash_info.multi_chip)
	{
		for(fls_idx=0;fls_idx<4;fls_idx++)
		{
			if(sector_addr<multi_flash_info.multi_chip_info[fls_idx].flash_size)
			{
				break;
			}
			else
			{
				sector_addr-=multi_flash_info.multi_chip_info[fls_idx].flash_size;
			}
		}
		if(0!=multi_flash_info.multi_chip_info[fls_idx].base_addr)
		{
			sys_flash_base_addr = multi_flash_info.multi_chip_info[fls_idx].base_addr;
		}
		else
		{
			return ERR_FLASH_OFFSET_ERROR;
		}
	}
	else
	{
		sys_flash_base_addr=SYS_FLASH_BASE_ADDR;
	}
	
	/*Support M3329E and M3202 SFlash addr mapping in 8M or 16M*/
	if(ALI_M3329E==sys_ic_get_chip_id()||1==sys_ic_is_M3202()||ALI_S3602==sys_ic_get_chip_id()||1 == sys_ic_is_M3101())
	{
		flash_base_addr = sys_flash_base_addr - (sector_addr&0xc00000);
		sector_addr &= 0x3fffff;
	}
	MUTEX_ENTER();
	sflash_global_protect(fls_idx, 0);
	sflash_write_enable(fls_idx, 1);
	/* Set CODE_HIT and ADDR_HIT to 1, DATA_HIT and DUMMY_HIT to 0. */
	write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR);
	/* Set SFLASH_INS to SE instruction. */
	
	/* Write to typical sector start address of serial flash. */
	if(m_EnableSoftProtection&&(sflash_reg_addr == SF_BASE_ADDR))
	{
		sflash_write_soft_protect(sector_addr,&data,1, 0xd8);
	}else{
	#ifdef _DEBUG_VERSION_
	#ifndef NO_POWERDOWN_TESTING	
		powerdown_test();
	#endif
	#endif
		write_uint8(SF_INS, 0xd8);
		((volatile UINT8 *)flash_base_addr)[sector_addr] = 0;
	};

	sflash_wait_free(fls_idx);
    	sflash_global_protect(fls_idx, 1);
    	MUTEX_LEAVE();
	return SUCCESS;
#endif
}

int sflash_copy(UINT32 flash_addr, UINT8 *data, UINT32 len)
{
	UINT8 fls_idx=0;
	UINT32 sys_flash_base_addr;
#if 1
	UINT32 i;
	UINT32 end_cnt;
	UINT32 flash_base_addr = 0;// = SYS_FLASH_BASE_ADDR;
	UINT32 flash_start_addr;
	
	if(!len)
		return SUCCESS;

	//Find start address flash index and flash chip base address.
	flash_start_addr=flash_addr;
	if(multi_flash_info.multi_chip)
	{
		for(fls_idx=0;fls_idx<4;fls_idx++)
		{
			if(flash_start_addr<multi_flash_info.multi_chip_info[fls_idx].flash_size)
			{
				break;
			}
			else
			{
				flash_start_addr-=multi_flash_info.multi_chip_info[fls_idx].flash_size;
			}
		}
		if(0!=multi_flash_info.multi_chip_info[fls_idx].base_addr)
		{
			sys_flash_base_addr=multi_flash_info.multi_chip_info[fls_idx].base_addr;
		}
		else
		{
			return ERR_FLASH_OFFSET_ERROR;
		}
	}
	else
	{
		sys_flash_base_addr=SYS_FLASH_BASE_ADDR;
	}

	//cross chip burning
	if(ALI_M3329E==sys_ic_get_chip_id()||1==sys_ic_is_M3202()||ALI_S3602==sys_ic_get_chip_id()||1 == sys_ic_is_M3101())
	{
		UINT32 cur_chip;
		UINT32 tge_chip;
		UINT32 end_addr=flash_addr+len;
		
		cur_chip=fls_idx;
		if(multi_flash_info.multi_chip)
		{
			for(tge_chip=0;tge_chip<4;tge_chip++)//tge_chip starts from fls_idx
			{
				if(end_addr<=multi_flash_info.multi_chip_info[tge_chip].flash_size)
				{
					break;
				}
				else
				{
					end_addr-=multi_flash_info.multi_chip_info[tge_chip].flash_size;
				}
			}
		}
		else
		{
			tge_chip=cur_chip;
		}

		if(cur_chip!=tge_chip)//multi chip
		{
			UINT32 inter_chip_len;
			inter_chip_len=multi_flash_info.multi_chip_info[cur_chip].flash_size-flash_start_addr;
			if(SUCCESS!=sflash_copy(flash_addr, data, inter_chip_len))
				return !SUCCESS;
			flash_addr+=inter_chip_len;
			data+=inter_chip_len;
			len-=inter_chip_len;
			cur_chip++;//to next chip

			while(tge_chip != cur_chip)
			{
				if(SUCCESS!=sflash_copy(flash_addr, data, \
					multi_flash_info.multi_chip_info[cur_chip].flash_size))
				{
					return !SUCCESS;
				}
				flash_addr += multi_flash_info.multi_chip_info[cur_chip].flash_size;
				data += multi_flash_info.multi_chip_info[cur_chip].flash_size;
				len -= multi_flash_info.multi_chip_info[cur_chip].flash_size;
				cur_chip++;
			}//copy continuous chip

			return sflash_copy(flash_addr, data, len);//copy the rest data
		}
	}

	//cross segment burning
	if(ALI_M3329E==sys_ic_get_chip_id()||1==sys_ic_is_M3202()||ALI_S3602==sys_ic_get_chip_id()||1 == sys_ic_is_M3101())
	{
		UINT32 cur_seg = (flash_addr&0xc00000)>>22;
		UINT32 tge_seg = ((flash_addr+len-1)&0xc00000)>>22;//each 4M is a segment
		if(cur_seg!=tge_seg)
		{
			UINT32 inter_seg_len;

			inter_seg_len = ((cur_seg+1)<<22) - flash_start_addr;
			if(SUCCESS!=sflash_copy(flash_addr, data, inter_seg_len))
				return !SUCCESS;
			flash_addr += inter_seg_len;
			data += inter_seg_len;
			len -= inter_seg_len;
			cur_seg++;
			
			while(tge_seg != cur_seg)
			{
				if(SUCCESS!=sflash_copy(flash_addr, data, 0x400000))
					return !SUCCESS;
				flash_addr += 0x400000;
				data += 0x400000;
				len -= 0x400000;
				cur_seg++;
			}
			
			return sflash_copy(flash_addr, data, len);
		}
		else
		{
			flash_base_addr = sys_flash_base_addr - (flash_addr&0xc00000);//which segment
			flash_addr &= 0x3fffff;//offset in the segment
		}
	}
       MUTEX_ENTER(); 
        sflash_global_protect(fls_idx, 0);
	#ifdef SYS_PIN_MUX_MODE_04
	{
		   //soc_printf("continue write value:%x\n",SF_CONT_WR);
		// CONT-Write NOT support now!
		for(i = 0; i < len; i=i+1)
		{
			sflash_write_enable(fls_idx, 1);
			/* Set CODE_HIT, ADDR_HIT, DATA_HIT to 1, DUMMY_HIT to 0. */
			write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA);
			/* Set SFLASH_INS to PP. */
    			/* Write to typical address of serial flash. */
    			if(m_EnableSoftProtection&&(sflash_reg_addr == SF_BASE_ADDR))
    			{
    				sflash_write_soft_protect(flash_addr + i,data,1, 0x02);data++;
    			}else{
    			#ifdef _DEBUG_VERSION_
			#ifndef NO_POWERDOWN_TESTING		
    				powerdown_test();
			#endif
			#endif	
    				write_uint8(SF_INS, 0x02);
				((volatile UINT8 *)flash_base_addr)[flash_addr + i] =*data++;
				//osal_delay(50000);
    			};
    			sflash_wait_free(fls_idx);
			//sflash_write_enable(0);
		}
		sflash_global_protect(fls_idx, 1);
		MUTEX_LEAVE();
		return SUCCESS;
	}
	#else
 	if(aai_copy_enable[fls_idx]) 
 	{
		//align addresses
		if((flash_addr&0x1)!=0) 
		{
   			sflash_write_enable(fls_idx, 1);
    			/* Set CODE_HIT, ADDR_HIT, DATA_HIT to 1, DUMMY_HIT to 0. */
    			write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA);
		#ifdef SOFT_FLASH_PROTECTION
   			sflash_write_soft_protect(flash_addr,data,1, 0x02);
		#else
			#ifdef _DEBUG_VERSION_
			#ifndef NO_POWERDOWN_TESTING	
			powerdown_test();
			#endif
			#endif
    			/* Set SFLASH_INS to PP. */
    			write_uint8(SF_INS, 0x02);
    			/* Write to typical address of serial flash. */
			write_uint8(flash_base_addr + flash_addr, *data);
		#endif
			sflash_wait_free(fls_idx);
			flash_addr ++; 	
			data ++; 
			len--;
		}
		//start AAI on aligned address with even length
		if(len>=2)
		{
			//start AAI mode
			sflash_write_enable(fls_idx, 1);
			write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA | SF_HIT_ADDR);
		
			write_uint8(SF_INS, 0xAD);
	   		*((unsigned short *)(flash_base_addr + flash_addr)) = (*data)|(*(data+1))<<8;

	   		flash_addr += 2;
	   		data +=2;
	   		len -=2;

		  	sflash_wait_free(fls_idx);
		   	//continue AAI
		   	end_cnt = len /2 ;
		   	len = len%2;
		   	for(i=0;i<end_cnt;i++)
			{
				write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_DATA);
			#ifdef SOFT_FLASH_PROTECTION
		   		sflash_write_soft_protect(flash_addr,data,2, 0xad);
			#else
				#ifdef _DEBUG_VERSION_
				#ifndef NO_POWERDOWN_TESTING	
				powerdown_test();
				#endif
				#endif
				write_uint8(SF_INS, 0xAD);
		   		*((unsigned short *)(flash_base_addr + flash_addr)) = (*data)|(*(data+1))<<8;
			#endif
		   	   	flash_addr += 2; data +=2;
		  		sflash_wait_free(fls_idx);
		   	}

			//terminate AAI mode by issuing WRDI command
			sflash_write_enable(fls_idx, 0);
		   	//wait AAI finished
			sflash_wait_free(fls_idx);
		}

		//any byte left ?
		if(len!=0) 
		{
	   		sflash_write_enable(fls_idx, 1);
	    		/* Set CODE_HIT, ADDR_HIT, DATA_HIT to 1, DUMMY_HIT to 0. */
			write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA);
		#ifdef SOFT_FLASH_PROTECTION
			sflash_write_soft_protect(flash_addr,data,1, 0x02);
		#else
			#ifdef _DEBUG_VERSION_
			#ifndef NO_POWERDOWN_TESTING	
			powerdown_test();
			#endif
			#endif
			/* Set SFLASH_INS to PP. */
			write_uint8(SF_INS, 0x02);
			/* Write to typical address of serial flash. */
			write_uint8(flash_base_addr + flash_addr, *data);
		#endif
			sflash_wait_free(fls_idx);
		}
		sflash_global_protect(fls_idx, 1);
		MUTEX_LEAVE();
		return SUCCESS;
	}

	if (flash_addr & 0x03)
	{
		for(i = 0; i < (4 - (flash_addr & 0x03)) && len > 0; i++)
		{
			sflash_write_enable(fls_idx, 1);
			/* Set CODE_HIT, ADDR_HIT, DATA_HIT to 1, DUMMY_HIT to 0. */
			write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA);
			/* Set SFLASH_INS to PP. */
			/* Write to typical address of serial flash. */
			if(m_EnableSoftProtection&&(sflash_reg_addr == SF_BASE_ADDR))
			{
				sflash_write_soft_protect(flash_addr + i,data,1, 0x02);data++;
			}else{
			#ifdef _DEBUG_VERSION_
			#ifndef NO_POWERDOWN_TESTING	
				powerdown_test();
			#endif
			#endif
				write_uint8(SF_INS, 0x02);
				((volatile UINT8 *)flash_base_addr)[flash_addr + i] = *data++;
			};
			len--;
			sflash_wait_free(fls_idx);
		};
		flash_addr += (4 - (flash_addr & 0x03));
    	};
	for (i = 0; len > 0; flash_addr += end_cnt, len -= end_cnt)
	{
		end_cnt = ((flash_addr + 0x100) & ~0xff) - flash_addr;	/* Num to align */
		end_cnt = end_cnt > len ? len : end_cnt;
		sflash_write_enable(fls_idx, 1);
		/* Set CODE_HIT, ADDR_HIT, DATA_HIT to 1, DUMMY_HIT to 0. */
		write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA | SF_CONT_WR);
		/* Set SFLASH_INS to PP. */
		
		/* Write to typical address of serial flash. */
		//Special patch for s-flash chip from ESI
		if(ALI_M3329E==sys_ic_get_chip_id()&&sys_ic_get_rev_id()<=IC_REV_2)
		{
			write_uint8(sflash_reg_addr+0x94, (end_cnt&0xff));
			write_uint8(sflash_reg_addr+0x95, ((end_cnt>>8)&0xff));
			write_uint8(sflash_reg_addr+0x96,  read_uint8(sflash_reg_addr+0x96)|1);
		}
		if(m_EnableSoftProtection&&(sflash_reg_addr == SF_BASE_ADDR))
		{
			sflash_write_soft_protect(flash_addr,data,end_cnt, 0x02);data+=end_cnt;
		}else{
		#ifdef _DEBUG_VERSION_
		#ifndef NO_POWERDOWN_TESTING	
			powerdown_test();
		#endif
		#endif
			write_uint8(SF_INS, 0x02);
			for (i = 0; i < end_cnt; i++)
			{
				((volatile UINT8 *)flash_base_addr)[flash_addr + i] = *data++;
			};
		};
		write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA);
		//Special patch for s-flash chip from ESI
		if(ALI_M3329E==sys_ic_get_chip_id()&&sys_ic_get_rev_id()<=IC_REV_2)
		{
			write_uint8(sflash_reg_addr+0x96,  read_uint8(sflash_reg_addr+0x96)&(~1));
		}
		sflash_wait_free(fls_idx);
	};
	sflash_global_protect(fls_idx, 1);
	MUTEX_LEAVE();
	return len != 0;
	#endif
#endif
}

int sflash_read(void *buffer, void *flash_addr, UINT32 len)
{
	UINT8 fls_idx=0;
	UINT32 sys_flash_base_addr;
#if 1
	UINT32 i, l, tmp;
	UINT32 flash_base_addr = 0;// = SYS_FLASH_BASE_ADDR;
	UINT32 flash_start_addr;
	UINT32 flash_offset = (UINT32)flash_addr-SYS_FLASH_BASE_ADDR;

	if(!len)
		return SUCCESS;

	//Find start address flash index and flash base address
	flash_start_addr=flash_offset;
	if(multi_flash_info.multi_chip)
	{
		for(fls_idx=0;fls_idx<4;fls_idx++)
		{
			if(flash_start_addr<multi_flash_info.multi_chip_info[fls_idx].flash_size)
			{
				break;
			}
			else
			{
				flash_start_addr-=multi_flash_info.multi_chip_info[fls_idx].flash_size;
			}
		}
		if(0!=multi_flash_info.multi_chip_info[fls_idx].base_addr)
		{
			sys_flash_base_addr=multi_flash_info.multi_chip_info[fls_idx].base_addr;
		}
		else
		{
			return ERR_FLASH_OFFSET_ERROR;
		}
	}
	else
	{
		sys_flash_base_addr=SYS_FLASH_BASE_ADDR;
	}

	//cross chip reading
	if(ALI_M3329E==sys_ic_get_chip_id()||1==sys_ic_is_M3202()||ALI_S3602==sys_ic_get_chip_id()||1 == sys_ic_is_M3101())
	{
		UINT32 cur_chip;
		UINT32 tge_chip;
		UINT32 end_addr=flash_offset+len;
		
		cur_chip=fls_idx;
		if(multi_flash_info.multi_chip)
		{
			for(tge_chip=0;tge_chip<4;tge_chip++)
			{
				if(end_addr<=multi_flash_info.multi_chip_info[tge_chip].flash_size)
				{
					break;
				}
				else
				{
					end_addr-=multi_flash_info.multi_chip_info[tge_chip].flash_size;
				}
			}
		}
		else
		{
			tge_chip=cur_chip;
		}

		if(cur_chip!=tge_chip)
		{
			UINT32 inter_chip_len;
			inter_chip_len=multi_flash_info.multi_chip_info[cur_chip].flash_size-flash_start_addr;
			sflash_read(buffer, (void *)(flash_offset+SYS_FLASH_BASE_ADDR), inter_chip_len);
			flash_offset += inter_chip_len;
			buffer = (void *)((UINT32)buffer+inter_chip_len);
			len -= inter_chip_len;
			cur_chip++;
			
			while(tge_chip!=cur_chip)
			{
				sflash_read(buffer, (void *)(flash_offset+SYS_FLASH_BASE_ADDR), \
					multi_flash_info.multi_chip_info[cur_chip].flash_size);
				flash_offset += multi_flash_info.multi_chip_info[cur_chip].flash_size;
				buffer = (void *)((UINT32)buffer+multi_flash_info.multi_chip_info[cur_chip].flash_size);
				len -= multi_flash_info.multi_chip_info[cur_chip].flash_size;
				cur_chip++;
			}
			
			return sflash_read(buffer, (void *)(flash_offset+SYS_FLASH_BASE_ADDR), len);
		}
	}

	//cross segment reading.
	if(ALI_M3329E==sys_ic_get_chip_id()||1==sys_ic_is_M3202()||ALI_S3602==sys_ic_get_chip_id()||1 == sys_ic_is_M3101())
	{
		UINT32 cur_seg = (flash_offset&0xc00000)>>22;
		UINT32 tge_seg = ((flash_offset+len-1)&0xc00000)>>22;
		if(cur_seg!=tge_seg)
		{
			UINT32 inter_seg_len;

			inter_seg_len = ((cur_seg+1)<<22) - flash_start_addr;
			sflash_read(buffer, (void *)(flash_offset+SYS_FLASH_BASE_ADDR), inter_seg_len);
			flash_offset += inter_seg_len;
			buffer = (void *)((UINT32)buffer+inter_seg_len);
			len -= inter_seg_len;
			cur_seg++;
			
			while(tge_seg != cur_seg)
			{
				sflash_read(buffer, (void *)(flash_offset+SYS_FLASH_BASE_ADDR), 0x400000);
				flash_offset += 0x400000;
				buffer = (void *)((UINT32)buffer+0x400000);
				len -= 0x400000;
				cur_seg++;
			}
			
			return sflash_read(buffer, (void *)(flash_offset+SYS_FLASH_BASE_ADDR), len);
		}
		else
		{
			flash_base_addr = sys_flash_base_addr - (flash_offset&0xc00000);
			flash_offset &= 0x3fffff;
		}	
	}
	flash_addr = (void *)(flash_base_addr+flash_offset);
	MUTEX_ENTER();
      #ifndef SYS_PIN_MUX_MODE_04
	/* Read data in head not align with 4 bytes */
	if ((UINT32)flash_addr & 3)
	{
		l = 4 - ((UINT32)flash_addr & 3);
		l = l > len ? len : l;
		MEMCPY(buffer, flash_addr, l);
		buffer=(void *)((UINT32)buffer + l);
		flash_addr=(void *)((UINT32)flash_addr +l);
		len -= l;
	}
	/* Read data in body align with 4 bytes */
	if (len > 0)
	{
#ifdef SYS_SFLASH_FAST_READ_SUPPORT
        /* Set CODE_HIT, ADDR_HIT and DATA_HIT to 1, DUMMY_HIT to 0 */
		write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA | SF_CONT_RD | SF_HIT_DUMM);
        /* Set SFLASH_INS to SE insturciton. */
		write_uint8(SF_INS, 0x0b);
#else
		/* Set CODE_HIT, ADDR_HIT and DATA_HIT to 1, DUMMY_HIT to 0 */
		write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA | SF_CONT_RD);
		/* Set SFLASH_INS to SE insturciton. */
		write_uint8(SF_INS, 0x03);
#endif
		/* Read data */
		if (((UINT32)buffer & 3) == 0)
		{
			for (i = 0; i < (len&(~0x3)); i+=4){
				*(UINT32 *)((UINT32)buffer + i) = *(UINT32 *)((UINT32)flash_addr + i);
			}
		}else{
			for (i = 0; i < (len&(~0x3)); i+=4)
			{
				tmp = *(UINT32 *)((UINT32)flash_addr + i);
				*(UINT8 *)((UINT32)buffer + i) = tmp & 0xff;
				*(UINT8 *)((UINT32)buffer + i + 1) = ((tmp >> 8) & 0xff);
				*(UINT8 *)((UINT32)buffer + i + 2) = ((tmp >> 16)& 0xff);
				*(UINT8 *)((UINT32)buffer + i + 3) = ((tmp >> 24) & 0xff);
			}
		}
		flash_addr = (UINT8 *)flash_addr + (len&(~0x3));
		buffer = (UINT8 *)buffer + (len&(~0x3));
		len -= (len&(~0x3));
	};
	write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA);
	sflash_wait_free(fls_idx);
	#endif
	MEMCPY(buffer, flash_addr, len);
	MUTEX_LEAVE();
	return SUCCESS;
#endif
}

int sflash_verify(unsigned int flash_addr, unsigned char *data, unsigned int len)
{
	UINT8 fls_idx=0;
	UINT32 sys_flash_base_addr;
#if 1
	unsigned char dst;
	unsigned char src;
	int ret = SUCCESS;
	UINT32 flash_base_addr = 0;// = SYS_FLASH_BASE_ADDR;
	UINT32 flash_start_addr;

	if(!len)
		return ret;

	//Find start address flash index and flash base address
	flash_start_addr=flash_addr;
	if(multi_flash_info.multi_chip)
	{
		for(fls_idx=0;fls_idx<4;fls_idx++)
		{
			if(flash_start_addr<multi_flash_info.multi_chip_info[fls_idx].flash_size)
			{
				break;
			}
			else
			{
				flash_start_addr-=multi_flash_info.multi_chip_info[fls_idx].flash_size;
			}
		}
		if(0!=multi_flash_info.multi_chip_info[fls_idx].base_addr)
		{
			sys_flash_base_addr=multi_flash_info.multi_chip_info[fls_idx].base_addr;
		}
		else
		{
			return ERR_FLASH_OFFSET_ERROR;
		}
	}
	else
	{
		sys_flash_base_addr=SYS_FLASH_BASE_ADDR;
	}

	//cross chip verify
	if(ALI_M3329E==sys_ic_get_chip_id()||1==sys_ic_is_M3202()||ALI_S3602==sys_ic_get_chip_id()||1 == sys_ic_is_M3101())
	{
		UINT32 cur_chip;
		UINT32 tge_chip;
		UINT32 end_addr=flash_addr+len;
		
		cur_chip=fls_idx;
		if(multi_flash_info.multi_chip)
		{
			for(tge_chip=0;tge_chip<4;tge_chip++)
			{
				if(end_addr<=multi_flash_info.multi_chip_info[tge_chip].flash_size)
				{
					break;
				}
				else
				{
					end_addr-=multi_flash_info.multi_chip_info[tge_chip].flash_size;
				}
			}
		}
		else
		{	
			tge_chip=cur_chip;
		}

		if(cur_chip!=tge_chip)
		{
			UINT32 inter_chip_len;

			inter_chip_len = multi_flash_info.multi_chip_info[cur_chip].flash_size - flash_start_addr;
			ret = sflash_verify(flash_addr, data, inter_chip_len);
			if(SUCCESS!=ret)
			{
				return ret;
			}
			flash_addr += inter_chip_len;
			data += inter_chip_len;
			len -= inter_chip_len;
			cur_chip++;
			
			while(tge_chip!=cur_chip)
			{
				ret = sflash_verify(flash_addr, data, \
					multi_flash_info.multi_chip_info[cur_chip].flash_size);
				if(SUCCESS!=ret)
				{
					return ret;
				}
				flash_addr += multi_flash_info.multi_chip_info[cur_chip].flash_size;
				data += multi_flash_info.multi_chip_info[cur_chip].flash_size;
				len -= multi_flash_info.multi_chip_info[cur_chip].flash_size;
				cur_chip++;
			}
			
			return sflash_verify(flash_addr, data, len);
		}
	}

	//cross segment verify.
	if(ALI_M3329E==sys_ic_get_chip_id()||1==sys_ic_is_M3202()||ALI_S3602==sys_ic_get_chip_id()||1 == sys_ic_is_M3101())
	{
		UINT32 cur_seg = (flash_addr&0xc00000)>>22;
		UINT32 tge_seg = ((flash_addr+len-1)&0xc00000)>>22;
		if(cur_seg!=tge_seg)
		{
			UINT32 inter_seg_len;

			inter_seg_len = ((cur_seg+1)<<22) - flash_start_addr;
			ret = sflash_verify(flash_addr, data, inter_seg_len);
			if(SUCCESS!=ret)
					return ret;
			flash_addr += inter_seg_len;
			data += inter_seg_len;
			len -= inter_seg_len;
			cur_seg++;
			
			while(tge_seg != cur_seg)
			{
				ret = sflash_verify(flash_addr, data, 0x400000);
				if(SUCCESS!=ret)
					return ret;
				flash_addr += 0x400000;
				data += 0x400000;
				len -= 0x400000;
				cur_seg++;
			}
			
			return sflash_verify(flash_addr, data, len);
		}
		else
		{
			flash_base_addr = sys_flash_base_addr - (flash_addr&0xc00000);
			flash_addr &= 0x3fffff;
		}
	}
	MUTEX_ENTER();
#ifdef SYS_SFLASH_FAST_READ_SUPPORT
    /* Set CODE_HIT, ADDR_HIT and DATA_HIT to 1, DUMMY_HIT to 0 */
	write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA | SF_HIT_DUMM);
    /* Set SFLASH_INS to SE insturciton. */
	write_uint8(SF_INS, 0x0b);
#else
	/* Set CODE_HIT, ADDR_HIT and DATA_HIT to 1, DUMMY_HIT to 0 */
	write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA);
	/* Set SFLASH_INS to SE insturciton. */
	write_uint8(SF_INS, 0x03);
#endif

	/* Read data */
	for (; len > 0; flash_addr++, len--)
	{
		dst = ((volatile UINT8 *)flash_base_addr)[flash_addr];
		src = *data++;
		dst ^= src;
		if (dst == 0)
			continue;
		ret = 1;//different,return 2 means need erase,1-0 return 1,0-1 return 2
		if (dst & src) //
		{
			ret = 2;
			break;
		}
	};
	write_uint8(SF_FMT, SF_HIT_CODE | SF_HIT_ADDR | SF_HIT_DATA);
	sflash_wait_free(fls_idx);
	MUTEX_LEAVE();
	return ret;
#endif
}
