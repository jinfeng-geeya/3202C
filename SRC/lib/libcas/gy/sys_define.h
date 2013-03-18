/***************************************************************************
  * Description
  *   This file defined all system MACROs, include system configuration,
  *   compiler configuration and system return code.
  *
  * Remarks
  *   . Those MACRO define "value" but "function".
  *   . Configuration MACRO only used to config source code that need fixed
  *   after compiler. If your system need dynamic configurable, please don't
  *   use system MACROs.
  *   . All configuration MACROs named as: SYS_XXXXXXX.
  *
  * Bugs
  *   None.
  *
  * TODO
  *     * To add more system configuration MACROs.
  *     * To add more system system return MACROs.
  * History
  *   <table>
  *   \Author     Date         Change Description
  *   ----------  -----------  -------------------
  *   Justin Wu   2006.10.17   Initialize.
  *   </table>
  *
  ***************************************************************************/
#ifndef __SYS_DEFINE_H__
#define __SYS_DEFINE_H__

/****************************************************************************
 * System ID coding, used for component ID management.
 ****************************************************************************/
#define SYS_DEFINE_NULL		0			/* NULL sys define */

/* System type and ID field mask */
#define SYS_ID_MASK_LAYR	0xFF00		/* ID layer field mask */
#define SYS_ID_MASK_TYPE	0x00FF		/* ID type  field mask */

/* System layer definition */
#define SYS_LAYER_HW		0x01		/* Layer ID for hardware */
#define SYS_LAYER_SW		0x02		/* Layer ID for software */
#define SYS_LAYER_ENG		0x03		/* Layer ID for engine */
#define SYS_LAYER_DEV		0x04		/* Layer ID for device driver */
#define SYS_LAYER_BUS		0x05		/* Layer ID for bus driver */

/* System type definition */
#define SYS_HW_TYPE_SOC		((SYS_LAYER_HW  << 8) + 0x01)	/* Chip */
#define SYS_HW_TYPE_CPU		((SYS_LAYER_HW  << 8) + 0x02)	/* CPU */
#define SYS_HW_TYPE_BOARD	((SYS_LAYER_HW  << 8) + 0x03)	/* Board type */

#define SYS_SW_TYPE_PRJ		((SYS_LAYER_SW  << 8) + 0x01)	/* Project */
#define SYS_SW_TYPE_OS		((SYS_LAYER_SW  << 8) + 0x02)	/* RTOS */

#define SYS_ENG_TYPE_NV		((SYS_LAYER_ENG << 8) + 0x01)	/* Navigator engine */

#define SYS_DEV_TYPE_ADEC	((SYS_LAYER_DEV << 8) + 0x01)	/* Audio decoder device */
#define SYS_DEV_TYPE_AENC	((SYS_LAYER_DEV << 8) + 0x02)	/* Audio encoder device */
#define SYS_DEV_TYPE_APRC	((SYS_LAYER_DEV << 8) + 0x03)	/* Audio process device */
#define SYS_DEV_TYPE_ACAP	((SYS_LAYER_DEV << 8) + 0x04)	/* Audio capture device */
#define SYS_DEV_TYPE_VDEC	((SYS_LAYER_DEV << 8) + 0x05)	/* Video decoder device */
#define SYS_DEV_TYPE_VENC	((SYS_LAYER_DEV << 8) + 0x06)	/* Video encoder device */
#define SYS_DEV_TYPE_VPRC	((SYS_LAYER_DEV << 8) + 0x07)	/* Video process device */
#define SYS_DEV_TYPE_VCAP	((SYS_LAYER_DEV << 8) + 0x08)	/* Video capture device */
#define SYS_DEV_TYPE_DMX	((SYS_LAYER_DEV << 8) + 0x09)	/* Stream de-multiplexer */
#define SYS_DEV_TYPE_RMX	((SYS_LAYER_DEV << 8) + 0x0A)	/* Stream re-multiplexer */
#define SYS_DEV_TYPE_GPE	((SYS_LAYER_DEV << 8) + 0x0B)	/* Graphics process engine */
#define SYS_DEV_TYPE_NET	((SYS_LAYER_DEV << 8) + 0x0C)	/* Network device */
#define SYS_DEV_TYPE_NIM	((SYS_LAYER_DEV << 8) + 0x0D)	/* Broadcasting NIM device */
#define SYS_DEV_TYPE_UIO	((SYS_LAYER_DEV << 8) + 0x0E)	/* User input/output device */
#define SYS_DEV_TYPE_STO	((SYS_LAYER_DEV << 8) + 0x0F)	/* Character storage device */
#define SYS_DEV_TYPE_MST	((SYS_LAYER_DEV << 8) + 0x10)	/* Block storage device */
#define SYS_DEV_TYPE_RFM	((SYS_LAYER_DEV << 8) + 0x11)	/* TV RF modulator device */
#define SYS_DEV_TYPE_SMC	((SYS_LAYER_DEV << 8) + 0x12)	/* smart card device */
#define SYS_DEV_TYPE_TTX	((SYS_LAYER_DEV << 8) + 0x13)	/* Teletext decoder device */
#define SYS_DEV_TYPE_SUB	((SYS_LAYER_DEV << 8) + 0x14)	/* Subtitle decoder device */
#define SYS_DEV_TYPE_CIC	((SYS_LAYER_DEV << 8) + 0x15)	/* CI controller device */
#define SYS_DEV_TYPE_DVP	((SYS_LAYER_DEV << 8) + 0x16)	/* ATV dvp device */
#define SYS_DEV_TYPE_ATVDEC	((SYS_LAYER_DEV << 8) + 0x17)	/* ATV dec device */
#define SYS_DEV_TYPE_AUDIO	((SYS_LAYER_DEV << 8) + 0x18)	/* ATV audio device */
#define SYS_DEV_TYPE_ATVMCU	((SYS_LAYER_DEV << 8) + 0x19)	/* ATV mcu device */
#define SYS_DEV_TYPE_HDMI	((SYS_LAYER_DEV << 8) + 0x1A)	/* ATV HDMI device */
#define SYS_DEV_TYPE_USB	((SYS_LAYER_DEV << 8) + 0x1B)	/* USB device */
#define SYS_DEV_TYPE_PUSB_MASS	((SYS_LAYER_DEV << 8) + 0x1C)	/* USB mass storage */
#define SYS_DEV_TYPE_PUSB_PUSH	((SYS_LAYER_DEV << 8) + 0x1D)	/* USB device(push data to PC) */

