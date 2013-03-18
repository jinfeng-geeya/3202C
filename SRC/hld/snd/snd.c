#include <sys_config.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>
#include <hld/snd/snd_dev.h>
#include <hld/snd/snd.h>
#include <hld/deca/deca_dev.h>
#include <hld/deca/deca.h>

struct deca_device * g_deca_snd_ref ;
struct snd_device * g_snd_stc_ref = NULL;
UINT8 snd_mute_num; 
UINT8 snd_mute_polar; 
UINT8 snd_is_ext_dac;
UINT8 snd_dac_format=CODEC_LEFT; 
UINT8 snd_dac_precision = 24;
UINT8 snd_contain_embedded_dac = 0;
UINT8 snd_support_spdif_mute = 0;
UINT8 snd_support_swap_lr_ch = 0;
UINT8 snd_dis_clk_while_ch_chg = 1;
UINT8 snd_ext_mute_mode = MUTE_BY_SCART;
UINT8 snd_chip_type_config;

UINT32 stc_delay = STC_DELAY;
__ATTRIBUTE_RAM_
RET_CODE get_stc(UINT32 * stc_msb32, UINT8 stc_num)
{
	if(NULL==g_snd_stc_ref)
		g_snd_stc_ref = (struct snd_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_SND);
	if((!g_snd_stc_ref) || (!g_snd_stc_ref->snd_get_stc) || (SND_STATE_ATTACH == g_snd_stc_ref->flags))
		return !RET_SUCCESS;
	return g_snd_stc_ref->snd_get_stc(g_snd_stc_ref->base_addr, stc_msb32, stc_num);
}

__ATTRIBUTE_RAM_
void set_stc(UINT32 stc_msb32, UINT8 stc_num)
{
	if((g_snd_stc_ref) && (g_snd_stc_ref->snd_set_stc) && (SND_STATE_ATTACH != g_snd_stc_ref->flags))
		g_snd_stc_ref->snd_set_stc(g_snd_stc_ref->base_addr, stc_msb32, stc_num);
}

__ATTRIBUTE_RAM_
void get_stc_divisor(UINT16 * stc_divisor, UINT8 stc_num)
{
	if(g_snd_stc_ref&&g_snd_stc_ref->snd_get_divisor)
		g_snd_stc_ref->snd_get_divisor(g_snd_stc_ref->base_addr, stc_divisor, stc_num);
}

__ATTRIBUTE_RAM_
void set_stc_divisor(UINT16 stc_divisor, UINT8 stc_num)
{
	if(g_snd_stc_ref&&g_snd_stc_ref->snd_set_divisor)
		g_snd_stc_ref->snd_set_divisor(g_snd_stc_ref->base_addr, stc_divisor, stc_num);
}

__ATTRIBUTE_RAM_
void stc_pause(UINT8 pause, UINT8 stc_num)
{
	if((g_snd_stc_ref) && (g_snd_stc_ref->snd_stc_pause) && (SND_STATE_ATTACH != g_snd_stc_ref->flags))
		g_snd_stc_ref->snd_stc_pause(g_snd_stc_ref->base_addr, pause, stc_num);
}

__ATTRIBUTE_RAM_
void stc_invalid(void)
{
	if(g_snd_stc_ref&&g_snd_stc_ref->snd_invalid_stc)
		g_snd_stc_ref->snd_invalid_stc();
}

void stc_valid(void)
{
	if(g_snd_stc_ref&&g_snd_stc_ref->snd_valid_stc)
		g_snd_stc_ref->snd_valid_stc();
}

void snd_output_config(struct snd_device * dev, struct snd_output_cfg * cfg_param)
{
	if(dev&&dev->output_config)
		dev->output_config(dev, cfg_param);
}
__ATTRIBUTE_REUSE_
RET_CODE snd_open(struct snd_device * dev)
{
	if(dev == NULL)return !RET_SUCCESS;

	if(dev->flags != SND_STATE_ATTACH)
	{
		//SND_PRINTF("snd_open: warning - device %s is not in ATTACH status!\n", dev->name);
		return !RET_SUCCESS;
	}
	/* Open this device */
	if (NULL!=dev->open)
	{
		return dev->open(dev);
	}
	return  !RET_SUCCESS;
}

RET_CODE snd_close(struct snd_device * dev)
{
       if(dev == NULL)return !RET_SUCCESS;

	if (dev->flags != SND_STATE_IDLE)
	{
		//SND_PRINTF("snd_close: warning - device %s is not in IDLE status!\n", dev->name);
		return !RET_SUCCESS;
	}
	/* close device */
	if ( NULL!= dev->close)
	{
		return dev->close(dev);
	}
	return !RET_SUCCESS;
}

