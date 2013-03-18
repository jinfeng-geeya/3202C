#include <hld/nim/nim_dev.h>
#include <hld/decv/decv.h>
#include <hld/deca/deca_dev.h>
#include <hld/snd/snd_dev.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <api/libc/string.h>
#include <bus/tsi/tsi.h>
#include <api/libsi/si_tdt.h>
#include <api/libpub/lib_frontend.h>
#include <api/libpub/lib_as.h>
#include <api/libpub/lib_device_manage.h>


/*******************************************************
* macro define
********************************************************/
#define CC_MAX_TS_ROUTE_NUM 5


/*******************************************************
* extern declare
********************************************************/
UINT16 l_cc_sema_id = OSAL_INVALID_ID;
#define ENTER_FRONTEND_API()	do	\
								{	\
									if (l_cc_sema_id != OSAL_INVALID_ID)	\
										osal_semaphore_capture(l_cc_sema_id, TMO_FEVR);	\
								}while(0)
#define LEAVE_FRONTEND_API()	do	\
								{	\
									if (l_cc_sema_id != OSAL_INVALID_ID)	\
										osal_semaphore_release(l_cc_sema_id);	\
								}while(0)
								

/*******************************************************
* struct and variable
********************************************************/

//extern struct dmx_device *g_dmx_dev, *g_dmx_dev2, *g_dmx_dev3;
INT8 nim_tsiid[3] = {TSI_SPI_0, TSI_SPI_1, TSI_SPI_TSG};
static struct ts_route_info g_ts_route_info[CC_MAX_TS_ROUTE_NUM];
typedef void (*v12set_callback)(UINT32 v12);
static v12set_callback l_v12set_callback = NULL;
/********************************************************
*internal functions
/*******************************************************/
void UIChChgSet12VSetCB(v12set_callback v12setcb)
{
	l_v12set_callback = v12setcb;
}

// copy from "ci_is_av_scrambled" as a common API
BOOL ts_route_is_av_scrambled(struct dmx_device *dmx, UINT16 v_pid, UINT16 a_pid)
{
	BOOL ret = FALSE;
	RET_CODE dmx_state = RET_FAILURE;
	struct io_param param;
	struct io_param_ex param_ex;
	UINT16 PID[2];
	UINT8 scramble_type=0;
		
	if (dmx != NULL)
	{
		PID[0] = v_pid;
		PID[1] = a_pid;
		if (ALI_M3329E == sys_ic_get_chip_id())
		{
			MEMSET(&param, 0, sizeof(param));
			param.io_buff_in = (UINT8 *)PID;
			param.io_buff_out= (UINT8 *)&scramble_type;
			dmx_state = dmx_io_control(dmx, IS_AV_SCRAMBLED_EXT, (UINT32)&param);			
		}
		else if(ALI_S3602F == sys_ic_get_chip_id())
		{
			MEMSET(&param_ex, 0, sizeof(param_ex));
			param_ex.io_buff_in = (UINT8 *)PID;
			param_ex.io_buff_out= (UINT8 *)&scramble_type;
			dmx_state = dmx_io_control(dmx, IS_AV_SCRAMBLED_EXT, (UINT32)&param_ex);			
		}
		else
		{
			//for 3602 do not support IS_AV_SCRAMBLED_EXT
			dmx_state = dmx_io_control(dmx, IS_AV_SCRAMBLED, (UINT32)&scramble_type);
		}
		
		//fix issue: for AC3 audio is AUD_STR_INVALID
//		if((dmx_state == SUCCESS ) && ((scramble_type & (VDE_TS_SCRBL|VDE_PES_SCRBL|AUD_TS_SCRBL|AUD_PES_SCRBL|VDE_STR_INVALID|AUD_STR_INVALID)) != 0))
		if((dmx_state == SUCCESS ) && ((scramble_type & (VDE_TS_SCRBL|VDE_PES_SCRBL|AUD_TS_SCRBL|AUD_PES_SCRBL)) != 0))
		{
			ret = TRUE;
		}
	}

	return ret;
}

/* Get Number of Valid TS Route */
static UINT8 __ts_route_get_valid(void)
{
	UINT16 i;

	for(i = 0; i < CC_MAX_TS_ROUTE_NUM; i++)
	{
		if(g_ts_route_info[i].enable == 0)
		{
			break;
		}
	}

	return i;
}

