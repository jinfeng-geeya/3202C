/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2006 Copyright (C)
 *
 *  File: smartcard.c
 *
 *  Description: This file contains all globe micros and functions declare
 *		             of smartcard reader.
 *
 *  History:
 *      Date            Author            Version   Comment
 *      ====        ======      =======  =======
 *  0.                 Victor Chen            Ref. code
 *  1. 2005.9.8  Gushun Chen     0.1.000    Initial
 *  2. 2006.1.5  Gushun Chen     0.2.000    Add ISO read/write, add semaphore
 *  3. 2006.12.5  Victor Chen	 0.3.000    Update the driver to only use smartcard module.
 ****************************************************************************/

#include <sys_config.h>
#include <types.h>
#include <retcode.h>
#include <hld/hld_dev.h>
#include <osal/osal.h>
#include <hal/hal_gpio.h>
#include <api/libc/printf.h>
#include <api/libc/alloc.h>
#include <api/libc/string.h>
#include <hld/smc/smc.h>
#include <bus/sci/sci.h>


#include "smartcard.h"
#include "smc_t1_buffer.h"
//#define SCR1

static INT32 smc_dev_open(struct smc_device *dev, void (*callback)(UINT32 param));
static INT32 smc_dev_close(struct smc_device *dev);
static INT32 smc_dev_card_exist(struct smc_device *dev);
static INT32 smc_dev_ioctl(struct smc_device *dev, INT32 cmd, UINT32 param);
static INT32 smc_dev_reset(struct smc_device *dev, UINT8 *buffer, UINT16 *atr_size);
static INT32 smc_dev_deactive(struct smc_device *dev);
static INT32 smc_dev_read(struct smc_device *dev, UINT8 *buffer, INT16 size, INT16 *actsize);
static INT32 smc_dev_write(struct smc_device *dev, UINT8 *buffer, INT16 size, INT16 *actsize);
static INT32 smc_dev_transfer_data(struct smc_device *dev, UINT8 *buffer, UINT16 size, UINT8 *recv_buffer, UINT16 reply_num, UINT16 *actsize);
static INT32 smc_dev_iso_transfer(struct smc_device *dev, UINT8 *command, INT16 num_to_write, UINT8 *response, INT16 num_to_read, INT16 *actual_size);
static INT32 smc_dev_iso_transfer_t1(struct smc_device *dev, UINT8 *command, INT16 num_to_write,UINT8 *response, INT16 num_to_read,INT32 *actual_size);
static void smc_init_hw(struct smc_device *dev);
static void smc_set_wclk(UINT32 ioaddr, UINT32 clk);
static INT32 smc_warm_reset(struct smc_device *dev);
static void invert(UINT8 *data, INT32 n);
static INT32 smc_dev_multi_class_reset(struct smc_device *dev, UINT8 *buffer, UINT16 *atr_size);

#define SMC_RESET_L_INTERVAL	60 // t >= 40000/f = 11.2ms, tongfang card need t < 500ms

#define WAITTIMEOUT		100000

#define	SMC_DEV_NUM	2

#define FORCE_TX_RX_THLD		2
/********************************************
T1 related Macro define
*********************************************/
/* I block */
#define T1_I_SEQ_SHIFT		6

/* R block */
#define T1_IS_ERROR(pcb)	((pcb) & 0x0F)
#define T1_EDC_ERROR		0x01
#define T1_OTHER_ERROR		0x02
#define T1_R_SEQ_SHIFT		4

/* S block stuff */
#define T1_S_IS_RESPONSE(pcb)	((pcb) & T1_S_RESPONSE)
#define T1_S_TYPE(pcb)		((pcb) & 0x0F)
#define T1_S_RESPONSE		0x20
#define T1_S_RESYNC		0x00
#define T1_S_IFS		0x01
#define T1_S_ABORT		0x02
#define T1_S_WTX		0x03

#define swap_nibbles(x) ( (x >> 4) | ((x & 0xF) << 4) )

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif


#define NAD 0
#define PCB 1
#define LEN 2
#define DATA 3

/* internal state, do not mess with it. */
/* should be != DEAD after reset/init */
enum 
{
	SENDING, 
	RECEIVING, 
	RESYNCH, 
	DEAD
};
/********************************************
T1 related Macro define ending 
*********************************************/
/* Name for the smart card reader, the last character must be Number for index */
static char smc_dev_name[HLD_MAX_NAME_SIZE] = "SMC_DEV_0";

static UINT32 scr_sys_clk = 0;
static UINT32 pwm_sys_clk = 0;
static UINT32 smc_chip_id = 0xffffffff;
static UINT32 smc_chip_version = 0xffffffff;
static UINT16 smc_tx_fifo_size = 64;
static UINT16 smc_rx_fifo_size = 64;
static struct /*The default setting is for M3602 serial*/
{
	UINT32 io_base;
	int    irq;
	struct smc_device * dev;
	UINT32 pwm_addr;
	UINT8 pwm_sel_ofst;
	UINT8 pwm_seh_ofst;
	UINT8 pwm_gpio_ofst;
	UINT8 pwm_cfg_ofst;
	UINT8 pwm_frac_ofst;
}smc_dev_set[SMC_DEV_NUM] = 
{
	{0xb8018800,20, NULL, 0xb8000000, 0, 0, 0, 0, 0},
	{0xb8018900,21, NULL, 0xb8000000, 0, 0, 0, 0, 0}
};

#define F_RFU	0
#define D_RFU	0
#define I_RFU	0

#define ATR_ID_NUM		16
#define ATR_FI_NUM		16
#define ATR_DI_NUM		16
#define ATR_I_NUM		4
static UINT32 atr_num_ib_table[ATR_ID_NUM] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};

static UINT32 atr_f_table[ATR_FI_NUM] ={372,372,558,744,1116,1488,1860,F_RFU,F_RFU,512,768,1024,1536,2048,F_RFU,F_RFU};

static UINT32 atr_d_table[ATR_DI_NUM] ={D_RFU,1,2,4,8,16,32,D_RFU,12,20,D_RFU,D_RFU,D_RFU,D_RFU,D_RFU,D_RFU};

static UINT32 atr_i_table[ATR_I_NUM] = {25, 50, 100, 0};

#define min( a, b )   ( ( ( a ) < ( b ) ) ? ( a ) : ( b ) )

/* Correct Table? */

static UINT16 crctab[256] = 
{
	0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
	0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
	0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
	0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
	0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
	0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
	0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
	0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
	0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
	0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
	0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
	0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
	0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
	0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
	0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
	0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
	0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
	0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
	0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
	0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
	0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
	0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
	0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
	0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
	0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
	0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
	0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
	0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
	0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
	0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
	0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
	0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};

/* Returns LRC of data.*/
 UINT32 t1_lrc(UINT8* data, UINT32 len, UINT8* rc) 
{
	UINT8	lrc = 0;

	while (len--)
		lrc ^= *data++;

	if (rc)
		*rc = lrc;
	return 1;
	 
}

/* Compute CRC of data.*/
 UINT32 t1_crc(UINT8* data, UINT32 len, UINT8*rc) 
{
	UINT16 v = 0xFFFF;

	while (len--) {
		v = ((v >> 8) & 0xFF) ^ crctab[(v ^ *data++) & 0xFF];
	}

	if (rc) {
		rc[0] = (v >> 8) & 0xFF;
		rc[1] = v & 0xFF;
	}

	return 2;
}

/***********************************************************************
 *
 *Below are functions about ISO/IEC 7816-3 Protocol T=1
 *The main interface for T1 is the function t1_transceive(), it is used to translate/receive
 *T1 data
 *
 *******************************************************************/

/*choose the error check algorithm*/
static void t1_set_checksum(t1_state_t *t1, UINT8 csum)
{
	switch (csum) {
	case IFD_PROTOCOL_T1_CHECKSUM_LRC:
		t1->rc_bytes = 1;
		t1->checksum = t1_lrc;
		break;
	case IFD_PROTOCOL_T1_CHECKSUM_CRC:
		t1->rc_bytes = 2;
		t1->checksum = t1_crc;
		break;
	}
}


 /* Set default T1 protocol parameters*/
static void t1_set_defaults(t1_state_t *t1)
{
	t1->retries  = 3;
	t1->ifsc     = ATR_DEFAULT_IFSC;
	t1->ifsd     = ATR_DEFAULT_IFSD;
	t1->nr	  = 0;
	t1->ns	  = 0;
	t1->wtx	  = 0;
}


/* set parmaters for T1 protocol*/
static INT32 t1_set_param(t1_state_t *t1, INT32 type, INT32 value)
{
	switch (type) 
	{
		case IFD_PROTOCOL_T1_CHECKSUM_LRC:
		case IFD_PROTOCOL_T1_CHECKSUM_CRC:
			t1_set_checksum(t1, type);
			break;
		case IFD_PROTOCOL_T1_IFSC:
			t1->ifsc = value;
			break;
		case IFD_PROTOCOL_T1_IFSD:
			t1->ifsd = value;
			break;
		case IFD_PROTOCOL_T1_STATE:
			t1->state = value;
			break;
		case IFD_PROTOCOL_T1_MORE:
			t1->more = value;
			break;
		default:
			T1PRINTF("Unsupported parameter %d\n", type);
			return -1;
	}

	return RET_SUCCESS;
}


/* check the block type by PCB*/
static UINT32 t1_block_type(UINT8 pcb)
{
	switch (pcb & 0xC0) 
	{
		case T1_R_BLOCK:
			return T1_R_BLOCK;
		case T1_S_BLOCK:
			return T1_S_BLOCK;
		default:
			return T1_I_BLOCK;
	}
}


/* set number sequnce for I/R block*/
static UINT32 t1_seq(UINT8 pcb)
{
	switch (pcb & 0xC0) 
	{
		case T1_R_BLOCK:
			return (pcb >> T1_R_SEQ_SHIFT) & 1;
		case T1_S_BLOCK:
			return 0;
		default:
			return (pcb >> T1_I_SEQ_SHIFT) & 1;
	}
}


/* Build checksum*/
static UINT32 t1_compute_checksum(t1_state_t *t1,UINT8 *data, UINT32 len)
{
	return len + t1->checksum(data, len, data + len);
}

/* verify checksum*/
static INT32 t1_verify_checksum(t1_state_t *t1, UINT8*rbuf,UINT32 len)
{
	unsigned char	csum[2];
	int		m, n;

	m = len - t1->rc_bytes;
	n = t1->rc_bytes;

	if (m < 0)
		return 0;

	t1->checksum(rbuf, m, csum);
	if (!MEMCMP(rbuf + m, csum, n))
		return 1;

	return 0;
}

/*init T1 */
INT32 t1_init(t1_state_t *t1)
{
	t1_set_defaults(t1);
	t1_set_param(t1, IFD_PROTOCOL_T1_CHECKSUM_LRC, 0);
	t1_set_param(t1, IFD_PROTOCOL_T1_STATE, SENDING);
	t1_set_param(t1, IFD_PROTOCOL_T1_MORE, FALSE);

	return 0;
}

/*Detach t1 protocol*/
void t1_release(t1_state_t *t1)
{

}

/*update the T1 block wait time when receiving S-wtx request*/
static void t1_update_BWT(t1_state_t *t1, UINT32 wtx)
{

	t1->BWT = wtx * t1->BWT;
	T1PRINTF("New timeout at WTX request: %d sec\n", t1->BWT);
}
static void t1_restore_BWT(struct smartcard_private *tp)
{
	if (tp != NULL)
		/*BWT = (2^BWI*960 + 11)etu*/
		tp->T1.BWT= tp->first_cwt;
}

/*construct the block*/
UINT32 t1_build(t1_state_t *t1, UINT8 *block, UINT8 dad, UINT8 pcb, t1_buf_t *bp, UINT32 *lenp)
{
	UINT32 len;
	INT8 more = FALSE;

	len = bp? t1_buf_avail(bp) : 0;
	if (len > t1->ifsc) 
	{
		pcb |= T1_MORE_BLOCKS;
		len = t1->ifsc;
		more = TRUE;
	}

	/* Add the sequence number */
	switch (t1_block_type(pcb)) 
	{
		case T1_R_BLOCK:
			pcb |= t1->nr << T1_R_SEQ_SHIFT;
			break;
		case T1_I_BLOCK:
			pcb |= t1->ns << T1_I_SEQ_SHIFT;
			t1->more = more;
			T1PRINTF("more bit: %d\n", more);
			break;
	}

	block[0] = dad;
	block[1] = pcb;
	block[2] = len;

	if (len)
		MEMCPY(block + 3, t1_buf_head(bp), len);
	if (lenp)
		*lenp = len;

	len = t1_compute_checksum(t1, block, len + 3);

	/* memorize the last sent block */
	/* only 4 bytes since we are only interesed in R-blocks */
	MEMCPY(t1->previous_block, block, 4);

	return len;
}


/*reconstruct the last sent block*/
static UINT32 t1_rebuild(t1_state_t *t1, UINT8 *block)
{
	UINT8 pcb = t1->previous_block[1];
	
	/* copy the last sent block */
	if (T1_R_BLOCK == t1_block_type(pcb))
		MEMCPY(block, t1->previous_block, 4);
	else
	{
		T1PRINTF("previous block was not R-Block: %02X\n", pcb);
		return 0;
	}

	return 4;
}


/* Send/receive block*/
static INT32 t1_xcv(struct smc_device *dev, UINT8*sblock, UINT32 slen, UINT8 *rblock, UINT32 rmax, UINT32 *ractual)
{
	INT32 n;
	UINT32 rmax_int,i,m;
	UINT32 actual=0;
	UINT8 dad, dad1;

	struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
	if(tp->T!=1)
		return -1;
	t1_state_t *t1 = &tp->T1;
	
	//T1PRINTF("sending block : %d bytes need to be send\n ", slen);

	if ((sblock == NULL)||(rblock == NULL))
		return -1;
	if (t1->wtx > 1)
	{
		/*set the new temporary timeout at WTX card request */
		t1_update_BWT(t1,t1->wtx);
	}

/************************************************************
  *Note: For some cards, the block head and body should be readed separately.
  *          If that happens, the below should be modified to write/read twice.
  ***********************************************************/
  	dad = *sblock;
#if 1
	//n = smc_uart_T1_write(dev, slen, sblock);
	n = smc_dev_write(dev, sblock, slen, (INT16 *)(&actual));
	t1->wtx = 0;	/* reset to default value ??????????*/
	if (n != RET_SUCCESS)
	{
		T1PRINTF("SMC write error in t1_xcv!\n");	
		return SMART_WRITE_ERROR;//return n;
	}
#else
	n = smc_uart_fifowrite(dev, sblock, slen, &actual);
	if ((n != RET_SUCCESS))//||(actual != slen))
	{
		T1PRINTF("SMC write error in t1_xcv!\n");	
		return SMART_WRITE_ERROR;//return n;
	}
#endif
	/* Get the response en bloc */
	MEMSET(rblock, 0, rmax);
	rmax_int = rmax;
	//n = smc_uart_T1_read(dev, rmax_int, rblock, &actual);
	n = smc_dev_read(dev, rblock, rmax_int, (INT16 *)(&actual));
	rmax = rmax_int;
		
	if ((n == SMART_PARITY_ERROR)|| (n == SMART_NO_ANSWER))  //current not implemented for parity check
	{
		T1PRINTF("SMC read-no answer!\n");
		return n;
	}	
	if ((n == SMART_NOT_INSERTED) ||(n == SMART_NOT_RESETED))
	{
		T1PRINTF("SMC read fetal error in t1_xcv!\n");
		return -1; //fetal error
	}

	//T1PRINTF("t1_xcv read ok, %d bytes got\n ", actual);
	#if 0
	for (i=0;i<actual; i++)
		libc_printf(" 0x%02x ", rblock[i]);
	libc_printf("\n");
	#endif
	dad1 = *rblock;
	invert(&dad1, 1);
	if ((actual > 0)&&(dad1 == dad))
	{
		m = rblock[2] + 3 + t1->rc_bytes;
		if (m < actual)
			actual = m;
	}
	
	if (actual > 0)
	{
//		T1PRINTF("t1_xcv read OK, %d bytes got!\n", actual);
	}
	*ractual = actual;

	/* Restore initial timeout */
	t1_restore_BWT(tp);
	return RET_SUCCESS;
}

INT32 t1_negociate_ifsd(struct smc_device*dev, UINT32 dad, INT32 ifsd)
{
	t1_buf_t	sbuf;
	UINT8 * sdata = NULL;
	UINT32 slen;
	UINT32 retries;
	UINT32 snd_len;
	INT32 n;
	UINT8 snd_buf[1];
	UINT32 actual =  0, i;
	struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
	t1_state_t *t1 = &tp->T1;
	sdata = &(t1->sdata[0]);
	retries = t1->retries;

	/* S-block IFSD request */
	snd_buf[0] = ifsd;
	snd_len = 1;

	/* Initialize send/recv buffer */
	t1_buf_set(&sbuf, (void *) snd_buf, snd_len);

	while (TRUE)
	{
		/* Build the block */
		slen = t1_build(t1, sdata, dad, T1_S_BLOCK | T1_S_IFS, &sbuf, NULL);

		/* Send the block */
		n = t1_xcv(dev, sdata, slen, sdata, T1_BUFFER_SIZE, &actual);

		retries--;
		/* ISO 7816-3 Rule 7.4.2 */
		if (retries == 0)
			goto error;

		if (-1 == n)
		{
			T1PRINTF("fatal: transmit/receive failed\n");
			goto error;
		}

		if ((SMART_NO_ANSWER== n)	|| (SMART_WRITE_ERROR == n)							/* Parity error */
			|| (sdata[DATA] != ifsd)				/* Wrong ifsd received */
			|| (sdata[NAD] != swap_nibbles(dad))	/* wrong NAD */
			|| (!t1_verify_checksum(t1, sdata, actual))	/* checksum failed */
			|| (actual != (UINT32)4 + t1->rc_bytes)				/* wrong frame length */
			|| (sdata[LEN] != 1)					/* wrong data length */
			|| (sdata[PCB] != (T1_S_BLOCK | T1_S_RESPONSE | T1_S_IFS))) /* wrong PCB */
			continue;

		/* no more error */
		goto done;
	}

done:
	#if 0
	for (i=0;i<actual;i++)
		T1PRINTF(" 0x%02x ", sdata[i]);
	#endif 
	return RET_SUCCESS;

error:
	t1->state = DEAD;
	return -1;
}

