/*
	Description: This file is used to define the parameters' position in binary code, the values in which is referenced both by bootloader and main code.
	History:
	2005-6-10	xionghua		Create

*/
#ifndef _SYS_PARAMETERS_M3327_
#define _SYS_PARAMETERS_M3327_


/* Chunk ID */
#define BOOT_ID_2           	0xE3000010
#define CHUNKID_SECLOADER       0x00FF0000
#define CHUNKID_SECLOADER_MASK  0xFFFF0000
#define CHUNKID_MAINCODE        0x01FE0000
#define CHUNKID_MAINCODE_MASK   0xFFFF0000
#define CHUNKID_SEECODE         0x06F90101
#define CHUNKID_SEECODE_MASK    0xFFFF0000
#define CHUNKID_SECSEECODE       0x07F80000
#define CHUNKID_SECSEECODE_MASK  0xFFFF0000
#define CHUNKID_KEY         0x09F60101
#define CHUNKID_KEY_MASK    0xFFFF0000
#define CHUNKID_GZ              0x00000000
#define CHUNKID_7Z              0x00000001

/* Used to define flash speed, length is 1 byte
 * size:					bit 7~6		0:512K / 1: 1M / 2: 2M / 3: 4M
 * speed:					bit 5~0
 */
#define HW_SET_FLASHSPEED		0x40
#define FLASH_SIZE_MASK			0xc0
#define FLASH_SPEED_MASK		0x3f

#define HW_FLASH_DEFSPEED		7

/* Used to define bootloader, length 2 bytes in big endian:
   onoff:					bit 15		1:on / 0:off
   tv format:				bit 12~14	0:PAL / 1:NTSC / 2:PAL_M / 3:PAL_N / 4:PAL_60 / 5:NTSC_443
   source picture height:	bit 0~11
   For example : if open logo in bootload, and display format is PAL, and source picture is 480, then it should be set to 0x81e0.
*/
#define HW_SET_BOOTLOGO			0x41
#define LOGO_SWITCH_MASK		0x8000
#define LOGO_FORMAT_MASK		0x7000
#define LOGO_FORMAT_PAL			0x0000
#define LOGO_FORMAT_NTSC		0x1000
#define LOGO_FORMAT_PAL_M		0x2000
#define LOGO_FORMAT_PAL_N		0x3000
#define LOGO_FORMAT_PAL_60		0x4000
#define LOGO_FORMAT_NTSC_443	0x5000
#define LOGO_HEIGHT_MASK		0x0fff

/* Used to define bootup type, length 1 byte:
 * cold_bootup_to_standby:	bit 7		1:yes / 0: no
 * reserved:				bit 6~bit0
 */
#define HW_SET_BOOTTYPE			0x43
#define BOOT_TYPE_MASK			0x80

/* Used to define STB S/N or ID */
#define HW_SET_STBID			0x60

#endif
