/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2008 Copyright (C)
 *
 *  File: irc_std.c
 *
 *  Description: This file implements Infra Receiver common
 *               driver function library.
 *  History:
 *      Date        Author      Version  Comment
 *      ====        ======      =======  =======
 *  1.  2009.5.22	Trueve Hu	0.1.000  Create file.
 ****************************************************************************/

#include <types.h>
#include <api/libc/printf.h>
#include <hld/pan/pan.h>

//#define IRC_DEBUG

#ifdef IRC_DEBUG
	#define IRC_DEBUG_PRINTF	libc_printf
#else
	#define IRC_DEBUG_PRINTF(...)
#endif

//#define IRC_TRACE
#ifdef IRC_TRACE
	#define IRC_TRACE_TYPE		7
	#define IRC_TRACE_PRINTF	libc_printf
#else
	#define IRC_TRACE_PRINTF(...)
#endif


#define PULSE_LOW		(0 << 31)
#define PULSE_HIGH		(1 << 31)
#define PULSE_POL(x)	(x & (1 << 31))
#define PULSE_VALUE(x)	(x & (~(1 << 31)))


enum irp_type
{
	IR_TYPE_NEC = 0,
	IR_TYPE_LAB,
	IR_TYPE_50560,
	IR_TYPE_KF,
	IR_TYPE_LOGIC,
	IR_TYPE_SRC,
	IR_TYPE_NSE,
	IR_TYPE_RC5,
	IR_TYPE_RC6,
};

static const char *irp_strs[] = 
{
	"nec",
	"lab",
	"50560",
	"kf",
	"logic",
	"src",
	"nse",
	"rc5",
	"rc6",
};

enum ir_waveform
{
	IR_LEADING	= 0,
	IR_SPECIAL,
	IR_DATA,
	IR_REPEAT_LEADING,
	IR_REPEAT_DATA,
	IR_STOP,
	IR_END,
	IR_NULL,
};

struct irc_pulse
{
	UINT32 type;
	UINT32 fst_half;
	UINT32 scd_half;
	UINT32 tolerance;
};

struct ir_attr
{
	UINT32 type;
	UINT32 bit_msb_first	: 1;
	UINT32 byte_msb_first	: 1;
	UINT32 pulse_invert		: 1;
	UINT32 repeat_enable	: 1;
	UINT32 ignore_lastpulse	: 1;
	UINT32 pulse_prec;
	UINT32 pulse_max_width;
	struct irc_pulse pulse[7];
	enum ir_waveform *normal;
	enum ir_waveform *repeat;
};

struct irc_decoder
{
	struct ir_attr *attr;
	UINT16 first_half_got;
	UINT16 decode_step;
	UINT32 key_bit_cnt;
	UINT32 last_pulse_width;
	UINT32 key_code[4];
	UINT32 last_key_code[4];
};


static enum ir_waveform ir_nec_normal[] = 
{
	IR_LEADING,   						 // start bit
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,  //8 bits command 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,   //8 bits address
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,  //8 bits ~command 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,  //8 bits ~address
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_END, 
};

static enum ir_waveform ir_nec_repeat[] = 
{
	IR_REPEAT_LEADING, 
	IR_END, 
};

static struct ir_attr ir_attr_nec = 
{
	IR_TYPE_NEC,
	1,
	0,
	0,
	1,
	0,
	280,
	15000,
	{
		{IR_LEADING, PULSE_LOW | 9000, PULSE_HIGH | 4500, 500}, // start pulse
		{IR_NULL, 0, 0, 0},
		{IR_DATA, PULSE_LOW | 560, PULSE_HIGH | 560, 200},  //logic '0'
		{IR_DATA, PULSE_LOW | 560, PULSE_HIGH | 1680, 300},  // logic '1'
		{IR_REPEAT_LEADING, PULSE_LOW | 9000, PULSE_HIGH | 2250, 500},
		{IR_REPEAT_DATA, PULSE_LOW | 560, PULSE_HIGH | 20000, 1000},
	{IR_STOP, PULSE_LOW | 500, PULSE_HIGH | 15000, 1000}, 
	},
	ir_nec_normal, 
	ir_nec_repeat, 
};

