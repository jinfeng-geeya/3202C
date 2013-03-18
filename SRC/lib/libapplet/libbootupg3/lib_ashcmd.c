/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2002 Copyright (C)
 *
 *  File: lib_ashcmd.c
 *
 *  Description: This file contains all functions definition ALi shell
 *		         command functions.
 *  History:
 *      Date        Author      Version  Comment
 *      ====        ======      =======  =======
 *  1   2004.12.13  Hong Zhang  MTW_3327 Initial
 *  2   2005.1.31   Hong Zhang 128B header
 *  3   2005.2.16   Hong Zhang  Comext Initial
 *  4   2006.5.16   Justin Wu   Update STO chunck support, and clean up code.
 ****************************************************************************/

#include <types.h>
#include <retcode.h>
#include <sys_config.h>
#include <bus/dog/dog.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libc/fastCRC.h>
#include <api/libchunk/chunk.h>
#include <bus/sci/sci.h>
#include <bus/flash/flash.h>
#include <hld/hld_dev.h>

#include "lib_ash.h"
#include "errorcode.h"
#include "packet.h"

#ifndef ENABLE_EROM
#if 0//def _DEBUG_VERSION_
#define DEBUG
#define SH_PRINTF   soc_printf
#else
#define SH_PRINTF(...)
#endif

#define PACKAGE_SIZE    1024
#if (SYS_SDRAM_SIZE == 2)
#define IMAGE_START     0x80080000
#else
	#if (SYS_SDRAM_SIZE >= 64)
		#define IMAGE_START     0x80200000
		#define ZIP_BUFFER	0x82000000 //0x81000000 for 32M SDRAM
	#elif (SYS_SDRAM_SIZE >= 16)
		#define IMAGE_START     0x80200000
		#define ZIP_BUFFER	0x80C00000 //0x81000000 for 32M SDRAM
	#else
		#define IMAGE_START     0x80200000
		#define ZIP_BUFFER	0x80400000
	#endif
#endif

#define LZMA_BASE_SIZE	1846
#define LZMA_LIT_SIZE	768
#define BUFFER_SIZE     ((LZMA_BASE_SIZE + (LZMA_LIT_SIZE << (0 + 2))) * sizeof(UINT16))
extern UINT32 g_last_status;
#ifdef PANEL_DISPLAY
extern struct pan_device* panel_dev;
#endif
extern unsigned char g_crc_bad;
extern struct sto_device *flash_dev;

static unsigned long start_addr;
static unsigned char *image_free;

static unsigned char *image_buf_addr = (char *)IMAGE_START;
static unsigned char *zip_buf_addr = (char *)ZIP_BUFFER;


#define TRANSFER_RETRY_TIMES 0xFFFFFFFF
#ifdef UPGRADE_ONCE
BOOL receive_comtest_falg = TRUE;//benjamin add.when upgrade via host which is in stanby mode,only allow upgrade once.
										//it's special case,only for smt
#endif

extern int un7zip(UINT8 *, UINT8 *, UINT8 *);

static UINT32 cmd_address(unsigned int argc, unsigned char *argv[]);
static UINT32 cmd_burn(unsigned int argc, unsigned char *argv[]);
static UINT32 cmd_comtest(unsigned int argc, unsigned char *argv[]);
static UINT32 cmd_move(unsigned int argc, unsigned char *argv[]);
static UINT32 cmd_reboot(unsigned int argc, unsigned char *argv[]);
static UINT32 cmd_transfer(unsigned int argc, unsigned char *argv[]);
static UINT32 cmd_version(unsigned int argc, unsigned char *argv[]);
static UINT32 cmd_chunk(unsigned int argc, unsigned char *argv[]);
static UINT32 cmd_dump(unsigned int argc, unsigned char *argv[]);

extern UINT32 g_protocol_version;
#if (SYS_SDRAM_SIZE == 2)
static UINT32 total_transfer_size;
static UINT32 total_burn_size;
static UINT32 transfer_size;
static UINT32 burn_size;
#endif

#define CI_PLUS_SUPPORT
#define CIPLUSKEY_CHUNK_ID 0x09F60101


/* Command <-> function map list */
struct ash_cmd lib_ash_command[LIB_ASH_COMMAND_MAX_NUM] =
{
    {"address", cmd_address},
    {"burn", cmd_burn},
    {"comtest", cmd_comtest},
    {"move", cmd_move},
    {"reboot", cmd_reboot},
    {"transfer", cmd_transfer},
    {"version", cmd_version},
    {"chunk", cmd_chunk},
    {"dump", cmd_dump},
};

UINT32 lib_ash_command_num=9;

BOOL cmd_reg_callback(struct ash_cmd cmd)
{
	if(lib_ash_command_num >= LIB_ASH_COMMAND_MAX_NUM)
		return FALSE;

	lib_ash_command[lib_ash_command_num] = cmd;

	lib_ash_command_num++;

	return TRUE;
}

UINT32 boot_uart_id = SCI_FOR_RS232;
extern UINT32 g_packet_index;
void boot_uart_set_id(UINT32 uart_id)
{
	boot_uart_id = uart_id;
	return;
}

UINT32 boot_uart_get_id()
{
	return boot_uart_id;
}

void upgrade_buf_addr_set(unsigned char *image_addr, unsigned char *zip_addr)	
{
	image_buf_addr = image_addr;
	zip_buf_addr = zip_addr;
}

#ifdef DEBUG
void dump(unsigned char *p, int l)
{
    int i = 0;

    while (l > 0)
    {
        if (i % 16 == 0)
        {
            SH_PRINTF("\n%08X:", (unsigned long)p);
        }
        SH_PRINTF(" %02X", *p++);
        i++;
        l--;
    }
    SH_PRINTF("\n");
}
#endif

#ifdef PANEL_DISPLAY
static const unsigned char down_str[] = { '_', '-' };
static const unsigned char burn_str[] = { '-', ' ' };
static const unsigned char dup_str[] = { '1', ' ' };
static unsigned int led_display_type = 0;

static void led_show(unsigned char *patn)
{
    unsigned int i;
    unsigned char led_dispay_str[4];

    for (i = 0; i < 4; i++)
        led_dispay_str[i] = patn[(i + led_display_type) % 2];
    led_display_type++;
    pan_display(panel_dev, led_dispay_str, 4);
}
#endif


static UINT32 cmd_comtest(unsigned int argc, unsigned char *argv[])
{
    unsigned int data_len;
	unsigned char ch = 0;
#ifdef UPGRADE_ONCE
	if(receive_comtest_falg == FALSE)
	{
		LIB_ASH_OC(':');
		LIB_ASH_OC('>');
		LIB_ASH_OC('_');
		return -1;
	}
#endif

#ifdef PANEL_DISPLAY
#if (SYS_SDRAM_SIZE != 2)
    pan_display(panel_dev,  "conn", 4);
#else
    pan_display(panel_dev,  "UP9", 4);
#endif
#endif

    if (argc != 2)
    {
#ifdef PANEL_DISPLAY
        pan_display(panel_dev,  "r101", 4);
#endif
        SH_PRINTF("Usage: comtest <data_num>\n");
        SH_PRINTF("       Then begin loopback <data_num> charactor to host.\n");
        return -1;
    }

    data_len = ATOI(argv[1]);
	ch = LIB_ASH_IC();
	char buf[5];
	data_len--;
#ifndef TRANSFER_FORMAT3_ONLY
	if('@' == ch)
	{
#endif
		LIB_ASH_OC(0xFF - ch);
		g_protocol_version = 0x30;
    	while (data_len--)
        	LIB_ASH_OC(0xFF - LIB_ASH_IC());

#ifndef TRANSFER_FORMAT3_ONLY
	}
	else
	{
		LIB_ASH_OC(ch);	
		g_protocol_version = 0x20;
		ch = LIB_ASH_IC();
		LIB_ASH_OC(ch);
		sprintf(buf, "T0%d", ch);
#ifdef PANEL_DISPLAY
        pan_display(panel_dev,  buf, 4);
#endif	
		data_len--;
    	while (data_len--)
        	LIB_ASH_OC(LIB_ASH_IC());
	}
#endif

    return 0;
}

