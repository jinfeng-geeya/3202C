/*********************************************************************
    Copyright (c) 2005 Embedded Internet Solutions, Inc
    All rights reserved. You are not allowed to copy or distribute
    the code without permission.
    This is the demo implenment of the Network Porting APIs needed by iPanel MiddleWare.
    Maybe you should modify it accorrding to Platform.

    Note: the "int" in the file is 32bits

    $ver0.0.0.1 $author Zouxianyun 2005/04/28
*********************************************************************/

#include <udi/ipanel/ipanel_base.h>
#include <udi/ipanel/ipanel_socket.h>
#include <udi/ipanel/ipanel_porting_event.h>

#define SOCKET              int
#define MAX_SOCKETS         8
#define MAX_EVENTS          128

#ifdef NETWORK_SUPPORT

typedef struct tagSystemSocketManager
{
    int n_idle;
    int sockets[MAX_SOCKETS];
    int maxfds;
    fd_set fds_r, fds_w, fds_e;
    int event_indx_read;
    int event_indx_write;
    int event_socket[MAX_EVENTS];
    int event_type[MAX_EVENTS];
} SystemSocketManager;

static SystemSocketManager *sock_mgr = NULL;

static SystemSocketManager *SystemSocketManager_new(void);
static void SystemSocketManager_delete(SystemSocketManager *me);
static int SystemSocketManager_register(SystemSocketManager *me, int s);
static int SystemSocketManager_unregister(SystemSocketManager *me, int s);
static void SystemSocketManager_put_event(SystemSocketManager *me, int s, int evt);
static int SystemSocketManager_pop_event(SystemSocketManager *me, int *s, int *event);
static int SystemSocketManager_poll(SystemSocketManager *me);
//static void SystemSocketManager_process(void);

static int SystemSocketManager_getSocketByIndx(SystemSocketManager *me, int indx);
static int SystemSocketManager_getIndxBySocket(SystemSocketManager *me, int s);

/******************************************************************/
/*Description: Open a Socket,It'll send(SOCKET_EVENT_TYPE_CONNECT)*/
/*             to iPanel MiddleWare if Connect Successful.        */
/*Input      : IP value, Port, and Type(TCP(0) or UDP(1))         */
/*Output     : No                                                 */
/*Return     : socket value or socket ID.(< 0, is Fail.)          */
/******************************************************************/
int ipanel_porting_socket_open(int ip, int port, int type)
{
    SOCKET m_s;
    struct sockaddr_in  serv_addr;
	struct sockaddr_in in_name;
    struct timeval      time_val = {2, 0};

    int index       = -1;
    int status      = -1;
    int sock_type   = SOCK_STREAM;


    dprintf("[ipanel_porting_socket_open] ip=0x%x port=%d type=%d,\n",
            ip, port, type);

    if (sock_mgr->n_idle <= 0)
    {
        dprintf("[ipanel_porting_socket_open] Socket mgr full\n");
        return -1;
    }

    bzero((char*)(&serv_addr), sizeof(serv_addr));
    serv_addr.sin_len           = sizeof(serv_addr);
    serv_addr.sin_family        = AF_INET;
    serv_addr.sin_addr.s_addr   = ntohl((unsigned int)ip);
    serv_addr.sin_port          = htons(port);

    if (0 == type)
    {
        sock_type = SOCK_STREAM;
    }
    else if (1 == type)
    {
        sock_type = SOCK_DGRAM;
    }

    m_s = socket(AF_INET, sock_type, IPPROTO_TCP);
    if (m_s < 0)
    {
        dprintf("[ipanel_porting_socket_open] CreateSocket Failed\n");
        return -2;
    }

//	status = connectWithTimeout(m_s,(struct sockaddr*)(&serv_addr),sizeof(serv_addr),&time_val );
    status = connect( m_s,(struct sockaddr*)(&serv_addr),sizeof(struct sockaddr));

    if (status == -1)
    {
        libc_printf("[ipanel_porting_socket_open] Socket %d Connect Error\n",m_s);
        closesocket(m_s);
        return -3;
    }
	else
		dprintf("[ipanel_porting_socket_open] Socket Connect success..\n");

    SystemSocketManager_put_event(sock_mgr, m_s, SOCKET_EVENT_TYPE_CONNECT);

    index = SystemSocketManager_register(sock_mgr, m_s);

    return index;
}

/******************************************************************/
/*Description: Send data throught socket.                         */
/*Input      : Socket, Data, and Data Length                      */
/*Output     : No                                                 */
/*Return     : Send real data Length                              */
/******************************************************************/
int ipanel_porting_socket_send(int indx, const char *buf, int len)
{
    int nbytes  = -1;

    SOCKET s    = SystemSocketManager_getSocketByIndx(sock_mgr, indx);

    dprintf("[ipanel_porting_socket_send] indx=%d, s=%d buf=%p len=%d\n",
            indx, s, buf, len);

    if (s >= 0)
    {
        nbytes = send(s, (char*)buf, len, 0);
		dprintf("[ipanel_porting_socket_send] nbytes =%d ..\n",nbytes);
    }

    return nbytes;
}

