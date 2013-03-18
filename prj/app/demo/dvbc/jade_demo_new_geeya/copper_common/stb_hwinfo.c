#include <sys_config.h>
#include <stdarg.h>
#include <stddef.h>
#include <osal/osal.h>
#include <api/libc/string.h>
#include <bus/sci/sci.h>
#include <types.h>
#include <retcode.h>
#include <api/libc/printf.h>
#include <hld/hld_dev.h>
#include <hld/sto/sto_dev.h>
//#include "com_api.h"
#include "stb_hwinfo.h"
//s#include <api/libstbinfo/stb_info_data.h>
#include "../key.h"
#include <../src/lib/libapplet/libbootupg3/lib_ash.h>
#include <../src/lib/libapplet/libbootupg3/errorcode.h>
#include <../src/lib/libapplet/libbootupg3/packet.h>

STB_HWINFO *g_stb_hwinfo=NULL;

static INT8 stb_hwinfo_load(STB_HWINFO *hwinfo)
{
	INT8 ret = 0, i;
	UINT32 check_data;
	UINT8  data[STB_HWINFO_LEN];
	BOOL old2new = FALSE;
		
	struct sto_device *sto = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
	if(NULL == sto)
	{	
		ret = -1;
		goto RETURN;
	}
	
	MEMSET(data,0,STB_HWINFO_LEN);
	sto_get_data(sto, data, STB_HWINFO_BASE_ADDR, sizeof(STB_HWINFO));
#if 0	
	check_data = ((data[0]<<24) |(data[1]<<16) |(data[2]<<8) |(data[3]));
	if(check_data == 0xffffffff)
	{	
		ret = -1;
		goto RETURN;
	}
#endif
	MEMCPY(hwinfo, data, sizeof(STB_HWINFO));

	//libc_printf("load oui = %d, hwmode = %d, hwversion = %d\n", hwinfo->stb_oui, hwinfo->stb_hw_model, hwinfo->stb_hw_version);
	
RETURN:	
	return ret;

}

STB_HWINFO *stb_hwinfo_get()
{
	if(g_stb_hwinfo==NULL)
	{
		g_stb_hwinfo = MALLOC(sizeof(STB_HWINFO));
		
		if(stb_hwinfo_load(g_stb_hwinfo) == -1)
			return NULL;
	}

	return g_stb_hwinfo;
}

INT8 get_stb_hwinfo(UINT32 *oui, UINT32 *hw_model, UINT32 *hw_version)
{
	INT8 ret = 0;
	STB_HWINFO *hwinfo;

	if(oui==NULL || hw_model==NULL || hw_version==NULL)
		return -1;
	
	hwinfo = stb_hwinfo_get();
	if(hwinfo == NULL)
		return -1;

	*oui = hwinfo->stb_oui;
	*hw_model = hwinfo->stb_hw_model;
	*hw_version = hwinfo->stb_hw_version;

	return ret;
}

INT8 get_stb_serial(UINT8* serial)
{
	INT8 ret = 0;
	STB_HWINFO *hwinfo;

	if(serial == NULL)
		return -1;
	
	hwinfo = stb_hwinfo_get();
	if(hwinfo == NULL)
		return -1;

	MEMCPY(serial, hwinfo->stb_serial, STB_HWINFO_SN_LEN);
	return ret;
}

INT8 get_stb_mac(UINT8* mac)
{
	INT8 ret = 0;
	STB_HWINFO *hwinfo;

	if(mac == NULL)
		return -1;
	
	hwinfo = stb_hwinfo_get();
	if(hwinfo == NULL)
		return -1;

	MEMCPY(mac, hwinfo->stb_mac, STB_HWINFO_MAC_LEN);
	return ret;
}

#ifdef BURN_SERIAL_NUM
extern struct sto_device *g_sto_dev;
extern struct pan_device  *g_pan_dev;

#define SERIAL_DEBUG(...) //libc_printf 
#define COMMAND_STATUS_DONE 255

static UINT32 g_com_detect_flag=COM_MONITOR_CHECK_PAUSE;
static ID g_com_ash_id=OSAL_INVALID_ID;

UINT32 cmd_process_monitor(unsigned int argc, unsigned char *argv[])
{
	UINT32 data[6 * 2];
	UINT32 i, j = 0x00000003;
	UINT32 ptm = osal_get_tick();

	MEMSET(data, 0, sizeof(data));
	pan_display(g_pan_dev, "Pdon", 4); // done, all ok!

	for(i = 0; i<6; i++)
	{
		UINT32 pm_finish = 0;
		UINT32 pm2_finish = 0;
		*((volatile UINT32 *)0xb8000300) = (j|i<<16);
		while(1)
		{
			pm_finish = *((volatile UINT32 *)0xb8000304);
			pm2_finish = *((volatile UINT32 *)0xb800030c);
			if(1==(pm_finish&0x1)&&1==(pm2_finish&0x1))
				break;
//			if((osal_get_tick()-ptm) > 2000)
				break;
			osal_task_sleep(1);
		}
		data[2  * i] = pm_finish>>16;
		data[2  * i + 1] = pm2_finish>>16;
		//libc_printf("PM_COUNT:  \t=\t 0x%04x,  \tPM_SEL =  \t%d\n", pm_finish>>16, i);
		//libc_printf("PM_COUNT2: \t=\t 0x%04x,  \tPM_SEL2 =  \t%d\n", pm2_finish>>16, i);
		*((volatile UINT32 *)0xb8000300) = 0;
	}
	
	SendDataPacket(0, (unsigned char *)data, sizeof(data));	
	return 0;
}

