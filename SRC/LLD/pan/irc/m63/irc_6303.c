/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2002 Copyright (C)
 *
 *  File: ir6311.c
 *
 *  Description: This file implements the Infra Receiver driver for M6303
 *               by using NEC standard IRC algorithm library.
 *  History:
 *      Date        Author      Version  Comment
 *      ====        ======      =======  =======
 *  1.  2003.1.11   Liu Lan     0.1.000  Initial
 *  2.  2003.2.20   Liu Lan     0.1.001  Tick Configure
 *  3.  2003.6.13   Justin Wu   0.1.002  Stability enhance
 *  4.  2003.7.5    Justin Wu   0.2.000  Update to new algorithm
 *  5.  2004.8.11   Justin Wu   0.2.001  Support new RLC IP (M3327/57)
 *  6.  2011.02.12  ryan.chen  0.2.002  Add RC6 decoder 
 ****************************************************************************/

#include <sys_config.h>

#include <types.h>
#include <api/libc/printf.h>
#include <hld/pan/pan.h>
#include <osal/osal.h>


#define IR_USE_HSR					1

#define IR_RLC_SIZE					256

#if (SYS_CHIP_MODULE == ALI_S3601||SYS_CHIP_MODULE == ALI_S3602)
	#if((SYS_CPU_MODULE == CPU_M6303) && (SYS_PROJECT_FE==PROJECT_FE_DVBT))//for 3105
		#define IOBASE				0xB8001100
	#else
		#define IOBASE				0xB8018100
	#endif
#else
	#define IOBASE				0xB8001100
#endif
#define INFRA_IRCCFG				(IOBASE + 0x00)
#define INFRA_FIFOCTRL				(IOBASE + 0x01)
#define INFRA_TIMETHR				(IOBASE + 0x02)
#define INFRA_NOISETHR				(IOBASE + 0x03)
#define INFRA_IER					(IOBASE + 0x06)
#define INFRA_ISR					(IOBASE + 0x07)
#define INFRA_RLCBYTE				(IOBASE + 0x08)

#define READ_INF_BYTE(addr)			(*(volatile unsigned char *)addr)
#define WRITE_INF_BYTE(addr,data)	(*(volatile unsigned char *)addr = data)

static UINT16 bufferin = 0;
static UINT16 bufferout = 0;
static UINT8 infra_buffer[IR_RLC_SIZE];
static UINT8 key_state = PAN_KEY_RELEASE;
static UINT32 last_act_code = PAN_KEY_INVALID;
UINT16 key_cnt = 0;
static UINT32 ms_tick;
static struct pan_hw_info *local_hw_info;
static UINT32 ir_repeat_width;

static void irc_m6303irc_lsr(UINT32 param);
#if (((SYS_SDRAM_SIZE != 2)  && (SYS_PROJECT_FE != PROJECT_FE_ATSC)) || (PORTING_ATSC == 1))
static void irc_m6303irc_count(UINT32 param);
#endif
/*****************************************************************************
 * void irc_m6303irc_init()
 * Description: M6303 IR controler init
 *
 * Arguments:
 *
 * Return Value:
 ****************************************************************************/
#define VALUE_CLK_CYC		8		/* Work clock cycle, in uS */
#ifndef NEW_IR_DRIVER
#define VALUE_TOUT			24000	/* Timeout threshold, in uS */
#else
#define VALUE_TOUT			18000	/* Timeout threshold, in uS */
#endif
#define VALUE_NOISETHR		80		/* Noise threshold, in uS */