RET_CODE snd_set_mute(struct snd_device * dev, enum SndSubBlock sub_blk, UINT8 enable)
{
	if(dev == NULL)return !RET_SUCCESS;

	if (dev->flags != SND_STATE_IDLE&&dev->flags != SND_STATE_PLAY)
	{
		//SND_PRINTF("snd_set_mute: warning - device %s is not in IDLE or PLAY status!\n", dev->name);
		return !RET_SUCCESS;
	}

	if ( NULL!= dev->set_mute)
	{
		return dev->set_mute(dev, sub_blk, enable);
	}
	return !RET_SUCCESS;
}

RET_CODE snd_set_volume(struct snd_device * dev, enum SndSubBlock sub_blk, UINT8 volume)
{
	if(dev == NULL)return !RET_SUCCESS;

	if (dev->flags != SND_STATE_IDLE&&dev->flags != SND_STATE_PLAY)
	{
		//SND_PRINTF("snd_set_volume: warning - device %s is not in IDLE or PLAY status!\n", dev->name);
		return !RET_SUCCESS;
	}

	if ( NULL!= dev->set_volume)
	{
		return dev->set_volume(dev, sub_blk, volume);
	}
	return !RET_SUCCESS;
}

UINT8 snd_get_volume(struct snd_device * dev)
{
	if(dev!=NULL&&dev->get_volume!=NULL)
		return dev->get_volume(dev);
	return 0;
}

RET_CODE snd_data_enough(struct snd_device * dev)
{
	if(dev == NULL)
	{
		//SND_PRINTF("snd_request_pcm_buff: warning - device %s dev == NULL!\n", dev->name);
		return !RET_SUCCESS;
	}
	if (dev->flags != SND_STATE_PLAY)//if (dev->flags != SND_STATE_PLAY)
	{
		//SND_PRINTF("snd_request_pcm_buff: warning - device %s is not in IDLE or PLAY status!\n", dev->name);
		return !RET_SUCCESS;
	}

	if ( NULL!= dev->data_enough)
	{
		return dev->data_enough(dev);
	}
	//SND_PRINTF("snd_request_pcm_buff: warning - device %s dev->request_pcm_buff== NULL!\n", dev->name);
	return !RET_SUCCESS;
}

RET_CODE snd_request_pcm_buff(struct snd_device * dev, UINT32 size)
{
	if(dev == NULL)
	{
		//SND_PRINTF("snd_request_pcm_buff: warning - device %s dev == NULL!\n", dev->name);
		return !RET_SUCCESS;
	}
	if (dev->flags != SND_STATE_IDLE&&dev->flags != SND_STATE_PLAY)//if (dev->flags != SND_STATE_PLAY)
	{
		//SND_PRINTF("snd_request_pcm_buff: warning - device %s is not in IDLE or PLAY status!\n", dev->name);
		return !RET_SUCCESS;
	}

	if ( NULL!= dev->request_pcm_buff)
	{
		return dev->request_pcm_buff(dev, size);
	}
	//SND_PRINTF("snd_request_pcm_buff: warning - device %s dev->request_pcm_buff== NULL!\n", dev->name);
	return !RET_SUCCESS;
}

__ATTRIBUTE_RAM_
void snd_write_pcm_data(struct snd_device*dev,struct pcm_output*pcm,UINT32*frame_header)
{
	if(dev == NULL)return;

	if (dev->flags != SND_STATE_IDLE&&dev->flags != SND_STATE_PLAY)
	//if (dev->flags != SND_STATE_PLAY)
	{
		//SND_PRINTF("snd_write_pcm_data: warning - device %s is not in IDLE or PLAY status!\n", dev->name);
		return;
	}

	if ( NULL!= dev->write_pcm_data)
	{
		dev->write_pcm_data(dev, pcm, frame_header);
	}
	return;
}

void snd_write_pcm_data2(struct snd_device * dev, UINT32 * frame_header,
	                                                UINT32 * left, UINT32 * right, UINT32 number, UINT32 ch_num)
{
	if(dev == NULL)return;

	//if (dev->flags != SND_STATE_IDLE&&dev->flags != SND_STATE_PLAY)
	if (dev->flags != SND_STATE_PLAY)
	{
		//SND_PRINTF("snd_write_pcm_data: warning - device %s is not in IDLE or PLAY status!\n", dev->name);
		return;
	}

	if ( NULL!= dev->write_pcm_data2)
	{
		dev->write_pcm_data2(dev, frame_header, left, right, number, ch_num);
	}
	return;
}

