/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2002 Copyright (C)
 *
 *  File: snd_dev.h
 *
 *  Description: This file define the struct of sound device.
 *  History:
 *      Date        Author         Version   Comment
 *      ====        ======         =======   =======
 *  1.  2004.07.15  Goliath Peng      0.1.000    Create.
 ****************************************************************************/
#ifndef _SND_DEV_H_
#define _SND_DEV_H_
#include <hld/hld_dev.h>
enum SndSubBlock
{
	SND_SUB_PP = 0x01,		// Audio post-process.
	SND_SUB_IN = 0x02,		// General audio input interface.
	SND_SUB_OUT = 0x04,		// General audio output interface.
	SND_SUB_MIC0 = 0x08,	// Micro phone 0 input interface.
	SND_SUB_MIC1 = 0x10,	// Micro phone 1 input interface.
	SND_SUB_SPDIFIN = 0x20,	// SPDIF input interface.
	SND_SUB_SPDIFOUT = 0x40,// SPDIF output interface.
	SND_SUB_SPDIFOUT_DDP = 0x80,
	SND_SUB_ALL	= 0xff		// All IO enabled.
};

enum SndDupChannel
{
	SND_DUP_NONE,
	SND_DUP_L,
	SND_DUP_R,
	SND_DUP_MONO
};


enum ASndOutSpdifType
{
	SND_OUT_SPDIF_INVALID = -1, 
	SND_OUT_SPDIF_PCM = 0,
	SND_OUT_SPDIF_BS = 1,
	SND_OUT_SPDIF_FORCE_DD = 2
};

//add for HDMI bs output src select 
enum SndSpoOutputSrcType
{
    SND_OUT_SRC_DDPSPO = 0,
    SND_OUT_SRC_SPO = 1
};

struct pcm_output{
	UINT32 ch_num ;
	UINT32 ch_mod;
	UINT32 samp_num ;
	UINT32 sample_rata_id;
	UINT32 inmode;
	UINT32 * ch_left ;
	UINT32 * ch_right ;
	UINT32 * ch_sl ;
	UINT32 * ch_sr ;
	UINT32 * ch_c ;
	UINT32 * ch_lfe ;
	UINT32 * ch_dl ;
	UINT32 * ch_dr ;
	UINT8 * raw_data_start;
	UINT32 raw_data_len;
	UINT32 iec_pc;
	
	UINT8 * raw_data_ddp_start; //KwunLeung
	UINT32 raw_data_ddp_len;
	UINT8 iec_pc_ddp;
};

enum EQ_TYPE {	
	EQ_SLIGHT=0,
	EQ_CLASSIC=1,
	EQ_ELECTRONIC=2,
	EQ_DANCE=3,
	EQ_LIVE=4,
	EQ_POP=5,
	EQ_ROCK=6,
};

struct snd_output_cfg
{
	UINT8 mute_num; //mute circuit gpio number.
	UINT8 mute_polar; //the polarity which will cause circuit mute 
	UINT8 dac_precision;//24bit or 16bit 
	UINT8 dac_format;//CODEC_I2S (0x0<<1), CODEC_LEFT (0x1<<1), CODEC_RIGHT (0x2<<1)
	UINT8 is_ext_dac; //for M3329 serial, always should be 1. 0: means embedded dac.	
	UINT8 reserved8;
	UINT16 gpio_mute_circuit:1; //FALSE: no mute circuit; TRUE: exists mute circuit controlled by GPIO
	UINT16 ext_mute_mode:2;
	UINT16 enable_hw_accelerator:1; 	//FALSE: do not enable M3202 audio HW accelerator;
									//TRUE: Enable M3202 audio HW accelerator;
    UINT8 chip_type_config:1;      //1:QFP.0:BGA.
    UINT16 reserved:11;
};

struct snd_device
{
	struct snd_device  *next;  /*next device */
	/*struct module *owner;*/
	INT32 type;
	INT8 name[HLD_MAX_NAME_SIZE];
	INT32 flags;

	INT32 hardware;
	INT32 busy;
	INT32 minor;

	void *priv;		/* Used to be 'private' but that upsets C++ */
	UINT32 base_addr;
	
	void      (*attach)(void);
	void      (*detach)(struct snd_device **);
	RET_CODE   (*open)(struct snd_device *);
	RET_CODE   (*close)(struct snd_device *);
	RET_CODE   (*set_mute)(struct snd_device *, enum SndSubBlock, UINT8);
	RET_CODE   (*set_volume)(struct snd_device *, enum SndSubBlock, UINT8);
	RET_CODE   (*set_sub_blk)(struct snd_device *, UINT8 , UINT8);
	RET_CODE   (*set_duplicate)(struct snd_device *, enum SndDupChannel);
	RET_CODE   (*request_pcm_buff)(struct snd_device *, UINT32);
	RET_CODE   (*data_enough)(struct snd_device *);
	RET_CODE   (*config)(struct snd_device *, UINT32, UINT16, UINT8);
	RET_CODE   (*set_spdif_type)(struct snd_device *, enum ASndOutSpdifType);
	RET_CODE   (*ioctl)(struct snd_device *, UINT32 , UINT32);
	void (*write_pcm_data)(struct snd_device*,struct pcm_output*,UINT32*);
	void (*write_pcm_data2)(struct snd_device *, UINT32 *, UINT32 *, UINT32 *, UINT32, UINT32);
	RET_CODE (*snd_get_stc)(UINT32, UINT32 *, UINT8);
	void (*snd_set_stc)(UINT32, UINT32, UINT8);
	void (*snd_get_divisor)(UINT32, UINT16 *, UINT8);
	void (*snd_set_divisor)(UINT32, UINT16, UINT8);
	void (*snd_stc_pause)(UINT32, UINT8, UINT8);
	void (*snd_invalid_stc)(void);
	void (*snd_valid_stc)(void);
	void (*start)(struct snd_device *);
	void (*stop)(struct snd_device *);
	UINT8 (*get_volume)(struct snd_device *);
	RET_CODE (*ena_pp_8ch)(struct snd_device * ,UINT8);
	RET_CODE (*set_pp_delay)(struct snd_device * ,UINT8);
	RET_CODE (*enable_virtual_surround)(struct snd_device *,UINT8);
	RET_CODE (*enable_eq)(struct snd_device * ,UINT8 ,enum EQ_TYPE);
	RET_CODE (*enable_bass)(struct snd_device *,UINT8);
#if 1	
	int (*gen_tone_voice)(struct snd_device *, struct pcm_output* , UINT8); //tone voice
	void (*stop_tone_voice)(struct snd_device *);  //tone voice
#endif
	void (*output_config)(struct snd_device *, struct snd_output_cfg *);
	RET_CODE (*spectrum_cmd)(struct snd_device *, UINT32 , UINT32);

    RET_CODE (*request_desc_pcm_buff)(struct snd_device *, UINT32);
    void (*write_desc_pcm_data)(struct snd_device*,struct pcm_output*,UINT32*);
};

#endif /* _SND_DEV_H_ */

