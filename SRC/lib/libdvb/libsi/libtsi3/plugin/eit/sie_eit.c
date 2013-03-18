#include <types.h>
#include <sys_config.h>
#include <osal/osal.h>
#include <api/libc/alloc.h>
#include <api/libc/string.h>

#include <api/libtsi/si_types.h>
#include <api/libtsi/sie.h>
#include <api/libsi/si_eit.h>
#include <api/libsi/si_module.h>
#include <api/libsi/lib_epg.h>

#define SIE_EIT_LEVEL				0
#if (SIE_DEBUG_LEVEL>1)
#include <api/libc/printf.h>
#define SIE_EIT_PRINTF				libc_printf
#elif (SIE_DEBUG_LEVEL==1)
#include <api/libc/printf.h>
#define SIE_EIT_PRINTF				PRINTF
#else
#define SIE_EIT_PRINTF(...)			do{}while(0)
#endif



static BOOL  eit_in_parsing = FALSE;
BOOL  eit_in_parsing_check = FALSE;
static struct dmx_device *demux=NULL;

INT32 start_eit(struct sie_eit_config *config, si_dmx_event_t event, si_handler_t handle)
{
	INT32 ret;
	struct si_filter_t *filter = NULL;
	struct si_filter_param fparam;
	UINT8 mask[16] = {0xFF, 0xFE, 0xFC, 0xFC, 0xF8, 0xF8, 0xF8, 0xF8,
		0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0};
#ifdef SI_SUPPORT_MUTI_DMX
	UINT8 dmx_id=0;
	dmx_id=lib_nimng_get_nim_play()-1;
	//SIE_EIT_PRINTF("%s(): dmx_id=%d\n", __FUNCTION__,dmx_id);
	struct dmx_device *dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, dmx_id);
#endif

		
	if((config->buffer==NULL)||(config->buf_len<PSI_LONG_SECTION_LENGTH))
	{
		SIE_EIT_PRINTF("%s(): buffer error!", __FUNCTION__);
		return !SUCCESS;
	}
	if (eit_in_parsing)
       	return SUCCESS;

	MEMSET(&fparam,0,sizeof(struct si_filter_param));
	if(config->eit_mode == SIE_EIT_SINGLE_SERVICE)
	{
		fparam.mask_value.value[1][3]=fparam.mask_value.value[0][3]=(config->cur_service_id)>>8;
		fparam.mask_value.value[1][4]=fparam.mask_value.value[0][4]=(config->cur_service_id)&0xFF;
		fparam.mask_value.multi_mask[0][3] = 0xFF;
		fparam.mask_value.multi_mask[0][4] = 0xFF;
		fparam.mask_value.multi_mask[1][3] = 0xFF;
		fparam.mask_value.multi_mask[1][4] = 0xFF;
	}
	else if(config->eit_mode == SIE_EIT_WHOLE_TP)
	{


	}
		
	
	//ret = sie_open(dev, PSI_MODULE_CONCURRENT_FILTER, NULL, 0);
	#ifdef SI_SUPPORT_MUTI_DMX
	filter = sie_alloc_filter_ext(dmx,PSI_EIT_PID, config->buffer, config->buf_len, PSI_LONG_SECTION_LENGTH,0);
	#else
	filter = sie_alloc_filter(PSI_EIT_PID, config->buffer, config->buf_len, PSI_LONG_SECTION_LENGTH);
	#endif
	if(NULL == filter)
	{
		SIE_EIT_PRINTF("%s(): alloc filter failed!\n", __FUNCTION__);
		return !SUCCESS;
	}
	fparam.timeout = OSAL_WAIT_FOREVER_TIME;
	fparam.attr[0] = fparam.attr[1] = SI_ATTR_HAVE_CRC32;
	fparam.mask_value.tb_flt_msk = 0x3;
#ifdef EPG_OTH_SCH_SUPPORT
	fparam.attr[2] = SI_ATTR_HAVE_CRC32;
	fparam.mask_value.tb_flt_msk = 0x7;
