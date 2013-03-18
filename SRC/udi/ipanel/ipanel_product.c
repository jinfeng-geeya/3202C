/*********************************************************************
    Copyright (c) 2005 Embedded Internet Solutions, Inc
    All rights reserved. You are not allowed to copy or distribute
    the code without permission.
    This is the demo implenment of the Product Config Porting APIs needed by iPanel MiddleWare. 
    Maybe you should modify it accorrding to Platform.
    
    Note: the "int" in the file is 32bits
    
    $ver0.0.0.1 $author Zouxianyun 2005/04/28
*********************************************************************/

#include <udi/ipanel/ipanel_product.h>
#include <api/libc/string.h>
#include <udi/ipanel/ipanel_base.h>

/**********************************************************************************/
/*Description: get current network ID or host ID, max length is 16 bytes          */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : current network ID or host ID                                      */
/**********************************************************************************/
const char *ipanel_porting_get_host(void)
{
	return "myHost-iPanel";
}


/**********************************************************************************/
/*Description: Set Current Network ID or Host ID, max length is 16 bytes          */
/*Input      : Current Network ID or Host ID string                               */
/*Output     : no                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
int ipanel_porting_set_host(const char *host)
{
	int ret = -1, len = 0;
	len = strlen(host);
	if (len > 0 && len <= 16) {
		/*set current network ID or host ID here*/		
		ret = 0;
	}
	return ret;
}


/**********************************************************************************/
/*Description: get current user device unique ID                                  */
/*Input      : No                                                                 */
/*Output     : no                                                                 */
/*Return     : user ID                                                            */
/**********************************************************************************/
const char *ipanel_porting_get_userid(void)
{
	return "iPanel-001-123";
}


/**********************************************************************************/
/*Description: get current network device description, max length is 32 bytes     */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : current network device description string                          */
/**********************************************************************************/
const char *ipanel_porting_get_description(void)
{
	return "my_network_lan_iPanel";
}


/**********************************************************************************/
/*Description: get NIC physical MAC address, format must be "xx:xx:xx:xx:xx:xx"   */
/*             where x --[0-9 a-f A-F]                                            */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : NIC MAC address string                                             */
/**********************************************************************************/
const char *ipanel_porting_get_macaddress(void)
{
	return "00:00:00:00:00:00";
}


/**********************************************************************************/
/*Description: get current DHCP work mode, 0 not use DHCP, 1 use DHCP             */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : 0 or 1                                                             */
/**********************************************************************************/
int ipanel_porting_get_DHCP(void)
{
	return 1;
}


/**********************************************************************************/
/*Description: set current DHCP work mode                                         */
/*Input      : DHCP work mode, 0 not use DHCP, 1 use DHCP                         */
/*Output     : no                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
int ipanel_porting_set_DHCP(int dhcp)
{
	int ret = -1;
	
	if ((dhcp == 0) || (dhcp == 1))
	{
		/*set physical dhcp here.*/
		ret = 0;
	}
		
	return ret;
}


/**********************************************************************************/
/*Description: get current network terminal ip, format such as "xxx.xxx.xxx.xxx"  */
/*             where x --[0-9]                                                    */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : ip string                                                          */
/**********************************************************************************/
const char *ipanel_porting_get_IPaddress(void)
{
	//return "192.168.24.211";
	return "192.168.28.122";
}


/**********************************************************************************/
/*Description: set current network terminal ip address                            */
/*Input      : ip string, format such as "xxx.xxx.xxx.xxx" , where x --[0-9]      */
/*Output     : no                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
int ipanel_porting_set_IPaddress(const char *ip)
{
	int ret = -1;
	/*set physical ip address here*/
	ret = 0;		
	return ret;
}


/**********************************************************************************/
/*Description: get current network terminal subnet mask, format such as           */
/*             "xxx.xxx.xxx.xxx"   where x --[0-9]                                */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : subnet mask                                                        */
/**********************************************************************************/
const char *ipanel_porting_get_mask(void)
{
	return "255.255.255.0";
}


/**********************************************************************************/
/*Description: set current network terminal subnet mask                           */
/*Input      : ip string, format such as "xxx.xxx.xxx.xxx"   where x --[0-9]      */
/*Output     : no                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
int ipanel_porting_set_mask(const char *mask)
{
	int ret = -1;
	/*set physical subnet mask here.*/
	ret = 0;
	return ret;
}