/* Send an APDU through T=1, rcv_len usually the size of rcv_buf, return the actual size readed to the rcv_buf*/
INT32 t1_transceive(struct smc_device*dev, UINT8 dad, const void *snd_buf, UINT32 snd_len, void *rcv_buf, UINT32 rcv_len)
{
	t1_buf_t	sbuf, rbuf, tbuf;
	UINT8	sblk[5];
	UINT32	slen, retries, resyncs, sent_length = 0;
	UINT32		last_send = 0;
	UINT32 	actual = 0,i;
	UINT8 * sdata = NULL;
	UINT8 * rdata = NULL;
	struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
	t1_state_t *t1 = &tp->T1;
	sdata = &(t1->sdata[0]);
	rdata = &(t1->rdata[0]);
	if (snd_len == 0)
		return -1;

	/* we can't talk to a dead card / reader. Reset it! */
	if (t1->state == DEAD)
	{
		T1PRINTF("T=1 state machine is DEAD. Reset the card first.\n");
		return -1;
	}

	t1->state = SENDING;
	retries = t1->retries+1;
	resyncs = 3;

	/* Initialize send/recv buffer */
	t1_buf_set(&sbuf, (void *) snd_buf, snd_len);
	t1_buf_init(&rbuf, rcv_buf, rcv_len);

	/* Send the first block */
	slen = t1_build(t1, sdata, dad, T1_I_BLOCK, &sbuf, &last_send);

	while (1) {
		UINT8 pcb;
		INT32 n;

		retries--;
		if (retries == 0)
			goto error;

		n = t1_xcv(dev, sdata, slen, rdata,T1_BUFFER_SIZE, &actual);
		#if 0
		if ((SMART_PARITY_ERROR == n) ) //not support parity error now
		{
			T1PRINTF("Parity error\n");
			/* ISO 7816-3 Rule 7.4.2 */
			if (retries == 0)
				goto resync;

			/* ISO 7816-3 Rule 7.2 */
			if (T1_R_BLOCK == t1_block_type(t1->previous_block[PCB]))
			{
				T1PRINTF("Rule 7.2\n");
				slen = t1_rebuild(t1, sdata);
				continue;
			}

			slen = t1_build(t1, sdata,
					dad, T1_R_BLOCK | T1_EDC_ERROR,
					NULL, NULL);
			continue;
		}
		#endif
		if ((SMART_NO_ANSWER == n)||(SMART_WRITE_ERROR==n))
		{
			if (retries == 0)
				goto error;
			continue; //resend the command
		}

		if (n < 0) 
		{
			T1PRINTF("fatal: transmit/receive failed\n");
			t1->state = DEAD;
			goto error;
		}

		if ((rdata[NAD] != swap_nibbles(dad)) /* wrong NAD */
			|| (rdata[LEN] == 0xFF))	/* length == 0xFF (illegal) */
		{
			T1PRINTF("Bad NAD, retry\n");
			for (i=0; i<actual; i++)
				T1PRINTF(" 0x%02x ", rdata[i]);
			/* ISO 7816-3 Rule 7.4.2 */
			if (retries == 0)
				goto resync;

			/* ISO 7816-3 Rule 7.2 */
			if (T1_R_BLOCK == t1_block_type(t1->previous_block[PCB]))
			{
				T1PRINTF("Rule 7.2\n");
				slen = t1_rebuild(t1, sdata);
				continue;
			}
			#if 0
			slen = t1_build(t1, sdata,
				dad, T1_R_BLOCK | T1_OTHER_ERROR,
				NULL, NULL);
			#endif
			continue;
		}

		if (!t1_verify_checksum(t1, rdata, actual)) 
		{
			T1PRINTF("checksum failed\n");
			for (i=0; i<actual; i++)
				T1PRINTF(" 0x%02x ", rdata[i]);
			/* ISO 7816-3 Rule 7.4.2 */
			if (retries == 0)
				goto resync;

			/* ISO 7816-3 Rule 7.2 */
			if (T1_R_BLOCK == t1_block_type(t1->previous_block[PCB]))
			{
				T1PRINTF("Rule 7.2\n");
				slen = t1_rebuild(t1, sdata);
				continue;
			}

			slen = t1_build(t1, sdata,
				dad, T1_R_BLOCK | T1_EDC_ERROR,
				NULL, NULL);
			continue;
		}

		pcb = rdata[PCB];
		switch (t1_block_type(pcb)) 
		{
		case T1_R_BLOCK:
			if ((rdata[LEN] != 0x00)	/* length != 0x00 (illegal) */
				|| (pcb & 0x20)			/* b6 of pcb is set */
			   )
			{
				T1PRINTF("R-Block required\n");
				/* ISO 7816-3 Rule 7.4.2 */
				if (retries == 0)
					goto resync;

				/* ISO 7816-3 Rule 7.2 */
				if (T1_R_BLOCK == t1_block_type(t1->previous_block[1]))
				{
					T1PRINTF("Rule 7.2\n");
					slen = t1_rebuild(t1, sdata);
					continue;
				}

				slen = t1_build(t1, sdata,
						dad, T1_R_BLOCK | T1_OTHER_ERROR,
						NULL, NULL);
				continue;
			}

			if (((t1_seq(pcb) != t1->ns)	/* wrong sequence number & no bit more */
					&& ! t1->more)
			   )
			{
				T1PRINTF("received: %d, expected: %d, more: %d\n",
					t1_seq(pcb), t1->ns, t1->more);

				/* ISO 7816-3 Rule 7.2 */
				if (T1_R_BLOCK == t1_block_type(t1->previous_block[PCB]))
				{
					T1PRINTF("Rule 7.2\n");
					slen = t1_rebuild(t1, sdata);
					continue;
				}

				T1PRINTF("R-Block required\n");
				/* ISO 7816-3 Rule 7.4.2 */
				if (retries == 0)
					goto resync;
				slen = t1_build(t1, sdata,
						dad, T1_R_BLOCK | T1_OTHER_ERROR,
						NULL, NULL);
				continue;
			}

			if (t1->state == RECEIVING) {
				/* ISO 7816-3 Rule 7.2 */
				if (T1_R_BLOCK == t1_block_type(t1->previous_block[1]))
				{
					T1PRINTF("Rule 7.2\n");
					slen = t1_rebuild(t1, sdata);
					continue;
				}

				T1PRINTF("");
				slen = t1_build(t1, sdata,
						dad, T1_R_BLOCK,
						NULL, NULL);
				break;
			}

			/* If the card terminal requests the next
			 * sequence number, it received the previous
			 * block successfully */
			if (t1_seq(pcb) != t1->ns) {
				t1_buf_get(&sbuf, NULL, last_send);
				sent_length += last_send;
				last_send = 0;
				t1->ns ^= 1;
			}

			/* If there's no data available, the ICC
			 * shouldn't be asking for more */
			if (t1_buf_avail(&sbuf) == 0)
				goto resync;

			slen = t1_build(t1, sdata, dad, T1_I_BLOCK,
					&sbuf, &last_send);
			break;

		case T1_I_BLOCK:
			/* The first I-block sent by the ICC indicates
			 * the last block we sent was received successfully. */
			if (t1->state == SENDING) {
				T1PRINTF("");
				t1_buf_get(&sbuf, NULL, last_send);
				last_send = 0;
				t1->ns ^= 1;
			}

			t1->state = RECEIVING;

			/* If the block sent by the card doesn't match
			 * what we expected it to send, reply with
			 * an R block */
			if (t1_seq(pcb) != t1->nr) {
				T1PRINTF("wrong nr\n");
				slen = t1_build(t1, sdata, dad,
						T1_R_BLOCK | T1_OTHER_ERROR,
						NULL, NULL);
				continue;
			}

			t1->nr ^= 1;

			if (t1_buf_put(&rbuf, rdata + 3, rdata[LEN]) < 0)
			{
				T1PRINTF("buffer overrun by %d bytes\n", rdata[LEN] - (rbuf.size - rbuf.tail));
				goto error;
			}

			if ((pcb & T1_MORE_BLOCKS) == 0)
				goto done;

			slen = t1_build(t1, sdata, dad, T1_R_BLOCK, NULL, NULL);
			break;

		case T1_S_BLOCK:
			if (T1_S_IS_RESPONSE(pcb) && t1->state == RESYNCH) {
				/* ISO 7816-3 Rule 6.2 */
				T1PRINTF("S-Block answer received\n");
				/* ISO 7816-3 Rule 6.3 */
				t1->state = SENDING;
				sent_length =0;
				last_send = 0;
				resyncs = 3;
				retries = t1->retries;
				t1_buf_init(&rbuf, rcv_buf, rcv_len);
				slen = t1_build(t1, sdata, dad, T1_I_BLOCK,
						&sbuf, &last_send);
				continue;
			}

			if (T1_S_IS_RESPONSE(pcb))
			{
				/* ISO 7816-3 Rule 7.4.2 */
				if (retries == 0)
					goto resync;

				/* ISO 7816-3 Rule 7.2 */
				if (T1_R_BLOCK == t1_block_type(t1->previous_block[PCB]))
				{
					T1PRINTF("Rule 7.2\n");
					slen = t1_rebuild(t1, sdata);
					continue;
				}

				T1PRINTF("wrong response S-BLOCK received\n");
				slen = t1_build(t1, sdata,
						dad, T1_R_BLOCK | T1_OTHER_ERROR,
						NULL, NULL);
				continue;
			}

			t1_buf_init(&tbuf, sblk, sizeof(sblk));

			T1PRINTF("S-Block request received\n");
			switch (T1_S_TYPE(pcb)) {
			case T1_S_RESYNC:
				if (rdata[LEN] != 0)
				{
					T1PRINTF("Wrong length: %d\n", rdata[LEN]);
					slen = t1_build(t1, sdata, dad,
						T1_R_BLOCK | T1_OTHER_ERROR,
						NULL, NULL);
					continue;
				}

				T1PRINTF("Resync requested\n");
				/* the card is not allowed to send a resync. */
				goto resync;

			case T1_S_ABORT:
				if (rdata[LEN] != 0)
				{
					T1PRINTF("Wrong length: %d\n", rdata[LEN]);
					slen = t1_build(t1, sdata, dad,
						T1_R_BLOCK | T1_OTHER_ERROR,
						NULL, NULL);
					continue;
				}

				/* ISO 7816-3 Rule 9 */
				T1PRINTF("abort requested\n");
				goto resync;

			case T1_S_IFS:
				if (rdata[LEN] != 1)
				{
					T1PRINTF("Wrong length: %d\n", rdata[LEN]);
					slen = t1_build(t1, sdata, dad,
						T1_R_BLOCK | T1_OTHER_ERROR,
						NULL, NULL);
					continue;
				}

				T1PRINTF("sent S-block with ifs=%u\n", rdata[DATA]);
				if (rdata[DATA] == 0)
					goto resync;
				t1->ifsc = rdata[DATA];
				t1_buf_putc(&tbuf, rdata[DATA]);
				break;

			case T1_S_WTX:
				if (rdata[LEN] != 1)
				{
					T1PRINTF("Wrong length: %d\n", rdata[LEN]);
					slen = t1_build(t1, sdata, dad,
						T1_R_BLOCK | T1_OTHER_ERROR,
						NULL, NULL);
					continue;
				}

				T1PRINTF("sent S-block with wtx=%u\n", rdata[DATA]);
				t1->wtx = rdata[DATA];
				t1_buf_putc(&tbuf, rdata[DATA]);
				break;

			default:
				T1PRINTF("T=1: Unknown S block type 0x%02x\n", T1_S_TYPE(pcb));
				goto resync;
			}

			slen = t1_build(t1, sdata, dad,
				T1_S_BLOCK | T1_S_RESPONSE | T1_S_TYPE(pcb),
				&tbuf, NULL);
		}

		/* Everything went just splendid */
		retries = t1->retries;
		continue;

resync:
		/* the number or resyncs is limited, too */
		/* ISO 7816-3 Rule 6.4 */
		if (resyncs == 0)
			goto error;

		/* ISO 7816-3 Rule 6 */
		resyncs--;
		t1->ns = 0;
		t1->nr = 0;
		slen = t1_build(t1, sdata, dad, T1_S_BLOCK|T1_S_RESYNC, NULL,
				NULL);
		t1->state = RESYNCH;
		t1->more = FALSE;
		retries = 1;
		continue;
	}

done:
	return t1_buf_avail(&rbuf);

error:
	t1->state = DEAD;
	return -1;
}

void smc_dev_enable()
{
}

void smc_dev_disable()
{
}
void smc_write_tx(UINT32 io_base, UINT16 val)
{
	if(smc_rx_fifo_size>64)
		OUTPUT_UINT16(io_base + REG_TX_CNT, val);
	else
		OUTPUT_UINT8(io_base + REG_TX_CNT, ((UINT8)val));
}

UINT16 smc_read_tx(UINT32 io_base)
{
	UINT16 val; 
	if(smc_rx_fifo_size>64)
		val = INPUT_UINT16(io_base+ REG_TX_CNT);
	else
		val = INPUT_UINT8(io_base + REG_TX_CNT);
	return val;
}

void smc_write_rx(UINT32 io_base, UINT16 val)
{
	if(smc_rx_fifo_size>64)
		WRITE_RX_CNT(io_base, val);
	else
		OUTPUT_UINT8(io_base + REG_RFIFO_CNT, ((UINT8)val));
}

UINT16 smc_read_rx(UINT32 io_base)
{
	UINT16 val; 
	if(smc_rx_fifo_size>64)
		val = READ_RX_CNT(io_base);
	else
		val = INPUT_UINT8(io_base + REG_RFIFO_CNT);
	return val;
}

static void smc_dev_hsr_0(UINT32 param)
{
	struct smc_device *dev = smc_dev_set[0].dev;
	if(!dev)
		return;
	struct smartcard_private *tp = (struct smartcard_private *)(dev->priv);
	if(param&SMC_INSERTED)
		osal_flag_clear(tp->smc_flag_id, SMC_REMOVED);
	if(param&SMC_REMOVED)
		osal_flag_clear(tp->smc_flag_id, SMC_INSERTED);
	osal_flag_set(tp->smc_flag_id, param);
}

static void smc_dev_hsr_1(UINT32 param)
{
	struct smc_device *dev = smc_dev_set[1].dev;
	if(!dev)
		return;
	struct smartcard_private *tp = (struct smartcard_private *)(dev->priv);
	if(param&SMC_INSERTED)
		osal_flag_clear(tp->smc_flag_id, SMC_REMOVED);
	if(param&SMC_REMOVED)
		osal_flag_clear(tp->smc_flag_id, SMC_INSERTED);
	osal_flag_set(tp->smc_flag_id, param);
}

void smc_gpio_detect_lsr(UINT32 dev)
{
	struct smartcard_private *tp = (struct smartcard_private *)((struct smc_device *)dev)->priv;
	UINT32 base_addr = ((struct smc_device *)dev)->base_addr;
	
	if(HAL_GPIO_INT_STA_GET(tp->gpio_cd_pos)==0)
		return;	
	HAL_GPIO_INT_CLEAR(tp->gpio_cd_pos);
	if((HAL_GPIO_BIT_GET(tp->gpio_cd_pos)) == tp->gpio_cd_pol)
	{
		if(tp->inserted)
		{
			smc_dev_deactive((struct smc_device *)dev);
		}
	}
}

/******************************************************************************************************
 * 	Name		:	smc_dev_attach()
 *	Description	:	Smart card reader init funciton.
 *	Parameter	:	int dev_id		: Index of smart card slot.
 *	Return		:	INT32			: SUCCESS or FAIL.
 *
 ******************************************************************************************************/
INT32 smc_dev_attach(int dev_id, struct smc_dev_config * config_param)
{
	struct smc_device *dev;
	struct smartcard_private *tp;
	void *priv_mem;
	UINT8 dev_num;
	if(0x80000000!=(((UINT32)config_param)&0xf0000000)&&
		0xa0000000!=(((UINT32)config_param)&0xf0000000)&&
		0xb0000000!=(((UINT32)config_param)&0xf0000000))
		return !SUCCESS;
	smc_chip_id = sys_ic_get_chip_id();
	smc_chip_version = sys_ic_get_rev_id();

	if(sys_ic_is_M3101())
	{
		smc_chip_id = ALI_M3101;
	}
    if(smc_chip_id == ALI_S3602 && smc_chip_version >= IC_REV_6)
    {
        smc_chip_id = ALI_S3602F;
        //smc_chip_version = IC_REV_0;
    }

	if(dev_id == 0)
		smc_dev_name[STRLEN(smc_dev_name) - 1] = '0';
	else if(dev_id == 1)
	{
		if(ALI_M3329E==smc_chip_id)
		{
			if(smc_chip_version>=IC_REV_5)
				return ERR_FAILUE;
		}
		smc_dev_name[STRLEN(smc_dev_name) - 1] = '1';	
	}
	else
		return ERR_FAILUE;
		
	SMC_PRINTF("%s\n",smc_dev_name);
	

	if(ALI_S3602==smc_chip_id)
	{
		smc_tx_fifo_size = 8;
		smc_rx_fifo_size = 32;
	}
	if((ALI_M3101==smc_chip_id)||(ALI_M3329E==smc_chip_id && smc_chip_version>=IC_REV_5) || (ALI_S3602F==smc_chip_id))
	{
        smc_tx_fifo_size = smc_rx_fifo_size = 256;
	}
	dev = dev_alloc(smc_dev_name, HLD_DEV_TYPE_SMC,sizeof(struct smc_device));
	
	if (dev == NULL)
	{
		PRINTF("Error: Alloc smart card reader error!\n");
		return ERR_NO_MEM;
	}
	smc_dev_set[dev_id].dev = dev;
	/* Alloc structure space of private */
	priv_mem = (void *)MALLOC(sizeof(struct smartcard_private));

	ASSERT(priv_mem != NULL);
	MEMSET(priv_mem, 0, sizeof(struct smartcard_private));
	
	dev->priv = priv_mem;

	tp = (struct smartcard_private *)dev->priv;
	tp->atr_info = (atr_t *)MALLOC(sizeof(atr_t));
	ASSERT(tp->atr_info!=NULL);
	MEMSET(tp->atr_info, 0, sizeof(atr_t));
	tp->inserted = 0;
	tp->reseted = 0;
	tp->inverse_convention = 0;
	tp->the_last_send = 0;
	if(0==dev_id)
		tp->hsr = smc_dev_hsr_0;
	else
		tp->hsr = smc_dev_hsr_1;
	
	if(config_param->init_clk_trigger&&
		config_param->init_clk_number&&
		config_param->init_clk_array)
	{
		tp->init_clk_number = config_param->init_clk_number;
		tp->init_clk_array = (UINT32 *)MALLOC(tp->init_clk_number*sizeof(UINT32));
		ASSERT(NULL!=tp->init_clk_array);
		MEMCPY(tp->init_clk_array, config_param->init_clk_array, (tp->init_clk_number*sizeof(UINT32)));
	}
	else
	{
		tp->init_clk_number = 1;
		tp->init_clk_array = &(tp->smc_clock);
		tp->smc_clock = DFT_WORK_CLK;
	}
	if(config_param->class_selection_supported&&
		(config_param->board_supported_class&0x7)&&
		(NULL!=config_param->class_select))
	{
		tp->class_selection_supported = 1;
		tp->board_supported_class = (config_param->board_supported_class&0x7);
		tp->class_select = config_param->class_select;
	}
	if(config_param->gpio_cd_trigger)
	{
		tp->use_gpio_cd = 1;
		tp->gpio_cd_io = config_param->gpio_cd_io;
		tp->gpio_cd_pol = config_param->gpio_cd_pol;
		tp->gpio_cd_pos = config_param->gpio_cd_pos;
	}
	if(config_param->gpio_vpp_trigger)
	{
		tp->use_gpio_vpp = 1;
		if((ALI_M3101==smc_chip_id)||(ALI_M3329E==smc_chip_id && smc_chip_version>=IC_REV_3) || (ALI_S3602F==smc_chip_id))
		{
			tp->internal_ctrl_vpp = 1;
		}
		tp->gpio_vpp_io = config_param->gpio_vpp_io;
		tp->gpio_vpp_pol = config_param->gpio_vpp_pol;
		tp->gpio_vpp_pos = config_param->gpio_vpp_pos;
	}
	if(config_param->def_etu_trigger)
	{
		tp->use_default_etu = 1;
		tp->default_etu = config_param->default_etu;
	}
	
	//check HW auto TX/RX
	if(ALI_M3329E==smc_chip_id)
	{
		if(smc_chip_version>=IC_REV_2)
			tp->auto_tx_rx_triger = 1;
		else
			tp->force_tx_rx_triger = 1;
		
		if(smc_chip_version>=IC_REV_3)
			tp->ts_auto_detect = 1;
		
		if(smc_chip_version>=IC_REV_5)
		{
			tp->invert_power = config_param->invert_power;
			tp->invert_detect = config_param->invert_detect;
		}
	}
    else if((ALI_M3101==smc_chip_id)||(ALI_S3602F==smc_chip_id))
    {
        tp->invert_power = config_param->invert_power;
		tp->invert_detect = config_param->invert_detect;
		 tp->auto_tx_rx_triger = 1;
		 tp->ts_auto_detect = 1;
    }
    
	if(sys_ic_is_M3202())
	{
		if(smc_chip_version>=IC_REV_2)
		{
			tp->auto_tx_rx_triger = 1;
			tp->invert_power = config_param->invert_power;
			tp->invert_detect = config_param->invert_detect;
			if(smc_chip_version>IC_REV_2)
				 tp->ts_auto_detect = 1;
		}
		else
			tp->force_tx_rx_triger = 1;
	}
	if(ALI_S3602==smc_chip_id)
	{
		if(smc_chip_version>=IC_REV_2)
		{
			tp->auto_tx_rx_triger = 1;
		}
	}
	
	tp->parity_disable = config_param->parity_disable_trigger;
	tp->parity_odd = config_param->parity_odd_trigger;
	tp->apd_disable = config_param->apd_disable_trigger;
	tp->warm_reset_enable = config_param->warm_reset_trigger;
	tp->disable_pps = config_param->disable_pps;
	if(ALI_M3329E==smc_chip_id)
	{
		if(smc_chip_version>=IC_REV_5)
		{
			smc_dev_set[0].io_base = 0xb8001800;
			smc_dev_set[0].irq = 20;
			smc_dev_set[1].io_base = 0xb8001800;
			smc_dev_set[1].irq = 20;
			scr_sys_clk = pwm_sys_clk = 108000000;
		}
		else
		{
			smc_dev_set[0].io_base = 0xb8001800;
			smc_dev_set[0].irq = 14;
			smc_dev_set[0].pwm_addr = 0xb8001430;
			smc_dev_set[0].pwm_sel_ofst = 0x0;
			smc_dev_set[0].pwm_seh_ofst = 0x2;
			smc_dev_set[0].pwm_gpio_ofst = 0x4;
			smc_dev_set[0].pwm_cfg_ofst = 0x5;
			smc_dev_set[0].pwm_frac_ofst = 0x6;
			
			smc_dev_set[1].io_base = 0xb8001900;
			smc_dev_set[1].irq = 15;
			smc_dev_set[1].pwm_addr = 0xb8001460;
			smc_dev_set[1].pwm_sel_ofst = 0x0;
			smc_dev_set[1].pwm_seh_ofst = 0x2;
			smc_dev_set[1].pwm_gpio_ofst = 0x4;
			smc_dev_set[1].pwm_cfg_ofst = 0x5;
			smc_dev_set[1].pwm_frac_ofst = 0x6;
			if(!scr_sys_clk)
			{
				UINT32 tmp;
				tmp = ((*((volatile UINT32 *)0xb8000070))>>2)&0x3;
				if(tmp==0x0)
		 			pwm_sys_clk = 135000000;
		 		else if(tmp==0x1)
		 			pwm_sys_clk = 120000000;
		 		else if(tmp==0x2)
		 			pwm_sys_clk = 166000000;
		 		else
		 			pwm_sys_clk = 154000000;		
	    			scr_sys_clk = 108000000;
			}
		}
	}
	if(sys_ic_is_M3202()==1)
	{
		smc_dev_set[0].io_base = 0xb8001800;
		smc_dev_set[0].irq = 20;
		smc_dev_set[0].pwm_addr = 0xb8001a00;
		smc_dev_set[0].pwm_sel_ofst = 0x0;
		smc_dev_set[0].pwm_seh_ofst = 0x2;
		smc_dev_set[0].pwm_gpio_ofst = 0x6;
		smc_dev_set[0].pwm_cfg_ofst = 0x4;
		smc_dev_set[0].pwm_frac_ofst = 0x5;
		
		smc_dev_set[1].io_base = 0xb8001900;
		smc_dev_set[1].irq = 21;
		smc_dev_set[1].pwm_addr = 0xb8001b00;
		smc_dev_set[1].pwm_sel_ofst = 0x0;
		smc_dev_set[1].pwm_seh_ofst = 0x2;
		smc_dev_set[1].pwm_gpio_ofst = 0x6;
		smc_dev_set[1].pwm_cfg_ofst = 0x4;
		smc_dev_set[1].pwm_frac_ofst = 0x5;
		if(!scr_sys_clk)
		{
			UINT32 tmp;
			if(config_param->sys_clk_trigger)
			{
				scr_sys_clk = config_param->smc_sys_clk;
				if(scr_sys_clk == 166000000)
	 				tmp = 0x3;
	 			else if(scr_sys_clk == 135000000)
	 				tmp = 0x1;
	 			else if(scr_sys_clk == 154000000)
	 				tmp = 0x2;
	 			else
	 			{
	 				tmp = 0x0;	
	 				scr_sys_clk = 108000000;
	 			}
				*((volatile UINT8 *)0xb800007a) &= ~(0x3<<1);
				*((volatile UINT8 *)0xb800007a) |= tmp<<1;
			}
			else
			{
				tmp = ((*((volatile UINT32 *)0xb8000078))>>17)&0x3;
	 			if(tmp==0x0)
	 				scr_sys_clk = 108000000;
	 			else if(tmp==0x1)
	 				scr_sys_clk = 135000000;
	 			else if(tmp==0x2)
	 				scr_sys_clk = 154000000;
	 			else
	 				scr_sys_clk = 166000000;
			}
			pwm_sys_clk = scr_sys_clk;
		}
	}
	if((ALI_S3602==smc_chip_id)\
		||(ALI_S3602F==smc_chip_id))
	{
		if(!scr_sys_clk)
		{
			scr_sys_clk = 108000000;
			pwm_sys_clk = scr_sys_clk;
		}
	}
	if(ALI_M3101==smc_chip_id)
	{
		smc_dev_set[0].io_base = 0xb8001800;
		smc_dev_set[0].irq = 20;
		smc_dev_set[1].io_base = 0xb8001800;
		smc_dev_set[1].irq = 20;
		scr_sys_clk = pwm_sys_clk = 108000000;
	}
	dev->base_addr = smc_dev_set[dev_id].io_base;
	dev->irq = smc_dev_set[dev_id].irq;
	
	/* Function point init */
	dev->open = smc_dev_open;
	dev->close = smc_dev_close;
	dev->card_exist = smc_dev_card_exist;
	if(tp->class_selection_supported)
		dev->reset = smc_dev_multi_class_reset;
	else
		dev->reset = smc_dev_reset;
	dev->deactive = smc_dev_deactive;
	dev->raw_read = smc_dev_read;
	dev->raw_write = smc_dev_write;
	dev->raw_fifo_write = smc_dev_write;
	//dev->transmit = smc_dev_transfer_data;
	dev->iso_transfer = smc_dev_iso_transfer;
	dev->iso_transfer_t1 = smc_dev_iso_transfer_t1;
	dev->do_ioctl = smc_dev_ioctl;
	dev->t1_transfer = t1_transceive;
	dev->t1_xcv = t1_xcv;
	dev->t1_negociate_ifsd = t1_negociate_ifsd;

	/* Add this device to queue */
	if (dev_register(dev) != SUCCESS)
	{
		SMC_PRINTF("Error: Register smart card reader device error!\n");
		FREE(priv_mem);
		dev_free(dev);
	}
	SMC_PRINTF("attach ok\n");
	return SUCCESS;
}

