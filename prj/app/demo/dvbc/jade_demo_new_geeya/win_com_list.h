#ifndef _WIN_COM_LIST_H_
#define _WIN_COM_LIST_H_
//win_com_list.h


#define COMLIST_MAX_DEP	12
#define MAX_MULTI_SL_ITEMS  6000
#define MAX_ITEM_STR_LEN    30

void win_comlist_reset(void);
void win_comlist_set_frame(UINT16 x, UINT16 y, UINT16 w, UINT16 h);
void win_comlist_set_sizestyle(UINT16 count, UINT16 dep, UINT16 lststyle);
void win_comlist_set_mapcallback(PFN_KEY_MAP listKeyMap, PFN_KEY_MAP winKeyMap, PFN_CALLBACK lstcallback);
void win_comlist_set_align(UINT16 ox, UINT16 oy, UINT8 alignStyle);
void win_comlist_set_str(UINT16 itemIdx, char *str, char *unistr, UINT16 strID);

void win_comlist_clear_sel(void);

void win_comlist_ext_set_ol_frame(UINT16 l, UINT16 t, UINT16 w, UINT16 h);
void win_comlist_ext_set_selmark_xaligen(UINT8 style, UINT8 offset);

void win_comlist_ext_set_title(char *str, char *unistr, UINT16 strID);
void win_comlist_ext_set_title_style(UINT8 idx);
void win_comlist_ext_set_ol_items_style(UINT8 sh_idx, UINT8 hi_idx, UINT8 sel_idx);
void win_comlist_ext_set_ol_items_allstyle(UINT8 sh_idx, UINT8 hi_idx, UINT8 sel_idx, UINT8 gry_idx);
void win_comlist_ext_set_win_style(UINT8 idx);

void win_comlist_ext_set_selmark(UINT16 iconid);

/* For LIST_MULTI_SLECT, return the select flash array */
void win_comlist_ext_get_sel_items(UINT32 **dwSelect);
BOOL win_comlist_ext_check_item_sel(UINT16 index);
void win_comlist_ext_set_item_sel(UINT16 index);
void win_comlist_ext_set_item_cur(UINT16 index);
void win_comlist_ext_enable_scrollbar(BOOL b);
void win_comlist_ext_set_item_attr(UINT16 index, UINT8 attr);

BOOL win_comlist_open(void);
void win_comlist_close(void);


/* Get select item index(for LIST_SINGLE_SLECT) or select item count (for LIST_MULTI_SLECT) */
UINT16 win_comlist_get_selitem(void);

void win_comlist_popup();
void win_comlist_popup_ext(UINT8 *back_saved);

#endif //_WIN_COM_LIST_H_
