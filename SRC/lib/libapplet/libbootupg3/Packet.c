/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2002 Copyright (C)
 *
 *  File: packet.cpp
 *
 *  Description: implement functions interface about packet 
 *		         
 *  History:
 *      Date		Author      Version		Comment
 *      ====        ======      =======		=======
 *  1   2007.8.13	Tony Wang	1.0			Create
 ****************************************************************************/

#include <types.h>
#include <sys_config.h>

#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libc/fastCRC.h>
#include <api/libchunk/chunk.h>
#include <bus/sci/sci.h>
#include <bus/flash/flash.h>
#include <hld/hld_dev.h>

#include "lib_ash.h"
#include "Packet.h"
#include "ErrorCode.h"

#if 0//def _DEBUG_VERSION_
#define DEBUG
#define SH_PRINTF   soc_printf
#else
#define SH_PRINTF(...)
#endif

unsigned char l_packet_head_flag[5] = "HEAD";
unsigned char l_packet_comtest[7] = "comtest";

#define Sleep(a)		osal_delay(a * 1000)
extern UINT32 boot_uart_id;

extern unsigned char g_packet_repeat;
extern UINT32 g_packet_index;

 UINT32 com_send(unsigned char * buffer, UINT32 nLen)
{
	UINT32 i = 0;
	for(i = 0; i < nLen; i++)
	{
		sci_write(boot_uart_id, buffer[i]);
	}
	return  i ;
}
 UINT32 com_read(unsigned char *buffer, UINT32 nLen)
{
	UINT32 nReturn = SUCCESS;
	UINT32 i = 0;
	for(i = 0; i < nLen; i++)
	{
		if(SUCCESS != sci_read_tm(boot_uart_id, buffer+i, 1))
		{
			nReturn = ERROR_READDATA_TIMEOUT;
			break;
		}
	}
	return nReturn;
}
UINT32 com_read_tm(unsigned char *buffer, UINT32 nLen, UINT32 nTimeOut)
{
	UINT32 nReturn = SUCCESS;
	UINT32 i = 0;
//	UINT32 nTicks = (nTimeOut/nLen) > 0 ? (nTimeOut /nLen) * 1000: 1000;
	for(i = 0; i < nLen; i++)
	{
		if(SUCCESS != sci_read_tm(boot_uart_id, buffer+i, nTimeOut * 1000))
		{
			nReturn = ERROR_READDATA_TIMEOUT;
			break;
		}
	}
	return nReturn;
}

/****************************************************************************
 *
 *  Name:		MakePacketHeadBuffer
 *	Function:	Make Packet head buffer
 *	Remark:		local function
 *	variable:	
 *  
 ****************************************************************************/
void MakePacketHeadBuffer(PPACKET pPacket, unsigned char *packet_head)
{
	MEMSET(packet_head, 0, PACKET_HEAD_LEN);

	// set packet head information;
	MEMCPY(packet_head + PACKET_HEAD_FLAG_OFFSET, l_packet_head_flag, sizeof(l_packet_head_flag) - 1);
	MEMCPY(packet_head + PACKET_TYPE_OFFSET, &(pPacket->packet_type), sizeof(pPacket->packet_type)); 
	MEMCPY(packet_head + PACKET_BLANKFLAG_OFFSET, &(pPacket->blank_flag), sizeof(pPacket->blank_flag));	

	store_long(packet_head + PACKET_INDEX_OFFSET, pPacket->packet_index);
	MEMCPY(packet_head + PACKET_REPEAT_OFFSET, &(pPacket->packet_repeat), sizeof(pPacket->packet_repeat));

	store_long( packet_head + PACKET_LENGTH_OFFSET, pPacket->packet_length);
	UINT32 nCRC = MG_Table_Driven_CRC(0xFFFFFFFF, packet_head + PACKET_TYPE_OFFSET, PACKET_HEAD_LEN - 8);
	store_long( packet_head + PACKET_HEAD_CRC_OFFSET, nCRC);
	
}

/****************************************************************************
 *
 *  Name:		
 *	Function:	Make Packet head buffer
 *	Remark:		local function
 *	variable:	
 *  
 ****************************************************************************/