/******************************************************************************************************
 * 	Name		:	smc_dev_interrupt()
 *	Description	:	smart card reader controler interrupt handle.
 *	Parameter	:	struct smc_device *dev	: Devcie handle.
 *	Return		:	none
 *
 ******************************************************************************************************/
static void smc_dev_interrupt(UINT32 param)
{
	struct smartcard_private *tp = (struct smartcard_private *)((struct smc_device *)param)->priv;
	UINT32 ioaddr = ((struct smc_device *)param)->base_addr;
	UINT8 isr0_status,isr1_status;
	UINT32 i;

	isr0_status = INPUT_UINT8(ioaddr + REG_ISR0);
	isr1_status = INPUT_UINT8(ioaddr + REG_ISR1);
	//SMC_PRINTF("isr0: %02x, isr1: %02x\n", isr0_status, isr1_status);
	tp->isr0_interrupt_status |= isr0_status;
	tp->isr1_interrupt_status |= isr1_status;

	OUTPUT_UINT8(ioaddr + REG_ISR0, isr0_status);
	OUTPUT_UINT8(ioaddr + REG_ISR1, isr1_status);
    
	if(isr0_status&SMC_ISR0_BYTE_RECV)
	{
		//SMC_PRINTF("byte : %d\n", smc_read_rx(ioaddr));
		if(tp->isr0_interrupt_status&SMC_ISR0_PE_RECV)
		{
            SMC_RW_PRINTF("RX PE ERR\n");
            if(smc_read_rx(ioaddr))
            {
                volatile UINT8 patch = 0;
                patch = INPUT_UINT8(ioaddr + REG_RBR);
                tp->isr0_interrupt_status &= (~SMC_ISR0_PE_RECV);
                SMC_RW_PRINTF("!!!\n");
            }
		}
		if((0!=tp->smc_rx_tail)&&(0==tp->got_first_byte))
		{
			tp->isr0_interrupt_status &= (~SMC_ISR0_BYTE_RECV);
			tp->got_first_byte = 1; 
			//OUTPUT_UINT8(ioaddr + REG_IER0,INPUT_UINT8(ioaddr + REG_IER0)&(~SMC_IER0_BYTE_RECV_TRIG));
			osal_interrupt_register_hsr(tp->hsr, SMC_RX_BYTE_RCV);
		}
		if(tp->smc_rx_buf==tp->smc_rx_tmp_buf)
		{
			UINT16 c;
			UINT32 rem_space = tp->smc_rx_tail - tp->smc_rx_head;
		
			c =smc_read_rx(ioaddr);
			//soc_printf("rcv %d bytes\n", c);
			c = (c<=rem_space?c:rem_space);			
			
			for(i=0;i<c;i++)
				tp->smc_rx_buf[tp->smc_rx_head+i] = INPUT_UINT8(ioaddr + REG_RBR);
				
			tp->smc_rx_head += c; 
			tp->isr0_interrupt_status &= (~SMC_ISR0_BYTE_RECV);
			//SMC_PRINTF("rcv %d bytes\n", tp->smc_rx_head);
		}
	}
	if((isr0_status&SMC_ISR0_FIFO_TRANS)&&(tp->force_tx_rx_triger||tp->auto_tx_rx_triger))
	{
        	tp->isr0_interrupt_status &= (~SMC_ISR0_FIFO_TRANS);
        	if(tp->smc_tx_wr)
        	{
			if(tp->smc_tx_rd==tp->smc_tx_wr)
			{
				//SMC_PRINTF("tx finish:\n");
				/*Once TX finished, set interface device to RX mode immediately*/
				if(tp->force_tx_rx_triger&&1==tp->force_tx_rx_state)
				{
					smc_write_tx(ioaddr, tp->force_tx_rx_thld);
					tp->force_tx_rx_state = 2;
					if(0==smc_read_tx(ioaddr))
					{	
						OUTPUT_UINT8(ioaddr + REG_SCR_CTRL,(INPUT_UINT8(ioaddr + REG_SCR_CTRL)&(~SMC_SCR_CTRL_TRANS))|SMC_SCR_CTRL_RECV);
						SMC_PRINTF("tmo2: too late!\n");
					}
					else
					{
						UINT32 force_loop_tmo;
						UINT8 force_tx_rx;
	
						force_tx_rx = 1;
					
						force_loop_tmo = (smc_read_tx(ioaddr)+1)*tp->char_frm_dura;
					
						UINT32 tmo = read_tsc();
						//if(0==smc_read_tx(ioaddr))libc_libc_printf("tmo2: too late!\n");
						
						while(0!=smc_read_tx(ioaddr))
						{
							osal_delay(1);
							force_loop_tmo--;
							if(!force_loop_tmo)
							{
								force_tx_rx = 0;
								SMC_PRINTF("tmo2: tmo %d\n", smc_read_tx(ioaddr));
								//libc_libc_printf("tmo2: tmo %d\n", smc_read_tx(ioaddr));
								break;
							}
							
							if((INPUT_UINT8(ioaddr + REG_ICCSR) & 0x80)== 0)
							{
								SMC_PRINTF("smc: smart card not inserted!\n");
								return ;
							}
							
						}
						if(force_tx_rx)
						{
							OUTPUT_UINT8(ioaddr + REG_SCR_CTRL,(INPUT_UINT8(ioaddr + REG_SCR_CTRL)&(~SMC_SCR_CTRL_TRANS))|SMC_SCR_CTRL_RECV);
						}
						SMC_PRINTF("tmo2: %d, %d\n", force_loop_tmo, (read_tsc()-tmo)/(SYS_CPU_CLOCK / 2000000));
						//libc_printf("tmo2: %d, %d\n", force_loop_tmo, (read_tsc()-tmo)/(SYS_CPU_CLOCK / 2000000));
					}
				}
				else
				{
					if(!(tp->auto_tx_rx_triger))
            				{	
            					OUTPUT_UINT8(ioaddr + REG_SCR_CTRL,(INPUT_UINT8(ioaddr + REG_SCR_CTRL)&(~SMC_SCR_CTRL_TRANS))|SMC_SCR_CTRL_RECV);		
					}
					osal_interrupt_register_hsr(tp->hsr, SMC_TX_FINISHED);
					if(ALI_S3602==smc_chip_id)
					{
						tp->the_last_send = 0;  
						tp->isr0_interrupt_status &= (~SMC_ISR0_FIFO_EMPTY);						
					}
				}
				
			}
			else
			{
				UINT32 size = tp->smc_tx_wr - tp->smc_tx_rd;
				//OUTPUT_UINT8(ioaddr + REG_SCR_CTRL,INPUT_UINT8(ioaddr + REG_SCR_CTRL)&(~SMC_SCR_CTRL_TRANS));
				if(size>((UINT32)(smc_tx_fifo_size>>1)))
				{	
					size = smc_tx_fifo_size>>1;
					if(ALI_S3602==smc_chip_id)
						OUTPUT_UINT8(ioaddr + REG_FIFO_CTRL,(INPUT_UINT8(ioaddr + REG_FIFO_CTRL)&0xf0)|(size));
					else
						smc_write_tx(ioaddr,size);
				}
				else
				{
					if(tp->force_tx_rx_triger)
					{
						if(0==smc_read_tx(ioaddr))
						{
							if(size>tp->force_tx_rx_thld)
							{	
								smc_write_tx(ioaddr, size - tp->force_tx_rx_thld);
								tp->force_tx_rx_state = 1;
							}
							else
							{	
								smc_write_tx(ioaddr, size);
								tp->force_tx_rx_state = 2;
							}
						}
						else
						{	
							smc_write_tx(ioaddr, (size+(smc_tx_fifo_size>>1)) - tp->force_tx_rx_thld);
							tp->force_tx_rx_state = 1;
						}
					}
					else
					{	
						if(ALI_S3602==smc_chip_id)
						{
							//OUTPUT_UINT8(ioaddr + REG_FIFO_CTRL,INPUT_UINT8(ioaddr + REG_FIFO_CTRL)|(size + (smc_tx_fifo_size>>1)));
		 		 			OUTPUT_UINT8(ioaddr + REG_FIFO_CTRL,(INPUT_UINT8(ioaddr + REG_FIFO_CTRL)&0xf0)|(size + (smc_tx_fifo_size>>1)));
							tp->the_last_send = 1;
							tp->isr0_interrupt_status &= (~SMC_ISR0_FIFO_EMPTY);
						}
						else
						{
							if(0==smc_read_tx(ioaddr))
								smc_write_tx(ioaddr,size);
							else	
								smc_write_tx(ioaddr,size+(smc_tx_fifo_size>>1));
						}
					}
				}
					
				for(i=0; i<size; i++)
				{
					if((tp->smc_tx_rd+i+1)==tp->smc_tx_wr)
					{
						if(tp->auto_tx_rx_triger)
						{
							OUTPUT_UINT8(ioaddr+REG_ICCSR, 1<<5); //tx->rx auto switch
							SMC_PRINTF("2: tx->rx auto: rd %d, cnt %d, wr %d\n", tp->smc_tx_rd, i, tp->smc_tx_wr);
						}
					}
					OUTPUT_UINT8(ioaddr + REG_THR, tp->smc_tx_buf[tp->smc_tx_rd+i]);
				}
				//OUTPUT_UINT8(ioaddr + REG_SCR_CTRL,INPUT_UINT8(ioaddr + REG_SCR_CTRL)|SMC_SCR_CTRL_TRANS);
				tp->smc_tx_rd += size;
				SMC_PRINTF("continue feed data %d \n", size);
				if(tp->smc_tx_rd==tp->smc_tx_wr&&
					1==tp->force_tx_rx_triger&&
					tp->force_tx_rx_state ==2)
				{
					UINT32 force_loop_tmo;
					UINT8 force_tx_rx;
	
					force_tx_rx = 1;
					
					force_loop_tmo = (smc_read_tx(ioaddr)+1)*tp->char_frm_dura;
					
					UINT32 tmo = read_tsc();
						//if(0==smc_read_tx(ioaddr ))libc_libc_printf("tmo2: too late!\n");
						
					while(0!=smc_read_tx(ioaddr))
					{
						osal_delay(1);
						force_loop_tmo--;
						if(!force_loop_tmo)
						{
							force_tx_rx = 0;
							SMC_PRINTF("tmo3: tmo %d\n", smc_read_tx(ioaddr));
							break;
						}
						if((INPUT_UINT8(ioaddr + REG_ICCSR) & 0x80)== 0)
						{
							SMC_PRINTF("smc: smart card not inserted!\n");
							return ;
						}
					}
					
					if(force_tx_rx)
					{
						OUTPUT_UINT8(ioaddr + REG_SCR_CTRL,(INPUT_UINT8(ioaddr + REG_SCR_CTRL)&(~SMC_SCR_CTRL_TRANS))|SMC_SCR_CTRL_RECV);
					}
					SMC_PRINTF("tmo2: %d, %d\n", force_loop_tmo, (read_tsc()-tmo)/(SYS_CPU_CLOCK / 2000000));
				}
			}
        	}
	}
	if(isr0_status& SMC_ISR0_FIFO_RECV)
	{
		UINT16 c;
		UINT32 rem_space = tp->smc_rx_tail - tp->smc_rx_head;
		tp->isr0_interrupt_status &= (~SMC_ISR0_FIFO_RECV);
		if(tp->smc_rx_tail&&tp->smc_rx_buf!=tp->smc_rx_tmp_buf)
		{
			c =smc_read_rx(ioaddr);
			c = (c<=rem_space?c:rem_space);			
		
			for(i=0;i<c;i++)
				tp->smc_rx_buf[tp->smc_rx_head+i] = INPUT_UINT8(ioaddr + REG_RBR);
			tp->smc_rx_head += c; 
	
			if(tp->smc_rx_head == tp->smc_rx_tail)
			{
				//SMC_PRINTF("Notify rx over: %d\n", tp->smc_rx_tail);
				//tp->smc_flag_ptn = SMC_RX_FINISHED;
				osal_interrupt_register_hsr(tp->hsr, SMC_RX_FINISHED);
			}
			else
			{
				rem_space = tp->smc_rx_tail - tp->smc_rx_head;    
				if(rem_space/smc_rx_fifo_size)
					smc_write_rx(ioaddr,32);
				else
					smc_write_rx(ioaddr,rem_space);	
				SMC_PRINTF("continue rx %d data\n", rem_space);		
			}
		}
	}
	if((ALI_S3602==smc_chip_id)&&(isr0_status& SMC_ISR0_FIFO_EMPTY)&&(1==tp->the_last_send))
	{
		if(!(tp->auto_tx_rx_triger))
            	{	
            		OUTPUT_UINT8(ioaddr + REG_SCR_CTRL,(INPUT_UINT8(ioaddr + REG_SCR_CTRL)&(~SMC_SCR_CTRL_TRANS))|SMC_SCR_CTRL_RECV);		
		}
		osal_interrupt_register_hsr(tp->hsr, SMC_TX_FINISHED);
		tp->the_last_send = 0;
		tp->isr0_interrupt_status &= (~SMC_ISR0_FIFO_EMPTY);						
	}
	 
	if((isr1_status & SMC_ISR1_CARD_INSERT) != 0)
	{
		tp->inserted = 1;
		tp->atr_rlt = SMC_ATR_NONE;
		tp->reseted = 0;
		osal_interrupt_register_hsr(tp->hsr, SMC_INSERTED);
		SMC_PRINTF("smart card inserted!\n");
       
	}
	else if((isr1_status & SMC_ISR1_CARD_REMOVE) != 0)
	{
		tp->inserted = 0;
		tp->reseted = 0;
		tp->atr_rlt = SMC_ATR_NONE;
		tp->smc_supported_class = 0;
		tp->smc_current_select = SMC_CLASS_NONE_SELECT;
		//smc_dev_deactive((struct smc_device *)param);
		tp->reseted = 0;
		if(tp->class_selection_supported)
		{
			INT32 cls;
			for(cls = 0; cls<3; cls++)
				if(tp->board_supported_class&(1<<cls))
				{
					SMC_PRINTF("smc remove: set class to %c\n", ('A'+cls));
					tp->class_select((enum class_selection)(SMC_CLASS_NONE_SELECT+cls+1));
					break;
				}
		}
		smc_init_hw((struct smc_device *)param);
		osal_interrupt_register_hsr(tp->hsr, SMC_REMOVED);
		SMC_PRINTF("smart card removed!\n");	
       
	}
	else
	{
		return;
	}
	
	if (((struct smc_device *)param)->callback != NULL)
	{
		osal_interrupt_register_hsr((OSAL_T_HSR_PROC_FUNC_PTR)(((struct smc_device *)param)->callback), tp->inserted);
	}
	return;
}

static void smc_set_wclk(UINT32 ioaddr, UINT32 clk)
{
	UINT32 scr_div_inte, pwm_div_inte;
	UINT32 scr_div_fract, pwm_div_fract;
	UINT32 dev_id;
	double db_temp;
	if(ioaddr==smc_dev_set[0].io_base)
		dev_id = 0;
	else
		dev_id = 1;
	db_temp = ((double)scr_sys_clk)/((double)clk);
	scr_div_inte = (UINT32)(db_temp*100);
	scr_div_fract = scr_div_inte%100;
	scr_div_inte = scr_div_inte/100;
	if(scr_div_fract)
	{
		UINT8 fract;
		db_temp = ((double)pwm_sys_clk)/((double)clk);
		pwm_div_inte = (UINT32)(db_temp*100);
		pwm_div_fract = pwm_div_inte%100;
		pwm_div_inte = pwm_div_inte/100;
		fract = 100/pwm_div_fract;
		
		if(ALI_M3329E==smc_chip_id)
		{
			OUTPUT_UINT8(smc_dev_set[dev_id].pwm_addr+smc_dev_set[dev_id].pwm_gpio_ofst, 0);	
		}
		OUTPUT_UINT8(ioaddr + REG_CLKH_SEH, scr_div_inte>>1); 	
		OUTPUT_UINT8(ioaddr + REG_CLKL_SEL, (scr_div_inte>>1)+(scr_div_inte&0x1));	
		OUTPUT_UINT16(smc_dev_set[dev_id].pwm_addr+smc_dev_set[dev_id].pwm_seh_ofst, pwm_div_inte>>1);
		OUTPUT_UINT16(smc_dev_set[dev_id].pwm_addr+smc_dev_set[dev_id].pwm_sel_ofst, (pwm_div_inte>>1)+(pwm_div_inte&0x1));
		OUTPUT_UINT8(smc_dev_set[dev_id].pwm_addr+smc_dev_set[dev_id].pwm_frac_ofst, fract);	
		OUTPUT_UINT8(smc_dev_set[dev_id].pwm_addr+smc_dev_set[dev_id].pwm_cfg_ofst, 0x81);	
		if((ALI_M3329E==smc_chip_id&&smc_chip_version>=IC_REV_5)\
			|| (ALI_S3602F==smc_chip_id) || (ALI_M3101==smc_chip_id)\
			||(sys_ic_is_M3202()&&smc_chip_version>IC_REV_2))
			OUTPUT_UINT8(ioaddr + REG_CLK_FRAC, fract); 	
	}
	else
	{
		if(ALI_M3329E==smc_chip_id)
		{
			OUTPUT_UINT8(smc_dev_set[dev_id].pwm_addr+smc_dev_set[dev_id].pwm_gpio_ofst, 0);	
		}
		OUTPUT_UINT8(smc_dev_set[dev_id].pwm_addr+smc_dev_set[dev_id].pwm_cfg_ofst, 0);	
		OUTPUT_UINT8(ioaddr + REG_CLKH_SEH, scr_div_inte>>1); 	
		OUTPUT_UINT8(ioaddr + REG_CLKL_SEL, (scr_div_inte>>1)+(scr_div_inte&0x1));		
	}
	
}

static void smc_init_hw(struct smc_device *dev)
{
	UINT32 rst_msk = 0;
	UINT32 sys_rst_addr = 0xb8000000;
	UINT32 ioaddr = dev->base_addr;
	UINT32 i=0;
	struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
	
	if(ALI_M3329E==smc_chip_id)
	{
		if(dev->base_addr==smc_dev_set[0].io_base)
			rst_msk = 1<<23;
		else
			rst_msk = 1<<24;
		sys_rst_addr = 0xb8000060;
	}
	if(sys_ic_is_M3202()==1)
	{
		if(dev->base_addr==smc_dev_set[0].io_base)
			rst_msk = 1<<15;
		else
			rst_msk = 1<<28;
		sys_rst_addr = 0xb8000060;
	}
    if((ALI_S3602==smc_chip_id) || (ALI_S3602F==smc_chip_id))
	{
		if(dev->base_addr==smc_dev_set[0].io_base)
			rst_msk = 1<<20;
		else
			rst_msk = 1<<21;
		sys_rst_addr = 0xb8000080;
	}
	if((ALI_M3101==smc_chip_id))
	{
		rst_msk = 1<<15;
		sys_rst_addr = 0xb8000060;
	}
	*((volatile UINT32 *)sys_rst_addr) |= rst_msk;
	osal_delay(3);
	*((volatile UINT32 *)sys_rst_addr) &= ~rst_msk;
	tp->smc_clock = DFT_WORK_CLK;     //Set default clk, is necessary.
	tp->smc_clock = tp->init_clk_array[0];
	if(tp->use_default_etu)
		tp->smc_etu = tp->default_etu;
	else
		tp->smc_etu = DFT_WORK_ETU;
	tp->inverse_convention = 0;
	SMC_PRINTF("init smc regiseter\n");
	if(tp->ts_auto_detect)
	{
		OUTPUT_UINT8(ioaddr + REG_DEV_CTRL, 0x10|(tp->invert_detect<<5)|(tp->invert_power<<6));//enable TS auto detecting.
	}
	OUTPUT_UINT8(ioaddr + REG_SCR_CTRL, 0x00);
	//osal_delay(2000);
	OUTPUT_UINT8(ioaddr + REG_SCR_CTRL, 0x80|(tp->parity_odd<<4)); //enable SCR interface
	if(tp->apd_disable/*ALI_S3602==smc_chip_id*/)
		OUTPUT_UINT8(ioaddr + REG_ICCR, 0x1|(tp->parity_disable<<5)|(tp->apd_disable<<4));	
	else	
		OUTPUT_UINT8(ioaddr + REG_ICCR, 0x41|(tp->parity_disable<<5)|(tp->apd_disable<<4));	 // power off
 	if(tp->use_gpio_vpp&&tp->internal_ctrl_vpp)
 	{
 		UINT8 temp_val =  INPUT_UINT8(ioaddr + REG_CLK_VPP);
 		temp_val &= 0xf3;
 		temp_val |= (tp->gpio_vpp_pol<<3);
 		if((ALI_S3602F==smc_chip_id)\
			||(ALI_M3101==smc_chip_id))
		{
		/*
		//s3602f SmartCard interface auto disable clock function has problem 
		while meet the smart card without parity bit such as Irdeto.
		need disable this function for s3602f
		*/
			temp_val |= 0x10;
		}		
 		OUTPUT_UINT8(ioaddr + REG_CLK_VPP, temp_val);
 		OUTPUT_UINT8(ioaddr + REG_VPP_GPIO, (tp->gpio_vpp_pos&0x3f)|0x80);
 	}
	if(sys_ic_is_M3202()&&(smc_chip_version==IC_REV_2))
	{
		*((volatile UINT8 *)0xb800001d) |= tp->invert_power<<7; 
		*((volatile UINT8 *)0xb800001d) |= tp->invert_detect <<6; 	
	}
	
	
	//enable interrupt
	OUTPUT_UINT8(ioaddr + REG_IER0, 0x7f);		//enable receive interrupt, enable wait timeout interrupt
	OUTPUT_UINT8(ioaddr + REG_IER1, 0xff);		//detect Card inserting or removal interrupt enable
	i =0;
	while(tp->invert_detect)
	{
		if(INPUT_UINT8(ioaddr + REG_ISR1) & (SMC_ISR1_CARD_REMOVE|SMC_ISR1_CARD_INSERT))
		{	
			SMC_PRINTF("i %d: %02x, isr0: %02x, isr1: %02x\n", i, INPUT_UINT8(ioaddr + REG_DEV_CTRL),INPUT_UINT8(ioaddr + REG_ISR0), INPUT_UINT8(ioaddr + REG_ISR1));
			OUTPUT_UINT8(ioaddr + REG_ISR1, SMC_ISR1_CARD_REMOVE|SMC_ISR1_CARD_INSERT);
			break;
		}
		osal_delay(1);
		i++;
		if(i>2000)
			break;
	}
	OUTPUT_UINT8(ioaddr + REG_PDBR, 15);		//set de-bounce to 15
	OUTPUT_UINT16(ioaddr + REG_ETU0, tp->smc_etu);		//set etu
	OUTPUT_UINT16(ioaddr + REG_GTR0, 12);//12		//set gtr to 12
	OUTPUT_UINT32(ioaddr + REG_CBWTR0, 12800);//9600	//set wt to 9600
	OUTPUT_UINT8(ioaddr + REG_RCVPR, 0x49);		//set value1_r to 4 and glitch_v to 3
	OUTPUT_UINT8(ioaddr + REG_RXTX_PP, 0x71);	//set rxpp to 3 and txpp to 3
	
	smc_set_wclk(ioaddr, tp->smc_clock);
}
/******************************************************************************************************
 * 	Name		:	smc_dev_open()
 *	Description	:	Smart card reader open funciton.
 *	Parameter	:	struct smc_device *dev		: Devcie handle.
 *				void (*callback)		: Callback function.
 *	Return		:	INT32				: SUCCESS.
 *
 ******************************************************************************************************/

