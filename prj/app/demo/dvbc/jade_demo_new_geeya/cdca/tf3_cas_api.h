#ifndef _TF3_CAS_API_H_
#define _TF3_CAS_API_H_
#include <api/libge/osd_lib.h>

#define CLEAR_CA_MSG 		0x01
#define CLEAR_CA_FP			0x02
#define CLEAR_CA_TOP_OSD 	0x04
#define CLEAR_CA_BTM_OSD 	0x08
#define CLEAR_MAILICON		0x10

PRESULT ap_cas_message_proc( UINT32 msg_type, UINT32 msg_code);

void ap_tf_cas_routine(POBJECT_HEAD menu);

void ap_tf_check_signal_status(POBJECT_HEAD menu, UINT32 unlock_count);

void ap_tf_cas_clear_screen(UINT32 flag);

#ifdef TF3_OTA_SUPPORT
void ap_tf_cas_ota_init();

void ap_tf_cas_ota_task_resume();

void ap_tf_cas_ota_task_suspend();
#endif

BOOL ap_tf_cas_osd_is_showing();

void ap_tf_cas_load_default();

void ap_tf_cas_process_update(UINT8 process, UINT8 mark);

#endif

