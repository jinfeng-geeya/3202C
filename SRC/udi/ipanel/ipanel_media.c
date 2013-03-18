/*********************************************************************
    Copyright (c) 2005 Embedded Internet Solutions, Inc
    All rights reserved. You are not allowed to copy or distribute
    the code without permission.
    This is the demo implenment of the Media Porting APIs needed by iPanel MiddleWare. 
    Maybe you should modify it accorrding to Platform.
    
    Note: the "int" in the file is 32bits
    
    $ver0.0.0.1 $author Zouxianyun 2005/04/28
*********************************************************************/

#include <udi/ipanel/ipanel_base.h>
#include <udi/ipanel/ipanel_media.h>
#include <udi/ipanel/ipanel_av.h>
#include <udi/ipanel/ipanel_graphics.h>
extern struct vpo_device*   g_vpo_dev ;
extern int video_status;

#if 1
#define   ipanel_porting_video_seek     ipanel_porting_program_seek
#define   ipanel_porting_video_play     ipanel_porting_program_play
#define   ipanel_porting_video_pause    ipanel_porting_program_pause
#define   ipanel_porting_video_stop     ipanel_porting_program_stop
#define   ipanel_porting_video_resume   ipanel_porting_program_resume
#define   ipanel_porting_video_rewind   ipanel_porting_program_rewind
#define   ipanel_porting_video_break    ipanel_porting_program_break
#define   ipanel_porting_video_forward  ipanel_porting_program_forward
#define   ipanel_porting_video_reverse  ipanel_porting_program_reverse
#define   ipanel_porting_video_slow     ipanel_porting_program_slow
#define   ipanel_porting_video_repeat   ipanel_porting_program_repeat
#endif


/**********************************************************************************/
/*Description: Open a media URL                                                   */
/*Input      : media url string, media type such as "MPEG" "mp3" NULL etc         */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
int ipanel_porting_media_open(const char *url, const char *mediatype)
{
	int ret = -1;

	if (NULL != url && strlen(url)>0) {
		/*open a physical media url here.*/
		ret = 0;
	}

	return ret;
}


/**********************************************************************************/
/*Description: Close the opened media                                             */
/*Input      : No                                                                 */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
int ipanel_porting_media_close(void)
{
	/*close the physical media url here.*/
	return 0;
}


/**********************************************************************************/
/*Description: get media track length for time                                    */
/*Input      : No                                                                 */
/*Output     : No                                                                 */
/*Return     : length, uints is seconds                                           */
/**********************************************************************************/
int ipanel_porting_media_duration(void)
{
	/*media track length is 1minute 30 seconds*/
	return 90;
}


/**********************************************************************************/
/*Description: get media status                                                   */
/*Input      : save speed buffer                                                  */
/*Output     : speed (optioned)                                                   */
/*Return     : media status                                                       */
/**********************************************************************************/
const char *ipanel_porting_media_status(int *speed)
{
	/*if running*/
	*speed = 0;   /* no use*/
	return "running";
	
	/*if slow
	  *speed = 2; //will be used
	  return "slow";
	  
	  if forward
	  *speed = 5; //will be used
	  return "forward";
	*/
}


/**********************************************************************************/
/*Description: Reset the opened media                                             */
/*Input      : No                                                                 */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
int ipanel_porting_reset_media_setting(void)
{
	/*reset physical media here*/
	return 0;
}


/**********************************************************************************/
/*Description: Set TV screen area to play. (0, 0, 0, 0) is full srceen            */
/*Input      : TV screen area x pixls, y pixls, w width, and h height.            */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
int ipanel_porting_video_location(int x, int y, int w, int h)
{
	int ret = -1;
	struct Rect SrcRect;
	struct Rect DstRect;
	ipanel_porting_dprintf("[ipanel_porting_video_location],rect (x=%d,y=%d,w=%d,h=%d).\n",x,y,w,h);
	if ( (x>=0 && x<=IPANEL_SCREENW) && (y>=0 && y<=IPANEL_SCREENH) 
		&& (w>=0 && w<=IPANEL_SCREENW) && (h>=0 && h<=IPANEL_SCREENH) ) {
		if (x==0 && y==0 && w==0 && h==0) {
			w = PICTURE_WIDTH;
			h = PICTURE_HEIGHT;
		}
		else{
			x+=DISPLAY_POS_X;
			y= (y+DISPLAY_POS_Y)*5;
			h *= 5;
		}
		/*Set physical TV screen area here*/
		SrcRect.uStartX = 0;
		SrcRect.uWidth = PICTURE_WIDTH;
		SrcRect.uStartY = 0;
		SrcRect.uHeight = PICTURE_HEIGHT;
		DstRect.uStartX = x;
		DstRect.uStartY = y;
		DstRect.uWidth = w;
		DstRect.uHeight = h;
//		vpo_win_onoff(g_vpo_dev,FALSE);
		if(RET_SUCCESS == vpo_zoom(g_vpo_dev ,&SrcRect , &DstRect))
			ret = 0;
		else
			ipanel_porting_dprintf("[ipanel_porting_video_location] failed.\n");
//		vpo_win_onoff(g_vpo_dev,TRUE);
	}	
	return ret;
}


