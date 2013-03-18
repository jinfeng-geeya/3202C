#ifndef __HLD_SCART_H__
#define __HLD_SCART_H__

/*IO command relative parameters*/
#define SCART_PARAM_BASE		0
#define TV_MODE_RGB			(SCART_PARAM_BASE+1)
#define TV_MODE_CVBS			(SCART_PARAM_BASE+2)
#define TV_MODE_VCR			(SCART_PARAM_BASE+3)

#define ASPECT_4_3				(SCART_PARAM_BASE+4)
#define ASPECT_16_9				(SCART_PARAM_BASE+5)
#define ASPECT_INTERNAL		(SCART_PARAM_BASE+6)

#define SOURCE_TV_IN			(SCART_PARAM_BASE+7)
#define SOURCE_VCR_IN			(SCART_PARAM_BASE+8)
#define SOURCE_STB_IN			(SCART_PARAM_BASE+9)

#define SCART_STATE_VCR_IN		0x00000001
#define SCART_STATE_TV_IN		0x00000002

/*IO command list*/
#define SCART_IO_BASE			0x10000
#define SCART_TV_MODE			(SCART_IO_BASE+1)//TV_MODE_RGB:TV_MODE_CVBS:TV_MODE_VCR
#define SCART_TV_ASPECT		(SCART_IO_BASE+2)//ASPECT_4_3:ASPECT_16_9:ASPECT_INTERNAL
#define SCART_VCR_ASPECT		(SCART_IO_BASE+3)//ASPECT_4_3:ASPECT_16_9:ASPECT_INTERNAL
#define SCART_TV_SOURCE		(SCART_IO_BASE+4)//SOURCE_VCR_IN:SOURCE_STB_IN
#define SCART_VCR_SOURCE		(SCART_IO_BASE+5)//SOURCE_TV_IN:SOURCE_STB_IN
#define SCART_CHK_STATE		(SCART_IO_BASE+6)//Compare return value with SCART_STATE_VCR_IN
#define SCART_AUDIO_MUTE		(SCART_IO_BASE+7)// 0: MUTE, 1:RESUME
#define SCART_VCR_SB_OUT		(SCART_IO_BASE+8)// 0: VCR SB INPUT, 1:VCR SB OUTPUT
#define SCART_ENTRY_STADNBY		(SCART_IO_BASE+9)// 0: VCR SB INPUT, 1:VCR SB OUTPUT
#define SCART_REG_UPDATE		(SCART_IO_BASE+10)


struct scart_init_param
{
	UINT32 i2c_type_id;
	UINT32 vcr_plug_pos;
	UINT32 vcr_callback;
	UINT16 scart_volume; 	/*0~7.  =0: Mute; =1: -6dB; =2: -3dB; =3: 0dB, =4: 3dB; =5: 6dB; =6: 9dB; =7: 12dB*/
	UINT16 reserved3;
};

struct scart_device
{
	struct scart_device  *next;  /*next device */
	/*struct module *owner;*/
	INT32 type;
	INT8 name[HLD_MAX_NAME_SIZE];
	INT32 flags; //This field used to record current running status
	
	INT32 hardware;
	INT32 busy;   //This field used to record sub state of DECA_STATE_PLAY, could be: busy, no data, no buffer.
	INT32 minor;//This field used to record previous running status

	void *priv;		/* Used to be 'private' but that upsets C++ */
	UINT32 base_addr;
	UINT32 i2c_type_id;
	UINT32 vcr_plug_pos;
	UINT32 vcr_callback;
	/* Hardware privative structure */
/*	
 *  Functions of this RF modulator device
 */
	INT32	(*attach)(struct scart_init_param * param);

	INT32	(*open)(struct scart_device *dev);

	INT32	(*close)(struct scart_device *dev);

	INT32	(*ioctl)(struct scart_device *dev, INT32 cmd, UINT32 param);
};

#endif/*__HLD_SCART_H__*/