static INT32 smc_dev_open(struct smc_device *dev, void (*callback)(UINT32 param))
{
	struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
	UINT32 ioaddr = dev->base_addr;
	UINT8 status;
	UINT32 temp;


	SMC_PRINTF("create smc semaphore\n");
	tp->smc_sema_id = OSAL_INVALID_ID;
	tp->smc_sema_id = osal_semaphore_create(1);
	if(OSAL_INVALID_ID == tp->smc_sema_id)
		ASSERT(0);
	tp->smc_flag_id = OSAL_INVALID_ID;
	tp->smc_flag_id = osal_flag_create(0);
	if(OSAL_INVALID_ID == tp->smc_flag_id)
		ASSERT(0);
	//tp->smc_tx_buf = (UINT8 *)MALLOC(SMC_TX_BUF_SIZE);
	//	ASSERT(tp->smc_tx_buf!=NULL);
	tp->smc_rx_tmp_buf = (UINT8 *)MALLOC(SMC_RX_BUF_SIZE);
		ASSERT(tp->smc_rx_tmp_buf!=NULL);
	tp->smc_rx_tmp_buf = 	(UINT8 *)((((UINT32)(tp->smc_rx_tmp_buf))&0xfffffff)|0xa0000000); 
	dev->callback = callback;
	if(tp->use_gpio_vpp)
	{
		HAL_GPIO_BIT_SET(tp->gpio_vpp_pos, tp->gpio_vpp_pol);
		HAL_GPIO_BIT_DIR_SET(tp->gpio_vpp_pos, tp->gpio_vpp_io);
	}
	//when open the card, init the T1 parameter
	t1_init(&tp->T1);
	smc_dev_deactive(dev);
	smc_init_hw(dev);
	if(tp->use_gpio_cd)
	{
		UINT32 gpio_int_irq;
		HAL_GPIO_INT_SET(tp->gpio_cd_pos, 1);
		HAL_GPIO_INT_REDG_SET(tp->gpio_cd_pos, 1);
		HAL_GPIO_INT_FEDG_SET(tp->gpio_cd_pos, 1);
		HAL_GPIO_INT_CLEAR(tp->gpio_cd_pos);
		if(ALI_S3602==smc_chip_id)
			gpio_int_irq = 8;
		else
		{
			if(tp->gpio_cd_pos < 32)
				gpio_int_irq = 9;
			else
			{
				if(ALI_M3329E==smc_chip_id)
				{
					if(tp->gpio_cd_pos<64)
						gpio_int_irq = 24;
					else
						gpio_int_irq = 31;
				}
				else
					gpio_int_irq = 24;
			}
		}
		HAL_GPIO_BIT_DIR_SET(tp->gpio_cd_pos,  tp->gpio_cd_io);
		osal_interrupt_register_lsr(gpio_int_irq, smc_gpio_detect_lsr, (UINT32)dev);	
	}
	//else
	{
		status = INPUT_UINT8(ioaddr + REG_ICCSR);
	
		SMC_PRINTF("to check card insert 0x80 : %x\n",status);
		if((status & 0x80) != 0)
		{
			SMC_PRINTF("card insert\n");
			tp->inserted = 1;
			dev->flags |= (HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);
			OUTPUT_UINT8(ioaddr + REG_ISR1, SMC_ISR1_CARD_INSERT);
			if(dev->callback != NULL)
			{
				osal_interrupt_register_hsr((OSAL_T_HSR_PROC_FUNC_PTR)(dev->callback), tp->inserted);
			}
		}
	}
	SMC_PRINTF("register interrupt server\n");
	osal_interrupt_register_lsr(dev->irq + 8, smc_dev_interrupt, (UINT32)dev);

	SMC_PRINTF("smc dev open ok\n");

	return RET_SUCCESS;
}

/******************************************************************************************************
 * 	Name		:	smc_dev_close()
 *	Description	:	Smart card reader controler close funciton.
 *	Parameter	:	struct smc_device *dev		: Devcie handle
 *	Return		:	INT32				: SUCCESS.
 *
 ******************************************************************************************************/
static INT32 smc_dev_close(struct smc_device *dev)
{
	UINT8 tmp;
	struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
	UINT32 ioaddr = dev->base_addr;
	tp->inserted = 0;
	tp->reseted = 0;

	/* Disable interrupt */
	OUTPUT_UINT8(ioaddr + REG_IER0, 0x00);
	OUTPUT_UINT8(ioaddr + REG_IER1, 0x00);
	osal_interrupt_unregister_lsr(dev->irq+ 8, smc_dev_interrupt);
	smc_dev_deactive(dev);
	osal_semaphore_delete(((struct smartcard_private *)dev->priv)->smc_sema_id);

	return RET_SUCCESS;
}

/******************************************************************************************************
 * 	Name		:	smc_dev_card_exist()
 *	Description	:	Smart card reader controler close funciton.
 *	Parameter	:	struct smc_device *dev		: Devcie handle.
 *	Return		:	INT32				: SUCCESS or FAIL.
 *
 ******************************************************************************************************/
static INT32 smc_dev_card_exist(struct smc_device *dev)
{
	struct smartcard_private *tp = (struct smartcard_private *)dev->priv;

	if(tp->inserted)
		return SUCCESS;
	else
		return !SUCCESS;
}


static void invert(UINT8 *data, INT32 n)
{
	INT32 i;
	static UINT8 swaptab[16] = {15, 7, 11, 3, 13, 5, 9, 1, 14, 6, 10, 2, 12, 4, 8, 0};
	for(i=n-1; i>=0; i--)
		data[i] = (swaptab[data[i]&0x0F]<<4) | swaptab[data[i]>>4];
}



//atr_t global_atr;

/******************************************************************************************************
 * 	Name		:	atr_config_parameter()
 *	Description	:	Initilize ATR structure.
 *	Parameter	:	atr_t atr		: Pointer to point ATR structure.
 *				UINT8 *buffer		: ATR buffer.
 *				UINT8 length		: ATR size.
 *	Return		:	INT32			: SUCCESS or FAIL.
 *
 ******************************************************************************************************/
static UINT32 atr_init(atr_t * atr, UINT8 *buffer, UINT8 length)
{
	UINT8 TDi;
	UINT8 pointer = 0, pn = 0;
	UINT8 i;
	
	/* Check size of buffer */
	if (length < 2)
		return SMART_WRONG_ATR;
	
	/* Store T0 and TS */
	atr->TS = buffer[0];

	atr->T0 = TDi = buffer[1];
	pointer = 1;

	/* Store number of historical bytes */
	atr->hbn = TDi & 0x0F;

	/* TCK is not present by default */
	(atr->TCK).present = FALSE;

	/* Extract interface bytes */
	while (pointer < length)
	{
		/* Check buffer is long enought */
		if (pointer + atr_num_ib_table[(0xF0 & TDi) >> 4] >= length)
		{
			return SMART_WRONG_ATR;
		}
		/* Check TAi is present */
		if ((TDi | 0xEF) == 0xFF)
		{
			pointer++;
			atr->ib[pn][ATR_INTERFACE_BYTE_TA].value = buffer[pointer];
			atr->ib[pn][ATR_INTERFACE_BYTE_TA].present = TRUE;
		}
		else
			atr->ib[pn][ATR_INTERFACE_BYTE_TA].present = FALSE;
		/* Check TBi is present */
		if ((TDi | 0xDF) == 0xFF)
		{
			pointer++;
			atr->ib[pn][ATR_INTERFACE_BYTE_TB].value = buffer[pointer];
			atr->ib[pn][ATR_INTERFACE_BYTE_TB].present = TRUE;
		}
		else
			atr->ib[pn][ATR_INTERFACE_BYTE_TB].present = FALSE;

		/* Check TCi is present */
		if ((TDi | 0xBF) == 0xFF)
		{
			pointer++;
			atr->ib[pn][ATR_INTERFACE_BYTE_TC].value = buffer[pointer];
			atr->ib[pn][ATR_INTERFACE_BYTE_TC].present = TRUE;
		}
		else
			atr->ib[pn][ATR_INTERFACE_BYTE_TC].present = FALSE;

		/* Read TDi if present */
		if ((TDi | 0x7F) == 0xFF)
		{
			pointer++;
			TDi = atr->ib[pn][ATR_INTERFACE_BYTE_TD].value = buffer[pointer];
			atr->ib[pn][ATR_INTERFACE_BYTE_TD].present = TRUE;
			(atr->TCK).present = ((TDi & 0x0F) != ATR_PROTOCOL_TYPE_T0);
			if (pn >= ATR_MAX_PROTOCOLS)
				return SMART_WRONG_ATR;
			pn++;
		}
		else
		{
			atr->ib[pn][ATR_INTERFACE_BYTE_TD].present = FALSE;
			break;
		}
	}

	/* Store number of protocols */
	atr->pn = pn + 1;

	/* Store historical bytes */
	if (pointer + atr->hbn >= length)
		return SMART_WRONG_ATR;

//	memcpy (atr->hb, buffer + pointer + 1, atr->hbn);
	for(i=0; i<atr->hbn; i++)
		*(atr->hb+i) = *(buffer+pointer+1+i);
	pointer += (atr->hbn);

	/* Store TCK  */
	if ((atr->TCK).present)
	{

		if (pointer + 1 >= length)
		return SMART_WRONG_ATR;

		pointer++;

		(atr->TCK).value = buffer[pointer];
	}

	atr->length = pointer + 1;
	return SMART_NO_ERROR;
}

/******************************************************************************************************
 * 	Name		:	atr_config_parameter()
 *	Description	:	By analyze ATR data to get the card information.
 *	Parameter	:	struct smartcard_private *handler	: Private structure handle.
 *				atr_t atr				: Pointer to point ATR buffer.
 *	Return		:	INT32					: SUCCESS or FAIL.
 *
 ******************************************************************************************************/

static UINT32 atr_config_parameter(struct smartcard_private *handler, atr_t *atr)
{
	UINT8 i,checksum=0;
	UINT8 FI, DI, II, PI1, PI2, N, WI;
	UINT32 F, D, I, P;
	UINT8 T = 0xFF;
	UINT16 rcnt_etu;
	UINT16 rcnt_3etu;
	UINT16 etu;
	UINT8 TA_AFTER_T15 = 0;
	UINT8 find_T15 = 0;
	if (atr->ib[0][ATR_INTERFACE_BYTE_TA].present)
	{
		FI = (atr->ib[0][ATR_INTERFACE_BYTE_TA].value & 0xF0) >> 4;
		F = atr_f_table[FI];
		//robbin change
		if(F == F_RFU)
			F = ATR_DEFAULT_F;
		SMC_RW_PRINTF("ATR: Clock Rate Conversion F=%d, FI=%d\n", F, FI);
	}
	else
	{
		F = ATR_DEFAULT_F;
		SMC_RW_PRINTF("ATR: Clock Rate Conversion F=(Default)%d\n", F);
	}
	handler->F = F;
	
	if (atr->ib[0][ATR_INTERFACE_BYTE_TA].present)
	{
		DI = (atr->ib[0][ATR_INTERFACE_BYTE_TA].value & 0x0F);
		D = atr_d_table[DI];
		if(D == D_RFU)
			D = ATR_DEFAULT_D;
		SMC_RW_PRINTF("ATR: Bit Rate Adjustment Factor D=%d, DI=%d\n", D, DI);
	}
	else
	{
		D = ATR_DEFAULT_D;
		SMC_RW_PRINTF("ATR: Bit Rate Adjustment Factor D=(Default)%d\n", D);
	}
	handler->D = D;

	if (atr->ib[0][ATR_INTERFACE_BYTE_TB].present)
	{
		II = (atr->ib[0][ATR_INTERFACE_BYTE_TB].value & 0x60) >> 5;
		I = atr_i_table[II];
		if(I == I_RFU)
			I = ATR_DEFAULT_I;
		SMC_RW_PRINTF("ATR: Programming Current Factor I=%d, II=%d\n", I, II);
	}
	else
	{
		I= ATR_DEFAULT_I;
		SMC_RW_PRINTF("ATR: Programming Current Factor I=(Default)%d\n", I);	
	}
	handler->I = I;
	
	if (atr->ib[1][ATR_INTERFACE_BYTE_TB].present)
	{
		PI2 = atr->ib[1][ATR_INTERFACE_BYTE_TB].value;
		P = PI2;
		SMC_RW_PRINTF("ATR: Programming Voltage Factor P=%d, PI2=%d\n", P, PI2);
	}
	else if (atr->ib[0][ATR_INTERFACE_BYTE_TB].present)
	{
		PI1 = (atr->ib[0][ATR_INTERFACE_BYTE_TB].value & 0x1F);
		P = PI1;
		SMC_RW_PRINTF("ATR: Programming Voltage Factor P=%d, PI1=%d\n", P, PI1);
	}
	else
	{
		P = ATR_DEFAULT_P;
		SMC_RW_PRINTF("ATR: Programming Voltage Factor P=(Default)%d\n", P);
	}
	handler->P = P;
	
	if (atr->ib[0][ATR_INTERFACE_BYTE_TC].present)
	{
		N = atr->ib[0][ATR_INTERFACE_BYTE_TC].value;
		/*if(N == 0xFF)
			N = 11;
		*/
	}
	else
		N = ATR_DEFAULT_N;
	SMC_RW_PRINTF("ATR: Extra Guardtime N=%d\n", N);
	handler->N = N;
	
	//for (i=0; i<ATR_MAX_PROTOCOLS; i++)
	//robbin change
	for (i=0; i<atr->pn; i++)
		if (atr->ib[i][ATR_INTERFACE_BYTE_TD].present && (0xFF == T))
		{
			/* set to the first protocol byte found */
			T = atr->ib[i][ATR_INTERFACE_BYTE_TD].value & 0x0F;
			SMC_RW_PRINTF("ATR: default protocol: T=%d\n", T);
		}
	//Try to find 1st TA after T = 15
	for (i=0; i<atr->pn; i++)
	{
		if (atr->ib[i][ATR_INTERFACE_BYTE_TD].present)
		{
			if((!find_T15)&&(0xF== (atr->ib[i][ATR_INTERFACE_BYTE_TD].value & 0x0F)))
			{
				find_T15 = 1;
				SMC_RW_PRINTF("Find T==15 at TD%d\n", i+1);
				continue;
			}
		}
		if ((find_T15)&&(atr->ib[i][ATR_INTERFACE_BYTE_TA].present))
		{
			TA_AFTER_T15 = atr->ib[i][ATR_INTERFACE_BYTE_TA].value;
			SMC_RW_PRINTF("Find 1st TA after T==15 at TA%d, value %02x\n", (i+1), TA_AFTER_T15);
			break;
		}
	}
	if(handler->class_selection_supported)
	{
		if(0==(TA_AFTER_T15&0x3f))
		{
			SMC_RW_PRINTF("SMC has no class indicator!\n");
			handler->smc_supported_class = 0x0;
		}
		else
			handler->smc_supported_class = TA_AFTER_T15&0x3f;	
	}
	//if has TA2, it indicate the special protocal
	if (atr->ib[1][ATR_INTERFACE_BYTE_TA].present)
	{
		T = atr->ib[1][ATR_INTERFACE_BYTE_TA].value & 0x0F;
		SMC_RW_PRINTF("ATR: specific mode found: T=%d\n", T);
		if (atr->ib[1][ATR_INTERFACE_BYTE_TA].value & 0x10) //check TA(2), bit 5
			handler->TA2_spec = 0;        //use the default value of F/D
		else
			handler->TA2_spec = 1;         //Use the value specified in the ATR 
	}
	
	if (0xFF == T)
	{
		SMC_RW_PRINTF("ATR: no default protocol found in ATR. Using T=0\n");
		T = ATR_PROTOCOL_TYPE_T0;
	}
	handler->T = T;

	if(handler->D != 0)
		handler->smc_etu = handler->F/handler->D;
	handler->first_cwt = FIRST_CWT_VAL; 
	handler->cwt = CWT_VAL;
	if(0 == T)
	{
		if (atr->ib[2][ATR_INTERFACE_BYTE_TC].present)
		{
			WI = atr->ib[2][ATR_INTERFACE_BYTE_TC].value;
		}
		else
			WI = ATR_DEFAULT_WI;
		SMC_RW_PRINTF("ATR: Work Waiting Time WI=%d\n", WI);
		handler->WI = WI;
		
		handler->first_cwt = handler->cwt = (960*WI*handler->F)/(handler->smc_clock/1000)+1;
	}
	else if(1 == T)
	{
		for (i = 1 ; i < atr->pn ; i++) 
    		{
      		  /* check for the first occurance of T=1 in TDi */
       		 if (atr->ib[i][ATR_INTERFACE_BYTE_TD].present && 
            			(atr->ib[i][ATR_INTERFACE_BYTE_TD].value & 0x0F) == ATR_PROTOCOL_TYPE_T1) 
        		{
         			   /* check if ifsc exist */
           			 if (atr->ib[i + 1][ATR_INTERFACE_BYTE_TA].present)
           			 	handler->T1.ifsc = atr->ib[i + 1][ATR_INTERFACE_BYTE_TA].value;
				else
					handler->T1.ifsc = ATR_DEFAULT_IFSC; /* default 32*/
                
           			 /* Get CWI */
           			 if (atr->ib[i + 1][ATR_INTERFACE_BYTE_TB].present)
            			    	handler->CWI = atr->ib[i + 1][ATR_INTERFACE_BYTE_TB].value & 0x0F;
           			 else
                			handler->CWI = ATR_DEFAULT_CWI; /* default 13*/
                		handler->cwt =  (((1<<(handler->CWI))+11 )*handler->smc_etu)/(handler->smc_clock/1000) + 1; 
           			 /*Get BWI*/
           			 if (atr->ib[i + 1][ATR_INTERFACE_BYTE_TB].present)
            				handler->BWI = (atr->ib[i + 1][ATR_INTERFACE_BYTE_TB].value & 0xF0) >> 4;
            			else
              			handler->BWI = ATR_DEFAULT_BWI; /* default 4*/    
				//handler->first_cwt= (((1<<(handler->BWI))*960+11)*handler->smc_etu)/(handler->smc_clock/1000); 		
                		handler->first_cwt = (11*handler->smc_etu)/(handler->smc_clock/1000)+((1<<(handler->BWI))*960*ATR_DEFAULT_F)/(handler->smc_clock/1000) + 2;	
            			if (atr->ib[i + 1][ATR_INTERFACE_BYTE_TC].present)
                			checksum = atr->ib[i + 1][ATR_INTERFACE_BYTE_TC].value & 0x01;
            			else
                			checksum = ATR_DEFAULT_CHK; /* default - LRC */
				handler->error_check_type = ((checksum==ATR_DEFAULT_CHK)?IFD_PROTOCOL_T1_CHECKSUM_LRC:IFD_PROTOCOL_T1_CHECKSUM_CRC);
            
        		}
    		}
		SMC_RW_PRINTF("T1 special: ifsc: %d,  CWI:%d,  BWI:%d, checksum:%d(3:LRC,2:CRC)\n",
						handler->T1.ifsc, handler->CWI, handler->BWI, handler->error_check_type);
	}	
	
	SMC_RW_PRINTF("First CWT: %d, CWT: %d\n", handler->first_cwt, handler->cwt);
	SMC_RW_PRINTF("ATR: HC:");
	for(i=0; i<atr->hbn; i++)
	{
		SMC_RW_PRINTF("%c ", (atr->hb)[i]);
	}
	SMC_RW_PRINTF("\n");
	
	return SMART_NO_ERROR;
}


