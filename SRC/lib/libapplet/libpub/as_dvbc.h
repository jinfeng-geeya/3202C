/*
 * AS_DVBC.H
 *
 * autoscan module internal API for DVB-C application.
  */
#ifndef __AS_DVBC_H__
#define __AS_DVBC_H__

#include <types.h>
#include <sys_config.h>
#include <api/libpub/as_module.h>

enum
{
#ifdef DVBC_FREQ_EXTEND
	DVBC_FREQUENCY_UHF_HIGH		= 86300,
#else
	DVBC_FREQUENCY_UHF_HIGH		= 85900,
#endif
	DVBC_FREQUENCY_UHF_LOW		= 47400,
	DVBC_FREQUENCY_VHF_HIGH		= 45900,
	DVBC_FREQUENCY_VHF_LOW		= 5100,
	DVBC_FREQUENCY_BAND_WIDTH	= 800,
};

#ifdef FE_DVBC_SUPPORT

static __inline__ void dvbc_convert_freq(T_NODE *tp, UINT32 freq)
{
	tp->frq = freq;
}

static __inline__ void dvbc_tp_info2db(TP_INFO *info, T_NODE *tp)
{
	MEMSET(tp, 0, sizeof(T_NODE));
	tp->ft_type = FRONTEND_TYPE_C;
	tp->network_id = info->c_info.onid;
	tp->t_s_id = info->c_info.tsid;
	tp->frq = info->c_info.frequency;
	tp->sym = info->c_info.symbol_rate;
	tp->FEC_inner= info->c_info.modulation;
}

#ifdef __cplusplus
extern "C"{
#endif
BOOL dvbc_init_search_param(struct as_tuning_param *atp, struct as_service_param *param);
//BOOL dvbc_init_search_param(struct as_tuning_param *atp, UINT32 from, 
//	UINT32 to);

BOOL dvbc_alloc_transponder(struct as_module_info *info,
	struct as_tuning_param *atp, T_NODE *tp, UINT16 sat_id);

BOOL dvbc_tp_sanity_check(BOOL voltage_ctrl, T_NODE *tp, S_NODE *sat);
#ifdef __cplusplus
}
#endif


#else /* FE_DVBC_SUPPORT */

#define dvbc_convert_freq(...)   do{}while(0)
#define dvbc_tp_info2db(...)   do{}while(0)
#define dvbc_init_search_param(...)   (TRUE)
#define dvbc_alloc_transponder(...)   (TRUE)
#define dvbc_tp_sanity_check(...)   (TRUE)


#endif /* FE_DVBC_SUPPORT */

#endif /* __AS_DVBC_H__ */

