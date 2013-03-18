#ifndef _WIN_COM_H_
#define _WIN_COM_H_

#include <api/libpub/lib_pub.h>
#include <hld/osd/osddrv.h>
#include <api/libupg/lib_upg4.h>
#include <api/libupg/ota_upgrade.h>

#ifdef USE_LIB_GE
#include <api/libge/osd_common.h>
#include <api/libge/osd_plugin.h>
#include <api/libge/osd_lib.h>
#include <api/libge/osd_vkey.h>
#else
#include <api/libosd/osd_vkey.h>
#include <api/libosd/osd_lib.h>
#include <api/libosd/osd_common.h>
#endif
#include "osdobjs_def.h"
#include "copper_common/dev_handle.h"
#include "copper_common/com_api.h"
#include "copper_common/menu_api.h"
#include "copper_common/com_epg.h"
#include "copper_common/system_data.h"
#include "copper_common/bat_private.h"

/* Menu Open type & parameter mask */
#define MENU_OPEN_TYPE_MASK		0xFF000000
#define MENU_OPEN_PARAM_MASK		0x000000FF

#define MENU_OPEN_TYPE_KEY 	0xFF000000		/* PARAM : vkey			*/
#define MENU_OPEN_TYPE_MENU 	0xFE000000		/* PARAM : menu item ID */
#define MENU_OPEN_TYPE_STACK	0xFD000000		/* PARAM : null */
#define MENU_OPEN_TYPE_OTHER	0xFC000000		/* PARAM : null */
#define PROG_ID_NOTFOUND (-4)/**/

extern UINT8 cur_tuner_idx;

#define IDE_MOUNT_NAME "/d"
#define USB_MOUNT_NAME "/c"


#ifdef SEARCH_DEFAULT_FTAONLY
#define FATONLY_DEFAULT SEARCH_DEFAULT_FTAONLY
#else
#define FATONLY_DEFAULT 1
#endif

#define TV_OFFSET_L     0
#define TV_OFFSET_T     0

struct help_item_resource
{
	UINT8 part1_type; //0:txt 1:bmp
	UINT16 part1_id;
	UINT16 part2_id;
};

void wincom_open_title(UINT16 title_strid, UINT16 title_bmpid);
void wincom_close_title(void);

void wincom_open_help(struct help_item_resource *helpinfo, UINT8 item_count);
void wincom_close_help(void);

/* For tuner index : see TUNER_TYPE_T*/
UINT16 get_tuner_strid(UINT32 tuner_idx);
UINT32 get_tuner_cnt(void);
UINT16 get_tuner_sat_cnt(UINT16 tuner_idx);
void get_tuner_sat(UINT32 tuner_idx, UINT32 sat_idx, S_NODE *snode);
void get_tuner_sat_name(UINT32 tuner_idx, UINT32 sat_idx, UINT16 *unistr);

void get_tp_name(UINT32 sat_id, UINT32 tp_pos, UINT16 *unistr);
UINT32 get_lnb_type_cnt(void);
UINT32 get_lnb_type_index(UINT32 lnb_type, UINT32 lnb_freq1, UINT32 lnb_freq2);
void get_lnb_type_setting(UINT32 lnb_idx, UINT32 *lnb_type, UINT32 *lnb_freq1, UINT32 *lnb_freq2);
void get_lnb_name(UINT32 lnb_idx, UINT16 *unistr);

UINT32 get_diseqc10_cnt(void);
void get_diseqc10_name(UINT32 index, char *str, UINT16 *unistr, UINT16 *strID);

UINT32 get_diseqc11_cnt(void);
void get_diseqc11_name(UINT32 val, UINT16 *unistr);
UINT32 diseqc11_db_to_ui(UINT32 diseqc11_type, UINT32 diseqc11_port);
void diseqc11_ui_to_db(UINT32 val, UINT8 *type, UINT8 *port);

void get_chan_group_name(char *group_name, UINT8 group_idx, UINT8 *group_type);
void get_cur_group_name(char *group_name, UINT8 *group_type);


//void sat2antenna_ext(S_NODE *sat, struct cc_antenna_info *antenna, UINT32 tuner_idx);

