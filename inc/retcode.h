/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2002 Copyright (C)
 *
 *  File: retcode.h
 *
 *  Description: This file define the return code for all application.
 *
 *  History:
 *      Date        Author      Version  Comment
 *      ====        ======      =======  =======
 *  1.  2002.11.12  Justin Wu   0.1.000  Initial
 *  2.  2002.11.21  Liu Lan     0.1.001  Add for OSD & Decoder
 *  3.  2003.02.19  Justin Wu   0.1.002  Add for I2C
 ****************************************************************************/

#ifndef	__RETCODE_H__
#define __RETCODE_H__

/* Return value of function calls */
#define SUCCESS         0       /* Success return */
#define ERR_NO_MEM      -1      /* Not enough memory error */
#define ERR_LONG_PACK   -2      /* Package too long */
#define ERR_RUNT_PACK   -3      /* Package too short */
#define ERR_TX_BUSY     -4      /* TX descriptor full */
#define ERR_DEV_ERROR   -5      /* Device work status error */
#define ERR_DEV_CLASH   -6      /* Device clash for same device in queue */
#define ERR_QUEUE_FULL  -7      /* Queue node count reached the max. val*/
#define ERR_NO_DEV      -8      /* Device not exist on PCI */
#define ERR_FAILURE		-9      /* Common error, operation not success */

#define ERR_PARA        -20     /* Parameter is invalid */
#define ERR_ID_FULL     -21     /* No more ID available */
#define ERR_ID_FREE     -22     /* Specified ID isn't allocated yet */

#define ERR_OFF_SCRN    -30     /* Block is out off the screen */
#define ERR_V_OVRLAP    -31     /* Block is overlaped in vertical */
#define ERR_BAD_CLR     -32     /* Invalid Color Mode code */
#define ERR_OFF_BLOCK   -33     /* Bitmap is out off the block */
#define ERR_TIME_OUT    -34     /* Waiting time out */

/* add by Sen */
#define ERR_FAILED		-40
#define ERR_BUSY		-41
#define ERR_ADDRESS		-42
/* end of Sen */

#define WAR_NO_PACK     1       /* No packet received */
#define WAR_NO_CHG      2       /* Drawing is not necessary */

/* added by David */
#define DMX_ERROR_BASE    	-100

/* Compatible with previous written error*/
#define ERR_FAILUE		-9

#define PROGRAM_CONFIG_EEPROM_BLANK      (DMX_ERROR_BASE - 1) /* no program config info*/
#define PROGRAM_DATABASE_EMPTY           (DMX_ERROR_BASE - 2) /* program database is empty*/
#define PROGRAM_CONFIG_EEPROM_ERROR      (DMX_ERROR_BASE - 3) /* program config error in eeprom*/

#define XPONDER_CONFIG_EEPROM_BLANK      (DMX_ERROR_BASE - 4) /* no transponder config info*/
#define DMX_XPNDR_DATABASE_EMPTY         (DMX_ERROR_BASE - 5) /* tansponder database is empty*/
#define XPONDER_CONFIG_EEPROM_ERROR      (DMX_ERROR_BASE - 6) /* transponder config error in eeprom*/
#define ERR_TS_PACK                      (DMX_ERROR_BASE - 7) 
#define ERR_NO_SDT                       (DMX_ERROR_BASE - 8) 
#define ERR_PROG_NUM					 (DMX_ERROR_BASE - 9)
/* end of David*/

#define I2C_ERROR_BASE		-200

#define ERR_I2C_SCL_LOCK	(I2C_ERROR_BASE - 1)	/* I2C SCL be locked */
#define ERR_I2C_SDA_LOCK	(I2C_ERROR_BASE - 2)	/* I2C SDA be locked */
#define ERR_I2C_NO_ACK		(I2C_ERROR_BASE - 3)	/* I2C slave no ack */

#define ERR_FLASH_OFFSET_ERROR  -300
#define ERR_FLASH_NO_BUFFER     -301
#define ERR_FLASH_NEED_ERASE    -302
#define ERR_FLASH_WRITE_FAIL    -303
#define ERR_FLASH_READ_FAIL    -304

#define ERR_TEMPINFO_ID_USED    -310
#define ERR_TEMPINFO_NO_SPACE   -311
#define ERR_TEMPINFO_NO_MEMORY  -312
#define ERR_TEMPINFO_NOT_OPEN   -313
#define ERR_TEMPINFO_WRITE_FAIL -314
#define ERR_TEMPINFO_NEED_ERASE -315

#endif /* __RETCODE_H__ */