static enum ir_waveform ir_lab[] = 
{
	IR_LEADING, 
	IR_SPECIAL, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA,
	IR_END, 
};

static struct ir_attr ir_attr_lab = 
{
	IR_TYPE_LAB,
	1,
	0,
	0,
	0,
	0,
	140,
	15000,
	{
	{IR_LEADING, PULSE_LOW | 280, PULSE_HIGH | 7300, 140},
	{IR_SPECIAL, PULSE_LOW | 280, PULSE_HIGH | 6150, 1500},
	{IR_DATA, PULSE_LOW | 280, PULSE_HIGH | 4780, 500},
	{IR_DATA, PULSE_LOW | 280, PULSE_HIGH | 7300, 500},
	{IR_NULL, 0, 0, 0},
	{IR_NULL, 0, 0, 0},
	{IR_STOP, PULSE_LOW | 500, PULSE_HIGH | 15000, 1000}, 
	},
	ir_lab, 
	NULL, 
};

static enum ir_waveform ir_50560[] = 
{
	IR_LEADING, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_SPECIAL,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_SPECIAL,
	IR_END, 
};

static struct ir_attr ir_attr_50560 = 
{
	IR_TYPE_50560,
	1,
	0,
	0,
	1,
	0,
	260,
	15000,
	{
	{IR_LEADING, PULSE_LOW | 8400, PULSE_HIGH | 4200, 500},
	{IR_SPECIAL, PULSE_LOW | 520, PULSE_HIGH | 4200, 500},
	{IR_DATA, PULSE_LOW | 520, PULSE_HIGH | 1050, 500},
	{IR_DATA, PULSE_LOW | 520, PULSE_HIGH | 2100, 500},
	{IR_NULL, 0, 0, 0},
	{IR_NULL, 0, 0, 0},
	{IR_STOP, PULSE_LOW | 500, PULSE_HIGH | 15000, 1000}, 
	},
	ir_50560, 
	NULL, 
};

static enum ir_waveform ir_kf[] = 
{
	IR_LEADING, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,  
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,  
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,  
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,  
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_END, 
};

static struct ir_attr ir_attr_kf = 
{
	IR_TYPE_KF,
	0,
	1,
	0,
	0,
	0,
	190,
	20000,
	{
	{IR_LEADING, PULSE_LOW | 3640, PULSE_HIGH | 1800, 500},
	{IR_NULL, 0, 0, 0},
	{IR_DATA, PULSE_LOW | 380, PULSE_HIGH | 380, 150},
	{IR_DATA, PULSE_LOW | 380, PULSE_HIGH | 1350, 300},
	{IR_NULL, 0, 0, 0},
	{IR_NULL, 0, 0, 0},
	{IR_STOP, PULSE_LOW | 500, PULSE_HIGH | 20000, 1000}, 
	},
	ir_kf, 
	NULL, 
};


static enum ir_waveform ir_logic[] = 
{
	IR_LEADING, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,  
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_END, 
};

static struct ir_attr ir_attr_logic = 
{
	IR_TYPE_LOGIC,
	0,
	0,
	0,
	0,
	0,
	125, 
	20000,
	{
	{IR_LEADING, PULSE_LOW | 5000, PULSE_HIGH | 5000, 500},
	{IR_NULL, 0, 0, 0},
	{IR_DATA, PULSE_LOW | 250, PULSE_HIGH | 500, 100},
	{IR_DATA, PULSE_LOW | 500, PULSE_HIGH | 1000, 200},
	{IR_NULL, 0, 0, 0},
	{IR_NULL, 0, 0, 0},
	{IR_STOP, PULSE_LOW | 500, PULSE_HIGH | 20000, 1000}, 
	},
	ir_logic, 
	NULL, 
};

