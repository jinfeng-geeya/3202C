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
#include <udi/ipanel/ipanel_tuner.h>
#include <udi/ipanel/ipanel_av.h>


#include <api/libtsi/db_3l.h>
#include <api/libsi/si_service_type.h>
#include <api/libpub/lib_frontend.h>

//#define ipanel_porting_dprintf libc_printf

#define time_ms                 ipanel_porting_time_ms

/**************************************************
*frequency unit 10KHz
*symborate unit KBaud
*modulation (4 to 8) corresponding QAM(16 to 256)
**************************************************/
//#define FREQUENCY_MIN           (17000)
#define FREQUENCY_MIN		(0)  //for ipanel demo test freq100 
#define FREQUENCY_MAX		(88700)
#define TUNER_TIMEOUT		(5000)

struTunerParam tunningPara;
static OSAL_ID TUNER_TASK_ID = 0;
static int tuner_flag = 0;
static int tuner_delivery = 0;
extern struct nim_device*   g_nim_dev;

UINT32 cur_freq = 0;
int my_ts_id = 0 ;
static unsigned short serviceid = 360;
//static unsigned short serviceid = 0xa; //for ipanel demo test freq100 
//static unsigned short serviceid = 0x3e8;
extern int video_status;

int ipanel_get_serviceInfo(Service_Info *service);

static void tuner_manager_task(UINT32 param1, UINT32 param2)
{
    unsigned int Tuner_fre;
    unsigned int Tuner_symrate;
    unsigned int Tuner_modl;
    UINT8 lock = 0;
    unsigned int start__time    = 0;
    static int count = 0;
    union ft_xpond ft;

    while (1)
    {
	osal_semaphore_capture(tunningPara.sem,OSAL_WAIT_FOREVER_TIME);
        switch (tunningPara.status)
        {
        case TUNER_NEW_INPUT:
		Tuner_fre		 =  tunningPara.freq;
		Tuner_symrate      =  tunningPara.symrate;
		Tuner_modl           =  tunningPara.modl;
		tunningPara.status =  TUNERRING;
		osal_semaphore_release(tunningPara.sem);
		MEMSET(&ft,0,sizeof(ft));
		ft.common.type = FRONTEND_TYPE_C;
		ft.c_info.frq = Tuner_fre;
		ft.c_info.sym = Tuner_symrate;
		ft.c_info.modulation = Tuner_modl;
		frontend_set_nim(g_nim_dev, NULL, &ft, 1);	
		//nim_channel_change(g_nim_dev,Tuner_fre, Tuner_symrate, Tuner_modl);
		start__time  = time_ms();
            break;

        case TUNERRING:
			nim_get_lock ( g_nim_dev, &lock );
			if(lock)	
				tunningPara.status =  TUNER_SUCCESS;
			else
			{
				/*MEMSET(&ft,0,sizeof(ft));
				ft.common.type = FRONTEND_TYPE_C;
				ft.c_info.frq = tunningPara.freq;
				ft.c_info.sym = tunningPara.symrate;
				ft.c_info.modulation = tunningPara.modl;
				frontend_set_nim(g_nim_dev, NULL, &ft, 1);
				//nim_channel_change(g_nim_dev,tunningPara.freq, tunningPara.symrate, tunningPara.modl);  
				*/
				osal_task_sleep(50);
			}
			if((time_ms() - start__time) > TUNER_TIMEOUT)
				tunningPara.status =  TUNER_FAIL;
			osal_semaphore_release(tunningPara.sem);
			break;

        case TUNER_SUCCESS:
	    	tuner_flag = 1;
            break;

        case TUNER_FAIL:    
	    	tuner_flag = -1;
            break;

        default:
            break;
        }
    }
}

int ipanel_tuner_init(UINT16 ts_id, UINT16 service_id)
{
	int ret = 0;
	OSAL_T_CTSK tuner_task ;

	TUNER_TASK_ID = 0;
	tuner_flag = 0;
	tuner_delivery = 0;
	my_ts_id = ts_id ;
	if(ipanel_get_run_mode()==1) //OC需要设置start service       
		serviceid = service_id;
	else  //网络模式不需要
		serviceid = 0;

	tunningPara.freq    = 0;
	tunningPara.symrate = 0;
	tunningPara.modl    = 0;
	tunningPara.status  = TUNER_UNUSED;
	tunningPara.sem     = osal_semaphore_create(0);

	tuner_task.task=tuner_manager_task ;
	tuner_task.name[0] = 'T';
	tuner_task.name[1] = 'U';
	tuner_task.name[2] = 'N';
	tuner_task.stksz   = 0x2000 ;
	tuner_task.itskpri = OSAL_PRI_NORMAL;
	tuner_task.quantum = 5 ;
	tuner_task.para1   = 0 ;
	tuner_task.para2   = 0 ;//Reserved for future use.
	TUNER_TASK_ID = osal_task_create(&tuner_task);
	if(OSAL_INVALID_ID == TUNER_TASK_ID)
		ret = -1;   
	return ret;
}