#endif
	fparam.mask_value.mask_len = 6;
#if((defined(SUPPORT_FRANCE_HD) || defined(EPG_OTH_SCH_SUPPORT)) && !defined(EPG_ONLY_OTH_SCH_SUPPORT))
	fparam.mask_value.multi_mask[0][0] = 0xFE;
#else
	fparam.mask_value.multi_mask[0][0] = 0xFF;
#endif
	fparam.mask_value.multi_mask[0][1] = 0x80;
	fparam.mask_value.multi_mask[0][5] = 0x01;
	fparam.mask_value.multi_mask[1][0] = mask[SCHEDULE_TABLE_ID_NUM-1];
	fparam.mask_value.multi_mask[1][1] = 0x80;
	fparam.mask_value.multi_mask[1][5] = 0x01;
	
	fparam.mask_value.value[0][0] = 0x4E;
	fparam.mask_value.value[0][1] = 0x80;
	fparam.mask_value.value[0][5] = 0x01;
	
	fparam.mask_value.value[1][0] = 0x50;
	fparam.mask_value.value[1][1] = 0x80;
	fparam.mask_value.value[1][5] = 0x01;
	
#ifdef EPG_OTH_SCH_SUPPORT
	fparam.mask_value.multi_mask[2][0] = mask[SCHEDULE_TABLE_ID_NUM-1];
	fparam.mask_value.multi_mask[2][1] = 0x80;
	fparam.mask_value.multi_mask[2][5] = 0x01;
	fparam.mask_value.value[2][0] = 0x60;
	fparam.mask_value.value[2][1] = 0x80;
	fparam.mask_value.value[2][5] = 0x01;
#endif

	fparam.section_event = event;
	fparam.section_parser = handle;
	sie_config_filter(filter, &fparam);
	ret = sie_enable_filter(filter);
	if(ret != SI_SUCCESS)
	{
		SIE_EIT_PRINTF("%s(): enable filter failed!\n", __FUNCTION__);
		return !SUCCESS;
	}
		
	eit_in_parsing=TRUE;
	eit_in_parsing_check=TRUE;
	return SUCCESS;
}

