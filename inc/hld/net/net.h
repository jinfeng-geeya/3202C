/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2007 Copyright (C)
*
*    File:    net.h
*
*    Description:     This file defines the structures of net device
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	May.17.2007      Trueve Hu      Ver 0.1    Create file.
*
*****************************************************************************/

#ifndef _NET_H_
#define _NET_H_


#include <hld/hld_dev.h>
#include <basic_types.h>
#if (SYS_NETWORK_MODULE == NET_USB_WIFI) 
#include <hld/wifi/iw_handler.h>   // WiFi Support
#endif


/* Driver flags */
#define NET_DEV_STATS_XMITTING	(1<<2)

/*
 *	IEEE 802.3 Ethernet magic constants.  The frame sizes omit the preamble
 *	and FCS/CRC (frame check sequence). 
 */

#define ETH_ALEN			6		/* Octets in one ethernet addr	 */
#define ETH_HLEN			14		/* Total octets in header.	 */
#define ETH_ZLEN			60		/* Min. octets in frame sans FCS */
#define ETH_DATA_LEN		1500	/* Max. octets in payload	 */
#define ETH_FRAME_LEN		1514	/* Max. octets in frame sans FCS */
#define ETH_FCS_LEN			4		/* Octets in the FCS		 */

typedef void (* spe_copy_func)(UINT8, UINT32, UINT32, UINT32, UINT32);

struct net_pkt_t
{
	UINT8 * buffer;
	UINT16 length;
	UINT16 reserved;
};

struct net_mc_list
{
	struct net_mc_list *next;
	UINT8 dmi_addr [ETH_ALEN];
};

typedef enum 
{
	NET_PHY_RMII = 0,
	NET_PHY_MII,
}net_phy_mode;

enum net_link_status
{
	NET_LINK_CONNECTED	= 0,
	NET_LINK_DISCONNECTED,
};

enum net_link_speed
{
	NET_LINK_10MBPS		= 0,
	NET_LINK_100MBPS,
};

enum net_link_mode
{
	NET_LINK_AUTO			= 0,
	NET_LINK_HALF,
	NET_LINK_FULL,
};

enum net_get_info_type
{
	NET_GET_PERMANENT_MAC	= 0,
	NET_GET_CURRENT_MAC,
	NET_GET_LINK_STATUS,
	NET_GET_LINK_SPEED,
	NET_GET_LINK_MODE,
	NET_GET_XMIT_BYTES,
	NET_GET_REV_BYTES,
	NET_GET_CAPABILITY,
};

enum net_set_info_type
{
	NET_SET_MAC			= 0,
	NET_SET_LINK_SPEED,
	NET_SET_LINK_MODE,
	NET_ADD_MULTICAST_ADDR,
	NET_DEL_MULTICAST_ADDR,
	NET_SET_PROMISCUOUS_MODE,
	NET_SET_MULTICAST_FILTER_MODE,
	NET_SET_OFFSET,
	NET_SET_CAPABILITY,
};

enum net_callback_cmd_type
{
	NET_RECV_PKT			= 0,
	NET_LINK_CHANGE,
};

struct net_device
{
	struct net_device  *next;  	/* Next device */
	INT32 type;
	INT8	  name[HLD_MAX_NAME_SIZE];
	INT32 flags;
	UINT32 base_addr;

	void *priv;				/* Used to be 'private' but that upsets C++ */

#if (SYS_NETWORK_MODULE == NET_USB_WIFI)   // WiFi Support
	/* List of functions to handle Wireless Extensions (instead of ioctl). */
	const struct iw_handler_def *	wireless_handlers;

	int (*scan_ap)(struct net_device *);
	int (*get_ap_list)(struct net_device *, UINT32, int);
	void (*connect_ap)(struct net_device *, UINT32);
	void (*disconnect_ap)(struct net_device *, UINT32);  //cpadd_110421
	int (*get_conn_status)(struct net_device *);
	int (*get_plug_status)(struct net_device *);
	int (*get_ap_quality)(struct net_device *); //cpadd_110420_add_quality
#endif

	RET_CODE (*attach) (void);
	RET_CODE (*detach) (struct net_device *);
	
	RET_CODE (*open) (struct net_device *, void (*callback) (UINT32, UINT32));
	RET_CODE (*close) (struct net_device *);
	RET_CODE (*ioctl) (struct net_device *, UINT32, UINT32);

	RET_CODE (*send_packet) (struct net_device *, void *, UINT16);
	void (*callback) (UINT32, UINT32);
	
	RET_CODE (*get_info) (struct net_device *, UINT32, void *);
	RET_CODE (*set_info) (struct net_device *, UINT32, void *);
};

