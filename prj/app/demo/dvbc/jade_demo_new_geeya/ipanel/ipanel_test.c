#include <udi/ipanel/ipanel_base.h>

#define ipanel_base_test_flag   0
#define ipanel_panIR_test_flag   0
#define ipanel_tuner_test_flag   0
#define ipanel_demux_test_flag   0
#define ipanel_avplay_test_flag   0
#define ipanel_gfx_test_flag  0
#define ipanel_pcm_test_flag   0
#define ipanel_socket_test_flag  0
#define ipanel_nvram_test_flag   0

#if ipanel_base_test_flag
void ipanel_base_test()
{
	unsigned int tm = 0;
	void *ptest = NULL;
	unsigned int memsize = 0;
	tm = ipanel_porting_time_ms();
	dprintf("[ipanel_base_test] the first time = %d,\n", tm);
	osal_task_sleep(100);
	tm = ipanel_porting_time_ms();
	dprintf("[ipanel_base_test] the second time = %d,\n\n", tm);

	memsize = 0x200000;
	ptest = ipanel_porting_malloc(memsize);
	if(ptest){
		//		ipanel_porting_free(ptest);
		dprintf("[ipanel_base_test] malloc 0x%x success.\n\n", memsize);
	}
	else
		dprintf("[ipanel_base_test] malloc 0x%x failed.\n\n", memsize);

	memsize = 0x400000;
	ptest = ipanel_porting_malloc(memsize);
	if(ptest){
		//		ipanel_porting_free(ptest);
		dprintf("[ipanel_base_test] malloc 0x%x success.\n\n", memsize);
	}
	else
		dprintf("[ipanel_base_test] malloc 0x%x failed.\n\n", memsize);
#if 1
	memsize = 0x800000;
	ptest = ipanel_porting_malloc(memsize);
	if(ptest){
		//		ipanel_porting_free(ptest);
		dprintf("[ipanel_base_test] malloc 0x%x success.\n\n", memsize);
	}
	else
		dprintf("[ipanel_base_test] malloc 0x%x failed.\n\n", memsize);
#endif
}

#endif

#if ipanel_panIR_test_flag
#include "ipanel_input.h"
void ipanel_panIR_test()
{
	unsigned int event[3] = {0};	
	int ret = 0;
	while(1) {	
		ret = eis_input_get(event);
		if (1 == ret)
		{
			dprintf("[ipanel_panIR_test] get key. event[0]=%d,event[1]=%d.\n\n\n", event[0], event[1]);
		}
		else
		{
			dprintf("[ipanel_panIR_test] get no key. .\n");
		}
		osal_task_sleep(500);
	}
}

#endif

#if ipanel_tuner_test_flag
#include <udi/ipanel/ipanel_tuner.h>
void ipanel_tuner_test()
{
	unsigned int event[3] = {0};	
	int ret = 0;
	//	ipanel_porting_delivery_tune(4350000,68750,3,2);
	while(1) {
		ipanel_porting_delivery_tune(4350000, 68750, 3);
		osal_task_sleep(3000);
		ret = get_tuner_event(event);
		if (1 == ret)
		{
			libc_printf("[ipanel_tuner_test] get tuner event. event[0]=%d,event[1]=%d,event[2]=%d..\n\n\n", event[0], event[1], event[2]);
		}
		else
		{
			libc_printf("[ipanel_tuner_test] get no tuner event. .\n");
		}
		osal_task_sleep(1000);
	}
}

#endif

#if ipanel_demux_test_flag
#include <udi/ipanel/ipanel_demux.h>
extern struct nim_device *g_nim_dev;
//static UINT8 process_buffer[8];
static UINT8 process_buffer[4096];
static int process_flag = 0;
static int copy_len = 0;
static int test_filter = 0;

