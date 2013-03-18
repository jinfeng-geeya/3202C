/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 Copyright (C)
*
*    File:    nim.c
*
*    Description:    This file contains all functions definition
*		             of TDS NIM driver.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	Feb.16.2003      Justin Wu       Ver 0.1    Create file.
*	2.	Aug.21.2003      Justin Wu       Ver 0.2    Update interface.
*****************************************************************************/

#include <types.h>
#include <retcode.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <osal/osal_int.h>
#include <hld/hld_dev.h>

#include <hld/nim/nim_dev.h>
#include <hld/nim/nim.h>

/*****************************************************************************
* INT32 nim_open(struct nim_device *dev)
*
* NIM open operation
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_open(struct nim_device *dev)
{
	INT32 result = SUCCESS;

    if (dev == NULL)
        return RET_SUCCESS;

	/* If openned already, exit */
	if (dev->flags & HLD_DEV_STATS_UP)
	{
#ifdef	SWITCH_NET_DEBUG_INFORMATION
		PRINTF("nim_open: warning - device %s openned already!\n", dev->name);
#endif
		return SUCCESS;
	}

	/* Open this device */
	if (dev->open)
	{
		result = dev->open(dev);
	}

	/* Setup init work mode */
	if (result == SUCCESS)
	{
		dev->flags |= (HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);

	}

	return result;
}

/*****************************************************************************
* INT32 nim_close(struct nim_device *dev)
*
* NIM close operation
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_close(struct nim_device *dev)
{
	INT32 result = SUCCESS;

    if (dev == NULL)
        return RET_SUCCESS;

	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
#ifdef	SWITCH_NET_DEBUG_INFORMATION
		PRINTF("nim_close: warning - device %s closed already!\n", dev->name);
#endif
		return SUCCESS;
	}

	/* Stop device */
	if (dev->stop)
	{
		result = dev->stop(dev);
	}

	/* Update flags */
	if (result == SUCCESS)
	{
		dev->flags &= ~(HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);
	}

	return result;
}