/* Get Lock Flag and Scramble Flag of TS Route */
static void __ts_route_check_flag(struct ts_route_info *ts_route)
{
	struct dmx_device *dmx = NULL;
	struct nim_device *nim = NULL;
	UINT32 scrabled, scrable_typ;
	P_NODE node;

	//may have problem!
	
	if((ts_route->state & TS_ROUTE_STATUS_PLAYACK) == 0)
	{
		dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, ts_route->dmx_id);
		nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, ts_route->nim_id);

		//Check whether NIM Signal is Locked or not
		nim_get_lock(nim, &ts_route->flag_lock);


		//Check whether TS Stream in DMX is Scrambled or not and Scamble Type
		scrabled = 0;
		if(dmx_io_control(dmx, IS_AV_SCRAMBLED, (UINT32)(&scrable_typ)) == RET_SUCCESS)
		{
			ts_route->flag_scramble = (scrable_typ & (VDE_TS_SCRBL|VDE_PES_SCRBL));
		}

	}

	return;
}


/*******************************************************
* external APIs
********************************************************/

/* Set NIM Info */
void frontend_set_nim(struct nim_device *nim, struct ft_antenna* antenna, 
    union ft_xpond* xpond, UINT8 force_set)
{
    	struct NIM_Channel_Change nim_param;
    	INT32 tuning_frq = 0;
    	struct nim_config config;

	//Get Old NIM Config Info
    	dev_get_nim_config(nim, xpond->common.type, &config);
    
    	MEMSET(&nim_param, 0, sizeof(nim_param));

		/* Tuning DVB-C NIM */
        if(xpond->common.type == FRONTEND_TYPE_C)
        {
        	tuning_frq = xpond->c_info.frq;
        	nim_param.sym = xpond->c_info.sym;
        	nim_param.modulation = xpond->c_info.modulation;
        }
   
    	nim_param.freq = tuning_frq;

		/* Set NIM Using the Tuning Param */	
    	if(nim->do_ioctl_ext)
    	{
        	nim->do_ioctl_ext(nim, NIM_DRIVER_CHANNEL_CHANGE, (void *)(&nim_param));
    	}

		//Store New NIM Config Info
    	MEMCPY(&config.xpond, xpond, sizeof(union ft_xpond));
    	dev_set_nim_config(nim, xpond->common.type, &config);
}

/* Set Antenna and NIM Info */
void frontend_tuning(struct nim_device *nim, struct ft_antenna* antenna, 
    union ft_xpond* xpond, UINT8 force_set)
{
    	frontend_set_nim(nim, antenna, xpond, force_set);
}

/* Set TS Route Info */
void frontend_set_ts_route(struct nim_device *nim, struct ft_ts_route *route)
{
    	struct dmx_device *dmx = NULL;

    	if((route == NULL) || (route->ts_route_enable == 0))
    	{
        	return;
    	}

	//Set TS Input Source Attribute
    	//tsi_mode_set(route->tsiid, route->tsi_mode); // move to system_hw_init
	//Select TS Input Source for Output TS of TSI
    	tsi_select(route->ts_id, route->tsiid);
	//Select TSI Output TS for DMX
    	tsi_dmx_src_select(route->dmx_id, route->ts_id);    	
}

/* Check whether NIM Param is changed or not */
BOOL frontend_check_nim_param(struct nim_device *nim, union ft_xpond* xpond)
{
	BOOL ret = FALSE;
	INT32 freq = 0;
	UINT8 fec = 0;
	UINT32 sym = 0;
	UINT32 tmp;

	if(nim == NULL || xpond == NULL)
	{
		return ret;
	}

	if(xpond->common.type == FRONTEND_TYPE_C)
	{
		//Get NIM Freq		
		if(SUCCESS != nim_get_freq(nim, &freq))
		{
			return ret;
		}
		
		//Freq Offset within 1MHz, Same TP
		if((UINT32)freq >= xpond->c_info.frq)
			tmp = freq - xpond->c_info.frq;
		else
			tmp = xpond->c_info.frq - freq;
		if(tmp > 100)
		{
			ret = TRUE;
		}
		//Check modulation
		else 
		{
			if(SUCCESS != nim_get_FEC(nim, &fec))
			{
				return ret;
			}
			if(fec != xpond->c_info.modulation)
			{
				ret = TRUE;
			}
		}
	}
	
	return ret;
}