void ipanel_demux_test()
{
	unsigned char coef[4]={0};
	unsigned char mask[4]={0};
	unsigned char excl[4]={0};
	UINT32 wide = 4;
	UINT32 channelID[16];
	UINT32 filterID[16];
	static UINT32 event[3];
	static UINT32 begin_time = 0, end_time = 0, flag = 0;
	UINT8 lock = 0;
	int i, j;
	dprintf("[ipanel_demux_test] test begin.\n");
	ipanel_porting_demux_init();
	ipanel_porting_delivery_tune(3150000, 68750, 3);
	while(1){
		osal_task_sleep(500); //ms	
		nim_get_lock(g_nim_dev, &lock);
		if(1 == lock){
			dprintf("[ipanel_demux_test] tune locked.\n");
			break;
		}else
			dprintf("[ipanel_demux_test] tune not locked.\n");
	}

	begin_time = time_ms();

	filterID[0] = ipanel_porting_filter_open(0x11, 0x46, 0xff, 0xffff, 0xffff, 0xc1);
	dprintf("[ipanel_demux_test] the filter[0]=0x%x.\n\n", filterID[0]);

	filterID[1] = ipanel_porting_filter_open(0x11, 0x42, 0xff, 0xffff, 0xffff, 0xc1);
	dprintf("[ipanel_demux_test] the filter[1]=0x%x.\n\n", filterID[1]);

	filterID[2] = ipanel_porting_filter_open(0x0, 0x0, 0xff, 0xffff, 0xffff, 0xc1);
	dprintf("[ipanel_demux_test] the filter[2]=0x%x.\n\n", filterID[2]);

	filterID[3] = ipanel_porting_filter_open(0x14, 0x73, 0xff, 0xffff, 0xffff, 0xc1);
	dprintf("[ipanel_demux_test] the filter[3]=0x%x.\n\n", filterID[3]);

	filterID[4] = ipanel_porting_filter_open(0x14, 0x70, 0xff, 0xffff, 0xffff, 0xc1);
	dprintf("[ipanel_demux_test] the filter[4]=0x%x.\n\n", filterID[4]);

	filterID[5] = ipanel_porting_filter_open(0x10, 0x40, 0xff, 0xffff, 0xffff, 0xc1);
	dprintf("[ipanel_demux_test] the filter[5]=0x%x.\n\n", filterID[5]);

	while (1)
	{
		if (1 == ipanel_porting_filter_poll(event))
		{
			ipanel_porting_filter_recv(event[2], process_buffer, 4096);
			dprintf("[ipanel_demux_test]section: %02x %02x %02x %02x %02x %02x %02x %02x --- filter = 0x%x \n",
					process_buffer[0], process_buffer[1], process_buffer[2], process_buffer[3],
					process_buffer[4], process_buffer[5], process_buffer[6], process_buffer[7],
					event[2]);
			ipanel_porting_filter_close(event[2]);
			dprintf("[ipanel_demux_test] the time =%d..\n\n\n", (time_ms() - begin_time));
		}

		osal_task_sleep(10);
		if (5000 < (time_ms() - begin_time))
			break;
	}

	ipanel_porting_delivery_tune(3230000, 68750, 3);
	while(1){
		osal_task_sleep(500); //ms	
		nim_get_lock(g_nim_dev, &lock);
		if(1 == lock){
			dprintf("[ipanel_demux_test] tune locked.\n");
			break;
		}else
			dprintf("[ipanel_demux_test] tune not locked.\n");
	}

	begin_time = time_ms();

	filterID[0] = ipanel_porting_filter_open(0x11, 0x42, 0xff, 0xffff, 0xffff, 0xc1);
	dprintf("[ipanel_demux_test] the filter[0]=0x%x.\n\n", filterID[0]);

	filterID[1] = ipanel_porting_filter_open(0x11, 0x46, 0xff, 0xffff, 0xffff, 0xc1);
	dprintf("[ipanel_demux_test] the filter[1]=0x%x.\n\n", filterID[1]);

	filterID[2] = ipanel_porting_filter_open(0x0, 0x0, 0xff, 0xffff, 0xffff, 0xc1);
	dprintf("[ipanel_demux_test] the filter[2]=0x%x.\n\n", filterID[2]);

	filterID[3] = ipanel_porting_filter_open(0x14, 0x73, 0xff, 0xffff, 0xffff, 0xc1);
	dprintf("[ipanel_demux_test] the filter[3]=0x%x.\n\n", filterID[3]);

	filterID[4] = ipanel_porting_filter_open(0x14, 0x70, 0xff, 0xffff, 0xffff, 0xc1);
	dprintf("[ipanel_demux_test] the filter[4]=0x%x.\n\n", filterID[4]);

	filterID[5] = ipanel_porting_filter_open(0x10, 0x40, 0xff, 0xffff, 0xffff, 0xc1);
	dprintf("[ipanel_demux_test] the filter[5]=0x%x.\n\n", filterID[5]);

	while (1)
	{
		if (1 == ipanel_porting_filter_poll(event))
		{
			ipanel_porting_filter_recv(event[2], process_buffer, 4096);
			dprintf("[ipanel_demux_test]section: %02x %02x %02x %02x %02x %02x %02x %02x --- filter = 0x%x \n",
					process_buffer[0], process_buffer[1], process_buffer[2], process_buffer[3],
					process_buffer[4], process_buffer[5], process_buffer[6], process_buffer[7],
					event[2]);
			ipanel_porting_filter_close(event[2]);
			dprintf("[ipanel_demux_test] the time =%d..\n\n\n", (time_ms() - begin_time));
		}

		osal_task_sleep(10);
		if (5000 < (time_ms() - begin_time))
			break;
	}

	ipanel_porting_demux_exit();
	dprintf("[ipanel_demux_test] test end.\n");
}