static UINT32 cmd_version(unsigned int argc, unsigned char *argv[])
{
  
	if( argc >= 2)//version have no parameter
	{
		#ifdef PANEL_DISPLAY
	       	pan_display(panel_dev,  "r101", 4);
		#endif
	       SH_PRINTF("Usage: version\r");
		SendStatusPacket(COMMAND_STATUS_ERROR,  ERROR_PARAMETER);
		return ERROR_PARAMETER;
	}


    	SH_PRINTF("version \n");

	UINT32 nVersion = 0x0031;
	unsigned char szBuffer[4];
	store_long(szBuffer, nVersion);
	return SendDataPacket(0, szBuffer, 4);
}
static UINT32 cmd_chunk(unsigned int argc, unsigned char *argv[])
{
	unsigned int i, j;
	unsigned long of;
	unsigned int n;
	unsigned int data_len;
	unsigned int chunk_len;
	unsigned long chid;
	unsigned long len;
	unsigned char buf[CHUNK_HEADER_SIZE * 15];


	if( argc >= 2 )//"-r"
	{
		#ifdef PANEL_DISPLAY
			pan_display(panel_dev,  "r101", 4);
		#endif
	       SH_PRINTF("Usage: chunk\r");
		SendStatusPacket(COMMAND_STATUS_ERROR,  ERROR_PARAMETER);
		return ERROR_PARAMETER;
	}
    
    
	 /* collect information */
	n = sto_chunk_count(0, 0);
	of = sto_chunk_goto(&chid, 0, n);
	len = sto_fetch_long(of + CHUNK_LENGTH);

	/* calculate chunk len */
	chunk_len = n * CHUNK_HEADER_SIZE + 2;

	/* save Flash ID */
	buf[0] = flash_id_val;	
	buf[1] = g_crc_bad;
	
	/* get Chunks */
	unsigned char *pos = buf + 2;
    	for (i = 1; i <= n; i++)
    	{
        	of = sto_chunk_goto(&chid, 0, i);
        	sto_get_data(flash_dev, pos, of, CHUNK_HEADER_SIZE);
       	pos += CHUNK_HEADER_SIZE;
	}


    /* flag for state invalid */
    start_addr = 0xFFFFFFFF;

    return SendDataPacket(0, buf, chunk_len);
}


static UINT32 cmd_address(unsigned int argc, unsigned char *argv[])
{
	unsigned int sector_no;
	unsigned long temp_start;
	unsigned int len;

	if (argc != 2)
	{
#ifdef PANEL_DISPLAY
		pan_display(panel_dev,  "r101", 4);
#endif
		SH_PRINTF("Usage: address <start_address>\n");
		SendStatusPacket(COMMAND_STATUS_ERROR,  ERROR_PARAMETER);
		return ERROR_PARAMETER;
	}

    start_addr = ATOI(argv[1]);
    image_free = (unsigned char *)image_buf_addr;
#if (SYS_SDRAM_SIZE == 2)
    transfer_size = 0;
    burn_size = 0;
    total_transfer_size = 0;
    total_burn_size = 0;
    if (4 == argc)
    {
        total_transfer_size = ATOI(argv[2]);
        total_burn_size = ATOI(argv[3]);
    }
#endif

    sector_no = flash_sector_align(start_addr);
    if (sector_no >= flash_sectors_val)
    {
#ifdef PANEL_DISPLAY
        pan_display(panel_dev,  "r302", 4);
#endif
        SH_PRINTF("Error: Address greater than flash size. (Flash ID: %02X)\n", flash_id_val);
        SendStatusPacket(COMMAND_STATUS_ERROR,  ERROR_ADDRESS_OVERFLOW);
        return ERROR_ADDRESS_OVERFLOW;
    }

    temp_start = flash_sector_start(sector_no);
    if (temp_start < start_addr)
    {
        len = start_addr - temp_start;
        sto_get_data(flash_dev, image_free, temp_start , len);
        image_free += len;
        start_addr = temp_start;
    }

    SendStatusPacket(COMMAND_STATUS_OK,  0);
    return SUCCESS;
}


static UINT32 cmd_move(unsigned int argc, unsigned char *argv[])
{
	unsigned long of;
	unsigned long chid;
	unsigned long len;
	unsigned long offset;

	if (argc != 3)
	{
#ifdef PANEL_DISPLAY
	pan_display(panel_dev,  "r401", 4);
#endif
		SH_PRINTF("Usage: move <id> <offset>\n");
		SendStatusPacket(COMMAND_STATUS_ERROR,  ERROR_PARAMETER);
		return ERROR_PARAMETER;
	}
	if (start_addr == 0xFFFFFFFF)
	{
#ifdef PANEL_DISPLAY
		pan_display(panel_dev,  "r402", 4);
#endif
		SH_PRINTF("Error: Start address hasn't been initialized.\n");
		SendStatusPacket(COMMAND_STATUS_ERROR,  ERROR_ADDRESS_NOINIT);
		return ERROR_ADDRESS_NOINIT;
	}

	chid = ATOI(argv[1]);
	offset = ATOI(argv[2]);

	of = sto_chunk_goto(&chid, 0xFFFFFFFF, 1);
	if (of & 0x80000000)
	{
#ifdef PANEL_DISPLAY
	pan_display(panel_dev,  "r403", 4);
#endif
		SH_PRINTF("Error: Can not found specified Chunk ID. (ID: %08X)\n", chid);
		SendStatusPacket(COMMAND_STATUS_ERROR,  ERROR_CHUNK_NOTFAND);
		return ERROR_CHUNK_NOTFAND;
	}

	if (offset)
		len = offset;
	else
		len = sto_fetch_long(of + CHUNK_LENGTH) + 16;
	sto_get_data(flash_dev, image_free, of, len);
	store_long(image_free + CHUNK_OFFSET, offset);
	image_free += len;

	SendStatusPacket(COMMAND_STATUS_OK,  0);
	return SUCCESS;
}

static UINT32 cmd_dump(unsigned int argc, unsigned char *argv[])
{
	UINT32 nReturn = SUCCESS;
    	unsigned char *p = NULL;
   	unsigned int i,j,k,cxx_cmd = 0;
	unsigned int page_num = 0;
	unsigned char ack_buff[9],head, answer_pnum;
	int dump_start, dump_len,len = 0;
	
   	if (argc != 3)
    	{
#ifdef PANEL_DISPLAY
        pan_display(panel_dev,  "err1", 4);
#endif
		SH_PRINTF("Usage: dump <start> <offset>\n");
		SendStatusPacket(COMMAND_STATUS_ERROR,  ERROR_PARAMETER);
		return ERROR_PARAMETER;
    	}
	
    	dump_start = ATOI(argv[1]);
    	dump_len = ATOI(argv[2]);

	if( dump_start + dump_len > flash_dev->totol_size )
   	{
#ifdef PANEL_DISPLAY
        pan_display(panel_dev,  "err2", 4);
#endif
		SendStatusPacket(COMMAND_STATUS_ERROR,  ERROR_PARAMETER);
		return ERROR_PARAMETER;
   	}
#ifdef PANEL_DISPLAY
    led_display_type = 0;
#endif
   	SendStatusPacket(COMMAND_STATUS_OK,  0);
	g_last_status = COMMAND_STATUS_RUNNING;
	
  	if( dump_len == 0 )
   		dump_len = flash_dev->totol_size - dump_start;

	//transfer dump len
	PACKET packet;

	UINT32 err_count = 0;
	UINT32 nPacketNum = 0;
	unsigned char buffer[1024];
	UINT32 l  = PACKAGE_SIZE;
	UINT32 nNum = 0;
	 p = (char *)dump_start;
	while(len < dump_len /*&& err_count < TRANSFER_RETRY_TIMES*/)
	{
		MEMSET(&packet, 0, sizeof(PACKET));
		nReturn = packet_receive(&packet,  5 * 1000);
		if(SUCCESS != nReturn)
		{
			err_count ++;
			continue;
		}

		if(packet.packet_index == g_packet_index && g_packet_index != 0)
		{
			if(packet.packet_type == PACKET_COMMAND)
				SendStatusPacket(g_last_status, 0);
			else if(packet.packet_type == PACKET_STATUS)
			{
				if(COMMAND_STATUS_CANCEL == packet.data_buffer[0])
				{
					return ERROR_USERABORT;
				}
			}
			else if(packet.packet_type == PACKET_REQUEST)
			{
				// if nPacketNum = 0 send dump len else
				//send data
				err_count = 0;
				nNum = fetch_long(packet.data_buffer);
				
				if(0 == nNum)
				{
					store_long(buffer, dump_len);
					SendDataPacket(0, buffer, 4);
					SH_PRINTF("dump length!\n");
				}
				else
				{
					l = dump_len - (nNum - 1) * PACKAGE_SIZE;
				       if (l > PACKAGE_SIZE)
				       	l = PACKAGE_SIZE;
					   
					sto_get_data(flash_dev,buffer,(UINT32)(p + (nNum - 1) * PACKAGE_SIZE),l);
					if(nNum == nPacketNum + 1)
					{
						nPacketNum++;
						len = len + l;
					}
					SH_PRINTF("dump a packet:%u!\n", nNum);
					SendDataPacket(nNum, buffer, l);
				}
#ifdef PANEL_DISPLAY
				led_show((unsigned char *)dup_str);
#endif
				continue;

			}
		}
		else if(PACKET_COMMAND == packet.packet_type)
		{
			return SUCCESS;
		}	
		err_count++;
	}
	
   	return 0;
}


