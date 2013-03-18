/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2005 Copyright (C)
 *
 *  File: flash_raw_pl.c
 *
 *  Description: Provide local paral flash raw operations.
 *
 *  History:
 *      Date        Author      Version  Comment
 *      ====        ======      =======  =======
 *  1.  2005.5.28   Liu Lan     0.1.000  Initial
 *  2.  2006.4.28   Justin Wu   0.2.000  Re-arch
 *
 ****************************************************************************/

#include <types.h>
#include <sys_config.h>
#include <osal/osal.h>
#include "sto_flash.h"
#include <hld/sto/sto_dev.h>
#define US_TICKS        (sys_ic_get_cpu_clock()*1000000 / 2000000)
#define WAIT_5ms        (5000* US_TICKS)

static unsigned long flash_cmdaddr;
static unsigned char flash_8bit_mode = 1;

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


void pflash_get_id(unsigned long *result, unsigned long cmdaddr)
{
    unsigned long read_o0, read_o1, read_o2;
    unsigned long read_n0, read_n1, read_n2;
    unsigned long base_addr = SYS_FLASH_BASE_ADDR;

    if(ALI_M3329E==sys_ic_get_chip_id())
    {
    	UINT32 strap_pin = *((volatile UINT32 *)0xb8000070);
	if(0x8000==(strap_pin&0x18880))
		flash_8bit_mode = 0;
		
    }
    if(m_EnableSoftProtection&&flash_8bit_mode)
    {
        pflash_soft_protect_init();
		osal_interrupt_register_lsr(7, pd_detect_lsr, 0);
    }
#ifdef _DEBUG_VERSION_	
   #ifndef NO_POWERDOWN_TESTING
    else
	  powerdown_test_init();	
		osal_interrupt_register_lsr(7, pd_detect_lsr, 0);
    #endif	
#endif	
    MUTEX_ENTER();	
    flash_cmdaddr = cmdaddr;
    read_o0 = ((volatile UINT32 *)base_addr)[0];
    read_o1 = ((volatile UINT32 *)base_addr)[64];
    read_o2 = ((volatile UINT32 *)base_addr)[7];

    /* Step 1: Send the Auto Select command */
    if(flash_8bit_mode)	
    {
      ((volatile UINT8 *)base_addr)[flash_cmdaddr] = 0xAA;
      ((volatile UINT8 *)base_addr)[flash_cmdaddr >> 1] = 0x55;
      ((volatile UINT8 *)base_addr)[flash_cmdaddr] = 0x90;
    }
    else
    {
       ((volatile UINT16 *)base_addr)[flash_cmdaddr] = 0xAA;
       ((volatile UINT16 *)base_addr)[flash_cmdaddr >> 1] = 0x55;
       ((volatile UINT16 *)base_addr)[flash_cmdaddr] = 0x90;
    }
    read_n0 = ((volatile UINT32 *)base_addr)[0];
    read_n1 = ((volatile UINT32 *)base_addr)[64];
    read_n2 = ((volatile UINT32 *)base_addr)[7];
    if(flash_8bit_mode)	
      ((volatile UINT8 *)base_addr)[0] = 0xF0;
    else
	((volatile UINT16 *)base_addr)[0] = 0xF0;	

    result[0] = read_o0 == read_n0 ? 0 : read_n0;
    result[1] = read_o1 == read_n1 ? 0 : read_n1;
    result[2] = read_o2 == read_n2 ? 0 : read_n2;
    MUTEX_LEAVE();
    return;
}


