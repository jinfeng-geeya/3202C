/*********************************************************************
    Copyright (c) 2005 Embedded Internet Solutions, Inc
    All rights reserved. You are not allowed to copy or distribute
    the code without permission.
    This is the demo implenment of the Url Network and Cursor Porting APIs needed by iPanel MiddleWare. 
    Maybe you should modify it accorrding to Platform.
    
    Note: the "int" in the file is 32bits
    
    $ver0.0.0.1 $author Zouxianyun 2005/04/28
*********************************************************************/
#include <basic_types.h>
#include <udi/ipanel/ipanel_UrlNetCursor.h>


/**********************************************************************************/
/*Description: iWM Get Start Page                                                 */
/*Input      : No                                                                 */
/*Output     : No                                                                 */
/*Return     : start page string                                                  */
/**********************************************************************************/
const char *ipanel_porting_get_startpage(void)
{	
  return NULL;
}

/**********************************************************************************/
/*Description: iWM Get Home Page                                                  */
/*Input      : No                                                                 */
/*Output     : No                                                                 */
/*Return     : home page string                                                   */
/**********************************************************************************/
const char *ipanel_porting_get_homepage(void)
{
	return NULL;
}


/**********************************************************************************/
/*Description: Connect network device                                             */
/*Input      : device name string                                                 */
/*Output     : No                                                                 */
/*Return     : 0 -- connect success, -1 -- fail                                   */
/**********************************************************************************/
int ipanel_porting_network_connect(const char *devname)
{
	int ret = -1;

	if (!strcmp(devname, "lan") || !strcmp(devname, "dialup") || !strcmp(devname, "pppoe")) {
		/*connect device*/
		/*if success or had connected*/
		ret = 0;
	}

	return ret;
}


/**********************************************************************************/
/*Description: Disconnect network device                                          */
/*Input      : device name string                                                 */
/*Output     : No                                                                 */
/*Return     : 0 -- disconnect success, -1 -- fail                                */
/**********************************************************************************/
int ipanel_porting_network_disconnect(const char *devname)
{
	int ret = -1;

	if (!strcmp(devname, "lan") || !strcmp(devname, "dialup") || !strcmp(devname, "pppoe")) {
		/*disconnect device*/
		/*if success or had connected*/
		ret = 0;
	}

	return ret;
}