static UINT32 cmd_transfer(unsigned int argc, unsigned char *argv[])
{
	unsigned int i;
	unsigned long l;
	unsigned char *p;
	unsigned char *pos = NULL;
	unsigned int data_len;
	unsigned int err_count;
	unsigned int last_error;
	unsigned long crc;
	unsigned long len;
	unsigned long offset;
	unsigned int zip = 0;
	unsigned int real_len = 0;
	UINT8 *buffer =  NULL;
	if (argc == 2)
	{
		len = ATOI(argv[1]);
		p = image_free;
	}
	else if(argc == 4 && strcmp(argv[1], "-z") == 0)
	{
		real_len = ATOI(argv[2]);
		len = ATOI(argv[3]);
		zip = 1;

		p = (unsigned char *)zip_buf_addr;
		pos = p;
		
		buffer = MALLOC(BUFFER_SIZE);
  		if (buffer == NULL)
  		{
  			SH_PRINTF("Error: No Buffer to compress tranfer data.\n");
			SendStatusPacket(COMMAND_STATUS_ERROR, ERROR_NOBUFFER);
  			return ERROR_NOBUFFER;
  		}
  			
	}
	else
	{
#ifdef PANEL_DISPLAY
        	pan_display(panel_dev,  "r501", 4);
#endif
        	SH_PRINTF("Usage: transfer <data_length>\n");
		SendStatusPacket(COMMAND_STATUS_ERROR, ERROR_PARAMETER);
  		return ERROR_PARAMETER;
	}

	if (start_addr == 0xFFFFFFFF)
	{
#ifdef PANEL_DISPLAY
        pan_display(panel_dev,  "r502", 4);
#endif
		SH_PRINTF("Error: Start address hasn't been initialized.\n");
		if(buffer != NULL)
			FREE(buffer);
		
		SendStatusPacket(COMMAND_STATUS_ERROR, ERROR_ADDRESS_NOINIT);
		return ERROR_ADDRESS_NOINIT;
	}
#ifdef PANEL_DISPLAY
    pan_display(panel_dev, "trn ", 4);
    led_display_type = 0;
#endif

	//send ok 
	SendStatusPacket(COMMAND_STATUS_OK, 0);
	g_last_status = COMMAND_STATUS_RUNNING;

	
	//transfer data
	data_len = 0;
    	last_error = 0;
	err_count = 0;
	UINT32 nReturn = SUCCESS;
	UINT32 nPacketNum = 0;
	PACKET packet;
	while(data_len < len /*&& err_count < TRANSFER_RETRY_TIMES*/)
	{
		MEMSET(&packet, 0, sizeof(PACKET));
		nReturn = packet_receive(&packet, 5 * 1000);
		if(SUCCESS != nReturn)
		{
#ifdef PANEL_DISPLAY
        pan_display(panel_dev,  "r503", 4);
#endif		
			SH_PRINTF("Error: receive packet.\n");	
			err_count ++;
			continue;
		}
#ifdef PANEL_DISPLAY
		led_show((unsigned char *)down_str);
#endif
		if(packet.packet_index == g_packet_index && g_packet_index != 0)
		{
			if(packet.packet_type == PACKET_COMMAND)
				SendStatusPacket(g_last_status, 0);
			else if(packet.packet_type == PACKET_REQUEST)
				SendStatusPacket(COMMAND_STATUS_OK, nPacketNum);
			else if(packet.packet_type == PACKET_STATUS)
			{
				if(COMMAND_STATUS_CANCEL == packet.data_buffer[0])
				{
#ifdef PANEL_DISPLAY
        pan_display(panel_dev,  "r999", 4);
#endif				
					return ERROR_USERABORT;
				}
			}
			else//packet is data packet receive data
			{
				UINT32 n = fetch_long(packet.data_buffer);
				if(n == nPacketNum + 1)
				{
					nPacketNum++;
					MEMCPY(p, packet.data_buffer + 4, packet.packet_length - 4);
					p += packet.packet_length - 4;
		                	data_len += packet.packet_length - 4;
	 		              err_count = 0;
				}
				SendStatusPacket(COMMAND_STATUS_OK, nPacketNum);
			}
				
		}	
		else if(PACKET_COMMAND == packet.packet_type)
		{
			return SUCCESS;
		}
				
		err_count++;
	}

	 if (data_len == len)
    /* transfer succeed */
    	{
	    	SH_PRINTF("SUCCESS: All data transfer ok .\n");
	    	nReturn = SUCCESS;
		if(zip == 0)
			image_free += len;
		else  //unzip
		{  	
			
			if(un7zip((UINT8 *)pos, (UINT8 *)image_free, buffer) == 0)
				image_free += real_len;
			else
			{
				SH_PRINTF("Error: Compress tranfer data error.\n");
				nReturn = ERROR_COMPRESSTRANSFERDATA;
			}
		}
    	}
	 else
	 {
		SH_PRINTF("Error: not transfer all data.\n");
	 	nReturn = ERROR_TRANSFERALLDATA;
	 }
	 
	if(buffer != NULL)
		FREE(buffer);
	
	return nReturn;
}


