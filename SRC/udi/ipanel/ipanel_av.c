/*********************************************************************
    Copyright (c) 2005 Embedded Internet Solutions, Inc
    All rights reserved. You are not allowed to copy or distribute
    the code without permission.
    This is the demo implenment of the EPG Porting APIs needed by iPanel MiddleWare.
    Maybe you should modify it accorrding to Platform.

    Note: the "int" in the file is 32bits

    $ver0.0.0.1 $author Zouxianyun 2005/04/28
*********************************************************************/
#include <udi/ipanel/ipanel_base.h>
#include <udi/ipanel/ipanel_porting_event.h>
#include <udi/ipanel/ipanel_av.h>

#include <api/libtsi/db_3l.h>
#include <api/libsi/si_service_type.h>




extern struct vdec_device*  g_decv_dev ;
extern struct deca_device*  g_deca_dev ;
extern struct snd_device*   g_snd_dev ;
extern struct vpo_device*   g_vpo_dev ;
extern struct dmx_device*   g_dmx_dev ;
extern UINT32 cur_freq;
extern int my_ts_id;
static struct io_param io_parameter;
static UINT16 PID_list[3]={0,0,0};

int video_status        =  0;
static short audioPid   =  0;
static short videoPid   =  0;
static short pcrPid     =  0;
static UINT16 progpos=0xffff;
int ipanel_get_service_ByPID(Service_Info *service)
{
    int i=0;
    short prog_cnt = 0;
    P_NODE prog;
    T_NODE tp;

    prog_cnt = get_prog_num(VIEW_ALL|PROG_ALL_MODE, 0);
    for(i = 0; i < prog_cnt; i++)
    {
        if(SUCCESS!=get_prog_at(i, &prog))
            return -1;
        ipanel_porting_dprintf("-->[ipanel_get_service_ByPID] service_id=%d,service_type=%d,.\n",
                                prog.prog_number,prog.service_type); 
         if(service->service_type!=prog.service_type)
		 	continue;
         if(service->video_pid != 0x0)
         {
            //use video_pid cross frq and do something
            if((service->video_pid == prog.video_pid)&&(service->audio_pid== (UINT32)prog.audio_pid[0])&&(service->pcr_pid==prog.pcr_pid))
             {
			 	progpos=get_prog_pos(prog.prog_id);
                if(SUCCESS != get_tp_by_id(prog.tp_id, &tp))
                {
                    ipanel_porting_dprintf("[ipanel_get_service_ByPID]  get_tp_by_id failed \n");
                    return -1;
                }
                service->ts_id = tp.t_s_id;
				service->net_id = tp.network_id;
                service->frequency = tp.frq;
                service->symbol_rate = tp.sym;
                service->modulation = tp.FEC_inner;
                service->service_type = prog.service_type;
                service->pcr_pid = prog.pcr_pid;
                service->video_pid = prog.video_pid;
                service->audio_pid = (UINT32)prog.audio_pid[0];
				service->service_id=prog.prog_number;
                return 0;
             }
         }
         else if(service->audio_pid != 0x0)
         {
             //use audio_pid cross frq and do something
            // if(service->audio_pid == (UINT32)prog.audio_pid[0])
             if((service->video_pid == prog.video_pid)&&(service->audio_pid== (UINT32)prog.audio_pid[0])&&(service->pcr_pid==prog.pcr_pid))
             {      
			 	 progpos=get_prog_pos(prog.prog_id);
                if(SUCCESS != get_tp_by_id(prog.tp_id, &tp))
                {
                    ipanel_porting_dprintf("[ipanel_get_service_ByPID]  get_tp_by_id failed \n");
                    return -1;
                }
                service->ts_id = tp.t_s_id;
				service->net_id = tp.network_id;
                service->frequency = tp.frq;
                service->symbol_rate = tp.sym;
                service->modulation = tp.FEC_inner;
                service->service_type = prog.service_type;
                service->pcr_pid = prog.pcr_pid;
                service->video_pid = prog.video_pid;
                service->audio_pid = (UINT32)prog.audio_pid[0];
				service->service_id=prog.prog_number;
                return 0;
             }
         }
    }
    ipanel_porting_dprintf("[ipanel_get_service_ByPID] no found  failed \n");
    return -1;
        
}