/**********************************************************************************/
/*Description: play the opened media                                              */
/*Input      : No                                                                 */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
int ipanel_porting_video_play(void)
{
	/*play the physical opened media*/
	return 0;
}


/**********************************************************************************/
/*Description: pause the opened media                                             */
/*Input      : No                                                                 */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
int ipanel_porting_video_pause(void)
{
	/*pause the physical opened media*/
	return 0;
}


/**********************************************************************************/
/*Description: stop the opened media                                              */
/*Input      : No                                                                 */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
int ipanel_porting_video_stop(void)
{
	ipanel_porting_dprintf("[ipanel_porting_video_stop] called..\n");
	ipanel_porting_av_stop();
	/*stop the physical opened media*/
	return 0;
}


/**********************************************************************************/
/*Description: resume the opened media                                            */
/*Input      : No                                                                 */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
int ipanel_porting_video_resume(void)
{
	/*resume the physical opened media*/
	return 0;
}


/**********************************************************************************/
/*Description: play the opened media fast                                         */
/*Input      : multi speed                                                        */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
int ipanel_porting_video_forward(int number)
{
	int ret = -1;

	if (number == 2 || number == 5) {
		/*play_speed *= number;*/
		/*play the physical opened media new speed here.*/
		ret = 0;
	}

	return ret; 
}


/**********************************************************************************/
/*Description: play the opened media slow                                         */
/*Input      : division speed                                                     */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
int ipanel_porting_video_slow(int number)
{
	int ret = -1;

	if (number == 2 || number == 5) {
		/*play_speed /= number;*/
		/*play the physical opened media new speed here.*/
		ret = 0;
	}

	return ret; 
}


/**********************************************************************************/
/*Description: repeat the opened media times                                      */
/*Input      : repeat times, -1 - no limits                                       */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
int ipanel_porting_video_repeat(int number)
{
	int ret = -1, times;

	if (number > 0) {
		for (times = 0; times < number; times++) {
			/*play the physical opened media here.*/
			;
		}
		ret = 0;
	}
	else if (number == -1) {
		while(1) {
			/*play the physical opened media here.*/
			;
		}
		ret = 0;			
	}

	return ret; 
}



/**********************************************************************************/
/*Description: rewind the opened media                                            */
/*Input      : No                                                                 */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
int ipanel_porting_video_rewind(void)
{
	/*rewind the physical opened media*/
	return 0;
}


/**********************************************************************************/
/*Description: seek a position of the opened media                                */
/*Input      : mode (0 - time mode, 1 - chapter mode, other fail.)                */
/*             value is (0 ~86399) in time mode, and >0 in chapter mode.          */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
int ipanel_porting_video_seek(int mode, int value)
{
	int ret = -1;
	
	if (value > 0) {
		if (1 == mode) {
			/*seek a position according chapter here.*/
			ret = 0;
		}
		else if (0==mode && value<86399) {
			/*seek a position according time here.*/
			ret = 0;
		}
	}
	
	return ret;
}



/**********************************************************************************/
/*Description: break the opened media                                             */
/*Input      : No                                                                 */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
int ipanel_porting_video_break(void)
{
	/*break the physical opened media*/
	return 0;
}


/**********************************************************************************/
/*Description: reverse the opened media                                           */
/*Input      : reverse speed                                                      */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
int ipanel_porting_video_reverse(int number)
{
	/*reverse the physical media here.*/
	return 0;
}


/**********************************************************************************/
/*Description: set the media mode, audio or video mode                            */
/*Input      : "av" or "tv"                                                       */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
int ipanel_porting_video_tvav(const char *mode)
{
	int ret = -1;
	
	if ( strcmp(mode, "av")==0 || strcmp(mode, "tv")==0 ) {
		/*set physical media mode here*/
		
		ret = 0;
	}
	return ret;
}


/**********************************************************************************/
/*Description: set parameter value,                                               */
/*Input      : parameter name and parameter value.                                */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
int ipanel_porting_vod_set_param(const char *name, const char *value)
{
	/*set parameter value here*/
	return 0;
}


/**********************************************************************************/
/*Description: get parameter value,                                               */
/*Input      : parameter name                                                     */
/*Output     : No                                                                 */
/*Return     : parameter value                                                    */
/**********************************************************************************/
const char *ipanel_porting_vod_get_param(const char*name)
{
	char *val = NULL;
	/*if (strcmp(name, "media") == 0)
		val = "Media";  //etc
	*/
	return val;
}