/*****************************************************************************
* INT32 nim_ioctl_ext(struct nim_device *dev, INT32 cmd, void * param_list);
*
* NIM device input/output control Extersion
*
* Arguments:
*  Parameter1: struct nim_device *dev	: Device
*  Parameter2: INT32  cmd				: Cmd Code
*  Parameter3: void*  param_list		: the pointer to the Parameter List
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_ioctl_ext(struct nim_device *dev, INT32 cmd, void * param_list)
{
	
    if (dev == NULL)
        return RET_SUCCESS;
	
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

    	if (dev->do_ioctl_ext)
    	{
		return dev->do_ioctl_ext(dev, cmd, param_list);
	}
	else
	{
	    	return ERR_DEV_ERROR;
	}
}


/*****************************************************************************
* INT32 nim_io_control(struct nim_device *dev, INT32 cmd, UINT32 param)
*
* NIM do input output control
*
* Arguments:
*  Parameter1: struct nim_device *dev	: Device
*  Parameter2: INT32  cmd				: Command
*  Parameter3: INT32  param				: Parameter
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_io_control(struct nim_device *dev, INT32 cmd, UINT32 param)
{
	if (dev == NULL)
		return ERR_NO_DEV;
	if((cmd & NIM_TUNER_COMMAND) && dev->do_ioctl)
        return dev->do_ioctl(dev, cmd, param);
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

    if (dev->do_ioctl)
    {
		return dev->do_ioctl(dev, cmd, param);
    }
    else
    {
    	return ERR_DEV_ERROR;
    }
}

/*****************************************************************************
* INT32 nim_get_lock(struct nim_device *dev, UINT8 *lock)
*
* Get NIM lock status
*
* Arguments:
*  Parameter1: struct nim_device *dev	: Device
*  Parameter2: UINT8 *lock				: Lock status
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_get_lock(struct nim_device *dev, UINT8 *lock)
{
    if (dev == NULL)
    {
        if (lock)
            *lock = 1;
        return RET_SUCCESS;
    }

	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->get_lock)
	{
		return dev->get_lock(dev, lock);
	}

	return SUCCESS;
}

/*****************************************************************************
* INT32 nim_get_freq(struct nim_device *dev, UINT32 *freq)
*
* Get NIM frequence
*
* Arguments:
*  Parameter1: struct nim_device *dev	: Device
*  Parameter2: UINT32 *freq				: Frequence
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_get_freq(struct nim_device *dev, UINT32 *freq)
{
    if (dev == NULL)
    {
        if (freq)
            *freq = 5150 - 4000;
        return RET_SUCCESS;
    }

	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->get_freq)
	{
		return dev->get_freq(dev, freq);
	}

	return SUCCESS;
}

/*****************************************************************************
* INT32 nim_get_FEC(struct nim_device *dev, UINT8 *fec)
*
* Get NIM code rate
*
* Arguments:
*  Parameter1: struct nim_device *dev	: Device
*  Parameter2: UINT8 *fec				: Code rate
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_get_FEC(struct nim_device *dev, UINT8 *fec)
{
    if (dev == NULL)
    {
        if (fec)
            *fec = 100;
        return RET_SUCCESS;
    }

	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->get_FEC)
	{
		return dev->get_FEC(dev, fec);
	}

	return SUCCESS;
}

/*****************************************************************************
* INT32 nim_get_SNR(struct nim_device *dev, UINT8 *snr)
*
* Get NIM Signal Noise Rate
*
* Arguments:
*  Parameter1: struct nim_device *dev	: Device
*  Parameter2: UINT8 *snr				: SNR
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_get_SNR(struct nim_device *dev, UINT8 *snr)
{
    if (dev == NULL)
    {
        if (snr)
            *snr = 77;
        return RET_SUCCESS;
    }

	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->get_SNR)
	{
		return dev->get_SNR(dev, snr);
	}

	return SUCCESS;
}

#if ( SYS_PROJECT_FE == PROJECT_FE_DVBS||SYS_PROJECT_FE == PROJECT_FE_DVBS2 )

/*****************************************************************************
* INT32 nim_get_AGC(struct nim_device *dev, UINT8 *agc)
*
* NIM DiSEqC operation
*
* Arguments:
*  Parameter1: struct nim_device *dev	: Device
*  Parameter2: UINT8 *agc				: AGC
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_get_AGC(struct nim_device *dev, UINT8 *agc)
{
    if (dev == NULL)
    {
        if (agc)
            *agc = 94;
        return RET_SUCCESS;
    }

	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->get_AGC)
	{
		return dev->get_AGC(dev, agc);
	}

	return SUCCESS;
}

/*****************************************************************************
* INT32 nim_DiSEqC_operate(struct nim_device *dev, UINT32 mode, UINT8* cmd, UINT8 cnt)
*
* NIM DiSEqC operation
*
* Arguments:
*  Parameter1: struct nim_device *dev	: Device
*  Parameter2: UINT32 mode				: DeSEqC mode
*  Parameter3: UINT8* cmd				: Command
*  Parameter4: UINT8  cnt				: Command length byte
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_DiSEqC_operate(struct nim_device *dev, UINT32 mode, UINT8* cmd, UINT8 cnt)
{
    if (dev == NULL)
    {
         return RET_SUCCESS;
    }

	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->DiSEqC_operate)
	{
		return dev->DiSEqC_operate(dev, mode, cmd, cnt);
	}

	return SUCCESS;
}

INT32 nim_DiSEqC2X_operate(struct nim_device *dev, UINT32 mode, UINT8* cmd, UINT8 cnt,
									UINT8 *rt_value, UINT8 *rt_cnt)
{
    if (dev == NULL)
    {
         return RET_SUCCESS;
    }

	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->DiSEqC2X_operate)
	{
		return dev->DiSEqC2X_operate(dev, mode, cmd, cnt, rt_value, rt_cnt);
	}

	return SUCCESS;
}
/*****************************************************************************
* INT32 nim_set_polar(struct nim_device *dev, UINT8 polar);
*
* NIM polarisation set up
*
* Arguments:
*  Parameter1: struct nim_device *dev	: Device
*  Parameter2: INT8 polar				: Polarisation
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_set_polar(struct nim_device *dev, UINT8 polar)
{
    if (dev == NULL)
    {
         return RET_SUCCESS;
    }

	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->set_polar)
	{
		return dev->set_polar(dev, polar);
	}

	return SUCCESS;
}

/*****************************************************************************
* INT32 nim_set_12v(struct nim_device *dev, UINT8 flag);
*
* LNB 12V supply
*
* Arguments:
*  Parameter1: struct nim_device *dev	: Device
*  Parameter2: INT8 flag				: 12V enable or not
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_set_12v(struct nim_device *dev, UINT8 flag)
{
    if (dev == NULL)
    {
         return RET_SUCCESS;
    }

	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->set_12v)
	{
		return dev->set_12v(dev, flag);
	}

	return SUCCESS;
}
/*****************************************************************************
* INT32 nim_channel_change(struct nim_device *dev, UINT32 freq, UINT32 sym, UINT8 fec);
*
* NIM channel change operation
*
* Arguments:
*  Parameter1: struct nim_device *dev	: Device
*  Parameter2: UINT32 freq				: Frequence in MHz
*  Parameter3: UINT32 sym				: Symbol rate in KHz
*  Parameter4: UINT8  fec				: Code rate
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_channel_change(struct nim_device *dev, UINT32 freq, UINT32 sym, UINT8 fec)
{
	struct NIM_Channel_Change dvb_s_CC_Param;

    if (dev == NULL)
    {
         return RET_SUCCESS;
    }

	dvb_s_CC_Param.freq = freq;
	dvb_s_CC_Param.sym = sym;
	dvb_s_CC_Param.fec = fec;
	
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->do_ioctl_ext)
	{
		return dev->do_ioctl_ext(dev, NIM_DRIVER_CHANNEL_CHANGE, (void *)(&dvb_s_CC_Param)); //channel_change(dev, (void *)(&dvb_s_CC_Param));
	}

	return SUCCESS;
}

/*****************************************************************************
* INT32 nim_channel_search(struct nim_device *dev, UINT32 freq);
*
* NIM channel search operation
*
* Arguments:
*  Parameter1: struct nim_device *dev	: Device
*  Parameter3: UINT8  freq				: Frequence in MHz
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_channel_search(struct nim_device *dev, UINT32 freq)
{
    if (dev == NULL)
    {
         return RET_SUCCESS;
    }

	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->channel_search)
	{
		return dev->channel_search(dev, freq);
	}

	return SUCCESS;
}

/*****************************************************************************
* INT32 nim_get_sym(struct nim_device *dev, UINT32 *sym)
*
* Get NIM symbol rate
*
* Arguments:
*  Parameter1: struct nim_device *dev	: Device
*  Parameter2: UINT32 *sym				: Symbol rate
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_get_sym(struct nim_device *dev, UINT32 *sym)
{
    if (dev == NULL)
    {
        if (sym)
            *sym = 30000;
         return RET_SUCCESS;
    }

	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->get_sym)
	{
		return dev->get_sym(dev, sym);
	}

	return SUCCESS;
}

/*****************************************************************************
* INT32 nim_get_BER(struct nim_device *dev, UINT32 *ber)
*
* Get NIM Bit Error Rate
*
* Arguments:
*  Parameter1: struct nim_device *dev	: Device
*  Parameter2: UINT32 *ber				: BER
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_get_BER(struct nim_device *dev, UINT32 *ber)
{
    if (dev == NULL)
    {
        if (ber)
            *ber = 0;
         return RET_SUCCESS;
    }

	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->get_BER)
	{
		return dev->get_BER(dev, ber);
	}

	return SUCCESS;
}

INT32 nim_get_fft_result(struct nim_device *dev, UINT32 freq, UINT32* start_adr)
{
    if (dev == NULL)
    {
         return RET_SUCCESS;
    }

	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->get_fft_result)
	{
		return dev->get_fft_result(dev, freq, start_adr);
	}
	else
		return ERR_FAILUE;
}

#elif (SYS_PROJECT_FE == PROJECT_FE_DVBT)||(SYS_PROJECT_FE == PROJECT_FE_ATSC) || (SYS_PROJECT_FE == PROJECT_FE_ISDBT)
#ifdef SMART_ANT_SUPPORT
INT32 nim_Set_Smartenna(struct nim_device *dev, UINT8 position,UINT8 gain,UINT8 pol,UINT8 channel)
{

	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->Set_Smartenna)
	{
		return dev->Set_Smartenna(dev, position,gain, pol, channel);
	}

	return SUCCESS;
}

INT32 nim_Get_SmartennaSetting(struct nim_device *dev,UINT8 *pPosition,UINT8 *pGain,UINT8 *pPol,UINT8 *pChannel)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->Get_SmartennaSetting)
	{
		return dev->Get_SmartennaSetting(dev, pPosition,pGain,pPol,pChannel);
	}

	return SUCCESS;
}


INT32 nim_Get_SmartennaMetric(struct nim_device *dev, UINT8 metric,UINT16 *pMetric)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->Get_SmartennaMetric)
	{
		return dev->Get_SmartennaMetric(dev,metric,pMetric);
	}

	return SUCCESS;
}

/*****************************************************************************
* INT32 nim_get_AGC(struct nim_device *dev, UINT8 *agc)
*
* NIM DiSEqC operation
*
* Arguments:
*  Parameter1: struct nim_device *dev	: Device
*  Parameter2: UINT8 *agc				: AGC
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_get_VSB_AGC(struct nim_device *dev, UINT16 *agc)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->get_VSB_AGC)
	{
		return dev->get_VSB_AGC(dev, agc);
	}

	return SUCCESS;
}


INT32 nim_get_VSB_SNR(struct nim_device *dev, UINT16 *snr)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->get_VSB_SNR)
	{
		return dev->get_VSB_SNR(dev, snr);
	}

	return SUCCESS;
}

INT32 nim_get_VSB_PER(struct nim_device *dev, UINT32 *per)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->get_VSB_PER)
	{
		return dev->get_VSB_PER(dev, per);
	}

	return SUCCESS;
}

#endif

/*****************************************************************************
* INT32 nim_get_AGC(struct nim_device *dev, UINT8 *agc)
*
* NIM DiSEqC operation
*
* Arguments:
*  Parameter1: struct nim_device *dev	: Device
*  Parameter2: UINT16 *agc				: AGC
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_get_AGC(struct nim_device *dev, UINT16 *agc)
{
	if (dev == NULL)
	{
		if (agc)
			*agc = 94;
		return RET_SUCCESS;
	}

	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->get_AGC)
	{
		return dev->get_AGC(dev, agc);
	}

	return SUCCESS;
}

INT32 nim_disable(struct nim_device *dev)
{
	UINT32 result;

	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	/* Stop device */
	if (dev->disable)
	{
		//soc_printf("nim_Disable!!,enter hld disable\n");
		result = dev->disable(dev);
		//soc_printf("nim_disable result=0x%x\n",result);
		
	}

	
	return SUCCESS;
}