static enum ir_waveform ir_src_normal[] = 
{
	IR_LEADING, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_SPECIAL, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_END, 
};

static struct ir_attr ir_attr_src = 
{
	IR_TYPE_SRC,
	1,
	0,
	0,
	1,
	0,
	280,
	60000,
	{
	{IR_LEADING, PULSE_LOW | 6000, PULSE_HIGH | 2000, 400},
	{IR_SPECIAL, PULSE_LOW | 3640, PULSE_HIGH | 3640, 400},
	{IR_DATA, PULSE_LOW | 560, PULSE_HIGH | 560, 200},
	{IR_DATA, PULSE_LOW | 560, PULSE_HIGH | 1120, 300},
	{IR_NULL, 0, 0, 0},
	{IR_NULL, 0, 0, 0},
	{IR_STOP, PULSE_LOW | 560, PULSE_HIGH | 20000, 1000}, 
	},
	ir_src_normal, 
	NULL, 
};

static enum ir_waveform ir_nse_normal[] = 
{
	IR_LEADING, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_LEADING, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA, 
	IR_END, 
};

static enum ir_waveform ir_nse_repeat[] = 
{
	IR_REPEAT_LEADING, 
	IR_END, 
};

static struct ir_attr ir_attr_nse = 
{
	IR_TYPE_NSE,
	1,
	0,
	0,
	1,
	0,
	280,
	60000,
	{
	{IR_LEADING, PULSE_LOW | 3640, PULSE_HIGH | 1800, 400},
	{IR_NULL, 0, 0, 0},
	{IR_DATA, PULSE_LOW | 560, PULSE_HIGH | 560, 200},
	{IR_DATA, PULSE_LOW | 560, PULSE_HIGH | 1120, 300},
	{IR_REPEAT_LEADING, PULSE_LOW | 3640, PULSE_HIGH | 3640, 500},
	{IR_REPEAT_DATA, PULSE_LOW | 560, PULSE_HIGH | 20000, 1000},
	{IR_STOP, PULSE_LOW | 560, PULSE_HIGH | 20000, 1000}, 
	},
	ir_nse_normal, 
	ir_nse_repeat, 
};

static enum ir_waveform ir_rc5[] = 
{
	IR_LEADING, IR_LEADING,   //S1 & S2 bit
	IR_SPECIAL, 			  //toggle bit
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,  // 5 bits addr & 6 bits command
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA,
	IR_END,    		 // ???
};

static struct ir_attr ir_attr_rc5 = 
{
	IR_TYPE_RC5,  // type name
	1,			//bits MSB
	0,			
	0,
	0,
	1,			// repeat enable 
	200,			//pulse prec
	15000,		// pulse max width 
	{
		{IR_LEADING, PULSE_HIGH | 830, PULSE_LOW | 830, 200}, // leading logic 
		{IR_SPECIAL, PULSE_HIGH | 830, PULSE_LOW | 830, 200},  // toggle logic 
		{IR_DATA, PULSE_LOW | 830, PULSE_HIGH | 830, 200},	   // data logic '0'
		{IR_DATA, PULSE_HIGH | 830, PULSE_LOW | 830, 200},    // data logic '1'
		{IR_NULL, 0, 0, 0},
		{IR_NULL, 0, 0, 0},
		{IR_STOP, PULSE_HIGH | 830, PULSE_LOW | 15000, 1000}, 
	},
	ir_rc5, 
	NULL, 
};

#define RC6_T			(445L)

#define RC6_2T_H     	(RC6_T * 20/10)
#define RC6_3T_H     	(RC6_T * 30/10)
#define RC6_4T_H     	(RC6_T * 40/10)
#define RC6_5T_H		(RC6_T * 50/10)
#define RC6_6T_H		(RC6_T * 60/10)

