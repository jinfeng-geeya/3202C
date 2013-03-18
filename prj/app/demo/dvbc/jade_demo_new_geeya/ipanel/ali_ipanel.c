#include <sys_config.h>
#include <basic_types.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libsi/si_module.h>
#include <api/libpub/lib_pub.h>
//#include <api/libosd/osd_lib.h>
#include "../win_com.h"
#include <udi/ipanel/ipanel_base.h>
#include <api/libsi/si_service_type.h>
#ifdef NETWORK_SUPPORT
#include <api/libtcpip/lwip/sockets.h>
#include <api/libtcpip/lwip/err.h>
#include <api/libtcpip/lwip/tcpip.h>
#endif

/*******************************************************
 * macro define
 ********************************************************/
//#define IPANEL_DEBUG
#ifdef IPANEL_DEBUG
#define IPANEL_PRINTF libc_printf
#else
#define IPANEL_PRINTF(...)    do{}while(0)
#endif

#define ALI_IPANEL_FLAG_ENTER_MW        0x00000001
#define ALI_IPANEL_FLAG_EXIT_MW         0x00000002



/*******************************************************
 * extern declare
 ********************************************************/
extern struct osd_device *g_osd_dev;


extern void key_task_suspend();
extern void key_task_resume();
extern void ota_task_suspend();
extern void ota_task_resume();



/*******************************************************
 * struct and variable
 ********************************************************/
static OSAL_ID ali_ipanel_flag_id = OSAL_INVALID_ID;

#define ALI_IPANEL_ENTER_MUTEX(flag)    {UINT32 flgptn;\
osal_flag_wait(&flgptn,ali_ipanel_flag_id,flag,TWF_ORW|TWF_CLR,OSAL_WAIT_FOREVER_TIME);}

#define ALI_IPANEL_LEAVE_MUTEX(flag)    {osal_flag_set(ali_ipanel_flag_id, flag);}


/*******************************************************
 * internal functions
 ********************************************************/

#ifdef NETWORK_SUPPORT
#define IP_ADDRESS          "192.168.0.3"
#define NM_ADDRESS          "255.255.255.0"
#define GW_ADDRESS          "192.168.0.1"

extern err_t ethernetif_init(struct netif *netif);
extern err_t tcpip_input(struct pbuf *p, struct netif *inp);

err_t sys_add_netif(struct ip_addr *ipaddr, struct ip_addr *netmask, struct ip_addr *gw)
{
	struct netif *new_netif = NULL;

	new_netif = MALLOC(sizeof(struct netif));

	if (new_netif == NULL)
		IPANEL_PRINTF("%s: Malloc for new net interface failed!\n", __FUNCTION__);

	new_netif = netif_add(new_netif, ipaddr, netmask, gw, (void *)NULL, ethernetif_init, tcpip_input);
	if (NULL == new_netif)
	{
		IPANEL_PRINTF("%s: Add network interface failed!\n", __FUNCTION__);
		return ERR_MEM;
	}

	netif_set_up(new_netif);
	netif_set_name(new_netif, "en");
	netif_set_default(new_netif);

	return ERR_OK;
}

int system_socket_init(void)
{
	struct ip_addr ipaddr, gw, netmask;
    	LWIP_CONFIG lwip_cfg;

	net_dm9ka_attach();
	
	ipaddr.addr=inet_addr(IP_ADDRESS);
	netmask.addr=inet_addr(NM_ADDRESS);
	gw.addr=inet_addr(GW_ADDRESS);

	lwip_cfg.mem.enable = TRUE;
	lwip_cfg.mem.start = __MM_SUB_PB_START_ADDR;//LWIP_MEM_ADDR can be a Macro to define a address value.
	lwip_cfg.mem.length = 32*1024;//At least 32*1024;
	
	lwip_cfg.memp.enable = TRUE;
	lwip_cfg.memp.start =  __MM_LWIP_MEMP_ADDR;//LWIP_MEMP_ADDR can be a Macro to define a address value.
	lwip_cfg.memp.length = __MM_LWIP_MEMP_LEN;//At least 1532*256;	
	tcpip_init(NULL, NULL, &lwip_cfg);
    
	if (ERR_OK != sys_add_netif(&ipaddr, &netmask, &gw))
	{
		return -1;
	}
	
	return 0;
}
#endif

/*******************************************************
 * external APIs
 ********************************************************/

INT32 ali_ipanel_exit_mw()
{
	INT32 ret = SUCCESS;
	UINT32 flag_value = 0;
	struct dmx_device *dmx = (struct dmx_device*)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);
	struct vpo_device *vpo_dev = (struct vpo_device*)dev_get_by_id(HLD_DEV_TYPE_DIS, 0);

	if (ali_ipanel_flag_id == OSAL_INVALID_ID)
	{
		IPANEL_PRINTF("%s() ali_ipanel_flag_id invalid! \n", __FUNCTION__);
		return  - 1;
	}
	
	//start dmx, ipanel porting stopped dmx and changed buff mode
	dmx_io_control(dmx, DMX_SWITCH_TO_UNI_BUF, 0);
	dmx_io_control(dmx, DMX_CHANGE_THLD, (0 << 16) | (16));
	dmx_start(dmx);

	//open vpo
	vpo_win_onoff(vpo_dev, TRUE);

	//open sie
