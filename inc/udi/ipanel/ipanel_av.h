/*********************************************************************
    Copyright (c) 2005 Embedded Internet Solutions, Inc
    All rights reserved. You are not allowed to copy or distribute
    the code without permission.
    There are the Tuner Porting APIs needed by iPanel MiddleWare. 
    
    Note: the "int" in the file is 32bits
   
*********************************************************************/

#ifndef _IPANEL_MIDDLEWARE_PORTING_AV_API_FUNCTOTYPE_H_
#define _IPANEL_MIDDLEWARE_PORTING_AV_API_FUNCTOTYPE_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TagService_Info{
	UINT32 net_id;                   
	UINT32 ts_id;
	UINT32 service_id;
	UINT32 service_type;
	UINT32 frequency;
	UINT32 symbol_rate;
	UINT32 modulation;
    UINT32 video_pid;
    UINT32 audio_pid;
    UINT32 pcr_pid;   
}Service_Info;


//void ipanel_porting_av_start(unsigned short apid, unsigned short vpid, unsigned short pcr_pid);
void ipanel_porting_av_stop(void);

int ipanel_porting_dvb_select_av(int pcr_pid,   int emm_pid, int video_pid,  int audio_pid, int audio_ecm_pid, int video_ecm_pid,int servicetype);


int ipanel_av_init(void);

int ipanel_av_exit(void);

#ifdef __cplusplus
}
#endif

#endif