void irc_m6303irc_init(struct pan_hw_info *hw_info)
{
    /* Initialize driver internal variables */
    local_hw_info = hw_info;
    ir_repeat_width = local_hw_info->intv_repeat_first;
    bufferin = bufferout = 0;

#ifndef NEW_IR_DRIVER
	/* Reset internal state, precision is 280uS, repeat timeout is 500mS */
#if (((SYS_SDRAM_SIZE != 2)  && (SYS_PROJECT_FE != PROJECT_FE_ATSC)) || (PORTING_ATSC == 1))
	if(!sys_ic_is_M3101())
		irc_LAB_mode_set(0, 70, 0);
#endif
	irc_NEC_mode_set(0, 280, 500000);
	//irc_RC6_mode_set(0, 222, 500000);
#endif

    WRITE_INF_BYTE(INFRA_IRCCFG, 0);
    /* Working clock expressions:
     * (SB_CLK / (32 * CLK_SEL)) = 1 / VALUE_CLK_CYC, SB_CLK = 12MHz
     * => CLK_SEL = (SB_CLK * VALUE_CLK_CYC / 32)
     */
    WRITE_INF_BYTE(INFRA_IRCCFG, 0x80 | ((12 * VALUE_CLK_CYC) >> 5));

    /* FIFO threshold */
    WRITE_INF_BYTE(INFRA_FIFOCTRL, 0xA0);	/* 32 bytes */

    /* Timeout threshold expressions:
     * ((TIMETHR + 1) * 128 * VALUE_CLK_CYC) = VALUE_TOUT
     * => TIMETHR = (VALUE_TOUT / (128 * VALUE_CLK_CYC)) - 1
     */
    WRITE_INF_BYTE(INFRA_TIMETHR, (VALUE_TOUT / (VALUE_CLK_CYC << 7) - 1));

    /* Noise pulse timeout expressions:
     * Value = VALUE_NOISETHR / VALUE_CLK_CYC
     */
    WRITE_INF_BYTE(INFRA_NOISETHR, VALUE_NOISETHR / VALUE_CLK_CYC);

#if (SYS_CHIP_MODULE == ALI_S3601||SYS_CHIP_MODULE == ALI_S3602)
	if(sys_ic_is_M3101())
		osal_interrupt_register_lsr(15, irc_m6303irc_lsr, 0);
	else
		osal_interrupt_register_lsr(27, irc_m6303irc_lsr, 0);
#else
	osal_interrupt_register_lsr(15, irc_m6303irc_lsr, 0);
#endif

#if (((SYS_SDRAM_SIZE != 2)  && (SYS_PROJECT_FE != PROJECT_FE_ATSC)) || (PORTING_ATSC == 1))
	/* Register an 1mS cyclic for IR Key release */
	osal_interrupt_register_lsr(7, irc_m6303irc_count, 0);
#endif

    /* Ensure no pending interrupt */
    WRITE_INF_BYTE(INFRA_ISR, 3);

    /* Enable IRC Interrupt */
    WRITE_INF_BYTE(INFRA_IER, 3);
}

/*****************************************************************************
 * void irc_m6303irc_close()
 * Description: M6303 IR controler stop
 *
 * Arguments:
 *
 * Return Value:
 ****************************************************************************/
void irc_m6303irc_close()
{
    WRITE_INF_BYTE(INFRA_IRCCFG, 0);	/* Disable IRC */
#if (SYS_CHIP_MODULE == ALI_S3601||SYS_CHIP_MODULE == ALI_S3602)
	if(sys_ic_is_M3101())
		osal_interrupt_unregister_lsr(15, irc_m6303irc_lsr);
	else
		osal_interrupt_unregister_lsr(27, irc_m6303irc_lsr);
#else
	osal_interrupt_unregister_lsr(15, irc_m6303irc_lsr);
#endif
#if (((SYS_SDRAM_SIZE != 2)  && (SYS_PROJECT_FE != PROJECT_FE_ATSC)) || (PORTING_ATSC == 1))
	osal_interrupt_unregister_lsr(7, irc_m6303irc_count);
#endif
}

/*****************************************************************************
 * void generate_code()
 * Description: M6303 IR controler generate code
 *
 * Arguments:
 *
 * Return Value:
 ****************************************************************************/
