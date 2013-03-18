/*********************************************************************
    Copyright (c) 2005 Embedded Internet Solutions, Inc
    All rights reserved. You are not allowed to copy or distribute
    the code without permission.
    This is the demo implenment of the VIDEO Porting APIs needed by iPanel MiddleWare. 
    Maybe you should modify it accorrding to Platform.
    
    Note: the "int" in the file is 32bits
    
    $ver0.0.0.1 $author Zouxianyun 2005/04/28
*********************************************************************/

#include <udi/ipanel/ipanel_video.h>

/**********************************************************************************/
/*Description: Set OSD alpha. 0-OSD visible, 100-OSD invisible, between 0 to 100  */
/*Input      : alpha value, if value is not in 0 to 100, the return fail          */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
int ipanel_porting_set_alpha(int alpha)
{
	int ret = -1;
	if (alpha >= 0 && alpha <= 100) {
		/*set physical OSD alpha here*/		
		ret = 0;
	}
	return ret;
}


/**********************************************************************************/
/*Description: Get OSD alpha. 0-OSD visible, 100-OSD invisible, between 0 to 100  */
/*Input      : No                                                                 */
/*Output     : No                                                                 */
/*Return     : OSD alpha value                                                    */
/**********************************************************************************/
int ipanel_porting_get_alpha(void)
{
	int alpha = 50; /*default OSD alpha is half alpha*/
	/*Get physical OSD alpha here.*/
	return alpha;
}


/**********************************************************************************/
/*Description: Set TV decode mode. 1 - NTSC, 2 - PAL, 3 - SECAM, 4 - AUTO         */
/*Input      : mode value, if value is not in 1 to 4, the return fail             */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
int ipanel_porting_set_video_mode(int mode)
{
	int ret = -1;
	if (mode > 0 && mode <= 4) {
		/*set physical TV decode mode here*/		
		ret = 0;
	}
	return ret;
}


/**********************************************************************************/
/*Description: Get TV decode mode. 1 - NTSC, 2 - PAL, 3 - SECAM, 4 - AUTO         */
/*Input      : No                                                                 */
/*Output     : No                                                                 */
/*Return     : TV decode mode                                                     */
/**********************************************************************************/
int ipanel_porting_get_video_mode(void)
{
	int mode = 4; /*default TV decode mode is AUTO*/
	/*Get physical TV decode mode here.*/
	return mode;
}


/**********************************************************************************/
/*Description: Set TV screen scale. 1 - width:height=4:3, 2 - width:height=16:9   */
/*Input      : scale value, if value is not in 1 to 2, the return fail            */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
int ipanel_porting_set_video_scale(int scale)
{
	int ret = -1;
	if (scale > 0 && scale <= 2) {
		/*set physical TV SCREEN scale here*/		
		ret = 0;
	}
	return ret;
}

/**********************************************************************************/
/*Description: Get TV screen scale. 1 - width:height=4:3, 2 - width:height=16:9   */
/*Input      : No                                                                 */
/*Output     : No                                                                 */
/*Return     : TV  screen scale                                                   */
/**********************************************************************************/
int ipanel_porting_get_video_scale(void)
{
	int mode = 1; /*default TV screen scale is width:height=4:3*/
	/*Get physical TV screen scale here.*/
	return mode;
}


/**********************************************************************************/
/*Description: Set TV picture chroma. 0 - dark, 100 - bright, between 0 to 100    */
/*Input      : picture chroma value, if value is not in 0 to 100, the return fail */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
int ipanel_porting_set_picture_chroma(int chroma)
{
	int ret = -1;
	if (chroma >= 0 && chroma <= 100) {
		/*set physical TV picture chroma here*/		
		ret = 0;
	}
	return ret;
}


/**********************************************************************************/
/*Description: Get TV picture chroma. 0 - dark, 100 - bright, between 0 to 100    */
/*Input      : No                                                                 */
/*Output     : No                                                                 */
/*Return     : TV picture chroma                                                  */
/**********************************************************************************/
int ipanel_porting_get_picture_chroma(void)
{
	int chroma = 50; /*default TV picture chroma is 50*/
	/*Get physical TV picture chroma here.*/
	return chroma;
}


/**********************************************************************************/
/*Description: Set TV picture color. 0 -no color, 100 - colorest, between 0 to 100*/
/*Input      : picture color value, if value is not in 0 to 100, the return fail  */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
int ipanel_porting_set_picture_color(int color)
{
	int ret = -1;
	if (color >= 0 && color <= 100) {
		/*set physical TV picture chroma here*/		
		ret = 0;
	}
	return ret;
}


/**********************************************************************************/
/*Description: Get TV picture color. 0 -no color, 100 - colorest, between 0 to 100*/
/*Input      : No                                                                 */
/*Output     : No                                                                 */
/*Return     : TV picture color                                                   */
/**********************************************************************************/
int ipanel_porting_get_picture_color(void)
{
	int color = 50; /*default TV picture color is 50*/
	/*Get physical TV picture color here.*/
	return color;
}


/**********************************************************************************/
/*Description: Set TV picture brightness. 0-black, 100-brightest, between 0 to 100*/
/*Input      : picture brightness, if value is not in 0 to 100, the return fail   */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
int ipanel_porting_set_picture_brightness(int bright)
{
	int ret = -1;
	if (bright >= 0 && bright <= 100) {
		/*set physical TV picture chroma here*/		
		ret = 0;
	}
	return ret;
}


/**********************************************************************************/
/*Description: Get TV picture brightness. 0-black, 100-brightest, between 0 to 100*/
/*Input      : No                                                                 */
/*Output     : No                                                                 */
/*Return     : TV picture brightness                                              */
/**********************************************************************************/
int ipanel_porting_get_picture_brightness(void)
{
	int bright = 50; /*default TV picture brightness is 50*/
	/*Get physical TV picture brightness here.*/
	return bright;
}