/**********************************************************************************/
/*Description: get current network terminal gateway ip                            */
/*             format such as "xxx.xxx.xxx.xxx"   where x --[0-9]                 */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : ip string                                                          */
/**********************************************************************************/
const char *ipanel_porting_get_gateway(void)
{
//	return "192.168.24.1";
	return "192.168.28.1";
}


/**********************************************************************************/
/*Description: set current network terminal gateway ip address                    */
/*Input      : ip string, format such as "xxx.xxx.xxx.xxx"   where x --[0-9]      */
/*Output     : no                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
int ipanel_porting_set_gateway(const char *gateway)
{
	int ret = -1;
	/*set physical gateway here.*/
	ret = 0;
	return ret;
}


/**********************************************************************************/
/*Description: get current network offered DHCP server ip,                        */
/*             format such as "xxx.xxx.xxx.xxx"   where x --[0-9]                 */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : ip string                                                          */
/**********************************************************************************/
const char *ipanel_porting_get_dhcpserver(void)
{
	return "192.168.10.252";
}


/**********************************************************************************/
/*Description: get current network offered master DNS server ip,                  */
/*             format such as "xxx.xxx.xxx.xxx"   where x --[0-9]                 */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : ip string                                                          */
/**********************************************************************************/
const char *ipanel_porting_get_DNSserver1(void)
{
	return "192.168.10.248";
}


/**********************************************************************************/
/*Description: set current network offered master DNS server ip address           */
/*Input      : ip string, format such as "xxx.xxx.xxx.xxx"   where x --[0-9]      */
/*Output     : no                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
int ipanel_porting_set_DNSserver1(const char *dns1)
{
	int ret = -1;
	/*set physical master DNS here.*/
	ret = 0;
	return ret;
}


/**********************************************************************************/
/*Description: get current network offered slave DNS server ip,                   */
/*             format such as "xxx.xxx.xxx.xxx"   where x --[0-9]                 */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : ip string                                                          */
/**********************************************************************************/
const char *ipanel_porting_get_DNSserver2(void)
{
	return "192.168.10.247";
}


/**********************************************************************************/
/*Description: set current network offered slave DNS server ip address            */
/*Input      : ip string, format such as "xxx.xxx.xxx.xxx"   where x --[0-9]      */
/*Output     : no                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
int ipanel_porting_set_DNSserver2(const char *dns2)
{
	int ret = -1;
	/*set physical slave DNS here.*/
	ret = 0;
	return ret;
}


/**********************************************************************************/
/*Description: get the time of obtained ip in DHCP mode, must as "HH:MM:SS"       */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : time string                                                        */
/**********************************************************************************/

/*!
 * \author wqs,lipan
 *
 */
const char *ipanel_porting_get_obtained(void)
{
	return "00:01:23";
}


/**********************************************************************************/
/*Description: get the time of ip expires in DHCP mode, must as "HH:MM:SS"        */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : time string                                                        */
/**********************************************************************************/
const char *ipanel_porting_get_expires(void)
{
	return "10:00:00";
}


/**********************************************************************************/
/*Description: get current network NTP server ip or domain                        */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : ip string or domain string                                         */
/**********************************************************************************/
const char *ipanel_porting_get_NTPserver(void)
{
	return "163.164.24.32";
}


/**********************************************************************************/
/*Description: set current network NTP server ip or domain                        */
/*Input      : ip string or domain string no more than 32 bytes                   */
/*Output     : no                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
int ipanel_porting_set_NTPserver(const char *ntp)
{
	int ret = -1;	
	return ret;
}


/**********************************************************************************/
/*Description: get the difference of local time and base time   (0 ~ 23)          */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : difference of local time and base time                             */
/**********************************************************************************/
int ipanel_porting_get_timezone(void)
{	
	return 8;
}


/**********************************************************************************/
/*Description: set the difference of local time and base time                     */
/*Input      : difference of local time and base time (0 ~ 23)                    */
/*Output     : no                                                                 */
/*Return     : 0 -- success, -1 ---fail                                           */
/**********************************************************************************/
int ipanel_porting_set_timezone(int timezone)
{
	int ret = -1;
	
	if ((timezone >0) && (timezone < 24))
	{
		//ret = set_timezone(timezone);
	}
	
	return ret;
}


/**********************************************************************************/
/*Description: get dialup mode, 0 not user dialup, 1 PPP, 2 PPPoE, other no effect*/
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : dialup mode                                                        */
/**********************************************************************************/
int ipanel_porting_get_dialup(void)
{
	return 2;
}