void smc_dev_clear_tx_rx_buf(struct smartcard_private *tp)
{
	tp->smc_tx_buf = NULL;
    	tp->smc_rx_buf = NULL;
	tp->smc_tx_rd = 0;
	tp->smc_tx_wr = 0;
	tp->smc_rx_head = 0;
	tp->got_first_byte = 0;
	tp->smc_rx_tail = 0;
}

/******************************************************************************************************
 * 	Name		:	smc_dev_get_card_etu()
 *	Description	:	To get the smart card ETU.
 *	Parameter	:	struct smc_device *dev	: Devcie handle.
 *	Return		:	INT32			: SUCCESS or FAIL.
 *
 ******************************************************************************************************/
static INT32 smc_dev_get_card_etu(struct smc_device *dev)
{
	int i=0;
	UINT32 ioaddr = dev->base_addr;
	UINT32 etu=0,etu3=0;
	UINT32 wai_atr_tmo = 0, wai_atr_time = 0;
	UINT8 cc = 0;
	UINT16 rx_cnt =0;
	UINT8 etu_trigger1 = 0;
	UINT8 etu_trigger2 = 0;
	UINT32 old_etu_tick1=0, old_etu_tick2=0;
	struct smartcard_private *tp = (struct smartcard_private *)dev->priv;

	SMC_RW_PRINTF("%s ioaddr: %x\n",__FUNCTION__,ioaddr);
	
	OUTPUT_UINT8( ioaddr + REG_SCR_CTRL, SMC_SCR_CTRL_OP|SMC_SCR_CTRL_RECV|(tp->parity_odd<<4));
	osal_delay(200);
	
	OUTPUT_UINT8(ioaddr + REG_ISR0, 0xff);
	OUTPUT_UINT8(ioaddr + REG_ISR1, 0xff);
	
	//no fifo mode
	//OUTPUT_UINT8(ioaddr + REG_FIFO_CTRL,0x00);
	OUTPUT_UINT8(ioaddr + REG_FIFO_CTRL,SMC_FIFO_CTRL_EN|SMC_FIFO_CTRL_TX_OP|SMC_FIFO_CTRL_RX_OP);                                         
	smc_write_rx(ioaddr,32);
	osal_interrupt_disable();
	if(tp->inserted != 1)  //card not insert
	{
		osal_interrupt_enable();
		return RET_FAILURE;
	}
	if(tp->warm_reset)
	{
		if(tp->apd_disable/*ALI_S3602==smc_chip_id*/)
			OUTPUT_UINT8(ioaddr + REG_ICCR, INPUT_UINT8(ioaddr + REG_ICCR)|SMC_RB_ICCR_PRT_EN);
		else
			OUTPUT_UINT8(ioaddr + REG_ICCR, INPUT_UINT8(ioaddr + REG_ICCR)|SMC_RB_ICCR_PRT_EN|0x40);
	}
	else	
	{
		if(tp->apd_disable/*ALI_S3602==smc_chip_id*/)
			OUTPUT_UINT8(ioaddr + REG_ICCR, SMC_RB_ICCR_PRT_EN);
		else	
			OUTPUT_UINT8(ioaddr + REG_ICCR, SMC_RB_ICCR_PRT_EN|0x40);
		osal_delay(ATV_VCC2IO>>1);
		if(tp->use_gpio_vpp)
		{
			if(tp->internal_ctrl_vpp)
				OUTPUT_UINT8(ioaddr + REG_CLK_VPP, INPUT_UINT8(ioaddr + REG_CLK_VPP)|SMC_RB_CTRL_VPP);
			else
				HAL_GPIO_BIT_SET(tp->gpio_vpp_pos, tp->gpio_vpp_pol);
		}
		osal_delay(/*20*/ATV_VCC2IO>>1);
		OUTPUT_UINT8(ioaddr + REG_ICCR, INPUT_UINT8(ioaddr + REG_ICCR)|SMC_RB_ICCR_DIO);
	}
	osal_interrupt_enable();
	if(!tp->warm_reset)
	{
		osal_delay(/*200*/ATV_IO2CLK);
		OUTPUT_UINT8(ioaddr + REG_ICCR, INPUT_UINT8(ioaddr + REG_ICCR)|SMC_RB_ICCR_CLK);
		osal_delay(200);
	}
	
	while(1)
	{
		if(tp->isr1_interrupt_status & SMC_ISR1_RST_LOW)
		{
			tp->isr1_interrupt_status &= (~SMC_ISR1_RST_LOW);
			SMC_RW_PRINTF("atr_on_low trigger!\n");
			if(ALI_S3602==smc_chip_id)
			{
				if(0!=smc_read_rx(ioaddr))
					break;
			}
			else
				break;
		}
		if(tp->isr1_interrupt_status & SMC_ISR1_COUNT_ST)
		{
			SMC_RW_PRINTF("set rst to high\n");
			tp->isr1_interrupt_status &= (~SMC_ISR1_RST_NATR);
			tp->isr1_interrupt_status &= (~SMC_ISR1_COUNT_ST);
			
			OUTPUT_UINT8(ioaddr + REG_ISR1, INPUT_UINT8(ioaddr + REG_ISR1)|SMC_ISR1_RST_NATR);
			OUTPUT_UINT8(ioaddr + REG_ICCR, INPUT_UINT8(ioaddr + REG_ICCR)|SMC_RB_ICCR_RST);
			while(0==smc_read_rx(ioaddr))
			{			
				if( (tp->isr1_interrupt_status & SMC_ISR1_RST_NATR)&&(etu_trigger1==0))
				{
					tp->isr1_interrupt_status &= (~SMC_ISR1_RST_NATR);
					etu_trigger1 = 1;
					old_etu_tick1 = osal_get_tick();
				}
				if(etu_trigger1)
				{
					if((osal_get_tick() - old_etu_tick1) > 100)
					{
						return !RET_SUCCESS;
					}
				}
				if(smc_dev_card_exist(dev) != SUCCESS)
				{
					return !RET_SUCCESS;
				}
			}
			cc = INPUT_UINT8(ioaddr + REG_RBR);
			if(0x3b==cc)
			{
				tp->inverse_convention = 0;
				SMC_RW_PRINTF("Normal card %02x\n", cc);
			}
			else
			{
				tp->inverse_convention = 1;
				SMC_RW_PRINTF("Inverse card %02x\n", cc);
			}
			while(1)
			{
				if(tp->isr1_interrupt_status & SMC_ISR1_RST_HIGH)
				{
					tp->isr1_interrupt_status &= (~SMC_ISR1_RST_HIGH);
					break;
				}
				
				if( (tp->isr1_interrupt_status & SMC_ISR1_RST_NATR)&&(etu_trigger2==0))
				{
					tp->isr1_interrupt_status &= (~SMC_ISR1_RST_NATR);
					etu_trigger2 = 1;
					old_etu_tick2 = osal_get_tick();
				}
				if(etu_trigger2)
				{
					if((osal_get_tick() - old_etu_tick2) > 100)
						return !RET_SUCCESS;
				}
				
				if(smc_dev_card_exist(dev) != SUCCESS)
					return !RET_SUCCESS;
				//osal_delay(1);
				//Why there is no time out??
			}
			break;
		}
		if(smc_dev_card_exist(dev) != SUCCESS) return !RET_SUCCESS;
		//osal_delay(1);
	}
	wai_atr_tmo =  (9600*372*2)/(tp->smc_clock/1000);
	wai_atr_time = osal_get_tick();
	rx_cnt = smc_read_rx(ioaddr); 
	while((tp->isr0_interrupt_status & SMC_ISR0_BYTE_RECV) != SMC_ISR0_BYTE_RECV )
	{
		if(smc_dev_card_exist(dev) != SUCCESS) return !RET_SUCCESS;
		if(rx_cnt == smc_read_rx(ioaddr))
		{
			osal_task_sleep(1);
			wai_atr_time = osal_get_tick() - wai_atr_time;
			if(wai_atr_tmo>=wai_atr_time)
				wai_atr_tmo -= wai_atr_time;
			else
			{
				SMC_RW_PRINTF("Wait ATR time out!\n");
				return !RET_SUCCESS;
			}	
		}
		else
		{
			rx_cnt = smc_read_rx(ioaddr);
			wai_atr_tmo = (9600*372*2)/(tp->smc_clock/1000);
		}
		wai_atr_time = osal_get_tick();
	}
	//Disable receiver mode
	OUTPUT_UINT8(ioaddr + REG_SCR_CTRL,(INPUT_UINT8(ioaddr + REG_SCR_CTRL)&(~SMC_SCR_CTRL_RECV)));
	tp->isr0_interrupt_status &= (~SMC_ISR0_BYTE_RECV);
	SMC_RW_PRINTF("Got card etu is %d, 3etu is %d. \n",INPUT_UINT16(ioaddr + REG_RCNT_ETU),INPUT_UINT16(ioaddr + REG_RCNT_3ETU));
	
	etu = INPUT_UINT16(ioaddr + REG_RCNT_ETU);

	etu3 = (INPUT_UINT16(ioaddr + REG_RCNT_3ETU)/3-15);
	etu = (etu3<DFT_WORK_ETU)? etu : etu3;

	SMC_RW_PRINTF("set card etu: %d\n",etu);
	tp->smc_etu = etu;
	OUTPUT_UINT16(ioaddr + REG_ETU0, etu);			//Set right etu

	return RET_SUCCESS;
}

static INT32 smc_dev_set_pps(struct smc_device *dev, UINT8 PPS0, UINT8 FI, UINT8 DI)
{
	UINT8 pps_buf[4], pps_echo[4];
	INT32 rlt = (!RET_SUCCESS);
	UINT16 actsize, rw_len;
	pps_buf[0] = 0xff;
	pps_buf[1] = PPS0;
	if(PPS0&0x10)
	{
		pps_buf[2] = ((FI&0xf)<<4)|(DI&0xf);
		pps_buf[3] = pps_buf[0]^pps_buf[1]^pps_buf[2];
		SMC_PRINTF("ppss %02x pps0 %02x pps1 %02x pck %02x\n", pps_buf[0], pps_buf[1], pps_buf[2], pps_buf[3]);
	}
	else
	{
		pps_buf[2] = pps_buf[0]^pps_buf[1];
		SMC_PRINTF("ppss %02x pps0 %02x pck %02x\n", pps_buf[0], pps_buf[1], pps_buf[2]);
	}
		
	if(PPS0&0x10)
		rw_len = 4;
	else
		rw_len = 3;
	MEMSET(pps_echo, 0x5a, 4);
	smc_dev_transfer_data(dev, pps_buf, rw_len, pps_echo, rw_len, &actsize);
	//smc_dev_write(dev, pps_buf, rw_len, &actsize);
	
	//smc_dev_read(dev, pps_echo, rw_len, &actsize);
	SMC_PRINTF("pps echo: %02x %02x %02x %02x\n", pps_echo[0], pps_echo[1], pps_echo[2], pps_echo[3]);
	if(actsize)
	{
		if(pps_buf[0] == pps_echo[0])
		{
			if((pps_buf[1]&0xf)==(pps_echo[1]&0xf))
			{
				if(PPS0&0x10)
				{
					if((pps_buf[1]&0x10)==(pps_echo[1]&0x10))
					{
						rlt  =RET_SUCCESS;
						SMC_PRINTF("pps SUCCESS!\n");
					}
					//else
					//	SMC_PRINTF("pps: USE default FI, DI\n");
				}
				else
				{
					rlt  =RET_SUCCESS;
					SMC_PRINTF("pps SUCCESS!\n");
				}
			}
			//else
			//	SMC_PRINTF("pps : USE default T!\n");
		}
		//else
		//	SMC_PRINTF("card don't support PPS!\n");
	}
	//else
	//	SMC_PRINTF("pps got NO Response!\n");
	
	return rlt;
}

/******************************************************************************************************
 * 	Name		:	smc_dev_get_card_atr()
 *	Description	:	To get the smart card ATR.
 *	Parameter	:	struct smc_device *dev	: Devcie handle.
 *				UINT8 *buffer		: Read data buffer.
 *				UINT16 *atr_size	: ATR data size.
 *	Return		:	INT32			: SUCCESS or FAIL.
 *
 ******************************************************************************************************/
static INT32 smc_dev_get_card_atr(struct smc_device *dev, UINT8 *buffer, UINT16 *atr_size)
{
	int i=0;
	int c=0;
	UINT32 wai_atr_tmo = 0, wai_atr_time = 0;
	unsigned char cc = 0,status=0;
	UINT16 rx_cnt=0;
	unsigned char cnt_trigger = 0;
	UINT32 old_tick = 0;
	UINT32 ioaddr = dev->base_addr;
	struct smartcard_private *tp = (struct smartcard_private *)dev->priv;

	SMC_RW_PRINTF("%s\n",__FUNCTION__);
	tp->force_tx_rx_thld = FORCE_TX_RX_THLD;
	
	//enable receiver mode and set to the direct mode
	OUTPUT_UINT8( ioaddr + REG_SCR_CTRL, SMC_SCR_CTRL_OP|SMC_SCR_CTRL_RECV|(tp->parity_odd<<4));

	OUTPUT_UINT8(ioaddr + REG_FIFO_CTRL,SMC_FIFO_CTRL_EN|SMC_FIFO_CTRL_TX_OP|SMC_FIFO_CTRL_RX_OP);                                         
	smc_write_rx(ioaddr, ATR_MAX_SIZE);
	
	//OUTPUT_UINT8(ioaddr + REG_IER0, 0x30);
	
	// clear interrupt status
	OUTPUT_UINT8(ioaddr + REG_ISR1, INPUT_UINT8(ioaddr + REG_ISR1));
	OUTPUT_UINT8(ioaddr + REG_ISR0, INPUT_UINT8(ioaddr + REG_ISR0));
	                                         
	osal_interrupt_disable();
	if(tp->inserted != 1)  //card not insert
	{
		osal_interrupt_enable();
		return RET_FAILURE;
	}
	
	if(tp->warm_reset)
	{
		if(tp->inverse_convention)
		{
			SMC_RW_PRINTF("Already know it's iverse card, dis apd\n");
			OUTPUT_UINT8(ioaddr + REG_ICCR, (INPUT_UINT8(ioaddr + REG_ICCR)&(~0x30))|SMC_RB_ICCR_AUTO_PRT);
		}
		else
		{
			SMC_RW_PRINTF("Already know it's directed card, parity %d, apd %d\n", !tp->parity_disable, !tp->apd_disable);
			OUTPUT_UINT8(ioaddr + REG_ICCR, INPUT_UINT8(ioaddr + REG_ICCR)&0xcf);
			OUTPUT_UINT8(ioaddr + REG_ICCR, INPUT_UINT8(ioaddr + REG_ICCR)|(tp->parity_disable<<5)|(tp->apd_disable<<4));
		}
	}
	else	
	{
		if(tp->inverse_convention)
		{
			SMC_RW_PRINTF("Already know it's iverse card, dis apd\n");
			if(tp->apd_disable/*ALI_S3602==smc_chip_id*/)
				OUTPUT_UINT8(ioaddr + REG_ICCR, SMC_RB_ICCR_AUTO_PRT);
			else	
				OUTPUT_UINT8(ioaddr + REG_ICCR, SMC_RB_ICCR_AUTO_PRT|0x40);
		}
		else
		{
			SMC_RW_PRINTF("Already know it's directed card, parity %d, apd %d\n", !tp->parity_disable, !tp->apd_disable);
			if(tp->apd_disable/*ALI_S3602==smc_chip_id*/)
				OUTPUT_UINT8(ioaddr + REG_ICCR, (tp->parity_disable<<5)|(tp->apd_disable<<4));
			else	
				OUTPUT_UINT8(ioaddr + REG_ICCR, (tp->parity_disable<<5)|(tp->apd_disable<<4)|0x40);
		}
		osal_delay(ATV_VCC2IO>>1);
		if(tp->use_gpio_vpp)
		{
			if(tp->internal_ctrl_vpp)
				OUTPUT_UINT8(ioaddr + REG_CLK_VPP, INPUT_UINT8(ioaddr + REG_CLK_VPP)|SMC_RB_CTRL_VPP);
			else
				HAL_GPIO_BIT_SET(tp->gpio_vpp_pos, tp->gpio_vpp_pol);
		}
		osal_delay(/*20*/ATV_VCC2IO>>1);
		OUTPUT_UINT8(ioaddr + REG_ICCR, INPUT_UINT8(ioaddr + REG_ICCR)|SMC_RB_ICCR_DIO);
	}
	/*Basically, we should not disable parity check while get ATR, but for irdeto cards, we have to do so.*/
	if(INPUT_UINT16(ioaddr + REG_ETU0)>550)//irdeto cards, disable parity check
		OUTPUT_UINT8(ioaddr + REG_ICCR, INPUT_UINT8(ioaddr + REG_ICCR)|0x20);

	osal_interrupt_enable();

	if(!tp->warm_reset)
	{
		osal_delay(/*200*/ATV_IO2CLK);
		OUTPUT_UINT8(ioaddr + REG_ICCR, INPUT_UINT8(ioaddr + REG_ICCR)|SMC_RB_ICCR_CLK);
		osal_delay(200);
	}
	
	while(1)
	{
		if(tp->isr1_interrupt_status & SMC_ISR1_RST_LOW)
		{
			SMC_RW_PRINTF("atr_on_low trigger!\n");
			tp->isr1_interrupt_status &= (~SMC_ISR1_RST_LOW);
			if(ALI_S3602==smc_chip_id)
			{
				if(0!=smc_read_rx(ioaddr))
					break;
			}
			else
				break;
		}
		if(tp->isr1_interrupt_status & SMC_ISR1_COUNT_ST)
		{
			//UINT32 tmo = 0;
			SMC_RW_PRINTF("set rst to high\n");
			tp->isr1_interrupt_status &= (~SMC_ISR1_COUNT_ST);
			tp->isr1_interrupt_status &= (~SMC_ISR1_RST_NATR);
			
			OUTPUT_UINT8(ioaddr + REG_ISR1, INPUT_UINT8(ioaddr + REG_ISR1)|SMC_ISR1_RST_NATR);
			OUTPUT_UINT8(ioaddr + REG_ICCR, INPUT_UINT8(ioaddr + REG_ICCR)|SMC_RB_ICCR_RST);
			
			while(1)
			{
				
				if(tp->isr1_interrupt_status & SMC_ISR1_RST_HIGH)
				{
					tp->isr1_interrupt_status &= (~SMC_ISR1_RST_HIGH);
					break;
				}			
				if(( tp->isr1_interrupt_status & SMC_ISR1_RST_NATR)&&(cnt_trigger == 0))
				{
					tp->isr1_interrupt_status &= (~SMC_ISR1_RST_NATR);
					cnt_trigger = 1;
					old_tick = osal_get_tick();
					
				}
				if(cnt_trigger)
				{		
					if((osal_get_tick() - old_tick) > 100)
					{
						tp->isr1_interrupt_status &= (~SMC_ISR1_RST_NATR);
						SMC_RW_PRINTF("None ATR!\n");
						return !RET_SUCCESS;
					}
				}
				/*
				if( tp->isr0_interrupt_status & SMC_ISR0_TIMEOUT)
				{
					tp->isr0_interrupt_status &= (~SMC_ISR0_TIMEOUT);
					SMC_PRINTF("Wait ATR Time Out!\n");
					return !RET_SUCCESS;
				}*/
				if(smc_dev_card_exist(dev) != SUCCESS)
				{
					return !RET_SUCCESS;
				}
				
			}
			break;
		}
		if(smc_dev_card_exist(dev) != SUCCESS)
			return !RET_SUCCESS;
		//osal_delay(1);
	}
	wai_atr_tmo =  (9600*2*INPUT_UINT16(ioaddr + REG_ETU0))/(tp->smc_clock/1000);
	wai_atr_time = osal_get_tick();
	rx_cnt = smc_read_rx(ioaddr); 
	while(1)
	{
		//osal_delay(1);
		if(smc_dev_card_exist(dev) != SUCCESS)
			return !RET_SUCCESS;
		if((tp->isr0_interrupt_status & SMC_ISR0_FIFO_RECV )||( tp->isr0_interrupt_status & SMC_ISR0_TIMEOUT))
		{
			tp->isr0_interrupt_status &= (~SMC_ISR0_FIFO_RECV);
			tp->isr0_interrupt_status &= (~SMC_ISR0_TIMEOUT);
			break;
		}
		if(0==(INPUT_UINT8(ioaddr + REG_ICCR)&0x30)&&(tp->isr0_interrupt_status&SMC_ISR0_PE_RECV))
		{
			SMC_RW_PRINTF("Get ATR Parity Error!\n");
			tp->isr0_interrupt_status &= (~SMC_ISR0_PE_RECV);
			return !RET_SUCCESS;
		}
		if(rx_cnt == smc_read_rx(ioaddr))
		{
			osal_task_sleep(1);
			wai_atr_time = osal_get_tick() - wai_atr_time;
			if(wai_atr_tmo>=wai_atr_time)
				wai_atr_tmo -= wai_atr_time;
			else
			{
				if(smc_read_rx(ioaddr))
					break;
				SMC_RW_PRINTF("Wait ATR time out!\n");
				return !RET_SUCCESS;
			}	
		}
		else
		{
			rx_cnt = smc_read_rx(ioaddr);
			wai_atr_tmo = (9600*2*INPUT_UINT16(ioaddr + REG_ETU0))/(tp->smc_clock/1000);
		}
		wai_atr_time = osal_get_tick();
	}
	

	c =smc_read_rx(ioaddr); 		
	if(0==c||c>ATR_MAX_SIZE)
	{
		SMC_RW_PRINTF("Invalid ATR length: %d\n", c);
		if(c>ATR_MAX_SIZE)
			c = ATR_MAX_SIZE;
		else
			return !RET_SUCCESS;
	}
	SMC_RW_PRINTF("%d ATR bytes received: ", c);
#if 1	
	tp->atr_size = *atr_size = c;
	for(i=0;i<c;i++)
	{
		cc = INPUT_UINT8(ioaddr + REG_RBR);
		SMC_RW_PRINTF("%02x ",cc);
		buffer[i] = cc;
	}
	
	SMC_RW_PRINTF("\n");
#else
    
UINT8 atr_test_str[] = {
	0X3B, 0XDA, 0X18, 0XFF, 0X81, 0XB1, 0XFE,
	0X75, 0X1F, 0X03, 0X00, 0X31, 0XC5, 0X73,
	0XC0, 0X01, 0X40, 0X00, 0X90, 0X00, 0X0C};
	
	tp->atr_size = *atr_size = c;
	for(i=0;i<c;i++)
	{
		cc = INPUT_UINT8(ioaddr + REG_RBR);
		//SMC_RW_PRINTF("%02x ",cc);
		buffer[i] = cc;
	}

	tp->atr_size = *atr_size = c = sizeof(atr_test_str);
	for(i=0;i<c;i++)
	{
		cc = atr_test_str[i];
		SMC_RW_PRINTF("%02x ",cc);
		buffer[i] = cc;
	}

	SMC_RW_PRINTF("\n");
#endif
	if(buffer[0]==0x03)
	{
		SMC_RW_PRINTF("Inv card detected!\n");
		tp->inverse_convention = 1;
		//set to the inverse mode.
		OUTPUT_UINT8( ioaddr + REG_SCR_CTRL, SMC_SCR_CTRL_OP|SMC_SCR_CTRL_INVESE|(tp->parity_odd<<4));
		invert(buffer, c);
	}
 	else if((buffer[0]==0x3f)&&(tp->ts_auto_detect==1))
 	{
 		SMC_RW_PRINTF("Inv card auto detected!\n");
 		tp->inverse_convention = 1;
 		//set to the inverse mode.
 		OUTPUT_UINT8( ioaddr + REG_SCR_CTRL, SMC_SCR_CTRL_OP|SMC_SCR_CTRL_INVESE|(tp->parity_odd<<4));
 		//invert(buffer, c);
 	}
	else if(buffer[0]==0x3b)
	{
		SMC_RW_PRINTF("Normal card detected!\n");
		tp->inverse_convention = 0;
	}
	else
		return !RET_SUCCESS;
	
	//disable receiver mode.
	OUTPUT_UINT8(ioaddr + REG_SCR_CTRL,(INPUT_UINT8(ioaddr + REG_SCR_CTRL)&(~SMC_SCR_CTRL_RECV)));

	return RET_SUCCESS;
}