static void generate_code(UINT32 tick)
{
	static UINT32 last_act_tick = 0, last_tick = 0;
	UINT32 code, pulse_width, last_width, pulse_polarity;
	UINT8  data;
	struct pan_key key;

#ifndef NEW_IR_DRIVER
#if (((SYS_SDRAM_SIZE != 2)  && (SYS_PROJECT_FE != PROJECT_FE_ATSC)) || (PORTING_ATSC == 1))
#if (SYS_CHIP_MODULE == ALI_S3602) && (SYS_PROJECT_FE == PROJECT_FE_DVBS)
#else
	irc_LAB_pulse_to_code((tick - last_tick) * 1000);	/* The time in idle */
#endif
#endif
	irc_NEC_pulse_to_code((tick - last_tick) * 1000);	/* The time in idle */
			//		irc_RC6_pulse_to_code((tick - last_tick) * 1000);	/* The time in idle */
#else
	irc_pulse_to_code((tick - last_tick) * 1000, 1);	/* The time in idle */
#endif
	last_tick = tick;

	pulse_width = 0;
	while (bufferin != bufferout)
	{
		data = infra_buffer[bufferout];

//		PRINTF("br=%d, bw=%d, data=%02x\n", bufferout, bufferin, data);
		/* The MSB bit is the status bit, LSB 7 bits is time ticks,
		 * If some one status is too long, mult-bytes maybe used.
		 */
		pulse_width += ((data & 0x7f) * VALUE_CLK_CYC);	/* Pulse width */
		pulse_polarity = (data & 0x80) ? 1 : 0;
		bufferout = ((bufferout + 1) & (IR_RLC_SIZE - 1));/* Next data */
		/* Long pulse */
		if ((!((data ^ infra_buffer[bufferout]) & 0x80)) && (bufferout != bufferin))
		{
			continue;
		}
//		PRINTF("\t\t\tlevel=%d, width=%d\n", ((data&0x80)>>7), pulse_width);
		if (
#ifndef NEW_IR_DRIVER
#if (((SYS_SDRAM_SIZE != 2)  && (SYS_PROJECT_FE != PROJECT_FE_ATSC)) || (PORTING_ATSC == 1))
#if (SYS_CHIP_MODULE == ALI_S3602) && (SYS_PROJECT_FE == PROJECT_FE_DVBS)
#else
		(code = irc_LAB_pulse_to_code(pulse_width)) != PAN_KEY_INVALID ||
#endif
#endif
		(code = irc_NEC_pulse_to_code(pulse_width)) != PAN_KEY_INVALID)
		//(code = irc_RC6_pulse_to_code(pulse_width)) != PAN_KEY_INVALID)
#else
		(code = irc_pulse_to_code(pulse_width, pulse_polarity)) != PAN_KEY_INVALID)
#endif
		{
			ms_tick = 0;
			last_width = tick - last_act_tick;
//			PRINTF("\t\t\tlast_width=%d\n", last_width);
			if(key_cnt == 0)		/* Receive a new key */
				last_act_code = PAN_KEY_INVALID;
			else if(key_cnt == 1)	/* Receive a continous key */
				ir_repeat_width = local_hw_info->intv_repeat_first;
			else
				ir_repeat_width = local_hw_info->intv_repeat;
		    if (last_act_code != code || last_width > ir_repeat_width)
		    {
				key.type = PAN_KEY_TYPE_REMOTE;
				key.state = PAN_KEY_PRESSED;
				key.count = key_cnt++;
				key.code = code;
				if(pan_buff_get_repeat() || key_cnt == 1)
				{
					pan_buff_queue_tail(&key);
				}
				last_act_code = code;
				last_act_tick = tick;
				key_state = PAN_KEY_PRESSED;
			}
		}
		pulse_width = 0;
	}
    return;
}

/*****************************************************************************
 * void irc_m6303irc_lsr(UINT32 param)
 * Description: M6303 IR controler interrupt
 *
 * Arguments:
 *     UINT32 param
 *
 * Return Value:
 ****************************************************************************/
static void irc_m6303irc_lsr(UINT32 param)
{
    volatile unsigned char status, num, num1;
    UINT16 new_bufferin = 0;

    while (status = (READ_INF_BYTE(INFRA_ISR) & 3))
    {
		WRITE_INF_BYTE(INFRA_ISR, status);

		switch (status)
		{
			case 0x02:		/* If timeout, generate IR code in HSR */
			case 0x01:		/* If FIFO trigger, copy data to buffer */
				do
				{
					num1 = num = READ_INF_BYTE(INFRA_FIFOCTRL) & 0x7F;
					#if 0
					/* Adjust write pointer and read pointer. */
					new_bufferin = (bufferin + num1)&(IR_RLC_SIZE - 1);
					if((bufferout > bufferin&& (bufferout <= new_bufferin ||new_bufferin < bufferin )) \
						|| (bufferout < bufferin && bufferout <= new_bufferin&& new_bufferin< bufferin) )
						bufferout = (bufferin + num1 + 1)&(IR_RLC_SIZE - 1);
					#endif

					while (num > 0)
					{
						/* Put RLC to buffer */
						infra_buffer[bufferin++] = READ_INF_BYTE(INFRA_RLCBYTE);
						bufferin &= (IR_RLC_SIZE - 1);
						num--;
					};
				} while (num1 > 0);
				break;
			default:
				break;
		}
		if (status == 0x02)
		{
#if (IR_USE_HSR == 0)
			generate_code(osal_get_tick());
#else
			osal_interrupt_register_hsr(generate_code, osal_get_tick());
#endif
		}
	}

	return;
}

#if (((SYS_SDRAM_SIZE != 2)  && (SYS_PROJECT_FE != PROJECT_FE_ATSC)) || (PORTING_ATSC == 1))
static void irc_m6303irc_count(UINT32 param)
{
    struct pan_key key;

    if (key_state == PAN_KEY_PRESSED)
    {
        if (++ms_tick > local_hw_info->intv_release)
        {
        	if(key_cnt > 1)						/* Solve UI ack IR slow issue */
	        	pan_buff_clear();
            key.type = PAN_KEY_TYPE_REMOTE;
            key.state = PAN_KEY_RELEASE;
            key.count = key_cnt;
            key.code = last_act_code;
            pan_buff_queue_tail(&key);
            key_state = PAN_KEY_RELEASE;
        }
    }
}
#endif

