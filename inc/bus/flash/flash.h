#ifndef FLASH_H
#define FLASH_H

#include <types.h>
#include <sys_config.h>
#include <retcode.h>


#define FLASH_PTR2OFF(p)        ((unsigned long)p - SYS_FLASH_BASE_ADDR)
#define FLASH_OFF2PTR(o)        ((unsigned char *)o + SYS_FLASH_BASE_ADDR)

#define FLASHTYPE_UNKNOWN       0
#define FLASHTYPE_400B          1
#define FLASHTYPE_400T          2
#define FLASHTYPE_800B          3
#define FLASHTYPE_800T          4
#define FLASHTYPE_160B          5
#define FLASHTYPE_160T          6
#define FLASHTYPE_040           7
#define FLASHTYPE_802           8
#define FLASHTYPE_802T          9
#define FLASHTYPE_39040         10
#define FLASHTYPE_39080         11
#define FLASHTYPE_39016         12
#define FLASHTYPE_39088         13
#define FLASHTYPE_39168         14
#define FLASHTYPE_K8D1716B      15
#define FLASHTYPE_K8D1716T      16
#define FLASHTYPE_S29AL032D_00  17
#define FLASHTYPE_S29AL032D_03  18
#define FLASHTYPE_S29AL032D_04  19

#define FLASHTYPE_SF_SST25VF016B 0x81

struct flash_chip_info
{
	UINT32 base_addr;
	UINT8  flash_id;
	UINT8  flash_io;
	UINT16 flash_sectors;
	UINT32 flash_size;
};
struct flash_info_st
{
	const UINT8  *flash_deviceid;
	const UINT8  *flash_id_p;
	const UINT8  *flash_io_p;
	UINT16 flash_deviceid_num;
	UINT8 flash_type;  //0: serial flash; 1: parallel flash;
	UINT8 reserved;

	const UINT16  *flash_deviceid_ex;
	const UINT8  *flash_id_p_ex;
	const UINT8  *flash_io_p_ex;
	UINT16 flash_deviceid_num_ex;
	
	UINT8  flash_id;
	UINT8  flash_io;
	UINT16 flash_sectors;
	UINT32 flash_size;

	void (* get_id)(UINT32 *buf, UINT32 cmd_addr);
	int  (* erase_chip)(void);
	int  (* erase_sector)(UINT32 addr);
	int  (* verify)(UINT32 addr, UINT8 *data, UINT32 len);
	int  (* write)(UINT32 addr, UINT8 *data, UINT32 len);
	int  (* read )(void *des, void* src, UINT32 len);
	void (* set_io)(UINT8 io_num, UINT8 chip_idx);
} flash_info;

struct multi_flash_st
{
	UINT8 multi_chip;
	UINT8 chip_num;
	UINT16 total_sectors;
	UINT32 total_size;
	struct flash_chip_info multi_chip_info[4];
} multi_flash_info;

#define flash_get_id(result, cmdaddr)		flash_info.get_id(result, cmdaddr)
#define flash_erase_chip()					flash_info.erase_chip()
#define flash_erase_sector(sector_addr)		flash_info.erase_sector(sector_addr)
#define flash_verify(flash_addr, data, len)	flash_info.verify(flash_addr, data, len)
#define flash_copy(flash_addr, data, len)	flash_info.write(flash_addr, data, len)

unsigned int  flash_sector_size(unsigned int);
unsigned int  flash_sector_align(unsigned long);
unsigned long flash_sector_start(unsigned int);
unsigned int  flash_identify(void);


/* Follow are compact with old code, should be removed */
#define FLASH_BASE		SYS_FLASH_BASE_ADDR

#define flash_id_val						flash_info.flash_id
#define flash_size_val						flash_info.flash_size
#define flash_sectors_val					flash_info.flash_sectors

#define FLASH_FAST_READ_EN()	*((volatile UINT8 *)(0xb8000099)) = (0x4D)
#define FLASH_FAST_READ_DIS()	*((volatile UINT8 *)(0xb8000099)) = (0x0D)

/* Follow is used compatiable with old STO module, can be removed after shuttle in */
extern UINT8  flash_id;
extern unsigned int flash_size;
extern unsigned int flash_sectors;

#endif