INT32 nim_channel_change(struct nim_device *dev, UINT32 freq, UINT32 bandwidth,UINT8 guard_interval, UINT8 fft_mode, UINT8 modulation, UINT8 fec, UINT8 usage_type, UINT8 inverse, UINT8 priority)
{
	/* If device not running, exit */
	
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->channel_change)
	{
		return dev->channel_change(dev, freq, bandwidth, guard_interval, fft_mode, modulation, fec, usage_type, inverse, priority);
	}

	return SUCCESS;	
}


INT32 nim_channel_search(struct nim_device *dev, UINT32 freq, UINT32 bandwidth, UINT8 guard_interval, UINT8 fft_mode, UINT8 modulation, UINT8 fec, UINT8 usage_type, UINT8 inverse,UINT16 freq_offset, UINT8 priority)
{
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->channel_search)
	{
		return dev->channel_search(dev,freq, bandwidth, guard_interval, fft_mode, modulation, fec, usage_type, inverse, freq_offset, priority);
	}

	return SUCCESS;	
}


INT32 nim_get_gi(struct nim_device *dev, UINT8 *guard_interval)
{
	/* If device not running, exit */

	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->get_guard_interval)
	{
		dev->get_guard_interval(dev, guard_interval);
	}
	return SUCCESS;
}


