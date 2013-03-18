/*
 * AS_DVBC.H
 *
 * autoscan module internal API implementation for DVB-C application.
 *
 * revision
 * 1. 2006.07.07	Zhengdao Li	0.01		Initialization.
 */
#include <types.h>
#include <sys_config.h>
#include <api/libc/string.h>
#include <api/libtsi/db_3l.h>
#include <api/libpub/as_module.h>
#include "../as_dvbc.h"

#ifdef DVBC_COUNTRY_BAND_SUPPORT
#ifdef NIT_AUTO_SEARCH_SUPPORT
static UINT8 b_nit = 0;
void as_set_nit_search_enable( UINT8 sel)
{
	b_nit = sel;
}
BOOL as_get_nit_search_status()
{
	if(b_nit > 0)
		return TRUE;

	return FALSE;
}
#endif
BOOL dvbc_init_search_param(struct as_tuning_param *atp, struct as_service_param *param)
{
	if(param->scan_cnt<1 || param->scan_cnt>MAX_BAND_COUNT)
	{
		atp->finish = TRUE;
		return FALSE;
	}
    if(param->ft_count<1)
        return FALSE;
#ifdef NIT_AUTO_SEARCH_SUPPORT
	if(as_get_nit_search_status())//enable nit auto search
		atp->nit_enable = 1;
	else 
		atp->nit_enable = 0;
#endif	

	atp->ft_type = param->as_frontend_type;

	atp->start = param->band_group_param[0].start_freq;
	atp->limit = param->band_group_param[0].end_freq;
	atp->step = param->band_group_param[0].freq_step;
	atp->pos = atp->start;
	atp->band_count = param->scan_cnt;
	atp->cur_band=0;
	atp->band_group_param = param->band_group_param;

	atp->finish = FALSE;

     atp->ft_count= 0;
     atp->ft.c_param.sym = param->ft[0].c_param.sym;
     atp->ft.c_param.constellation = param->ft[0].c_param.constellation;
     atp->timeout = osal_get_tick()+ AS_TUNE_TIMEOUT_C;

	return TRUE;

}
#else
BOOL dvbc_init_search_param(struct as_tuning_param *atp, struct as_service_param *param)
//BOOL dvbc_init_search_param(struct as_tuning_param *atp, UINT32 from,
//	UINT32 to)
{
	UINT32 from,to;
	from = param->as_from;
	to = param->as_to;
	atp->limit = (to>DVBC_FREQUENCY_UHF_HIGH)? DVBC_FREQUENCY_UHF_HIGH: to;

    if(from < DVBC_FREQUENCY_VHF_LOW)
        atp->pos = DVBC_FREQUENCY_VHF_LOW;
    else if(from <=  DVBC_FREQUENCY_VHF_HIGH)
    {
        atp->pos = from -(from-DVBC_FREQUENCY_VHF_LOW)%DVBC_FREQUENCY_BAND_WIDTH;
        if((from-DVBC_FREQUENCY_VHF_LOW)%DVBC_FREQUENCY_BAND_WIDTH != 0)
            atp->pos += DVBC_FREQUENCY_BAND_WIDTH;
        if(atp->pos > DVBC_FREQUENCY_VHF_HIGH && atp->pos < DVBC_FREQUENCY_UHF_LOW)
            atp->pos = DVBC_FREQUENCY_UHF_LOW;
    }
    else if(from < DVBC_FREQUENCY_UHF_LOW)
    {
        atp->pos = DVBC_FREQUENCY_UHF_LOW;
    }
    else
    {
        atp->pos = from -(from-DVBC_FREQUENCY_UHF_LOW)%DVBC_FREQUENCY_BAND_WIDTH;
        if((from-DVBC_FREQUENCY_UHF_LOW)%DVBC_FREQUENCY_BAND_WIDTH != 0)
            atp->pos += DVBC_FREQUENCY_BAND_WIDTH;

    }

	atp->start = atp->pos;
	atp->step = 800;
	return (atp->pos>atp->limit)? FALSE: TRUE;
}
#endif

BOOL dvbc_tp_sanity_check(BOOL voltage_ctrl, T_NODE *tp, S_NODE *sat)
{
    if((tp->frq< AS_MIN_CAB_FREQUENCY)||(tp->frq>AS_MAX_CAB_FREQUENCY))
        return FALSE;
    else
        return TRUE;
}


BOOL dvbc_alloc_transponder(struct as_module_info *info,
	struct as_tuning_param *atp, T_NODE *tp, UINT16 sat_id)
{
	INT32 ret;
	UINT32 freq;
	INT32 search_ret;

	MEMSET(tp, 0, sizeof(T_NODE));
	tp->sat_id = sat_id;
	tp->ft_type = FRONTEND_TYPE_C;
	nim_get_freq(info->nim, &freq);

    //If offset is less than 0.4MHz,delete the offset,or add up to 1MHz.
    //So we just show integer(MHz).
    /*
    if(freq%100<=40)
        tp->frq = freq/100*100;
    else
        tp->frq = freq/100*100+100;
	//tp->frq = freq;
	*/

#ifdef DVBC_COUNTRY_BAND_SUPPORT
    tp->frq = freq;
	tp->sym = atp->ft.c_param.sym;
	tp->FEC_inner = atp->ft.c_param.constellation;
#else
    if(freq%100<=40)
        tp->frq = freq/100*100;
    else
        tp->frq = freq/100*100+100;
    tp->sym=info->param.ft.c_param.sym;
    tp->FEC_inner=info->param.ft.c_param.constellation;
#endif
	search_ret =db_search_lookup_node(TYPE_SEARCH_TP_NODE, tp);
	if ( search_ret != SUCCESS)
     {
		ret = add_node(TYPE_SEARCH_TP_NODE, tp->sat_id, tp);
		if (ret!=SUCCESS)
			return FALSE;
	}
	atp->tp_id = tp->tp_id;
	return TRUE;
}



