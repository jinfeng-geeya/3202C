/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2006 Copyright (C)
 *
 *  File: dog.h
 *
 *  Description: This file provide common watchdog interface.
 *
 *  History:
 *      Date        Author      Version  Comment
 *      ====        ======      =======  =======
 *  1.  2006.6.13   Justin Wu   0.1.000  Initial
 ****************************************************************************/

#ifndef	__LLD_DOG_H__
#define __LLD_DOG_H__

#include <sys_config.h>

/* Mode setting */
#define DOG_MODE_WATCHDOG	0x0000
#define DOG_MODE_TIMER		0x0001


#if (SYS_SCI_MODULE == UART16550)

/* Duration and time is in uS, callback is interrupt handler if in timer mode */
#define dog_mode_set(id, mode, duration, callback)	dog_m3327e_set_mode(id, mode, duration, callback)
#define dog_get_time(id)							dog_m3327e_get_time(id)
#define dog_set_time(id, us)						dog_m3327e_set_time(id, us)
#define dog_pause(id, en)							dog_m3327e_pause(id, en)
#define dog_stop(id)								dog_m3327e_stop(id)

#else

#define dog_mode_set(...)							do{}while(0)
#define dog_get_time(...)							0
#define dog_set_time(...)							do{}while(0)
#define dog_pause(...)								do{}while(0)
#define dog_stop(...)								do{}while(0)

#endif


#endif	/* __LLD_DOG_H__ */
