#ifndef _WIN_SIGNAL_H_
#define _WIN_SIGNAL_H_

void win_signal_open(POBJECT_HEAD w);
void win_signal_close();
void win_signal_hide(void);

BOOL win_signal_set_level_quality(UINT32 level, UINT32 quality, UINT32 lock);
BOOL win_signal_update(void);

void dem_signal_to_display(UINT8 lock, UINT8 *level, UINT8 *quality);
static UINT32 map_ber_value(UINT32 *ber);


#endif //_WIN_SIGNAL_H_
