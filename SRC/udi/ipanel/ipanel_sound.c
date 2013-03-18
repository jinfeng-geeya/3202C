/*********************************************************************
    Copyright (c) 2005 Embedded Internet Solutions, Inc
    All rights reserved. You are not allowed to copy or distribute
    the code without permission.
    This is the demo implenment of the SOUND Porting APIs needed by iPanel MiddleWare. 
    Maybe you should modify it accorrding to Platform.
    
    Note: the "int" in the file is 32bits
    
    $ver0.0.0.1 $author Zouxianyun 2005/04/28
*********************************************************************/

#include <stdio.h>
#include <udi/ipanel/ipanel_sound.h>
#include <udi/ipanel/ipanel_porting_event.h>


#define NEED_BUF_NUMBER   (8)
#define BUF_SIZE_PER_BLOCK  (4*1024)

/**********************************************************************************/
/*Description: Initialize sound. you maybe create a new pthread/task to play data */
/*Input      : No                                                                 */
/*Output     : No                                                                 */
/*Return     : No                                                                 */
/**********************************************************************************/
void ipanel_porting_sound_init(void)
{
}


/**********************************************************************************/
/*Description: Free all resource in function ipanel_porting_sound_init()          */
/*Input      : No                                                                 */
/*Output     : No                                                                 */
/*Return     : No                                                                 */
/**********************************************************************************/
void ipanel_porting_sound_exit(void)
{
}


/**********************************************************************************/
/*Description: Get STB sound channel parameters                                   */
/*Input      : the Address to save number of play buffers, and a play buffer size */
/*Output     : number of buffers,  a play buffer size                             */
/*Return     : 0 -- success, -1 -- fail.                                          */
/**********************************************************************************/
int ipanel_porting_sound_get_info(int *numberofbuf, int *buf_size)
{
	*numberofbuf = NEED_BUF_NUMBER;
	*buf_size = BUF_SIZE_PER_BLOCK;
	
	/* you maybe give other parameter, and they must fit your stb.*/
	
	if (*numberofbuf <= 0 || *buf_size < 1024 || (*buf_size)*(*numberofbuf)<32768)
		return -1;
	else
		return 0;  
}


/**********************************************************************************/
/*Description: Open sound channel, maybe you should stop the running channel.     */
/*Input      : soundformat(1 - PCM, 2 - mp3, 3 - WMA)                             */
/*             soundsamplerate(units is Hz, 44100 22050 11025 8000)               */
/*             bitsperSample(8 - 8bits, 16 - 16bits)                              */ 
/*             soundchannels(1 - mono, 2 - stereo)                                */ 
/*             optionflag(0 - "unsigned format", other - reserve)                 */ 
/*Output     : No                                                                 */
/*Return     : >0 -- the handle or Idescription, 0 -- fail.                       */
/**********************************************************************************/
unsigned int ipanel_porting_sound_open(int format, int samplerate, int bitsperSample, int channels, int optionflag)
{
	int handle = 0;
	
	if ( (format>0 && format<=3) && 
		 (samplerate==44100 ||  samplerate==22050 ||  samplerate==11025 ||  samplerate==8000) &&
		 (bitsperSample==8 || bitsperSample==16) &&
		 (channels>0 && channels<=2) ) {
		/*Setup Physical sound device here*/

		handle = 1;
	}
	return handle;
}


/**********************************************************************************/
/*Description: Play sound data, return no block. and no free buffer after return. */
/*             the data buffer will free when a play-complete message sent to iMW.*/
/*Input      : device Id, Sound data and length                                   */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail.                                          */
/**********************************************************************************/
int ipanel_porting_sound_play(unsigned int id, const char *buffer, int len)
{
	int index;
	return 0;
}

/**********************************************************************************/
/*Description: Close sound device.                                                */
/*Input      : device handle or id                                                */ 
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail.                                          */
/**********************************************************************************/
int ipanel_porting_sound_close(unsigned int handle)
{
	return 0;
}


/**********************************************************************************/
/*Description: Poll sound data is play completely or not. will send a event Msg to*/
/*             iMW(ipanel MiddleWare) when sound data is play completely.         */
/*Input      : message Event Array                                                */
/*Output     : event message                                                      */
/*Return     : 0 -- no event, 1 -- have event.                                    */
/**********************************************************************************/
int ipanel_porting_sound_poll(unsigned int event[3])
{
	int index;

	return 0;
}