#endif

#if ipanel_avplay_test_flag
#include <udi/ipanel/ipanel_av.h>
#define VIDEO_PID 1921
#define AUDIO_PID 1922
#define PCR_PID   1921
extern struct nim_device *g_nim_dev;
void ipanel_avplay_test()
{
	UINT8 lock = 0;
	ipanel_porting_delivery_tune(4350000, 68750, 3, 2);
	while(1){
		osal_task_sleep(500); //ms	
		nim_get_lock(g_nim_dev, &lock);
		if(1 == lock){
			dprintf("[ipanel_avplay_test] tune locked.\n");
			break;
		}else
			dprintf("[ipanel_avplay_test] tune not locked.\n");
	}
	while(1){
		dprintf("\n[ipanel_avplay_test] test begin!\n");
		ipanel_porting_av_start(AUDIO_PID, VIDEO_PID, PCR_PID);
		dprintf("[ipanel_avplay_test] AV was playing!\n");
		osal_task_sleep(10000);
		ipanel_porting_av_stop();
		osal_task_sleep(3000);
		dprintf("[ipanel_avplay_test] test end!\n\n");
	}
}

#endif

#if ipanel_gfx_test_flag
#include <udi/ipanel/ipanel_graphics.h>
#define TEST_GE_SRC1_ADDR	0xA0C00000
#define TEST_GE_SRC2_ADDR	0xA0DC0000
unsigned char src_buf_r[288*16 + 4] = {
#include "inc/red.h"
};

unsigned char src_buf_b[288*16 + 4] = {
#include "inc/blue.h"
};

void ipanel_gfx_test()
{
	while(1){
		ipanel_porting_draw_image(0, 0, 144, 16, (UINT8*)src_buf_r, IPANEL_SCREENW);
		osal_task_sleep(5000);
		ipanel_porting_draw_image(0, 0, 144, 16, (UINT8*)src_buf_b, IPANEL_SCREENW);
		osal_task_sleep(5000);
		//		ipanel_porting_draw_image(0,0,IPANEL_SCREENW,IPANEL_SCREENH,(UINT8*)TEST_GE_SRC1_ADDR,IPANEL_SCREENW);
		//		osal_task_sleep(5000);
	}
}

#endif

#if ipanel_pcm_test_flag
#include <udi/ipanel/ipanel_sound.h>
void ipanel_pcm_test()
{

}
#endif

#if ipanel_socket_test_flag
#include <udi/ipanel/ipanel_socket.h>
#include "ipanel_input.h"
#include <udi/ipanel/ipaneldtv_api.h>

#define NP   libc_printf
#define IP_ADDRESS			"192.168.28.23"
#define NM_ADDRESS			"255.255.255.0"
#define GW_ADDRESS			"192.168.28.1"

/* TCP server */
#define TCP_SERVER_IP		"192.168.28.20"
#define TCP_SERVER_PORT	5500	
#define TCP_CLIENT_PORT		5555
#define TCP_BUF_SIZE		4096
UINT8 tcp_buf[2 *TCP_BUF_SIZE];