#define RC6_2T_L     	(RC6_T * 10/10)
#define RC6_3T_L     	(RC6_T * 20/10)

/*Notice  : we use protocol  rc6_mode_1 */
static enum ir_waveform ir_rc6[] =    // decode step 
{
	IR_LEADING,	// LS pulse
	IR_DATA,    //  SB bit
	IR_DATA,   IR_DATA, IR_DATA,  // MB2...MB0
	IR_SPECIAL, 			  //TR bits
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,  // 8 bits  command
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,  // 8 bits addr 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,  // 8 bits  command
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,  // 8 bits addr 
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_END,    		 // ???  6T signal free 
};

static struct ir_attr ir_attr_rc6 = 
{
	IR_TYPE_RC6,  // type name
	1,			//bits MSB
	0,			// byte MSB
	0,			//pluse_invert	
	0,			// repeat enable 
	1,			//ignore_lastpulse
	200,			//pulse prec
	15000,		// pulse max width 
	{
		{IR_LEADING, PULSE_LOW | RC6_6T_H, PULSE_HIGH | RC6_2T_H, 600}, // leading pulse
		{IR_SPECIAL, PULSE_LOW | RC6_2T_H, PULSE_HIGH | RC6_2T_H, 200},  //TR logic 
		{IR_DATA, PULSE_HIGH | RC6_T, PULSE_LOW | RC6_T, 200},	  // data logic 
		{IR_DATA, PULSE_LOW | RC6_T, PULSE_HIGH | RC6_T, 200},	   // data logic 
		{IR_NULL, 0, 0, 0},
		{IR_NULL, 0, 0, 0},
		{IR_END, PULSE_HIGH | RC6_5T_H,PULSE_HIGH | RC6_T, 1000},// Clear Warning {IR_END, PULSE_HIGH | RC6_6T_H, 1000}, 
	},
	ir_rc6, //pluse_step
	NULL, 
};

static struct irc_decoder irc_decoders[] = 
{
	{
		.attr = &ir_attr_nec,
		.first_half_got = 0,
		.decode_step = 0,
		.key_bit_cnt = 0,
		.last_pulse_width = 0,
		.key_code = {0,},
		.last_key_code = {0,},
	},
	{
		.attr = &ir_attr_lab,
		.first_half_got = 0,
		.decode_step = 0,
		.key_bit_cnt = 0,
		.last_pulse_width = 0,
		.key_code = {0,},
		.last_key_code = {0,},
	},
	{
		.attr = &ir_attr_50560,
		.first_half_got = 0,
		.decode_step = 0,
		.key_bit_cnt = 0,
		.last_pulse_width = 0,
		.key_code = {0,},
		.last_key_code = {0,},
	},
	{
		.attr = &ir_attr_kf,
		.first_half_got = 0,
		.decode_step = 0,
		.key_bit_cnt = 0,
		.key_code = {0,},
		.last_pulse_width = 0,
		.last_key_code = {0,},
	},
	{
		.attr = &ir_attr_logic,
		.first_half_got = 0,
		.decode_step = 0,
		.key_bit_cnt = 0,
		.last_pulse_width = 0,
		.key_code = {0,},
		.last_key_code = {0,},
	},
	{
		.attr = &ir_attr_src,
		.first_half_got = 0,
		.decode_step = 0,
		.key_bit_cnt = 0,
		.last_pulse_width = 0,
		.key_code = {0,},
		.last_key_code = {0,},
	},
	{
		.attr = &ir_attr_nse,
		.first_half_got = 0,
		.decode_step = 0,
		.key_bit_cnt = 0,
		.last_pulse_width = 0,
		.key_code = {0,},
		.last_key_code = {0,},
	},
	{
		.attr = &ir_attr_rc5,
		.first_half_got = 0,
		.decode_step = 0,
		.key_bit_cnt = 0,
		.last_pulse_width = 0,
		.key_code = {0,},
		.last_key_code = {0,},
	},
/*   Notice : RC6 & RC5 decoder are uncompatible at irc_decode function !!!! , so 
    if  we  choose RC6 decoder , the RC5 decoder must be disable . In the same , if enable RC5,
    RC6 must be disable .
	{
		.attr = &ir_attr_rc6,
		.first_half_got = 0,
		.decode_step = 0,
		.key_bit_cnt = 0,
		.last_pulse_width = 0,
		.key_code = {0,},
		.last_key_code = {0,},
	},
	*/

};