static UINT32 cmd_burn(unsigned int argc, unsigned char *argv[])
{
	unsigned char *p;
	unsigned int sector_no;
	unsigned long temp_start;
	unsigned long temp_end;
	unsigned int len;
	unsigned int ret;
	unsigned char strbuf[16];

	if (start_addr == 0xFFFFFFFF || image_free == \
			(unsigned char *)image_buf_addr)
	{
#ifdef PANEL_DISPLAY
        	pan_display(panel_dev,  "r601", 4);
#endif
		SH_PRINTF("Error: Nothing to burn.\n");
		SendStatusPacket(COMMAND_STATUS_ERROR,  ERROR_BURN_NOTHING);
		return ERROR_BURN_NOTHING;
	}
#ifdef PANEL_DISPLAY
    led_display_type = 0;
#endif
	SendStatusPacket(COMMAND_STATUS_OK,  0);
	g_last_status = COMMAND_STATUS_RUNNING;
#ifdef HDCP_IN_FLASH
	// Check burn data include Bootloader sector or not.
	if( start_addr  < 0x10000 || start_addr  < 0x20000)
	{
		UINT8 	*hdcp_internal_keys;
		CHUNK_HEADER hdcp_chuck_hdr;
        UINT32 hdcp_chunk_offset_addr;
		UINT32 hdcp_chuck_id = HDCPKEY_CHUNK_ID;   
		if(sto_get_chunk_header(hdcp_chuck_id,&hdcp_chuck_hdr)!=0)
		{
			// read back the hdcp key from flash
			hdcp_internal_keys = (UINT8 *)MALLOC(sizeof(UINT8)*(hdcp_chuck_hdr.offset));
            hdcp_chunk_offset_addr = sto_chunk_goto(&hdcp_chuck_id,0xFFFFFFFF,1);
			sto_get_data( (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0), hdcp_internal_keys,
											hdcp_chunk_offset_addr, hdcp_chuck_hdr.offset);		
			// restore back the hdcp key to all code image					
			MEMCPY(image_buf_addr +hdcp_chunk_offset_addr,hdcp_internal_keys,hdcp_chuck_hdr.offset);		
			FREE(hdcp_internal_keys);
		}       
        
	}
#endif

#ifdef CI_PLUS_SUPPORT
	UINT8 	*ciplus_internal_keys;
	CHUNK_HEADER ciplus_chuck_hdr;
	UINT32 ciplus_chunk_offset_addr;
	UINT32 ciplus_chuck_id = CIPLUSKEY_CHUNK_ID;//  #define CIPLUSKEY_CHUNK_ID 0x09F60101
	if(sto_get_chunk_header(ciplus_chuck_id,&ciplus_chuck_hdr)!=0)
	{
		// read back the hdcp key from flash
		ciplus_internal_keys = (UINT8 *)MALLOC(sizeof(UINT8)*(ciplus_chuck_hdr.offset));
		ciplus_chunk_offset_addr = sto_chunk_goto(&ciplus_chuck_id,0xFFFFFFFF,1);
		sto_get_data( (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0), ciplus_internal_keys,ciplus_chunk_offset_addr, ciplus_chuck_hdr.offset);		
		MEMCPY(image_buf_addr +ciplus_chunk_offset_addr,ciplus_internal_keys,ciplus_chuck_hdr.offset);		
		FREE(ciplus_internal_keys);
	}
   
#endif


#ifdef DIVX_CERT_ENABLE
	// Check burn data include Bootloader sector or not.
	{
		UINT8 	*divx_internal_keys;
		CHUNK_HEADER divx_chuck_hdr;
		UINT32 divx_chunk_offset_addr;
		UINT32 divx_chuck_id = DIVX_CHUCK_ID;        
		if(sto_get_chunk_header(divx_chuck_id,&divx_chuck_hdr)!=0)
		{
			// read back the hdcp key from flash
			divx_internal_keys = (UINT8 *)MALLOC(sizeof(UINT8)*(divx_chuck_hdr.offset));
            		divx_chunk_offset_addr = sto_chunk_goto(&divx_chuck_id,0xFFFFFFFF,1);
			sto_get_data( (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0), divx_internal_keys,
											divx_chunk_offset_addr, divx_chuck_hdr.offset);		
			// restore back the hdcp key to all code image					
			MEMCPY(image_buf_addr +divx_chunk_offset_addr,divx_internal_keys,divx_chuck_hdr.offset);		
			FREE(divx_internal_keys);
		}	
	}
#endif


	    /* Begin burn: Erase sector */
	temp_end = image_free - (unsigned char *)image_buf_addr + \
		start_addr;
	if (temp_end < (unsigned long)(flash_dev->totol_size))
	{
		sector_no = flash_sector_align(temp_end);
		temp_start = flash_sector_start(sector_no);
		if (temp_start < temp_end)
		{
			len = temp_start + flash_sector_size(sector_no) - \
				temp_end;
			sto_get_data(flash_dev, image_free, temp_end, len);
			temp_end += len;
		}
	}
	else
		temp_end = flash_dev->totol_size;

	p = (unsigned char *)image_buf_addr;
	sector_no = flash_sector_align(start_addr);
	temp_start = start_addr;
	while (temp_start < temp_end)
	{
		len = flash_sector_size(sector_no++);
		switch (flash_verify(temp_start, p, len))
		{
		case 2:
			flash_erase_sector(temp_start);
		case 1:
			ret = flash_copy(temp_start, p, len);
			if (ret == 0)
				ret = flash_verify(temp_start, p, len);
			if (ret == 0)
				sprintf(strbuf, "B%04dK\n", temp_start >> 10);
			else
				sprintf(strbuf, "E%07d\n", ret);
           		//Send Data Packet
			SendDataPacket(0, strbuf, STRLEN(strbuf));
			SH_PRINTF("burn:");
			SH_PRINTF(strbuf);
			//receive packet
			PACKET packet;
			UINT32 nReturn = packet_receive(&packet, 1);
			if(SUCCESS == nReturn)
			{
				if(PACKET_COMMAND == packet.packet_type &&
					g_packet_index == packet.packet_index)
					SendStatusPacket(g_last_status, 0);
			}
			break;
		case 0:
			break;	
        	}
#ifdef PANEL_DISPLAY		
		led_show((unsigned char *)burn_str);
#endif	
		p += len;
		temp_start += len;
	}

#ifdef DEBUG
	unsigned int i;
    	for (i = 0; i < flash_sectors_val; i++)
       	dump(FLASH_OFF2PTR(flash_sector_start(i)), 16);
#endif

	SendDataPacket(0, "FINISH", 6);

#ifdef PANEL_DISPLAY
#if (SYS_SDRAM_SIZE != 2)
    pan_display(panel_dev, "end-", 4);
#endif
#endif
#ifdef UPGRADE_ONCE
	receive_comtest_falg = FALSE;
#endif

	return SUCCESS;
}


static UINT32 cmd_reboot(unsigned int argc, unsigned char *argv[])
{
	SendStatusPacket(COMMAND_STATUS_OK,  0);
    osal_delay((WORD)100000);//100ms
#ifdef PANEL_DISPLAY
	pan_display(panel_dev, "off", 4);
#endif
	sys_ic_exit_standby();
#ifdef PANEL_DISPLAY
	pan_close((struct pan_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_PAN));
#endif
#ifdef WATCH_DOG_SUPPORT
    dog_stop(0);
#endif 
	sys_watchdog_reboot();
	return SUCCESS;
}

#endif

#ifndef TRANSFER_FORMAT3_ONLY


static UINT32 cmd_address_old(unsigned int argc, unsigned char *argv[]);
static UINT32 cmd_burn_old(unsigned int argc, unsigned char *argv[]);
static UINT32 cmd_move_old(unsigned int argc, unsigned char *argv[]);
static UINT32 cmd_reboot_old(unsigned int argc, unsigned char *argv[]);
static UINT32 cmd_transfer_old(unsigned int argc, unsigned char *argv[]);
static UINT32 cmd_transferraw_old(unsigned int argc, unsigned char *argv[]);
static UINT32 cmd_version_old(unsigned int argc, unsigned char *argv[]);
static UINT32 cmd_dump_old(unsigned int argc, unsigned char *argv[]);
static UINT32 cmd_set_old(unsigned int argc, unsigned char *argv[]);

/* Command <-> function map list */
struct ash_cmd lib_ash_command_old[] =
{
    {"address", cmd_address_old},
    {"burn", cmd_burn_old},
    {"comtest", cmd_comtest},
    {"move", cmd_move_old},
    {"reboot", cmd_reboot_old},
    {"transfer", cmd_transfer_old},
    {"transferraw", cmd_transferraw_old},
    {"version", cmd_version_old},
    {"dump", cmd_dump_old},
    {"set", cmd_set_old}
};

static void dump_str(unsigned char *p)
{
    while (*p)
        LIB_ASH_OC(*p++);
}

static UINT32 cmd_version_old(unsigned int argc, unsigned char *argv[])
{
    unsigned int i, j;
    unsigned long of;
    unsigned int n;
    unsigned int data_len;
    unsigned long crc;
    unsigned long chid;
    unsigned long len;
    unsigned long crc1;
    unsigned char buf[CHUNK_HEADER_SIZE];
    unsigned int nVersion = 0xb722;

    //LIB_ASH_OC(argc & 0xFF);
    //LIB_ASH_OC(argv[1][0]);
    //LIB_ASH_OC(argv[1][1]);
    if( argc >= 2 && argv[1][0] == 0x2d && argv[1][1] == 0x72)//"-r"
    {
#ifdef TRANSFER_FORMAT2_SUPPORT
        LIB_ASH_OC((nVersion >> 8) & 0xFF);
        LIB_ASH_OC( nVersion & 0xFF);
#endif
    }
    
    
    /* collect information */
    n = sto_chunk_count(0, 0);
    of = sto_chunk_goto(&chid, 0, n);
    len = sto_fetch_long(of + CHUNK_LENGTH);

    /* send Length */
    data_len = n * CHUNK_HEADER_SIZE + 2;
    LIB_ASH_OC((data_len >> 8) & 0xFF);
    LIB_ASH_OC(data_len & 0xFF);

    crc = 0xFFFFFFFF;
    /* send Flash ID */
    LIB_ASH_OC(flash_id_val);
    crc = MG_Table_Driven_CRC(crc, &flash_id_val, 1);
    /* send Chunk Status */
    LIB_ASH_OC(g_crc_bad);
    crc = MG_Table_Driven_CRC(crc, &g_crc_bad, 1);
    /* send Chunks */
    for (i = 1; i <= n; i++)
    {
        of = sto_chunk_goto(&chid, 0, i);
        sto_get_data(flash_dev, buf, of, CHUNK_HEADER_SIZE);
        data_len = sto_fetch_long(of + CHUNK_LENGTH);
        crc1 = MG_Table_Driven_CRC(0xFFFFFFFF, (unsigned char *) \
              SYS_FLASH_BASE_ADDR + of + 16, data_len);
        store_long(buf + CHUNK_CRC, crc1);
        for (j = 0; j < CHUNK_HEADER_SIZE; j++)
            LIB_ASH_OC(buf[j]);
        crc = MG_Table_Driven_CRC(crc, buf, CHUNK_HEADER_SIZE);
    }
    /* send CRC */
    LIB_ASH_OC((crc >> 24) & 0xFF);
    LIB_ASH_OC((crc >> 16) & 0xFF);
    LIB_ASH_OC((crc >> 8) & 0xFF);
    LIB_ASH_OC(crc & 0xFF);

    /* flag for state invalid */
    start_addr = 0xFFFFFFFF;

    return 0;
}


