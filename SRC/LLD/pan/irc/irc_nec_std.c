/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2002 Copyright (C)
 *
 *  File: irc_nec_std.c
 *
 *  Description: This file implements the NEC mode Infra Receiver common
 *               driver function library.
 *  History:
 *      Date        Author      Version  Comment
 *      ====        ======      =======  =======
 *  1.  2003.7.5    Justin Wu   0.1.000  Create
 *  2.  2003.7.18   Justin Wu   0.2.000  Upgrade statement machime
 *  3.  2004.9.21   Justin Wu   0.3.000  Enhance compatibility
 ****************************************************************************/

#include <types.h>
#include <api/libc/printf.h>
#include <hld/pan/pan.h>


/* NEC IRC code standard params */
#if (SYS_CHIP_MODULE == ALI_S3602)
#define IRC_NEC_PULSE_UNIT		530							/* 560uS */
#else
#define IRC_NEC_PULSE_UNIT		560							/* 560uS */
#endif
#define IRC_NEC_LEADING			(16 * IRC_NEC_PULSE_UNIT)	/* 8.96mS */
#define IRC_NEC_LEADING_REPT	(20 * IRC_NEC_PULSE_UNIT)	/* 11.2mS */
#define IRC_NEC_LEADING_FUNC    (24 * IRC_NEC_PULSE_UNIT)	/* 13.44mS */

extern unsigned short key_cnt;

static UINT32 irc_NEC_state;		/* IRC state */
static UINT32 irc_NEC_precision;	/* Pulse uint precision,
									   should < IRC_NEC_PULSE_UNIT/2 */
static UINT32 irc_NEC_prec_lead;	/* Leading precision,
									   should < IRC_NEC_PULSE_UNIT*2 */
static UINT32 irc_NEC_repeat_to;

/*****************************************************************************
 * void irc_NEC_mode_set(UINT32 state, UINT32 precision, UINT32 repeat_to)
 * Description: NEC mode IR controler: mode setup
 *
 * Arguments:
 *    UINT32 state		: Current IRC state, init is 0
 *    UINT32 precision	: Code pulse precision
 *    UINT32 repeat_to	: Repeat code timeout limit
 *
 * Return Value:
 ****************************************************************************/
void irc_NEC_mode_set(UINT32 state, UINT32 precision, UINT32 repeat_to)
{
	irc_NEC_state = state;
	irc_NEC_precision = precision;
	irc_NEC_prec_lead = (precision << 2);
	irc_NEC_repeat_to = repeat_to;
}

/*****************************************************************************
 * UINT32 irc_NEC_pulse_to_code(UINT32 pulse_width)
 * Description: NEC mode IR controler: translate pulse width to IR code
 *              This function support both of one and two trigle per bit mode.
 *
 * Arguments:
 *    UINT32 pulse_width	: Input pulse width, in uS
 *
 * Return Value:
 *    INT32					: Key code
 ****************************************************************************/
UINT32 irc_NEC_pulse_to_code(UINT32 pulse_width)
{
	static UINT32 code, last_code = PAN_KEY_INVALID;
	static UINT32 accum_width = 0;
	static UINT32 first_half = 1;		/* For dule-pulse senser */
	static UINT32 last_width = 0;

	last_width += pulse_width;
    /* Ignore burr pluses */
    if (pulse_width < irc_NEC_precision)
    {
        return PAN_KEY_INVALID;
    }

//	PRINTF("state: %d, totol width: %d, first_half: %d\n",
//	  irc_NEC_state, last_width, first_half);

	/* Pre process: Combine high & low together */
    if (irc_NEC_state == 0)
    {
    	/* Is leading pulse, can goto the next state */
        if (pulse_width > IRC_NEC_LEADING - irc_NEC_prec_lead &&
            pulse_width < IRC_NEC_LEADING + irc_NEC_prec_lead)
        {
            first_half = 0;
            return PAN_KEY_INVALID;
        }
    } else
    {
        if (first_half == 1 &&
            pulse_width < (IRC_NEC_PULSE_UNIT << 1) - irc_NEC_precision)
        {
            first_half = 0;
            return PAN_KEY_INVALID;
        }
    }

	pulse_width = last_width;
	last_width = 0;

    first_half = 1;
    accum_width += pulse_width;

	/* Pulse process: transmit cycle pulse to code.
	 * All of waveform send out by remote is inverted at receiver end.
	 */
    if (irc_NEC_state == 0)
    {
    	/* For 16 + 8ms leading code: function code, can goto next state */
        if (pulse_width > IRC_NEC_LEADING_FUNC - irc_NEC_prec_lead &&
            pulse_width < IRC_NEC_LEADING_FUNC + irc_NEC_prec_lead)
        {
            /* New key pressed: Leading pulse finished */
            accum_width = pulse_width;
            last_code = PAN_KEY_INVALID;
            key_cnt = 0;
        }
        /* For 16 + 4ms leading code: repeat code */
        else if (pulse_width > IRC_NEC_LEADING_REPT - irc_NEC_prec_lead &&
                 pulse_width < IRC_NEC_LEADING_REPT + irc_NEC_prec_lead)
        {
            /* Reset state */
            irc_NEC_state = 0;
            if (accum_width < irc_NEC_repeat_to)
            {
                /* Reset key-repeat time count, wait new repeat */
                accum_width = 0;
                if (last_code != PAN_KEY_INVALID)
                {
					return last_code;
                }
            }
            return PAN_KEY_INVALID;
        }
        else
        {
			/* Unknow Leading pulse, reset state, support continue repeat */
			if (pulse_width < IRC_NEC_LEADING_REPT)
			{
	            irc_NEC_state = 0;
	        }
            return PAN_KEY_INVALID;
        }
    }
    else
    {
        if (pulse_width > IRC_NEC_PULSE_UNIT &&
            pulse_width < IRC_NEC_PULSE_UNIT * 5)
        {
	        code = (code << 1) + (pulse_width > IRC_NEC_PULSE_UNIT * 3);
	        /* Generate code */
	        if (irc_NEC_state == 32)
	        {
	            last_code = code;
	            /* Got a key code */
				irc_NEC_state = 0;
				return code;
	        }
	    } else
	    {
			irc_NEC_state = 0;
	        return PAN_KEY_INVALID;
	    }
    }

	/* Next state */
	irc_NEC_state++;
    return PAN_KEY_INVALID;
}
/****************** Local definitions     **/
#define RCU_HIGH_WORD   0x8046