typedef struct {
	UINT16	cs_by_gpio: 1;  	/*1: we need use one GPIO do Chip Select.*/
	UINT16	cs2_by_gpio: 1;	/*1: Second GPIO cs for PHY setting*/
	UINT16	rst_by_gpio: 1;	/*1: we need use one GPIO do Chip Reset.*/
	UINT16	int_by_gpio: 1;	/*1: GPIO detect enc interrupt.*/
	UINT16	init_mac_addr: 1;
	UINT16	spe_cpy_en: 1;
	UINT16	reserved1: 10;
	UINT16	cs_polar	: 1;	/* Polarity of GPIO, 0 or 1 active(light) */
	UINT16	cs_io		: 1;	/* HAL_GPIO_I_DIR or HAL_GPIO_O_DIR in hal_gpio.h */
	UINT16	cs_position: 14;	/* GPIO index*/
	UINT16	cs2_polar	: 1;	/* Polarity of GPIO, 0 or 1 active(light) */
	UINT16	cs2_io		: 1;	/* HAL_GPIO_I_DIR or HAL_GPIO_O_DIR in hal_gpio.h */
	UINT16	cs2_position: 14;	/* GPIO index*/
	UINT16	rst_polar	: 1;	/* Polarity of GPIO, 0 or 1 active(light) */
	UINT16	rst_io		: 1;	/* HAL_GPIO_I_DIR or HAL_GPIO_O_DIR in hal_gpio.h */
	UINT16	rst_position: 14;	/* GPIO index*/
	UINT16	int_polar	: 1;	/* Polarity of GPIO, 0 or 1 active(light) */
	UINT16	int_io		: 1;	/* HAL_GPIO_I_DIR or HAL_GPIO_O_DIR in hal_gpio.h */
	UINT16	int_position: 14;	/* GPIO index*/
	enum net_link_mode link_mode;
	UINT8	mac_addr[6];
	UINT8	spe_id;
	UINT32    map_addr;
	spe_copy_func dma_cpy;
	UINT32 	reserved2;
}ENC_CONFIG;

typedef struct {
	UINT16 en_gpio_setting : 1;  //SMSC9220 uses 4 GPIOs: 2 as address lines & 1 as Fifo select & 1 as interrupt.
	UINT16 en_mac_setting : 1;  //MAC Address set by APP.
	UINT16 en_ata_setting : 1;  //ata timing settings, should set it to 0. DO NOT THE TIMING!
	UINT16 reserved : 13;

	//GPIO settings
	UINT8 cs_gpio_position;  //lan_cs
	UINT8 a7_gpio_position;  //lan_a7
	UINT8 a6_gpio_position;  //lan_a6
	UINT8 sel_gpio_position;  //lan_fifo_sel
	UINT8 int_gpio_position;  //lan_int
	UINT8 int_gpio_polar;  //Polarity of GPIO, 0 or 1 active(default is 0)
	
	//MAC setting
	UINT8 mac_addr[6];
	
	//ATA setting
	UINT32 ata_timing; 
} SMSC_CONFIG;

typedef struct {	UINT16 en_gpio_setting : 1;  //AX88796B uses 4 GPIOs: 2 as address lines & 1 as Fifo select & 1 as interrupt.	UINT16 en_mac_setting : 1;  //MAC Address set by APP.	UINT16 en_ata_setting : 1;  //ata timing settings, should set it to 0. DO NOT THE TIMING!	UINT16 reserved : 13;
	//GPIO settings	UINT8 cs_gpio_position;  //lan_cs	UINT8 sel_gpio_position;  //lan_fifo_sel	UINT8 int_gpio_position;  //lan_int	UINT8 int_gpio_polar;  //Polarity of GPIO, 0 or 1 active(default is 0)
	//MAC setting	UINT8 mac_addr[6];
	//ATA setting	UINT32 ata_timing; } ASIX_CONFIG;

typedef struct {

	//MAC setting
	UINT8 mac_addr[6];
	UINT8 phy_addr;
	net_phy_mode phy_mode;

	UINT32 reserved0;	
	UINT32 reserved1;
	UINT32 reserved2;
	UINT32 reserved3;
	UINT32 reserved4;
	UINT32 reserved5;
	UINT32 reserved6;
	UINT32 reserved7;
	UINT32 reserved8;
	UINT32 reserved9;
	UINT32 reserved10;
} MAC_CONFIG;


RET_CODE net_open(struct net_device *dev, void (*callback) (UINT32, UINT32));
RET_CODE net_close(struct net_device *dev);
RET_CODE net_io_ctl(struct net_device *dev, UINT32, UINT32);

RET_CODE net_send_packet (struct net_device *dev, void *buf, UINT16 len);

RET_CODE net_get_info(struct net_device *dev, UINT32 info_type, void *info_buf);
RET_CODE net_set_info(struct net_device *dev, UINT32 info_type, void *info_buf);

#endif

