/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2003 Copyright (C)
 *
 *  File: power.c
 *
 *  Description: Routine for power off.
 *
 *  History:
 *      Date        Author      Version  Comment
 *      ====        ======      =======  =======
 *  1.  2003.5.1    Liu Lan     0.1.000  Initial
 *
 ****************************************************************************/

#include <sys_config.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/pan/pan_key.h>
#include <hld/pan/pan_dev.h>
#include <osal/osal.h>
#include <hal/hal_gpio.h>
#include <hld/dis/vpo.h>
#include <hld/deca/deca_dev.h>
#include <hld/pan/pan.h>

#include <api/libpub/lib_hde.h>
#include <api/libpub/lib_pub.h>
//#include <api/libosd/osd_vkey.h>

#include "win_com.h"
#include "key.h"
#include "control.h"

#define POW_PRINTF  libc_printf

#define IR_POWER_KEY_BASE		0x0160
#define PAN_POWER_KEY_BASE	0x0170

extern UINT8 led_display_flag;


void power_off_process(void)
{
	struct YCbCrColor bgcolor;

	if (hde_get_mode() != VIEW_MODE_MULTI)
		api_stop_play(1);

	vpo_win_onoff(g_vpo_dev, FALSE);
	OSD_ShowOnOff(C_OFF);
	si_monitor_off(0xFFFFFFFF);
	stop_tdt();
	epg_off();
	dm_set_onoff(0);

	bgcolor.uY = 16;
	bgcolor.uCb = 128;
	bgcolor.uCr = 128;
	vpo_ioctl(g_vpo_dev, VPO_IO_SET_BG_COLOR, (UINT32) &bgcolor);

	//close drivers
	dmx_stop(g_dmx_dev);
	dmx_close(g_dmx_dev);
	deca_stop(g_deca_dev, 0, ADEC_STOP_IMM);
	deca_close(g_deca_dev);
	vdec_close(g_decv_dev);
	vpo_close(g_vpo_dev);
	snd_close(g_snd_dev);
	nim_close(g_nim_dev);
	if (g_nim_dev2)
		nim_close(g_nim_dev2);

	//power_delay(200);
	led_display_flag = 0;
    
#if	(TTX_ON == 1)
	ttx_enable(FALSE);
#endif
#if (SUBTITLE_ON == 1)
	subt_enable(FALSE);
#endif

}

void power_on_process(void)
{
	date_time dt; //ctchen
	SYSTEM_DATA *sys_data;

	sys_data = sys_data_get();

	key_pan_display(" on ", 4);

	pan_close(g_pan_dev);

	if (sys_data->local_time.buseGMT)
		get_STC_time(&sys_data->sys_dt);
	else
		get_local_time(&sys_data->sys_dt);
	//ctchen

	sys_data->bstandmode = 0;
	sys_data_save(1);

	sys_watchdog_reboot();
}

#if 0
//Add power key auto-study and save to flash, by trueve_hu(08-05-15)
void power_key_auto_study(void)
{
	struct sto_device *sto_dev = NULL;
	UINT32 ir_pkey, ir_pkey_f, pan_pkey, pan_pkey_f = 0;
	int i;

	sto_dev = dev_get_by_id(HLD_DEV_TYPE_STO, 0);
	if (NULL == sto_dev)
	{
		POW_PRINTF("%s: Can not find any storage device!\n", __FUNCTION__);
		return;
	}

	ir_pkey = IR_HKEY_POWER;
	pan_pkey = PAN_HKEY_POWER;

	for (i=0; i<4; i++)
	{
		ir_pkey_f = sto_fetch_long(IR_POWER_KEY_BASE + 4*i);
		ir_pkey_f = ((ir_pkey_f>>24)&0xff) | ((ir_pkey_f>>8)&0xff00) | \
			((ir_pkey_f<<8)&0xff0000) | ((ir_pkey_f<<24)&0xff000000);

		if (ir_pkey_f == 0xFFFFFFFF)
		{
			sto_put_data(sto_dev, IR_POWER_KEY_BASE + 4*i, (UINT8 *)&ir_pkey, 4);
			break;
		}
		
		if (ir_pkey_f ==ir_pkey)
		{
			break;
		}
	}

	for (i=0; i<4; i++)
	{
		pan_pkey_f = sto_fetch_long(PAN_POWER_KEY_BASE + 4*i);
		pan_pkey_f = ((pan_pkey_f>>24)&0xff) | ((pan_pkey_f>>8)&0xff00) | \
			((pan_pkey_f<<8)&0xff0000) | ((pan_pkey_f<<24)&0xff000000);

		if (pan_pkey_f == 0xFFFFFFFF)
		{
			sto_put_data(sto_dev, PAN_POWER_KEY_BASE + 4*i, (UINT8 *)&pan_pkey, 4);

			break;
		}
		
		if (pan_pkey_f == pan_pkey)
		{
			break;
		}
	}
}
#endif

