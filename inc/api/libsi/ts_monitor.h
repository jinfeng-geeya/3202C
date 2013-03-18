#ifndef ___TS_MONITOR__H__
#define ___TS_MONITOR__H__

typedef void(*service_changed_cb_t)(DB_TP_ID tp_id);

INT ts_monitor_start(UINT32 index);
INT ts_monitor_stop(void);
void ts_monitor_register(service_changed_cb_t callback);
#endif