/*********************************************************************
    Copyright (c) 2005 Embedded Internet Solutions, Inc
    All rights reserved. You are not allowed to copy or distribute
    the code without permission.
    There are the NETWORK Porting APIs needed by iPanel MiddleWare. 
    
    Note: the "int" in the file is 32bits
    
    $ver0.0.0.1 $author Zouxianyun 2005/04/28
*********************************************************************/

#ifndef _IPANEL_MIDDLEWARE_PORTING_SOCKET_API_FUNCTOTYPE_H_
#define _IPANEL_MIDDLEWARE_PORTING_SOCKET_API_FUNCTOTYPE_H_

#ifdef __cplusplus
extern "C" {
#endif

/*Get max socket count*/
int ipanel_porting_get_max_sockets(void);

/*Open socket, the function is belong to socket part.*/
int ipanel_porting_socket_open(int ip, int port, int type);

/*Receive data from socket, the function is belong to socket part.*/
int ipanel_porting_socket_recv(int s, char *buffer, int buflen);

/*Send data, the function is belong to socket part.*/
int ipanel_porting_socket_send(int s, const char *buffer, int buflen);

/*Close socket, the function is belong to socket part.*/
int ipanel_porting_socket_close(int s);

/*poll socket, the function is belong to socket part.*/
int ipanel_porting_socket_ontimer(unsigned int event[3]);

/*Network initialize*/
void ipanel_porting_network_init(void);

/*free network resoure*/
void ipanel_porting_network_exit(void);

/*get network status.*/
int ipanel_porting_get_network_status(void);

/*
int ipanel_porting_network_connect(const char *networkName);
int ipanel_porting_network_disconnect(const char *networkName);
int ipanel_porting_network_get_parameter(const char *networkName, const char *name, char *buf, int len);
int ipanel_porting_network_set_parameter(const char *networkName, const char *name, const char *val);
*/

#ifdef __cplusplus
}
#endif

#endif