#ifdef TRANSFER_FORMAT1_EXTEN
static UINT32 cmd_dump_old(unsigned int argc, unsigned char *argv[])
{
    unsigned long crc;
    unsigned char *p = (unsigned char *)FLASH_BASE;
    unsigned int i,j,k,cxx_cmd = 0;
	unsigned int page_num = 0;
	unsigned int len = 0;
	unsigned char blank_pck[8];
	unsigned char ack_buff[9],head;

	blank_pck[0] = 'B';
	blank_pck[1] = 'L';
	blank_pck[2] = 'A';
	blank_pck[3] = 'N';
	blank_pck[4] = 'K';
	blank_pck[5] = 'P';
	blank_pck[6] = 'C';
	blank_pck[7] = 'K';

#ifdef PANEL_DISPLAY
    pan_display(panel_dev, "dup ", 4);
    led_display_type = 0;
#endif

    /* send flash length*/
    LIB_ASH_OC((flash_dev->totol_size >> 24) & 0xFF);
    LIB_ASH_OC((flash_dev->totol_size >> 16) & 0xFF);
    LIB_ASH_OC((flash_dev->totol_size >> 8) & 0xFF);
    LIB_ASH_OC(flash_dev->totol_size & 0xFF);

    i = 0;
    while (i < flash_dev->totol_size)
    {
		if (sci_read_tm(SCI_FOR_RS232, &head, 500 * 1000) != SUCCESS)
        {
            continue;
        }
		if(head != 'H')
		{
            if (head == 'c')
            {
                cxx_cmd++;
                if (cxx_cmd > 4)
                    return -1;
            }
            else if (head != '\r')
            {
                cxx_cmd = 0;
            }
            continue;
		}

		len = PACKAGE_SIZE;
		if(flash_dev->totol_size - i < PACKAGE_SIZE)
			len = flash_dev->totol_size - i;

	    crc = 0xFFFFFFFF;
		/* send page number */
	    LIB_ASH_OC((page_num >> 24) & 0xFF);
	    LIB_ASH_OC((page_num >> 16) & 0xFF);
	    LIB_ASH_OC((page_num >> 8) & 0xFF);
	    LIB_ASH_OC(page_num & 0xFF);
		/* send page length */
	    LIB_ASH_OC((len >> 24) & 0xFF);
	    LIB_ASH_OC((len >> 16) & 0xFF);
	    LIB_ASH_OC((len >> 8) & 0xFF);
	    LIB_ASH_OC(len & 0xFF);
		/* check if the packet is full of 0xFF,then send the blank packet to host */
		for(j=0;j<len;j++)
		{
			if(*((UINT8 *)(p+page_num*PACKAGE_SIZE+j)) != 0xFF)
				break;
		}
		if(j != len)
		{
			for(k=0;k<len;k++)
			{
				LIB_ASH_OC(*(p+page_num*1024+k));
				crc = MG_Table_Driven_CRC(crc, p+page_num*1024+k,1);
			}
		}
		else/* send blank packet */
		{
			for(k=0;k<8;k++)
			{
				LIB_ASH_OC(*(blank_pck+k));
			}
			crc = MG_Table_Driven_CRC(0xFFFFFFFF, blank_pck,8);

		}
		/* send CRC */
	    LIB_ASH_OC((crc >> 24) & 0xFF);
	    LIB_ASH_OC((crc >> 16) & 0xFF);
	    LIB_ASH_OC((crc >> 8) & 0xFF);
	    LIB_ASH_OC(crc & 0xFF);
		/* wait acknowledge from host */
		for(k=0;k<9;k++)
		{
			if(sci_read_tm(SCI_FOR_RS232,ack_buff+k,50*1000) != SUCCESS)
				break;
		}
		if(k == 9)
		{
			if(MG_Table_Driven_CRC(0xFFFFFFFF, ack_buff,5) != fetch_long(ack_buff+5))
				continue;
			if(ack_buff[0] == 'E')
				continue;
			if(ack_buff[0] == 'O')
			{
				if(fetch_long(ack_buff+1) == page_num)
					page_num++;
				else
					continue;
			}
		}
		else
			continue;

#ifdef PANEL_DISPLAY
        if (i % PACKAGE_SIZE == 0)
        {
            led_show((unsigned char *)dup_str);
        }
#endif
        i += PACKAGE_SIZE;
    }

   return 0;
}
#else
#ifdef TRANSFER_FORMAT2_SUPPORT
static UINT32 cmd_dump_old(unsigned int argc, unsigned char *argv[])
{
    unsigned long crc;
    unsigned char *p = NULL;
    unsigned int i,j,k,cxx_cmd = 0;
	unsigned int page_num = 0;
	unsigned int len = 0;
	unsigned char blank_pck[8];
	unsigned char ack_buff[9],head, answer_pnum;
	int dump_start, dump_len;

	blank_pck[0] = 'B';
	blank_pck[1] = 'L';
	blank_pck[2] = 'A';
	blank_pck[3] = 'N';
	blank_pck[4] = 'K';
	blank_pck[5] = 'P';
	blank_pck[6] = 'C';
	blank_pck[7] = 'K';

#ifdef PANEL_DISPLAY
	pan_display(panel_dev, "dup ", 4);
	led_display_type = 0;
#endif

   if (argc == 3)
    {
//#ifdef PANEL_DISPLAY
//        pan_display(panel_dev,  "err1", 4);
//#endif
//        SH_PRINTF("Usage: dump <start> <offset>\n");
        dump_start = ATOI(argv[1]);
		dump_len = ATOI(argv[2]);
    }
   else
   {
	   dump_start = 0;
	   dump_len = 0;
   }    

   if( dump_start + dump_len > flash_dev->totol_size )
   {
#ifdef PANEL_DISPLAY
        pan_display(panel_dev,  "err2", 4);
#endif
        return -1;
   }

   if( dump_len == 0 )
   {
   	 	/* send flash length*/
	   dump_len = flash_dev->totol_size - dump_start;
	    LIB_ASH_OC((dump_len >> 24) & 0xFF);
	    LIB_ASH_OC((dump_len >> 16) & 0xFF);
	    LIB_ASH_OC((dump_len >> 8) & 0xFF);
	    LIB_ASH_OC(dump_len & 0xFF);
   }

   p = ((unsigned char *)FLASH_BASE) + dump_start;

    i = 0;
    while (i < dump_len)
    {
		if (sci_read_tm(boot_uart_id, &head, 500 * 1000) != SUCCESS)
        {
            continue;
        }
		if(head != 'H')
		{
            if (head == 'c')
            {
                cxx_cmd++;
                if (cxx_cmd > 4)
                    return -1;
            }
            else if (head != '\r')
            {
                cxx_cmd = 0;
            }
            continue;
		}

		len = PACKAGE_SIZE;
		if(dump_len - i < PACKAGE_SIZE)
			len = dump_len - i;

		/* send page number */
	    LIB_ASH_OC((page_num >> 24) & 0xFF);
	    LIB_ASH_OC((page_num >> 16) & 0xFF);
	    LIB_ASH_OC((page_num >> 8) & 0xFF);
	    LIB_ASH_OC(page_num & 0xFF);

		/* send page length */
	    LIB_ASH_OC((len >> 24) & 0xFF);
	    LIB_ASH_OC((len >> 16) & 0xFF);
	    LIB_ASH_OC((len >> 8) & 0xFF);
	    LIB_ASH_OC(len & 0xFF);

		crc = MG_Table_Driven_CRC(0xFFFFFFFF, (unsigned char*)&page_num,4);
		LIB_ASH_OC((crc >> 24) & 0xFF);
	    	LIB_ASH_OC((crc >> 16) & 0xFF);
	    	LIB_ASH_OC((crc >> 8) & 0xFF);
	    	LIB_ASH_OC(crc & 0xFF);
		crc = MG_Table_Driven_CRC(0xFFFFFFFF, (unsigned char*)&len,4);
		LIB_ASH_OC((crc >> 24) & 0xFF);
	   	LIB_ASH_OC((crc >> 16) & 0xFF);
	    	LIB_ASH_OC((crc >> 8) & 0xFF);
	    	LIB_ASH_OC(crc & 0xFF);
		if (sci_read_tm(boot_uart_id, &answer_pnum, 500 * 1000) != SUCCESS)
        	{
            		continue;
        	}
		if(answer_pnum != 'G')
		{
			continue;
		}


		crc = 0xFFFFFFFF;
		/* check if the packet is full of 0xFF,then send the blank packet to host */
		for(j=0;j<len;j++)
		{
			if(*((UINT8 *)(p+page_num*PACKAGE_SIZE+j)) != 0xFF)
				break;
		}
		if(j != len)
		{
			for(k=0;k<len;k++)
			{
				LIB_ASH_OC(*(p+page_num*1024+k));
				crc = MG_Table_Driven_CRC(crc, p+page_num*1024+k,1);
			}
		}
		else/* send blank packet */
		{
			for(k=0;k<8;k++)
			{
				LIB_ASH_OC(*(blank_pck+k));
			}
			crc = MG_Table_Driven_CRC(0xFFFFFFFF, blank_pck,8);

		}
		/* send CRC */
	    LIB_ASH_OC((crc >> 24) & 0xFF);
	    LIB_ASH_OC((crc >> 16) & 0xFF);
	    LIB_ASH_OC((crc >> 8) & 0xFF);
	    LIB_ASH_OC(crc & 0xFF);
		/* wait acknowledge from host */
		for(k=0;k<9;k++)
		{
			if(sci_read_tm(boot_uart_id,ack_buff+k,50*1000) != SUCCESS)
				break;
		}
		if(k == 9)
		{
			if(MG_Table_Driven_CRC(0xFFFFFFFF, ack_buff,5) != fetch_long(ack_buff+5))
				continue;
			if(ack_buff[0] == 'E')
				continue;
			if(ack_buff[0] == 'O')
			{
				if(fetch_long(ack_buff+1) == page_num)
					page_num++;
				else
					continue;
			}
		}
		else
			continue;

#ifdef PANEL_DISPLAY
        if (i % PACKAGE_SIZE == 0)
        {
            led_show((unsigned char *)dup_str);
        }
#endif
        i += PACKAGE_SIZE;
    }

   return 0;
}
#else