void api_set_com_check_flag(UINT32 upgrade_flag)
{
	g_com_detect_flag = upgrade_flag;
}

UINT32 cmd_stbid(unsigned int argc, unsigned char *argv[])
{
	UINT8 i = 0, j = 0, ch = 0xff;
	UINT8 *_stbid_flag = "SRI";
	INT32 _stbid_flag_len = 3;
	UINT8 _serial_data[1024]; // max receive 1KB data
	UINT32 timeout = 1000, stbid_offset = STB_HWINFO_SERIAL_OFF, _stbid_crc = 0, _stbid_len = 0, _serial_len = 0, _crc = 0, _crc_pos = 0;
	UINT8 *buffer = NULL;
	UINT32 nReturn = SUCCESS;
	UINT32 nPacketNum = 0;
	PACKET packet;
	UINT32 tick = osal_get_tick();
	UINT8 retry_num = 5, _tr_num = 5;
	ID _task_id = g_com_ash_id;
	
	osal_task_dispatch_off();	
	
	SendStatusPacket(COMMAND_STATUS_OK,  0);
	pan_display(g_pan_dev, "Stb-", 4);
	
RETRY:	
	_tr_num = 5;
	
	//transfer data
	MEMSET(&packet, 0, sizeof(PACKET));
	
	nReturn = packet_receive(&packet, 5 * 1000);
	if(SUCCESS != nReturn)
	{
		SERIAL_DEBUG("receive packet fail!\n");	
		retry_num--;
		goto RETURN;
	}
	
	if(packet.packet_type == PACKET_DATA)
	{	
		_serial_len = packet.packet_length-4;
		MEMCPY(_serial_data, packet.data_buffer+4, packet.packet_length-4);
	}
	else
	{
		SERIAL_DEBUG("receive %d packet, ignore!\n", packet.packet_type);
		retry_num--;
		goto RETURN;
	}
	
	SERIAL_DEBUG("stbid get data total len %d finish, data: \n", _serial_len);
	for(i=0; i<_serial_len; i++)
	{
		SERIAL_DEBUG("%c", _serial_data[i]);
	}
	SERIAL_DEBUG("\n");
	
	pan_display(g_pan_dev, "GET", 4);
	if((_serial_data[0] != _stbid_flag[0]) || (_serial_data[1] != _stbid_flag[1]) || (_serial_data[2] != _stbid_flag[2])) // received flag tag
	{
		SERIAL_DEBUG("Error: SRI flag missing!\n");	
		retry_num--;
		goto RETURN;
	}
	
	pan_display(g_pan_dev, "FLAG", 4);
	_stbid_len = _serial_len-4-8;
	if(_stbid_len > STB_HWINFO_MAC_OFF)
	{
		SERIAL_DEBUG("Error: stbid len %d != [%d], please resend cmd!\n", _stbid_len, STB_HWINFO_MAC_OFF);
		retry_num--;
		goto RETURN;
	}
	
	pan_display(g_pan_dev, "LENG", 4);
	
	// do crc check
	_crc_pos = _stbid_flag_len+1+_stbid_len;
	_stbid_crc = 0;
	for(i=0; i<8; i++)
	{
		_stbid_crc |= (((_serial_data[_crc_pos+i]>'9') ? (_serial_data[_crc_pos+i]-'A'+10) : (_serial_data[_crc_pos+i]-'0'))<<((7-i)*4));
	}
	
	_crc = MG_Table_Driven_CRC(0xFFFFFFFF, _serial_data, _crc_pos);
	if(_stbid_crc != _crc)
	{
		// fail, need re-trans
		SERIAL_DEBUG("stbid crc fail, calcu = 0x%x!\n", _crc);
		retry_num--;
		goto RETURN;
	}
	
	pan_display(g_pan_dev, "CRC", 4);
	// burn code, enable drive auto-erase
	for(i=0; i<(STB_HWINFO_OUI_OFF-STB_HWINFO_MAC_OFF); i++) // init mac
	{
		ch = _serial_data[STB_HWINFO_MAC_OFF+4-12+i*2];
		_serial_data[i+STB_HWINFO_MAC_OFF+4] = (((ch>'9') ? ((ch>='a') ? (ch-'a'+10) : (ch-'A'+10)) : (ch-'0'))<<4);
		ch = _serial_data[STB_HWINFO_MAC_OFF+4-12+i*2+1];
		_serial_data[i+STB_HWINFO_MAC_OFF+4] |= ((ch>'9') ? ((ch>='a') ? (ch-'a'+10) : (ch-'A'+10)) : (ch-'0'));
	}
	
	buffer = MALLOC(64*1024);
	if(buffer == NULL)
	{
		SDBBP();
	}
	
	sto_io_control(g_sto_dev, STO_DRIVER_SECTOR_BUFFER, (UINT32)buffer);
	sto_io_control(g_sto_dev, STO_DRIVER_SET_FLAG, STO_FLAG_AUTO_ERASE|STO_FLAG_SAVE_REST);
	
	SERIAL_DEBUG("Now burn stbid: ");	
	for(i=0; i<STB_HWINFO_OUI_OFF; i++)
	{
		SERIAL_DEBUG("%c", _serial_data[i+_stbid_flag_len+1]);
	}
	SERIAL_DEBUG("\n");
	
	sto_put_data(g_sto_dev, STB_HWINFO_BASE_ADDR, &_serial_data[_stbid_flag_len+1], STB_HWINFO_OUI_OFF);	
	if(buffer)
	{
		FREE(buffer);
		buffer = NULL;
		sto_io_control(g_sto_dev, STO_DRIVER_SECTOR_BUFFER, 0);
		sto_io_control(g_sto_dev, STO_DRIVER_SET_FLAG, 0);
	}
	
	if(g_stb_hwinfo != NULL)
	{
		FREE(g_stb_hwinfo);
		g_stb_hwinfo = NULL;
	}
	
	pan_display(g_pan_dev, "-tr-", 4);
	SERIAL_DEBUG("stbid finish, task %d deleted!\n", g_com_ash_id);
	SERIAL_DEBUG("cmd_stbid takes %dms\n", osal_get_tick()-tick);
	retry_num = 0;

RESEND:	
	SendStatusPacket(COMMAND_STATUS_DONE, 0);
	MEMSET(&packet, 0, sizeof(PACKET));
	osal_task_sleep(100);
	
	nReturn = packet_receive(&packet, 5 * 1000);
	if((SUCCESS != nReturn) || (packet.packet_type != PACKET_STATUS))
	{
		if(_tr_num-- > 0)
		{
			SERIAL_DEBUG("stbid finish, but signal send fail, now re-send!\n");
			goto RESEND;
		}
		else
		{
			pan_display(g_pan_dev, "dStb", 4); // done, but notice fail!
		}
	}
	else
	{
		pan_display(g_pan_dev, "-Stb", 4); // done, all ok!
	}
	
RETURN:
	if(retry_num >0)
	{
		SendStatusPacket(COMMAND_STATUS_ERROR, 0);
		goto RETRY;
	}
	else
	{
		SERIAL_DEBUG("error, please redo!\n");
		api_set_com_check_flag(COM_MONITOR_CHECK_STBID);
	}
	
	g_com_ash_id = INVALID_ID;
	osal_task_dispatch_on();
	osal_task_delete(_task_id);
}