/**********************************************************************************/
/*Description: set dialup mode                                                    */
/*Input      : 0 not user dialup, 1 PPP, 2 PPPoE, other no effect                 */
/*Output     : no                                                                 */
/*Return     : 0 --- success, -1 --- fail                                         */
/**********************************************************************************/
int ipanel_porting_set_dialup(int type)
{
	int ret = -1;
	
	if ((type>=1) && (type<=2))
	{
	//	ret = set_dialup(type);
	}
	
	return ret;
}


/**********************************************************************************/
/*Description: get current dialup mode user name, string no more the 16 bytes     */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : user name string                                                   */
/**********************************************************************************/
const char *ipanel_porting_get_dialup_user(void)
{
	return "user1";
}


/**********************************************************************************/
/*Description: set current dialup mode user name                                  */
/*Input      : string no more the 16 bytes                                        */
/*Output     : no                                                                 */
/*Return     : 0 --- success, -1 --- fail                                         */
/**********************************************************************************/
int ipanel_porting_set_dialup_user(const char *usr)
{
	int ret = -1;
	
	if (strlen(usr) <= 16)
	{
		//ret = set_dialup_user(usr);
	}
	
	return ret;
}


/**********************************************************************************/
/*Description: get current dialup mode user password, string no more the 16 bytes */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : password string                                                    */
/**********************************************************************************/
const char *ipanel_porting_get_dialup_password(void)
{
	return "123456";
}


/**********************************************************************************/
/*Description: set current dialup mode user password                              */
/*Input      : string no more the 16 bytes                                        */
/*Output     : no                                                                 */
/*Return     : 0 --- success, -1 --- fail                                         */
/**********************************************************************************/
int ipanel_porting_set_dialup_password(const char *pwd)
{
	int ret = -1;
	
	if (strlen(pwd) <= 16)
	{
		//ret = set_dialup_password(pwd);
	}
	
	return ret;
}


/**********************************************************************************/
/*Description: get current offered proxy server ip or domain,                     */
/*             string as "192.168.10.228"                                         */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : ip or domain string                                                */
/**********************************************************************************/
const char *ipanel_porting_get_proxy_server(void)
{
	return "192.168.10.228";
}


/**********************************************************************************/
/*Description: set current offered proxy server ip or domain                      */
/*Input      : string as "192.168.10.228" or "nero.hkbn.net.hk"                   */
/*Output     : no                                                                 */
/*Return     : 0 --- success, -1 --- fail                                         */
/**********************************************************************************/
int ipanel_porting_set_proxy_server(const char *proxy)
{
	int ret = -1;
	
	return ret;
}


/**********************************************************************************/
/*Description: get current offered proxy server port                              */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : port number                                                        */
/**********************************************************************************/
int ipanel_porting_get_proxy_port(void)
{
	return 1152;
}


/**********************************************************************************/
/*Description: set current offered proxy server port                              */
/*Input      : port number                                                        */
/*Output     : no                                                                 */
/*Return     : 0 --- success, -1 --- fail                                         */
/**********************************************************************************/
int ipanel_porting_set_proxy_port(int port)
{
	int ret = -1;
	
	// ret = set_proxy_port(port);
	
	return ret;
}


/**********************************************************************************/
/*Description: get current offered proxy serve user name,                         */ 
/*             string no more the 16 bytes                                        */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : name string                                                        */
/**********************************************************************************/
const char *ipanel_porting_get_proxy_user(void)
{
	return "user";
}


/**********************************************************************************/
/*Description: set current offered proxy serve user name                          */
/*Input      : string no more the 16 bytes                                        */
/*Output     : no                                                                 */
/*Return     : 0 --- success, -1 --- fail                                         */
/**********************************************************************************/
int ipanel_porting_set_proxy_user(const char *usr)
{
	int ret = -1, len;

	len = strlen(usr);
	if (len>0 && len<=16)
	{
		/*set real proxy user here.*/
		ret = 0;
	}
	
	return ret;
}


/**********************************************************************************/
/*Description: get current offered proxy serve user password,                     */
/*             string no more the 16 bytes                                        */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : password string                                                    */
/**********************************************************************************/
const char *ipanel_porting_get_proxy_password(void)
{
	return "123456";
}


