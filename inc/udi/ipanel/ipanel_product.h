/*!
	* \file 
	*/
/*********************************************************************
    Copyright (c) 2005 Embedded Internet Solutions, Inc
    All rights reserved. You are not allowed to copy or distribute
    the code without permission.
    There are the Product Config Porting APIs needed by iPanel MiddleWare. 
    Note: the "int" in the file is 32bits
    $ver0.0.0.1 $author Zouxianyun 2005/04/28
*********************************************************************/
#ifndef _IPANEL_MIDDLEWARE_PORTING_PRODUCT_CONFIG_API_FUNCTOTYPE_H_
#define _IPANEL_MIDDLEWARE_PORTING_PRODUCT_CONFIG_API_FUNCTOTYPE_H_
#ifdef __cplusplus
extern "C" {
#endif
/* get network description or host description */
const char *ipanel_porting_get_host(void);
/* set network description or host description */
int ipanel_porting_set_host(const char *host);
/* get current user device unique ID */
//const char *ipanel_porting_get_userid(void);   接口已废除
/* get current network device description */
const char *ipanel_porting_get_description(void);
/* get NIC physical MAC address */
const char *ipanel_porting_get_macaddress(void);
/* get current DHCP work mode */
int ipanel_porting_get_DHCP(void);
/* set current DHCP work mode */
int ipanel_porting_set_DHCP(int dhcp);
/* get current network terminal ip address */
const char *ipanel_porting_get_IPaddress(void);
/* set current network terminal ip address */
int ipanel_porting_set_IPaddress(const char *ip);
/* get current network terminal subnet mask */
const char *ipanel_porting_get_mask(void);
/* set current network terminal subnet mask */
int ipanel_porting_set_mask(const char *mask);
/* get current network terminal gateway ip address */
const char *ipanel_porting_get_gateway(void);
/* set current network terminal gateway ip address */
int ipanel_porting_set_gateway(const char *gateway);
/* get current network offered DHCP server ip address */
const char *ipanel_porting_get_dhcpserver(void);
/*get current network offered DHCP server port*/
/*!
 * \brief 			获取当前网络上提供 DHCP 服务的服务器端口号。
 * \author 			
 * \param[in] 	无
 * \param[out]	无
 * \return			DHCP 服务的服务器端口号
 * \version			
 * \date				
 * \warning			
 * \sa					ipanel_product_demo.c
 * \deprecated	
*/ 
int ipanel_porting_get_dhcp_port(void);
/* get current network offered master DNS server ip address */
const char *ipanel_porting_get_DNSserver1(void);
/* set current network offered master DNS server ip address */
int ipanel_porting_set_DNSserver1 (const char *dns1);
/* get current network offered slave DNS server ip address */
const char *ipanel_porting_get_DNSserver2(void);
/* set current network offered slave DNS server ip address */
int ipanel_porting_set_DNSserver2(const char *dns2);
/* get the time of obtained ip address */
const char *ipanel_porting_get_obtained(void);
/* get the time of ip address expires */
const char *ipanel_porting_get_expires(void);
/* get current network NTP server ip or domain */
const char *ipanel_porting_get_NTPserver(void);
/* set current network NIP server ip */
int ipanel_porting_set_NTPserver(const char *ntp);
/* get the difference of local time and base time */
int ipanel_porting_get_timezone(void);
/* set the difference of local time and base time */
int ipanel_porting_set_timezone(int timezone);
/* get dialup mode */
int ipanel_porting_get_dialup(void);
/* set dialup mode */
int ipanel_porting_set_dialup(int type);
/* get current dialup mode user name */
const char *ipanel_porting_get_dialup_user(void);
/* set current dialup mode user name */
int ipanel_porting_set_dialup_user(const char *usr);
/* get current dialup mode user password */
const char *ipanel_porting_get_dialup_password(void);
/* set current dialup mode user password */
int ipanel_porting_set_dialup_password(const char *pwd);
/* get current offered proxy server ip or domain */
const char *ipanel_porting_get_proxy_server(void);
/* set current offered proxy server ip or domain */
int ipanel_porting_set_proxy_server(const char *proxy);
/* get current proxy serve port */
int ipanel_porting_get_proxy_port(void);
/* set current proxy serve port */
int ipanel_porting_set_proxy_port(int port);
/* get current proxy serve access user name */
const char *ipanel_porting_get_proxy_user(void);
/* set current proxy serve access user name */
int ipanel_porting_set_proxy_user(const char *usr);
/* get current proxy serve access user password */
const char *ipanel_porting_get_proxy_password(void);
/* set current proxy serve access user password */
int ipanel_porting_set_proxy_password(const char *pwd);
/* get current system loader name */
const char *ipanel_porting_get_loader_name(void);
/* get current system loader version */
const char *ipanel_porting_get_loader_version(void);
/* get current system loader provider */
const char *ipanel_porting_get_loader_provider(void);
/* get current system loader size */
const char *ipanel_porting_get_loader_size(void);
/* get current driver name */
const char *ipanel_porting_get_driver_name(void);
/* get current driver version */
const char *ipanel_porting_get_driver_version(void);
/* get current driver provider */
const char *ipanel_porting_get_driver_provider(void);
/* get current driver size */
const char *ipanel_porting_get_driver_size(void);
/* get current OS name */
const char *ipanel_porting_get_os_name(void);
/* get current OS version */
const char *ipanel_porting_get_os_version(void);
/* get current OS provider */
const char *ipanel_porting_get_os_provider(void);
/* get current OS size */
const char *ipanel_porting_get_os_size(void);
/* get current middleware name */
const char *ipanel_porting_get_MWclient_name(void);
/* get current middleware version */
const char *ipanel_porting_get_MWclient_version(void);
/* get current middleware provider */
const char *ipanel_porting_get_MWclient_provider(void);
/* get current middleware size */
const char *ipanel_porting_get_MWclient_size(void);
/* get current application layer name */
const char *ipanel_porting_get_STBapp_name(void);
/* get current application layer version */
const char *ipanel_porting_get_STBapp_version(void);
/* get current application layer provider */
const char *ipanel_porting_get_STBapp_provider(void);
/* get current application layer size */
const char *ipanel_porting_get_STBapp_size(void);
/* get current CA name */
const char *ipanel_porting_get_ca_name(void);
/* get current CA version */
const char *ipanel_porting_get_ca_version(void);
/* get current CA provider */
const char *ipanel_porting_get_ca_provider(void);
/* get current CA size */
const char *ipanel_porting_get_ca_size(void);
/* get current product brand */
const char *ipanel_porting_get_product_brand(void);
/* get current product name */
const char *ipanel_porting_get_product_name(void);
/* get current product serial number */
const char *ipanel_porting_get_product_serial(void);
/* get current product cpu type */
const char *ipanel_porting_get_product_SOC(void);
/* get current product PCB id */
const char *ipanel_porting_get_product_PCB(void);
/* get product flash size */
int ipanel_porting_get_product_flash(void);
/* get product RAM size */
int ipanel_porting_get_product_RAM(void);
/* get product upgrade mode */
int ipanel_porting_get_upgrade_mode(void);
/* set product upgrade mode */
int ipanel_porting_set_upgrade_mode(int mode);
/* get product upgrade server ip or domain */
const char *ipanel_porting_get_upgrade_server(void);
/* set product upgrade server ip or domain */
int ipanel_porting_set_upgrade_server(const char *server);
/* get product upgrade server port */
int ipanel_porting_get_upgrade_port(void);
/* set product upgrade server port */
int ipanel_porting_set_upgrade_port(int port);
/* get product upgrade server user name */
const char *ipanel_porting_get_upgrade_user(void);
/* set product upgrade server user name */
int ipanel_porting_set_upgrade_user(const char *usr);
/* get product upgrade server user password */
const char *ipanel_porting_get_upgrade_password(void);
/* set product upgrade server user password */
int ipanel_porting_set_upgrade_password(const char *pwd);
/*same as ipconfig /all*/
int ipanel_porting_system_ipconfig_all(void);
/*same as ipconfig /flush dns*/
int	ipanel_porting_system_ipconfig_flushdns(void);
/*same as ipconfig /register dns*/
int	ipanel_porting_system_ipconfig_registerdns(void);
/*same as ipconfig /release */
int	ipanel_porting_system_ipconfig_release(void);
/*same as ipconfig /renew */
int	ipanel_porting_system_ipconfig_renew(void);
/*same as ipconfig /save */
int	ipanel_porting_system_ipconfig_save(void);
/*reboot stb after s seconds*/
int	ipanel_porting_system_reboot(int s);
/*update stb after s seconds*/
int	ipanel_porting_system_upgrade(int s);
/*Get Remote control type id string*/
const char *ipanel_porting_get_remote_control(void);
/*get remote keyboard type id string*/
const char *ipanel_porting_get_remote_keyboard(void);
/*get remote sensitivity times*/
/*!
 * \brief 			获取遥控器的灵敏度，即在规定时间内会出现的最大重复次数。
 * \author 			
 * \param[in] 	无
 * \param[out]	无
 * \return			遥控器的灵敏度
 * \version			
 * \date				
 * \warning			
 * \sa					ipanel_product_demo.c
 * \deprecated	
*/ 
int ipanel_porting_get_remote_sensitivity (void);
/*set remote sensitivity times*/
/*!  
 *  \brief 设置遥控器的灵敏度，即在规定时间内会出现的最大重复次数。
 *  \author
 *  @param[in] sens 遥控器的灵敏度（每秒的最大次数）
 *  @param[out] 无
 *  \return 0 – 成功，-1 – 失败
 *  \version 
 *  \date     
 *  \warning 
 *  \sa ipanel_product_demo.c
  *  \deprecated
 */
int ipanel_porting_set_remote_sensitivity (int sens);
/*get administrator password*/
const char *ipanel_porting_get_admin_password(void);
/*set administrator password*/
int ipanel_porting_set_admin_password(const char *pwd);
/*get administrator name*/
const char *ipanel_porting_get_admin_user(void);
/*set administrator name*/
int ipanel_porting_set_admin_user(const char *usr);
int ipanel_porting_hotkey_add(int number, const char *url);
int ipanel_porting_hotkey_clean(void);
int ipanel_porting_hotkey_del(int number);
int ipanel_porting_hotkey_edit(int number);
int ipanel_porting_hotkey_go(int number);
int ipanel_porting_hotkey_list(void);
int ipanel_porting_hotkey_recall(void);
int ipanel_porting_hotkey_save(void);
#ifdef __cplusplus
}
#endif
#endif