static INT32 smc_warm_reset(struct smc_device *dev)
{
	UINT32 base_addr = dev->base_addr;
	struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    UINT32 i;
	OUTPUT_UINT8(base_addr + REG_ICCR, (INPUT_UINT8(base_addr + REG_ICCR) & (~SMC_RB_ICCR_RST)));		// RST L
    for(i=0;i<11;i++)
        osal_delay(1000);
    SMC_PRINTF("%s: over\n", __FUNCTION__);
	return RET_SUCCESS;
}

static void smc_set_etu(struct smc_device *dev, UINT32 new_etu)
{
	struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
	UINT32 ioaddr = dev->base_addr;
	UINT8 FI = 1;
	UINT8 DI = 1;

	if (tp->atr_info->ib[0][ATR_INTERFACE_BYTE_TA].present)
	{
		FI = (tp->atr_info->ib[0][ATR_INTERFACE_BYTE_TA].value & 0xF0) >> 4;
		DI =  (tp->atr_info->ib[0][ATR_INTERFACE_BYTE_TA].value & 0x0F); 
	}
	if(tp->reseted)
	{
		if(tp->T==0)
		{
			UINT8 WI = ATR_DEFAULT_WI;
				
			if (tp->atr_info->ib[2][ATR_INTERFACE_BYTE_TC].present)
				WI = tp->atr_info->ib[2][ATR_INTERFACE_BYTE_TC].value;
			
			OUTPUT_UINT32(ioaddr + REG_CBWTR0, 960*WI*DI);
		}
		else if(tp->T==1)
		{
			UINT8 BWI = tp->BWI; 
			OUTPUT_UINT32(ioaddr + REG_CBWTR0, 11+((960*372*(1<<BWI)*DI)/FI));
		}
		tp->smc_etu = new_etu;
		OUTPUT_UINT16(ioaddr + REG_ETU0, tp->smc_etu);	
	}
	
}
/******************************************************************************************************
 * 	Name		:	smc_dev_reset()
 *	Description	:	Smart card reset.
 *	Parameter	:	struct smc_device *dev	: Devcie handle.
 *				UINT8 *buffer		: Read data buffer.
 *				UINT16 *atr_size	: ATR data size.
 *	Return		:	INT32			: SUCCESS or FAIL.
 *
 ******************************************************************************************************/
static INT32 smc_dev_reset(struct smc_device *dev, UINT8 *buffer, UINT16 *atr_size)
{
	INT32 ret,err=0;
	UINT8 i = 0;
	UINT8 tmp;
	UINT32 work_etu, init_clk_idx;
	UINT32 ioaddr = dev->base_addr;
	UINT8 need_reset_etu = 0;
	UINT8 FI = 1;
	UINT8 DI = 1;
	struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
	int etu,card_mode;

	SMC_RW_PRINTF("%s\n",__FUNCTION__);
	
	osal_semaphore_capture(tp->smc_sema_id, OSAL_WAIT_FOREVER_TIME);
	t1_init(&tp->T1);
	smc_dev_clear_tx_rx_buf(tp);
	if(tp->use_default_etu)
		tp->smc_etu = tp->default_etu;
	else
		tp->smc_etu = DFT_WORK_ETU;
	work_etu = tp->smc_etu;
	OUTPUT_UINT16(ioaddr + REG_ETU0, work_etu);	
	OUTPUT_UINT16(ioaddr + REG_GTR0, 12);//12		//set gtr to 12
	OUTPUT_UINT32(ioaddr + REG_CBWTR0, 12800);
	if(0==tp->inserted)
	{
		SMC_RW_PRINTF("smc: smart card not inserted!\n");
		osal_semaphore_release(tp->smc_sema_id);
		return !RET_SUCCESS;
	}
	if(1==tp->reseted)
	{
		tp->reseted = 0;
		if(1==tp->warm_reset_enable)
			tp->warm_reset = 1;
	}
	else
		tp->warm_reset = 0;
	SMC_RW_PRINTF("warm reset %d\n", tp->warm_reset);
	tp->isr0_interrupt_status = 0;
	tp->isr1_interrupt_status = 0;
	OUTPUT_UINT8(ioaddr + REG_IER0,INPUT_UINT8(ioaddr + REG_IER0)&(~SMC_IER0_TRANS_FIFO_EMPY));
	tp->init_clk_idx = 0;
	tp->atr_rlt = SMC_ATR_NONE;
	while(tp->init_clk_idx<tp->init_clk_number)
	{
		if(tp->inserted == 0)
		{
			SMC_PRINTF("smc: smart card not inserted!\n");
			osal_semaphore_release(tp->smc_sema_id);
			return !RET_SUCCESS;
		}
		
		if(!tp->warm_reset)
		{
			tp->smc_clock = tp->init_clk_array[tp->init_clk_idx];
			SMC_RW_PRINTF("try init clk %d, No. %d\n", tp->smc_clock, tp->init_clk_idx);
			smc_set_wclk(ioaddr, tp->smc_clock);
			osal_task_sleep(1);
			smc_dev_deactive(dev);
			osal_task_sleep(1);
		}
		else
		{
			osal_task_sleep(1);
			smc_warm_reset(dev);
			osal_task_sleep(1);
		}
		err = 0;
		if(tp->use_default_etu)
		{
			if(smc_dev_get_card_atr(dev,buffer,atr_size) != SUCCESS)
			{
				if(0==tp->inverse_convention&&0==tp->ts_auto_detect)
				{
					if(!tp->warm_reset)
					{
						smc_dev_deactive(dev);
						osal_task_sleep(1);
					}
					else
					{
						smc_warm_reset(dev);
						osal_task_sleep(1);
					}
					tp->inverse_convention = 1;
					if(smc_dev_get_card_atr(dev,buffer,atr_size) != SUCCESS)
					{
						tp->inverse_convention = 0;
						err = 1;
					}
				}
				else
					err = 1;
			}
		}
		else
		{
			if(smc_dev_get_card_etu(dev) == SUCCESS)
			{
				if(!tp->warm_reset)
				{
					smc_dev_deactive(dev);
					osal_task_sleep(1);
				}
				else
				{
					smc_warm_reset(dev);
					osal_task_sleep(1);
				}
				if(smc_dev_get_card_atr(dev,buffer,atr_size) != SUCCESS)
					err = 1;
			}
			else
				err = 1;
		}
		if(err)
		{
			if(!tp->warm_reset)
				tp->init_clk_idx++;
			else
				break;
		}
		else
			break;
	}
	//osal_semaphore_release(tp->smc_sema_id);
	if(err)
	{
		smc_dev_deactive(dev);
	       osal_semaphore_release(tp->smc_sema_id);
		return !RET_SUCCESS;
	}
	SMC_DUMP(buffer, *atr_size);
	MEMSET(tp->atr_info, 0, sizeof(atr_t));
	
	if(SMART_WRONG_ATR==atr_init(tp->atr_info, buffer, *atr_size))
		tp->atr_rlt = SMC_ATR_WRONG;
	else
		tp->atr_rlt = SMC_ATR_OK;
	//tp->atr_info = &global_atr;
	atr_config_parameter(tp, tp->atr_info);
	if(tp->T == 1)
	{
		if(tp->D != 0)
		{
			tp->smc_etu = work_etu = tp->F/tp->D;
			SMC_RW_PRINTF("work etu : %d\n",work_etu);
		}
		//OUTPUT_UINT16(ioaddr + REG_ETU0, work_etu);		//set work etu.	
		osal_interrupt_disable();
		if(tp->inserted != 1)  //card not insert
		{
			osal_interrupt_enable();
			osal_semaphore_release(tp->smc_sema_id);
			return RET_FAILURE;
		}
		if(tp->apd_disable/*ALI_S3602==smc_chip_id*/)
			OUTPUT_UINT8(ioaddr + REG_ICCR, SMC_RB_ICCR_AUTO_PRT|SMC_RB_ICCR_OP);
		else	
			OUTPUT_UINT8(ioaddr + REG_ICCR, SMC_RB_ICCR_AUTO_PRT|SMC_RB_ICCR_OP|0x40);		//disable auto parity error pull down control for T=1 card.
		osal_interrupt_enable();
	}
	else if(tp->T == 14)
	{
		osal_interrupt_disable();
		if(tp->inserted != 1)  //card not insert
		{
			osal_interrupt_enable();
			osal_semaphore_release(tp->smc_sema_id);
			return RET_FAILURE;
		}        
		if(tp->apd_disable/*ALI_S3602==smc_chip_id*/)
			OUTPUT_UINT8(ioaddr + REG_ICCR, SMC_RB_ICCR_PRT_EN|SMC_RB_ICCR_OP);
		else	
			OUTPUT_UINT8(ioaddr + REG_ICCR, SMC_RB_ICCR_PRT_EN|SMC_RB_ICCR_OP|0x40);		//disable parity control for T=14 card.
		osal_interrupt_enable();
	}
	else
	{
		if(tp->D != 0)
		{
			tp->smc_etu = work_etu = tp->F/tp->D;
			SMC_RW_PRINTF("work etu : %d\n",work_etu);
		}
		osal_interrupt_disable();
		if(tp->inserted != 1)  //card not insert
		{
			osal_interrupt_enable();
			osal_semaphore_release(tp->smc_sema_id);
			return RET_FAILURE;
		}        
		if(tp->apd_disable/*ALI_S3602==smc_chip_id*/)
			OUTPUT_UINT8(ioaddr + REG_ICCR, (tp->apd_disable<<4)|SMC_RB_ICCR_OP);	
		else	
			OUTPUT_UINT8(ioaddr + REG_ICCR, (tp->apd_disable<<4)|SMC_RB_ICCR_OP|0x40);	
		//OUTPUT_UINT8(ioaddr + REG_ICCR, SMC_RB_ICCR_OP|0x40);				//enable parity and auto parity error pull down control.T=0 card.
		osal_interrupt_enable();
	}

	if(buffer != tp->atr)
		MEMCPY(tp->atr, buffer, *atr_size);
	//calc the bwt,cwt,bgt for T1, in us(1/1000000 second)
	if (tp->T == 1)
	{
		/*BGT = 22 etu*/
		tp->T1.BGT =  (22*tp->smc_etu)/(tp->smc_clock/1000);
		/*CWT = (2^CWI + 11) etu*/
		tp->T1.CWT =  tp->cwt;  
		/*BWT = (2^BWI*960 + 11)etu, 
		  *Attention: BWT in ms, If calc in us, it will overflow for UINT32
		  */
		tp->T1.BWT= tp->first_cwt;

		/*Add error check type*/
		t1_set_checksum(&tp->T1, tp->error_check_type);
		/*reset the T1 state to undead state*/
		//t1_set_param(&tp->T1, IFD_PROTOCOL_T1_STATE, SENDING);
		t1_set_param(&(tp->T1), IFD_PROTOCOL_T1_STATE, SENDING);
		SMC_RW_PRINTF("T1 special: BGT:%d, CWT:%d, us BWT:%d  ms \n",tp->T1.BGT, tp->T1.CWT, tp->T1.BWT);

	}

	tp->reseted = 1;
#ifdef SUPPORT_NDS_CARD
	if((tp->T==0) && (tp->inverse_convention==1))
	{
  		if( (buffer[1] == 0x7F || buffer[1] == 0xFF || buffer[1] == 0xFD ) && 
			( buffer[2] == 0x13 || buffer[2] == 0x11 ) )
  		{
  			tp->atr_info->ib[1][ATR_INTERFACE_BYTE_TA].present = TRUE;
			need_reset_etu = 1;
  		}
	}
#endif
	if(FALSE == tp->atr_info->ib[1][ATR_INTERFACE_BYTE_TA].present)
	{
		UINT8 pps0 = 0;
		UINT32 first_cwt ; 
		UINT32 cwt;
		UINT8 T = 0xff;
		UINT8 multi_protocol = 0;
		UINT8 diff_f_d = 0;
		if(tp->disable_pps)
		{
			if(1==tp->T)
			{
				tp->smc_etu = INPUT_UINT16(ioaddr + REG_ETU0);
				tp->cwt =  (((1<<(tp->CWI))+11 )*tp->smc_etu)/(tp->smc_clock/1000) + 1; 
           			tp->first_cwt = (11*tp->smc_etu)/(tp->smc_clock/1000)+((1<<((UINT32)tp->BWI))*960*ATR_DEFAULT_F)/(tp->smc_clock/1000) + 2;	
				/*BGT = 22 etu*/
				tp->T1.BGT =  (22*tp->smc_etu)/(tp->smc_clock/1000);
				/*CWT = (2^CWI + 11) etu*/
				tp->T1.CWT =  tp->cwt;  
				/*BWT = (2^BWI*960 + 11)etu, 
		  		*Attention: BWT in ms, If calc in us, it will overflow for UINT32
		  		*/
				tp->T1.BWT= tp->first_cwt;
				SMC_PRINTF("T1 disable PPS: CWT %d, BWT %d\n", tp->cwt,  tp->first_cwt);
			}

			osal_task_sleep(10);	
			osal_semaphore_release(tp->smc_sema_id);
			return RET_SUCCESS;
		}
		if (tp->atr_info->ib[0][ATR_INTERFACE_BYTE_TA].present)
		{
			UINT32 f, d;
			FI = (tp->atr_info->ib[0][ATR_INTERFACE_BYTE_TA].value & 0xF0) >> 4;
			DI =  (tp->atr_info->ib[0][ATR_INTERFACE_BYTE_TA].value & 0x0F); 
			f = atr_f_table[FI];
			if(F_RFU==f)
				f = ATR_DEFAULT_F;
			d = atr_d_table[DI];
			if(D_RFU==d)
				d = ATR_DEFAULT_D;
			if(ATR_DEFAULT_F!=f||ATR_DEFAULT_D!=d)
				diff_f_d = 1;
		}
		for (i=0; i<ATR_MAX_PROTOCOLS; i++)
			if (tp->atr_info->ib[i][ATR_INTERFACE_BYTE_TD].present)
			{
				UINT8 T_type = tp->atr_info->ib[i][ATR_INTERFACE_BYTE_TD].value & 0x0F;
				if(0xf==T_type)
					break;
				if(0xFF == T)
					T = T_type;
				else if(T!=T_type)
				{
					T = T_type;
					multi_protocol = 1;
				}
			}
	
		if(multi_protocol||diff_f_d)
		{
			first_cwt = tp->first_cwt; 
			cwt = tp->cwt;
			if (diff_f_d)
			{
				//FI = (tp->atr_info->ib[0][ATR_INTERFACE_BYTE_TA].value & 0xF0) >> 4;
				//DI =  (tp->atr_info->ib[0][ATR_INTERFACE_BYTE_TA].value & 0x0F); 
				pps0 |= 0x10;
			}
			SMC_RW_PRINTF("pps0%d T %d, FI %d, DI %d \n ", pps0, tp->T, FI, DI);
			tp->first_cwt = tp->cwt =  (960*ATR_DEFAULT_WI*ATR_DEFAULT_F)/(tp->smc_clock/1000);
		
			if(RET_SUCCESS==smc_dev_set_pps(dev, pps0|(tp->T&0xf), FI, DI))
			{
				//osal_semaphore_capture(tp->smc_sema_id, OSAL_WAIT_FOREVER_TIME);
				tp->first_cwt = first_cwt;
				tp->cwt = cwt;
				need_reset_etu = 1;
				//osal_semaphore_release(tp->smc_sema_id);
				SMC_RW_PRINTF("pps: OK, set etu as %d\n", tp->smc_etu);
			}
            else
            {
                SMC_RW_PRINTF("pps: NG, deactive smart card\n");
                smc_dev_deactive(dev);
	            osal_semaphore_release(tp->smc_sema_id);
		        return !RET_SUCCESS;
            }
		}
		
	}
	else
	{	
		SMC_RW_PRINTF("Specific mode!\n");
		if(!((tp->atr_info->ib[1][ATR_INTERFACE_BYTE_TA].value)&0x10))
			need_reset_etu = 1;
	}
	if(need_reset_etu)
	{
		if(tp->T==0)
		{
			UINT8 WI = ATR_DEFAULT_WI;
				
			if (tp->atr_info->ib[2][ATR_INTERFACE_BYTE_TC].present)
				WI = tp->atr_info->ib[2][ATR_INTERFACE_BYTE_TC].value;
			
			OUTPUT_UINT32(ioaddr + REG_CBWTR0, 960*WI*DI);
		}
		else if(tp->T==1)
		{
			UINT8 BWI = ATR_DEFAULT_BWI; 
			if (tp->atr_info->ib[i + 1][ATR_INTERFACE_BYTE_TB].present)
            			BWI = (tp->atr_info->ib[i + 1][ATR_INTERFACE_BYTE_TB].value & 0xF0) >> 4;
			OUTPUT_UINT32(ioaddr + REG_CBWTR0, 11+((960*372*(1<<BWI)*DI)/FI));
		}
		OUTPUT_UINT16(ioaddr + REG_ETU0, tp->smc_etu);	
	}
	if(tp->N == 0xFF)
	{
		if(tp->T == 1)
			OUTPUT_UINT16(ioaddr + REG_GTR0, 11);		//set guart time value.
		else
			OUTPUT_UINT16(ioaddr + REG_GTR0, 12);	
	}
	else
		OUTPUT_UINT16(ioaddr + REG_GTR0, 12+tp->N);	
	osal_task_sleep(10);	
	osal_semaphore_release(tp->smc_sema_id);
	return RET_SUCCESS;
}

/******************************************************************************************************
 * 	Name		:	smc_dev_multi_class_reset()
 *	Description	:	Smart card reset support class selection.
 *	Parameter	:	struct smc_device *dev	: Devcie handle.
 *				UINT8 *buffer		: Read data buffer.
 *				UINT16 *atr_size	: ATR data size.
 *	Return		:	INT32			: SUCCESS or FAIL.
 *
 ******************************************************************************************************/
static INT32 smc_dev_multi_class_reset(struct smc_device *dev, UINT8 *buffer, UINT16 *atr_size)
{
	INT32 i;
	INT32 ret_code = RET_SUCCESS;
	struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
	if(SMC_CLASS_NONE_SELECT == tp->smc_current_select)
	{
		for(i = 0; i<3; i++)
			if(tp->board_supported_class&(1<<i))
			{
				SMC_RW_PRINTF("first class select %c\n", ('A'+i));
				tp->smc_current_select = (enum class_selection)(SMC_CLASS_NONE_SELECT+i+1);
				break;
			}
	}
	i = ((INT32)tp->smc_current_select -SMC_CLASS_NONE_SELECT -1);
	SMC_RW_PRINTF("class select, start from %c:\n", ('A'+i));
	for(; i<3; i++)
		if(tp->board_supported_class&(1<<i))
		{
			tp->smc_current_select = (enum class_selection)(SMC_CLASS_NONE_SELECT+i+1);
			tp->class_select(tp->smc_current_select);
			SMC_RW_PRINTF("class select, try %c: \n", ('A'+i));
			if(RET_SUCCESS==smc_dev_reset(dev, buffer, atr_size))
			{
				SMC_RW_PRINTF("class select, smc reset OK!\n");
				if(0==(tp->smc_supported_class&0x7))
				{
					SMC_RW_PRINTF("SMC has no class indicator!\n");
					break;
				}
				else
				{
					SMC_RW_PRINTF("SMC has class indicator: %02x!\n", (tp->smc_supported_class&0x7));
					if(tp->smc_supported_class&(1<<i))
					{
						SMC_RW_PRINTF("SMC class selection match at %c !\n", ('A'+i));
						break;
					}
				}
			}
			else
			{
				SMC_RW_PRINTF("class select failed!\n");
				smc_dev_deactive(dev);
				//osal_task_sleep(10);
			}
			
		}
	if(i>2)
	{
		tp->smc_current_select = SMC_CLASS_NONE_SELECT;
		ret_code = !RET_SUCCESS;
	}
	return ret_code;
}


/******************************************************************************************************
 * 	Name		:	smc_dev_deactive()
 *	Description	:	Smart card deactive.
 *	Parameter	:	struct smc_device *dev		: Devcie handle
 *	Return		:	INT32				: SUCCESS.
 *
 ******************************************************************************************************/
