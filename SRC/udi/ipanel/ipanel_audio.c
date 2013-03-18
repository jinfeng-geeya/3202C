/*********************************************************************
    Copyright (c) 2005 Embedded Internet Solutions, Inc
    All rights reserved. You are not allowed to copy or distribute
    the code without permission.
    This is the demo implenment of the AUDIO Porting APIs needed by iPanel MiddleWare. 
    Maybe you should modify it accorrding to Platform.
    
    Note: the "int" in the file is 32bits
    
    $ver0.0.0.1 $author Zouxianyun 2005/04/28
*********************************************************************/

#include <udi/ipanel/ipanel_audio.h>
#include "udi/ipanel/ipanel_base.h"

extern struct snd_device * g_snd_dev;

/**********************************************************************************/
/*Description: Set Audio volume. 0 - mute, 100 - strength sound, between 0 to 100 */
/*Input      : volume value, if value is not in 0 to 100, the return fail         */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
int ipanel_porting_set_volume(int vol)
{
	int ret = -1;
	if (vol >= 0  && vol <= 100)
	{
		snd_set_volume(g_snd_dev, SND_SUB_OUT, vol);/*set physical OSD alpha here*/		
		ret = 0;
	}
	return ret;
}


/**********************************************************************************/
/*Description: Get Audio volume. 0 - mute, 100 - strength sound, between 0 to 100 */
/*Input      : No                                                                 */
/*Output     : No                                                                 */
/*Return     : Audio volume value                                                 */
/**********************************************************************************/
int ipanel_porting_get_volume(void)
{
	/*Get physical Audio volume here.*/
	return snd_get_volume(g_snd_dev);
}



/**********************************************************************************/
/*Description: Set Audio decode mode. 1 - stereo  2 - mono  3 - left  4 - right   */
/*             5 - AC3  6 - Dolby  7 - DTS  8  - surround  9 - auto  total 9 mode */
/*Input      : mode value, if value is not in 1 to 9, the return fail             */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
int ipanel_porting_set_audio_mode(int mode)
{
	int ret = -1;
	if (mode > 0 && mode <= 9) {
		/*set physical Audio decode mode here*/		
		ret = 0;
	}
	return ret;
}


/**********************************************************************************/
/*Description: Get Audio decode mode. 1 - stereo  2 - mono  3 - left  4 - right   */
/*             5 - AC3  6 - Dolby  7 - DTS  8  - surround  9 - auto  total 9 mode */
/*Input      : No                                                                 */
/*Output     : No                                                                 */
/*Return     : Audio decode mode                                                  */
/**********************************************************************************/
int ipanel_porting_get_audio_mode(void)
{
	int mode = 9; /*default Audio decode mode is AUTO*/
	/*Get physical TV decode mode here.*/
	return mode;
}


/**********************************************************************************/
/*Description: Set Audio Language. 1 - english  2 - chinese  3 - Japanese         */
/*Input      : track (1 ~ 3)                                                      */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
int ipanel_porting_audio_set_language(int track)
{
	int ret = -1;
	if (track > 0 && track <= 3) {
		/*set physical Audio language here*/		
		ret = 0;
	}
	return ret;
}

int ipanel_porting_isMute(void)
{
	return 1;
}

