/*
 * Copyright (C) ALi Shanghai Corp. 2005.
 *
 * si_monitor.h
 *
 * a SI information tracking utility header file.
 */
#ifndef __DVB_SI_MONITOR_H__
#define __DVB_SI_MONITOR_H__
#include <types.h>
#include <sys_config.h>

#include <api/libtsi/db_3l.h>
#include <api/libtsi/p_search.h>
#include <api/libsi/sie_monitor.h>

#ifdef CA_SYSTEM_INFO
typedef void(*on_ca_info_update_t)(void);
void si_monitor_ca_info_register(on_ca_info_update_t ca_info_update);
UINT32 si_monitor_ca_info_get(UINT16 ca_system_id[4], UINT8 *ca_count);
#endif

#ifdef PSI_MONITOR_SUPPORT
//typedef void(*on_pid_change_t)(BOOL need_chchg);
#if(MPEG4_DEFINITION==SYS_FUNC_ON)
typedef void(*mpeg4_exist_t)(PROG_INFO *);
#endif
#ifdef  DYNAMIC_ADD_PROG
typedef void(*add_new_prog_t)(BOOL);
#endif

typedef void (*si_monitor_cb_t)(UINT8 *section, INT32 length);
//typedef INT32 (*section_parse_cb_t)(UINT8 *section, INT32 length, UINT32 param);

INT32 si_monitor_on(UINT32 index);

void si_monitor_off(UINT32 index);

void si_monitor_register(on_pid_change_t pid_change);

INT32 si_monitor_register_cb(si_monitor_cb_t callback);
INT32 si_monitor_register_pmt_cb(section_parse_cb_t callback);
INT32 si_monitor_register_pmt_cb_fta(section_parse_cb_t callback);
#if(MPEG4_DEFINITION==SYS_FUNC_ON)
void mpeg4_monitor_register(mpeg4_exist_t exist);
#endif

#ifdef _MHEG5_ENABLE_
void reset_pat_acquire_flag();
#endif

#else
#define si_monitor_on(...)		do{}while(0)
#define si_monitor_off(...)		do{}while(0)
#define si_monitor_register(...)	do{}while(0)
#define si_monitor_register_cb(...)	do{}while(0)
#endif

#endif /* __DVB_SI_MONITOR_H__ */
