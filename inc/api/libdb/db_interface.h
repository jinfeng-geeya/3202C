#ifndef __DB_INTERFACE_H__
#define __DB_INTERFACE_H__


//db interface api
INT32 set_default_value(UINT8 set_mode,UINT32 default_value_addr);
INT32 init_db(UINT32 db_base_addr, UINT32 db_length, UINT32 tmp_info_base, UINT32 tmp_info_len);
INT32 clear_db();

INT32 get_prog_at(UINT16 pos,  P_NODE* node);
INT32 get_prog_by_id(UINT32 pg_id,  P_NODE *node);
INT32 get_tp_by_id(DB_TP_ID tp_id,  T_NODE *node);

INT32 add_node(UINT8 n_type, UINT32 parent_id,void *node);
INT32 modify_prog(UINT32 pg_id,  P_NODE *node);
INT32 modify_tp(DB_TP_ID tp_id,  T_NODE *node);
INT32 del_prog_at(UINT16 pos);
INT32 del_prog_by_id(UINT32 pg_id);
INT32 del_child_prog(UINT8 parent_type, UINT32 parent_id);
INT32 del_tp_by_id(DB_TP_ID tp_id);

UINT16 get_prog_pos(UINT32 pg_id);
INT32 move_prog(UINT16 dest_pos,UINT16 src_pos);

void *get_cur_view(void);
INT32 get_cur_view_feature(UINT16 *create_flag, UINT32 *param);
INT32 recreate_prog_view(UINT16 create_flag, UINT32 param);
INT32 recreate_tp_view(UINT16 create_flag, UINT32 param);

INT32 lookup_node(UINT8 n_type, void *node, UINT32 parent_id);

INT32 db_search_init(UINT8 *search_buf, UINT32 buf_len);
INT32 db_search_create_tp_view(UINT16 parent_id);
INT32 db_search_create_pg_view(UINT8 parent_type,UINT32 parent_id, UINT8 prog_mode);
INT32 db_search_lookup_node(UINT8 n_type, void *node);
INT32 db_search_exit();

INT32 update_data( );
INT32 db_unsave_data_changed();
UINT16 get_prog_num(UINT16 filter_mode,UINT32 param);

INT32 get_specific_prog_num(UINT8 prog_flag,UINT16 *ch_v_cnt,UINT16 *ch_a_cnt,UINT16 *fav_v_cnt,UINT16 *fav_a_cnt);

INT32 set_prog_del_flag(UINT16 pos, UINT8 flag);
BOOL get_prog_del_flag(UINT16 pos);

void get_audio_lang2b(UINT8 *src,UINT8 *des);
void get_audio_lang3b(UINT8 *src,UINT8 *des);

//db interface extension

//UINT16 find_prog_by_name(name_compare_t name_cmp,UINT8 *name, P_NODE *node);
UINT16 find_prog_by_name(UINT8 *name, P_NODE *node);
INT32 get_find_prog_by_pos(UINT16 f_pos, P_NODE *node);
void clear_node_find_flag();

INT32 sort_prog_node(UINT8 sort_flag);
INT32 sort_tp_node(UINT8 sort_flag);
INT32 sort_prog_node_advance(UINT8 sort_flag, UINT32 sort_param);
UINT16 get_provider_num(void *name_buf, UINT32 buf_size);
UINT16 get_cas_num(UINT16*cas_id_buf, UINT32 buf_size);


INT32 update_stream_info(P_NODE *prev, P_NODE *next, INT32 *node_pos);
INT32 undo_prog_modify(UINT32 tp_id,UINT16 prog_number);
#endif

