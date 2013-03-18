/*********************************************************************
    Copyright (c) 2005 Embedded Internet Solutions, Inc
    All rights reserved. You are not allowed to copy or distribute
    the code without permission.
    There are the SOUND Porting APIs needed by iPanel MiddleWare. 
    
    Note: the "int" in the file is 32bits
    
    $ver0.0.0.1 $author Zouxianyun 2005/04/28
*********************************************************************/

#ifndef _IPANEL_MIDDLEWARE_PORTING_SOUND_API_FUNCTOTYPE_H_
#define _IPANEL_MIDDLEWARE_PORTING_SOUND_API_FUNCTOTYPE_H_

#ifdef __cplusplus
extern "C" {
#endif

/*initialize sound. you maybe create a new pthread /task to play data.*/
void ipanel_porting_sound_init(void);

/*exit from sound, free all resource in function ipanel_porting_sound_init().*/
void ipanel_porting_sound_exit(void);

/*get STB sound channel parameters*/
int ipanel_porting_sound_get_info(int *numberofbuf, int *buf_size);

int ipanel_porting_sound_set_info(int numberofbuf, int buf_size);

/*play sound data*/
int ipanel_porting_sound_play(unsigned int handle, const char *buffer, int len);

/*poll playing buffer is complete.*/
int ipanel_porting_sound_poll(unsigned int event[3]);

/*close sound device*/
int ipanel_porting_sound_close(unsigned int handle);


/* the following is moved from iPanelTV/include/ipanel_sound.h */
enum{ 
	EIS_AUDIO_PLAY_COMPLETE=0x01F,
	EIS_AUDIO_OPEN,
	EIS_AUDIO_CLOSE,
	EIS_AUDIO_DONE,
	EIS_INAUDIO_OPEN,
	EIS_INAUDIO_CLOSE,
	EIS_INAUDIO_DATA
};

#if 1

#define EIS_WAVE_FORMAT_PCM     1
/* defines for dwFormat field of WAVEINCAPS and WAVEOUTCAPS */
#define EIS_WAVE_INVALIDFORMAT     0x00000000       /* invalid format */
#define EIS_WAVE_FORMAT_1M08       0x00000001       /* 11.025 kHz, Mono,   8-bit  */
#define EIS_WAVE_FORMAT_1S08       0x00000002       /* 11.025 kHz, Stereo, 8-bit  */
#define EIS_WAVE_FORMAT_1M16       0x00000004       /* 11.025 kHz, Mono,   16-bit */
#define EIS_WAVE_FORMAT_1S16       0x00000008       /* 11.025 kHz, Stereo, 16-bit */
#define EIS_WAVE_FORMAT_2M08       0x00000010       /* 22.05  kHz, Mono,   8-bit  */
#define EIS_WAVE_FORMAT_2S08       0x00000020       /* 22.05  kHz, Stereo, 8-bit  */
#define EIS_WAVE_FORMAT_2M16       0x00000040       /* 22.05  kHz, Mono,   16-bit */
#define EIS_WAVE_FORMAT_2S16       0x00000080       /* 22.05  kHz, Stereo, 16-bit */
#define EIS_WAVE_FORMAT_4M08       0x00000100       /* 44.1   kHz, Mono,   8-bit  */
#define EIS_WAVE_FORMAT_4S08       0x00000200       /* 44.1   kHz, Stereo, 8-bit  */
#define EIS_WAVE_FORMAT_4M16       0x00000400       /* 44.1   kHz, Mono,   16-bit */
#define EIS_WAVE_FORMAT_4S16       0x00000800       /* 44.1   kHz, Stereo, 16-bit */

unsigned int ipanel_porting_sound_open(int format, int sampleRate, int bitsperSample, int channels, int optionflag);

#endif // if 0

#ifdef __cplusplus
}
#endif

#endif
