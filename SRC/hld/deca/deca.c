#include <sys_config.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>
#include <hld/deca/deca_dev.h>
#include <hld/deca/deca.h>
#include <hld/snd/snd_dev.h>
#include <hld/snd/snd.h>

//struct snd_device * g_snd_deca_ref ;
void (*fp_deca_pcm_process_func)(UINT32, UINT8 *, UINT32, UINT32, UINT32) = NULL;
void (*fp_deca_process_pcm_bitstream)(UINT32, UINT8 *, UINT32, UINT8 *, UINT32, UINT32, UINT32) = NULL;
void deca_pcm_buf_resume(UINT32 val)
{
	//DECA_PRINTF("*****Enter PCM buffer resume in HLD. *****\n");
	struct deca_device * dev = (struct deca_device *)val;
	if(dev!=NULL&&dev->pcm_buf_resume!=NULL)
	{
		//DECA_PRINTF("***** pcm buf resume in hsr. *****\n");
		dev->pcm_buf_resume(dev);
	}
}

__ATTRIBUTE_REUSE_
RET_CODE deca_open(struct deca_device * dev,
                                enum AudioStreamType stream_type, 
                                enum AudioSampleRate samp_rate, 
                                enum AudioQuantization quan,
                                UINT8 channel_num,
                                UINT32 info_struct)
{
	if(NULL==dev)return !RET_SUCCESS;
     
	if(DECA_STATE_ATTACH!=dev->flags)
	{
		//DECA_PRINTF("deca_open: warning- device is not in ATTACH status! \n");
		return !RET_SUCCESS;
	}

	if(NULL!=dev->open)
	{
		return dev->open(dev, stream_type, samp_rate, quan, channel_num, info_struct);
	}
	return !RET_SUCCESS;
}

RET_CODE deca_close(struct deca_device * dev)
{
	if(NULL==dev)return !RET_SUCCESS;
     
	if(DECA_STATE_IDLE!=dev->flags)
	{
		//DECA_PRINTF("deca_close: warning- device is not in IDLE status! \n");
		return !RET_SUCCESS;
	}

	if(NULL!=dev->close)
	{
		return dev->close(dev);
	}
	return !RET_SUCCESS;
}

RET_CODE deca_set_sync_mode(struct deca_device * dev, enum ADecSyncMode mode)
{
	if(NULL==dev)return !RET_SUCCESS;
     
	if(DECA_STATE_ATTACH==dev->flags)
	{
		//DECA_PRINTF("deca_set_sync_mode: warning- device is not in right status! \n");
		return !RET_SUCCESS;
	}

	if(NULL!=dev->set_sync_mode)
	{
		return dev->set_sync_mode(dev, mode);
	}
	return !RET_SUCCESS;
}

RET_CODE deca_start(struct deca_device * dev, UINT32 high32_pts)
{
	if(NULL==dev)return !RET_SUCCESS;
     
	if((DECA_STATE_IDLE!=dev->flags)&&(DECA_STATE_PAUSE!=dev->flags))
	{
		//DECA_PRINTF("deca_start: warning- device is not in right status! \n");
		return !RET_SUCCESS;
	}

	if(NULL!=dev->start)
	{
		return dev->start(dev, high32_pts);
	}
	return !RET_SUCCESS;
}

RET_CODE deca_stop(struct deca_device * dev, UINT32 high32_pts, enum ADecStopMode mode)
{
	if(NULL==dev)return !RET_SUCCESS;
     
	if((DECA_STATE_PLAY!=dev->flags)&&(DECA_STATE_PAUSE!=dev->flags))
	{
		//DECA_PRINTF("deca_stop: warning- device is not in right status! \n");
		return !RET_SUCCESS;
	}

	if(NULL!=dev->stop)
	{
		return dev->stop(dev, high32_pts, mode);
	}
	return !RET_SUCCESS;
}