RET_CODE snd_io_control(struct snd_device * dev, UINT32 cmd, UINT32 param)
{
	if(NULL==dev)
		return !RET_SUCCESS;
	if(NULL!=dev->spectrum_cmd)
	{
		switch(cmd)
		{
		case SND_SPECTRUM_START:
		case SND_SPECTRUM_STOP:
		case SND_SPECTRUM_CLEAR:
		case SND_SPECTRUM_STEP_TABLE:
		case SND_SPECTRUM_VOL_INDEPEND:
		case SND_SPECTRUM_CAL_COUNTER:
			return dev->spectrum_cmd(dev, cmd, param);
		default:
			break;
		}
	}
	if(NULL!=dev->ioctl)
	{
		return dev->ioctl(dev, cmd, param);
	}
	return !RET_SUCCESS;
}

RET_CODE snd_set_sub_blk(struct snd_device * dev, UINT8 sub_blk, UINT8 enable)
{
	if(dev == NULL)return !RET_SUCCESS;

	if (dev->flags != SND_STATE_IDLE&&dev->flags != SND_STATE_PLAY)
	{
		//SND_PRINTF("snd_set_sub_blk: warning - device %s is not in IDLE or PLAY status!\n", dev->name);
		return !RET_SUCCESS;
	}
	/* close device */
	if ( NULL!= dev->set_sub_blk)
	{
		return dev->set_sub_blk(dev, sub_blk, enable);
	}
	return !RET_SUCCESS;
}

RET_CODE snd_set_duplicate(struct snd_device * dev, enum SndDupChannel channel)
{
	if(dev == NULL)return !RET_SUCCESS;

	if (dev->flags != SND_STATE_IDLE&&dev->flags != SND_STATE_PLAY)
	{
		//SND_PRINTF("snd_set_duplicate: warning - device %s is not in IDLE or PLAY status!\n", dev->name);
		return !RET_SUCCESS;
	}
	/* close device */
	if ( NULL!= dev->set_duplicate)
	{
		return dev->set_duplicate(dev, channel);
	}
	return !RET_SUCCESS;
}

RET_CODE snd_set_spdif_type(struct snd_device * dev, enum ASndOutSpdifType type)
{
	if(dev == NULL)return !RET_SUCCESS;
	if(dev->flags != SND_STATE_IDLE)
	{
		SND_PRINTF("snd_set_spdif_type: warning - device %s is not in IDLE status!\n", dev->name);
		return !RET_SUCCESS;
	}
	if(NULL!= dev->set_spdif_type)
	{
		return dev->set_spdif_type(dev, type);
	}
	return !RET_SUCCESS;
}

RET_CODE snd_config(struct snd_device * dev, UINT32 sample_rate, UINT16 sample_num, UINT8 precision)
{
	if(dev == NULL)return !RET_SUCCESS;

	if (dev->flags != SND_STATE_IDLE)
	{
		//SND_PRINTF("snd_config: warning - device %s is not in IDLE status!\n", dev->name);
		return !RET_SUCCESS;
	}
	/* start device */
	if ( NULL!= dev->config)
	{
		return dev->config(dev, sample_rate, sample_num, precision);
	}
	return !RET_SUCCESS;
}

void snd_start(struct snd_device * dev)
{
	if(dev == NULL)
	{
		//SND_PRINTF("snd_stop: dev==NULL.\n");
		return;
	}

	if (dev->flags != SND_STATE_IDLE)
	{
		//SND_PRINTF("snd_start: warning - device %s is not in IDLE status!\n", dev->name);
		return ;
	}
	/* start device */
	if ( NULL== dev->start)
	{
		//SND_PRINTF("snd_start: dev->start==NULL.\n");
		return;
	}
	dev->start(dev);
}

void snd_stop(struct snd_device*dev)
{
	if(dev == NULL)
	{
		//SND_PRINTF("snd_stop: dev==NULL.\n");
		return;
       }
	//if (dev->flags != SND_STATE_PLAY)
	if (dev->flags != SND_STATE_IDLE&&dev->flags != SND_STATE_PLAY)//if (dev->flags != SND_STATE_PLAY)
	{
		//SND_PRINTF("snd_stop: warning - device %s is not in PLAY status!\n", dev->name);
		return ;
	}
	/* start device */
	if ( NULL== dev->stop)
	{
		//SND_PRINTF("snd_stop: dev->stop==NULL.\n");
		return ;
	}
	dev->stop(dev);
}

void snd_gen_tone_voice(struct snd_device * dev, struct pcm_output*pcm, UINT8 init)  //tone voice
{

	if(dev == NULL)
	{
		return;
	}


	if(NULL== dev->gen_tone_voice)
		return;
	dev->gen_tone_voice(dev, pcm, init);
}