void MakePacketHead(unsigned char *packet_head, PPACKET pPacket)
{
	MEMSET(pPacket, 0, sizeof(PACKET));

	// set packet head information;
	pPacket->packet_type = packet_head[PACKET_TYPE_OFFSET];
	pPacket->blank_flag = packet_head[PACKET_BLANKFLAG_OFFSET];
	
	pPacket->packet_index = fetch_long(packet_head + PACKET_INDEX_OFFSET);
	pPacket->packet_repeat = packet_head[PACKET_REPEAT_OFFSET];

	pPacket->packet_length = fetch_long(packet_head + PACKET_LENGTH_OFFSET);
	
}

/****************************************************************************
 *
 *  Name:		packet_send
 *	Function:	send a packet
 *	variable:	pPacket 	-- packet information point
 *  
 ****************************************************************************/
UINT32 packet_send(PPACKET pPacket)
{
	//create packet head buffer;
	unsigned char packet_head[PACKET_HEAD_LEN];
	unsigned char szCRC[4];
	
	MakePacketHeadBuffer(pPacket, packet_head);
	
	//send packet header
	if(PACKET_HEAD_LEN != com_send(packet_head, PACKET_HEAD_LEN))
	{
		return ERROR_SENDPACKETHEAD;
	}

	UINT32 nLength = pPacket->packet_length;

	if(0 != pPacket->blank_flag && PACKET_DATA == pPacket->packet_type)
		nLength = 4;
	
	if(nLength != com_send(pPacket->data_buffer, nLength))
	{
		return ERROR_SENDPACKETDATA;
	}

	UINT32 nCRC = MG_Table_Driven_CRC(0xFFFFFFFF, pPacket->data_buffer, nLength);
	store_long(szCRC, nCRC);
	if(4 != com_send(szCRC, 4))
	{
		return ERROR_SENDPACKETCRC;
	}	

	return SUCCESS;
}

/****************************************************************************
 *
 *  Name:		packet_receive
 *	Function:	receive a packet
 *	variable:	pPacket 	-- packet information point
 *				nTimeOut	-- time out   
 *  
 ****************************************************************************/
UINT32 packet_receive(PPACKET pPacket, UINT32 nTimeOut)
{
	//receive and compare packet head flag
	unsigned char c = 0;
	BOOL bPacketHead = FALSE;
	BOOL bComtest = FALSE;
	UINT32 i = 0;
	int n = 0;
	UINT32 nLength = 0;
	MEMSET(pPacket, 0, sizeof(PACKET));
	for(i = 0; i < nTimeOut;)
	{
		if( c == 'H')
		{
			// judge if packet head
			for(n = 0; n < 3; n++)
			{
				if(SUCCESS != com_read_tm(&c, 1, 1))
				{
					c = 0;
					break;
				}
				if(c != l_packet_head_flag[n + 1])
					break;
			}

			if(n == 3)
			{
				bPacketHead = TRUE;
				break;
			}
		}
		else if(c == 'c')
		{
			pPacket->data_buffer[0] = 'c';
			LIB_ASH_OC(c);
			//judge if comtest
			for(n = 0; n < 6; n++)
			{
		//		c = LIB_ASH_IC();
				if(SUCCESS != com_read_tm(&c, 1, 100))
				{
					c = 0;
					SH_PRINTF("comtest not all : %d\n", n);
					break;
				}
				if(c != l_packet_comtest[n + 1])
					break;
				LIB_ASH_OC(c);
				
			}

			if(n == 6)
			{
				SH_PRINTF("RECEIVE comtest\n");
				bComtest = TRUE;
				break;
			}

		}
		else if(SUCCESS != com_read(&c, 1))
		{
			Sleep(1);
			i++;
		}	
		
	}

	
	if(bPacketHead)
	{
		//receive packet head
		unsigned char packet_head[PACKET_HEAD_LEN];
		if(SUCCESS != com_read_tm(packet_head + PACKET_TYPE_OFFSET, PACKET_HEAD_LEN - 4, 1000))
		{
			SH_PRINTF("ERROR:receive head error");
			return ERROR_PACKET_RECEIVEHEAD;
		}
		
		//compare CRC about packet head
		UINT32 nCRC = MG_Table_Driven_CRC(0xFFFFFFFF, packet_head + PACKET_TYPE_OFFSET, PACKET_HEAD_LEN - 8);
		if(nCRC != fetch_long(packet_head + PACKET_HEAD_CRC_OFFSET))
			return ERROR_PACKET_HEADCRC;
		
		MakePacketHead(packet_head, pPacket);

		//Receive data and CRC
		nLength = pPacket->packet_length;
	
		// if blank packet receive 4B packet number only
		if(0 != pPacket->blank_flag && PACKET_DATA == pPacket->packet_type)
			nLength = 4;
		
		if(SUCCESS != com_read_tm(pPacket->data_buffer, nLength + 4, 1000))
			return ERROR_PACKET_RECEIVEDATA;

		nCRC = MG_Table_Driven_CRC(0xFFFFFFFF, pPacket->data_buffer, nLength);
		if(nCRC != fetch_long(pPacket->data_buffer + nLength))
			return ERROR_PACKET_DATACRC;
		if(PACKET_DATA == pPacket->packet_type)
		{
			if(1 == pPacket->blank_flag)
				memset(pPacket->data_buffer + 4, 0xFF, pPacket->packet_length - 4);
			else if(2 == pPacket->blank_flag)
				memset(pPacket->data_buffer + 4, 0x00, pPacket->packet_length - 4);
		}
	}
	else if(bComtest)
	{
		pPacket->packet_type = PACKET_COMMAND;
		MEMCPY(pPacket->data_buffer, l_packet_comtest, 7);
		i = 7;
		while(i < 128)
		{
			pPacket->data_buffer[i] = LIB_ASH_IC();
			c = pPacket->data_buffer[i];
			LIB_ASH_OC(c);
			if (c == 0x0D || c == 0x0A)
		            break;			
			i++;
		}

		pPacket->packet_length = STRLEN(pPacket->data_buffer);
//		for(i = 0; i < pPacket->packet_length; i++)
//			LIB_ASH_OC(pPacket->data_buffer[i]);
	
       	SH_PRINTF(pPacket->data_buffer);
		
		g_packet_index = 0;
		g_packet_repeat = 0;
		
	}	
	else
		return ERROR_NOPACKETRECEIVE;
	
	return SUCCESS;
}