INT32 start_eit_ext(struct dmx_device *dmx, struct sie_eit_config *config, si_dmx_event_t event, si_handler_t handle,
				UINT8 mode)
{
	INT32 ret;
	struct si_filter_t *filter = NULL;
	struct si_filter_param fparam;
	UINT8 mask[16] = {0xFF, 0xFE, 0xFC, 0xFC, 0xF8, 0xF8, 0xF8, 0xF8,
		0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0};
		
	if((config->buffer==NULL)||(config->buf_len<PSI_LONG_SECTION_LENGTH))
	{
		SIE_EIT_PRINTF("%s(): buffer error!", __FUNCTION__);
		return !SUCCESS;
	}
	if (eit_in_parsing)
       	return SUCCESS;

	MEMSET(&fparam,0,sizeof(struct si_filter_param));
	if(config->eit_mode == SIE_EIT_SINGLE_SERVICE)
	{
		fparam.mask_value.value[1][3]=fparam.mask_value.value[0][3]=(config->cur_service_id)>>8;
		fparam.mask_value.value[1][4]=fparam.mask_value.value[0][4]=(config->cur_service_id)&0xFF;
		fparam.mask_value.multi_mask[0][3] = 0xFF;
		fparam.mask_value.multi_mask[0][4] = 0xFF;
		fparam.mask_value.multi_mask[1][3] = 0xFF;
		fparam.mask_value.multi_mask[1][4] = 0xFF;
	}
	else if(config->eit_mode == SIE_EIT_WHOLE_TP)
	{


	}
		
	
	//ret = sie_open(dev, PSI_MODULE_CONCURRENT_FILTER, NULL, 0);
	filter = sie_alloc_filter_ext(dmx, PSI_EIT_PID, config->buffer, config->buf_len, PSI_LONG_SECTION_LENGTH, mode);
	if(NULL == filter)
	{
		SIE_EIT_PRINTF("%s(): alloc filter failed!\n", __FUNCTION__);
		return !SUCCESS;
	}

	demux = dmx;
	
	fparam.timeout = OSAL_WAIT_FOREVER_TIME;
	fparam.attr[0] = fparam.attr[1] = SI_ATTR_HAVE_CRC32;
	fparam.mask_value.tb_flt_msk = 0x3;
#ifdef EPG_OTH_SCH_SUPPORT
	fparam.attr[2] = SI_ATTR_HAVE_CRC32;
	fparam.mask_value.tb_flt_msk = 0x7;
#endif
	fparam.mask_value.mask_len = 6;
#if(defined(EPG_OTH_SCH_SUPPORT))
	fparam.mask_value.multi_mask[0][0] = 0xFE;
#else
	fparam.mask_value.multi_mask[0][0] = 0xFF;
#endif
	fparam.mask_value.multi_mask[0][1] = 0x80;
	fparam.mask_value.multi_mask[0][5] = 0x01;
	fparam.mask_value.multi_mask[1][0] = mask[SCHEDULE_TABLE_ID_NUM-1];
	fparam.mask_value.multi_mask[1][1] = 0x80;
	fparam.mask_value.multi_mask[1][5] = 0x01;
	
	fparam.mask_value.value[0][0] = 0x4E;
	fparam.mask_value.value[0][1] = 0x80;
	fparam.mask_value.value[0][5] = 0x01;
	
	fparam.mask_value.value[1][0] = 0x50;
	fparam.mask_value.value[1][1] = 0x80;
	fparam.mask_value.value[1][5] = 0x01;
	
#ifdef EPG_OTH_SCH_SUPPORT
	fparam.mask_value.multi_mask[2][0] = mask[SCHEDULE_TABLE_ID_NUM-1];
	fparam.mask_value.multi_mask[2][1] = 0x80;
	fparam.mask_value.multi_mask[2][5] = 0x01;
	fparam.mask_value.value[2][0] = 0x60;
	fparam.mask_value.value[2][1] = 0x80;
	fparam.mask_value.value[2][5] = 0x01;
#endif

	fparam.section_event = event;
	fparam.section_parser = handle;
	sie_config_filter(filter, &fparam);
	ret = sie_enable_filter(filter);
	if(ret != SI_SUCCESS)
	{
		SIE_EIT_PRINTF("%s(): enable filter failed!\n", __FUNCTION__);
		return !SUCCESS;
	}
		
	eit_in_parsing=TRUE;
	eit_in_parsing_check=TRUE;
	return SUCCESS;
}

INT32 stop_eit()
{

#ifdef SI_SUPPORT_MUTI_DMX
	UINT8 dmx_id=0;
	dmx_id=lib_nimng_get_nim_play()-1;
	//SIE_EIT_PRINTF("%s(): dmx_id=%d\n", __FUNCTION__,dmx_id);
	struct dmx_device *dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, dmx_id);
#endif

	if (!eit_in_parsing)
		return SUCCESS;
	#ifdef SI_SUPPORT_MUTI_DMX
	if(SI_SUCCESS != sie_abort_ext(dmx,NULL, PSI_EIT_PID,NULL))
		return !SUCCESS;
	#else
   	if(SI_SUCCESS != sie_abort(PSI_EIT_PID,NULL))
		return !SUCCESS;
	#endif
	eit_in_parsing = FALSE;
	eit_in_parsing_check=FALSE;
	return SUCCESS;
}

INT32 stop_eit_ext()
{
	if (!eit_in_parsing)
		return SUCCESS;
	if(NULL==demux)
		return SUCCESS;
   	if(SI_SUCCESS != sie_abort_ext(demux,NULL, PSI_EIT_PID,NULL))
		return !SUCCESS;

	eit_in_parsing = FALSE;
	eit_in_parsing_check=FALSE;
	demux = NULL;
	return SUCCESS;
}