/* Get Status of NIM or DMX Device in TS Route */
void _ts_route_get_dev_state(UINT32 hld_type, void *dev, UINT32 *status)
{
	UINT16 i;
	UINT8 dev_id;
	
	switch(hld_type)
	{
	//Get NIM Device Status
	case HLD_DEV_TYPE_NIM:
		dev_id = (((struct nim_device *)dev)->type) & HLD_DEV_ID_MASK;
		*status = DEV_FRONTEND_STATUS_PLAY;
		for(i=0; i<CC_MAX_TS_ROUTE_NUM; i++)
		{
			if((g_ts_route_info[i].enable == 1) && (g_ts_route_info[i].nim_id == dev_id))
			{
				if(g_ts_route_info[i].state & TS_ROUTE_STATUS_RECORD)
				{
					*status = DEV_FRONTEND_STATUS_RECORD;
					break;
				}
			}
		}
		break;
	//Get DMX Device Status
	case HLD_DEV_TYPE_DMX:
		dev_id = (((struct dmx_device *)dev)->type) & HLD_DEV_ID_MASK;
		*status = DEV_DMX_STATUS_PLAY;
		for(i=0; i<CC_MAX_TS_ROUTE_NUM; i++)
		{
			if((g_ts_route_info[i].enable == 1) && (g_ts_route_info[i].dmx_id == dev_id))
			{
				if(g_ts_route_info[i].state & TS_ROUTE_STATUS_RECORD)
				{
					*status = DEV_DMX_STATUS_RECORD;
					break;
				}
			}
		}
		break;
	default:
		break;
	}

	return;
}

/* Get DMX Config Info in TS Route */
void _ts_route_get_dmx_config(struct dmx_device *dmx, struct dmx_config *config)
{
	UINT16 i, j = 0;
	UINT8 dev_id;
	
	MEMSET(config, 0, sizeof(struct dmx_config));
	dev_id = (dmx->type) & HLD_DEV_ID_MASK;

	for(i=0; i<CC_MAX_TS_ROUTE_NUM; i++)
	{
		if((g_ts_route_info[i].enable == 1) && (g_ts_route_info[i].dmx_id == dev_id))
		{
			config->tp_id = g_ts_route_info[i].tp_id;
			if(g_ts_route_info[i].sim_num)
			{
				MEMCPY(&config->dmx_sim_info[j], &g_ts_route_info[i].dmx_sim_info[0], g_ts_route_info[i].sim_num * sizeof(struct dmx_sim));
				j += g_ts_route_info[i].sim_num;
			}
		}
	}
	
	return;
}

/* Create TS Route */
UINT32 ts_route_create(struct ts_route_info *ts_route)
{
	UINT32 ret = 0;

	//Get TS Route Number for Check whether TS Route is Valid or not 
	ret = __ts_route_get_valid();
	if(ret >= CC_MAX_TS_ROUTE_NUM)
	{
		return ret;
	}

	//Copy TS Route Info
	MEMSET(&g_ts_route_info[ret], 0, sizeof(struct ts_route_info));
	MEMCPY(&g_ts_route_info[ret], ts_route, sizeof(struct ts_route_info));
	g_ts_route_info[ret].enable = 1;
	g_ts_route_info[ret].id = ret;
	g_ts_route_info[ret].flag_lock = 1;
	g_ts_route_info[ret].flag_scramble = 0;

	return ret;
}

/* Delete TS Route */
RET_CODE ts_route_delete(UINT32 id)
{
	RET_CODE ret = RET_SUCCESS;

	if(g_ts_route_info[id].enable == 0)
	{
		ret = RET_FAILURE;
	}
	else
	{
		MEMSET(&g_ts_route_info[id], 0, sizeof(struct ts_route_info));
	}
	
	return ret;
}

/* Get TS Route Info according to TS Route ID */
UINT8 ts_route_get_num(void)
{
	UINT8 ret = 0;
	UINT16 i;

	for(i=0; i<CC_MAX_TS_ROUTE_NUM; i++)
	{
		if(g_ts_route_info[i].enable == 1)
		{
			ret++;
		}
	}
	
	return ret;
}

/* Get TS Route Info according to TS Route ID */
RET_CODE ts_route_get_by_id(UINT32 id, struct ts_route_info *ts_route)
{
	RET_CODE ret = RET_SUCCESS;

	if(g_ts_route_info[id].enable == 0)
	{
		ret = RET_FAILURE;
	}
	else
	{
		MEMCPY(ts_route, &g_ts_route_info[id], sizeof(struct ts_route_info));
		__ts_route_check_flag(ts_route);
	}
	
	return ret;
}

