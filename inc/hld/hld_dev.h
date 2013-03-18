/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 Copyright (C)
*
*    File:    hld_dev.h
*
*    Description:    This file contains all functions definations of
*		             HLD device management.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	Jan.26.2003      Justin Wu      Ver 0.1    Create file.
*	2.
*****************************************************************************/

#ifndef	__HLD_DEV_H__
#define __HLD_DEV_H__

#include <types.h>


#define	HLD_DEV_TYPE_TVE		0x01010000	/* TV encoder device */
#define HLD_DEV_TYPE_DMX		0x01020000	/* Stream de-multiplex device */
#define HLD_DEV_TYPE_OSD		0x01030000	/* OSD device */
#define HLD_DEV_TYPE_NET		0x01040000	/* Network device */
#define HLD_DEV_TYPE_NIM		0x01050000	/* NIM (Demodulator + tuner) */
#define HLD_DEV_TYPE_IRC		0x01060000	/* IR controler */
#define HLD_DEV_TYPE_PAN		0x01070000	/* Front panel device */
#define HLD_DEV_TYPE_SMC		0x01080000	/* Smart-card reader device */
#define HLD_DEV_TYPE_SND		0x01090000	/* Sound card device */
#define HLD_DEV_TYPE_DECA		0x010A0000	/* Audio decoder device */
#define HLD_DEV_TYPE_DECV		0x010B0000	/* Video decoder device */
#define HLD_DEV_TYPE_STO		0x010C0000	/* Character storage device */
#define HLD_DEV_TYPE_RFM		0x010D0000	/* RF modulator device */
#define HLD_DEV_TYPE_MST		0x010E0000	/* Block storage device */
#define HLD_DEV_TYPE_DIS		0x010F0000	/* Display device */
#define HLD_DEV_TYPE_CIC		0x01100000	/* DVB CI controler device */
#define HLD_DEV_TYPE_TTX		0x01110000	/* Teletext decoder */
#define HLD_DEV_TYPE_SDEC		0x01120000	/* Subtitle decoder */
#define HLD_DEV_TYPE_VBI		0x01130000	/* VBI decoder device */
#define HLD_DEV_TYPE_VENC		0x01140000	/* Video encoder */
#define HLD_DEV_TYPE_SGDMA		0x01150000	/* Scatter Gather DMA Engine */
#define HLD_DEV_TYPE_GE			0x01160000	/* Graphic Engine */
#define HLD_DEV_TYPE_HDMI		0x01170000	/* HDMI device */
#define HLD_DEV_TYPE_USB_HOST		0x01180000	/*USB Host Controller*/
#define HLD_DEV_TYPE_USB_HUB		0x01190000	/*USB HUB Device*/
#define HLD_DEV_TYPE_USB_MASS		0x011a0000	/*USB Mass Storage Device*/

#define HLD_DEV_TYPE_ACI		0x011b0000	/* ACI device */
#define HLD_DEV_TYPE_SCART		0x011c0000	/*Scart Switch device*/
#define HLD_DEV_TYPE_SD		0x011d0000	/*M33 SD device Controller*/

#define HLD_DEV_TYPE_DSC        0x011e0000   /*Des module*/
#define HLD_DEV_TYPE_CE         0x011f0000   /*CE module*/
#define HLD_DEV_TYPE_DES         0x01200000   /*CE module*/
#define HLD_DEV_TYPE_AES         0x01210000   /*CE module*/
#define HLD_DEV_TYPE_SHA         0x01220000   /*CE module*/
#define HLD_DEV_TYPE_CSA         0x01230000   /*CE module*/
#define HLD_DEV_TYPE_SATA         0x01240000   /*SATA Host*/
#define HLD_DEV_TYPE_GMA		0x01250000 /*Graphic Memory Access module*/
#define HLD_DEV_TYPE_NAND		0x01260000  /*nand flash*/

#define HLD_DEV_TYPE_MASK		0xFFFF0000	/* Device type mask */
#define HLD_DEV_ID_MASK			0x0000FFFF	/* Device id mask */

#define HLD_DEV_STATS_UP		0x01000001	/* Device is up */
#define HLD_DEV_STATS_RUNNING	0x01000002	/* Device is running */

#define HLD_MAX_DEV_NUMBER		32			/* Max device number */
#define HLD_MAX_NAME_SIZE		16			/* Max device name length */

struct hld_device
{
	struct hld_device   *next;						/* Next device structure */
	UINT32				type;						/* Device type */
	INT8				name[HLD_MAX_NAME_SIZE];	/* Device name */
};

#ifdef DUAL_ENABLE
struct remote_hld_device
{
	struct remote_hld_device  *next;				/* Next device structure */
	UINT32				type;						/* Device type */
	INT8				name[HLD_MAX_NAME_SIZE];	/* Device name */
    struct hld_device   *remote;						/* remote device structure */
};
#endif

/*
  * this struct is used to save device IO parameter
  */
struct io_param
{
     UINT8 *io_buff_in;
     UINT32 buff_in_len;
     UINT8 *io_buff_out;
     UINT32 buff_out_len;
};

struct io_param_ex
{
     UINT8 *io_buff_in;
     UINT32 buff_in_len;
     UINT8 *io_buff_out;
     UINT32 buff_out_len;
	 UINT32 hnd;
	 UINT8 h264_flag;
	 UINT8 is_scrambled;
     UINT8 record_all;
};

#ifdef __cplusplus
extern "C"
{
#endif

void *dev_get_by_name(INT8 *name);
void *dev_get_by_type(void *sdev, UINT32 type);
void *dev_get_by_id(UINT32 type, UINT16 id);
void dev_list_all(void *sdev);

void *dev_alloc(INT8 *name, UINT32 type, UINT32 size);
INT32 dev_register(void *dev);
void  dev_free(void *dev);
#ifdef DUAL_ENABLE
void dev_en_remote(UINT8 en);
#endif

#ifdef SEE_CPU
void hld_cpu_callback(UINT32 para);
#endif

#ifdef __cplusplus
}
#endif

#endif	/* __HLD_DEV_H__ */