int ipanel_porting_dvb_select_av(int pcr_pid,   int emm_pid, int vpid,  int apid, int audio_ecm_pid, int video_ecm_pid,int servicetype)
{
	RET_CODE ret_code = 0;
	Service_Info tmp_service;

	dprintf("[ipanel_porting_dvb_select_av] V/A/Pcr = %d/%d/%d  video_status = %d \n",vpid,apid,pcr_pid, video_status);
		//return 0;
	if((apid==0)&&(vpid==0)&&(pcr_pid==0))
	{
        	return -1;
    	}
		
	if((audioPid==apid) && (videoPid==vpid) && (pcrPid==pcr_pid))
	{   
        	return 0;
    	}
		
	else if((audioPid||videoPid||pcrPid)&&(video_status==1))
	{
        	ipanel_porting_av_stop();
    	}
		
	/*here the program PID, audio PID video PID were fixed ,   so this demo code was banded with a specified ts.
	just modify these related marco when you use this demo code. */
	io_parameter.io_buff_in = (UINT8 *)PID_list;
	PID_list[0] = vpid;//pCmd->pids.video_id;
	PID_list[1] = apid;//pCmd->pids.audio_id;
	PID_list[2] = pcr_pid;//pCmd->pids.pcr_id;
	pcrPid = pcr_pid;
    //if(ipanel_get_run_mode()==1)   //if mode is flash mode,search by vid or aid
    {

        int ret=0;
        tmp_service.video_pid = vpid;
        tmp_service.audio_pid = apid;
		tmp_service.pcr_pid=pcr_pid;
		tmp_service.service_type=servicetype;
        if(ipanel_get_run_mode()!=1)
        {
            *((volatile UINT8 *)0xb800002C) &= ~0x10; //set pin-mux to S-Flash
	        osal_task_sleep(2000);
        }
	         
        ret = ipanel_get_service_ByPID(&tmp_service);
        
        if(ipanel_get_run_mode()!=1)
        {    
	    *((volatile UINT8 *)0xb800002C) |= 0x10;  //set pin-mux to Ethernet
            osal_task_sleep(2000); //for not crash quickly
        }

                    
        if(ret == SUCCESS)
        {     
			PID_list[2] = pcrPid = tmp_service.pcr_pid;
			if(tmp_service.frequency != cur_freq )
            { 
		        my_ts_id = tmp_service.ts_id;
                ipanel_porting_delivery_tune(tmp_service.frequency*100,tmp_service.symbol_rate*10,(tmp_service.modulation-3));
                osal_task_sleep(300);
    		    ipanel_porting_dprintf("[ipanel_porting_dvb_select_av] ts_id : 0x%x ,tune %d.\n",tmp_service.ts_id,tmp_service.frequency);
            }
        }
        else
        {
            ipanel_porting_dprintf("[ipanel_porting_dvb_select_av] can't get the service.but still play av by pid\n");
            //return -1;
        }
            
    }
	UINT32 create_stream = 0;
	UINT32 enable_stream = 0;

	if(vpid && apid)
	{
		create_stream = IO_CREATE_AV_STREAM;
		enable_stream = IO_STREAM_ENABLE;
	}
	else if((vpid==0) && apid)
	{
		create_stream = IO_CREATE_AUDIO_STREAM;
		enable_stream = AUDIO_STREAM_ENABLE;
	}
	else if((apid==0) && vpid)
	{
		create_stream = IO_CREATE_VIDEO_STREAM;
		enable_stream = VIDEO_STREAM_ENABLE;
	}
	if(create_stream!=0)
		ret_code = dmx_io_control(g_dmx_dev,create_stream,(UINT32)&io_parameter);
	if(ret_code != RET_SUCCESS)
    	{
		dprintf("[ipanel_porting_av_start] dmx create stream fail\n");
		return -1;
	}

	if(vpid)
    	{
		ret_code = vdec_start(g_decv_dev);
		if(ret_code != RET_SUCCESS)
        	{
			dprintf("[ipanel_porting_av_start]	cc2: VDec_Decode() fail\n");
			return -1;
		}
		else
        	{
	    		dprintf("[ipanel_porting_av_start]	cc2: vdec_start() OK\n");
		}
		videoPid = vpid;
	}
	
	if(apid)
    	{
		deca_io_control(g_deca_dev, DECA_SET_STR_TYPE, AUDIO_MPEG2);
		ret_code = deca_start(g_deca_dev,0);	/* open the audio device */
		if(ret_code != RET_SUCCESS)
        	{
			dprintf("[ipanel_porting_av_start]	cc2: deca_start() fail \n");
			return -1;
		}
		else
		{
	    	dprintf("[ipanel_porting_av_start]	cc2: deca_start() OK \n");
		}
		audioPid = apid;
	}
	if(enable_stream!=0)
		ret_code = dmx_io_control(g_dmx_dev,enable_stream,(UINT32)&io_parameter);
	if(ret_code != RET_SUCCESS)
	{
		dprintf("[ipanel_porting_av_start]	cc2: dmx enable stream fail\n");
		return -1;
	}
	else
	{
		dprintf("[ipanel_porting_av_start]	cc2: dmx enable stream OK\n");
	}
	video_status = 1;

	


	if(progpos!=0xffff){
		api_mcas_start_service(0);
		api_mcas_start_transponder();
		si_monitor_on(progpos);
		
	}

				
	dprintf("[ipanel_porting_av_start]	cc2: V/A/Pcr = %d/%d/%d\n",PID_list[0],PID_list[1],PID_list[2]);
	return 0;
}

