/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2002 Copyright (C)
 *
 *  File: lib_ash.c
 *
 *  Description: This file contains all functions definition ALi shell
 *		         operation functions.
 *  History:
 *      Date        Author      Version  Comment
 *      ====        ======      =======  =======
 *  1.  2003.7.17  Justin Wu    0.0.000  Initial
 *  2.		
 ****************************************************************************/

#include <types.h>
#include <retcode.h>
#include <sys_config.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/sci/sci.h>
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


#ifdef PANEL_DISPLAY
struct pan_device* panel_dev=NULL;
#endif
struct sto_device *flash_dev=NULL;

#ifdef UPGRADE_ONCE
extern BOOL receive_comtest_falg;
#endif

static unsigned char cmd_buffer[LIB_ASH_COMMAND_LEN];
static unsigned char *argv[LIB_ASH_COMMAND_LST];
static unsigned int argc;

unsigned char g_packet_repeat = 0;
UINT32 g_packet_index = 0;
UINT32 g_last_error = 0;
UINT32 g_last_status = 0;
UINT32 g_protocol_version = 0x30;
	
static void split_command(unsigned char *buffer, UINT32 nBufLen)
{
    	unsigned char *p, *q;
    	p = buffer;
    	q = buffer + nBufLen - 1;	//delete last char '\r' or '\n'
 	
    	while (p < q)
    	{
       	 /* skip blank */
	        while ((*p == ' ' || *p == '\t' ) && p < q)
       	p++;
		/* new word */
		argv[argc++] = p;
		 /* find end of word */
		while (*p != ' ' && p < q)
            		p++;
        	*p++ = 0;
    	}

    unsigned int i;
    SH_PRINTF("Command: ");
    for (i = 0; i < argc; i++)
        SH_PRINTF("%s ", argv[i]);
    SH_PRINTF("\n");

}	

static UINT32 lib_ash_get_command(void)
{
	PACKET 	packet;
	UINT32 nReturn = packet_receive(&packet, 0xFFFFFFFF);
	if(SUCCESS == nReturn)
	{
		nReturn = GENERIC_ERROR;
		//if command line packet 
		if(packet.packet_type == PACKET_COMMAND)
		{
			//if new command line packet out to run it 		
			if(packet.packet_index != g_packet_index ||
				g_packet_index == 0)
			{
				g_packet_index = packet.packet_index;
				g_packet_repeat = packet.packet_repeat;
				split_command(packet.data_buffer, packet.packet_length);
				nReturn = SUCCESS;
			}
			else
			{
				//if old command line
				//judge if executed ok return executed status
				//else if command can retry retry it 
				//else send error status and error code
				g_packet_repeat = packet.packet_repeat;
				if(COMMAND_STATUS_EXECUTED != g_last_status &&
					RETRY_CODE_MAX > g_last_error)
					nReturn = SUCCESS;
			
			}

				
		}
		//if request data or data packet return error
			
	}
	else
	{
		// if receive packet head error(or head crc error) or no packet receive do nothing
		// else if receive packet data error send status packet to report error
		if(ERROR_NOPACKETRECEIVE == nReturn ||
			ERROR_PACKET_RECEIVEHEAD == nReturn ||
			ERROR_PACKET_HEADCRC == nReturn)
			return SUCCESS;

	}
	
	return nReturn;
}

void lib_ash_process_new()
{
	unsigned int i;
	UINT32 nReturn = lib_ash_get_command();
	
	if(SUCCESS != nReturn)
	{
		SendStatusPacket(g_last_status, g_last_error);
		return;
	}
	
	if (argc == 0)
		return;
	
	for (i = 0; i < lib_ash_command_num; i++)
	{
		if (STRCMP(lib_ash_command[i].command, argv[0]) == 0)
		{
			g_last_error = 0;
			g_last_status = COMMAND_STATUS_RUNNING;
			g_last_error = lib_ash_command[i].handler(argc, argv);
			if(SUCCESS == g_last_error)
				g_last_status = COMMAND_STATUS_EXECUTED;
			else
				g_last_status = COMMAND_STATUS_ERROR;		
			break;
		}
	}
		
	if (i == lib_ash_command_num)
	{
		SH_PRINTF("Unknow command: %s\n", argv[0]);
		SendStatusPacket(COMMAND_STATUS_ERROR, ERROR_NORECOGNIZECMD);
	}
	
}

#ifndef TRANSFER_FORMAT3_ONLY
static void lib_ash_get_command_old(void)
{
#ifdef DEBUG
    unsigned int i;
#endif
    unsigned char c;
    unsigned char *p, *q;

    p = cmd_buffer;
    q = cmd_buffer + LIB_ASH_COMMAND_LEN - 1;
    while (p < q)
    {
        /* Get a charactor and echo it */
        LIB_ASH_OC(c = LIB_ASH_IC());

        if (c == 0x0D || c == 0x0A)
        {
            q = p;
            break;
        }
        /* Backspace */
        else if (c == 0x08)
        {
            if (p != cmd_buffer)
                p--;
        }
        else
            *p++ = c;
    }

    argc = 0;
    p = cmd_buffer;
    while (p < q)
    {
        /* skip blank */
        while ((*p == ' ' || *p == '\t') && p < q)
            p++;
        /* new word */
        argv[argc++] = p;
        /* find end of word */
        while (*p != ' ' && p < q)
            p++;
        *p++ = 0;
    }

#ifdef DEBUG
    SH_PRINTF("Command: ");
    for (i = 0; i < argc; i++)
        SH_PRINTF("%s ", argv[i]);
    SH_PRINTF("\n");
#endif
}

void lib_ash_process_old()
{
	lib_ash_get_command_old();

	unsigned int i = 0;
	if (argc == 0)
		return;

    for (i = 0; i < LIB_ASH_OLDCOMMAND_NUM; i++)
	{
		if (STRCMP(lib_ash_command_old[i].command, argv[0]) == 0)
		{
			lib_ash_command_old[i].handler(argc, argv);
			break;
		}
	}
	
#ifdef DEBUG
	if (i == LIB_ASH_OLDCOMMAND_NUM)
	{
		SH_PRINTF("Unknow command: %s\n", argv[0]);
	}
#endif
}
#endif

void lib_ash_shell()
{
	static BOOL flash_need_identify=TRUE;
#ifdef PANEL_DISPLAY
    if(panel_dev==NULL)
        panel_dev=(struct pan_device*)dev_get_by_id(HLD_DEV_TYPE_PAN, 0);
#endif
    if(flash_dev==NULL)
        flash_dev=(struct sto_device*)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
	if(TRUE==flash_need_identify)
	{
		if (flash_identify() == 0)
		{
			ERRMSG("Unknow Flash type, Halt.\n");
			while (1);
	   	}
		flash_need_identify=FALSE;
	}


#ifdef PANEL_DISPLAY
#if (SYS_SDRAM_SIZE != 2)
    pan_display(panel_dev, "ASH ", 4);
#endif
#endif

	/* Command line process */
	while (1)
	{
		argc = 0;
#ifndef TRANSFER_FORMAT3_ONLY
		if(g_protocol_version >= 0x30)
			lib_ash_process_new();
		else
			lib_ash_process_old();
#else
		lib_ash_process_new();
#endif
    }
}

#endif

