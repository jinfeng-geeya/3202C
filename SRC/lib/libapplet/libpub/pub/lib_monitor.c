#include <osal/osal_task.h>
#include <api/libc/printf.h>
#include <api/libpub/lib_pub.h>
#include <api/libpub/lib_hde.h>
#include <api/libtsi/db_3l.h>
#include <api/libnim/lib_nim.h>
#include <hld/hld_dev.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/deca/deca_dev.h>
#include <hld/deca/deca.h>
#include <hld/decv/vdec_driver.h>
#include <hld/snd/snd_dev.h>
#include <hld/snd/snd.h>
#include <hld/deca/deca_dev.h>

/*******************************************************
* macro define
********************************************************/
#define MONITOR_PRINTF  PRINTF//libc_printf


#define LIBPUB_MONITOR_INTERVAL     50

/*******************************************************
* extern  declare
********************************************************/
extern OSAL_ID libpub_flag_id;

#define MONITOR_ENTER_MUTEX()   {UINT32 flgptn;\
	osal_flag_wait(&flgptn,libpub_flag_id,LIBPUB_MONITOR_MUTEX,TWF_ORW|TWF_CLR,OSAL_WAIT_FOREVER_TIME);}
#define MONITOR_LEAVE_MUTEX()   osal_flag_set(libpub_flag_id, LIBPUB_MONITOR_MUTEX);



/*******************************************************
* struct and variable
********************************************************/
static UINT32 monitor_check_interval =  0;
static libpub_monitor_callback monitor_callback = NULL;

static INT32 nim_unlock_cnt = 0;
static INT32 monitor_check_cnt = 0;
static UINT32 vdec_frame_idx = 0;

/*******************************************************
* internal function
********************************************************/
static void check_nim_status(struct nim_device *nim)
{
    UINT8 lock;

    nim->get_lock(nim, &lock);
    if(lock==0)
    {
        nim_unlock_cnt++;
        if(nim_unlock_cnt==25)
        {
            nim_unlock_cnt = 0;
            if(monitor_callback)
                monitor_callback(LIB_MONITOR_NIM_UNLOCK,(UINT32)nim);
            MONITOR_PRINTF("\nnim device 0x%x unlock, tick=%d\n",nim,osal_get_tick());
        }
    }
    else
    {
        nim_unlock_cnt = 0;
    }
}


static void check_decv_status(struct vdec_device *vdec)
{
    struct VDec_StatusInfo vdec_info;
	
    monitor_check_cnt++;
    if(monitor_check_cnt==100)
    {
        monitor_check_cnt = 0;
        vdec_io_control(vdec, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);
        if(vdec_info.uCurStatus==VDEC27_STARTED)
        {
            if(vdec_info.display_idx==vdec_frame_idx)
            {
                if(monitor_callback)
                    monitor_callback(LIB_MONITOR_VDEC_HOLD, (UINT32)vdec);
                MONITOR_PRINTF("\nvdec device 0x%x hold, tick=%d\n",vdec,osal_get_tick());
            }
        }
        vdec_frame_idx = vdec_info.display_idx;
    }
}


/*******************************************************
* external API
********************************************************/
void monitor_switch_dev_monitor(UINT32 device_flag, UINT8 onoff)
{
    if(onoff==0)
        osal_flag_clear(libpub_flag_id,device_flag);
    else if(onoff==1)
        osal_flag_set(libpub_flag_id,device_flag);
}



void MonitorInit(UINT32 interval, libpub_monitor_callback func)
{
    if(interval!=0)
        monitor_check_interval = interval;
    else
        monitor_check_interval = LIBPUB_MONITOR_INTERVAL;
    if(func!=NULL)
        monitor_callback = func;
    if(libpub_flag_id!=OSAL_INVALID_ID)
        MONITOR_LEAVE_MUTEX();
}


void  MonitorRst()
{
    MONITOR_ENTER_MUTEX();

    monitor_check_cnt = 0;
    vdec_frame_idx = 0;

    MONITOR_LEAVE_MUTEX();
   
}


void MonitorSwitch(UINT32 onoff)
{
    if(onoff==0)
    {
        MONITOR_ENTER_MUTEX();
        monitor_check_cnt = 0;
        nim_unlock_cnt = 0;
        MONITOR_LEAVE_MUTEX();
    }

    monitor_switch_dev_monitor(LIBPUB_MONITOR_CHECK_NIM, onoff);

    monitor_switch_dev_monitor(LIBPUB_MONITOR_CHECK_VDEC, onoff);
}

void MonitorProc()
{
    UINT32 flgptn;
    UINT16 device_id;
  
    osal_task_sleep(monitor_check_interval);
    MONITOR_ENTER_MUTEX();

    //check nim device
    if(osal_flag_wait(&flgptn, libpub_flag_id, LIBPUB_MONITOR_CHECK_NIM, OSAL_TWF_ANDW, 0) == OSAL_E_OK) 
    {
        struct nim_device *nim = NULL;

        device_id = 0;
        while(NULL!=(nim=dev_get_by_id(HLD_DEV_TYPE_NIM, device_id)))
        {
            check_nim_status(nim);
            device_id++;
        }
    }

    //check vdec device
    if (osal_flag_wait(&flgptn, libpub_flag_id, LIBPUB_MONITOR_CHECK_VDEC, OSAL_TWF_ANDW, 0) == OSAL_E_OK) 
    {
        struct vdec_device *vdec = NULL;

        device_id = 0;
        while(NULL!=(vdec=dev_get_by_id(HLD_DEV_TYPE_DECV, device_id)))
        {
            check_decv_status(vdec);
            device_id++;
        }       
    }

    //check other device if needed
    
      
    MONITOR_LEAVE_MUTEX();

}