static INT32 smc_dev_deactive(struct smc_device *dev)
{
	
	UINT32 base_addr = dev->base_addr;
	struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    UINT32 i;
	OUTPUT_UINT8(base_addr + REG_ICCR, INPUT_UINT8(base_addr + REG_ICCR) & (~SMC_RB_ICCR_RST));		// RST L
	osal_delay(/*100*/DATV_RST2CLK);
	OUTPUT_UINT8(base_addr + REG_ICCR, INPUT_UINT8(base_addr + REG_ICCR) &(~SMC_RB_ICCR_CLK));		// CLK L
	osal_delay(/*100*/DATV_CLK2IO);
	OUTPUT_UINT8(base_addr + REG_ICCR, INPUT_UINT8(base_addr + REG_ICCR) &(~SMC_RB_ICCR_DIO));// DIO L
	osal_delay(DATV_IO2VCC>>1);
	if(tp->use_gpio_vpp)
	{	
		if(tp->internal_ctrl_vpp)
			OUTPUT_UINT8(base_addr + REG_CLK_VPP, INPUT_UINT8(base_addr + REG_CLK_VPP)&(~SMC_RB_CTRL_VPP));
		else
			HAL_GPIO_BIT_SET(tp->gpio_vpp_pos, !(tp->gpio_vpp_pol));
		SMC_PRINTF("vpp Low! ");
		//osal_delay(100);
	}
	osal_delay(/*100*/DATV_IO2VCC>>1);
	OUTPUT_UINT8(base_addr + REG_ICCR, INPUT_UINT8(base_addr + REG_ICCR) | SMC_RB_ICCR_VCC);		// VCC Inactive
    for(i=0;i<11;i++)
        osal_delay(1000);
    SMC_PRINTF("%s: over\n", __FUNCTION__);
	tp->reseted = 0;

	return RET_SUCCESS;
}

/******************************************************************************************************
 * 	Name		:   smc_dev_check_transfer_complete()						    
 *	Description	:   To check the transmit whether it is over.
 *	Parameter	:   struct smc_device *dev	: Devcie handle.
 *			    UINT8 *buffer		: Transmit data buffer.
 *			    UINT8 trans_mode		: Direction of transmit.
 *			    UINT8 F_T			: Use the FIFO trigger or TIMEOUT trigger 
 *								to detect transfer complete.
 *			    UINT16 *actsize 		: Actual size of response data.
 *	Return		:   INT32			: SUCCESS or FAIL.
 *
 ******************************************************************************************************/
static INT32 smc_dev_check_transfer_complete(struct smc_device *dev, UINT8 *buffer, UINT8 trans_mode, UINT8 F_T, UINT16 *actsize)
{
	struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
	UINT32 ioaddr = dev->base_addr;

	return RET_SUCCESS;

}


/******************************************************************************************************
 * 	Name		:   smc_dev_transfer_data()						    
 *	Description	:   Send the command or data to the device.
 *	Parameter	:   struct smc_device *dev	: Devcie handle
 *			    UINT8 *buffer		: Wrtie data buffer.
 *			    UINT16 size			: Write data size.
 *			    UINT8 *recv_buffer		: Receive buffer to store the response data from the card.
 *			    UINT16 reply_num		: Response number.
 *	Return		:   INT32			: SUCCESS or FAIL
 *
 ******************************************************************************************************/
static INT32 smc_dev_transfer_data(struct smc_device *dev, UINT8 *buffer, UINT16 size, UINT8 *recv_buffer, UINT16 reply_num, UINT16 *actsize)
{
	SMC_PRINTF("%s: in %d, out %d\n", __FUNCTION__, size, reply_num);
	struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
	UINT32 ioaddr = dev->base_addr;
	UINT16 i, j;
	UINT32 loop=0;
	UINT32 flgptn, waitmo = 6000;
	OSAL_ER result;
	UINT16 real_work_etu = INPUT_UINT16(ioaddr + REG_ETU0);
	UINT16 char_frame = INPUT_UINT16(ioaddr + REG_GTR0);
	
	if(tp->inserted == 0)
	{
		SMC_PRINTF("smc: smart card not inserted!\n");
		return !RET_SUCCESS;
	}
	else if(tp->reseted != 1)
	{
		SMC_PRINTF("smc: smart card not reseted!\n");
		return !RET_SUCCESS;
	}	
	
    	tp->smc_tx_buf = buffer;
    	tp->smc_rx_buf = recv_buffer;
	tp->smc_tx_rd = 0;
	tp->smc_tx_wr = size;
	tp->smc_rx_head = 0;
	tp->got_first_byte = 0;
	if(reply_num)
		tp->smc_rx_tail = reply_num;
	else
		tp->smc_rx_tail = SMC_RX_BUF_SIZE;
    	
	if(size>smc_tx_fifo_size)
		loop = 1;
   
	//SMC_PRINTF("loop %d size %d\n",loop,size);
	//enable transmit mode disable receiver mode
	OUTPUT_UINT8(ioaddr + REG_SCR_CTRL,INPUT_UINT8(ioaddr + REG_SCR_CTRL)&(~(SMC_SCR_CTRL_RECV|SMC_SCR_CTRL_TRANS)));
		
    	OUTPUT_UINT8(ioaddr + REG_IER0,INPUT_UINT8(ioaddr + REG_IER0)&(~(SMC_IER0_BYTE_RECV_TRIG|SMC_IER0_BYTE_TRANS_TRIG)));
	if(reply_num)
	{
		if(reply_num/smc_rx_fifo_size)
		{
			smc_write_rx(ioaddr, (smc_rx_fifo_size>>1));
			//SMC_PRINTF("set rx thld %d\n", 32);
		}
		else
		{
			smc_write_rx(ioaddr,reply_num);	
			//SMC_PRINTF("set rx thld %d\n", reply_num);
		}
	}
	else
	{
		smc_write_rx(ioaddr,0);
		//SMC_PRINTF("set rx thld %d\n", 0);
	}
	/*Always enable byte receive int*/
	OUTPUT_UINT8(ioaddr + REG_IER0,INPUT_UINT8(ioaddr + REG_IER0)|SMC_IER0_BYTE_RECV_TRIG);
	
	OUTPUT_UINT8(ioaddr + REG_FIFO_CTRL, SMC_FIFO_CTRL_EN|SMC_FIFO_CTRL_TX_OP|SMC_FIFO_CTRL_RX_OP);
	if((!tp->force_tx_rx_triger)&&(!tp->auto_tx_rx_triger))
	{
		UINT16 loop_cnt = 0;
		UINT16 loop_remain = 0;
		UINT32 tmo;
		//Enable Fifo Empty Interrupt
		OUTPUT_UINT8(ioaddr + REG_IER0,INPUT_UINT8(ioaddr + REG_IER0)|SMC_IER0_TRANS_FIFO_EMPY);
		if(size>5)
		{
			loop_remain = size - 5;
			loop_cnt = loop_remain/smc_tx_fifo_size;
			loop_remain = loop_remain%smc_tx_fifo_size;
			size = 5;
		}
		osal_flag_clear(tp->smc_flag_id, SMC_RX_BYTE_RCV|SMC_RX_FINISHED|SMC_TX_FINISHED);
		OUTPUT_UINT8(ioaddr + REG_SCR_CTRL,INPUT_UINT8(ioaddr + REG_SCR_CTRL)|SMC_SCR_CTRL_TRANS);
		for(i=0;i<loop_cnt;i++)
		{
			for(j=0;j<smc_tx_fifo_size;j++)
			{
				OUTPUT_UINT8(ioaddr + REG_THR,tp->smc_tx_buf[tp->smc_tx_rd+j]);
			}
			tp->smc_tx_rd += smc_tx_fifo_size;
			tmo = (smc_tx_fifo_size+2)*2;
			while(0==(tp->isr0_interrupt_status&SMC_ISR0_FIFO_EMPTY))
			{
				osal_task_sleep(1);
				tmo--;
				if(0==tmo)
					return !RET_SUCCESS;
				if((INPUT_UINT8(ioaddr + REG_ICCSR) & 0x80)== 0)
				{
					SMC_PRINTF("smc: smart card not inserted!\n");		
					return !RET_SUCCESS;
				}
			}
			tp->isr0_interrupt_status &= ~SMC_ISR0_FIFO_EMPTY;
		}
		if(loop_remain)
		{
			for(j=0;j<loop_remain;j++)
			{
				OUTPUT_UINT8(ioaddr + REG_THR,tp->smc_tx_buf[tp->smc_tx_rd+j]);
			}
			tp->smc_tx_rd += loop_remain;
			tmo = (loop_remain+2)*2;
			while(0==(tp->isr0_interrupt_status&SMC_ISR0_FIFO_EMPTY))
			{
				osal_task_sleep(1);
				tmo--;
				if(0==tmo)
					return !RET_SUCCESS;
				if((INPUT_UINT8(ioaddr + REG_ICCSR) & 0x80)== 0)
				{
					SMC_PRINTF("smc: smart card not inserted!\n");		
					return !RET_SUCCESS;
				}
				
			}
			tp->isr0_interrupt_status &= ~SMC_ISR0_FIFO_EMPTY;
		}
		osal_interrupt_disable();
		for(j=0;j<size;j++)
		{
			OUTPUT_UINT8(ioaddr + REG_THR,tp->smc_tx_buf[tp->smc_tx_rd+j]);
		}
		tp->smc_tx_rd += size;
		tmo = size*2000;
		while(0==(INPUT_UINT8(ioaddr + REG_ISR0)&SMC_ISR0_FIFO_EMPTY))
		{
			osal_delay(1);
			tmo--;
			if(0==tmo)
			{
				osal_interrupt_enable();
				return !RET_SUCCESS;
			}
			if((INPUT_UINT8(ioaddr + REG_ICCSR) & 0x80)== 0)
			{
				SMC_PRINTF("smc: smart card not inserted!\n");	
				osal_interrupt_enable();
				return !RET_SUCCESS;
			}
		}
		OUTPUT_UINT8(ioaddr + REG_ISR0, SMC_ISR0_FIFO_EMPTY);
		OUTPUT_UINT8(ioaddr + REG_SCR_CTRL,(INPUT_UINT8(ioaddr + REG_SCR_CTRL)&(~SMC_SCR_CTRL_TRANS))|SMC_SCR_CTRL_RECV);
		osal_interrupt_enable();	
		tp->hsr(SMC_TX_FINISHED);
	}
	else
	{
		tp->char_frm_dura = char_frame*((real_work_etu*1000)/(tp->smc_clock/1000));
		if(tp->force_tx_rx_triger)
		{
			#define TX_RX_THLD	4000//MS
			tp->force_tx_rx_state = 0;
			/*((thld*char_faram*etu*1000)/(smc_clock/1000))>TX_RX_THLD*/
			tp->force_tx_rx_thld = ((tp->smc_clock/1000)*(TX_RX_THLD/1000))/(real_work_etu*char_frame);
			if(tp->force_tx_rx_thld < 5)
				tp->force_tx_rx_thld = 5;
			//if(tp->force_tx_rx_thld > 10)tp->force_tx_rx_thld = 10;
				
			
			SMC_PRINTF("force tx rx thld %d, char frm dura %d\n", tp->force_tx_rx_thld, tp->char_frm_dura);
		}
		if(loop)
		{
			size = smc_tx_fifo_size;
			if(ALI_S3602==smc_chip_id)
				OUTPUT_UINT8(ioaddr + REG_FIFO_CTRL,(INPUT_UINT8(ioaddr + REG_FIFO_CTRL)&0xf0)|(size>>1));
			else
				smc_write_tx(ioaddr, smc_tx_fifo_size>>1);
		}
		else
		{
			if(tp->force_tx_rx_triger&&size>tp->force_tx_rx_thld)
			{
				smc_write_tx(ioaddr,size-tp->force_tx_rx_thld);
				tp->force_tx_rx_state = 1;
			}
			else
			{
				if(ALI_S3602==smc_chip_id)
					OUTPUT_UINT8(ioaddr + REG_FIFO_CTRL,(INPUT_UINT8(ioaddr + REG_FIFO_CTRL)&0xf0)|(size));
				else
					smc_write_tx(ioaddr,size);
			}
		}
		//SMC_PRINTF("w fc %x\n",INPUT_UINT8(ioaddr + REG_FIFO_CTRL)|SMC_FIFO_CTRL_TX_OP|SMC_FIFO_CTRL_RX_OP|smc_tx_fifo_size);
			
		for(j=0;j<size;j++)
		{
			if((tp->smc_tx_rd+j+1)==tp->smc_tx_wr)
			{
				if(tp->auto_tx_rx_triger)
				{
					OUTPUT_UINT8(ioaddr+REG_ICCSR, 1<<5); //tx->rx auto switch
					SMC_PRINTF("1: tx->rx auto: rd %d, cnt %d, wr %d\n", tp->smc_tx_rd, j, tp->smc_tx_wr);
				}
			}
			OUTPUT_UINT8(ioaddr + REG_THR,buffer[j]);
		}
		tp->smc_tx_rd+=size;
		osal_flag_clear(tp->smc_flag_id, SMC_RX_BYTE_RCV|SMC_RX_FINISHED|SMC_TX_FINISHED);
		
	       OUTPUT_UINT8(ioaddr + REG_IER0,INPUT_UINT8(ioaddr + REG_IER0)|SMC_ISR0_FIFO_TRANS);	
	
		osal_interrupt_disable();
		OUTPUT_UINT8(ioaddr + REG_SCR_CTRL,INPUT_UINT8(ioaddr + REG_SCR_CTRL)|SMC_SCR_CTRL_TRANS);
		if(0)//3==tp->smc_tx_wr)
		{
	            while(0!=smc_read_tx(ioaddr))
	                osal_delay(10);
	            OUTPUT_UINT8(ioaddr + REG_SCR_CTRL, (INPUT_UINT8(ioaddr + REG_SCR_CTRL)|(1<<4)));
	            while(0==(INPUT_UINT8(ioaddr + REG_ISR0)&SMC_ISR0_PE_RECV))
	                osal_delay(10);
	            OUTPUT_UINT8(ioaddr + REG_SCR_CTRL, (INPUT_UINT8(ioaddr + REG_SCR_CTRL)&(~(1<<4))));
		}
		if(tp->force_tx_rx_triger)
		{	
			if((0==loop)&&(0==tp->force_tx_rx_state))
			{
				if(0==smc_read_tx(ioaddr))
				{	
					OUTPUT_UINT8(ioaddr + REG_SCR_CTRL,(INPUT_UINT8(ioaddr + REG_SCR_CTRL)&(~SMC_SCR_CTRL_TRANS))|SMC_SCR_CTRL_RECV);
					SMC_PRINTF("tmo1: too late!\n");
				}
				else
				{
					UINT32 force_loop_tmo = (smc_read_tx(ioaddr)+1)*tp->char_frm_dura;
					UINT8   force_tx_rx = 1;
				
					//libc_printf("tmo1 rdy: %d\n", force_loop_tmo);
					UINT32 tmo = read_tsc();
					//if(0==smc_read_tx(ioaddr))libc_libc_printf("tmo1: too late!\n");
					
					while(0!=smc_read_tx(ioaddr))
					{
						osal_delay(1);
						force_loop_tmo--;
						if(!force_loop_tmo)
						{
							force_tx_rx = 0;
							SMC_PRINTF("tmo1: tmo %d\n", smc_read_tx(ioaddr));
							//libc_libc_printf("tmo1: tmo %d\n", smc_read_tx(ioaddr));
							break;
						}
						if((INPUT_UINT8(ioaddr + REG_ICCSR) & 0x80)== 0)
						{
							SMC_PRINTF("smc: smart card not inserted!\n");
							osal_interrupt_enable();
							return !RET_SUCCESS;
						}
					}
					if(force_tx_rx)
					{
						OUTPUT_UINT8(ioaddr + REG_SCR_CTRL,(INPUT_UINT8(ioaddr + REG_SCR_CTRL)&(~SMC_SCR_CTRL_TRANS))|SMC_SCR_CTRL_RECV);
					}
					SMC_PRINTF("tmo1: %d, %d\n", force_loop_tmo, (read_tsc()-tmo)/(SYS_CPU_CLOCK / 2000000));
					//libc_printf("tmo1: %d, %d\n", force_loop_tmo, (read_tsc()-tmo)/(SYS_CPU_CLOCK / 2000000));
				}
				osal_interrupt_enable();
			}
			else
				osal_interrupt_enable();   
		}
		else
			osal_interrupt_enable();   
	}
       //SMC_PRINTF("set to trans\n");

	//SMC_PRINTF("w scr %x\n",INPUT_UINT8(ioaddr + REG_SCR_CTRL)|SMC_SCR_CTRL_TRANS);
	//Wait TX ready:
	waitmo = ((tp->smc_tx_wr+1)*tp->char_frm_dura*3)/1000 + 1;
	
	flgptn = 0;
	
	//SMC_PRINTF("%s wat tx rdy with tmo %d\n", __FUNCTION__, waitmo);
	result = osal_flag_wait(&flgptn, tp->smc_flag_id, SMC_REMOVED|SMC_TX_FINISHED, OSAL_TWF_ORW, waitmo);
	
	if(OSAL_E_OK!=result)
	{
		if(tp->smc_tx_wr)
        	{
        		osal_interrupt_disable();
			if(tp->smc_tx_rd==tp->smc_tx_wr)
			{
				if(tp->force_tx_rx_triger&&1==tp->force_tx_rx_state)
				{
					smc_write_tx(ioaddr, tp->force_tx_rx_thld);
					tp->force_tx_rx_state = 2;					
				}
			}
			else
			{
				SMC_PRINTF("tx not finished: w %d: r %d!\n", tp->smc_tx_wr, tp->smc_tx_rd);
				tp->smc_tx_rd=tp->smc_tx_wr;
				if(tp->force_tx_rx_triger)
					tp->force_tx_rx_state = 2;	
				osal_interrupt_enable();
				return !RET_SUCCESS;
			}
			osal_interrupt_enable();
		}	
	}
	
	//SMC_PRINTF("wt tx tmo %d, rdy %d, flgptn %08x\n", (result!=OSAL_E_OK), (0!=(flgptn&SMC_TX_FINISHED)), flgptn);
	if(tp->isr0_interrupt_status&SMC_ISR0_PE_TRANS)
	{
		SMC_PRINTF("TX Parity Error!\n");
		tp->isr0_interrupt_status &= (~SMC_ISR0_PE_TRANS);
	}
	if(tp->smc_rx_tail&&tp->smc_rx_buf!=tp->smc_rx_tmp_buf&&(flgptn&SMC_TX_FINISHED)&&(!(flgptn&SMC_REMOVED)))
	{
		flgptn = 0;
		waitmo = tp->first_cwt;
		//SMC_PRINTF("%s wait 1st byte %d ms\n", __FUNCTION__, waitmo);        
		result = osal_flag_wait(&flgptn, tp->smc_flag_id, SMC_REMOVED|SMC_RX_FINISHED|SMC_RX_BYTE_RCV, OSAL_TWF_ORW, waitmo);
		if(OSAL_E_OK!=result)
		{
			*actsize = tp->smc_rx_head;
			SMC_PRINTF("%s wait 1st byte TMO with %d\n", __FUNCTION__,*actsize);  
			osal_flag_clear(tp->smc_flag_id, SMC_RX_FINISHED|SMC_TX_FINISHED|SMC_RX_BYTE_RCV);
			smc_dev_clear_tx_rx_buf(tp);
			//osal_semaphore_release(tp->smc_sema_id);
			return !RET_SUCCESS;
		}
		if(flgptn&SMC_RX_BYTE_RCV)
		{
			UINT16 current_cnt = smc_read_rx(ioaddr);
			UINT32 current_head = tp->smc_rx_head;
			
			osal_flag_clear(tp->smc_flag_id, SMC_RX_BYTE_RCV);
			waitmo = tp->cwt;
			//waitmo = (waitmo>4000?waitmo: 4000);
			//SMC_PRINTF("%s wait %d bytes: %d ms\n", __FUNCTION__, (tp->smc_rx_tail - 1), waitmo);
			do
			{
				UINT16 tmp_cnt;
				flgptn = 0;
				result = osal_flag_wait(&flgptn, tp->smc_flag_id, SMC_REMOVED|SMC_RX_FINISHED, OSAL_TWF_ORW, waitmo);
				if(result == OSAL_E_OK)		
					break;
				
				tmp_cnt = smc_read_rx(ioaddr);
				//SMC_PRINTF("loop wat: prv c %d, h %d: cur c%d, h %d\n", current_cnt, current_head, tmp_cnt, tp->smc_rx_head);
				if(tmp_cnt!=current_cnt||current_head!=tp->smc_rx_head)
				{
					current_cnt = tmp_cnt;
					current_head=tp->smc_rx_head;
					result = OSAL_E_OK;
				}
			}while(result!=OSAL_E_TIMEOUT);
		}
	}
	
	if(OSAL_E_OK!=result)
	{
		if(reply_num)
		{
			if(tp->smc_rx_tail)
			{	
				UINT32 rem_space;
				UINT32 i;
				UINT16 c, total_cnt;
				//<== patch for miss tx finish flag
				UINT32  cur_rx_head, smc_rd_tmo, tmp_rd_tick;
				cur_rx_head = tp->smc_rx_head;
				if(cur_rx_head)
					smc_rd_tmo = tp->cwt;
				else
					smc_rd_tmo = tp->first_cwt;
				tmp_rd_tick= osal_get_tick();
				while(tp->smc_rx_head<tp->smc_rx_tail )
				{
					if(tp->inserted == 0)
					{
						SMC_PRINTF("smc: smart card not inserted!\n");
						return !RET_SUCCESS;
					}
					osal_task_sleep(1);
					if(cur_rx_head!=tp->smc_rx_head)
					{
						cur_rx_head = tp->smc_rx_head;
						smc_rd_tmo = tp->cwt;
						tmp_rd_tick = osal_get_tick();
					}
					if(smc_rd_tmo<(osal_get_tick() - tmp_rd_tick))
					{	
						smc_rd_tmo = 0;
						break;
					}
				}
				//==>Patch By Goliath
				osal_interrupt_disable();
				rem_space = tp->smc_rx_tail - tp->smc_rx_head;
				total_cnt = c =smc_read_rx(ioaddr);
				c = (c<=rem_space?c:rem_space);			
				total_cnt -= c;
				for(i=0;i<c;i++)
					tp->smc_rx_buf[tp->smc_rx_head+i] = INPUT_UINT8(ioaddr + REG_RBR);
				tp->smc_rx_head += c; 
				osal_interrupt_enable();
				for(i=0; i<total_cnt; i++)		
					c = INPUT_UINT8(ioaddr + REG_RBR);
				SMC_PRINTF("%s TMO with %d, rem %d \n", __FUNCTION__, tp->smc_rx_head, total_cnt);
			}
			*actsize = tp->smc_rx_head;
		
        		if(tp->smc_rx_head)
        		{
        			smc_dev_clear_tx_rx_buf(tp);
        			//osal_semaphore_release(tp->smc_sema_id);
            			return RET_SUCCESS;
        		}
			SMC_PRINTF("%s Failed \n", __FUNCTION__); 	
			smc_dev_clear_tx_rx_buf(tp);	
			//osal_semaphore_release(tp->smc_sema_id);	
			return !RET_SUCCESS;
		}
		else
		{
			if(tp->smc_tx_rd!=tp->smc_tx_wr)
			{
				*actsize = tp->smc_tx_rd;
				SMC_PRINTF("tx tmo %d: w %d: r %d!\n", waitmo, tp->smc_tx_wr, tp->smc_tx_rd);
				return !RET_SUCCESS;
			}
		}
		
	}
	if(flgptn&SMC_REMOVED)
	{
		osal_flag_clear(tp->smc_flag_id, SMC_REMOVED);
		*actsize = 0;
		SMC_PRINTF("%s CARD removed!\n\n", __FUNCTION__);
		smc_dev_clear_tx_rx_buf(tp);
		//osal_semaphore_release(tp->smc_sema_id);
		return !RET_SUCCESS;
	}
	if(reply_num)
		*actsize = tp->smc_rx_head;
	else
		*actsize = tp->smc_tx_rd;
	SMC_PRINTF("%s SUCCESS with %d\n\n", __FUNCTION__, *actsize);
	osal_interrupt_disable();
	tp->smc_tx_buf = NULL;
    //	tp->smc_rx_buf = NULL;
	tp->smc_tx_rd = 0;
	tp->smc_tx_wr = 0;
	osal_interrupt_enable();
	//tp->smc_rx_head = 0;
	//tp->got_first_byte = 0;
	//tp->smc_rx_tail = 0;
	//smc_dev_clear_tx_rx_buf(tp);
	//osal_semaphore_release(tp->smc_sema_id);
	return RET_SUCCESS;
}

