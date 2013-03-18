#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <api/libpub/lib_pub.h>
#include <api/libpub/lib_as.h>
#include <hld/nim/nim.h>

#include <hld/decv/decv.h>

#include <api/libosd/osd_lib.h>
#include <api/libtsi/psi_db.h>
#include <api/libsi/si_service_type.h>
#include <api/libsi/si_module.h>
#include <api/libsi/sie_monitor.h>
#include <api/libsi/ts_monitor.h>

#define SIM_INVALID_MON_ID	0xFFFFFFFF

static UINT32 uCurChanIndex = 0xffffffff;
static UINT32 ts_monitor_id = SIM_INVALID_MON_ID;
static service_changed_cb_t service_changed_notify = NULL;
static UINT16	view_create_flag_bk;
static UINT32	view_create_parm_bk;
static UINT32 last_crc = 0xffffffff;

static INT32 ts_monitor_callback(UINT8 *section, INT32 length, UINT32 param)
{
	INT service_count = 0;
	INT prog_num;
	INT16 prog_map_len;	
	UINT8 *prog_map = NULL;	
	P_NODE node;

	if(get_prog_by_id(uCurChanIndex, &node) != SUCCESS)
	{
		return SUCCESS;
	}	

	if(section != NULL)
	{
		UINT8 *pcrc = section + length - 4;
		UINT32 crc = (pcrc[0]<<24) | (pcrc[1]<<16) | (pcrc[2]<<8) | pcrc[3];
		
		if(last_crc == crc)
		{
			return SUCCESS;
		}
		else
		{
			last_crc = crc;
		}
		
		get_cur_view_feature(&view_create_flag_bk, &view_create_parm_bk);
		
		if(recreate_prog_view(VIEW_SINGLE_TP|PROG_ALL_MODE, node.tp_id) != DB_SUCCES)
		{
			recreate_prog_view(view_create_flag_bk, view_create_parm_bk);
			return SUCCESS;
		}
		prog_num = get_prog_num(VIEW_SINGLE_TP|PROG_ALL_MODE, node.tp_id); 
	
		prog_map_len = (((section[1]&0xF)<<8)|section[2]) - 9;
		prog_map = section + 8;

		/*get PMT pid using program number*/
		while(prog_map_len > 0)
		{
			P_NODE node;			
			INT i;
			BOOL found = FALSE;
			UINT16 service_id = prog_map[0]<<8 | prog_map[1];
			UINT16 pmt_pid = (prog_map[2]&0x1F)<<8 | prog_map[3];
			
			for(i=0; i<prog_num; i++)				
			{
				get_prog_at(i, &node);
				if(node.prog_number == service_id)
				{
			//		if(node.pmt_pid == pmt_pid)
					{
						found = TRUE;
						break;
					}
				}
			}

			if(!found && service_id!=0)
			{
				//need update...
				recreate_prog_view(view_create_flag_bk, view_create_parm_bk);
				if(service_changed_notify != NULL)
					service_changed_notify(node.tp_id);
			
				return SUCCESS;
			}

			if(service_id != 0)
				service_count++;

			prog_map_len -= 4;
			prog_map += 4;
		}	

		recreate_prog_view(view_create_flag_bk, view_create_parm_bk);

		if(service_count != prog_num)
		{
			//need update...
			if(service_changed_notify != NULL)
				service_changed_notify(node.tp_id);
		
			return SUCCESS;
		}
	}

	return SUCCESS;
}

INT ts_monitor_start(UINT32 index)
{
	struct dmx_device * dmx;
	
	uCurChanIndex = index;	
	
	if(ts_monitor_id != SIM_INVALID_MON_ID)
		return SUCCESS;

#if (SYS_PROJECT_FE == PROJECT_FE_DVBT) || (SYS_PROJECT_FE == PROJECT_FE_ISDBT)
#ifdef PVR_DYNAMIC_PID_MONITOR_SUPPORT
	dmx =(struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, (lib_nimng_get_nim_play()-1));
#else
	dmx = (struct dmx_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DMX);
#endif
#else
	dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);
#endif

	ts_monitor_id = sim_start_monitor(dmx, MONITE_TB_PAT, PSI_PAT_PID, 0);
	if(ts_monitor_id == SIM_INVALID_MON_ID)
	{
		return !SUCCESS;
	}
	sim_register_scb(ts_monitor_id, ts_monitor_callback, NULL);

	return SUCCESS;
}

INT ts_monitor_stop(void)
{
	if(ts_monitor_id == SIM_INVALID_MON_ID)
		return SUCCESS;

	sim_stop_monitor(ts_monitor_id);
	ts_monitor_id = SIM_INVALID_MON_ID;
	uCurChanIndex = 0xffffffff;
	
	return SUCCESS;
}

void ts_monitor_register(service_changed_cb_t callback)
{
	service_changed_notify = callback;
}