int pflash_erase_chip(void)
{
    unsigned long tick;
    unsigned long base_addr = SYS_FLASH_BASE_ADDR;
    MUTEX_ENTER();	
    if(flash_8bit_mode)	
    {
    	((volatile UINT8 *)base_addr)[flash_cmdaddr] = 0xAA;
    	((volatile UINT8 *)base_addr)[flash_cmdaddr >> 1] = 0x55;
    	((volatile UINT8 *)base_addr)[flash_cmdaddr] = 0x80;
    	((volatile UINT8 *)base_addr)[flash_cmdaddr] = 0xAA;
    	((volatile UINT8 *)base_addr)[flash_cmdaddr >> 1] = 0x55;
    	((volatile UINT8 *)base_addr)[flash_cmdaddr] = 0x10;
    }
    else
    {
    	((volatile UINT16 *)base_addr)[flash_cmdaddr] = 0xAA;
    	((volatile UINT16 *)base_addr)[flash_cmdaddr >> 1] = 0x55;
    	((volatile UINT16 *)base_addr)[flash_cmdaddr] = 0x80;
    	((volatile UINT16 *)base_addr)[flash_cmdaddr] = 0xAA;
    	((volatile UINT16 *)base_addr)[flash_cmdaddr >> 1] = 0x55;
    	((volatile UINT16 *)base_addr)[flash_cmdaddr] = 0x10;    
    }
    while (1)
    {
        tick = read_tsc();
        while (read_tsc() - tick < (unsigned long)WAIT_5ms) ;
        if (((volatile UINT8 *)base_addr)[0] == 0xFF)
        {
            MUTEX_LEAVE();
            return SUCCESS;
        }
    }
}


int pflash_erase_sector(unsigned int sector_addr)
{
    unsigned long tick;
    unsigned long base_addr = SYS_FLASH_BASE_ADDR;
    MUTEX_ENTER();
    if(flash_8bit_mode)	
    {
    	((volatile UINT8 *)base_addr)[flash_cmdaddr] = 0xAA;
    	((volatile UINT8 *)base_addr)[flash_cmdaddr >> 1] = 0x55;
    	((volatile UINT8 *)base_addr)[flash_cmdaddr] = 0x80;
    	((volatile UINT8 *)base_addr)[flash_cmdaddr] = 0xAA;
    	((volatile UINT8 *)base_addr)[flash_cmdaddr >> 1] = 0x55;
    }
    else
    {
    	((volatile UINT16 *)base_addr)[flash_cmdaddr] = 0xAA;
    	((volatile UINT16 *)base_addr)[flash_cmdaddr >> 1] = 0x55;
    	((volatile UINT16 *)base_addr)[flash_cmdaddr] = 0x80;
    	((volatile UINT16 *)base_addr)[flash_cmdaddr] = 0xAA;
    	((volatile UINT16 *)base_addr)[flash_cmdaddr >> 1] = 0x55;    
    }
    if(m_EnableSoftProtection&&flash_8bit_mode)
        pflash_write_soft_protect(sector_addr,0x30);
    else
    {
    	#ifdef _DEBUG_VERSION_
	#ifndef NO_POWERDOWN_TESTING		
        powerdown_test();
	#endif
	#endif
	if(flash_8bit_mode)
          ((volatile UINT8 *)base_addr)[sector_addr] = 0x30;
	else
	   ((volatile UINT16 *)base_addr)[sector_addr>>1] = 0x30;	
    }
    while (1)
    {
        tick = read_tsc();
        while (read_tsc() - tick < (unsigned long)WAIT_5ms) ;
        if (((volatile UINT8 *)base_addr)[sector_addr] == 0xFF)
        {
            MUTEX_LEAVE();
            return SUCCESS;
        }
    }
}