/**********************************************************************************/
/*Description: set current offered proxy serve user password                      */
/*Input      : string no more the 16 bytes                                        */
/*Output     : no                                                                 */
/*Return     : 0 --- success, -1 --- fail                                         */
/**********************************************************************************/
int ipanel_porting_set_proxy_password(const char *pwd)
{
	int ret = -1;
	
	if (strlen(pwd) <= 16)
	{
		/*set real proxy user password here.*/
		ret = 0;
	}
	
	return ret;
}


/**********************************************************************************/
/*Description: get current system loader name, string no more the 40 bytes        */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : loader name string                                                 */
/**********************************************************************************/
const char *ipanel_porting_get_loader_name(void)
{
	return "iPanel_loader_name";
}


/**********************************************************************************/
/*Description: get current system loader version, string no more the 40 bytes     */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : loader version string                                              */
/**********************************************************************************/
const char *ipanel_porting_get_loader_version(void)
{
	return "2.0.0.2";
}


/**********************************************************************************/
/*Description: get current system loader provider, string no more the 40 bytes    */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : provider string                                                    */
/**********************************************************************************/
const char *ipanel_porting_get_loader_provider(void)
{
	return "iPanel_loader_provider";
}


/**********************************************************************************/
/*Description: get current system loader size, string no more the 40 bytes        */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : size string                                                        */
/**********************************************************************************/
const char *ipanel_porting_get_loader_size(void)
{
	return "204800";
}


/**********************************************************************************/
/*Description: get current system driver name, string no more the 40 bytes        */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : driver name string                                                 */
/**********************************************************************************/
const char *ipanel_porting_get_driver_name(void)
{
	return "iPanel_driver_name";
}


/**********************************************************************************/
/*Description: get current system driver version, string no more the 40 bytes     */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : driver versin string                                               */
/**********************************************************************************/
const char *ipanel_porting_get_driver_version(void)
{
	return "3.2.1";
}


/**********************************************************************************/
/*Description: get current system driver provider, string no more the 40 bytes    */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : driver provider string                                             */
/**********************************************************************************/
const char *ipanel_porting_get_driver_provider(void)
{
	return "iPanel_driver_provider";
}


/**********************************************************************************/
/*Description: get current system driver size, string no more the 40 bytes        */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : driver size string                                                 */
/**********************************************************************************/
const char *ipanel_porting_get_driver_size(void)
{
	return "204800";
}

#if 0
/**********************************************************************************/
/*Description: get current CA name, string no more the 40 bytes                   */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : name string                                                        */
/**********************************************************************************/
const char *ipanel_porting_get_ca_name(void)
{
	return "iPanel_CA_name";
}


/**********************************************************************************/
/*Description: get current CA version, string no more the 40 bytes                */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : version string                                                     */
/**********************************************************************************/
const char *ipanel_porting_get_ca_version(void)
{
	return "0.0.12";
}


/**********************************************************************************/
/*Description: get current CA provider, string no more the 40 bytes               */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : provider string                                                    */
/**********************************************************************************/
const char *ipanel_porting_get_ca_provider(void)
{
	return "iPanel_rpovider";
}


/**********************************************************************************/
/*Description: get current CA size, string no more the 40 bytes                   */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : size string                                                        */
/**********************************************************************************/
const char *ipanel_porting_get_ca_size(void)
{
	return "11534244";
}

#endif
/**********************************************************************************/
/*Description: get product serial number, string no more the 32 bytes             */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : serial number string                                               */
/**********************************************************************************/
const char *ipanel_porting_get_product_serial(void)
{
	return "12345678901234567890";
}


/**********************************************************************************/
/*Description: get product flash size, int bytes                                  */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : size in bytes                                                      */
/**********************************************************************************/
int ipanel_porting_get_product_flash(void)
{
	return 0x200000;
}


/**********************************************************************************/
/*Description: get product RAM size, in bytes                                     */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : size in bytes */
/**********************************************************************************/
int ipanel_porting_get_product_RAM(void)
{
	return 0x2000000;
}


/**********************************************************************************/
/*Description: get product upgrade mode, 1: DvbSi£¬2: Unicast£¬3: remote£¬4: local*/
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : upgrade mode                                                       */
/**********************************************************************************/
int ipanel_porting_get_upgrade_mode(void)
{
	int ret = -1;
	
	// ret = get_upgrade_mode();
	
	return ret;
}