extern UINT16 key_cnt;


#define INRANGE(x, value, tol)		((x) > ((value)-(tol)) && (x) < ((value)+(tol)))
#define IRP_CNT					(sizeof(irc_decoders) / sizeof(struct irc_decoder))

#define COPY_CODE(dst, src)		do {int i; for(i=0;i<16;i++) *((UINT8 *)dst+i) = *((UINT8 *)src+i);} while(0)
#define CLEAR_CODE(p)			do {int i; for(i=0;i<16;i++) *((UINT8 *)p+i) = 0;} while(0)

static void reverse_code_bit(UINT32 *code, UINT32 bit_cnt)
{
	UINT32 i, j;
	UINT32 src_code[4] = {0,};
	UINT32 dst_code[4] = {0,};
	
	COPY_CODE(src_code, code);

	for (i=0; i<bit_cnt; i++)
	{
		if (src_code[i / 32] & (1 << (i % 32)))
			dst_code[(bit_cnt - 1 - i) / 32] |= 1 << ((bit_cnt - 1 - i) % 32);
	}
	
	COPY_CODE(code, dst_code);
}

static void reverse_code_byte(UINT32 *code, UINT32 bit_cnt)
{
	UINT32 i, j;
	UINT8 src_code[16] = {0,};
	UINT8 dst_code[16] = {0,};
	UINT32 byte_cnt = (bit_cnt  + 7) / 8;

	COPY_CODE(src_code, code);

	for (i=0; i<byte_cnt; i++)
	{
		dst_code[i] = src_code[byte_cnt - i - 1];
	}
	
	COPY_CODE(code, dst_code);
}

enum irc_decode_ret
{
	IRC_DECODE_SUCCEED,
	IRC_DECODE_DATA0,
	IRC_DECODE_DATA1,
	IRC_DECODE_FAIL,
	IRC_DECODE_STOP,
};