int pflash_copy16(unsigned int flash_addr, unsigned char *data, unsigned int len)
{
    unsigned short dst;
    unsigned short src;
    unsigned long base_addr = SYS_FLASH_BASE_ADDR;
    unsigned long len_align;
    MUTEX_ENTER();
    if(flash_addr&1)
    {
    	dst = ((volatile UINT16 *)base_addr)[flash_addr>>1];
	src = ((*data)<<8)|(dst&0xff);
	dst ^= src;
	if(dst)
	{
	    if(dst & src)
	    {
	        MUTEX_LEAVE();
	        return 1;
	    }
	    ((volatile UINT16 *)base_addr)[flash_cmdaddr] = 0xAA;
           ((volatile UINT16 *)base_addr)[flash_cmdaddr >> 1] = 0x55;
           ((volatile UINT16 *)base_addr)[flash_cmdaddr] = 0xA0;
           #ifdef _DEBUG_VERSION_	
	    #ifndef NO_POWERDOWN_TESTING	
               powerdown_test();
	     #endif
	     #endif	
             ((volatile UINT16 *)base_addr)[flash_addr>>1] = src;
              while (((volatile UINT16 *)base_addr)[flash_addr>>1] != src) ;	
	}
	flash_addr++;
	data++; 
	len --;
    }
    len_align = len&(~1);
    len = len&1;	
    for (; len_align > 0; flash_addr += 2, len_align -= 2)
    {
        dst = ((volatile UINT16 *)base_addr)[flash_addr>>1];
	 if((UINT32)data&1)
        	src = data[0]|(data[1]<<8);
	 else
		src = *((UINT16 *)data);
	 data += 2;
        dst ^= src;
        if (dst == 0)
            continue;
        if (dst & src)
            break;

        ((volatile UINT16 *)base_addr)[flash_cmdaddr] = 0xAA;
        ((volatile UINT16 *)base_addr)[flash_cmdaddr >> 1] = 0x55;
        ((volatile UINT16 *)base_addr)[flash_cmdaddr] = 0xA0;
        #ifdef _DEBUG_VERSION_	
	 #ifndef NO_POWERDOWN_TESTING	 	
         powerdown_test();
	 #endif
	 #endif	
          ((volatile UINT16 *)base_addr)[flash_addr>>1] = src;
        
        while (((volatile UINT16 *)base_addr)[flash_addr>>1] != src) ;
    }
    if(len)
    {
    	dst = ((volatile UINT16 *)base_addr)[flash_addr>>1];
	src = (*data)|(dst&0xff00);
	dst ^= src;
	if(dst)
	{
	    if(dst & src)
	    {
	        MUTEX_LEAVE();
	        return 1;
	    }
	    ((volatile UINT16 *)base_addr)[flash_cmdaddr] = 0xAA;
           ((volatile UINT16 *)base_addr)[flash_cmdaddr >> 1] = 0x55;
           ((volatile UINT16 *)base_addr)[flash_cmdaddr] = 0xA0;
           #ifdef _DEBUG_VERSION_	
	     #ifndef NO_POWERDOWN_TESTING	   
               powerdown_test();
	     #endif 
	     #endif	
             ((volatile UINT16 *)base_addr)[flash_addr>>1] = src;
              while (((volatile UINT16 *)base_addr)[flash_addr>>1] != src) ;	
	}
	flash_addr++;
	data++; 
	len --;
    }
    MUTEX_LEAVE();	
    return len != 0;
}

int pflash_copy(unsigned int flash_addr, unsigned char *data, unsigned int len)
{
    unsigned char dst;
    unsigned char src;
    unsigned long base_addr = SYS_FLASH_BASE_ADDR;
    if(0==len)
	return 0;	
    if(0==flash_8bit_mode)
	return pflash_copy16(flash_addr, data, len);	
    MUTEX_ENTER();	
    for (; len > 0; flash_addr++, len--)
    {
        dst = ((volatile UINT8 *)base_addr)[flash_addr];
        src = *data++;
        dst ^= src;
        if (dst == 0)
            continue;
        if (dst & src)
            break;

        ((volatile UINT8 *)base_addr)[flash_cmdaddr] = 0xAA;
        ((volatile UINT8 *)base_addr)[flash_cmdaddr >> 1] = 0x55;
        ((volatile UINT8 *)base_addr)[flash_cmdaddr] = 0xA0;
        if(m_EnableSoftProtection&&flash_8bit_mode)
            pflash_write_soft_protect(flash_addr,src);
        else
        {
        #ifdef _DEBUG_VERSION_	
	  #ifndef NO_POWERDOWN_TESTING		
            powerdown_test();
	  #endif
	#endif	
            ((volatile UINT8 *)base_addr)[flash_addr] = src;
        }
        while (((volatile UINT8 *)base_addr)[flash_addr] != src) ;
    }
    MUTEX_LEAVE();
    return len != 0;
}

