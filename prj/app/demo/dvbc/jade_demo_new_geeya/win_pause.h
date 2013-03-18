#ifndef _WIN_PAUSE_H_
#define _WIN_PAUSE_H_

#include "win_com.h"
#define PAUSE_L 640
#define PAUSE_T 68//66
#define PAUSE_W 40
#define PAUSE_H 40
#define PAUSE_ICON  IM_PAUSE

#define PAUSE_STATE    		1
#define UNPAUSE_STATE		0

BOOL GetPauseState();
void ShowPauseOSDOnOff(UINT8 flag);
void ShowPauseOnOff(void);
void SetPauseOnOff(BOOL showOsd);
void GetPauseRect(OSD_RECT *rect);
void SetPauseStatus(BOOL status);

#endif
