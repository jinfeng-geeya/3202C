/*
 * Copyright (C) ALi Shanghai Corp. 2005-2006
 *
 * File name	: lib_pub.c
 * Description	: pub middle layer implementation file.
 *
 */
#include <types.h>
#include <osal/osal.h>
#include <sys_config.h>
#include <retcode.h>
#include <api/libc/string.h>
#include <api/libc/printf.h>
#include <api/libpub/lib_pub.h>
#include <api/libtsi/sie.h>
#include <api/libtsi/si_types.h>
#include <api/libsi/si_module.h>


/*******************************************************
* macro define
********************************************************/
#define LIBPUB_OVERLAY_CMD_NUM		10
#define LIBPUB_HANDLER_MAP_NUM		5


/*******************************************************
* extern  declare
********************************************************/



/*******************************************************
* struct and variable
********************************************************/
struct cmd_handler_map
{
	UINT16 cmd_type;
	libpub_cmd_handler cmd_handler;
};

struct pub_module_config libpub_config;

OSAL_ID libpub_flag_id;
static OSAL_ID libpub_msgbuf_id;
static OSAL_ID libpub_task_id;
static UINT16 libpub_overlay_cmd_list[LIBPUB_OVERLAY_CMD_NUM];
static struct cmd_handler_map libpub_handler_map[LIBPUB_HANDLER_MAP_NUM];


/*******************************************************
* internal function
********************************************************/
/* Check whether command is in the overlay list or not */
static BOOL is_cmd_in_overlay_list(UINT16 cmd)
{
	UINT32 i;
    
	for(i=0;i<LIBPUB_OVERLAY_CMD_NUM;i++)
	{
		if(libpub_overlay_cmd_list[i] == cmd )
			break;
	}
	if(i>=LIBPUB_OVERLAY_CMD_NUM)
		return FALSE;
	else
		return TRUE;
}

/* Libpub Main Task: Recieve and Process CC Command */
static void libpub_frame(void)
{
	ER ret_val;
	UINT32 i;
	UINT32 overlay_number;
	BOOL is_next_cmd_exist = FALSE;
	UINT32 cmd_size;
	struct libpub_cmd cmd_in_queue;
	struct libpub_cmd this_cmd;//for implement this time
	static struct libpub_cmd s_next_cmd;  
 
	s_next_cmd.cmd_type = LIBPUB_CMD_NULL;//init 
	
	while(1)
	{
		if( LIBPUB_CMD_NULL == s_next_cmd.cmd_type)
		{
			if(osal_msgqueue_receive((VP)&cmd_in_queue, &cmd_size, libpub_msgbuf_id,10)==E_OK)
			    MEMCPY(&s_next_cmd,&cmd_in_queue,sizeof(struct libpub_cmd));
			else
			{
				if (libpub_config.dm_enable)
					MonitorProc();
				continue;
			}
		}
		if(is_cmd_in_overlay_list(s_next_cmd.cmd_type))
		{
			overlay_number = 0;
	    	do /* wait, and fetch last same command in buffer */
			{
		    	ret_val = osal_msgqueue_receive((VP)&cmd_in_queue, &cmd_size, libpub_msgbuf_id,TMO_POL);
		    	if ((ret_val != E_OK) || (cmd_size != sizeof(struct libpub_cmd)))
		    	{
		    		is_next_cmd_exist = FALSE;
		    		break; // if no more cmd, just break out while loop
		    	}
		    	else if(cmd_in_queue.cmd_type != s_next_cmd.cmd_type)
		    	{
		    		is_next_cmd_exist = TRUE;
		    		break; // if new cmd has diffrent type, record it
		    	}
		    	MEMCPY(&s_next_cmd,&cmd_in_queue,sizeof(struct libpub_cmd));
		    	overlay_number ++;
		    	PRINTF("\nlibpub: overlay cmd 0x%04x found %d times\n",s_next_cmd.cmd_type,overlay_number);
			}while (cmd_in_queue.cmd_type == s_next_cmd.cmd_type);
		}
		else //if not overlay cmd, just do it
		{
			is_next_cmd_exist = FALSE;
		}
		MEMCPY(&this_cmd,&s_next_cmd,sizeof(struct libpub_cmd));//full this cmd with next
		if(is_next_cmd_exist)
			MEMCPY(&s_next_cmd,&cmd_in_queue,sizeof(struct libpub_cmd));//full next with in queue
		else
			s_next_cmd.cmd_type = LIBPUB_CMD_NULL;//clear next cmd

		for(i=0;i<LIBPUB_HANDLER_MAP_NUM;i++)
		{
			if(libpub_handler_map[i].cmd_type == (this_cmd.cmd_type&LIBPUB_CMD_MAIN_TYPE_MASK))
			{		
				libpub_handler_map[i].cmd_handler(&this_cmd);
				if (libpub_config.dm_enable)
					MonitorRst();
				break;
			}
		}
		if(i>=LIBPUB_HANDLER_MAP_NUM)
			PRINTF("libpub: unregistered cmd found !\n");
	}
}