void power_switch(UINT32 mode)
{
	unsigned long keycode;
	SYSTEM_DATA *sys_data;
	UINT32 vkey;
	UINT32 times = 0, display_type = 0;
	date_time dt;
	UINT32 hh, mm, ss;
	char time_str[10];
	struct pan_key key_struct;
	UINT32 wakeup_time;

#ifdef  MULTI_CAS
#if (CAS_TYPE == CAS_CONAX)
    clean_mmi_msg(5,1);
#endif
#endif

	sys_data = sys_data_get();

	sys_data->bstandmode = 1;
	sys_data_save(1);

	system_state = SYS_STATE_POWER_OFF;
/*@jingjin for FrontPanel Driver*/	
#if (FRONT_PANEL_TYPE == FP_CT1642)
	key_pan_display("    ", 4);
#else
	if (mode != 1)
		key_pan_display("oFF ", 4);
#endif

	power_off_process();

	if (1)
	 /* Real Standby*/
	{
		UINT32 cur_time, target_time;

		key_pan_display_lock(0);

		get_local_time(&dt);
		// disable interrupt
#ifndef FRONT_PANEL_SINO
		pan_close(g_pan_dev);
#endif
		//power_key_auto_study();
		wakeup_time = api_get_recently_timer();
		osal_interrupt_disable();

		cur_time = ( dt.sec & 0x3F ) | ( (dt.min & 0x3F ) << 6 )  | ( ( dt.hour & 0x1F ) << 12 ) | ( ( dt.day & 0x1F ) << 17 )
		           | ( ( dt.month & 0xF ) << 22 ) | ( ( dt.year % 100 ) << 26 );

#if 0 //loader will close the power
#if (SYS_MAIN_BOARD == BOARD_S3202C_DEMO_01V01)
		//disable GPIO[1] power	
		*((volatile UINT32*)0xb8000100) |= (1 << 12);	//pinmux spdif gpio[1]
		*((volatile UINT32*)0xb8000040) |= (1<<0);	// gpio enable
		*((volatile UINT32*)0xb8000058) |= (1<<0);	// dir: output enable
		*((volatile UINT32*)0xb8000054)  &= ~(1<<0);	// output 0
#elif (SYS_MAIN_BOARD == BOARD_S3202C_NEWLAND_02V01||SYS_MAIN_BOARD == BOARD_S3202C_DEMO_02V01)
		//disable GPIO[32] power	
		*((volatile UINT32*)0xb80000A0) |= (1<<0);	//enable gpio 32
		*((volatile UINT32*)0xb80000B8) |= (1<<0);	// set gpio 32 to be output
		*((volatile UINT32*)0xb80000B4)  &= ~(1<<0);	// set gpio 32 output high level
#endif
#endif




#ifndef FRONT_PANEL_SINO
		sys_ic_enter_standby(wakeup_time-60, cur_time);
#else
		UINT32 keyvalue_ir_wakeup, keyvalue_panel_wakeup;
		UINT32 standby_param[6] = {0};
		keyvalue_panel_wakeup = 0x0001;
		keyvalue_ir_wakeup = 0x8f70df60;


				
		//build up the standby param
		standby_param[0] = dt.day;
		standby_param[0]<<=8;
		standby_param[0] |= dt.hour;
		standby_param[0]<<=8;
		standby_param[0] |= dt.min;
		standby_param[0]<<=8;
		standby_param[0] |= dt.sec;
		standby_param[1] = dt.year;
		standby_param[1]<<=8;
		standby_param[1] |= dt.month;

		
		standby_param[2] = (wakeup_time>>17) & 0x1f; //day
		standby_param[2]<<=8;
		standby_param[2] |= ((wakeup_time>>12) & 0x1f); //hour
		standby_param[2]<<=8;
		standby_param[2] |= (wakeup_time>>6) & 0x3f; //min
		standby_param[2]<<=8;
		standby_param[2] |= wakeup_time & 0x3f;//sec
		standby_param[3] = wakeup_time>>26 + 2000; //year
		standby_param[3]<<=8;
		standby_param[3] |= (wakeup_time>>22) & 0xf; //month

		standby_param[4] = keyvalue_panel_wakeup;
		standby_param[5] = keyvalue_ir_wakeup;
				
		if(pan_io_control(g_pan_dev, PAN_DRIVER_STANDBY, standby_param))
			libc_printf("pan_io_control failed!\n");
		libc_printf("enter PAN_STANDBY!\n");
#endif	
		// enable interrupt
		osal_interrupt_enable();
	}
	else
	{
		key_pan_display_standby(1);
		key_pan_display_lock(0);
	}
#ifndef FRONT_PANEL_SINO
	while (1)
	{
		//sys_ic_enter_standby();

		vkey = V_KEY_NULL;

		if (key_get_key(&key_struct, 0))
		{
			//32bit key info: bit31-28 pan_key_type; 27-24 key_status; bit23-16 key_cnt; bit15-0 key_value
			keycode = (key_struct.type << 28) | (key_struct.state << 24) | (key_struct.count << 16) | (key_struct.code &0xFFFF);
			ap_hk_to_vk(0, keycode, &vkey);
		}
		else
			keycode = PAN_KEY_INVALID;

		if (vkey == V_KEY_POWER)
		{
			power_on_process();
		}
		else
		{
			times++;
			times = times % 100;
			osal_delay(5000);

			if (times == 0)
			{
				//get_cur_time(&hh,&mm,&ss);
				get_local_time(&dt);
				hh = dt.hour;
				mm = dt.min;

				if (display_type == 0)
					sprintf(time_str, "%02d%02d ", hh, mm);
				else
					sprintf(time_str, "%02d.%02d", hh, mm);

				key_pan_display(time_str, 5);
				display_type++;
				display_type %= 2;
			}
		}
	}
#endif		
}