static UINT32 cmd_dump_old(unsigned int argc, unsigned char *argv[])
{
    unsigned long crc;
    unsigned char *p = (unsigned char *)FLASH_BASE;
    unsigned int i;

#ifdef PANEL_DISPLAY
    pan_display(panel_dev, "dup ", 4);
    led_display_type = 0;
#endif

    /* send flash length*/
    LIB_ASH_OC((flash_dev->totol_size>> 24) & 0xFF);
    LIB_ASH_OC((flash_dev->totol_size >> 16) & 0xFF);
    LIB_ASH_OC((flash_dev->totol_size >> 8) & 0xFF);
    LIB_ASH_OC(flash_dev->totol_size & 0xFF);

    /* wait for host answer*/
    if (LIB_ASH_IC() != 'O')
    {
#ifdef PANEL_DISPLAY
        pan_display(panel_dev,  "r801", 4);
#endif
        return -1;
    }

    crc = 0xFFFFFFFF;
    i = 0;
    while (i < (unsigned int)(flash_dev->totol_size))
    {
        LIB_ASH_OC(*p);
        crc = MG_Table_Driven_CRC(crc, p++, 1);
#ifdef PANEL_DISPLAY
        if (i % PACKAGE_SIZE == 0)
        {
            led_show((unsigned char *)dup_str);
        }
#endif
        i++;
    }

    /* send CRC */
    LIB_ASH_OC((crc >> 24) & 0xFF);
    LIB_ASH_OC((crc >> 16) & 0xFF);
    LIB_ASH_OC((crc >> 8) & 0xFF);
    LIB_ASH_OC(crc & 0xFF);

    return 0;
}

#endif
#endif
static UINT32 cmd_address_old(unsigned int argc, unsigned char *argv[])
{
    unsigned int sector_no;
    unsigned long temp_start;
    unsigned int len;

#if (SYS_SDRAM_SIZE == 2)
    if (argc < 2)
#else
    if (argc != 2)
#endif
    {
#ifdef PANEL_DISPLAY
        pan_display(panel_dev,  "r301", 4);
#endif
        SH_PRINTF("Usage: address <start_address>\n");
        return -1;
    }

    start_addr = ATOI(argv[1]);
    image_free = (unsigned char *)image_buf_addr;
#if (SYS_SDRAM_SIZE == 2)
    transfer_size = 0;
    burn_size = 0;
    total_transfer_size = 0;
    total_burn_size = 0;
    if (4 == argc)
    {
        total_transfer_size = ATOI(argv[2]);
        total_burn_size = ATOI(argv[3]);
    }
#endif

    sector_no = flash_sector_align(start_addr);
    if (sector_no >= flash_sectors_val)
    {
#ifdef PANEL_DISPLAY
        pan_display(panel_dev,  "r302", 4);
#endif
        SH_PRINTF("Error: Address greater than flash size. (Flash ID: %02X)\n", flash_id_val);
        LIB_ASH_OC('E');
        return -1;
    }

    temp_start = flash_sector_start(sector_no);
    if (temp_start < start_addr)
    {
        len = start_addr - temp_start;
        sto_get_data(flash_dev, image_free, temp_start , len);
        image_free += len;
        start_addr = temp_start;
    }

    LIB_ASH_OC('O');
    return 0;
}


static UINT32 cmd_move_old(unsigned int argc, unsigned char *argv[])
{
    unsigned long of;
    unsigned long chid;
    unsigned long len;
    unsigned long offset;

    if (argc != 3)
    {
#ifdef PANEL_DISPLAY
        pan_display(panel_dev,  "r401", 4);
#endif
        SH_PRINTF("Usage: move <id> <offset>\n");
        return -1;
    }
    if (start_addr == 0xFFFFFFFF)
    {
#ifdef PANEL_DISPLAY
        pan_display(panel_dev,  "r402", 4);
#endif
        SH_PRINTF("Error: Start address hasn't been initialized.\n");
        LIB_ASH_OC('E');
        return -1;
    }

    chid = ATOI(argv[1]);
    offset = ATOI(argv[2]);

    of = sto_chunk_goto(&chid, 0xFFFFFFFF, 1);
    if (of & 0x80000000)
    {
#ifdef PANEL_DISPLAY
        pan_display(panel_dev,  "r403", 4);
#endif
        SH_PRINTF("Error: Can not found specified Chunk ID. (ID: %08X)\n", chid);
        LIB_ASH_OC('E');
        return -1;
    }

    if (offset)
        len = offset;
    else
        len = sto_fetch_long(of + CHUNK_LENGTH) + 16;
    sto_get_data(flash_dev, image_free, of, len);
    store_long(image_free + CHUNK_OFFSET, offset);
    image_free += len;

    LIB_ASH_OC('O');
    return 0;
}


#ifdef TRANSFER_FORMAT2_SUPPORT
static UINT32 cmd_transfer_old(unsigned int argc, unsigned char *argv[])
{
    unsigned int i;
    unsigned int data_len;
    unsigned long crc;
    unsigned long len;
    unsigned long l;
    unsigned long offset;
    unsigned long package_num;
    unsigned char *p;
    unsigned char info[9];
    unsigned char err_state;
    unsigned char cxx_cmd;
#if (SYS_SDRAM_SIZE == 2)
    unsigned char msg[10];
    unsigned long progress;
#endif

    if (argc != 2)
    {
#ifdef PANEL_DISPLAY
        pan_display(panel_dev,  "r501", 4);
#endif
        SH_PRINTF("Usage: transfer <data_length>\n");
        return -1;
    }
    if (start_addr == 0xFFFFFFFF)
    {
#ifdef PANEL_DISPLAY
        pan_display(panel_dev,  "r502", 4);
#endif
        SH_PRINTF("Error: Start address hasn't been initialized.\n");
        LIB_ASH_OC('E');
        return -1;
    }

#ifdef PANEL_DISPLAY
#if (SYS_SDRAM_SIZE != 2)
    pan_display(panel_dev, "trn ", 4);
#endif
    led_display_type = 0;
#endif

    len = ATOI(argv[1]);

    cxx_cmd = 0;

    data_len = 0;
    while (data_len < len)
    {
        /* get header char as sub-command */
        info[0] = 0;
        if (sci_read_tm(boot_uart_id, info, 500 * 1000) != SUCCESS)
        {
            continue;
        }
        if (info[0] == 'H')
        {
            /* transfer head information */
            for (i = 0; i < 8; i++)
            {
                if (sci_read_tm(boot_uart_id, info + 1 + i, 500 * 1000) != SUCCESS)
                {
                    break;
                }
            }
            if (i != 8)
            {
                continue;
            }
            if (MG_Table_Driven_CRC(0xFFFFFFFF, info + 1, 4) != \
                  fetch_long(info + 5))
            {
                continue;
            }
            package_num = fetch_long(info + 1);
            /* acknowledge for got head information */
            LIB_ASH_OC('C');
        }
        else
        {
            if (info[0] == 'c')
            {
                cxx_cmd++;
                if (cxx_cmd > 7)
                {
                    SH_PRINTF("Error: Might in comtest.\n");
                    LIB_ASH_OC('E');
                    return -1;
                }
            }
            else if (info[0] != '\r')
            {
                cxx_cmd = 0;
            }

            /* ignore */
            continue;
        }

        p = image_free + package_num * PACKAGE_SIZE;
        l = len - data_len;
        if (l > PACKAGE_SIZE)
            l = PACKAGE_SIZE;
        /* transfer one data block + CRC */
        for (i = 0; i < l + 4; i++)
        {
            if (sci_read_tm(boot_uart_id, p + i, 500 * 1000) != SUCCESS)
            {
                err_state = 1;
                break;
            }
        }
        if (i == l + 4)
        {
            err_state = MG_Table_Driven_CRC(0xFFFFFFFF, p, l) != \
                  fetch_long(p + l);
#ifdef PANEL_DISPLAY
#if (SYS_SDRAM_SIZE == 2)
            transfer_size += l;
            if (total_transfer_size > 0)
            {
                progress = transfer_size*100/total_transfer_size;
                if (progress < 100)
                    sprintf(msg,"d%.2d",progress);
                else
                    sprintf(msg,"100 ");

                pan_display(panel_dev,  msg, 4);
            }
            else
                led_show((unsigned char *)down_str);
#else
            led_show((unsigned char *)down_str);
#endif
#endif
        }

        if (err_state == 0)
        {
            data_len = p + l - image_free;
            /* Acknowledge for packet OK */
            info[0] = 'O';
            store_long(info + 1, package_num);
            store_long(info + 5, MG_Table_Driven_CRC(0xFFFFFFFF, \
                  info + 1, 4));
            for (i = 0; i < 9; i++)
            {
                LIB_ASH_OC(info[i]);
            }
        }
        else
        {
            /* Acknowledge for packet Error */
            LIB_ASH_OC('E');
        }
    }

    if (data_len == len)
    /* transfer succeed */
    {
        p += l;
        offset = fetch_long(image_free + CHUNK_OFFSET);
        if (offset == 0)
            offset = fetch_long(image_free + CHUNK_LENGTH) + 16;
        /* if offset > transfer length, fill gap with old data */
        if (offset > len)
        {
            l = offset - len;
            i = p - (unsigned char *)image_buf_addr + start_addr;
            sto_get_data(flash_dev, p, i, l);
        }
        image_free += offset;
    }

    return 0;
}

