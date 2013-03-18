/*********************************************************************
    Copyright (c) 2005 Embedded Internet Solutions, Inc
    All rights reserved. You are not allowed to copy or distribute
    the code without permission.
    There are the Media Porting APIs needed by iPanel MiddleWare. 
    
    Note: the "int" in the file is 32bits
    
    $ver0.0.0.1 $author Zouxianyun 2005/04/28
*********************************************************************/

#ifndef _IPANEL_MIDDLEWARE_PORTING_MEDIA_API_FUNCTOTYPE_H_
#define _IPANEL_MIDDLEWARE_PORTING_MEDIA_API_FUNCTOTYPE_H_

#ifdef __cplusplus
extern "C" {
#endif

/*open a media url*/
int ipanel_porting_media_open(const char *url, const char *mediatype);

/*close the opened media*/
int ipanel_porting_media_close(void);

/*get media track length for time*/
int ipanel_porting_media_duration(void);

/*get media status, special need speed*/
const char *ipanel_porting_media_status(int *speed);

/*set play position on TV*/
int ipanel_porting_video_location(int x, int y, int w, int h);

/*play the opened media*/
int ipanel_porting_program_play(void);

/*pause the opened media*/
int ipanel_porting_program_pause(void);

/*stop the opened media*/
int ipanel_porting_program_stop(void);

/*resume the opened media*/
int ipanel_porting_program_resume(void);

/*play the opened media fast*/
int ipanel_porting_program_forward(int number);

/*play the opened media slow*/
int ipanel_porting_program_slow(int number);

/*repeat the opened media times*/
int ipanel_porting_program_repeat(int number);

/*rewind the opened media*/
int ipanel_porting_program_rewind(void);

/*seek a position of the opened media*/
int ipanel_porting_program_seek(int mode, int value);

/*break the opened media*/
int ipanel_porting_program_break(void);

/*reverse the opened media*/
int ipanel_porting_program_reverse(int number);

/*set parameter value*/
int ipanel_porting_vod_set_param(const char *name, const char *value);

/*get parameter value*/
const char *ipanel_porting_vod_get_param(const char*name);

#ifdef __cplusplus
}
#endif

#endif
