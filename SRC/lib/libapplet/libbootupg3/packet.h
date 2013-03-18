/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2002 Copyright (C)
 *
 *  File: Packet.h
 *
 *  Description: define information packet variable & function for commicate
 *		         
 *  History:
 *      Date		Author      Version		Comment
 *      ====        ======      =======		=======
 *  1   2007.8.15	Tony Wang	1.0			Create
 ****************************************************************************/


#ifndef __PACKET_H__
#define __PACKET_H__

#ifdef __cplusplus
extern "C"
{
#endif

/********************************* Variable Define *********************************/

#define PACKET_HEAD_LEN		19
#define MAX_PACKET_SIZE		2048
#define STATUS_DATA_LEN		5


#define PACKET_HEAD_FLAG_OFFSET		0
#define PACKET_TYPE_OFFSET			4
#define PACKET_BLANKFLAG_OFFSET		5
#define PACKET_INDEX_OFFSET			6
#define PACKET_REPEAT_OFFSET		10
#define PACKET_LENGTH_OFFSET		11
#define PACKET_HEAD_CRC_OFFSET		15

typedef enum
{
	PACKET_COMMAND = 0, 
	PACKET_DATA,
	PACKET_REQUEST,
	PACKET_STATUS
}PACKET_TYPE;

typedef enum
{
	COMMAND_STATUS_OK = 0,
	COMMAND_STATUS_ERROR,
	COMMAND_STATUS_RUNNING,
	COMMAND_STATUS_EXECUTED,
	COMMAND_STATUS_CANCEL
}COMMAND_STATUS;

typedef struct
{
	unsigned char	packet_type;
	unsigned char	blank_flag;
	UINT32			packet_index;
	unsigned char	packet_repeat;
	UINT32			packet_length;
	unsigned char	data_buffer[MAX_PACKET_SIZE];
}PACKET, * PPACKET;


/********************************* Function Define *********************************/
/****************************************************************************
 *
 *  Name:		packet_send
 *	Function:	send a packet
 *	variable:	pPacket 	-- packet information point
 *  
 ****************************************************************************/
UINT32 packet_send(PPACKET pPacket);

/****************************************************************************
 *
 *  Name:		packet_receive
 *	Function:	receive a packet
 *	variable:	pPacket 	-- packet information point
 *				nTimeOut	-- time out   
 *
 ****************************************************************************/
UINT32 packet_receive(PPACKET pPacket, UINT32 nTimeOut);


/****************************************************************************
 *
 *  Name:		packet_send
 *	Function:	send a status packet
 *	variable:	cStatus	-- status
 *			nCode 	-- error code or status code
 *				
 *
 ****************************************************************************/
UINT32 SendStatusPacket(unsigned char cStatus, UINT32 nCode);

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
UINT32 SendDataPacket(UINT32 nPacketNum, unsigned char *szBuffer, UINT32 nLen);

#ifdef __cplusplus
}
#endif		/*	__cplusplus		*/

#endif	/* __PACKET_H__ */