#define SYS_BUS_TYPE_SCI	((SYS_LAYER_BUS << 8) + 0x01)	/* Serial interface */
#define SYS_BUS_TYPE_I2C	((SYS_LAYER_BUS << 8) + 0x02)	/* I2C interface */
#define SYS_BUS_TYPE_IDE	((SYS_LAYER_BUS << 8) + 0x03)	/* ATA interface */
#define SYS_BUS_TYPE_LOC	((SYS_LAYER_BUS << 8) + 0x04)	/* Local bus interface */
#define SYS_BUS_TYPE_HCI	((SYS_LAYER_BUS << 8) + 0x05)	/* OHCI interface */
#define SYS_BUS_TYPE_USB	((SYS_LAYER_BUS << 8) + 0x06)	/* USB interface */
#define SYS_BUS_TYPE_SPI	((SYS_LAYER_BUS << 8) + 0x07)	/* SPI interface */
#define SYS_BUS_TYPE_CARD	((SYS_LAYER_BUS << 8) + 0x08)	/* Card interface */
#define SYS_BUS_TYPE_PLATFORM	((SYS_LAYER_BUS << 8) + 0x09)/* Platform bus*/
#define SYS_BUS_TYPE_SMI	((SYS_LAYER_BUS << 8) + 0x0A)	/* SMI interface */

/****************************************************************************
 * Section for SW configuration.
 ****************************************************************************/
#define SYS_OS_NOS			((SYS_SW_TYPE_OS << 16) + 1)	/* None OS */
#define SYS_OS_NUCLEUS		((SYS_SW_TYPE_OS << 16) + 2)	/* Nucleus */
#define SYS_OS_NUCLEUS2K	((SYS_SW_TYPE_OS << 16) + 3)	/* Nucleus 2K */
#define SYS_OS_MAGUS		((SYS_SW_TYPE_OS << 16) + 4)	/* Magus */
#define SYS_OS_MODULE		SYS_DEFINE_NULL		/* Default OS module  */

/****************************************************************************
 * Section for HW configuration.
 ****************************************************************************/
#define SYS_CHIP_SIM900		((SYS_HW_TYPE_SOC << 16) + 0x00)/* HTV900 Simulator */
#define SYS_CHIP_HTV900FP	((SYS_HW_TYPE_SOC << 16) + 0x01)/* HTV900 FPGA */
#define SYS_CHIP_HTV900		((SYS_HW_TYPE_SOC << 16) + 0x02)/* HTV900 */

#define SYS_CHIP_BUCKSSIM	((SYS_HW_TYPE_SOC << 16) + 0x04)/* Bucks Simulator */
#define SYS_CHIP_BUCKSFP	((SYS_HW_TYPE_SOC << 16) + 0x05)/* Bucks FPGA */
#define SYS_CHIP_BUCKS		((SYS_HW_TYPE_SOC << 16) + 0x06)/* Bucks */