/**********************************************************************************/
/*Description: set product upgrade mode                                           */
/*Input      : 1: DvbSi£¬2: Unicast£¬3: remote£¬4: local                          */
/*Output     : no                                                                 */
/*Return     : 0 --- success, -1 --- fail                                         */
/**********************************************************************************/
int ipanel_porting_set_upgrade_mode(int mode)
{
	int ret = -1;
	
	if ((mode>0) && (mode<=4))
	{
		//ret = set_upgrade_mode(mode);
	}
	
	return ret;
}


/**********************************************************************************/
/*Description: get product upgrade server ip or domain, must as "192.168.22.222"  */
/*             or "kdf.dk.hk.cn", string normaly no more the 32 bytes             */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : ip string or domain string                                         */
/**********************************************************************************/
const char *ipanel_porting_get_upgrade_server(void)
{
	return "192.168.22.222";
}


/**********************************************************************************/
/*Description: set product upgrade server ip or domain                            */
/*Input      : ip string of domain string no more than 32 bytes                   */
/*Output     : no                                                                 */
/*Return     : 0 --- success, -1 --- fail                                         */
/**********************************************************************************/
int ipanel_porting_set_upgrade_server(const char *server)
{
	int ret = -1;
	
	return ret;
}


/**********************************************************************************/
/*Description: get product upgrade port                                           */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : port number                                                        */
/**********************************************************************************/
int ipanel_porting_get_upgrade_port(void)
{
	return 1024;
}


/**********************************************************************************/
/*Description: set product upgrade port                                           */
/*Input      : port number                                                        */
/*Output     : no                                                                 */
/*Return     : 0 --- success, -1 --- fail                                         */
/**********************************************************************************/
int ipanel_porting_set_upgrade_port(int port)
{
	int ret = -1;
	
	return ret;
}


/**********************************************************************************/
/*Description: get product upgrade user name, string no more the 16 bytes         */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : name string                                                        */
/**********************************************************************************/
const char *ipanel_porting_get_upgrade_user(void)
{
	return "user";
}


/**********************************************************************************/
/*Description: set product upgrade user name                                      */
/*Input      : string no more the 16 bytes                                        */
/*Output     : no                                                                 */
/*Return     : 0 --- success, -1 --- fail                                         */
/**********************************************************************************/
int ipanel_porting_set_upgrade_user(const char *usr)
{
	int ret = -1;
	
	if (strlen(usr) <= 16)
	{
    //   ret = set_upgrade_password(usr);	
	}
	
	return ret;
}


/**********************************************************************************/
/*Description: get product upgrade user password, string no more the 16 bytes     */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : password string                                                    */
/**********************************************************************************/
const char *ipanel_porting_get_upgrade_password(void)
{
	return "123456";
}


/**********************************************************************************/
/*Description: set product upgrade user password                                  */
/*Input      : string no more the 16 bytes                                        */
/*Output     : no                                                                 */
/*Return     : 0 --- success, -1 fail                                             */
/**********************************************************************************/
int ipanel_porting_set_upgrade_password(const char *pwd)
{
	int ret = -1;
	
	if (strlen(pwd) <= 16)
	{
    //   ret = set_upgrade_password;	
	}
	
	return ret;
}

/**********************************************************************************/
/*Description: config ip information                                              */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : 0 --- success, -1 fail                                             */
/**********************************************************************************/
int ipanel_porting_system_ipconfig_all(void)
{
	/*config ip information here.*/
	return 0;
}

/**********************************************************************************/
/*Description: flush dns ip information                                           */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : 0 --- success, -1 fail                                             */
/**********************************************************************************/
int ipanel_porting_system_ipconfig_flushdns(void)
{
	/*flush dns ip information here.*/
	return 0;
}

/**********************************************************************************/
/*Description: Register dns ip information                                        */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : 0 --- success, -1 fail                                             */
/**********************************************************************************/
int ipanel_porting_system_ipconfig_registerdns(void)
{
	/*Register dns ip information here.*/
	return 0;
}

/**********************************************************************************/
/*Description: Release ip information                                             */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : 0 --- success, -1 fail                                             */
/**********************************************************************************/
int ipanel_porting_system_ipconfig_release(void)
{
	/*Release ip information here.*/
	return 0;
}

/**********************************************************************************/
/*Description: Renew config ip information                                        */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : 0 --- success, -1 fail                                             */
/**********************************************************************************/
int ipanel_porting_system_ipconfig_renew(void)
{
	/*Reconfig ip information here.*/
	return 0;
}

/**********************************************************************************/
/*Description: Save ip config information                                         */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : 0 --- success, -1 fail                                             */
/**********************************************************************************/
int ipanel_porting_system_ipconfig_save(void)
{
	/*Save ip config information here.*/
	return 0;
}

