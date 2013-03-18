/*********************************************************************
Copyright (c) 2005 Embedded Internet Solutions, Inc
All rights reserved. You are not allowed to copy or distribute
the code without permission.
This is construction how to use  APIs by iPanel MiddleWare to run.

Note: you should implenment to get outside message.

$ver0.1 $author Zouxianyun 2005/04/28
 *********************************************************************/

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include <udi/ipanel/ipanel_base.h>
#include <udi/ipanel/ipanel_graphics.h>
#include <udi/ipanel/ipanel_video.h>
#include <udi/ipanel/ipanel_audio.h>
#include <udi/ipanel/ipanel_sound.h>
#include <udi/ipanel/ipanel_media.h>
#include <udi/ipanel/ipanel_nvram.h>
#include <udi/ipanel/ipanel_socket.h>
#include <udi/ipanel/ipanel_demux.h>
#include <udi/ipanel/ipanel_tuner.h>
#include <udi/ipanel/ipanel_product.h>
#include <udi/ipanel/ipanel_UrlNetCursor.h>
#include <udi/ipanel/ipanel_av.h>
#include <udi/ipanel/ipanel_porting_event.h>

#include "ipanel_input.h"


static unsigned int ipanel_flag = 1;
static unsigned int ipanel_mode = 0;

int ipanel_init(UINT16 ts_id, UINT16 service_id)
{
	int ret = 0;
	ret += ipanel_base_init();
	ret += ipanel_tuner_init(ts_id, service_id);
	ret += ipanel_av_init();
	ret += ipanel_gfx_init();
	return ret;
}

int ipanel_exit()
{
	int ret = 0;
	ret += ipanel_gfx_exit();
	ret += ipanel_av_exit();
	ret += ipanel_tuner_exit();
	ret += ipanel_porting_dmx_exit();
	return ret;
}

void ipaneldtv_task(UINT32 param1, UINT32 param2)
{
	void *pDtvhandle = NULL;
	unsigned int event[3] = {0};
	int ret = 0;
	struct browser_entry *pentry = (struct browser_entry *)param2;
	UINT16 ts_id = 0;
	UINT16 service_id = 0;
	UINT32 freq = 0;

	ipanel_flag = 1;
	ipanel_mode = param1;
	if(pentry!=NULL)
	{
		ts_id = pentry->ts_id;
		service_id = pentry->service_id;
		freq = pentry->freq;
	}
	ret = ipanel_init(ts_id, service_id);
	if(ret<0){
		dprintf("[ipaneldtv_task] ipanel init failed...\n");
		return ;
	}
/*	
	ipanel_test();
        while(1)
	{
		osal_task_sleep(3000);
	}
*/
	pDtvhandle =(void *)ipaneldtv_create(IPANEL_MW_BUF_SIZE-0x400000);
	if (NULL != pDtvhandle)
	{
        //进入时，设置全屏
	    ipanel_porting_video_location(0, 0, 0, 0);	
        
		if (param1 == 1)
		{
			ipanel_porting_delivery_tune((freq*100),68750,3);
			osal_task_sleep(300);
		}
		else
		{
			ipaneldtv_open_url(pDtvhandle, "http://192.168.10.207/");
		}

		while(ipanel_flag) 
        	{
			ret = eis_input_get(event);
			if (1 == ret)
			{
				if (event[1] == EIS_IRKEY_RED)
				{
                    			//ipaneldtv_open_url(pDtvhandle, "http://192.168.10.207/");
                    			ipaneldtv_open_url(pDtvhandle, "http://192.168.10.207/testsuite/suite/javascript/js-api-222/media/av/media.av().htm");
                		}		
				else
				{   
                    			ipaneldtv_proc(pDtvhandle, (int)event[0], event[1], event[2]);
                		}
			}
			else
			{
				if(get_tuner_event(event) == 1)	
                		{
					ipaneldtv_proc(pDtvhandle, (int)event[0], event[1], event[2]);
				}
				else 
                		{
					ipaneldtv_proc(pDtvhandle, 0, 0, 0);
				}
			}
		}
		
		/* Exit dmx module firstly, because if still got sections to parse, need 
		ipanel filter management handle, but below call of "ipaneldtv_destroy"
		will destroy that handle */
		ipanel_porting_dmx_task_exit();
		
		ipaneldtv_destroy(pDtvhandle);
	}

	ret = ipanel_exit();
	if(ret < 0)
    {
		dprintf("[ipaneldtv_task] ipanel exit failed...\n");
	}
	ipanel_porting_dprintf("[ipaneldtv_task] end.\n");
}

/*!
 * \brief 			关闭browser，退出到各种应用。这个函数由 iPanel MiddleWare 来调用。
 * \author 		niefangxing	
 * \param[in] 	type 		
\n
\li \c -1或 0 :正常退出。
\n
\li \c 1 :退出browser，启动EPG应用。
\n
\li \c 2 : 退出browser，启动NVOD应用。
\n
\li \c 3 : 退出browser，启动mosaic应用。
 * \param[out]	无
 * \return			0 – 成功，-1 – 失败

 */
int ipanel_porting_system_close(int type)
{
	ipanel_porting_dprintf("[ipanel_porting_system_close] type=%d..\n", type);
	if(type==0||type==1)
    {
		ipanel_flag = 0;
		return 0;
	}
    else
    {   
        return  - 1;
    }		
}

unsigned int ipanel_get_run_mode(void)
{
	return ipanel_mode;
}