/* Get TS Route ID and Info according to TS Route Type */
RET_CODE ts_route_get_by_type(UINT32 type, UINT32 *ts_route_id, struct ts_route_info *ts_route)
{
	RET_CODE ret = RET_FAILURE;
	UINT16 i;

	if((ts_route_id != NULL) || (ts_route != NULL))
	{
		for(i=0; i<CC_MAX_TS_ROUTE_NUM; i++)
		{
			if((g_ts_route_info[i].enable == 1) && (g_ts_route_info[i].type == type))
			{
				if(ts_route_id != NULL)
				{
					*ts_route_id = i;
				}
				if(ts_route!= NULL)
				{
					MEMCPY(ts_route, &g_ts_route_info[i], sizeof(struct ts_route_info));
					__ts_route_check_flag(ts_route);
				}
				ret = RET_SUCCESS;
				break;
			}
		}
	}

	return ret;
}

/* Get TS Route ID and Info of Prog which is Recording according to Prog ID */
RET_CODE ts_route_get_record(UINT32 prog_id, UINT32 *ts_route_id, struct ts_route_info *ts_route)
{
	RET_CODE ret = RET_FAILURE;
	UINT16 i;

	if((ts_route_id != NULL) || (ts_route != NULL))
	{
		for(i=0; i<CC_MAX_TS_ROUTE_NUM; i++)
		{
			if((g_ts_route_info[i].enable == 1) && (g_ts_route_info[i].is_recording == 1) && (g_ts_route_info[i].prog_id == prog_id))
			{
				if(ts_route_id != NULL)
				{
					*ts_route_id = i;
				}
				if(ts_route!= NULL)
				{
					MEMCPY(ts_route, &g_ts_route_info[i], sizeof(struct ts_route_info));
					__ts_route_check_flag(ts_route);
				}
				ret = RET_SUCCESS;
				break;
			}
		}
	}

	return ret;
}

/* Update TS Route */
RET_CODE ts_route_update(UINT32 id, struct ts_route_info *ts_route)
{
	RET_CODE ret = RET_SUCCESS;

	if(g_ts_route_info[id].enable == 0)
	{
		ret = RET_FAILURE;
	}
	else
	{
		MEMCPY(&g_ts_route_info[id], ts_route, sizeof(struct ts_route_info));
	}
	
	return ret;
}

/* Get TS Route ID of TS Route which Use Right CI Slot */
INT16 ts_route_check_ci(UINT8 ci_id)
{
	INT16 ret = -1;
	return ret;
}

/* Get ID and Number of All TS Routes which Include Right CI Slot and Get ID of TS Route which Use Right CI Slot Currently */
INT16 ts_route_check_ci_route(UINT8 ci_id, UINT8 *route_num, UINT16 *routes)
{
	INT16 ret = -1;
	return ret;
}

/* Get TS ID of Matched TS Route according to Input DMX ID */
UINT8 ts_route_check_TS_by_dmx(UINT8 dmx_id)
{
	UINT8 ret = 0;
	UINT16 i;

	for(i=0; i<CC_MAX_TS_ROUTE_NUM; i++)
	{
		if(g_ts_route_info[i].enable == 1)
		{
			if(g_ts_route_info[i].dmx_id == dmx_id)
			{
				ret = g_ts_route_info[i].ts_id;
				break;
			}			
		}
	}

	return ret;
}

/* Get ID and Number of Matched TS Route according to Input DMX ID */
void ts_route_check_dmx_route(UINT8 dmx_id, UINT8 *route_num, UINT16 *routes)
{
	UINT16 i;

	*route_num = 0;
	for(i=0; i<CC_MAX_TS_ROUTE_NUM; i++)
	{
		if(g_ts_route_info[i].enable == 1)
		{
			if(g_ts_route_info[i].dmx_id == dmx_id)
			{
				routes[*route_num] = i;
				(*route_num)++;
			}			
		}
	}
}

/* Get ID and Number of Matched TS Route according to Input TP ID */
void ts_route_check_tp_route(UINT32 tp_id, UINT8 *route_num, UINT16 (*routes)[])
{
	UINT16 i;

	*route_num = 0;
	for(i=0; i<CC_MAX_TS_ROUTE_NUM; i++)
	{
		if(g_ts_route_info[i].enable == 1)
		{
			if(g_ts_route_info[i].tp_id == tp_id)
			{
				(*routes)[*route_num] = i;
				(*route_num)++;
			}			
		}
	}
}