int ipanel_tuner_exit(void)
{
	int ret=0,result=0;
	
	ret = osal_task_delete(TUNER_TASK_ID);	
	if(ret != OSAL_E_OK)
		result += -1;
	TUNER_TASK_ID = 0;
	
	ret = osal_semaphore_delete(tunningPara.sem);
	if(ret != OSAL_E_OK)
		result += -1;
	tunningPara.sem = 0;

	return result;
}

int get_tuner_event(unsigned int event[3])
{
	int ret = 0;
	if (tuner_flag == 1)
	{
		tuner_flag = 0;
		if (tuner_delivery == 1)
		{
			event[0] = EIS_EVENT_TYPE_DVB;
			event[1] = EIS_DVB_DELIVERY_TUNE_SUCCESS;
			event[2] = my_ts_id;
			my_ts_id = 0;
			tuner_delivery = 0;
			ret = 1;
			ipanel_porting_dprintf("[get_tuner_event] EIS_DVB_DELIVERY_TUNE_SUCCESS,event[2] =0x%x. \n",event[2] );
		}
	}
	else if (tuner_flag == -1)
	{
		tuner_flag = 0;
		if (tuner_delivery == 1)
		{
			event[0] = EIS_EVENT_TYPE_DVB;
			event[1] = EIS_DVB_DELIVERY_TUNE_FAILED;
			event[2] = my_ts_id;
			my_ts_id = 0;
			tuner_delivery = 0;			
			ret = 1;
		}	
	}
	return ret;
}
/**********************************************************************************/
/*Description: Tuner the frequency                                                */
/*Input      : frequency -- uints is 100Hz, symbol_rate -- uints is 100sym/s,     */
/*           : modulation (1 - 4) 1 - QAM16, 2 - QAM32, 3 - QAM64, 4 - QAM128     */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail.                                          */
/**********************************************************************************/
int ipanel_porting_delivery_tune(int frequency,int symbol_rate,int modulation)
{
    ipanel_porting_dprintf("[ipanel_porting_delivery_tune] freq:%d, rate:%d, mod:%d\n",frequency, symbol_rate, modulation);
    frequency = frequency/100;
    symbol_rate = symbol_rate/10;
    modulation += 3;
    
    if (    (symbol_rate > 0)
        && ((modulation  > 3) && (modulation < 9))
        && ((frequency >= FREQUENCY_MIN) && (frequency <= FREQUENCY_MAX)) )
    {
        /*if (    ((unsigned int)frequency   != tunningPara.freq)
             || ((unsigned int)symbol_rate != tunningPara.symrate)
             || ((unsigned int)modulation  != tunningPara.modl)   )   //same params.*/
        {
            tunningPara.status  = TUNER_NEW_INPUT;
            tunningPara.freq    = (unsigned int)frequency;
            tunningPara.symrate = (unsigned int)symbol_rate;
            tunningPara.modl    = (unsigned int)modulation;
            cur_freq = frequency;
        }
    }
    else
    {
        tunningPara.status  = TUNER_IDLE;
        tunningPara.freq    = 0;
    }
    tuner_delivery = 1;
    osal_semaphore_release(tunningPara.sem);

    return 0;
}

/**********************************************************************************/
/*Description: Get Signal Quality (0 ~ 100), 0 -- min, and 100 -- max             */
/*Input      : No                                                                 */
/*Output     : No                                                                 */
/*Return     : signal quality                                                     */
/**********************************************************************************/
int ipanel_porting_get_delSignalQuality(void)
{
	unsigned char quality = 0;
	int ret=0;
	ret = nim_get_SNR(g_nim_dev, &quality);
	if(OSAL_E_OK == ret )
		return (int)quality;
	else
		return -1;
}

/**********************************************************************************/
/*Description: Get Signal Strength (0 ~ 100), 0 -- min, and 100 -- max            */
/*Input      : No                                                                 */
/*Output     : No                                                                 */
/*Return     : signal strength                                                    */
/**********************************************************************************/
int ipanel_porting_get_delSignalStrength(void)
{
	unsigned char strength = 0;
	int ret=0;
	ret = nim_get_AGC(g_nim_dev, &strength);
	if(OSAL_E_OK == ret )
		return (int)strength;
	else
		return -1;
}


