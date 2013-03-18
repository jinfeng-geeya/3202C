#ifndef _SIGNAL_STATUS_H_
#define _SIGNAL_STATUS_H_

typedef enum
{
    SIGNAL_STATUS_UNLOCK = 0,
    SIGNAL_STATUS_LOCK,
} signal_lock_status;

typedef enum
{
    SIGNAL_STATUS_SCRAMBLED = 0,
    SIGNAL_STATUS_UNSCRAMBLED,
} signal_scramble_status;


typedef enum
{
    SIGNAL_STATUS_LNBSHORT = 0,
    SIGNAL_STATUS_LNBNORMAL
} signal_lnbshort_status;

typedef enum
{

    SIGNAL_STATUS_PARENT_UNLOCK = 0,
    SIGNAL_STATUS_PARENT_LOCK
} signal_parentlock_status;

typedef enum
{

    SIGNAL_STATUS_SIGNAL_STOP = 0,
    SIGNAL_STATUS_SIGNAL_START
} signal_stop_status;
BOOL GetSignalStatus(  signal_lock_status* lock_flag,
                        signal_scramble_status* scramble_flag,
                        signal_parentlock_status* parrentlock_flag);

void SetChannelParrentLock(signal_parentlock_status parrentlock_flag);

BOOL GetChannelParrentLock();



void GetSignalStatusRect(OSD_RECT *rect);

void SetChannelNimBusy(UINT32 flag);
void shift_signal_osd(void);
void restore_signal_osd(void);
#endif //_SIGNAL_STATUS_H_