/*****************************************************************************
 * UINT32 irc_decode(UINT32 pulse_width)
 * Description: Translate pulse width to IR code
 *
 * Arguments:
 *	  struct irc_decoder *ird	: IR decode structure
 *    UINT32 pulse_width		: Input pulse width, in uS
 *    UINT32 pulse_polarity		: Input pulse polarity, 
 *									'1' = high level, '0' = low level
 *
 * Return Value:
 *    INT32						: Key code
 ****************************************************************************/
 static UINT32 irc_decode(struct irc_decoder *ird, UINT32 pulse_width, UINT32 pulse_polarity)
{
	UINT32 accum_pulse_width = 0;
	enum irc_decode_ret result = IRC_DECODE_FAIL;
	static UINT32 key_got_tick;
	static UINT32 rc6_tick =0 ;  // only for RC6

	/* Ignore burr pulses */
	if (pulse_width < ird->attr->pulse_prec)
	{
		ird->first_half_got = 0;
		IRC_DEBUG_PRINTF("Burr pulses!\n");
		return PAN_KEY_INVALID;
	}

	if (ird->first_half_got == 0)  	  /* first_half_got is flag  for dule-pulse senser */
	{
		if (ird->decode_step == 0) /*just check leading pulse*/
		{
			UINT32 half_pol;
			UINT32 half_width;
			if (PULSE_POL(ird->attr->pulse[0].fst_half)) /* if leading start with high level */
			{
				half_pol = PULSE_POL(ird->attr->pulse[0].scd_half);
				half_width = PULSE_VALUE(ird->attr->pulse[0].scd_half);
			}
			else
			{
				half_pol = PULSE_POL(ird->attr->pulse[0].fst_half);
				half_width = PULSE_VALUE(ird->attr->pulse[0].fst_half);
			}

			/* Check leading code first half pulse polarity */
			if (half_pol ^ (pulse_polarity << 31))
			{
				#ifdef IRC_TRACE
				if (ird->attr->type == IRC_TRACE_TYPE)
				#endif
				IRC_DEBUG_PRINTF("Error leading half pulse polarity!\n");
				return PAN_KEY_INVALID;
			}
			/* Check leading code first half pulse width */
			if (!INRANGE(pulse_width, half_width, ird->attr->pulse[0].tolerance))
			{
				#ifdef IRC_TRACE
				if (ird->attr->type == IRC_TRACE_TYPE)
				#endif
				IRC_DEBUG_PRINTF("Leading half pulse not in range!(%d)\n", pulse_width);
				return PAN_KEY_INVALID;
			}
			rc6_tick =0 ;  
		}
		ird->first_half_got = 1;
		ird->last_pulse_width = pulse_width;

		if (ird->decode_step == 0 && (PULSE_POL(ird->attr->pulse[0].fst_half) ? 1 : 0))
		{
			ird->first_half_got = 0;
			ird->last_pulse_width = 0;
			ird->decode_step++;
		}
		return PAN_KEY_INVALID;
	}
	else
	{
		accum_pulse_width = ird->last_pulse_width + pulse_width;
		ird->last_pulse_width = 0;
		ird->first_half_got = 0;

		/* Process normal key */
		if (ird->attr->normal)
		{
			enum ir_waveform step = ird->attr->normal[ird->decode_step];
			enum ir_waveform next_step = ird->attr->normal[ird->decode_step + 1];
			if (step > IR_DATA)
				step++;

			UINT32 accum_pulse = PULSE_VALUE(ird->attr->pulse[step].fst_half) + PULSE_VALUE(ird->attr->pulse[step].scd_half);
			UINT32 tolerance_pulse = ird->attr->pulse[step].tolerance;
			UINT32 same_polarity = !(PULSE_POL(ird->attr->pulse[step].scd_half) ^ (pulse_polarity << 31));
				
			if( (ird->attr->type == IR_TYPE_RC5)||(ird->attr->type == IR_TYPE_RC6))
			{
				accum_pulse += PULSE_VALUE(ird->attr->pulse[next_step].fst_half);
				tolerance_pulse += PULSE_VALUE(ird->attr->pulse[next_step].fst_half);
			}
			
			if (ird->attr->normal[ird->decode_step] == IR_DATA)
			{
				UINT32 accum_pulse_1 = PULSE_VALUE(ird->attr->pulse[IR_DATA + 1].fst_half) + PULSE_VALUE(ird->attr->pulse[IR_DATA + 1].scd_half);
				UINT32 tolerance_pulse_1 = ird->attr->pulse[IR_DATA + 1].tolerance;
				UINT32 same_polarity_1 = !(PULSE_POL(ird->attr->pulse[IR_DATA + 1].scd_half) ^ (pulse_polarity << 31));
				
			
				if( (ird->attr->type == IR_TYPE_RC5)||(ird->attr->type == IR_TYPE_RC6))
				{
					accum_pulse_1 += PULSE_VALUE(ird->attr->pulse[IR_DATA + 1].fst_half);
					tolerance_pulse_1 += PULSE_VALUE(ird->attr->pulse[IR_DATA + 1].fst_half);
				}
				if (((ird->attr->ignore_lastpulse && ird->attr->normal[ird->decode_step + 1] == IR_END) || \
					INRANGE(accum_pulse_width, accum_pulse, tolerance_pulse)) && (same_polarity ? 1 : 0))
				{
					result = IRC_DECODE_DATA0;
					#ifdef IRC_TRACE
					if (ird->attr->type == IRC_TRACE_TYPE)
					#endif
					IRC_TRACE_PRINTF("[0]");
				}
				else if (((ird->attr->ignore_lastpulse && ird->attr->normal[ird->decode_step + 1] == IR_END) || \
					INRANGE(accum_pulse_width, accum_pulse_1, tolerance_pulse_1)) && (same_polarity_1 ? 1 : 0))
				{
					result = IRC_DECODE_DATA1;
					#ifdef IRC_TRACE
					if (ird->attr->type == IRC_TRACE_TYPE)
					#endif
					IRC_TRACE_PRINTF("[1]");
				}
				else
					result = IRC_DECODE_FAIL;
			}
			else
			{
				if (INRANGE(accum_pulse_width, accum_pulse, tolerance_pulse))
				{
					if (((ird->attr->type == IR_TYPE_RC5) ||(ird->attr->type == IR_TYPE_RC6)) && step == IR_SPECIAL)
						result = IRC_DECODE_SUCCEED;
					else if (same_polarity)
						result = IRC_DECODE_SUCCEED;
					else
						result = IRC_DECODE_FAIL;
				}
				else
					result = IRC_DECODE_FAIL;
			}
			
			if (result == IRC_DECODE_SUCCEED || \
				result == IRC_DECODE_DATA0 || \
				result == IRC_DECODE_DATA1)
			{
				ird->decode_step++;
				if (result == IRC_DECODE_DATA0 || \
					result == IRC_DECODE_DATA1)
				{
					if (ird->attr->type == IR_TYPE_RC6 && rc6_tick ++ <4)
						;		// RC6 ignore SB MB2-MB0 4 bits data
					else{
							if (result == IRC_DECODE_DATA1)
								/*decode ONE BIT success*/
								ird->key_code[ird->key_bit_cnt / 32] |= 1 << (ird->key_bit_cnt % 32);
							ird->key_bit_cnt++;
						}
				}
				
				if (((ird->attr->type == IR_TYPE_RC5)||(ird->attr->type == IR_TYPE_RC6)) && INRANGE(accum_pulse_width, accum_pulse, ird->attr->pulse[next_step].tolerance))
				{
					ird->first_half_got = 1;
					ird->last_pulse_width = PULSE_VALUE(ird->attr->pulse[next_step].fst_half);
				}

				/* deal with bit/byte reverse if need at last */
				if (ird->attr->normal[ird->decode_step] == IR_END)
				{
					UINT32 last_key = PAN_KEY_INVALID;
					if (ird->attr->bit_msb_first)
						reverse_code_bit(ird->key_code, ird->key_bit_cnt);
					if (ird->attr->byte_msb_first)
						reverse_code_byte(ird->key_code, ird->key_bit_cnt);
					ird->decode_step = 0;
					ird->key_bit_cnt = 0;
					last_key = ird->last_key_code[0];
					COPY_CODE(ird->last_key_code, ird->key_code);
					CLEAR_CODE(ird->key_code);
					IRC_TRACE_PRINTF("^_^ %08x!\n", ird->last_key_code[0]);
					if (last_key != ird->last_key_code[0] || ird->attr->repeat != NULL)
						key_cnt = 0;
					key_got_tick = osal_get_tick();
					if (ird->attr->type == IR_TYPE_RC6)
					{
						ird->last_key_code[0] &=(0x1 <<15) ;  // bit15 is toggle bit
					}
					return ird->last_key_code[0];
				}
				return PAN_KEY_INVALID;
			}
			else if (result == IRC_DECODE_FAIL)
			{
				#ifdef IRC_TRACE
				if (ird->attr->type == IRC_TRACE_TYPE)
				#endif
				IRC_DEBUG_PRINTF("decode failed @ step %d!\n", ird->decode_step);
				
				UINT16 step = ird->decode_step;
				ird->decode_step = 0;
				ird->key_bit_cnt = 0;
				CLEAR_CODE(ird->key_code);
				if (ird->attr->normal[step] != IR_LEADING)
					return PAN_KEY_INVALID;
			}
			else
			{
				ird->decode_step = 0;
				ird->key_bit_cnt = 0;
				CLEAR_CODE(ird->key_code);
				return PAN_KEY_INVALID;
			}
		}

		/* Process repeat key */
		if (ird->attr->repeat_enable && ird->attr->repeat)
		{
			enum ir_waveform step = ird->attr->repeat[ird->decode_step] + 1;
			
			if (INRANGE(accum_pulse_width, PULSE_VALUE(ird->attr->pulse[step].fst_half) + PULSE_VALUE(ird->attr->pulse[step].scd_half), ird->attr->pulse[step].tolerance))
				result = IRC_DECODE_SUCCEED;
			else
				result = IRC_DECODE_FAIL;
			
			if (result == IRC_DECODE_SUCCEED)
			{
				ird->decode_step++;
				if (ird->attr->repeat[ird->decode_step] == IR_END)
				{
					ird->decode_step = 0;
					ird->key_bit_cnt = 0;
					
					if (osal_get_tick() > (key_got_tick + 200))
					{
						ird->last_key_code[0] = PAN_KEY_INVALID;
						return PAN_KEY_INVALID;
					}
					key_got_tick = osal_get_tick();
					if (ird->attr->type == IR_TYPE_RC6)
					{
						ird->last_key_code[0] &=(0x1 <<15) ;  // bit15 is toggle bit
					}
					return ird->last_key_code[0];
				}
				return PAN_KEY_INVALID;
			}
			else if (result == IRC_DECODE_FAIL)
			{
				ird->decode_step = 0;
				ird->key_bit_cnt = 0;
				CLEAR_CODE(ird->key_code);
				return PAN_KEY_INVALID;
			}
			else
			{
				ird->decode_step = 0;
				ird->key_bit_cnt = 0;
				CLEAR_CODE(ird->key_code);
				return PAN_KEY_INVALID;
			}
		}
		else
		{
			if (accum_pulse_width < ird->attr->pulse_max_width)
				ird->last_pulse_width = accum_pulse_width;
			else
				ird->last_pulse_width = 0;
			
			ird->decode_step = 0;
			ird->key_bit_cnt = 0;
			CLEAR_CODE(ird->key_code);
			return PAN_KEY_INVALID;
		}
	}
}