#define RC6_T			(445L)   

#define RC6_2T_H     	(RC6_T * 24/10)    // 1068
#define RC6_3T_H     	(RC6_T * 34/10)	//1513
#define RC6_4T_H     	(RC6_T * 44/10)	//1958
#define RC6_5T_H		(RC6_T * 54/10)	//2403
#define RC6_6T_H		(RC6_T * 64/10)	//2848

#define RC6_2T_L     	(RC6_T * 16/10)	//712
#define RC6_3T_L     	(RC6_T * 26/10)	//1157


static UINT32 irc_RC6_state;		/* IRC state */
static UINT32 irc_RC6_precision;	/* Pulse uint precision */

void irc_RC6_mode_set(UINT32 state, UINT32 precision, UINT32 repeat_to)
{
	irc_RC6_state = state;
	irc_RC6_precision = precision;
}

/*****************************************************************************
 * UINT32 irc_RC6_pulse_to_code(UINT32 pulse_width, UINT32 pulse_polarity )
 * Description: RC6 mode IR controler: translate pulse width to IR code
 *              This function support both of one and two trigle per bit mode.
 *
 * Arguments:
 *    UINT32 pulse_width	: Input pulse width, in uS
 *    UINT32 pulse_polarity : Input pulse width polarity , '1'  is high level , '0' is low level
 * Return Value:
 *    INT32					: Key code
 ****************************************************************************/
UINT32 irc_RC6_pulse_to_code(UINT32 pulse_width, UINT32 pulse_polarity )
{
    static UINT32 xRcuData, nRcuBits;
	static UINT32 first_half = 1;		/* For dule-pulse senser */
    static UINT32 last_width = 0;

//	libc_printf("%d (%d)\t", pulse_width ,  pulse_polarity);

    last_width += pulse_width;
    /* Ignore burr pluses */
    if (pulse_width < irc_RC6_precision)
    {
        return PAN_KEY_INVALID;
    }

    if (last_width>RC6_6T_H)  
    {
        irc_RC6_state=0;
        first_half=1;
        last_width=0;
        return PAN_KEY_INVALID;
    }

    if (irc_RC6_state==0){//LB
        if (last_width<RC6_6T_H)
        {
            irc_RC6_state++;
        }
        last_width=0;
        return PAN_KEY_INVALID;
    }

    if (first_half == 1)
    {
        first_half = 0;
        return PAN_KEY_INVALID;
    }

    pulse_width = last_width;
    last_width = 0;

    first_half = 1;

    switch (irc_RC6_state) {
        case 1: //SB
			if (pulse_width>RC6_3T_L && pulse_width<RC6_3T_H) {
                xRcuData=1;
                nRcuBits=1;
				irc_RC6_state=2;
			} else {
				irc_RC6_state=0;
			}
			break;

		case 2: //mode
			nRcuBits++;
			xRcuData <<= 1;

			if (pulse_width<RC6_2T_L) {
				irc_RC6_state=0;
				break;
			} else if (pulse_width<RC6_2T_H) {
			    if (xRcuData & 0x2) {
			        xRcuData++;
			    }
			} else if(pulse_width<RC6_3T_H){
			    if (!(xRcuData & 0x2)) {
			        xRcuData<<=1;
					xRcuData++;
			        nRcuBits++;
			    }
			} else if(pulse_width<RC6_4T_H){
			    if (xRcuData & 0x2) {
			        xRcuData<<=1;
			        xRcuData++;
			        nRcuBits++;
			    } else {
			    	irc_RC6_state=0;
					break;
			    }
			} else if(pulse_width<RC6_5T_H) {
				if (nRcuBits==5) {
					xRcuData=1;
					nRcuBits=1;
					irc_RC6_state++;
				} else {
					irc_RC6_state=0;
				}
				break;
			} else {
			    irc_RC6_state=0;
				break;
			}
			if(nRcuBits>=5) {
				irc_RC6_state=0;
			}
			break;

		case 3: //custom code and information
			nRcuBits++;
			xRcuData <<= 1;

			if (pulse_width<RC6_2T_L) {
				irc_RC6_state=0;
			} else if (pulse_width<RC6_2T_H) {
			    if (xRcuData & 0x2) {
			        xRcuData++;
			    }
			} else if(pulse_width<RC6_3T_H){
			    if (!(xRcuData & 0x2)) {
			        xRcuData<<=1;
			        xRcuData++;
			        nRcuBits++;
			    }
			} else if(pulse_width<RC6_4T_H){
			    if (xRcuData & 0x2) {
			        xRcuData<<=1;
			        xRcuData++;
			        nRcuBits++;
			    } else {
			    	irc_RC6_state=0;
			    }
			} else {// illegal data
			    irc_RC6_state=0;
			}
			if (nRcuBits==33) {
				irc_RC6_state=0;
			} else if(nRcuBits==32) {
				irc_RC6_state=0;
                return xRcuData;
			}
			break;
	}
    last_width=0;
    return PAN_KEY_INVALID;
}
