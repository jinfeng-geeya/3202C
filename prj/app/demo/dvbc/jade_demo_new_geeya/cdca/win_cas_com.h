#ifndef _WIN_CAS_COM_H_
#define _WIN_CAS_COM_H_
#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
//#include <api/libosd/osd_lib.h>

#include "../osdobjs_def.h"
#include "../string.id"
#include "../images.id"
#include "../osd_config.h"
#include "../win_com.h"
#include "../win_com_list.h"
#include "../menus_root.h"
#include "../win_com_popup.h"
#include "../win_com_list.h"
#include "../win_signalstatus.h"


#define	PIN_PWD_LEN		6
#define	MAX_PIN_PWD_ARRAY	3
#define	PIN_POSTION			30//which display_strs[] would be used to save the content

#define	PIN_ARRAY_0			0
#define	PIN_ARRAY_1			1
#define	PIN_ARRAY_2			2

#define DVT_MAXLEN_EMAIL			50

#define PIN_LEN_ERROR  101


extern UINT8	pin_pwd_password_str[MAX_PIN_PWD_ARRAY][PIN_PWD_LEN];

void pin_pwd_set_status(UINT8 bit, UINT8 status);
void pin_pwd_init(UINT8 index);
void pin_pwd_set_str(UINT8 index) ;
void pin_pwd_input(UINT8 item, UINT8 number);
UINT8 pin_pwd_is_same(UINT8 index1, UINT8 index2, UINT32 len);

BOOL ap_cas_wait_sc_ok(UINT32 tmo);
#endif