void ts_route_check_nim_route(UINT32 nim_id, UINT8 *route_num, UINT16 *routes)
{
	UINT16 i;

	*route_num = 0;
	for(i=0; i<CC_MAX_TS_ROUTE_NUM; i++)
	{
		if(g_ts_route_info[i].enable == 1)
		{
			if(g_ts_route_info[i].nim_id == nim_id)
			{
				routes[*route_num] = i;
				(*route_num)++;
			}			
		}
	}
}

UINT32 ts_route_get_by_prog_id(UINT32 prog_id, UINT8 dmx_id,  UINT8 is_pip, struct ts_route_info *ts_route)
{
	RET_CODE ret = RET_SUCCESS;
	UINT8 i;

	for(i=0; i<CC_MAX_TS_ROUTE_NUM; i++)
	{
		if((g_ts_route_info[i].enable == 1))
		{
			if((g_ts_route_info[i].type == TS_ROUTE_PIP_PLAY) && (is_pip) && (g_ts_route_info[i].prog_id == prog_id))
				break;
			if((g_ts_route_info[i].type != TS_ROUTE_PIP_PLAY) && (!is_pip) && (g_ts_route_info[i].prog_id == prog_id) && (g_ts_route_info[i].dmx_id == dmx_id))
				break;
		}	
	}

	if(i >= CC_MAX_TS_ROUTE_NUM)
	{
		ret = RET_FAILURE;
	}
	else
	{
		MEMCPY(ts_route, &g_ts_route_info[i], sizeof(struct ts_route_info));
	}
	return ret;
}
UINT8 ts_route_set_ci_info(UINT32 sim_id,CA_INFO *ca_info,UINT8 ca_count)
{
	UINT8	i;
	for (i=0;i<5;i++)
	{
		if((g_ts_route_info[i].enable)&&(g_ts_route_info[i].dmx_sim_info[0].sim_id == sim_id))
		{
			MEMCPY(g_ts_route_info[i].ca_info,ca_info,ca_count*sizeof(CA_INFO));
			g_ts_route_info[i].ca_count = ca_count;
			g_ts_route_info[i].ca_info_enable= 1;
			return RET_SUCCESS;
		}
	}
	return RET_FAILURE;
}

UINT8 is_ca_info_in_ts_route(UINT32 sim_id)
{
	UINT8 i;
	for (i=0;i<5;i++)
	{
		if((g_ts_route_info[i].dmx_sim_info[0].sim_id == sim_id)&&(g_ts_route_info[i].ca_info_enable == 1))
			return RET_SUCCESS;
	}
	return RET_FAILURE;
}

UINT8 ts_route_get_by_sim_id(UINT32 sim_id,UINT8 is_pip,	struct ts_route_info* ts_route)
{
	UINT8 i,j;
	if (ts_route ==NULL)
		return RET_FAILURE;
	for(i=0;i<5;i++)
	{
		if (!g_ts_route_info[i].enable)
			continue;
		for(j=0;j<DEV_DMX_SIM_CNT>>1;j++)
		{
			if ((g_ts_route_info[i].dmx_sim_info[j].sim_id ==sim_id)&&(g_ts_route_info[i].type ==  TS_ROUTE_PIP_PLAY)&&(is_pip == 1))
			{
				MEMCPY(ts_route,&g_ts_route_info[i],sizeof(struct ts_route_info));
				return RET_SUCCESS;
			}
			if((g_ts_route_info[i].dmx_sim_info[j].sim_id ==sim_id)
				&&((g_ts_route_info[i].type ==  TS_ROUTE_MAIN_PLAY) ||(g_ts_route_info[i].type ==  TS_ROUTE_BACKGROUND))
				&&(is_pip == 0))
			{
				MEMCPY(ts_route,&g_ts_route_info[i],sizeof(struct ts_route_info));
				return RET_SUCCESS;
			}
		}
	}
	MEMSET(ts_route,0,sizeof(struct ts_route_info));
	return RET_FAILURE;		
}

INT8 ts_route_get_nim_tsiid(UINT8 nim_id)
{
	return nim_tsiid[nim_id];
}

void ts_route_set_nim_tsiid(UINT8 nim_id, INT8 tsiid)
{
	nim_tsiid[nim_id] = tsiid;
}


