#ifndef _WIN_MUTE_H_
#define _WIN_MUTE_H_

#define MUTE_L 580
#define MUTE_T 68//20//66
#define MUTE_W 40
#define MUTE_H 40
#define MUTE_ICON   IM_MUTE


#define MUTE_STATE    		1
#define UNMUTE_STATE		0

BOOL GetMuteState();
void ShowMuteOSDOnOff(UINT8 flag);
void ShowMuteOnOff(void);
void SetMuteOnOff(BOOL showOsd);
void GetMuteRect(OSD_RECT *rect);
void SetMuteState(BOOL state);

void MuteOSDClear();
void restore_back();
BOOL backup_back();
void mute_onoff_onmenu();

#endif