/******************************************************************/
/*Description: Receive data from socket.                          */
/*Input      : Socket, Buffer, and Buffer Length                  */
/*Output     : Data to buffer                                     */
/*Return     : Receive real data Length                           */
/******************************************************************/
int ipanel_porting_socket_recv(int indx, char *buf, int len)
{
    int nbytes  = -1;

    SOCKET s    = SystemSocketManager_getSocketByIndx(sock_mgr, indx);

    dprintf("[ipanel_porting_socket_recv] indx=%d, s=%d buf=%p len=%d\n",indx, s, buf, len);

    if (s >= 0)
    {
        nbytes = recv(s, buf, len, 0);
        if (nbytes <= 0)
        {
        	dprintf("[ipanel_porting_socket_recv] recv failed or finish.close socket %d.\n",s);
            SystemSocketManager_put_event(sock_mgr, s, SOCKET_EVENT_TYPE_CLOSE);  //have received complete
        }
    }
	dprintf("[ipanel_porting_socket_recv] nbytes : %d.\n",nbytes);
    return nbytes;
}

/******************************************************************/
/*Description: Close a Socket                                     */
/*Input      : Socket                                             */
/*Output     : No                                                 */
/*Return     : 0 -- success, other fail.                          */
/******************************************************************/
int ipanel_porting_socket_close(int indx)
{
    int ret  = -1;

	SOCKET s = SystemSocketManager_getSocketByIndx(sock_mgr, indx);

    dprintf("[ipanel_porting_socket_close] indx=%d, s=%d\n", indx, s);

    if (s >= 0)
    {
        SystemSocketManager_unregister(sock_mgr, s);

        ret = closesocket(s);
    }

    return ret;
}

/******************************************************************/
/*Description: get max sockets                                    */
/*Input      : No                                                 */
/*Output     : No                                                 */
/*Return     : max sockets.                          			  */
/******************************************************************/
int ipanel_porting_get_max_sockets(void)
{
    return MAX_SOCKETS;
}

/******************************************************************************
* Description: Poll Socket Event, and Send Socket Message to BRWS.
*    The following is a description of Browser and Socket Module:
*      Browser Module(BRWS)                 Socket Module
*    1) Http Request Call              Poll or Select all Socket.
*       "eis_socket_open()";
*    2)                                When Connected success,
*                                      Send CONNECT message to BRWS;
*    3) When Receive CONNECT Message,
*       Call "eis_socket_send()" to send data;
*    4)                                When have data on socket,
*                                     Send READ message to BRWS;
*    5) When Receive READ Message,
*       Call "eis_socket_recv()" to receive data;
*    6)                                 When error on socket,
*                                     Send ERROR message to BRWS;
*    7) process socket manager
*    8)                                 When no data on socket,
*                                      Send CLOSE message to BRWS
*    9) When Receive CLOSE Message,
*
*       Call "eis_socket_close()" to close a close.
* Input      : Event Array[3]
* Output     : event value
* 1) event[0] is iPanel Event Message Type, as following value.
* enum {
*     EIS_EVENT_TYPE_NETWORK = 4
* };
* 2) event[1] is the socket value or socket ID.
* 3) event[2] is the socket status, as following enum value.
* enum {
*     SOCKET_EVENT_TYPE_CONNECT = 1,
*     SOCKET_EVENT_TYPE_CLOSE,
*     SOCKET_EVENT_TYPE_READ,
*     SOCKET_EVENT_TYPE_WRITE,
*     SOCKET_EVENT_TYPE_ACCEPT,
*     SOCKET_EVENT_TYPE_READ_SHUTDOWN,
*     SOCKET_EVENT_TYPE_WRITE_SHUTDOWN,
*     SOCKET_EVENT_TYPE_ERROR
* };
* Return     : 1 -- have Event, 0 -- no Event.
*******************************************************************************/
int ipanel_porting_socket_ontimer(unsigned int event[3])
{
    int eventCode;

    if (sock_mgr->n_idle >= MAX_SOCKETS)
    {
        return 0;
    }

    eventCode = SystemSocketManager_pop_event(sock_mgr, &event[2], &event[1]);

    if (eventCode == 0)
    {
        SystemSocketManager_poll(sock_mgr);
        eventCode = SystemSocketManager_pop_event(sock_mgr, &event[2], &event[1]);
    }
    if (eventCode != 0)
    {
        int i = event[2] - 1;
        event[0] = EIS_EVENT_TYPE_NETWORK;
        return 1;
    }
    else
    {
        event[0] = 0;
        return 0;
    }

	return 0;
}