int pflash_read( void *des, void* src, UINT32 len)
{
	MUTEX_ENTER();    
	if(flash_8bit_mode)
		libc_memcpy(des,src,len);
	else
	{
		UINT32 i;
		UINT8 remain_one = 0;
		UINT16 tmp;
		UINT8 * buf = (UINT8 *)des;
		UINT8 * flash_addr = (UINT8 *)src;
		if(((UINT32)flash_addr)&1)
		{
			tmp = *((volatile UINT16 *)((UINT32)flash_addr));
			buf[0] = (UINT8)(tmp>>8);
			flash_addr ++;
			buf++;
			len--;
		}
		if(len&1)
		{
			remain_one = 1;
			len --;
		}
		if(((UINT32)buf)&1)
		{
			for(i=0; i<len; i += 2)
			{
				tmp = *((volatile UINT16 *)((UINT32)flash_addr+i));
				buf[i] = (UINT8)tmp;
				buf[i+1] = (UINT8)(tmp>>8);
			}
		}
		else
		{
			UINT16 * buf16 = (UINT16 *)buf;
			for(i=0; i<len; i += 2)
				buf16[i>>1] = *((volatile UINT16 *)((UINT32)flash_addr+i));
		}
		if(remain_one)
		{
			tmp = *((volatile UINT16 *)((UINT32)flash_addr+i));
			buf[len] = (UINT8)tmp;
		}
	}
	MUTEX_LEAVE();
	return SUCCESS;
}

int pflash_verify(unsigned int flash_addr, unsigned char *data, unsigned int len)
{
	unsigned char dst;
	unsigned char src;
	unsigned long base_addr = SYS_FLASH_BASE_ADDR;
	int ret = 0;
	MUTEX_ENTER();
	if(flash_8bit_mode)
	{
		for (; len > 0; flash_addr++, len--)
		{
			dst = ((volatile UINT8 *)base_addr)[flash_addr];
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
	}
	else
	{
		unsigned short dst16;
		unsigned short src16;
		UINT8 remain_one = 0;
		if(flash_addr&1)
		{
			dst16 = ((volatile UINT16 *)base_addr)[flash_addr++];
			dst = (unsigned char)(dst16>>8);
			src = *data++;
			dst ^= src;
			len --;
			if (dst!= 0)
			{	
				ret = 1;
				if (dst & src)
				{
					ret = 2;
					{
						MUTEX_LEAVE();	
						return ret;
					}
				}
			}
		}
		if(len&1)
		{
			remain_one = 1;
			len--;
		}
		for (; len > 0; flash_addr += 2, len -= 2)
		{
			dst16 = ((volatile UINT16 *)base_addr)[flash_addr];
			src16 = data[0]|(data[1]<<8);
			data += 2;
			dst16 ^= src16;
			if (dst16 == 0)
				continue;
			ret = 1;
			if (dst16 & src16)
			{
				ret = 2;
				MUTEX_LEAVE();	
				return ret;
			}
		}
		if(remain_one)
		{
			dst16 = ((volatile UINT16 *)base_addr)[flash_addr];
			dst = (unsigned char)dst16;
			src = *data++;
			dst ^= src;
			if (dst!= 0)
			{	
				ret = 1;
				if (dst & src)
				{
					ret = 2;
					MUTEX_LEAVE();
					return ret;
				}
			}
		}
	}
	MUTEX_LEAVE();
	return ret;
}