INT32 nim_get_fftmode(struct nim_device *dev, UINT8 *fft_mode)
{
	/* If device not running, exit */
	
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->get_fftmode)
	{
		dev->get_fftmode(dev, fft_mode);
	}
	
	return SUCCESS;
}


INT32 nim_get_modulation(struct nim_device *dev, UINT8 *modulation)
{
	/* If device not running, exit */
	
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->get_modulation)
	{
		dev->get_modulation(dev, modulation);
	}
	
	return SUCCESS;
}

INT32	nim_get_spec_inv(struct nim_device *dev, UINT8 *inv)
{
	/* If device not running, exit */

	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->get_spectrum_inv)
	{
		dev->get_spectrum_inv(dev, inv);
	}
	return SUCCESS;
}

#if (SYS_SDRAM_SIZE == 8)
INT32 nim_get_HIER_mode(struct nim_device *dev, UINT8*hier)
{
	/* If device not running, exit */
	
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->get_HIER)
	{
		dev->get_HIER(dev, hier);
		
	//soc_printf("hd hier %d\n",*hier);
	}
	
	return SUCCESS;
}

INT8 nim_get_priority(struct nim_device *dev, UINT8*priority)
{
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->get_priority)
	{
		dev->get_priority(dev, priority);
	}
	
	return SUCCESS;
}
#endif

