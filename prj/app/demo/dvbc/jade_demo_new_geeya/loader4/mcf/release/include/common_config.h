#ifndef _COMMON_CONFIG_H_
#define _COMMON_CONFIG_H_

/***********************************************************************
 OS object number maro define
*************************************************************************/
#define SYS_OS_MODULE			NO_OS//ALI_TDS2
#define SYS_OS_TASK_NUM		64
#define SYS_OS_SEMA_NUM		64
#define SYS_OS_FLAG_NUM			64
#define SYS_OS_MSBF_NUM		64
#define SYS_OS_MUTX_NUM         	64

/***********************************************************************
S.W module configration related marco define
*************************************************************************/
#define SYS_PROJECT				SYS_DEFINE_NULL
#define SYS_PROJECT_FE			PROJECT_FE_DVBC
#define SYS_MW_MODULE			SYS_DEFINE_NULL
#define SYS_CA_MODULE			SYS_DEFINE_NULL

/***********************************************************************
 System Hardware related marco define
*************************************************************************/
/*==========Main Hardware Feature==========*/
/*SOC Devices*/
#define SYS_CHIP_MODULE  ALI_M3202
#define SYS_CPU_MODULE  CPU_M6303
#define SYS_CPU_ENDIAN	ENDIAN_LITTLE
/*Clock*/
#define SYS_CHIP_CLOCK	27000000
#define SYS_CPU_CLOCK 	216000000
/*Memory*/
#define SYS_SDRAM_SIZE 32
/*Board*/
#define SYS_MAIN_BOARD			BOARD_S3202C_DEMO_02V01//BOARD_S3202C_DEMO_01V01
/*Flash*/
#define SYS_FLASH_BASE_ADDR 0xafc00000	// 0xaf000000
#define SYS_FLASH_SIZE      0x800000
#define ENABLE_SERIAL_FLASH
//#define STO_PROTECT_BY_MUTEX	//protocting flashw wirte/erase by mutex     

/*==========Peripheral Devices==========*/
#define SYS_GPIO_MODULE	M6303GPIO
#define SYS_SCI_MODULE	UART16550
#define SYS_TSI_MODULE	SYS_DEFINE_NULL
#define SYS_DMX_MODULE	SYS_DEFINE_NULL

/*IR*/
#define SYS_IRP_MOUDLE	ALI25C01

/*I2C*/
#define SYS_I2C_MODULE		M6303I2C
#define SYS_I2C_SDA				SYS_DEFINE_NULL	/* I2C SDA GPIO pin number */
#define SYS_I2C_SCL				SYS_DEFINE_NULL	/* I2C SDL GPIO pin number */

/*SSI TS IN/OUT */
//#define SSI_TS_OUT_IN_SET

/*RFM*/
#define SYS_RFM_MODULE			SYS_DEFINE_NULL /* RF modulator configuration */
#define SYS_RFM_BASE_ADDR		SYS_DEFINE_NULL /* RF modulator device address */

#define SYS_TUN_MODULE			SYS_DEFINE_NULL	/* Tuner configuration */
#define SYS_TUN_BASE_ADDR		SYS_DEFINE_NULL	/* Tuner device address */

#define SYS_DEM_MODULE			SYS_DEFINE_NULL	/* Demodulator configuration */
#define SYS_DEM_BASE_ADDR		SYS_DEFINE_NULL	/* Demodulator device address */

/*EEPROM*/
#define SYS_EEPROM_MODULE		SYS_DEFINE_NULL	/* EEPROM configuration */
#define SYS_EEPROM_BASE_ADDR	0xA0			/* EEPROM base address */
#define SYS_EEPROM_SIZE			1024			/* EEPROM size, in byte */

/***********************************************************************
Chunk ID marco define
*************************************************************************/
#define STB_CHUNK_BOOTLOADER_ID 	0xE3000010
#define LOGO_ID					0x02FD0100

/***********************************************************************
for stb_info_data structure
*************************************************************************/
// must be same between bootloader/upgloader/maincode!!
#define STB_MAX_PART_NUM 1
#define STB_PART_MAX_CHUNK_NUM 32
#define STB_MAX_CHUNK_NUM 32

/***********************************************************************
 AP feature support  maro define
*************************************************************************/
#define DB_VERSION					40

#define ENABLE_EROM
#define PANEL_DISPLAY
#define TRANSFER_FORMAT3_ONLY
#define SHOW_BOOT_LOGO

#define BOOT_DEBUG_ON	1
#define BOOT_DEBUG_OFF	0

#define BOOT_DEBUG BOOT_DEBUG_OFF

#if (BOOT_DEBUG==BOOT_DEBUG_ON)
#define STO_CHUNK_PRINTF libc_printf	// chunk_sto.c
#define CHUNK_PRINTF libc_printf	// chunk.c
#define BL_PRINTF libc_printf		// bootloader.c
#define STB_DATA_PRINTF libc_printf	// stb_info_data.c
#define STBINFO_PRINTF libc_printf	// stb_info.c
#endif

#endif	/* _SYS_CONFIG_H_ */