/* end new added socket porting interface */

static SystemSocketManager *SystemSocketManager_new(void)
{
    int i;
    SystemSocketManager *me;

   me = (SystemSocketManager*)MALLOC(sizeof(SystemSocketManager));
//	me = (SystemSocketManager*)ipanel_porting_malloc(sizeof(SystemSocketManager));
    memset(me, 0, sizeof(SystemSocketManager));
    FD_ZERO(&me->fds_r);
    FD_ZERO(&me->fds_w);
    FD_ZERO(&me->fds_e);
    for (i = 0; i < MAX_SOCKETS; i++)
    {
        me->sockets[i] = -1;
    }
    me->n_idle = MAX_SOCKETS;

    return me;
}

static void SystemSocketManager_delete(SystemSocketManager *me)
{
    int i;

    for (i = 0; i < MAX_SOCKETS; i++)
    {
        if (me->sockets[i] != -1)
        {
            closesocket(me->sockets[i]);
        }
    }
    FREE(me);
//	ipanel_porting_free(me);
}

static int SystemSocketManager_register(SystemSocketManager *me, int s)
{
    int i, indx = -1;

    for (i = 0; i < MAX_SOCKETS; i++)
    {
        if (me->sockets[i] == -1)
        {
            me->sockets[i] = s;
            if (me->maxfds < s)
                me->maxfds = s;
            FD_SET(s, &me->fds_r);
            FD_SET(s, &me->fds_w);
            me->n_idle--;
            indx = i + 1;
            break;
        }
    }

    return indx;
}

static int SystemSocketManager_unregister(SystemSocketManager *me, int s)
{
    int i, indx = -1;

    for (i = 0; i < MAX_SOCKETS; i++)
    {
        if (me->sockets[i] == s)
        {
            me->sockets[i] = -1;
            FD_CLR(s, &me->fds_r);
            FD_CLR(s, &me->fds_w);
            me->n_idle++;
            indx = i + 1;
            break;
        }
    }

    return indx;
}

static void SystemSocketManager_put_event(SystemSocketManager *me, int s, int evt)
{
    int indx = me->event_indx_write;

    if ((indx + 1) % MAX_EVENTS == me->event_indx_read)
    {
        dprintf("[SystemSocketManager_put_event] lose event, queque overflow!\n");
        return;
    }
    me->event_socket[indx]  = s;
    me->event_type[indx]    = evt;
    indx++;
    if (indx >= MAX_EVENTS)
        indx = 0;
    me->event_indx_write = indx;
}

static int SystemSocketManager_pop_event(SystemSocketManager *me,
                                                       int *indx,
                                                      int *event)
{
    int ret = 0;

    if (me->event_indx_read != me->event_indx_write)
    {
        int i = me->event_indx_read;

        //*s = me->event_socket[i];
        *indx  = SystemSocketManager_getIndxBySocket(me, me->event_socket[i]);
        *event = me->event_type[i];
        me->event_socket[i]  = 0;
        me->event_type[i]    = 0;
        me->event_indx_read++;
        if (me->event_indx_read >= MAX_EVENTS)
            me->event_indx_read = 0;
        ret = 1;
    }

    return ret;
}

static int SystemSocketManager_poll(SystemSocketManager *me)
{
    int nactive;
    struct timeval tm = {0, 700};
    fd_set fds_r = me->fds_r;
    fd_set fds_e;

    FD_ZERO(&fds_e);
    dprintf("[socket poll] fds_r %x,%x\n",fds_r.fd_bits[0],fds_r.fd_bits[1]);
    nactive = select(me->maxfds + 1, &fds_r, NULL, NULL, &tm);
    dprintf("[socket poll] after select fds_r:%x,%x.nactive:%d..\n",fds_r.fd_bits[0],fds_r.fd_bits[1],nactive);
    if (nactive > 0)
    {
        int i;
        SOCKET m_s;

        for (i = 0; i < MAX_SOCKETS; i++)
        {
            m_s = me->sockets[i];
            if (-1 == m_s)
            {
                continue;
            }

            if (FD_ISSET(m_s, &fds_r))
            {
                SystemSocketManager_put_event(me, m_s, SOCKET_EVENT_TYPE_READ);
            }
        }
    }

    return nactive;
}

static int SystemSocketManager_getSocketByIndx(SystemSocketManager *me, int indx)
{

    int ret = -1;

    if ((indx > 0) && (indx <= MAX_SOCKETS))
    {
        ret = me->sockets[indx - 1];
//		dprintf("[SystemSocketManager_getSocketByIndx] index=%d,socket=%d..\n",myindx,ret);
    }
	
    return ret;
}