/*	if (SUCCESS != sie_open(dmx, PSI_MODULE_CONCURRENT_FILTER, NULL, 0))
	{
		IPANEL_PRINTF("%s() sie_open failed! \n", __FUNCTION__);
		return  - 1;
	}*/

	//resum key task
	key_task_resume();

	//suspend ota task
	ota_task_resume();

	//start tdt
	start_tdt();

       sdt_monitor_on(0);

	//set flag to enable ali application
	ALI_IPANEL_LEAVE_MUTEX(ALI_IPANEL_FLAG_EXIT_MW);
	return ret;
}


void ipanel_task(UINT32 param1, UINT32 param2)
{
	ALI_IPANEL_ENTER_MUTEX(ALI_IPANEL_FLAG_ENTER_MW);

#ifdef NETWORK_SUPPORT	
	if(param1 != 1)
	{		
 	 	*((volatile UINT8 *)0xb800002C) |= 0x10;
		system_socket_init();
	}
#endif

	while (1)
	{
		ipaneldtv_task(param1, param2);
		//osal_task_sleep(1000);
		break;
	}
	//force change pinmux to s-flash.
	*((volatile UINT8 *)0xb800002C) &= ~0x10;

	//exit ipanel mw, resume ali application
	ali_ipanel_exit_mw();

	osal_task_exit(0);
}


void ipanel_task_init(UINT32 param1, UINT32 param2)
{
	OSAL_T_CTSK task_param;

	task_param.task = ipanel_task;
	task_param.name[0] = 'D';
	task_param.name[1] = 'E';
	task_param.name[2] = 'O';
	task_param.stksz = 0x18000;
	task_param.itskpri = OSAL_PRI_NORMAL;
	task_param.quantum = 10;
	task_param.para1 = param1;
	task_param.para2 = param2; //Reserved for future use.
	if (OSAL_INVALID_ID == osal_task_create(&task_param))
		ASSERT(0);
	ALI_IPANEL_LEAVE_MUTEX(ALI_IPANEL_FLAG_ENTER_MW);
}

INT32 ali_ipanel_enter_mw(UINT32 param1, UINT32 param2)
{
	struct osd_device *osd_dev = NULL;
	INT32 ret = SUCCESS;
	VSCR *lpVscr;
	
	if (ali_ipanel_flag_id == OSAL_INVALID_ID)
	{
		ali_ipanel_flag_id = osal_flag_create(0);
		if (ali_ipanel_flag_id == OSAL_INVALID_ID)
		{
			IPANEL_PRINTF("%s() create flag failed\n", __FUNCTION__);
			return  - 1;
		}
	}
	//close lib monitor
	dm_set_onoff(0);

	stop_tdt();

	epg_off();

       sdt_monitor_off();

	//suspend key task
	key_task_suspend();

	//suspend ota task
	ota_task_suspend();

	//close sie to stop get si section
	/*if (SUCCESS != sie_close())
	{
		IPANEL_PRINTF("%s() close sie failed\n", __FUNCTION__);
		return  - 1;
	}*/

	//close vpo
	struct vpo_device *vpo_dev = (struct vpo_device *)dev_get_by_id ( HLD_DEV_TYPE_DIS, 0 );
	vpo_win_onoff(vpo_dev, FALSE);

#if(CAS_TYPE==CAS_CDCA3||CAS_TYPE == CAS_TF||CAS_TYPE==CAS_CDCA)
	ap_osd_layer2_exit();
#endif

	//close osd
	osd_dev = (struct osd_device *)dev_get_by_id(HLD_DEV_TYPE_OSD, 0);
	if(osd_dev!=NULL)
		OSDDrv_Close((HANDLE)osd_dev);
	osd_dev = (struct osd_device *)dev_get_by_id(HLD_DEV_TYPE_OSD, 1);
	if(osd_dev!=NULL)
		OSDDrv_Close((HANDLE)osd_dev);

#ifdef USE_LIB_GE
	 struct ge_device* ge_dev= (struct ge_device*)dev_get_by_id(HLD_DEV_TYPE_GE, 0);

     app_gui_release(1);
	 ge_close(ge_dev);     
#endif
	
	//free osd vscr buf
	OSD_TaskBuffer_Free(osal_task_get_current_id(), NULL);
	lpVscr = OSD_GetTaskVscr(osal_task_get_current_id());
	lpVscr->lpbScr = NULL;

    	recreate_prog_view(VIEW_ALL|PROG_ALL_MODE, 0);
	ipanel_porting_dmx_init();
	
	//call ipanel init
	ipanel_task_init(param1, param2);

	//suspend on ali_ipanel_flag_id wait ipanel exit
	ALI_IPANEL_ENTER_MUTEX(ALI_IPANEL_FLAG_EXIT_MW);

	osd_dev = (struct osd_device *)dev_get_by_id(HLD_DEV_TYPE_OSD, 0);
	if(osd_dev!=NULL)
        ap_osd_init();

	#if 0//ndef MIS_AD
	#ifdef MULTI_CAS
	#if((CAS_TYPE==CAS_CDCA)||(CAS_TYPE==CAS_TF)||(CAS_TYPE==CAS_DVT))
	osd_dev = (struct osd_device *)dev_get_by_id(HLD_DEV_TYPE_OSD, 1);
	if(osd_dev!=NULL)
		ap_osd_layer2_init();
	#endif
	#endif
	#endif

	return ret;
}

