/*********************************************************************
    Copyright (c) 2005 Embedded Internet Solutions, Inc
    All rights reserved. You are not allowed to copy or distribute
    the code without permission.
    There are the VIDEO Porting APIs needed by iPanel MiddleWare. 
    
    Note: the "int" in the file is 32bits
    
    $ver0.0.0.1 $author Zouxianyun 2005/04/28
*********************************************************************/

#ifndef _IPANEL_MIDDLEWARE_PORTING_VIDEO_API_FUNCTOTYPE_H_
#define _IPANEL_MIDDLEWARE_PORTING_VIDEO_API_FUNCTOTYPE_H_

#ifdef __cplusplus
extern "C" {
#endif

/*set OSD alpha*/
int ipanel_porting_set_alpha(int alpha);

/*get OSD alpha*/
int ipanel_porting_get_alpha(void);

/*set video decode mode*/
int ipanel_porting_set_video_mode(int mode);

/*get video decode mode*/
int ipanel_porting_get_video_mode(void);

/*set the video scale*/
int ipanel_porting_set_video_scale(int scale);

/*get the video scale*/
int ipanel_porting_get_video_scale(void);

/*set the TV picture chroma*/
int ipanel_porting_set_picture_chroma(int chroma);

/*get the TV picture chroma*/
int ipanel_porting_get_picture_chroma(void);

/*set the TV picture color*/
int ipanel_porting_set_picture_color(int color);

/*get the TV picture color*/
int ipanel_porting_get_picture_color(void);

/*set the TV picture brightness*/
int ipanel_porting_set_picture_brightness(int bright);

/*get the TV picture brightness*/
int ipanel_porting_get_picture_brightness(void);

#ifdef __cplusplus
}
#endif

#endif