UINT32 api_com_monitor()
{
	UINT8 i = 0;
	UINT8 ch = 0xff;
	UINT8 *comtest_command = "comtest";
	INT32 comtest_command_len = STRLEN(comtest_command) - 1;
	UINT8 sync = 0;
	UINT32 timeout = 1000;
	OSAL_T_CTSK t_ctsk;
	
	if (COM_MONITOR_CHECK_STBID &g_com_detect_flag)
	{
		i = 0;
		while(sci_read_tm(SCI_FOR_RS232, &ch, timeout) == SUCCESS)
		{
			if((i <= comtest_command_len) && (comtest_command[i] == ch))
			{
				timeout = 1000 * 100;
				sci_write(SCI_FOR_RS232, ch);
				i++;
				if (i > comtest_command_len) /* Is a comtest command string, wait for serial id trans */
				{					
					t_ctsk.stksz = 0x2000;
					t_ctsk.quantum = 10;
					t_ctsk.itskpri = OSAL_PRI_NORMAL;
					t_ctsk.name[0] = 'A';
					t_ctsk.name[1] = 'S';
					t_ctsk.name[2] = 'H';
					t_ctsk.task = (FP)lib_ash_shell;
					if(g_com_ash_id != OSAL_INVALID_ID)
					{
						osal_task_delete(g_com_ash_id);					
					}
					g_com_ash_id = osal_task_create(&t_ctsk);
					if (g_com_ash_id == OSAL_INVALID_ID)
						return 1;
					
					api_set_com_check_flag(COM_MONITOR_CHECK_PAUSE);
					break;
				}
			}
			else
				break;
		}
	}

	return 0;
}
#endif