static int SystemSocketManager_getIndxBySocket(SystemSocketManager *me, int s)
{
    int i, indx = -1;

    for (i = 0; i < MAX_SOCKETS; i++)
    {
        if (s == me->sockets[i])
        {
            indx = i + 1;
            break;
        }
    }

    return indx;
}

void ipanel_porting_network_init(void)
{
	sock_mgr = NULL;
	ipanel_porting_dprintf("[ipanel_porting_network_init] called..\n");
    sock_mgr = SystemSocketManager_new();
}

void ipanel_porting_network_exit(void)
{
	ipanel_porting_dprintf("[ipanel_porting_network_exit] called..\n");
    SystemSocketManager_delete(sock_mgr);
    sock_mgr = NULL;
}

int ipanel_porting_get_network_status(void)
{
	return 1;
}
#else

/******************************************************************/
/*Description: Open a Socket,It'll send(SOCKET_EVENT_TYPE_CONNECT)*/
/*             to iPanel MiddleWare if Connect Successful.        */
/*Input      : IP value, Port, and Type(TCP(0) or UDP(1))         */
/*Output     : No                                                 */
/*Return     : socket value or socket ID.(< 0, is Fail.)          */
/******************************************************************/
int ipanel_porting_socket_open(int ip, int port, int type)
{
	return -1;
}

/******************************************************************/
/*Description: Send data throught socket.                         */
/*Input      : Socket, Data, and Data Length                      */
/*Output     : No                                                 */
/*Return     : Send real data Length                              */
/******************************************************************/
int ipanel_porting_socket_send(int indx, const char *buf, int len)
{
	int nbytes  = -1;
	return nbytes;
}

/******************************************************************/
/*Description: Receive data from socket.                          */
/*Input      : Socket, Buffer, and Buffer Length                  */
/*Output     : Data to buffer                                     */
/*Return     : Receive real data Length                           */
/******************************************************************/
int ipanel_porting_socket_recv(int indx, char *buf, int len)
{
	int nbytes  = -1;
	return nbytes;
}

/******************************************************************/
/*Description: Close a Socket                                     */
/*Input      : Socket                                             */
/*Output     : No                                                 */
/*Return     : 0 -- success, other fail.                          */
/******************************************************************/
int ipanel_porting_socket_close(int indx)
{
	int ret  = -1;

	return ret;
}

/******************************************************************/
/*Description: get max sockets                                    */
/*Input      : No                                                 */
/*Output     : No                                                 */
/*Return     : max sockets.                          			  */
/******************************************************************/
int ipanel_porting_get_max_sockets(void)
{
    	return MAX_SOCKETS;
}

/******************************************************************************
* Description: Poll Socket Event, and Send Socket Message to BRWS.
*    The following is a description of Browser and Socket Module:
*      Browser Module(BRWS)                 Socket Module
*    1) Http Request Call              Poll or Select all Socket.
*       "eis_socket_open()";
*    2)                                When Connected success,
*                                      Send CONNECT message to BRWS;
*    3) When Receive CONNECT Message,
*       Call "eis_socket_send()" to send data;
*    4)                                When have data on socket,
*                                     Send READ message to BRWS;
*    5) When Receive READ Message,
*       Call "eis_socket_recv()" to receive data;
*    6)                                 When error on socket,
*                                     Send ERROR message to BRWS;
*    7) process socket manager
*    8)                                 When no data on socket,
*                                      Send CLOSE message to BRWS
*    9) When Receive CLOSE Message,
*
*       Call "eis_socket_close()" to close a close.
* Input      : Event Array[3]
* Output     : event value
* 1) event[0] is iPanel Event Message Type, as following value.
* enum {
*     EIS_EVENT_TYPE_NETWORK = 4
* };
* 2) event[1] is the socket value or socket ID.
* 3) event[2] is the socket status, as following enum value.
* enum {
*     SOCKET_EVENT_TYPE_CONNECT = 1,
*     SOCKET_EVENT_TYPE_CLOSE,
*     SOCKET_EVENT_TYPE_READ,
*     SOCKET_EVENT_TYPE_WRITE,
*     SOCKET_EVENT_TYPE_ACCEPT,
*     SOCKET_EVENT_TYPE_READ_SHUTDOWN,
*     SOCKET_EVENT_TYPE_WRITE_SHUTDOWN,
*     SOCKET_EVENT_TYPE_ERROR
* };
* Return     : 1 -- have Event, 0 -- no Event.
*******************************************************************************/
int ipanel_porting_socket_ontimer(unsigned int event[3])
{
	return 0;
}

void ipanel_porting_network_init(void)
{
}

void ipanel_porting_network_exit(void)
{
}

int ipanel_porting_get_network_status(void)
{
	return 1;
}

#endif


