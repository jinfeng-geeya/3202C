/*!
	* \file 
	*/
/*********************************************************************
    Copyright (c) 2005 Embedded Internet Solutions, Inc
    All rights reserved. You are not allowed to copy or distribute
    the code without permission.
    There are the Url and Network and Cursor Porting APIs needed by iPanel MiddleWare. 
    Note: the "int" in the file is 32bits
    $ver0.0.0.1 $author Zouxianyun 2005/04/28
*********************************************************************/
#ifndef _IPANEL_MIDDLEWARE_PORTING_URL_NET_CURSOR_API_FUNCTOTYPE_H_
#define _IPANEL_MIDDLEWARE_PORTING_URL_NET_CURSOR_API_FUNCTOTYPE_H_
#ifdef __cplusplus
extern "C" {
#endif
/*get start page*/
const char *ipanel_porting_get_startpage(void);
/*set start page*/
/*!  
 *  \brief 设置首页信息，该函数iPanel MiddleWare可能会通过网页中的JavaScript程序调用。
 *  \author
 *  @param[in] 首页的url型字符串；
 *  @param[out] 无
 *  \return 0 – 成功，-1 – 失败
 *  \version 
 *  \date     
 *  \warning 
 *  \sa ipanel_UrlNetCursor_demo.c
  *  \deprecated
 */
int ipanel_porting_set_startpage(const char *url);
/*get home page*/
const char *ipanel_porting_get_homepage(void);
/*set home page*/
/*!  
 *  \brief 设置主页信息，该函数iPanel MiddleWare可能会通过网页中的JavaScript程序调用。
 *  \author
 *  @param[in] 首页的url型字符串；
 *  @param[out] 无
 *  \return 0 – 成功，-1 – 失败
 *  \version 
 *  \date     
 *  \warning 
 *  \sa ipanel_UrlNetCursor_demo.c
  *  \deprecated
 */
int ipanel_porting_set_homepage(const char *url);
/*get help page*/
/*!
 * \brief 			获取帮助页信息，该函数iPanel MiddleWare可能会调用。
 * \author 			
 * \param[in] 	无
 * \param[out]	无
 * \return			帮助页的url型字符串
 * \version			
 * \date				
 * \warning			
 * \sa					iipanel_UrlNetCursor_demo.c
 * \deprecated	
*/ 
const char *ipanel_porting_get_helppage(void);
/*set help page*/
/*!  
 *  \brief 设置帮助页信息，该函数iPanel MiddleWare可能会通过网页中的JavaScript程序调用。
 *  \author
 *  @param[in] 帮助页的url型字符串；
 *  @param[out] 无
 *  \return 0 – 成功，-1 – 失败
 *  \version 
 *  \date     
 *  \warning 
 *  \sa ipanel_UrlNetCursor_demo.c
  *  \deprecated
 */
int ipanel_porting_set_helppage(const char *url);
/*connect network*/
/*!  
 *  \brief 连接网络设备，设备的名字目前仅限于LAN， Dialup，PPPoE三种，对应的字符串是“lan”, “dialup”, “pppoe”。
 *  \author
 *  @param[in] devname 网络设备名称，不在上述三者之列直接返回-1；
 *  @param[out] 无
 *  \return 0 – 连接成功 -1 – 失败或设备不存在
 *  \version 
 *  \date     
 *  \warning 
 *  \sa ipanel_UrlNetCursor_demo.c
 *  \deprecated
 */
int ipanel_porting_network_connect(const char *devname);
/*disconnect network*/
/*!  
 *  \brief 断开网络连接，设备的名字目前仅限于LAN， Dialup，PPPoE三种，对应的字符串是“lan”, “dialup”, “pppoe”。
 *  \author
 *  @param[in] 网络设备名称，不在上述三者之列直接返回-1
 *  @param[out] 无
 *  \return 0 – 断开连接成功或本身就没有连接 -1 – 失败
 *  \version 
 *  \date     
 *  \warning 
 *  \sa ipanel_UrlNetCursor_demo.c
 *  \deprecated
 */
int ipanel_porting_network_disconnect(const char *devname);
/*get network parameter*/
/*!  
 *  \brief 获取网络连接中参数名称叫做name的参数值/属性。Name对应设备的详细范围说明如下：格式：[“设备名”:  “参数名”/“参数值”] \n
 	设备名	参数名	参数值	说明 \n
lan	status	disconnect	没有连接上 \n
		connect	连接上 \n
	ipaddr	型如：xxx.xxx.xxx.xxx  [0-9]	值小于40个字符 \n
	netmask	型如：xxx.xxx.xxx.xxx  [0-9]	值小于40个字符 \n
	gateway	型如：xxx.xxx.xxx.xxx  [0-9]	值小于40个字符 \n
	nameserver	型如：xxx.xxx.xxx.xxx  [0-9]	值小于40个字符 \n
	nameserver2	型如：xxx.xxx.xxx.xxx  [0-9]	值小于40个字符 \n
	dhcp	1	值小于8个字符,有效 \n
		0	值小于8个字符,无效 \n
	proxy	型如：http://xxx.xxx.xxx.xxx:8080	值小于40个字符 \n
	proxyflag	1	值小于8个字符,有效 \n
		0	值小于8个字符,无效 \n
	username		值小于40个字符 \n
	password		值小于40个字符 \n
	unproxyurls		值小于250个字符 \n
dialup	status	Disconnect, idle, dialing, no dialtone, dialing, no modem connection, line busy, PPP failed, disconnecting, 	没有连接上 \n
		connect, PPP Handshaking, connection dropped, delay error	连接上或没有连接上 \n
	username		值小于40个字符 \n
	password		值小于40个字符 \n
	phonenumber	型如：xxxxxxxxxxxx   [0-9]	值小于40个字符 \n
	nameserver	型如：xxx.xxx.xxx.xxx  [0-9]	值小于40个字符 \n
	nameserver2	型如：xxx.xxx.xxx.xxx  [0-9]	值小于40个字符 \n
	idletime	型如：xxxxxxxxxxxx   [0-9]单位s	值小于40个字符 \n
	dialcount	型如：xxxxxxxxxxxx   [0-9]	值小于40个字符 \n
	speed	型如：xxxxxxxxxxxx   [0-9]	值小于40个字符 \n
	atcommand	型如：xxxxxxxx   [0-9 a-z A- Z]	值小于40个字符 \n
	proxy	型如：http://xxx.xxx.xxx.xxx:8080	值小于40个字符 \n
	proxyflag	1	值小于8个字符,有效 \n
		0	值小于8个字符,无效 \n
pppoe	status	disconnect	没有连接上 \n
		connect	连接上 \n
	username		值小于40个字符 \n
	password		值小于40个字符 \n
	timeout	型如：xxxxxxxxxxxx   [0-9]	值小于40个字符 \n
 *  \author
 *  @param[in] devname 网络设备名称
 *  @param[in] name 参数名称字符串
 *  @param[in] buf 存储参数值的缓冲区
 *  @param[in] len 存储参数值的尺寸
 *  @param[out] 参数值
 *  \return 参数的长度
 *  \version 
 *  \date     
 *  \warning 
 *  \sa ipanel_UrlNetCursor_demo.c
 *  \deprecated
 */
int ipanel_porting_network_get_parameter(const char *devname, const char *name, char *buf, int len);
/*set network parameter*/
/*!  
 *  \brief 设置Network 连接中参数名称叫做name的参数值/属性。Name对应设备的详细范围说明如下：格式：[“设备名”:  “参数名”/“参数值”] \n
设备名	参数名	参数值	说明 \n
lan	ipaddr	型如：xxx.xxx.xxx.xxx  [0-9]	值小于40个字符 \n
	netmask	型如：xxx.xxx.xxx.xxx  [0-9]	值小于40个字符 \n
	gateway	型如：xxx.xxx.xxx.xxx  [0-9]	值小于40个字符 \n
	nameserver	型如：xxx.xxx.xxx.xxx  [0-9]	值小于40个字符 \n
	nameserver2	型如：xxx.xxx.xxx.xxx  [0-9]	值小于40个字符 \n
	dhcp	1	值小于8个字符,有效 \n
		0	值小于8个字符,无效 \n
	proxy	型如：http://xxx.xxx.xxx.xxx:8080	值小于40个字符 \n
	proxyflag	1	值小于8个字符,有效 \n
		0	值小于8个字符,无效 \n
	username		值小于40个字符 \n
	password		值小于40个字符 \n
	unproxyurls		值小于250个字符 \n
dialup	username		值小于40个字符 \n
	password		值小于40个字符 \n
	phonenumber	型如：xxxxxxxxxxxx   [0-9]	值小于40个字符 \n
	nameserver	型如：xxx.xxx.xxx.xxx  [0-9]	值小于40个字符 \n
	nameserver2	型如：xxx.xxx.xxx.xxx  [0-9]	值小于40个字符 \n
	idletime	型如：xxxxxxxxxxxx   [0-9]单位s	值小于40个字符 \n
	dialcount	型如：xxxxxxxxxxxx   [0-9]	值小于40个字符 \n
	atcommand	型如：xxxxxxxx   [0-9 a-z A- Z]	值小于40个字符 \n
	proxy	型如：http://xxx.xxx.xxx.xxx:8080	值小于40个字符 \n
	proxyflag	1	值小于8个字符,有效 \n
		0	值小于8个字符,无效 \n
pppoe	username		值小于40个字符 \n
	password		值小于40个字符 \n
	timeout	型如：xxxxxxxxxxxx   [0-9]	值小于40个字符 \n
 *  \author
 *  @param[in] devname 网络设备名称;名字和属性不在上述之列，返回-1
 *  @param[in] name 参数名称字符串;名字和属性不在上述之列，返回-1
 *  @param[in] val 存储参数值/属性;名字和属性不在上述之列，返回-1
 *  @param[out] 无
 *  \return 0 – 成功 -1 – 失败
 *  \version 
 *  \date     
 *  \warning 
 *  \sa ipanel_UrlNetCursor_demo.c
 *  \deprecated
 */
int ipanel_porting_network_set_parameter(const char *devname, const char *name, const char *val);
/*get mouse or cursor position*/
void ipanel_porting_cursor_get_position(int *x, int *y);
/*set mouse or cursor position*/
void ipanel_porting_cursor_set_position(int x, int y);
/*get cursor shape*/
int ipanel_porting_cursor_get_shape(void);
/*set cursor shape*/
int ipanel_porting_cursor_set_shape(int shape);
/*int ipanel_porting_cursor_show(int showflag);*/
#ifdef __cplusplus
}
#endif
#endif
