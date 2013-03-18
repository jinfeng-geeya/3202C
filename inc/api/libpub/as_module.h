/*
 * AS_MODULE.H
 *
 * autoscan module internal structure definition file.
 *
 * revision history
 * 1. 2006.07.08	Zhengdao Li	0.01	Initialization.
 */
#ifndef __AS_MODULE_H__
#define __AS_MODULE_H__
#include <types.h>
#include <sys_config.h>

#include <hld/hld_dev.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/nim/nim_dev.h>
#include <hld/nim/nim.h>
#include <api/libtsi/db_3l.h>

#include <api/libpub/lib_frontend.h>
#include <api/libpub/lib_as.h>

enum
{
	AS_MODULE_MAX_P_NUM		= 256,
	AS_MODULE_MAX_T_NUM		= 1024,
	AS_MODULE_MAX_S_NUM		= 1,
	AS_MODULE_SLEEP_SLICE	= 50,
#ifdef TFN_SCAN
	AS_TUNE_TIMEOUT_C			= 4000,
#else
	AS_TUNE_TIMEOUT_C			= 1000,
#endif
	AS_MIN_CAB_FREQUENCY		= 4800,
	AS_MAX_CAB_FREQUENCY	= 86200,
};

enum
{
	ATP_TYPE_DBASE,		/* database based */
	ATP_TYPE_STEP,			/* step based*/
};

struct as_tuning_param
{
	UINT32 start;
	UINT32 limit;
	UINT32 step;
	UINT32 pos;
	UINT32 tp_id;
	UINT32 nit_enable;
    UINT32 si_enable;
	UINT32 timeout;
	BOOL finish;
 	UINT8 ft_type;//front end type in lib_frontend.h       

#if ((SYS_PROJECT_FE == PROJECT_FE_DVBC) && defined(DVBC_COUNTRY_BAND_SUPPORT))
    union as_frontend_param ft;
    UINT8 ft_count;
#endif

#if (SYS_PROJECT_FE == PROJECT_FE_DVBT) || (SYS_PROJECT_FE == PROJECT_FE_ISDBT) \
	|| ((SYS_PROJECT_FE == PROJECT_FE_DVBC) && defined(DVBC_COUNTRY_BAND_SUPPORT))
 	UINT32 band_count;
 	UINT32 cur_band;
	Band_param	*band_group_param;//[MAX_BAND_COUNT];	/*pointer array to store the parameters of a group of continuous bands.*/
#endif
};


struct as_module_info
{
	UINT32 stop_scan_signal;	/* stop signal */
	struct dmx_device *dmx;
	struct nim_device *nim;
	S_NODE cur_sat;
	T_NODE cur_tp;
	T_NODE xp;
	struct as_service_param param;

	//DVB-S nim operation related section.
	BOOL voltage_ctrl;
	UINT32 polar;
	UINT32 dual_LNB;
	struct ft_antenna antenna;
	union ft_xpond xponder;
	struct NIM_Auto_Scan nim_para;
	
	struct as_tuning_param atp;
	INT32 tp_nr;
	INT32 sat_nr;
		
	UINT32 tp_id[AS_MODULE_MAX_T_NUM];
	UINT8 tp_nit[AS_MODULE_MAX_T_NUM];
    UINT8 tp_si[AS_MODULE_MAX_T_NUM];
	UINT16 sat_id[AS_MODULE_MAX_S_NUM];
};

#endif /* __AS_MODULE_H__ */