#else
#define     UNZIP_IN_BUF                0xa0100000//0xa0000200
#define     UNZIP_TEMP_BUF              0xa01a0000
#define     TRANSER_RAW_DATA            0
#define     TRANSER_ZIP_SECTION_DATA    1
#define     TRANSER_ZIP_ALL_DATA        2
// command format: 1. transfer raw_length (transfer raw data.)
//                 2. transfer C|A zip_length raw_length (transfer compress zip code data)
//               C - zip compress data; A - transfer all compress firmware data one time.
//               zip_length - zip compress data length
//               raw_length -  the raw data length
static UINT32 cmd_transfer_old(unsigned int argc, unsigned char *argv[])
{
    unsigned int i;
    unsigned long l;
    unsigned char *p;
    unsigned int data_len;
    unsigned int err_count;
    unsigned int last_error;
    unsigned long crc;
    unsigned long len;
    unsigned long raw_len=0;
    unsigned long offset;
    unsigned char cTransferType;
#if (SYS_SDRAM_SIZE == 2)
    unsigned char msg[10];
    unsigned long progress;
#endif

#if (SYS_SDRAM_SIZE == 2)
    if (argc < 2)
#else
    if (argc != 2)
#endif
    {
#ifdef PANEL_DISPLAY
        pan_display(panel_dev,  "r501", 4);
#endif
        SH_PRINTF("Usage: transfer <data_length>\n");
        return -1;
    }

    cTransferType = TRANSER_RAW_DATA;
    if (argc == 2)
    {
        len = ATOI(argv[1]);
    }
    else if (argc == 4)
    {
        if (argv[1][0] == 'C')
            cTransferType = TRANSER_ZIP_SECTION_DATA;
        else if (argv[1][0] == 'A')
            cTransferType = TRANSER_ZIP_ALL_DATA;
        else
            return -1;
        len = ATOI(argv[2]);
        raw_len = ATOI(argv[3]);
    }
    else
        return -1;

    if (start_addr == 0xFFFFFFFF)
    {
#ifdef PANEL_DISPLAY
        pan_display(panel_dev,  "r502", 4);
#endif
        SH_PRINTF("Error: Start address hasn't been initialized.\n");
        LIB_ASH_OC('E');
        return -1;
    }

#ifdef PANEL_DISPLAY
#if (SYS_SDRAM_SIZE != 2)
    pan_display(panel_dev, "trn ", 4);
#endif
    led_display_type = 0;
#endif

    data_len = 0;
    last_error = 0;
    err_count = 0;

    if (cTransferType != TRANSER_RAW_DATA)
        p = (unsigned char*)(UNZIP_IN_BUF);
    else
        p = image_free;
    while (data_len < len&& err_count < TRANSFER_RETRY_TIMES)
    {
        l = len - data_len;
        if (l > PACKAGE_SIZE)
            l = PACKAGE_SIZE;
        /* Acknowledge for next data block */
        LIB_ASH_OC(last_error ? 'E' : 'O');
        /* transfer one data block + CRC */
        for (i = 0; i < l + 4; i++)
        {
            if (sci_read_tm(boot_uart_id, p + i, 1500 * 1000) != SUCCESS)
            {
                err_count++;
                break;
            }
        }
        if (i == l + 4)
        {
            crc = fetch_long(p + l);
            last_error = (crc != MG_Table_Driven_CRC(0xFFFFFFFF, \
                  p, l));
            if (!last_error)
            {
                p += l;
                data_len += l;
                err_count = 0;
            #if (SYS_SDRAM_SIZE == 2)
                transfer_size += l;
            #endif
            }
    #ifdef PANEL_DISPLAY
        #if (SYS_SDRAM_SIZE == 2)
            if (total_transfer_size > 0)
            {
                if (cTransferType != TRANSER_RAW_DATA)
                    progress = transfer_size*100/len;
                else
                    progress = transfer_size*100/total_transfer_size;
                if (progress < 100)
                    sprintf(msg,"d%.2d",progress);
                else
                    sprintf(msg,"100 ");

                pan_display(panel_dev,  msg, 4);
            }
            else
                led_show((unsigned char *)down_str);
        #else
            led_show((unsigned char *)down_str);
        #endif
    #endif
        }
        else
            last_error = 1;
    }

    if (cTransferType != TRANSER_RAW_DATA)
    {
        int ret;
        ret = un7zip((unsigned char *)UNZIP_IN_BUF, image_free, (unsigned char *)UNZIP_TEMP_BUF);
        if (ret != 0)
        {
            LIB_ASH_OC('E');
            return -1;
        }
    }

    if (data_len == len)
    /* transfer succeed */
    {
        offset = fetch_long(image_free + CHUNK_OFFSET);
        if (offset == 0)
            offset = fetch_long(image_free + CHUNK_LENGTH) + 16;
        /* if offset > transfer length, fill gap with old data */
        if (cTransferType != TRANSER_RAW_DATA)
        {
            if ((TRANSER_ZIP_SECTION_DATA == cTransferType) && (offset > raw_len))
            {
                l = offset - raw_len;
                i = image_free+raw_len - (unsigned char *)image_buf_addr + start_addr;
                sto_get_data(flash_dev, p, i, l);
            }
        }
        else
        {
            if (offset > len)
            {
                l = offset - len;
                i = p - (unsigned char *)image_buf_addr + start_addr;
                sto_get_data(flash_dev, p, i, l);
            }
        }
        if (TRANSER_ZIP_ALL_DATA == cTransferType)
            image_free += raw_len;
        else
            image_free += offset;
    }

    LIB_ASH_OC('O');
    return 0;
}
#endif

static UINT32 cmd_set_old(unsigned int argc, unsigned char *argv[])
{
	unsigned long len;
	char c;

 	if (argc != 3)
    {
#ifdef PANEL_DISPLAY
        pan_display(panel_dev,  "r901", 4);
#endif
        SH_PRINTF("Usage: set <char> <length>\n");
        return -1;
    }

	 if (start_addr == 0xFFFFFFFF)
    {
#ifdef PANEL_DISPLAY
        pan_display(panel_dev,  "r902", 4);
#endif
        SH_PRINTF("Error: Start address hasn't been initialized.\n");
        LIB_ASH_OC('E');
        return -1;
    }

	c =  *(argv[1]);
	len = ATOI(argv[2]);

	MEMSET(image_free, c, len);
	
    image_free += len;

    LIB_ASH_OC('O');
	 	
}