/******************************************************************************************************
 * 	Name		:   smc_dev_read()
 *	Description	:   UART smartcard read.
 *	Parameter	:	struct smc_device *dev	: Devcie handle
 *					UINT8 *buffer			: Read data buffer
 *					INT16 size				: Read data size
 *					INT16 *actsize			: Read data size
 *	Return		:	INT32					: return value
 *
 ******************************************************************************************************/
static INT32 smc_dev_read(struct smc_device *dev, UINT8 *buffer, INT16 size, INT16 *actsize)
{
	struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
	UINT32 smc_rd_tmo = tp->first_cwt;
	UINT32 rd_tick = osal_get_tick();
	UINT32 cur_rx_head;
	UINT32 tmp_rd_tick= osal_get_tick();
	//SMC_PRINTF("%s\n", __FUNCTION__);
	osal_semaphore_capture(tp->smc_sema_id, OSAL_WAIT_FOREVER_TIME);	
	if(smc_read_rx(dev->base_addr))
	{
		osal_interrupt_disable();
		UINT32 rem_space = tp->smc_rx_tail - tp->smc_rx_head;
		UINT32 i;
		UINT16 c, total_cnt;

		total_cnt = c =smc_read_rx(dev->base_addr);
		c = (c<=rem_space?c:rem_space);			
		total_cnt -= c;
		for(i=0;i<c;i++)
			tp->smc_rx_buf[tp->smc_rx_head+i] = INPUT_UINT8(dev->base_addr + REG_RBR);
		tp->smc_rx_head += c; 
		osal_interrupt_enable();
	}
	cur_rx_head = tp->smc_rx_head;
	if(cur_rx_head)
		smc_rd_tmo = tp->cwt;
	while(tp->smc_rx_head<((UINT32)size))
	{
		if(tp->inserted == 0)
		{
			SMC_PRINTF("smc: smart card not inserted!\n");
			osal_semaphore_release(tp->smc_sema_id);
			return SMART_NOT_INSERTED;
		}
		else if(tp->reseted != 1)
		{
			SMC_PRINTF("smc: smart card not reseted!\n");
			osal_semaphore_release(tp->smc_sema_id);
			return SMART_NOT_RESETED;
		}	
		osal_task_sleep(1);
		
		
		/*	
		smc_rd_tmo--;
		if(!smc_rd_tmo)
			break;
		*/	
		if(cur_rx_head!=tp->smc_rx_head)
		{
			cur_rx_head = tp->smc_rx_head;
			smc_rd_tmo = tp->cwt;
			tmp_rd_tick = osal_get_tick();
		}
		if(smc_rd_tmo<(osal_get_tick() - tmp_rd_tick))
		{	
			//libc_printf("tmo: %d, size: %d\n", smc_rd_tmo, size);
			smc_rd_tmo = 0;
			break;
		}
	}
	
	osal_interrupt_disable();
	if(((UINT32)size)>tp->smc_rx_head)
		size = tp->smc_rx_head;
	//if(tp->smc_rx_tmp_buf[0]==0x60)ASSERT(0);
		
	MEMCPY(buffer, (const void *)tp->smc_rx_tmp_buf, size);
	*actsize = size;
	tp->smc_rx_head -= (UINT32)size;
	if(tp->smc_rx_head)
		MEMCPY((void *)tp->smc_rx_tmp_buf, (const void *)(tp->smc_rx_tmp_buf+size), tp->smc_rx_head);
	osal_interrupt_enable();
	if(0==smc_rd_tmo&&0==(*actsize))
	{
		SMC_PRINTF("smc: read %d bytes TMO with %d!\n", size, osal_get_tick() - rd_tick);
		osal_semaphore_release(tp->smc_sema_id);
		return SMART_NO_ANSWER;
	}
	#ifdef SMC_RW_DEBUG
		INT16 len = *actsize;
		INT16 k;
		SMC_RW_PRINTF("%d\t \tR: ", size);
		size = *actsize;
		if(len>26)
			len = 26;
		
		for(k=0; k<len; k++)
			SMC_RW_PRINTF("%02x ", buffer[k]);
		if(size>26)
			SMC_RW_PRINTF(". . . %02x %02x %02x %02x ", buffer[size-4],buffer[size-3], buffer[size-2], buffer[size-1]);
		SMC_RW_PRINTF("\n");
	#endif
	SMC_PRINTF("read %d bytes with %d\n", *actsize, osal_get_tick() - rd_tick);
	osal_semaphore_release(tp->smc_sema_id);
	return RET_SUCCESS;
}

/******************************************************************************************************
 * 	Name		:   smc_dev_write()
 *	Description	:   UART CAM write.
 *	Parameter	:	struct smc_device *dev	: Devcie handle
 *					UINT8 *buffer			: Wrtie data buffer
 *					UINT16 size				: Write data size
 *					UINT16 *actsize			: Write data size
 *	Return		:	INT32					: return value
 *
 ******************************************************************************************************/

static INT32 smc_dev_write(struct smc_device *dev, UINT8 *buffer, INT16 size, INT16 *actsize)
{
	UINT32 wr_tick = osal_get_tick();
//	SMC_PRINTF("%s\n", __FUNCTION__);
	struct smartcard_private * tp = (struct smartcard_private * )(dev->priv);
	INT32 rlt; 
	osal_task_sleep(3);//Patch for S3602F
	osal_semaphore_capture(tp->smc_sema_id, OSAL_WAIT_FOREVER_TIME);	
	#ifdef SMC_RW_DEBUG
		SMC_RW_PRINTF("%d\n", osal_get_tick()-wr_tick);
		INT16 len = size;
		INT16 k;
		if(len>5)
			len = 5;
		SMC_RW_PRINTF("%d\t \tW: ", size);
		for(k=0; k<len; k++)
			SMC_RW_PRINTF("%02x ", buffer[k]);
		if(size>5)
			SMC_RW_PRINTF(". . . %02x %02x %02x %02x ", buffer[size-4], buffer[size-3], buffer[size-2], buffer[size-1]);
		SMC_RW_PRINTF("\n");
	#endif
	rlt = smc_dev_transfer_data(dev, buffer, size, (UINT8 *)tp->smc_rx_tmp_buf, 0, actsize);
	osal_semaphore_release(tp->smc_sema_id);
	SMC_PRINTF("%s OVER: %d\n", __FUNCTION__, osal_get_tick() - wr_tick);
	return rlt;
}

/******************************************************************************************************
 * 	Name		:   smc_dev_transfer_data()						    
 *	Description	:   After command send to analyze the reply data.
 *	Parameter	:   struct smc_device *dev	: Devcie handle.
 *			    UINT8 INS			: The instruction code.
 *			    INT16 num_to_transfer	: Transmit data size
 *			    UINT8 *status		: Status buffer pointer.
 *			    INT16 size			: Response size.
 *	Return		:   INT32			: SUCCESS or FAIL
 *
 ******************************************************************************************************/
static INT32 smc_process_procedure_bytes(struct smc_device *dev, UINT8 INS, INT16 num_to_transfer, UINT8 status[2])
{
	INT16 r = 0;
	INT16 act_size;
	UINT8 buff;
	SMC_PRINTF("iso: <- PROC:\n ");
	do
	{
		do
		{
			if(smc_dev_read(dev, &buff, 1, &act_size) != RET_SUCCESS) 
			{
				SMC_PRINTF("smc pcss: 1 read error!\n");
				return -1;
			}
			SMC_PRINTF("%02x ", buff);
		} while(buff == 0x60);	/*NULL, send by the card to reset WWT*/

		if((buff&0xF0)==0x60 || (buff&0xF0)==0x90)	 // SW1/SW2
		{
			status[0]=buff;
			if(smc_dev_read(dev, &buff, 1, &act_size) != RET_SUCCESS)
			{
				SMC_PRINTF("smc pcss: 2 read error!\n");
				return -1;
			}
			SMC_PRINTF("%02x\n", buff);
			status[1] = buff;
			return 0;
		}
		else
		{
			if((buff ^ INS) == 0)	/* ACK == INS*/
			{
				/*Vpp is idle. All remaining bytes are transfered subsequently.*/
				r = num_to_transfer;
			}
			else if((buff ^ INS) == 0xFF)		/* ACK == ~INS*/
			{
				/*Vpp is idle. Next byte is transfered subsequently.*/
				r = 1;
			}
			else if((buff ^ INS) == 0x01)	/* ACK == INS+1*/
			{
				/*Vpp is active. All remaining bytes are transfered subsequently.*/
				r = num_to_transfer;
			}
			else if((buff ^ INS) == 0xFE)	/* ACK == ~INS+1*/
			{
				/*Vpp is active. Next bytes is transfered subsequently.*/
				r = 1;
			}
//###########################################################			
//seca exceptions
			else if(((buff ^ INS) == 0x3C)||((buff ^ INS) == 0x40))	
			{
				r = num_to_transfer;
			}

//###########################################################			
			else
			{
				SMC_PRINTF("cannot handle procedure %02x (INS=%02x)\n", buff, INS);
				return -1;
			}
			if(r > num_to_transfer)
			{
				SMC_PRINTF("data overrun r=%d num_to_transfer=%d\n", r, num_to_transfer);
				return -1;

			}
		}
	} while(r==0);
	return r;
}

/******************************************************************************************************
 * 	Name		:   	smc_dev_iso_transfer()
 *	Description	:	Combines the functionality of both write and read.
 *					Implement ISO7816-3 command transfer. 
 *	Parameter	:	struct smc_device *dev		: Device structuer.
 *					UINT8 *command			: ISO7816 command buffer pointer.
 *					INT16 num_to_write		: Number to transmit.
 *					UINT8 *response, 			: Response data buffer pointer.
 *					INT16 num_to_read		: number to read from SMC, 0 means it's write cmd
 *					INT16 *actual_size			: pointer to the actual size received from SMC
 *	Return		:	INT32 					: SUCCESS or FAIL.
 ******************************************************************************************************/
 #define MAX_LENGTH	256	//max response length
 #define CMD_LENGTH	5
 #define CLA_OFFSET	0
 #define INS_OFFSET	1
 #define P1_OFFSET	2
 #define P2_OFFSET	3
 #define P3_OFFSET	4

static INT32 smc_dev_iso_transfer(struct smc_device *dev, UINT8 *command, INT16 num_to_write, 
						UINT8 *response, INT16 num_to_read, INT16 *actual_size)
{
	INT32 ret;
	INT32 error = SMART_NO_ERROR;
	UINT8 write_flag = 0;
	INT16 size;
	INT16 num_to_transfer;
	INT16 length = 0;
	INT16 temp_length = 0;
	UINT8 status[2] = {0,0};

	struct smartcard_private *tp = (struct smartcard_private *)dev->priv;

	if(tp->inserted == 0)
	{
		SMC_PRINTF("%s: smart card not inserted!\n",__FUNCTION__);
		return !RET_SUCCESS;
	}
	else if(tp->reseted != 1)
	{
		SMC_PRINTF("%s: smart card not reseted!\n",__FUNCTION__);
		return !RET_SUCCESS;
	}

	if(tp->T > 1)
		return SMART_INVALID_PROTOCOL;

	SMC_PRINTF("CMD: IFD -> SMC ");
	SMC_DUMP(command, CMD_LENGTH);

	*actual_size = 0;
	
	if(num_to_write > CMD_LENGTH)
	{
		write_flag = 1;
		num_to_transfer = command[P3_OFFSET];
		//ASSERT(num_to_transfer == (num_to_write - CMD_LENGTH))
	}
	else if(num_to_write == CMD_LENGTH)
	{
		write_flag = 0;
		//if(response == NULL) 	/* write data to smart card*/
		if(num_to_read==0)
			num_to_transfer = 0;
		else		/*read data from smart card*/
			num_to_transfer = (command[P3_OFFSET] == 0) ? MAX_LENGTH : command[P3_OFFSET];
	}
	else
	{
		SMC_PRINTF("%s: error command length!\n", __FUNCTION__);
		return !RET_SUCCESS;
	}

	SMC_PRINTF("%s:write_flag = %d\n", __FUNCTION__, write_flag);

	/* Check the CLA and INS bytes are valid */
	if (command[CLA_OFFSET] != 0xFF)
	{
		if ((command[INS_OFFSET] & 0xF0) != 0x60 &&
			(command[INS_OFFSET] & 0xF0) != 0x90)
		{
			if(smc_dev_write(dev, command, CMD_LENGTH, &size) != SUCCESS)
			{
				SMC_PRINTF("%s: 1 write cmd error!\n", __FUNCTION__);
				return !RET_SUCCESS;
			}
			
			length = 0;
			while(1)
			{
				temp_length = smc_process_procedure_bytes(dev, command[INS_OFFSET], num_to_transfer - length, status);
				if(temp_length == 0)
				{
					if (NULL != response)
					{
						response[*actual_size] = status[0];
						response[(*actual_size)+1] = status[1];
						*actual_size += 2;
					}
					return RET_SUCCESS;
				}
				else if(temp_length < 0)
				{
					SMC_PRINTF("%s: procedure return error! CMD is:\n", __FUNCTION__);
					SMC_DUMP(command, num_to_write);
					return !RET_SUCCESS;
				}
				
				if(write_flag == 1)
				{
					if(smc_dev_write(dev, command + CMD_LENGTH + length, temp_length, &size) != RET_SUCCESS) 
					{
						SMC_PRINTF("%s: 2 write data error!\n", __FUNCTION__);
						return !RET_SUCCESS;
					}
					SMC_PRINTF("DATA: IFD -> SMC: ");
					SMC_DUMP(command + CMD_LENGTH + length, temp_length);
				}
				else
				{
					if(smc_dev_read(dev, response + length, temp_length, &size) != RET_SUCCESS)
					{
						SMC_PRINTF("%s: data read error!\n", __FUNCTION__);
						return !RET_SUCCESS;
					}
					*actual_size += temp_length;
					SMC_PRINTF("DATA: IFD <- SMC, ");
					SMC_DUMP(response + length, temp_length);
				}
				length += temp_length;
			}
		}
		else
		{
			/* INS is invalid */
			error = SMART_INVALID_CODE;
		}
	}
	else
	{
		/* CLA is invalid */
		error = SMART_INVALID_CLASS;
	}
	return error;
}
static INT32 smc_dev_iso_transfer_t1(struct smc_device *dev, UINT8 *command, INT16 num_to_write, 
						UINT8 *response, INT16 num_to_read,INT32 *actual_size)
{
	INT32 ret;
	INT32 error = RET_SUCCESS;
	UINT8 write_to_smartcard = 0;
	INT16 size;
	INT16 num_to_transfer;
	INT16 length = 0;
	INT16 temp_length = 0;

	struct smartcard_private *tp = (struct smartcard_private *)dev->priv;

	if(tp->inserted == 0)
	{
		SMC_PRINTF("smc: smart card not inserted!\n");
		return !RET_SUCCESS;
	}
	else if(tp->reseted != 1)
	{
		SMC_PRINTF("smc: smart card not reseted!\n");
		return !RET_SUCCESS;
	}

	osal_semaphore_capture(tp->smc_sema_id, OSAL_WAIT_FOREVER_TIME);	

	if(smc_dev_transfer_data(dev, command, num_to_write, response, num_to_read, &size) != SUCCESS)
	{
		osal_semaphore_release(tp->smc_sema_id);
		return !RET_SUCCESS;
	}
	//SMC_DUMP(response,size);
	*actual_size = size;
    	osal_semaphore_release(tp->smc_sema_id);
	return error;
}
/******************************************************************************************************
 * 	Name		:	smc_dev_ioctl()
 *	Description	:	Smart card reader IO control function.
 *	Parameter	:	struct smc_device *dev		: Devcie handle
 *				INT32 cmd			: IO command
 *				UINT32 param			: Command parameter
 *	Return		:	INT32				: return value
 *
 ******************************************************************************************************/
static INT32 smc_dev_ioctl(struct smc_device *dev, INT32 cmd, UINT32 param)
{
	INT32 ret_code = RET_SUCCESS;
	struct smartcard_private *tp = (struct smartcard_private *)dev->priv;

	switch (cmd)
	{
		case SMC_DRIVER_SET_IO_ONOFF:
			if(param == SMC_IO_ON)
			{
				smc_dev_enable();
			}
			else if(param == SMC_IO_OFF)
			{
				smc_dev_disable();
			}
			break;
		case SMC_DRIVER_CHECK_STATUS:
			if(tp->inserted == 0)
				*(UINT8 *) param = SMC_STATUS_NOT_EXIST;
			else if(tp->reseted == 0)
				*(UINT8 *) param = SMC_STATUS_NOT_RESET;
			else
				*(UINT8 *) param = SMC_STATUS_OK;
			break;
		case SMC_DRIVER_SET_WWT:
			osal_semaphore_capture(tp->smc_sema_id, OSAL_WAIT_FOREVER_TIME);
			tp->first_cwt = param;
			osal_semaphore_release(tp->smc_sema_id);
			break;
		case SMC_DRIVER_SET_CWT:
			osal_semaphore_capture(tp->smc_sema_id, OSAL_WAIT_FOREVER_TIME);
			tp->cwt = param;
			osal_semaphore_release(tp->smc_sema_id);
			break;
		case SMC_DRIVER_SET_ETU:
			osal_semaphore_capture(tp->smc_sema_id, OSAL_WAIT_FOREVER_TIME);
			smc_set_etu(dev, param);
			osal_semaphore_release(tp->smc_sema_id);
			break;
		case SMC_DRIVER_GET_F:
			osal_semaphore_capture(tp->smc_sema_id, OSAL_WAIT_FOREVER_TIME);
			if(!tp->reseted)
				ret_code = !RET_SUCCESS;
			else
				*((UINT32 *)param) = tp->F;
			osal_semaphore_release(tp->smc_sema_id);
			break;
		case SMC_DRIVER_GET_D:
			osal_semaphore_capture(tp->smc_sema_id, OSAL_WAIT_FOREVER_TIME);
			if(!tp->reseted)
				ret_code = !RET_SUCCESS;
			else
				*((UINT32 *)param) = tp->D;
			osal_semaphore_release(tp->smc_sema_id);
			break;	
		case SMC_DRIVER_GET_ATR_RESULT:
			osal_semaphore_capture(tp->smc_sema_id, OSAL_WAIT_FOREVER_TIME);
			*((enum smc_atr_result *)param) = tp->atr_rlt;
			osal_semaphore_release(tp->smc_sema_id);
			break;
		case SMC_DRIVER_GET_PROTOCOL:
			osal_semaphore_capture(tp->smc_sema_id, OSAL_WAIT_FOREVER_TIME);
			if(tp->inserted&&tp->reseted)
				*((UINT32 *)param) = tp->T;
			else 
				*((UINT32 *)param) = 0xffffffff;
			osal_semaphore_release(tp->smc_sema_id);	
			break;
		case SMC_DRIVER_GET_HB:
			osal_semaphore_capture(tp->smc_sema_id, OSAL_WAIT_FOREVER_TIME);
			{
				UINT8 len = tp->atr_info->hbn;
				struct smc_hb_t * p = (struct smc_hb_t *)param;
				if(len>(sizeof(struct smc_hb_t)-1))
					len = sizeof(struct smc_hb_t)-1;
				MEMCPY((void *)p->hb, tp->atr_info->hb, len);
				p->hbn = len;
			}
			osal_semaphore_release(tp->smc_sema_id);
			break;
		case SMC_DRIVER_SET_WCLK:
			osal_semaphore_capture(tp->smc_sema_id, OSAL_WAIT_FOREVER_TIME);
			tp->init_clk_array[tp->init_clk_idx] = param;
			osal_semaphore_release(tp->smc_sema_id);
			break;
		case SMC_DRIVER_GET_CLASS:
			osal_semaphore_capture(tp->smc_sema_id, OSAL_WAIT_FOREVER_TIME);
			if((tp->class_selection_supported)&&(!tp->reseted))
				*((enum class_selection *)param) = tp->smc_current_select;
			else
				ret_code = !RET_SUCCESS;
			osal_semaphore_release(tp->smc_sema_id);
			break;
		case SMC_DRIVER_SET_CLASS:
			osal_semaphore_capture(tp->smc_sema_id, OSAL_WAIT_FOREVER_TIME);
			if((tp->class_selection_supported)&&(!tp->reseted))
				tp->smc_current_select = (enum class_selection )param;
			else
				ret_code = !RET_SUCCESS;
			osal_semaphore_release(tp->smc_sema_id);
			break;	
		default:
			break;
	}

	return ret_code;
}