/*****************************************************************************
 * UINT32 irc_pulse_to_code(UINT32 pulse_width)
 * Description: Translate pulse width to IR code
 *
 *		       From now on, irc_decode support fellow protocols :  
 *				NEC   LAB  50560  KF Logic  SRC  NSE  RC5  RC6_mode1  ----2010.2.11 by ryan.chen
 * Arguments:
 *    UINT32 pulse_width	: Input pulse width, in uS
 *    UINT32 pulse_polarity	: Input pulse polarity, 
 *								'1' = high level, '0' = low level
 *
 * Return Value:
 *    INT32				: Key code
 ****************************************************************************/
UINT32 irc_pulse_to_code(UINT32 pulse_width, UINT32 pulse_polarity)
{
	UINT32 i;
	UINT32 key_code = PAN_KEY_INVALID;

	//libc_printf("%d\t(%d)\n", pulse_width, pulse_polarity);

	for (i = 0; i< IRP_CNT; i++)
	{
		key_code = irc_decode(&irc_decoders[i], pulse_width, pulse_polarity);
		if (key_code != PAN_KEY_INVALID && key_code != 0)
		{
			//libc_printf("%s: Got key code %08X coded by %s.\n", __FUNCTION__, key_code, irp_strs[irc_decoders[i].attr->irp_type]);
			return key_code;
		}
	}

	if (i == IRP_CNT)
		return PAN_KEY_INVALID;
}