/* Create and Initialize Libpub Task */
static BOOL pub_task_init(void)
{
	T_CMBF t_cmbf;
	T_CTSK t_ctsk;

	MEMSET(libpub_handler_map,0,sizeof(libpub_handler_map));
	MEMSET(libpub_overlay_cmd_list,0,sizeof(libpub_overlay_cmd_list));
	t_cmbf.bufsz =  sizeof(struct libpub_cmd) * 20;//Command Message Buffer can Receive 20 Commands
	t_cmbf.maxmsz = sizeof(struct libpub_cmd);
	libpub_msgbuf_id = osal_msgqueue_create(&t_cmbf);
	if (libpub_msgbuf_id == OSAL_INVALID_ID)
	{
	    	PRINTF("cre_mbf libpub_msgbuf_id failed in %s\n",__FUNCTION__);
	    	return FALSE;
	}

	t_ctsk.task = (FP)libpub_frame;
	t_ctsk.stksz = 0x4000;
	t_ctsk.quantum = 10;
	t_ctsk.itskpri = OSAL_PRI_NORMAL;
	t_ctsk.name[0] = 'P';
	t_ctsk.name[1] = 'U';
	t_ctsk.name[2] = 'B';
	libpub_task_id = osal_task_create(&t_ctsk);
	if (libpub_task_id == OSAL_INVALID_ID)
	{
	    	PRINTF("cre_tsk libpub_task_id failed in %s\n",__FUNCTION__);
	    	return FALSE;
	}

	return TRUE;	
}


/*******************************************************
* external API
********************************************************/
/* Register Main Type Command */
BOOL libpub_cmd_handler_register(UINT16 cmd_main_type,libpub_cmd_handler cmd_handler)
{
	UINT32 i;
    
	for(i=0;i<LIBPUB_HANDLER_MAP_NUM;i++)
	{
		if((libpub_handler_map[i].cmd_type == LIBPUB_CMD_NULL) //empty cell
			|| (libpub_handler_map[i].cmd_type == (cmd_main_type&LIBPUB_CMD_MAIN_TYPE_MASK)))//exist type
		{
			libpub_handler_map[i].cmd_type = (cmd_main_type&LIBPUB_CMD_MAIN_TYPE_MASK);
			libpub_handler_map[i].cmd_handler = cmd_handler;
			break;
		}
	}
	if(i>=LIBPUB_HANDLER_MAP_NUM)//no room to register
	{
		PRINTF("Libpub: Fail to register cmd 0x%04x handler = %x\n",cmd_main_type,cmd_handler);
		return FALSE;
	}
	else
	{
		PRINTF("Libpub: OK register cmd 0x%04x handler = %x\n",cmd_main_type,cmd_handler);
		return TRUE;
	}
}

/* Register Subtype Command of Main Type */
BOOL libpub_overlay_cmd_register(UINT16 overlay_cmd)
{
	UINT32 i;
	for(i=0;i<LIBPUB_OVERLAY_CMD_NUM;i++)
	{
		if((libpub_overlay_cmd_list[i] == LIBPUB_CMD_NULL) //empty cell
			|| (libpub_overlay_cmd_list[i] == overlay_cmd ))//exist cmd
		{
			libpub_overlay_cmd_list[i] = overlay_cmd;
			break;
		}
	}
	if(i>=LIBPUB_OVERLAY_CMD_NUM)//no room to registe
	{
		PRINTF("Libpub: Fail to register overlay cmd 0x%04x \n",overlay_cmd);		
		return FALSE;
	}
	else
	{
		PRINTF("Libpub: OK register overlay cmd 0x%04x \n",overlay_cmd);		
		return TRUE;
	}
}

/* Clear Command Message Buffer */
INT32  libpub_clear_message_queue()
{
	UINT32 cmd_size;
	struct libpub_cmd cmd_in_queue;
	
   	while(osal_msgqueue_receive((VP)&cmd_in_queue, &cmd_size, libpub_msgbuf_id,0)==E_OK);
	
	return  RET_SUCCESS;
}

/* Send Command to Command Message Buffer */
INT32 libpub_send_cmd(struct libpub_cmd* cmd,UINT32 tmout)
{
    return osal_msgqueue_send(libpub_msgbuf_id, cmd, sizeof(struct libpub_cmd),tmout);
}

/* Wait Command Finish */
INT32 libpub_wait_cmd_finish(UINT32 cmd_bit,UINT32 tmout)
{
	UINT32 flgptn_after =0;
	
	return osal_flag_wait(&flgptn_after,libpub_flag_id, cmd_bit, OSAL_TWF_ANDW|OSAL_TWF_CLR, tmout);
}

/* Declare Command Finish */
void libpub_declare_cmd_finish(UINT32 cmd_bit)
{
	osal_flag_set(libpub_flag_id,cmd_bit);
}

/* Initialization */
__ATTRIBUTE_REUSE_
RET_CODE libpub_init(struct pub_module_config *config)
{
	INT32 ret = RET_SUCCESS;
	struct dmx_device *dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);
	
	libpub_config.dm_enable = config->dm_enable;
	
 	libpub_flag_id = osal_flag_create(0);
	if(OSAL_INVALID_ID==libpub_flag_id)
	{
		PRINTF("Create flag failed!\n");
		ret=RET_FAILURE;
	}
	
	if(SI_SUCCESS==sie_module_init())
		if(SI_SUCCESS==sie_open(dmx, PSI_MODULE_CONCURRENT_FILTER, NULL, 0))
			ret = RET_SUCCESS;
	
	/*--------------middle layer task----------- */
	pub_task_init();
	/*--------------channel change----------- */
	chchg_init();
	/*--------------multiview----------- */
    	mv_init();
	/*--------------monitor----------- */
	MonitorInit(config->monitor_interval,config->function);
	/*--------------search--------------*/
	as_service_register();
	
	return ret;
}