/**********************************************************************************/
/*Description: Get network parameter                                              */
/*Input      : device name, property name, buffer&length to save parameter        */
/*Output     : parameter                                                          */
/*Return     : parameter length, or 0                                             */
/**********************************************************************************/
int ipanel_porting_network_get_parameter(const char *devname, const char *name, char *buf, int len)
{
	int nbytes = 0;

	if (!strcmp(devname, "lan")) {
		if (!strcmp(name, "status")) {
			if(len>=7) {
				memcpy(buf, "connect", 7);
				nbytes = 7;
			}
			/*or 
			if(len>=10) {
				memcpy(buf,"disconnect", 10);
				nbytes = 10;
			}
			*/
		}
		else if (!strcmp(name, "ipaddr")) {
			if(len>=14) {
				memcpy(buf, "192.168.10.121", 14);
				nbytes = 14;
			}
		}
		else if (!strcmp(name, "netmask")) {
			if(len>=15) {
				memcpy(buf, "255.255.255.128", 15);
				nbytes = 15;
			}
		}
		else if (!strcmp(name, "gateway")) {
			if(len>=12) {
				memcpy(buf, "192.168.10.1", 12);
				nbytes = 12;
			}
		}
		else if (!strcmp(name, "nameserver")) {
			if(len>=14) {
				memcpy(buf, "192.168.10.241", 14);
				nbytes = 14;
			}
		}
		else if (!strcmp(name, "nameserver2")) {
			if(len>=14) {
				memcpy(buf, "192.168.10.242", 14);
				nbytes = 14;
			}
		}
		else if (!strcmp(name, "dhcp")) {
			if(len>=1) {
				memcpy(buf, "1", 1);
				nbytes = 1;
			}
			/*or 
			if(len>=1) {
				memcpy(buf,"0", 1);
				nbytes = 1;
			}
			*/
		}
		else if (!strcmp(name, "proxy")) {
			if(len>=24) {
				memcpy(buf, "http://192.168.10.1:8080", 24);
				nbytes = 24;
			}
		}
		else if (!strcmp(name, "proxyflag")) {
			if(len>=1) {
				memcpy(buf, "1", 1);
				nbytes = 1;
			}
			/*or 
			if(len>=1) {
				memcpy(buf,"0", 1);
				nbytes = 1;
			}
			*/
		}
		else if (!strcmp(name, "username")) {
			if(len>=6) {
				memcpy(buf, "ipanel", 6);
				nbytes = 6;
			}
		}
		else if (!strcmp(name, "password")) {
			if(len>=6) {
				memcpy(buf, "123456", 6);
				nbytes = 6;
			}
		}
		else if (!strcmp(name, "unproxyurls")) {
			if(len>=29) {
				memcpy(buf, "http://192.168.10.1/proxy.htm", 29);
				nbytes = 29;
			}
		}
	}
	else if (!strcmp(devname, "dialup")) {
		if (!strcmp(name, "status")) {
			if(len>=7) {
				memcpy(buf, "connect", 7);
				nbytes = 7;
			}
			/*or 
			if(len>=10) {
				memcpy(buf,"disconnect", 10);
				nbytes = 10;
			}
			  or 
			idle, dialing, no dialtone, 
			dialing, no modem connection, 
			line busy, PPP failed, disconnecting,
			PPP Handshaking, connection dropped, delay error
			*/
		}
		else if (!strcmp(name, "username")) {
			if(len>=6) {
				memcpy(buf, "ipanel", 6);
				nbytes = 6;
			}
		}
		else if (!strcmp(name, "password")) {
			if(len>=6) {
				memcpy(buf, "123456", 6);
				nbytes = 6;
			}
		}
		else if (!strcmp(name, "phonenumber")) {
			if(len>=11) {
				memcpy(buf, "075582374586", 11);
				nbytes = 11;
			}
		}
		else if (!strcmp(name, "nameserver")) {
			if(len>=14) {
				memcpy(buf, "192.168.10.241", 14);
				nbytes = 14;
			}
		}
		else if (!strcmp(name, "nameserver2")) {
			if(len>=14) {
				memcpy(buf, "192.168.10.242", 14);
				nbytes = 14;
			}
		}
		else if (!strcmp(name, "idletime")) {
			if(len>=3) {
				memcpy(buf, "300", 3);
				nbytes = 3;
			}
		}
		else if (!strcmp(name, "dialcount")) {
			if(len>=2) {
				memcpy(buf, "10", 2);
				nbytes = 2;
			}
		}
		else if (!strcmp(name, "speed")) {
			if(len>=6) {
				memcpy(buf, "128000", 6);
				nbytes = 6;
			}
		}
		else if (!strcmp(name, "atcommand")) {
			if(len>=3) {
				memcpy(buf, "atd", 3);
				nbytes = 3;
			}
		}
		else if (!strcmp(name, "proxy")) {
			if(len>=24) {
				memcpy(buf, "http://192.168.10.1:8080", 24);
				nbytes = 24;
			}
		}
		else if (!strcmp(name, "proxyflag")) {
			if(len>=1) {
				memcpy(buf, "1", 1);
				nbytes = 1;
			}
			/*or 
			if(len>=1) {
				memcpy(buf,"0", 1);
				nbytes = 1;
			}
			*/
		}
	}
	else if (!strcmp(devname, "pppoe")) {
		if (!strcmp(name, "status")) {
			if(len>=7) {
				memcpy(buf, "connect", 7);
				nbytes = 7;
			}
			/*or 
			if(len>=10) {
				memcpy(buf,"disconnect", 10);
				nbytes = 10;
			}
			*/
		}
		else if (!strcmp(name, "username")) {
			if(len>=6) {
				memcpy(buf, "ipanel", 6);
				nbytes = 6;
			}
		}
		else if (!strcmp(name, "password")) {
			if(len>=6) {
				memcpy(buf, "123456", 6);
				nbytes = 6;
			}
		}
		else if (!strcmp(name, "timeout")) {
			if(len>=3) {
				memcpy(buf, "900", 3);
				nbytes = 3;
			}
		}
	}

	return nbytes;
}


