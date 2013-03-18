#include <basic_types.h>
#include <mediatypes.h>

#include <sys_config.h>
#ifndef DISABLE_OTA

#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libc/alloc.h>

#include <api/libdbc/lib_dbc.h>

#include <api/libtsi/sec_pat.h>
#include <api/libtsi/sec_pmt.h>
#include <api/libtsi/p_search.h>
//#include <api/libnim/lib_nim.h>

#include <hld/hld_dev.h>
#include <hld/sto/sto.h>
#include <hld/dmx/dmx.h>
#include <hld/decv/vdec_driver.h>
#include <hld/nim/nim_dev.h>
#include <api/libchunk/chunk.h>
#include <bus/flash/flash.h>

//#include <api/libtsi/si_descriptor.h>
#include <api/libupg/ota_upgrade.h>
#include <api/libupg/lib_upg4.h>

#include "../lib_upg4_com.h"

static OSAL_ID g_ota_tsk_id;
static OSAL_ID g_ota_mbf_id = OSAL_INVALID_ID;
OSAL_ID  g_ota_flg_id = OSAL_INVALID_ID;

static UINT8 ota_task_suspend_flag = 0;

t_parse_cb ota_parse_cb;

INT32 ota_send_cmd(struct ota_cmd* cmd,UINT32 tmout);

void ota_reg_parse_callback(t_parse_cb parse_cb)
{
    if(parse_cb == NULL)
        return;
    
    ota_parse_cb = parse_cb;
}

INT32 ota_send_cmd(struct ota_cmd* cmd,UINT32 tmout)
{
    return osal_msgqueue_send(g_ota_mbf_id, cmd, sizeof(struct ota_cmd),tmout);
}

BOOL ota_start_service(UINT16 pid,struct dl_info *info)
{
	struct ota_cmd this_cmd;
	UINT32 flgptn_after;

	this_cmd.cmd_type = OTA_START_SERVICE;
	this_cmd.para16 = pid;
	this_cmd.para32 = (UINT32)info;

	return (E_OK == ota_send_cmd(&this_cmd,OSAL_WAIT_FOREVER_TIME));
}

BOOL ota_stop_service(void)
{
	struct ota_cmd this_cmd;
	UINT flgptn = 0;
	
	struct dmx_device * dmx_dev;
	
	if((dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0)) == NULL)
	{
		UPG4_DEBUG("ERROR : get dmx dev failure !");
		return FALSE;   //OTA_FAILURE;
	}	
	dmx_io_control(dmx_dev, IO_STOP_GET_SECTION, (UINT32)NULL);

	this_cmd.cmd_type = OTA_STOP_SERVICE;
	this_cmd.para16 = 0;
	this_cmd.para32 = 0;

	ota_send_cmd(&this_cmd,OSAL_WAIT_FOREVER_TIME);
	return TRUE;
}

BOOL ota_check_service(UINT16* pid)
{	
	struct ota_cmd this_cmd;
	UINT32 flgptn_after;

	this_cmd.cmd_type = OTA_CHECK_SERVICE;
	this_cmd.para16 = 0;
	this_cmd.para32 = (UINT32)pid;
	return (E_OK == ota_send_cmd(&this_cmd,OSAL_WAIT_FOREVER_TIME));
}

BOOL ota_get_download_info(UINT16 pid,struct dl_info *info)
{
	struct ota_cmd this_cmd;
	UINT32 flgptn_after;

	this_cmd.cmd_type = OTA_GET_DOWNLOADINFO;
	this_cmd.para16 = pid;
	this_cmd.para32 = (UINT32)info;

	return (E_OK == ota_send_cmd(&this_cmd,OSAL_WAIT_FOREVER_TIME));
}

BOOL ota_start_download(UINT16 pid,t_progress_disp progress_disp)
{
	struct ota_cmd this_cmd;
	UINT32 flgptn_after;

	this_cmd.cmd_type = OTA_START_DOWNLOAD;
	this_cmd.para16 = pid;
	this_cmd.para32 = (UINT32)progress_disp;

	return (E_OK == ota_send_cmd(&this_cmd,OSAL_WAIT_FOREVER_TIME));
}

BOOL ota_start_burnflash(t_progress_disp progress_disp)
{
	struct ota_cmd this_cmd;
	UINT32 flgptn_after;

	this_cmd.cmd_type = OTA_START_BURNFLASH;
	this_cmd.para16 = 0;
	this_cmd.para32 = (UINT32)progress_disp;

	return (E_OK == ota_send_cmd(&this_cmd,100));
}