/**********************************************************************************/
/*Description: Reboot STB after s seconds                                         */
/*Input      : time (uints is seconds)                                            */
/*Output     : no                                                                 */
/*Return     : 0 --- success, -1 fail                                             */
/**********************************************************************************/
int ipanel_porting_system_reboot(int s)
{
	/*sleep(s);
	rebootsystem();*/
	return 0;
}

/**********************************************************************************/
/*Description: Update STB after s seconds                                         */
/*Input      : time (uints is seconds)                                            */
/*Output     : no                                                                 */
/*Return     : 0 --- success, -1 fail                                             */
/**********************************************************************************/
int ipanel_porting_system_upgrade(int s)
{
	/*sleep(s);
	updatesystem();*/
	return 0;
}



/**********************************************************************************/
/*Description: Get remote control                                                 */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : remote control string                                              */
/**********************************************************************************/
const char *ipanel_porting_get_remote_control(void)
{
	return "Sony-2";
}


/**********************************************************************************/
/*Description: Get Remote Keyboard                                                */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : remote Keyboard string                                             */
/**********************************************************************************/
const char *ipanel_porting_get_remote_keyboard(void)
{
	return "US-1";
}

/**********************************************************************************/
/*Description: Get administrator password                                         */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : administrator password                                             */
/**********************************************************************************/
const char *ipanel_porting_get_admin_password(void)
{
	return "123456";
}


/**********************************************************************************/
/*Description: Set Administrator password                                         */
/*Input      : Administrator password                                             */
/*Output     : no                                                                 */
/*Return     : 0 --- success, -1 fail                                             */
/**********************************************************************************/
int ipanel_porting_set_admin_password(const char *pwd)
{
	/*set administrator password here*/
	return 0;
}



/**********************************************************************************/
/*Description: Get administrator user name                                        */
/*Input      : no                                                                 */
/*Output     : no                                                                 */
/*Return     : administrator user name                                            */
/**********************************************************************************/
const char *ipanel_porting_get_admin_user(void)
{
	return "ipanel";
}



/**********************************************************************************/
/*Description: Set Administrator Usr Name                                         */
/*Input      : Administrator Usr Name                                             */
/*Output     : no                                                                 */
/*Return     : 0 --- success, -1 fail                                             */
/**********************************************************************************/
int ipanel_porting_set_admin_user(const char *usr)
{
	/*set administrator User Name here*/
	return 0;
}


int ipanel_porting_hotkey_add(int number, const char *url)
{
	return 0;
}

int ipanel_porting_hotkey_clean(void)
{
	return 0;
}

int ipanel_porting_hotkey_del(int number)
{
	return 0;
}
int ipanel_porting_hotkey_edit(int number)
{
	return 0;
}

int ipanel_porting_hotkey_go(int number)
{
	return 0;
}

int ipanel_porting_hotkey_list(void)
{
	return 0;
}

int ipanel_porting_hotkey_recall(void)
{
	return 0;
}

int ipanel_porting_hotkey_save(void)
{
	return 0;
}

int ipanel_porting_protocol_processor(const char *protocolUrl)
{
//	dprintf("[ipanel_porting_protocol_processor] called..\n");
//	return 0;
        dprintf("[ipanel_porting_protocol_processor] called..\n"); 
#ifdef NL_BZ 
        return STARVOD_ProtocolProcessor(protocolUrl); 
#else 
        return 0; 
#endif 

}

int	ipanel_porting_device_read(const char * params,char * buf,int length)
{
	dprintf("[ipanel_porting_device_read] called..\n");
	return 0;	
}

int ipanel_porting_device_write(const char * params,char * buf,int length)
{
	dprintf("[ipanel_porting_device_write] called..\n");
	return 0;
}

int ipanel_porting_get_smartcard_id(char * buf,int len)
{
	strcpy(buf,"smartcard_id");
	return strlen(buf);
}

const char *ipanel_porting_get_helppage(void)
{
	return "help.htm"; 
}

const char *ipanel_porting_get_STBapp_version(void)
{
	return "ipanel.2008.02.20.00.00.00";
}

const char *ipanel_porting_get_ca_name(void)
{
	return "12";
}
const char *ipanel_porting_get_ca_version(void)
{
	return "12";
}
const char *ipanel_porting_get_ca_provider(void)
{
	return "12";
}