#define SYS_CHIP_PACERSSIM	((SYS_HW_TYPE_SOC << 16) + 0x08)/* Pacers Simulator */
#define SYS_CHIP_PACERSFP	((SYS_HW_TYPE_SOC << 16) + 0x09)/* Pacers FPGA */
#define SYS_CHIP_PACERS		((SYS_HW_TYPE_SOC << 16) + 0x0A)/* Pacers */

#define SYS_CHIP_LAKERSSIM	((SYS_HW_TYPE_SOC << 16) + 0x0C)/* Lakers Simulator */
#define SYS_CHIP_LAKERSFP	((SYS_HW_TYPE_SOC << 16) + 0x0D)/* Lakers FPGA */
#define SYS_CHIP_LAKERS		((SYS_HW_TYPE_SOC << 16) + 0x0E)/* Lakers */

#define SYS_CHIP_PACERS2SIM	((SYS_HW_TYPE_SOC << 16) + 0x0F)/* Pacers2 Simulator */
#define SYS_CHIP_PACERS2FP	((SYS_HW_TYPE_SOC << 16) + 0x10)/* Pacers2 FPGA */
#define SYS_CHIP_PACERS2	((SYS_HW_TYPE_SOC << 16) + 0x11)/* Pacers2 */

#define SYS_CHIP_H2SIM		((SYS_HW_TYPE_SOC << 16) + 0x12)/* H2 Simulator */
#define SYS_CHIP_H2FP		((SYS_HW_TYPE_SOC << 16) + 0x13)/* H2 FPGA */
#define SYS_CHIP_H2			((SYS_HW_TYPE_SOC << 16) + 0x14)/* H2 */

#define SYS_CHIP_MODULE		SYS_DEFINE_NULL		/* Default chip module */

#define SYS_CHIP_CLOCK		SYS_DEFINE_NULL		/* Default chip working clock */

#define SYS_CPU_X86			((SYS_HW_TYPE_CPU << 16) + 1)	/* X86 for i386 system */
#define SYS_CPU_MIPS		((SYS_HW_TYPE_CPU << 16) + 2)	/* MIPS */
#define SYS_CPU_MODULE		SYS_DEFINE_NULL		/* Default chip module */

#define SYS_CPU_CLOCK		SYS_DEFINE_NULL		/* Default CPU working clock */

#define SYS_DRAM_CLOCK		SYS_DEFINE_NULL		/* Default DRAM working clock */
#define SYS_DRAM_SIZE		SYS_DEFINE_NULL		/* Default DRAM size in bytes */

/****************************************************************************
 * Section for compiler and linker configuration.
 ****************************************************************************/
/* Compiler and language related definition */
#define __SYS_ENDIAN_LE__						/* Default defined to little endian */

#ifdef _ARC
#define __SYS_ENV_ARC__							/* Defined runtime environment is ARC */
#ifndef __cplusplus
#define inline				_Inline				/* Key word "inline" in ARC CC */
#endif
#if (defined _BE || defined _ARC_BE)
#undef  __SYS_ENDIAN_LE__
#define __SYS_ENDIAN_BE__						/* Defined if current is big endian */
#else
#undef  __SYS_ENDIAN_BE__
#define __SYS_ENDIAN_LE__						/* Defined if current little endian */
#endif
#endif

#ifdef _WIN32
#ifndef __GNUC__
#define inline				__inline			/* Key word "inline" in MSVC */
#define __attribute__(x)  /*NOTHING*/
#define __inline__
#endif
#define __SYS_ENV_WIN32__						/* Defined runtime environment is WIN32 */
#undef  __SYS_ENDIAN_BE__
#define __SYS_ENDIAN_LE__						/* Defined if current little endian */
#endif

/* Memory mapping option */
#define SYS_MAP_BOOT_CMD	SYS_DEFINE_NULL		/* Default system boot command address */
#define SYS_MAP_BOOT_DAT	SYS_DEFINE_NULL		/* Default system boot data address */

#define SYS_MAP_APP_ENTRY	SYS_DEFINE_NULL		/* Default system application entry address */
#define SYS_MAP_BTL_ENTRY	SYS_DEFINE_NULL		/* Default boot loader entry address */

#define SYS_MAP_HEAP_TOP	SYS_DEFINE_NULL		/* Default system heap top address */
#define SYS_MAP_HEAP_BTM	SYS_DEFINE_NULL		/* Default system heap buttom address */

#include <sys_config.h>

#endif	/* __SYS_DEFINE_H__ */