BOOL ota_cmd_implement(UINT16 ota_cmd, UINT16 para16, UINT32 para32)
{
	BOOL ret_bool = FALSE;
	
	switch(ota_cmd)
	{
#ifdef MAINCODE_OTA_SUPPORT	
	case OTA_CHECK_SERVICE:
		ret_bool = ota_cmd_check_service((UINT16*)para32);
		osal_flag_set(g_ota_flg_id,(ret_bool == TRUE)?PROC_SUCCESS:PROC_FAILURE);
		break;		
	case OTA_GET_DOWNLOADINFO:
		ret_bool = ota_cmd_get_download_info(para16,(struct dl_info *)para32);
		osal_flag_set(g_ota_flg_id,(ret_bool == TRUE)?PROC_SUCCESS:PROC_FAILURE);
		break;		
	case OTA_START_DOWNLOAD:
		ret_bool = ota_cmd_start_download(para16);
		osal_flag_set(g_ota_flg_id,(ret_bool == TRUE)?PROC_SUCCESS:PROC_FAILURE);
		break;
	case OTA_START_BURNFLASH:
		ret_bool = ota_cmd_start_burnflash();
        	osal_flag_set(g_ota_flg_id,(ret_bool == TRUE)?PROC_SUCCESS:PROC_FAILURE);
		break;
	case OTA_STOP_SERVICE:
		ret_bool = ota_cmd_service_stopped();
		osal_flag_set(g_ota_flg_id,PROC_STOPPED);
		break;		
#endif		
	default :		
		break;

	}
	
	return ret_bool;
}

void ota_cmd_implement_wrapped(struct ota_cmd * ota_cmd)
{
	ota_cmd_implement(ota_cmd->cmd_type,ota_cmd->para16,ota_cmd->para32);
}


void ota_task_suspend()
{
	ota_task_suspend_flag = 1;
}

void ota_task_resume()
{
	ota_task_suspend_flag = 0;
}


void ota_task(void)
{
	OSAL_ER ret_val;
	UINT32 msg_size;	
	struct ota_cmd msg;	
	while(1)
	{
		ret_val = osal_msgqueue_receive((VP)&msg, &msg_size, g_ota_mbf_id,2000);//OSAL_WAIT_FOREVER_TIME);

		/*suspend ota task for ipanel mw*/
		if(ota_task_suspend_flag==1)
		{
			osal_task_sleep(100);
			continue;
		}

		if ((ret_val != E_OK) || (msg_size != sizeof(struct ota_cmd)))
		{
			//            UPG4_DEBUG("OTA: parse begin\n");
			if(ota_parse_cb)
				ota_parse_cb();
			//            UPG4_DEBUG("OTA: parse end\n\n");
			continue;
		}
		ota_cmd_implement_wrapped(&msg);
	}
}

BOOL ota_init(void)
{
	T_CMBF t_cmbf;
	T_CTSK t_ctsk;

	t_cmbf.bufsz =  sizeof(struct ota_cmd) * 20;
	t_cmbf.maxmsz = sizeof(struct ota_cmd);
	g_ota_mbf_id = osal_msgqueue_create(&t_cmbf);

	if (g_ota_mbf_id == OSAL_INVALID_ID)
	{
		UPG4_DEBUG("OTA: cre_mbf ota_msgbuf_id failed in %s\n",__FUNCTION__);
		return FALSE;
	}

	g_ota_flg_id=osal_flag_create(0x00000000);
	if(OSAL_INVALID_ID==g_ota_flg_id)
	{
	    UPG4_DEBUG("OTA: cre_flg g_ota_flg_id failed in %s\n",__FUNCTION__);
	    return FALSE;  
	}	

	t_ctsk.task = (FP)ota_task;
	t_ctsk.stksz = 0x1000;
	t_ctsk.quantum = 10;
	t_ctsk.itskpri = OSAL_PRI_NORMAL;
	t_ctsk.name[0] = 'O';
	t_ctsk.name[1] = 'T';
	t_ctsk.name[2] = 'A';

	g_ota_tsk_id = osal_task_create(&t_ctsk);	
	if (g_ota_tsk_id == OSAL_INVALID_ID)
	{
	    UPG4_DEBUG("OTA: cre_tsk g_ota_tsk_id failed in %s\n",__FUNCTION__);
	    return FALSE;
	}
	
	
    return TRUE;
}

BOOL ota_close()
{
    if(E_OK != OS_DelMessageBuffer(g_ota_mbf_id))
    {
        UPG4_DEBUG("OTA: task message buffer free failed in %s\n",__FUNCTION__);
        return FALSE;
    }

    if(E_OK != OS_DelFlag(g_ota_flg_id))
    {
        UPG4_DEBUG("OTA: flag delete failed in %s\n",__FUNCTION__);
        return FALSE;
    }
    
    if(E_OK != osal_task_delete(g_ota_tsk_id))
    {
        UPG4_DEBUG("OTA: task delete failed in %s\n",__FUNCTION__);
        return FALSE;
    }

    return TRUE;
}
#endif