#if (SYS_SDRAM_SIZE == 8 || GET_BER == SYS_FUNC_ON)
INT32 nim_get_BER(struct nim_device *dev, UINT32 *ber)
{
	if (dev == NULL)
	{
		if (ber)
			*ber = 0;
		return RET_SUCCESS;
	}
	
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->get_BER)
	{
		return dev->get_BER(dev, ber);
	}

	return SUCCESS;
}
#endif

#if ((SYS_SDRAM_SIZE == 2 || SYS_SDRAM_SIZE == 8) \
	||(SYS_PROJECT_FE==PROJECT_FE_ATSC) || (SYS_PROJECT_FE==PROJECT_FE_DVBT) || (SYS_PROJECT_FE == PROJECT_FE_ISDBT))
INT32 nim_get_freq_offset(struct nim_device *dev, INT32 *freq_offset)//051222 yuchun
{
	/* If device not running, exit */
	
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->get_freq_offset)
	{
		dev->get_freq_offset(dev, freq_offset);
	}
	
	return SUCCESS;
}
#endif

#elif ( SYS_PROJECT_FE == PROJECT_FE_DVBC )
INT32 nim_channel_change(struct nim_device *dev, UINT32 freq, UINT32 sym, UINT8 fec)
{
	struct NIM_Channel_Change dvb_c_CC_Param;

	dvb_c_CC_Param.freq = freq;
	dvb_c_CC_Param.sym = sym;
	dvb_c_CC_Param.modulation = fec;
	
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->do_ioctl_ext)
	{
		return dev->do_ioctl_ext(dev, NIM_DRIVER_CHANNEL_CHANGE, (void *)(&dvb_c_CC_Param)); //channel_change(dev, (void *)(&dvb_c_CC_Param));
	}

	return SUCCESS;
}
INT32 nim_quick_channel_change(struct nim_device *dev, UINT32 freq, UINT32 sym, UINT8 fec)
{
	struct NIM_Channel_Change dvb_c_CC_Param;

	dvb_c_CC_Param.freq = freq;
	dvb_c_CC_Param.sym = sym;
	dvb_c_CC_Param.modulation = fec;
	
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->do_ioctl_ext)
	{
		return dev->do_ioctl_ext(dev, NIM_DRIVER_QUICK_CHANNEL_CHANGE, (void *)(&dvb_c_CC_Param)); //channel_change(dev, (void *)(&dvb_c_CC_Param));
	}

	return SUCCESS;
}
INT32 nim_channel_search(struct nim_device *dev, UINT32 freq)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->channel_search)
	{
		return dev->channel_search(dev, freq);
	}

	return SUCCESS;
}
INT32 nim_get_AGC(struct nim_device *dev, UINT8 *agc)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->get_AGC)
	{
		return dev->get_AGC(dev, agc);
	}

	return SUCCESS;
}
INT32 nim_get_sym(struct nim_device *dev, UINT32 *sym)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->get_sym)
	{
		return dev->get_sym(dev, sym);
	}

	return SUCCESS;
}
INT32 nim_get_BER(struct nim_device *dev, UINT32 *ber)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->get_BER)
	{
		return dev->get_BER(dev, ber);
	}

	return SUCCESS;
}
INT32 nim_get_fft_result(struct nim_device *dev, UINT32 freq, UINT32* start_adr)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->get_fft_result)
	{
		return dev->get_fft_result(dev, freq, start_adr);
	}
	else
		return ERR_FAILUE;
}
#endif