void ipanel_porting_av_stop(void)
{
	RET_CODE ret_code;

	if(video_status == 1)
	{
		if(audioPid)
		{
			ret_code = deca_stop(g_deca_dev,0,ADEC_STOP_IMM);
			if (ret_code != RET_SUCCESS)
			{
				dprintf("[ipanel_porting_av_stop]  cc2: deca_stop() fail..\n");
			}	
			else
            		{
				dprintf("[ipanel_porting_av_stop]  cc2: deca_stop() ok..\n");
				audioPid = 0;
			}
		}
		
		if(videoPid)
		{
			ret_code = vdec_stop(g_decv_dev,TRUE,TRUE);

			if (ret_code == RET_BUSY)
			{
				ret_code = vdec_stop(g_decv_dev,TRUE,TRUE);
			}
			if (ret_code != RET_SUCCESS)
			{
				dprintf("[ipanel_porting_av_stop]  cc2: Vdec_Stop() fail \n");
            		}		
			else
            		{
				dprintf("[ipanel_porting_av_stop]  cc2: Vdec_Stop() ok \n");
				videoPid = 0;
			}
		}

		pcrPid = 0;
		ret_code = dmx_io_control(g_dmx_dev,IO_STREAM_DISABLE,(UINT32)&io_parameter);
		if (ret_code != RET_SUCCESS)
        	{
			dprintf("[ipanel_porting_av_stop]  cc2: Dmx disable stream fail \n");
			return;
		}
		else
		{
			dprintf("[ipanel_porting_av_stop]  cc2: Dmx disable stream OK \n");
        	}		
		ret_code = dmx_io_control(g_dmx_dev,IO_DELETE_AV_STREAM,(UINT32)&io_parameter);
		if(ret_code != RET_SUCCESS)
        	{
			dprintf("[ipanel_porting_av_stop] dmx delete stream fail..\n");
		}
		else
		{
			dprintf("[ipanel_porting_av_stop] dmx delete stream ok..\n");
        	}

		video_status = 0;

		if(  progpos!=0xffff)
		{
			api_mcas_stop_transponder();
			api_mcas_stop_service();
			si_monitor_off(progpos);
			progpos=0xffff;
			
		}

	}
}

int ipanel_av_init(void)
{
	struct pes_retrieve_param pes_service;
	
	struct VDecPIPInfo vInitInfo;
	struct MPSource_CallBack vMPCallBack;
	struct PIPSource_CallBack vPIPCallBack;

	MEMSET((UINT8 *)&io_parameter, 0, sizeof(struct io_param));
	PID_list[0]=0;
	PID_list[1]=0;
	PID_list[2]=0;
	video_status	=  0;
	audioPid   =  0;
	videoPid   =  0;
	pcrPid     =  0;
	
	vdec_set_output ( g_decv_dev,	MP_MODE, &vInitInfo, &vMPCallBack, &vPIPCallBack );
	//if(RET_SUCCESS!= VDec_Decode(0,ALL_PIC,	FALSE,0,FALSE,FALSE,FALSE))
	//	ASSERT(0);
	vpo_win_mode ( g_vpo_dev, VPO_MAINWIN, &vMPCallBack, &vPIPCallBack );
#ifdef VDEC_AV_SYNC
	vdec_sync_mode ( g_decv_dev, 0x00, VDEC_SYNC_I | VDEC_SYNC_P | VDEC_SYNC_B );
	deca_set_sync_mode ( g_deca_dev, ADEC_SYNC_PTS );
#endif

	//Binding Video Decoder to DMX
	pes_service.device = g_decv_dev;
	pes_service.filter_idx = 0;
	pes_service.retrieve_fmt = PES_HEADER_DISCARDED;
	pes_service.str_type = VIDEO_STR;
	pes_service.request_write = vdec_vbv_request ;
	pes_service.update_write = vdec_vbv_update ;
	dmx_io_control ( g_dmx_dev, DMX_BINDING_PES_RETRIEVE, ( UINT32 ) ( &pes_service ) );

	//Binding Audio Decoder to DMX
	pes_service.device = g_deca_dev;
	pes_service.filter_idx = 1;
	pes_service.retrieve_fmt = PES_HEADER_DISCARDED;
	pes_service.str_type = AUDIO_STR;
	pes_service.request_write = deca_request_write;
	pes_service.update_write = deca_update_write ;
	dmx_io_control ( g_dmx_dev, DMX_BINDING_PES_RETRIEVE, ( UINT32 ) ( &pes_service ) );
	
	return 0;
}

int ipanel_av_exit(void)
{
	if(video_status==1)
	{
		ipanel_porting_av_stop();
	}
    //退出时，设置全屏
	ipanel_porting_video_location(0, 0, 0, 0);	
    return 0;
}