/**********************************************************************************/
/*Description: Set network parameter                                              */
/*Input      : device name, property name, parameter                              */
/*Output     : No                                                                 */
/*Return     : 0  -- success, -1 -- fail.                                         */
/**********************************************************************************/
int ipanel_porting_network_set_parameter(const char *devname, const char *name, const char *val)
{
	int ret = -1;

	if (!strcmp(devname, "lan")) {
		if (!strcmp(name, "ipaddr") && strlen(val)<=40) {
			/*set physical ip address*/
			ret = 0;
		}
		else if (!strcmp(name, "netmask") && strlen(val)<=40) {
			/*set physical ip netmask*/
			ret = 0;
		}
		else if (!strcmp(name, "gateway") && strlen(val)<=40) {
			/*set physical ip gateway*/
			ret = 0;
		}
		else if (!strcmp(name, "nameserver") && strlen(val)<=40) {
			/*set physical ip nameserver*/
			ret = 0;
		}
		else if (!strcmp(name, "nameserver2") && strlen(val)<=40) {
			/*set physical ip nameserver2*/
			ret = 0;
		}
		else if (!strcmp(name, "dhcp") && strlen(val)<=8) {
			/*set physical DHCP is effect or not*/
			ret = 0;
		}
		else if (!strcmp(name, "proxy") && strlen(val)<=40) {
			/*set physical ip proxy*/
			ret = 0;
		}
		else if (!strcmp(name, "proxyflag") && strlen(val)<=8) {
			/*set physical PROXY is effect or not*/
			ret = 0;
		}
		else if (!strcmp(name, "username") && strlen(val)<=40) {
			/*set physical ip user name*/
			ret = 0;
		}
		else if (!strcmp(name, "password") && strlen(val)<=40) {
			/*set physical ip user password*/
			ret = 0;
		}
		else if (!strcmp(name, "unproxyurls") && strlen(val)<=250) {
			/*set physical ip user proxy url page*/
			ret = 0;
		}
	}
	else if (!strcmp(devname, "dialup")) {
		if (!strcmp(name, "username") && strlen(val)<=40) {
			/*set physical ip user name*/
			ret = 0;
		}
		else if (!strcmp(name, "password") && strlen(val)<=40) {
			/*set physical ip user password*/
			ret = 0;
		}
		else if (!strcmp(name, "phonenumber") && strlen(val)<=40) {
			/*set physical ip user phonenumber*/
			ret = 0;
		}
		else if (!strcmp(name, "nameserver") && strlen(val)<=40) {
			/*set physical ip nameserver*/
			ret = 0;
		}
		else if (!strcmp(name, "nameserver2") && strlen(val)<=40) {
			/*set physical ip nameserver2*/
			ret = 0;
		}
		else if (!strcmp(name, "idletime") && strlen(val)<=40) {
			/*set physical idle time, uint is second*/
			ret = 0;
		}
		else if (!strcmp(name, "dialcount") && strlen(val)<=40) {
			/*set physical dial count*/
			ret = 0;
		}
		else if (!strcmp(name, "proxy") && strlen(val)<=40) {
			/*set physical ip proxy*/
			ret = 0;
		}
		else if (!strcmp(name, "proxyflag") && strlen(val)<=8) {
			/*set physical PROXY is effect or not*/
			ret = 0;
		}
	}
	else if (!strcmp(devname, "pppoe")) {
		if (!strcmp(name, "username") && strlen(val)<=40) {
			/*set physical ip user name*/
			ret = 0;
		}
		else if (!strcmp(name, "password") && strlen(val)<=40) {
			/*set physical ip user password*/
			ret = 0;
		}
		else if (!strcmp(name, "timeout") && strlen(val)<=40) {
			/*set physical timeout*/
			ret = 0;
		}
	}

	return ret;
}



/**********************************************************************************/
/*Description: Get cursor position, Note the origin position is in the left and   */
/*             top of the screen .                                                */
/*Input      : the address to save x and y.                                       */
/*Output     : x and y value                                                      */
/*Return     : No                                                                 */
/**********************************************************************************/
void ipanel_porting_cursor_get_position(int *x, int *y)
{
	#define SCREENWIDTH (640)
	#define SCREENHEIGTH (480)

	/*the default position of cursor is in center of the TV screen.*/
	*x = SCREENWIDTH/2;
	*y = SCREENHEIGTH/2;
}



/**********************************************************************************/
/*Description: Set cursor position, Note the origin position is in the left and   */
/*             top of the screen .                                                */
/*Input      : x and y.                                                           */
/*Output     : No                                                                 */
/*Return     : No                                                                 */
/**********************************************************************************/
void ipanel_porting_cursor_set_position(int x, int y)
{
	#define SCREENWIDTH (640)
	#define SCREENHEIGTH (480)

	if (x>=0 && x<=SCREENWIDTH && y>=0 && y<=SCREENHEIGTH) {
		/*set physical cursor position here*/
	}
}

/**********************************************************************************/
/*Description: iWM Get Cursor(1~4)shape: 1)arrow head 2)hand 3)hourglass 4)I type */
/*Input      : No                                                                 */
/*Output     : No                                                                 */
/*Return     : Cursor(1~4)shape                                                   */
/**********************************************************************************/
int ipanel_porting_cursor_get_shape(void)
{
	return 1; /*arrow head is the default value*/
	/*you can use following code
	return 2; //hand
	or
	return 3; //hourglass
	or
	return 4; //I type
	*/
}


/**********************************************************************************/
/*Description: Set Cursor(1~4)shape: 1)arrow head 2)hand 3)hourglass 4)I type     */
/*Input      : Cursor(1~4)shape                                                   */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
int ipanel_porting_cursor_set_shape(int shape)
{
	int ret = -1;

	if (shape>0 && shape<=4) {
		/*Set physical cursor here.*/
		ret = 0;
	}

	return ret;
}



