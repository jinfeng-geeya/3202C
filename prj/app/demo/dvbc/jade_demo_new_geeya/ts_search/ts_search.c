#include <sys_config.h>

#ifdef TS_MONITOR_FEATURE

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
#include "ts_search.h"
#include "../copper_common/com_api.h"
#include "../copper_common/system_data.h"

extern void sdt_monitor_on(UINT32 index);

static UINT32 tss_last_index = 0xffffffff;
static UINT32 tss_tv_num = 0;
static UINT32 tss_radio_num = 0;

static void tss_event_handler(UINT8 event_type, UINT32 value)
{
	if(event_type==ASM_TYPE_ADD_PROGRAM)
	{
		P_NODE node;
		
		get_prog_at(value, &node);
		if(node.service_type == SERVICE_TYPE_DTV)
		{
			tss_last_index = node.prog_id;
                     tss_tv_num++;
		}
              else if(node.service_type == SERVICE_TYPE_DRADIO && tss_last_index == 0xffffffff)
              {
                    tss_last_index = node.prog_id;
                    tss_radio_num++;
              }
	}
	else if((event_type==ASM_TYPE_PROGRESS)&&(value==AS_PROGRESS_SCANOVER))
	{
		ap_send_msg(CTRL_MSG_SUBTYPE_CMD_TP_SERVICE_SEARCH_OVER, 0, FALSE);
	}
}

INT ts_service_search(DB_TP_ID tp_id)
{
	struct as_service_param parm;
	T_NODE node;
	struct vdec_device *vdec;
	struct vdec_io_get_frm_para_advance vfrm_param;
	UINT32 addr, len;

	if(SUCCESS != get_tp_by_id(tp_id, &node))
	{
		return !SUCCESS;
	}

	api_stop_play(0);
       stop_tdt();
       sdt_monitor_off();
	osal_task_sleep(100);
       tss_last_index = 0xffffffff;
       tss_tv_num = 0;
       tss_radio_num = 0;

	MEMSET(&parm, 0, sizeof(struct as_service_param));
	parm.as_prog_attr = P_SEARCH_TV | P_SEARCH_RADIO | P_SEARCH_DATA |
						P_SEARCH_FTA | P_SEARCH_SCRAMBLED;

	parm.as_frontend_type = FRONTEND_TYPE_C;
	parm.as_sat_cnt = 1;
	parm.sat_ids[0] = 1;
	parm.ft.c_param.sym = node.sym;
	parm.ft.c_param.constellation = node.FEC_inner;
	parm.as_from = tp_id;
	parm.as_to = 0;
	parm.as_method = AS_METHOD_TP;
	parm.as_p_add_cfg = PROG_ADD_REPLACE_OLD;
	parm.as_handler = tss_event_handler;

	vdec = (struct vdec_device*)dev_get_by_id(HLD_DEV_TYPE_DECV, 0);
	vfrm_param.ufrm_mode = VDEC_UN_DISPLAY;
	vfrm_param.request_frm_number = 1;
	vdec_io_control(vdec, VDEC_IO_GET_FRM_ADVANCE, (UINT32) &vfrm_param);
	if (vfrm_param.return_frm_number > 0)
	{
		addr = vfrm_param.tFrmInfo[0].uC_Addr;
		len = 720 * 576 * 3 / 2;
		db_search_init((UINT8*)addr, len);
	}
	else
		ASSERT(0);
	
	return as_service_start(&parm);
}

INT ts_service_search_end(void)
{
	UINT16 cur_channel;
       UINT8 av_flag = TV_CHAN;
       
	as_service_stop();
	db_search_exit();
       start_tdt();
       sdt_monitor_on(0);
       
       sys_data_check_channel_groups();
       if(tss_tv_num == 0 && tss_radio_num > 0)
            av_flag = RADIO_CHAN;
       sys_data_set_cur_chan_mode(av_flag);
       sys_data_change_group(0);
       
	cur_channel = get_prog_pos(tss_last_index);
	if(cur_channel != 0xFFFF)
		api_play_channel(cur_channel, TRUE, FALSE, FALSE);
    
       BackToFullScrPlay();
	return 0;
}

#endif