void snd_stop_tone_voice(struct snd_device * dev)  //tone voice
{
	if(dev == NULL)
		return;



	if(NULL== dev->stop_tone_voice)
		return;

	dev->stop_tone_voice(dev);
}

RET_CODE snd_ena_pp_8ch(struct snd_device * dev, UINT8 enable)
{
	if(dev == NULL)return !RET_SUCCESS;

	if (dev->flags != SND_STATE_IDLE&&dev->flags != SND_STATE_PLAY)
	{
		//SND_PRINTF("snd_set_volume: warning - device %s is not in IDLE or PLAY status!\n", dev->name);
		return !RET_SUCCESS;
	}

	if ( NULL!= dev->ena_pp_8ch)
	{
		return dev->ena_pp_8ch(dev, enable);
	}
	return !RET_SUCCESS;
}

RET_CODE snd_set_pp_delay(struct snd_device * dev, UINT8 delay)
{
	if(dev == NULL)return !RET_SUCCESS;

	if (dev->flags != SND_STATE_IDLE&&dev->flags != SND_STATE_PLAY)
	{
		//SND_PRINTF("snd_set_volume: warning - device %s is not in IDLE or PLAY status!\n", dev->name);
		return !RET_SUCCESS;
	}

	if ( NULL!= dev->set_pp_delay)
	{
		return dev->set_pp_delay(dev, delay);
	}
	return !RET_SUCCESS;
}

RET_CODE snd_enable_virtual_surround(struct snd_device * dev, UINT8 enable)
{
	if(dev == NULL)return !RET_SUCCESS;

	if (dev->flags != SND_STATE_IDLE&&dev->flags != SND_STATE_PLAY)
	{
		//SND_PRINTF("snd_set_volume: warning - device %s is not in IDLE or PLAY status!\n", dev->name);
		return !RET_SUCCESS;
	}

	if ( NULL!= dev->enable_virtual_surround)
	{
		return dev->enable_virtual_surround(dev, enable);
	}
	return !RET_SUCCESS;
}

RET_CODE snd_enable_eq(struct snd_device * dev, UINT8 enable,enum EQ_TYPE type)
{
	if(dev == NULL)return !RET_SUCCESS;

	if (dev->flags != SND_STATE_IDLE&&dev->flags != SND_STATE_PLAY)
	{
		//SND_PRINTF("snd_set_volume: warning - device %s is not in IDLE or PLAY status!\n", dev->name);
		return !RET_SUCCESS;
	}

	if ( NULL!= dev->enable_eq)
	{
		return dev->enable_eq(dev, enable,type);
	}
	return !RET_SUCCESS;
}

RET_CODE snd_enable_bass(struct snd_device * dev, UINT8 enable)
{
	if(dev == NULL)return !RET_SUCCESS;

	if (dev->flags != SND_STATE_IDLE&&dev->flags != SND_STATE_PLAY)
	{
		//SND_PRINTF("snd_set_volume: warning - device %s is not in IDLE or PLAY status!\n", dev->name);
		return !RET_SUCCESS;
	}

	if ( NULL!= dev->enable_bass)
	{
		return dev->enable_bass(dev, enable);
	}
	return !RET_SUCCESS;
}


RET_CODE snd_request_desc_pcm_buff(struct snd_device * dev, UINT32 size)
{
	if(dev == NULL)
	{
		//SND_PRINTF("snd_request_pcm_buff: warning - device %s dev == NULL!\n", dev->name);
		return !RET_SUCCESS;
	}
	if (dev->flags != SND_STATE_IDLE&&dev->flags != SND_STATE_PLAY)//if (dev->flags != SND_STATE_PLAY)
	{
		//SND_PRINTF("snd_request_pcm_buff: warning - device %s is not in IDLE or PLAY status!\n", dev->name);
		return !RET_SUCCESS;
	}

	if ( NULL!= dev->request_desc_pcm_buff)
	{
		return dev->request_desc_pcm_buff(dev, size);
	}
	//SND_PRINTF("snd_request_pcm_buff: warning - device %s dev->request_pcm_buff== NULL!\n", dev->name);
	return !RET_SUCCESS;
}

void snd_write_desc_pcm_data(struct snd_device*dev,struct pcm_output*pcm,UINT32*frame_header)
{
	if(dev == NULL)return;

	if (dev->flags != SND_STATE_IDLE&&dev->flags != SND_STATE_PLAY)
	//if (dev->flags != SND_STATE_PLAY)
	{
		//SND_PRINTF("snd_write_pcm_data: warning - device %s is not in IDLE or PLAY status!\n", dev->name);
		return;
	}

	if ( NULL!= dev->write_desc_pcm_data)
	{
		dev->write_desc_pcm_data(dev, pcm, frame_header);
	}
	return;
}

