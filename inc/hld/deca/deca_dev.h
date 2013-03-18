/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2002 Copyright (C)
 *
 *  File: deca_dev.h
 *
 *  Description: This file define the struct of audio decoder device.
 *               
 *  History:
 *      Date        Author         Version   Comment
 *      ====        ======         =======   =======
 *  1.  2004.07.15  Goliath Peng      0.1.000    Create.
 ****************************************************************************/
#ifndef _DECA_DEV_H_
#define _DECA_DEV_H_

#include <mediatypes.h>

// Audio decoder stop mode
enum ADecStopMode
{
	ADEC_STOP_IMM = 1,	// Audio decoder stop immediately
	ADEC_STOP_PTS,		// Audio decoder stop according to PTS
	ADEC_STOP_END		// No more data will be sent from parser to decoder,
						// and decoder will stop automatically after handle
						// all data
};

enum ADecSyncMode
{
	ADEC_SYNC_FREERUN = 1,	//Audio decoder just decode and send decoded frame to OUTPUT, not caring APTS and STC		
	ADEC_SYNC_PTS				//Audio decoder free run, but it will modify STC frequency according to the 
								//difference between STC value and APTS at output. 
								//And decoder need to compare APTS of 1st audio frame and STC to decide 
								//when to decode and send it to output.
};


struct deca_device
{
	struct deca_device  *next;  /*next device */
	/*struct module *owner;*/
	INT32 type;
	INT8 name[HLD_MAX_NAME_SIZE];
	INT32 flags; //This field used to record current running status
	
	INT32 hardware;
	INT32 busy;   //This field used to record sub state of DECA_STATE_PLAY, could be: busy, no data, no buffer.
	INT32 minor;//This field used to record previous running status

	void *priv;		/* Used to be 'private' but that upsets C++ */
	UINT32 base_addr;
	INT32 ase_flags; //This field used to record current running status of ASE task
	
	void             (*attach)(void);
	void             (*detach)(struct deca_device **);
	RET_CODE   (*open)(struct deca_device *, enum AudioStreamType, enum AudioSampleRate, enum AudioQuantization, UINT8, UINT32);
	RET_CODE   (*close)(struct deca_device *);
	RET_CODE   (*start)(struct deca_device *, UINT32);
	RET_CODE   (*stop)(struct deca_device *, UINT32, enum ADecStopMode);
	RET_CODE   (*pause)(struct deca_device *);
	RET_CODE   (*set_sync_mode)(struct deca_device *, enum ADecSyncMode);
	RET_CODE   (*ioctl)(struct deca_device *, UINT32 , UINT32);
	RET_CODE   (*request_write)(struct deca_device*, UINT32, void ** , UINT32*, struct control_block*);
	void             (*update_write)(struct deca_device *, UINT32);
	void (*pcm_buf_resume)(struct deca_device *);
	void (*tone_voice)(struct deca_device *, UINT32, UINT8);   //tone voice
	void (*stop_tone_voice)(struct deca_device *);
	RET_CODE   (*ase_cmd)(struct deca_device *, UINT32 , UINT32);
    /* add for audio description*/
    RET_CODE   (*request_desc_write)(struct deca_device*, UINT32, void ** , UINT32*, struct control_block*);
	void             (*update_desc_write)(struct deca_device *, UINT32);
    void    (*ase_init)(struct deca_device *);
};

#endif /* _DECA_DEV_H_ */