/**********************************************************************************/
/*Description: Open a service                                                     */
/*Input      : service play information                                           */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail.                                          */
/**********************************************************************************/
int ipanel_porting_service_open(DVBServicePlay *serviceplay)
{
	int ret = -1;
	ipanel_porting_dprintf("[ipanel_porting_service_open] called..\n");
	if (NULL != serviceplay) {
		/*physical deal here.
		  such as play programmer.etc.
		  play_program(serviceplay->serviceid, serviceplay->pcr_pid, serviceplay->video_pids[0],  serviceplay->audio_pids[0]);
		*/
		ret = 0;
	}
	return ret;  
}

/**********************************************************************************/
/*Description: Select a TS service                                                */
/*Input      : network id, ts stream id, service id                               */
/*Output     : No                                                                 */
/*Return     : 0 -- iMW admin service, 1 -- iMW don't admin service.              */
/**********************************************************************************/
int ipanel_porting_select_service(int network_id, int ts_id, int service_id)
{
	/*if iMW don't admin service.
	  post network_id, ts_id, service_id to another user,
	*/
	int i = 0,index = -1,ret = 0;
	unsigned int event[3]={0};
	Service_Info service;
	ipanel_porting_dprintf("[ipanel_porting_select_service] network_id=0x%x,ts_id=0x%x,service_id=0x%x..\n",network_id,ts_id,service_id);
	serviceid = service_id;
    
	service.net_id = network_id;
	service.ts_id = ts_id;
	service.service_id = service_id;
    
	if(ipanel_get_run_mode()!=1)
		*((volatile UINT8 *)0xb800002C) &= ~0x10; //set pin-mux to S-Flash
	ret=ipanel_get_serviceInfo(&service);
	if(ipanel_get_run_mode()!=1)
		*((volatile UINT8 *)0xb800002C) |= 0x10;  //set pin-mux to Ethernet
    
	if(0==ret)
	{
		my_ts_id = service.ts_id;
		if (video_status==1)
			ipanel_porting_av_stop();
	//shoule set nim each time
        //if(service.frequency != cur_freq)
	        {
			ipanel_porting_delivery_tune(service.frequency*100,service.symbol_rate*10,(service.modulation-3));
			osal_task_sleep(300);
			ipanel_porting_dprintf("[ipanel_porting_select_service] tune %d.\n",service.frequency);
	        }
		if(service.service_type != 12)
		{      
			ipanel_porting_dvb_select_av(service.pcr_pid, 0, service.video_pid, service.audio_pid, 0, 0,service.service_type);  
		}       
	}
	if(ipanel_get_run_mode()!=1)//for not auto jump frq under net mode
	{
		serviceid = 0;
	}
         
	return 1;   
}


/**********************************************************************************/
/*Description: Get the startup service id                                         */
/*Input      : network id, ts stream id, service id                               */
/*Output     : No                                                                 */
/*Return     : 0 -- iMW admin service, 1 -- iMW don't admin service.              */
/**********************************************************************************/
unsigned short ipanel_porting_get_startup_serviceid(void)
{
	ipanel_porting_dprintf("[ipanel_porting_get_startup_serviceid] serviceid = %d..\n",serviceid);
	return serviceid;
}

int ipanel_get_serviceInfo(Service_Info *service)
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
        ipanel_porting_dprintf("-->[ipanel_get_serviceInfo] service_id=%d,service_type=%d,service_name=%s.\n",
                                prog.prog_number,prog.service_type,prog.service_name); 
        if(service->service_id==prog.prog_number)
        {
             if(SUCCESS!=get_tp_by_id(prog.tp_id, &tp))
             {
                ipanel_porting_dprintf("[ipanel_get_serviceInfo]  get_tp_by_id failed \n");
                return -1;
             }
             if((service->ts_id==tp.t_s_id)&&(service->net_id==tp.network_id))
             {
                service->net_id = tp.network_id;                
                service->ts_id= tp.t_s_id;
                service->service_id = prog.prog_number;	             
                service->frequency = tp.frq;
                service->symbol_rate = tp.sym;
                service->modulation = tp.FEC_inner;
                service->service_type = prog.service_type;
                service->pcr_pid = prog.pcr_pid;
                service->video_pid = prog.video_pid;
                service->audio_pid = (UINT32)prog.audio_pid[0];
                return 0;
             }
             else if((service->ts_id==0) && (service->net_id==0))
             {
            //	if(tp.frq == cur_freq)
				{
		                    service->net_id = tp.network_id;                
		                    service->ts_id= tp.t_s_id;
		                    service->service_id = prog.prog_number;					
					service->frequency = tp.frq;
					service->symbol_rate = tp.sym;
					service->modulation = tp.FEC_inner;
					service->service_type = prog.service_type;
					service->pcr_pid = prog.pcr_pid;
					service->video_pid = prog.video_pid;
					service->audio_pid = (UINT32)prog.audio_pid[prog.cur_audio];
					return 0;
				}
             }
        }
    }
	
    ipanel_porting_dprintf("[ipanel_get_serviceInfo] no found  failed \n");
    return -1;
    
}