/****************************************************************************
 *
 *  Name:		SendStatusPacket
 *	Function:	send a status packet
 *	variable:	cStatus	-- status
 *			nCode 	-- error code or status code
 *				
 *
 ****************************************************************************/
UINT32 SendStatusPacket(unsigned char cStatus, UINT32 nCode)
{
	PACKET packet;
	MEMSET(&packet, 0 , sizeof(PACKET));
	packet.packet_type = PACKET_STATUS;
	packet.packet_index = g_packet_index;
	packet.packet_repeat = g_packet_repeat;
	packet.blank_flag = 0;
	packet.packet_length = 5;
	packet.data_buffer[0] = cStatus;
	store_long(packet.data_buffer + 1, nCode);
	return packet_send(&packet);
}


/****************************************************************************
 *
 *  Name:		SendDataPacket
 *	Function:	Send a data Packet
 *	variable:	szBuffer		-- data buffer to send
 *				nLen			-- data buffer length
 *				nPacketNum		-- packet number about data
 *				
 *				
 *  
 ****************************************************************************/
 BOOL IsBlankPacket(unsigned char *szBuffer, UINT32 nLen)
{
	BOOL bBlankPacket = TRUE;
	UINT32 i = 0;
	for(i = 0; i < nLen; i++)
	{
		if(0xFF != szBuffer[i])
		{
			bBlankPacket = FALSE;
			break;
		}	
	}
	return bBlankPacket;
}
UINT32 SendDataPacket(UINT32 nPacketNum, unsigned char *szBuffer, UINT32 nLen)
{
	//data in packet = (4B)PacketNum + data
	PACKET	packet;
	MEMSET(&packet, 0, sizeof(PACKET));
	packet.packet_type = PACKET_DATA;
	packet.packet_length = nLen + 4;		

	store_long(packet.data_buffer, nPacketNum);
	MEMCPY(packet.data_buffer + 4, szBuffer, nLen);
	if(IsBlankPacket(szBuffer, nLen))	
		packet.blank_flag = 1;
	else
		packet.blank_flag = 0;
	
	packet.packet_index = g_packet_index;
	packet.packet_repeat = g_packet_repeat;

	return packet_send(&packet);
}