void ipanel_socket_test()
{
#if 0
	unsigned int event[3] = {0};
	void *phandle = NULL;
	phandle = ipaneldtv_create(0xC00000);
	if (NULL != phandle){	
		ipaneldtv_open_url(phandle, "http://192.168.28.20/ALI/index.htm");
		while(1) {						
			if (1 ==  eis_input_get(event)){
				ipanel_porting_dprintf("[ipanel_socket_test] get key. event[0]=%d,event[1]=%d.\n\n", event[0], event[1]);
				ipaneldtv_proc(phandle, (int)event[0], event[1], event[2]);
			}
			else{
				ipaneldtv_proc(phandle, 0, 0, 0);
			}
			osal_task_sleep(10);
		}
		ipaneldtv_destroy(phandle);
	}
#else

	int status =  - 1, nactive =  - 1, sock =  - 1;
	struct sockaddr_in serv_addr;
	struct sockaddr_in in_name;
	fd_set fds_r, fds_w, fds_e;

    struct timeval tm = {0, 700};

	FD_ZERO(&fds_r);
	FD_ZERO(&fds_w);
	FD_ZERO(&fds_e);

	bzero((char*)(&serv_addr), sizeof(serv_addr));
	serv_addr.sin_len = sizeof(serv_addr);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = ntohl(0xc0a81c14);
	serv_addr.sin_port = htons(80);

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock < 0)
	{
		dprintf("[ipanel_porting_socket_open] CreateSocket Failed\n");
		return ;
	}

	dprintf("[ipanel_porting_socket_open] create socket successfully socket is %d\n", sock);

	while (1)
	{
		status = connect(sock, (struct sockaddr*)(&serv_addr), sizeof(struct sockaddr));
        if(status==-1){
			dprintf("[ipanel_porting_socket_open] Socket Connect Error\n");
			osal_task_sleep(100);
		}
		else{
			dprintf("[ipanel_porting_socket_open] Socket Connect success\n");
			break;
		}
	}

	FD_SET(sock, &fds_r);
	FD_SET(sock, &fds_w);

	tm.tv_sec = 0;
	tm.tv_usec = 0;
	dprintf("\n1 after set socket,start select.no write or read event back,test timeout(0 ms)..\n");
	nactive = select(sock + 1, &fds_r, NULL, &fds_e, &tm);
	dprintf("1  after select..nactive = %d..\n", nactive);


	tm.tv_sec = 0;
	tm.tv_usec = 1000;
	dprintf("\n2  start select.no recv or read event back,test timeout(1000 us)..\n");
	nactive = select(sock + 1, &fds_r, NULL, &fds_e, &tm);
	dprintf("2  after select..nactive = %d..\n", nactive);


	tm.tv_sec = 0;
	tm.tv_usec = 100;
	dprintf("\n3  start select.no recv or read event back,test timeout(100 us)..\n");
	nactive = select(sock + 1, &fds_r, NULL, &fds_e, &tm);
	dprintf("3  after select..nactive = %d..\n", nactive);

	while(1);

#endif
}

#endif

#if ipanel_nvram_test_flag
#include <udi/ipanel/ipanel_nvram.h>

#define TEST_VNRAM_LEN	0x10000
unsigned char buffer1[TEST_VNRAM_LEN];
unsigned char buffer2[TEST_VNRAM_LEN];

void ipanel_nvram_test()
{
	unsigned char *addr;
	int numberofsections;
	int sect_size, i, j = 0;

	dprintf("[ipanel_nvram_test] dprintf test begin.\n");

	if (ipanel_porting_nvram_info(&addr, &numberofsections, &sect_size, 0) != 0)
	{
		dprintf("ipanel_porting_nvram_info failed\n");
		return ;
	}

	while (1)
	{
		j++;
		for (i = 0; i < TEST_VNRAM_LEN; i++)
		{
			buffer1[i] = j;
		}
		MEMSET(buffer2, 0, TEST_VNRAM_LEN);

		if (ipanel_porting_nvram_burn((UINT32)addr, buffer1, TEST_VNRAM_LEN) < 0)
		{
			libc_printf("[ipanel_nvram_test] ipanel_porting_nvram_burn failed\n");
			return ;
		}
		while (ipanel_porting_nvram_status((UINT32)addr, TEST_VNRAM_LEN) != 1)
		 /* 0 -- writing, 1 -- success and complete, -1 - fail */
		{
			osal_task_sleep(1000);
		}
		osal_task_sleep(1000);
		libc_printf("buffer2[0] = %d,buffer2[1]=%d..\n", buffer2[0], buffer2[1]);
		if (ipanel_porting_nvram_read((UINT32)addr, buffer2, TEST_VNRAM_LEN) !=  - 1)
		{
			libc_printf("buffer2[0] = %d,buffer2[1]=%d..\n", buffer2[0], buffer2[1]);
			if (MEMCMP(buffer2, buffer1, TEST_VNRAM_LEN) == 0)
				libc_printf("[ipanel_nvram_test] nvram works well...\n");
			else
				libc_printf("[ipanel_nvram_test] read data error...\n");
		}
		else
			libc_printf("[ipanel_nvram_test] ipanel_porting_nvram_read failed\n");
		libc_printf("[ipanel_nvram_test] write cnt :%d\n", j);
		osal_task_sleep(1000);
	}
}

#endif

/******* main test **********/
void ipanel_test()
{
#if ipanel_base_test_flag
	ipanel_base_test();
#endif

#if ipanel_panIR_test_flag
	ipanel_panIR_test();
#endif

#if ipanel_tuner_test_flag
	ipanel_tuner_test();
#endif

#if ipanel_demux_test_flag
	ipanel_demux_test();
#endif

#if ipanel_avplay_test_flag
	ipanel_avplay_test();
#endif

#if ipanel_gfx_test_flag
	ipanel_gfx_test();
#endif

#if ipanel_pcm_test_flag
	ipanel_pcm_test();
#endif

#if ipanel_socket_test_flag
	ipanel_socket_test();
#endif

#if ipanel_nvram_test_flag
	ipanel_nvram_test();
#endif
}