void remove_menu_item(CONTAINER *root, OBJECT_HEAD *item, UINT32 height);
UINT16 win_load_default_satidx();
////////////////////////////////////////////////////////
// sub list (SAT / TP / LNB / DisEqC1.0 )

typedef enum
{
	POP_LIST_TYPE_SAT,  // param :  id - > SAT_POP_LIST_PARAM_T, cur -> current selection or focus
	POP_LIST_TYPE_TP,  // param :  id -> S_NODE's ID			
	POP_LIST_TYPE_LNB,  // param:  id -> S_NODE's ID,
	POP_LIST_TYPE_DISEQC10,  //NULL
	POP_LIST_TYPE_DISEQC11,  //NULL
	POP_LIST_TYPE_CHGRPALL,
	POP_LIST_TYPE_CHGRPFAV,
	POP_LIST_TYPE_CHGRPSAT,
	POP_LIST_TYPE_FAVSET,
	POP_LIST_TYPE_SORTSET,
	POP_LIST_TYPE_RECORDLIST,
	POP_LIST_TYPE_CHANLIST,
	POP_LIST_MENU_LANGUAGE_OSD,
	POP_LIST_MENU_LANGUAGE_STREAM,
	POP_LIST_MENU_TVSYS,
	POP_LIST_MENU_OSDSET,
	POP_LIST_MENU_CA_OPERATOR_INFO,
	POP_LIST_MENU_TIME,
	POP_LIST_MENU_KEYLIST,	
	POP_LIST_MENU_MATURITY_RATING,
	POP_LIST_TYPE_MPSORTSET,
} COM_POP_LIST_TYPE_T;

typedef enum
{
	TUNER_FIRST = 0,
	TUNER_SECOND,
	TUNER_EITHER,	//
	TUNER_EITHER_SELECT,
} TUNER_TYPE_T;

typedef enum
{
	POP_LIST_NOSELECT = 0,	
	POP_LIST_SINGLESELECT,
	POP_LIST_MULTISELECT	
} COM_POP_LIST_SELECT_TYPE_T;


typedef struct
{
	UINT32 id; /* Tuner for satlist / sat ID for tplist */
	UINT32 cur; /* Current selection / focus */
	UINT32 selecttype; /* COM_POP_LIST_SELECT_TYPE_T for satlist only */
	UINT8 *select_array;
} COM_POP_LIST_PARAM_T;

UINT16 win_com_open_sub_list(COM_POP_LIST_TYPE_T type, OSD_RECT *pRect, COM_POP_LIST_PARAM_T *param);
void display_str_init(UINT8 max);
void ForceToScrnDonothing();
void BackToFullScrPlay();
static void wincom_draw_preview(void);
static void wincom_draw_volume_bar();
void restore_scene();
void record_scene();
void del_from_timer(UINT32 prg_id);
inline INT32 find_prg_id_from_index(UINT16 index);
void DrawGrpName(TEXT_FIELD * P_text, UINT8 grp_idx);
void Draw_localGrp_Name(TEXT_FIELD * P_text, UINT8 grp_idx);
void win_clear_trash(UINT16 * start, UINT32 count);
void restore_mutepause();
#ifdef USE_LIB_GE
BOOL wincom_backup_region(POSD_RECT frame,UINT8 *backBuf);
void wincom_restore_region(PGUI_RECT frame,UINT8 *backBuf,BOOL freeBuf);
#else
BOOL wincom_backup_region(POSD_RECT frame, lpVSCR pvscr);
void wincom_restore_region(lpVSCR pvscr);
#endif
void wincom_scroll_set_delay_cnt(INT8 cnt);
void wincom_scroll_textfield_pause(void);
void wincom_scroll_textfield_resume(void);
void wincom_scroll_textfield_stop(PTEXT_FIELD txt);
BOOL wincom_scroll_get_over(PTEXT_FIELD txt);
void wincom_scroll_textfield(PTEXT_FIELD txt);
BOOL getVolumeState();
void setVolumeOn(BOOL input);


/*some marcos*/
#ifndef USE_LIB_GE
#define OSD_GetTextFieldStrPoint(x) ((UINT8 *)(x)->pString)
#endif

#endif //_WIN_COM_H_