RET_CODE deca_pause(struct deca_device * dev)
{
	if(NULL==dev)return !RET_SUCCESS;
     
	if(DECA_STATE_PLAY!=dev->flags)
	{
		//DECA_PRINTF("deca_pause: warning- device is not in PLAY status! \n");
		return !RET_SUCCESS;
	}

	if(NULL!=dev->pause)
	{
		return dev->pause(dev);
	}
	return !RET_SUCCESS;
}

RET_CODE deca_io_control(struct deca_device * dev, UINT32 cmd, UINT32 param)
{
	if(NULL==dev)return !RET_SUCCESS;
     
	if(DECA_STATE_ATTACH==dev->flags)
	{
		//DECA_PRINTF("deca_io_control: warning- device is not in right status! \n");
		return !RET_SUCCESS;
	}
	if(NULL!=dev->ase_cmd)
	{
		switch(cmd)
		{
		case DECA_AUDIO_KEY:
		case DECA_BEEP_INTERVAL:
		case DECA_STR_PLAY:
		case DECA_STR_PAUSE:
		case DECA_STR_RESUME:
		case DECA_STR_STOP:
			return dev->ase_cmd(dev, cmd, param);
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


RET_CODE deca_request_write(void  * device, UINT32 req_size, 
	                                        void ** ret_buf, UINT32 * ret_buf_size,
	                                        struct control_block * ctrl_blk)
{
	struct deca_device * dev = (struct deca_device *)device;
     
	if(NULL==dev)
	{
		return RET_STA_ERR;
	}
    
	//if((DECA_STATE_PLAY!=dev->flags)&&(DECA_STATE_PAUSE!=dev->flags))
	if(DECA_STATE_PLAY!=dev->flags)
	{
		//DECA_PRINTF("deca_request_write: warning- device is not in right status!addr%08x flags%08x \n", dev, dev->flags);
		return RET_STA_ERR;
	}

	if(NULL!=dev->request_write)
	{
		return dev->request_write(dev, req_size, ret_buf, ret_buf_size, ctrl_blk);
	}
	return RET_STA_ERR;
}

RET_CODE deca_request_desc_write(void  * device, UINT32 req_size, 
	                                        void ** ret_buf, UINT32 * ret_buf_size,
	                                        struct control_block * ctrl_blk)
{
	struct deca_device * dev = (struct deca_device *)device;
     
	if(NULL==dev)
	{
		return RET_STA_ERR;
	}
    
	//if((DECA_STATE_PLAY!=dev->flags)&&(DECA_STATE_PAUSE!=dev->flags))
	if(DECA_STATE_PLAY!=dev->flags)
	{
		//DECA_PRINTF("deca_request_write: warning- device is not in right status!addr%08x flags%08x \n", dev, dev->flags);
		return RET_STA_ERR;
	}

	if(NULL!=dev->request_desc_write)
	{
		return dev->request_desc_write(dev, req_size, ret_buf, ret_buf_size, ctrl_blk);
	}
	return RET_STA_ERR;
}

void deca_update_write(void * device, UINT32 size)
{
	struct deca_device * dev = (struct deca_device *)device;
     
	if(NULL==dev)
	{
		return ;
	}
     
	//if((DECA_STATE_PLAY!=dev->flags)&&(DECA_STATE_PAUSE!=dev->flags))
	if(DECA_STATE_PLAY!=dev->flags)
	{
		//DECA_PRINTF("deca_update_write: warning- device is not in right status! \n");
		return ;
	}

	if(NULL!=dev->update_write)
	{
		dev->update_write(dev, size);
	}

	return;
}

void deca_update_desc_write(void * device, UINT32 size)
{
	struct deca_device * dev = (struct deca_device *)device;
     
	if(NULL==dev)
	{
		return ;
	}
     
	//if((DECA_STATE_PLAY!=dev->flags)&&(DECA_STATE_PAUSE!=dev->flags))
	if(DECA_STATE_PLAY!=dev->flags)
	{
		//DECA_PRINTF("deca_update_write: warning- device is not in right status! \n");
		return ;
	}

	if(NULL!=dev->update_desc_write)
	{
		dev->update_desc_write(dev, size);
	}

	return;
}

void deca_tone_voice(struct deca_device * dev, UINT32 SNR, UINT8 init)  //tone voice// clear waring 050907 ming yi
{
	if(NULL==dev)return;
 
    
	if(NULL==dev->tone_voice)
	{
		return; 
	}
	dev->tone_voice(dev, SNR, init);
}

void deca_stop_tone_voice(struct deca_device * dev)  //tone voice // clear waring 050907 ming yi
{
	if(NULL==dev)return;
 

	if(NULL ==dev->stop_tone_voice)
	{
		return; 
	}
	dev->stop_tone_voice(dev);
}

void deca_process_pcm_samples(UINT32 pcm_bytes_len, UINT8 *pcm_raw_buf, UINT32 sample_rate, UINT32 channel_num, UINT32 sample_precision)
{
	if(NULL!=fp_deca_pcm_process_func)
		fp_deca_pcm_process_func(pcm_bytes_len, pcm_raw_buf, sample_rate, channel_num, sample_precision);
	else
	{
		deca_io_control((struct deca_device * )dev_get_by_type(NULL, HLD_DEV_TYPE_DECA), 
		DECA_REG_PCM_PROCESS_FUNC, (UINT32)(&fp_deca_pcm_process_func));
		if(NULL!=fp_deca_pcm_process_func)
			fp_deca_pcm_process_func(pcm_bytes_len, pcm_raw_buf, sample_rate, channel_num, sample_precision);
	}
}

void deca_process_pcm_bitstream(UINT32 pcm_bytes_len, UINT8 *pcm_raw_buf,
    UINT32 bs_length, UINT8 *un_processed_bs, UINT32 sample_rate, UINT32 channel_num, UINT32 sample_precision)
{
    if(NULL!=fp_deca_process_pcm_bitstream)
        fp_deca_process_pcm_bitstream(pcm_bytes_len, pcm_raw_buf,
            bs_length, un_processed_bs, sample_rate, channel_num, sample_precision);
    else
    {
        deca_io_control((struct deca_device * )dev_get_by_type(NULL, HLD_DEV_TYPE_DECA),
        DECA_REG_PCM_BS_PROCESS_FUNC, (UINT32)(&fp_deca_process_pcm_bitstream));
        if(NULL!=fp_deca_process_pcm_bitstream)
            fp_deca_process_pcm_bitstream(pcm_bytes_len, pcm_raw_buf,
            bs_length, un_processed_bs, sample_rate, channel_num, sample_precision);
    }
}

void deca_init_ase(struct deca_device * dev)
{

	if(NULL==dev)return;
     
	if(DECA_STATE_IDLE!=dev->flags)
	{
		//DECA_PRINTF("deca_open: warning- device is not in ATTACH status! \n");
		return;
	}

	if(NULL!=dev->ase_init)
	{
		return dev->ase_init(dev);
	}
}

////////////////////////////////////////////////////////////////////////////////
#if 0
RET_CODE deca_set_speed(struct deca_device * dev, enum ADecPlaySpeed speed)
{
	return RET_SUCCESS;
}

UINT32 deca_get_current_out_pts(struct deca_device * dev)
{
	return 0;
}

RET_CODE deca_set_dm_tbl(struct deca_device * dev, struct ADecDownMixTable * table_buf)
{
	return RET_SUCCESS;
}

RET_CODE deca_set_channel_order(struct deca_device * dev, enum ADecChannel * channel_order)
{
	return RET_SUCCESS;
}

RET_CODE deca_en_channel(struct deca_device * dev, UINT8 flag)
{
	return RET_SUCCESS;
}
#endif


