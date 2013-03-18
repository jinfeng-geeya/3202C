/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2006 Copyright (C)
 *
 *  File: sto_flash.h
 *
 *  Description: Provide general flash driver head file for sto type device.
 *
 *  History:
 *      Date        Author      Version  Comment
 *      ====        ======      =======  =======
 *  1.  2006.4.xx Shipman Yuan  0.1.000  Initial
 *
 ****************************************************************************/
#ifndef _STO_FLASH_H_
#define _STO_FLASH_H_

#include <types.h>
#include <hld/sto/sto_dev.h>

//#define MUTEX_ENTER     osal_task_dispatch_off
//#define MUTEX_LEAVE     osal_task_dispatch_on

#define MS_TICKS       (sys_ic_get_cpu_clock()*1000000 / 2000)
#define RF_READ_TMO (50*1000)  // in unit of US

#define FLASH_ADDR_MASK     0xFFFFFF

struct flash_private
{
	const UINT8  *flash_deviceid;
	const UINT8  *flash_id_p;
	UINT16 flash_deviceid_num;

	UINT8  flash_id;
	UINT16 flash_sectors;
	UINT32 flash_size;
	//UINT32 flash_cmdaddr;

	int  (* io_ctrl)(UINT32 cmd, UINT32 val);
	int  (* open)( void *arg);  // save baudrate
	int  (* close)( void *arg); // switch baudrate back to 115200

	void (* get_id)( UINT32 *buf, UINT32 cmd_addr );
	int  (* erase_chip)( void );
	int  (* erase_sector)(UINT32 addr);
	int  (* write)( UINT32 addr, UINT8 *data, UINT32 len );
	int  (* read )( void *des, void* src, UINT32 len);
	int  (* verify)(UINT32 addr, UINT8 *data, UINT32 len );
};

void sto_fp_init(struct sto_device *dev,INT32 (*init)(struct sto_flash_info *param));
unsigned int sto_pflash_identify(struct sto_device *dev, UINT32 mode);
unsigned int sto_sflash_identify(struct sto_device *dev, UINT32 mode);
unsigned int sto_flash_sector_align(struct sto_device *dev,\
				unsigned long addr);
unsigned long sto_flash_sector_size(struct sto_device *dev,\
				unsigned int sector);
unsigned long sto_flash_sector_start(struct sto_device *dev,\
				unsigned long sector_no);

extern BOOL m_EnableSoftProtection;
extern unsigned long unpro_addr_low;
extern unsigned long unpro_addr_up;
extern unsigned long remote_flash_read_tmo;

#endif  /*_STO_FLASH_H_*/