#define RETURNCODE(code) { \
	if(buffer != NULL) FREE(buffer);	\
	return code;				\
}
static UINT32 cmd_transferraw_old(unsigned int argc, unsigned char *argv[])
{
    unsigned int i;
    unsigned long l;
    unsigned char *p;
	unsigned char *pos = NULL;
    unsigned int data_len;
    unsigned int err_count;
    unsigned int last_error;
    unsigned long crc;
    unsigned long len;
    unsigned long offset;
    unsigned int zip = 0;
	unsigned int real_len = 0;
	UINT8 *buffer =  NULL;

    if (argc == 2)
	{
		len = ATOI(argv[1]);
		p = image_free;
	}
	else if(argc == 4 && strcmp(argv[1], "-z") == 0)
	{
		real_len = ATOI(argv[2]);
		len = ATOI(argv[3]);
		zip = 1;

		p = (unsigned char *)zip_buf_addr;
		pos = p;
		
		buffer = MALLOC(BUFFER_SIZE);
  		if (buffer == NULL)
  		{
  			SH_PRINTF("Error: No Buffer to compress tranfer data.\n");
  			LIB_ASH_OC('E');
  			RETURNCODE(-1);
  		}
  			
	}
	else
    {
#ifdef PANEL_DISPLAY
        pan_display(panel_dev,  "r501", 4);
#endif
        SH_PRINTF("Usage: transfer <data_length>\n");
        RETURNCODE(-1);
    }

    if (start_addr == 0xFFFFFFFF)
    {
#ifdef PANEL_DISPLAY
        pan_display(panel_dev,  "r502", 4);
#endif
        SH_PRINTF("Error: Start address hasn't been initialized.\n");
        LIB_ASH_OC('E');
		RETURNCODE(-1);
    }
#ifdef PANEL_DISPLAY
    pan_display(panel_dev, "trn ", 4);
    led_display_type = 0;
#endif

    data_len = 0;
    last_error = 0;
    err_count = 0;
    // read transfer data
    while (data_len < len && err_count < TRANSFER_RETRY_TIMES)
    {
        l = len - data_len;
        if (l > PACKAGE_SIZE)
            l = PACKAGE_SIZE;
        /* Acknowledge for next data block */
        LIB_ASH_OC(last_error ? 'E' : 'O');
        /* transfer one data block + CRC */
        for (i = 0; i < l + 4; i++)
        {
            if (sci_read_tm(boot_uart_id, p + i, 1500 * 1000) != SUCCESS)
            {
                err_count++;
                break;
            }
        }
        if (i == l + 4)
        {
            crc = fetch_long(p + l);
            last_error = (crc != MG_Table_Driven_CRC(0xFFFFFFFF, \
                  p, l));
            if (!last_error)
            {
                p += l;
                data_len += l;
                err_count = 0;
            }
#ifdef PANEL_DISPLAY
            led_show((unsigned char *)down_str);
#endif
        }
        else
            last_error = 1;
    }

    if (data_len == len)
    /* transfer succeed */
    {
		if(zip == 0)
			image_free += len;
		else  //unzip
		{  	
			
			if(un7zip((UINT8 *)pos, (UINT8 *)image_free, buffer) == 0)
				image_free += real_len;
			else
			{
				SH_PRINTF("Error: Compress tranfer data error.\n");
				LIB_ASH_OC('E');
				RETURNCODE(-1);
			}
		}
    }
	LIB_ASH_OC('O');
	RETURNCODE(0);
}



static UINT32 cmd_burn_old(unsigned int argc, unsigned char *argv[])
{
#ifdef DEBUG
    unsigned int i;
#endif
    unsigned char *p;
    unsigned int sector_no;
    unsigned long temp_start;
    unsigned long temp_end;
    unsigned int len;
    unsigned int ret;
    unsigned char strbuf[16];

    if (start_addr == 0xFFFFFFFF || image_free == \
          (unsigned char *)image_buf_addr)
    {
#ifdef PANEL_DISPLAY
        pan_display(panel_dev,  "r601", 4);
#endif
        SH_PRINTF("Error: Nothing to burn.\n");
        LIB_ASH_OC('E');
        return -1;
    }

    LIB_ASH_OC('O');
#ifdef PANEL_DISPLAY
    pan_display(panel_dev, "bun ", 4);
    led_display_type = 0;
#endif
#ifdef HDCP_IN_FLASH
	// Check burn data include Bootloader sector or not.
	if( start_addr  < 0x10000 )
	{
		UINT8 	*hdcp_internal_keys;
		CHUNK_HEADER hdcp_chuck_hdr;
        UINT32 hdcp_chunk_offset_addr;
		UINT32 hdcp_chuck_id = HDCPKEY_CHUNK_ID;        
		if(sto_get_chunk_header(hdcp_chuck_id,&hdcp_chuck_hdr)!=0)
		{
			// read back the hdcp key from flash
			hdcp_internal_keys = (UINT8 *)MALLOC(sizeof(UINT8)*(hdcp_chuck_hdr.offset));
            hdcp_chunk_offset_addr = sto_chunk_goto(&hdcp_chuck_id,0xFFFFFFFF,1);
			sto_get_data( (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0), hdcp_internal_keys,
											hdcp_chunk_offset_addr, hdcp_chuck_hdr.offset);		
			// restore back the hdcp key to all code image					
			MEMCPY(image_buf_addr +hdcp_chunk_offset_addr,hdcp_internal_keys,hdcp_chuck_hdr.offset);		
			FREE(hdcp_internal_keys);
		}	
	}
#endif
#ifdef DIVX_CERT_ENABLE
	// Check burn data include Bootloader sector or not.
	{
		UINT8 	*divx_internal_keys;
		CHUNK_HEADER divx_chuck_hdr;
		UINT32 divx_chunk_offset_addr;
		UINT32 divx_chuck_id = DIVX_CHUCK_ID;        
		if(sto_get_chunk_header(divx_chuck_id,&divx_chuck_hdr)!=0)
		{
			// read back the hdcp key from flash
			divx_internal_keys = (UINT8 *)MALLOC(sizeof(UINT8)*(divx_chuck_hdr.offset));
            		divx_chunk_offset_addr = sto_chunk_goto(&divx_chuck_id,0xFFFFFFFF,1);
			sto_get_data( (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0), divx_internal_keys,
											divx_chunk_offset_addr, divx_chuck_hdr.offset);		
			// restore back the hdcp key to all code image					
			MEMCPY(image_buf_addr +divx_chunk_offset_addr,divx_internal_keys,divx_chuck_hdr.offset);		
			FREE(divx_internal_keys);
		}	
	}
#endif

    /* Begin burn: Erase sector */
    temp_end = image_free - (unsigned char *)image_buf_addr + \
          start_addr;
    if (temp_end < (unsigned long)flash_dev->totol_size)
    {
        sector_no = flash_sector_align(temp_end);
        temp_start = flash_sector_start(sector_no);
        if (temp_start < temp_end)
        {
            len = temp_start + flash_sector_size(sector_no) - \
                  temp_end;
            sto_get_data(flash_dev, image_free, temp_end, len);
            temp_end += len;
        }
    }
    else
        temp_end = flash_dev->totol_size;

    p = (unsigned char *)image_buf_addr;
    sector_no = flash_sector_align(start_addr);
    temp_start = start_addr;
    while (temp_start < temp_end)
    {
        len = flash_sector_size(sector_no++);
        switch (flash_verify(temp_start, p, len))
        {
        case 2:
            flash_erase_sector(temp_start);
        case 1:
            ret = flash_copy(temp_start, p, len);
            if (ret == 0)
                ret = flash_verify(temp_start, p, len);
            if (ret == 0)
                sprintf(strbuf, "B%04dK\n", temp_start >> 10);
            else
                sprintf(strbuf, "E%07d\n", temp_start);
            dump_str(strbuf);
#ifdef PANEL_DISPLAY
            led_show((unsigned char *)burn_str);
#endif

        }
        p += len;
        temp_start += len;
    }

#ifdef DEBUG
    for (i = 0; i < flash_sectors_val; i++)
        dump(FLASH_OFF2PTR(flash_sector_start(i)), 16);
#endif
    LIB_ASH_OC('F');

#ifdef PANEL_DISPLAY

    pan_display(panel_dev, "end-", 4);
#endif
#ifdef UPGRADE_ONCE
	receive_comtest_falg = FALSE;
#endif

#if (SYS_SDRAM_SIZE == 2)
	sys_watchdog_reboot();
#endif
    return 0;
}


static UINT32 cmd_reboot_old(unsigned int argc, unsigned char *argv[])
{
    LIB_ASH_OC('O');
#ifdef PANEL_DISPLAY
    pan_display(panel_dev, "off", 4);
#endif
    sys_ic_exit_standby();
#ifdef PANEL_DISPLAY
    pan_close((struct pan_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_PAN));
#endif
#ifdef WATCH_DOG_SUPPORT
    dog_stop(0);
#endif 
    sys_watchdog_reboot();
    return 0;
}

#endif


